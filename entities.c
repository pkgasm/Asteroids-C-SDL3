#include "entities.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Nave ---

void reset_ship(Game* game, bool invincible) {
    game->ship.pos = (SDL_FPoint){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    game->ship.vel = (SDL_FPoint){0, 0};
    game->ship.angle = -90.0f; // Apuntando hacia arriba
    game->ship.accelerating = false;
    if (invincible) {
        game->respawn_timer = 3.0f;
    } else {
        game->respawn_timer = 0.0f;
    }
}

void update_ship(Game* game, float dt) {
    const bool* state = SDL_GetKeyboardState(NULL);
    if (game->state == GAME_STATE_PLAYING && !game->hyperspace_active) {
        if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
            game->ship.accelerating = true;
        } else {
            game->ship.accelerating = false;
        }
        if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
            game->ship.angle -= SHIP_TURN_SPEED * dt;
        }
        if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
            game->ship.angle += SHIP_TURN_SPEED * dt;
        }
    } else {
        game->ship.accelerating = false;
    }

    if (game->ship.accelerating) {
        float angle_rad = game->ship.angle * (M_PI / 180.0f);
        game->ship.vel.x += cosf(angle_rad) * SHIP_ACCELERATION * dt;
        game->ship.vel.y += sinf(angle_rad) * SHIP_ACCELERATION * dt;
    }

    // Fricción
    game->ship.vel.x *= (1.0f - SHIP_FRICTION * dt);
    game->ship.vel.y *= (1.0f - SHIP_FRICTION * dt);
}

void render_ship(Game* game) {
    // No dibujar la nave si está en hiperespacio
    if (game->hyperspace_active) {
        return;
    }

    if (game->respawn_timer > 0) {
        // Parpadeo durante la invencibilidad
        if ((int)(game->respawn_timer * 10) % 2 == 0) {
            return;
        }
    }

    SDL_FPoint ship_center = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    // Dibujar escudo si está activo
    if (game->shield_timer > 0) {
        SDL_SetRenderDrawColor(game->renderer, 100, 100, 255, 100);
        for (int i = 0; i < 360; i += 15) {
            float rad1 = i * (M_PI / 180.0f);
            float rad2 = (i + 15) * (M_PI / 180.0f);
            SDL_RenderLine(game->renderer, ship_center.x + cosf(rad1) * (SHIP_SIZE + 5), ship_center.y + sinf(rad1) * (SHIP_SIZE + 5),
                                         ship_center.x + cosf(rad2) * (SHIP_SIZE + 5), ship_center.y + sinf(rad2) * (SHIP_SIZE + 5));
        }
    }

    float angle_rad = game->ship.angle * (M_PI / 180.0f);
    // Vértices para una forma de nave más clásica
    SDL_FPoint ship_points[] = {
        {ship_center.x + cosf(angle_rad) * SHIP_SIZE, ship_center.y + sinf(angle_rad) * SHIP_SIZE},
        {ship_center.x + cosf(angle_rad + 2.4f) * SHIP_SIZE, ship_center.y + sinf(angle_rad + 2.4f) * SHIP_SIZE},
        {ship_center.x + cosf(angle_rad - M_PI) * SHIP_SIZE * 0.5f, ship_center.y + sinf(angle_rad - M_PI) * SHIP_SIZE * 0.5f},
        {ship_center.x + cosf(angle_rad - 2.4f) * SHIP_SIZE, ship_center.y + sinf(angle_rad - 2.4f) * SHIP_SIZE},
        {ship_center.x + cosf(angle_rad) * SHIP_SIZE, ship_center.y + sinf(angle_rad) * SHIP_SIZE}
    };
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderLines(game->renderer, ship_points, 5);

    if (game->ship.accelerating) {
        // Llama parpadeante y de tamaño variable para más dinamismo
        float flame_size = SHIP_SIZE * (0.8f + ((float)rand() / RAND_MAX) * 0.4f); // Varía entre 0.8 y 1.2
        if (rand() % 3 == 0) { // Parpadeo ocasional
            return;
        }

        SDL_FColor flame_color = {1.0f, 0.5f, 0.0f, 1.0f};
        SDL_Vertex flame_vertices[] = {
            { {ship_center.x + cosf(angle_rad - M_PI) * flame_size, ship_center.y + sinf(angle_rad - M_PI) * flame_size},
              flame_color, {0, 0} },
            { {ship_center.x + cosf(angle_rad - M_PI + 0.5f) * flame_size * 0.5f, ship_center.y + sinf(angle_rad - M_PI + 0.5f) * flame_size * 0.5f},
              flame_color, {0, 0} },
            { {ship_center.x + cosf(angle_rad - M_PI - 0.5f) * flame_size * 0.5f, ship_center.y + sinf(angle_rad - M_PI - 0.5f) * flame_size * 0.5f},
              flame_color, {0, 0} },
        };
        SDL_RenderGeometry(game->renderer, NULL, flame_vertices, 3, NULL, 0);
    }
}

void activate_hyperspace(Game* game) {
    // Solo se puede activar si no está ya activo y si no está en cooldown
    if (!game->hyperspace_active && game->hyperspace_cooldown <= 0) {
        game->hyperspace_active = true;
        game->hyperspace_timer = HYPERSPACE_DURATION;
        game->hyperspace_cooldown = HYPERSPACE_COOLDOWN;
        spawn_explosion(game, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, (SDL_FColor){0.5f, 0.5f, 1.0f, 1.0f}, 40);
    }
}

void update_hyperspace(Game* game, float dt) {
    if (game->hyperspace_cooldown > 0) {
        game->hyperspace_cooldown -= dt;
    }

    if (game->hyperspace_active) {
        game->hyperspace_timer -= dt;
        if (game->hyperspace_timer <= 0) {
            game->hyperspace_active = false;

            // Teletransportar el "mundo" a una nueva posición aleatoria
            float new_x = (float)(rand() % SCREEN_WIDTH);
            float new_y = (float)(rand() % SCREEN_HEIGHT);
            float dx = new_x - (SCREEN_WIDTH / 2.0f);
            float dy = new_y - (SCREEN_HEIGHT / 2.0f);

            for (int i = 0; i < MAX_ASTEROIDS; ++i) {
                if (game->asteroids[i].active) {
                    game->asteroids[i].pos.x += dx;
                    game->asteroids[i].pos.y += dy;
                }
            }
            
            // Mover el OVNI si está activo
            if (game->ufo.active) {
                game->ufo.pos.x += dx;
                game->ufo.pos.y += dy;
            }

            // Mover todas las balas (del jugador y del OVNI)
            for (int i = 0; i < MAX_BULLETS; ++i) {
                if (game->bullets[i].active) {
                    game->bullets[i].pos.x += dx;
                    game->bullets[i].pos.y += dy;
                }
                if (game->ufo_bullets[i].active) {
                    game->ufo_bullets[i].pos.x += dx;
                    game->ufo_bullets[i].pos.y += dy;
                }
            }

            // Mover los power-ups
            for (int i = 0; i < MAX_POWERUPS; ++i) {
                if (game->powerups[i].active) {
                    game->powerups[i].pos.x += dx;
                    game->powerups[i].pos.y += dy;
                }
            }

            // Reiniciar la velocidad de la nave
            game->ship.vel = (SDL_FPoint){0, 0};

            spawn_explosion(game, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, (SDL_FColor){0.8f, 0.8f, 1.0f, 1.0f}, 40);
        }
    }
}

// --- Balas del Jugador ---

void fire_bullet(Game* game) {
    float base_angle_rad = game->ship.angle * (M_PI / 180.0f);
    if (game->hyperspace_active) return;
    if (game->triple_shot_timer > 0) {
        float angles[] = { base_angle_rad - 0.2f, base_angle_rad, base_angle_rad + 0.2f };
        for (int j = 0; j < 3; j++) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!game->bullets[i].active) {
                    game->bullets[i].active = true;
                    game->bullets[i].lifetime = BULLET_LIFESPAN;
                    game->bullets[i].pos = (SDL_FPoint){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
                    game->bullets[i].vel.x = cosf(angles[j]) * BULLET_SPEED;
                    game->bullets[i].vel.y = sinf(angles[j]) * BULLET_SPEED;
                    break; // Dispara una bala y busca el siguiente slot
                }
            }
        }
    } else {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!game->bullets[i].active) {
                game->bullets[i].active = true;
                game->bullets[i].lifetime = BULLET_LIFESPAN;
                game->bullets[i].pos = (SDL_FPoint){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
                game->bullets[i].vel.x = cosf(base_angle_rad) * BULLET_SPEED;
                game->bullets[i].vel.y = sinf(base_angle_rad) * BULLET_SPEED;
                return;
            }
        }
    }
}

void update_bullets(Game* game, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->bullets[i].active) {
            game->bullets[i].pos.x += game->bullets[i].vel.x * dt;
            game->bullets[i].pos.y += game->bullets[i].vel.y * dt;

            // El jugador está siempre en el centro. Para simular su movimiento,
            // movemos el resto del mundo en la dirección opuesta.
            game->bullets[i].pos.x -= game->ship.vel.x * dt;
            game->bullets[i].pos.y -= game->ship.vel.y * dt;

            game->bullets[i].lifetime -= dt;

            if (game->bullets[i].lifetime <= 0 ||
                game->bullets[i].pos.x < 0 || game->bullets[i].pos.x > SCREEN_WIDTH ||
                game->bullets[i].pos.y < 0 || game->bullets[i].pos.y > SCREEN_HEIGHT) {
                game->bullets[i].active = false;
            }
        }
    }
}

void render_bullets(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->bullets[i].active) {
            // Dibujar la bala como una pequeña línea para dar sensación de velocidad
            float speed = sqrtf(game->bullets[i].vel.x * game->bullets[i].vel.x + game->bullets[i].vel.y * game->bullets[i].vel.y);
            float end_x = game->bullets[i].pos.x - (game->bullets[i].vel.x / speed) * 4.0f; // 4 píxeles de largo
            float end_y = game->bullets[i].pos.y - (game->bullets[i].vel.y / speed) * 4.0f;
            SDL_RenderLine(game->renderer, game->bullets[i].pos.x, game->bullets[i].pos.y, end_x, end_y);
        }
    }
}

// --- Asteroides ---

void create_asteroid(Game* game, float x, float y, int size, const SDL_FPoint* parent_vel, const SDL_FPoint* bullet_vel) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!game->asteroids[i].active) {
            game->asteroids[i].active = true;
            game->asteroids[i].pos = (SDL_FPoint){x, y};
            game->asteroids[i].size = size;
            game->asteroids[i].angle = 0.0f; // El ángulo inicial no es tan importante, lo ponemos a 0.
            game->asteroids[i].rotation_speed = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * (M_PI / 2.0f); // Entre -PI/2 y +PI/2 rad/s

            if (parent_vel) {
                // Es un fragmento: hereda velocidad + impulso de la bala + explosión
                float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
                float speed = (ASTEROID_SPEED / size) * (0.8f + ((float)rand() / RAND_MAX) * 0.4f); // Velocidad de explosión variable

                game->asteroids[i].vel.x = parent_vel->x + cosf(angle) * speed * game->difficulty_factor;
                game->asteroids[i].vel.y = parent_vel->y + sinf(angle) * speed * game->difficulty_factor;

                // Añadir un pequeño empuje de la bala
                if (bullet_vel) {
                    game->asteroids[i].vel.x += bullet_vel->x * 0.05f;
                    game->asteroids[i].vel.y += bullet_vel->y * 0.05f;
                }
            } else {
                // Es un asteroide nuevo (inicio de nivel), velocidad completamente aleatoria
                float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
                game->asteroids[i].vel.x = cosf(angle) * (ASTEROID_SPEED / size) * game->difficulty_factor;
                game->asteroids[i].vel.y = sinf(angle) * (ASTEROID_SPEED / size) * game->difficulty_factor;
            }

            for (int j = 0; j < ASTEROID_MAX_VERTS; j++) {
                game->asteroids[i].vert_offsets[j] = 0.7f + ((float)rand() / (float)RAND_MAX) * 0.6f;
            }
            return;
        }
    }
}
void start_level(Game* game) {
    game->level++;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        game->asteroids[i].active = false;
    }

    int num_asteroids = game->level + 2;
    if (num_asteroids > MAX_ASTEROIDS) {
        num_asteroids = MAX_ASTEROIDS;
    }

    for (int i = 0; i < num_asteroids; i++) {
        int x, y;
        if (rand() % 2 == 0) {
            x = (rand() % 2 == 0) ? -20 : SCREEN_WIDTH + 20;
            y = rand() % SCREEN_HEIGHT;
        } else {
            x = rand() % SCREEN_WIDTH;
            y = (rand() % 2 == 0) ? -20 : SCREEN_HEIGHT + 20;
        }
        create_asteroid(game, x, y, 3, NULL, NULL); // NULL para indicar que no hay padre
    }
}

void update_asteroids(Game* game, float dt) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (game->asteroids[i].active) {
            game->asteroids[i].pos.x += game->asteroids[i].vel.x * dt;
            game->asteroids[i].pos.y += game->asteroids[i].vel.y * dt;

            // El jugador está siempre en el centro. Para simular su movimiento,
            // movemos el resto del mundo en la dirección opuesta.
            game->asteroids[i].pos.x -= game->ship.vel.x * dt;
            game->asteroids[i].pos.y -= game->ship.vel.y * dt;

            game->asteroids[i].angle += game->asteroids[i].rotation_speed * dt;

            // Screen wrapping
            if (game->asteroids[i].pos.x < -50) game->asteroids[i].pos.x = SCREEN_WIDTH + 49;
            if (game->asteroids[i].pos.x > SCREEN_WIDTH + 50) game->asteroids[i].pos.x = -49;
            if (game->asteroids[i].pos.y < -50) game->asteroids[i].pos.y = SCREEN_HEIGHT + 49;
            if (game->asteroids[i].pos.y > SCREEN_HEIGHT + 50) game->asteroids[i].pos.y = -49;
        }
    }
}

void render_asteroids(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (game->asteroids[i].active) {
            SDL_FPoint points[ASTEROID_MAX_VERTS + 1];
            for (int j = 0; j < ASTEROID_MAX_VERTS; j++) { // Corregido: el ángulo del asteroide ya está en radianes
                float a = (float)j / ASTEROID_MAX_VERTS * 2.0f * M_PI + game->asteroids[i].angle;
                float r = game->asteroids[i].size * 10.0f * game->asteroids[i].vert_offsets[j];
                points[j].x = game->asteroids[i].pos.x + cosf(a) * r;
                points[j].y = game->asteroids[i].pos.y + sinf(a) * r;
            }
            points[ASTEROID_MAX_VERTS] = points[0];
            SDL_RenderLines(game->renderer, points, ASTEROID_MAX_VERTS + 1);
        }
    }
}

// --- OVNI ---

void spawn_ufo(Game* game) {
    game->ufo.active = true;
    game->ufo.shoot_timer = 1.0f;
    game->ufo.type = (rand() % 4 == 0) ? UFO_SMALL : UFO_LARGE; // 25% de probabilidad de OVNI pequeño

    if (rand() % 2 == 0) {
        game->ufo.pos.x = -30.0f;
        game->ufo.vel.x = ((game->ufo.type == UFO_SMALL) ? UFO_SPEED * 1.5f : UFO_SPEED) * game->difficulty_factor;
    } else {
        game->ufo.pos.x = SCREEN_WIDTH + 30.0f;
        game->ufo.vel.x = ((game->ufo.type == UFO_SMALL) ? -UFO_SPEED * 1.5f : -UFO_SPEED) * game->difficulty_factor;
    }
    game->ufo.pos.y = (float)(rand() % (SCREEN_HEIGHT / 2)) + (SCREEN_HEIGHT / 4); // Aparece en la mitad central
    game->ufo.vel.y = 0;

    if (game->ufo.type == UFO_SMALL) {
        // El OVNI pequeño tiene un movimiento vertical sinusoidal
        if (rand() % 2 == 0) {
            game->ufo.vel.y = UFO_SPEED * 0.5f;
        } else {
            game->ufo.vel.y = -UFO_SPEED * 0.5f;
        }
    }
}

void update_ufo(Game* game, float dt) {
    if (!game->ufo.active) {
        game->ufo.spawn_timer -= dt * game->difficulty_factor; // El OVNI aparece más rápido con el tiempo
        if (game->ufo.spawn_timer <= 0) {
            spawn_ufo(game);
        }
        return;
    }

    if (game->ufo.type == UFO_SMALL) {
        if (game->ufo.pos.y < SCREEN_HEIGHT * 0.1f || game->ufo.pos.y > SCREEN_HEIGHT * 0.9f) {
            game->ufo.vel.y *= -1;
        }
    }
    game->ufo.pos.x += game->ufo.vel.x * dt;
    game->ufo.pos.y += game->ufo.vel.y * dt;
    game->ufo.pos.x -= game->ship.vel.x * dt;
    game->ufo.pos.y -= game->ship.vel.y * dt;

    game->ufo.shoot_timer -= dt;
    if (game->ufo.shoot_timer <= 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!game->ufo_bullets[i].active) {
                game->ufo_bullets[i].active = true;
                game->ufo_bullets[i].lifetime = BULLET_LIFESPAN;
                game->ufo_bullets[i].pos = game->ufo.pos;
                float angle = atan2f((SCREEN_HEIGHT / 2.0f) - game->ufo.pos.y, (SCREEN_WIDTH / 2.0f) - game->ufo.pos.x);
                game->ufo_bullets[i].vel.x = cosf(angle) * BULLET_SPEED;
                game->ufo_bullets[i].vel.y = sinf(angle) * BULLET_SPEED;
                break;
            }
        }
        if (game->ufo.type == UFO_SMALL) {
            game->ufo.shoot_timer = (0.5f + (float)(rand() % 50) / 100.0f) / game->difficulty_factor; // Dispara más rápido
        } else {
            game->ufo.shoot_timer = (1.0f + (float)(rand() % 100) / 100.0f) / game->difficulty_factor;
        }
    }

    if (game->ufo.pos.x < -50 || game->ufo.pos.x > SCREEN_WIDTH + 50) {
        game->ufo.active = false;
        game->ufo.spawn_timer = UFO_SPAWN_TIME;
    }
}

void render_ufo(Game* game) {
    if (game->ufo.active) {
        SDL_SetRenderDrawColor(game->renderer, 200, 50, 200, 255);
        float ufo_size = (game->ufo.type == UFO_SMALL) ? SHIP_SIZE * 0.8f : SHIP_SIZE * 1.6f;

        // Forma de platillo volante clásico
        SDL_FPoint body_points[] = {
            {game->ufo.pos.x - ufo_size, game->ufo.pos.y},
            {game->ufo.pos.x - ufo_size * 0.6f, game->ufo.pos.y - ufo_size * 0.4f},
            {game->ufo.pos.x + ufo_size * 0.6f, game->ufo.pos.y - ufo_size * 0.4f},
            {game->ufo.pos.x + ufo_size, game->ufo.pos.y},
            {game->ufo.pos.x - ufo_size, game->ufo.pos.y}
        };
        SDL_RenderLines(game->renderer, body_points, 5);

        SDL_FPoint dome_points[] = {
            {game->ufo.pos.x - ufo_size * 0.4f, game->ufo.pos.y - ufo_size * 0.4f},
            {game->ufo.pos.x, game->ufo.pos.y - ufo_size * 0.8f},
            {game->ufo.pos.x + ufo_size * 0.4f, game->ufo.pos.y - ufo_size * 0.4f}
        };
        SDL_RenderLines(game->renderer, dome_points, 3);
    }
}

// --- Balas del OVNI ---

void update_ufo_bullets(Game* game, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->ufo_bullets[i].active) {
            game->ufo_bullets[i].pos.x += game->ufo_bullets[i].vel.x * dt;
            game->ufo_bullets[i].pos.y += game->ufo_bullets[i].vel.y * dt;
            game->ufo_bullets[i].pos.x -= game->ship.vel.x * dt;
            game->ufo_bullets[i].pos.y -= game->ship.vel.y * dt;
            game->ufo_bullets[i].lifetime -= dt;

            if (game->ufo_bullets[i].lifetime <= 0) {
                game->ufo_bullets[i].active = false;
            }
        }
    }
}

void render_ufo_bullets(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->ufo_bullets[i].active) {
            // Dibujar un pequeño cuadrado para que sea más visible
            SDL_FRect bullet_rect = { game->ufo_bullets[i].pos.x - 1, game->ufo_bullets[i].pos.y - 1, 3.0f, 3.0f };
            SDL_RenderFillRect(game->renderer, &bullet_rect);
        }
    }
}

// --- Power-ups ---

void spawn_powerup(Game* game, float x, float y) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerups[i].active) {
            game->powerups[i].active = true;
            game->powerups[i].pos = (SDL_FPoint){x, y};
            game->powerups[i].vel = (SDL_FPoint){0, 0}; // Los power-ups no se mueven por sí mismos
            game->powerups[i].lifetime = POWERUP_LIFESPAN;
            game->powerups[i].type = (rand() % 2 == 0) ? POWERUP_SHIELD : POWERUP_TRIPLE_SHOT;
            return;
        }
    }
}

void update_powerups(Game* game, float dt) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (game->powerups[i].active) {
            // Movimiento relativo al mundo
            game->powerups[i].pos.x -= game->ship.vel.x * dt;
            game->powerups[i].pos.y -= game->ship.vel.y * dt;

            game->powerups[i].lifetime -= dt;
            if (game->powerups[i].lifetime <= 0) {
                game->powerups[i].active = false;
            }
        }
    }
}

void render_powerups(Game* game) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (game->powerups[i].active) {
            // Parpadeo para llamar la atención
            if ((int)(game->powerups[i].lifetime * 4) % 2 == 0) {
                continue;
            }

            SDL_FRect rect = {
                game->powerups[i].pos.x - POWERUP_SIZE / 2,
                game->powerups[i].pos.y - POWERUP_SIZE / 2,
                POWERUP_SIZE,
                POWERUP_SIZE
            };

            if (game->powerups[i].type == POWERUP_SHIELD) {
                SDL_SetRenderDrawColor(game->renderer, 100, 100, 255, 255); // Azul para escudo
            } else { // POWERUP_TRIPLE_SHOT
                SDL_SetRenderDrawColor(game->renderer, 255, 165, 0, 255); // Naranja para disparo triple
            }
            SDL_RenderFillRect(game->renderer, &rect);

            // Borde blanco
            SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
            SDL_RenderRect(game->renderer, &rect);
        }
    }
}

// --- Fondo de Estrellas ---

void init_stars(Game* game) {
    for (int i = 0; i < MAX_STARS; i++) {
        game->stars[i].pos.x = (float)(rand() % SCREEN_WIDTH);
        game->stars[i].pos.y = (float)(rand() % SCREEN_HEIGHT);
        game->stars[i].layer = rand() % 3; // Capas 0, 1, o 2
    }
}

void update_stars(Game* game, float dt) {
    for (int i = 0; i < MAX_STARS; i++) {
        // El multiplicador de capa hace que las capas más altas (cercanas) se muevan más rápido
        float speed_multiplier = 0.1f + (float)game->stars[i].layer * 0.2f;

        game->stars[i].pos.x -= game->ship.vel.x * dt * speed_multiplier;
        game->stars[i].pos.y -= game->ship.vel.y * dt * speed_multiplier;

        // Screen wrapping para las estrellas
        if (game->stars[i].pos.x < 0) {
            game->stars[i].pos.x += SCREEN_WIDTH;
        } else if (game->stars[i].pos.x >= SCREEN_WIDTH) {
            game->stars[i].pos.x -= SCREEN_WIDTH;
        }

        if (game->stars[i].pos.y < 0) {
            game->stars[i].pos.y += SCREEN_HEIGHT;
        } else if (game->stars[i].pos.y >= SCREEN_HEIGHT) {
            game->stars[i].pos.y -= SCREEN_HEIGHT;
        }
    }
}

void render_stars(Game* game) {
    for (int i = 0; i < MAX_STARS; i++) {
        // Las estrellas más lejanas (capa 0) son más tenues
        Uint8 brightness = 80 + game->stars[i].layer * 80;
        SDL_SetRenderDrawColor(game->renderer, brightness, brightness, brightness, 255);

        // Las estrellas más cercanas (capa 2) pueden ser un poco más grandes
        if (game->stars[i].layer == 2) {
            SDL_FRect star_rect = { game->stars[i].pos.x, game->stars[i].pos.y, 2.0f, 2.0f };
            SDL_RenderFillRect(game->renderer, &star_rect);
        } else {
            SDL_RenderPoint(game->renderer, game->stars[i].pos.x, game->stars[i].pos.y);
        }
    }
}

// --- Efectos (Explosiones) ---

void spawn_explosion(Game* game, float x, float y, SDL_FColor color, int count) {
    for (int i = 0; i < count; ++i) {
        for (int j = 0; j < MAX_PARTICLES; ++j) {
            if (!game->particles[j].active) {
                game->particles[j].active = true;
                game->particles[j].pos = (SDL_FPoint){x, y};
                float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
                float speed = ((float)rand() / RAND_MAX) * 100.0f + 50.0f;
                game->particles[j].vel.x = cosf(angle) * speed;
                game->particles[j].vel.y = sinf(angle) * speed;
                game->particles[j].color = color;
                game->particles[j].lifetime = PARTICLE_LIFESPAN * (0.5f + ((float)rand() / RAND_MAX) * 0.5f);
                break;
            }
        }
    }
}

void update_particles(Game* game, float dt) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (game->particles[i].active) {
            game->particles[i].pos.x += game->particles[i].vel.x * dt;
            game->particles[i].pos.y += game->particles[i].vel.y * dt;

            // Movimiento relativo al mundo
            game->particles[i].pos.x -= game->ship.vel.x * dt;
            game->particles[i].pos.y -= game->ship.vel.y * dt;

            // Fricción para las partículas
            game->particles[i].vel.x *= (1.0f - 1.5f * dt);
            game->particles[i].vel.y *= (1.0f - 1.5f * dt);

            game->particles[i].lifetime -= dt;
            if (game->particles[i].lifetime <= 0) {
                game->particles[i].active = false;
            }
        }
    }
}

void render_particles(Game* game) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (game->particles[i].active) {
            SDL_FColor color = game->particles[i].color;
            // Hacer que la partícula se desvanezca
            float alpha = (game->particles[i].lifetime / PARTICLE_LIFESPAN);
            // SDL_SetRenderDrawColorFloat no existe. Usamos la versión de 8-bit.
            // Para que el alpha blending funcione en primitivas, el blend mode del renderer debe ser SDL_BLENDMODE_BLEND.
            SDL_SetRenderDrawColor(game->renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(alpha * 255));
            SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderPoint(game->renderer, game->particles[i].pos.x, game->particles[i].pos.y);
        }
    }
}


// --- Funciones Auxiliares de Colisión ---

static void handle_bullet_asteroid_collisions(Game* game) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!game->asteroids[i].active) continue;

        for (int j = 0; j < MAX_BULLETS; j++) {
            if (game->bullets[j].active) {
                float dx = game->asteroids[i].pos.x - game->bullets[j].pos.x;
                float dy = game->asteroids[i].pos.y - game->bullets[j].pos.y;
                float dist_sq = dx * dx + dy * dy;
                float radius = game->asteroids[i].size * 10.0f;

                if (dist_sq < radius * radius) {
                    game->bullets[j].active = false;
                    game->asteroids[i].active = false;
                    game->score += (4 - game->asteroids[i].size) * 10;
                    spawn_explosion(game, game->asteroids[i].pos.x, game->asteroids[i].pos.y, (SDL_FColor){1.0f, 1.0f, 1.0f, 1.0f}, 15);

                    // Probabilidad de soltar un power-up
                    if (game->asteroids[i].size > 1 && (rand() % 10 == 0)) { // 10% de probabilidad
                        spawn_powerup(game, game->asteroids[i].pos.x, game->asteroids[i].pos.y);
                    }

                    if (game->asteroids[i].size > 1) {
                        create_asteroid(game, game->asteroids[i].pos.x, game->asteroids[i].pos.y, game->asteroids[i].size - 1, &game->asteroids[i].vel, &game->bullets[j].vel);
                        create_asteroid(game, game->asteroids[i].pos.x, game->asteroids[i].pos.y, game->asteroids[i].size - 1, &game->asteroids[i].vel, &game->bullets[j].vel);
                    }
                }
            }
        }
    }
}

static void handle_ship_asteroid_collisions(Game* game) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!game->asteroids[i].active) continue;

        if (game->respawn_timer <= 0 && game->shield_timer <= 0) {
            float dx = game->asteroids[i].pos.x - (SCREEN_WIDTH / 2.0f);
            float dy = game->asteroids[i].pos.y - (SCREEN_HEIGHT / 2.0f);
            float dist_sq = dx * dx + dy * dy; // Distancia al cuadrado
            float radius_sum = game->asteroids[i].size * 10.0f + SHIP_SIZE * 0.5f;
            float radius_sum_sq = radius_sum * radius_sum; // Suma de radios al cuadrado

            if (dist_sq < radius_sum_sq) {
                spawn_explosion(game, game->asteroids[i].pos.x, game->asteroids[i].pos.y, (SDL_FColor){1.0f, 0.2f, 0.2f, 1.0f}, 30);
                game->lives--;
                game->shake_timer = 0.5f; // Duración de la sacudida en segundos
                game->shake_intensity = 10.0f; // Intensidad inicial en píxeles
                if (game->lives <= 0) {
                    game->state = GAME_STATE_GAMEOVER;
                } else {
                    reset_ship(game, true);
                }
            }
        }
    }
}

static void handle_bullet_ufo_collisions(Game* game) {
    if (game->ufo.active) {
        for (int j = 0; j < MAX_BULLETS; j++) {
            if (game->bullets[j].active) {
                float dx_ufo = game->ufo.pos.x - game->bullets[j].pos.x;
                float dy_ufo = game->ufo.pos.y - game->bullets[j].pos.y;
                float dist_sq_ufo = dx_ufo * dx_ufo + dy_ufo * dy_ufo;
                float ufo_size_multiplier = (game->ufo.type == UFO_SMALL) ? 0.7f : 1.5f;
                float ufo_radius = SHIP_SIZE * ufo_size_multiplier;

                if (dist_sq_ufo < ufo_radius * ufo_radius) {
                    game->bullets[j].active = false;
                    game->ufo.active = false;
                    game->ufo.spawn_timer = UFO_SPAWN_TIME;
                    game->score += (game->ufo.type == UFO_SMALL) ? 500 : 200;
                    spawn_explosion(game, game->ufo.pos.x, game->ufo.pos.y, (SDL_FColor){0.8f, 0.2f, 0.8f, 1.0f}, 25);
                }
            }
        }
    }
}

static void handle_ufo_bullet_ship_collisions(Game* game) {
    if (game->respawn_timer <= 0 && game->shield_timer <= 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (game->ufo_bullets[i].active) {
                float dx = game->ufo_bullets[i].pos.x - (SCREEN_WIDTH / 2.0f);
                float dy = game->ufo_bullets[i].pos.y - (SCREEN_HEIGHT / 2.0f);
                float dist_sq = dx * dx + dy * dy;
                float ship_radius = SHIP_SIZE * 0.8f;

                if (dist_sq < ship_radius * ship_radius) {
                    spawn_explosion(game, game->ufo_bullets[i].pos.x, game->ufo_bullets[i].pos.y, (SDL_FColor){1.0f, 0.2f, 0.2f, 1.0f}, 30);
                    game->ufo_bullets[i].active = false;
                    game->lives--;
                    game->shake_timer = 0.5f;
                    game->shake_intensity = 10.0f;
                    if (game->lives <= 0) {
                        game->state = GAME_STATE_GAMEOVER;
                    } else {
                        reset_ship(game, true);
                    }
                }
            }
        }
    }
}

static void handle_ship_powerup_collisions(Game* game) {
    if (game->respawn_timer <= 0) {
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (game->powerups[i].active) {
                float dx = game->powerups[i].pos.x - (SCREEN_WIDTH / 2.0f);
                float dy = game->powerups[i].pos.y - (SCREEN_HEIGHT / 2.0f);
                float dist_sq = dx * dx + dy * dy;
                float radius_sum = POWERUP_SIZE + SHIP_SIZE * 0.5f;

                if (dist_sq < radius_sum * radius_sum) {
                    game->powerups[i].active = false;
                    if (game->powerups[i].type == POWERUP_SHIELD) {
                        game->shield_timer = SHIELD_DURATION;
                    } else if (game->powerups[i].type == POWERUP_TRIPLE_SHOT) {
                        game->triple_shot_timer = TRIPLE_SHOT_DURATION;
                    }
                }
            }
        }
    }
}

// --- Colisiones ---

void check_collisions(Game* game) {
    // Las colisiones que destruyen la nave deben ir primero para evitar
    // que la nave destruida interactúe con otras cosas en el mismo frame.
    handle_ship_asteroid_collisions(game);
    if (game->state != GAME_STATE_PLAYING) return; // Si el juego terminó, no seguir.

    handle_ufo_bullet_ship_collisions(game);
    if (game->state != GAME_STATE_PLAYING) return;

    handle_bullet_asteroid_collisions(game);
    handle_bullet_ufo_collisions(game);
    handle_ship_powerup_collisions(game);
}