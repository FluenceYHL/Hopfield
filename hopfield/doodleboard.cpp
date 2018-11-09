#include "doodleboard.h"
#include "ui_doodleboard.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QImage>
#include <QPoint>
#include <QMatrix>
#include <QImageReader>
#include <QMessageBox>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include "scopeguard.hpp"
#include "hopfield.hpp"
#include "clusters.h"

namespace {
    void logCall(const QString& message) {
        QMessageBox::warning (nullptr, "警告!", message, QMessageBox::Yes);
    }
    QColor getColor() {
        static QList<QColor> colors{ Qt::red, Qt::black, Qt::blue, Qt::yellow, Qt::green, Qt::gray };
        if(!colors.empty ()) {
            auto one = colors.front ();
            colors.pop_front ();
            return one;
        }
        int a = rand() % 255, b = rand() % 255, c = rand() % 255;
        return qRgb(a, b, c);
    }
    std::vector<double> getVector(const QImage& image) {
        std::vector<double> collect;
        QRgb *oneLine = nullptr;
        const int height = image.height ();
        for(int i = 0;i < height; ++i) {
            oneLine = (QRgb*)image.scanLine (i);
            const int width = image.width ();
            for(int j = 0;j < width; ++j) {
                if(qRed(oneLine[j]) == 0 || qGreen (oneLine[j] == 0 || qBlue (oneLine[j]) == 0))
                    collect.emplace_back(1);
                else
                    collect.emplace_back(0);
            }
        }
        return collect;
    }

    QImage getImage(const QImage& image, const std::vector<QPoint>& travels) {
        int left = 1e5;
        int right = 0;
        int ceil = 1e5;
        int floor = 0;
        int meanX = 0, meanY = 0;
        for(const auto& it : travels) {
            if(left > it.x ()) left = it.x ();
            if(right < it.x ()) right = it.x ();
            if(ceil > it.y ()) ceil = it.y ();
            if(floor < it.y ()) floor = it.y ();
            meanX += it.x ();
            meanY += it.y ();
        }
        const int len = travels.size ();
        meanX /= len;
        meanY /= len;
        auto radius = std::max (right - left, floor - ceil) >> 1;
        QRect one(std::max (0, meanX - radius), std::max (0, meanY - radius - 25),
                  radius << 1, (radius << 1) + 40);
        return image.copy (one);
    }

    QImage getHandled(const QImage& image) {
        QRgb *oneLine = nullptr;
        const int height = image.height ();
        std::vector<QPoint> collect;
        for(int i = 0;i < height; ++i) {
            oneLine = (QRgb*)image.scanLine (i);
            const int width = image.width ();
            for(int j = 0;j < width; ++j) {
                if(qRed(oneLine[j]) == 0 || qGreen (oneLine[j] == 0 || qBlue (oneLine[j]) == 0))
                    collect.emplace_back(j, i);
            }
        }
        return getImage (image, collect);
    }
}

doodleBoard::doodleBoard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::doodleBoard)
{
    ui->setupUi(this);
    this->setWindowTitle ("YHL 涂鸦板");

    QFont font;
    font.setPointSize (100);
    ui->result->setFont (font);
    ui->result->setStyleSheet("background-color:#DDDDDD");
    ui->result->setAlignment (Qt::AlignCenter);
    ui->result->setWordWrap (true);

    pix = QPixmap(600, this->height ());
    pix.fill (Qt::white);
    this->initSave ();

    srand(static_cast<unsigned int>(time(nullptr)));
}

doodleBoard::~doodleBoard()
{
    delete ui;
    ui = nullptr;
    std::ofstream out("./samples/count.txt", std::ios::trunc);
    YHL::ON_EXIT_SCOPE([&]{
        out.close ();
    });
    out << this->sampleCnt << "\n";
    for(const auto it : this->answers) {
        out << it << "\n";
    }
}

void doodleBoard::initSave()
{
    this->numDialog = std::make_unique<number>();
    connect (this->numDialog.get (), &number::send_number, [&](const int arg){
        ++this->sampleCnt;
        this->answers.emplace_back (arg);
        QString path = "./samples/.png";
        path.insert (10, QString::number (sampleCnt));
        pix.save (path);
        QImage image("./handled.png");
        image.save (path.insert (9, '2'));
    });
    std::ifstream in("./samples/count.txt");
    YHL::ON_EXIT_SCOPE([&]{
        in.close ();
    });
    in >> this->sampleCnt;
    int res;
    for(int i = 0;i < this->sampleCnt; ++i) {
        in >> res;
        this->answers.emplace_back (res);
    }
}

void doodleBoard::paintEvent(QPaintEvent *)
{
    QPainter help(&pix);
    help.setPen (QPen(Qt::black, 18));
    help.drawLine (lastPoint, endPoint);
    lastPoint = endPoint;
    QPainter painter(this);
    painter.drawPixmap (0, 0, pix);
}

void doodleBoard::mousePressEvent(QMouseEvent *e)
{
    if(e->button () == Qt::LeftButton) {
        lastPoint = e->pos ();
    }
    endPoint = lastPoint;
}

void doodleBoard::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons () & Qt::LeftButton) {
        endPoint = e->pos ();
        static long long cnt = 0;
        if(++cnt % 3 == 0) {
            this->travels.emplace_back(endPoint);
            if(cnt >= std::numeric_limits<long long>::max ()) cnt = 0;
        }
        update ();
    }
}

void doodleBoard::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button () == Qt::LeftButton) {
        endPoint = e->pos (); update ();
        this->runBP ();
    }
}

void doodleBoard::closeEvent(QCloseEvent *)
{
    this->close ();
}

void doodleBoard::on_close_button_clicked()
{
    this->close ();
}

void doodleBoard::on_clear_button_clicked()
{
    pix.fill (Qt::white);
    update ();
    lastPoint.setX (0); lastPoint.setY (0);
    endPoint.setX (0); endPoint.setY (0);
    this->travels.clear ();
}

void doodleBoard::on_recognize_clicked()
{
    this->runBP ();
}

void doodleBoard::on_save_button_clicked()
{
    assert (this->numDialog);
    numDialog->show ();
}

void doodleBoard::on_train_button_clicked()
{
    if(this->being_trained == true) {
        logCall ("网络正在训练中"); return;
    }
    auto one = std::make_unique<Hopfield>();
    assert (one);
    one.swap(this->hopfield);
    this->trained = false;
    this->being_trained = true;
    this->trainThread = std::thread([&]{
//        QString path = "./dataSet/9.23.png";
//        QImage image(path);
//        QImage handled = getHandled (image).scaled (28, 28);
//        handled.save ("sample.png");
//        auto input = getVector (handled);
        this->trained = true;
        this->being_trained = false;
    });
    this->trainThread.detach ();
}

void doodleBoard::runBP()
{
    if(!this->hopfield) {
        logCall ("网络尚未训练 !"); return;
    }
    if(this->being_trained == true) {
        logCall ("网络正在训练中......"); return;
    }
    std::lock_guard<std::mutex> lck(mtx);

    this->pix.save ("./result.png");
    QImage image("./result.png");
    auto handled = getImage (image, this->travels).scaled (28, 28);
    handled.save ("./handled.png");

    QString oneFlock = "";
    ui->result->setText (oneFlock);
}

void doodleBoard::on_set_button_clicked()
{
    std::pair<QVector<double>, QVector<double> > one;
    QCustomPlot *gragh = new QCustomPlot();
    this->litters.emplace_back (gragh);
    gragh->addGraph (gragh->xAxis, gragh->yAxis);
    gragh->graph (0)->setName ("随着迭代次数增加，误差函数的变化");
    gragh->setInteractions (QCP::iRangeDrag | QCP::iRangeZoom);
    gragh->legend->setVisible (true);
    gragh->xAxis->setRange (0, 500);
    gragh->yAxis->setRange (0, 0.5);
    gragh->xAxis->setLabel ("迭代次数");
    gragh->yAxis->setLabel ("损失函数");
    gragh->show ();

    auto curGragh = gragh->addGraph ();
    curGragh->setPen (QPen(getColor (), 6));
    curGragh->setData (one.first, one.second, false);
    curGragh->setName ("损失函数曲线");
    gragh->replot ();
    gragh->savePng ("efficiency.png");
}
