#pragma once
#include<string.h>
using namespace debugger;

template <typename T>
class MyGraph {
public:
    MyGraph(int capacity);
    ~MyGraph();
    //添加的顶点对应的索引依次为0 ... capacity-1. 
    bool addVertex(T data);

    //给无向图添加边。 
    bool setValue2Matrix4UndirectedGraph(int row, int col, int val = 1);
    void resetVertex(int index=-1);

    bool DFS(int currentIndex, int color_limit,atomic<float> *pTimeInterval, atomic<int>* pColoringNum, atomic<int>* depth = nullptr);

    T getData(int index);

    bool getVisited(int index);
    void setVisited(int index);

    TIMER timer;
private:
    bool isAvailable(int row, int col) const;   //row, col合法性判断
    int getValueFromMatrix(int row, int col) const;

    struct Vertex {
        T data;
        bool isVisited;
    };
    int capacity;   //最多可容纳顶点数
    int cntV;       //已添加顶点数
    Vertex* arrV;   //顶点数组
    int* pMatrix;   //邻接矩阵 

    int start_index;
};



template <typename T>
MyGraph<T>::MyGraph(int capacity) {
    this->capacity = capacity;
    cntV = 0;
    arrV = new Vertex[capacity];
    pMatrix = new int[capacity * capacity];
    memset(pMatrix, 0, capacity * capacity * sizeof(int));
}

template <typename T>
MyGraph<T>::~MyGraph() {
    delete[] arrV;
    delete[] pMatrix;
}

template <typename T>
bool MyGraph<T>::addVertex(T data) {
    if (cntV == capacity) return false;
    arrV[cntV].data = data;
    arrV[cntV++].isVisited = false;
    return true;
}

template <typename T>
void MyGraph<T>::resetVertex(int index) 
{
    for (int i = 0; i < cntV; ++i)
    {
        arrV[i].isVisited = false;
    }
    start_index = index;
    return;
}

template<typename T>
bool MyGraph<T>::DFS(int currentIndex, int color_limit, atomic<float> *pTimeInterval, atomic<int>* pColoringNum,atomic<int>* depth)
{
    if (pColoringNum->load() < 0)
    {
        return 0;
    }
    if (pTimeInterval->load() > 0)
    {
        while (timer.stop_timer() < pTimeInterval->load());
        timer.start_timer();
    }
    
    bool color_used[8];
    memset(color_used, 0, sizeof(color_used));
    if (depth != nullptr)
    {
        depth->operator++();
    }
    
    arrV[currentIndex].isVisited = true;
    
    pColoringNum->operator++();
    
    for (int i = 0; i < cntV; ++i) 
    {
        if (getValueFromMatrix(currentIndex, i) > 0 && arrV[i].isVisited)
        {
            color_used[arrV[i].data->color_index] = 1;  //mark used color
        }
        
    }

    //record status
    vector<bool> isVisitedStatus(cntV);
    for (int i = 0; i < cntV; ++i) 
    {
        isVisitedStatus[i] = arrV[i].isVisited;
    }

    bool flag = 0;  //success flag
    for (int i = 0; i < color_limit; i++)
    {
        
        if (!color_used[i])
        {
            arrV[currentIndex].data->color_index = i;
            flag = 1;
            for (int j  = 0; j < cntV; ++j)
            {
                if (getValueFromMatrix(currentIndex, j) > 0 && !arrV[j].isVisited)
                {
                    if (!DFS(j, color_limit, pTimeInterval,pColoringNum,depth))
                    {
                        flag = 0;
                        break;
                    }
                }
            }
            if (!flag)
            {
                if (pColoringNum->load() < 0)
                {
                    return 0;
                }
                //recover status when failed
                for (int i = 0; i < cntV; i++)
                {
                    arrV[i].isVisited = isVisitedStatus[i];
                }
                continue;
            }
            else
            {
                if (depth != nullptr)
                {
                    depth->operator--();
                }
                
                return 1;
            }
            
        }
    }
    if (depth != nullptr)
    {
        depth->operator--();
    }
    arrV[currentIndex].isVisited = 0;
    return 0;
}


template<typename T>
T MyGraph<T>::getData(int index)
{
    if (index >= cntV || index < 0)
    {
        return T();
    }

    return arrV[index].data;
}

template<typename T>
inline bool MyGraph<T>::getVisited(int index)
{
    if (index >= cntV || index < 0)
    {
        return 0;
    }
    return arrV[index].isVisited;
}

template<typename T>
inline void MyGraph<T>::setVisited(int index)
{
    if (index >= cntV || index < 0)
    {
        return;
    }
    arrV[index].isVisited = 1;
    return;
}



template <typename T>
bool MyGraph<T>::setValue2Matrix4UndirectedGraph(int row, int col, int val) {
    if (!isAvailable(row, col)) return false;
    pMatrix[row * capacity + col] = pMatrix[col * capacity + row] = val;
    return true;
}



template <typename T>
bool MyGraph<T>::isAvailable(int row, int col) const {
    return 0 <= row && row < capacity && 0 <= col && col < capacity;
}

template <typename T>
int MyGraph<T>::getValueFromMatrix(int row, int col) const {
    if (!isAvailable(row, col)) return -1;
    return pMatrix[row * capacity + col];
}