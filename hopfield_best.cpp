#include <iostream>
#include <functional>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <ctime>
#include <assert.h>
#include <unordered_map>    
#include <cstring>
#include "scopeguard.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace {
    double sigmoid(const double net) {
        return net > 0 ? 1 : -1;
    }

    void plot(const int dimension, const std::vector<double>& input) {
        cv::Mat image(120, 100, CV_8UC3, cv::Scalar(130,240,205));
        for(int i = 0;i < dimension; ++i) {
            if(input[i] == -1)
                cv::circle(image, cv::Point(5+10*(i%10),5+10*(i/10)),1, cv::Scalar(220,0,150),1,8,0);
            else
                cv::rectangle(image, cv::Point(10*(i%10)+2,10*(i/10)+2), cv::Point(8+10*(i%10),8+10*(i/10)),cv::Scalar(220,0,150),-1,8,0);
        }
        std::string result = "YHL.png";
        static int cnt = 0;
        result.insert(3, std::to_string(++cnt));
        cv::namedWindow(result, 0);
        cv::imshow(result, image);
        cv::imwrite(result, image);
    }
}

class Hopfield {
private:
    int len;
    int dimension;
    std::vector< std::vector<int> > samples;
    std::vector< std::vector<double> > weights;

    void initWeight() {
        this->weights.assign(this->dimension, std::vector<double>(this->dimension, 0.00));
        for(int i = 0;i < dimension; ++i) {
            for(int j = 0;j < i; ++j) {
                for(int k = 0;k < len; ++k)
                    weights[i][j] = weights[i][j] + samples[k][i] * samples[k][j];
                weights[j][i] = weights[i][j];
            }
        }
    }

    void dynamic(std::vector<double>& input) {
        while(true) {
            bool changed = false;
            plot(this->dimension, input);
            for(int i = 0;i < dimension; ++i) {
                double output = 0.00;
                for(int j = 0;j < dimension; ++j) 
                    output += weights[i][j] * input[j];
                if(sigmoid(output) not_eq input[i])
                    changed = true;
                input[i] = sigmoid(output);
            }
            std::cout << std::boolalpha << "changed  :  " << changed << "\n";
            if(changed == false) break;
        }
    }

public:
    void testFile(const std::string& fileName) {
        std::ifstream in(fileName.c_str());
        YHL::ON_SCOPE_EXIT([&]{
            in.close();
        });
        assert(in);
        for(int i = 0;i < len; ++i) {
            std::vector<double> input(this->dimension, 0);
            for(int j = 0;j < dimension; ++j) 
                in >> input[j];
            this->dynamic(input);
        }
    }

    void initWeight(const std::string& fileName) {
        std::ifstream in(fileName.c_str());
        YHL::ON_SCOPE_EXIT([&]{
            in.close();
        });
        assert(in);
        in >> this->len >> this->dimension;
        this->samples.assign(this->len, std::vector<int>(this->dimension, 0));
        for(int i = 0;i < len; ++i) 
            for(int j = 0;j < dimension; ++j) 
                in >> samples[i][j];
        this->initWeight();
    }
};

int main()
{
    Hopfield one;
    one.initWeight("best2.txt");
    one.testFile("best.txt");
    cv::waitKey();
    return 0;
}