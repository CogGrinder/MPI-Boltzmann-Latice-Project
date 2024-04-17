/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
//
// GOAL: Implement a 1D communication scheme along
//       X axis with blocking communications.
//
// SUMMARY:
//     - 1D splitting along X
//     - Blocking communications
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"
#define SEND_LEFT 0
#define SEND_RIGHT 1

/****************************************************/
void lbm_comm_init_ex1(lbm_comm_t * comm, int total_width, int total_height)
{
	//
	// TODO: calculate the splitting parameters for the current task.
	//
	// HINT: You can look in exercise_0.c to get an example for the sequential case.
	//
	//get infos
	int rank;
	int comm_size;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &comm_size );

	//we must split only in x because y is contiguous in memory

	//        calculate the number of tasks along X axis and Y axis.
	comm->nb_x = comm_size;
	comm->nb_y = 1;

	//        calculate the current task position in the splitting
	comm->rank_x = rank;
	comm->rank_y = 0;

	//        calculate the local sub-domain size (do not forget the 
	//        ghost cells). Use total_width & total_height as starting 
	//        point.
	comm->width = total_width/comm_size + 2;
	comm->height = total_height + 2;
	//fill in the gap on the last sub-domain :
	if (comm->rank_x == comm_size -1)
		comm->width += total_width%comm_size;

	//        calculate the absolute position in the global mesh.
	//        without accounting the ghost cells
	//        (used to setup the obstable & initial conditions).
	comm->x = total_width/comm_size * rank; //TODO is it ?
	printf( " RANK %d ( DIVISION %d ) (REST %d ) \n",
			rank,
			total_width/comm_size,
			total_width%comm_size);

	//reasoning : for each rank, there is overlap with the next,
	//except rank comm_size-1 where it is only with next - making an offset +1
	//in other words, there is an offset +1 due to extra space on the side of rank 0
	//no need to correct last rank due to varied width because width does not change x
	
	/* TODO : is this necessary ?
	if (comm->rank_x > 0)
		comm->x += 1;
	*/


	comm->y = 0;

	//if debug print comm
	#ifndef NDEBUG
	lbm_comm_print( comm );
	#endif
}

/****************************************************/
void lbm_comm_ghost_exchange_ex1(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	//
	// TODO: Implement the 1D communication with blocking MPI functions (MPI_Send & MPI_Recv)
	//
	// To be used:
	//    - DIRECTIONS: the number of doubles composing a cell
	//    - double[DIRECTIONS] lbm_mesh_get_cell(mesh, x, y): function to get the address of a particular cell.
	//    - comm->width : The with of the local sub-domain (containing the ghost cells)
	//    - comm->height : The height of the local sub-domain (containing the ghost cells)
	
	//example to access cell
	//double * cell = lbm_mesh_get_cell(mesh, local_x, local_y);
	//double * cell = lbm_mesh_get_cell(mesh, comm->width - 1, 0);

	
	#ifndef COMMENT

	if (comm->rank_x%2) {
		/*
		SENDING
		*/
		//To left ghost
		if (comm->rank_x>0)
			MPI_Ssend( lbm_mesh_get_cell(mesh,1            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x-1 , SEND_LEFT  , MPI_COMM_WORLD);
		//To right ghost
		if (comm->rank_x<comm->nb_x-1)
			MPI_Ssend( lbm_mesh_get_cell(mesh,comm->width-2,0) , comm->height * DIRECTIONS , MPI_DOUBLE , 
				comm->rank_x+1 , SEND_RIGHT , MPI_COMM_WORLD);

		
		/*
		RECEIVING
		*/

		//From right ghost aka SEND_LEFT
		if (comm->rank_x<comm->nb_x-1)
		{
			MPI_Recv( lbm_mesh_get_cell(mesh,comm->width-1,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x+1 , SEND_LEFT  , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		}
			
		//From left ghost aka SEND_RIGHT
		if (comm->rank_x>0)
		{
			MPI_Recv( lbm_mesh_get_cell(mesh,0            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x-1 , SEND_RIGHT , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		}
	} else {

		/*
		RECEIVING
		*/

		//From right ghost aka SEND_LEFT
		if (comm->rank_x<comm->nb_x-1)
		{
			MPI_Recv( lbm_mesh_get_cell(mesh,comm->width-1,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x+1 , SEND_LEFT  , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		}
			
		//From left ghost aka SEND_RIGHT
		if (comm->rank_x>0)
		{
			MPI_Recv( lbm_mesh_get_cell(mesh,0            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x-1 , SEND_RIGHT , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		}

		/*
		SENDING
		*/
		//To left ghost
		if (comm->rank_x>0)
			MPI_Ssend( lbm_mesh_get_cell(mesh,1            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
				comm->rank_x-1 , SEND_LEFT  , MPI_COMM_WORLD);
		//To right ghost
		if (comm->rank_x<comm->nb_x-1)
			MPI_Ssend( lbm_mesh_get_cell(mesh,comm->width-2,0) , comm->height * DIRECTIONS , MPI_DOUBLE , 
				comm->rank_x+1 , SEND_RIGHT , MPI_COMM_WORLD);


	}

	
	#endif
}
