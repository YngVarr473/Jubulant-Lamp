import pygame
import sys
import random
import psutil
import os
import numpy as np

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

# Load and resize images efficiently
grass_image= sand_image = load_image("center.png", (tile_size, tile_size))
water_image = load_image("water.png", (tile_size, tile_size))
forest_image = load_image("forest.png", (tile_size, tile_size))
rock_big_image = load_image("rock_big.png", (tile_size, tile_size))
rock_medium_image = load_image("rock_medium.png", (tile_meidum_size, tile_meidum_size))
romasha_big_image = load_image("romashka_big.png", (tile_small_size, tile_small_size))
flower2_big_image = load_image("flower2_big.png", (tile_small_size, tile_small_size))
bush_big_image = load_image("bush_big.png", (tile_small_size, tile_small_size))

# Player class
class Player(pygame.sprite.Sprite):
    def __init__(self, map_data, tile_size):
        super().__init__()
        self.image = pygame.Surface((player_size, player_size))
        self.image.fill(player_color)
        self.rect = self.image.get_rect()
        self.rect.center = (map_width * tile_size // 2, map_height * tile_size // 2)  # Start in the center of the map
        self.map_data = map_data
        self.tile_size = tile_size
    def update(self, keys_pressed):
        new_x, new_y = self.rect.x, self.rect.y
        if keys_pressed[pygame.K_LEFT]:
            new_x -= player_speed
        if keys_pressed[pygame.K_RIGHT]:
            new_x += player_speed
        if keys_pressed[pygame.K_UP]:
            new_y -= player_speed
        if keys_pressed[pygame.K_DOWN]:
            new_y += player_speed

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

                if map_matrix[i][j] == 2 and random.random() < 0.02:  # 1% chance to place a romasha big on grass
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
        map_matrix = np.zeros((self.height, self.width), dtype=int)

        # Initialize the map with water tiles around the borders
        map_matrix[0, :] = 0
        map_matrix[-1, :] = 0
        map_matrix[:, 0] = 0
        map_matrix[:, -1] = 0
        map_matrix[1:-1, 1:-1] = 2  # Grass tile (temporary, will be overwritten)

        # Generate the rest of the map
        for i in range(1, self.height - 1):
            for j in range(1, self.width - 1):
                tile = random.randint(0, 10)  # Generate a random number between 0 and 10
                if tile < 2:  # 20% chance for water
                    map_matrix[i, j] = 0
                elif tile < 5:  # 30% chance for sand
                    map_matrix[i, j] = 1
                else:  # 50% chance for grass
                    map_matrix[i, j] = 2

        # Remove isolated water tiles
        for i in range(1, self.height - 1):
            for j in range(1, self.width - 1):
                if map_matrix[i, j] == 0:
                    neighbors = [
                        (i-1, j), (i+1, j), (i, j-1), (i, j+1)
                    ]
                    has_water_neighbor = any(
                        0 <= ni < self.height and 0 <= nj < self.width and map_matrix[ni, nj] == 0
                        for ni, nj in neighbors
                    )
                    if not has_water_neighbor:
                        map_matrix[i, j] = 2  # Replace isolated water with grass
        return map_matrix

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
        # Calculate the visible area
        start_x = max(0, -camera.camera.x // self.tile_size)
        end_x = min(self.width, (start_x + camera.width // self.tile_size) + 1)
        start_y = max(0, -camera.camera.y // self.tile_size)
        end_y = min(self.height, (start_y + camera.height // self.tile_size) + 1)

        # Draw the visible tiles
        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                if self.map_data[y][x] == 1:
                    image = sand_image
                elif self.map_data[y][x] == 2:
                    image = grass_image
                elif self.map_data[y][x] == 0:
                    image = water_image
                else:
                    continue
                screen.blit(image, camera.apply(rect))

        # Draw the props layer
        for y in range(start_y, end_y):
            for x in range(start_x, end_x):
                rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                if self.props_data[y][x] == 3:
                    image = forest_image
                elif self.props_data[y][x] == 4:
                    image = rock_big_image
                elif self.props_data[y][x] == 5:
                    image = rock_medium_image
                elif self.props_data[y][x] == 6:
                    image = romasha_big_image
                elif self.props_data[y][x] == 7:
                    image = flower2_big_image
                elif self.props_data[y][x] == 8:
                    image = bush_big_image
                else:
                    continue
                screen.blit(image, camera.apply(rect))

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
