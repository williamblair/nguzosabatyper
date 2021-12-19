#include <Music.h>

Music::Music() :
    mMusic(nullptr)
{
}

Music::~Music()
{
    if (mMusic != nullptr) {
        Mix_FreeMusic(mMusic);
    }
}

void Music::Init(const char* fileName)
{
    mMusic = Mix_LoadMUS(fileName);
    if (!mMusic) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "Failed to load music: " << Mix_GetError() << std::endl;
        throw std::runtime_error(Mix_GetError());
    }
}

void Music::Play(const bool loop)
{
    int loopVal = loop ? -1 : 1;
    if (Mix_PlayMusic(mMusic, loopVal) == -1) {
        std::cerr << "Music play failed: " << Mix_GetError()
            << std::endl;
    }
}
