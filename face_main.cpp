#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"
#include "objdetect/objdetect.hpp"
#include "rsdgMission.h"
#include "video/video.hpp"

#include <cmath>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace std;
using namespace cv;

rsdgMission *faceMission;
rsdgPara *pyramidPara;
rsdgPara *selectPara;
rsdgPara *eyesPara;
bool RSDG = false;
bool offline = false;
int totSec;
int totUnit;
int UNIT_PER_CHECK = 5;
int pyramid = 20;
int selectivity = 2;
int eyes = 0;
string pic_index = "";
string XML_PATH = "";

void setupMission(); // setup the rapid mission

void read_images(std::vector<string> &image_list) {
  std::ifstream index(pic_index);
  string img;
  while (index >> img) {
    img = "pics/" + img;
    image_list.push_back(img);
  }
  totUnit = image_list.size();
}

void save_image(Mat image, std::vector<Rect> faces, string filename,
                int pyramid, int selectivity, int smart_eyes) {

  for (size_t i = 0; i < faces.size(); i++) {
    rectangle(image, Point(faces[i].x, faces[i].y),
              Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height),
              Scalar(255, 0, 255), 2, 8, 0);
  }

  char out[100];
  sprintf(out, "./out/%s_%d_%d_%d.jpg", filename.c_str(), pyramid, selectivity,
          smart_eyes);
  string outputloc(out);
  imwrite(outputloc.c_str(), image);
}

std::vector<Rect> detect(string imgname, CascadeClassifier face_cascade,
                         CascadeClassifier eye_cascade, Mat image,
                         int pyramidLevels, int selectivity, int smart_eyes) {

  std::clock_t start, end;

  start = std::clock();

  int min_dim = std::min(image.rows, image.cols);
  float scaleFactor = pow((min_dim / (float)24), (1 / (float)pyramidLevels));

  int groupThreshold = 0;

  std::vector<Rect> faces;
  face_cascade.detectMultiScale(image, faces, scaleFactor);

  if (selectivity > 0) {

    groupThreshold = 3;

    int offset = 5;

    std::vector<Rect> north, east, south, west;

    face_cascade.detectMultiScale(
        image(cv::Range(0, image.rows - offset), cv::Range(0, image.cols)),
        north, scaleFactor);
    faces.insert(faces.end(), north.begin(), north.end());

    face_cascade.detectMultiScale(
        image(cv::Range(0, image.rows), cv::Range(offset, image.cols)), east,
        scaleFactor);
    faces.insert(faces.end(), east.begin(), east.end());

    face_cascade.detectMultiScale(
        image(cv::Range(offset, image.rows), cv::Range(0, image.cols)), south,
        scaleFactor);
    faces.insert(faces.end(), south.begin(), south.end());
    face_cascade.detectMultiScale(
        image(cv::Range(0, image.rows), cv::Range(0, image.cols - offset)),
        west, scaleFactor);
    faces.insert(faces.end(), west.begin(), west.end());
    if (selectivity > 1) {

      groupThreshold = 7;

      std::vector<Rect> northeast, southeast, southwest, northwest;

      face_cascade.detectMultiScale(
          image(cv::Range(0, image.rows - offset), cv::Range(0, image.cols)),
          northeast, scaleFactor);
      faces.insert(faces.end(), northeast.begin(), northeast.end());
      face_cascade.detectMultiScale(
          image(cv::Range(0, image.rows), cv::Range(offset, image.cols)),
          southeast, scaleFactor);
      faces.insert(faces.end(), southeast.begin(), southeast.end());
      face_cascade.detectMultiScale(
          image(cv::Range(offset, image.rows), cv::Range(0, image.cols)),
          southwest, scaleFactor);
      faces.insert(faces.end(), southwest.begin(), southwest.end());
      face_cascade.detectMultiScale(
          image(cv::Range(0, image.rows), cv::Range(0, image.cols - offset)),
          northwest, scaleFactor);
      faces.insert(faces.end(), northwest.begin(), northwest.end());
    }
  }
  groupRectangles(faces, groupThreshold, 0.5);

  if (smart_eyes > 0) {
    std::vector<Rect> temp;
    for (size_t i = 0; i < faces.size(); i++) {

      Mat img_face = image(faces[i]);

      std::vector<Rect> eyes;

      eye_cascade.detectMultiScale(img_face, eyes);

      if (eyes.size() >= smart_eyes)
        temp.push_back(faces[i]);
    }

    faces = temp;
  }

  end = std::clock();
  double runtime = (end - start) / (double)CLOCKS_PER_SEC;

  save_image(image, faces, imgname, pyramidLevels, selectivity, smart_eyes);
  return faces;
}

void *change_pyramid_Num(void *arg) {
  int Pyranum = pyramidPara->intPara;
  cout << "num of pyramid changes from " << pyramid << " to " << Pyranum
       << endl;
  pyramid = Pyranum;
}

void *change_eyes_Num(void *arg) {
  int eyesnum = eyesPara->intPara;
  cout << "num of eyes changes from " << eyes << " to " << eyesnum << endl;
  eyes = eyesnum;
}

void *change_select_Num(void *arg) {
  int selectnum = selectPara->intPara;
  cout << "num of select chagnes from " << selectivity << " to " << selectnum
       << endl;
  selectivity = selectnum;
}

void setupMission() {
  string name = "facedetect";
  faceMission = new rsdgMission(name);
  pyramidPara = new rsdgPara();
  eyesPara = new rsdgPara();
  selectPara = new rsdgPara();

  // register services
  /*for (int i = 0; i < 4; i++) {
    string nodename = to_string(20 - i * 5) + "p";
    faceMission->regService("pyramid", nodename, &change_pyramid_Num, true,
                            make_pair(pyramidPara, i + 1));
  }
  for (int j = 0; j < 3; j++) {
    string nodename = to_string(4 - j * 2) + "s";
    faceMission->regService("select", nodename, &change_select_Num, true,
                            make_pair(selectPara, j + 1));
  }
  for (int k = 0; k < 2; k++) {
    string nodename = to_string(2 - k * 2) + "e";
    faceMission->regService("eyes", nodename, &change_eyes_Num, true,
                            make_pair(eyesPara, k + 1));
  }*/
  faceMission->regContService("pyramidNum", "pyramid", &change_pyramid_Num,
                              pyramidPara);
  faceMission->regContService("selectivityNum", "selectivity",
                              &change_select_Num, selectPara);
  faceMission->regContService("eyesNum", "eyes", &change_eyes_Num, eyesPara);
  faceMission->generateProb(XML_PATH);
  faceMission->setSolver(rsdgMission::GUROBI, rsdgMission::LOCAL,
                         rsdgMission::RAPIDM);
  faceMission->setUnitBetweenCheckpoints(UNIT_PER_CHECK);
  faceMission->setBudget(totSec * 1000);
  faceMission->setUnit(totUnit);
  if (offline) {
    faceMission->readCostProfile();
    faceMission->readMVProfile();
    faceMission->setOfflineSearch();
  }
  faceMission->addConstraint("pyramidNum", true);
  faceMission->addConstraint("selectivityNum", true);
  faceMission->addConstraint("eyesNum", true);
  cout << endl << "RSDG setup finished" << endl;
}

string parseResult(vector<Rect> faces) {
  string result = "";
  int num_of_faces = faces.size();
  result += std::to_string(num_of_faces) + "\n";
  for (int i = 0; i < num_of_faces; i++) {
    string line = "";
    Rect face = faces[i];
    int x = face.x;
    int y = face.y;
    int width = face.width;
    int height = face.height;
    char out[100];
    sprintf(out, "%d %d %d %d 1\n", x, y, width, height);
    result += out;
  }
  return result;
}

/** @function main */
int main(int argc, const char **argv) {
  ofstream result_file;
  result_file.open("./result.txt");
  // process the arguments
  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-index"))
        pic_index = argv[++i];
      if (!strcmp(argv[i], "-rsdg"))
        RSDG = true;
      if (!strcmp(argv[i], "-offline"))
        offline = true;
      if (!strcmp(argv[i], "-b"))
        totSec = stoi(argv[++i]);
      if (!strcmp(argv[i], "-p"))
        pyramid = stoi(argv[++i]);
      if (!strcmp(argv[i], "-s"))
        selectivity = stoi(argv[++i]);
      if (!strcmp(argv[i], "-e"))
        eyes = stoi(argv[++i]);
      if (!strcmp(argv[i], "-xml"))
        XML_PATH = argv[++i];
      if (!strcmp(argv[i], "-u"))
        UNIT_PER_CHECK = stoi(argv[++i]);
    }
  }
  // load cascade XML files
  CascadeClassifier face_cascade, eye_cascade;
  face_cascade.load(
      "/home/liuliu/Research/mara_bench/mara_face/req/face_cascade.xml");
  eye_cascade.load(
      "/home/liuliu/Research/mara_bench/mara_face/req/eye_cascade.xml");

  std::vector<string> image_list;
  read_images(image_list);

  Mat image;
  std::vector<Rect> faces;
  if (RSDG) {
    setupMission();
  }

  for (int i = 0; i < image_list.size(); i++) {
    if (RSDG && i % UNIT_PER_CHECK == 0) {
      faceMission->reconfig();
      if (faceMission->isFailed())
        break;
    }
    // write the img name
    result_file << image_list[i].substr(5) << endl;

    cout << image_list[i] << '\n';

    // load image
    image = imread("/home/liuliu/Research/mara_bench/mara_face/" +
                   image_list[i] + ".jpg");

    // define output location
    size_t pos = 0;
    string token = "";
    std::string finalname = image_list[i];
    while ((pos = finalname.find('/')) != std::string::npos) {
      token = finalname.substr(pos + 1);
      finalname.erase(0, pos + 1);
    }

    // perform face dectection
    faces = detect(token, face_cascade, eye_cascade, image, pyramid,
                   selectivity, eyes);
    result_file << parseResult(faces);
    // faces.clear(); // comment this out to save memory
    if (RSDG)
      faceMission->finish_one_unit();
  }
  if (RSDG){
    faceMission->finish();
  }

  result_file.close();

  return 0;
}
