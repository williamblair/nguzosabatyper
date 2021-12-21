#ifndef SOUND_EFFECT_H_INCLUDED
#define SOUND_EFFECT_H_INCLUDED

#include <iostream>
#include <SDL2/SDL_mixer.h>

class SoundEffect
{
public:
    SoundEffect();
    ~SoundEffect();
    
    void Init(const char* fileName);
    
    void Play(int channel = -1);

private:
    Mix_Chunk* mChunk;
};

#endif // SOUND_EFFECT_H_INCLUDED
