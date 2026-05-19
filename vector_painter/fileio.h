#ifndef FILEIO_H
#define FILEIO_H

#include "types.h"

char   *serializeStroke(const Stroke *stroke);
Stroke *deserializeStroke(const char *str);
bool    saveCanvas(const Canvas *canvas, const char *filepath);
Canvas *loadCanvas(const char *filepath);

#endif
