#define _CRT_SECURE_NO_WARNINGS

#include "esp_data.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int getESPData(FILE *filepointer, ESPDataPoint* data) {

	// Counter for number of ESP data points read
	int count = 0;

	// Confirm successful file opening
	if (filepointer == NULL) {
		printf("Error opening ESP data file.\n");
		return -1;
	}
	else {
		printf("Reading ESP data...\n");

		ESPDataPoint* ESPData = data; // Pointer to the ESPData array

		char buffer[64]; // Buffer to hold each line read from the file

		// Temp strings to hold parsed data for verification and conversion to appropriate types
		char latStr[32];
		char lonStr[32];
		double tempSpeed;
		char yearStr[32];
		char monthStr[32];
		char dayStr[32];
		char timeStr[32];

		// Read each line from the file, and store the data in the ESPData array
		while (fgets(buffer, sizeof(buffer), filepointer) != NULL) {
			if (count >= MAX_ESP_DATA_POINTS) {
				printf("Maximum ESP data points reached. Some data may not be read.\n");
				break;
			}
			int n = sscanf(buffer, "%[^,],%[^,],%lf,%[^,],%[^,],%[^,],%[^\n]",
				latStr,
				lonStr,
				&tempSpeed,
				yearStr,
				monthStr,
				dayStr,
				timeStr
			);

			// Check if the line was parsed correctly by confirming the number of items read
			if (n != 7) {
				printf("Error parsing ESP data line: %s\n", buffer);
				continue; // Skip malformed lines
			}

			// Confirm validity of parsed data
			if (strcmp(latStr, "INVALID_LAT") == 0 || strcmp(lonStr, "INVALID_LNG") == 0 || strcmp(yearStr, "INVALID_DATE") == 0 || strcmp(timeStr, "INVALID_TIME") == 0) {
				printf("Skipping invalid ESP data point: %s\n", buffer);
				continue; // Skip invalid data points
			}

			// Convert parsed data to appropriate types
			double tempLat = atof(latStr);
			double tempLon = atof(lonStr);
			int tempYear = atoi(yearStr);
			int tempMonth = atoi(monthStr);
			int tempDay = atoi(dayStr);
			int tempHour, tempMinute, tempSecond;

			// Convert time string to individual components
			sscanf(timeStr, "%d:%d:%d", &tempHour, &tempMinute, &tempSecond);

			tempHour -= TIME_OFFSET;               // Convert from UTC to local (UTC-7)
			if (tempHour < 0) tempHour += 24;      // Wrap around midnight
			int totalSeconds = tempHour * 3600 + tempMinute * 60 + tempSecond;

			// Store the converted data in the ESPData array
			ESPData[count].lat = tempLat;
			ESPData[count].lon = tempLon;
			ESPData[count].speed = tempSpeed;
			ESPData[count].year = tempYear;
			ESPData[count].month = tempMonth;
			ESPData[count].day = tempDay;
			ESPData[count].time = totalSeconds; // Convert time to seconds
			count++;

			// Print the read data point for verification (Remove or comment out later)
			printf("Read ESP data point %d: Lat: %f, Lon: %f, Speed: %f, Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
				count,
				ESPData[count - 1].lat,
				ESPData[count - 1].lon,
				ESPData[count - 1].speed,
				ESPData[count - 1].year,
				ESPData[count - 1].month,
				ESPData[count - 1].day,
				tempHour,
				tempMinute,
				tempSecond
			);
		}

	}


	return count; // Indicate successful reading
}

int processPoint(ESPDataPoint* data, int startIndex, Segment* segments, int numSegments,
	int numPoints, ValidTraversal* traversals, int* traversalCount) {

	for (int j = 0; j < numSegments; j++) {
		// Check if this point is inside the segment
		if (data[startIndex].lat <= segments[j].max_lat && data[startIndex].lat >= segments[j].min_lat &&
			data[startIndex].lon <= segments[j].max_lon && data[startIndex].lon >= segments[j].min_lon) {

			// Check if previous point was outside → entry point
			if (startIndex == 0 ||
				!(data[startIndex - 1].lat <= segments[j].max_lat && data[startIndex - 1].lat >= segments[j].min_lat &&
					data[startIndex - 1].lon <= segments[j].max_lon && data[startIndex - 1].lon >= segments[j].min_lon)) {

				double duration = traversalTime(data, startIndex, &segments[j], numPoints);
				if (duration < 0) return startIndex;  // invalid traversal

				int result = recordTraversal(traversals, traversalCount, &segments[j], duration, &data[startIndex]);
				if (result == 0) {
					// Find the index where traversal ended
					int endIndex = startIndex;
					while (endIndex + 1 < numPoints &&
						data[endIndex].lat <= segments[j].max_lat && data[endIndex].lat >= segments[j].min_lat &&
						data[endIndex].lon <= segments[j].max_lon && data[endIndex].lon >= segments[j].min_lon) {

						endIndex++;

						// Safety guard: break if stuck near the end of the dataset
						if (endIndex >= numPoints - 1)
							break;
					}

					printf("Exited segment %d at index %d (duration: %.1f sec)\n", segments[j].segment_id, endIndex, duration);
					return endIndex;  // return index after exiting segment ✅
				}
			}
		}
	}

	// If no segment matched this point, move to the next safely
	return (startIndex + 1 < numPoints) ? startIndex + 1 : numPoints;
}

double traversalTime(ESPDataPoint* data, int startIndex, Segment* segment, int numPoints) {
	int i = startIndex;
	int startTime = data[startIndex].time;

	while(i < numPoints && 
		data[i].lat <= segment->max_lat && data[i].lat >= segment->min_lat &&
		data[i].lon <= segment->max_lon && data[i].lon >= segment->min_lon) {
			i++;
	}

	if(i >= numPoints) {
		return -1; // Invalid traversal if data ends before exiting segment
	}

	double duration = data[i - 1].time - startTime;
	if(duration < 10 || duration > MAX_TRAVERSAL_DURATION) {
		return -1; // Invalid traversal if duration is negative or exceeds maximum allowed
	}
	return duration;
}

int recordTraversal (ValidTraversal* traversals, int* traversalCount, Segment* segment, double duration, ESPDataPoint* dataPoint) {
	if(*traversalCount >= MAX_TRAVERSALS) {
		printf("Maximum number of traversals reached. Cannot record more traversals.\n");
		return -1;
	}
	ValidTraversal* vt = &traversals[*traversalCount];
	vt->segment_id = segment->segment_id;
	vt->duration = (int)duration;
	vt->year = dataPoint->year;
	vt->month = dataPoint->month;
	vt->day = dataPoint->day;
	vt->startTime = dataPoint->time;
	(*traversalCount)++;
	return 0; // Successful recording
}