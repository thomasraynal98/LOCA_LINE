#include <string.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <math.h>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctime>

#include <sys/stat.h>
#include <cstring>

#include <sw/redis++/redis++.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

enum class mode {NO_MODE, READY_TO_RECORD, RECORDING, SAVING, MATCHING, PROBLEM};
static const char *modeString[] = { 
    "NO_MODE", 
    "READY_TO_RECORD", 
    "RECORDING", 
    "SAVING", 
    "MATCHING", 
    "PROBLEM"
};

struct Vect2i
{
    int i, j;

    Vect2i(int _i, int _j)
        : i(_i)
        , j(_j)
        {}

    Vect2i operator=(Vect2i a)
    {
        this->i = a.i;
        this->j = a.j;
    }
};

void init_redis_var_id(sw::redis::Redis* redis);
void init_redis_var(sw::redis::Redis* redis);
void set_redis_var(sw::redis::Redis* redis, std::string channel, std::string value);
void pub_redis_var(sw::redis::Redis* redis, std::string channel, std::string value);
std::string get_redis_str(sw::redis::Redis* redis, std::string channel);
int get_redis_multi_str(sw::redis::Redis* redis, std::string channel, std::vector<std::string>& stockage);
int get_multi_str(std::string str, std::vector<std::string>& vec_str);
int64_t get_curr_timestamp();
std::string get_event_str(int ID_event, std::string event_description, std::string event_info);
void read_yaml(sw::redis::Redis* redis, cv::FileStorage* file_mng, std::string channel);
double frequency_to_ms(int frequency);
bool time_is_over(int64_t curr_timestamp, int64_t ref_timestamp, int64_t max_duration_ms);
bool time_is_over(int64_t curr_timestamp, int64_t end_timestamp);
bool is_same_time(int64_t timesptamp1, int64_t timesptamp2);
int64_t get_elapsed_time(int64_t timesptamp1, int64_t timesptamp2);
void print_redis(sw::redis::Redis* redis, std::string channel_str);
std::string get_standard_robot_id_str(sw::redis::Redis* redis);
bool compare_redis_var(sw::redis::Redis* redis, std::string channel, std::string compare);
double rad_to_deg(double rad);
double deg_to_rad(double deg);
bool file_exist(std::string file_path);
void Write_TXT_file(std::string path, std::string file_data);
std::string get_date_of_today();
double read_all_kilometrage(sw::redis::Redis* redis, std::string path);
double save_kilometrage(sw::redis::Redis* redis, std::string path);

void add_webcam_module(cv::Mat& mat, cv::Mat& frame);
void add_text_to_image(cv::Mat& image, const std::string& text, const cv::Point& position, const cv::Scalar& color, double font_scale);

void add_status_bar(cv::Mat& mat, const cv::Scalar& color, std::string text) ;
std::string enum_to_string(int id);