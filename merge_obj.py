import trimesh
import numpy as np
import json

def uniform_scale_matrix(size: float):
    return np.array([
        [size, 0.0, 0.0, 0.0],
        [0.0, size, 0.0, 0.0],
        [0.0, 0.0, size, 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])

def rotation_x_matrix(theta: float):
    rad = np.deg2rad(theta)
    return np.array([
        [1.0, 0.0, 0.0, 0.0],
        [0.0, np.cos(rad), - np.sin(rad), 0.0],
        [1.0, np.sin(rad), np.cos(rad), 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])

def rotation_y_matrix(theta: float):
    rad = np.deg2rad(theta)
    return np.array([
        [np.cos(rad), 0.0, np.sin(rad), 0.0],
        [0.0, 1.0, 0.0, 0.0],
        [- np.sin(rad), 0.0, np.cos(rad), 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])

def rotation_z_matrix(theta: float):
    rad = np.deg2rad(theta)
    return np.array([
        [np.cos(rad), - np.sin(rad), 0.0, 0.0],
        [np.sin(rad), np.cos(rad), 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])

def translation_matrix(x, y, z):
    return np.array([
        [1.0, 0.0, 0.0, x],
        [0.0, 1.0, 0.0, y],
        [0.0, 0.0, 1.0, z],
        [0.0, 0.0, 0.0, 1.0]
    ])

with open('input/volcanic_archipelago.json', 'r') as f:
    existing_data = json.load(f)

scene = trimesh.Scene()

for m in existing_data["scene"]["meshes"]:
    fileName = "input/" + m["fileName"]
    transforms = m["pick_instances"][0]["transforms"]
    mesh = trimesh.load(fileName)

    if m["fileName"] == "volcano.obj" or m["fileName"] == "water_surface.obj":
        mesh.invert()

    t = np.array([
        [1.0, 0.0, 0.0, 0.0],
        [0.0, 1.0, 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, 1.0]
    ])
    for i in range(len(transforms)):
        if transforms[i]["type"] == "translate":
            x, y, z = transforms[i]["x"], transforms[i]["y"], transforms[i]["z"]
            t = np.matmul(t, translation_matrix(x, y, z))
    for i in range(len(transforms)):
        if transforms[i]["type"] == "rotateX":
            deg = transforms[i]["degrees"]
            t = np.matmul(t, rotation_x_matrix(deg))
        elif transforms[i]["type"] == "rotateY":
            deg = transforms[i]["degrees"]
            t = np.matmul(t, rotation_y_matrix(deg))
        elif transforms[i]["type"] == "rotateZ":
            deg = transforms[i]["degrees"]
            t = np.matmul(t, rotation_z_matrix(deg))
    for i in range(len(transforms)):
        if transforms[i]["type"] == "scaleUniform":
            scale = transforms[i]["scale"]
            t = np.matmul(t, uniform_scale_matrix(scale))

    transformed_mesh = mesh.apply_transform(t)
    scene.add_geometry(transformed_mesh)

with open("scene.obj", 'w+') as file:
    file.write(trimesh.exchange.obj.export_obj(scene))