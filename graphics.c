/*
	This file is part of AutoStart.

    AutoStart is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AutoStart is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AutoStart.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <png.h>
#include <pspgu.h>
#include <string.h>
#include "error.h"
#include "graphics.h"

Color* graphicsVRamAddr = (Color*) (0x40000000 | 0x04000000);

#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define FALSE 0
#define TRUE !FALSE

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH * SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

typedef struct {
	unsigned short u, v;
	short x, y, z;
} Vertex;

typedef struct {
     float u, v;
     short x, y, z;
} ResizeVertex;

extern u8 msx[];

unsigned int __attribute__((aligned(16))) list[262144];
static int graphicsBufferNumber;
static char graphicsInitialized = 0;

ImageStack_Entry* imageStack = NULL;

static int getNextPower2(int inWidth) {
     int tempOut = inWidth;
     int tempBits;
     for(tempBits = 0; tempOut != 0; tempBits++)
          tempOut >>= 1;
     tempOut = 1 << tempBits;
     if(tempOut == 2 * inWidth)
          tempOut >>= 1;
     return tempOut;
}

static void drawLine(int inX0, int inY0, int inX1, int inY1, Color inColor, Color* inDestination, int inWidth)
{
     int tempDY = inY1 - inY0;
     int tempDX = inX1 - inX0;
     int tempStepX, tempStepY;

     if(tempDY < 0) {
          tempDY = -tempDY;  tempStepY = -inWidth; 
     } else {
          tempStepY = inWidth;
     }

     if(tempDX < 0) {
          tempDX = -tempDX;  tempStepX = -1;
     } else {
          tempStepX = 1;
     }

     tempDY <<= 1;
     tempDX <<= 1;
	
     inY0 *= inWidth;
     inY1 *= inWidth;
     inDestination[inX0 + inY0] = inColor;
     if(tempDX > tempDY) {
          int tempFraction = tempDY - (tempDX >> 1);
          while(inX0 != inX1) {
               if(tempFraction >= 0) {
                    inY0 += tempStepY;
                    tempFraction -= tempDX;
               }
               inX0 += tempStepX;
               tempFraction += tempDY;
               inDestination[inX0 + inY0] = inColor;
          }
     } else {
          int tempFraction = tempDX - (tempDY >> 1);
          while(inY0 != inY1) {
               if(tempFraction >= 0) {
                    inX0 += tempStepX;
                    tempFraction -= tempDY;
               }
               inY0 += tempStepY;
               tempFraction += tempDX;
               inDestination[inX0 + inY0] = inColor;
          }
     }
}

void graphicsInit() {
     graphicsBufferNumber = 0;

     sceGuInit();

     sceGuStart(GU_DIRECT, list);
     sceGuDrawBuffer(GU_PSM_8888, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
     sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
     sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
     sceGuDepthBuffer((void*) (FRAMEBUFFER_SIZE*2), PSP_LINE_SIZE);
     sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
     sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
     sceGuDepthRange(0xc350, 0x2710);
     sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
     sceGuEnable(GU_SCISSOR_TEST);
     sceGuAlphaFunc(GU_GREATER, 0, 0xff);
     sceGuEnable(GU_ALPHA_TEST);
     sceGuDepthFunc(GU_GEQUAL);
     sceGuEnable(GU_DEPTH_TEST);
     sceGuFrontFace(GU_CW);
     sceGuShadeModel(GU_SMOOTH);
     sceGuEnable(GU_CULL_FACE);
     sceGuEnable(GU_TEXTURE_2D);
     sceGuEnable(GU_CLIP_PLANES);
     sceGuTexMode(GU_PSM_8888, 0, 0, 0);
     sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
     sceGuTexFilter(GU_NEAREST, GU_NEAREST);
     sceGuAmbientColor(0xffffffff);
     sceGuEnable(GU_BLEND);
     sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
     sceGuFinish();
     sceGuSync(0, 0);

     sceDisplayWaitVblankStart();
     sceGuDisplay(GU_TRUE);
     graphicsInitialized = TRUE;
}

void graphicsDisable() {
     graphicsInitialized = FALSE;
}

Color* graphicsVRamDrawBuffer() {
     Color* tempVRam = (Color*)graphicsVRamAddr;
     if (graphicsBufferNumber == 0)
          tempVRam += FRAMEBUFFER_SIZE / sizeof(Color);
     return tempVRam;
}

Color* graphicsVRamDisplayBuffer() {
     Color* tempVRam = (Color*)graphicsVRamAddr;
     if (graphicsBufferNumber == 1)
          tempVRam += FRAMEBUFFER_SIZE / sizeof(Color);
     return tempVRam;
}

void graphicsSwapBuffers() {
     if(!graphicsInitialized)
          return;
     sceGuSwapBuffers();
     graphicsBufferNumber ^= 1;
}

void graphicsCopyBuffers() {
     if(!graphicsInitialized)
          return;
     Color* tempVRamDraw = graphicsVRamDrawBuffer();
     Color* tempVRamDisp = graphicsVRamDisplayBuffer();
     memcpy(tempVRamDraw, tempVRamDisp, (SCR_HEIGHT * BUF_WIDTH) * sizeof(Color));
}

Image* imageCreate(int inWidth, int inHeight) {
     Image* tempImage = (Image*) malloc(sizeof(Image));
     if(!tempImage) {
          char tempString[256];
          sprintf(tempString, "Couldn't create image (%ix%i).\nOut of memory.", inWidth, inHeight);    
          errorThrow(tempString);
          return NULL;
     }
     tempImage->imageWidth = inWidth;
     tempImage->imageHeight = inHeight;
     tempImage->imageTextureWidth = getNextPower2(inWidth);
     tempImage->imageTextureHeight = getNextPower2(inHeight);
     tempImage->imageData = (Color*) memalign(16, tempImage->imageTextureWidth * tempImage->imageTextureHeight * sizeof(Color));
     if(!tempImage->imageData) {
          char tempString[256];
          sprintf(tempString, "Couldn't create image (%ix%i).\nOut of memory.", inWidth, inHeight);    
          errorThrow(tempString);
          return NULL;
     }
     memset(tempImage->imageData, 0, tempImage->imageTextureWidth * tempImage->imageTextureHeight * sizeof(Color));
     return tempImage;
}

Image* imageFromScreen() {
     if(!graphicsInitialized)
          return NULL;
     Image* tempImage = imageCreate(SCR_WIDTH, SCR_HEIGHT);
     if(!tempImage) {
          char tempString[256];
          sprintf(tempString, "Can't create screenshot.\nOut of memory.");
          errorThrow(tempString);
     }
     Color* tempVRam = graphicsVRamDisplayBuffer();
     int i;
     for(i = 0; i < SCR_HEIGHT; i++) {
          memcpy(&tempImage->imageData[i * tempImage->imageTextureWidth], &tempVRam[PSP_LINE_SIZE * i] , (SCR_WIDTH * sizeof(Color)));
     }
     return(tempImage);
}

Image* imageFromBuffer() {
     if(!graphicsInitialized)
          return NULL;
     Image* tempImage = imageCreate(SCR_WIDTH, SCR_HEIGHT);
     if(!tempImage) {
          char tempString[256];
          sprintf(tempString, "Can't create screenshot.\nOut of memory.");
          errorThrow(tempString);
     }
     Color* tempVRam = graphicsVRamDrawBuffer();
     int i;
     for(i = 0; i < SCR_HEIGHT; i++) {
          memcpy(&tempImage->imageData[i * tempImage->imageTextureWidth], &tempVRam[PSP_LINE_SIZE * i] , (SCR_WIDTH * sizeof(Color)));
     }
     return(tempImage);
}

Image* imageThumbnail(Image* inImage) {
     Image* tempImage = imageCreate((inImage->imageWidth >> 1), (inImage->imageHeight >> 1));
     int i, j; Color tempColor;
     for(j = 0; j < tempImage->imageHeight; j++) {
          for(i = 0; i < tempImage->imageWidth; i++) {
               tempColor = colorMerge(
                 colorMerge(inImage->imageData[((j << 1) * inImage->imageTextureWidth) + (i << 1)], inImage->imageData[((j << 1) * inImage->imageTextureWidth) + (i << 1) + 1]), 
                 colorMerge(inImage->imageData[(((j << 1) + 1) * inImage->imageTextureWidth) + (i << 1)], inImage->imageData[(((j << 1) + 1) * inImage->imageTextureWidth) + (i << 1) + 1]));
               tempImage->imageData[(j * tempImage->imageTextureWidth) + i] = tempColor;
          }
     }
     return tempImage;
}

void imageLoadPngWarning(png_structp png_ptr, png_const_charp warning_msg) {
     char tempString[256];
     sprintf(tempString, "Unknown libpng warning.");    
     errorThrow(tempString);
}

Image* imageLoadPNG(char* inPath) {
     ImageStack_Entry* tempImageStackEntry = imageStackFindByPath(inPath);
     if(tempImageStackEntry != NULL)
          return tempImageStackEntry->imageStackImage;
     png_structp png_ptr;
     png_infop info_ptr;
     unsigned int sig_read = 0;
     png_uint_32 tempWidth, tempHeight;
     int bit_depth, color_type, interlace_type, x, y;
     u32* tempLine;
     FILE* tempFile;
     Image* tempImage = (Image*) malloc(sizeof(Image));
     if(!tempImage) {
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nOut of memory.", inPath);    
          errorThrow(tempString);
          return NULL;
     }

	 if((tempFile = fopen(inPath, "rb")) == NULL) {
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nDoesn't exist.", inPath);    
          errorThrow(tempString);
          return NULL;
     }
     
     png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
     if (png_ptr == NULL) {
          free(tempImage);
          fclose(tempFile);
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nInvalid data.", inPath);    
          errorThrow(tempString);
          return NULL;
     }
     
     png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, imageLoadPngWarning);
     info_ptr = png_create_info_struct(png_ptr);
     if (info_ptr == NULL) {
          free(tempImage);
          fclose(tempFile);
          png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nInvalid data header.", inPath);    
          errorThrow(tempString);
          return NULL;
     }
     
     png_init_io(png_ptr, tempFile);
     png_set_sig_bytes(png_ptr, sig_read);
     png_read_info(png_ptr, info_ptr);
     png_get_IHDR(png_ptr, info_ptr, &tempWidth, &tempHeight, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
     if(tempWidth > 512 || tempHeight > 512) {
          free(tempImage);
          fclose(tempFile);
          png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nImage too large.", inPath);    
          errorThrow(tempString);
          return NULL;
     }

     tempImage->imageWidth = tempWidth;
     tempImage->imageHeight = tempHeight;
     tempImage->imageTextureWidth = getNextPower2(tempWidth);
     tempImage->imageTextureHeight = getNextPower2(tempHeight);
     png_set_strip_16(png_ptr);
     png_set_packing(png_ptr);
     if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
     if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
     if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
     png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
     
     tempImage->imageData = (Color*) memalign(16, tempImage->imageTextureWidth * tempImage->imageTextureHeight * sizeof(Color));
     if (!tempImage->imageData) {
          free(tempImage);
          fclose(tempFile);
          png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nOut of memory.", inPath);    
          errorThrow(tempString);
          return NULL;
     }
     
     tempLine = (u32*) malloc(tempWidth * 4);
     if (!tempLine) {
          free(tempImage->imageData);
          free(tempImage);
          fclose(tempFile);
          png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
          char tempString[256];
          sprintf(tempString, "Couldn't load image (%s).\nOut of memory.", inPath);    
          errorThrow(tempString);
          return NULL;
     }
     
     for(y = 0; y < tempHeight; y++) {
          png_read_row(png_ptr, (u8*) tempLine, png_bytep_NULL);
          for(x = 0; x < tempWidth; x++) {
               u32 tempColor = tempLine[x];
               tempImage->imageData[x + y * tempImage->imageTextureWidth] =  tempColor;
          }
     }
     free(tempLine);
     png_read_end(png_ptr, info_ptr);
     png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
     fclose(tempFile);
     
     imageStackAdd(inPath, tempImage);
     return tempImage;
}

void imageSavePNG(const char* inPath, Image* inImage, int inSaveAlpha)
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* tempFile;
	int i, x, y;
	u8* tempLine;
	
     if((tempFile = fopen(inPath, "wb")) == NULL) {
          char tempString[256];
          sprintf(tempString, "Couldn't save image (%s).\nFile Error.", inPath);
          errorThrow(tempString);
          return;
     }

     png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
     if(!png_ptr) {
          char tempString[256];
          sprintf(tempString, "Couldn't save image (%s).\nOut of memory.", inPath);
          errorThrow(tempString);
          return;
     }
     
     info_ptr = png_create_info_struct(png_ptr);
     if(!info_ptr) {
          png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		  char tempString[256];
          sprintf(tempString, "Couldn't save image (%s).\nOut of memory.", inPath);
          errorThrow(tempString);
          return;
     }
     
     png_init_io(png_ptr, tempFile);
     png_set_IHDR(png_ptr, info_ptr, inImage->imageWidth, inImage->imageHeight, 8,
        inSaveAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
     png_write_info(png_ptr, info_ptr);
     tempLine = (u8*) malloc(inImage->imageWidth * (inSaveAlpha ? 4 : 3));
     for(y = 0; y < inImage->imageHeight; y++) {
          for(i = 0, x = 0; x < inImage->imageWidth; x++) {
               Color tempColor = inImage->imageData[x + y * inImage->imageTextureWidth];
               u8 r = tempColor & 0xff; 
               u8 g = (tempColor >> 8) & 0xff;
               u8 b = (tempColor >> 16) & 0xff;
               u8 a = inSaveAlpha ? (tempColor >> 24) & 0xff : 0xff;
               tempLine[i++] = r;
               tempLine[i++] = g;
               tempLine[i++] = b;
               if(inSaveAlpha)
                    tempLine[i++] = a;
          }
          png_write_row(png_ptr, tempLine);
     }
     free(tempLine);
     png_write_end(png_ptr, info_ptr);
     png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
     fclose(tempFile);
}

void imageFree(Image* inImage) {
     if(inImage == NULL)
          return;

     ImageStack_Entry* tempImageStackEntry = imageStackFindByImage(inImage);
     if(tempImageStackEntry != NULL) {
          imageStackRemove(inImage);
          return;
     }

     if(inImage->imageData != NULL)
          free(inImage->imageData);
     free(inImage);
}

void imageClear(Color inColor, Image* inImage) {
     int tempSize = inImage->imageTextureWidth * inImage->imageTextureHeight;
     Color* tempData = inImage->imageData;
     int i;
     for (i = 0; i < tempSize; i++, tempData++)
          *tempData = inColor;
}

Color imageGetPixel(int inX, int inY, Image* inImage) {
     return inImage->imageData[inX + (inY * inImage->imageTextureWidth)];
}

void imageSetPixel(int inX, int inY, Color inColor, Image* inImage) {
     inImage->imageData[inX + (inY * inImage->imageTextureWidth)] = inColor;
}

void imageDrawLine(int inX0, int inY0, int inX1, int inY1, Color inColor, Image* inImage) {
     drawLine(inX0, inY0, inX1, inY1, inColor, inImage->imageData, inImage->imageTextureWidth);
}

void imageDrawRect(int inX, int inY, int inWidth, int inHeight, Color inColor, Image* inImage) {
     int tempX = inX + inWidth;
     int tempY = inY + inHeight;
     int i;
     for(i = inX; i < tempX; i++) {
          inImage->imageData[(inY * inImage->imageTextureWidth) + i] = inColor;
          inImage->imageData[((tempY - 1) * inImage->imageTextureWidth) + i] = inColor;
     }
     for(i = inY; i < tempY; i++) {
          inImage->imageData[(i * inImage->imageTextureWidth) + inX] = inColor;
          inImage->imageData[(i * inImage->imageTextureWidth) + (tempX - 1)] = inColor;
     }
}

void imageDrawFilledRect(int inX, int inY, int inWidth, int inHeight, Color inColor, Image* inImage) {
     int tempSkipX = inImage->imageTextureWidth - inWidth;
     int x, y;
     Color* tempData = inImage->imageData + inX + (inY * inImage->imageTextureWidth);
     for(y = 0; y < inHeight; y++, tempData += tempSkipX) {
          for(x = 0; x < inWidth; x++, tempData++)
               *tempData = inColor;
     }
}

void imageDraw(int inDrawX, int inDrawY, Image* inImage) {
     if (!graphicsInitialized)
          return;

     Color* tempVRam = graphicsVRamDrawBuffer();
     sceKernelDcacheWritebackInvalidateAll();
     sceGuStart(GU_DIRECT, list);
     sceGuCopyImage(GU_PSM_8888, 0, 0, inImage->imageWidth, inImage->imageHeight, inImage->imageTextureWidth, inImage->imageData, inDrawX, inDrawY, PSP_LINE_SIZE, tempVRam);
     sceGuFinish();
     sceGuSync(0,0);
}

void imageDrawCenter(Image* inImage) {
     imageDraw(((SCR_WIDTH - inImage->imageWidth) >> 1), ((SCR_HEIGHT - inImage->imageHeight) >> 1), inImage);
}

void imageDrawAlpha(int inDrawX, int inDrawY, Image* inImage) {
     if (!graphicsInitialized)
          return;

     sceKernelDcacheWritebackInvalidateAll();
     sceGuStart(GU_DIRECT, list);
     sceGuTexImage(0, inImage->imageTextureWidth, inImage->imageTextureHeight, inImage->imageTextureWidth, (void*)inImage->imageData);
     float u = 1.0f / ((float)inImage->imageTextureWidth);
     float v = 1.0f / ((float)inImage->imageTextureHeight);
     sceGuTexScale(u, v);
	
     int j = 0;
     while(j < inImage->imageWidth) {
          Vertex* tempVertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
          int tempSliceWidth = 64;
          if(j + tempSliceWidth > inImage->imageWidth)
               tempSliceWidth = inImage->imageWidth - j;
          tempVertices[0].u = j;
          tempVertices[0].v = 0;
          tempVertices[0].x = inDrawX + j;
          tempVertices[0].y = inDrawY;
          tempVertices[0].z = 0;
          tempVertices[1].u = j + tempSliceWidth;
          tempVertices[1].v = inImage->imageHeight;
          tempVertices[1].x = inDrawX + j + tempSliceWidth;
          tempVertices[1].y = inDrawY + inImage->imageHeight;
          tempVertices[1].z = 0;
          sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, tempVertices);
		j += tempSliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

void imageDrawSection(int inStartX, int inStartY, int inWidth, int inHeight, int inDrawX, int inDrawY, Image* inImage) {
     if (!graphicsInitialized)
          return;

     Color* tempVRam = graphicsVRamDrawBuffer();
     sceKernelDcacheWritebackInvalidateAll();
     sceGuStart(GU_DIRECT, list);
     sceGuCopyImage(GU_PSM_8888, inStartX, inStartY, inWidth, inHeight, inImage->imageTextureWidth, inImage->imageData, inDrawX, inDrawY, PSP_LINE_SIZE, tempVRam);
     sceGuFinish();
     sceGuSync(0,0);
}

void imageDrawSectionAlpha(int inStartX, int inStartY, int inWidth, int inHeight, int inDrawX, int inDrawY, Image* inImage) {
     if (!graphicsInitialized)
          return;

     sceKernelDcacheWritebackInvalidateAll();
     sceGuStart(GU_DIRECT, list);
     sceGuTexImage(0, inImage->imageTextureWidth, inImage->imageTextureHeight, inImage->imageTextureWidth, (void*)inImage->imageData);
     float u = 1.0f / ((float)inImage->imageTextureWidth);
     float v = 1.0f / ((float)inImage->imageTextureHeight);
     sceGuTexScale(u, v);
	
     int j = 0;
     while(j < inWidth) {
          Vertex* tempVertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
          int tempSliceWidth = 64;
          if(j + tempSliceWidth > inWidth)
               tempSliceWidth = inWidth - j;
          tempVertices[0].u = inStartX + j;
          tempVertices[0].v = inStartY;
          tempVertices[0].x = inDrawX + j;
          tempVertices[0].y = inDrawY;
          tempVertices[0].z = 0;
          tempVertices[1].u = inStartX + j + tempSliceWidth;
          tempVertices[1].v = inStartY + inHeight;
          tempVertices[1].x = inDrawX + j + tempSliceWidth;
          tempVertices[1].y = inDrawY + inHeight;
          tempVertices[1].z = 0;
          sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, tempVertices);
		j += tempSliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

void imageDrawAdvanced(int inSrcX, int inSrcY, int inSrcWidth, int inSrcHeight, int inDestX, int inDestY, int inDestWidth, int inDestHeight, Image* inImage) {
     if (!graphicsInitialized)
          return;

     sceKernelDcacheWritebackInvalidateAll();
     sceGuStart(GU_DIRECT, list);
     sceGuTexImage(0, inImage->imageTextureWidth, inImage->imageTextureHeight, inImage->imageTextureWidth, (void*)inImage->imageData);
     float u = 1.0f / ((float)inImage->imageTextureWidth);
     float v = 1.0f / ((float)inImage->imageTextureHeight);
     sceGuTexScale(u, v);

     float start, end;
     float cur_u = inSrcX;
     float cur_x = inDestX;
     float x_end = inDestX + inDestWidth;
     float slice = 64.f;
     float ustep = (float)(inSrcWidth-inSrcX)/(float)inDestWidth * slice;

     // blit maximizing the use of the texture-cache
     for(start = 0, end = inDestWidth; start < end; start += slice ) {
          ResizeVertex* vertices = (ResizeVertex*)sceGuGetMemory(2 * sizeof(ResizeVertex));

          float poly_width = ((cur_x + slice) > x_end) ? (x_end - cur_x) : slice;
          float source_width = ((cur_u + ustep) > inSrcWidth) ? (inSrcWidth - cur_u) : ustep;

          vertices[0].u = cur_u;
          vertices[0].v = inSrcY;
          vertices[0].x = cur_x; 
          vertices[0].y = inDestY; 
          vertices[0].z = 0;

          cur_u += source_width;
          cur_x += poly_width;

          vertices[1].u = cur_u;
          vertices[1].v = inSrcHeight;
          vertices[1].x = cur_x;
          vertices[1].y = (inDestY + inDestHeight);
          vertices[1].z = 0;

          sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
     }
     
     sceGuFinish();
     sceGuSync(0, 0);
}

void imageDrawImage(int inStartX, int inStartY, int inWidth, int inHeight, int inDestX, int inDestY, Image* inSource, Image* inDestination) {
     Color* tempDestinationData = &inDestination->imageData[(inDestination->imageTextureWidth * inDestY) + inDestX];
     int tempDestinationSkipX = inDestination->imageTextureWidth - inWidth;
     Color* tempSourceData = &inSource->imageData[inSource->imageTextureWidth * inStartY + inStartX];
     int tempSourceSkipX = inSource->imageTextureWidth - inWidth;
     int x, y;
     for(y = 0; y < inHeight; y++, tempDestinationData += tempDestinationSkipX, tempSourceData += tempSourceSkipX) {
          for(x = 0; x < inWidth; x++, tempDestinationData++, tempSourceData++) {
               *tempDestinationData = *tempSourceData;
          }
     }
}

void imageDrawImageAlpha(int inStartX, int inStartY, int inWidth, int inHeight, int inDestX, int inDestY, Image* inSource, Image* inDestination) {
     Color* tempDestinationData = &inDestination->imageData[(inDestination->imageTextureWidth * inDestY) + inDestX];
     int tempDestinationSkipX = inDestination->imageTextureWidth - inWidth;
     Color* tempSourceData = &inSource->imageData[inSource->imageTextureWidth * inStartY + inStartX];
     int tempSourceSkipX = inSource->imageTextureWidth - inWidth;
     int x, y;
     for(y = 0; y < inHeight; y++, tempDestinationData += tempDestinationSkipX, tempSourceData += tempSourceSkipX) {
          for(x = 0; x < inWidth; x++, tempDestinationData++, tempSourceData++) {
               *tempDestinationData = colorBlend(*tempSourceData, *tempDestinationData);
          }
     }
}

void imagePrintText(int inX, int inY, const char* inText, Color inColor, Image* inImage) {
     int c, i, j, l;
     u8 *tempFont;
     Color *tempDataPtr;
     Color *tempData;
	
     for(c = 0; c < strlen(inText); c++) {
          if((inX < 0) || ((inX + 8) > inImage->imageWidth) || (inY < 0) || ((inY + 8) > inImage->imageHeight))
               break;
          char tempChar = inText[c];
          tempData = inImage->imageData + inX + (inY * inImage->imageTextureWidth);
		
          tempFont = &msx[(int)tempChar * 8];
          for(i = l = 0; i < 8; i++, l += 8, tempFont++) {
               tempDataPtr  = tempData;
               for(j = 0; j < 8; j++) {
                    if((*tempFont & (128 >> j)))
                         *tempDataPtr = inColor;
                    tempDataPtr++;
               }
               tempData += inImage->imageTextureWidth;
          }
          inX += 8;
     }
}

void imageReplaceColor(Color inReplace, Color inColor, Image* inImage) {
     int i, j;
     for(j = 0; j < inImage->imageHeight; j++) {
          for(i = 0; i < inImage->imageWidth; i++) {
               if((inImage->imageData[(j * inImage->imageTextureWidth) + i] & 0x00FFFFFF) == (inReplace & 0x00FFFFFF))
                    inImage->imageData[(j * inImage->imageTextureWidth) + i] = inColor;
          }
     }
}

void imageReplaceColorIgnoreAlpha(Color inReplace, Color inColor, Image* inImage) {
     int i, j;
     for(j = 0; j < inImage->imageHeight; j++) {
          for(i = 0; i < inImage->imageWidth; i++) {
               if((inImage->imageData[(j * inImage->imageTextureWidth) + i] & 0x00FFFFFF) == (inReplace & 0x00FFFFFF)) {
                    if((inImage->imageData[(j * inImage->imageTextureWidth) + i] & 0xFF000000) != 0x00)
                         inImage->imageData[(j * inImage->imageTextureWidth) + i] = inColor;
               }
          }
     }
}


void screenClear(Color inColor) {
     if (!graphicsInitialized)
          return;
     sceGuStart(GU_DIRECT, list);
     sceGuClearDepth(0);
     sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
     sceGuFinish();
     sceGuSync(0, 0);
     if((inColor & 0x00FFFFFF) != 0)
          screenDrawFilledRect(0, 0, SCR_WIDTH, SCR_HEIGHT, inColor);
}

Color screenGetPixel(int inX, int inY) {
     if(!graphicsInitialized)
          return 0x00000000;
     Color* tempVRam = graphicsVRamDrawBuffer();
     return tempVRam[inX + (inY * PSP_LINE_SIZE)];
}

void screenSetPixel(int inX, int inY, Color inColor) {
     if(!graphicsInitialized)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     tempVRam[inX + (inY * PSP_LINE_SIZE)] = inColor;
}

void screenDrawLine(int inX0, int inY0, int inX1, int inY1, Color inColor) {
     if(!graphicsInitialized)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     drawLine(inX0, inY0, inX1, inY1, inColor, tempVRam, PSP_LINE_SIZE);
}

void screenDrawInvRect(int inX, int inY, int inWidth, int inHeight) {
     if(!graphicsInitialized)
          return;
     if((inX < 0) || (inY < 0) || ((inX + inWidth) > SCREEN_WIDTH) || ((inY + inHeight) > SCREEN_HEIGHT))
          return;
     if((inHeight * inWidth) <= 0)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     int i, j;
     for(j = inY; j < (inY + inHeight); j++) {
          for(i = inX; i < (inX + inWidth); i++) {
               tempVRam[(j * PSP_LINE_SIZE) + i] = colorInvert(tempVRam[(j * PSP_LINE_SIZE) + i]);
          }
     }
}

void screenDrawHashRect(int inX, int inY, int inWidth, int inHeight, Color inColor) {
     if(!graphicsInitialized)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     int tempSkipX = PSP_LINE_SIZE - inWidth;
     int x, y;
     Color* tempData = tempVRam + inX + (inY * PSP_LINE_SIZE);
     for(y = 0; y < inHeight; y++, tempData += tempSkipX) {
          for(x = 0; x < inWidth; x++, tempData++)
               if((y & 1) == (x & 1))
                    *tempData = inColor;
     }
}

void screenDrawRect(int inX, int inY, int inWidth, int inHeight, Color inColor) {
     if(!graphicsInitialized)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     int tempX = inX + inWidth;
     int tempY = inY + inHeight;
     int i;
     for(i = inX; i < tempX; i++) {
          tempVRam[(inY * PSP_LINE_SIZE) + i] = inColor;
          tempVRam[((tempY - 1) * PSP_LINE_SIZE) + i] = inColor;
     }
     for(i = inY; i < tempY; i++) {
          tempVRam[(i * PSP_LINE_SIZE) + inX] = inColor;
          tempVRam[(i * PSP_LINE_SIZE) + (tempX - 1)] = inColor;
     }
}

void screenDrawFilledRect(int inX, int inY, int inWidth, int inHeight, Color inColor) {
     if(!graphicsInitialized)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     int tempSkipX = PSP_LINE_SIZE - inWidth;
     int x, y;
     Color* tempData = tempVRam + inX + (inY * PSP_LINE_SIZE);
     for(y = 0; y < inHeight; y++, tempData += tempSkipX) {
          for(x = 0; x < inWidth; x++, tempData++)
               *tempData = inColor;
     }
}

void screenPrintText(int inX, int inY, const char* inText, Color inColor) {
     if(!graphicsInitialized)
          return;
     int c, i, j, l;
     u8 *tempFont;
     Color *tempDataPtr;
     Color *tempData;
	
     for(c = 0; c < strlen(inText); c++) {
          if((inX < 0) || ((inX + 8) > SCR_WIDTH) || (inY < 0) || ((inY + 8) > SCR_HEIGHT))
               break;
          char tempChar = inText[c];
          tempData = graphicsVRamDrawBuffer() + inX + (inY * PSP_LINE_SIZE);
		
          tempFont = &msx[(int)tempChar * 8];
          for(i = l = 0; i < 8; i++, l += 8, tempFont++) {
               tempDataPtr  = tempData;
               for(j = 0; j < 8; j++) {
                    if((*tempFont & (128 >> j)))
                         *tempDataPtr = inColor;
                    tempDataPtr++;
               }
               tempData += PSP_LINE_SIZE;
          }
          inX += 8;
     }
}

void screenEffectWaves(int inHeight, int inStart) {
     if(inHeight <= 0)
          inHeight = 8;
     while(inStart < 0)
          inStart += (inHeight << 2);
     while(inStart >= (inHeight << 2))
          inStart -= (inHeight << 2);
     Color* tempVRam = graphicsVRamDrawBuffer();
     Color* tempLine = (Color*) malloc((SCR_WIDTH - 1) * sizeof(Color));
     int i; int j; int k = 1;
     if(inStart > (inHeight << 1)) {
          k = -1;
          inStart -= (inHeight << 1);
     }
     for(i = inStart; i < SCR_HEIGHT; i += (inHeight << 1)) {
          for(j = i; (j < (i + inHeight)) && (j < SCR_HEIGHT); j++) {
               if(k == 1) {
                    memcpy(&tempVRam[j * PSP_LINE_SIZE], &tempVRam[(j * PSP_LINE_SIZE) + 1], ((SCR_WIDTH - 1) * sizeof(Color)));
               } else {
                    //memcpy(&vram[(j * PSP_LINE_SIZE) + 1], &tempVRam[j * PSP_LINE_SIZE], ((SCR_WIDTH - 1) * sizeof(Color)));
                    memcpy(tempLine, &tempVRam[j * PSP_LINE_SIZE], ((SCR_WIDTH - 1) * sizeof(Color)));
                    memcpy(&tempVRam[(j * PSP_LINE_SIZE) + 1], tempLine, ((SCR_WIDTH - 1) * sizeof(Color)));
               }
          }
          k = 0 - k;
     }
     free(tempLine);
}

void screenEffectFade(unsigned int inFade) {
     if(inFade == 0xFF) {
          screenClear(0xFF000000);
          return;
     }
     
     Color* tempVRam = graphicsVRamDrawBuffer();
     int i, j;
     for(j = 0; j < 272; j++) {
          for(i = 0; i < 480; i++) {
	           tempVRam[(j * PSP_LINE_SIZE) + i] = colorFade(tempVRam[(j * PSP_LINE_SIZE) + i], inFade);
          }
     }
}

void screenHShift(int inShift) {
     if(inShift == 0)
          return;
     Color* tempVRam = graphicsVRamDrawBuffer();
     Color* tempLine = (Color*) malloc((SCR_WIDTH - MAX(inShift, -inShift)) * sizeof(Color));
     int j = 0;
     if(inShift > 0) {
          for(j = 0; j < SCR_HEIGHT; j++) {
               memcpy(tempLine, &tempVRam[j * PSP_LINE_SIZE], ((SCR_WIDTH - inShift) * sizeof(Color)));
               memcpy(&tempVRam[(j * PSP_LINE_SIZE) + inShift], tempLine, ((SCR_WIDTH - inShift) * sizeof(Color)));
          }
     } else {
          for(j = 0; j < SCR_HEIGHT; j++) {
               memcpy(tempLine, &tempVRam[(j * PSP_LINE_SIZE) - inShift], ((SCR_WIDTH + inShift) * sizeof(Color)));
               memcpy(&tempVRam[j * PSP_LINE_SIZE], tempLine, ((SCR_WIDTH + inShift) * sizeof(Color)));
          }
     }
     free(tempLine);
}

Color colorMerge(Color inColorA, Color inColorB) {
     if(inColorA == inColorB)
          return inColorA;
     Color tempColor = ((inColorA & 0xFF000000) >> 2);
     tempColor += ((inColorB & 0xFF000000) >> 2);
     tempColor = (tempColor >> 1) & 0xFF000000;
     Color tempOut = (tempColor << 2);
     tempColor = (inColorA & 0x00FF0000);
     tempColor += (inColorB & 0x00FF0000);
     tempColor = (tempColor >> 1) & 0x00FF0000;
     tempOut += tempColor;
     tempColor = (inColorA & 0x0000FF00);
     tempColor += (inColorB & 0x0000FF00);
     tempColor = (tempColor >> 1) & 0x0000FF00;
     tempOut += tempColor;
     tempColor = (inColorA & 0x000000FF);
     tempColor += (inColorB & 0x000000FF);
     tempColor = (tempColor >> 1) & 0x000000FF;
     tempOut += tempColor;
     return tempOut;
}

Color colorBlend(Color inColorSrc, Color inColorDest) {
     unsigned char tempAlpha = (inColorSrc & 0xFF000000) >> 24;
     if(tempAlpha > 127)
          tempAlpha++;
     Color tempColor = ((((inColorSrc & 0x00FF0000) * tempAlpha) >> 8) + (((inColorDest & 0x00FF0000) * (255 - tempAlpha)) >> 8)) & 0x00FF0000;
     tempColor += ((((inColorSrc & 0x0000FF00) * tempAlpha) >> 8) + (((inColorDest & 0x0000FF00) * (255 - tempAlpha)) >> 8)) & 0x0000FF00;
     tempColor += ((((inColorSrc & 0x000000FF) * tempAlpha) >> 8) + (((inColorDest & 0x000000FF) * (255 - tempAlpha)) >> 8)) & 0x000000FF;
     return tempColor;
}

Color colorFade(Color inColor, unsigned int inFade) {
     Color tempColor = ((((inColor & 0x00FF0000) * inFade) >> 8) & 0x00FF0000);
     tempColor += ((((inColor & 0x0000FF00) * inFade) >> 8) & 0x0000FF00);
     tempColor += ((((inColor & 0x000000FF) * inFade) >> 8) & 0x000000FF);
     return tempColor;
}

Color colorInvert(Color inColor) {
     inColor = (0xFF000000 & inColor) + (0x00FFFFFF & (~inColor));
     return inColor;
}

ImageStack_Entry* imageStackAdd(char* inPath, Image* inImage) {
     if((inPath == NULL) || (inImage == NULL))
          return NULL;

     ImageStack_Entry* tempEntry = imageStackFindByPath(inPath);
     if(tempEntry != NULL) {
          if(tempEntry->imageStackImage != inImage) {
               if(inImage->imageData != NULL)
                    free(inImage->imageData);
               free(inImage);
          }
          return tempEntry;
     }

     tempEntry = imageStackFindByImage(inImage);
     if(tempEntry != NULL)
          return tempEntry;

     tempEntry = (ImageStack_Entry*) malloc(sizeof(ImageStack_Entry));
     tempEntry->imageStackPath = (char*) malloc((strlen(inPath) + 1) * sizeof(char));
     tempEntry->imageStackImage = inImage;
     tempEntry->imageStackNext = imageStack;
     imageStack = tempEntry;
     return tempEntry;
}

ImageStack_Entry* imageStackFindByPath(char* inPath) {
     ImageStack_Entry* tempPtr = imageStack;
     while(tempPtr != NULL) {
          if(memcmp(tempPtr->imageStackPath, inPath, strlen(inPath)) == 0)
               return tempPtr;
          tempPtr = (ImageStack_Entry*) tempPtr->imageStackNext;
     }
     return NULL;
}

ImageStack_Entry* imageStackFindByImage(Image* inImage) {
     ImageStack_Entry* tempPtr = imageStack;
     while(tempPtr != NULL) {
          if(tempPtr->imageStackImage == inImage)
               return tempPtr;
          tempPtr = (ImageStack_Entry*) tempPtr->imageStackNext;
     }
     return NULL;
}

void imageStackRemove(Image* inImage) {
     ImageStack_Entry* tempDelPtr = imageStackFindByImage(inImage);
     if(tempDelPtr == NULL)
          return;
     if(tempDelPtr == imageStack) {
          imageStack = (ImageStack_Entry*) tempDelPtr->imageStackNext;
          if(tempDelPtr->imageStackImage != NULL) {
               if(tempDelPtr->imageStackImage->imageData != NULL)
                    free(tempDelPtr->imageStackImage->imageData);
               free(tempDelPtr->imageStackImage);
          }
          free(tempDelPtr);
          return;
     }
     ImageStack_Entry* tempPtr = imageStack;
     while(tempPtr->imageStackNext != NULL) {
          if((ImageStack_Entry*)tempPtr->imageStackNext == tempDelPtr) {
               tempPtr->imageStackNext = tempDelPtr->imageStackNext;
               if(tempDelPtr->imageStackImage != NULL) {
                    if(tempDelPtr->imageStackImage->imageData != NULL)
                         free(tempDelPtr->imageStackImage->imageData);
                    free(tempDelPtr->imageStackImage);
               }
               free(tempDelPtr);
               return;
          }
          tempPtr = (ImageStack_Entry*)tempPtr->imageStackNext;
     }
}

void imageStackFreeEntry(ImageStack_Entry* inEntry) {
     if(inEntry->imageStackNext != NULL)
          imageStackFreeEntry((ImageStack_Entry*) inEntry->imageStackNext);
     if(inEntry->imageStackImage != NULL) {
          if(inEntry->imageStackImage->imageData != NULL)
               free(inEntry->imageStackImage->imageData);
          free(inEntry->imageStackImage);
     }
     free(inEntry);
}

void imageStackFree() {
     imageStackFreeEntry(imageStack);
}
