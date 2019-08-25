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

#define GET_TIME(x);	if (clock_gettime(CLOCK_MONOTONIC, &(x)) < 0)\
			{ perror("clock_gettime():"); exit(EXIT_FAILURE);} 

const double radius = 1;
double pi;
int circle_points;
int num_threads;

//lock for shared vriables between the threads.

pthread_mutex_t lock;

using namespace std;

//Calculate time from begin to end for the computations

float elapsed_time_msec(struct timespec *begin, struct timespec *end, long *sec,long *nsec)
{
  if (end->tv_nsec < begin->tv_nsec) 
  {
    *nsec = 1000000000 - (begin->tv_nsec - end->tv_nsec);
    *sec = end->tv_sec - begin->tv_sec -1;
  }
  else 
  {
    *nsec = end->tv_nsec - begin->tv_nsec;
    *sec = end->tv_sec - begin->tv_sec;
  }
  return (float) (*sec) * 1000 + ((float) (*nsec)) / 1000000;
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

float pi_serial(){
	pi = 0;
	circle_points = 0;
	double x,y;

	srand(time(NULL));

	//time measurement
  	struct timespec t1, t2;
  	long sec, nsec;
  	float time_elapsed; 

  	GET_TIME(t1);

	for(int i=0; i<nTimes; i++){
		x = (double)random()/(double)RAND_MAX;
		y = (double)random()/(double)RAND_MAX;
		if(in_circle(x,y))
			circle_points++;
	}

	pi = 4* ((double)circle_points/(double)nTimes);

	GET_TIME(t2);

	time_elapsed = elapsed_time_msec(&t1,&t2,&sec,&nsec);

	cout << "Pi (seq) = " << pi << endl;

	cout << "Time elapsed in ms = " << time_elapsed << endl;

	return time_elapsed;
}

////////////////////////////////////////////////////////////////////////////////////

//Parallel implementation

//Code for one thread implementation

void* pi_thread(void* arg){
	double x,y;
	double temp_pi = 0;

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
	
	//temp_pi = (double)thread_circle_points;
	
	pthread_exit(0);
}

//PI computation using threads

float pi_parallel(){
	pi = 0;
	circle_points = 0;
	pthread_t threads[num_threads];

	//time measurement
  	struct timespec t1, t2;
  	long sec, nsec;
  	float total_pthread_time; 
  	
  	GET_TIME(t1);

	//creating required number of threads
	for(int i=0; i<num_threads; i++){

		void* temp = &i;

		pthread_create(&threads[i], NULL, pi_thread, (void*)temp);
	}

	//Collecting all threads back to the main
	for(int i=0; i<num_threads; i++){
		//void *returnvalue;
		pthread_join(threads[i], NULL);
		//circle_points += *(double *)returnvalue;
	}

	//calculating the value of pi

	pi = 4* ((double)circle_points/(double)nTimes);

	//clock_t t2 = clock();

	GET_TIME(t2);
  
  	total_pthread_time = elapsed_time_msec(&t1,&t2,&sec,&nsec);
  	cout << "Pi (pthreads) = " << pi << endl;
  	cout << "Time elapsed in ms = " << total_pthread_time <<endl;

	//cout << "Pi calculated = " << pi <<" " << endl;

	return total_pthread_time;

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

	int itr_for_avg = 30;

	//sequential execution
	if(argc == 2 && strcmp(args[1],"-s")==0){
		double seq_time = 0;

		for(int i=0;i<itr_for_avg;i++)
			seq_time += pi_serial();

		cout << "Avg. Time for Seq. over 30 itrations = " <<seq_time/(double)itr_for_avg<<endl;
	}

	//Parallel Execution

	else if(argc == 3 && strcmp(args[1],"-p")==0)
    {
      double par_time = 0;

      num_threads = atoi(args[2]);
      
      for(int i=0;i<itr_for_avg;i++)
		par_time += pi_parallel();
      
      pthread_mutex_destroy(&lock);
      
      cout << "Avg. Time for Parallel over 30 itrations = " << par_time/(double)itr_for_avg<<endl;
    }

    else 
    {
      cout << "Please provide valid input arguments" << endl;
    }

    out << "Pi computed = " << pi <<endl;

	num_threads = 2;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	num_threads = 4;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	num_threads = 8;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	num_threads = 16;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	num_threads = 32;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	num_threads = 40;
	out << num_threads <<" " << pi_serial() <<" " << pi_parallel() << endl;

	/*for(int i=2; i<40; i = 2*i ){
		float t = pi_parallel();

		num_threads = i;

		cout <<"time for " <<i <<" threads = " << t << endl <<endl;

		//xy_pts_A.push_back(make_pair(t,i));

		out <<i <<" " << t <<endl;

		//num_threads = 2*i;

		//i = 2*i;
	
	}*/

	out.close();

	//gp << "set xrange [0:10] \n";
	gp << "plot \'output.txt\' every ::1 u 1:2 w l title \'Seq\', \'output.txt\' every ::1 u 1:3 w l title \'Parallel\'" <<endl;

	return 0;
}