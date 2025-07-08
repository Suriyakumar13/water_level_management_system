import time
import random
from datetime import datetime
from flask import Flask, render_template, jsonify, request
from flask_cors import CORS  # Needed for cross-origin requests, especially during development

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes, important for frontend to access API

# --- Tank Configuration and Calibration Constants ---
DEVICE_ID = "ESP32-TANK-A1"

# Calibration for inverted height:
#   20 cm => Full (1000 ml)
#  190 cm => Empty (   0 ml)
CALIBRATION_SLOPE = -1000.0 / (190.0 - 20.0)  # ≈ -5.8824 ml per cm
CALIBRATION_INTERCEPT = 1000.0 - (CALIBRATION_SLOPE * 20.0)  # ≈ 1117.6471 ml

# --- Global variable to hold the latest sensor data ---
current_sensor_data = {
    "device_id": DEVICE_ID,
    "timestamp": int(time.time()),
    "water_level_cm": 190.0,     # Start empty
    "water_available_liters": 0.0
}

def calculate_volume_liters(sensor_reading_cm):
    """
    Calculates the water volume in liters based on the sensor reading in cm.
    Uses Volume (ml) = slope * reading_cm + intercept, then clamps to [0,1000].
    """
    volume_ml = (CALIBRATION_SLOPE * sensor_reading_cm) + CALIBRATION_INTERCEPT
    volume_ml = max(0.0, min(volume_ml, 1000.0))  # Clamp between 0 and 1000 ml
    return volume_ml / 1000.0

@app.route('/')
def index():
    """Renders the main dashboard HTML page."""
    return render_template('index.html')

@app.route('/api/data', methods=['GET'])
def get_sensor_data():
    """
    API endpoint to return the latest sensor data.
    In a real system, this would fetch from a database or message queue.
    """
    return jsonify(current_sensor_data)

@app.route('/api/simulate', methods=['POST'])
def simulate_sensor_data():
    """
    API endpoint to simulate new sensor data, as if received from an ESP-NOW gateway.
    This replaces the 'loop' logic from your Arduino sketch for web simulation.
    """
    global current_sensor_data

    # Simulate realistic fill/drain swings between 20 cm (full) and 190 cm (empty)
    base_level = random.uniform(20.0, 190.0)
    water_level_reading = round(base_level + random.uniform(-0.5, 0.5), 2)
    water_level_reading = max(20.0, min(water_level_reading, 190.0))

    water_available_liters = calculate_volume_liters(water_level_reading)

    current_sensor_data = {
        "device_id": DEVICE_ID,
        "timestamp": int(time.time()),
        "water_level_cm": water_level_reading,
        "water_available_liters": round(water_available_liters, 2)
    }

    app.logger.debug(f"Simulated new data: {current_sensor_data}")
    return jsonify({"status": "success", "data": current_sensor_data})

if __name__ == '__main__':
    # Install dependencies with:
    #   pip install Flask Flask-Cors
    print("Flask app starting on http://0.0.0.0:5000")
    print("Use POST /api/simulate to generate new readings.")
    app.run(debug=True, host='0.0.0.0')
