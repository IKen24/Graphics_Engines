#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

// Screen dimensions
#define WIDTH 800
#define HEIGHT 600

// Define colors
#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define BLUE  0x0000FF

// Player settings
#define FOV (M_PI / 3) // 60 degrees field of view
#define MOVEMENT_SPEED 0.05
#define ROTATION_SPEED 0.05
#define PLAYER_COLLISION_RADIUS 0.2
#define NUM_RAYS 240
#define MAX_DEPTH 16

// Map settings (1 is wall, 0 is empty space)
int MAP[7][8] = {
    {2, 1, 1, 1, 1, 1, 1, 2},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {2, 1, 1, 1, 1, 1, 1, 2}
};

// Function prototypes
void ray_casting(SDL_Renderer *renderer, float player_x, float player_y, float player_angle);
bool is_wall(int x, int y);
bool is_wall_collision(float x, float y);
void draw_minimap(SDL_Renderer *renderer, float player_x, float player_y, float player_angle);
void draw_ui(SDL_Renderer *renderer, float player_x, float player_y, float player_angle);

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("C Raycast Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Player starting position
    float player_x = 2.5f, player_y = 2.5f;
    float player_angle = 0.0f;

    // Frame rate control
    Uint32 frameStart;
    int frameTime;

    bool running = true;
    SDL_Event e;

    while (running) {
        frameStart = SDL_GetTicks();

        // Event handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Get keyboard state
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);

        // Store the player's original position (before movement)
        float original_x = player_x, original_y = player_y;

        // Move forward/backward
        if (keyState[SDL_SCANCODE_W]) {
            float next_x = player_x + cosf(player_angle) * MOVEMENT_SPEED;
            float next_y = player_y + sinf(player_angle) * MOVEMENT_SPEED;
            if (!is_wall_collision(next_x, next_y)) {
                player_x = next_x;
                player_y = next_y;
            }
        }
        if (keyState[SDL_SCANCODE_S]) {
            float next_x = player_x - cosf(player_angle) * MOVEMENT_SPEED;
            float next_y = player_y - sinf(player_angle) * MOVEMENT_SPEED;
            if (!is_wall_collision(next_x, next_y)) {
                player_x = next_x;
                player_y = next_y;
            }
        }

        // Rotate left/right
        if (keyState[SDL_SCANCODE_A]) {
            player_angle -= ROTATION_SPEED;
        }
        if (keyState[SDL_SCANCODE_D]) {
            player_angle += ROTATION_SPEED;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Cast rays and render the scene
        ray_casting(renderer, player_x, player_y, player_angle);

        // Draw minimap
        draw_minimap(renderer, player_x, player_y, player_angle);

        // Draw UI
        draw_ui(renderer, player_x, player_y, player_angle);

        // Update the display
        SDL_RenderPresent(renderer);

        // Frame rate control
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 1000 / 60) {
            SDL_Delay((1000 / 60) - frameTime);
        }
    }

    // Quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// Function to cast rays and draw the walls
void ray_casting(SDL_Renderer *renderer, float player_x, float player_y, float player_angle) {
    float ray_angle = player_angle - FOV / 2;
    float ray_step = FOV / NUM_RAYS;

    for (int ray = 0; ray < NUM_RAYS; ray++) {
        float ray_x = cosf(ray_angle);
        float ray_y = sinf(ray_angle);

        for (int depth = 0; depth < MAX_DEPTH * 10; depth++) {
            float target_x = player_x + ray_x * depth / 10;
            float target_y = player_y + ray_y * depth / 10;

            if (target_x < 0 || target_x >= 8 || target_y < 0 || target_y >= 7) {
                break;
            }

            if (MAP[(int)target_y][(int)target_x] > 0) {
                float distance = depth / 10.0f;
                int wall_height = (int)((HEIGHT / (distance + 0.0001)) * MAP[(int)target_y][(int)target_x]);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, ray * (WIDTH / NUM_RAYS), (HEIGHT - wall_height) / 2,
                                   ray * (WIDTH / NUM_RAYS), (HEIGHT + wall_height) / 2);
                break;
            }
        }

        ray_angle += ray_step;
    }
}

// Function to check if a position is within a wall
bool is_wall(int x, int y) {
    return MAP[y][x] == 1;
}

// Function to check for collision with walls
bool is_wall_collision(float x, float y) {
    if (is_wall((int)x, (int)y)) return true;
    if (is_wall((int)(x + PLAYER_COLLISION_RADIUS), (int)y)) return true;
    if (is_wall((int)(x - PLAYER_COLLISION_RADIUS), (int)y)) return true;
    if (is_wall((int)x, (int)(y + PLAYER_COLLISION_RADIUS))) return true;
    if (is_wall((int)x, (int)(y - PLAYER_COLLISION_RADIUS))) return true;
    return false;
}

// Function to draw the minimap
void draw_minimap(SDL_Renderer *renderer, float player_x, float player_y, float player_angle) {
    int MAP_SCALE = 10;

    for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 8; x++) {
            int color = (MAP[y][x] > 0) ? WHITE : BLACK;
            SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
            SDL_Rect tile = { x * MAP_SCALE, y * MAP_SCALE, MAP_SCALE, MAP_SCALE };
            SDL_RenderFillRect(renderer, &tile);
        }
    }

    // Draw player on the minimap
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect player_rect = { (int)(player_x * MAP_SCALE - 2.5f), (int)(player_y * MAP_SCALE - 2.5f), 5, 5 };
    SDL_RenderFillRect(renderer, &player_rect);

    // Draw player direction
    SDL_RenderDrawLine(renderer, (int)(player_x * MAP_SCALE), (int)(player_y * MAP_SCALE),
                       (int)((player_x + cosf(player_angle)) * MAP_SCALE),
                       (int)((player_y + sinf(player_angle)) * MAP_SCALE));
}

//
