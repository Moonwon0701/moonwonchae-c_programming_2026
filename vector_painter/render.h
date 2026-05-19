#ifndef RENDER_H
#define RENDER_H

#include "types.h"

void renderBackground(SDL_Renderer *renderer, Color bgColor);
void renderStroke(SDL_Renderer *renderer, const Stroke *stroke);
void renderAllStrokes(SDL_Renderer *renderer, const Canvas *canvas);
void renderEraserCursor(SDL_Renderer *renderer, float x, float y, float radius);

#endif
