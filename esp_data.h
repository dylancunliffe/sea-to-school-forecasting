#ifndef ESP_DATA_H
#define ESP_DATA_H

#include <stdlib.h>
#include <stdio.h>

#define MAX_ESP_DATA_POINTS 100000 // Maximum number of ESP data points, sufficient for > 1 month of data at 1-second invervals
#define MAX_TRAVERSALS 1000  // Maximum number of valid traversals to store
#define MAX_TRAVERSAL_DURATION 1800 // Maximum valid traversal duration in seconds (2 hours)
#define TIME_OFFSET 7 // Time offset in hours for local time adjustment (e.g., UTC-7 for PDT)

// Individual ESP data point structure for each line in the file
typedef struct {
	double lat;
	double lon;
	double speed;
	int year;
	int month;
	int day;
	int time; // In seconds
} ESPDataPoint;

typedef struct {
	int segment_id;
	double min_lat;
	double min_lon;
	double max_lat;
	double max_lon;
} Segment;

typedef struct {
	int segment_id;
	int duration;
	int year;
	int month;
	int day;
	int startTime;
} ValidTraversal;

int getESPData(FILE* filepointer, ESPDataPoint* data);
int processPoint(ESPDataPoint* data, int i, Segment* segments, int numSegments, int numPoints, ValidTraversal* traversals, int* traversalCount);
double traversalTime(ESPDataPoint* data, int startIndex, Segment* segment, int numPoints);
int recordTraversal(ValidTraversal* traversals, int* traversalCount, Segment* segment, double duration, ESPDataPoint* dataPoint);

#endif // esp_data_h