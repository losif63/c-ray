import numpy as np
import trimesh
from noise import pnoise3
from PIL import Image

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

# Create a grid of x, y values
x = np.linspace(-3, 3, 100)
z = np.linspace(-3, 3, 100)
x, z = np.meshgrid(x, z)

# Calculate z values
y = volcano(x, z)

# Add perlin noise
y_noisy = add_perlin_noise(y)
vertex_normals = calculate_normals(x, y_noisy, z).reshape(-1, 3)
vertices, faces = generate_mesh(x, y_noisy, z)

# Generate UV coordinates
u = (x - x.min()) / (x.max() - x.min())
v = (z - z.min()) / (z.max() - z.min())
uv_coords = np.column_stack([u.ravel(), v.ravel()])

# Load the fiery texture
texture_image = Image.open('input/fiery_texture.jpg')
texture_image = texture_image.resize((512, 512))  # Resize if necessary

# Create a Trimesh object with UV coordinates
mesh = trimesh.Trimesh(vertices=vertices, faces=faces, vertex_normals=vertex_normals, visual=trimesh.visual.TextureVisuals(uv=uv_coords, image=texture_image))

# Export to OBJ file
mesh.export('input/tempvolcano.obj')

