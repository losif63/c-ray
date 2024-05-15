import numpy as np
import math

def create_sphere(radius, divisions):
    vertices = []
    faces = []
    
    # Polar (not uniform) vertices generation
    for i in range(divisions + 1):
        lat = math.pi * i / divisions
        for j in range(divisions + 1):
            lon = 2 * math.pi * j / divisions
            x = radius * math.sin(lat) * math.cos(lon)
            y = radius * math.sin(lat) * math.sin(lon)
            z = radius * math.cos(lat)
            vertices.append((x, y, z))
    
    # Create faces
    for i in range(divisions):
        for j in range(divisions):
            current = i * (divisions + 1) + j
            next = current + divisions + 1
            faces.append((current + 1, next + 1, next + 2))
            faces.append((current + 1, next + 2, current + 2))
    
    return vertices, faces

def save_obj(filename, vertices, faces):
    with open(filename, 'w') as file:
        file.write("# OBJ file\n")
        for v in vertices:
            file.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
        for f in faces:
            file.write(f"f {f[0]} {f[1]} {f[2]}\n")

# Usage
radius = 1
divisions = 10  # More divisions, smoother sphere
vertices, faces = create_sphere(radius, divisions)
save_obj("input/sphere10.obj", vertices, faces)
