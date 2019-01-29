/* sorcerian character view program */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>

#include "png.h"

#define CMAX	0x100
#define PLANE 4
#define ROW 8
#define BITSpPIX 8

#define PCOL 8
#define PROW 8
#define PMAX 8
#define CPAT 2

#define PNG_HEIGHT (CMAX*ROW)
#define PNG_WIDTH (16)

#define P_HEIGHT (PMAX*PROW*ROW)
#define P_WIDTH (CPAT*PCOL*16)

#define P_FILES 3

enum { C_BLUE = 0, C_RED, C_GREEN, C_ALPHA };

const unsigned char *cfiles[3] = { "C_990", "C_991", "C_992" };
const unsigned char *pfiles[3] = { "P_990", "P_991", "P_992" };
const unsigned char *cfiles_out[3] = { "C_990.png", "C_991.png", "C_992.png" };
const unsigned char *pfiles_out[3] = { "P_990.png", "P_991.png", "P_992.png" };

unsigned __int8 cbuf3[CMAX][PLANE-1][ROW][2];	/* 12288 bytes = 16 * 8 * 3 * 256 */
unsigned __int8 pbuf[PMAX][CPAT][PROW][PCOL];	/* 1024 bytes = 8 * 8 * 2 * 8 */

unsigned __int64 decoded_pattern[CMAX][ROW][2];
unsigned __int64 vbuf[PMAX][PROW][ROW][CPAT][PCOL][2];

#pragma pack(1)
void decode_16bit_wide_3plane(void)
{
	for (size_t pat = 0; pat < CMAX; pat++) {
		for (size_t y = 0; y < ROW; y++) {
			union {
				__int64 a;
				__int8 a8[8];
			} u[2];
			for (size_t x = 0; x < 8; x++) {
				u[0].a8[x] = (!!(cbuf3[pat][0][y][0] & (1 << x))) | (!!(cbuf3[pat][1][y][0] & (1 << x))) << 1 | (!!(cbuf3[pat][2][y][0] & (1 << x))) << 2;
				u[1].a8[x] = (!!(cbuf3[pat][0][y][1] & (1 << x))) | (!!(cbuf3[pat][1][y][1] & (1 << x))) << 1 | (!!(cbuf3[pat][2][y][1] & (1 << x))) << 2;
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

void box_copy(size_t c, size_t p, size_t py, size_t px, unsigned __int8 index)
{
	for (size_t r = 0; r < ROW; r++) {
		vbuf[c][py][r][p][px][0] = decoded_pattern[index][r][0];
		vbuf[c][py][r][p][px][1] = decoded_pattern[index][r][1];
	}
}

int main(void)
{
	FILE *pFi, *pFo;

	for (size_t nfile = 0; nfile < P_FILES; nfile++) {
		errno_t ecode = fopen_s(&pFi, cfiles[nfile], "rb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", cfiles[nfile]);
			exit(ecode);
		}
		size_t rcount = fread_s(cbuf3, sizeof(cbuf3), sizeof(cbuf3), 1, pFi);
		if (rcount != 1) {
			fprintf_s(stderr, "File read error %s.\n", cfiles[nfile]);
			fclose(pFi);
			exit(-2);
		}
		fclose(pFi);

		ecode = fopen_s(&pFi, pfiles[nfile], "rb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", pfiles[nfile]);
			exit(ecode);
		}
		rcount = fread_s(pbuf, sizeof(pbuf), sizeof(pbuf), 1, pFi);
		if (rcount != 1) {
			fprintf_s(stderr, "File read error %s.\n", pfiles[nfile]);
			fclose(pFi);
			exit(-2);
		}
		fclose(pFi);
		decode_16bit_wide_3plane();

		ecode = fopen_s(&pFo, cfiles_out[nfile], "wb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", cfiles_out[nfile]);
			exit(ecode);
		}
		unsigned __int8 colours = maximum_colour_code() + 1;

		png_structp png_ptr;
		png_infop info_ptr;
		png_color pal[8] = {{0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF}};

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
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		free(image);
		fclose(pFo);

		for (size_t chara = 0; chara < PMAX; chara++) {
			for (size_t pattern = 0; pattern < CPAT; pattern++) {
				for (size_t p_col = 0; p_col < PCOL; p_col++) {
					if (p_col & 1) {
						for (size_t p_row = 0; p_row < PROW; p_row++) {
							box_copy(chara, pattern, p_row, p_col, pbuf[chara][pattern][p_col][p_row]);
						}
					} else {
						for (size_t p_row = 0; p_row < PROW; p_row++) {
							box_copy(chara, pattern, PROW - 1 - p_row, p_col, pbuf[chara][pattern][p_col][p_row]);
						}
					}
				}
			}
		}

		ecode = fopen_s(&pFo, pfiles_out[nfile], "wb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", pfiles_out[nfile]);
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
		png_set_PLTE(png_ptr, info_ptr, pal, colours);
		png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		free(image);
		fclose(pFo);
	}
	exit(0);
}
