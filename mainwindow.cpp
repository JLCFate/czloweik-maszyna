#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<std::vector<cv::Point2f>>();

    camImg = new videothread(this);

    //połączenie sygnał slot
    connect(camImg, SIGNAL(NewCamImg(QImage)), this, SLOT(onNewCamImg(QImage)));
    connect(camImg, SIGNAL(emitGesture(std::vector<cv::Point2f>)), this, SLOT(onEmitGesture(std::vector<cv::Point2f>)));

    connect(this, SIGNAL(setScalarMin(cv::Scalar)), camImg, SLOT(getScalarMin(cv::Scalar)));
    connect(this, SIGNAL(setScalarMax(cv::Scalar)), camImg, SLOT(getScalarMax(cv::Scalar)));

    savePattern = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewCamImg(QImage qimg)
{
    ui->img->setPixmap(QPixmap::fromImage(qimg).scaled(ui->img->width(),ui->img->height()));
}

void MainWindow::onEmitGesture(std::vector<cv::Point2f> g)
{
    for(int i=0;i<g.size();i++)
    {
        qDebug()<<g[i].x<<";"<<g[i].y;
    }
    gesture = g;
    if(savePattern)
    {
        try
        {
            cv::Mat corr;
            cv::matchTemplate(pattern, gesture, corr, cv::TM_CCOEFF_NORMED);
        //qDebug()<<corr.at<float>(0,0);
            ui->label->setText(QString::number(corr.at<float>(0,0)));
        }
        catch (cv::Exception& e) {
            qDebug()<<e.what();
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video"), "../", tr("Image Files (*.mp4 *.avi *.wav)"));
    camImg->videoInit("fileName");
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    hsvmin.val[0] = value;
    ui->val_h1->setNum(value);
    // = cv::Scalar(hmin, smin, vmin);
    emit setScalarMin(hsvmin);
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    hsvmin.val[1] = value;
    ui->val_s1->setNum(value);
    smin = value;
    emit setScalarMin(hsvmin);
}

void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    hsvmin.val[2] = value;
    ui->val_v1->setNum(value);
    vmin = value;
    emit setScalarMin(hsvmin);
}

void MainWindow::on_horizontalSlider_4_valueChanged(int value)
{
    hsvmax.val[0] = value;
    ui->val_h2->setNum(value);
    emit setScalarMax(hsvmax);
}

void MainWindow::on_horizontalSlider_5_valueChanged(int value)
{
    hsvmax.val[1] = value;
    ui->val_s2->setNum(value);
    emit setScalarMax(hsvmax);
}

void MainWindow::on_horizontalSlider_6_valueChanged(int value)
{
    hsvmax.val[2] = value;
    ui->val_v2->setNum(value);
    emit setScalarMax(hsvmax);
}

void MainWindow::on_pushButton_2_clicked()
{
    savePattern = true;
    pattern = gesture;
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    camImg->img_no = arg1;
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->horizontalSlider->setValue(22);
    ui->horizontalSlider_2->setValue(33);
    ui->horizontalSlider_3->setValue(22);

    ui->horizontalSlider_4->setValue(63);
    ui->horizontalSlider_5->setValue(231);
    ui->horizontalSlider_6->setValue(231);
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->horizontalSlider->setValue(107);
    ui->horizontalSlider_2->setValue(56);
    ui->horizontalSlider_3->setValue(31);

    ui->horizontalSlider_4->setValue(250);
    ui->horizontalSlider_5->setValue(250);
    ui->horizontalSlider_6->setValue(255);
}

void MainWindow::on_pushButton_5_clicked()
{

}
