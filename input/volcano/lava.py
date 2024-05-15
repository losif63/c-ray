import numpy as np
import json
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Number of points
n_points = 800

# Initial position
initial_position = np.array([0, 2.7, 0])
initial_r = 0.5

# Generate random values
velocity = np.random.normal(loc=3, scale=1.5, size=n_points)
velocity = np.clip(velocity, 0, 6)  # Ensuring velocity is within [0, 6]

theta = np.random.uniform(0, 2 * np.pi, size=n_points)
phi = np.random.uniform(np.pi / 6, np.pi / 3, size=n_points)
time = np.power(np.random.uniform(0, 1, size=n_points), 1.5) * 4
radius = np.random.normal(0.015, 0.015, size=n_points)

# Calculate positions
x = initial_position[0] + initial_r * np.cos(theta) + velocity * np.cos(phi) * np.cos(theta) * time
y = initial_position[1] + velocity * np.sin(phi) * time - 0.5 * 2.5 * (time ** 2)
z = initial_position[2] + initial_r * np.sin(theta) + velocity * np.cos(phi) * np.sin(theta) * time

# for a in zip(velocity, theta, phi, time, x, y, z):
#     print(a)

# # Plotting the points
# fig = plt.figure()
# ax = fig.add_subplot(111, projection='3d')
# ax.scatter(x, y, z, c='b', marker='o')

# # Labels and title
# ax.set_xlabel('X')
# ax.set_ylabel('Y')
# ax.set_zlabel('Z')
# ax.set_title('3D Positions of Random Points')

# # Show plot
# plt.show()

# Template for the JSON object
template = {
    "fileName": "sphere10.obj",
    "pick_instances": [
        {
            "for": "sphere10",
            "materials": [
                {
                    "replace": "Unknown",
                    "type": "emissive",
                    "color": {
                        "type": "blackbody",
                        "degrees": 800
                    },
                    "strength": 30
                }
            ],
            "transforms": [
                {
                    "type": "scaleUniform",
                    "scale": 1
                },
                {
                    "type": "translate",
                    "x": 0,
                    "y": 0,
                    "z": 0
                }
            ]
        }
    ]
}

# Load the existing JSON file
with open('input/test.json', 'r') as f:
    existing_data = json.load(f)

# Filter out objects with "fileName" equal to "sphere10.obj"
existing_data["scene"]["meshes"] = [
    mesh for mesh in existing_data["scene"]["meshes"]
    if mesh["fileName"] != "sphere10.obj"
]

# List to store the JSON objects
json_objects = []

# Create JSON objects based on the template and numpy array values
for i in range(len(x)):
    json_object = json.loads(json.dumps(template))  # Create a deep copy of the template
    json_object["pick_instances"][0]["transforms"][1]["x"] = x[i]
    json_object["pick_instances"][0]["transforms"][1]["y"] = y[i]
    json_object["pick_instances"][0]["transforms"][1]["z"] = z[i]
    json_object["pick_instances"][0]["transforms"][0]["scale"] = radius[i]
    json_objects.append(json_object)

# Append the new JSON objects to the "meshes" list in the existing data
existing_data["scene"]["meshes"].extend(json_objects)

# Save the updated JSON structure back to the file
with open('input/test.json', 'w') as f:
    json.dump(existing_data, f, indent=4)
