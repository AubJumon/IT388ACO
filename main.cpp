/*
@author:	Diogo A. B. Fernandes
@contact:	diogoabfernandes@gmail.com
@license:	see LICENSE
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

#define NUMBEROFANTS	(int) 4 //this should be set to the number of threads you want to run with
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

int main() {
	int my_rank, nproc, N_local;

    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &my_rank);

	ACO *LocalANTS = (ACO*)malloc(sizeof(ACO));

	if(my_rank == 0){

		ACO *ANTS = new ACO (NUMBEROFANTS, NUMBEROFCITIES, 
							ALPHA, BETA, Q, RO, TAUMAX,
							INITIALCITY);

		ANTS -> init();
		
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
			ANTS -> setCITYPOSITION (i,  x,  y);
		}
	}

	cout<<"start calculations" << endl;
	
	// ANTS -> setCITYPOSITION(8, 26, 20);
	auto start = std::chrono::high_resolution_clock::now();//add start time

	//ANTS -> printGRAPH ();

	//ANTS -> printPHEROMONES ();

	MPI_Bcast(&ANTS, 1, ACO, 0, comm);

	ANTS -> optimize (ITERATIONS, my_rank);

	if(my_rank == 0){

		auto end = std::chrono::high_resolution_clock::now(); //end timer

		ANTS -> printRESULTS ();
		auto time_taken = duration_cast<milliseconds>(end - start);;
		cout << "Time taken by program is : " << time_taken.count() << " ms " << endl;
	}
	return 0;
}
