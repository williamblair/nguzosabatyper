#include <string>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <cstring>
#include <cstdint>
#include <fstream>

#include <Renderer.h>
#include <Input.h>
#include <Texture.h>
#include <Music.h>
#include <SoundEffect.h>
#include <GameTimer.h>
#include <Font.h>
#include <cmath>

/* MinGW SDL compile error thing */
#ifdef __WIN32
#undef main
#endif

#define S_WIDTH 800
#define S_HEIGHT 600

static Renderer render;
static Input input;
static Texture texture;
static Texture kwanzaPatternBG;
static Texture symbolKeyTex;
static Texture greenClothTex;
static Texture kwanzaCandlesTex;
static Texture titleTex;
static Texture titleBackgroundTex;
static Texture selectionIconTex;
static Texture descriptionTex;
static Texture highScoreBGTex;
static Music music;
static Music titleMusic;
static SoundEffect badEntryEffect;
static SoundEffect goodEntryEffect;
static SoundEffect textEntryEffect;
static SoundEffect loseLifeEffect;
static SoundEffect menuSelectEffect;
static GameTimer timer;
static Font font;
static Font titleFont;

#define NUM_SYMBOLS 7
static Texture Symbols[NUM_SYMBOLS];
static std::unordered_map<std::string, Texture*> SymbolsMap;
static std::vector<std::string> SymbolStrings;

uint64_t playerScore = 0;
struct HighScore
{
    char name[4];
    uint8_t nameLen;
    uint64_t score;
};
static std::list<HighScore> highScores;

/* Reserved channel for playing repetitive sound effects
 * so they don't use up all of the remaining channels */
#define EFFECT_PLAY_CHANNEL 0

void LoadHighScores()
{
    std::ifstream saveData("assets/savedata.bin", std::ifstream::binary);
    if (!saveData.is_open()) {
        std::cerr << __FILE__ << ": " << __LINE__ 
            << ": failed to open save data for loading" << std::endl;
        return;
    }
    
    while (true)
    {
        HighScore score;
        saveData.read((char*)&score, sizeof(score));
        if (!saveData) {
            break;
        }
        std::cout << "Score Name: " << score.name[0]
            << score.name[1] << score.name[2] << std::endl;
        std::cout << "Score value: " << score.score << std::endl;
        highScores.push_back(score);
    }
}

void SaveHighScores()
{
    std::ofstream saveData("assets/savedata.bin", std::ofstream::binary);
    if (!saveData.is_open()) {
        std::cerr << __FILE__ << ": " << __LINE__
            << ": failed to open save data file" << std::endl;
        return;
    }
    for (auto s = highScores.begin(); s != highScores.end(); ++s)
    {
        saveData.write((char*)&(*s), sizeof(*s));
    }
}

void InitSymbols()
{
    Symbols[0].Init("assets/umoja.png", render);
    SymbolsMap["UMOJA"] = &Symbols[0];
    
    Symbols[1].Init("assets/kujichagulia.png", render);
    SymbolsMap["KUJICHAGULIA"] = &Symbols[1];
    
    Symbols[2].Init("assets/ujima.png", render);
    SymbolsMap["UJIMA"] = &Symbols[2];
    
    Symbols[3].Init("assets/ujamaa.png", render);
    SymbolsMap["UJAMAA"] = &Symbols[3];
    
    Symbols[4].Init("assets/nia.png", render);
    SymbolsMap["NIA"] = &Symbols[4];
    
    Symbols[5].Init("assets/kuumba.png", render);
    SymbolsMap["KUUMBA"] = &Symbols[5];
    
    Symbols[6].Init("assets/imani.png", render);
    SymbolsMap["IMANI"] = &Symbols[6];
    
    SymbolStrings = {
        "UMOJA",
        "KUJICHAGULIA",
        "UJIMA",
        "UJAMAA",
        "NIA",
        "KUUMBA",
        "IMANI"
    };
}

inline std::string GetRandomTargetString()
{
    size_t index = rand() % SymbolStrings.size();
    return SymbolStrings[index];
}

class LifeBar
{
public:
    LifeBar(int x, int y, int width, int height, int health) :
        mX(x),
        mY(y),
        mWidth(width),
        mHeight(height),
        mCurHealth(health),
        mTotalHealth(health)
    {}
    
    void Draw(Renderer& render)
    {
        int lifeWidth = (int)((float)mWidth * 
            (float)mCurHealth / (float)mTotalHealth);
        int remainWidth = mWidth - lifeWidth;

        /* Draw green current life */
        render.DrawRect(mX, mY, lifeWidth, mHeight, 0,255,0,255);
        if (mCurHealth != mTotalHealth) {
            /* Draw red missing life */
            render.DrawRect(
                mX+lifeWidth, mY,
                remainWidth, mHeight,
                255,0,0,255
            );
        }
    }
    
    inline int GetHealth() const { return mCurHealth; }
    
    /* Returns true if health is now at or below zero after updating */
    bool AddHealth(int health) { 
        mCurHealth += health;
        if (mCurHealth > mTotalHealth) {
            mCurHealth = mTotalHealth;
        }
        else if (mCurHealth <= 0) {
            return true;
        }
        return false;
    }
    
private:
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    int mCurHealth;
    int mTotalHealth;
};

struct SymbolPiece
{
    std::string name;
    Texture* symbolTex;
    float x;
    float y;
    uint64_t id;
};

void TypingGameLoop()
{
    static const int maxUserStrLen = 15;
    char curUserStr[maxUserStrLen+1];
    char userScoreStr[50];
    std::string scoreText = "SCORE:";
    std::string lifeText = "LIFE:";
    std::string inputText = "INPUT: ";
    int userStrIndex = 0;
    float pieceSpeed = 20.0f;
    float pieceAddTimer = 6.0f; // seconds until new piece added
    uint64_t pieceIdCtr = 0;
    std::list<SymbolPiece> pieces;
    std::queue<uint64_t> pieceRemoveQueue;
    size_t userLife = 100;
    size_t level = 1;
    const int lifeBarWidth = 100;
    const int lifeBarHeight = 10;
    const int startingHealth = 25;
    const int lifeBarX = greenClothTex.GetWidth()-lifeBarWidth-10;
    const int lifeBarY = S_HEIGHT - 50;
    bool blinkCursorOn = true;
    float blinkCursorTime = 0.5f;
    float blinkCursorCounter = 0.0f;
    
    playerScore = 0;
    memset(curUserStr, 0, sizeof(curUserStr));
    sprintf(userScoreStr, "%llu", playerScore);
    
    LifeBar lifeBar(
        lifeBarX, lifeBarY,
        lifeBarWidth, lifeBarHeight,
        startingHealth
    );
    
#define AddNewPiece()                                                       \
    {                                                                       \
        std::string str = GetRandomTargetString();                          \
        Texture* tex = SymbolsMap[str];                                     \
        pieces.push_back({                                                  \
            str,                                                            \
            SymbolsMap[str],                                                \
            (float)(rand() % (S_WIDTH - tex->GetWidth() - symbolKeyTex.GetWidth())), \
            (float)(-(tex->GetHeight())),                                   \
            pieceIdCtr++                                                    \
        });                                                                 \
    }

    /* Initial piece */
    AddNewPiece()

#define RemovePiece(inId)                                                 \
    {                                                                     \
        for (auto piece = pieces.begin(); piece != pieces.end(); ++piece) \
        {                                                                 \
            if (piece->id == inId) {                                      \
                std::cout << "Found piece with id, erasing" << std::endl; \
                pieces.erase(piece);                                      \
                break;                                                    \
            }                                                             \
        }                                                                 \
    }
    
    bool quit = false;
    while (!quit)
    {
        render.Clear();
        const float dt = timer.Update();
        blinkCursorCounter += dt;
        if (blinkCursorCounter > blinkCursorTime)
        {
            blinkCursorCounter = 0.0f;
            blinkCursorOn = !blinkCursorOn;
        }
        
        kwanzaPatternBG.Draw(render, 0,0);
        symbolKeyTex.Draw(render, S_WIDTH-symbolKeyTex.GetWidth(), 0);
        
        for (auto piece = pieces.begin(); piece != pieces.end(); ++piece)
        {
            piece->symbolTex->Draw(render, (int)piece->x, (int)piece->y);
            
            piece->y += pieceSpeed * dt;
            // TODO - figure out value besides 100
            if (piece->y + piece->symbolTex->GetHeight() >= S_HEIGHT - 100) {
                std::cout << "Add piece to remove queue w id: " << piece->id << std::endl;
                pieceRemoveQueue.push(piece->id);
                loseLifeEffect.Play();
                
                /* Returns true if health reached zero... */
                if (lifeBar.AddHealth(-piece->name.size())) {
                    std::cout << "Ran out of health; game over" << std::endl;
                    quit = true;
                }
            }
        }
        
        /* Cloth background for text */
        greenClothTex.Draw(render, 0, S_HEIGHT - greenClothTex.GetHeight());
        /* Current user text input */
        font.Draw(render, 10, S_HEIGHT - 20, inputText.c_str());
        if (userStrIndex > 0) {
            font.Draw(
                render, 
                10 + inputText.size()*font.GetCharWidth() + 10, 
                S_HEIGHT - 20, 
                curUserStr
            );
        }
        /* User text input cursor */
        if (blinkCursorOn) {
            render.DrawRect(
                10 + inputText.size()*font.GetCharWidth() + 10 + 
                    strlen(curUserStr)*font.GetCharWidth(),
                S_HEIGHT - 20,
                font.GetCharWidth(), font.GetCharHeight(),
                255,255,255,255
            );
        }
        /* User score */
        font.Draw(render, 10, S_HEIGHT - 50, scoreText.c_str());
        font.Draw(
            render,
            10 + (scoreText.size()+2)*font.GetCharWidth(),
            S_HEIGHT - 50,
            userScoreStr
        );
        /* Lifebar */
        font.Draw(
            render,
            lifeBarX - (lifeText.size()+1)*font.GetCharWidth(),
            lifeBarY,
            lifeText.c_str()
        );
        lifeBar.Draw(render);
        
        /* User enter press */
        if (input.Confirm()) {
            curUserStr[userStrIndex] = '\0';
            std::cout << "User str was: " << curUserStr << std::endl;
            bool foundMatch = false;
            for (auto piece = pieces.begin(); piece != pieces.end(); ++piece)
            {
                if (std::string(curUserStr) == piece->name) {
                    std::cout << "Input matches!" << std::endl;
                    playerScore += piece->name.size();
                    sprintf(userScoreStr, "%llu", playerScore);
                    std::cout << "New Score: " << playerScore << std::endl;
                    pieceRemoveQueue.push(piece->id);
                    foundMatch = true;
                    break;
                }
            }
            if (!foundMatch) {
                // TODO - flash screen or text or something
                badEntryEffect.Play();
            }
            else {
                // TODO - some other good indication
                goodEntryEffect.Play();
            }
            memset(curUserStr, 0, sizeof(curUserStr));
            userStrIndex = 0;
        }
        /* User character press */
        if ((int)input.CharEntered() != 0 &&
            userStrIndex < maxUserStrLen)
        {
            curUserStr[userStrIndex++] = input.CharEntered();
            textEntryEffect.Play(EFFECT_PLAY_CHANNEL);
        }
        /* User backspace press */
        if (input.BackSpace())
        {
            if (userStrIndex > 0) {
                userStrIndex--;
                memset(
                    &curUserStr[userStrIndex],
                    0,
                    sizeof(curUserStr)-userStrIndex
                );
            }
        }
        
        while (!pieceRemoveQueue.empty())
        {
            std::cout << "Removing piece from queue" << std::endl;
            uint64_t pieceId = pieceRemoveQueue.front();
            RemovePiece(pieceId);
            pieceRemoveQueue.pop();
        }
        
        pieceAddTimer -= dt;
        if (pieceAddTimer <= 0.0f) {
            AddNewPiece();
            std::cout << "Added new piece" << std::endl;
            pieceAddTimer = 6.0f;
        }
        
        quit |= input.Quit();
        input.Update();
        render.Update();
    }
}

void ViewHighScoresLoop()
{
    char scoreStr[50];
    
    int charWidth = font.GetCharWidth();
    int charHeight = font.GetCharHeight();
    
    bool quit = false;
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        render.Clear();
        
        highScoreBGTex.Draw(render, 0,0);
        
        int scoreY = 160;
        int scoreX = 170;
        for (size_t i=0; i<highScores.size(); ++i)
        {
            /* Draw a score entry */
            auto hsIter = highScores.begin();
            for (size_t j=0; j<i; ++j) {
                ++hsIter;
            }
            HighScore& hs = *hsIter;
            int nameLen = hs.nameLen;
            sprintf(scoreStr, "%llu", hs.score);
            for (size_t j=0; j<nameLen; ++j)
            {
                char curStr[2];
                curStr[0] = hs.name[j];
                curStr[1] = '\0';
                font.Draw(
                    render,
                    scoreX + charWidth*j + 5,
                    scoreY,
                    curStr
                );
            }
            font.Draw(
                render,
                550, scoreY,
                scoreStr
            );
            
            scoreY += font.GetCharHeight()+20;
        }
        
        if (input.Quit()) { quit = true; }
        if (input.Confirm()) { quit = true; }
        render.Update();
    }
}

void EnterHighScoreLoop(size_t scoreInsertIndex)
{
    /* Ignore name for now; user will enter */
    HighScore newScore;
    newScore.score = playerScore;
    {
        auto hsIter = highScores.begin();
        for (size_t i=0; i<scoreInsertIndex; ++i) {
            ++hsIter;
        }
        highScores.insert(hsIter, newScore);
    }
    
    /* Allow a maximum of 5 scores */
    if (highScores.size() > 5) {
        highScores.pop_back();
    }
    
    /* For cursor blinking */
    bool blinkCursorOn = true;
    float blinkCursorTime = 0.5f;
    float blinkCursorCounter = 0.0f;
    
    size_t playerNameIndex = 0;
    char playerName[4] = {'\0'};
    
    int charWidth = font.GetCharWidth();
    int charHeight = font.GetCharHeight();
    
    char scoreStr[50];
    
    bool quit = false;
    
    std::string message1 = "NEW HIGH SCORE!";
    std::string message2 = "ENTER YOUR NAME";
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        highScoreBGTex.Draw(render, 0,0);
        
        int msgX = (S_WIDTH/2)-(message1.size()*charWidth)/2;
        int msgY = 250;
        font.Draw(render, msgX, msgY, message1.c_str());
        msgX = (S_WIDTH/2)-(message2.size()*charWidth)/2;
        msgY += charHeight+20;
        font.Draw(render, msgX, msgY, message2.c_str());
        
        if (input.Confirm()) { 
            std::cout << "Input.confirm, breaking" << std::endl;
            break;
        }
        if (input.Quit()) { quit = true; }
        
        render.Update();
    }
    
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        blinkCursorCounter += dt;
        if (blinkCursorCounter > blinkCursorTime) {
            blinkCursorOn = !blinkCursorOn;
            blinkCursorCounter = 0.0f;
        }
        
        highScoreBGTex.Draw(render, 0,0);
        
        int scoreY = 160;
        int scoreX = 170;
        for (size_t i=0; i<highScores.size(); ++i)
        {
            /* Draw the player's score and current entered name */
            if (i == scoreInsertIndex)
            {
                for (size_t j=0; j<playerNameIndex; ++j)
                {
                    char curStr[2];
                    curStr[0] = playerName[j];
                    curStr[1] = '\0';
                    font.Draw(
                        render,
                        scoreX + charWidth*j + 5,
                        scoreY,
                        curStr
                    );
                    //std::cout << "cur str: " << curStr << std::endl;
                }
                /* Draw input cursor */
                if (blinkCursorOn) {
                    render.DrawRect(
                        scoreX + playerNameIndex*charWidth + 5,
                        scoreY,
                        charWidth,charHeight,
                        255,255,255,255
                    );
                }
                
                scoreY += font.GetCharHeight()+20;
                continue;
            }
            
            /* Otherwise draw a score entry */
            auto hsIter = highScores.begin();
            for (size_t j=0; j<i; ++j) {
                ++hsIter;
            }
            HighScore& hs = *hsIter;
            int nameLen = hs.nameLen;
            sprintf(scoreStr, "%llu", hs.score);
            for (size_t j=0; j<nameLen; ++j)
            {
                char curStr[2];
                curStr[0] = hs.name[j];
                curStr[1] = '\0';
                font.Draw(
                    render,
                    scoreX + charWidth*j + 5,
                    scoreY,
                    curStr
                );
            }
            font.Draw(
                render,
                550, scoreY,
                scoreStr
            );
            
            scoreY += font.GetCharHeight()+20;
        }
        
        /* User character press */
        if ((int)input.CharEntered() != 0 &&
            playerNameIndex < 3)
        {
            std::cout << "Adding char entered: " << input.CharEntered()
                << std::endl;
            playerName[playerNameIndex++] = input.CharEntered();
            textEntryEffect.Play(EFFECT_PLAY_CHANNEL);
        }
        /* User backspace press */
        if (input.BackSpace())
        {
            if (playerNameIndex > 0) {
                playerNameIndex--;
                memset(
                    &playerName[playerNameIndex],
                    0,
                    sizeof(playerName)-playerNameIndex
                );
            }
        }
        /* User tried to entered their name */
        if (input.Confirm())
        {
            /* Need at least one character for the name */
            if (playerNameIndex > 0) {
                auto hsIter = highScores.begin();
                for (size_t i=0; i<scoreInsertIndex; ++i) {
                    ++hsIter;
                }
                char* saveStr = hsIter->name;
                for (size_t i=0; i<playerNameIndex; ++i) {
                    saveStr[i] = playerName[i];
                }
                hsIter->nameLen = playerNameIndex;
                
                /* Save data to file */
                SaveHighScores();
                
                /* Done with entry loop */
                quit = true;
            }
            else {
                /* TODO - negative sound effect */
            }
        }
        
        quit |= input.Quit();
        
        render.Update();
    }
}

void GameOverLoop()
{
    /* Check if player score was a high score */
    size_t scoreInsertIndex=0;
    bool newHighScore = false;
    for (auto hs = highScores.begin(); hs != highScores.end(); ++hs)
    {
        /* Found high score */
        if (hs->score < playerScore) {
            newHighScore = true;
            break;
        }
        ++scoreInsertIndex;
    }
    /* If we have less than the maximum entries we can enter the
     * score regardless */
    if (!newHighScore && highScores.size() < 5 && playerScore > 0) {
        newHighScore = true;
        scoreInsertIndex = highScores.size();
    }
    if (newHighScore) {
        EnterHighScoreLoop(scoreInsertIndex);
    }
    
    /* Once new high score entered (or not), view list of 
     * high scores */
    ViewHighScoresLoop();
}

class TitleMenu
{
public:

    constexpr static int SELECT_DIRECTIONS = 0;
    constexpr static int SELECT_PLAY = 1;
    constexpr static int SELECT_ABOUT = 2;
    constexpr static int SELECT_HIGHSCORES = 3;
    constexpr static int SELECT_EXIT = 4;

    TitleMenu(int y, Font* font) :
        mY(y),
        mFont(font),
        mSelection(SELECT_DIRECTIONS)
    {
    }
    
    int Update()
    {
        if (input.Up()) {
            mSelection--;
            if (mSelection < 0) { mSelection = SELECT_EXIT; }
            menuSelectEffect.Play(EFFECT_PLAY_CHANNEL);
        }
        else if (input.Down()) {
            mSelection++;
            if (mSelection > SELECT_EXIT) { 
                mSelection = SELECT_DIRECTIONS;
            }
            menuSelectEffect.Play(EFFECT_PLAY_CHANNEL);
        }
        /* User selected a menu entry, return which selection */
        else if (input.Confirm()) {
            return mSelection;
        }
        /* No selection */
        return -1;
    }
    
    void Draw()
    {
        int curY = mY;
        static std::string menuTexts[SELECT_EXIT+1] = {
            "DIRECTIONS",
            "PLAY",
            "ABOUT",
            "HIGH SCORES",
            "EXIT"
        };
        for (int i = SELECT_DIRECTIONS; i <= SELECT_EXIT; ++i)
        {
            std::string& msg = menuTexts[i];
            int drawX = S_WIDTH/2 - (msg.size()*mFont->GetCharWidth())/2;
            mFont->Draw(render, drawX, curY, msg.c_str());
            
            /* Draw icon indicator to the left and right of the text */
            if (i == mSelection)
            {
                int iconX = drawX - 50 - selectionIconTex.GetWidth();
                int iconY = curY - (selectionIconTex.GetHeight()/2);
                selectionIconTex.Draw(render, iconX, iconY);
                iconX = drawX + (msg.size()*mFont->GetCharWidth()) + 50;
                selectionIconTex.Draw(render, iconX, iconY);
            }
            
            curY += mFont->GetCharHeight()+20;
        }
    }
    
private:
    int mY;
    Font* mFont;
    int mSelection;
};

/* Returns the menu selection */
int TitleScreenLoop()
{
    static const char* jamText = "PDUBS HOLIDAY JAM 2021";
    static const char* authorText = "BJ BLAIR";
    static const char* directionText =
        "USE KEYBOARD UP, DOWN TO NAVIGATE MENU. PRESS ENTER TO SELECT.";
    static const int jamTextX = 10;
    static const int jamTextY = 580;
    static const int authorTextX = 300;
    static const int authorTextY = 580;
    #define NUM_TITLE_RECTS 9
    static SDL_Rect titleClipRects[NUM_TITLE_RECTS];
    static SDL_Rect titleDrawRects[NUM_TITLE_RECTS];
    static float titleAnimTimes[NUM_TITLE_RECTS];
    bool quit = false;
    float directionTextX = S_WIDTH - 
        titleFont.GetCharWidth()*strlen(directionText);
    float directionTextSpeed = 40.0f;
    /* Center on screen */
    int candlesX = (S_WIDTH/2) - (kwanzaCandlesTex.GetWidth()/2);
    int candlesY = (S_HEIGHT/2) - (kwanzaCandlesTex.GetHeight()/2);
    
    TitleMenu menu(375, &titleFont);
    
    /* Init title text drawing positions */
    titleClipRects[0] = {0,0,64,88}; // N
    titleDrawRects[0] = {165,100,64,88};
    titleClipRects[1] = {64,0,48,88}; // g
    titleDrawRects[1] = {224,100,48,88};
    titleClipRects[2] = {112,0,50,88}; // u
    titleDrawRects[2] = {269,100,50,88};
    titleClipRects[3] = {163,0,40,88}; // z
    titleDrawRects[3] = {320,100,40,88};
    titleClipRects[4] = {204,0,48,88}; // o
    titleDrawRects[4] = {362,100,48,88};
    titleClipRects[5] = {274,0,48,88}; // S
    titleDrawRects[5] = {432,100,48,88};
    titleClipRects[6] = {323,0,46,88}; // a
    titleDrawRects[6] = {481,100,46,88};
    titleClipRects[7] = {368,0,48,88}; // b
    titleDrawRects[7] = {525,100,48,88};
    titleClipRects[8] = {417,0,48,88}; // a
    titleDrawRects[8] = {575,100,48,88};
    /* Init animation times */
    float animTime = 0.0f;
    const float titleAnimSpeed = 5.0f;
    for (int i=0; i<NUM_TITLE_RECTS; ++i)
    {
        titleAnimTimes[i] = animTime;
        animTime += 0.5f;
    }
    
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        render.Clear();
        
        /* Background */
        titleBackgroundTex.Draw(render, 0,0);
        
        /* Kwanza Candles */ 
        kwanzaCandlesTex.Draw(render, candlesX, candlesY);
        
        /* Title Text */
        for (int i=0; i<NUM_TITLE_RECTS; ++i)
        {
            SDL_Rect* clip = &titleClipRects[i];
            SDL_Rect* draw = &titleDrawRects[i];
            int drawY = draw->y + (sin(titleAnimTimes[i])*10.0f);
            titleAnimTimes[i] += dt * titleAnimSpeed;
            titleTex.SetUVWH(clip->x, clip->y, clip->w, clip->h);
            titleTex.Draw(render, draw->x, drawY);
        }
        
        /* Menu Text */
        menu.Draw();
        
        /* Top directions text */
        titleFont.Draw(render, (int)directionTextX, 5, directionText);
        
        /* Footnotes text */
        titleFont.Draw(render, jamTextX, jamTextY, jamText);
        titleFont.Draw(render, authorTextX, authorTextY, authorText);
        
        if (input.Quit()) { quit = true; }
        
        /* User selection to exit title screen */
        int menuSelect = menu.Update();
        if (menuSelect != -1) {
            std::cout << "Menu Select: " << menuSelect << std::endl;
            return menuSelect;
        }
        /* Scroll top directions */
        directionTextX -= directionTextSpeed * dt;
        if (directionTextX < 
            (-((int)titleFont.GetCharWidth()*(int)strlen(directionText))))
        {
            directionTextX = S_WIDTH;
        }
        
        render.Update();
    }
    
    /* No menu select, user quit */
    return -1;
}

void DirectionsLoop()
{
    std::vector<std::string> textLines = {
        "TYPE THE NAMES OF THE SYMBOLS AS THEY APPEAR ON SCREEN",
        "YOU CAN PRESS BACKSPACE TO DELETE CHARACTERS",
        "ONCE YOU ARE DONE TYPING, PRESS ENTER TO SUBMIT",
        "",
        "YOU WILL LOSE HP IF A SYMBOL REACHES THE GROUND",
        "ONCE YOUR HP REACHES ZERO, THE GAME IS OVER!",
        "",
        "PRESS ENTER TO RETURN TO THE TITLE"
    };
    int finalTextY = 150;
    int textX = 125;
    float textY = 400;
    float scrollSpeed = 20.0f;
    bool quit = false;
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        if ((int)textY > finalTextY) {
            textY -= scrollSpeed * dt;
        }
        
        render.Clear();
        
        titleBackgroundTex.Draw(render, 0,0);
        
        int curY = (int)textY;
        for (std::string& line : textLines)
        {
            if (line.size() > 0) {
                titleFont.Draw(render, textX, curY, line.c_str());
            }
            curY += titleFont.GetCharHeight()+5;
        }
        
        render.Update();
        
        if (input.Quit()) { quit = true; }
        if (input.Confirm()) { quit = true; }
    }
}

void AboutLoop()
{
    bool quit = false;
    int texWidth = descriptionTex.GetWidth();
    int totalTexHeight = descriptionTex.GetHeight();
    int texHeight = S_HEIGHT - 100;
    int texX = 25;
    int texY = 50;
    float texScrollY = 0;
    float scrollSpeed = 50.0f;
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        if (input.UpHeld() && texScrollY > 0) {
            texScrollY -= scrollSpeed * dt;
            if (texScrollY < 0.0f) { texScrollY = 0.0f; }
        }
        else if (input.DownHeld() &&
            texScrollY < totalTexHeight - texHeight)
        {
            texScrollY += scrollSpeed * dt;
            if (texScrollY > totalTexHeight - texHeight) {
                texScrollY = totalTexHeight - texHeight;
            }
        }
        
        render.Clear();
        
        titleBackgroundTex.Draw(render, 0,0);
        descriptionTex.SetUVWH(0,(int)texScrollY,texWidth,texHeight);
        descriptionTex.Draw(render, texX, texY);
        
        render.Update();
        
        if (input.Quit()) { quit = true; }
        if (input.Confirm()) { quit = true; }
    }
}

/* 7 principles of kwanza is the theme (nguzo saba) */
int main(int argc, char **argv)
{
    try
    {
        srand(time(0));
        render.Init("PDubs Holiday Jam 2021", S_WIDTH, S_HEIGHT);
        texture.Init("assets/kwanzaCandle.jpg", render);
        kwanzaPatternBG.Init("assets/kwanzaPattern.jpeg", render);
        symbolKeyTex.Init("assets/symbolkey_gimp.png", render);
        greenClothTex.Init("assets/greenclothtexture.jpg", render);
        kwanzaCandlesTex.Init("assets/kwanzacandles.png", render);
        titleTex.Init("assets/titleText.png", render);
        titleBackgroundTex.Init("assets/africaBackground.jpg", render);
        selectionIconTex.Init("assets/africanmaskScaled.png", render);
        descriptionTex.Init("assets/principlesDescription.png", render);
        highScoreBGTex.Init("assets/highscorebackground.png", render);
        font.Init("assets/SaikyoBlack.png", 18, 18, render);
        titleFont.Init("assets/TrioDX.png", 9, 17, render);
        music.Init("assets/475150__kevp888__190621-0386-fr-africandrums.wav");
        titleMusic.Init("assets/135811__reinsamba__110611-005-chora-harp-from-gambia.wav");
        titleMusic.Play(true);
        
        
        badEntryEffect.Init("assets/badentry.wav");
        goodEntryEffect.Init("assets/goodentry.wav");
        textEntryEffect.Init("assets/congahit.wav");
        loseLifeEffect.Init("assets/djembedrum.wav");
        menuSelectEffect.Init("assets/menuselect.wav");
        
        InitSymbols();
        LoadHighScores();
        
        bool quit = false;
        while (!input.Quit() && !quit)
        {
            
            int menuSelect = TitleScreenLoop();
            if (input.Quit()) { break; }
            
            switch (menuSelect)
            {
            case TitleMenu::SELECT_DIRECTIONS:
            {
                DirectionsLoop();
                break;
            }
            case TitleMenu::SELECT_PLAY:
            {
                titleMusic.Stop();
                music.Play(true);
                TypingGameLoop();
                GameOverLoop();
                music.Stop();
                titleMusic.Play(true);
                break;
            }
            case TitleMenu::SELECT_ABOUT:
            {
                AboutLoop();
                break;
            }
            case TitleMenu::SELECT_HIGHSCORES:
            {
                ViewHighScoresLoop();
                break;
            }
            case TitleMenu::SELECT_EXIT:
            {
                quit = true;
                break;
            }
            default:
                throw std::runtime_error("Invalid menu selection");
                break;
            }
        }
    }
    catch (std::runtime_error& e)
    {
        std::cerr << "Main caught runtime error: " 
            << e.what() 
            << std::endl;
        return 1;
    }
    return 0;
}
