import numpy as np
from PIL import Image
from noise import pnoise2

# Image dimensions
width, height = 512, 512

# Center of the yellow area
center_x, center_y = int(width / 2), int(height / 2)

# Create a new image
image = Image.new('RGB', (width, height))
pixels = image.load()

# Colors
white_color = (255, 255, 0)
yellow_color = (255, 0, 0)
brown_color = (39, 24, 4)

hole_radius = 18
lava_radius = 75

# Function to interpolate between two colors
def lerp_color(color1, color2, t):
    return tuple(int(a + (b - a) * t) for a, b in zip(color1, color2))

# Generate the radial gradient with yellow in the center and dark at the edges
for i in range(width):
    for j in range(height):
        # Distance from the center
        dist = np.sqrt((i - 10 - center_x)**2 + (j - center_y)**2)
        t = (dist - hole_radius) / (lava_radius - hole_radius)
        
        # Set pixel color based on distance
        if dist <= hole_radius:
            pixels[i, j] = brown_color
        elif dist <= lava_radius:
            pixels[i, j] = lerp_color(white_color, yellow_color, t)
        else:
            pixels[i, j] = brown_color

# Add Perlin noise for reddish colors
scale = 10.0
octaves = 6
persistence = 0.5
lacunarity = 2.0

# for i in range(width):
#     for j in range(height):
#         # Generate Perlin noise value
#         noise_value = pnoise2(i / scale, j / scale, octaves=octaves, persistence=persistence, lacunarity=lacunarity)
        
#         # Normalize noise value to [0, 1]
#         noise_value = (noise_value + 1) / 2
        
#         # Apply reddish color based on noise
#         r, g, b = pixels[i, j]
#         r = int(r + noise_value * 255 * 0.5) # Mix in some red based on noise
#         g = int(g * (1 - noise_value * 0.5)) # Reduce green component based on noise
#         b = int(b * (1 - noise_value * 0.5)) # Reduce blue component based on noise
        
#         pixels[i, j] = (r, g, b)

# Save the image
image.save('input/fiery_texture.jpg')
image.show()
