#include "prediction.h"

// Calculate the day of the week for a given date using Zeller's Congruence
int getDayOfWeek(ValidTraversal* traversal) {
	int dayOfWeek;
	int day = traversal->day;
	int month = traversal->month;
	int year = traversal->year;
	int adjMonth;
	int adjYear;

	if (month == 1 || month == 2) {
		adjMonth = 12 + month;
	}
	else {
		adjMonth = month;
	}

	if (adjMonth == 13 || adjMonth == 14) {
		adjYear = year - 1;
	}
	else {
		adjYear = year;
	}

	int K = adjYear % 100; // year of the century
	int J = 20; // century for 2000s
	dayOfWeek = (day + ((13 * (adjMonth + 1)) / 5) + K + (K / 4) + (J / 4) + (5 * J)) % 7;

	dayOfWeek = (dayOfWeek + 6) % 7; // Adjust to make sunday = 0

	return dayOfWeek;
}

// Calculate the number of days between two dates
int daysBetween (int year1, int month1, int day1, int year2, int month2, int day2) {
	int monthLengths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // Days in each month
	int days1 = 0;
	int days2 = 0;

	// Date 1 calculations
	days1 = year1 * 365 + (year1 / 4) - (year1 / 100) + (year1 / 400); // Leap year adjustments
	for (int m = 1; m < month1; m++) {
		days1 += monthLengths[m - 1]; // Add days for each month leading up to month1
		if (m == 2 && ((year1 % 4 == 0 && year1 % 100 != 0) || (year1 % 400 == 0))) {
			days1 += 1; // Leap year
		}
	}
	days1 += day1; // Add days in the current month
	
	// Calculations for date 2
	days2 = year2 * 365 + (year2 / 4) - (year2 / 100) + (year2 / 400); // Leap year adjustments
	for (int m = 1; m < month2; m++) { // Add days for each month leading up to month2
		days2 += monthLengths[m - 1];
		if (m == 2 && ((year2 % 4 == 0 && year2 % 100 != 0) || (year2 % 400 == 0))) {
			days2 += 1; // Leap year
		}
	}
	days2 += day2; // Add days in the current month

	return abs(days2 - days1);
}

double computeWeights(ValidTraversal t, int targetTime, int targetDOW, int targetYear, int targetMonth, int targetDay) {
	double weight = 1.0;
	double dowWeight = 1.0;
	double timeWeight = 1.0;
	double dateWeight = 1.0;

	int dow = getDayOfWeek(&t);

	int diff = abs(t.startTime - targetTime);
	int timeDiff = (diff > 43200) ? (86400 - diff) : diff;  // wrap around midnight

	timeWeight = exp(-0.5 * pow(timeDiff / HALF_LIFE_SECONDS, 2.0)); // Gaussian decay based on time difference (std dev = 30 minutes)

	int dowDiff = abs(dow - targetDOW); // difference in day of week

	if(dowDiff == 0) {
		dowWeight = 2.0; // Higher weight for same day of week
	}
	else if(dowDiff == 1) {
		dowWeight = 1.2; // Medium weight for adjacent days
	}
	else {
		dowWeight = 1.0; // Lower weight for other days
	}

	if(dow > 1 && dow < 6 && targetDOW > 1 && targetDOW < 6) {
		// Apply additional weighting if both days are on weekdays
		dowWeight *= 1.2;
	}
	else if ((dow == 0 || dow == 6) && (targetDOW == 0 || targetDOW == 6)) {
		// Apply additional weighting if both days are on weekends
		dowWeight *= 1.2;
	}

	int dateDiff = daysBetween(t.year, t.month, t.day, targetYear, targetMonth, targetDay);
	dateWeight = pow(2.0, (double)-abs(dateDiff) / HALF_LIFE_DAYS); // Exponential decay based on date difference

	if (dateWeight < 1e-6) dateWeight = 1e-6; // Prevent weights from becoming too small over long periods of time

	// Compute weights by multiplying individual weights
	weight = timeWeight * dowWeight * dateWeight;

	return weight;	
}

// Inputs: arrays of durations and corresponding weights, and the count of elements
// Outputs: weighted mean and standard deviation as a combined double (mean in integer part, stddev in fractional part)
void weightedMeanAndStd(double* durations, double* weights, int count, double *mean, double *stddev) {
	double sumWeights = 0.0;
	double weightedSum = 0.0;
	double weightedSumSquares = 0.0;

	// Handle case with no data points
	if(count == 0) {
		*mean = 0.0;
		*stddev = 0.0;
		return;
	}

	// Calculate weighted mean
	for (int i = 0; i < count; i++) {
		sumWeights += weights[i];
		weightedSum += durations[i] * weights[i];
	}
	if (sumWeights == 0.0) { // Prevent division by zero
		*mean = 0.0;
		*stddev = 0.0;
		return;
	}

	*mean = weightedSum / sumWeights;

	// Calculate weighted variance and standard deviation
	for (int i = 0; i < count; i++) {
		weightedSumSquares += weights[i] * pow(durations[i] - *mean, 2.0);
	}

	double variance = weightedSumSquares / sumWeights;
	*stddev = sqrt(variance);
}

void predictSegmentDuration(int* segment_id, ValidTraversal *traversals, int traversalCount, int targetYear, int targetMonth, int targetDay, int targetTime, int targetDOW, double* predictedMean, double* predictedStdDev) {
	double* durations = (double*) malloc(MAX_TRAVERSALS * sizeof(double));
	double* weights = (double*) malloc(MAX_TRAVERSALS * sizeof(double));
	int count = 0;

	// Collect durations and weights for the specified segment from array of all traversals
	for(int i = 0; i < traversalCount; i++) {
		if(traversals[i].segment_id == *segment_id) {
			durations[count] = (double)traversals[i].duration; // Store duration
			weights[count] = computeWeights(traversals[i], targetTime, targetDOW, targetYear, targetMonth, targetDay); // Compute and store weight
			count++;
		}
	}

	weightedMeanAndStd(durations, weights, count, predictedMean, predictedStdDev);

	free(durations);
	free(weights);
}

void predictOverallDuration(Segment* segments, ValidTraversal* traversals, int traversalCount, int targetTime, int targetDay, int targetMonth, int targetYear, int targetDOW, double* routeMean, double* routeStddev) {
	double totalDuration = 0.0;
	double totalVar = 0.0;

	double currentTime = targetTime;

	double segmentMean = 0.0;
	double segmentStdDev = 0.0;

	for (int i = 0; i < NUM_SEGMENTS; i++) {
		predictSegmentDuration(&segments[i].segment_id, traversals, traversalCount, targetYear, targetMonth, targetDay, currentTime, targetDOW, &segmentMean, &segmentStdDev);
		totalDuration += segmentMean;
		totalVar += segmentStdDev * segmentStdDev;

		currentTime += segmentMean;
		if (currentTime >= 86400) {
			currentTime -= 86400; // Wrap around midnight
		}
	}

	*routeMean = totalDuration;
	*routeStddev = sqrt(totalVar);

}
