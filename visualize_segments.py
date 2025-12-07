import folium
import pandas as pd
from datetime import datetime

# Load GPS data safely
data = pd.read_csv(
    "gpsdata.txt",
    header=None,
    names=["lat", "lon", "speed", "year", "month", "day", "time"],
    dtype=str,
    on_bad_lines="skip"
)

# Filter valid rows
data = data[pd.to_numeric(data["lat"], errors="coerce").notnull()]
data["lat"] = data["lat"].astype(float)
data["lon"] = data["lon"].astype(float)

# Combine date and time into datetime object for gap detection
def parse_time(row):
    try:
        return datetime.strptime(f"{row['year']}-{row['month']}-{row['day']} {row['time']}", "%Y-%m-%d %H:%M:%S")
    except Exception:
        return None

data["datetime"] = data.apply(parse_time, axis=1)
data = data.dropna(subset=["datetime"]).sort_values("datetime").reset_index(drop=True)

# Split GPS data into drives if gap > 5 minutes
drives = []
current_drive = [data.iloc[0]]
for i in range(1, len(data)):
    delta = (data["datetime"][i] - data["datetime"][i - 1]).total_seconds()
    if delta > 300:  # 5 minutes gap → new drive
        drives.append(current_drive)
        current_drive = []
    current_drive.append(data.iloc[i])
if current_drive:
    drives.append(current_drive)

# Define segments
segments = [
    {"id": 1, "name": "13th & Marine → Taylor Way", "min_lat": 49.3260, "min_lon": -123.1516, "max_lat": 49.3283, "max_lon": -123.1340},
    {"id": 2, "name": "Taylor Way → Lions Gate Bridge", "min_lat": 49.3239, "min_lon": -123.1335, "max_lat": 49.3278, "max_lon": -123.1290},
    {"id": 3, "name": "Lions Gate Bridge", "min_lat": 49.3117, "min_lon": -123.1429, "max_lat": 49.3238, "max_lon": -123.1308},
    {"id": 4, "name": "Causeway → Denman", "min_lat": 49.2925, "min_lon": -123.1518, "max_lat": 49.3117, "max_lon": -123.1333},
    {"id": 5, "name": "Denman → Pacific", "min_lat": 49.2868, "min_lon": -123.1425, "max_lat": 49.2924, "max_lon": -123.1332},
    {"id": 6, "name": "Pacific → Burrard St Bridge", "min_lat": 49.2766, "min_lon": -123.1430, "max_lat": 49.2867, "max_lon": -123.1320},
    {"id": 7, "name": "Burrard Bridge", "min_lat": 49.2720, "min_lon": -123.1465, "max_lat": 49.2764, "max_lon": -123.1326},
    {"id": 8, "name": "Cornwall", "min_lat": 49.2721, "min_lon": -123.1634, "max_lat": 49.2732, "max_lon": -123.1468},
    {"id": 9, "name": "Macdonald → W 4th", "min_lat": 49.2680, "min_lon": -123.1692, "max_lat": 49.2729, "max_lon": -123.1637},
    {"id": 10, "name": "W 4th → Blanca", "min_lat": 49.2671, "min_lon": -123.2165, "max_lat": 49.2693, "max_lon": -123.1698},
    {"id": 11, "name": "Chancellor Blvd", "min_lat": 49.2668, "min_lon": -123.2477, "max_lat": 49.2737, "max_lon": -123.2171},
    {"id": 12, "name": "Chancellor Roundabout → Fraser Parkade", "min_lat": 49.2673, "min_lon": -123.2597, "max_lat": 49.2737, "max_lon": -123.2481},

]





# Colours
colors = [
    "red", "orange", "yellow", "green", "blue", "purple",
    "cyan", "magenta", "lime", "darkred", "darkblue", "darkgreen"
]

# Create map centered on route
m = folium.Map(location=[49.28, -123.16], zoom_start=12, tiles="OpenStreetMap")

# Add segments with hover titles
for i, s in enumerate(segments):
    color = colors[i % len(colors)]
    folium.Rectangle(
        bounds=[(s["min_lat"], s["min_lon"]), (s["max_lat"], s["max_lon"])],
        color=color,
        fill=True,
        fill_opacity=0.25,
        fill_color=color,
        popup=f"Segment {s['id']}: {s['name']}",
        tooltip=f"Segment {s['id']}: {s['name']}"
    ).add_to(m)

# Add each drive separately to avoid long jumps
for drive in drives:
    points = [(p["lat"], p["lon"]) for _, p in pd.DataFrame(drive).iterrows()]
    folium.PolyLine(points, color="black", weight=2.5, opacity=0.9).add_to(m)

# Save map
m.save("segments_map.html")
print("Map saved as 'segments_map.html' — open it in your browser.")

