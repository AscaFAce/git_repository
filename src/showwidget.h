#ifndef SHOWWIDGET_H
#define SHOWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QImage>
#include <QMouseEvent>
#include <QAction>
#include <QColorDialog>
#include <QPainter>
#include <QOpenGLWidget>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <iostream>
#include <mutex>

#include "vision3d_vars.h"
#include "vision3d_core.h"
#include "rs2_thread.h"


class vision3d_core;
class ShowWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:

    ShowWidget(vision3d_core *pvision_core, QWidget *parent = nullptr);
    ~ShowWidget();

protected:
    //对三个纯虚函数的重定义
    void initializeGL() override;//建立OpenGL的资源和状态
    void paintGL() override;//渲染OpenGL场景，更新Widget时调用
    void resizeGL(int width, int height) override;//设置OpenGL视口，投影等


    //void paintEvent(QPaintEvent *e);
    //鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event);

    void updateScene();
    void qNormalizeAngle(int &angle);

    //void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void renderImage();
    void drawModel();
    void drawScene();
    int  faceAtPosition(const QPoint &pos);
    void recalculatePosition();
    ivImage_3D loadPLYSimple(QString fileName, int withNormals);

    bool    m_sceneChanged;     // Indicates when OpenGL view is to be redrawn
    QImage  m_renderImg;        // Qt image to be rendered
    QImage  m_resizeImg;        // the image rendered
    cv::Mat m_srcImg;
    float   m_imgratio;         // height/width ratio

    int m_renderH; // Resized Image height
    int m_renderW; // Resized Image width
    int m_renderPosX; // Top left X position to render image in the center of widget
    int m_renderPosY; // Top left Y position to render image in the center of widget

    vision3d_core   *m_vision_core;

    GLfloat rotate_X;
    GLfloat rotate_Y;
    GLfloat rotate_Z;
    GLfloat m_zoom;
    QColor  faceColors[4];


    int     m_width;
    int     m_height;
    int     m_xRot;
    int     m_yRot;
    int     m_zRot;
    double  m_xTranslate;
    double  m_yTranslate;
    double  m_zTranslate;
    double  m_xZoomScale;
    double  m_yZoomScale;
    double  m_zZoomScale;

    bool    m_bShowAxis;
    bool    m_bUseRgbComponent;

    //ShowPattern m_pattern;
    QPoint      m_lastPos;
    ivImage_3D  m_Model;
    ivImage_3D  m_capturescene;
    QString     m_ModelName;
    QAction     *mouseAction;

    std::mutex  m_Mutex;


public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setXTranslate(double distance);
    void setYTranslate(double distance);
    void setZTranslate(double distance);
    void setViewZoomIn(double zoomScale);
    void setViewZoomOut(double zoomScale);


signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void imageSizeChanged(int outW, int outH );  // Used to resize the image outside the widget

public:
    void LoadModel(QString fileName) { m_Model = loadPLYSimple(fileName, 1);}
    void Load2DImage(QString fileName) {m_renderImg.load(fileName);}
    void refreshImg2D(cv::Mat img);
    void refreshImg3D(ivImage_3D img);
    void setSceneModel(ShowPattern pattern);

    QImage cvMat2QImage(const cv::Mat mat);

private:
     rs2_thread  *m_rs2thread;

};

#endif // SHOWWIDGET_H
