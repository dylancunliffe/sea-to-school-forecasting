#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#undef _UNICODE
#include "esp_data.h"
#include "prediction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <commdlg.h> // Common dialogs (file picker)

#define NUM_SEGMENTS 12

// Function prototypes for file dialogs
int openFileDialog(char* outPath, const char* filter, const char* title);
int saveFileDialog(char* outPath, const char* filter, const char* title);
void generatePredictions(char* predictionfilename, char* traversalfilename, Segment* segments);
void generatePredictionSet(char* predictionfilename, char* traversalfilename, Segment* segments);
void clearScreen();
void clearInputBuffer();
void pauseScreen();
void printMenu();
void processESPData(Segment* segments, char* inputfilename, char* traversalfilename);
void selectESPDataFile(char* inputfilename);
void selectTraversalOutputFile(char* traversalfilename);
void selectPredictionOutputFile(char* predictionfilename);

int main(void) {	
	// Drive segment dictionary
	Segment segments[NUM_SEGMENTS] = {
		// 1. 13th & Marine → Taylor Way
		{1, 49.3260, -123.1516, 49.3283, -123.1340},

		// 2. Taylor Way → Lions Gate Bridge
		// entry moved slightly east of segment 1 exit
		{2, 49.3239, -123.1335, 49.3278, -123.1290},

		// 3. Lions Gate Bridge
		{3, 49.3117, -123.1429, 49.3238, -123.1308},

		// 4. Causeway → Denman
		{4, 49.2925, -123.1518, 49.3117, -123.1333},

		// 5. Denman → Pacific
		{5, 49.2868, -123.1425, 49.2924, -123.1332},

		// 6. Pacific → Burrard St Bridge
		{6, 49.2766, -123.1430, 49.2867, -123.1320},

		// 7. Burrard Bridge
		{7, 49.2720, -123.1465, 49.2764, -123.1326},

		// 8. Cornwall
		{8, 49.2721, -123.1634, 49.2732, -123.1468},

		// 9. Macdonald → W 4th
		{9, 49.2680, -123.1692, 49.2729, -123.1637},

		// 10. W 4th → Blanca
		{10, 49.2671, -123.2165, 49.2693, -123.1698},

		// 11. Chancellor Blvd
		{11, 49.2668, -123.2477, 49.2737, -123.2171},

		// 12. Chancellor Roundabout → Fraser Parkade
		{12, 49.2673, -123.2597, 49.2737, -123.2481}
	};

	// File paths and default naming
	char inputfilename[MAX_PATH];
	char traversalfilename[MAX_PATH];
	char predictionfilename[MAX_PATH];

	strncpy(inputfilename, "esp_data.txt", sizeof(inputfilename) - 1);
	inputfilename[sizeof(inputfilename) - 1] = '\0';
	strncpy(traversalfilename, "traversals_output.txt", sizeof(traversalfilename) - 1);
	traversalfilename[sizeof(traversalfilename) - 1] = '\0';
	strncpy(predictionfilename, "predictions_output.txt", sizeof(predictionfilename) - 1);
	predictionfilename[sizeof(predictionfilename) - 1] = '\0';

	int choice = 0;
	// Main menu loop and interfacting
	do {
		clearScreen();
		printMenu();

		printf("Enter choice: ");
		if (scanf("%d", &choice) != 1) {
			clearInputBuffer();  // handles bad input like letters
			continue;
		}

		switch (choice) {
		case 1:
			// Select ESP Data File
			selectESPDataFile(inputfilename);
			break;

		case 2:
			// Select Output Traversal File
			selectTraversalOutputFile(traversalfilename);
			break;

		case 3:
			// Select Output Prediction File
			selectPredictionOutputFile(predictionfilename);
			break;

		case 4:
			processESPData(segments, inputfilename, traversalfilename);
			break;

		case 5:
			generatePredictions(predictionfilename, traversalfilename, segments);
			break;

		case 6:
			generatePredictionSet(predictionfilename, traversalfilename, segments);
			break;

		case 7:
			break;

		default:
			printf("Invalid choice.\n");
			pauseScreen();
			break;
		}
	} while (choice != 7);


	printf("Exiting program...\n");
	return 0;
}
	
void selectESPDataFile(char* inputfilename) {
	if (openFileDialog(inputfilename, "CSV Files\0*.csv\0All Files\0*.*\0", "Select ESP Data File")) {
		printf("Selected file: %s\n", inputfilename);
		system("pause");
	}
	else {
		printf("File selection canceled.\n");
		system("pause");
	}
}
	
void selectTraversalOutputFile(char* traversalfilename) {
	if (saveFileDialog(traversalfilename, "CSV Files\0*.csv\0All Files\0*.*\0", "Save Traversal Output File")) {
		printf("Output file: %s\n", traversalfilename);
		system("pause");
	}
	else {
		printf("File selection canceled.\n");
		system("pause");
	}
}

void selectPredictionOutputFile(char* predictionfilename) {
	if (saveFileDialog(predictionfilename, "CSV Files\0*.csv\0All Files\0*.*\0", "Save Prediction Output File")) {
		printf("Output file: %s\n", predictionfilename);
		system("pause");
	}
	else {
		printf("File selection canceled.\n");
		system("pause");
	}
}

void processESPData(Segment* segments, char* inputfilename, char* traversalfilename) {
	FILE* datafile = fopen(inputfilename, "r");

	int numPoints = 0;
	int traversalCount = 0;

	ValidTraversal* traversals = (ValidTraversal*)malloc(MAX_TRAVERSALS * sizeof(ValidTraversal)); // Array to hold valid traversals
	ESPDataPoint* ESPData = (ESPDataPoint*)malloc(MAX_ESP_DATA_POINTS * sizeof(ESPDataPoint)); // Array of struct to hold ESP data points for preprocessing

	// Check for successful memory allocation
	if (!ESPData || !traversals) {
		fprintf(stderr, "Memory allocation failed.\n");
		if (ESPData) free(ESPData);
		if (traversals) free(traversals);
		return;
	}

	// Confirm successful file opening
	if (datafile == NULL) {
		perror("Error opening file");
		return;
	}
	else {
		printf("File opened successfully\n");

		numPoints = getESPData(datafile, ESPData);

		printf("Number of ESP data points read: %d\n", numPoints);
		printf("Processing ESP data points...\n");

		for (int i = 0; i < numPoints;) {
			i = processPoint(ESPData, i, segments, NUM_SEGMENTS, numPoints, traversals, &traversalCount);
		}

		printf("Processing complete. Number of valid traversals recorded: %d\n", traversalCount);
		// Print all recorded traversals for verification
		for (int i = 0; i < traversalCount; i++) {
			printf("Traversal %d: Segment ID: %d, Duration: %d seconds, Date: %04d-%02d-%02d, Start Time: %02d:%02d:%02d\n",
				i + 1,
				traversals[i].segment_id,
				traversals[i].duration,
				traversals[i].year,
				traversals[i].month,
				traversals[i].day,
				traversals[i].startTime / 3600,
				(traversals[i].startTime % 3600) / 60,
				traversals[i].startTime % 60
			);
		}
	}

	// Write valid traversals to output file
	printf("Writing traversals to output file...\n");
	FILE* traversalFile = fopen(traversalfilename, "w");
	if (traversalFile == NULL) {
		perror("Error opening output file");
		return;
	}
	for (int i = 0; i < traversalCount; i++) {
		fprintf(traversalFile, "%d,%d,%04d-%02d-%02d,%02d:%02d:%02d\n",
			traversals[i].segment_id,
			traversals[i].duration,
			traversals[i].year,
			traversals[i].month,
			traversals[i].day,
			traversals[i].startTime / 3600,
			(traversals[i].startTime % 3600) / 60,
			traversals[i].startTime % 60
		);
	}
	printf("Traversals successfully saved to '%s'.\n", traversalfilename);

	fclose(datafile);
	fclose(traversalFile);

	free(ESPData);
	free(traversals);

	system("pause");
}

void generatePredictions (char* predictionfilename, char*traversalfilename, Segment* segments) {
	FILE* traversalFile = fopen(traversalfilename, "r");

	if (traversalFile == NULL) {
		perror("Error opening traversal file");
		return;
	}
		
	int j = 0;
	int traversalCount = 0;
	int tempHour, tempMinute, tempSecond;
	ValidTraversal* traversals = (ValidTraversal*)malloc(MAX_TRAVERSALS * sizeof(ValidTraversal));

	// Read traversals from file into array
	while (fscanf(traversalFile, "%d,%d,%d-%d-%d,%d:%d:%d\n",
		&traversals[j].segment_id,
		&traversals[j].duration,
		&traversals[j].year,
		&traversals[j].month,
		&traversals[j].day,
		&tempHour,
		&tempMinute,
		&tempSecond) == 8) {

		traversals[j].startTime = tempHour * 3600 + tempMinute * 60 + tempSecond;
		j++;
		if (j >= MAX_TRAVERSALS) break; // Prevent overflow
	}
	traversalCount = j;

	double routeMean, routeStddev;
	int targetTime, targetDay, targetDOW, targetMonth, targetYear;
	int hour, minute;

	printf("Enter target date (YYYY-MM-DD): \n");
	scanf("%d-%d-%d", &targetYear, &targetMonth, &targetDay);
	printf("Enter target time of the day (HH:MM): \n");
	scanf("%d:%d", &hour, &minute);

	targetTime = hour * 3600 + minute * 60;

	ValidTraversal tempTraversal = { 0, 0, targetYear, targetMonth, targetDay, targetTime };
	targetDOW = getDayOfWeek(&tempTraversal);

	predictOverallDuration(segments, traversals, traversalCount, targetTime, targetDay, targetMonth, targetYear, targetDOW, &routeMean, &routeStddev);

	printf("Predicted overall duration: %.2f seconds\n", routeMean);
	printf("Predicted overall standard deviation: %.2f seconds\n", routeStddev);

	fclose(traversalFile);

	free(traversals);

	system("pause");
}

void generatePredictionSet(char* predictionfilename, char* traversalfilename, Segment* segments) {
	FILE* traversalFile = fopen(traversalfilename, "r");
	FILE* predictionFile = fopen(predictionfilename, "w");

	if (traversalFile == NULL || predictionFile == NULL) {
		perror("Error opening traversal file");
		return;
	}

	int j = 0;
	int traversalCount = 0;
	int tempHour, tempMinute, tempSecond;
	ValidTraversal* traversals = (ValidTraversal*)malloc(MAX_TRAVERSALS * sizeof(ValidTraversal));

	// Read traversals from file into array
	while (fscanf(traversalFile, "%d,%d,%d-%d-%d,%d:%d:%d\n",
		&traversals[j].segment_id,
		&traversals[j].duration,
		&traversals[j].year,
		&traversals[j].month,
		&traversals[j].day,
		&tempHour,
		&tempMinute,
		&tempSecond) == 8) {

		traversals[j].startTime = tempHour * 3600 + tempMinute * 60 + tempSecond;
		j++;
		if (j >= MAX_TRAVERSALS) break; // Prevent overflow
	}
	traversalCount = j;

	for (int m = 0; m < 1440; m++) {
		// Generate predictions for each minute of the day
		int targetTime = m * 60;  // Convert minutes to seconds
		double routeMean, routeStddev;
		int targetDay = 1; // Default to the first day of the month
		int targetMonth = 10; // Default to October
		int targetYear = 2025; // Default to 2025
		int targetDOW = 4; // Default to Wednesday

		predictOverallDuration(segments, traversals, traversalCount, targetTime, targetDay, targetMonth, targetYear, targetDOW, &routeMean, &routeStddev);

		fprintf(predictionFile, "Time: %02d:%02d, Predicted Mean: %.2f, Std Dev: %.2f\n", m / 60, m % 60, routeMean, routeStddev);
	}
	printf("Prediction set printed to file.\n");

	fclose(traversalFile);
	fclose(predictionFile);

	free(traversals);

	system("pause");
}

void printMenu() {
	printf("=== Traffic Forecasting ESP Data Processor ===\n");
	printf("1. Select ESP Data File\n");
	printf("2. Select Output Traversal File\n");
	printf("3. Select Output Prediction File\n");
	printf("4. Process ESP Data\n");
	printf("5. Generate predictions for a specific time (command line readout)\n");
	printf("6. Generate prediction set (file readout)\n");
	printf("7. Exit\n");
	printf("-------------------------------\n");
}

void clearScreen() {
	system("cls");
}

void clearInputBuffer() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}

void pauseScreen() {
	printf("Press Enter to continue...");
	getchar();
}

int openFileDialog(char* outPath, const char* filter, const char* title) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(outPath, MAX_PATH);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = outPath;
	ofn.nMaxFile = MAX_PATH;

	// 👇 Show all files by default, still allow CSV filter as an option
	ofn.lpstrFilter = "All Files\0*.*\0CSV Files\0*.csv\0";
	ofn.nFilterIndex = 1; // Selects "All Files" by default

	ofn.lpstrTitle = title;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	return GetOpenFileName(&ofn);
}

int saveFileDialog(char* outPath, const char* filter, const char* title) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(outPath, MAX_PATH);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = outPath;
	ofn.nMaxFile = MAX_PATH;

	// 👇 All files first, CSV second
	ofn.lpstrFilter = "All Files\0*.*\0CSV Files\0*.csv\0";
	ofn.nFilterIndex = 1; // Selects "All Files" by default

	ofn.lpstrTitle = title;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	return GetSaveFileName(&ofn);
}