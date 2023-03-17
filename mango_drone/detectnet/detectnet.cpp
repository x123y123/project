/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "detectNet.h"
#include <stdio.h>
#include "cudaDraw.h"
#include "videoOutput.h"
#include "videoSource.h"

#include <sched.h>
#include <signal.h>
#include <time.h>

#ifdef HEADLESS
#define IS_HEADLESS() "headless"  // run without display
#else
#define IS_HEADLESS() (const char *) NULL
#endif

#define output_stream

// x, y, area
float distance_vec[3] = {0.f};

bool signal_recieved = false;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        LogVerbose("received SIGINT\n");
        signal_recieved = true;
    }
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000.0;
    } else {
        diff.tv_sec = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int usage()
{
    printf(
        "usage: detectnet [--help] [--network=NETWORK] [--threshold=THRESHOLD] "
        "...\n");
    printf("                 input_URI [output_URI]\n\n");
    printf(
        "Locate objects in a video/image stream using an object detection "
        "DNN.\n");
    printf(
        "See below for additional arguments that may not be shown above.\n\n");
    printf("positional arguments:\n");
    printf(
        "    input_URI       resource URI of input stream  (see videoSource "
        "below)\n");
    printf(
        "    output_URI      resource URI of output stream (see videoOutput "
        "below)\n\n");
    printf("%s", detectNet::Usage());
    printf("%s", videoSource::Usage());
    printf("%s", videoOutput::Usage());
    printf("%s", Log::Usage());

    return 0;
}

int find_max(float *bb_area)
{
    int max = 0;
    for (int i = 1; i < sizeof(bb_area) / 4; i++) {
        if (bb_area[i] > bb_area[i - 1])
            max = i;
        else
            max = i - 1;
    }
    return max;
}

int main(int argc, char **argv)
{
    /*
     * parse command line
     */
    commandLine cmdLine(argc, argv, IS_HEADLESS());


    if (cmdLine.GetFlag("help"))
        return usage();


    /*
     * attach signal handler
     */
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        LogError("can't catch SIGINT\n");


    /*
     * create input stream
     */
    videoSource *input = videoSource::Create(cmdLine, ARG_POSITION(0));

    if (!input) {
        LogError("detectnet:  failed to create input stream\n");
        return 1;
    }

#ifdef output_stream
    /*
     * create output stream
     */
    videoOutput *output = videoOutput::Create(cmdLine, ARG_POSITION(1));

    if (!output)
        LogError("detectnet:  failed to create output stream\n");
#endif

    /*
     * create detection network
     */
    detectNet *net = detectNet::Create(cmdLine);

    if (!net) {
        LogError("detectnet:  failed to load detectNet model\n");
        return 1;
    }

    // parse overlay flags
    const uint32_t overlayFlags = detectNet::OverlayFlagsFromStr(
        cmdLine.GetString("overlay", "box,labels,conf"));

    // True-vector
    int imgc_x;
    int imgc_y;
    imgc_x = input->GetWidth() / 2;
    imgc_y = input->GetHeight() / 2;

    /*
     * processing loop
     */
    while (!signal_recieved) {
        // capture next image image
        uchar3 *image = NULL;

        if (!input->Capture(&image, 1000)) {
            // check for EOS
            if (!input->IsStreaming())
                break;

            LogError("detectnet:  failed to capture video frame\n");
            continue;
        }

        // detect objects in the frame
        detectNet::Detection *detections = NULL;

        const int numDetections =
            net->Detect(image, input->GetWidth(), input->GetHeight(),
                        &detections, overlayFlags);
        float bbc_x[numDetections];
        float bbc_y[numDetections];
        float bb_area[numDetections];
        int max_detection;  // the max area of detections
        if (numDetections > 0) {
            LogVerbose("%i objects detected\n", numDetections);

            for (int n = 0; n < numDetections; n++) {
                // get bound-box center pos and image center pos
                float b_top = detections[n].Top;
                float b_bottom = detections[n].Bottom;
                float b_left = detections[n].Left;
                float b_right = detections[n].Right;

                bbc_x[n] = (b_right - b_left) / 2 + b_left;
                bbc_y[n] = (b_top - b_bottom) / 2 + b_bottom;
                bb_area[numDetections] =
                    detections[n].Height() * detections[n].Width();
                LogVerbose("detected obj %i  class #%u (%s)  confidence=%f\n",
                           n, detections[n].ClassID,
                           net->GetClassDesc(detections[n].ClassID),
                           detections[n].Confidence);
                LogVerbose("bounding box %i  (%f, %f)  (%f, %f)  w=%f  h=%f\n",
                           n, detections[n].Left, detections[n].Top,
                           detections[n].Right, detections[n].Bottom,
                           detections[n].Width(), detections[n].Height());
                LogVerbose(
                    "bounding center %i (%f, %f) Area= %f, screen center %i "
                    "(%d, %d)\n",
                    n, bbc_x[n], bbc_y[n], bb_area[numDetections], n, imgc_x,
                    imgc_y);
            }
            max_detection = find_max(bb_area);

            CUDA(cudaDrawCircle(image, input->GetWidth(), input->GetHeight(),
                                bbc_x[max_detection], bbc_y[max_detection], 10,
                                make_float4(0, 255, 127, 200)));
            CUDA(cudaDrawCircle(image, input->GetWidth(), input->GetHeight(),
                                imgc_x, imgc_y, 10,
                                make_float4(0, 255, 127, 200)));

            printf("\n\t#########################\n");
            printf("\tmax detection num = %d, class = %s\n", max_detection,
                   net->GetClassDesc(detections[max_detection].ClassID));
            printf("\n\t#########################\n");

            // distance_vec[0] = imgc_x - bbc_x[];
            // distance_vec[1] = imgc_y - bbc_y[];
            // distance_vec[2] = ;
        }
#ifdef output_stream
        // render outputs
        if (output != NULL) {
            output->Render(image, input->GetWidth(), input->GetHeight());

            // update the status bar
            char str[256];
            sprintf(str, "TensorRT %i.%i.%i | %s | Network %.0f FPS",
                    NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH,
                    precisionTypeToStr(net->GetPrecision()),
                    net->GetNetworkFPS());
            output->SetStatus(str);

            // check if the user quit
            if (!output->IsStreaming())
                signal_recieved = true;
        }
#endif
        // print out timing info
        net->PrintProfilerTimes();
    }
    /*
     * destroy resources
     */
    LogVerbose("detectnet:  shutting down...\n");

    SAFE_DELETE(input);
#ifdef output_stream
    SAFE_DELETE(output);
#endif
    SAFE_DELETE(net);

    LogVerbose("detectnet:  shutdown complete.\n");
    return 0;
}
