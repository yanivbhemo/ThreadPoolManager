#include <iostream>
#include <queue>
#include <pthread.h>
 
using namespace std;
 
class job
{
public:
    job(int id) : jobID(id) {}
    virtual ~job(){}
    //will be overrided by specific JOB
    void virtual working()
    {
        pthread_mutex_lock(&jobLock);
        finished_jobs++;
        pthread_mutex_unlock(&jobLock);
        cout << "JOB:" << jobID << " starts!\n";
        sleep(1);
         
    }
    static int finished_jobs;
    static pthread_mutex_t jobLock;
private:
    int jobID;
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
    cout << "Search for jobs to do" << endl;
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
            oneJob->working();
        delete oneJob;
        oneJob = NULL;
    }
}
 
//============================================================================//
#define THREADS_NUM 5
#define JOBS_NUM 20
int main()
{
    thread_pool *myPool = new thread_pool(THREADS_NUM);
    pthread_t threads[THREADS_NUM];
    myPool->initThreads(threads);
    /*for(int i = 0; i < JOBS_NUM; i++)
    {
        job *newJob = new job(i);
        myPool->assignJob(newJob);
    }*/
	job *newJob = new job(0);
	myPool->assignJob(newJob);
    while(job::finished_jobs < JOBS_NUM);
}
