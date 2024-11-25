#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to generate the map
int** generateMap(int height, int width) {
    // Allocate memory for the 2D array
    int** map_data = (int**)malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        map_data[i] = (int*)malloc(width * sizeof(int));
    }

    // Initialize the map with water tiles on the borders
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                map_data[i][j] = 0;  // Water tile
            }
        }
    }

    // Fill the inner part of the map with random tiles
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int tile = rand() % 11;  // Generate a random number between 0 and 10
            if (tile < 2) {  // 20% chance for water
                map_data[i][j] = 0;
            } else if (tile < 5) {  // 30% chance for sand
                map_data[i][j] = 1;
            } else {  // 50% chance for grass
                map_data[i][j] = 2;
            }
        }
    }

    // Replace isolated water tiles with grass
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            if (map_data[i][j] == 0) {
                int neighbors[4][2] = {
                    {i-1, j}, {i+1, j}, {i, j-1}, {i, j+1}
                };
                int has_water_neighbor = 0;
                for (int k = 0; k < 4; k++) {
                    int ni = neighbors[k][0];
                    int nj = neighbors[k][1];
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width && map_data[ni][nj] == 0) {
                        has_water_neighbor = 1;
                        break;
                    }
                }
                if (!has_water_neighbor) {
                    map_data[i][j] = 2;  // Replace isolated water with grass
                }
            }
        }
    }

    printf("Map data generated successfully.\n");
    return map_data;
}

// Function to free the allocated memory for the map
void freeMap(int** map_data, int height) {
    for (int i = 0; i < height; i++) {
        free(map_data[i]);
    }
    free(map_data);
}

// Function to rotate the matrix by 90 degrees clockwise
int** rotateMatrix90(int** map_data, int height, int width) {
    int** rotated_map = (int**)malloc(width * sizeof(int*));
    for (int i = 0; i < width; i++) {
        rotated_map[i] = (int*)malloc(height * sizeof(int));
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            rotated_map[j][height - 1 - i] = map_data[i][j];
        }
    }

    // Free the original map
    freeMap(map_data, height);

    return rotated_map;
}

int main() {
    int height = 10;  // Example height
    int width = 10;   // Example width

    // Seed the random number generator
    srand(time(NULL));

    // Generate the map
    int** map_data = generateMap(height, width);

    // Print the original map (for debugging purposes)
    printf("Original Map:\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%d ", map_data[i][j]);
        }
        printf("\n");
    }

    // Rotate the map by 90 degrees
    map_data = rotateMatrix90(map_data, height, width);

    // Print the rotated map (for debugging purposes)
    printf("Rotated Map:\n");
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            printf("%d ", map_data[i][j]);
        }
        printf("\n");
    }

    // Free the allocated memory
    freeMap(map_data, width);

    return 0;
}


