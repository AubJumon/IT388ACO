/*
mpirun -np 5 ./aco
*/

#include <iostream>
#include <cstdlib>
#include <bits/stdc++.h>

#include "mpi.h"

#include <cmath>
#include <limits>
#include <climits>
#include <chrono>

#include "ACO.h"

using namespace std;
using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

#define ITERATIONS		(int) 5

#define NUMBEROFANTS	(int) 5 //this should be set to the number of threads you want to run with
#define NUMBEROFCITIES	(int) 100 //originally 8 create larger number to slow down program

// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA			(double) 0.5
// if (BETA  == 0) { sub-optimal route }
#define BETA			(double) 0.8
// Estimation of the suspected best route.
#define Q				(double) 80
// Pheromones evaporation. 
#define RO				(double) 0.2
// Maximum pheromone random number.
#define TAUMAX			(int) 2

#define INITIALCITY		(int) 0

bool isPrime(int n)
{
    // Corner cases
    if (n <= 1)  return false;
    if (n <= 3)  return true;
   
    // This is checked so that we can skip 
    // middle five numbers in below loop
    if (n%2 == 0 || n%3 == 0) return false;
   
    for (int i=5; i*i<=n; i=i+6)
        if (n%i == 0 || n%(i+2) == 0)
           return false;
   
    return true;
}
 
// Function to return the smallest
// prime number greater than N
int nextPrime(int N)
{
 
    // Base case
    if (N <= 1)
        return 2;
 
    int prime = N;
    bool found = false;
 
    // Loop continuously until isPrime returns
    // true for a number greater than n
    while (!found) {
        prime++;
 
        if (isPrime(prime))
            found = true;
    }
 
    return prime;
}

int main(int argc, char *argv[]) {

		int my_rank, nproc;

    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &my_rank);

	ACO *ANTS = new ACO(nproc, NUMBEROFCITIES, 
		ALPHA, BETA, Q, RO, TAUMAX,
		INITIALCITY);
	ANTS -> init();

	double** local_PHEROMONES = ANTS->getPHEROMONES();

	double** local_CITIES = ANTS->getCITIES();

	int** local_GRAPH = ANTS->getGRAPH();

	if(my_rank == 0){
		
		cout<<"initialize connections" << endl;

		//"randomize" connections of cities, give a seemingly random connection of cities using prime numbers
		for(int i = 0; i < NUMBEROFCITIES - 1; i ++){
			for(int j = i + 1; j <  NUMBEROFCITIES; j++){
				if(j != i+1 || isPrime(j)){
					ANTS -> connectCITIES (i, j);
				}
			}
		}
		cout<<"initialize cities" << endl;

		//create a grid from the cities
		for(int i = 0; i < NUMBEROFCITIES - 1; i ++){
			int x, y = i+1;
			x = (i%10)*5;
			y = (i%10) + i;
			ANTS -> setCITYPOSITION(i, x,y);
		}
		local_PHEROMONES = ANTS->getPHEROMONES();
		local_CITIES = ANTS->getCITIES();
		local_GRAPH = ANTS->getGRAPH();
	}
	
	// ANTS -> setCITYPOSITION(8, 26, 20);
	auto start = std::chrono::high_resolution_clock::now();//add start time

	//ANTS -> printGRAPH ();
	MPI_Barrier(comm);
	MPI_Bcast(local_PHEROMONES, NUMBEROFCITIES*NUMBEROFCITIES, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(local_CITIES, NUMBEROFCITIES*NUMBEROFCITIES, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(local_GRAPH, NUMBEROFCITIES*NUMBEROFCITIES, MPI_INT, 0, MPI_COMM_WORLD);

	if(my_rank != 0){
		ANTS->setPHEROMONES(local_PHEROMONES);
		ANTS->setCITIES(local_CITIES);
		ANTS->setGRAPH(local_GRAPH);
		cout << local_PHEROMONES[0][0] << endl;
	}	
	// ANTS -> setCITYPOSITION(8, 26, 20);

	//ANTS -> printGRAPH ();

	//ANTS -> printPHEROMONES ();

	//ANTS -> optimize (ITERATIONS);

	int* global_ROUTES = (int*)malloc(NUMBEROFANTS*NUMBEROFCITIES * sizeof(int));

	for (int iterations=1; iterations<=ITERATIONS; iterations++) {
		//cout << my_rank << "optimize" <<endl;
		//cout << "ITERATION " << iterations << " HAS STARTED!" << endl << endl;
			//double x = ANTS->optimize(my_rank, ITERATIONS);
			double rlength =  ANTS->optimize(my_rank, iterations);
				double y;
				cout << rlength << endl;
				//cout << y << endl;
			MPI_Reduce(&rlength, &y, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
				cout << " : ant  reduce" << endl;
			//MPI_Bcast(&global_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			// 	cout << " : ant " << my_rank << " BCAST" << endl;
			ANTS->setBESTLENGTH(y);
			int *BESTROUTE = (int*)malloc(NUMBEROFCITIES*sizeof(int));
			for (int i=0; i<NUMBEROFCITIES; i++) {
				int best;
				int temp = (ANTS->getROUTES())[my_rank][i];
				MPI_Reduce(&temp, &BESTROUTE[i], 1, MPI_INT, MPI_MIN, 0, comm);
			}
			ANTS->setBESTROUTE(BESTROUTE);
			
			//cout << " : ant " << my_rank << " has ended!" << endl;
		MPI_Gather(&(ANTS->getROUTES())[my_rank][0], NUMBEROFCITIES, MPI_INT, global_ROUTES, NUMBEROFCITIES, MPI_INT, 0, MPI_COMM_WORLD);
		cout << "gather " << endl;
		// cout << endl
		// 	 << "updating PHEROMONES . . .";
		if(my_rank == 0){
			ANTS->updatePHEROMONES(global_ROUTES);
		}
		// cout << " done!" << endl
		// 	 << endl;
		// printPHEROMONES();

		cout << endl << "ITERATION " << iterations << " HAS ENDED!" << endl << endl;
	}













		auto end = std::chrono::high_resolution_clock::now(); //end timer

		ANTS -> printRESULTS ();
		auto time_taken = duration_cast<milliseconds>(end - start);;
		cout << "Time taken by program is : " << time_taken.count() << " ms " << endl;
	
	return 0;
}
