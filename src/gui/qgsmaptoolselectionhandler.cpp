/***************************************************************************
    qgsmaptoolselectionhandler.cpp
    ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Viktor Sklencar
    email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaplayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"
#include "qgscoordinateutils.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsmaptoolselectutils.h"

#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>
#include <QMenu>
#include <qlabel.h>
#include <qgsdoublespinbox.h>
#include <qgisinterface.h>

class QgisInterface;
class QgsMapToolIdentifyAction;


QgsDistanceWidget::QgsDistanceWidget( const QString &label, QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QHBoxLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  mLayout->setAlignment( Qt::AlignLeft );
  setLayout( mLayout );

  if ( !label.isEmpty() )
  {
    QLabel *lbl = new QLabel( label, this );
    lbl->setAlignment( Qt::AlignRight | Qt::AlignCenter );
    mLayout->addWidget( lbl );
  }

  mDistanceSpinBox = new QgsDoubleSpinBox( this );
  mDistanceSpinBox->setSingleStep( 1 );
  mDistanceSpinBox->setValue( 0 );
  mDistanceSpinBox->setMinimum( 0 );
  mDistanceSpinBox->setMaximum( 1000000000 );
  mDistanceSpinBox->setDecimals( 6 );
  mDistanceSpinBox->setShowClearButton( false );
  mDistanceSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mLayout->addWidget( mDistanceSpinBox );

  // connect signals
  mDistanceSpinBox->installEventFilter( this );
  connect( mDistanceSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsDistanceWidget::distanceSpinBoxValueChanged );

  // config focus
  setFocusProxy( mDistanceSpinBox );
}

void QgsDistanceWidget::setDistance( double distance )
{
  mDistanceSpinBox->setValue( distance );
}

double QgsDistanceWidget::distance()
{
  return mDistanceSpinBox->value();
}

bool QgsDistanceWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mDistanceSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit distanceEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit distanceEditingFinished( distance(), event );
      return true;
    }
  }

  return false;
}

void QgsDistanceWidget::distanceSpinBoxValueChanged( double distance )
{
  emit distanceChanged( distance );
}

QgsMapToolSelectionHandler::QgsMapToolSelectionHandler( QgsMapCanvas *canvas, QgsMapToolSelectionHandler::SelectionMode selectionMode )
  : QObject()
  , mSelectionMode( selectionMode )
{
  mCanvas = canvas;
  mFillColor = QColor( 254, 178, 76, 63 );
  mStrokeColor = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectionHandler::~QgsMapToolSelectionHandler()
{
  deleteRotationWidget();
}

void QgsMapToolSelectionHandler::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      selectFreehandReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      selectRadiusReleaseEvent( e );
      break;
  }
}


void QgsMapToolSelectionHandler::canvasMoveEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      selectFreehandMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      selectRadiusMoveEvent( e );
      break;
  }
}

void QgsMapToolSelectionHandler::selectFeaturesPressEvent()
{
  if ( selectionRubberBand() )
    selectionRubberBand()->reset();
  initRubberBand();
}

void QgsMapToolSelectionHandler::canvasPressEvent( QgsMapMouseEvent *e )
{
  mInitDragPos = e -> pos();
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesPressEvent();
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonPressEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      break;
  }
}

bool QgsMapToolSelectionHandler::escapeSelection( QKeyEvent *e )
{
  if ( mSelectionActive && e->key() == Qt::Key_Escape )
  {
    selectionRubberBand()->reset();
    mSelectionActive = false;
    deleteRotationWidget();
    return true;
  }
  return false;
}

void QgsMapToolSelectionHandler::deactivate()
{
  deleteRotationWidget();
  selectionRubberBand()->reset();
  mSelectionActive = false;
}

void QgsMapToolSelectionHandler::selectFeaturesMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  QRect rect;
  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    rect = QRect( e->pos(), e->pos() );
  }
  else
  {
    rect = QRect( e->pos(), mInitDragPos );
  }
  QgsMapToolSelectUtils::setRubberBand( mCanvas, rect, selectionRubberBand() );
}

void QgsMapToolSelectionHandler::selectFeaturesReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectFeatures = false;
  QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionActive = false;
    mSelectFeatures = true;
    setSelectedGeometry( QgsGeometry::fromPointXY( toMapCoordinates( e ->pos() ) ), e );
  }

  if ( selectionRubberBand() && mSelectionActive )
  {
    mSelectFeatures = true;
    setSelectedGeometry( selectionRubberBand()->asGeometry(), e );
    selectionRubberBand()->reset();
    delete selectionRubberBand();
    setSelectionRubberBand( nullptr );
  }

  mSelectionActive = false;
}

QgsPointXY QgsMapToolSelectionHandler::toMapCoordinates( QPoint point )
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates( point );
}

void QgsMapToolSelectionHandler::selectPolygonMoveEvent( QgsMapMouseEvent *e )
{
  if ( !selectionRubberBand() )
    return;

  if ( selectionRubberBand()->numberOfVertices() > 0 )
  {
    selectionRubberBand()->movePoint( this->toMapCoordinates( e->pos() ) );
  }
}

void QgsMapToolSelectionHandler::selectPolygonPressEvent( QgsMapMouseEvent *e )
{
  mSelectFeatures = false;
  if ( !selectionRubberBand() )
  {
    initRubberBand();
  }
  if ( e->button() == Qt::LeftButton )
  {
    selectionRubberBand()->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( selectionRubberBand()->numberOfVertices() > 2 )
    {
      mSelectFeatures = true;
      setSelectedGeometry( selectionRubberBand()->asGeometry(), e );
    }
    selectionRubberBand()->reset();
    delete selectionRubberBand();
    setSelectionRubberBand( nullptr );
    mJustFinishedSelection = mSelectionActive;
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectFreehandMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive || !selectionRubberBand() )
    return;

  selectionRubberBand()->addPoint( toMapCoordinates( e->pos() ) );
}

void QgsMapToolSelectionHandler::selectFreehandReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectFeatures = false;
  if ( !mSelectionActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !selectionRubberBand() )
    {
      initRubberBand();
    }
    else
    {
      selectionRubberBand()->reset( QgsWkbTypes::PolygonGeometry );
    }
    selectionRubberBand()->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( selectionRubberBand() && selectionRubberBand()->numberOfVertices() > 2 )
      {
        mSelectFeatures = true;
        setSelectedGeometry( selectionRubberBand()->asGeometry(), e );
      }
    }

    selectionRubberBand()->reset();
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectRadiusMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();

  if ( !mSelectionActive )
  {
    mSelectFeatures = true;
    return;
  }

  if ( !selectionRubberBand() )
  {
    initRubberBand();
    mSelectFeatures = false;
  }

  updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolSelectionHandler::selectRadiusReleaseEvent( QgsMapMouseEvent *e )
{

  if ( e->button() == Qt::RightButton )
  {
    mSelectionActive = false;
    mSelectFeatures = false;
    deleteRotationWidget();
    selectionRubberBand()->reset();
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    mSelectFeatures = true;
    mRadiusCenter = e->snapPoint();
    createRotationWidget();
  }
  else
  {
    if ( !selectionRubberBand() )
    {
      initRubberBand();
    }
    mSelectionActive = false;
    mSelectFeatures = true;
    setSelectedGeometry( selectionRubberBand()->asGeometry(), e );
    selectionRubberBand()->reset( QgsWkbTypes::PolygonGeometry );
    deleteRotationWidget();
  }
}


void QgsMapToolSelectionHandler::initRubberBand()
{
  delete mSelectionRubberBand;
  mSelectionRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectionRubberBand->setFillColor( mFillColor );
  mSelectionRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolSelectionHandler::setIface( QgisInterface *iface )
{
  mQgisInterface = iface;
}

void QgsMapToolSelectionHandler::createRotationWidget()
{
  if ( !mCanvas || !mQgisInterface )
  {
    return;
  }

  deleteRotationWidget();

  mDistanceWidget = new QgsDistanceWidget( QStringLiteral( "Selection radius:" ) );
  if ( mQgisInterface )
  {
    mQgisInterface->addUserInputWidget( mDistanceWidget );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
  }

  connect( mDistanceWidget, &QgsDistanceWidget::distanceChanged, this, &QgsMapToolSelectionHandler::updateRubberband );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingFinished, this, &QgsMapToolSelectionHandler::radiusValueEntered );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingCanceled, this, &QgsMapToolSelectionHandler::cancel );
}

void QgsMapToolSelectionHandler::deleteRotationWidget()
{
  if ( mDistanceWidget )
  {
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceChanged, this, &QgsMapToolSelectionHandler::updateRubberband );
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceEditingFinished, this, &QgsMapToolSelectionHandler::radiusValueEntered );
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceEditingCanceled, this, &QgsMapToolSelectionHandler::cancel );
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}

void QgsMapToolSelectionHandler::selectFromRubberband( const Qt::KeyboardModifiers &modifiers )
{
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, selectionRubberBand()->asGeometry(), modifiers, mQgisInterface->messageBar() );
  mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  deleteRotationWidget();
  mSelectionActive = false;
  mSelectFeatures = true;
}

void QgsMapToolSelectionHandler::radiusValueEntered( const double &radius, QKeyEvent *event )
{
  updateRubberband( radius );
  setSelectedGeometry( selectionRubberBand()->asGeometry(), event );
  selectionRubberBand()->reset();
  deleteRotationWidget();
  mSelectionActive = false;
}

void QgsMapToolSelectionHandler::cancel()
{
  deleteRotationWidget();
  selectionRubberBand()->reset();
  mSelectionActive = false;
}

QPoint QgsMapToolSelectionHandler::initDragPos() const
{
  return mInitDragPos;
}

void QgsMapToolSelectionHandler::setSelectionRubberBand( QgsRubberBand *selectionRubberBand )
{
  mSelectionRubberBand = selectionRubberBand;
}

QgsPointXY QgsMapToolSelectionHandler::radiusCenter() const
{
  return mRadiusCenter;
}

bool QgsMapToolSelectionHandler::justFinishedSelection() const
{
  return mJustFinishedSelection;
}

void QgsMapToolSelectionHandler::setJustFinishedSelection( bool justFinishedSelection )
{
  mJustFinishedSelection = justFinishedSelection;
}

void QgsMapToolSelectionHandler::updateRubberband( const double &radius )
{
  if ( !selectionRubberBand() )
    initRubberBand();

  const int RADIUS_SEGMENTS = 80;

  selectionRubberBand()->reset( QgsWkbTypes::PolygonGeometry );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                            mRadiusCenter.y() + radius * std::sin( theta ) );
    selectionRubberBand()->addPoint( radiusPoint, false );
  }
  selectionRubberBand()->closePoints( true );
}

void QgsMapToolSelectionHandler::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  if ( mDistanceWidget )
  {
    mDistanceWidget->setDistance( radius );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
    mDistanceWidget->editor()->selectAll();
  }
  else
  {
    updateRubberband( radius );
  }
}

QgsGeometry QgsMapToolSelectionHandler::selectedGeometry()
{
  return mSelectionGeometry;
}

void QgsMapToolSelectionHandler::setSelectedGeometry( QgsGeometry geometry, QInputEvent *event)
{
  mSelectionGeometry = geometry;
  emit geometryChanged( event);
}

void QgsMapToolSelectionHandler::setSelectionMode( SelectionMode mode )
{
  mSelectionMode = mode;
}

QgsMapToolSelectionHandler::SelectionMode QgsMapToolSelectionHandler::selectionMode()
{
  return mSelectionMode;
}

bool QgsMapToolSelectionHandler::selectionActive()
{
  return mSelectionActive;
}

QgsRubberBand  *QgsMapToolSelectionHandler::selectionRubberBand()
{
  return mSelectionRubberBand;
}

