/***************************************************************************
qgsmaptoolselectfreehand.cpp  -  map tool for selecting features by freehand
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaptoolselectfreehand.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsmapcanvas.h"
#include "qgis.h"
#include "qgisapp.h"

#include <QMouseEvent>
class QgsMapToolSelectionHandler;


QgsMapToolSelectFreehand::QgsMapToolSelectFreehand( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mCursor = Qt::ArrowCursor;
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas, QgsMapToolSelectionHandler::SelectFreehand );
  connect( mSelectionHandler, &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolSelectFreehand::selectFeatures );

}

QgsMapToolSelectFreehand::~QgsMapToolSelectFreehand()
{
  disconnect( mSelectionHandler, &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolSelectFreehand::selectFeatures );
  delete mSelectionHandler;
}

void QgsMapToolSelectFreehand::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasMoveEvent( e );
}

void QgsMapToolSelectFreehand::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasReleaseEvent( e );
}

void QgsMapToolSelectFreehand::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionHandler->escapeSelection( e ) )
  {
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolSelectFreehand::selectFeatures( QInputEvent *e )
{
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionHandler->selectedGeometry(), e->modifiers(), QgisApp::instance()->messageBar() );
}
