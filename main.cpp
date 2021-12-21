#include <string>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <cstring>
#include <cstdint>

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
static Music music;
static SoundEffect badEntryEffect;
static SoundEffect goodEntryEffect;
static SoundEffect textEntryEffect;
static SoundEffect loseLifeEffect;
static GameTimer timer;
static Font font;
static Font titleFont;

#define NUM_SYMBOLS 7
static Texture Symbols[NUM_SYMBOLS];
static std::unordered_map<std::string, Texture*> SymbolsMap;
static std::vector<std::string> SymbolStrings;

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
    int userStrIndex = 0;
    float pieceSpeed = 20.0f;
    float pieceAddTimer = 6.0f; // seconds until new piece added
    uint64_t pieceIdCtr = 0;
    std::list<SymbolPiece> pieces;
    std::queue<uint64_t> pieceRemoveQueue;
    uint64_t userScore = 0;
    size_t userLife = 100;
    size_t level = 1;
    const int lifeBarWidth = 100;
    const int lifeBarHeight = 10;
    const int startingHealth = 100;
    const int lifeBarX = greenClothTex.GetWidth()-lifeBarWidth-10;
    const int lifeBarY = S_HEIGHT - 50;
    
    memset(curUserStr, 0, sizeof(curUserStr));
    sprintf(userScoreStr, "%llu", userScore);
    
    
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
        if (userStrIndex > 0) {
            font.Draw(render, 10, S_HEIGHT - 20, curUserStr);
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
                    userScore += piece->name.size();
                    sprintf(userScoreStr, "%llu", userScore);
                    std::cout << "New Score: " << userScore << std::endl;
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
            textEntryEffect.Play();
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

void GameOverLoop()
{
    // TODO
    bool quit = false;
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        render.Clear();
        
        if (input.Quit()) { quit = true; }
        
        render.Update();
    }
}

class TitleMenu
{
public:

    constexpr static int SELECT_DIRECTIONS = 0;
    constexpr static int SELECT_PLAY = 1;
    constexpr static int SELECT_ABOUT = 2;

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
            if (mSelection < 0) { mSelection = SELECT_ABOUT; }
        }
        else if (input.Down()) {
            mSelection++;
            if (mSelection > SELECT_ABOUT) { 
                mSelection = SELECT_DIRECTIONS;
            }
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
        static std::string menuTexts[SELECT_ABOUT+1] = {
            "DIRECTIONS",
            "PLAY",
            "ABOUT"
        };
        for (int i=SELECT_DIRECTIONS; i<=SELECT_ABOUT; ++i)
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
    static const int jamTextX = 10;
    static const int jamTextY = 580;
    static const int authorTextX = 300;
    static const int authorTextY = 580;
    #define NUM_TITLE_RECTS 9
    static SDL_Rect titleClipRects[NUM_TITLE_RECTS];
    static SDL_Rect titleDrawRects[NUM_TITLE_RECTS];
    static float titleAnimTimes[NUM_TITLE_RECTS];
    bool quit = false;
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
        
        render.Update();
    }
    
    /* No menu select, user quit */
    return -1;
}

void DirectionsLoop()
{
    bool quit = false;
    while (!quit)
    {
        float dt = timer.Update();
        input.Update();
        
        render.Clear();
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
        font.Init("assets/SaikyoBlack.png", 18, 18, render);
        titleFont.Init("assets/TrioDX.png", 9, 17, render);
        //music.Init("assets/475150__kevp888__190621-0386-fr-africandrums.wav");
        //music.Play(true);
        
        badEntryEffect.Init("assets/badentry.wav");
        goodEntryEffect.Init("assets/goodentry.wav");
        textEntryEffect.Init("assets/congahit.wav");
        loseLifeEffect.Init("assets/djembedrum.wav");
        
        InitSymbols();
        
        while (!input.Quit())
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
                TypingGameLoop();
                GameOverLoop();
                break;
            }
            case TitleMenu::SELECT_ABOUT:
            {
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
