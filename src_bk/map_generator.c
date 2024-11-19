// map_generator.c
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Function to generate the map
void generate_map(int *map_data, int width, int height) {
    srand(time(NULL));

    // Initialize the map with water tiles around the borders
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                map_data[i * width + j] = 0;  // Water tile
            } else {
                map_data[i * width + j] = 2;  // Grass tile (temporary, will be overwritten)
            }
        }
    }

    // Generate the rest of the map
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int tile = rand() % 11;  // Generate a random number between 0 and 10
            if (tile < 2) {  // 20% chance for water
                map_data[i * width + j] = 0;
            } else if (tile < 5) {  // 30% chance for sand
                map_data[i * width + j] = 1;
            } else {  // 50% chance for grass
                map_data[i * width + j] = 2;
            }
        }
    }

    // Remove isolated water tiles
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            if (map_data[i * width + j] == 0) {
                int neighbors[4][2] = {
                    {i-1, j}, {i+1, j}, {i, j-1}, {i, j+1}
                };
                int has_water_neighbor = 0;
                for (int k = 0; k < 4; k++) {
                    int ni = neighbors[k][0];
                    int nj = neighbors[k][1];
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width && map_data[ni * width + nj] == 0) {
                        has_water_neighbor = 1;
                        break;
                    }
                }
                if (!has_water_neighbor) {
                    map_data[i * width + j] = 2;  // Replace isolated water with grass
                }
            }
        }
    }
}
