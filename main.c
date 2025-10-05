#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480

#define NUM_FLURRIES 3
#define FLURRY_RADIUS 20
#define INITIAL_VELOCITY_X 50.0f
#define INITIAL_VELOCITY_Y 40.0f
#define VELOCITY_VARIANCE 1000.0f
#define MAX_SPEED 300.0f
#define STEP 10.0f

#define MAX_PARTICLES 50000

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef struct {
  float cx, cy, vx, vy;
  int r;
} Flurry;

typedef struct {
  float x, y;
} Particle;

static int pidx[NUM_FLURRIES] = {0};
static Particle particles[NUM_FLURRIES][MAX_PARTICLES];
static Flurry flurries[NUM_FLURRIES];

static float center_x, center_y, center_vx, center_vy;

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

  srand(time(NULL));
  center_x = WIDTH / 2.0f;
  center_y = HEIGHT / 2.0f;
  center_vx = INITIAL_VELOCITY_X;
  center_vy = INITIAL_VELOCITY_Y;

  for (int i = 0; i < NUM_FLURRIES; i++) {
    flurries[i].cx = 0;
    flurries[i].cy = 0;
    flurries[i].vx = 0;
    flurries[i].vy = 0;
    flurries[i].r = FLURRY_RADIUS;
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

    if (rand() % 200 == 0) {
      const float angle = ((float)rand() / RAND_MAX - 0.5f) * 0.08f;
      float tmp_vx = center_vx;
      center_vx = center_vx * cosf(angle) - center_vy * sinf(angle);
      center_vy = tmp_vx * sinf(angle) + center_vy * cosf(angle);
    }

    center_vx += ((float)rand() / RAND_MAX - 0.5f) * VELOCITY_VARIANCE * elapsed;
    center_vy += ((float)rand() / RAND_MAX - 0.5f) * VELOCITY_VARIANCE * elapsed;

    if (rand() % 20000 == 0) {
      center_vx *= -1;
      center_vy *= -1;
    }

    float speed = SDL_sqrtf(center_vx * center_vx + center_vy * center_vy);
    if (speed > MAX_SPEED) {
      center_vx = (center_vx / speed) * MAX_SPEED;
      center_vy = (center_vy / speed) * MAX_SPEED;
    }

    center_x += center_vx * elapsed;
    center_y += center_vy * elapsed;

    if (center_x < FLURRY_RADIUS || center_x > WIDTH - FLURRY_RADIUS) {
      center_vx *= -1;
      center_x = SDL_clamp(center_x, (float)FLURRY_RADIUS, WIDTH - (float)FLURRY_RADIUS);
    }
    if (center_y < FLURRY_RADIUS || center_y > HEIGHT - FLURRY_RADIUS) {
      center_vy *= -1;
      center_y = SDL_clamp(center_y, (float)FLURRY_RADIUS, HEIGHT - (float)FLURRY_RADIUS);
    }

    const double now = ((double)curr) / 1000.0;

    for (int i = 0; i < NUM_FLURRIES; i++) {
      float angle = (i/(float)NUM_FLURRIES)*2.0f*M_PI + now * 0.5f;
      float radius = 20.0f + 20.0f * sinf(now + i);

      flurries[i].cx = center_x + cosf(angle) * radius;
      flurries[i].cy = center_y + sinf(angle) * radius;

      particles[i][pidx[i]].x = flurries[i].cx + ((float)rand() / RAND_MAX - 0.5f) * 15.0f;
      particles[i][pidx[i]].y = flurries[i].cy + ((float)rand() / RAND_MAX - 0.5f) * 15.0f;
      pidx[i] = (pidx[i] + 1) % MAX_PARTICLES;
    }

    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));

    SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    for (int i = 0; i < NUM_FLURRIES; i++) {
      SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */
      SDL_RenderPoints(renderer, (SDL_FPoint*)particles[i], MAX_PARTICLES);

      for (int dy = -flurries[i].r; dy <= flurries[i].r; dy++) {
        for (int dx = -flurries[i].r; dx <= flurries[i].r; dx++) {
          if (dx*dx + dy*dy <= flurries[i].r*flurries[i].r) {
            SDL_RenderPoint(renderer, flurries[i].cx + dx, flurries[i].cy + dy);
          }
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
