#ifndef ENTITIES_H
#define ENTITIES_H

#include "game.h"

// --- Prototipos de Funciones de Entidades ---

// Nave
void reset_ship(Game* game, bool invincible);
void update_ship(Game* game, float dt);
void render_ship(Game* game);

// Hiperespacio
void activate_hyperspace(Game* game);
void update_hyperspace(Game* game, float dt);

// Balas del Jugador
void fire_bullet(Game* game);
void update_bullets(Game* game, float dt);
void render_bullets(Game* game);

// Asteroides
void start_level(Game* game);
void create_asteroid(Game* game, float x, float y, int size, const SDL_FPoint* parent_vel, const SDL_FPoint* bullet_vel);
void update_asteroids(Game* game, float dt);
void render_asteroids(Game* game);

// OVNI
void spawn_ufo(Game* game);
void update_ufo(Game* game, float dt);
void render_ufo(Game* game);

// Balas del OVNI
void update_ufo_bullets(Game* game, float dt);
void render_ufo_bullets(Game* game);

// Power-ups
void spawn_powerup(Game* game, float x, float y);
void update_powerups(Game* game, float dt);
void render_powerups(Game* game);

// Fondo de estrellas
void init_stars(Game* game);
void update_stars(Game* game, float dt);
void render_stars(Game* game);

// Efectos (Explosiones)
void spawn_explosion(Game* game, float x, float y, SDL_FColor color, int count);
void update_particles(Game* game, float dt);
void render_particles(Game* game);

// Colisiones
void check_collisions(Game* game);

#endif // ENTITIES_H