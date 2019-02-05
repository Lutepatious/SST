/* sorcerian character view program */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>

#include "png.h"

#define CMAX 32
#define PLANE 4
#define ROW 8
#define BITSpPIX 8

#define PCOL 8
#define PROW 8
#define PMAX 4

#define PNG_HEIGHT (CMAX*ROW)
#define PNG_WIDTH (16)

#define P_HEIGHT (PMAX*PROW*ROW)
#define P_WIDTH (PCOL*16)

enum { C_BLUE = 0, C_RED, C_GREEN, C_ALPHA };

const unsigned char *cfiles = "C_600";
const unsigned char *pfiles = "P_600";
const unsigned char *cfiles_out = "C_600.png";
const unsigned char *pfiles_out = "P_600.png";

unsigned __int8 cbuf[CMAX][PLANE][ROW][2];	/* 2048 bytes = 16 * 8 * 4 * 32 */
unsigned __int8 pbuf[PMAX][PROW][PCOL];	/* 256 bytes = 8 * 8 * 4 */

unsigned __int64 decoded_pattern[CMAX][ROW][2];
unsigned __int64 vbuf[PMAX][PROW][ROW][PCOL][2];

#pragma pack(1)
unsigned __int8 decode_w16(unsigned __int64(*dst)[ROW][2], const unsigned __int8(*src)[PLANE][ROW][2], size_t len)
{
	unsigned __int8 max_ccode = 0;
	while (len--) {
		for (size_t y = 0; y < ROW; y++) {
			union {
				__int64 a;
				__int8 a8[8];
			} u[2];
			for (size_t x = 0; x < 8; x++) {
				u[0].a8[x] = (((*src)[0][y][0] & (1 << x)) ? 1 : 0) | (((*src)[1][y][0] & (1 << x)) ? 2 : 0) | (((*src)[2][y][0] & (1 << x)) ? 4 : 0) | (((*src)[3][y][0] & (1 << x)) ? 8 : 0);
				u[1].a8[x] = (((*src)[0][y][1] & (1 << x)) ? 1 : 0) | (((*src)[1][y][1] & (1 << x)) ? 2 : 0) | (((*src)[2][y][1] & (1 << x)) ? 4 : 0) | (((*src)[3][y][1] & (1 << x)) ? 8 : 0);
				if (max_ccode < u[0].a8[x]) {
					max_ccode = u[0].a8[x];
				}
				if (max_ccode < u[1].a8[x]) {
					max_ccode = u[1].a8[x];
				}
			}
			(*dst)[y][0] = _byteswap_uint64(u[0].a);
			(*dst)[y][1] = _byteswap_uint64(u[1].a);
		}
		src++;
		dst++;
	}
	return max_ccode + 1;
}
#pragma pack()

void box_copy(size_t c, size_t p, size_t py, size_t px, unsigned __int8 index)
{
	for (size_t r = 0; r < ROW; r++) {
		vbuf[c][py][r][px][0] = decoded_pattern[index][r][0];
		vbuf[c][py][r][px][1] = decoded_pattern[index][r][1];
	}
}

int main(void)
{
	FILE *pFi, *pFo;

	errno_t ecode = fopen_s(&pFi, cfiles, "rb");

	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", cfiles);
		exit(ecode);
	}
	size_t rcount = fread_s(cbuf, sizeof(cbuf), sizeof(cbuf), 1, pFi);
	if (rcount != 1) {
		fprintf_s(stderr, "File read error %s.\n", cfiles);
		fclose(pFi);
		exit(-2);
	}
	fclose(pFi);

	ecode = fopen_s(&pFi, pfiles, "rb");
	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", pfiles);
		exit(ecode);
	}
	rcount = fread_s(pbuf, sizeof(pbuf), sizeof(pbuf), 1, pFi);
	if (rcount != 1) {
		fprintf_s(stderr, "File read error %s.\n", pfiles);
		fclose(pFi);
		exit(-2);
	}
	fclose(pFi);

	unsigned __int8 colours = decode_w16(decoded_pattern, cbuf, CMAX);

	ecode = fopen_s(&pFo, cfiles_out, "wb");
	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", cfiles_out);
		exit(ecode);
	}

	png_structp png_ptr;
	png_infop info_ptr;
	png_color pal[16] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF},
						  {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF} };

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
	png_byte trans[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						   0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	png_set_tRNS(png_ptr, info_ptr, trans, colours, NULL);

	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(image);
	fclose(pFo);

#if 0
	for (size_t chara = 0; chara < PMAX; chara++) {
		for (size_t p_col = 0; p_col < PCOL; p_col++) {
			if (p_col & 1) {
				for (size_t p_row = 0; p_row < PROW; p_row++) {
					box_copy(chara, pattern, p_row, p_col, pbuf[chara][pattern][p_col][p_row]);
				}
			}
			else {
				for (size_t p_row = 0; p_row < PROW; p_row++) {
					box_copy(chara, pattern, PROW - 1 - p_row, p_col, pbuf[chara][pattern][p_col][p_row]);
				}
			}
		}
	}

	ecode = fopen_s(&pFo, pfiles_out, "wb");
	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", pfiles_out);
		exit(ecode);
	}

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

	image = (png_bytepp)malloc(P_HEIGHT * sizeof(png_bytep));
	if (image == NULL) {
		fprintf_s(stderr, "Memory allocation error.\n");
		fclose(pFo);
		exit(-2);
	}
	for (size_t j = 0; j < P_HEIGHT; j++)
		image[j] = (png_bytep)&vbuf[j / (PROW * ROW)][(j / ROW) % PROW][j % ROW];

	png_init_io(png_ptr, pFo);
	png_set_IHDR(png_ptr, info_ptr, P_WIDTH, P_HEIGHT,
		BITSpPIX, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(png_ptr, info_ptr, pal, 8);
	png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(image);
	fclose(pFo);
#endif
	exit(0);
}
