#pragma once
#include"framework_base.h"
#include"myGraph.h"


class BJMAP_LOADING_PAGE :Drawable
{
public:
    void update(float timeScale);
    void rend();
    BJMAP_LOADING_PAGE();
private:
    constexpr static float MAX_SPEED = 6, CENTER_POSX = 800, CENTER_POSY = 500, RADIUS = 50, BASE_SPEED = 1;
    float arc_angel1, arc_angel2;
    float arc_speed1, arc_speed2;
    float timeStamp;
};

class BJMAP_PAGE : public PAGE
{
public:
    BJMAP_PAGE();
    ~BJMAP_PAGE();
    bool Init();
    void Update();
    void FixedUpdate();
    void Rend();

    bool EnterPage();
    bool ExitPage();

    void OnLoad();

    constexpr static int PAGE_INDEX = PAGE_GAME;
private:
    constexpr static int introEff = Effect::WHITE_SWITCH;
    

    BJMAP_LOADING_PAGE loadingPage;
    Button* returnButton,*startButton,*selectButton;

    struct MAP_NODE
    {
        string name;
        string aliasName;
        string strbuf[16];
        int color_index = 0;
    };
    MAP_NODE nodes[16];

    MyGraph<MAP_NODE*>* graph;
    int targetIndex, colorLimitNum, coloringResult;
    atomic<float> *timePerStep;
    atomic<int>* coloringNum;
    

    HANDLE hThreadColoring;
    atomic<int> threadColoringSignal;
    
    
    friend unsigned __stdcall Thread_Coloring(LPVOID lpParameter);
};

unsigned __stdcall Thread_Coloring(LPVOID lpParameter);

namespace THREAD_SIGNAL
{
    constexpr static int SIGNAL_IDLE = 0;
    constexpr static int SIGNAL_PREPARED = 1;
    constexpr static int SIGNAL_START = 2;
    constexpr static int SIGNAL_RUNNING = 3;
    constexpr static int SIGNAL_STOP = 4;
    constexpr static int SIGNAL_QUIT = 5;
    constexpr static int SIGNAL_STOPPED = 6;
}


class MAP_COLORING_PAGE : public PAGE
{
public:
    MAP_COLORING_PAGE();
    ~MAP_COLORING_PAGE();
    bool Init();
    void Update();
    void FixedUpdate();
    void Rend();

    bool EnterPage();
    bool ExitPage();

    void OnLoad();

    constexpr static int PAGE_INDEX = 101;
private:
    constexpr static int introEff = Effect::WHITE_SWITCH;
    

    BJMAP_LOADING_PAGE loadingPage;
    Button* returnButton, * startButton,*timeButton;

    
    int targetIndex, colorLimitNum, coloringResult;
    atomic<float>* timePerStep;
    atomic<int>* coloringNum,*depth;
    TIMER timer;

    struct MAP_BLOCK
    {
        float posx1 = 0, posx2 = 0, posy1 = 0, posy2 = 0;
        int fa = 0;
        int node_index = 0;
        bool edges[4] = { 1,1,1,1 };    //up,down,left,right
        bool isVisited = 0;
        void rend_dbg();
    };
    MAP_BLOCK* blocks;

    struct MAP_NODE
    {
        int color_index = 0;
        vector<MAP_BLOCK*> block_included;
        void rend(bool isVisitedInGraph);
    };
    MAP_NODE* nodes;

    MyGraph<MAP_NODE*>* graph;
    int getFa(int index);
    bool mergeBlock(int index); //return succees
    void connectEdge(int index);
    int dfsAddToNode(int blockIndex, int nodeIndex);

    int nodes_num, block_num;
    int lines_num, cols_num;

    HANDLE hThreadColoring;
    atomic<int> threadColoringSignal;


    friend unsigned __stdcall Thread_Coloring(LPVOID lpParameter);
};

struct CLASS_TYPE
{
    int type = 0;
    void* Object = nullptr;
    constexpr static int CLASS_BJMAP_PAGE = 1;
    constexpr static int CLASS_MAP_COLORING_PAGE = 2;
};