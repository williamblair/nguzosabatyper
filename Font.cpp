#include <Font.h>
#include <cstring>

Font::Font() :
    mCharWidth(0),
    mCharHeight(0),
    mCharsPerRow(0)
{
}

Font::~Font()
{
}

void Font::Init(
    const char* imageFile,
    const int charWidth,
    const int charHeight,
    Renderer& render
)
{
    mCharWidth = charWidth;
    mCharHeight = charHeight;

    mTexture.Init(imageFile, render);
    mCharsPerRow = mTexture.GetWidth() / mCharWidth;
}

void Font::Draw( Renderer& render, int x, int y, const char* msg )
{
    const int msgLen = strlen(msg);
    for (int i = 0; i < msgLen; ++i) {
        char letter = msg[i];
        /* convert to uppercase if necessary */
        if ( letter >= 'a' && letter <= 'z' ) {
            letter += 'A' - 'a';
        }

        /* TODO - assumes starts at ! char; handle otherwise */
        int glyphIndex = letter - '!';
        int v = glyphIndex / mCharsPerRow;
        int u = glyphIndex % mCharsPerRow;
        v *= mCharHeight;
        u *= mCharWidth;
        if ( u < 0 || u + mCharWidth > mTexture.GetWidth() ||
             v < 0 || v + mCharWidth > mTexture.GetHeight() ) {
            std::cerr << "Invalid uv calc" << std::endl;
            return;
        }
        mTexture.SetUVWH(u, v, mCharWidth, mCharHeight); 
        mTexture.Draw(render, x, y);

        x += mCharWidth;
    }
}


