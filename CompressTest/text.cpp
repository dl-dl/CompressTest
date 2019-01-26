#include "text.h"
#include "devfont.h"
#include "graph.h"

const DevFont *const Fonts[8] =
{
// &font21x15,
// &font24x18,
// &font24x19,
// &font27x21,
// &font28x21,
	&font32x25,
	0
};

static const unsigned char* FindChr(const DevFont* f, unsigned int c)
{
	const ui32 numBlocks = sizeof(f->block) / sizeof(*(f->block));
	for (ui32 i = 0; i < numBlocks; ++i)
	{
		if ((c >= f->block[i].base) && (c <= f->block[i].base + f->block[i].sz))
			return &f->block[i].sym[(c - f->block[i].base) * (f->maxW * f->bytesH + 1)];
	}
	return f->block[0].sym;
}

static unsigned int PrintChr(unsigned int chr, unsigned int x, unsigned int y, const DevFont* f, ui8 color, Screen* screen)
{
	const unsigned char* c = FindChr(f, chr);
	for (ui32 i = 0; (i < c[0]); ++i)
	{
		for (ui32 j = 0; (j < f->h); ++j)
		{
			if (c[i * f->bytesH + 1 + j / 8] & (1 << (j % 8)))
				Pixel(x + i, y + j, color, screen);
		}
	}
	return c[0];
}

void PrintStr(const char* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color, Screen* screen)
{
	const DevFont* f = Fonts[fontType];
	if (f == 0)
		return;

	for (ui32 i = 0; s[i]; ++i)
		x += PrintChr(s[i], x, y, f, color, screen);
}

void PrintStrW(const WIDE_CHAR* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color, Screen* screen)
{
	const DevFont* f = Fonts[fontType];
	if (f == 0)
		return;

	for (ui32 i = 0; s[i]; ++i)
		x += PrintChr(s[i], x, y, f, color, screen);
}
