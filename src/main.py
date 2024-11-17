import pygame
import sys
import random
import psutil

# Initialize Pygame
pygame.init()

# Player settings
player_size = 50
player_color = (0, 128, 255)
player_speed = 5

# Map settings
tile_size = 50
map_width = 50  # Number of tiles wide
map_height = 50  # Number of tiles high

# Load and resize images
grass_image = pygame.transform.scale(pygame.image.load("../Assets/grass.png"), (tile_size, tile_size))
sand_image = pygame.transform.scale(pygame.image.load("../Assets/sand.png"), (tile_size, tile_size))
water_image = pygame.transform.scale(pygame.image.load("../Assets/water.png"), (tile_size, tile_size))
forest_image = pygame.transform.scale(pygame.image.load("../Assets/forest.png"), (tile_size, tile_size))

# Player class
class Player(pygame.sprite.Sprite):
    def __init__(self):
        super().__init__()
        self.image = pygame.Surface((player_size, player_size))
        self.image.fill(player_color)
        self.rect = self.image.get_rect()
        self.rect.center = (map_width * tile_size // 2, map_height * tile_size // 2)  # Start in the center of the map

    def update(self, keys_pressed):
        if keys_pressed[pygame.K_LEFT]:
            self.rect.x -= player_speed
        if keys_pressed[pygame.K_RIGHT]:
            self.rect.x += player_speed
        if keys_pressed[pygame.K_UP]:
            self.rect.y -= player_speed
        if keys_pressed[pygame.K_DOWN]:
            self.rect.y += player_speed

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

# Map class
class Map:
    def __init__(self, width, height, tile_size):
        self.width = width
        self.height = height
        self.tile_size = tile_size
        self.map_data = self.generate_mapv2()
        self.map_surface = self.create_map_surface()

    def generate_mapv2(self):
        map_matrix = []

        matrix_height = self.height
        matrix_width = self.width

        for i in range(matrix_height):
            layer = []
            for j in range(matrix_width):
                tile = random.randint(0, 10)  # Generate a random number between 0 and 10
                if tile < 2:  # 20% chance for water
                    layer.append(0)
                elif tile < 5:  # 30% chance for sand
                    layer.append(1)
                else:  # 50% chance for grass
                    layer.append(2)
            map_matrix.append(layer)

        # Place forest tiles on some grass tiles
        for i in range(matrix_height):
            for j in range(matrix_width):
                if map_matrix[i][j] == 2 and random.random() < 0.1:  # 10% chance to place a forest on grass
                    map_matrix[i][j] = 3  # Forest tile

        # Remove isolated water tiles
        for i in range(matrix_height):
            for j in range(matrix_width):
                if map_matrix[i][j] == 0:
                    neighbors = [
                        (i-1, j), (i+1, j), (i, j-1), (i, j+1)
                    ]
                    has_water_neighbor = any(
                        0 <= ni < matrix_height and 0 <= nj < matrix_width and map_matrix[ni][nj] == 0
                        for ni, nj in neighbors
                    )
                    if not has_water_neighbor:
                        map_matrix[i][j] = 2  # Replace isolated water with grass

        return map_matrix

    def create_map_surface(self):
        map_surface = pygame.Surface((self.width * self.tile_size, self.height * self.tile_size))
        for y, row in enumerate(self.map_data):
            for x, tile in enumerate(row):
                rect = pygame.Rect(x * self.tile_size, y * self.tile_size, self.tile_size, self.tile_size)
                if tile == 1:
                    image = sand_image  # Border tile image
                elif tile == 2:
                    image = grass_image  # Object tile image
                elif tile == 3:
                    image = forest_image  # Forest tile image
                elif tile == 0:
                    image = water_image  # Empty tile image
                else:
                    continue
                map_surface.blit(image, rect)
        return map_surface

    def draw(self, screen, camera):
        screen.blit(self.map_surface, camera.apply(self.map_surface.get_rect()))

# Game class
class Game:
    def __init__(self):
        self.width = 500
        self.height = 500
        self.flags = pygame.RESIZABLE
        self.screen = pygame.display.set_mode((self.width, self.height), self.flags)
        self.running = True
        self.background_color = (0, 0, 0)

        self.player = Player()
        self.all_sprites = pygame.sprite.Group()
        self.all_sprites.add(self.player)

        self.camera = Camera(self.width, self.height)
        self.map = Map(map_width, map_height, tile_size)

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
