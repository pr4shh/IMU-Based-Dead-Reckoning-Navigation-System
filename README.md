[README.md](https://github.com/user-attachments/files/34237577/README.md)
# IMU-Based Dead Reckoning Navigation System

A handheld unit that tracks its own X-Y position and heading using only an IMU (MPU-series accelerometer + gyroscope). No GPS, no camera, no external signal. Built for the *Aerospace Defence Electronics Basics* course as a simplified version of the inertial navigation used in GPS-denied scenarios (aircraft, missiles, submarines).

## How it works

1. **Calibration (5 s, board still and flat):** averages 250 samples to find the gyro offsets and the starting roll/pitch from the direction of gravity.
2. **Orientation:** roll, pitch and yaw are updated by integrating the gyroscope.
3. **Complementary filter:** roll and pitch are pulled 2% per step toward the tilt measured by the accelerometer (`ALPHA = 0.98`, 98% gyro / 2% accel). This is only applied when the measured acceleration is close to 1 g, so it is not fooled while the unit is accelerating.
4. **World-frame acceleration:** body acceleration is rotated into the world frame using roll, pitch and yaw. This also cancels gravity in X and Y.
5. **Double integration:** acceleration is integrated to velocity, and velocity to position.
6. **Zero-velocity update (ZUPT):** if horizontal acceleration is below 0.30 m/s² **and** rotation is below 0.08 rad/s for 12 samples in a row (about 0.25 s), the unit is treated as stopped. Velocity is reset to zero and the gyro offsets are slowly re-learned. This removes most of the velocity drift that would otherwise grow without bound.

### Known limitation

Yaw (heading) comes from gyro integration only, so it drifts slowly over time. The magnetometer in the MPU9250 is **not** used in this version. Position error also grows with distance because of double integration. Both are expected for a low-cost IMU and are the reason real inertial navigation systems are periodically corrected by an external reference.

## Hardware

| Part | Notes |
|---|---|
| Arduino Uno (or ESP32) | |
| MPU-series IMU module (MPU9250 board) | Read directly over I2C at address `0x68`, no library needed |
| Jumper wires, USB cable | |

### Wiring

| IMU pin | Arduino Uno | ESP32 |
|---|---|---|
| VCC | 5V (check your module) | 3.3V |
| GND | GND | GND |
| SCL | A5 | GPIO22 |
| SDA | A4 | GPIO21 |

The sensor was mounted **upside-down**, so axis signs are flipped in the code (`FLIP_ABOUT_X`, `SX/SY/SZ`). If your board is mounted face-up, set the signs to `+1`.

## Files

| File | Purpose |
|---|---|
| `mpureadings/mpureadings.ino` | Arduino sketch: reads the IMU, runs the filter and dead reckoning, prints CSV over serial |
| `tools/plot_any.py` | Python script that plots the logged path with matplotlib |

## How to run

1. Open `mpureadings/mpureadings.ino` in Arduino IDE and upload it at 115200 baud.
2. Keep the board **flat and completely still** for the 5-second calibration.
3. Move the unit along your test path.
4. Serial commands: `r` zeroes position and heading, `c` recalibrates.

Serial output (10 Hz):

```
ms,x,y,yaw_deg,zupt,roll_deg,pitch_deg
```

### Plotting

Save the serial output to a text file (for example `log.txt`), then:

```
pip install matplotlib
python tools/plot_any.py log.txt
```

This saves `log_plot.png` and prints the sample count, total path length and start-to-end distance. The `ROTATE90` flag at the top of the script rotates the plot by 90 degrees to match the axis convention of the sketch version used for the logs.

## Results

*Add after the walk test.*

| Test | Real path | Tracked end-point error | Notes |
|---|---|---|---|
| Square, 2 m sides | | | |

Add your plot images to an `images/` folder and link them here.
