#pragma once
#include <fstream>
#include <sstream>
#include <iostream>

#include "opencv2/dnn/dnn.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <ctime>
#include <unistd.h>
#include<pthread.h>
int helmetDetection(int argc, char** argv);