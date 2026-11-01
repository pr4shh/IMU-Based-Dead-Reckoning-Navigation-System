#include <Wire.h>

#define ZUPT_THRESH 0.30  // m/s^2 horizontal accel (gravity removed) below which we call it "still"
#define ZUPT_GYRO   0.08  // rad/s (~4.6 deg/s): must also be rotating slower than this
#define ZUPT_COUNT  12    // ...for this many samples in a row (~0.25 s)
#define ALPHA       0.98  // complementary filter: trust gyro 98%, accel 2%
#define G           9.81

// Upside-down mounting: 1 = flipped over about the sensor's X axis, 0 = about its Y axis.
// If forward AND left tests both read negative, toggle this.
#define FLIP_ABOUT_X 1

#if FLIP_ABOUT_X
  #define SX  1.0
  #define SY -1.0
#else
  #define SX -1.0
  #define SY  1.0
#endif
#define SZ -1.0

#define MPU 0x68

// ---------- state ----------   
float roll = 0, pitch = 0, yaw = 0;   // radians
float vx = 0, vy = 0;                 // m/s (world frame)
float px = 0, py = 0;                 // m   (world frame)
float gxB = 0, gyB = 0, gzB = 0;      // gyro offsets
float lax = 0, lay = 0, laz = G, lgx = 0, lgy = 0, lgz = 0;
int stillCount = 0;
bool zupt = false;
unsigned long lastMicros = 0;
int printDiv = 0;

float wrapPi(float a) {
  while (a >  PI) a -= 2 * PI;
  while (a < -PI) a += 2 * PI;
  return a;
}

void startI2C() {
  Wire.begin();
#ifdef WIRE_HAS_TIMEOUT
  Wire.setWireTimeout(3000, true);
#endif
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool readRegs(uint8_t reg, uint8_t n, uint8_t *buf) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)MPU, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

void wakeIMU() {
  writeReg(0x6B, 0x00);   // wake up
  delay(100);
  writeReg(0x1A, 0x03);   // gyro low-pass ~41 Hz (less noise)
  writeReg(0x1B, 0x00);   // gyro  +-250 dps
  writeReg(0x1C, 0x00);   // accel +-2 g
  writeReg(0x1D, 0x03);   // accel low-pass ~41 Hz
}

// Accel in m/s^2, gyro in rad/s, already converted to the "x forward, y left, z up" frame
void readIMU(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
  static uint8_t fails = 0;
  uint8_t b[14];
  if (readRegs(0x3B, 14, b)) {
    fails = 0;
    int16_t rax = (b[0] << 8) | b[1];
    int16_t ray = (b[2] << 8) | b[3];
    int16_t raz = (b[4] << 8) | b[5];
    int16_t rgx = (b[8] << 8) | b[9];
    int16_t rgy = (b[10] << 8) | b[11];
    int16_t rgz = (b[12] << 8) | b[13];
    lax = SX * rax / 16384.0 * G;
    lay = SY * ray / 16384.0 * G;
    laz = SZ * raz / 16384.0 * G;
    lgx = SX * rgx / 131.0 * 0.0174533;
    lgy = SY * rgy / 131.0 * 0.0174533;
    lgz = SZ * rgz / 131.0 * 0.0174533;
  } else if (++fails >= 5) {            // bus problem: reset it and wake the sensor
    fails = 0;
    Wire.end();
    startI2C();
    wakeIMU();
  }
  ax = lax; ay = lay; az = laz; gx = lgx; gy = lgy; gz = lgz;
}

// Keep the board FLAT and COMPLETELY STILL while this runs
void calibrate() {
  Serial.println("HOLD STILL for 5 seconds...");
  float sax = 0, say = 0, saz = 0, sgx = 0, sgy = 0, sgz = 0;
  float ax, ay, az, gx, gy, gz;
  const int N = 250;
  for (int i = 0; i < N; i++) {
    readIMU(ax, ay, az, gx, gy, gz);
    sax += ax; say += ay; saz += az;
    sgx += gx; sgy += gy; sgz += gz;
    delay(20);
  }
  float mx = sax / N, my = say / N, mz = saz / N;
  gxB = sgx / N; gyB = sgy / N; gzB = sgz / N;
  roll  = atan2(my, mz);                        // starting tilt from gravity
  pitch = atan2(-mx, sqrt(my * my + mz * mz));
  yaw = 0; vx = 0; vy = 0; px = 0; py = 0; stillCount = 0;

  Serial.print("Start roll/pitch (deg): ");
  Serial.print(roll * 57.2958, 1); Serial.print(" / ");
  Serial.println(pitch * 57.2958, 1);
  Serial.println("Calibrated. Running.  (type r = zero position, c = recalibrate)");
  Serial.println("ms,x,y,yaw_deg,zupt,roll_deg,pitch_deg");
  lastMicros = micros();
}

void setup() {
  Serial.begin(115200);
  startI2C();
  wakeIMU();
  uint8_t who = 0;
  readRegs(0x75, 1, &who);
  Serial.print("WHO_AM_I = 0x"); Serial.println(who, HEX);
  calibrate();
}

void loop() {
  // Serial commands: r = zero position/heading, c = recalibrate
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') { px = 0; py = 0; vx = 0; vy = 0; yaw = 0; stillCount = 0; Serial.println("zeroed"); }
    if (c == 'c') calibrate();
  }

  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0;
  if (dt < 0.02) return;                 // run at ~50 Hz
  lastMicros = now;
  if (dt > 0.2) dt = 0.02;               // ignore a long stall

  float ax, ay, az, gx, gy, gz;
  readIMU(ax, ay, az, gx, gy, gz);
  float rawGx = gx, rawGy = gy, rawGz = gz;
  gx -= gxB; gy -= gyB; gz -= gzB;

  // 1) Orientation: integrate the gyro (roll, pitch, yaw)
  float sr = sin(roll), cr = cos(roll), cp = cos(pitch), tp = tan(pitch);
  roll  += (gx + (gy * sr + gz * cr) * tp) * dt;
  pitch += (gy * cr - gz * sr) * dt;
  yaw   += ((gy * sr + gz * cr) / cp) * dt;

  // 2) Complementary filter: pull roll/pitch toward the accelerometer's gravity direction
  float an = sqrt(ax * ax + ay * ay + az * az);
  if (fabs(an - G) < 1.0) {              // only trust accel when it's mostly gravity
    float rollAcc  = atan2(ay, az);
    float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az));
    roll  = ALPHA * roll  + (1 - ALPHA) * rollAcc;
    pitch = ALPHA * pitch + (1 - ALPHA) * pitchAcc;
  }
  yaw = wrapPi(yaw);

  // 3) Rotate body acceleration into the world frame (this also cancels gravity in X/Y)
  sr = sin(roll); cr = cos(roll);
  float sp = sin(pitch); cp = cos(pitch);
  float sy = sin(yaw),   cy = cos(yaw);
  float awx = (cy * cp) * ax + (cy * sp * sr - sy * cr) * ay + (cy * sp * cr + sy * sr) * az;
  float awy = (sy * cp) * ax + (sy * sp * sr + cy * cr) * ay + (sy * sp * cr - cy * sr) * az;

  // 4) Zero-velocity update: barely accelerating AND barely rotating for a while = stopped
  float amag = sqrt(awx * awx + awy * awy);
  float gmag = sqrt(gx * gx + gy * gy + gz * gz);
  if (amag < ZUPT_THRESH && gmag < ZUPT_GYRO) stillCount++; else stillCount = 0;
  zupt = (stillCount >= ZUPT_COUNT);

  if (zupt) {
    vx = 0; vy = 0;                      // kills accumulated velocity drift
    gxB += 0.02 * (rawGx - gxB);         // while stopped, slowly re-learn gyro offsets
    gyB += 0.02 * (rawGy - gyB);
    gzB += 0.02 * (rawGz - gzB);
  } else {
    vx += awx * dt;                      // accel -> velocity
    vy += awy * dt;
    px += vx * dt;                       // velocity -> position
    py += vy * dt;
  }

  // 5) Output at 10 Hz
  if (++printDiv >= 5) {
    printDiv = 0;
    Serial.print(millis()); Serial.print(',');
    Serial.print(px, 3);    Serial.print(',');
    Serial.print(py, 3);    Serial.print(',');
    Serial.print(yaw * 57.2958, 1); Serial.print(',');
    Serial.print(zupt ? 1 : 0); Serial.print(',');
    Serial.print(roll * 57.2958, 1); Serial.print(',');
    Serial.println(pitch * 57.2958, 1);
  }
}
