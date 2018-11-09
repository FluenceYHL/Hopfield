#ifndef HOPFIELD_H
#define HOPFIELD_H
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
#include "scopeguard.hpp"
#include <QVector>
#include <QDebug>

namespace {
    double sigmoid(const double net) {
        return net > 0 ? 1 : -1;
    }

    std::string hash(const std::vector<int>& array) {
        std::string ans;
        for(const auto& it : array)
            ans += std::to_string(it);
        return ans;
    }
}

class Hopfield {
private:
    int len;
    int dimension;
    std::vector< std::vector<int> > samples;
    std::unordered_map<std::string, int> answers;
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

    void dynamic(std::vector<int>& input) {
        while(true) {
            bool changed = false;
            for(int i = 0;i < dimension; ++i) {
                double output = 0.00;
                for(int j = 0;j < dimension; ++j)
                    output += weights[i][j] * input[j];
                if(sigmoid(output) not_eq input[i])
                    changed = true;
                input[i] = sigmoid(output);
            }
            if(changed == false) break;
        }
        auto index = hash(input);
        auto one = this->answers.find(index);
        if(one not_eq this->answers.end())
            std::cout << "识别结果  :  " << one->second << "\n";
        else
            this->answers.emplace(index, -1);
    }

public:
    void testFile(const std::string& fileName) {
        std::ifstream in(fileName.c_str());
        YHL::ON_SCOPE_EXIT([&]{
            in.close();
        });
        assert(in);
        for(int i = 0;i < len; ++i) {
            std::vector<int> input(this->dimension, 0);
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
        int answer;
        for(int i = 0;i < len; ++i) {
            for(int j = 0;j < dimension; ++j)
                in >> samples[i][j];
            in >> answer;
            this->answers.emplace(hash(samples[i]), answer);
        }
        this->initWeight();
    }

    std::pair<QVector<double>, QVector<double> > getEfficiency(){
        std::pair<QVector<double>, QVector<double> > one;
        return one;
    }
};
/*
int main()
{
    Hopfield one;
    one.initWeight("best2.txt");
    one.testFile("best.txt");
    cv::waitKey();
    return 0;
}
*/
#endif // HOPFIELD_H
