#ifndef DOODLEBOARD_H
#define DOODLEBOARD_H
#include "qcustomplot.h"
#include <QMainWindow>
#include <QMouseEvent>
#include <memory>
#include <number.h>
#include <unordered_map>
#include <QTimerEvent>
#include <vector>
#include <thread>
#include <mutex>

namespace Ui {
class doodleBoard;
}
namespace YHL {
class Hierarchical;
}
class Hopfield;

class doodleBoard : public QMainWindow
{
    Q_OBJECT
public:
    explicit doodleBoard(QWidget *parent = nullptr);
    ~doodleBoard();

private:
    void initSave ();

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void closeEvent(QCloseEvent*);

private:
    void runBP();

private slots:
    void on_close_button_clicked();

    void on_clear_button_clicked();

    void on_recognize_clicked();

    void on_save_button_clicked();

    void on_train_button_clicked();

    void on_set_button_clicked();

private:
    // 神经网络相关
    std::unique_ptr<Hopfield> hopfield;
    bool trained = false;
    bool being_trained = false;

    // 线程辅助
    std::thread trainThread;
    std::mutex mtx;

    // 绘图相关
    Ui::doodleBoard *ui;
    QPixmap pix;
    QPoint lastPoint;
    QPoint endPoint;
    std::list<QCustomPlot*> litters;

    // 收集样本
    int sampleCnt;
    std::vector<int> answers;
    std::vector<QPoint> travels;      // 轨迹
    std::unique_ptr<number> numDialog;// 保存样本的窗口
};

#endif // DOODLEBOARD_H
