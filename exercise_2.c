/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement odd/even 1D blocking communication scheme 
//       along X axis.
//
// SUMMARY:
//     - 1D splitting along X
//     - Blocking communications
// NEW:
//     - >>> Odd/even communication ordering <<<<
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"
#define SEND_LEFT 0
#define SEND_RIGHT 1

/****************************************************/
void lbm_comm_init_ex2(lbm_comm_t * comm, int total_width, int total_height)
{
	//we use the same implementation then ex1
	lbm_comm_init_ex1(comm, total_width, total_height);
}

/****************************************************/
void lbm_comm_ghost_exchange_ex2(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	//
	// TODO: Implement the 1D communication with blocking MPI functions using
	//       odd/even communications.
	//
	// To be used:
	//    - DIRECTIONS: the number of doubles composing a cell
	//    - double[9] lbm_mesh_get_cell(mesh, x, y): function to get the address of a particular cell.
	//    - comm->width : The with of the local sub-domain (containing the ghost cells)
	//    - comm->height : The height of the local sub-domain (containing the ghost cells)
	
	//example to access cell
	//double * cell = lbm_mesh_get_cell(mesh, local_x, local_y);
	//double * cell = lbm_mesh_get_cell(mesh, comm->width - 1, 0);

	//if the rank_x is odd, we send before receiving
	//if the rank_x is even, we do the opposite
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

}
