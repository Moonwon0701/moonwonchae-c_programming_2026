#include "undo.h"
#include "stroke.h"

void pushUndoStack(Canvas *canvas, ActionType type, const Stroke *stroke) {
    if (!canvas || !stroke) return;
    if (canvas->undoTop >= UNDO_STACK_SIZE) {
        for (int i = 0; i < UNDO_STACK_SIZE-1; i++)
            canvas->undoStack[i] = canvas->undoStack[i+1];
        canvas->undoTop = UNDO_STACK_SIZE-1;
    }
    canvas->undoStack[canvas->undoTop].type   = type;
    canvas->undoStack[canvas->undoTop].stroke = *stroke;
    canvas->undoTop++;
    canvas->redoTop = 0;
}

void undo(Canvas *canvas) {
    if (!canvas || canvas->undoTop == 0) return;
    Action *a = &canvas->undoStack[--canvas->undoTop];
    if (a->type == ACTION_ADD_STROKE) {
        removeStrokeFromCanvas(canvas, a->stroke.id);
    } else {
        Stroke *s = createStroke(a->stroke.penType, a->stroke.color, a->stroke.thickness);
        if (s) { *s = a->stroke; addStrokeToCanvas(canvas, s); }
    }
    if (canvas->redoTop < UNDO_STACK_SIZE) {
        canvas->redoStack[canvas->redoTop].type   = a->type;
        canvas->redoStack[canvas->redoTop].stroke = a->stroke;
        canvas->redoTop++;
    }
}

void redo(Canvas *canvas) {
    if (!canvas || canvas->redoTop == 0) return;
    Action *a = &canvas->redoStack[--canvas->redoTop];
    if (a->type == ACTION_ADD_STROKE) {
        Stroke *s = createStroke(a->stroke.penType, a->stroke.color, a->stroke.thickness);
        if (s) { *s = a->stroke; addStrokeToCanvas(canvas, s); }
    } else {
        removeStrokeFromCanvas(canvas, a->stroke.id);
    }
    if (canvas->undoTop < UNDO_STACK_SIZE) {
        canvas->undoStack[canvas->undoTop].type   = a->type;
        canvas->undoStack[canvas->undoTop].stroke = a->stroke;
        canvas->undoTop++;
    }
}
