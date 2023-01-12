#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "p3150149-p3150123-pizza.h"

void *functionPizzaria(void *t){
	int *threadId = (int*)t;
	int pizzasOrdered = 0;
	struct timespec start, startCooling, stop;
	int timePassed, deliveryTime;
	
	
	/* 			--CALLING TIME--			 */
	
	
	//Initializing waiting time counter
	if(clock_gettime(CLOCK_REALTIME,&start)==-1){
		printf("ERROR:clock gettime");
		pthread_exit(&threadId);
	}
	
	//Locking the availTel variable
	int rc;
	rc = pthread_mutex_lock(&telMutex);
	
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	//Customer waits for an available line 
	while (availTel==0) {
		
    	rc = pthread_cond_wait(&telThresholdCondition, &telMutex);
    	
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

	}
	
	//Customer is speaking
	availTel--;	
	rc = pthread_mutex_unlock(&telMutex);
	
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	//Stopping timer
	if(clock_gettime(CLOCK_REALTIME,&stop)==-1){
		printf("ERROR:clock gettime");
		pthread_exit(&threadId);
	}
	
	//Time waiting in line
	timePassed = (stop.tv_sec-start.tv_sec);
	
	//Calculating the max waiting time
	if (timePassed>maxWaitingTime) {
		rc = pthread_mutex_lock(&maxWaitingTimeMutex);
		maxWaitingTime = timePassed;
		rc = pthread_mutex_unlock(&maxWaitingTimeMutex);	
	}
	
	//Calculating the average waiting time
	rc = pthread_mutex_lock(&avgWaitingTimeMutex);
	avgWaitingTime+= (double)timePassed/Ncust;
	rc = pthread_mutex_unlock(&avgWaitingTimeMutex);
	
	
	//Customer is giving their order
	pizzasOrdered = Norderlow +(rand_r(&seed)%Norderhigh);
	
	//Accepting the Customer's credit card with possibility of failure at 5%
	if (rand()<Pfail*((double)RAND_MAX+1.0)){
		
		rc = pthread_mutex_lock(&screenMutex);
		printf("Order No %d failed!\n",*threadId);
		rc = pthread_mutex_unlock(&screenMutex);
		
		rc = pthread_mutex_lock(&telMutex);
		availTel++;
		rc = pthread_cond_signal(&telThresholdCondition);
		rc = pthread_mutex_unlock(&telMutex);
		
		rc = pthread_mutex_lock(&failedMutex);
		failed++;
		rc = pthread_mutex_unlock(&failedMutex);
		
		pthread_exit(&threadId);
	}
	
	rc = pthread_mutex_lock(&screenMutex);
	printf("Order No %d received successfully!\n",*threadId);
	rc = pthread_mutex_unlock(&screenMutex);
	
	//Increasing the successfull orders
	rc = pthread_mutex_lock(&successfullMutex);
	successfull++;
	rc = pthread_mutex_unlock(&successfullMutex);
	
	//Adding to the total revenue
	rc = pthread_mutex_lock(&revenueSumMutex);
	
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	revenueSum += pizzasOrdered*Cpizza;
	
	rc = pthread_mutex_unlock(&revenueSumMutex);
	
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	sleep(Tpaymentlow +(rand_r(&seed)%Tpaymenthigh));
	
	//Customer gave their order, availTel variable is unlocked
	rc = pthread_mutex_lock(&telMutex);
	availTel++;
	rc = pthread_cond_signal(&telThresholdCondition);
	rc = pthread_mutex_unlock(&telMutex);	
	
	
	/*             --COOKING TIME--           */
	
	//Order waiting for available Cooks
	rc = pthread_mutex_lock(&cookMutex);
	
	while (availCook==0) {
		
    	rc = pthread_cond_wait(&cookThresholdCondition, &cookMutex);
    	
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

	}
	availCook--;
	
	rc = pthread_mutex_unlock(&cookMutex);
	
	
	//Cook waiting for available oven(s) 
	rc = pthread_mutex_lock(&ovenMutex);
	
	while (availOven<pizzasOrdered) {
		
    	rc = pthread_cond_wait(&ovenThresholdCondition, &ovenMutex);
    	
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

	}
	
	availOven -= pizzasOrdered;
	rc = pthread_mutex_unlock(&ovenMutex);
	
	//Cook preparing the pizza(s) 
	sleep(Tprep*pizzasOrdered);
	
	rc = pthread_mutex_lock(&cookMutex);
	availCook++;
	rc = pthread_cond_signal(&cookThresholdCondition);
	rc = pthread_mutex_unlock(&cookMutex);
	
	//Cook preparing the pizza(s) 
	sleep(Tbake);
	
	//Initializing cooling time counter
	if(clock_gettime(CLOCK_REALTIME,&startCooling)==-1){
		printf("ERROR:clock gettime");
		pthread_exit(&threadId);
	}
	
	/*             --PACKING TIME--           */
	
	rc = pthread_mutex_lock(&packerMutex);
	
	while (packer==0) {
		
    	rc = pthread_cond_wait(&packerThresholdCondition, &packerMutex);
    	
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

	}
	
	packer = 0;
	rc = pthread_mutex_unlock(&packerMutex);
	
	sleep(Tpack);
	
	//Stopping timer
	if(clock_gettime(CLOCK_REALTIME,&stop)==-1){
		printf("ERROR:clock gettime");
		pthread_exit(&threadId);
	}
	
	//Time passed from the customer calling to packing
	timePassed = (stop.tv_sec-start.tv_sec);
	
	rc = pthread_mutex_lock(&screenMutex);
	printf("Order No %d ready in %d minutes \n",*threadId,timePassed);
	rc = pthread_mutex_unlock(&screenMutex);
	
	//Ovens are available again
	rc = pthread_mutex_lock(&ovenMutex);
	availOven += pizzasOrdered;
	rc = pthread_cond_signal(&ovenThresholdCondition);
	rc = pthread_mutex_unlock(&ovenMutex);
	
	//Packer is available again
	rc = pthread_mutex_lock(&packerMutex);
	packer = 1;
	rc = pthread_cond_signal(&packerThresholdCondition);
	rc = pthread_mutex_unlock(&packerMutex);
	
	/*			--DELIVERY TIME--		 */
	
	//Order waiting for available Deliverer
	rc = pthread_mutex_lock(&delivererMutex);
	
	while (availDeliverer==0) {
		
    	rc = pthread_cond_wait(&delivererThresholdCondition, &delivererMutex);
    	
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

	}
	availDeliverer--;
	
	rc = pthread_mutex_unlock(&delivererMutex);
	
	deliveryTime = Tdellow +(rand_r(&seed)%(Tdelhigh-Tdellow));
	sleep(deliveryTime);
	
	//Stopping timer
	if(clock_gettime(CLOCK_REALTIME,&stop)==-1){
		printf("ERROR:clock gettime");
		pthread_exit(&threadId);
	}
	
	//Time passed from the customer calling to delivery
	timePassed = (stop.tv_sec-start.tv_sec);
	
	rc = pthread_mutex_lock(&screenMutex);
	printf("Order No %d delivered in %d minutes \n",*threadId,timePassed);
	rc = pthread_mutex_unlock(&screenMutex);
	
	//Calculating the max service time
	if (timePassed>maxServiceTime) {
		rc = pthread_mutex_lock(&maxServiceTimeMutex);
		maxServiceTime = timePassed;
		rc = pthread_mutex_unlock(&maxServiceTimeMutex);	
	}
	
	//Calculating the average service time
	rc = pthread_mutex_lock(&sumServiceTimeMutex);
	serviceTimeSum+= (double)timePassed;
	rc = pthread_mutex_unlock(&sumServiceTimeMutex);
	
	//Time passed from the pizza(s) being ready to delivery
	timePassed = (stop.tv_sec-startCooling.tv_sec);
	
	//Calculating the max cooling time
	if (timePassed>maxCoolingTime) {
		rc = pthread_mutex_lock(&maxCoolingTimeMutex);
		maxCoolingTime = timePassed;
		rc = pthread_mutex_unlock(&maxCoolingTimeMutex);	
	}
	
	//Calculating the average cooling time
	rc = pthread_mutex_lock(&sumCoolingTimeMutex);
	coolingTimeSum+= (double)timePassed;
	rc = pthread_mutex_unlock(&sumCoolingTimeMutex);
	
	sleep(deliveryTime);
	
	rc = pthread_mutex_lock(&delivererMutex);
	availDeliverer++;
	rc = pthread_cond_signal(&delivererThresholdCondition);
	rc = pthread_mutex_unlock(&delivererMutex);
	
	pthread_exit(&threadId);
}

int main (int argc,char *argv[]){
	
	Ncust = atoi(argv[1]);
	seed = atoi(argv[2]);
	int rc;
	size_t i;
	pthread_t threads[Ncust];
	int t[Ncust];
	
	//Initializing mutexes
	rc = pthread_mutex_init(&telMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&maxWaitingTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&avgWaitingTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&screenMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&failedMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&successfullMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&revenueSumMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&cookMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&ovenMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&packerMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&delivererMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&maxServiceTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&sumServiceTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&sumCoolingTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_init(&maxCoolingTimeMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}
	
	//Initializing conditions
  	rc = pthread_cond_init(&telThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_init(&cookThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_init(&ovenThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_init(&packerThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_init(&delivererThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}
	
	//Creating Ncust threads 
	for(i=0; i<Ncust; i++){
		t[i] = i+1;
		if((rc = pthread_create (&threads[i], NULL,functionPizzaria, &t[i]))){
			printf ("ERROR: return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		
		//The caller waits from 1 to 5mins
		sleep(Torderlow +(rand_r(&seed)%Torderhigh));
		
	}
	
	//Joining the threads so that the main thread waits for them to be done before 
	//ending.
	void *status;
	
	for(i=0; i<Ncust; i++){
		if((rc = pthread_join(threads[i], &status))){
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	
	
	printf("Total Revenue: %d Euros. \nSuccessfull Orders: %d. \nFailed Orders: %d.\n", revenueSum, successfull, failed);
	printf("Average Waiting Time: %.2f minutes. \nMaximum Waiting Time: %.2f minutes.\n", avgWaitingTime, maxWaitingTime);
	printf("Average Service Time: %.2f minutes.\nMaximum Service Time: %.2f minutes.\n", serviceTimeSum/successfull, maxServiceTime);
	printf("Average Cooling Time: %.2f minutes.\nMaximum Cooling Time: %.2f minutes.\n", coolingTimeSum/successfull, maxCoolingTime);
	
	//Destroying mutexes
  	rc = pthread_mutex_destroy(&telMutex);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	
	rc = pthread_mutex_destroy(&maxWaitingTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&avgWaitingTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&screenMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&failedMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&successfullMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&revenueSumMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&cookMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&ovenMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&packerMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&delivererMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&maxServiceTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&sumServiceTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&sumCoolingTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_mutex_destroy(&maxCoolingTimeMutex);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	//Destroying conditions
  	rc = pthread_cond_destroy(&telThresholdCondition);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_destroy(&cookThresholdCondition);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_destroy(&ovenThresholdCondition);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_destroy(&packerThresholdCondition);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	rc = pthread_cond_destroy(&delivererThresholdCondition);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	}
	
	return 1;
}
