/*
 *	File	: myFunctions.h
 *
 *	Description	: This file contains the declarations of the functions that are going
 *  to be used in the workFunction struct.
 *
 *	Author : Michael Karatzas
 *	Date  : March 2020
 *
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define PI 3.141592654
#define N_OF_FUNCTIONS 5
#define FUNC_REPS 10

/* Declaration of the workFuctions that consist the fuctions array, their definition
is in the myFunctions.c file*/

//This function calculates the 5th power of the arg number by multiplying it with itself.
void * calc5thPower(void * arg);

//This function calculates 10 cosines staring with the args' one.
void * calcCos(void * arg);

//This function calculates 10 sines staring with the args' one.
void * calcSin(void * arg);

//This function calculates the sum of argument's sine and cosine and prints it.
void * calcCosSumSin(void * arg);

//This function calculates argument's square root and prints it.
void * calcSqRoot(void * arg);





#endif
