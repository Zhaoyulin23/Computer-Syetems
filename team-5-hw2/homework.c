/* 
 * file:        homework.c
 * description: Skeleton code for CS 5600 Homework 2
 *
 * Peter Desnoyers, Northeastern CCIS, 2011
 * $Id: homework.c 530 2012-01-31 19:55:02Z pjd $
 */

#include <stdio.h>
#include <stdlib.h>
#include "hw2.h"

/********** YOUR CODE STARTS HERE ******************/

/*
 * Here's how you can initialize global mutex and cond variables
 */
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t C = PTHREAD_COND_INITIALIZER;

pthread_cond_t BARBER = PTHREAD_COND_INITIALIZER;
pthread_cond_t CUSTOMER = PTHREAD_COND_INITIALIZER; 
pthread_cond_t BARBERCHAIR = PTHREAD_COND_INITIALIZER;

int N = 10;                    // Number of Customers 
int BarberNum = 1;             // Number of Barbers
int TakenSeats = 0;            // Number of waiting seats has been taken
int TotalSeats = 4;            // Total numbers of seats 
int ShopOpen = 0;              
double T1 = 10.0;             

/* Q3: Declare counters and timers */
void *c_TotalCustomers = NULL;              // q3-1
void *c_TurnAwayCustomers = NULL ;          // q3-1
void *c_InShopCustomers = NULL;             // q3-3
void *c_SitCustomers = NULL;                // q3-4
void *t_PerCustomer = NULL;                 // q3-2

void BarberIsBusy(int customer_num)
{
    // printf("%f customer %d is cutting hair\n", 
    //                          timestamp(), customer_num);
    sleep_exp(1.2, &m);
    // printf("%f Barber done!, customer %d stands up\n", 
    //                          timestamp(), customer_num);
}

/* the barber method
 */
void barber(void)
{
    pthread_mutex_lock(&m);  
    while (ShopOpen) {
        /* your code here */

        // Barber check if there is customres 
        if(TakenSeats > 0)  
        {
            // Barber is calling a waiting customer.
            pthread_cond_signal(&CUSTOMER);
            TakenSeats--; 
            
            // Barber is cutting hair 
            pthread_cond_wait(&BARBERCHAIR,&m);
        }else{
            printf("DEBUG: %f barber goes to sleep\n", timestamp());
            pthread_cond_wait(&BARBER, &m);
            printf("DEBUG: %f barber wakes up\n", timestamp());
        }
    }
    pthread_mutex_unlock(&m);
}

/* the customer method
 */
void customer(int customer_num)
{
    pthread_mutex_lock(&m);
    /* your code here */ 
    printf("DEBUG: %f customer %d enters shop\n", timestamp(), customer_num);
    
    stat_count_incr(c_TotalCustomers);                  // q3-1
    stat_count_incr(c_InShopCustomers);                 // q3-3
    
    // Cusomter checks if there is a free seat. 
    if(TakenSeats < TotalSeats){
        stat_timer_start(t_PerCustomer);                // q3-2
        
        // customer seat down
        TakenSeats++; 
        
        // customer is waking up Barber 
        pthread_cond_signal(&BARBER);

        // customre is waiting for Barber free
        pthread_cond_wait(&CUSTOMER, &m);
        printf("DEBUG: %f customer %d starts haircut\n", timestamp(), customer_num); 
        
        stat_count_incr(c_SitCustomers);                // q3-4
        
        // Barber is cutting hair 
        BarberIsBusy(customer_num);
        
        stat_timer_stop(t_PerCustomer);                 // q3-2
        
        stat_count_decr(c_SitCustomers);                // q3-4
        
        // Barber has finished!
        pthread_cond_signal(&BARBERCHAIR);
    } 
    else{
        stat_count_incr(c_TurnAwayCustomers);           // q3-1
    }
    
    stat_count_decr(c_InShopCustomers);                 // q3-3
    printf("DEBUG: %f customer %d leaves shop\n", timestamp(), customer_num);
    pthread_mutex_unlock(&m);
}

/* Threads which call these methods. Note that the pthread create
 * function allows you to pass a single void* pointer value to each
 * thread you create; we actually pass an integer (the customer number)
 * as that argument instead, using a "cast" to pretend it's a pointer.
 */

/* the customer thread function - create 10 threads, each of which calls
 * this function with its customer number 0..9
 */
void *customer_thread(void *context) 
{
    int customer_num = (int)context; 
    while(ShopOpen)
    {      
       sleep_exp(T1, NULL);
       customer(customer_num);
    }
    return 0;
}

/*  barber thread
 */
void *barber_thread(void *context)
{
    barber(); /* never returns */
    return 0;
}

void q2(void)
{
    /* to create a thread:
        pthread_t t; 
        pthread_create(&t, NULL, function, argument);
       note that the value of 't' won't be used in this homework
    */
    /* your code goes here */ 
    pthread_t t_barber;
    pthread_t t_customer[N]; 
    int error = 0;

    /*Open the shop*/
    ShopOpen = 1;

    /* Create barber thread */
    error = pthread_create(&t_barber, NULL, barber_thread, (void*)BarberNum);
    if (error){
        printf("Failed to create t_barber thread!\n");
    }

    /* Create customer threads */
    int i;
    for (i = 0; i < N; ++i)
    {
       error = pthread_create(&t_customer[i], NULL, customer_thread, (void*)i);
       if (error){
            printf("Failed to create t_customer[%d] thread!\n", i);
       }
    }
    
    wait_until_done();
}

/* For question 3 you need to measure the following statistics:
 *
 * 1. fraction of  customer visits result in turning away due to a full shop 
 *    (calculate this one yourself - count total customers, those turned away)
 * 2. average time spent in the shop (including haircut) by a customer 
 *     *** who does not find a full shop ***. (timer)
 * 3. average number of customers in the shop (counter)
 * 4. fraction of time someone is sitting in the barber's chair (counter)
 *
 * The stat_* functions (counter, timer) are described in the PDF. 
 */

void q3(void)
{
    /* your code goes here */ 
    pthread_t t_barber;
    pthread_t t_customer[N]; 
    int error = 0;

    /* Initial Counters and Timers*/
    c_TotalCustomers = stat_counter();
    c_TurnAwayCustomers = stat_counter();
    c_InShopCustomers = stat_counter();
    c_SitCustomers = stat_counter();
    t_PerCustomer = stat_timer();

    /*Open the shop*/
    ShopOpen = 1;

    /* Create barber thread */
    error = pthread_create(&t_barber, NULL, barber_thread, (void*)BarberNum);
    if (error){
        printf("Failed to create t_barber thread!\n");
    }

    /* Create customer threads */
    int i;
    for (i = 0; i < N; ++i)
    {
       error = pthread_create(&t_customer[i], NULL, customer_thread, (void*)i);
       if (error){
            printf("Failed to create t_customer[%d] thread!\n", i);
       }
    }

    wait_until_done();

    // q3-1: fraction of customer turn away 
    double val1 = stat_count_mean(c_TotalCustomers);
    // printf("c_TotalCustomers are %f\n", val1);
    double val2 = stat_count_mean(c_TurnAwayCustomers);
    // printf("c_TurnAwayCustomers are %f\n", val2);
    double q1 = val2*1.0/val1;
    printf("OUTPUT q3-1: %f\n", q1);

    // q3-2: Average time spend in shop per customer
    double q2 = stat_timer_mean(t_PerCustomer);
    printf("OUTPUT q3-2: %f\n", q2);

    // q3-3: Average number of customers in shop 
    double q3 = stat_count_mean(c_InShopCustomers);
    printf("OUTPUT q3-3: %f\n", q3);

    // q3-4: fraction of time for each customer sitting on a chair
    double q4 = stat_count_mean(c_SitCustomers);
    printf("OUTPUT q3-4: %f\n", q4);
}
