#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define FLURRY_RADIUS 50

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct {
  float cx;
  float cy;
  float vx;
  float vy;
  int r;
} Flurry;

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

  Flurry flurry = { WIDTH / 2.0f, HEIGHT / 2.0f, 50.0f, 30.0f, FLURRY_RADIUS };

  uint64_t curr, prev;
  prev = SDL_GetTicks();

  const float can_change = 500.0f;
  const float max_speed = 500.0f;

  srand(time(NULL));

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

    flurry.vx += ((float)rand() / RAND_MAX - 0.5f) * can_change * elapsed;
    flurry.vy += ((float)rand() / RAND_MAX - 0.5f) * can_change * elapsed;

    float speed = SDL_sqrtf(flurry.vx * flurry.vx + flurry.vy * flurry.vy);
    if (speed > max_speed) {
      flurry.vx = (flurry.vx / speed) * max_speed;
      flurry.vy = (flurry.vy / speed) * max_speed;
    }

    flurry.cx += flurry.vx * elapsed;
    flurry.cy += flurry.vy * elapsed;

    if (flurry.cx < flurry.r || flurry.cx > WIDTH - flurry.r) {
      flurry.vx *= -1;
      flurry.cx = SDL_clamp(flurry.cx, (float)flurry.r, WIDTH - (float)flurry.r);
    }
    if (flurry.cy < flurry.r || flurry.cy > HEIGHT - flurry.r) {
      flurry.vy *= -1;
      flurry.cy = SDL_clamp(flurry.cy, (float)flurry.r, HEIGHT - (float)flurry.r);
    }

    const double now = ((double)curr) / 1000.0;
    prev = curr;

    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));

    SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */
    for (int dy = -flurry.r; dy <= flurry.r; dy++) {
      for (int dx = -flurry.r; dx <= flurry.r; dx++) {
        if (dx*dx + dy*dy <= flurry.r*flurry.r) {
          SDL_RenderPoint(renderer, flurry.cx + dx, flurry.cy + dy);
        }
      }
    }

    SDL_RenderPresent(renderer);

  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return 0;
}
