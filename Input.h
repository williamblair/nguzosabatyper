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
    inline char CharEntered() { return mCharEntered; }

private:
    bool mQuit;
    bool mConfirm;
    char mCharEntered;
};

#endif // INPUT_H_INCLUDED
