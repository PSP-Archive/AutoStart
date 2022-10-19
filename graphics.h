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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <psptypes.h>

extern u32* g_vram_base;

#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define COLOR_RED        0xFF0000FF
#define COLOR_ORANGE     0xFF007FFF
#define COLOR_YELLOW     0xFF00FFFF
#define COLOR_GREEN      0xFF00FF00
#define COLOR_TURQOISE   0xFFFFFF00
#define COLOR_BLUE       0xFFFF0000
#define COLOR_SKYBLUE    0xFFFF7F00
#define COLOR_PURPLE     0xFFFF00FF
#define COLOR_WHITE      0xFFFFFFFF
#define COLOR_BLACK      0xFF000000


typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))

typedef struct {
	int imageTextureWidth;
	int imageTextureHeight;
	int imageWidth;
	int imageHeight;
	Color* imageData;
} Image;

typedef struct {
	char* imageStackPath;
	Image* imageStackImage;
	void* imageStackNext;
} ImageStack_Entry;

typedef struct {
     int viewportX, viewportY;
     int viewportWidth, viewportHeight;
} Viewport;

// Initialize graphics
extern void   graphicsInit();
// Disable graphics
extern void   graphicsDisable();
// Get the pointer to the beginning of the VRam draw buffer
extern Color* graphicsVRamDrawBuffer();
// Get the pointer to the beginning of the VRam display buffer
extern Color* graphicsVRamDisplayBuffer();
// Swap the buffers (i.e update the screen)
extern void   graphicsSwapBuffers();
// Copy the display buffer into the draw buffer
extern void   graphicsCopyBuffers();

// Create a blank image
extern Image* imageCreate(int inWidth, int inHeight);
// Get an image of the screen
extern Image* imageFromScreen();
// Get an image of the draw buffer
extern Image* imageFromBuffer();
// Get an quarter size version of an image
extern Image* imageThumbnail(Image* inImage);
// Load a png image
extern Image* imageLoadPNG(char* inPath);
// Save a png image (unoptimized)
extern void   imageSavePNG(const char* inPath, Image* inImage, int inSaveAlpha);
// Free an image
extern void   imageFree(Image* inImage);
// Clear an image so that all pixels are one color
extern void   imageClear(Color inColor, Image* inImage);
// Get the color of a specific pixel of an image
extern Color  imageGetPixel(int inX, int inY, Image* inImage);
// Set the color of a specific pixel of an image
extern void   imageSetPixel(int inX, int inY, Color inColor, Image* inImage);
// Draw a line onto an image
extern void   imageDrawLine(int inX0, int inY0, int inX1, int inY1, Color inColor, Image* inImage);
// Draw a rectangle onto an image
extern void   imageDrawRect(int inX, int inY, int inWidth, int inHeight, Color inColor, Image* inImage);
// Draw a filled rectangle onto an image
extern void   imageDrawFilledRect(int inX, int inY, int inWidth, int inHeight, Color inColor, Image* inImage);
// Draw an image onto the screen ignoring alpha
extern void   imageDraw(int inDrawX, int inDrawY, Image* inImage);
// Draw an image onto the center of the screen ignoring alpha
extern void   imageDrawCenter(Image* inImage);
// Blit an image onto the screen taking alpha into account
extern void   imageDrawAlpha(int inDrawX, int inDrawY, Image* inImage);
// Draw a segment of an image onto the screen ignoring alpha
extern void   imageDrawSection(int inStartX, int inStartY, int inWidth, int inHeight, int inDrawX, int inDrawY, Image* inImage);
// Blit a segment of an image onto the screen taking alpha into account
extern void   imageDrawSectionAlpha(int inStartX, int inStartY, int inWidth, int inHeight, int inDrawX, int inDrawY, Image* inImage);
// Blit a rectangle of an image onto a rectangle of the screen (rectangles can be different dimensions (i.e stretch/shrink)).
extern void   imageDrawAdvanced(int inSrcX, int inSrcY, int inSrcWidth, int InSrcHeight, int inDestX, int inDestY, int inDestWidth, int InDestHeight, Image* inImage);
// Draw a segment of an image onto another image ignoring alpha
extern void   imageDrawImageSection(int inStartX, int inStartY, int inWidth, int inHeight, int inDestX, int inDestY, Image* inSource, Image* inDestination);
// Blit a segment of an image onto another image taking alpha into account
extern void   imageDrawImageSectionAlpha(int inStartX, int inStartY, int inWidth, int inHeight, int inDestX, int inDestY, Image* inSource, Image* inDestination);
// Print text onto an image
extern void   imagePrintText(int inX, int inY, const char* inText, Color inColor, Image* inImage);
// Replace a color in an image (can be used to turn a specific color into alpha)
extern void   imageReplaceColor(Color inReplace, Color inColor, Image* inImage);
// Replace a color in an image, but do not replace a color that is not visible
extern void   imageReplaceColorIgnoreAlpha(Color inReplace, Color inColor, Image* inImage);


// Clear the screen to be a specific color
extern void  screenClear(Color inColor);
// Get a pixel from the screen
extern Color screenGetPixel(int inX, int inY);
// Set a screen pixel
extern void  screenSetPixel(int inX, int inY, Color inColor);
// Draw a line onto the screen
extern void  screenDrawLine(int inX0, int inY0, int inX1, int inY1, Color inColor);
// Draw an inverted rectangle area onto the screen
extern void  screenDrawInvRect(int inX, int inY, int inWidth, int inHeight);
// Draw a filled rectangle onto the screen only drawing alternate pixels
extern void screenDrawHashRect(int inX, int inY, int inWidth, int inHeight, Color inColor);
// Draw a rectangle onto the screen
extern void  screenDrawRect(int inX, int inY, int inWidth, int inHeight, Color inColor);
// Draw a filled rectangle onto the screen
extern void  screenDrawFilledRect(int inX, int inY, int inWidth, int inHeight, Color inColor);
// Print text onto the screen
extern void  screenPrintText(int inX, int inY, const char* inText, Color inColor);
// A wavy screen effect (start has to be incremented every time it is called for it to look good, inHeight should be about 8)
extern void  screenEffectWaves(int inHeight, int inStart);
// Fade the screen, although it takes an int, values should be between (0-255)
extern void  screenEffectFade(unsigned int inFade);
// Move the screen buffer inShift number of pixels horizontally
extern void screenHShift(int inShift);

// Merge to colors 1:1
extern Color colorMerge(Color inColorA, Color inColorB);
// Blend two colors taking their alpha channel into account
extern Color colorBlend(Color inColorSrc, Color inColorDest);
// Fade a color (inFade should be (0-255))
extern Color colorFade(Color inColor, unsigned int inFade);
// Invert a color
extern Color colorInvert(Color inColor);

// These control the image stack, which is a useful tool making sure images are never duplicated
// and also allowing for all images to be cleared without keeping a pointer to them, don't use these
// unless you understand what they do.
// Add an image onto the image stack (called on every load)
extern ImageStack_Entry* imageStackAdd(char* inPath, Image* inImage);
// Find a stack entry using it's path as a key
extern ImageStack_Entry* imageStackFindByPath(char* inPath);
// Find a stack entry using it's pointer as a key
extern ImageStack_Entry* imageStackFindByImage(Image* inImage);
// Remove an image from the stack
extern void              imageStackRemove(Image* inImage);
// Free the whole image stack (should be called when your program terminates)
extern void              imageStackFree();

#ifdef __cplusplus
}
#endif

#endif
