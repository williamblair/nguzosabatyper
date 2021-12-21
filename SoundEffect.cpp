#include <SoundEffect.h>

SoundEffect::SoundEffect() :
    mChunk(nullptr)
{
}

SoundEffect::~SoundEffect()
{
}

void SoundEffect::Init(const char* fileName)
{
    mChunk = Mix_LoadWAV(fileName);
    if (!mChunk) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "failed to load sound effect: " << Mix_GetError()
            << std::endl;
        throw std::runtime_error(Mix_GetError());
    }
}

void SoundEffect::Play(int channel)
{
    if (Mix_PlayChannel(channel, mChunk, 0) == -1) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "failed to play sound effect: " << Mix_GetError()
            << std::endl;
        //throw std::runtime_error(Mix_GetError());
    }
}
