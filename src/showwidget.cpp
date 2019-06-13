#include "showwidget.h"
#include <QHBoxLayout>



ShowWidget::ShowWidget(vision3d_core *pvision_core, QWidget *parent) : QOpenGLWidget(parent)
{

    m_vision_core = pvision_core;

    setFixedSize(600, 400);
    m_width = 1280;
    m_height = 720;

    rotate_X = 15.0;
    rotate_Y = 15.0;
    rotate_Z = -0.0;

//    m_xRot = 0;
//    m_yRot = 0;
//    m_zRot = 0;
//    m_xTranslate = 0.0;
//    m_yTranslate = 0.0;
//    m_zTranslate = -12.0;
//    m_xZoomScale = 1.0;
//    m_yZoomScale = 1.0;
//    m_zZoomScale = 1.0;

    setMouseTracking(true);
    //initializeGL();

    //imageLabel = new QLabel;
    //imageLabel->setScaledContents(true);
    //text = new QTextEdit;
    //QHBoxLayout *mainLayout = new QHBoxLayout(this);

    //mainLayout->addWidget(imageLabel);
    //mainLayout->addWidget(text);

    m_sceneChanged = false;
    m_renderImg = QImage();

    m_imgratio =  600.0f/400.0f;  // Default image ratio
    m_renderH = 0;
    m_renderW = 0;
    m_renderPosX = 0;
    m_renderPosY = 0;

    m_ModelName = "";
    m_Model.numSize = 0;
    m_Model.data = NULL;

}

ShowWidget::~ShowWidget()
{
}


void ShowWidget::initializeGL()
{
    makeCurrent();

    //initialize OpenGL
    initializeOpenGLFunctions();

    // Dark blue background
    glClearColor(0, 0, 0, 1);

    glShadeModel(GL_FLAT);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    //Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    glClear(GL_COLOR_BUFFER_BIT);

}

//paintGL 是当OpenGL Widget等待更新时被调用的一个函数
//该函数清除现有图像并调用renderImage 来高效地绘制图像

void ShowWidget::paintGL()
{
    makeCurrent();

    if(!m_sceneChanged)
    {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderImage();
    //m_sceneChanged = false;
}

void ShowWidget::resizeGL(int width, int height)
{
    makeCurrent();
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);//对投影矩阵应用随后的矩阵操作
    glLoadIdentity(); //重置当前指定的矩阵为单位矩阵,glLoadIdentity()之后我们为场景设置了透视图

    glOrtho(0, width, -height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);//设置当前矩阵为模型视图矩阵，

    //glOrtho(width, 0, height, 0, 0, 1);
    //glOrtho(0, width, 0, height, -1, 1); // To Draw image in the center of the area
    // // To Draw image in the center of the area
    recalculatePosition();

    emit imageSizeChanged(m_renderW, m_renderH);

    updateScene();
}

void ShowWidget::updateScene()
{
    if( m_sceneChanged && this->isVisible())
    {
        update();
    }
}


void ShowWidget::renderImage()
{
    m_Mutex.lock();

    makeCurrent();
    if(m_vision_core->getShowPattern() == show_2D)
    {
        //glDisable(GL_DEPTH_TEST);

        glClear(GL_COLOR_BUFFER_BIT);

        if (!m_renderImg.isNull())
        {
            glLoadIdentity();

            glPushMatrix();

            //if (m_resizeImg.width() <= 0)
            {
                if (m_renderW == m_renderImg.width() && m_renderH == m_renderImg.height())
                    m_resizeImg = m_renderImg;
                else
                    m_resizeImg = m_renderImg.scaled(QSize(m_renderW, m_renderH),
                                                     Qt::IgnoreAspectRatio,
                                                     Qt::SmoothTransformation);
            }

            glRasterPos2i(m_renderPosX, m_renderPosY);
            glPixelZoom(1, -1);//对图像进行缩放的函数
            glDrawPixels(m_resizeImg.width(), m_resizeImg.height(), GL_RGBA, GL_UNSIGNED_BYTE, m_resizeImg.bits());

            glPopMatrix();

            glFlush();   //end
        }
    }

    if(m_vision_core->getShowPattern() == show_3D_Model)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        //glTranslatef(m_xTranslate, m_yTranslate, m_zTranslate);
        //glScalef(m_xZoomScale, m_yZoomScale, m_zZoomScale);


        //glRotatef(m_xRot / 16.0, 1.0, 0.0, 0.0);
        //glRotatef(m_yRot / 16.0, 0.0, 1.0, 0.0);
        //glRotatef(m_zRot / 16.0, 0.0, 0.0, 1.0);

        gluPerspective(90, GLfloat(m_width) / m_height, 0.01f, 1000.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //glTranslatef(0,0,-Observer_distance);
        //glRotatef(Observer_angle_x,1,0,0);
        //glRotatef(Observer_angle_y,0,1,0);
        glTranslatef(0.0, 0.0, -m_zoom*30);
        glRotatef(rotate_X, 1.0, 0.0, 0.0);
        glRotatef(rotate_Y, 0.0, 1.0, 0.0);

        //glEnable(GL_DEPTH_TEST);
        //glEnable(GL_CULL_FACE);

        drawModel();
        //m_sceneChanged = true;
    }

    if(m_vision_core->getShowPattern() == show_3D_Scene)
    {
        glMatrixMode(GL_PROJECTION);
        //glPushMatrix();

        glLoadIdentity();

        //glTranslatef(m_xTranslate, m_yTranslate, m_zTranslate);
        //glScalef(m_xZoomScale, m_yZoomScale, m_zZoomScale);


        //glRotatef(m_xRot / 16.0, 1.0, 0.0, 0.0);
        //glRotatef(m_yRot / 16.0, 0.0, 1.0, 0.0);
        //glRotatef(m_zRot / 16.0, 0.0, 0.0, 1.0);

        gluPerspective(60, 1280 / 720, 0.01f, 1000.0f);
        //gluPerspective(60, GLfloat(m_width) / m_height, 0.01f, 1000.0f);
        //glOrtho(-m_width/2, m_width/2, -m_height/2, m_height/2, -10000, 10000.f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //glPushMatrix();
        gluLookAt(0, 0, 0, 0, 0, 1, 0, -1, 0);

        //glTranslatef(0,0,-Observer_distance);
        //glRotatef(Observer_angle_x,1,0,0);
        //glRotatef(Observer_angle_y,0,1,0);
        glTranslatef(0.0, 0.0, -m_zoom*10);
        glRotatef(rotate_X, 1.0, 0.0, 0.0);
        glRotatef(rotate_Y, 0.0, 1.0, 0.0);
        glTranslatef(0, 0, -0.5f);

        glPointSize(1280 / 600);
        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_CULL_FACE);

        drawScene();
    }

    m_Mutex.unlock();
    m_sceneChanged = true;
    update();
}

void ShowWidget::recalculatePosition()
{
    m_renderW = this->size().width();
    m_renderH = floor(m_renderW/m_imgratio);

    if (m_renderH > this->size().height())
    {
        m_renderH = this->size().height();
        m_renderW = floor(m_renderH * m_imgratio);
    }

    m_renderPosX = floor((this->size().width() - m_renderW) / 2);
    m_renderPosY = -floor((this->size().height() - m_renderH) / 2);

    m_resizeImg = QImage();
}

void ShowWidget::refreshImg2D(cv::Mat img)
{
    m_Mutex.lock();
    if(img.data)
    {
        m_renderImg = cvMat2QImage(img);
    }
    m_Mutex.unlock();
    m_sceneChanged = true;
    paintGL();
}

void ShowWidget::refreshImg3D(ivImage_3D img)
{
    m_Mutex.lock();
    if(img.data)
    {
        m_capturescene = img;
    }
    m_Mutex.unlock();
    m_sceneChanged = true;
    paintGL();
}

QImage ShowWidget:: cvMat2QImage(const cv::Mat mat)
{
    if(mat.data)
    {
        // 8-bits unsigned, NO. OF CHANNELS = 1
        if(mat.type() == CV_8UC1)
        {
            QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
            // Set the color table (used to translate colour indexes to qRgb values)
            image.setColorCount(256);
            for(int i = 0; i < 256; i++)
            {
                image.setColor(i, qRgb(i, i, i));
            }
            // Copy input Mat
            uchar *pSrc = mat.data;
            for(int row = 0; row < mat.rows; row ++)
            {
                uchar *pDest = image.scanLine(row);
                memcpy(pDest, pSrc, mat.cols);
                pSrc += mat.step;
            }
            return image;
        }
        // 8-bits unsigned, NO. OF CHANNELS = 3
        else if(mat.type() == CV_8UC3)
        {
            // Copy input Mat
            const uchar *pSrc = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }
        else if(mat.type() == CV_8UC4)
        {
            qDebug() << "CV_8UC4";
            // Copy input Mat
            const uchar *pSrc = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image.copy();
        }
        else
        {
            qDebug() << "ERROR: Mat could not be converted to QImage.";
            return QImage();
        }
    }
}


void ShowWidget::drawScene()
{
    glBegin(GL_POINTS);

    if (!m_capturescene.data || m_capturescene.numSize<=0)
        return;

    for (int i = 0; i < m_capturescene.numSize; i++)
    {
        glLoadName(i);

        GLfloat x = m_capturescene.data[i][0];
        GLfloat y = m_capturescene.data[i][1];
        GLfloat z = m_capturescene.data[i][2];
        glVertex3f(x,y,z);
    }
    glEnd();

}

void ShowWidget::drawModel()
{
    glBegin(GL_POINTS);

    if (!m_Model.data || m_Model.numSize<=0)
        return;

    for (int i = 0; i < m_Model.numSize; i++)
    {
        glLoadName(i);

        GLfloat x = m_Model.data[i][0];
        GLfloat y = m_Model.data[i][1];
        GLfloat z = m_Model.data[i][2];
        //glNormal3f(model.data[i][3], model.data[i][4], model.data[i][5]);
        glVertex3f(x,y,z);
    }
    glEnd();
}


void ShowWidget::qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void ShowWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_xRot)
    {
        rotate_X = angle;
        emit xRotationChanged(angle);
        //update();
    }
}

void ShowWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_yRot)
    {
        m_yRot = angle;
        emit yRotationChanged(angle);
        //update();
    }
}

void ShowWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_zRot)
    {
        m_zRot = angle;
        emit zRotationChanged(angle);
        //update();
    }
}

void ShowWidget::setXTranslate(double distance)
{
    m_xTranslate += distance / 1000.0;
}

void ShowWidget::setYTranslate(double distance)
{
   m_yTranslate -= distance / 1000.0;
}

void ShowWidget::setZTranslate(double distance)
{
   m_zTranslate += distance / 1000.0;
}

void ShowWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void ShowWidget::setViewZoomIn(double zoomScale)
{
    m_xZoomScale += zoomScale;
    m_yZoomScale += zoomScale;
    m_zZoomScale += zoomScale;
}

void ShowWidget::setViewZoomOut(double zoomScale)
{
    if (m_xZoomScale > 0.1)
    {
        m_xZoomScale -= zoomScale;
    }

    if (m_yZoomScale > 0.1)
    {
        m_yZoomScale -= zoomScale;
    }

    if (m_zZoomScale > 0.1)
    {
        m_zZoomScale -= zoomScale;
    }
}

void ShowWidget::wheelEvent(QWheelEvent *event)
{
    GLfloat delta = event->delta();
    if(delta>0)
    {
        m_zoom += 0.5;//1;
    }
    else
    {
        m_zoom -= 0.5;//1;
    }
    //updateGL();
    update();

//    if (event->delta() > 0)
//    {
//        emit setViewZoomIn(0.1);
//    }
//    else if (event->delta() < 0)
//    {
//        emit setViewZoomOut(0.1);
//    }



}

void ShowWidget::mouseMoveEvent(QMouseEvent *event)
{
    GLfloat dx = GLfloat(event->x() - m_lastPos.x()) / width();
    GLfloat dy = GLfloat(event->y() - m_lastPos.y()) / height();
    if(event->buttons() & Qt::LeftButton)
    {
        rotate_X += 180 * dy;
        rotate_Y += 180 * dx;
        //updateGL();
        update();
    }
    else if(event->buttons() & Qt::RightButton)
    {
        rotate_X += 180 * dy;
        rotate_Z += 180 * dx;
        //updateGL();
        update();
    }
    m_lastPos = event->pos();
//    int dx = event->x() - m_lastPos.x();
//    int dy = event->y() - m_lastPos.y();

//    if (event->buttons() & Qt::LeftButton)
//    {
//        setXRotation(m_xRot + 1 * dy);
//        setYRotation(m_yRot + 1 * dx);
//        setZRotation(m_zRot + 1 * dx);
//    }
//    else if (event->buttons() & Qt::RightButton)
//    {
//        setXTranslate(4 * dx);
//        setYTranslate(4 * dy);
//    }

//    m_lastPos = event->pos();
}

/*
void ShowWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    int face = faceAtPosition(event->pos());
    if(face != -1)
    {
        QColor color = QColorDialog::getColor(faceColors[face], this);
        if(color.isValid())
        {
            faceColors[face] = color;
            //updateGL();
            update();
        }
    }
}
*/

int ShowWidget::faceAtPosition(const QPoint &pos)
{
    const int MaxSize = 512;
    GLuint buffer[MaxSize];
    GLint viewport[4];
    makeCurrent();
    glGetIntegerv(GL_VIEWPORT, viewport);
    glSelectBuffer(MaxSize, buffer);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    //gluPickMatrix(GLdouble(pos.x()), GLdouble(viewport[3] - pos.y()), 5.0, 5.0, viewport);
    GLfloat x = GLfloat(width()) / height();
    glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    if(!glRenderMode(GL_RENDER))
        return -1;
    return buffer[3];
}

ivImage_3D ShowWidget::loadPLYSimple(QString fileName, int withNormals)
{

    FILE* ifs=NULL;
    int numVertices = 0, numCols = 3,has_normals = 0,t_size,i;

    char tokens[10][10]={0};
    char format[10]={0};
    char has_color=false;
    char has_alpha=false;
    char buf[100] = {0};
    char des[100] = {0};

    ifs = fopen(fileName.toStdString().c_str(),"r+");
    fgets(buf, 100, ifs);
    strncpy(des, buf, 10);

    while (strcmp(des,"end_header"))
    {
        t_size=sscanf(buf,"%s%s%s",&tokens[0],&tokens[1],&tokens[2]);
       if(t_size==3)
       {
            if (!strcmp(tokens[0],"property"))
           {
             if (!strcmp(tokens[2],"nx") || !strcmp(tokens[2],"normal_x"))
             {
               has_normals = -1;
               numCols += 3;
             }
             else if (!strcmp(tokens[2] ,"r") || !strcmp(tokens[2],"red"))
             {
               has_color = true;
               numCols += 3;
             }
             else if (!strcmp(tokens[2], "a") || !strcmp(tokens[2], "alpha"))
             {
               has_alpha = true;
               numCols += 1;
             }
            }
            else if (!strcmp(tokens[0], "format") )//&& strcmp(tokens[1], "ascii"))
            {
                strncpy(format,tokens[1],10);
                //printf("%s","Cannot read file, only ascii ply format is currently supported...");
                //return ;

            }
            else if (!strcmp(tokens[0],"element") && !strcmp(tokens[1],"vertex"))
            {
              numVertices = atoi(tokens[2]);
            }

         }
      fgets(buf,100,ifs);
      strncpy(des,buf,10) ;
    }

    withNormals &= has_normals;

    ivImage_3D cloud;
    cloud.data = (ivPoint_3D*)malloc(numVertices*sizeof(ivPoint_3D));
    cloud.numSize = numVertices;

    for (int i = 0; i < numVertices; i++)
    {
        double x[6]={0};
        fgets(buf,100,ifs);

        if((sscanf(buf, "%lf%lf%lf%lf%lf%lf", &x[0], &x[1], &x[2],&x[3], &x[4], &x[5])==3)&&!withNormals)
        {
            cloud.data[i][0]=x[0];
            cloud.data[i][1]=x[1];
            cloud.data[i][2]=x[2];
        }
        else if((sscanf(buf, "%lf%lf%lf%lf%lf%lf", &x[0], &x[1], &x[2],&x[3], &x[4], &x[5]) == 6)&&withNormals)
        {
            cloud.data[i][0]=x[0];
            cloud.data[i][1]=x[1];
            cloud.data[i][2]=x[2];
            cloud.data[i][3]=x[3];
            cloud.data[i][4]=x[4];
            cloud.data[i][5]=x[5];
            double norm = sqrt(cloud.data[i][3]*cloud.data[i][3] + cloud.data[i][4]*cloud.data[i][4] + cloud.data[i][5]*cloud.data[i][5]);
            if (norm>0.00001)
            {
                cloud.data[i][3]/=(float)(norm);
                cloud.data[i][4]/=(float)(norm);
                cloud.data[i][5]/=(float)(norm);
            }
        }
    }

    return cloud;
}
