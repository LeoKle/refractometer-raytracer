import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("lightSamples.csv")
fig = plt.figure()
ax = fig.add_subplot(projection="3d")
ax.scatter(df.x, df.y, df.z, s=0.5, alpha=0.3)
plt.savefig("test.png")
plt.show()
fig, axes = plt.subplots(1, 3)
axes[0].scatter(df.x, df.y, s=0.5)  # XY
axes[1].scatter(df.x, df.z, s=0.5)  # XZ
axes[2].scatter(df.y, df.z, s=0.5)  # YZ
plt.show()
