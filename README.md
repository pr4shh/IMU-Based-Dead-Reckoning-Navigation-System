# IMU-Based Dead Reckoning Navigation System

A handheld navigation unit that estimates its own X-Y position and heading using an IMU (MPU-series accelerometer and gyroscope), without GPS, a camera, or external signals.

Developed as an educational project for the Aerospace Defence Electronics Basics course, this project demonstrates simplified inertial navigation in GPS-denied environments.

## Project Goals

- Estimate orientation and movement using IMU measurements.
- Convert sensor measurements into world-frame acceleration.
- Estimate velocity and 2D position through integration.
- Apply Zero-Velocity Update (ZUPT) to reduce velocity drift.
- Record navigation data and visualize the estimated path using Python.

> **Note:** This is an educational dead-reckoning prototype, not a precision navigation system.

## Project Structure

```text
IMU-Based-Dead-Reckoning-Navigation-System/
├── Smart_Project/
│   └── sensor_monitor.ino
├── python/
│   └── data_logger.py
└── README.md
```

## Hardware Requirements

- Arduino Uno or ESP32
- MPU-series IMU module (MPU9250 board)
- Jumper wires
- USB cable
- Computer with Arduino IDE and Python 3

## Wiring

| IMU Pin | Arduino Uno | ESP32 |
|---|---|---|
| VCC | 5V (check module) | 3.3V |
| GND | GND | GND |
| SCL | A5 | GPIO22 |
| SDA | A4 | GPIO21 |

**Power note:** Check your specific IMU breakout board's voltage requirements before connecting VCC.

## How It Works

1. **Calibration:** The unit remains flat and still for approximately 5 seconds to estimate gyro offsets and initial tilt.
2. **Orientation Tracking:** Gyroscope readings are integrated to estimate orientation.
3. **Complementary Filter:** Accelerometer-based tilt correction helps reduce roll and pitch drift.
4. **Acceleration Processing:** Sensor acceleration is transformed into the world frame, with gravity removed.
5. **Dead Reckoning:** Acceleration is integrated to estimate velocity and position.
6. **ZUPT:** Stationary detection resets velocity to reduce accumulated drift.

## Software

- Arduino IDE
- Python 3
- Matplotlib (for plotting, if required by the Python script)

Install Matplotlib:

```bash
pip install matplotlib
```

## How to Run

### 1. Arduino Setup

1. Connect the Arduino to your computer.
2. Open `Smart_Project/sensor_monitor.ino` in Arduino IDE.
3. Select the correct board and COM port.
4. Upload the sketch.
5. Keep the sensor flat and still during calibration.
6. Begin the movement test.

### 2. Python Data Logging and Visualization

The Python script is located at:

`python/data_logger.py`

Run the script from the repository root:

```bash
python python/data_logger.py
```

If the script requires a saved serial log or additional arguments, follow the configuration and instructions inside the script.

## Testing

For a repeatable test:

1. Select a clear, level area.
2. Calibrate the sensor while it is stationary.
3. Follow a predefined route.
4. Record the navigation data.
5. Generate the path visualization.
6. Compare the estimated path with the intended route.
7. Repeat the test to evaluate consistency.

## Known Limitations

- **Heading Drift:** Gyroscope-only yaw estimation accumulates error.
- **Position Drift:** Double integration increases position error over time.
- **Sensor Alignment:** Mounting orientation affects the estimated movement.
- **Calibration Sensitivity:** Movement during calibration can affect the results.
- **ZUPT Limitations:** Stationary detection cannot eliminate all navigation errors.

## Results

Add actual test results and generated plots here after completing validation.

| Test | Real Path | Tracked Endpoint Error | Notes |
|---|---|---|---|
| Square route (2 m sides) | | | |

## Contributors

- **PRASHASTH** — [@pr4shh](https://github.com/pr4shh)
- **NANDAN** — [@nandann13](https://github.com/nandann13)

## Future Improvements

- Evaluate heading correction methods.
- Compare repeated test runs.
- Quantify endpoint error.
- Improve drift handling.
- Test different routes and sensor orientations.
