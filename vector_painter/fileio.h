#ifndef FILEIO_H
#define FILEIO_H

#include "types.h"

/* 획 하나를 문자열로 직렬화 (호출자가 free 해야 함) */
char   *serializeStroke(const Stroke *stroke);

/* 문자열에서 획 복원 */
Stroke *deserializeStroke(const char *str);

/* 전체 캔버스를 파일로 저장 */
bool    saveCanvas(const Canvas *canvas, const char *filepath);

/* 파일에서 캔버스 복원 */
Canvas *loadCanvas(const char *filepath);

#endif /* FILEIO_H */
