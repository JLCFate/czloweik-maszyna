#include "videothread.h"

videothread::videothread(QObject *parent) :
    QThread(parent)
{
    vidFromFile = false;

    ismoving = false;
    frame_count = 0;
}
//INICJALIZACJA WIDEO AVI
void videothread::videoInit(QString fName)
{
    //capture.open(fname.toStdString());

    //capture.open(fName.toUtf8().constData());
    capture.open(0);
    if(capture.isOpened() == false)
    {
        qDebug()<<"No camera";
        return;
    }

    //vidFromFile = true;
    this->start();
}

//Star naszego watku
void videothread::run()
{
    while(1)
    {
        //QMutex mutex;
        //mutex.lock();

        //mutex.unlock();
        cv::Mat frame, frame_gray, img;
        //capture.read(input_img);
        capture.read(input_img);
        //paly video in the loop

        if(vidFromFile)
        {

            frame_count++;
            if(frame_count == capture.get(cv::CAP_PROP_FRAME_COUNT)-1)
            {
                capture.set(cv::CAP_PROP_POS_FRAMES, 0);
                frame_count = 0;
            }
        }
        if(input_img.empty() == true) return;


        //TU WPISUJEMY WSZYSTKO CO DZIEJE SIE Z NASZYM OBRAZEM Z KAMERY

        //Segmentacja koloru skóry
        cv::Mat skinMask;
        cv::Rect rect;
        std::vector<cv::Point> hull_points;
        std::vector<cv::Point> def_points;

        //Wartości dla koloru zielonego

        //min: 40 59 76
        //max: 98 154 236

        //skin_segmentation(input_img, skinMask, cv::Scalar(40,59,76), cv::Scalar(98,154,236));
        skin_segmentation(input_img, skinMask, min_val, max_val);
        morphology(skinMask);

        std::vector<cv::Rect> boxy;

        marker_detection(skinMask, boxy);


        //Detekcja wkleslosci i wypuklosci w konturze
        //feature_detection(skinMask, rect, hull_points, def_points);

        //prosta detekcja gestu otwarte-zamkniete
        //gesture_detection(rect, hull_points, def_points);
        double avg_area = 0;

        if(boxy.size()>0)
        {
            for(int i=0;i<boxy.size();i++)
            {
                avg_area+=boxy[i].area();
                cv::rectangle(input_img, boxy[i], cv::Scalar(0,255,0), 5, 8, 0);
            }
            if(prev_x.size()<10)
            {
                prev_x.push_back(boxy[0].tl());
            }
            else
            {
                for(int i=0;i<prev_x.size()-1;i++)
                {
                    double res = cv::norm(prev_x[i+1] - prev_x[i]);
                    if(res > 5)
                    {
                        ismoving = true;
                    }
                    else
                    {
                        ismoving = false;
                    }
                }
                prev_x.clear();
            }
            if(ismoving)
            {
                gesture.push_back(boxy[0].tl());
                //qDebug()<<"TRUE";
                //qDebug()<<boxy[0].x<<";"<<boxy[0].y;
            }
            else
            {
                if(gesture.size()>5)
                {
                    qDebug()<<"EMITED";
                    emit emitGesture(gesture);
                }
                gesture.clear();
            }
        }
        else
        {
            if(gesture.size()>5)
            {
                qDebug()<<"EMITED";
                emit emitGesture(gesture);
                gesture.clear();
            }
        }
        //avg_area=cvRound(avg_area/boxy.size());
        //qDebug()<<avg_area;
        //zaznaczenie cech na obrazie
        //draw_features(input_img, rect, hull_points, def_points);

        //msleep(1000/capture.get(cv::CAP_PROP_FPS));
        //Konwersja do QImage

        //QImage qimginput = qinputimage(img);
        //Emisja sygnału do watku głównego
        QImage qimginput;
        if(img_no == 1)
        {
            qimginput = qinputimage(skinMask);
        }
        else
        {
            qimginput = qinputimage(input_img);
        }
        emit NewCamImg(qimginput);
    }
}

QImage videothread::qinputimage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void videothread::getScalarMin(cv::Scalar s1)
{
    min_val = s1;
    qDebug()<<s1.val[0]<<";"<<s1.val[1]<<";"<<s1.val[2];
}

void videothread::getScalarMax(cv::Scalar s1)
{
    max_val = s1;
    qDebug()<<s1.val[0]<<";"<<s1.val[1]<<";"<<s1.val[2];
}

void videothread::morphology(cv::Mat &mask)
{
    cv::erode(mask, mask, cv::Mat());
    cv::erode(mask, mask, cv::Mat());
    cv::erode(mask, mask, cv::Mat());
    cv::erode(mask, mask, cv::Mat());

    cv::dilate(mask, mask, cv::Mat());
    cv::dilate(mask, mask, cv::Mat());
}

void videothread::marker_detection(cv::Mat &mask, std::vector<cv::Rect> &boxy)
{
    //detekcja konturow
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat m = mask.clone();
    cv::Mat drawing = input_img.clone();
    cv::findContours(m, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    double maxS = 0;
    //int ci = 1;
    cv::Rect rect1;
    for (int i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if(area>maxS)
        {
            maxS = area;
            //ci = i;
        }
    }

    if(maxS>0)
    {
        for (int i = 0; i < contours.size(); i++)
        {
            cv::Rect rect;
            double area = cv::contourArea(contours[i]);
            if(area>0.3*maxS)
            {
                boxy.push_back(cv::boundingRect(contours[i]));
            }
        }
    }
}

void videothread::skin_segmentation(cv::Mat img, cv::Mat &mask, cv::Scalar min, cv::Scalar max)
{
    cv::Mat hsvimg;
    cv::cvtColor(img,hsvimg, CV_BGR2HSV);

    //rozmycie gaussowskie
    cv::GaussianBlur(hsvimg, hsvimg, cv::Size(5,5), 5, 5);

    //progowanie koloru skory
    //cv::Scalar(0,60,130), cv::Scalar(24, 255, 255)
    cv::inRange(hsvimg, min, max, mask);

}

void videothread::feature_detection(cv::Mat mask, cv::Rect &rect, std::vector<cv::Point> &hull_points, std::vector<cv::Point> &def_points)
{
    //detekcja konturow
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat m = mask.clone();
    cv::Mat drawing = input_img.clone();
    cv::findContours(m, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    double maxS = 0;
    int ci = 1;
    cv::Rect rect1;
    for (int i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if(area>maxS)
        {
            maxS = area;
            ci = i;
        }
    }

    //estymacja powierzchni konturu prostokątem
    if(maxS>0)
    {
        rect = cv::boundingRect(contours[ci]); // Bounding box
    }

    //qDebug()<<ci;
    //detekcja wypukłości i wklęsłości konturu
    std::vector<std::vector<cv::Point> >hull( contours.size() );
    std::vector<std::vector<int> >hullI( contours.size() );
    std::vector<std::vector<int> >hullI_sort( contours.size() );

    std::vector<cv::Vec4i> defects;

    if(!contours.empty())
    {


        cv::convexHull(cv::Mat(contours[ci]), hull[ci], false, true);
        cv::convexHull(cv::Mat(contours[ci]), hullI[ci], false);

        try {
            cv::convexityDefects(cv::Mat(contours[ci]), hullI[ci], defects);
        }
        catch (cv::Exception& e) {
            qDebug()<<e.what();
        }
        int nm1 = 20;
        for(int i=0; i<hull[ci].size()-1;i++)
        {
            double d = dist(hull[ci][i], hull[ci][i+1]);

            if(d<nm1)
            {
                hull_points.push_back(hull[ci][i]);
            }
        }

        for(int i=0;i<defects.size();i++)
        {
            int ind = defects[i][2];
            float depth = defects[i][3];

            if(depth > 10*256)
            {
                def_points.push_back(contours[ci][ind]);
            }
        }
    }

}

void videothread::gesture_detection(cv::Rect rect, std::vector<cv::Point> hull_points, std::vector<cv::Point> def_points)
{
    //prosta detekcja gestów
    std::vector<double> dist_hull;
    std::vector<double> dist_deff;

    double avg_dist_hull = 0, avg_dist_deff = 0;

    cv::Point hand_c;
    hand_c.x = rect.x + rect.width/2;
    hand_c.y = rect.y + rect.height/2;
    //cv::circle(drawing, hand_c, 25, CV_RGB(255,0,0), 8, -1);

    for(int i=0;i<hull_points.size();i++)
    {
        double d = dist(hand_c, hull_points[i]);
        dist_hull.push_back(d);
        avg_dist_hull +=d;
    }

    for(int i=0;i<def_points.size();i++)
    {
        double d = dist(hand_c, def_points[i]);
        dist_deff.push_back(d);
        avg_dist_deff +=d;
    }

    avg_dist_hull = avg_dist_hull/dist_hull.size();
    avg_dist_deff = avg_dist_deff/dist_deff.size();

    if(avg_dist_deff/avg_dist_hull >0.7)
    {
        //CLOSE
        qDebug()<<"CLOSE";
    }
    else
    {
        //OPEN
        qDebug()<<"OPEN";
    }
}

void videothread::draw_features(cv::Mat img, cv::Rect rect, std::vector<cv::Point> hull_points, std::vector<cv::Point> def_points)
{
    cv::rectangle(img, rect, CV_RGB(255,255,255), 3, 8, 0);


    for(int i=0;i<def_points.size();i++)
    {
        cv::circle(img,def_points[i],15,cv::Scalar(255,255,255),-1);
    }

    int nm1 = 20;
    for(int i=0;i<hull_points.size()-1;i++)
    {
        double d = dist(hull_points[i], hull_points[i+1]);

        if(d>=nm1)
        {
            cv::circle(img, hull_points[i],15,cv::Scalar( 0, 255, 0 ), 3);
        }
    }
}

double videothread::dist(cv::Point p1, cv::Point p2)
{
    double dist = sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
    return dist;
}
