#pragma once
#include<string>
using namespace std;
struct setting_general_desc
{
    int extra_mode_size = 0;
    int demo_mode_timePerStep = 2;
    int extra_mode_timePerStep = 0;
    int color_limit = 4;
    bool language_translation = 1;
};

struct setting_graphics_desc
{
    int MSAA = 1;
    int sampleCount = 0;
    int resolution = 3;
    int particleDensity = 2;
    bool show_framerate = 0;
    bool window_mode = 1;
    bool vsync = 1;
};

struct setting_audio_desc
{
    int music_volume = 20;
    int se_volume = 50;
    bool thread_method = 0;
    bool spatial_sound = 1;
};



struct stringTable
{
    string test = "test";
    string start = "Start";
    string history = "History";
    string connect = "Connect";
    string PVE = "PVE";
    string quit = "Quit";
    string str_return = "Return";
    string general = "General";
    string graphics = "Graphics";
    string audio = "Audio";

    string language_translation = "Language Translation";

    string window_mode = "Window Mode";
    string vsync = "V-Sync";
    string MSAA = "Anti-aliasing";
    string sampleCount = "Current SampleCount: ";
    string resolution = "Render Resolution";
    string show_framerate = "Show Framerate";

    string music_volume = "Music Volume";
    string se_volume = "SE Volume";
    string thread_method = "Thread Method";

    string on = "On";
    string off = "Off";
    string mode1 = "Mode1";
    string mode2 = "Mode2";
    string windowed = "Windowed";
    string fullscreen = "Fullscreen";
    string lastpage = "Last Page";
    string nextpage = "Next Page";
    string end_of_list = "End of list.";
    string record = "Record";
    //string scoresum = "Sum of Score:";
    string timesum = "Play time:";
    string accountTime = "Account time at";
    string save_succeed = "Save Succeed.";
    string save_failed = "Save Failed.";
    string load_failed = "Load Failed.";
    string enter_username = "Enter Username:";
    string loading = "Loading";
    string no_limit = "No Limit";
    string easy = "Easy";
    string normal = "Normal";
    string hard = "Hard";
    string low = "Low";
    string mid = "Mid";
    string high = "High";
    string free_view = "View Mode: Free";
    


    string language_translation_description = "Use external translation data.";

    
    string window_mode_description = "Change window display mode.";
    string vsync_description = "Enable vertical synchronization to decrease screen tearing and save power.";
    string MSAA_description = "Enable Anti-Aliasing to make the image smoother. Restart application to apply.";
    
    string resolution_description = "Change the resolution of the program.\nRestart application to apply.";
    string show_framerate_description = "Show realtime fps and drawcall times per frame.";
    
    string thread_method_description = "Change sleeping methed of the audio thread. Mode1=Balance Mode2=Performance\nKeep this option at mode 1 unless the audio latency is unbearble.";

    string save_overwrite_warning = "Overwrite current save, Sure?";
    string load_warning = "Load save and LOSE current progress, Sure?";

    string agreement_status_on_description = "Network feature enabled.";
    string agreement_status_off_description = "Enable network feature. You can't disable this feature once enabled.";


    string paste_ip = "Paste IP";
    string paste_port = "Paste Port";

    string copy_succeed = "Copy Succeed";
    string copy_failed = "Copy Failed";

    string connect_succeed = "Connected";
    string connect_failed = "Connect Failed";
    string disconnect = "Disconnected";
    string require_ip = "IP Address Required";

    string io_error = "Failed to access file, Check your permissions";
    string file_corrupt = "File Corrupted";


    string agree = "I see";
    string enable = "Enable Feature";
    string user_agreement = "This program is for learning purpose only.\nOnline game allows you match with other players under the same local area network, hotspots are recommended. You hold responsible for your own device's safety.";
    
    string match_num = "Numbers of Matches";
    string win_num = "Numbers of Winnings";

    string color_limit = "Color Num";
    string color_limit_description = "Change Color Num limitation in coloring";

    string demo_mode_timePerStep = "Demo Mode Time Per Step";
    string demo_mode_timePerStep_description = "Change time per step in demo mode";
    string extra_mode_timePerStep = "Extra Mode Time Per Step";
    string extra_mode_timePerStep_description = "Change time per step in extra mode";
    string extra_mode_size = "Extra Mode Size";
    string extra_mode_size_description = "Select map size in extra mode";

    string PE_warning = "A very small percentage of people may experience a seizure when exposed to certain visual images, including flashing lights or patterns that may appear in video games. Even people who have no history of seizures or epilepsy may have an undiagnosed condition that can cause these “photosensitive epileptic seizures” while playing video games. If you or any of your relatives have a history of seizures or epilepsy, consult a doctor before playing video games.\nImmediately stop playing and consult a doctor if you experience any symptoms.";
};

extern setting_general_desc set1[2];
extern setting_graphics_desc set2[2];
extern setting_audio_desc set3[2];

extern stringTable lan[3];

//保存设置至文件
void saveConfig();

//读取设置文件
void readConfig();

string addFormat(string ori);

