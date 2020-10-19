#define __GNU_SOURCE

#include <time.h> // clock_gettime
#include <unistd.h> // sleep
#include <stdio.h> // printf
#include <stdint.h>

#include "dtime.h"

typedef unsigned long long _llu_t;

static uint64_t calibrate_get_dtime_loop(void)
{
   	uint64_t start, elapsedTime;

	start = get_dtime();
	elapsedTime = get_dtime() - start;

	int i;
	for(i=2; i<=1024; i++) {
		start = get_dtime();
		elapsedTime += get_dtime() - start;
	}
		
	return(elapsedTime / i);
	
}

static uint64_t calibrate_get_dtime_sleep(void)
{
   	uint64_t start = get_dtime();
	
	sleep(1);
		
	return(get_dtime() - start);
}

uint64_t dtime_calibrate(void)
{
	uint64_t cycleTime = calibrate_get_dtime_loop();
	uint64_t elapsedTime, ecdt;
	double emhz;

	printf("%s: calibrate_get_dtime_cycles(%016llu)\n", __FUNCTION__, (_llu_t)cycleTime);

	elapsedTime = 0;

	for(int i = 1; i <= 3; i++) {
		elapsedTime += calibrate_get_dtime_sleep() - cycleTime;

		ecdt = elapsedTime / i;
		emhz = ecdt / MHz(1);
		printf("%s: elapsed time: %016llu  ecdt: %016llu  estMHz: %010.4f\n", __FUNCTION__, (_llu_t)elapsedTime, (_llu_t)ecdt, emhz);
	}
	return(ecdt);
}
