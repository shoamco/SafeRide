#include "PhoneDetection.h"




using namespace cv;
using namespace dnn;
using namespace std;
static string  USER_ID = "121212";
static std::vector<pthread_t> thread_alart_vector;
// Initialize the parameters
static float confThreshold = 0.6; // Confidence threshold
static float nmsThreshold = 0.4;  // Non-maximum suppression threshold
static int inpWidth = 416;  // Width of network's input image
static int inpHeight = 416; // Height of network's input image
static vector<string> classes;

// Remove the bounding boxes with low confidence using non-maxima suppression
static void  postprocess(Mat& frame, const vector<Mat>& outs, int& num_detection, int frame_height, int frame_width);

// Draw the predicted bounding box
static void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame, int frame_height, int frame_width);

// Get the names of the output layers
static vector<String> getOutputsNames(const Net& net);
//void detectionInHelmtFrame(Mat& frame, Mat& blob, const Net& net, CommandLineParser& parser, VideoWriter& video, string& outputFile);
static void save_frame_in_image(Mat& frame, size_t framecounter);
static void* alarmOn(void* id) {
    std::cout << "in alarmOn\n";
    system("omxplayer -o both alarm_cut.mp3");
    return NULL;
}
static void voiceOn() {
    pthread_t thread_alart;
    thread_alart_vector.push_back(thread_alart);

    int p = pthread_create(&thread_alart, NULL, alarmOn, (NULL));
    if (p) {
        cout << "unable to create sound thread\n";
    }

}
int phoneDetection(int argc, char** argv)
{
    std::cout << "in phoneDetection\n";

    string classesFile = "./coco.names";
    ifstream ifs(classesFile.c_str());
    string line;
    while (getline(ifs, line)) classes.push_back(line);

    // Give the configuration and weight files for the model
    String modelConfiguration = "./yolov3.cfg";
    String modelWeights = "./yolov3.weights";

    // Load the network
    Net net = readNetFromDarknet(modelConfiguration, modelWeights);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    // Open a video file or an image file or a camera stream.
    string str, outputFile, outputVideo;
    /* VideoCapture cap;
     VideoWriter video;*/
    VideoCapture cap(0);
    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    //VideoWriter video=// Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file. 
    VideoWriter video("outcpp.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(frame_width, frame_height));
    Mat frame, blob;

    try {

        outputVideo = "./yolo_out_cpp.avi";
        outputFile = "./yolo_out_cpp.jpg";

        /*std::cout << "outputFile " << outputFile << std::endl;
        std::cout << "outputVideo " << outputVideo << std::endl;*/


    }
    catch (...) {
        std::cout << "Could not open the input image/video stream" << endl;
        return 0;
    }

    // Get the video writer initialized to save the output video
  /*  if (!parser.has("image")) {
        video.open(outputFile, VideoWriter::fourcc('M', 'J', 'P', 'G'), 28, Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT)));
    }*/
    //video.open(outputVideo, VideoWriter::fourcc('M', 'P', 'P', 'G'), 28, Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT)));

    // Create a window
    static const string kWinName = "Deep learning object detection in OpenCV";

    if (!cap.isOpened())
    {
        std::cout << "--(!)Error opening video capture\n";
        return -1;
    }
    ////Mat frame;
    //while (capture.read(frame))
    size_t framecounter = 0;
    //std::cout << "waitKey(1) " << waitKey(1) << std::endl;
    bool flag = true;
    cap.open(0);
    if (!cap.isOpened())
    {
        std::cout << "--(!)Error opening video capture\n";
        return -1;
    }
    //while (flag)
    bool foundPhone = false;
    bool alarm_on = false;
    int num_frame_without_helmet = 0;
    clock_t begin = clock();

    clock_t end;
    clock_t begin_detection;
    clock_t end_detection;
    clock_t begin_save_frame;
    clock_t end_save_frame;
    double time_frame;
    double time_detection;
    double time_save_frame;
    int test_num_of_detection_helmet = 0;
    while (cap.read(frame) && flag == true)
        //while (waitKey(1) < 0)
    {

        //sleep(5);
        end = clock();
        time_frame = (double(end - begin) / CLOCKS_PER_SEC) * 100;
        //std::cout << "time  " << time_frame << endl;
        if (int(time_frame) % 4 == 0) {
            cap >> frame;
            framecounter++;

            //if (framecounter++ % 30 == 0) {
                // Stop the program if reached end of video
                //if (frame.empty()) {
                //if (framecounter > 200) {
            if (framecounter > 10) {

                std::cout << "Done processing !!!" << endl;


                flag = false;
                //waitKey(3000);
                break;
            }
            else {
                std::cout << "in else framecounter!!!!! " << framecounter - 1 << endl;
                begin_detection = clock();
                // Create a 4D blob from a frame.
                blobFromImage(frame, blob, 1 / 255.0, Size(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);

                //Sets the input to the network
                net.setInput(blob);

                // Runs the forward pass to get output of the output layers
                vector<Mat> outs;
                net.forward(outs, getOutputsNames(net));

                // Remove the bounding boxes with low confidence
                int num_detection = 0;
                postprocess(frame, outs, num_detection, frame_height, frame_width);

                std::cout << "****** num detection phone  " << num_detection << "   ******" << endl;
                end_detection = clock();
                time_detection = (double(end_detection - begin_detection) / CLOCKS_PER_SEC) / 10;
                cout << "time_detection " << time_detection << endl;
                if (num_detection>0) {//detecte phone

                    num_frame_without_helmet++;
                    system("omxplayer -o both alarm_cut.mp3");
                    cout << "num_frame_without_helmet " << num_frame_without_helmet << endl;
                    //if (num_frame_without_helmet >=1) {
                    foundPhone = true;
                    alarm_on = true;

                    //voiceOn();
                  /*  pthread_t thread_alart;
                    thread_alart_vector.push_back(thread_alart);

                    int p = pthread_create(&thread_alart,NULL, alarmOn,(NULL));
                    if (p) {
                        cout << "unable to create sound thread\n";
                    }*/
                    //}
                            //system("omxplayer -o both alarm_cut.mp3");


                }

                else {
                    test_num_of_detection_helmet++;
                    num_frame_without_helmet = 0;
                    alarm_on = false;
                    /*   for (int i = 0; i < thread_alart_vector.size(); i++) {
                           pthread_join(thread_alart_vector[i], NULL);
                       }*/

                }
                begin_save_frame = clock();
                // Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
                vector<double> layersTimes;
                double freq = getTickFrequency() / 1000;
                double t = net.getPerfProfile(layersTimes) / freq;
                string label = format("Inference time for a frame : %.2f ms", t);
                putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
                ;
                // Write the frame with the detection boxes
                Mat detectedFrame;
                frame.convertTo(detectedFrame, CV_8U);

                //video.write(detectedFrame);
                //imwrite(outputFile, detectedFrame);
                save_frame_in_image(frame, framecounter);

                end_save_frame = clock();
                time_save_frame = (double(end_save_frame - begin_save_frame) / CLOCKS_PER_SEC) / 10;
                cout << "time save fame " << time_save_frame << endl;
                //imshow(kWinName, frame);
            }

        }
        if (alarm_on) {
            //voiceOn();
            system("omxplayer -o both alarm_cut.mp3");
        }
    }
    //}
    //video.write(frame);
    //string save_image= "./yolo_out_cpp.jpg";


//}



    std::cout << "after while\n";
    std::cout << "  foundPhone in drive!!!" << foundPhone << endl;
    std::cout << " test_num_of_detection_helmet" << test_num_of_detection_helmet << endl;
    end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "time elapsed_secs " << elapsed_secs << endl;
    if (foundPhone) {
        string s = "python3 postRest.py " + std::string(USER_ID) + " 0";
        system("python3 postRest.py 121212 0");

    }
    else {//all drive with helmet
        system("python3 postRest.py 121212 1");
    }
    cap.release();
    //if (!parser.has("image")) video.release();
    video.release();
    for (int i = 0; i < thread_alart_vector.size(); i++) {
        pthread_join(thread_alart_vector[i], NULL);
        //pthread_cancel(thread_alart_vector[i]);
    }
    return 0;
}
void save_frame_in_image(Mat& frame, size_t framecounter) {

    // current date/time based on current system
    time_t now = time(0);

    // convert now to string form
    char* dt = ctime(&now);

    cout << "The local date and time is: " << dt << endl;
    cout << "save_frame_in_image\n";
    std::stringstream ss;
    //ss << dt << ".jpg";
    //ss << dt<<".jpg";
    ss << "phone frame" << framecounter << ".jpg";


    std::string save_image = ss.str();
    //std::string save_image = dt;
    //std::replace(save_image.begin(), save_image.end(), ':', 'I');  // replace ':' by ' '
    //std::replace(save_image.begin(), save_image.end(), ' ', 'I');  // replace ':' by ' '
    //save_image += ".jpg";
  /*  vector<int> array;
    stringstream ss(save_image);
    int temp;
    while (ss >> temp)
        array.push_back(temp);*/


    cout << "save file " << save_image << endl;
    imwrite(save_image, frame);

}

// Remove the bounding boxes with low confidence using non-maxima suppression
void  postprocess(Mat& frame, const vector<Mat>& outs, int& num_detection, int frame_height, int frame_width)
{
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
    //vector<Rect> boxes_down_helmet;
    num_detection = 0;
    for (size_t i = 0; i < outs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            //cout << "confidence " << confidence << endl;
            if (confidence > confThreshold)
            {

                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
          
                    cout << "In the top half of the image\n";
                   /* ++num_detection;*/
              

                boxes.push_back(Rect(left, top, width, height));

                /*    }*/
                   /* else {
                        boxes_down_helmet.push_back(Rect(left, top, width, height));;
                    }*/
            }
            /*   else
               {

               }*/
        }

    }
    /*  clock_t begin_save_frame;
      clock_t end_save_frame;
      double time_frame;*/
      // Perform non maximum suppression to eliminate redundant overlapping boxes with
      // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        if (classIds[idx] == 67) {
            ++num_detection;
            drawPred(classIds[idx], confidences[idx], box.x, box.y,
                box.x + box.width, box.y + box.height, frame, frame_width, frame_height);
        }
   
    }
}

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame, int frame_height, int frame_width)
{

    //Draw a rectangle displaying the bounding box
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);
    //rectangle(frame, Point(0, frame_width), Point(0, frame_height), Scalar(255, 178, 50), 3);

    // line for x1, y1, x2, y2 
    //line(0,  frame_height/2, frame_width, frame_height/2);


    //Get the label for the class name and its confidence
    string label = format("%.2f", conf);
    if (!classes.empty())
    {
        //CV_Assert(classId < (int)classes.size());
        label = classes[classId] + ":" + label;
    }

    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(frame, Point(left, top - round(1.5 * labelSize.height)), Point(left + round(1.5 * labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net)
{
    static vector<String> names;
    if (names.empty())
    {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers = net.getUnconnectedOutLayers();

        //get the names of all the layers in the network
        vector<String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}