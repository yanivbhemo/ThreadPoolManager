#ifndef _MANAGER_H
#define	_MANAGER_H
 
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
 
using namespace std;

class job
{
public:
    job(int id, int code, string userGuess) : jobID(id), code(code) { stringstream toString(userGuess); toString >> guess; result = 1; }
    virtual ~job(){}
    //will be overrided by specific JOB
    int virtual working()
    {
        pthread_mutex_lock(&jobLock);
        finished_jobs++;
        pthread_mutex_unlock(&jobLock);
        cout << "JOB:" << jobID << " starts!\n";
        sleep(1);
        return 0; 
    }
    int compare()
    {
	const char *path = "game_log.txt";
	int flag=1; //0 Bulls-eye, 1 Wrong
	if(code == guess){
		flag = 0;
	}
	
	char buffer[50];
	if(flag==0){
		sprintf(buffer, "Game %d - Code: %d - Guess: %d - Correct!\n", jobID, code, guess); 
		printf("Game %d - Code: %d - Guess: %d - Correct!\n", jobID, code, guess);
	}
	else {
		sprintf(buffer, "Game %d - Code: %d - Guess: %d - Wrong!\n", jobID, code, guess); 
		printf("Game %d - Code: %d - Guess: %d - Wrong!\n", jobID, code, guess); 
	}

	pthread_mutex_lock(&jobLock);
	int logfile_fd = open(path, O_APPEND | O_WRONLY, 0600);
	write(logfile_fd, buffer, strlen(buffer));
	result = flag;
	pthread_mutex_unlock(&jobLock);	
        sleep(1);
	if(flag == 0)
		return 0;
	else
		return 1;	
    }
	int GetCompareResult() { return result; }
    static int finished_jobs;
    static pthread_mutex_t jobLock;
private:
  	int jobID;
	int code;
	int guess;
	int result;
};

class thread_pool
{
public:
    static pthread_mutex_t jobQueue_lock;
    static pthread_cond_t jobQueue_cond;
    thread_pool(){ thread_pool(2); }
    thread_pool(int num) : numOfThreads(num) {}
    virtual ~thread_pool() { while(!jobQueue.empty()) jobQueue.pop(); };
    void initThreads(pthread_t *);
    void assignJob(job *_job_);
    bool loadJob(job *&_job_);
    static void *threadExecute(void *);
private:
    queue<job*> jobQueue;
    int numOfThreads;
};
 
int job::finished_jobs = 0;
 
pthread_mutex_t job::jobLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_pool::jobQueue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_pool::jobQueue_cond = PTHREAD_COND_INITIALIZER;
 
void thread_pool::initThreads(pthread_t *threads)
{
     
    for(int i = 0; i < numOfThreads; i++)
    {
        pthread_create(&threads[i], NULL, &thread_pool::threadExecute, (void *)this);
        cout << "Thread:" << i << " is alive now!\n";
    }
}
 
void thread_pool::assignJob(job* _job_)
{
    pthread_mutex_lock(&jobQueue_lock);
    jobQueue.push(_job_);
    pthread_mutex_unlock(&jobQueue_lock);
    pthread_cond_signal(&jobQueue_cond);
}
 
bool thread_pool::loadJob(job*& _job_)
{
    pthread_mutex_lock(&jobQueue_lock);
    while(jobQueue.empty())
        pthread_cond_wait(&jobQueue_cond, &jobQueue_lock);
    _job_ = jobQueue.front();
    jobQueue.pop();
    pthread_mutex_unlock(&jobQueue_lock);
    return true;
}
 
void *thread_pool::threadExecute(void *param)
{
    thread_pool *p = (thread_pool *)param;
    job *oneJob = NULL;
    while(p->loadJob(oneJob))
    {
        if(oneJob)
            oneJob->compare();
        delete oneJob;
        oneJob = NULL;
    }
}

#endif
