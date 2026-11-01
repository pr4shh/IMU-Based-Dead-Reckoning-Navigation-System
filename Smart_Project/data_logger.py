import sys
import matplotlib.pyplot as plt

fname = sys.argv[1] if len(sys.argv) > 1 else "log.txt"
ROTATE90 = True   # True for the OLD sketch (what you are running now)

xs, ys = [], []
for line in open(fname, errors="ignore"):
    if "WHO_AM_I" in line:
        xs, ys = [], []
        continue
    parts = line.strip().split(",")
    if len(parts) != 7:
        continue
    try:
        ms, x, y, yaw, z, roll, pitch = map(float, parts)
    except ValueError:
        continue
    if ROTATE90:
        x, y = -y, x
    xs.append(x)
    ys.append(y)

if not xs:
    sys.exit("No data lines found in " + fname)

path = sum(((xs[i+1]-xs[i])**2 + (ys[i+1]-ys[i])**2) ** 0.5
           for i in range(len(xs) - 1))
end_dist = (xs[-1] ** 2 + ys[-1] ** 2) ** 0.5

plt.figure(figsize=(6, 6))
plt.plot(xs, ys, "b-", linewidth=1.5, label="Tracked path")
plt.plot(xs[0], ys[0], "go", markersize=10, label="Start")
plt.plot(xs[-1], ys[-1], "kx", markersize=12, markeredgewidth=3, label="End (tracked)")
plt.axis("equal")
plt.grid(True)
plt.legend()
plt.xlabel("X forward (m)")
plt.ylabel("Y left (m)")
plt.title("IMU dead reckoning: tracked path\n"
          f"distance from start = {end_dist:.2f} m")
plt.savefig(fname.replace(".txt", "_plot.png"), dpi=150)
plt.show()
print(f"samples: {len(xs)}   path length: {path:.2f} m   start-to-end: {end_dist:.2f} m")
