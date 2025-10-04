#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480
#define FLURRY_RADIUS 50

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct {
  int cx;
  int cy;
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

  Flurry flurry = { WIDTH / 2.0f, HEIGHT / 2.0f, FLURRY_RADIUS };

  int running = 1;
  while (running) {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = 0;
      }
    }

    const double now = ((double)SDL_GetTicks()) / 1000.0;

    flurry.cx = WIDTH / 2.0f + 200.0f * SDL_sinf(now);
    flurry.cy = HEIGHT / 2.0f + 150.0f * SDL_cosf(now * 0.8f);

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

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);

  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();

  return 0;
}
