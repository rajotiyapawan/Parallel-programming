/*
	Author: Pawan Rajotiya, Computer Science Department, IIT Delhi
	Program: Monte Carlo PI Calculation sequential and parallel implementations

*/


#include <iostream>
#include <math.h>
#include <time.h>
#include <random>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

//number of times the random numbers are generated.

#define nTimes 1000*1000*100

const double radius = 1;
double pi;
int circle_points;

//lock for shared vriables between the threads.

pthread_mutex_t lock;

using namespace std;

float time_elapsed(clock_t t1, clock_t t2){
	float t = (float)(t2 - t1)/CLOCKS_PER_SEC;
	return t;
}

bool in_circle(double x, double y){
	if(pow(x,2) + pow(y,2) <= pow(radius,2)){
		return true;
	}
	else
		return false;
}

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


int main(int argc, char *args[]){

	cout << pi_serial() << endl;
}