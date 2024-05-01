/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement non-blocking 1D communication scheme
//       along X axis.
//
// SUMMARY:
//     - 1D splitting along X
// NEW:
//     - >>> Non-blocking communications <<<
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"
#define SEND_LEFT 0
#define SEND_RIGHT 1

/****************************************************/
void lbm_comm_init_ex3(lbm_comm_t * comm, int total_width, int total_height)
{
	//we use the same implementation then ex1
	lbm_comm_init_ex1(comm, total_width, total_height);
}

//bugged
/***************************************************
void copy_column_to_buffer(lbm_comm_t * comm, lbm_mesh_t * mesh, double * temp, int x)
{
	double* pointer = lbm_mesh_get_cell(mesh,x,0);
	for (size_t i = 0; i < comm->height * DIRECTIONS; i++)
	{
		temp[i] = *(pointer + i);
	}
}*/

//bugged
/***************************************************
void copy_column_from_buffer(lbm_comm_t * comm, lbm_mesh_t * mesh, double * temp, int x)
{
	double* pointer = lbm_mesh_get_cell(mesh,x,0);
	for (size_t i = 0; i < comm->height * DIRECTIONS; i++)
	{
		*(pointer + i) = temp[i];
	}
}*/

/****************************************************/
void lbm_comm_ghost_exchange_ex3(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	//
	// TODO: Implement the 1D communication with non-blocking MPI functions.
	//
	// To be used:
	//    - DIRECTIONS: the number of doubles composing a cell
	//    - double[9] lbm_mesh_get_cell(mesh, x, y): function to get the address of a particular cell.
	//    - comm->width : The with of the local sub-domain (containing the ghost cells)
	//    - comm->height : The height of the local sub-domain (containing the ghost cells)
	
	//example to access cell
	//double * cell = lbm_mesh_get_cell(mesh, local_x, local_y);
	//double * cell = lbm_mesh_get_cell(mesh, comm->width - 1, 0);

	int request_count = 0;
	MPI_Request requests[4];

	/*
	SENDING
	*/
	//To left ghost
	if (comm->rank_x>0)
	{
		MPI_Issend( lbm_mesh_get_cell(mesh,1            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			comm->rank_x-1 , SEND_LEFT  , MPI_COMM_WORLD , &requests[request_count++]);
	}
	
	
	/*
	RECEIVING
	*/
	//From right ghost aka SEND_LEFT
	if (comm->rank_x<comm->nb_x-1)
	{
		MPI_Irecv( lbm_mesh_get_cell(mesh,comm->width-1,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			comm->rank_x+1 , SEND_LEFT  , MPI_COMM_WORLD , &requests[request_count++]);
	}

	/*
	SENDING
	*/
	//To right ghost
	if (comm->rank_x<comm->nb_x-1)
	{
		MPI_Issend( lbm_mesh_get_cell(mesh,comm->width-2,0) , comm->height * DIRECTIONS , MPI_DOUBLE , 
			comm->rank_x+1 , SEND_RIGHT , MPI_COMM_WORLD , &requests[request_count++]);
	}

	/*
	RECEIVING
	*/
	//From left ghost aka SEND_RIGHT
	if (comm->rank_x>0)
	{
		MPI_Irecv( lbm_mesh_get_cell(mesh,0            ,0) , comm->height * DIRECTIONS , MPI_DOUBLE ,
			comm->rank_x-1 , SEND_RIGHT , MPI_COMM_WORLD , &requests[request_count++]);
	}

	MPI_Waitall( request_count , requests , MPI_STATUSES_IGNORE);
}
