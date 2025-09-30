#include "game.h"
#include "entities.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// --- Prototipos de Funciones Internas (solo para funciones definidas en este archivo y no en game.h) ---
int load_highscore(void);

bool init_sdl(Game* game) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo inicializar SDL: %s", SDL_GetError());
        return false;
    }

    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo inicializar SDL_ttf: %s", SDL_GetError());
        return false;
    }

    game->window = SDL_CreateWindow("Asteroids con SDL3", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!game->window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo crear la ventana: %s", SDL_GetError());
        return false;
    }

    game->renderer = SDL_CreateRenderer(game->window, NULL);
    if (!game->renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo crear el renderizador: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(game->renderer, 1);

    game->font = TTF_OpenFont("Press_Start_2P.ttf", 20);
    if (!game->font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo cargar la fuente 'Press_Start_2P.ttf': %s", SDL_GetError());
        // No es fatal, el juego puede continuar sin texto.
    }

    return true;
}

void start_new_game(Game* game) {
    game->score = 0;
    game->lives = 3;
    game->level = 0;
    game->state = GAME_STATE_PLAYING;
    game->shield_timer = 0.0f;
    game->triple_shot_timer = 0.0f;
    game->hyperspace_active = false;
    game->hyperspace_timer = 0.0f;
    game->hyperspace_cooldown = 0.0f;
    game->difficulty_factor = 1.0f;
    game->shake_timer = 0.0f;
    game->shake_intensity = 0.0f;
    reset_ship(game, false);

    game->ufo.active = false;
    game->ufo.spawn_timer = UFO_SPAWN_TIME;
    for (int i = 0; i < MAX_BULLETS; i++) {
        game->bullets[i].active = false;
        game->ufo_bullets[i].active = false;
    }
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        game->asteroids[i].active = false;
    }
    for (int i = 0; i < MAX_POWERUPS; i++) {
        game->powerups[i].active = false;
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        game->particles[i].active = false;
    }
}

void init_game_state(Game* game) {
    game->highscore = load_highscore();
    game->fullscreen = false;
    game->state = GAME_STATE_MENU;
    game->menu_selection = 0;
    init_stars(game);
}

void handle_events(Game* game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            game->running = false;
        }

        // Manejo de eventos globales (independientes del estado)
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_F11) {
                game->fullscreen = !game->fullscreen;
                SDL_SetWindowFullscreen(game->window, game->fullscreen);
            }
        }

        // Manejo de eventos por estado
        switch (game->state) {
            case GAME_STATE_MENU:
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_UP) game->menu_selection = 0;
                    if (event.key.scancode == SDL_SCANCODE_DOWN) game->menu_selection = 1;
                    if (event.key.scancode == SDL_SCANCODE_RETURN || event.key.scancode == SDL_SCANCODE_KP_ENTER) {
                        if (game->menu_selection == 0) { // Jugar
                            start_new_game(game);
                            start_level(game);
                        } else { // Salir
                            game->running = false;
                        }
                    }
                }
                break;
            case GAME_STATE_PLAYING:
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_SPACE && game->respawn_timer <= 0) fire_bullet(game);
                    if (event.key.scancode == SDL_SCANCODE_LSHIFT) activate_hyperspace(game);
                    if (event.key.scancode == SDL_SCANCODE_P || event.key.scancode == SDL_SCANCODE_ESCAPE) game->state = GAME_STATE_PAUSED;
                }
                break;
            case GAME_STATE_PAUSED:
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_P || event.key.scancode == SDL_SCANCODE_ESCAPE) game->state = GAME_STATE_PLAYING;
                }
                break;
            case GAME_STATE_GAMEOVER:
                if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_SPACE) {
                    game->state = GAME_STATE_MENU;
                }
                break;
        }
    }
}

int load_highscore(void) {
    FILE* file = fopen("highscore.txt", "r");
    if (!file) {
        return 0;
    }
    int score = 0;
    if (fscanf(file, "%d", &score) != 1) {
        score = 0;
    }
    fclose(file);
    return score;
}

void save_highscore(int score) {
    FILE* file = fopen("highscore.txt", "w");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No se pudo guardar el highscore.");
        return;
    }
    fprintf(file, "%d", score);
    fclose(file);
}

void cleanup(Game* game) {
    TTF_CloseFont(game->font);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    TTF_Quit();
    SDL_Quit();
}