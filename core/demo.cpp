#include "00_function.h"

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

std::thread thread_compute;
std::thread thread_render;

int cam_id = 0;
Vect2i resolution = Vect2i(480, 640);
cv::Mat input_image;
cv::Mat interface_brut    = cv::Mat(500, 800, CV_8UC3);
cv::Mat interface_brut_cp = cv::Mat(500, 800, CV_8UC3);
bool exit_cap = false;
int ms_to_compute = 0;

mode _mode = mode::NO_MODE;

void f_thread_compute()
{
    double ms_for_loop = frequency_to_ms(30);
    auto next = std::chrono::high_resolution_clock::now();
    
    int64_t compute_ts = get_curr_timestamp();


    cv::VideoCapture cap(cam_id);
    if (!cap.isOpened())
    {
        std::cerr << "Erreur: impossible d'ouvrir la webcam" << std::endl;
        return;
    }

    // Définir la taille souhaitée
    int width = resolution.j;
    int height = resolution.i;

    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    // cap.set(cv::CAP_PROP_FPS, 30);

    while(true)
    {
        // next += std::chrono::milliseconds((int)ms_for_loop);
        // std::this_thread::sleep_until(next);

        cap.read(input_image);

        ms_to_compute = get_elapsed_time(get_curr_timestamp(), compute_ts);
        compute_ts = get_curr_timestamp();

        if(exit_cap) cap.release();
    }
}

void f_thread_render()
{
    double ms_for_loop = frequency_to_ms(30);
    auto next = std::chrono::high_resolution_clock::now();

    // Créer une fenêtre pour afficher la vidéo
    std::string window_name = "Webcam";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL);

    int64_t rendering_ts = get_curr_timestamp();

    while (true)
    {
        next += std::chrono::milliseconds((int)ms_for_loop);
        std::this_thread::sleep_until(next);
        
        interface_brut_cp = interface_brut.clone();

        if(_mode == mode::NO_MODE) add_status_bar(interface_brut_cp, cv::Scalar(0,0,0), enum_to_string((int)_mode));

        if(_mode == mode::READY_TO_RECORD) 
        {
            add_status_bar(interface_brut_cp, cv::Scalar(0,0,50), enum_to_string((int)_mode));
        }

        if(_mode == mode::RECORDING) 
        {
            add_status_bar(interface_brut_cp, cv::Scalar(0,0,255), enum_to_string((int)_mode));
        }

        if(_mode == mode::SAVING) 
        {
            add_status_bar(interface_brut_cp, cv::Scalar(255,0,0), enum_to_string((int)_mode));
        }

        if(input_image.cols == resolution.j) add_webcam_module(interface_brut_cp, input_image);

        add_text_to_image(interface_brut_cp, "RENDERING FPS : " + std::to_string(1000.0/get_elapsed_time(get_curr_timestamp(), rendering_ts)).substr(0, 6) + " (" + std::to_string(get_elapsed_time(get_curr_timestamp(), rendering_ts)) + "ms)", cv::Point(10, 370), cv::Scalar(0,0,0), 0.6);
        add_text_to_image(interface_brut_cp, "COMPUTION FPS : " + std::to_string(1000.0/ms_to_compute).substr(0, 6) + " (" + std::to_string(ms_to_compute) + "ms)", cv::Point(10, 390), cv::Scalar(0,0,0), 0.6);

        rendering_ts = get_curr_timestamp();
        cv::imshow(window_name, interface_brut_cp);

        // Attendre l'appui sur une touche pour quitter
        int key = cv::waitKey(1);
        if(key == 27) // touche 'ESC'
        {
            exit_cap = true;
            break; 
        }
        if(key == 114) _mode = mode::RECORDING;
        if(key == 115 && _mode == mode::RECORDING) _mode = mode::SAVING;
    }

    // Fermer la fenêtre et libérer les ressources
    cv::destroyWindow(window_name);

    return ;
}

int main(int argc, char** argv) {
    bool myFlag = false;
    int myParam = 0;
    std::string myString = "";

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-flag") == 0) {
            myFlag = true;
        } else if(strcmp(argv[i], "-record") == 0) {
            _mode = mode::READY_TO_RECORD;
        } else if (strcmp(argv[i], "-param") == 0 && i + 1 < argc) {
            myParam = stoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-string") == 0 && i + 1 < argc) {
            myString = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cam_id = stoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            int value = stoi(argv[i+1]);

            if(value == 1080) resolution = Vect2i(1080, 1920); // OK
            if(value == 720)  resolution = Vect2i( 720, 1280); // OK
            if(value == 480)  resolution = Vect2i( 480,  640); // OK
            if(value == 360)  resolution = Vect2i( 360,  480); // PAS COMPATIBLE
            if(value == 240)  resolution = Vect2i( 240,  320); // OK
            if(value == 144)  resolution = Vect2i( 144,  256); // PAS COMPATIBLE

            i++;
        } else {
            cout << "Unknown option: " << argv[i] << endl;
        }
    }

    interface_brut.setTo(cv::Scalar(255, 255, 255));
    input_image = cv::Mat(resolution.i, resolution.j, CV_8UC3);

    cout << "myFlag = " << myFlag << endl;
    cout << "myParam = " << myParam << endl;
    cout << "myString = " << myString << endl;

    thread_compute = std::thread(&f_thread_compute);
    thread_render  = std::thread(&f_thread_render);

    thread_compute.join();
    thread_render.join();

    return 0;
}