#include "convert.h"
#include "sizes.h"
#include <string.h>
#ifdef _MSC_VER
#include <assert.h>
#else
#include "sound.h"
#define assert(expression) (void)((!!(expression)) || (Sound(4), 0))
//#define assert(expression) ((void)0)
#endif

void DecompImit(DecompState *s, ui8 *dst)
{
 s->x = s->y = 0;
 s->numEqPixel = 0;
 s->eqString = false;
 s->dst = dst;
}

static bool CheckStateByte(const DecompState *s, ui8 n)
{
 if (s->x >= TILE_DX)
  {
   assert(0);
   return false;
  }
 if (s->y + n > TILE_DY / 2)
  {
   assert(0);
   return false;
  }
 return true;
}

static bool CheckStateLine(const DecompState *s, ui8 n)
{
 if (s->x + n > TILE_DX)
  {
   assert(0);
   return false;
  }
 if (s->y)
  {
   assert(0);
   return false;
  }
 return true;
}

static void PutByte(DecompState *s, ui8 v)
{
 *(s->dst + s->x * TILE_DY / 2 + s->y) = v;
 s->y++;
 if (s->y >= TILE_DY / 2)
  {
   s->y = 0;
   s->x++;
  }
}

static void CopyPrevLine(DecompState *s)
{
 ui8 *dstCol = s->dst + s->x * TILE_DY / 2;
 memcpy(dstCol, dstCol - TILE_DY / 2, TILE_DY / 2);
 s->x++;
}

bool DeCompressOne(ui8 src, DecompState *s)
{
 if (s->eqString)
  {
   if (!CheckStateLine(s, src))
    return false;
   while (src--) // src == repeat count
    CopyPrevLine(s);
   s->eqString = false;
  }
 else if (s->numEqPixel)
  {
   if (!CheckStateByte(s, s->numEqPixel))
    return false;
   while (s->numEqPixel)
    {
     PutByte(s, src);
     s->numEqPixel--;
    }
  }
 else
  {
   if (0x01 == src)
    {
     s->eqString = true;
    }
   else if (src & 0x01)
    {
     s->numEqPixel = (src >> 1) + 2;
    }
   else if (src & 0x10)
    {
     if (!CheckStateByte(s, 2))
      return false;
     PutByte(s, src & ~0x10);
     PutByte(s, src & ~0x10);
    }
   else
    {
     if (!CheckStateByte(s, 1))
      return false;
     PutByte(s, src);
    }
  }
 return true;
}
