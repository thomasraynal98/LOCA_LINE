#include "00_function.h"

#include <iostream>
#include <string>
#include <cstring>
#include <DBoW2/DBoW2.h>

using namespace std;

std::thread thread_compute;
std::thread thread_render;

int cam_id                  = 0;
Vect2i resolution           = Vect2i(480, 640);
cv::Mat input_image;
cv::Mat interface_brut      = cv::Mat(500, 800, CV_8UC3);
cv::Mat interface_brut_cp   = cv::Mat(500, 800, CV_8UC3);
bool exit_cap               = false;
int ms_to_compute           = 0;

mode _mode                  = mode::NO_MODE;
std::string folder_name     = "";
double record_fps           = 0.0;
bool start_record           = false;
int64_t start_recording     = get_curr_timestamp();
int counter_recording       = 0;
std::string file_name       = "image";

std::vector<std::vector<cv::Mat>> dataset;
bool start_matching         = false;

using namespace DBoW2;
void loadFeatures(vector<vector<cv::Mat>> &features);
void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out);
void testVocCreation(const vector<vector<cv::Mat>> &features);
void testDatabase(const vector<vector<cv::Mat>> &features);

void save_record_voc(const vector<vector<cv::Mat > > &features)
{
    // branching factor and depth levels 
    const int k = 9;
    const int L = 3;
    const WeightingType weight = TF_IDF;
    const ScoringType scoring = L1_NORM;

    OrbVocabulary voc(k, L, weight, scoring);
    voc.create(features);
    voc.save(folder_name + "_" + std::to_string(resolution.i) + ".yml.gz");

    std::string command = "cp ~/Dev/LOCA_LINE/build/" + folder_name + "_" + std::to_string(resolution.i) + ".yml.gz ~/Dev/LOCA_LINE/data/" + folder_name + "_" + std::to_string(resolution.i) + "p/";
    system(command.c_str());
}

void read_record_voc()
{
    OrbVocabulary voc(folder_name + "_" + std::to_string(resolution.i) + ".yml.gz");
}

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

    int64_t last_recording = get_curr_timestamp();

    while(true)
    {
        // next += std::chrono::milliseconds((int)ms_for_loop);
        // std::this_thread::sleep_until(next);

        cap.read(input_image);

        ms_to_compute = get_elapsed_time(get_curr_timestamp(), compute_ts);
        compute_ts = get_curr_timestamp();

        if(_mode == mode::RECORDING)
        {
            if(!start_record)
            {
                std::string command = "mkdir ../data/" + folder_name + "_" + std::to_string(resolution.i) + "p";
                system(command.c_str());
                start_record = true;
            }
            else
            {
                if(!input_image.empty())
                {
                    if(get_elapsed_time(get_curr_timestamp(), last_recording) > 1000.0/record_fps)
                    {
                        std::string curr_file_name = "../data/" + folder_name + "_" + std::to_string(resolution.i) + "p/" + file_name + "_" + std::to_string(counter_recording) + ".png";
                        cv::imwrite(curr_file_name, input_image);
                        cv::waitKey(10);
                        counter_recording++;
                        last_recording = get_curr_timestamp();
                    }
                }
            }
        }

        if(_mode == mode::SAVING)
        {
            std::cout << "New dataset \"" + folder_name + "_" + std::to_string(resolution.i) + "p\" saved with " << std::to_string(number_of_file_in_folder(("../data/" + folder_name + "_" + std::to_string(resolution.i) + "p").c_str())) << " images. Size = " << std::to_string(get_directory_size(("../data/" + folder_name + "_" + std::to_string(resolution.i) + "p").c_str())) << " Mo" << std::endl;
            _mode = mode::NO_MODE;

            std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::milliseconds((int)1000));

            uint64 extrator_ts = get_curr_timestamp();
            std::vector<vector<cv::Mat>> features;
            loadFeatures(features);
            std::cout << "Extraction completed = " << std::to_string(get_elapsed_time(get_curr_timestamp(), extrator_ts)) << " ms" << std::endl;

            save_record_voc(features);
            std::cout << "Vocubalary saved." << std::endl;
        }

        if(_mode == mode::MATCHING)
        {
            if(!start_matching)
            {

                start_matching = true;
            }
        }

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
        } else if(strcmp(argv[i], "-m") == 0 && i + 2 < argc) {
            _mode = mode::MATCHING;
            folder_name = argv[i+1];
        } else if(strcmp(argv[i], "-record") == 0 && i + 2 < argc) {
            folder_name = argv[i+1];
            record_fps  = std::stod(argv[i+2]);
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// number of training images
const int NIMAGES = 6;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}

// ----------------------------------------------------------------------------

// int main()
// {
//   vector<vector<cv::Mat > > features;
//   loadFeatures(features);

//   testVocCreation(features);

//   wait();

//   testDatabase(features);

//   return 0;
// }

// ----------------------------------------------------------------------------

void loadFeatures(vector<vector<cv::Mat>> &features)
{
    // 3. Récupérer le nombre de photo disponible.
    for(int i = 0; i < 10000000; i++)
    {
        if(!file_exist("~/Dev/LOCA_LINE/data/" + folder_name + "_" + std::to_string(resolution.i) + "p/image_" + std::to_string(counter_recording) + ".png")) break;
        counter_recording++;
    }

    features.clear();
    features.reserve(counter_recording);

    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    std::cout << "Extracting ORB features in " << std::to_string(counter_recording) << " images." << std::endl;
    for(int i = 0; i < counter_recording; ++i)
    {
        cv::Mat image = cv::imread(("../data/" + folder_name + "_" + std::to_string(resolution.i) + "p/image_" + std::to_string(i) + ".png"), 0);
        cv::Mat mask;

        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        orb->detectAndCompute(image, mask, keypoints, descriptors);

        features.push_back(vector<cv::Mat >());
        changeStructure(descriptors, features.back());
        std::cout << "extraction (" << i+1 << "|" <<  counter_recording << ")" << std::endl;
    }
}

// ----------------------------------------------------------------------------

void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out)
{
  out.resize(plain.rows);

  for(int i = 0; i < plain.rows; ++i)
  {
    out[i] = plain.row(i);
  }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<cv::Mat > > &features)
{
  // branching factor and depth levels 
  const int k = 9;
  const int L = 3;
  const WeightingType weight = TF_IDF;
  const ScoringType scoring = L1_NORM;

  OrbVocabulary voc(k, L, weight, scoring);

  cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
  voc.create(features);
  cout << "... done!" << endl;

  cout << "Vocabulary information: " << endl
  << voc << endl << endl;

  // lets do something with this vocabulary
  cout << "Matching images against themselves (0 low, 1 high): " << endl;
  BowVector v1, v2;
  for(int i = 0; i < features.size(); i++)
  {
    voc.transform(features[i], v1);
    for(int j = 0; j < features.size(); j++)
    {
      voc.transform(features[j], v2);
      
      double score = voc.score(v1, v2);
    //   cout << "Image " << i << " vs Image " << j << ": " << score << endl;
    }
  }

  // save the vocabulary to disk
  cout << endl << "Saving vocabulary..." << endl;
  voc.save("small_voc.yml.gz");
  cout << "Done" << endl;
}

// ----------------------------------------------------------------------------

void testDatabase(const vector<vector<cv::Mat > > &features)
{
  cout << "Creating a small database..." << endl;

  // load the vocabulary from disk
  OrbVocabulary voc("small_voc.yml.gz");
  
  OrbDatabase db(voc, false, 0); // false = do not use direct index
  // (so ignore the last param)
  // The direct index is useful if we want to retrieve the features that 
  // belong to some vocabulary node.
  // db creates a copy of the vocabulary, we may get rid of "voc" now

  // add images to the database
  for(int i = 0; i < NIMAGES; i++)
  {
    db.add(features[i]);
  }

  cout << "... done!" << endl;

  cout << "Database information: " << endl << db << endl;

  // and query the database
  cout << "Querying the database: " << endl;

  QueryResults ret;
  for(int i = 0; i < NIMAGES; i++)
  {
    db.query(features[i], ret, 4);

    // ret[0] is always the same image in this case, because we added it to the 
    // database. ret[1] is the second best match.

    cout << "Searching for Image " << i << ". " << ret << endl;
  }

  cout << endl;

  // we can save the database. The created file includes the vocabulary
  // and the entries added
  cout << "Saving database..." << endl;
  db.save("small_db.yml.gz");
  cout << "... done!" << endl;
  
  // once saved, we can load it again  
  cout << "Retrieving database once again..." << endl;
  OrbDatabase db2("small_db.yml.gz");
  cout << "... done! This is: " << endl << db2 << endl;
}