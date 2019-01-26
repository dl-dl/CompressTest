#ifndef __DEVFONT_H__
#define __DEVFONT_H__

typedef struct
{
	unsigned int base, sz;
	const unsigned char* sym;
} DevFontBlock;

typedef struct
{
	unsigned char h, bytesH, maxW;
	DevFontBlock block[2];
} DevFont;

#ifdef __cplusplus
extern "C" {
#endif
	extern const DevFont font21x15;
	extern const DevFont font24x18;
	extern const DevFont font24x19;
	extern const DevFont font27x21;
	extern const DevFont font32x25;
#ifdef __cplusplus
}
#endif

#endif
