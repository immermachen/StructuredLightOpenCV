#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/opencv.hpp"
#include "Capturador.h"
#include "Decoder.h"
#include "CameraCalibrator.h"
#include "MatrixUtil.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    COptions* options = new COptions(1024, 768, 10,4,true,true,true,false);
    string ruta = "Patterns/pattern-0";
    CCameraCalibrator* cal = new CCameraCalibrator();
    Mat dist, camera;
    //cal->Calibrate("..//resources//in_calib.xml", camera, dist);
    cal->GetCalibrationFromFile("..//resources//out_camera_data3.xml", camera, dist);
    cout.setf(ios::fixed,ios::floatfield);
    cout.precision(3);
    cout<<endl<<Mat(camera)<<endl;
    //camera.at<double>(0,0) = camera.at<double>(0,0)*0.8;
    //camera.at<double>(1,1) = camera.at<double>(1,1)*0.8;
    //cout<<endl<<Mat(camera)<<endl;
    CCapturador* cap = new CCapturador(options,ruta);

    //cap->CapturePatternsUndisorted(camera,dist,250);

    cap->LoadCapturesFromFilesUndisorted("..//resources//Captures/V4GP/Capture-",camera,dist);
    //cap->LoadCapturesFromFiles("..//resources//Captures/V4GP/Capture-");
    CDecoder* decoder = new CDecoder(options, cap->m_vCaptures);
    decoder->Decode();
    if(decoder->Calibrate(camera,dist))
        if(!decoder->Generate3Dpoints(camera,dist))
            cout<<"Error no se pudo generar nube de puntos."<<endl;
    cvWaitKey();
}

MainWindow::~MainWindow()
{
    delete ui;
}
