/*
	Author: Pawan Rajotiya, Computer Science Department, IIT Delhi
	Program: Monte Carlo PI Calculation sequential and parallel implementations

*/


#include <fstream>
#include <map>
#include <vector>
#include <cmath>

#include "gnuplot-iostream/gnuplot-iostream.h"
#include <boost/tuple/tuple.hpp>

#include <iostream>
#include <math.h>
#include <time.h>
#include <random>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

//number of times the random numbers are generated.

#define nTimes 1000*1000*100

const double radius = 1;
double pi;
int circle_points;
int num_threads;

//lock for shared vriables between the threads.

pthread_mutex_t lock;

using namespace std;

//Calculate time from begin to end for the computations

float time_elapsed(clock_t t1, clock_t t2){
	float t = (float)(t2 - t1)/CLOCKS_PER_SEC;
	return t;
}

//Checking whether the points generated are inside the circle

bool in_circle(double x, double y){
	if(pow(x,2) + pow(y,2) <= pow(radius,2)){
		return true;
	}
	else
		return false;
}

//Sequential inplementation

double pi_serial(){
	pi = 0;
	circle_points = 0;
	double x,y;

	srand(time(NULL));

	clock_t t1 = clock();

	for(int i=0; i<nTimes; i++){
		x = (double)random()/(double)RAND_MAX;
		y = (double)random()/(double)RAND_MAX;
		if(in_circle(x,y))
			circle_points++;
	}

	pi = 4* ((double)circle_points/(double)nTimes);

	clock_t t2 = clock();

	cout << "Time elapsed in seconds = " << time_elapsed(t1,t2) <<" " << endl;

	return pi;
}

////////////////////////////////////////////////////////////////////////////////////

//Parallel implementation

//Code for one thread implementation

void* pi_thread(void* arg){
	double x,y;

	//different seed for random generator for each thread
	unsigned int* seed_for_rand = (unsigned int *)&arg;

	//number if iterations for thread
	int itr_pi_thread = nTimes/num_threads;

	int thread_circle_points = 0;

	for(int i=0; i<itr_pi_thread; i++){
		x = (double)rand_r(seed_for_rand)/(double)RAND_MAX;
		y = (double)rand_r(seed_for_rand)/(double)RAND_MAX;

		if (in_circle(x,y))
			thread_circle_points++;
	}

	//using lock to update the shared variable
	pthread_mutex_lock(&lock);
	circle_points += thread_circle_points;
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

//PI computation using threads

float pi_parallel(){
	pi = 0;
	circle_points = 0;
	pthread_t threads[num_threads];

	clock_t t1 = clock();

	//creating required number of threads
	for(int i=0; i<num_threads; i++){

		void* temp = &i;

		pthread_create(&threads[i], NULL, pi_thread, (void*)temp);
	}

	//Collecting all threads back to the main
	for(int i=0; i<num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	//calculating the value of pi

	pi = 4* ((double)circle_points/(double)nTimes);

	clock_t t2 = clock();

	float t = time_elapsed(t1,t2);

	cout << "Pi calculated = " << pi <<" " << endl;

	return t;

}

/////////////////////////////////////////////////////////

//saving data to file for plot
/*
void savedata(const string& filename, int data1, float data2){
	fstream out(filename);
	out <<data1 <<" " << data2 <<endl;
}*/

/////////////////////////////////////////////////////////

int main(int argc, char *args[]){

	ofstream out("output.txt");

	Gnuplot gp;

	//vector<pair<double, double> > xy_pts_A;

	cout << pi_serial() << endl;

	num_threads = 1;

	for(int i=1; i<70; ){
		float t = pi_parallel();

		cout <<"time for " <<i <<" threads = " << t << endl <<endl;

		//xy_pts_A.push_back(make_pair(t,i));

		out <<t <<" " << i <<endl;

		num_threads = 2*i;

		i = 2*i;
	
	}

	out.close();

	//gp << "set xrange [0:10] \n";
	gp << "plot \'output.txt\' u 1:2 w l" <<endl;

	return 0;
}