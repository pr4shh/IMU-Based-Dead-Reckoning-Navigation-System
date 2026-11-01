# IMU-Based Dead Reckoning Navigation System

A handheld navigation unit that estimates its own X-Y position and heading using only an IMU (MPU-series accelerometer and gyroscope). It does not rely on GPS, a camera, or any external signal. Developed for the *Aerospace Defence Electronics Basics* course, this project is a simplified demonstration of inertial navigation in GPS-denied environments, such as aircraft, missiles, and submarines.

## How It Works

1. **Calibration (5 seconds, board still and level):** Averages 250 samples to estimate gyro offsets and initial roll/pitch from gravity.
2. **Orientation tracking:** Roll, pitch, and yaw are updated by integrating gyroscope readings.
3. **Complementary filter:** Roll and pitch are corrected 2% per step toward the accelerometer's tilt estimate (`ALPHA = 0.98`, or 98% gyro / 2% accelerometer). This correction is applied only when measured acceleration is close to 1 g, helping avoid errors while the unit is accelerating.
4. **World-frame acceleration:** Body-frame acceleration is rotated into the world frame using roll, pitch, and yaw. This also removes gravity from the X and Y components.
5. **Double integration:** Acceleration is integrated to obtain velocity, then velocity is integrated to estimate position.
6. **Zero-velocity update (ZUPT):** When horizontal acceleration is below 0.30 m/s² **and** rotation is below 0.08 rad/s for 12 consecutive samples (about 0.25 seconds), the unit is considered stationary. Velocity is reset to zero, and gyro offsets are gradually re-learned. This reduces the velocity drift that would otherwise keep accumulating.

### Known Limitations

Yaw (heading) is calculated using gyroscope integration alone, so it gradually drifts over time. The MPU9250 magnetometer is **not** used in this version. Position error also increases with distance because of double integration. These are expected limitations of a low-cost IMU and explain why practical inertial navigation systems are periodically corrected using an external reference.

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

The sensor was mounted **upside-down**, so the code flips the axis signs (`FLIP_ABOUT_X`, `SX/SY/SZ`). If your board is mounted face-up, change the signs to `+1`.

## Project Files

| File | Purpose |
|---|---|
| `mpureadings/mpureadings.ino` | Arduino sketch that reads the IMU, runs the filter and dead-reckoning calculations, and sends CSV data over serial |
| `tools/plot_any.py` | Python script that plots the recorded path using matplotlib |

## How to Run

1. Open `mpureadings/mpureadings.ino` in the Arduino IDE and upload it at **115200 baud**.
2. During the 5-second calibration, keep the board **flat and completely still**.
3. Move the unit along your test path.
4. Serial commands:
   - `r` — resets position and heading to zero
   - `c` — runs calibration again

Serial output (10 Hz):

```text
ms,x,y,yaw_deg,zupt,roll_deg,pitch_deg
```

### Plotting the Path

Save the serial output to a text file (for example, `log.txt`), then run:

```bash
pip install matplotlib
python tools/plot_any.py log.txt
```

The script saves `log_plot.png` and displays the sample count, total path length, and start-to-end distance. The `ROTATE90` flag near the top of the script rotates the plot by 90 degrees to match the axis convention used by the sketch version that generated the logs.

## Results

*Add results after completing the walk test.*

| Test | Real path | Tracked end-point error | Notes |
|---|---|---|---|
| Square, 2 m sides | | | |

Place plot images in an `images/` folder and link them here.
