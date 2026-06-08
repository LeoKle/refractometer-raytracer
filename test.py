import pandas as pd
import matplotlib.pyplot as plt

df1 = pd.read_csv("intersections1.csv")
df2 = pd.read_csv("intersections2.csv")

datasets = [
    (df1, "intersections1"),
    (df2, "intersections2"),
]

# 3D plot
fig = plt.figure()
ax = fig.add_subplot(projection="3d")
for df, label in datasets:
    ax.scatter(df.x, df.y, df.z, s=0.5, alpha=1, label=label)
ax.legend()
plt.savefig("test_3d.png")
plt.show()

# 2D projections
fig, axes = plt.subplots(1, 3)
for df, label in datasets:
    axes[0].scatter(df.x, df.y, s=0.5, alpha=0.3, label=label)
    axes[1].scatter(df.x, df.z, s=0.5, alpha=0.3, label=label)
    axes[2].scatter(df.y, df.z, s=0.5, alpha=0.3, label=label)

axes[0].set(xlabel="x", ylabel="y")
axes[1].set(xlabel="x", ylabel="z")
axes[2].set(xlabel="y", ylabel="z")
axes[0].legend()
plt.savefig("test_2d.png")
plt.show()
