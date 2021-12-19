#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

class Input
{
public:
    Input();
    ~Input();

    void Update();

    inline bool Quit() { return mQuit; }

private:
    bool mQuit;
};

#endif // INPUT_H_INCLUDED
