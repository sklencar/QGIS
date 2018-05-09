/***************************************************************************
    qgsmaptoolselectrectangle.cpp  -  map tool for selecting features by
                                   rectangle
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolselectrectangle.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgis.h"
#include "qgisapp.h"

#include <QMouseEvent>
#include <QRect>

class QgisApp;

QgsMapToolSelectFeatures::QgsMapToolSelectFeatures( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mToolName = tr( "Select features" );
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Select ) );
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas, QgsMapToolSelectionHandler::SelectSimple );
  connect( mSelectionHandler, &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolSelectFeatures::selectFeatures );
}

QgsMapToolSelectFeatures::~QgsMapToolSelectFeatures()
{
  disconnect( mSelectionHandler, &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolSelectFeatures::selectFeatures );
  delete mSelectionHandler;
}

void QgsMapToolSelectFeatures::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
  mSelectionHandler->canvasPressEvent( e );
}

void QgsMapToolSelectFeatures::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasMoveEvent( e );
}

void QgsMapToolSelectFeatures::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasReleaseEvent( e );
}

void QgsMapToolSelectFeatures::selectFeatures( QInputEvent *e )
{
  if ( mSelectionHandler->selectedGeometry().type() == QgsWkbTypes::PointGeometry )
  {
    QgsVectorLayer *vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas, QgisApp::instance()->messageBar() );
    QgsPointXY point = mSelectionHandler->selectedGeometry().asPoint();
    double sr = searchRadiusMU( mCanvas );
    QgsRectangle r = toLayerCoordinates( vlayer, QgsRectangle( point.x() - sr, point.y() - sr, point.x() + sr, point.y() + sr ) );
    mSelectionHandler->setSelectedGeometry( QgsGeometry::fromRect( r ), e );
  }
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionHandler->selectedGeometry(), e->modifiers(), QgisApp::instance()->messageBar() );
}
