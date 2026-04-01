#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <format>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>


// -2147483647 - 2147483647
// 0 - 2147483647 * 2


void Blit(SDL_Surface* src, SDL_Surface* dst, int x, int y, int w = 0, int h = 0, int sx = 0, int sy = 0, int sw = 0, int sh = 0) {
  SDL_Rect dest = {x, y, w, h};
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


struct SprayCan{
  SDL_FRect rect;
  SDL_Surface *surface;
  int speed = SDL_rand(3) + 2;
  bool collected = false;

  SprayCan(int x_position, int y_position) {
    rect = {.x = (float)x_position, .y = (float)y_position, .w = 32.0f, .h = 32.0f};
    surface = SDL_LoadPNG("./Assets/spray_can.png");
  }

  void Update(SDL_Renderer *renderer) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
    rect.x -= speed;
  }
};


struct Player {
  SDL_FRect rect{.x = 32.0f, .y = 32.0f, .w = 32.0f, .h = 32.0f};
  float y_vel = 0.0f;
  bool on_ground = false;
  std::string state = "skate";
  int frame = 1;
  int frame_timer = 4;
  bool trick = false;
  int score = 0;
  int spray_cans = 0;

  void Update(SDL_Renderer *renderer, const std::vector<Platform> platforms, std::vector<SprayCan> &spraycans, Controller port) {
    std::string path = "./Assets/kenny/" + std::string(state) + std::to_string(frame) + ".png";
    SDL_Surface *surface = SDL_LoadPNG(path.c_str());
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
    rect.y -= y_vel;
    
    for (auto &platform : platforms) {
      if (rect.x + rect.w > platform.rect.x && rect.x < platform.rect.x + platform.rect.w && rect.y + rect.h > platform.rect.y && rect.y < platform.rect.y + platform.rect.h) {
        on_ground = true;
        rect.y = platform.rect.y - (rect.h - 1);
        break;
      }
      else {
        on_ground = false;
      }
    }

    if (!on_ground) {
      y_vel -= 0.25;
      if (!trick) {state = "hop"; frame = 1;}
      else {
        frame_timer --;
        if (frame_timer < 0){
          frame ++;
          frame_timer = 4;
        }
        if (!SDL_LoadPNG(path.c_str())) {
          state = "hop"; frame = 1; trick = false;
        }
      }
    }
    else {y_vel = 0; state = "skate"; frame = 1; trick = false;}

    for (auto &spraycan : spraycans) {
      if (rect.x + rect.w > spraycan.rect.x && rect.x < spraycan.rect.x + spraycan.rect.w && rect.y + rect.h > spraycan.rect.y && rect.y < spraycan.rect.y + spraycan.rect.h && !spraycan.collected) {
        spray_cans ++;
        spraycan.collected = true;
      }
    }

    if (port.A && on_ground) {
      on_ground = false;
      rect.y -= 2;
      y_vel = 5.0f;
    }
    if (port.Right && !on_ground && !trick) {
      trick = true;
      state = "frontflip";
    }
    if (port.Left && !on_ground && !trick) {
      trick = true;
      state = "backflip";
    }
    if (port.Up && !on_ground && !trick) {
      trick = true;
      state = "right_tailwhip";
    }
    if (port.Down && !on_ground && !trick) {
      trick = true;
      state = "left_tailwhip";
    }
    if (port.Right && on_ground && !trick) {
      trick = true;
      state = "popshoveit";
      rect.y -= 2;
      y_vel = 2.0f;
    }
    if (port.Left && on_ground && !trick) {
      trick = true;
      state = "ollie";
      rect.y -= 2;
      y_vel = 2.0f;
    }
    if (port.Up && on_ground && !trick) {
      trick = true;
      state = "kickflip";
      rect.y -= 2;
      y_vel = 2.0f;
    }
    if (port.Down && on_ground && !trick) {
      trick = true;
      state = "heelflip";
      rect.y -= 2;
      y_vel = 2.0f;
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
  int score_obtain_timer = 60;
  int spraycan_timer = 500;
  short int gamestate = 0;
  Player kenny;
  std::vector<Platform> platforms;
  std::vector<SprayCan> spraycans;

  Main() {
    SDL_Log("Starting game...");
    SDL_SetAppMetadata("Skater KENNY.", "1.0", "com.skater-kenny-2");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");
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

    char *path = NULL;
    MIX_Audio *audio = NULL;
    MIX_Init();
    MIX_Mixer *mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    SDL_asprintf(&path, "./Sounds/tracks/Today's_Your_Shot.mp3", SDL_GetBasePath());
    audio = MIX_LoadAudio(mixer, path, false);
    SDL_free(path);
    MIX_Track *track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, audio);
    MIX_PlayTrack(track, 0);

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
    //MIX_Quit();
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

    if (port1.Start) startGame();
  }

  void Gameplay(){
    kenny.Update(renderer, platforms, spraycans, port1);
    for (auto& platform : platforms) {
      platform.Update(renderer);
    }
    platforms.erase(std::remove_if(platforms.begin(), platforms.end(),[](const Platform& p) {
      return p.rect.x + p.rect.w < 0;}),
      platforms.end()
    );
    for (auto& spraycan : spraycans) {
      spraycan.Update(renderer);
    }
    spraycans.erase(std::remove_if(spraycans.begin(), spraycans.end(),[](const SprayCan& s) {
      return s.rect.x + s.rect.w < 0 || s.collected;}),
      spraycans.end()
    );
    if (port1.Back) {
      gamestate = 0;
      kenny.rect.x = 32.0f;
      kenny.rect.y = 32.0f;
      kenny.y_vel = 0.0f;
      platforms.clear();
    }
    if (kenny.rect.y > height) { //When player is dead
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_SetRenderScale(renderer, 2.5f, 2.5f);
      SDL_RenderDebugText(renderer, 60.0f, 40.0f, "Game Over!");
      SDL_SetRenderScale(renderer, 2.0f, 2.0f);
        SDL_RenderDebugText(renderer, 60.0f, 100.0f, "Push A to restart");
        SDL_RenderDebugText(renderer, 50.0f, 140.0f, "Push START to return");
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);
      if (port1.A) startGame();
    }
    else { // When player is alive
      std::string scoreString = "Score: " + std::to_string(kenny.score);
      SDL_SetRenderScale(renderer, 1.8f, 1.8f);
      SDL_RenderDebugText(renderer, 1.0f, 1.0f, scoreString.c_str());
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);

      SDL_Surface *sc_surface = SDL_LoadPNG("./Assets/spray_can.png");
      SDL_Texture *sc_texture = SDL_CreateTextureFromSurface(renderer, sc_surface);
      SDL_FRect dst {.x = 0.0f, .y = 16.0f, .w = 32.0f, .h = 32.0f};
      SDL_RenderTexture(renderer, sc_texture, nullptr, &dst);
      std::string scString = "x " + std::to_string(kenny.spray_cans);
      SDL_SetRenderScale(renderer, 1.8f, 1.8f);
      SDL_RenderDebugText(renderer, 20.0f, 16.0f, scString.c_str());
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);

      score_obtain_timer -= 1;
      if (score_obtain_timer < 0) {
        kenny.score ++;
        score_obtain_timer = 60;
      }
      spraycan_timer --;
      if (spraycan_timer < 0) {
        spraycans.emplace_back(width, SDL_rand(175) + 175);
        spraycan_timer = 500;
      }
    }
  }

  void startGame() {
    kenny.rect.x = 32.0f;
    kenny.rect.y = 32.0f;
    kenny.y_vel = 0.0f;
    kenny.score = 0;
    kenny.spray_cans = 0;
    platforms.clear();
    spraycans.clear();
    gamestate = 1;
    // for (int i = 0; i < 10; i++) {
    //   platforms.emplace_back("block", 150 + (i * 200), 300);
    // }
    // for (int i = 0; i < 5; i++) {
    //   platforms.emplace_back("platform", 200 + (i * 400), 250);
    // }
    for (int i = 0; i < 5; i++) {
      platforms.emplace_back("platform", 150 + (i * 100), 350);
    }
    platforms.emplace_back("block", 750, 325);
    platforms.emplace_back("block", 750, 325);
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