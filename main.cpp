#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <format>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>




// -2147483647 - 2147483647
// 0 - 2147483647 * 2


void Blit(SDL_Surface* src, SDL_Surface* dst, int x, int y, int w = 0, int h = 0, int sx = 0, int sy = 0, int sw = 0, int sh = 0) {
    SDL_Rect dest = { x, y, w, h };
    SDL_Rect srcrect;
    SDL_Rect* srcPtr = nullptr;
    if (sw != 0 && sh != 0) {srcrect = {sx, sy, sw, sh}; srcPtr = &srcrect;}
    SDL_BlitSurface(src, srcPtr, dst, &dest);
}

struct Controller{
  bool A;
  bool B;
  //bool X;
  //bool Y;
  bool Right;
  bool Left;
  bool Up;
  bool Down;
  //bool L;
  //bool R;
  bool Select;
  bool Start;
  bool Back;
};

struct Building{
  std::vector<SDL_FRect> rooms;
  SDL_FRect rect;
  short int height;
  short int length;

  Building(int screenWidth, int screenHeight, int x_position = 0) {
    height = (SDL_rand(20) + 5) * 10;
    length = 3 + (SDL_rand(10) + 5) * 10;
    if (x_position == 0) rect = {.x = (float)screenWidth, .y = (float)screenHeight - height, .w = (float)length, .h = (float)height};
    else rect = {.x = (float)x_position, .y = (float)screenHeight - height, .w = (float)length, .h = (float)height};

    for (int x = 0; x < std::floor(length / 10); x++){
      for (int y = 0; y < std::floor((rect.y + rect.h) / 10); y++) {
        if (SDL_rand(2) == 1) {
          SDL_FRect room{
            .x = rect.x + 5 + (x * 10), .y = rect.y + 5 + (y * 10),
            .w = 6, .h = 6
          };
          rooms.push_back(room);
        }
      }
    }
  }

  void Update(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    rect.x -= 1;
    for (auto& room : rooms) {
      room.x -= 1;
      SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
      SDL_RenderFillRect(renderer, &room);
    }
  }
};


struct Platform{
  SDL_FRect rect;
  std::string type;
  std::string path;
  SDL_Surface *surface;
  SDL_Texture *texture;

  Platform(std::string platform_type, int x_position, int y_position) {
    type = platform_type;
    path = "./Assets/" + type + ".png";
    surface = SDL_LoadPNG(path.c_str());
    rect = {.x = (float)x_position, .y = (float)y_position, .w = (float)surface->w, .h = (float)surface->h};
  }

  void Update(SDL_Renderer *renderer) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
    rect.x -= 3;
  }
};


struct Player {
  SDL_FRect rect{.x = 32.0f, .y = 32.0f, .w = 32.0f, .h = 32.0f};
  float y_vel = 0.0f;
  bool on_ground = false;
  std::string state = "skate";
  int frame = 1;
  bool tricks = false;

  void Update(SDL_Renderer *renderer, const std::vector<Platform> platforms, Controller port) {
    std::string path = "./Assets/kenny/" + std::string(state) + std::to_string(frame) + ".png";
    SDL_Surface *surface = SDL_LoadPNG(path.c_str());
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
    rect.y -= y_vel;
    
    for (auto& platform : platforms) {
      if (rect.x + rect.w > platform.rect.x && rect.x < platform.rect.x + platform.rect.w && rect.y + rect.h > platform.rect.y && rect.y < platform.rect.y + platform.rect.h) {
        on_ground = true;
        rect.y = platform.rect.y - rect.h;
        break;
      }
      else {
        on_ground = false;
      }
    }

    if (!on_ground) y_vel -= 0.25;
    else y_vel = 0;

    if (port.A && on_ground) {
      on_ground = false;
      rect.y -= 2;
      y_vel = 5.0f;
    }
  }
};



struct Main{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  Controller port1{};
  const short int width = 512;
  const short int height = 448;
  const short int fps = 60;
  bool active = true;
  SDL_Surface *s_title;
  SDL_Texture *title;
  std::vector<SDL_FPoint> star_points;
  std::vector<Building> buildings;
  float building_timer = 50.0;
  short int gamestate = 0;
  Player kenny;
  std::vector<Platform> platforms;

  Main() {
    SDL_Log("Starting game...");
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Skater KENNY.", width, height, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, "opengl");
    SDL_Log("Backend Hardware Accelerated Renderer: %s", SDL_GetRendererName(renderer));
    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
      SDL_Log("%d. %s", i + 1, SDL_GetRenderDriver(i));
    }
    s_title = SDL_LoadPNG("./Assets/title.png");
    title = SDL_CreateTextureFromSurface(renderer, s_title);

    for (int i = 0; i < 300; i++) {
      SDL_FPoint point{
        .x = (float)SDL_rand(width),
        .y = (float)SDL_rand(width),
      };
      star_points.push_back(point);
    };
    buildings.emplace_back(width, height);
    buildings.emplace_back(width, height, 5);
    buildings.emplace_back(width, height, 100);
    buildings.emplace_back(width, height, 200);
    buildings.emplace_back(width, height, 300);
    buildings.emplace_back(width, height, 400);
  }

  ~Main() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroySurface(s_title);
    SDL_Quit();
  }

  void Update() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 75, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPoints(renderer, star_points.data(), star_points.size());
    for (auto& star : star_points) {
      star.x -= 1;
      if (star.x < 0) {star.x = width; star.y = SDL_rand(height);}
    }
    for (auto& building : buildings) {
      building.Update(renderer);
    }
    buildings.erase(std::remove_if(buildings.begin(), buildings.end(),[](const Building& b) {
      return b.rect.x + b.rect.w < 0;}),
      buildings.end()
    );
    building_timer -= 0.5;
    if (building_timer < 0) {
      buildings.emplace_back(width, height);
      building_timer = 50.0;
    }

    if (gamestate == 0) Menu();
    if (gamestate == 1) Gameplay();

    SDL_RenderPresent(renderer);

    Events();
  }

  void Menu(){
    SDL_FRect dst {.x = ((float)width / 2.0f) - ((float)s_title->w / 2.0f), .y = 80.0f, .w = 240.0f, .h = 149.0f};
    dst.x += (float)std::sin(building_timer / 2) * 2;
    //SDL_SetTextureScaleMode(title, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, title, nullptr, &dst);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_SetRenderScale(renderer, 2.5f, 2.5f);
    SDL_RenderDebugText(renderer, 60.0f, 120.0f, "Push START");
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    if (port1.Start) {
      gamestate = 1;
      for (int i = 0; i < 10; i++) {
        platforms.emplace_back("block", 150 + (i * 200), 300);
      }
    }
  }

  void Gameplay(){
    kenny.Update(renderer, platforms, port1);
    for (auto& platform : platforms) {
      platform.Update(renderer);
    }
    platforms.erase(std::remove_if(platforms.begin(), platforms.end(),[](const Platform& p) {
      return p.rect.x + p.rect.w < 0;}),
      platforms.end()
    );
    if (port1.Back) {
      gamestate = 0;
      kenny.rect.x = 32.0f;
      kenny.rect.y = 32.0f;
      kenny.y_vel = 0.0f;
      platforms.clear();
    }
  }

  void Events() {
    //port1.A = false;
    //port1.B = false;
    port1.Select = false;
    port1.Start = false;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        active = false;
      }
      else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_E) {port1.A = true;}
        if (event.key.key == SDLK_Q || event.key.key == SDLK_BACKSPACE) {port1.B = true;}
        if (event.key.key == SDLK_I) {port1.Select = true;}
        if (event.key.key == SDLK_RETURN) {port1.Start = true;}
        if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {port1.Right = true;}
        if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT) {port1.Left = true;}
        if (event.key.key == SDLK_W || event.key.key == SDLK_UP) {port1.Up = true;}
        if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN) {port1.Down = true;}
        if (event.key.key == SDLK_ESCAPE) {port1.Back = true;}
      }

      else if (event.type == SDL_EVENT_KEY_UP) {
        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_E) {port1.A = false;}
        if (event.key.key == SDLK_Q || event.key.key == SDLK_BACKSPACE) {port1.B = false;}
        if (event.key.key == SDLK_I) {port1.Select = false;}
        if (event.key.key == SDLK_RETURN) {port1.Start = false;}
        if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {port1.Right = false;}
        if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT) {port1.Left = false;}
        if (event.key.key == SDLK_W || event.key.key == SDLK_UP) {port1.Up = false;}
        if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN) {port1.Down = false;}
        if (event.key.key == SDLK_ESCAPE) {port1.Back = false;}
      }
    }
  }
};


int main(int argc, char *argv[])
{
  SDL_Log("Loading SDL...");
  Uint64 currentTime = 0;
  Uint64 deltaTime = 0;
  Uint64 lastTime = 0;
  Uint64 frames = 0;

  Main main;

  while (true) {
    Uint64 currentTick = SDL_GetTicks();

    main.Update();
    if (!main.active) {break;}
    
    Uint64 deltaTime = SDL_GetTicks() - currentTick;
    if (deltaTime < (1000 / main.fps)) {SDL_Delay((1000 / main.fps) - deltaTime);}
    frames++;
    if (currentTick >= lastTime + 1000) {lastTime = currentTime; frames = 0;}
  }

  return 0;
}