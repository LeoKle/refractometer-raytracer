import numpy as np
import matplotlib.pyplot as plt

resolution_x = 640
resolution_y = 512

img = np.fromfile("image.bin", dtype=np.float32).reshape(resolution_y, resolution_x)
plt.imshow(img, cmap="gray", origin="lower")
plt.colorbar()
plt.show()
