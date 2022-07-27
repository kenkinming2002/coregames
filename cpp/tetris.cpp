#include <SDL2/SDL.h>

#include <tuple>

#include <stdlib.h>
#include <assert.h>

static constexpr int BOARD_HEIGHT = 10;
static constexpr int BOARD_WIDTH  = 8;
static constexpr int TETROMINO_WIDTH = 4;

static constexpr int TETROMINO_COUNT = 7;

static constexpr int TILE_WIDTH  = 32;
static constexpr int TILE_MARGIN1 = 4;
static constexpr int TILE_MARGIN2 = 8;

static constexpr int WINDOW_HEIGHT = BOARD_HEIGHT * TILE_WIDTH;
static constexpr int WINDOW_WIDTH  = BOARD_WIDTH  * TILE_WIDTH;

enum class Color
{
  NONE,
  LIGHT_BLUE,
  BLUE,
  ORANGE,
  YELLOW,
  GREEN,
  PURPLE,
  RED,
};

using Tetromino = Color[TETROMINO_WIDTH][TETROMINO_WIDTH];
const Tetromino tetrominoes[TETROMINO_COUNT] = {
  // Line
  {
    {Color::LIGHT_BLUE, Color::LIGHT_BLUE, Color::LIGHT_BLUE,  Color::LIGHT_BLUE},
    {Color::NONE,       Color::NONE,       Color::NONE,        Color::NONE      },
    {Color::NONE,       Color::NONE,       Color::NONE,        Color::NONE      },
    {Color::NONE,       Color::NONE,       Color::NONE,        Color::NONE      },
  },

  // L shape
  {
    {Color::BLUE, Color::NONE, Color::NONE, Color::NONE},
    {Color::BLUE, Color::BLUE, Color::BLUE, Color::NONE},
    {Color::NONE, Color::NONE, Color::NONE, Color::NONE},
    {Color::NONE, Color::NONE, Color::NONE, Color::NONE},
  },
  {
    {Color::NONE,   Color::NONE,   Color::ORANGE, Color::NONE},
    {Color::ORANGE, Color::ORANGE, Color::ORANGE, Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE,   Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE,   Color::NONE},
  },

  // Square
  {
    {Color::YELLOW, Color::YELLOW, Color::NONE, Color::NONE},
    {Color::YELLOW, Color::YELLOW, Color::NONE, Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE, Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE, Color::NONE},
  },

  // Inverted Z shape
  {
    {Color::NONE,  Color::GREEN, Color::GREEN, Color::NONE},
    {Color::GREEN, Color::GREEN, Color::NONE,  Color::NONE},
    {Color::NONE,  Color::NONE,  Color::NONE,  Color::NONE},
    {Color::NONE,  Color::NONE,  Color::NONE,  Color::NONE},
  },

  // T shape
  {
    {Color::NONE,   Color::PURPLE, Color::NONE,   Color::NONE},
    {Color::PURPLE, Color::PURPLE, Color::PURPLE, Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE,   Color::NONE},
    {Color::NONE,   Color::NONE,   Color::NONE,   Color::NONE},
  },

  // Z shape
  {
    {Color::NONE,  Color::RED,  Color::RED,  Color::NONE},
    {Color::RED,   Color::RED,  Color::NONE, Color::NONE},
    {Color::NONE,  Color::NONE, Color::NONE, Color::NONE},
    {Color::NONE,  Color::NONE, Color::NONE, Color::NONE},
  },
};

const int tetromino_sizes[TETROMINO_COUNT] = { 4, 3, 3, 2, 3, 3, 3 };

enum class State
{
  RUNNING,
  LOSED
};

State current_state = State::RUNNING;

int current_y, current_x;

Tetromino current_tetromino;
int current_tetromino_size;

Color board[BOARD_HEIGHT][BOARD_WIDTH];

enum class Result
{
  OK,
  OVERLAP_Y,
  OVERLAP_X,
  COLLIDED,
};

bool between(int v, int lo, int hi)
{
  return lo<=v && v<=hi;
}

Result check()
{
  for(int y=0; y<TETROMINO_WIDTH; ++y)
    for(int x=0; x<TETROMINO_WIDTH; ++x)
    {
      if(current_tetromino[y][x] == Color::NONE)
        continue;

      if(!between(current_y+y, 0, BOARD_HEIGHT-1)) return Result::OVERLAP_Y;
      if(!between(current_x+x, 0, BOARD_WIDTH-1))  return Result::OVERLAP_X;
      if(board[current_y+y][current_x+x] != Color::NONE) return Result::COLLIDED;
    }

  return Result::OK;
}

void write_tetromino()
{
  for(int y=0; y<TETROMINO_WIDTH; ++y)
    for(int x=0; x<TETROMINO_WIDTH; ++x)
    {
      if(current_tetromino[y][x] == Color::NONE)
        continue;

      board[current_y+y][current_x+x] = current_tetromino[y][x];
    }
}

void rotate(bool direction)
{
  Tetromino old_tetromino, new_tetromino;

  // We have to think about the size of the pice
  memcpy(&old_tetromino, &current_tetromino, sizeof current_tetromino);
  memcpy(&new_tetromino, &current_tetromino, sizeof current_tetromino);
  for(int y=0; y<current_tetromino_size; ++y)
    for(int x=0; x<current_tetromino_size; ++x)
      new_tetromino[y][x] = direction
        ? old_tetromino[current_tetromino_size-x-1][y]
        : old_tetromino[x][current_tetromino_size-y-1];

  memcpy(&current_tetromino, &new_tetromino, sizeof current_tetromino);
  if(check() != Result::OK)
    memcpy(&current_tetromino, &old_tetromino, sizeof current_tetromino);
}

void eliminate_lines()
{
  int write_y = BOARD_HEIGHT-1;
  for(int y=BOARD_HEIGHT-1; y>=0; --y)
  {
    bool eliminate = true;
    for(int x=0; x<BOARD_WIDTH; ++x)
      if(board[y][x] == Color::NONE)
      {
        eliminate = false;
        break;
      }

    if(eliminate)
      continue;

    if(write_y != y)
      memcpy(&board[write_y], &board[y], sizeof board[write_y]);

    --write_y;
  }
}

void select_next_tetromino()
{
  current_y = 0;
  current_x = (BOARD_WIDTH - TETROMINO_WIDTH) / 2;

  int i = rand() % TETROMINO_COUNT;
  memcpy(&current_tetromino, &tetrominoes[i], sizeof current_tetromino);
  current_tetromino_size = tetromino_sizes[i];

  if(check() != Result::OK)
  {
    current_state = State::LOSED;
    return;
  }
}

void move(int y_offset, int x_offset)
{
  current_y+=y_offset;
  current_x+=x_offset;
  auto result = check();
  if(result == Result::OK)
    return;

  current_y-=y_offset;
  current_x-=x_offset;
  if(result == Result::OVERLAP_X)
    return;

  write_tetromino();
  eliminate_lines();
  select_next_tetromino();
}

void render_single_at(SDL_Renderer *renderer, size_t x, size_t y, Color color)
{
  if(color != Color::NONE)
  {
    uint8_t r1, g1, b1;
    switch(color)
    {
    case Color::LIGHT_BLUE: std::tie(r1, g1, b1) = std::make_tuple(80,  80, 200); break;
    case Color::BLUE:       std::tie(r1, g1, b1) = std::make_tuple(40,  40, 200); break;
    case Color::ORANGE:     std::tie(r1, g1, b1) = std::make_tuple(100, 40, 40);  break;
    case Color::YELLOW:     std::tie(r1, g1, b1) = std::make_tuple(70,  30, 30);  break;
    case Color::GREEN:      std::tie(r1, g1, b1) = std::make_tuple(30, 200, 30);  break;
    case Color::PURPLE:     std::tie(r1, g1, b1) = std::make_tuple(180, 40, 180); break;
    case Color::RED:        std::tie(r1, g1, b1) = std::make_tuple(200, 30, 30);  break;
    default:
      assert(false && "Unreachable");
    }

    uint8_t r2, g2, b2;
    r2 = (unsigned)r1 * 7 / 8;
    g2 = (unsigned)g1 * 7 / 8;
    b2 = (unsigned)b1 * 7 / 8;

    SDL_Rect rect1 = {};
    rect1.x = x * TILE_WIDTH + TILE_MARGIN1;
    rect1.y = y * TILE_WIDTH + TILE_MARGIN1;
    rect1.w = TILE_WIDTH - 2 * TILE_MARGIN1;
    rect1.h = TILE_WIDTH - 2 * TILE_MARGIN1;

    SDL_Rect rect2 = {};
    rect2.x = x * TILE_WIDTH + TILE_MARGIN2;
    rect2.y = y * TILE_WIDTH + TILE_MARGIN2;
    rect2.w = TILE_WIDTH - 2 * TILE_MARGIN2;
    rect2.h = TILE_WIDTH - 2 * TILE_MARGIN2;

    SDL_SetRenderDrawColor(renderer, r1, g1, b1, 255);
    SDL_RenderFillRect(renderer, &rect1);

    SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
    SDL_RenderFillRect(renderer, &rect2);
  }
}

void render(SDL_Renderer *renderer)
{
  for(size_t y=0; y<BOARD_HEIGHT; ++y)
    for(size_t x=0; x<BOARD_WIDTH; ++x)
      render_single_at(renderer, x, y, board[y][x]);

  if(current_state == State::RUNNING)
    for(size_t y=0; y<TETROMINO_WIDTH; ++y)
      for(size_t x=0; x<TETROMINO_WIDTH; ++x)
        render_single_at(renderer, current_x + x, current_y + y, current_tetromino[y][x]);
}

int main()
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "SDL Error:%s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;
  if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) == -1)
  {
    fprintf(stderr, "SDL Error:%s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  select_next_tetromino();

  bool quit = false;

  Uint32 old_time = SDL_GetTicks();
  Uint32 dt = 0;

  while(!quit)
  {
    SDL_Event event;
    while(SDL_PollEvent(&event))
      switch(event.type)
      {
      case SDL_QUIT: quit = true; break;
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
        case SDLK_r:
          if(current_state == State::RUNNING)
            rotate((event.key.keysym.mod & KMOD_LSHIFT) || (event.key.keysym.mod & KMOD_RSHIFT));
          break;
        case SDLK_LEFT:  if(current_state == State::RUNNING) move(0, -1); break;
        case SDLK_RIGHT: if(current_state == State::RUNNING) move(0,  1); break;
        case SDLK_DOWN:  if(current_state == State::RUNNING) move(1,  0); break;
        }
        break;
      }

    Uint32 new_time = SDL_GetTicks();
    dt += (new_time - old_time);
    old_time = new_time;

    while(dt >= 1000)
    {
      dt -= 1000;
      if(current_state == State::RUNNING)
        move(1,  0);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    render(renderer);

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
}
