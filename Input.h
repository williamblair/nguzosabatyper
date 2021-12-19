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
    inline bool Confirm() { return mConfirm; }

private:
    bool mQuit;
    bool mConfirm;
};

#endif // INPUT_H_INCLUDED
