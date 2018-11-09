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
#include <QImage>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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

    // 可以先哈希，再做......
    int getHaiMing(const std::vector<int>& lhs, const std::vector<int>& rhs) {
        int distance = 0;
        int l = lhs.size ();
        for(int i = 0;i < l; ++i)
            distance += (lhs[i] not_eq rhs[i]);
        return distance;
    }

    std::vector<int> getVector(const QImage& image) {
        std::vector<int> collect;
        QRgb *oneLine = nullptr;
        const int height = image.height ();
        for(int i = 0;i < height; ++i) {
            oneLine = (QRgb*)image.scanLine (i);
            const int width = image.width ();
            for(int j = 0;j < width; ++j) {
                if(qRed(oneLine[j]) == 0 || qGreen (oneLine[j] == 0 || qBlue (oneLine[j]) == 0))
                    collect.emplace_back(1);
                else
                    collect.emplace_back(-1);
            }
        }
        return collect;
    }
}

class Hopfield {
private:
    int len;
    int dimension;
    std::vector< std::vector<int> > samples;
    std::vector<int> answers;
    std::vector< std::vector<double> > weights;

    void initWeight() {
        this->weights.assign(this->dimension, std::vector<double>(this->dimension, 0.00));
        for(int i = 0;i < dimension; ++i) {
            for(int j = 0;j < i; ++j) {
                for(int k = 0;k < len; ++k) {
                    weights[i][j] = weights[i][j] + samples[k][i] * samples[k][j];
                }
                weights[i][j] /= dimension;
                weights[j][i] = weights[i][j];
            }
        }
    }

public:
    Hopfield(const int _d) : dimension(_d) {}

    void train() {
        std::ifstream in("./samples/count.txt");
        YHL::ON_EXIT_SCOPE([&]{
            in.close ();
        });
        int res;
        in >> this->len;
        this->answers.assign (this->len, 0);
        this->samples.assign(this->len, std::vector<int>(this->dimension, 0));
        for(int i = 0;i < this->len; ++i) {
            QString fileName = "./samples2/.png";
            fileName.insert (11, QString::number (i + 1));
            QImage image(fileName);
            this->samples[i] = getVector (image);
            in >> res;
            this->answers[i] = res;
        }
        this->initWeight ();
    }

    int recognize(const QImage& image) const {
        auto input = getVector(image);
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
        auto min = 1e12;
        auto best = -1;
        for(int i = 0;i < this->len; ++i) {
            auto distance = getHaiMing (this->samples[i], input);
            if(distance < min) {
                min = distance;
                best = this->answers[i];
            }
        }
        qDebug() << "distance  :  " << min << "\tbest  :  " << best;
        return best;
    }

    std::pair<QVector<double>, QVector<double> > getEfficiency(){
        std::pair<QVector<double>, QVector<double> > one;
        return one;
    }
};


#endif // HOPFIELD_H
