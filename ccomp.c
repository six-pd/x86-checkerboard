// graf_io.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

typedef struct
{
	int width, height;
	unsigned char* pImg;
	int imgSize;
} imgInfo;

typedef struct
{
	unsigned short bfType; 
	unsigned long  bfSize; 
	unsigned short bfReserved1; 
	unsigned short bfReserved2; 
	unsigned long  bfOffBits; 
	unsigned long  biSize; 
	long  biWidth; 
	long  biHeight; 
	short biPlanes; 
	short biBitCount; 
	unsigned long  biCompression; 
	unsigned long  biSizeImage; 
	long biXPelsPerMeter; 
	long biYPelsPerMeter; 
	unsigned long  biClrUsed; 
	unsigned long  biClrImportant;
} bmpHdr;

void * checkerboard24(imgInfo * img, unsigned int sqsize, unsigned int c1, unsigned int c2);

void* freeResources(FILE* pFile, void* pFirst, void* pSnd)
{
	if (pFile != 0)
		fclose(pFile);
	if (pFirst != 0)
		free(pFirst);
	if (pSnd !=0)
		free(pSnd);
	return 0;
}

imgInfo* InitImage (int w, int h)
{
	imgInfo *pImg;
	if ( (pImg = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
		return 0;
	pImg->height = h;
	pImg->width = w;
	pImg->pImg = (unsigned char*) malloc((((w * 3 + 3) >> 2) << 2) * h);
	if (pImg->pImg == 0)
	{
		free(pImg);
		return 0;
	}
	memset(pImg->pImg, 0, (((w * 3 + 3) >> 2) << 2) * h);
	return pImg;
}

imgInfo * copyImage(const imgInfo* pImg)
{
	imgInfo *pNew = InitImage(pImg->width, pImg->height);
	if (pNew != 0)
		memcpy(pNew->pImg, pImg->pImg, pNew->imgSize);
	return pNew;
}

void FreeImage(imgInfo* pInfo)
{
	if (pInfo && pInfo->pImg)
		free(pInfo->pImg);
	if (pInfo)
		free(pInfo);
}


imgInfo* readBMP(const char* fname)
{
	imgInfo* pInfo = 0;
	FILE* fbmp = 0;
	bmpHdr bmpHead;
	int lineBytes, y;
	unsigned char* ptr;

	pInfo = 0;
	fbmp = fopen(fname, "rb");
	if (fbmp == 0)
		return 0;

	fread((void *) &bmpHead, sizeof(bmpHead), 1, fbmp);
	// par� sprawdze�
	if (bmpHead.bfType != 0x4D42 || bmpHead.biPlanes != 1 ||
		bmpHead.biBitCount != 24 || 
		(pInfo = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo ? pInfo->pImg : 0, pInfo);

	pInfo->width = bmpHead.biWidth;
	pInfo->height = bmpHead.biHeight;
	if ((pInfo->pImg = (unsigned char*) malloc(bmpHead.biSizeImage)) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	// porz�dki z wysoko�ci� (mo�e by� ujemna)
	ptr = pInfo->pImg;
	lineBytes = ((pInfo->width * 3 + 3) >> 2) << 2; // rozmiar linii w bajtach
	if (pInfo->height > 0)
	{
		// "do g�ry nogami", na pocz�tku d� obrazu
		ptr += lineBytes * (pInfo->height - 1);
		lineBytes = -lineBytes;
	}
	else
		pInfo->height = -pInfo->height;

	// sekwencja odczytu obrazu 
	// przesuni�cie na stosown� pozycj� w pliku
	if (fseek(fbmp, bmpHead.bfOffBits, SEEK_SET) != 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	for (y=0; y<pInfo->height; ++y)
	{
		fread(ptr, 1, abs(lineBytes), fbmp);
		ptr += lineBytes;
	}
	fclose(fbmp);
	return pInfo;
}

int saveBMP(const imgInfo* pInfo, const char* fname)
{
	int lineBytes = ((pInfo->width * 3 + 3) >> 2)<<2;
	bmpHdr bmpHead = 
	{
	0x4D42,				// unsigned short bfType; 
	sizeof(bmpHdr),		// unsigned long  bfSize; 
	0, 0,				// unsigned short bfReserved1, bfReserved2; 
	sizeof(bmpHdr),		// unsigned long  bfOffBits; 
	40,					// unsigned long  biSize; 
	pInfo->width,		// long  biWidth; 
	pInfo->height,		// long  biHeight; 
	1,					// short biPlanes; 
	24,					// short biBitCount; 
	0,					// unsigned long  biCompression; 
	lineBytes * pInfo->height,	// unsigned long  biSizeImage; 
	11811,				// long biXPelsPerMeter; = 300 dpi
	11811,				// long biYPelsPerMeter; 
	2,					// unsigned long  biClrUsed; 
	0,					// unsigned long  biClrImportant;
	};

	FILE * fbmp;
	unsigned char *ptr;
	int y;

	if ((fbmp = fopen(fname, "wb")) == 0)
		return -1;
	if (fwrite(&bmpHead, sizeof(bmpHdr), 1, fbmp) != 1)
	{
		fclose(fbmp);
		return -2;
	}

	ptr = pInfo->pImg + lineBytes * (pInfo->height - 1);
	for (y=pInfo->height; y > 0; --y, ptr -= lineBytes)
		if (fwrite(ptr, sizeof(unsigned char), lineBytes, fbmp) != lineBytes)
		{
			fclose(fbmp);
			return -3;
		}
	fclose(fbmp);
	return 0;
}


/* na galerze kompiluj� i konsoliduj�:

	gcc -m32 -fpack-struct=1 graf_io.c

*/

int main(int argc, char* argv[])
{
	imgInfo *pInfo;

	unsigned int color1 = 0;
	unsigned int color2 = 0;

	if (sizeof(bmpHdr) != 54)
	{
		printf("Trzeba zmieni� opcje kompilacji, tak by rozmiar struktury bmpHdr wynosi� 54 bajty.\n");
		return 1;
	}

	if(argc != 6){
		printf("Usage: checkerboard24 square_size width height (hex)color1 (hex)color2\n");
		return -1;
	}
	int i, j;
	for(i = 1; i <= 5; ++i){
		for(j = 1; argv[i][j] != 0; ++j)
		{
			if(argv[i][j] < '0' || (argv[i][j] > '9' && argv[i][j] < 'a') || argv[i][j] > 'f'){
				printf("Arguments must be integers\n");
				return -2;
			}
		}
	}

	if(strlen(argv[4]) != 6 || strlen(argv[5]) != 6){
		printf("Hex RGB values must be 6 digits\n");
		return -3;
	}

	for (i = 0; i <= 5; ++i)
	{
		color1 = color1 * 16;
		if(argv[4][i] > '9'){
			color1 = color1 + (argv[4][i] - 87); //for digits a, b, c, d, e, f
		}
		else
		{
			color1 = color1 + (argv[4][i] - '0');
		}
		
	}

	for (i = 0; i <= 5; ++i)
	{
		color2 = color2 * 16;
		if(argv[5][i] > '9'){
			color2 = color2 + (argv[5][i] - 87); //for digits a, b, c, d, e, f
		}
		else
		{
			color2 = color2 + (argv[5][i] - '0');
		}
		
	}
	
	pInfo = InitImage(atoi(argv[2]), atoi(argv[3]));

	if(pInfo == 0){
		printf("Failed to initialize image.\n");
		return -4;
	}

	checkerboard24(pInfo, atoi(argv[1]), color1, color2);

	if(saveBMP(pInfo, "checkerboard.bmp") != 0){
		printf("Failed to save image.\n");
		return -5;
	}

	FreeImage(pInfo);
	return 0;
}

