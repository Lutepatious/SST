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

#define PCOL 2
#define PROW 3
#define PMAX 8

#define PNG_HEIGHT (PROW*ROW)
#define PNG_WIDTH (PMAX*(PCOL+2)*16)

#define P_RACE 4
#define P_SEX 2
#define P_AGE 3


#define C_FILES 24

const unsigned char *chfiles[C_FILES] = { "CH000","CH001","CH002","CH010","CH011","CH012",
									"CH100","CH101","CH102","CH110","CH111","CH112",
									"CH200","CH201","CH202","CH210","CH211","CH212",
									"CH300","CH301","CH302","CH310","CH311","CH312" };

const unsigned char *chfiles_out[C_FILES] = { "CH000.png","CH001.png","CH002.png","CH010.png","CH011.png","CH012.png",
										"CH100.png","CH101.png","CH102.png","CH110.png","CH111.png","CH112.png",
										"CH200.png","CH201.png","CH202.png","CH210.png","CH211.png","CH212.png",
										"CH300.png","CH301.png","CH302.png","CH310.png","CH311.png","CH312.png" };

unsigned char *pattern_file = "PLPTR";


unsigned __int8 cbuf[CMAX][PLANE][ROW][2];	/* 2048 bytes = 16 * 8 * 4 * 32 */
unsigned __int64 decoded_pattern[CMAX][ROW][2];

#pragma pack(1)
typedef struct CH_PATTERN {
	unsigned __int8 pat[PMAX][PCOL][PROW];
	unsigned __int8 dummy_pad[16];
} chpat;
#pragma pack()

chpat pattern[P_RACE][P_SEX][P_AGE + 1];
unsigned __int64 vbuf[PROW][ROW][PMAX][PCOL + 2][2];

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

void box_copy(size_t pat, size_t py, size_t px, unsigned __int8 index)
{
	for (size_t r = 0; r < ROW; r++) {
		vbuf[py][r][pat][px][0] = decoded_pattern[index][r][0];
		vbuf[py][r][pat][px][1] = decoded_pattern[index][r][1];
	}
}

int main(void)
{
	FILE *pFi, *pFo;

	errno_t ecode = fopen_s(&pFi, pattern_file, "rb");
	if (ecode) {
		fprintf_s(stderr, "File open error %s.\n", pattern_file);
		exit(ecode);
	}
	size_t rcount = fread_s(pattern, sizeof(pattern), sizeof(pattern), 1, pFi);
	if (rcount != 1) {
		fprintf_s(stderr, "File read error %s.\n", pattern_file);
		fclose(pFi);
		exit(-2);
	}
	fclose(pFi);

	for (size_t nfile = 0; nfile < C_FILES; nfile++) {
		memset(vbuf, 0x8, sizeof(vbuf));
		errno_t ecode = fopen_s(&pFi, chfiles[nfile], "rb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", chfiles[nfile]);
			exit(ecode);
		}
		size_t rcount = fread_s(cbuf, sizeof(cbuf), sizeof(cbuf), 1, pFi);
		if (rcount != 1) {
			fprintf_s(stderr, "File read error %s.\n", chfiles[nfile]);
			fclose(pFi);
			exit(-2);
		}
		fclose(pFi);

		decode_16bit_wide();
		unsigned __int8 colours = maximum_colour_code() + 1;


		for (size_t pat = 0; pat < PMAX; pat++) {
			for (size_t p_col = 0; p_col < PCOL; p_col++) {
				if (p_col & 1) {
					for (size_t p_row = 0; p_row < PROW; p_row++) {
						size_t idx = pattern[nfile / (P_AGE * P_SEX)][(nfile / P_AGE) % P_SEX][nfile%P_AGE].pat[pat][p_col][p_row];
						box_copy(pat, p_row, p_col + 1, idx);
					}
				}
				else {
					for (size_t p_row = 0; p_row < PROW; p_row++) {
						size_t idx = pattern[nfile / (P_AGE * P_SEX)][(nfile / P_AGE) % P_SEX][nfile%P_AGE].pat[pat][p_col][p_row];
						box_copy(pat, PROW - 1 - p_row, p_col + 1, idx);
					}
				}
			}
		}
		box_copy(4, 1, 3, 30);
		box_copy(5, 1, 3, 30);
		box_copy(6, 1, 0, 31);
		box_copy(7, 1, 0, 31);

		printf_s("Processing %s.\n", chfiles_out[nfile]);


		ecode = fopen_s(&pFo, chfiles_out[nfile], "wb");
		if (ecode) {
			fprintf_s(stderr, "File open error %s.\n", chfiles_out[nfile]);
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
			image[j] = (png_bytep)&vbuf[j / ROW][j % ROW];

		png_init_io(png_ptr, pFo);
		png_set_IHDR(png_ptr, info_ptr, PNG_WIDTH, PNG_HEIGHT,
			BITSpPIX, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_PLTE(png_ptr, info_ptr, pal, 16);
		png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
		png_byte trans[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		png_set_tRNS(png_ptr, info_ptr, trans, 16, NULL);

		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);


		free(image);
		fclose(pFo);
	}
}