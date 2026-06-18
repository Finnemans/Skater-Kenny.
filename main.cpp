#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <format>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <array>
#include <unordered_map>


static std::unordered_map<std::string, SDL_Texture*> gTextureCache;

static SDL_Texture* LoadCachedTexture(SDL_Renderer* renderer, const std::string& path) {
  auto cacheIt = gTextureCache.find(path);
  if (cacheIt != gTextureCache.end()) {
    return cacheIt->second;
  }

  SDL_Surface* surface = SDL_LoadPNG(path.c_str());
  if (!surface) {
    SDL_Log("Failed to load texture '%s': %s", path.c_str(), SDL_GetError());
    gTextureCache[path] = nullptr;
    return nullptr;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);
  if (!texture) {
    SDL_Log("Failed to create texture '%s': %s", path.c_str(), SDL_GetError());
  } else {
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  }
  gTextureCache[path] = texture;
  return texture;
}

static void FreeCachedTextures() {
  for (auto& [_, texture] : gTextureCache) {
    if (texture) SDL_DestroyTexture(texture);
  }
  gTextureCache.clear();
}


struct ManagedTexture {
  SDL_Surface* surface = nullptr;
  SDL_Texture* texture = nullptr;

  void Load(SDL_Renderer* renderer, const char* path) {
    surface = SDL_LoadPNG(path);
    if (!surface) {
      SDL_Log("Failed to load texture '%s': %s", path, SDL_GetError());
      return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) SDL_Log("Failed to create texture '%s': %s", path, SDL_GetError());
    else SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  }

  void Destroy() {
    if (texture) SDL_DestroyTexture(texture);
    if (surface) SDL_DestroySurface(surface);
    texture = nullptr;
    surface = nullptr;
  }
};


// -2147483647 - 2147483647
// 0 - 2147483647 * 2

void PlaySFX(std::string path, MIX_Mixer* mixer) {
  char *track_path = NULL;
  MIX_Audio *audio = NULL;
  SDL_asprintf(&track_path, "%s", path.c_str());
  audio = MIX_LoadAudio(mixer, track_path, false);
  SDL_free(track_path);
  MIX_Track *track = MIX_CreateTrack(mixer);
  MIX_SetTrackAudio(track, audio);
  MIX_PlayTrack(track, false);
}


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

  void Update(SDL_Renderer *renderer, bool finish_reached = false) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    if (!finish_reached) rect.x -= 1;
    for (auto& room : rooms) {
      if (!finish_reached) room.x -= 1;
      SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
      SDL_RenderFillRect(renderer, &room);
    }
  }
};

struct Building2{
  std::vector<SDL_FRect> rooms;
  SDL_FRect rect;
  short int height;
  short int length;

  Building2(int screenWidth, int screenHeight, int x_position = 0) {
    height = (SDL_rand(20) + 5) * 15;
    length = 3 + (SDL_rand(10) + 5) * 7;
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

  void Update(SDL_Renderer *renderer, bool finish_reached = false) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 55, 255);
    SDL_RenderFillRect(renderer, &rect);
    if (!finish_reached) rect.x -= 0.8;
    for (auto& room : rooms) {
      if (!finish_reached) room.x -= 0.8;
      SDL_SetRenderDrawColor(renderer, 125, 125, 20, 255);
      SDL_RenderFillRect(renderer, &room);
    }
  }
};


struct Spotlight{
  SDL_FRect rect;
  std::string path;

  Spotlight(int screenWidth, int screenHeight, int x_position = 0, bool face_right = true) {
    rect = {.x = (float)screenWidth, .y = (float)screenHeight - 488, .w = (float)280, .h = (float)488};
    if (face_right) {path = "./Assets/spotlight_right.png";}
    else {path = "./Assets/spotlight_left.png";}
  }

  void Update(SDL_Renderer *renderer, bool finish_reached = false) {
    SDL_Texture *texture = LoadCachedTexture(renderer, path);
    SDL_RenderTexture(renderer, texture, nullptr, &rect);
    if (!finish_reached) rect.x -= 0.7;
  }
};


struct Platform{
  SDL_FRect rect;
  std::string type;
  std::string path;
  int move_x;
  int move_y;
  int x_lapsed;
  int y_lapsed;

  Platform(std::string platform_type, int x_position, int y_position, int movex = 0, int movey = 0) {
    move_x = movex;
    move_y = movey;
    x_lapsed = 0;
    y_lapsed = 0;
    type = platform_type;
    path = "./Assets/" + type + ".png";
    SDL_Surface* surface = SDL_LoadPNG(path.c_str());
    if (surface) {
      rect = {.x = (float)x_position, .y = (float)y_position, .w = (float)surface->w, .h = (float)surface->h};
      SDL_DestroySurface(surface);
    } else {
      rect = {.x = (float)x_position, .y = (float)y_position, .w = 32.0f, .h = 32.0f};
    }
  }

  void Update(SDL_Renderer *renderer, bool &finish_reached) {
    SDL_Texture *texture = LoadCachedTexture(renderer, path);
    SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
    if (type == "platform") {
      for (int i = 3; i < rect.w; i += rect.w / 8) {
        SDL_FRect pole {.x = (float)(rect.x + i), .y = rect.y, .w = 4.0f, .h = 550.0f};
        SDL_RenderFillRect(renderer, &pole);
      }
    }
    if (texture) SDL_RenderTexture(renderer, texture, nullptr, &rect);
    if (type == "building" && rect.x <= 515 - rect.w) finish_reached = true;
    else {
      if (!finish_reached) rect.x -= 3;
    }
    if (move_x != 0) {
      if (move_x > 0) rect.x ++;
      else rect.x --;
      x_lapsed += 1;
      if (x_lapsed >= std::abs(move_x)) {
        move_x = -move_x;
        x_lapsed = 0;
      }
    }
    if (move_y != 0) {
      if (move_y > 0) rect.y ++;
      else rect.y --;
      y_lapsed += 1;
      if (y_lapsed >= std::abs(move_y)) {
        move_y = -move_y;
        y_lapsed = 0;
      }
    }
  }
};


struct Cone{
  SDL_FRect rect;
  bool strangled;
  int strangle_timer;
  bool on_ground;

  Cone(int x_position, int y_position) {
    rect = {.x = (float)x_position, .y = (float)y_position, .w = 12.0f, .h = 12.0f};
    strangled = false;
    strangle_timer = 0;
  }

  void Update(SDL_Renderer *renderer, const std::vector<Platform>& platforms, bool finish_reached = false) {
    std::string path = "./Assets/cone1.png";
    if (strangle_timer >= 1) path = "./Assets/cone2.png";
    if (strangle_timer >= 9) path = "./Assets/cone3.png";

    SDL_Texture* texture = LoadCachedTexture(renderer, path);
    if (texture) SDL_RenderTexture(renderer, texture, nullptr, &rect);
    if (!finish_reached) rect.x -= 3;

    if (strangled && strangle_timer < 10) strangle_timer ++;
    if (strangle_timer == 1) rect.x += 3;
    if (strangle_timer == 9) rect.x += 3;

    // for (auto &platform : platforms) {
    //   if (!(rect.x + rect.w > platform.rect.x && rect.x < platform.rect.x + platform.rect.w && rect.y + rect.h > platform.rect.y && rect.y < platform.rect.y + platform.rect.h)) {
    //     rect.y = platform.rect.y - rect.h;
    //     break;
    //   }
    //   else rect.y += 4;
    // }
  }
};


struct SprayCan{
  SDL_FRect rect;
  int speed = SDL_rand(3) + 2;
  bool collected = false;

  SprayCan(int x_position, int y_position) {
    rect = {.x = (float)x_position, .y = (float)y_position, .w = 10.0f, .h = 24.0f};
  }

  void Update(SDL_Renderer *renderer) {
    SDL_Texture *texture = LoadCachedTexture(renderer, "./Assets/spray_can.png");
    if (texture) SDL_RenderTexture(renderer, texture, nullptr, &rect);
    rect.x -= speed;
  }
};


struct Player{
  SDL_FRect rect{.x = 32.0f, .y = 32.0f, .w = 32.0f, .h = 32.0f};
  float y_vel = 0.0f;
  bool on_ground = false;
  std::string state = "skate";
  int frame = 1;
  int frame_timer = 4;
  bool trick_active = false;
  int score = 0;
  int spray_cans = 0;
  int trick_timer = 0;
  int spray_can_collect_timer = 0;
  float strangle_timer = 0;
  float land_shake_timer = 0;
  int grind_timer = 0;
  std::string trick = "";

  void Update(SDL_Renderer *renderer, const std::vector<Platform>& platforms, std::vector<SprayCan> &spraycans, std::vector<Cone> &cones, bool finish_reached, MIX_Mixer* mixer, Controller port) {
    bool was_on_ground = on_ground;
    if (((state == "popshoveit" || state == "ollie" || state == "kickflip" || state == "heelflip") && frame == 6) || ((state == "backflip" || state == "frontflip") && frame == 6) || ((state == "right_tailwhip" || state == "left_tailwhip") && frame == 7)) {
      if (state == "backflip" || state == "frontflip" || state == "right_tailwhip" || state == "left_tailwhip") score -= 1;
      score -= 2;
      trick_timer = 100;
      state = "hop"; frame = 1; trick_active = false;
    }
    std::string path = "./Assets/kenny/" + std::string(state) + std::to_string(frame) + ".png";
    SDL_Texture *texture = LoadCachedTexture(renderer, path);
    if (texture) {
      SDL_RenderTexture(renderer, texture, nullptr, &rect);
    }
    float fall_speed = y_vel;
    rect.y -= y_vel;
    if (finish_reached && state != "win") rect.x += 3;
    
    bool found_collision = false;
    for (auto &platform : platforms) {
      if (rect.x + rect.w > platform.rect.x && rect.x < platform.rect.x + platform.rect.w && rect.y + rect.h > platform.rect.y && rect.y < platform.rect.y + (platform.rect.h / 2)) {
        if (platform.type != "building_top") {
          if (!on_ground) PlaySFX("./Sounds/sfx/land.wav", mixer);
          found_collision = true;
          rect.y = platform.rect.y - (rect.h - 2);
          if (platform.type == "rail") {
            state = "grind";
            grind_timer --;
            if (grind_timer < 0){
              grind_timer = 4;
              PlaySFX("./Sounds/sfx/grind.wav", mixer);
            }
          }
          break;
        }
      }
    }
    if (found_collision && !was_on_ground) {
      float landing_height = std::abs(fall_speed);
      if (landing_height >= 10.0f) land_shake_timer = 5.0f;
    }
    on_ground = found_collision;

    if (!on_ground) {
      y_vel -= 0.25;
      if (!trick_active) {state = "hop"; frame = 1;}
      else {
        frame_timer --;
        if (frame_timer < 0){
          frame ++;
          frame_timer = 4;
        }
      }
    }
    else {
      if (trick_active) {
        if (trick_active) PlaySFX("./Sounds/sfx/crash.wav", mixer);
        strangle_timer = 10.0;
        score += 3;
        trick_active = false;
        trick_timer = 100;
        trick = "CRASH! +3";
      }
      if (state != "win") {
        if (state != "grind") state = "skate";
        frame = 1;
      }
      y_vel = 0;
      trick_active = false;
    }
    
    if (trick_timer >= 10) {
      trick_timer --;
      SDL_SetRenderDrawColor(renderer, 255, 50 + (trick_timer / 5), 50 + (trick_timer / 5), 255);
      SDL_SetRenderScale(renderer, 2.0f, 2.0f);
      SDL_RenderDebugText(renderer, (float)(55 + trick_timer), 40.0f, trick.c_str());
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    }
    
    if (spray_can_collect_timer > 0) {spray_can_collect_timer--;}

    for (auto &spraycan : spraycans) {
      if (rect.x + rect.w > spraycan.rect.x && rect.x < spraycan.rect.x + spraycan.rect.w && rect.y + rect.h > spraycan.rect.y && rect.y < spraycan.rect.y + spraycan.rect.h && !spraycan.collected) {
        spray_cans ++;
        spray_can_collect_timer = 10;
        spraycan.collected = true;
        PlaySFX("./Sounds/sfx/spraycan.wav", mixer);
      }
    }

    for (auto &cone : cones) {
      if (rect.x + rect.w > cone.rect.x && rect.x < cone.rect.x + cone.rect.w && rect.y + rect.h > cone.rect.y && rect.y < cone.rect.y + cone.rect.h && !cone.strangled) {
        PlaySFX("./Sounds/sfx/cone.wav", mixer);
        cone.strangled = true;
        score += 3;
        strangle_timer = 10.0;
        break;
      }
    }

    if (strangle_timer > 0) {
      strangle_timer -= 0.2;
      state = "strangle";
      if ((int)(std::round(strangle_timer)) % 2) frame = 1;
      else frame = 2;
    }
    
    if (rect.x >= 512 - 150 && on_ground) {
      state = "win";
      frame_timer --;
      if (frame_timer < 0 and frame < 58){
        frame ++;
        frame_timer = 6;
      }
      if (frame >= 35) {
        if (frame < 57) rect.x += 2;
        int green = 255;
        if (frame == 36 || frame == 38 || frame == 40 || frame == 42) green = 0;
        SDL_SetRenderDrawColor(renderer, 255, green, 0, 255);
        SDL_SetRenderScale(renderer, 2.0f, 2.0f);
        SDL_RenderDebugText(renderer, 60.0f, 40.0f, "Track Complete!");
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        if (frame >= 44) {
          SDL_SetRenderScale(renderer, 1.5f, 1.5f);
          std::string scString = "Your Score: " + std::to_string(score);
          SDL_RenderDebugText(renderer, 105.0f, 80.0f, scString.c_str());
          SDL_SetRenderScale(renderer, 1.0f, 1.0f);
        }
      }
    }

    if (state != "strangle" && state != "win") {
      if (state != "grind") {
        if (port.A && on_ground) {
          on_ground = false;
          rect.y -= 2;
          score++;
          y_vel = 5.0f;
          PlaySFX("./Sounds/sfx/jump.wav", mixer);
        }
      }
      if (rect.y < 448) {
        if (port.Right && (!on_ground) && (!trick_active) && state == "hop") {
          trick_active = true;
          state = "frontflip";
          trick = "FRONT FLIP! -3";
          PlaySFX("./Sounds/sfx/grandtrick.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Left && (!on_ground) && (!trick_active) && state == "hop") {
          trick_active = true;
          state = "backflip";
          trick = "BACK FLIP! -3";
          PlaySFX("./Sounds/sfx/grandtrick.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Up && (!on_ground) && (!trick_active) && state == "hop") {
          trick_active = true;
          state = "right_tailwhip";
          trick = "RIGHT TAILWHIP! -3";
          PlaySFX("./Sounds/sfx/grandtrick.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Down && (!on_ground) && (!trick_active) && state == "hop") {
          trick_active = true;
          state = "left_tailwhip";
          trick = "LEFT TAILWHIP! -3";
          PlaySFX("./Sounds/sfx/grandtrick.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Right && on_ground && !trick_active) {
          trick_active = true;
          trick_timer = 0;
          state = "popshoveit";
          rect.y -= 2;
          y_vel = 4.0f;
          trick = "POP SHOVE-IT! -2";
          PlaySFX("./Sounds/sfx/trick2.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Left && on_ground && !trick_active) {
          trick_active = true;
          state = "ollie";
          rect.y -= 2;
          y_vel = 4.0f;
          trick = "OLLIE! -2";
          PlaySFX("./Sounds/sfx/trick2.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Up && on_ground && !trick_active) {
          trick_active = true;
          state = "kickflip";
          rect.y -= 2;
          y_vel = 4.0f;
          trick = "KICKFLIP! -2";
          PlaySFX("./Sounds/sfx/trick1.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " COMBO!";
            score--;
          }
          trick_timer = 0;
        }
        if (port.Down && on_ground && !trick_active) {
          trick_active = true;
          state = "heelflip";
          rect.y -= 2;
          y_vel = 4.0f;
          trick = "HEELFLIP! -2";
          PlaySFX("./Sounds/sfx/trick1.wav", mixer);
          if (trick_timer >= 90) {
            trick = trick + " -1 COMBO!";
            score--;
          }
          trick_timer = 0;
        }
      }
    }
    if (score < 0) score = 0;
  }
};



struct Main{
  SDL_Window *window;
  SDL_Renderer *renderer;
  char *track_path = NULL;
  MIX_Audio *audio = NULL;
  MIX_Mixer *mixer = NULL;
  MIX_Track *track;
  SDL_PropertiesID options;
  SDL_PropertiesID options2;
  SDL_PropertiesID options3;
  SDL_Event event;
  Controller port1{};
  const short int width = 512;
  const short int height = 448;
  const short int fps = 60;
  bool active = true;
  ManagedTexture title;
  ManagedTexture keikei_art;
  ManagedTexture kenny_and_keikei_skyline;
  ManagedTexture kenny_mark;
  ManagedTexture fountain;
  ManagedTexture cd3;
  ManagedTexture cd2;
  ManagedTexture cd1;
  ManagedTexture rap_bar;
  ManagedTexture kakr1;
  ManagedTexture kakr2;
  ManagedTexture kakr_speak1;
  ManagedTexture kakr_speak2;
  ManagedTexture kakr_think;
  ManagedTexture kakr_hit;
  ManagedTexture kakr_fail;
  std::vector<SDL_FPoint> star_points;
  std::vector<Building> buildings;
  std::vector<Building2> buildings2;
  std::vector<Spotlight> spotlights;
  float menu_text_scroll = (float)width;
  float building_timer = 50.0;
  float building2_timer = 100.0;
  int spotlight_timer = 1000;
  bool which_spotlight = true;
  int score_obtain_timer = 60;
  int spraycan_timer = 500;
  int cone_timer = 200;
  int gamestate = 0;
  int transition_timer = 0;
  int transition_release = 0;
  int slide = 0;
  int time_lapsed = 0;
  bool finish_reached = false;
  int level = 1;
  int r_beats = 0;
  int r_lap_beats = 0;
  int r_kenny_score = 0;
  int r_keikei_score = 0;
  int r_state = 0;
  std::array<std::string, 10> intro_texts = {"It was a normal day in Kenny's city, Panpace.", "It's a city of skating and graffiti.", "And Kenny has always been the star of the city.", "One day, somebody soon took his place from the spotlight...", "It was a youngster by the name of KeiKei.", "And Kenny is an ave.", "Everybody turned their gazes away from Kenny!", "KeiKei started leaving grafitti marks all around the vicinity.", "It was time for Kenny to grab hold of his place again!", "- Push START button -"};
  std::array<std::string, 6> stage_2_texts = {"Kenny's journey to the spotlight was going well.", "He had to endure the ridicule of the public...", "KeiKei was making a game of his efforts.", "So KeiKei challenged you to a rap battle!", "Kenny has no choice to accept it.", "- Push START button -"};
  std::array<std::string, 10> stage_3_texts = {"Kenny's journey to the spotlight was going well.", "The public started to acknowledge him and his talents.", "He was determined to get back to the top!", "However, KeiKei was not going to let that happen.", "KeiKei started putting up billboards all around the city.", "KeiKei is a mouse.", "And Kenny is an ave.", "Kenny had no choice but to get rid of those billboards.", "Kenny's mission was clear: destroy all of KeiKei's billboards!", "- Push START button -"};
  Player kenny;
  std::vector<Platform> platforms;
  std::vector<SprayCan> spraycans;
  std::vector<Cone> cones;

  Main() {
    SDL_Log("Starting game...");
    SDL_SetAppMetadata("Skater KENNY.", "1.0", "com.skater-kenny-2");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");
    SDL_Log("Initializing video mode...");
    SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO);
    window = SDL_CreateWindow("Skater KENNY.", width, height, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, "opengl"); //nullptr
    SDL_Log("Backend Hardware Accelerated Renderer: %s", SDL_GetRendererName(renderer));
    SDL_Log("Logical CPU Cores: %d", SDL_GetNumLogicalCPUCores());
    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    // for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
    //   SDL_Log("%d. %s", i + 1, SDL_GetRenderDriver(i));
    // }
    title.Load(renderer, "./Assets/title.png");
    keikei_art.Load(renderer, "./Assets/keikei_art.png");
    kenny_and_keikei_skyline.Load(renderer, "./Assets/kenny_and_keikei_skyline.png");
    kenny_mark.Load(renderer, "./Assets/mark.png");
    fountain.Load(renderer, "./Assets/fountain.png");
    cd3.Load(renderer, "./Assets/3.png");
    cd2.Load(renderer, "./Assets/2.png");
    cd1.Load(renderer, "./Assets/1.png");
    rap_bar.Load(renderer, "./Assets/rap_bar.png");
    kakr1.Load(renderer, "./Assets/kenny_and_keikei1.png");
    kakr2.Load(renderer, "./Assets/kenny_and_keikei2.png");
    kakr_speak1.Load(renderer, "./Assets/keikei_speak1.png");
    kakr_speak2.Load(renderer, "./Assets/keikei_speak2.png");
    kakr_think.Load(renderer, "./Assets/kenny_think.png");
    kakr_hit.Load(renderer, "./Assets/kenny_correct.png");
    kakr_fail.Load(renderer, "./Assets/kenny_incorrect.png");
    
    SDL_Log("Loading audio stream...");
    MIX_Init();
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    options = SDL_CreateProperties();
    options2 = SDL_CreateProperties();
    options3 = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, 10300);
    SDL_SetNumberProperty(options2, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options2, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, 6200);
    SDL_SetNumberProperty(options3, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options3, MIX_PROP_PLAY_LOOP_START_MILLISECOND_NUMBER, 9510);

    SDL_asprintf(&track_path, "./Sounds/tracks/You_Are_A_Hit.mp3");
    audio = MIX_LoadAudio(mixer, track_path, false);
    SDL_free(track_path);
    track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, audio);
    MIX_PlayTrack(track, options);

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
    buildings2.emplace_back(width, height);
    buildings2.emplace_back(width, height, 5);
    buildings2.emplace_back(width, height, 100);
    buildings2.emplace_back(width, height, 200);
    buildings2.emplace_back(width, height, 300);
    buildings2.emplace_back(width, height, 400);
  }

  ~Main() {
    FreeCachedTextures();
    MIX_Quit();
    title.Destroy();
    keikei_art.Destroy();
    kenny_and_keikei_skyline.Destroy();
    fountain.Destroy();
    cd3.Destroy();
    cd2.Destroy();
    cd1.Destroy();
    rap_bar.Destroy();
    kakr1.Destroy();
    kakr2.Destroy();
    kakr_speak1.Destroy();
    kakr_speak2.Destroy();
    kakr_think.Destroy();
    kakr_hit.Destroy();
    kakr_fail.Destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  int ComputeLandingShakeOffset(float timer) {
    if (timer <= 0.0f) return 0;
    switch ((int)std::ceil(timer)) {
      case 5: return 2;
      case 4: return -2;
      case 3: return 1;
      case 2: return -1;
      default: return 0;
    }
  }

  int ComputeConeShakeOffset(float timer) {
    if (timer <= 0.0f) return 0;
    switch ((int)std::ceil(timer)) {
      case 10: return 3;
      case 9: return -3;
      case 8: return 2;
      case 7: return -2;
      case 6: return 0;
      default: return 0;
    }
  }

  void ApplyGameplayViewport() {
    if (gamestate != 2) {
      SDL_SetRenderViewport(renderer, nullptr);
      return;
    }

    int shake_y = 0;
    if (kenny.land_shake_timer > 0.0f) shake_y = ComputeLandingShakeOffset(kenny.land_shake_timer);
    if (kenny.strangle_timer > 0.0f) {
      int cone_offset = ComputeConeShakeOffset(kenny.strangle_timer);
      if (cone_offset != 0) shake_y = cone_offset;
    }

    SDL_Rect viewport {.x = 0, .y = shake_y, .w = width, .h = height};
    SDL_SetRenderViewport(renderer, &viewport);
  }

  void Update() {
    if (gamestate == 2 && kenny.land_shake_timer > 0.0f) {
      kenny.land_shake_timer -= 1.0f;
    }
    ApplyGameplayViewport();
    SDL_SetRenderDrawColor(renderer, 0, 0, 75, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPoints(renderer, star_points.data(), star_points.size());
    for (auto& star : star_points) {
      if (!finish_reached) star.x -= 0.5;
      if (star.x < 0) {star.x = width; star.y = SDL_rand(height);}
    }
    for (auto& spotlight : spotlights) {
      spotlight.Update(renderer, finish_reached);
    }
    spotlights.erase(std::remove_if(spotlights.begin(), spotlights.end(),[](const Spotlight& b) {
      return b.rect.x + b.rect.w < 0;}),
      spotlights.end()
    );
    spotlight_timer -= 1;
    if (spotlight_timer < 0) {
      spotlights.emplace_back(width, height, 0, which_spotlight);
      spotlight_timer = 1000;
      which_spotlight = !which_spotlight;
    }
    // background buildings
    for (auto& building2 : buildings2) {
      building2.Update(renderer, finish_reached);
    }
    buildings2.erase(std::remove_if(buildings2.begin(), buildings2.end(),[](const Building2& b) {
      return b.rect.x + b.rect.w < 0;}),
      buildings2.end()
    );
    building2_timer -= 0.5;
    if (building2_timer < 0) {
      buildings2.emplace_back(width, height);
      building2_timer = 100.0;
    // foreground buildings
    }
    for (auto& building : buildings) {
      building.Update(renderer, finish_reached);
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
    if (gamestate == 1) Introduction();
    if (gamestate == 2) Gameplay();
    if (gamestate == 3) StoryProgression1();
    if (gamestate == 4) RapBattle();

    SDL_RenderPresent(renderer);
    SDL_SetRenderViewport(renderer, nullptr);

    Events();
  }

  void Menu(){
    SDL_FRect dst {.x = ((float)width / 2.0f) - ((float)title.surface->w / 2.0f), .y = 80.0f, .w = 240.0f, .h = 149.0f};
    dst.x += (float)std::sin(building_timer / 2) * 2;
    //SDL_SetTextureScaleMode(title, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, title.texture, nullptr, &dst);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_SetRenderScale(renderer, 2.5f, 2.5f);
    SDL_RenderDebugText(renderer, 60.0f, 120.0f, "Push START");
    SDL_SetRenderScale(renderer, 1.2f, 1.3f);
    
    SDL_RenderDebugText(renderer, menu_text_scroll, 2.0f, "Set out on a new Kenny game featuring five stages in Panpace and meet your new contending rival, KeiKei!");
    SDL_RenderDebugText(renderer, 2.0f, 320.0f, "Opdracht voor Skills! Door Kaan, Finn en Sebastian");
    SDL_RenderDebugText(renderer, 2.0f, 332.0f, "Game by Tunari, no rights reserved");
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    menu_text_scroll--;
    if (menu_text_scroll < -(width * 1.6)) menu_text_scroll = (float)width;

    if (port1.Start) transition_release = true;
    if (port1.Back) active = false;

    for (int i = 0; i < width; i += width / 10) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 10, 255);
      SDL_FRect transition_rect {.x = (float)i, .y = 0.0f, .w = (float)(std::clamp(transition_timer - (i / 10), 0, 100)), .h = (float)height};
      SDL_RenderFillRect(renderer, &transition_rect);
    }
    if (transition_release) transition_timer += 2;
    if (transition_timer > 200) {startGame();}// gamestate = 3;
  }
  
  void Introduction(){
    if (slide >= 4 && slide <= 6) SDL_RenderTexture(renderer, keikei_art.texture, nullptr, nullptr);
    if (slide >= 7 && slide <= 9) SDL_RenderTexture(renderer, kenny_and_keikei_skyline.texture, nullptr, nullptr);

    SDL_SetRenderScale(renderer, 1.0f, 1.5f);
    if (slide >= 9) {
      SDL_SetRenderScale(renderer, 1.5f, 1.8f);
    }
    const std::string& text = intro_texts[slide];
    float charWidth = 8.0f;
    float textWidth = text.size() * charWidth;
    float x = (width - textWidth) / 2.0f;
    SDL_RenderDebugText(renderer, x, 10.0f, text.c_str());
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    if (port1.Start) {
      if (slide >= 9) {
        if (port1.Start) transition_release = true;
      }
      else slide ++;
    }

    for (int i = 0; i < width; i += width / 10) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 10, 255);
      SDL_FRect transition_rect {.x = (float)i, .y = 0.0f, .w = (float)(std::clamp(transition_timer - (i / 10), 0, 100)), .h = (float)height};
      SDL_RenderFillRect(renderer, &transition_rect);
    }
    if (transition_release) transition_timer += 2;
    if (transition_timer > 200) startGame(true);
  }

  void StoryProgression1(){
    SDL_SetRenderScale(renderer, 1.0f, 1.5f);
    if (slide >= 5) {
      SDL_SetRenderScale(renderer, 1.5f, 1.8f);
    }
    const std::string& text = stage_2_texts[slide];
    float charWidth = 8.0f; // approximate debug font width
    float textWidth = text.size() * charWidth;
    float x = (width - textWidth) / 2.0f;
    SDL_RenderDebugText(renderer, x, 10.0f, text.c_str());
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    if (port1.Start) {
      if (slide >= 5) {
        if (port1.Start) transition_release = true;
      }
      else slide ++;
    }

    for (int i = 0; i < width; i += width / 10) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 10, 255);
      SDL_FRect transition_rect {.x = (float)i, .y = 0.0f, .w = (float)(std::clamp(transition_timer - (i / 10), 0, 100)), .h = (float)height};
      SDL_RenderFillRect(renderer, &transition_rect);
    }
    if (transition_release) transition_timer += 2;
    if (transition_timer > 200) startGame(true);
  }

  void Gameplay(){
    for (auto& platform : platforms) {
      if (platform.type == "building_top") {
        platform.Update(renderer, finish_reached);
        SDL_FRect dst {.x = platform.rect.x + 15.0f, .y = platform.rect.y + 30.0f, .w = 32.0f, .h = 32.0f};
        if (kenny.frame >= 28) SDL_RenderTexture(renderer, kenny_mark.texture, nullptr, &dst);
      }
    }
    kenny.Update(renderer, platforms, spraycans, cones, finish_reached, mixer, port1);
    int last_platform_x = 0;
    for (auto& platform : platforms) {
      if (platform.type != "building_top") platform.Update(renderer, finish_reached);
      if (platform.rect.x > last_platform_x) last_platform_x = platform.rect.x + 2;
    }
    platforms.erase(std::remove_if(platforms.begin(), platforms.end(),[](const Platform& p) {
      return p.rect.x + p.rect.w < 0;}), platforms.end());
    for (auto& spraycan : spraycans) {
      spraycan.Update(renderer);
    }
    spraycans.erase(std::remove_if(spraycans.begin(), spraycans.end(),[](const SprayCan& s) {
      return s.rect.x + s.rect.w < 0 || s.collected;}), spraycans.end());
    for (auto& cone : cones) {
      cone.Update(renderer, platforms, finish_reached);
    }
    cones.erase(std::remove_if(cones.begin(), cones.end(),[](const Cone& c) {
      return c.rect.x + c.rect.w < 0;}), cones.end());
    if (kenny.state == "win") {
      if (kenny.frame == 1) MIX_StopTrack(track, 0);
      if (kenny.frame == 5 and kenny.frame_timer == 1) {
        MIX_StopTrack(track, 0);
        SDL_asprintf(&track_path, "./Sounds/tracks/Track_Complete!.mp3");
        audio = MIX_LoadAudio(mixer, track_path, false);
        SDL_free(track_path);
        MIX_Track *track = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(track, audio);
        MIX_PlayTrack(track, false);
      }
      if (kenny.frame >= 44) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(renderer, 2.5f, 2.5f);
        SDL_RenderDebugText(renderer, 60.0f, 120.0f, "Push START");
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);
        if (port1.Start) {
          if (level == 1) {
            level = 2;
            gamestate = 3;
            slide = 0;
            transition_release = false;
            transition_timer = 0;
            slide = 0;
            time_lapsed = 0;
            finish_reached = false;
            port1.Start = false;
            MIX_StopTrack(track, 1.0f);
            SDL_asprintf(&track_path, "./Sounds/tracks/You_Are_A_Hit.mp3");
            audio = MIX_LoadAudio(mixer, track_path, false);
            SDL_free(track_path);
            track = MIX_CreateTrack(mixer);
            MIX_SetTrackAudio(track, audio);
            MIX_PlayTrack(track, options);
          }
          else if (level == 2) {
            level = 3;
            startGame();
          }
          else if (level == 3) {
            level = 1;
            startGame();
          }
        }
      }
    }
    if (port1.Back) {
      gamestate = 0;
      level = 1;
      kenny.rect.x = 32.0f;
      kenny.rect.y = 32.0f;
      kenny.y_vel = 0.0f;
      platforms.clear();
      MIX_StopTrack(track, 1.0f);
      SDL_asprintf(&track_path, "./Sounds/tracks/You_Are_A_Hit.mp3");
      audio = MIX_LoadAudio(mixer, track_path, false);
      SDL_free(track_path);
      track = MIX_CreateTrack(mixer);
      MIX_SetTrackAudio(track, audio);
      MIX_PlayTrack(track, options);
    }
    if (kenny.rect.y > height && kenny.state != "win") { //When player has lost
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_SetRenderScale(renderer, 2.5f, 2.5f);
      SDL_RenderDebugText(renderer, 50.0f, 40.0f, "Game Over!");
      SDL_SetRenderScale(renderer, 2.0f, 2.0f);
      SDL_RenderDebugText(renderer, 60.0f, 100.0f, "Push A to restart");
      SDL_RenderDebugText(renderer, 50.0f, 140.0f, "Push START to return");
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);
      if (port1.A) startGame();
    }
    else if (kenny.state != "win") { // When player is alive
      std::string scoreString = "Keep Your Score Low: " + std::to_string(kenny.score);
      SDL_SetRenderScale(renderer, 1.8f, 1.8f);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderDebugText(renderer, 1.0f, 1.0f, scoreString.c_str());
      std::string tString = "Track " + std::to_string(level);
      SDL_RenderDebugText(renderer, 230.0f, 1.0f, tString.c_str());
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);

      SDL_Texture *sc_texture = LoadCachedTexture(renderer, "./Assets/spray_can.png");
      float sch = 24.0f;
      if (kenny.spray_can_collect_timer == 10) {sch = 22.0f;}
      if (kenny.spray_can_collect_timer == 9) {sch = 19.0f;}
      if (kenny.spray_can_collect_timer == 8) {sch = 18.0f;}
      if (kenny.spray_can_collect_timer == 7) {sch = 17.0f;}
      if (kenny.spray_can_collect_timer == 6) {sch = 17.0f;}
      if (kenny.spray_can_collect_timer == 5) {sch = 25.0f;}
      if (kenny.spray_can_collect_timer == 4) {sch = 30.0f;}
      if (kenny.spray_can_collect_timer == 3) {sch = 28.0f;}
      if (kenny.spray_can_collect_timer == 2) {sch = 26.0f;}
      if (kenny.spray_can_collect_timer == 1) {sch = 25.0f;}
      SDL_FRect dst {.x = 11.0f, .y = sch - 4, .w = 10.0f, .h = sch};
      SDL_RenderTexture(renderer, sc_texture, nullptr, &dst);
      std::string scString = "x " + std::to_string(kenny.spray_cans) + " / 5";
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_SetRenderScale(renderer, 1.8f, 1.8f);
      SDL_RenderDebugText(renderer, 20.0f, 16.0f, scString.c_str());
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);

      score_obtain_timer -= 1;
      if (score_obtain_timer < 0) {
        kenny.score ++;
        score_obtain_timer = 55 - (level * 5);
      }
      spraycan_timer --;
      if (spraycan_timer < 0) {
        if (kenny.spray_cans < 5) spraycans.emplace_back(width, std::clamp(SDL_rand(300), 175, (int)(kenny.rect.y) + 50));
        spraycan_timer = 450;
      }
      // cone_timer --;
      // if (cone_timer < 0) {
      //   cones.emplace_back(width * 2, 0);
      //   cone_timer = 400;
      // }
      time_lapsed += 3;
      if (time_lapsed >= 4000 && time_lapsed <= 4004 and kenny.spray_cans < 5) loadLevel(750);
      if (last_platform_x <= width + 3 && kenny.spray_cans >= 5) {
        platforms.emplace_back("building", width + 250, height - 100);
        platforms.emplace_back("building_top", width + 250 + 63, height - 165);
        for (int i = 0; i < 5; i++)
          platforms.emplace_back("platform", width + 478 + (i * 100), height - 100);
      }
    }
  }

  void RapBattle(){
    SDL_RenderTexture(renderer, fountain.texture, nullptr, nullptr);

    //Elke zeven beats = 1 Tracker patroon
    static Uint32 lastRapBattleLogTime = 0;
    Uint32 now = SDL_GetTicks();
    if (now - lastRapBattleLogTime >= static_cast<Uint32>(60000 / 85)) { //milliseconds (ms) / beats per minute (tempo/bpm)
      lastRapBattleLogTime = now;
      r_beats++;
      r_lap_beats++;

      if (r_beats > 7) {
        if (r_lap_beats == 5) PlaySFX("./Sounds/sfx/tickticktick.wav", mixer);
      }
      if (r_beats >= 20 && r_lap_beats == 1) {
        if (r_state == 1) PlaySFX("./Sounds/sfx/rhymematch.wav", mixer);
        else PlaySFX("./Sounds/sfx/rhymemismatch.wav", mixer);
        r_state = 0;
      }
    }
    if (r_beats == 12) SDL_RenderTexture(renderer, cd3.texture, nullptr, nullptr); //show countdown 3
    if (r_beats == 13) SDL_RenderTexture(renderer, cd2.texture, nullptr, nullptr); //show countdown 2
    if (r_beats == 14) SDL_RenderTexture(renderer, cd1.texture, nullptr, nullptr); //show countdown 1
    if (r_lap_beats > 6) {
      r_lap_beats = 0;
    }
    //reset beats because we loop laps every 7 beats of the song
    //(its pattern length in Furnace is 54 not 64)
    
    static Uint32 lastFrameLogTime = 0;
    static bool first_frame = true;

    if (now - lastFrameLogTime >= static_cast<Uint32>(60000 / 170)) { //to add animation
      lastFrameLogTime = now;
      first_frame = !first_frame;
    }
    if (first_frame) SDL_RenderTexture(renderer, kakr1.texture, nullptr, nullptr);
    else SDL_RenderTexture(renderer, kakr2.texture, nullptr, nullptr);
    if (r_beats > 15 && r_lap_beats >= 3 && r_lap_beats <= 4) {
      if (first_frame) SDL_RenderTexture(renderer, kakr_speak1.texture, nullptr, nullptr);
      else SDL_RenderTexture(renderer, kakr_speak2.texture, nullptr, nullptr);
    }
    if (r_beats > 15 && (r_lap_beats >= 5 || r_lap_beats == 0)) SDL_RenderTexture(renderer, kakr_think.texture, nullptr, nullptr);
    if (r_beats > 17 && r_lap_beats >= 1 && r_lap_beats <= 2) {
      if (r_state == 1) SDL_RenderTexture(renderer, kakr_hit.texture, nullptr, nullptr);
      else SDL_RenderTexture(renderer, kakr_fail.texture, nullptr, nullptr);
    }

    SDL_RenderTexture(renderer, rap_bar.texture, nullptr, nullptr);
  }

  void startGame(bool playtheme = false) {
    kenny.rect.x = 32.0f;
    kenny.rect.y = 32.0f;
    kenny.y_vel = 0.0f;
    kenny.score = 0;
    kenny.spray_cans = 5;
    kenny.land_shake_timer = 0.0f;
    kenny.strangle_timer = 0.0f;
    platforms.clear();
    spraycans.clear();
    cones.clear();
    if (gamestate == 0) gamestate = 1;
    else if (gamestate == 3) gamestate = 4;
    else gamestate = 2;
    transition_release = false;
    transition_timer = 0;
    slide = 0;
    time_lapsed = 0;
    finish_reached = false;
    kenny.trick_timer = 0;
    r_beats = 0;
    r_lap_beats = 0;
    r_kenny_score = 0;
    r_keikei_score = 0;
    // for (int i = 0; i < 10; i++) {
    //   platforms.emplace_back("block", 150 + (i * 200), 300);
    // }
    // for (int i = 0; i < 5; i++) {
    //   platforms.emplace_back("platform", 200 + (i * 400), 250);
    // }
    loadLevel();
    
    if (gamestate != 4)
      {
      if (playtheme) {
        MIX_StopTrack(track, 1.0f);
        SDL_asprintf(&track_path, "./Sounds/tracks/Today's_Your_Shot.mp3");
        audio = MIX_LoadAudio(mixer, track_path, false);
        SDL_free(track_path);
        track = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(track, audio);
        MIX_PlayTrack(track, options2);
      }
    }
    else {
      MIX_StopTrack(track, 1.0f);
      SDL_asprintf(&track_path, "./Sounds/tracks/Rap_Battle_Theme.mp3");
      audio = MIX_LoadAudio(mixer, track_path, false);
      SDL_free(track_path);
      track = MIX_CreateTrack(mixer);
      MIX_SetTrackAudio(track, audio);
      MIX_PlayTrack(track, options3);
    }
  }

  void loadLevel(int x_offset = 0) {
    time_lapsed = 0;
    if (level == 1) {
      for (int i = 0; i < 5; i++) {
        platforms.emplace_back("platform", 150 + (i * 100) + x_offset, 350);
      }
      if (SDL_rand(2) == 1) cones.emplace_back(500 + 20 + x_offset, 350 - 12);

      platforms.emplace_back("block", 750 + x_offset, 325);
      platforms.emplace_back("block", 950 + x_offset, 350);

      platforms.emplace_back("block", 1100 + x_offset, 300);
      platforms.emplace_back("block", 1200 + x_offset, 250);
      if (SDL_rand(2) == 1) cones.emplace_back(1200 + 50 + x_offset, 250 - 12);
      platforms.emplace_back("block", 1300 + x_offset, 200);

      platforms.emplace_back("platform", 1500 + x_offset, 250);
      platforms.emplace_back("platform", 1650 + x_offset, 200);
      if (SDL_rand(2) == 1) cones.emplace_back(1650 + 65 + x_offset, 200 - 12);
      platforms.emplace_back("platform", 1800 + x_offset, 150);

      platforms.emplace_back("platform", 2000 + x_offset, 180);
      platforms.emplace_back("platform", 2200 + x_offset, 220);
      platforms.emplace_back("platform", 2400 + x_offset, 260);

      for (int i = 0; i < 2; i++) {
        platforms.emplace_back("platform", 2650 + (i * 120) + x_offset, 350);
      }
      if (SDL_rand(2) == 1) cones.emplace_back(2650 + 82 + x_offset, 350 - 12);

      platforms.emplace_back("rail", 2900 + x_offset, 280);
      platforms.emplace_back("platform", 3150 + x_offset, 350);

      platforms.emplace_back("block", 3300 + x_offset, 300);
      platforms.emplace_back("block", 3450 + x_offset, 250);
      if (SDL_rand(2) == 1) cones.emplace_back(3450 + 56 + x_offset, 250 - 12);
      platforms.emplace_back("block", 3600 + x_offset, 200);

      platforms.emplace_back("platform", 3800 + x_offset, 200);
      platforms.emplace_back("rail", 3950 + x_offset, 250);
      platforms.emplace_back("platform", 4200 + x_offset, 300);
      if (SDL_rand(2) == 1) cones.emplace_back(4200 + 50 + x_offset, 300 - 12);
      platforms.emplace_back("block", 4350 + x_offset, 250);
      platforms.emplace_back("block", 4450 + x_offset, 200);
      if (SDL_rand(2) == 1) cones.emplace_back(4450 + 34 + x_offset, 200 - 12);

      platforms.emplace_back("rail", 4550 + x_offset, 200);
      platforms.emplace_back("platform", 4750 + x_offset, 225);
      if (SDL_rand(2) == 1) cones.emplace_back(4750 + 85 + x_offset, 225 - 12);
      platforms.emplace_back("platform", 4950 + x_offset, 200);
    }
    else if (level == 2) {
      for (int i = 0; i < 5; i++) {
        platforms.emplace_back("platform", 150 + (i * 100) + x_offset, 360);
      }

      platforms.emplace_back("platform", 700 + x_offset, 340);
      platforms.emplace_back("platform", 900 + x_offset, 360);

      platforms.emplace_back("block", 1050 + x_offset, 350);
      if (SDL_rand(2) == 1) cones.emplace_back(1050 + x_offset, 350 - 12);

      platforms.emplace_back("block", 1200 + x_offset, 330, 0, 20);
      platforms.emplace_back("block", 1300 + x_offset, 300, 0, 20);
      platforms.emplace_back("block", 1400 + x_offset, 270, 0, 20);

      platforms.emplace_back("platform", 1550 + x_offset, 290, 0, 30);

      platforms.emplace_back("rail", 1700 + x_offset, 260);
      platforms.emplace_back("rail", 1950 + x_offset, 300);

      platforms.emplace_back("platform", 2200 + x_offset, 340);
      platforms.emplace_back("platform", 2400 + x_offset, 310, 20);
      platforms.emplace_back("platform", 2600 + x_offset, 340);
      platforms.emplace_back("platform", 2800 + x_offset, 310, 20);

      platforms.emplace_back("platform", 3000 + x_offset, 330);
      if (SDL_rand(2) == 1) cones.emplace_back(3085 + x_offset, 330 - 12);
      platforms.emplace_back("platform", 3200 + x_offset, 350);

      platforms.emplace_back("block", 3350 + x_offset, 330, 0, 30);
      platforms.emplace_back("block", 3500 + x_offset, 320);
      platforms.emplace_back("block", 3700 + x_offset, 340);
      if (SDL_rand(2) == 1) cones.emplace_back(3750 + x_offset, 340 - 12);

      platforms.emplace_back("platform", 3865 + x_offset, 360);
      if (SDL_rand(2) == 1) cones.emplace_back(3945 + x_offset, 360 - 12);
      platforms.emplace_back("platform", 3975 + x_offset, 340, 50);

      platforms.emplace_back("block", 4200 + x_offset, 320);
      platforms.emplace_back("block", 4350 + x_offset, 290);
      if (SDL_rand(2) == 1) cones.emplace_back(4400 + x_offset, 290 - 12);
      platforms.emplace_back("block", 4500 + x_offset, 250, 0, 50);

      platforms.emplace_back("platform", 4725 + x_offset, 375);
      if (kenny.spray_cans >= 5) {
        platforms.emplace_back("platform", 4975 + x_offset, 350, 75);
        platforms.emplace_back("platform", 4875 + x_offset, 350, 75);
      }
    }
    else if (level == 3) {
      for (int i = 0; i < 5; i++) {
        platforms.emplace_back("platform", 150 + (i * 100) + x_offset, 360);
      }
      platforms.emplace_back("platform", 700 + x_offset, 340);
      platforms.emplace_back("platform", 850 + x_offset, 320);
      if (SDL_rand(2) == 1) cones.emplace_back(875 + x_offset, 320 - 12);
      platforms.emplace_back("platform", 1000 + x_offset, 330);

      platforms.emplace_back("block", 1150 + x_offset, 330);
      platforms.emplace_back("block", 1275 + x_offset, 330, 50, 0);
      platforms.emplace_back("block", 1450 + x_offset, 320);
      if (SDL_rand(2) == 1) cones.emplace_back(1490 + x_offset, 320 - 12);
      platforms.emplace_back("block", 1600 + x_offset, 310);
      platforms.emplace_back("platform", 1800 + x_offset, 300, 0, 50);

      for (int i = 0; i < 3; i++) {
        platforms.emplace_back("platform", 2000 + (i * 120) + x_offset, 200, 0, 100);
      }
      for (int i = 0; i < 3; i++) {
        platforms.emplace_back("platform", 2480 + (i * 120) + x_offset, 250, 0, 100);
      }
      platforms.emplace_back("platform", 2950 + x_offset, 280, 0, 50);
      platforms.emplace_back("platform", 3185 + x_offset, 350, 60);

      for (int i = 0; i < 4; i++) {
        platforms.emplace_back("block", 3400 + (i * 125) + x_offset, 310 - (i * 50));
      }
      for (int i = 0; i < 9; i++) {
        platforms.emplace_back("platform", 3850 + (i * 100) + x_offset, 130);
        if (SDL_rand(4) == 1) cones.emplace_back(3870 + (i * 100) + x_offset, 130 - 12);
        if (SDL_rand(4) == 1 && i < 8) cones.emplace_back(3950 + (i * 100) + x_offset, 130 - 12);
      }
      for (int i = 0; i < 10; i++) {
        platforms.emplace_back("platform", 3750 + (i * 100) + x_offset, 350);
        if (SDL_rand(5) == 1) cones.emplace_back(3770 + (i * 100) + x_offset, 350 - 12);
        if (SDL_rand(5) == 1 && i < 9) cones.emplace_back(3850 + (i * 100) + x_offset, 350 - 12);
      }
      platforms.emplace_back("block", 4800 + x_offset, 160);
      if (SDL_rand(2) == 1) cones.emplace_back(4810 + x_offset, 160 - 12);
      platforms.emplace_back("block", 4800 + x_offset, 330);
    }
  }

  void Events() {
    //port1.A = false;
    //port1.B = false;
    port1.Select = false;
    port1.Start = false;
    port1.Back = false;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        active = false;
      }
      else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_E || event.key.key == SDLK_A) {port1.A = true;}
        if (event.key.key == SDLK_Q || event.key.key == SDLK_BACKSPACE) {port1.B = true;}
        if (event.key.key == SDLK_I) {port1.Select = true;}
        if (event.key.key == SDLK_RETURN) {port1.Start = true;}
        if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {port1.Right = true;}
        if (event.key.key == SDLK_LEFT) {port1.Left = true;}
        if (event.key.key == SDLK_W || event.key.key == SDLK_UP) {port1.Up = true;}
        if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN) {port1.Down = true;}
        if (event.key.key == SDLK_P) {kenny.spray_cans ++;}
        if (event.key.key == SDLK_ESCAPE) {port1.Back = true;}
      }

      else if (event.type == SDL_EVENT_KEY_UP) {
        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_E || event.key.key == SDLK_A) {port1.A = false;}
        if (event.key.key == SDLK_Q || event.key.key == SDLK_BACKSPACE) {port1.B = false;}
        if (event.key.key == SDLK_I) {port1.Select = false;}
        if (event.key.key == SDLK_RETURN) {port1.Start = false;}
        if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT) {port1.Right = false;}
        if (event.key.key == SDLK_LEFT) {port1.Left = false;}
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
    
    deltaTime = SDL_GetTicks() - currentTick;
    if (deltaTime < (Uint64)(1000 / main.fps)) {SDL_Delay((1000 / main.fps) - deltaTime);}
    frames++;
    if (currentTick >= lastTime + 1000) {lastTime = currentTime; frames = 0;}

    // Uint32 targetTime = 1000 / main.fps;
    // Uint32 frameTime = (Uint32)(SDL_GetTicks() - currentTick);
    // if (frameTime < targetTime) {
    //     SDL_Delay(targetTime - frameTime);
    // }
  }

  return 0;
}