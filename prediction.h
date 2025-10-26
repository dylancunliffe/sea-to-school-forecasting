#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "esp_data.h"

#define HALF_LIFE_DAYS 30.0
#define HALF_LIFE_SECONDS 1800.0 // 30 minutes
#define MAX_TRAVERSALS 1000
#define NUM_SEGMENTS 12

int getDayOfWeek(ValidTraversal* traversal);
int daysBetween(int year1, int month1, int day1, int year2, int month2, int day2);
double computeWeights(ValidTraversal t, int targetTime, int targetDOW, int targetYear, int targetMonth, int targetDay);
void weightedMeanAndStd(double* durations, double* weights, int count, double* mean, double* stddev);
void predictOverallDuration(Segment* segments, ValidTraversal* traversals, int traversalCount, int targetTime, int targetDay, int targetMonth, int targetYear, int targetDOW, double* routeMean, double* routeStddev);