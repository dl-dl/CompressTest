#ifndef __CONVERT2_H
#define __CONVERT2_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 // PNG layout to normal
 void Invert24(const void *src, void *dst);
 void Invert8(const void *src, void *dst);
 void Convert24To8(const void *src, void *dst);
 void Convert8To4(const void *src, void *dst);
 void Convert4To24(const void *src, void *dst);
 void Convert8To24(const void *src, void *dst);

 unsigned int Compress4BitBuffer(const void *src, void *dst);
 #ifdef __cplusplus
}
#endif

#endif
