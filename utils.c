#include "utils.h"

void draw_text(Game* game, const char* text, int x, int y, SDL_Color color) {
    if (!game->font) return;

    SDL_Surface* surface = TTF_RenderText_Solid(game->font, text, 0, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
        if (texture) {
            SDL_FRect dest_rect = {(float)x, (float)y, (float)surface->w, (float)surface->h};
            SDL_RenderTexture(game->renderer, texture, NULL, &dest_rect);
            SDL_DestroyTexture(texture);
        }
        SDL_DestroySurface(surface);
    }
}