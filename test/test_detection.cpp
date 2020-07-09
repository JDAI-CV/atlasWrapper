

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include "Net.hpp"
#include <chrono>

int main(int argc, char* argv[]) {
    int device = 0;
    std::string modelPath = "../test/models/ssd.om";
    std::string enginePath = "./";
    atlas::Net *net = new atlas::Net(modelPath, enginePath, device);
    
    std::cout<<"init end"<<"\n";

    cv::Mat img = cv::imread("../test/images/test1.png");
    std::vector<cv::Mat> img_vector;
    img_vector.push_back(img);
    std::vector<atlas::NetResult> results;
    auto t1=std::chrono::steady_clock::now();
    net->inference(img_vector, results);
    auto t2=std::chrono::steady_clock::now();
    double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
    printf("api call elasped %f\n",dr_ms);

    for(int j=0; j<results.size(); j++) {
        printf("results size %d\n",results.size());
        int batchSize = results[j].batchSize;
        int outputSize_1 = results[j].outputDims[0];
        int outputSize_2 = results[j].outputDims[1];

        std::vector<float> output_1, output_2;
        output_1.assign( results[j].output.begin(), results[j].output.begin() + outputSize_1);
        output_2.assign(results[j].output.begin() + outputSize_1, results[j].output.end());
        int singleSize = output_1.size() / batchSize; 
        int resultSize = 7;
        float threshold = 0.3;
        int maxAmount = 20;

        for(int index = 0; index < batchSize; index++){
            int detCount = int(output_2[index]);
            std::cout<<"batch index"<< index <<"\n";
            std::cout<<detCount<<"\n";
            
            int batch_start_index = singleSize * index;

            int thresholdCount = 0;
            for(int k=0; k<detCount; k++) {
                if (thresholdCount >= maxAmount) {
                    break;
                }
                int id = int(output_1[ batch_start_index + k * resultSize + 0 ]);
                int classId = int(output_1[ batch_start_index + k * resultSize + 1 ]);
                float score = output_1[ batch_start_index + k * resultSize + 2 ];
                float left = output_1[ batch_start_index + k * resultSize + 3 ];
                float top = output_1[ batch_start_index + k * resultSize + 4 ];
                float right = output_1[ batch_start_index + k * resultSize + 5 ];
                float bottom = output_1[ batch_start_index + k * resultSize + 6 ];

                printf("index, class, score, left, top, right, bottom: %d, %d, %f, %f, %f, %f, %f\n", id, classId, score, left, top, right, bottom);
                printf("index*j: %d\n", j*batchSize+index);
                printf("img_vector cols, rows: %d, %d\n", img_vector[j*batchSize+index].cols, img_vector[j*batchSize+index].rows);
                
                int x1 = int( floor(left * img_vector[j*batchSize+index].cols) );
                int y1 = int( floor(top * img_vector[j*batchSize+index].rows) );
                int x2 = int( floor(right * img_vector[j*batchSize+index].cols) );
                int y2 = int( floor(bottom * img_vector[j*batchSize+index].rows) );

                x1 = std::max(0, x1);
                y1 = std::max(0, y1);
                x2 = std::min(img_vector[j*batchSize+index].cols, x2);
                y2 = std::min(img_vector[j*batchSize+index].rows, y2);

                printf("index, class, score, x1, y1, x2, y2: %d, %d, %f, %d, %d, %d, %d\n", id, classId, score, x1, y1, x2, y2);
                if((j)==0 || id == 0) {
                    cv::rectangle(img, cv::Rect(x1,y1,x2-x1,y2-y1), cv::Scalar(0,0,255),1,8,0);           
                }
            }
        }
    }
    cv::imwrite("save_det.jpg",img);

    delete net;
    return 0;
}
