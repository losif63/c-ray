import numpy as np
import trimesh
from noise import pnoise3

# Define the water surface using Perlin noise
def water_surface(x, y, scale, amplitude, frequency, time):
    z = np.zeros_like(x)
    for i in range(x.shape[0]):
        for j in range(x.shape[1]):
            # Add Perlin noise
            perlin_noise = pnoise3(x[i, j] * scale, y[i, j] * scale, time, octaves=8, persistence=0.5, lacunarity=2.0)
            # Combine Perlin noise with a sine wave to simulate water waves
            z[i, j] = amplitude * perlin_noise + 0.05 * np.sin(frequency * (x[i, j] + y[i, j] + time))
    return z

# Calculate normals
def calculate_normals(x, y, z):
    dzdx = np.gradient(z, axis=1, edge_order=2) / np.gradient(x, axis=1, edge_order=2)
    dzdy = np.gradient(z, axis=0, edge_order=2) / np.gradient(y, axis=0, edge_order=2)
    normal = np.zeros((x.shape[0], x.shape[1], 3))
    normal[..., 0] = -dzdx
    normal[..., 1] = -dzdy
    normal[..., 2] = 1
    norm = np.linalg.norm(normal, axis=2)
    normal[..., 0] /= norm
    normal[..., 1] /= norm
    normal[..., 2] /= norm
    return normal

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

# Generate a grid of x and y values
x = np.concatenate([np.linspace(-5000, -100, 10), np.linspace(-100, -45, 55), np.linspace(-45, -25, 40), np.linspace(-25, -10, 60), np.linspace(-10, 10, 160), np.linspace(10, 25, 60), np.linspace(25, 45, 40), np.linspace(45, 100, 55), np.linspace(100, 5000, 10)])
z = np.concatenate([np.linspace(-5000, -100, 10), np.linspace(-100, -45, 55), np.linspace(-45, -25, 40), np.linspace(-25, -10, 60), np.linspace(-10, 10, 160), np.linspace(10, 25, 60), np.linspace(25, 45, 40), np.linspace(45, 100, 55), np.linspace(100, 5000, 10)])
x, z = np.meshgrid(x, z)

# Simulate water surface at time t=0
y = water_surface(x, z, scale=0.25, amplitude=0.35, frequency=2, time=0)
vertex_normals = calculate_normals(x, y, z).reshape(-1, 3)
vertices, faces = generate_mesh(x, y, z)
mesh = trimesh.Trimesh(vertices=vertices, faces=faces, vertex_normals=vertex_normals)

# Export to OBJ file
mesh.export('input/water_surface.obj')
