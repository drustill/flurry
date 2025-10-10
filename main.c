#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480

#define NUM_FLURRIES 4
#define FLURRY_RADIUS 1
#define INITIAL_VELOCITY_X 0.0f
#define INITIAL_VELOCITY_Y 0.0f
#define VELOCITY_VARIANCE 20.0f
#define MAX_SPEED 100.0f
#define STEP 10.0f

#define DRAG 0.92f
#define LIFETIME 5.0f
#define MAX_PARTICLES 5000

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct {
  /* float cx, cy; */
  /* float vx, vy; */
  /* int r; */
  float base_angle;
  float length;
  float growth_rate;
  float curve_rate;
  double birthday;
  int active;
} Flurry;

typedef struct {
  float x, y, vx, vy;
  double birthday;
} Particle;

static int pidx[NUM_FLURRIES] = {0};
static Particle particles[NUM_FLURRIES][MAX_PARTICLES];
static Flurry flurries[NUM_FLURRIES];

static float center_x, center_y;

int main()
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("Flurry", WIDTH, HEIGHT, 0, &window, &renderer)) {
    SDL_Log("Couldn't create video/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

  srand(time(NULL));
  center_x = WIDTH / 2.0f;
  center_y = HEIGHT / 2.0f;

  for (int i = 0; i < NUM_FLURRIES; i++) {
    flurries[i].base_angle = ((float)i / NUM_FLURRIES) * 2.0f * M_PI;
    flurries[i].length = 0.0f;
    flurries[i].growth_rate = 50.0f + ((float)rand() / RAND_MAX) * 50.0f;
    flurries[i].curve_rate = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
    flurries[i].birthday = 0;
    flurries[i].active = 1;
  }

  uint64_t curr, prev;
  prev = SDL_GetTicks();

  int running = 1;
  while (running) {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    curr = SDL_GetTicks();
    const float elapsed = (float)(curr - prev) / 1000.0f;
    const double now = ((double)curr) / 1000.0;

    for (int i = 0; i < NUM_FLURRIES; i++) {
      if (!flurries[i].active && rand() % 60 == 0) {
        flurries[i].base_angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        flurries[i].length = 0.0f;
        flurries[i].growth_rate = 50.0f + ((float)rand() / RAND_MAX) * 100.0f;
        flurries[i].curve_rate = ((float)rand() / RAND_MAX - 0.5f) * 1.0f;
        flurries[i].birthday = now;
        flurries[i].active = 1;
      }
    }

    for (int i = 0; i < NUM_FLURRIES; i++) {
      if (!flurries[i].active) continue;

      flurries[i].length += flurries[i].growth_rate * elapsed;

      if (flurries[i].length > 300.0f) {
        flurries[i].active = 0;
        continue;
      }

      float current_angle = flurries[i].base_angle + flurries[i].curve_rate * flurries[i].length * 0.01f;
      float tip_x = center_x + cosf(current_angle) * flurries[i].length;
      float tip_y = center_y + sinf(current_angle) * flurries[i].length;

      for (int k = 0; k < 5; k++) {
        particles[i][pidx[i]].birthday = now;
        particles[i][pidx[i]].x = tip_x + ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        particles[i][pidx[i]].y = tip_y + ((float)rand() / RAND_MAX - 0.5f) * 10.0f;

        particles[i][pidx[i]].vx = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        particles[i][pidx[i]].vy = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;

        pidx[i] = (pidx[i] + 1) % MAX_PARTICLES;
      }
    }

    for (int i = 0; i < NUM_FLURRIES; i++) {
      for (int j = 0; j < MAX_PARTICLES; j++) {
        particles[i][j].x += particles[i][j].vx * elapsed;
        particles[i][j].y += particles[i][j].vy * elapsed;
        particles[i][pidx[i]].vx *= DRAG;
        particles[i][pidx[i]].vy *= DRAG;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < NUM_FLURRIES; i++) {
      const float red = (float) (0.5 + 0.5 * SDL_sin(now + i));
      const float green = (float) (0.5 + 0.5 * SDL_sin(i + now + SDL_PI_D * 2 / 3));
      const float blue = (float) (0.5 + 0.5 * SDL_sin(i + now + SDL_PI_D * 4 / 3));

      for (int j = 0; j < MAX_PARTICLES; j++) {
        float age = now - particles[i][j].birthday;
        float alpha = (1.0f - (age / LIFETIME)) * 0.4f;
        if (alpha > 0) {
          SDL_SetRenderDrawColorFloat(renderer, red, green, blue, alpha);
          SDL_FRect rect = {
            particles[i][j].x - 3.0f,
            particles[i][j].y - 3.0f,
            6.0f,
            6.0f
          };
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }

    SDL_RenderPresent(renderer);
    prev = curr;
  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return 0;
}
