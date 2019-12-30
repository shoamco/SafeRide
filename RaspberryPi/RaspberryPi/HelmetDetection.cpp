#include "HelmetDetection.h"

//// This code is written at BigVision LLC. It is based on the OpenCV project. It is subject to the license terms in the LICENSE file found in this distribution and at http://opencv.org/license.html

// Usage example:  ./object_detection_yolo.out --video=run.mp4
//                 ./object_detection_yolo.out --image=bird.jpg



using namespace cv;
using namespace dnn;
using namespace std;

// Initialize the parameters
float confThreshold = 0.6; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;  // Width of network's input image
int inpHeight = 416; // Height of network's input image
vector<string> classes;

// Remove the bounding boxes with low confidence using non-maxima suppression
void  postprocess(Mat& frame, const vector<Mat>& outs, int& num_detection);

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);
void detectionInHelmtFrame(Mat& frame, Mat& blob, const Net& net, CommandLineParser& parser, VideoWriter& video, string& outputFile);
void save_frame_in_image(Mat& frame, size_t framecounter);
int helmetDetection(int argc, char** argv)
{
    cout << "in helmetDetection\n";
    //CommandLineParser parser(argc, argv, keys);
    //parser.about("Use this script to run object detection using YOLO3 in OpenCV.");
    //if (parser.has("help"))
    //{
    //    parser.printMessage();
    //    return 0;
    //}
    // Load names of classes
    string classesFile = "./obj.names";
    ifstream ifs(classesFile.c_str());
    string line;
    while (getline(ifs, line)) classes.push_back(line);

    // Give the configuration and weight files for the model
    String modelConfiguration = "./yolov3-obj.cfg";
    String modelWeights = "./yolov3-obj_2400.weights";

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

        //outputFile = "yolo_out_cpp.mp4";
        //outputFile = "yolo_out_cpp.mp4";
        std::cout << "outputFile " << outputFile << std::endl;
        std::cout << "outputVideo " << outputVideo << std::endl;
        

    }
    catch (...) {
        cout << "Could not open the input image/video stream" << endl;
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
        cout << "--(!)Error opening video capture\n";
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
        cout << "--(!)Error opening video capture\n";
        return -1;
    }
    //while (flag)
    bool No_Helmet = false;
    bool alarm_on = false;
    int num_frame_without_helmet=0;
    clock_t begin = clock();

    clock_t end;

    double time_frame;
    while (cap.read(frame) && flag == true)
        //while (waitKey(1) < 0)
    {

        //sleep(5);
        end = clock();
        time_frame = (double(end - begin) / CLOCKS_PER_SEC)*100;
        //std::cout << "time  " << time_frame << endl;
        if (int(time_frame) % 5 == 0) {
            cap >> frame;
            framecounter++;

            //if (framecounter++ % 30 == 0) {
                // Stop the program if reached end of video
                //if (frame.empty()) {
                //if (framecounter > 200) {
            if (framecounter > 10) {

                cout << "Done processing !!!" << endl;
              

                flag = false;
                //waitKey(3000);
                break;
            }
            else {
                cout << "in else framecounter!!!!! " << framecounter - 1 << endl;
               
                // Create a 4D blob from a frame.
                blobFromImage(frame, blob, 1 / 255.0, Size(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);

                //Sets the input to the network
                net.setInput(blob);

                // Runs the forward pass to get output of the output layers
                vector<Mat> outs;
                net.forward(outs, getOutputsNames(net));

                // Remove the bounding boxes with low confidence
                int num_detection = 0;
                postprocess(frame, outs, num_detection);
                cout << "num_detection " << num_detection << endl;
                if (num_detection == 0) {

                    num_frame_without_helmet++;
                    if (num_frame_without_helmet > 2) {
                        No_Helmet = true; 
                        alarm_on = true;
                   
                            system("omxplayer -o both alarm_cut.mp3");
                       
                     
                    }
                }
                else {
                    num_frame_without_helmet = 0;
                    alarm_on = false;
                }

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
                //imshow(kWinName, frame);
            }
          
        }
        if (alarm_on) {
            system("omxplayer -o both alarm_cut.mp3");
        }
    }
        //}
        //video.write(frame);
        //string save_image= "./yolo_out_cpp.jpg";


    //}
   
    cout << "after while\n";
    cout << "  No_Helmet in drive!!!" << No_Helmet << endl;
     end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "time elapsed_secs " << elapsed_secs << endl;
    if (No_Helmet) {
        system("python3 postRest.py 16454 0");

    }
    else {//all drive with helmet
        system("python3 postRest.py 16454 1");
    }
    cap.release();
    //if (!parser.has("image")) video.release();
    video.release();

    return 0;
}
void save_frame_in_image(Mat& frame, size_t framecounter) {
    cout << "save_frame_in_image\n";
    std::stringstream ss;
    ss << "frame" << framecounter << ".jpg";;


    std::string save_image = ss.str();
    cout << "save file " << save_image << endl;
    imwrite(save_image, frame);

}

// Remove the bounding boxes with low confidence using non-maxima suppression
void  postprocess(Mat& frame, const vector<Mat>& outs,int &num_detection)
{
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
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
                boxes.push_back(Rect(left, top, width, height));
                ++num_detection;
            }
         /*   else
            {
              
            }*/
        }
  
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        drawPred(classIds[idx], confidences[idx], box.x, box.y,
            box.x + box.width, box.y + box.height, frame);
    }
}

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

    //Get the label for the class name and its confidence
    string label = format("%.2f", conf);
    if (!classes.empty())
    {
        CV_Assert(classId < (int)classes.size());
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