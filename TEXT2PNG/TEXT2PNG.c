/* sorcerian character view program */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>

#include "png.h"

#define CMAX 256
#define ROW 8
#define BITSpPIX 8

#define PNG_HEIGHT (CMAX*ROW)
#define PNG_WIDTH (16)


const unsigned char *cfiles = "TEXT";
const unsigned char *cfiles_out = "TEXT.png";

unsigned __int8 mbuf[CMAX][ROW][2];	/* 4096 bytes = 16 * 8 * 256 */
unsigned __int64 decoded_pattern[CMAX][ROW][2];

#pragma pack(1)
void decode_16bit_wide_mono(void)
{
	for (size_t pat = 0; pat < CMAX; pat++) {
		for (size_t y = 0; y < ROW; y++) {
			union {
				__int64 a;
				__int8 a8[8];
			} u[2];
			for (size_t x = 0; x < 8; x++) {
				u[0].a8[x] = (!!(mbuf[pat][y][0] & (1 << x)));
				u[1].a8[x] = (!!(mbuf[pat][y][1] & (1 << x)));
			}
			decoded_pattern[pat][y][0] = _byteswap_uint64(u[0].a);
			decoded_pattern[pat][y][1] = _byteswap_uint64(u[1].a);
		}
	}
}
#pragma pack()

unsigned __int8 maximum_colour_code(void)
{
	unsigned __int8 *d = decoded_pattern, max_ccode = 0;
	for (size_t c = 0; c < CMAX*ROW * 16; c++) {
		if (max_ccode < d[c]) {
			max_ccode = d[c];
		}
	}
	return max_ccode;
}

int main(void)
{
	FILE *pFi, *pFo;

	errno_t ecode = fopen_s(&pFi, cfiles, "rb");

	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", cfiles);
		exit(ecode);
	}
	size_t rcount = fread_s(mbuf, sizeof(mbuf), sizeof(mbuf), 1, pFi);
	if (rcount != 1) {
		fprintf_s(stderr, "File read error %s.\n", cfiles);
		fclose(pFi);
		exit(-2);
	}
	fclose(pFi);

	decode_16bit_wide_mono();

	ecode = fopen_s(&pFo, cfiles_out, "wb");
	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", cfiles_out);
		exit(ecode);
	}
	unsigned __int8 colours = maximum_colour_code() + 1;

	png_structp png_ptr;
	png_infop info_ptr;
	png_color pal[2] = { {0,0,0}, {0xFF,0xFF,0xFF} };

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(pFo);
		return;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(pFo);
		return;
	}

	unsigned char **image;
	image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
	if (image == NULL) {
		fprintf_s(stderr, "Memory allocation error.\n");
		fclose(pFo);
		exit(-2);
	}
	for (size_t j = 0; j < PNG_HEIGHT; j++)
		image[j] = (png_bytep)&decoded_pattern[j / ROW][j % ROW];


	png_init_io(png_ptr, pFo);
	png_set_IHDR(png_ptr, info_ptr, PNG_WIDTH, PNG_HEIGHT,
		BITSpPIX, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(png_ptr, info_ptr, pal, colours);
	png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
	png_byte trans[2] = { 0, 0xFF };
	png_set_tRNS(png_ptr, info_ptr, trans, 2, NULL);

	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(image);
	fclose(pFo);

	exit(0);
}
