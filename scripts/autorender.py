import numpy as np
import json
import subprocess
import os

# Template for the JSON object
translate = {
    "type": "translate",
    "x": 0,
    "y": 1,
    "z": 0
}

rotate = {
    "type": "rotateY",
    "degrees": 0
}

# Load the existing JSON file
with open('input/volcanic_archipelago.json', 'r') as f:
    existing_data = json.load(f)

thetas = np.linspace(-30, 30, 120)

existing_data["camera"]["transforms"]

for i in range(len(thetas)):
    if os.path.isfile(f"output/project/scene{i:03d}.png"):
        continue
    theta = thetas[i]
    existing_data["camera"]["transforms"] = []

    json_trans = json.loads(json.dumps(translate))
    json_trans["x"] = 10 * np.sin(np.deg2rad(theta))
    json_trans["z"] = -10 * np.cos(np.deg2rad(theta))
    existing_data["camera"]["transforms"].extend([json_trans])

    json_rot = json.loads(json.dumps(rotate))
    json_rot["degrees"] = theta
    existing_data["camera"]["transforms"].extend([json_rot])
    # print(existing_data["camera"]["transforms"])
    with open('input/tempscene.json', 'w+') as f:
        json.dump(existing_data, f, indent=4)
    subprocess.call(["bin/c-ray", "input/tempscene.json"])
    subprocess.call(["mv", "output/rendered_0000.png", f"output/project/scene{i:03d}.png"])

