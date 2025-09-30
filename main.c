#include "game.h"
#include "entities.h"
#include "utils.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// --- Prototipos de Funciones (definidas en main.c) ---
// (Funciones definidas en game.c y entities.c son declaradas en sus respectivos .h)

void update_game(Game* game, float dt);
void render_game(Game* game);

// --- Función Principal ---
int main(int argc, char* argv[]) {
    Game game = {0};
    srand((unsigned int)time(NULL));

    if (!init_sdl(&game)) {
        return 1;
    }

    init_game_state(&game);

    game.running = true;
    game.last_time = SDL_GetPerformanceCounter();

    while (game.running) {
        Uint64 current_time = SDL_GetPerformanceCounter();
        float dt = (current_time - game.last_time) / (float)SDL_GetPerformanceFrequency();
        game.last_time = current_time;

        // Limitar el delta time para evitar saltos en la física si el juego se congela
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        handle_events(&game);
        update_game(&game, dt);
        render_game(&game);
    }

    cleanup(&game);
    return 0;
}

void update_playing(Game* game, float dt) {
    // Aumentar la dificultad con el tiempo, con un límite
    if (game->score > game->highscore) {
        game->highscore = game->score;
    }

    // Aumentar la dificultad con el tiempo, con un límite
    game->difficulty_factor += 0.002f * dt; // Aumenta un 0.12 por minuto
    if (game->difficulty_factor > 3.0f) {
        game->difficulty_factor = 3.0f; // Límite para no hacerlo imposible
    }

    // --- Actualizar Nave ---
    if (game->respawn_timer > 0) {
        game->respawn_timer -= dt;
    }
    update_ship(game, dt);

    update_hyperspace(game, dt);

    // Actualizar timers de power-ups
    if (game->shield_timer > 0) {
        game->shield_timer -= dt;
    }
    if (game->triple_shot_timer > 0) {
        game->triple_shot_timer -= dt;
    }

    // Actualizar screen shake
    if (game->shake_timer > 0) {
        game->shake_timer -= dt;
        if (game->shake_timer <= 0) {
            game->shake_timer = 0.0f;
            game->shake_intensity = 0.0f;
        }
    }

    update_stars(game, dt);
    update_ufo(game, dt);
    update_bullets(game, dt);
    update_ufo_bullets(game, dt);
    update_asteroids(game, dt);
    update_powerups(game, dt);
    update_particles(game, dt);

    check_collisions(game);

    bool level_cleared = true;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (game->asteroids[i].active) {
            level_cleared = false;
            break;
        }
    }

    if (level_cleared && game->state == GAME_STATE_PLAYING) {
        start_level(game);
        reset_ship(game, false);
    }
}

void update_game(Game* game, float dt) {
    // Las estrellas se mueven en el menú para dar un efecto dinámico
    if (game->state == GAME_STATE_MENU) {
        update_stars(game, dt);
    }

    if (game->state == GAME_STATE_PLAYING) {
        update_playing(game, dt);
    }

    if (game->state == GAME_STATE_GAMEOVER) {
        if (game->score > game->highscore) {
            save_highscore(game->score);
        }
    }
}

void render_menu(Game* game) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    draw_text(game, "ASTEROIDS", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 100, white);

    SDL_Color play_color = (game->menu_selection == 0) ? yellow : white;
    draw_text(game, "Jugar", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, play_color);

    SDL_Color exit_color = (game->menu_selection == 1) ? yellow : white;
    draw_text(game, "Salir", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 40, exit_color);
}

void render_playing(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    // Renderizar las estrellas primero para que queden en el fondo
    render_stars(game);

    render_ship(game);
    render_bullets(game);
    render_ufo(game);
    render_ufo_bullets(game);
    render_powerups(game);
    render_asteroids(game);
    render_particles(game);

    // --- Dibujar UI ---
    SDL_Color white = {255, 255, 255, 255};
    char text_buffer[100];
    snprintf(text_buffer, sizeof(text_buffer), "SCORE: %d", game->score);
    draw_text(game, text_buffer, 10, 10, white);

    snprintf(text_buffer, sizeof(text_buffer), "HIGH: %d", game->highscore);
    draw_text(game, text_buffer, SCREEN_WIDTH / 2 - 70, 10, white);

    snprintf(text_buffer, sizeof(text_buffer), "LIVES: %d", game->lives); // "LIVES: X" son 8 caracteres
    draw_text(game, text_buffer, SCREEN_WIDTH - (8 * 20) - 10, 10, white); // 8 chars * 20px/char (aprox) + 10px padding

    if (game->state == GAME_STATE_PAUSED) {
        draw_text(game, "PAUSA", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 20, white);
    }
}

void render_gameover(Game* game) {
    SDL_Color white = {255, 255, 255, 255};
    draw_text(game, "GAME OVER", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 - 50, white);
    draw_text(game, "Press SPACE to return to menu", SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2, white);
}

void render_game(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    // Aplicar Screen Shake
    if (game->shake_timer > 0) {
        float offset_x = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * game->shake_intensity;
        float offset_y = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * game->shake_intensity;
        SDL_Rect viewport = { (int)offset_x, (int)offset_y, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_SetRenderViewport(game->renderer, &viewport);
    }

    if (game->state == GAME_STATE_MENU) {
        render_stars(game);
        render_menu(game);
    } else if (game->state == GAME_STATE_PLAYING || game->state == GAME_STATE_PAUSED) {
        render_playing(game);
    } else if (game->state == GAME_STATE_GAMEOVER) {
        render_playing(game); // Dibuja el estado final del juego detrás del texto de Game Over
        render_gameover(game);
    }

    // Restaurar el offset del renderizador para que la UI no se vea afectada (si la hubiera)
    if (game->shake_timer > 0) {
        SDL_SetRenderViewport(game->renderer, NULL);
    }

    SDL_RenderPresent(game->renderer);
}
