#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#include <iostream>
#include <Texture.h>
#include <Renderer.h>

class Font
{
public:

    Font();
    ~Font();

    /*
        @brief Load character image atlas and settings
        @param [imageFile] full path of the atlas image file
        @param [charWidth] width in px of a single character
        @param [charHeight] height in px of a single character
        @return true if the image was successfully loaded, false otherwise
    */
    void Init(
        const char* imageFile,
        const int charWidth,
        const int charHeight,
        Renderer& render
    );

    /*
        @brief draw text using this font
        @param [render] the renderer to draw with
        @param [x] the top left x to start drawing from
        @param [y] the top left y to start drawing from
        @param [msg] the letters to draw
    */
    void Draw(
        Renderer& render,
        int x,
        int y,
        const char* msg
    );

    int GetCharWidth() { return mCharWidth; }
    int GetCharHeight() { return mCharHeight; }

private:

    int mCharWidth;
    int mCharHeight;

    int mCharsPerRow;

    Texture mTexture;
};

#endif // FONT_H_INCLUDED

