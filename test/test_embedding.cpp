

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include "Net.hpp"
#include <chrono>

int main(int argc, char* argv[])
{
    int device = 0 ;
    std::string modelPath = "../test/models/resnet18.om";

    std::string enginePath = "./";
    atlas::Net *reid = new atlas::Net(modelPath, enginePath, device);
    

    std::cout<<"init end"<<"\n";

    for (size_t i = 0; i <4; i++)
    {
      cv::Mat img = cv::imread("/home/huawei/SDK/FaceSDK_v3.6/extractor/data/19.jpg");
       std::vector<cv::Mat> img_vector;
       for(int aaa=0;aaa<1;aaa++){
          img_vector.push_back(img);
  
       }
        std::vector<atlas::NetResult> results;
        auto t1=std::chrono::steady_clock::now();
        reid->inference(img_vector, results);
        auto t2=std::chrono::steady_clock::now();

        double dr_ms=std::chrono::duration<double,std::milli>(t2-t1).count();
        printf("api call elasped %f\n",dr_ms);

        for(int j=0; j<results.size(); j++){
            printf("results size %d\n",results.size());
            int batchSize = results[j].batchSize;
            int outputSize_1 = results[j].outputDims[0];

            std::vector<float> output_1, output_2;
            output_1.assign( results[j].output.begin(), results[j].output.begin() + outputSize_1);
            printf("output_1 size %d\n", outputSize_1);
            int singleSize = output_1.size() / batchSize; 

            for(int index = 0; index < batchSize; index++){
                
                int batch_start_index = singleSize * index;
                for(int f = 0; f < 10; f++ ){
                    printf("%f\n", output_1[batch_start_index + f]);
                }

                if(index == 0){
                    FILE *fp = fopen("feat_atlas_new.txt","w");
                    for(int f = 0; f < 512; f++)
                    {
                        fprintf(fp,"%.6f\n", output_1[f]);
                    }
                    fclose(fp);
                }
            }
        }
    }
   
    delete reid;
    return 0;
}
