#ifndef FRACTORIUM_PCH_H
#define FRACTORIUM_PCH_H//GCC doesn't like #pragma once

#define XFORM_COLOR_COUNT 14

#ifdef _WIN32
	#pragma warning(disable : 4251; disable : 4661; disable : 4100)
#endif

//Has to come first on non-Windows platforms due to some weird naming collisions on *nix.
#ifndef _WIN32
    #include <QtWidgets>
#endif

#include "Renderer.h"
#include "RendererCL.h"
#include "VariationList.h"
#include "OpenCLWrapper.h"
#include "XmlToEmber.h"
#include "EmberToXml.h"
#include "SheepTools.h"
#include "JpegUtils.h"
#include "EmberCommon.h"

#ifdef _WIN32
	#include <QtWidgets>
#endif

#include <math.h>
#include <deque>
#include "qfunctions.h"

#include <QApplication>
#include <QBrush>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QConicalGradient>
#include <QDebug>
#include <QDial>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDialog>
#include <QFontMetrics>
#include <QFrame>
#include <QFuture>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHash>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLinearGradient>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QModelIndex>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPixmap>
#include <QPoint>
#include <QPolygon>
#include <QPushButton>
#include <QRect>
#include <QResizeEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QSize>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QtCore/qglobal.h>
#include <QtCore/QMultiHash>
#include <QtCore/QPair>
#include <QtCore/QSharedData>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QTextEdit>
#include <QTextStream>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtGui/QSyntaxHighlighter>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QTreeWidget>
#include <QtWidgets/QMainWindow>
#include <QVarLengthArray>
#include <QVBoxLayout>
#include <QVector>
#include <QWheelEvent>
#include <QWidget>
#include <QWidgetAction>

//#define GLM_FORCE_RADIANS 1
//#define GLM_ENABLE_EXPERIMENTAL 1

//#ifndef __APPLE__
//	#define GLM_FORCE_INLINE 1
//#endif
//
//#include "glm/glm.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//#include "glm/gtc/type_ptr.hpp"

#ifndef _WIN32
	#undef Bool
#endif

using namespace std;
using namespace EmberNs;
using namespace EmberCLns;
using namespace EmberCommon;

#endif
