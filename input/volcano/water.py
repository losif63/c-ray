import numpy as np
import trimesh
from noise import pnoise3

# Define the water surface using Perlin noise
def water_surface(x, y, scale=1, amplitude=0.5, frequency=1, time=0, noise_amplitude = 0.3):
    z = np.zeros_like(x)
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            # Add Perlin noise
            perlin_noise = pnoise3(x[i, j] * scale, y[i, j] * scale, time, octaves=4, persistence=0.5, lacunarity=2.0)
            # Combine Perlin noise with a sine wave to simulate water waves
            z[i, j] = amplitude * perlin_noise * noise_amplitude + 0.1 * np.sin(frequency * (x[i, j] + y[i, j] + time))
    return z

# Generate a grid of x and y values
x = np.power(np.linspace(-1, 1, 500), 3) * 30
y = np.power(np.linspace(-1, 1, 500), 3) * 30
x, y = np.meshgrid(x, y)

# Simulate water surface at time t=0
z = water_surface(x, y, scale=0.5, amplitude=1, frequency=2, time=0)

# Create the 3D mesh data
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

vertices, faces = generate_mesh(x, y, z)
mesh = trimesh.Trimesh(vertices=vertices, faces=faces)

# Export to OBJ file
mesh.export('input/water_surface.obj')
