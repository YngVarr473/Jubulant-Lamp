import pygame
import sys
import random
import psutil
import os
import numpy as np
import hashlib
from concurrent.futures import ThreadPoolExecutor, as_completed
import ctypes

# Initialize Pygame
pygame.init()

# Player settings
player_size = 50
player_color = (0, 128, 255)
player_speed = 5

# Map settings
tile_size = 150
tile_meidum_size = 30
tile_small_size = 50
map_width = 50  # Number of tiles wide
map_height = 50  # Number of tiles high

# Define the asset directory
asset_dir = os.path.abspath("../Assets")

# Function to load and resize images
def load_image(file_name, size):
    file_path = os.path.join(asset_dir, file_name)
    print(f"Loading image from: {file_path}")  # Debug print
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"File not found: {file_path}")
    try:
        image = pygame.image.load(file_path)
        return pygame.transform.scale(image, size)
    except pygame.error as e:
        raise RuntimeError(f"Error loading image {file_path}: {e}")

# Load and resize images efficiently using ThreadPoolExecutor
def load_images():
    with ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(load_image, "center.png", (tile_size, tile_size)): "grass_image",
            executor.submit(load_image, "water.png", (tile_size, tile_size)): "water_image",
            executor.submit(load_image, "forest.png", (tile_size, tile_size)): "forest_image",
            executor.submit(load_image, "rock_big.png", (tile_size, tile_size)): "rock_big_image",
            executor.submit(load_image, "rock_medium.png", (tile_meidum_size, tile_meidum_size)): "rock_medium_image",
            executor.submit(load_image, "romashka_big.png", (tile_small_size, tile_small_size)): "romasha_big_image",
            executor.submit(load_image, "flower2_big.png", (tile_small_size, tile_small_size)): "flower2_big_image",
            executor.submit(load_image, "bush_big.png", (tile_small_size, tile_small_size)): "bush_big_image",
            executor.submit(load_image, "Character/w1.png", (player_size, player_size)): "character_w1_image",
            executor.submit(load_image, "Character/w2.png", (player_size, player_size)): "character_w2_image",
            executor.submit(load_image, "Character/w3.png", (player_size, player_size)): "character_w3_image",
            executor.submit(load_image, "Character/w4.png", (player_size, player_size)): "character_w4_image",
            executor.submit(load_image, "Character/w5.png", (player_size, player_size)): "character_w5_image",
            executor.submit(load_image, "Character/w6.png", (player_size, player_size)): "character_w6_image",
            executor.submit(load_image, "Character/r1.png", (player_size, player_size)): "character_r1_image",
            executor.submit(load_image, "Character/r2.png", (player_size, player_size)): "character_r2_image",
            executor.submit(load_image, "Character/s1.png", (player_size, player_size)): "character_s1_image",  # Idle sprite
        }
        images = {}
        for future in as_completed(futures):
            key = futures[future]
            images[key] = future.result()
        return images

images = load_images()
grass_image = sand_image = images["grass_image"]
water_image = images["water_image"]
forest_image = images["forest_image"]
rock_big_image = images["rock_big_image"]
rock_medium_image = images["rock_medium_image"]
romasha_big_image = images["romasha_big_image"]
flower2_big_image = images["flower2_big_image"]
bush_big_image = images["bush_big_image"]

# Character sprites
character_images = [
    images["character_w1_image"],
    images["character_w2_image"],
    images["character_w3_image"],
    images["character_w4_image"],
    images["character_w5_image"],
    images["character_w6_image"]
]

# Function to calculate the hash of all files in the asset directory
def calculate_hash_of_files(directory):
    hash_md5 = hashlib.md5()
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            with open(file_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    hash_md5.update(chunk)
    return hash_md5.hexdigest()

# Calculate the hash of all files in the asset directory
file_hash = calculate_hash_of_files('.')

class Player(pygame.sprite.Sprite):
    def __init__(self, map_data, tile_size):
        super().__init__()
        self.walk_images = character_images  # Original walking animation frames
        self.run_images = [
            images["character_r1_image"],
            images["character_r2_image"]
        ]  # Running animation frames
        self.idle_image = images["character_s1_image"]  # Idle sprite
        self.down_images = [
            load_image("Character/d1.png", (player_size, player_size)),
            load_image("Character/d2.png", (player_size, player_size)),
            load_image("Character/d3.png", (player_size, player_size)),
            load_image("Character/d4.png", (player_size, player_size)),
            load_image("Character/d5.png", (player_size, player_size)),
            load_image("Character/d6.png", (player_size, player_size))
        ]  # Down animation frames
        self.up_images = [
            load_image("Character/u1.png", (player_size, player_size)),
            load_image("Character/u2.png", (player_size, player_size)),
            load_image("Character/u3.png", (player_size, player_size)),
            load_image("Character/u4.png", (player_size, player_size)),
            load_image("Character/u5.png", (player_size, player_size)),
            load_image("Character/u6.png", (player_size, player_size))
        ]  # Up animation frames
        self.images = self.walk_images  # Start with walking animation
        self.index = 0
        self.image = self.images[self.index]
        self.rect = self.image.get_rect()
        self.rect.center = (map_width * tile_size // 2, map_height * tile_size // 2)  # Start in the center of the map
        self.map_data = map_data
        self.tile_size = tile_size
        self.animation_speed = 10  # Speed of the animation in frames
        self.animation_counter = 0
        self.flipped = False  # Track the flip state
        self.current_direction = None  # Track the current direction
        self.running = False  # Track if the player is running
        self.moving = False  # Track if the player is moving
        self.speed = player_speed  # Normal walking speed
        self.run_speed = player_speed * 2  # Running speed

    def update(self, keys_pressed):
        new_x, new_y = self.rect.x, self.rect.y
        new_direction = None

        current_speed = self.run_speed if keys_pressed[pygame.K_LSHIFT] or keys_pressed[pygame.K_RSHIFT] else self.speed

        if keys_pressed[pygame.K_LEFT]:
            new_x -= current_speed
            new_direction = 'left'
        if keys_pressed[pygame.K_RIGHT]:
            new_x += current_speed
            new_direction = 'right'
        if keys_pressed[pygame.K_UP]:
            new_y -= current_speed
            new_direction = 'up'
        if keys_pressed[pygame.K_DOWN]:
            new_y += current_speed
            new_direction = 'down'

        # Check if the new position is on a water tile
        player_bottom_y = (new_y + player_size) // self.tile_size
        player_top_y = new_y // self.tile_size
        player_left_x = new_x // self.tile_size
        player_right_x = (new_x + player_size) // self.tile_size

        if (0 <= player_left_x < len(self.map_data[0]) and 0 <= player_bottom_y < len(self.map_data) and
            0 <= player_right_x < len(self.map_data[0]) and 0 <= player_top_y < len(self.map_data)):
            if (self.map_data[player_bottom_y][player_left_x] != 0 and
                self.map_data[player_bottom_y][player_right_x] != 0):
                self.rect.x = new_x
                self.rect.y = new_y

        # Update animation
        self.animation_counter += 1
        if self.animation_counter >= self.animation_speed:
            self.animation_counter = 0
            self.index = (self.index + 1) % len(self.images)
            self.image = self.images[self.index]

        # Flip the image based on the direction change
        if new_direction != self.current_direction:
            self.current_direction = new_direction
            if new_direction == 'left':
                self.images = self.walk_images
                self.flipped = False
            elif new_direction == 'right':
                self.images = self.walk_images
                self.flipped = True
            elif new_direction == 'down':
                self.images = self.down_images
                self.index = 0
            elif new_direction == 'up':
                self.images = self.up_images
                self.index = 0

        # Apply the flip state to the image
        if self.flipped:
            self.image = pygame.transform.flip(self.images[self.index], True, False)
        else:
            self.image = self.images[self.index]

        # Handle running animation
        if keys_pressed[pygame.K_LSHIFT] or keys_pressed[pygame.K_RSHIFT]:
            if not self.running:
                self.running = True
                self.images = self.run_images
                self.index = 0
        else:
            if self.running:
                self.running = False
                if self.current_direction == 'down':
                    self.images = self.down_images
                elif self.current_direction == 'up':
                    self.images = self.up_images
                else:
                    self.images = self.walk_images
                self.index = 0

        # Check if the player is moving
        self.moving = new_direction is not None

        # Set the image to the idle sprite if not moving
        if not self.moving:
            self.image = self.idle_image

# Camera class
class Camera:
    def __init__(self, width, height):
        self.camera = pygame.Rect(0, 0, width, height)
        self.width = width
        self.height = height

    def apply(self, rect):
        return rect.move(self.camera.topleft)

    def update(self, target):
        x = -target.rect.centerx + int(self.width / 2)
        y = -target.rect.centery + int(self.height / 2)

        # Limit scrolling to map size
        x = min(0, x)  # Left
        y = min(0, y)  # Top
        x = max(-(map_width * tile_size - self.width), x)  # Right
        y = max(-(map_height * tile_size - self.height), y)  # Bottom

        self.camera = pygame.Rect(x, y, self.width, self.height)

    def resize(self, width, height):
        self.width = width
        self.height = height
        self.camera.width = width
        self.camera.height = height

class Map:
    def __init__(self, width, height, tile_size):
        self.width = width
        self.height = height
        self.tile_size = tile_size
        self.map_data = self.generate_mapv2()
        self.map_surface = self.create_map_surface()

        self.props_data = self.create_props_map()
        self.props_surface = self.create_props_surface()

    def create_props_map(self):
        map_matrix = self.map_data

        matrix_height = self.height
        matrix_width = self.width
        for i in range(1, matrix_height - 1):
            for j in range(1, matrix_width - 1):
                if map_matrix[i][j] == 2 and random.random() < 0.1:  # 10% chance to place a forest on grass
                    map_matrix[i][j] = 3  # Forest tile
                if map_matrix[i][j] == 2 and random.random() < 0.03:  # 5% chance to place a rock on grass
                    map_matrix[i][j] = 4  # Rock tile
                if map_matrix[i][j] == 2 and random.random() < 0.03:  # 5% chance to place a rock on grass
                    map_matrix[i][j] = 5  # Rock tile

                if map_matrix[i][j] == 2 and random.random() < 0.01:  # 1% chance to place a romasha big on grass
                    map_matrix[i][j] = 6  # romasha_big tile
                if map_matrix[i][j] == 2 and random.random() < 0.02:  # 1% chance to place a flower2 big on grass
                    map_matrix[i][j] = 7  # flower2_big tile

                if map_matrix[i][j] == 2 and random.random() < 0.02:  # 1% chance to place a bush on grass
                    map_matrix[i][j] = 8  # bush tile

        return map_matrix

    def create_props_surface(self):
        props_surface = pygame.Surface((self.width * self.tile_size, self.height * self.tile_size), pygame.SRCALPHA)
        # Draw the forest layer on top of the grass tiles
        for y, row in enumerate(self.props_data):
            for x, tile in enumerate(row):
                if tile == 3:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(forest_image, rect)
                if tile == 4:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(rock_big_image, rect)
                if tile == 5:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(rock_medium_image, rect)
                if tile == 6:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(romasha_big_image, rect)
                if tile == 7:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(flower2_big_image, rect)
                if tile == 8:
                    rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                    props_surface.blit(bush_big_image, rect)

        return props_surface

    def generate_mapv2(self):
        # Load the C library
        map_generator = ctypes.CDLL('./map_generator.so')

        # Define the argument and return types
        map_generator.generate_map.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]
        map_generator.generate_map.restype = None

        # Create a numpy array to hold the map data
        map_data = np.zeros((self.height, self.width), dtype=np.int32)

        # Call the C function
        map_generator.generate_map(map_data.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), self.width, self.height)

        return map_data

    def create_map_surface(self):
        map_surface = pygame.Surface((self.width * self.tile_size, self.height * self.tile_size), pygame.SRCALPHA)

        # Draw the base layer (grass, sand, water)
        for y, row in enumerate(self.map_data):
            for x, tile in enumerate(row):
                rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                if tile == 1:
                    image = sand_image  # Sand tile image
                elif tile == 2:
                    image = grass_image  # Grass tile image
                elif tile == 0:
                    image = water_image  # Water tile image
                else:
                    continue
                map_surface.blit(image, rect)
        return map_surface

    def draw(self, screen, camera):
        screen.blit(self.map_surface, camera.apply(self.map_surface.get_rect()))
        screen.blit(self.props_surface, camera.apply(self.props_surface.get_rect()))

# Game class
class Game:
    def __init__(self):
        self.width = 500
        self.height = 500
        self.flags = pygame.RESIZABLE
        self.screen = pygame.display.set_mode((self.width, self.height), self.flags)
        self.running = True
        self.background_color = (255, 0, 0)

        self.map = Map(map_width, map_height, tile_size)
        self.player = Player(self.map.map_data, tile_size)
        self.all_sprites = pygame.sprite.Group()
        self.all_sprites.add(self.player)

        self.camera = Camera(self.width, self.height)

        self.start_time = pygame.time.get_ticks()  # Track the start time
        self.current_phase = "Day"  # Initialize the phase
        self.__update()

    def __eventHandler(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.VIDEORESIZE:
                self.width, self.height = event.size
                self.screen = pygame.display.set_mode((self.width, self.height), self.flags)
                self.camera.resize(self.width, self.height)
                self.camera.update(self.player)

    def __draw(self):
        self.map.draw(self.screen, self.camera)
        for sprite in self.all_sprites:
            self.screen.blit(sprite.image, self.camera.apply(sprite.rect))

        # Draw the file hash on the left bottom of the screen
        font = pygame.font.Font(None, 24)
        hash_text = font.render(f"build-hash: {file_hash}", True, (255, 255, 255))
        self.screen.blit(hash_text, (10, self.height - 30))

        # Draw the in-game time in the top-right corner
        elapsed_time = (pygame.time.get_ticks() - self.start_time) // 1000  # Convert milliseconds to seconds
        hours = elapsed_time // 60
        minutes = elapsed_time % 60
        time_text = font.render(f"{hours:02}:{minutes:02}", True, (255, 255, 255))
        self.screen.blit(time_text, (self.width - 60, 10))

        # Draw the current phase
        phase_text = font.render(f"Phase: {self.current_phase}", True, (255, 255, 255))
        self.screen.blit(phase_text, (self.width - 160, 40))

    def __update(self):
        clock = pygame.time.Clock()
        font = pygame.font.Font(None, 36)
        while self.running:
            self.__eventHandler()
            keys_pressed = pygame.key.get_pressed()
            self.all_sprites.update(keys_pressed)
            self.camera.update(self.player)
            self.screen.fill(self.background_color)
            self.__draw()

            # Update the current phase based on the elapsed time
            elapsed_time = (pygame.time.get_ticks() - self.start_time) // 1000  # Convert milliseconds to seconds
            if elapsed_time % 50 < 15:
                self.current_phase = "Day"
            elif elapsed_time % 50 < 25:
                self.current_phase = "Dusk"
            elif elapsed_time % 50 < 35:
                self.current_phase = "Night"
            elif elapsed_time % 50 < 45:
                self.current_phase = "Dawn"

            # Display FPS and CPU/GPU usage
            fps = clock.get_fps()
            cpu_usage = psutil.cpu_percent(interval=None)
            gpu_usage = psutil.virtual_memory().percent  # Using virtual memory as a placeholder for GPU usage
            fps_text = font.render(f"FPS: {fps:.2f}", True, (255, 255, 255))
            cpu_text = font.render(f"CPU: {cpu_usage:.2f}%", True, (255, 255, 255))
            gpu_text = font.render(f"GPU: {gpu_usage:.2f}%", True, (255, 255, 255))
            self.screen.blit(fps_text, (10, 10))
            self.screen.blit(cpu_text, (10, 40))
            self.screen.blit(gpu_text, (10, 70))

            pygame.display.flip()
            clock.tick(60)

        pygame.quit()
        sys.exit()

# Create and run the game
game = Game()
