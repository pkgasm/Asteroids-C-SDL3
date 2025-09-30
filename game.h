#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>

#include "defs.h"

// --- Definiciones de Tipos ---

// Máquina de estados del juego
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAMEOVER
} GameState;
// --- Estructuras de Datos ---

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    float angle;
    bool accelerating;
} Ship;

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    float lifetime;
    bool active;
} Bullet;

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    int size; // 3 = grande, 2 = mediano, 1 = pequeño
    float angle;
    float rotation_speed;
    bool active;
    float vert_offsets[ASTEROID_MAX_VERTS];
} Asteroid;

typedef enum {
    UFO_LARGE,
    UFO_SMALL
} UFOType;

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    bool active;
    float spawn_timer;
    float shoot_timer;
    UFOType type;
} UFO;

typedef enum {
    POWERUP_SHIELD,
    POWERUP_TRIPLE_SHOT
} PowerUpType;

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    PowerUpType type;
    bool active;
    float lifetime;
} PowerUp;

typedef struct {
    SDL_FPoint pos;
    // Capa de profundidad: 0=lejos (lento), 1=medio, 2=cerca (rápido)
    int layer;
} Star;

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    SDL_FColor color;
    float lifetime;
    bool active;
} Particle;

// Estructura principal del juego
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool running;
    Uint64 last_time;

    Ship ship;
    Bullet bullets[MAX_BULLETS];
    Asteroid asteroids[MAX_ASTEROIDS];
    UFO ufo;
    Bullet ufo_bullets[MAX_BULLETS];
    PowerUp powerups[MAX_POWERUPS];
    Star stars[MAX_STARS];
    Particle particles[MAX_PARTICLES];

    int score;
    int highscore;
    int lives;
    int level;

    // Máquina de estados del juego (definida antes de la struct Game)
    GameState state;
    int menu_selection; // 0 = Jugar, 1 = Salir

    float respawn_timer;

    // Estado del Hiperespacio
    bool hyperspace_active;
    float hyperspace_timer;
    float hyperspace_cooldown;

    // Dificultad progresiva
    float difficulty_factor;

    // Timers para power-ups activos
    float shield_timer;
    float triple_shot_timer;
    bool fullscreen;

    // Efecto de Screen Shake
    float shake_timer;
    float shake_intensity;
} Game;

// --- Prototipos de Funciones del Juego ---
void save_highscore(int score);
bool init_sdl(Game* game);
void init_game_state(Game* game);
void handle_events(Game* game);
void cleanup(Game* game);
void start_new_game(Game* game);

#endif // GAME_H