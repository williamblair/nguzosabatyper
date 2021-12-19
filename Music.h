#ifndef MUSIC_H_INCLUDED
#define MUSIC_H_INCLUDED

#include <iostream>
#include <SDL2/SDL_mixer.h>

class Music
{
public:
    Music();
    ~Music();
    
    void Init(const char* fileName);
    
    void Play(const bool loop);
    
private:
    Mix_Music* mMusic;
};

#endif // MUSIC_H_INCLUDED
