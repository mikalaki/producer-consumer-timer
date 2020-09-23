/*
 *	File	: prod-cons_timer.c
 *
 *	Description	: Modify a pthreads based solution to the producer consumer problem using
 *  a timer structure,through which producer threads add function pointers in the common FIFO queue
 *  periodically, and consumers execute the functions added to the queue with the right order.
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
#include "myFunctions.h"

#define QUEUESIZE 1000
#define LOOP 10000
#define DEBUG 1
// #define N_OF_FUNCTIONS 5
#define N_OF_ARGS 10
// #define P 4
// #define Q 4
int P; // number of producer Threads
int Q; // number of consumer Threads


FILE * inQueueWaitingTimes;
FILE * producerAssignDelays;
FILE * actualPeriods;

//Given struct for defining the workFuction our thread will execute.
struct workFunction {
  void * (*work)(void *);
  void * arg;
};



//Consumer and Producers Threads functions declaration.
void *producer (void *args);
void *consumer (void *args);


//Struct defined for the queue implementation
typedef struct {
  struct workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

//queue methods (functions) declaration
queue *queueInit (void);
void queueDelete (queue *q);

void queueAdd (queue *q, struct workFunction in);
void queueExec ( queue *q,struct workFunction  workFunc,int currHead);


//Struct defined for the timer implementation, based on given specification.
typedef struct {
  unsigned int Period; //Period given in milliseconds
  unsigned int TasksToExecute;
  unsigned int StartDelay;
  pthread_t producer_tid;

  struct workFunction * TimerFcn;

  void * (* StartFcn)(void *);
  void * (* StopFcn)(void *);
  void * (* ErrorFcn)(void *);

  void * userData;

  queue *Q;
} Timer;

//timer methods (functions) declarations
Timer * timerInit (unsigned int t_Period,unsigned int t_TasksToExecute,
  unsigned int t_StartDelay,struct workFunction * t_TimerFcn);
void timerDelete(Timer * t);

void * def_StartFcn(void * arg);
void * def_StopFcn(void * arg);
void * def_ErrorFcn(void * arg);

void start(Timer * t);



//Function that prints the execution alternatives menu
int printExecutionMenu();

//Initialization of the workFunctions' fuctions array
void * (* functions[N_OF_FUNCTIONS])(void *)= {&calc5thPower,&calcCos ,&calcSin,&calcCosSumSin,&calcSqRoot};

//Initialization of the workFunctions' arguments array
int arguments[N_OF_ARGS]={ 0    , 10   , 25   , 30 ,45  ,
                    60   , 75   , 90   ,100 ,120 };


/*The startInQueueWaitTimes array, has the same size with the queue.It help us calculate the
waiting time of a workFunction in the queue.When a new item is stored in a queue position (index),
 the time is stored in the same position in the startInQueueWaitTimesArray */
struct timeval startInQueueWaitTimes[QUEUESIZE] ;

/*we get also time in nanoconds for bigger precision(nanosecond precision) where the execution time is
small (for instance 0.8 usec). Generaly we store the measurements from gettimeofday as we are asked,
but when waiting time is less than 5 microseconds we store the measurement fron clock_gettime
convereted in microseconds (usec)*/
struct timespec startInQueueWaitTimes2[QUEUESIZE] ; //getting time is nanoseconds

//variable that holds the number of total fuctions' executions by the consumers
long functionsCounter ;

//variable that hold the mean waiting-time of a function in the queue
double meanWaitingTime;
// long minWaitingTime; // these are computed in matlab
// long maxWaitingTime;

//variable that helps get the termination condition
int terminationStatus;

int main (int argc, char* argv[])
{
  int user_choice;
  int nOftimers=0;
  int timerIndex=0;

  P=atoi(argv[1]);
  Q=atoi(argv[2]);
  printf("TIMER PROGRAMM HAS STARTED!!\n");
  user_choice=printExecutionMenu();
  //Check if user provides an number that doesn't corresponds to a selection.
  while(user_choice!= 1 && user_choice!= 2 && user_choice!= 3 && user_choice!= 4){
    printf("Please enter one valid option.\n" );
    user_choice=printExecutionMenu();
  }

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
  //Getting results in files
  //declaring files' pointers
  FILE  *dataFileMean, *dataFileMax , *dataFileMin , *textFile;


  char filename1[sizeof "InQueueWaitingTimesPXXX_QXXX.csv"];

  //Giving to the file the proper name
  sprintf(filename1, "InQueueWaitingTimesP%03d_Q%03d.csv", P,Q);

  char filename2[sizeof "producerAssignDelaysPXXX_QXXX.csv"];

  //Giving to the file the proper name
  sprintf(filename2, "producerAssignDelaysP%03d_Q%03d.csv", P,Q);

  char filename3[sizeof "actualPeriodsPXXX_QXXX.csv"];

  //Giving to the file the proper name
  sprintf(filename3, "actualPeriodsP%03d_Q%03d.csv", P,Q);


  //Open the file where all the function waiting times of the current execution are stored
  inQueueWaitingTimes = fopen(filename1,"a");
  producerAssignDelays = fopen(filename2,"a");
  actualPeriods = fopen(filename3,"a");


  queue *fifo; //queue declaration
  pthread_t producers[P], consumers[Q];//threads declaration
  Timer * timers[nOftimers];

  //Initializing to zero the two global variables for the mean waiting-time calculations .
  functionsCounter=0;
  meanWaitingTime=0;

  // //Initialize max waiting time equal to zero.
  // maxWaitingTime=0;
  //
  // //Initialize min waiting time equal to zero.
  // minWaitingTime=INFINITY;


  terminationStatus=0;

  fifo = queueInit (); //queue initialization
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  /* We first spawn the consumer threads, in order to be able to execute functions
  as soon as queue is not empty , with the help of conds and mutexes*/
  for (int i = 0; i < Q; i++)
    pthread_create (&consumers[i], NULL, consumer, fifo);

///////////////////initializing the timer


  unsigned int t_periods[3]={1e3,1e2,10};//periods are in milliseconds
  unsigned int t_TasksToExecute[3]={2,20,200};
  unsigned int t_StartDelay=0;
  int argIndex, funcIndex;

  // unsigned int t_periods[P]={1e3,1e2,10};//periods are in milliseconds
  // unsigned int t_TasksToExecute[P];
  // unsigned int t1_StartDelay=0;
  // int argIndex, funcIndex;

  for(int i=0; i<nOftimers; i++){
    //getting the workFuctions' function and argument randomly.
    argIndex=1;
    funcIndex=1+i;
    // Declaring the workFuction that is going to be added in the queue.
    struct workFunction t_TimerFcn;
    t_TimerFcn.work = functions[funcIndex];
    t_TimerFcn.arg = (void *) arguments[argIndex];

    timers[i]= timerInit(t_periods[i+timerIndex], t_TasksToExecute[i+timerIndex], t_StartDelay, &t_TimerFcn);
    (timers[i] -> Q)= fifo;
    (timers[i] -> producer_tid)=producers[i];
    start(timers[i]);

  }

  for(int i=0; i<nOftimers; i++){
    pthread_join ((timers[i]-> producer_tid), NULL);
    (timers[i]-> StopFcn)(NULL);
    // delete timer
    timerDelete(timers[i]);
  }

  // //producers threads spawning
  // for (int i = 0; i < P; i++)
  //   pthread_create (&producers[i], NULL, producer, fifo);
  //
  //
  //
  //
  // //producers' threads joining
  // for (int i = 0; i < P; i++)
  //   pthread_join (producers[i], NULL);

  //consumers' threads joining
  for (int i = 0; i < Q; i++)
    pthread_join (consumers[i], NULL);

  //queue deletion
  queueDelete (fifo);

  printf("\nPROGRAMM EXECUTION FINISHED. \n");

  //printf("For P=%d, and Q=%d ,QUEUESIZE=%d the min waiting-time is : %ld nsec \n \n",P,Q,QUEUESIZE,minWaitingTime);
  // printf("\nFor P=%d, and Q=%d ,QUEUESIZE=%d the min waiting-time is : %f usec \n",P,Q,QUEUESIZE,((float)minWaitingTime )/1000);
  // printf("For P=%d, and Q=%d ,QUEUESIZE=%d the max waiting-time is : %ld usec  \n",P,Q,QUEUESIZE,maxWaitingTime);
  printf("For P=%d, and Q=%d ,QUEUESIZE=%d the mean waiting-time is : %lf usec \n \n",P,Q,QUEUESIZE,meanWaitingTime);


  //open the files that store mean , min and max waiting time of all executions
  dataFileMean=fopen("dataΜΕΑΝ.csv","a");
  // dataFileMin=fopen("dataMIN.csv","a");
  // dataFileMax=fopen("dataMAX.csv","a");
  textFile=fopen("consolePrints.txt","a");

  //printing to files the mean , min and max waiting time of the executions
  fprintf(dataFileMean,"%d,%d,%lf\n",P,Q,meanWaitingTime);
  // fprintf(dataFileMin,"%d,%d,%lf\n",P,Q,((float)minWaitingTime )/1000);
  // fprintf(dataFileMax,"%d,%d,%ld\n",P,Q,maxWaitingTime);


  fprintf(textFile,"\nFor P=%d, and Q=%d ,QUEUESIZE=%d the mean waiting-time is : %lf usec \n ",P,Q,QUEUESIZE,meanWaitingTime);
  // fprintf(textFile,"For P=%d, and Q=%d ,QUEUESIZE=%d the min waiting-time is : %f usec \n ",P,Q,QUEUESIZE,((float)minWaitingTime )/1000);
  // fprintf(textFile,"For P=%d, and Q=%d ,QUEUESIZE=%d the max waiting-time is : %ld usec \n \n",P,Q,QUEUESIZE,maxWaitingTime);


  fclose(inQueueWaitingTimes);
  fclose(producerAssignDelays);
  fclose(actualPeriods);
  fprintf(textFile,"FunctionsCounter: %ld \n ",functionsCounter);

  return 0;
}

//producers' threads function
void *producer (void * t)
{
  struct timeval assignTimestamp;
  struct timeval previousAssignTimestamp;
  unsigned int actualPeriod;
  int drift;

  Timer * timer; //pointer to a queue struct item
  // // int argIndex, funcIndex;

  timer = (Timer *)t;
  unsigned int executions= (timer -> TasksToExecute);
  unsigned int initialPeriod=(timer->Period)*1e3;
  int fixedPeriod= initialPeriod; // Period is in milliseconds
                                           // initialPeriod and fixedPeriod is in microseconds
  //producers' loop
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
    /*
    // // Declaring the workFuction that is going to be added in the queue.
    // struct workFunction  func;
    //
    // //getting the workFuctions' function and argument randomly.
    // argIndex=rand() % N_OF_ARGS;
    // funcIndex=rand() % N_OF_FUNCTIONS;
    //
    //
    // func.work = functions[funcIndex];
    // func.arg = (void *) arguments[argIndex];
    */
    //Get the time the function is assigned.
    gettimeofday(&assignTimestamp,NULL);
    //the function that adds a work function in the queue
    queueAdd ((timer-> Q), *(timer-> TimerFcn));
    pthread_mutex_unlock ((timer-> Q)->mut);
    pthread_cond_signal ((timer-> Q)->notEmpty);
    executions--;


    if(executions< (timer-> TasksToExecute -1)){

      actualPeriod= (assignTimestamp.tv_sec*1e6 -previousAssignTimestamp.tv_sec*1e6);
      actualPeriod+= (assignTimestamp.tv_usec-previousAssignTimestamp.tv_usec);
      drift= actualPeriod- initialPeriod;

      fixedPeriod= (fixedPeriod - drift)> 0 ?fixedPeriod - drift :fixedPeriod   ;
      if(DEBUG)printf("drift is : %d\n",drift);
      if(DEBUG)printf("fixedPeriod is : %d\n",fixedPeriod);
      fprintf(producerAssignDelays,"%f\n ",(float)(drift/1e6));
      fprintf(actualPeriods,"%f\n ",(float)(actualPeriod/1e6));
    }
    previousAssignTimestamp=assignTimestamp;



    if(!executions)
      break;
    usleep((fixedPeriod)); //fixedPeriod is in microseconds
/*
    // //Check if we are in the last rep of producer
    // if(executions< LOOP-1){
    //   pthread_mutex_unlock ((timer-> Q)->mut); //unlocking
    //   pthread_cond_signal ((timer-> Q)->notEmpty); //signal in case cons thread is blocked
    // }
    // If we are in the last repetion of a producer in the loop, we update the terminationStatus value.
    // If terminationStatus variable gets value equal to P (producers' number), means that all producers finished
    // adding functions in the queue.
    // else{
      // terminationStatus++; //updating terminationStatus

    // }
  */
  }
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
    if(DEBUG)
      printf("I am in consumer.\n");
    /*When all the producers finished adding new workFunction items in the queue (terminationStatus==P)
     and the fifo queue is empty , the termination condition is met and the
     consumer Threads return.*/
    while (fifo->empty ) {
      //printf ("consumer: queue EMPTY.\n");

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
queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

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
  //variable to store the waiting time of the current workFunc
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


  // //Check if the current waiting time is the min waiting time.
  // if(currWaitingTime2 < minWaitingTime){
  //   minWaitingTime=currWaitingTime2;
  //
  // }
  //
  // //Check if the current waiting time is the max waiting time.
  // if(currWaitingTime > maxWaitingTime){
  //   maxWaitingTime=currWaitingTime;
  //
  // }

  if(currWaitingTime>=5)
    fprintf(inQueueWaitingTimes,"%ld\n ",currWaitingTime);
  else
    //for small waiting times we use nanoseconds precision
    fprintf(inQueueWaitingTimes, "%lf\n", ((float)currWaitingTime2)/1000);

  //updating global variables that are used for calculating the mean waiting time.
  ++functionsCounter;


  //Updating Head Value for the next consumer thread,before unlocking the mutex
  q->head++;
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

  /*Executing the workFunction function after unlocking the mutex , leads to parallel
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
  if (t == NULL) return (NULL);
  // t -> Q = (queue *)malloc (sizeof (queue));

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
  // free( t -> TimerFcn );
  // free( t -> StartFcn );
  // free( t -> StopFcn );
  // free( t -> ErrorFcn );
  // free( t -> userData );
  free( t );
}

void start(Timer * t){
  if(DEBUG){
    printf("I am inside start(t) , bellow the function:\n");
    (t->TimerFcn->work)((t->TimerFcn->arg));
    printf("DEBUG inside start() ends.\n");
  }
  (t-> StartFcn)(NULL);
  pthread_t t_producer;
  pthread_create (&(t->producer_tid) , NULL, producer, t);

}




void * def_StartFcn(void * arg){
  printf("A timer has started.\n");
}

//NA DW
void * def_StopFcn(void * arg){
  printf("A timer finished assigning new functions in the queue.\n");
}

void * def_ErrorFcn(void * arg){
  printf("The queue is full, no function can be assigned at the moment. \n");
}


int printExecutionMenu(){
  int choice;

  printf("Select one of the programm execution alternatives bellow:\n");
  printf("1. For one timer with period of 1 sec type '1' and press Enter.\n" );
  printf("2. For one timer with period of 0.1 sec type '2' and press Enter.\n" );
  printf("3. For one timer with period of 0.01 sec type '3' and press Enter.\n" );
  printf("4. For running and the three above timers simultaneously type '4' and press Enter.\n" );

  //Check if someone inputs an invalid data type as scanf input.
  if(scanf("%d", &choice)!= 1) {
    scanf("%*s");
    printf("Please enter one valid option. \n" );
    choice=printExecutionMenu();
  };
  return choice;
}
