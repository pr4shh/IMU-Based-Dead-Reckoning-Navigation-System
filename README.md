# IMU-Based Dead Reckoning Navigation System

A handheld navigation unit that estimates its own X-Y position and heading using only an IMU (MPU-series accelerometer and gyroscope). It does not rely on GPS, a camera, or any external signal.

Developed for the *Aerospace Defence Electronics Basics* course, this project is a simplified demonstration of inertial navigation in GPS-denied environments, such as aircraft, missiles, and submarines.

## Project Goals

- Demonstrate how an IMU can be used to estimate orientation and movement.
- Convert sensor measurements into a world-frame acceleration estimate.
- Estimate velocity and 2D position through integration.
- Apply a zero-velocity update (ZUPT) to reduce velocity drift.
- Record navigation output and visualize the estimated path with Python.

> **Scope:** This is an educational dead-reckoning prototype, not a precision navigation system.

## How It Works

1. **Calibration (5 seconds, board still and level):** Averages 250 samples to estimate gyro offsets and initial roll/pitch from gravity.
2. **Orientation tracking:** Roll, pitch, and yaw are updated by integrating gyroscope readings.
3. **Complementary filter:** Roll and pitch are corrected 2% per step toward the accelerometer's tilt estimate (`ALPHA = 0.98`, or 98% gyro / 2% accelerometer). This correction is applied only when measured acceleration is close to 1 g, helping avoid errors while the unit is accelerating.
4. **World-frame acceleration:** Body-frame acceleration is rotated into the world frame using roll, pitch, and yaw. This also removes gravity from the X and Y components.
5. **Double integration:** Acceleration is integrated to obtain velocity, then velocity is integrated to estimate position.
6. **Zero-velocity update (ZUPT):** When horizontal acceleration is below 0.30 m/s² **and** rotation is below 0.08 rad/s for 12 consecutive samples (about 0.25 seconds), the unit is considered stationary. Velocity is reset to zero, and gyro offsets are gradually re-learned. This reduces the velocity drift that would otherwise keep accumulating.

## System Overview

**Data flow**

1. The IMU provides accelerometer and gyroscope measurements to the microcontroller over I2C.
2. The Arduino sketch processes the measurements and estimates orientation and position.
3. The sketch sends navigation values as serial output.
4. The output can be saved as a text log.
5. The Python plotting script reads the saved log and generates a path visualization.

## Hardware

| Part | Notes |
|---|---|
| Arduino Uno (or ESP32) | |
| MPU-series IMU module (MPU9250 board) | Read directly over I2C at address `0x68`, without a library |
| Jumper wires and USB cable | |

### Wiring

| IMU pin | Arduino Uno | ESP32 |
|---|---|---|
| VCC | 5V (check your module) | 3.3V |
| GND | GND | GND |
| SCL | A5 | GPIO22 |
| SDA | A4 | GPIO21 |

**Power note:** Check your specific IMU breakout board's voltage requirements before connecting VCC. Do not assume every module supports 5V.

### Sensor Orientation

The sensor was mounted **upside-down**, so the code flips the axis signs (`FLIP_ABOUT_X`, `SX/SY/SZ`). If your board is mounted face-up, change the signs to `+1`.

If you change the sensor's mounting direction, verify the axis configuration before performing a test. A mounting change can affect the sign and interpretation of the estimated movement.

## Project Files

| File | Purpose |
|---|---|
| `mpureadings/mpureadings.ino` | Arduino sketch that reads the IMU, runs the filter and dead-reckoning calculations, and sends CSV data over serial |
| `tools/plot_any.py` | Python script that plots the recorded path using matplotlib |

## Software Requirements

- Arduino IDE
- Python 3
- `matplotlib` Python package
- USB connection between the board and the computer

Install the plotting dependency with:

```bash
pip install matplotlib
```

## How to Run

### 1. Upload and calibrate

1. Connect the Arduino to your computer.
2. Open `mpureadings/mpureadings.ino` in the Arduino IDE.
3. Select the correct board and port.
4. Upload the sketch at **115200 baud**.
5. Place the board **flat and completely still** for the 5-second calibration.
6. Move the unit along your test path.

### 2. Serial commands

- `r` — resets position and heading to zero
- `c` — runs calibration again

Serial output is reported at **10 Hz**.

### 3. Record and plot

Save the serial output to a text file (for example, `log.txt`). Then run:

```bash
python tools/plot_any.py log.txt
```

The script saves `log_plot.png` and displays the sample count, total path length, and start-to-end distance.

The `ROTATE90` flag near the top of the script rotates the plot by 90 degrees to match the axis convention used by the sketch version that generated the logs.

**Logging tip:** Keep a copy of the original serial log. It can help reproduce plots or compare different test runs. Use a separate filename for each test.

## Serial Output Format

The serial output uses this header (10 Hz):

```text
ms,x,y,yaw_deg,zupt,roll_deg,pitch_deg
```

| Column | Description |
|---|---|
| `ms` | Time in milliseconds |
| `x`, `y` | Estimated horizontal position coordinates |
| `yaw_deg` | Estimated heading/yaw angle in degrees |
| `zupt` | Zero-velocity-update status indicator |
| `roll_deg` | Estimated roll angle in degrees |
| `pitch_deg` | Estimated pitch angle in degrees |

Keep the header and the log rows together when saving data, unless the plotting script's expected input format has been adjusted.

## Testing Procedure

For a repeatable walk test:

1. Choose a clear, level area and define a simple route.
2. Keep the sensor mounting and orientation consistent between runs.
3. Start from a known starting point and complete the 5-second stationary calibration.
4. Record the serial output while following the route.
5. Save the log with a descriptive name.
6. Run the plotting script and compare the estimated path with the route you intended to walk.
7. Repeat the same route to see how consistent the results are.

Avoid treating a single test as proof of accuracy. Record the route dimensions and test conditions so results can be interpreted fairly.

## Known Limitations

- **Heading drift:** Yaw is calculated using gyroscope integration alone, so it gradually drifts over time.
- **No magnetometer correction:** The MPU9250 magnetometer is **not** used in this version.
- **Position drift:** Position error can increase with distance because acceleration is integrated twice.
- **Motion sensitivity:** Sensor movement, tilt, vibration, calibration quality, and mounting alignment can affect the estimate.
- **ZUPT dependence:** The velocity correction depends on correctly detecting when the unit is stationary; it cannot eliminate every source of error.

These are expected limitations of a low-cost IMU and explain why practical inertial navigation systems are periodically corrected using an external reference.

## Troubleshooting Notes

- **No serial output:** Confirm the selected board and port, USB connection, and the sketch's configured baud rate.
- **Serial output looks corrupted:** Check that the serial viewer is set to **115200 baud**.
- **Upload fails:** Verify the selected board and port, and close other programs that may be using the serial port.
- **Plot does not match the expected direction:** Check the sensor mounting orientation and the `FLIP_ABOUT_X` / `SX/SY/SZ` settings. Also check whether `ROTATE90` is appropriate for the log.
- **Unexpected drift:** Recheck that the board is flat and still during calibration, and repeat the test using the same route and handling.

## Results

*Add results after completing the walk test.*

| Test | Real path | Tracked end-point error | Notes |
|---|---|---|---|
| Square, 2 m sides | | | |

Add generated plot images to an `images/` folder and link them here. Include the route, test conditions, and any relevant observations alongside each plot.

## Contributors

- **[Your Name]** — [@yourusername](https://github.com/yourusername)
- **[Friend's Name]** — [@friendusername](https://github.com/friendusername)

Replace the placeholders with the contributors' names and GitHub usernames, and describe contributions accurately.

## Future Improvements

Possible next steps for experimentation:

- Evaluate heading correction using an appropriate external reference.
- Compare repeated test runs and quantify endpoint error.
- Test different routes and sensor mounting orientations.
- Improve drift handling and document calibration behavior.
