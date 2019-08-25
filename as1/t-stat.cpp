/*
	Author: Pawan Rajotiya, Computer Science Department, IIT Delhi
	Program: Bootstrap T-statics Calculation sequential and parallel implementations
	
*/

#include <fstream>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string.h>


using namespace std;
//define number of times the sampling is done

#define GET_TIME(x);	if (clock_gettime(CLOCK_MONOTONIC, &(x)) < 0)\
			{ perror("clock_gettime():"); exit(EXIT_FAILURE);} 

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

/////////////////////////////////////////////
//global variables

int nTimes = 1000*1000;
float t_tailed;
int t_count;
int num_threads;
float data1[20];
float data2[20];
pthread_mutex_t lock;

float mean_calc(float data[]){
	float sum = 0.0;
	float mean = 0.0;

	for(int i = 0; i<20 ; ++i){
		sum += data[i];
	}

	mean = sum/20;

	//cout << "mean for this data = " << mean<<endl;

	return mean;
}

float SD_calc(float data[], float mean){
	float stdDeviation = 0.0;

	for(int i=0; i<20 ; ++i){
		float q = data[i] - mean;
		stdDeviation += pow(q, 2);
	}

	float sd = sqrt(stdDeviation/20);

	//cout << "Std deviation  = " << sd <<endl;

	return sd;
}

float Tsat_calc(float m1,float m2,float s1, float s2){
	float m_diff = m2-m1;
	float d = sqrt((pow(s1,2) + pow(s2,2))/20);

	float tstat = m_diff/d;

	//cout << "tstat = " << tstat <<endl;

	return tstat;
}

float* sample_gen(float data[]){
	float *temp = new float[20];

	srand(time(NULL));

	for(int i=0; i<20; i++){
		temp[i] = data[rand()%20 + 1];
	}

	return temp;
}


///////////////////////////////////////////////////////////////////////////
//serial implementation

pair<int, float> t_serial(){
	t_count = 0;

	//time measurement
  	struct timespec t1, t2;
  	long sec, nsec;
  	float time_elapsed; 

  	GET_TIME(t1);

  	//cout <<"serial function entered."<<endl;
  	//cout << nTimes<<endl;

	for(int i=0; i<nTimes; i++){
		float *sample1 = sample_gen(data1);
		float *sample2 = sample_gen(data2);

		float m1 = mean_calc(sample1); //mean for data1
		float m2 = mean_calc(sample2); //mean for data2

		float s1 = SD_calc(sample1,m1); //standard deviation for data1
		float s2 = SD_calc(sample2,m2); //standard deviation for data2

		float t_sample = Tsat_calc(m1,m2,s1,s2);

		if(t_sample >= t_tailed)
			t_count++;

		//cout << "tstat for sample"<< i<<" is  = " << t_sample <<endl<<endl;

	}

	GET_TIME(t2);

	time_elapsed = elapsed_time_msec(&t1,&t2,&sec,&nsec);

	//cout << "Count for t_count = "<<t_count <<endl<<endl;
	//cout << "Time elapsed for serial execution in ms = "<<time_elapsed <<endl<<endl;

	return make_pair(t_count, time_elapsed);
}

//////////////////////////////////////////////////////////////////////

//Parallel Implementation

void* t_thread(void* arg){
	int itr = nTimes/num_threads;

	for(int i=0; i<itr; i++){
		float *sample1 = sample_gen(data1);
		float *sample2 = sample_gen(data2);

		float m1 = mean_calc(sample1); //mean for data1
		float m2 = mean_calc(sample2); //mean for data2

		float s1 = SD_calc(sample1,m1); //standard deviation for data1
		float s2 = SD_calc(sample2,m2); //standard deviation for data2

		float t_sample = Tsat_calc(m1,m2,s1,s2);

		if(t_sample >= t_tailed){
			//using lock to update the shared variable
			pthread_mutex_lock(&lock);
			t_count++;
			pthread_mutex_unlock(&lock);
		}

		//cout << "tstat for sample"<< i<<" is  = " << t_sample <<endl<<endl;

	}

	pthread_exit(0);
}

pair<int, float> t_parallel(){
	pthread_t threads[num_threads];

	t_count = 0;

	//time measurement
  	struct timespec t1, t2;
  	long sec, nsec;
  	float time_elapsed; 

  	GET_TIME(t1);

  	//creating required number of threads
	for(int i=0; i<num_threads; i++){

		void* temp = &i;

		pthread_create(&threads[i], NULL, t_thread, NULL);
	}

	GET_TIME(t2);

	time_elapsed = elapsed_time_msec(&t1,&t2,&sec,&nsec);

	//cout << "Count for t_count = "<<t_count <<endl<<endl;
	//cout << "Time elapsed for parallel execution in ms = "<<time_elapsed <<endl<<endl;

	return make_pair(t_count, time_elapsed);
}


int main(int argc, char *args[]){

	//opening the input file

	ifstream inFile;

	inFile.open("datafile.txt");

	if(!inFile){
		cerr << "Unable to open file datafile.txt";
		//exit(0); //system call to stop
	}

	//reading from file

	float a,b;
	char c;
	int count =0;
	while ((inFile >>a >>c >>b)&&(c==',')) {
		data1[count] = a;
		data2[count] = b;
		count++;
	}

	inFile.close();

	//Processing for one-tailed result

	float m1 = mean_calc(data1); //mean for data1
	float m2 = mean_calc(data2); //mean for data2

	float s1 = SD_calc(data1,m1); //standard deviation for data1
	float s2 = SD_calc(data2,m2); //standard deviation for data2

	t_tailed = Tsat_calc(m1,m2,s1,s2);

	cout << "tstat for whole is  = " << t_tailed <<endl<<endl;

	/*pair<int,float> s = t_serial();

	cout << "Count for serial = "<<s.first <<" Time = "<<s.second<<endl<<endl;

	//Parallel Processing

	num_threads = 4;

	pair<int,float> p = t_parallel();

	cout << "Count for parallel = "<<p.first <<" Time = "<<p.second<<endl<<endl;*/

	int itr_for_avg = 30;

	//sequential execution
	if(argc == 2 && strcmp(args[1],"-s")==0){
		//double seq_time = 0;

		//for(int i=0;i<itr_for_avg;i++){
			pair<int,float> s = t_serial();
			//seq_time += s.second;
		//}

		cout << "Time for Seq. = " <<s.second<<endl;//eq_time/(double)itr_for_avg<<endl;
	}

	//Parallel Execution

	else if(argc == 3 && strcmp(args[1],"-p")==0)
    {
      //double par_time = 0;

      num_threads = atoi(args[2]);
      
      //for(int i=0;i<itr_for_avg;i++){
		pair<int,float> p = t_parallel();
		//par_time += p.second;
	//}
      
      pthread_mutex_destroy(&lock);
      
      cout << "Time for Parallel = " << p.second<<endl;//par_time/(double)itr_for_avg<<endl;
    }

   

    //output for report2.txt
    else if(argc == 2 &&strcmp(args[1],"-o")==0)
    {
    	cout<<"Generating report2.txt"<<endl;
    	ofstream outfile;
    	outfile.open("report2.txt",ios::out);

    	
		for(int i=2; i<33; i=2*i){
			num_threads = i;
			pair<int,float> s = t_serial();//cout<<"parallel function called"<<endl;
    		pair<int,float> p = t_parallel();
    		//cout << num_threads <<" " << s.second <<" " << p.second << endl;
			outfile << num_threads <<" " << s.second <<" " << p.second << endl;
		}

	    num_threads = 40;
	    pair<int,float> s = t_serial();
	    pair<int,float> p = t_parallel();
		outfile << num_threads <<" " << s.second <<" " << p.second << endl;

	    nTimes = 1000*1000*10;

	    for(int i=2; i<33; i=2*i){
			num_threads = i;
			pair<int,float> s1 = t_serial();//cout<<"parallel function called"<<endl;
    		pair<int,float> p1 = t_parallel();
    		//cout << num_threads <<" " << s.second <<" " << p.second << endl;
			outfile << num_threads <<" " << s1.second <<" " << p1.second << endl;
		}

	    num_threads = 40;
	    s = t_serial();
	    p = t_parallel();
		outfile << num_threads <<" " << s.second <<" " << p.second << endl;

		outfile.close();
	}

	 else 
    {
      cout << "Please provide valid input arguments" << endl;
    }

	return 0;
}