#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

#define FLG_NOATR	1
#define FLG_NOBRG	(1<<1)

void getScreen(SDL_Surface* surf, char* buf, size_t sze, int flag) {
	char* ptr = surf->pixels;
	int sadr = 0;
	int aadr = 0x1800;
	char scr, atr, scr2, atr2;
	char col,col2;
	int x,y,c;
	for (y = 0; y < 192; y++) {
		for (x = 0; x < 32; x++) {
			atr = buf[aadr];
			scr = buf[sadr];
			atr2 = buf[aadr + 0x1b00];
			scr2 = buf[sadr + 0x1b00];
			if (flag & FLG_NOATR) {
				atr = 0x47;
				atr2 = 0x47;
			}
			if (flag & FLG_NOBRG) {
				atr &= 0xbf;
				atr2 &= 0xbf;
			}
			for (c = 0; c < 8; c++) {
				if (scr & 0x80) {
					col = ((atr & 0x40) >> 3) | (atr & 0x07);	// ink
				} else {
					col = (atr & 0x78) >> 3;			// pap
				}
				if (sze == 0x3600) {
					if (scr2 & 0x80) {
						col2 = ((atr2 & 0x40) >> 3) | (atr2 & 0x07);
					} else {
						col2 = (atr2 & 0x78) >> 3;
					}
				} else {
					col2 = col;
				} 
				col = col | (col2 << 4);
				*(ptr++) = col;
				*(ptr++) = col;
				*(ptr+510) = col;
				*(ptr+511) = col;
				scr <<= 1;
				scr2 <<= 1;
			}
			sadr++;
			aadr++;
		}
		ptr += 512;
		aadr -= 32;
		sadr += 0xe0;
		if ((sadr & 0x700) == 0) {
			aadr += 32;
			sadr -= 0x7e0;
			if (sadr & 0x700) {
				sadr += 0x700;
			}
		}
	}
	SDL_UpdateRect(surf,0,0,0,0);
}

int main(int ac, char** av) {
	if (ac < 2) {
		printf("filename needed\n");
		return 1;
	}
	char buf[0x3600];
	FILE* file = fopen(av[1],"rb");
	if (!file) {
		printf("Can't open file '%s'\n",av[1]);
		return 1;
	}
	fseek(file,0,SEEK_END);
	size_t sze = ftell(file);
	rewind(file);
	fread(buf,0x3600,1,file);
	fclose(file);
	
	if (sze == 0x1800) {
		memset(buf + 0x1800, 0x07, 0x300);
	} else if (sze == 0x1b00) {
	} else if (sze == 0x3600) {	
	} else {
		memset(buf,0x00,0x3600);
	}
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface* surf = SDL_SetVideoMode(512,384,8,SDL_SWSURFACE);
	SDL_Color pal[256];
	int c, idx, idx2;
	for (c = 0; c < 16; c++) {
		idx = (c << 4) | c;
		pal[idx].b = (c & 1) ? ((c & 8) ? 0xff : 0xc0) : 0x00;
		pal[idx].r = (c & 2) ? ((c & 8) ? 0xff : 0xc0) : 0x00;
		pal[idx].g = (c & 4) ? ((c & 8) ? 0xff : 0xc0) : 0x00;
	}
	for (c = 0; c < 256; c++) {
		idx = (c & 0x0f) | ((c & 0x0f) << 4);
		idx2 = (c & 0xf0) | ((c & 0xf0) >> 4);
		pal[c].b = (pal[idx].b >> 1) + (pal[idx2].b >> 1);
		pal[c].r = (pal[idx].r >> 1) + (pal[idx2].r >> 1);
		pal[c].g = (pal[idx].g >> 1) + (pal[idx2].g >> 1);
	}
	SDL_SetColors(surf, pal, 0, 256);
	
	int flags = 0;
	
	getScreen(surf, buf, sze, flags);
	
	int work = 1;
	SDL_Event ev;
	do {
		SDL_Delay(20);
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_QUIT:
					work = 0;
					break;
				case SDL_KEYDOWN:
					switch (ev.key.keysym.sym) {
						case SDLK_a:
							flags ^= FLG_NOATR;
							getScreen(surf, buf, sze, flags);
							break;
						case SDLK_b:
							flags ^= FLG_NOBRG;
							getScreen(surf, buf, sze, flags);
							break;
						case SDLK_ESCAPE:
							work = 0;
							break;
						default:
							break;
					}
					break;
			}
		}
	} while (work);
	
	SDL_Quit();
	
	return 1;	
}