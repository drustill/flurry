#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480
#define NUM_MAGNETS 4

#define DRAG 0.99f
#define LIFETIME 8.0f
#define GRAVITY 500000.0f
#define MAX_PARTICLES 5000

#define INV_RAND_MAX (1.0f / RAND_MAX)
#define RANDF() (rand() * INV_RAND_MAX - 0.5f)

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct {
  float x, y;
  float phase;
  float orbit_speed;
  float orbit_radius;
} Magnet;

typedef struct {
  float x, y, vx, vy;
  double birthday;
} Particle;

static int pidx = 0;
static Particle particles[MAX_PARTICLES];
static Magnet magnets[NUM_MAGNETS];

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
  float center_x = WIDTH / 2.0f;
  float center_y = HEIGHT / 2.0f;

  for (int i = 0; i < NUM_MAGNETS; i++) {
    magnets[i].orbit_speed = 0.3f + (float)(NUM_MAGNETS - i) * 0.3f;
    magnets[i].orbit_radius = 80.0f + (float)(NUM_MAGNETS - i) * 20.0f;
    magnets[i].phase = (float)i * 2.0f * M_PI / NUM_MAGNETS;
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

    for (int i = 0; i < NUM_MAGNETS; i++) {
      float angle = now * magnets[i].orbit_speed + magnets[i].orbit_radius;
      magnets[i].x = center_x + cosf(angle) * magnets[i].orbit_radius;
      magnets[i].y = center_y + sinf(angle) * magnets[i].orbit_radius;
    }

    for (int k = 0; k < 3; k++) {
      particles[pidx].birthday = now;
      particles[pidx].x = center_x + RANDF() * 10.0f;
      particles[pidx].y = center_y + RANDF() * 10.0f;
      particles[pidx].vx = RANDF() * 20.0f;
      particles[pidx].vy = RANDF() * 20.0f;
      pidx = (pidx + 1) % MAX_PARTICLES;
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
      for (int j = 0; j < NUM_MAGNETS; j++) {
        float dx = particles[i].x - magnets[j].x;
        float dy = particles[i].y - magnets[j].y;
        float rsquared = dx*dx + dy*dy;

        if (rsquared < 2500.0f) {
          rsquared = 2500.0f;
        }

        float force = GRAVITY / rsquared;
        float magnitude = force / sqrt(rsquared);

        particles[i].vx -= (dx * magnitude) * elapsed;
        particles[i].vy -= (dy * magnitude) * elapsed;
      }

      particles[i].vx *= DRAG;
      particles[i].vy *= DRAG;

      particles[i].x += particles[i].vx * elapsed;
      particles[i].y += particles[i].vy * elapsed;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < NUM_MAGNETS; i++) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_FRect magnet_rect = { magnets[i].x - 5.0f, magnets[i].y - 5.0f, 10.0f, 10.0f };
      SDL_RenderFillRect(renderer, &magnet_rect);
    }

    const float r = 0.5f;
    const float g = 0.8f;
    const float b = 1.0f;
    /* const float r = (float) (0.4 + 0.4 * SDL_sin(now + i)); */
    /* const float g = (float) (0.4 + 0.4 * SDL_sin(i + now + SDL_PI_D * 2 / 3)); */
    /* const float b = (float) (0.4 + 0.4 * SDL_sin(i + now + SDL_PI_D * 4 / 3)); */

    for (int j = 0; j < MAX_PARTICLES; j++) {
      float age = now - particles[j].birthday;
      float alpha = (1.0f - (age / LIFETIME)) * 0.5f;

      if (alpha > 0) {
        SDL_SetRenderDrawColorFloat(renderer, r, g, b, alpha);
        SDL_FRect rect = {
          particles[j].x - 3.0f,
          particles[j].y - 3.0f,
          6.0f,
          6.0f
        };
        SDL_RenderFillRect(renderer, &rect);
      }
    }

    SDL_RenderPresent(renderer);
    prev = curr;
  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return 0;
}
