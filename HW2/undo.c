#include "undo.h"
#include "stroke.h"

/* ─────────────────────────────────────────
   pushUndoStack
   - 실행된 액션(추가/삭제)을 undo 스택에 저장
   - 새 액션 발생 시 redo 스택 초기화
───────────────────────────────────────── */
void pushUndoStack(Canvas *canvas, ActionType type, const Stroke *stroke) {
    if (!canvas || !stroke) return;
    if (canvas->undoTop >= UNDO_STACK_SIZE) {
        /* 스택 꽉 참: 가장 오래된 항목 버리고 밀기 */
        for (int i = 0; i < UNDO_STACK_SIZE - 1; i++)
            canvas->undoStack[i] = canvas->undoStack[i+1];
        canvas->undoTop = UNDO_STACK_SIZE - 1;
    }
    canvas->undoStack[canvas->undoTop].type   = type;
    canvas->undoStack[canvas->undoTop].stroke = *stroke; // 값 복사
    canvas->undoTop++;

    /* 새 액션 발생 → redo 스택 무효화 */
    canvas->redoTop = 0;
}

/* ─────────────────────────────────────────
   undo
   - 마지막 액션 되돌리기
   - ADD_STROKE → 해당 획 삭제
   - REMOVE_STROKE → 해당 획 복원
───────────────────────────────────────── */
void undo(Canvas *canvas) {
    if (!canvas || canvas->undoTop == 0) return;

    Action *a = &canvas->undoStack[--canvas->undoTop];

    if (a->type == ACTION_ADD_STROKE) {
        /* 추가를 되돌림: 획 삭제 */
        removeStrokeFromCanvas(canvas, a->stroke.id);
    } else {
        /* 삭제를 되돌림: 획 복원 */
        Stroke *s = createStroke(a->stroke.penType,
                                 a->stroke.color,
                                 a->stroke.thickness);
        if (s) {
            *s = a->stroke; // 전체 값 복사
            addStrokeToCanvas(canvas, s);
        }
    }

    /* redo 스택에 역액션 저장 */
    if (canvas->redoTop < UNDO_STACK_SIZE) {
        canvas->redoStack[canvas->redoTop].type   = a->type;
        canvas->redoStack[canvas->redoTop].stroke = a->stroke;
        canvas->redoTop++;
    }
}

/* ─────────────────────────────────────────
   redo
   - undo된 액션 재실행
───────────────────────────────────────── */
void redo(Canvas *canvas) {
    if (!canvas || canvas->redoTop == 0) return;

    Action *a = &canvas->redoStack[--canvas->redoTop];

    if (a->type == ACTION_ADD_STROKE) {
        Stroke *s = createStroke(a->stroke.penType,
                                 a->stroke.color,
                                 a->stroke.thickness);
        if (s) {
            *s = a->stroke;
            addStrokeToCanvas(canvas, s);
        }
    } else {
        removeStrokeFromCanvas(canvas, a->stroke.id);
    }

    /* undo 스택에 다시 넣기 */
    if (canvas->undoTop < UNDO_STACK_SIZE) {
        canvas->undoStack[canvas->undoTop].type   = a->type;
        canvas->undoStack[canvas->undoTop].stroke = a->stroke;
        canvas->undoTop++;
    }
}
