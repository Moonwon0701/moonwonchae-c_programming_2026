#ifndef UNDO_H
#define UNDO_H

#include "types.h"

void pushUndoStack(Canvas *canvas, ActionType type, const Stroke *stroke);
void undo(Canvas *canvas);
void redo(Canvas *canvas);

#endif
