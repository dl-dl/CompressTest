#include "convert.h"
#include "sizes.h"
#include <string.h>
#ifdef _MSC_VER
#include <assert.h>
#else
#define assert(expression) ((void)0)
#endif

void DecompImit(DecompState *s, ui8 *dst)
{
 s->x = s->y = 0;
 s->numEqPixel = 0;
 s->eqString = false;
 s->dst = dst;
}

static inline void PutByte(DecompState *s, ui8 v)
{
 assert(s->x < TILE_DX);
 assert(s->y < TILE_DY);
 if (s->x >= TILE_DX)
  return;
 if (s->y >= TILE_DY)
  return;

 *(s->dst + s->x * TILE_DY / 2 + s->y) = v;
 s->y++;
 if (s->y >= TILE_DY / 2)
  {
   s->y = 0;
   s->x++;
  }
}

static inline void CopyPrevLine(DecompState *s)
{
 assert(s->x < TILE_DX);
 if (s->x >= TILE_DX)
  return;

 ui8 *dstCol = s->dst + s->x * TILE_DY / 2;
 memcpy(dstCol, dstCol - TILE_DY / 2, TILE_DY / 2);
 s->x++;
}

void DeCompressOne(ui8 src, DecompState *s)
{
 if (s->eqString)
  {
   while (src--) // src == repeat count
    CopyPrevLine(s);
   s->eqString = false;
  }
 else if (s->numEqPixel)
  {
   for (ui8 cnt = 0; cnt < s->numEqPixel; cnt++)
    PutByte(s, src);
   s->numEqPixel = 0;
  }
 else
  {
   if (0x01 == src)
    {
     s->eqString = true;
    }
   else if (src & 0x01)
    {
     s->numEqPixel = src >> 1;
    }
   else if (src & 0x10)
    {
     PutByte(s, src & ~0x10);
     PutByte(s, src & ~0x10);
    }
   else
    {
     PutByte(s, src);
    }
  }
}
