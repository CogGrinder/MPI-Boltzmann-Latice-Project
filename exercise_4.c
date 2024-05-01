/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement 2D grid communication scheme with
//       8 neighbors using manual copy for non
//       contiguous side and blocking communications
//
// SUMMARY:
//     - 2D splitting along X and Y
//     - 8 neighbors communications
//     - Blocking communications
//     - Manual copy for non continguous cells
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"
#define SEND_LEFT 0
#define SEND_RIGHT 1
#define SEND_UP 2
#define SEND_DOWN 3

/****************************************************/
void lbm_comm_init_ex4(lbm_comm_t * comm, int total_width, int total_height)
{
	//
	// TODO: calculate the splitting parameters for the current task.
	//
	//get infos
	int rank;
	int comm_size;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &comm_size );


	//        calculate the number of tasks along X axis and Y axis.
	comm->nb_x = comm_size/2; //TODO : assuming comm_size is nonzero divisible by 2
	comm->nb_y = 2;

	//        calculate the current task position in the splitting
	comm->rank_x = rank%(comm->nb_x); // we choose horizontal order ie rank_x order
	comm->rank_y = rank/(comm->nb_x);

	//        calculate the local sub-domain size (do not forget the 
	//        ghost cells). Use total_width & total_height as starting 
	//        point.
	comm->width  = total_width /comm->nb_x + 2;
	comm->height = total_height/comm->nb_y + 2;
	//fill in the gap on the last sub-domains :
	if (comm->rank_x == comm->nb_x -1){
		comm->width  += total_width%comm->nb_x;
	}
	if (comm->rank_y == comm->nb_y -1){
		comm->height += total_height%comm->nb_y;
	}



	//        calculate the absolute position  (in cell number) in the global mesh.
	//        without accounting the ghost cells
	//        (used to setup the obstable & initial conditions).
	comm->x = total_width /comm->nb_x * comm->rank_x;
	comm->y = total_height/comm->nb_y * comm->rank_y;
	// printf( " RANK %d ( XDIVISION %d ) (XREST %d ) \n"
	// 		"         ( YDIVISION %d ) (YREST %d ) \n",
	// 		rank,
	// 		total_width /comm->nb_x,
	// 		total_width %comm->nb_x,
	// 		total_height/comm->nb_y,
	// 		total_height%comm->nb_y);

	//OPTIONAL : if you want to avoid allocating temporary copy buffer
	//           for every step :
	//comm->buffer_recv_down, comm->buffer_recv_up, comm->buffer_send_down, comm->buffer_send_up

	//if debug print comm
	#ifndef NDEBUG
	lbm_comm_print( comm );
	#endif
}

/****************************************************/
void lbm_comm_release_ex4(lbm_comm_t * comm)
{
	//free allocated ressources
}

/****************************************************/
int get_rank(lbm_comm_t * comm, int rank_x, int rank_y)
{
	return rank_y * comm->nb_x + rank_x;
}

/****************************************************/
void copy_line_to_buffer(lbm_comm_t * comm, lbm_mesh_t * mesh, double * temp, int y)
{
	for (size_t i = 0; i < comm->width; i++)
	{
		//we fetch the i^th cell ie the i^th x
		double* pointer = lbm_mesh_get_cell(mesh,i,y);

		for (size_t j = 0; j < DIRECTIONS; j++){
			temp[i*DIRECTIONS + j] = *(pointer + j);
		}
	}
}

/****************************************************/
void copy_line_from_buffer(lbm_comm_t * comm, lbm_mesh_t * mesh, double * temp, int y)
{
	for (size_t i = 0; i < comm->width; i++)
	{
		//we fetch the i^th cell ie the i^th x
		double* pointer = lbm_mesh_get_cell(mesh,i,y);

		for (size_t j = 0; j < DIRECTIONS; j++){
			*(pointer + j) = temp[i*DIRECTIONS + j];
		}
	}
}

/****************************************************/
void lbm_comm_ghost_exchange_ex4(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	//
	// TODO: Implement the 2D communication with :
	//         - blocking MPI functions
	//         - manual copy in temp buffer for non contiguous side // TODO : ?
	//
	// To be used:
	//    - DIRECTIONS: the number of doubles composing a cell
	//    - double[9] lbm_mesh_get_cell(mesh, x, y): function to get the address of a particular cell.
	//    - comm->width : The with of the local sub-domain (containing the ghost cells)
	//    - comm->height : The height of the local sub-domain (containing the ghost cells)
	//
	// TIP: create a function to get the target rank from x,y task coordinate. 
	// TIP: You can use MPI_PROC_NULL on borders. // TODO : ?
	// TIP: send the corner values 2 times, with the up/down/left/write communication
	//      and with the diagonal communication in a second time, this avoid
	//      special cases for border tasks.

	//example to access cell
	//double * cell = lbm_mesh_get_cell(mesh, local_x, local_y);
	//double * cell = lbm_mesh_get_cell(mesh, comm->width - 1, 0);

	//TODO:
	//   - implement left/write communications
	//   - implement top/bottom communication (non contiguous)
	//   - implement diagonal communications

	/*********** HORIZONTAL ***********/
	// for horizontal communication, we send the inner sides
	/*
	SENDING
	*/
	//To left ghost
	if (comm->rank_x>0)
	{
		MPI_Ssend( lbm_mesh_get_cell(mesh,1            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x-1,comm->rank_y) , SEND_LEFT  , MPI_COMM_WORLD);
	}

	/*
	RECEIVING
	*/
	//From right ghost aka SEND_LEFT
	if (comm->rank_x<comm->nb_x-1)
	{
		MPI_Recv( lbm_mesh_get_cell(mesh,comm->width-1,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x+1,comm->rank_y) , SEND_LEFT  , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
	}


	/*
	SENDING
	*/
	//To right ghost
	if (comm->rank_x<comm->nb_x-1)
	{
		MPI_Ssend( lbm_mesh_get_cell(mesh,comm->width-2,0) , comm->height * DIRECTIONS , MPI_DOUBLE , 
			get_rank(comm,comm->rank_x+1,comm->rank_y) , SEND_RIGHT , MPI_COMM_WORLD);
	}


	/*
	RECEIVING
	*/
	//From left ghost aka SEND_RIGHT
	if (comm->rank_x>0)
	{
		MPI_Recv( lbm_mesh_get_cell(mesh,0            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x-1,comm->rank_y) , SEND_RIGHT , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
	}



	// TODO : allocate memory and horizontal cache ??
	double* temp = malloc(sizeof(double) * comm->width * DIRECTIONS *2);


	/*********** VERTICAL ***********/
	// for horizontal communication, we send the inner top and bottom
	/*
	SENDING
	*/
	//To upper ghost
	if (comm->rank_y>0)
	{
		copy_line_to_buffer(comm , mesh , temp , 1);
		
		MPI_Ssend( temp , comm->width * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x,comm->rank_y-1) , SEND_UP  , MPI_COMM_WORLD);
	}
	
	
	/*
	RECEIVING
	*/
	//From upper ghost aka SEND_UP
	if (comm->rank_y<comm->nb_y-1)
	{
		MPI_Recv( temp , comm->width * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x,comm->rank_y+1) , SEND_UP  , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		
		copy_line_from_buffer(comm , mesh , temp , comm->height-1);
	}


	/*
	SENDING
	*/
	//To lower ghost
	if (comm->rank_y<comm->nb_y-1)
	{
		copy_line_to_buffer(comm , mesh , temp , comm->height-2);
		
		MPI_Ssend( temp , comm->width * DIRECTIONS , MPI_DOUBLE , 
			get_rank(comm,comm->rank_x,comm->rank_y+1) , SEND_DOWN , MPI_COMM_WORLD);
	}


	/*
	RECEIVING
	*/
	//From lower ghost aka SEND_DOWN
	if (comm->rank_y>0)
	{
		MPI_Recv( temp , comm->width * DIRECTIONS , MPI_DOUBLE ,
			get_rank(comm,comm->rank_x,comm->rank_y-1) , SEND_DOWN , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		
		copy_line_from_buffer(comm , mesh , temp , 0);
	}


	free(temp);
}
