/*
 *	File	: prod-cons_timer.c
 *
 *	Description	: Create a C timer using a multithreading producer consumers solution,
 *  we developed in the previous class assignment.
 *
 *	Author : Michael Karatzas
 *	Date  :  September 2020
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "myFunctions.h"


#define DEBUG 1
// #define N_OF_FUNCTIONS 5
#define N_OF_ARGS 10
// #define P 4
// #define Q 4
// #define QUEUESIZE 1000

int P=1; // number of producer Threads, set it by default to 1, value will be updated in the programm
int Q=1; // number of consumer Threads, set it by default to 1, value will be updated in the programm
int QUEUESIZE=10; //work Functions queue capacity, set it by default to 10, value will be updated in the programm


//pointers to auxilary files, for storing wanted data.
FILE * inQueueWaitingTimes;
FILE * producerAssignDelays;
FILE * actualPeriods;
//FILE * executionTime;
FILE * errorFile;

//Given struct for defining the workFuction our consumer threads will execute.
struct workFunction {
  void * (*work)(void *);
  void * arg;
};



//Consumer and Producers Threads' functions declaration.
void *producer (void *args);
void *consumer (void *args);


//Struct defined for the queue implementation
typedef struct {
  struct workFunction * buf;
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

//queue methods (functions) declaration
queue *queueInit (int capacity);
void queueDelete (queue *q);

void queueAdd (queue *q, struct workFunction in);
void queueExec ( queue *q,struct workFunction  workFunc,int currHead);


//Struct defined for the timer implementation, based on given specification.
typedef struct {
  unsigned int Period; //Period given in milliseconds
  unsigned int TasksToExecute; //How many times the workFunc will by executed by times
  unsigned int StartDelay;// StartDellay is in seconds, the initial dellay of workFunc execution
  pthread_t producer_tid; // the producer thread id correspoding to timer

  struct workFunction * TimerFcn;// TimerFcn

  void * (* StartFcn)(void *);
  void * (* StopFcn)(void *);
  void * (* ErrorFcn)(void *);

  void * userData;

  queue *Q;
} Timer;

//timer methods (functions) declarations, based on given specification
Timer * timerInit (unsigned int t_Period,unsigned int t_TasksToExecute,
  unsigned int t_StartDelay,struct workFunction * t_TimerFcn);
void timerDelete(Timer * t);

void * def_StartFcn(void * arg);
void * def_StopFcn(void * arg);
void * def_ErrorFcn(void * arg);

void start(Timer * t);
void startat(Timer * t,int y,int m,int d,int h,int min,int sec);


//Function that prints the execution alternatives menu
int printExecutionMenu();

//Initialization of the workFunctions' fuctions array (from previous assignment)
void * (* functions[N_OF_FUNCTIONS])(void *)= {&calc5thPower,&calcCos ,&calcSin,&calcCosSumSin,&calcSqRoot};

//Initialization of the workFunctions' arguments array (from previous assignment)
int arguments[N_OF_ARGS]={ 0    , 10   , 25   , 30 ,45  ,
                    60   , 75   , 90   ,100 ,120 };


/*The startInQueueWaitTimes array, will be allocated to have the same length with the queue.It help us calculate the
waiting time of a workFunction in the queue.When a new item is stored in a queue position (index),
the time is stored in the same position(index) in the startInQueueWaitTimesArray */
struct timeval * startInQueueWaitTimes ;

/*we get also time in nanoseconds for bigger precision(nanosecond precision) where the waiting time is
small (for instance 0.8 usec). Generaly we store the measurements from gettimeofday as we are asked,
but when waiting time is less than 5 microseconds we store the measurement fron clock_gettime
convereted in microseconds (usec), this is achieved by the bellow variable.*/
struct timespec * startInQueueWaitTimes2 ;

//variable that holds the number of total fuctions' executions by the consumers
long functionsCounter ;

//variable that hold the mean waiting-time of a function in the queue
double meanWaitingTime;

//initializing the TotalDrift variables , of all executions to 0
int TotalDrift=0;

//variable that helps get the termination condition
int terminationStatus;

int main (int argc, char* argv[])
{
  //varable that indicates the execution alternative,user chose
  int user_choice;
  //initialiazing variables to 0.
  int nOftimers=0;
  int timerIndex=0;

  //getting queue capacity and number of consumer threads as command arguments
  QUEUESIZE=atoi(argv[1]);
  Q=atoi(argv[2]);

  printf("TIMER PROGRAMM HAS STARTED!!\n");

  //Print the menu with the 4 program execution alternatives available
  user_choice=printExecutionMenu();

  //Check if user provides an number that doesn't corresponds to a execution selection.
  while(user_choice!= 1 && user_choice!= 2 && user_choice!= 3 && user_choice!= 4){
    printf("Please enter one valid option.\n" );
    user_choice=printExecutionMenu();
  }

  //Set the program variables in respect to the user choice
  switch (user_choice) {
    case 1:
      nOftimers=1;
      P=nOftimers;
      timerIndex=0;
      break;

    case 2:
      nOftimers=1;
      P=nOftimers;
      timerIndex=1;
      break;

    case 3:
      nOftimers=1;
      P=nOftimers;
      timerIndex=2;
      break;

    case 4:
      nOftimers=3;
      P=nOftimers;
      timerIndex=0;
      break;

    default:
      break;

  }


  //declaring files' pointers
  FILE  *dataFileMean, *textFile;


  //In this file we save every waiting-time of a function inside the queue
  char filename1[sizeof "InQueueWaitingTimesQUEUESIZEXX_QXXX_caseX.csv"];
  //Giving to the csv file the proper name
  sprintf(filename1, "InQueueWaitingTimesQUEUESIZE%02d_Q%03d_case%d.csv", QUEUESIZE,Q,user_choice);

  //In this file we save every dellay to the desirable Period of the producer.
  char filename2[sizeof "producerAssignDelaysQUEUESIZEXX_QXXX_caseX.csv"];
  //Giving to the csv file the proper name
  sprintf(filename2, "producerAssignDelaysQUEUESIZE%02d_Q%03d_case%d.csv", QUEUESIZE,Q,user_choice);

  //In this file we get the producer's actual Periods between 2 assignments in queue
  char filename3[sizeof "actualPeriodsQUEUESIZEXX_QXXX_caseX.csv"];
  //Giving to the file the proper name
  sprintf(filename3, "actualPeriodsQUEUESIZE%02d_Q%03d_case%d.csv", QUEUESIZE,Q,user_choice);

  //In this file an error message is printed if the queue gets full.
  char filename4[sizeof "errorFileQUEUESIZEXX_QXXX_caseX.txt"];
  //Giving to the file the proper name
  sprintf(filename4, "errorFileQUEUESIZE%02d_Q%03d.txt", QUEUESIZE,Q,user_choice);


  //Open the auxilary files, in order to store the wanted data
  inQueueWaitingTimes = fopen(filename1,"a");
  producerAssignDelays = fopen(filename2,"a");
  actualPeriods = fopen(filename3,"a");
  errorFile = fopen(filename4,"a");


  queue *fifo; //queue declaration
  pthread_t producers[P], consumers[Q];//threads declaration
  Timer * timers[nOftimers];

  //Initializing to zero the two global variables for the mean waiting-time calculations .
  functionsCounter=0;
  meanWaitingTime=0;

  //initializing terminationstatus to zero, if it gets equal to P, then it is met.
  terminationStatus=0;

  fifo = queueInit (QUEUESIZE); //queue initialization
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  //Allocating heap space for the arrays that will store the starts of waiting times in queue
  startInQueueWaitTimes = (struct timeval *) malloc (sizeof(struct timeval)*QUEUESIZE);
  startInQueueWaitTimes2 = (struct timespec *) malloc (sizeof(struct timespec)*QUEUESIZE);
  if(  (!startInQueueWaitTimes) || (!startInQueueWaitTimes2)  ){
    printf("Couldn't Allocate Memory!\n" );
    exit(1);
  }

  /* We first spawn the consumer threads, in order to be able to execute functions
  as soon as queue is not empty , with the help of conds and mutexes*/
  for (int i = 0; i < Q; i++)
    pthread_create (&consumers[i], NULL, consumer, fifo);

  //Setting the timer parameters
  unsigned int t_periods[3]={1e3,1e2,10};//timer periods are in milliseconds
  unsigned int t_TasksToExecute[3]={10,10,10};
  unsigned int t_StartDelay=0;// default initial timer's dellay is zero
  int argIndex, funcIndex; //variables that indicate the function and the argument

  //With the loop we achive initialization of all the desired timers.
  for(int i=0; i<nOftimers; i++){
    //getting the workFuctions' function and argument randomly.
    argIndex=1+i;
    funcIndex=1;

    // Setting the timerFcn function and it arguments.
    struct workFunction t_TimerFcn;
    t_TimerFcn.work = functions[funcIndex];
    t_TimerFcn.arg = (void *) arguments[argIndex];

    //initializeing the timer.
    timers[i]= timerInit(t_periods[i+timerIndex], t_TasksToExecute[i+timerIndex], t_StartDelay, &t_TimerFcn);
    (timers[i] -> Q)= fifo;
    (timers[i] -> producer_tid)=producers[i];

    // //beggining the timer timers[i]
    // start(timers[i]);

    //beggining the timer timers[i] in d/m/y h:min:sec with startat(timers[i],y,m,d,h,min,sec)
    startat(timers[i],2020,9,25,21,10,0);
  }

  //With this loop, we are joining the timers' prod threads and we delete the timer objects, when they are not needed.
  for(int i=0; i<nOftimers; i++){
    pthread_join ((timers[i]-> producer_tid), NULL);
    (timers[i]-> StopFcn)(NULL);
    // delete timer
    timerDelete(timers[i]);
  }


  //consumers' threads joining
  for (int i = 0; i < Q; i++)
    pthread_join (consumers[i], NULL);

  //queue deletion
  queueDelete (fifo);

  printf("\nPROGRAMM EXECUTION FINISHED. \n");


  printf("For P=%d, and Q=%d ,QUEUESIZE=%d the mean waiting-time is : %lf usec \n \n",P,Q,QUEUESIZE,meanWaitingTime);


  //open the files that store mean waiting time and the file when we get the programms output.
  dataFileMean=fopen("dataΜΕΑΝ.csv","a");
  textFile=fopen("consolePrints.txt","a");

  //printing to files the mean waiting time of the executions
  fprintf(dataFileMean,"%d,%d,%lf\n",QUEUESIZE,Q,meanWaitingTime);



  fprintf(textFile,"\nFor P=%d, and Q=%d ,QUEUESIZE=%d the mean waiting-time is : %lf usec \n ",P,Q,QUEUESIZE,meanWaitingTime);

  //Closing the files previously opened
  fclose(inQueueWaitingTimes);
  fclose(producerAssignDelays);
  fclose(actualPeriods);
  fclose(errorFile);
  fprintf(textFile,"FunctionsCounter: %ld \n ",functionsCounter);

  //free memory alocated in the heap
  free(startInQueueWaitTimes);
  free(startInQueueWaitTimes2);

  return 0;
}

//producers' threads function
void *producer (void * t)
{
  //Declaration of auxilary time structs for calculating the producer statistics.
  struct timeval assignTimestamp;
  struct timeval previousAssignTimestamp;
  unsigned int actualPeriod;


  int currentPeriodDeclination=0;


  Timer * timer; //pointer to a queue struct item
  // // int argIndex, funcIndex;

  timer = (Timer *)t;
  unsigned int executions= (timer -> TasksToExecute);
  unsigned int initialPeriod=(timer->Period)*1e3;
  int fixedPeriod= initialPeriod; // Period is in milliseconds
                                  // initialPeriod and fixedPeriod is in microseconds

  // Sleep untill the wanted time moment (d/m/y h:min:sec)
  if(timer->StartDelay){

    sleep(timer->StartDelay);

    printf("A timer is starting now as scheduled, with the respect to its startDelay!\n");
  }

  //producers' loop, equal to
  while (executions)
  {
    pthread_mutex_lock ((timer-> Q)->mut);
    if(DEBUG)
      printf("I am in producer.\n");

    while ((timer-> Q)->full) {
      //printf ("producer: queue FULL.\n");
      timer->ErrorFcn(NULL);
      pthread_cond_wait ((timer-> Q)->notFull, (timer-> Q)->mut);

    }

    //Get the time the function is assigned in the queue.
    gettimeofday(&assignTimestamp,NULL);
    //the function that adds a work function in the queue
    queueAdd ((timer-> Q), *(timer-> TimerFcn));
    //unlock the mutex and signaling the consumers that queue is not empty
    pthread_mutex_unlock ((timer-> Q)->mut);
    pthread_cond_signal ((timer-> Q)->notEmpty);

    //total executions of timer- producer assgments is decreased by one
    executions--;

    //For the 2nd assigment of producer and beyond, we calcute period and period dellay.
    if(executions< (timer-> TasksToExecute -1)){

      actualPeriod= (assignTimestamp.tv_sec*1e6 -previousAssignTimestamp.tv_sec*1e6);
      actualPeriod+= (assignTimestamp.tv_usec-previousAssignTimestamp.tv_usec);
      currentPeriodDeclination= actualPeriod- initialPeriod;

      fixedPeriod= (fixedPeriod - currentPeriodDeclination)> 0 ?fixedPeriod - currentPeriodDeclination :fixedPeriod   ;
      if(DEBUG)printf("currentPeriodDeclination is : %d\n",currentPeriodDeclination);
      if(DEBUG)printf("fixedPeriod is : %d\n",fixedPeriod);

      //Print the current Period (assign) dellay and the actual period vallue, to the proper files in seconds
      fprintf(producerAssignDelays,"%f\n ",(float)(currentPeriodDeclination/1e6));
      fprintf(actualPeriods,"%f\n ",(float)(actualPeriod/1e6));
    }
    previousAssignTimestamp=assignTimestamp;



    if(!executions)
      break;
    usleep((fixedPeriod)); //fixedPeriod is in microseconds

  }
  //if a producer finished assigns functions in queue, it indicates it with increase of terminationStatus variavle.
  terminationStatus++;
  //The bellow false signing is dor avoid race conditions
  pthread_cond_signal ((timer-> Q)->notEmpty);
  return (NULL);
}

//consumers' threads function
void *consumer (void *q)
{
  queue *fifo;
  fifo = (queue *)q;

  while (1) {

    pthread_mutex_lock (fifo->mut);

    /*When all the producers finished adding new workFunction items in the queue (terminationStatus==P)
     and the fifo queue is empty , the termination condition is met and the
     consumer Threads return.*/
    while (fifo->empty ) {

      //Termination condition,terminationStatus ==P means that all producers finished adding workFuncs in the queue.
      if(terminationStatus ==P){

        //Unlock the mutex before termination in order all the producers threads to terminate.
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);
        printf("IAM CON: All produced functions are executed, producer's function,unlocks and returns. \n");
        return NULL;
      }

      pthread_cond_wait (fifo->notEmpty, fifo->mut);

    }
    //variable to store the function the producer thread will execute, after unlocking
    struct workFunction execFunc;
    //The head value that corresponds to the queue position of the workFunction thread will execute
    long currHead;

    currHead=fifo->head;
    execFunc=fifo->buf[currHead];

    queueExec (fifo,execFunc,currHead);


  }
  return (NULL);
}

//Queue Initialization Function
queue *queueInit (int capacity)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  //Allocating size for the queue buffer.
  q->buf=(struct workFunction *) malloc (sizeof(struct workFunction)*capacity);
  if(!(q->buf)){
    printf("Couldn't Allocate Memory!\n" );
    exit(1);
  }
  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);

  return (q);
}

//Queue Destroy Function
void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

//Method that adds a new workFunction in the queue
void queueAdd (queue *q, struct workFunction in)
{

  //A new workFunction is added in the queue
  q->buf[q->tail] = in;

  // THE BEGINNING of the WAITING TIME is after the workFunction is added in the queue.
  gettimeofday(&startInQueueWaitTimes[q->tail],NULL);
  clock_gettime(CLOCK_MONOTONIC, &startInQueueWaitTimes2[q->tail]);

  q->tail++;
  if(DEBUG)printf("QUEUSIZE inside queueADD= %d \n ",QUEUESIZE);
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueExec ( queue *q,struct workFunction  workFunc,int currHead)
{


  ////TIME CALCULATIONS/////
  //variables to store the waiting time of the current workFunc
  long currWaitingTime =0 ;
  long currWaitingTime2=0 ;

  //variable to get the time that workFunction is getting out of the queue( before execution)
  struct timeval endTime;
  struct timespec endTime2;




  //The END of the waiting time , is the moment exactly before the function is executed.
  gettimeofday(&endTime,NULL);
  clock_gettime(CLOCK_MONOTONIC, &endTime2);

  //calculating waiting time in microseconds.
  currWaitingTime= (endTime.tv_sec*1e6 -(startInQueueWaitTimes[currHead] ).tv_sec*1e6);
  currWaitingTime+= (endTime.tv_usec-(startInQueueWaitTimes[currHead] ).tv_usec);




  //calculating waiting time in nanoseconds.
  currWaitingTime2=(endTime2.tv_sec-(startInQueueWaitTimes2[currHead ]).tv_sec) * 1e9  ;
  currWaitingTime2+=(endTime2.tv_nsec-(startInQueueWaitTimes2[currHead ]).tv_nsec  );

  //Printing the waiting time of a function in the queue to the proper file, is useconds(microseconds)
  if(currWaitingTime>=5)
    fprintf(inQueueWaitingTimes,"%ld\n ",currWaitingTime);
  else
    //for small waiting times we use nanoseconds precision
    fprintf(inQueueWaitingTimes, "%lf\n", ((float)currWaitingTime2)/1000);

  //updating global variable that is used for calculating the mean waiting time.
  ++functionsCounter;


  //Updating Head Value for the next consumer thread,before unlocking the mutex
  q->head++;
  if(DEBUG)printf("QUEUSIZE inside queueEXEC= %d \n ",QUEUESIZE);
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  //Updating the mean waiting time of a function value
  meanWaitingTime= (meanWaitingTime*(functionsCounter-1) + (double)currWaitingTime )/(functionsCounter) ;




  //Unlocking the mutex
  pthread_mutex_unlock (q->mut);
  pthread_cond_signal (q->notFull);

  /*Executing the workFunction function after unlocking the mutex ,this leads to parallel
  execution of workFunction functions*/
  (workFunc.work)((workFunc.arg));

  return;

}


//Timer Initialization Function
Timer * timerInit (  unsigned int t_Period,
  unsigned int t_TasksToExecute,
  unsigned int t_StartDelay,
  struct workFunction * t_TimerFcn){

  Timer *t;

  t = (Timer *)malloc (sizeof (Timer));
  if (!t){
    printf("Failed to allocate memory for the timer!");
    return (NULL);
  }

  t->Period = t_Period;
  t->TasksToExecute = t_TasksToExecute;
  t->StartDelay = t_StartDelay;

  t->TimerFcn = t_TimerFcn;

  t->StartFcn = &def_StartFcn ;
  t->StopFcn = &def_StopFcn;
  t->ErrorFcn = &def_ErrorFcn;

  t->userData = NULL;

  return (t);
}

void timerDelete(Timer * t){

  //free the timer pointer
  free( t );
}

void start(Timer * t){

  (t-> StartFcn)(NULL);
  pthread_t t_producer;
  pthread_create (&(t->producer_tid) , NULL, producer, t);

}

void startat(Timer * t,int y,int m,int d,int h,int min,int sec){


  int totalDelayInSeconds=0;

  struct timeval tv;
  time_t nowtime;
  struct tm *nowtm;

  //get the current timeStamp
  gettimeofday(&tv, NULL);
  nowtime = tv.tv_sec;
  nowtm = localtime(&nowtime);

  //Print informational message about the timer beggining in a certain time and date.
  printf("The time now is: %d-%02d-%02d %02d:%02d:%02d\n", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
  printf("A timer is set to execute at: %d-%02d-%02d %02d:%02d:%02d\n", y, m, d, h, min, sec);

  //Initializing a struct tm with the timestamp, at which the timer will be executed.
  struct tm * timerExecStart = (struct tm *)malloc(sizeof(struct tm));
  timerExecStart->tm_year = y -1900;
  timerExecStart->tm_mon = m -1;
  timerExecStart->tm_mday = d;
  timerExecStart->tm_hour = h;
  timerExecStart->tm_min = min;
  timerExecStart->tm_sec = sec;

  //Getting the time difference in seconds between the current moment and the moment the timer we want to start.
  totalDelayInSeconds=(int) difftime( mktime(timerExecStart),mktime(nowtm));

  printf("inside startat() ,totalDelayInSeconds: %d \n",totalDelayInSeconds);

  //Initialize the timer's initial dellay (StartDelay variable) to the proper value.
  if(totalDelayInSeconds>0)
    t-> StartDelay = totalDelayInSeconds; //startDelay is in seconds

  pthread_t t_producer;

  //Spawining the producer threead corresponding to timer
  pthread_create (&(t->producer_tid) , NULL, producer, t);
  if(DEBUG)printf("i created the prod trhead!\n");
}




void * def_StartFcn(void * arg){
  printf("A timer is set for execution.\n");
}


void * def_StopFcn(void * arg){
  printf("A timer finished assigning new functions in the queue.\n");
}

void * def_ErrorFcn(void * arg){
  printf("The queue is full, no function can be assigned at the moment. \n");

  struct timeval tv;
  time_t nowtime;
  struct tm *nowtm;
  gettimeofday(&tv, NULL);
  nowtime = tv.tv_sec;
  nowtm = localtime(&nowtime);

  //we print the error status in the error file and the time the error (full queue) reported.
  fprintf(errorFile, "ERROR: QUEUE IS FULL, time is :%d-%02d-%02d %02d:%02d:%02d\n", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec );

}

//Definition of the function that prints the user menu.
int printExecutionMenu(){
  int choice;

  printf("Select one of the programm execution alternatives bellow:\n");
  printf("1. For one timer with period of 1 sec type '1' and press Enter.\n" );
  printf("2. For one timer with period of 0.1 sec type '2' and press Enter.\n" );
  printf("3. For one timer with period of 0.01 sec type '3' and press Enter.\n" );
  printf("4. For running and the three above timers simultaneously type '4' and press Enter.\n" );

  //Check if someone inputs an invalid data type as scanf input .
  if(scanf("%d", &choice)!= 1) {
    scanf("%*s");
    printf("Please enter one valid option. \n" );
    choice=printExecutionMenu();
  };
  //return the user choice.
  return choice;
}
