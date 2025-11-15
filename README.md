# sea-to-school-forecasting
A predictive traffic analysis system integrating embedded sensing and data modeling to forecast real-time commute durations between West Vancouver and UBC.

## Project Overview
This project uses embedded hardware to collect traffic/commute data, processes it via custom segmentation and modelling algorithms, and produces real-time predictions of commute durations for the route between West Vancouver and the University of British Columbia.
Key goals:
- Deploy gps sensor to capture traversal times and segment data.
- Build a segmentation map of the route (see `segments_map.html`).
- Develop prediction logic to estimate the remaining commute time given current position and time.
- Create visualization tools (`visualize_segments.py`) + output files (`predictions_output.txt`, `traversals_output.txt`) for analysis.

## Repository Structure
```
├── esp_data.cpp          # Embedded firmware to capture/sense data
├── esp_data.h            # Header definitions for firmware
├── gpsdata.txt           # Raw data collected from sensors
├── main.cpp              # Main firmware logic
├── prediction.cpp        # Prediction algorithm implementation
├── prediction.h          # Header for prediction logic
├── traversals_output.txt # Output of traversal time analysis
├── predictions_output.txt# Output of predicted commute durations
├── segments_map.html     # Map visualization of route segments
├── visualize_segments.py # Python script to visualize segments
└── README.md             # (this document)
```
## Key Features
- Embedded data acquisition: capturing GPS and time stamps along the route.
- Route segmentation: dividing the commute into manageable segments for modelling and analysis.
- Prediction engine: uses the segment data and current traversal to estimate remaining time.
- Visualization: interactive map and Python plotting to assist with understanding segmentation and modelling results.
- Data output: plain-text files (`traversals_output.txt`, `predictions_output.txt`) for post-processing and portfolio showcase.

## Technical Details
- Firmware written in C for the embedded sensing unit (`*.cpp`, `*.h`).
- Python script for visualization and auxiliary analysis.
- Model: custom algorithm(s) implemented in `prediction.cpp`.
- Route: West Vancouver → University of British Columbia.
- Data: raw GPS/time data collected along the route (in `gpsdata.txt`), used to construct model inputs and outputs.
- Output: predicted commute durations and segment-based traversal summaries.

## Future Work
Several extensions and improvements are planned to enhance both accuracy and usability of the system:
  - **Real-time data integration**  
    Incorporate live traffic APIs to supplement sensor data and improve prediction accuracy.
  - **Enhanced prediction models**  
    Move beyond heuristic/segment-based methods and experiment with machine-learning approaches such as regression models, gradient boosting, or LSTM networks.
  - **Additional sensor inputs**  
    Integrate higher-accuracy GPS modules or IMU data to capture more detailed traversal characteristics.
  - **Dynamic route handling**  
    Automatically detect route deviations, detours, or different commute paths and adjust predictions accordingly.
  - **Web or mobile dashboard**  
    Develop a simple UI displaying real-time commute estimates, historical performance, and segment heatmaps.
  - **Multi-route support**  
    Expand the system to support additional commutes or generalize the codebase for arbitrary routes.
