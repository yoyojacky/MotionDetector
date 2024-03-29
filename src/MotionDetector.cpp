//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>

using namespace cv;
using namespace std;

// Global variables
Mat frame; //current frame
Mat resizeBlurImg;
Mat binaryImg;
Mat contourImg;
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard

int main(int argc, char* argv[])
{
    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask MOG 2");
    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(300,32,true); //MOG2 approach

    //morphology element
    Mat element = getStructuringElement(MORPH_RECT, Size(7, 7), Point(3,3) );

    //create the capture object
    VideoCapture capture(0);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open camera." << endl;
        exit(EXIT_FAILURE);
    }


    for(;;){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        //Resize
        resize(frame, resizeBlurImg, Size(frame.size().width/3, frame.size().height/3) );
        //Blur
        blur(resizeBlurImg, resizeBlurImg, Size(4,4) );
        //Background subtraction
        pMOG2->apply(resizeBlurImg, fgMaskMOG2, -1);//,-0.5);

        ///////////////////////////////////////////////////////////////////
        //pre procesing
        //1 point delete
        //morphologyEx(fgMaskMOG2, fgMaskMOG2, CV_MOP_ERODE, element);
        morphologyEx(fgMaskMOG2, binaryImg, CV_MOP_CLOSE, element);
        //morphologyEx(fgMaskMOG2, testImg, CV_MOP_OPEN, element);

        //Shadow delete
        //Binary
        threshold(binaryImg, binaryImg, 128, 255, CV_THRESH_BINARY);

        //Find contour
        contourImg = binaryImg.clone();
        //less blob delete
        vector< vector< Point> > contours;
        findContours(contourImg,
                     contours, // a vector of contours
                     CV_RETR_EXTERNAL, // retrieve the external contours
                     CV_CHAIN_APPROX_NONE); // all pixels of each contours

        vector< Rect > output;
        vector< vector< Point> >::iterator itc= contours.begin();
        while (itc!=contours.end()) {

            //Create bounding rect of object
            //rect draw on origin image
            Rect mr= boundingRect(Mat(*itc));
            rectangle(resizeBlurImg, mr, CV_RGB(255,0,0));
            ++itc;
        }

        //Display
        imshow("Shadow_Removed", binaryImg);
        imshow("Blur_Resize", resizeBlurImg);
        imshow("MOG2", fgMaskMOG2);

        if(waitKey(17)==27)break;
    }
    return EXIT_SUCCESS;
}
