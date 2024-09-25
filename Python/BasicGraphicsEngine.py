import pygame
import math


# Initialize pygame
pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Python Raycast Engine")

# Define colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
BLUE = (0, 0, 255)

# Set the clock for controlling frame rate
clock = pygame.time.Clock()

# Map settings (1 is wall, 0 is empty space)
MAP = [
    [2, 1, 1, 1, 1, 1, 1, 2],
    [1, 0, 0, 0, 0, 0, 0, 1],
    [1, 0, 0, 0, 0, 0, 0, 1],
    [1, 0, 0, 0, 0, 0, 0, 1],
    [1, 0, 0, 0, 0, 0, 0, 1],
    [1, 0, 0, 0, 0, 0, 0, 1],
    [2, 1, 1, 1, 1, 1, 1, 2],
]

# Load the texture for the walls
texture = pygame.image.load('Desktop/Graphics_Engines/Python/wall.jpg')
texture = pygame.transform.scale(texture, (64, 64))  # Scale texture for easier mapping

# Player starting position and settings
player_x, player_y = 2.5, 2.5  # Start in the middle of the map
player_angle = 0  # Player's looking angle (radians)
FOV = math.pi / 3  # Field of view (60 degrees)

# Raycasting settings
NUM_RAYS = 240  # Number of rays cast for each frame
MAX_DEPTH = 16   # Maximum depth the ray can travel

# Movement settings
MOVEMENT_SPEED = 0.05  # Adjusted for finer movement
ROTATION_SPEED = 0.05
PLAYER_COLLISION_RADIUS = 0.2  # Radius of collision around player

# Font for UI
font = pygame.font.SysFont('Arial', 24)

# Function to cast rays and draw the textured walls
def ray_casting(screen, player_x, player_y, player_angle):
    ray_angle = player_angle - FOV / 2  # Starting angle for the first ray
    ray_step = FOV / NUM_RAYS           # Step between rays

    for ray in range(NUM_RAYS):
        # Calculate each ray's direction
        ray_x = math.cos(ray_angle)
        ray_y = math.sin(ray_angle)

        # Step through the map until a wall is hit
        for depth in range(MAX_DEPTH * 10):  # Increase precision by multiplying by 10
            target_x = player_x + ray_x * depth / 10
            target_y = player_y + ray_y * depth / 10

            # If the ray is outside the map boundaries, stop
            if target_x < 0 or target_x >= len(MAP[0]) or target_y < 0 or target_y >= len(MAP):
                break

            # If the ray hits a wall (check the map)
            if MAP[int(target_y)][int(target_x)] > 0:
                # Calculate distance to the wall (for perspective)
                distance = depth / 10
                wall_height = (HEIGHT / (distance + 0.0001)) * MAP[int(target_y)][int(target_x)]

                # Find the exact hit position on the wall to get the correct texture offset
                hit_x = target_x - int(target_x)
                hit_y = target_y - int(target_y)

                if abs(hit_x) > abs(hit_y):
                    texture_x = hit_x
                else:
                    texture_x = hit_y

                texture_x = int(texture_x * texture.get_width()) % texture.get_width()

                # Extract the vertical slice of the texture
                texture_slice = pygame.transform.scale(texture.subsurface((texture_x, 0, 1, texture.get_height())),
                                                      (int(WIDTH / NUM_RAYS), int(wall_height)))

                # Draw the vertical slice of the wall using the texture
                screen.blit(texture_slice, (ray * (WIDTH / NUM_RAYS), (HEIGHT - wall_height) // 2))
                break  # Stop once we hit a wall
        
        ray_angle += ray_step  # Move to the next ray angle

# Function to check if a position is within a wall
def is_wall(x, y):
    return MAP[int(y)][int(x)] == 1

# Function to check collision with a radius
def is_wall_collision(x, y):
    if is_wall(x, y):
        return True
    # Check around the player for a small radius to prevent "sticking" to the walls
    if is_wall(x + PLAYER_COLLISION_RADIUS, y):
        return True
    if is_wall(x - PLAYER_COLLISION_RADIUS, y):
        return True
    if is_wall(x, y + PLAYER_COLLISION_RADIUS):
        return True
    if is_wall(x, y - PLAYER_COLLISION_RADIUS):
        return True
    return False

# Function to draw the minimap
def draw_minimap(screen, player_x, player_y, player_angle):
    MAP_SCALE = 10
    for y, row in enumerate(MAP):
        for x, tile in enumerate(row):
            color = WHITE if tile > 0 else BLACK
            pygame.draw.rect(screen, color, (x * MAP_SCALE, y * MAP_SCALE, MAP_SCALE, MAP_SCALE))

    # Draw player on the minimap
    pygame.draw.circle(screen, BLUE, (int(player_x * MAP_SCALE), int(player_y * MAP_SCALE)), 5)
    
    # Draw the direction the player is facing
    pygame.draw.line(screen, BLUE, 
                     (player_x * MAP_SCALE, player_y * MAP_SCALE),
                     ((player_x + math.cos(player_angle)) * MAP_SCALE, 
                      (player_y + math.sin(player_angle)) * MAP_SCALE), 2)

# Function to draw the UI
def draw_ui(screen, player_x, player_y, player_angle):
    position_text = font.render(f"Position: ({player_x:.2f}, {player_y:.2f})", True, WHITE)
    angle_text = font.render(f"Angle: {math.degrees(player_angle):.2f}°", True, WHITE)
    screen.blit(position_text, (10, HEIGHT - 50))
    screen.blit(angle_text, (10, HEIGHT - 30))

# Game loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Get the keys for movement
    keys = pygame.key.get_pressed()

    # Store the player's original position (before movement)
    original_x, original_y = player_x, player_y

    # Move forward/backward
    if keys[pygame.K_w]:
        next_x = player_x + math.cos(player_angle) * MOVEMENT_SPEED
        next_y = player_y + math.sin(player_angle) * MOVEMENT_SPEED

        # Check if the next position is a wall or not
        if not is_wall_collision(next_x, next_y):
            player_x = next_x
            player_y = next_y

    if keys[pygame.K_s]:
        next_x = player_x - math.cos(player_angle) * MOVEMENT_SPEED
        next_y = player_y - math.sin(player_angle) * MOVEMENT_SPEED

        # Check if the next position is a wall or not
        if not is_wall_collision(next_x, next_y):
            player_x = next_x
            player_y = next_y

    # Rotate left/right
    if keys[pygame.K_a]:
        player_angle -= ROTATION_SPEED
    if keys[pygame.K_d]:
        player_angle += ROTATION_SPEED

    # Clear the screen
    screen.fill(BLACK)

    # Cast rays and render the scene
    ray_casting(screen, player_x, player_y, player_angle)

    # Draw the minimap
    draw_minimap(screen, player_x, player_y, player_angle)

    # Draw UI
    draw_ui(screen, player_x, player_y, player_angle)

    # Update the display
    pygame.display.flip()

    # Control the frame rate
    clock.tick(60)

# Quit pygame
pygame.quit()
