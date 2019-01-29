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
#define PMAX 256

#define PNG_HEIGHT (CMAX*ROW)
#define PNG_WIDTH (16)

#define P_HEIGHT (PROW*ROW)
#define P_WIDTH (PCOL*16)

#define P_FILES 24

enum { C_BLUE = 0, C_RED, C_GREEN, C_ALPHA };

const unsigned char *cfiles[24] = { "EC000", "EC001", "EC002", "EC010", "EC011", "EC012",
									"EC100", "EC101", "EC102", "EC110", "EC111", "EC112",
									"EC200", "EC201", "EC202", "EC210", "EC211", "EC212",
									"EC300", "EC301", "EC302", "EC310", "EC311", "EC312" };

const unsigned char *pfiles[24] = { "EP000", "EP001", "EP002", "EP010", "EP011", "EP012",
									"EP100", "EP101", "EP102", "EP110", "EP111", "EP112",
									"EP200", "EP201", "EP202", "EP210", "EP211", "EP212",
									"EP300", "EP301", "EP302", "EP310", "EP311", "EP312" };

const unsigned char *cfiles_out[24] = { "EC000.png", "EC001.png", "EC002.png", "EC010.png", "EC011.png", "EC012.png",
										"EC100.png", "EC101.png", "EC102.png", "EC110.png", "EC111.png", "EC112.png",
										"EC200.png", "EC201.png", "EC202.png", "EC210.png", "EC211.png", "EC212.png",
										"EC300.png", "EC301.png", "EC302.png", "EC310.png", "EC311.png", "EC312.png" };

const unsigned char *pfiles_out[24] = { "EP000.png", "EP001.png", "EP002.png", "EP010.png", "EP011.png", "EP012.png",
										"EP100.png", "EP101.png", "EP102.png", "EP110.png", "EP111.png", "EP112.png",
										"EP200.png", "EP201.png", "EP202.png", "EP210.png", "EP211.png", "EP212.png",
										"EP300.png", "EP301.png", "EP302.png", "EP310.png", "EP311.png", "EP312.png" };

const unsigned char *wfile = "EWAKU";

unsigned __int8 cbuf[CMAX][PLANE][ROW][2];	/* 2048 bytes = 16 * 8 * 4 * 32 */
unsigned __int8 pbuf[PMAX];	/* 256 bytes */

unsigned __int64 decoded_pattern[CMAX][ROW][2];
unsigned __int64 vbuf[PROW][ROW][PCOL][2];

#define WMAX 8
unsigned __int8 wbuf[WMAX][PLANE][ROW][2];	/* 512 bytes = 16 * 8 * 4 * 8 */
unsigned __int64 decoded_pattern_w[WMAX][ROW][2];

#pragma pack(1)
void decode_16bit_wide(void)
{
	for (size_t pat = 0; pat < CMAX; pat++) {
		for (size_t y = 0; y < ROW; y++) {
			union {
				__int64 a;
				__int8 a8[8];
			} u[2];
			for (size_t x = 0; x < 8; x++) {
				u[0].a8[x] = (!!(cbuf[pat][0][y][0] & (1 << x))) | (!!(cbuf[pat][1][y][0] & (1 << x))) << 1 | (!!(cbuf[pat][2][y][0] & (1 << x))) << 2 | (!!(cbuf[pat][3][y][0] & (1 << x))) << 3;
				u[1].a8[x] = (!!(cbuf[pat][0][y][1] & (1 << x))) | (!!(cbuf[pat][1][y][1] & (1 << x))) << 1 | (!!(cbuf[pat][2][y][1] & (1 << x))) << 2 | (!!(cbuf[pat][3][y][1] & (1 << x))) << 3;
			}
			decoded_pattern[pat][y][0] = _byteswap_uint64(u[0].a);
			decoded_pattern[pat][y][1] = _byteswap_uint64(u[1].a);
		}
	}
}

void decode_16bit_wide_w(void)
{
	for (size_t pat = 0; pat < WMAX; pat++) {
		for (size_t y = 0; y < ROW; y++) {
			union {
				__int64 a;
				__int8 a8[8];
			} u[2];
			for (size_t x = 0; x < 8; x++) {
				u[0].a8[x] = (!!(wbuf[pat][0][y][0] & (1 << x))) | (!!(wbuf[pat][1][y][0] & (1 << x))) << 1 | (!!(wbuf[pat][2][y][0] & (1 << x))) << 2 | (!!(wbuf[pat][3][y][0] & (1 << x))) << 3;
				u[1].a8[x] = (!!(wbuf[pat][0][y][1] & (1 << x))) | (!!(wbuf[pat][1][y][1] & (1 << x))) << 1 | (!!(wbuf[pat][2][y][1] & (1 << x))) << 2 | (!!(wbuf[pat][3][y][1] & (1 << x))) << 3;
			}
			decoded_pattern_w[pat][y][0] = _byteswap_uint64(u[0].a);
			decoded_pattern_w[pat][y][1] = _byteswap_uint64(u[1].a);
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

void box_copy(size_t py, size_t px, unsigned __int8 index)
{
	for (size_t r = 0; r < ROW; r++) {
		vbuf[py][r][px][0] = decoded_pattern[index][r][0];
		vbuf[py][r][px][1] = decoded_pattern[index][r][1];
	}
}

void box_copy_w(size_t py, size_t px, unsigned __int8 index)
{
	for (size_t r = 0; r < ROW; r++) {
		vbuf[py][r][px][0] = decoded_pattern_w[index][r][0];
		vbuf[py][r][px][1] = decoded_pattern_w[index][r][1];
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
		size_t rcount = fread_s(cbuf, sizeof(cbuf), sizeof(cbuf), 1, pFi);
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
		decode_16bit_wide();

		ecode = fopen_s(&pFo, cfiles_out[nfile], "wb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", cfiles_out[nfile]);
			exit(ecode);
		}
		unsigned __int8 colours = maximum_colour_code() + 1;

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
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		free(image);
		fclose(pFo);

		if (pbuf[0] != 6) {
			fprintf_s(stderr, "DAT not match %s. Skip!\n", pfiles[nfile]);
			continue;
		}

		unsigned __int8 (*pbox)[6][6];
		pbox = &pbuf[1];

		for (size_t p_col = 1; p_col < PCOL - 1; p_col++) {
			if ((p_col - 1) & 1) {
				for (size_t p_row = 1; p_row < PROW - 1; p_row++) {
					box_copy(p_row, p_col, (*pbox)[p_col - 1][p_row - 1]);
				}
			}
			else {
				for (size_t p_row = 1; p_row < PROW - 1; p_row++) {
					box_copy(PROW - 1 - p_row, p_col, (*pbox)[p_col - 1][p_row - 1]);
				}
			}
		}

		ecode = fopen_s(&pFi, wfile, "rb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", wfile);
			exit(ecode);
		}
		rcount = fread_s(wbuf, sizeof(wbuf), sizeof(wbuf), 1, pFi);
		if (rcount != 1) {
			fprintf_s(stderr, "File read error %s.\n", wfile);
			fclose(pFi);
			exit(-2);
		}
		fclose(pFi);

		decode_16bit_wide_w();
		box_copy_w(0, 0, 0);
		box_copy_w(0, 1, 1);
		box_copy_w(0, 2, 1);
		box_copy_w(0, 3, 1);
		box_copy_w(0, 4, 1);
		box_copy_w(0, 5, 1);
		box_copy_w(0, 6, 1);
		box_copy_w(0, 7, 2);
		box_copy_w(1, 0, 3);
		box_copy_w(2, 0, 3);
		box_copy_w(3, 0, 3);
		box_copy_w(4, 0, 3);
		box_copy_w(5, 0, 3);
		box_copy_w(6, 0, 3);
		box_copy_w(1, 7, 4);
		box_copy_w(2, 7, 4);
		box_copy_w(3, 7, 4);
		box_copy_w(4, 7, 4);
		box_copy_w(5, 7, 4);
		box_copy_w(6, 7, 4);
		box_copy_w(7, 0, 5);
		box_copy_w(7, 1, 6);
		box_copy_w(7, 2, 6);
		box_copy_w(7, 3, 6);
		box_copy_w(7, 4, 6);
		box_copy_w(7, 5, 6);
		box_copy_w(7, 6, 6);
		box_copy_w(7, 7, 7);

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
			image[j] = (png_bytep)&vbuf[j / ROW][j % ROW];

		png_init_io(png_ptr, pFo);
		png_set_IHDR(png_ptr, info_ptr, P_WIDTH, P_HEIGHT,
			BITSpPIX, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_PLTE(png_ptr, info_ptr, pal, 16);
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
