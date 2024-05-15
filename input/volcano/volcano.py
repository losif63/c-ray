import numpy as np
import trimesh
from noise import pnoise3

# Define the function
def volcano(x, y):
    return 2.5 * np.exp(-0.4*(x**2 + y**2)) - 2.5 * np.exp(-30*((x-0.15)**2 + y**2))

def generate_mesh(x, y, z):
    vertices = np.column_stack([x.ravel(), y.ravel(), z.ravel()])
    faces = []
    num_x, num_y = x.shape
    for i in range(num_x - 1):
        for j in range(num_y - 1):
            idx = i * num_y + j
            faces.append([idx, idx + 1, idx + num_y])
            faces.append([idx + 1, idx + 1 + num_y, idx + num_y])
    return vertices, np.array(faces)

def add_perlin_noise(z, scale=3, octaves=10, persistence=0.2, lacunarity=2.0, noise_amplitude = 0.15):
    shape = z.shape
    noise = np.zeros(shape)
    for i in range(shape[0]):
        for j in range(shape[1]):
            noise[i][j] = pnoise3(x[i][j] * scale, y[i][j] * scale, 0, octaves, persistence, lacunarity) * noise_amplitude
    return z + noise

# Create a grid of x, y values
x = np.linspace(-3, 3, 100)
y = np.linspace(-3, 3, 100)
x, y = np.meshgrid(x, y)

# Calculate z values
z = volcano(x, y)

# Add perlin noise
z_noisy = add_perlin_noise(z)

vertices, faces = generate_mesh(x, y, z_noisy)
mesh = trimesh.Trimesh(vertices=vertices, faces=faces)

# Export to OBJ file
mesh.export('input/tempvolcano.obj')
