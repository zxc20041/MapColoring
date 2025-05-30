#pragma once
#include "framework_base.h"
#include <al.h>
#include <alut.h>
#include <alc.h>
#include <vorbis/vorbisfile.h>

constexpr int SE_MAX = 256;
constexpr int SE_AUDIO_NUM = 64;
constexpr int SE_AUDIO_SOURCE_NUM = 7;


//extern int thread_smusic_response, isloop, last_music_request;
//extern int current_playing_music;




struct ALcoms
{
    string name = "";
    bool loaded = 0;
    ALint state = NULL;                            // The state of the sound source
    ALuint bufferID = NULL;                        // The OpenAL sound buffer ID
    ALuint sourceID = NULL;                        // The OpenAL sound source
    ALsizei size2 = NULL;                          // For wave format
    ALenum format = NULL;                          // The sound data format
    ALsizei freq = NULL;                           // The frequency of the sound data
    //vector<char> bufferData;                // The sound buffer data from file
};

struct WAVE_Data {
    char subChunkID[4]; //should contain the word data
    long subChunk2Size; //Stores the size of the data block
};

struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

struct RIFF_Header {
    char chunkID[4];
    long chunkSize;//size not including chunkSize or chunkID
    char format[4];
};

struct musicinfo
{
    string name = "";
};


struct seinfo
{
    string filename = "";
    ALcoms ALwav[SE_AUDIO_SOURCE_NUM];
};

struct SE_DESC
{
    int se_index;
    bool sound_3d_position;
    float sound_positions[3];   //max 1,1 while z=0.5f
    float volumn;   //0-1
};


//����ogg�ļ�
//bool LoadOGG(const char* name, ALuint* buffer, ALsizei* freq, ALenum* format, ALsizei* size);

//����wave�ļ�
//bool loadWavFile(const string filename, ALuint* buffer, ALsizei* size, ALsizei* frequency, ALenum* format);



//����&��Ч�����߳�
unsigned __stdcall ThreadPlayMusic(LPVOID lpParameter);

class AudioManager
{
private:
    seinfo se_p[SE_AUDIO_NUM];  //se sources
    SE_DESC se[SE_MAX]; //se requests
    int thread_Audio_target_music;
    bool thread_Audio_volume_changed, thread_Audio_volume_changed_music, thread_Audio_switch_immediately, thread_Audio_quit_single;
    int current_playing_music, last_music_request;
    int thread_smusic_response, isloop;
    friend unsigned __stdcall ThreadPlayMusic(LPVOID lpParameter);
    friend unsigned __stdcall ThreadPlaySingleMusic(LPVOID lpParameter);

    //create buffers and sources
    void Init();

    // Load the WAVE file into memory
    void LoadWavFromFiles();

    // Clean up sound buffer and source
    void Cleanup();

    void ApplySEVolumeChange();

    //play se in SE_DESC
    void updateSE();
public:
    AudioManager();
    
    //bool getLoop();
    //������Ч
    void playEffectSound(int num);

    //void playEffectSound(int num, float x, float y, float z,float volumn);

    //����ָ���ռ�λ�õ���Ч
    void playEffectSound(int num, float x, float y, float z, float volumn=1.0f);

    void PlayBGM(int music_index, bool immediate,bool loop);

    void PlayMusicGroup(int group);

    void setMusicVolume(int volume);

    void setSEVolume(int volume);


};

extern AudioManager g_am;