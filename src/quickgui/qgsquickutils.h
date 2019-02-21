/***************************************************************************
  qgsquickutils.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKUTILS_H
#define QGSQUICKUTILS_H


#include <QObject>
#include <QString>
#include <QUrl>
#include <QtPositioning/QGeoCoordinate>

#include <limits>

#include "qgis.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgspointxy.h"
#include "qgsunittypes.h"
#include "qgsquickmapsettings.h"
#include "qgsquickfeaturelayerpair.h"
#include "qgis_quick.h"
#include "qgscoordinateformatter.h"


class QgsFeature;
class QgsVectorLayer;
class QgsCoordinateReferenceSystem;

/**
 * \ingroup quick
 *
 * Encapsulating the common utilies for QgsQuick library.
 *
 * \note QML Type: Utils (Singleton)
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickUtils: public QObject
{
    Q_OBJECT

    /**
      * "dp" is useful for building building components that work well with different screen densities.
      * It stands for density-independent pixels. A width of 10dp is going to be the same physical size
      * on all screens regardless their density. In QML code, all values are specified in screen pixels,
      * so in order to set a width of 10dp, one would use the following code: "width: 10 * QgsQuick.Utils.dp"
      *
      * 1dp is approximately 0.16mm. When screen has 160 DPI (baseline), the value of "dp" is 1.
      * On high DPI screen the value will be greater, e.g. 1.5.
      *
      * This is a readonly property.
      */
    Q_PROPERTY( qreal dp READ screenDensity CONSTANT )

  public:
    //! Create new utilities
    QgsQuickUtils( QObject *parent = nullptr );
    //! Destructor
    ~QgsQuickUtils() = default;

    //! \copydoc QgsQuickUtils::dp
    qreal screenDensity() const;

    /**
      * Creates crs from epsg code in QML
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsCoordinateReferenceSystem coordinateReferenceSystemFromEpsgId( long epsg );

    /**
      * Creates QgsPointXY in QML
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsPointXY pointXY( double x, double y );

    /**
      * Creates QgsPoint in QML
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsPoint point( double x, double y, double z = std::numeric_limits<double>::quiet_NaN(), double m = std::numeric_limits<double>::quiet_NaN() );

    /**
      * Converts QGeoCoordinate to QgsPoint
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsPoint coordinateToPoint( const QGeoCoordinate &coor );

    /**
      * Transforms point between different crs from QML
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsPointXY transformPoint( const QgsCoordinateReferenceSystem &srcCrs,
        const QgsCoordinateReferenceSystem &destCrs,
        const QgsCoordinateTransformContext &context,
        const QgsPointXY &srcPoint );

    /**
      * Calculates the distance in meter representing baseLengthPixels pixels on the screen based on the current map settings.
      */
    Q_INVOKABLE static double screenUnitsToMeters( QgsQuickMapSettings *mapSettings, int baseLengthPixels );

    /**
      * Returns whether file on path exists
      * \since QGIS 3.4
      */
    Q_INVOKABLE static bool fileExists( const QString &path );

    /**
     * Returns filename from given path. If a param prefixPath is not empty, returns relative path of the file to this path.
     * If a path starts with "file://", this prefix is ignored.
     * @param path Absolute path to file
     * @param prefixPath Optional parameter; defines if the function returns fileName or a relative path to this path.
     * \since QGIS 3.4
     */
    Q_INVOKABLE static QString getFileName( const QString &path, const QString &prefixPath = QStringLiteral( "" ) );

    /**
      * Log message in QgsMessageLog
      */
    Q_INVOKABLE static void logMessage( const QString &message,
                                        const QString &tag = QString( "QgsQuick" ),
                                        Qgis::MessageLevel level = Qgis::Warning );

    /**
      * QgsQuickFeatureLayerPair factory for tuple of QgsFeature and QgsVectorLayer used in QgsQUick library.
      * \param feature QgsFeature linked to new QgsQuickFeature instance.
      * \param layer QgsVectorLayer which the feature belongs to, optional.
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QgsQuickFeatureLayerPair featureFactory( const QgsFeature &feature, QgsVectorLayer *layer = nullptr );

    /**
      * Returns QUrl to image from library's /images folder.
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static const QUrl getThemeIcon( const QString &name );

    /**
      * Returns url to field editor component for a feature form.
      * If the widgetName does not match any supported widget, text edit is returned.
      * \param widgetName name of the attribute field widget
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static const QUrl getEditorComponentSource( const QString &widgetName );

    /**
     * \copydoc QgsCoordinateFormatter::format()
     *
     * \since QGIS 3.4
     */
    Q_INVOKABLE static QString formatPoint(
      const QgsPoint &point,
      QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatPair,
      int decimals = 3,
      QgsCoordinateFormatter::FormatFlags flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix );

    /**
      * Converts distance to human readable distance
      *
      * This is useful for scalebar texts or output of the GPS accuracy
      *
      * The resulting units are determined automatically,
      * based on requested system of measurement.
      * e.g. 1222.234 m is converted to 1.2 km
      *
      * \param distance distance in units
      * \param units units of dist
      * \param decimals decimal to use
      * \param destSystem system of measurement of the result
      * \returns string represetation of dist in desired destSystem. For distance less than 0, 0 is returned.
      *
      * \since QGIS 3.4
      */
    Q_INVOKABLE static QString formatDistance( double distance,
        QgsUnitTypes::DistanceUnit units,
        int decimals,
        QgsUnitTypes::SystemOfMeasurement destSystem = QgsUnitTypes::MetricSystem );

    /**
      * Deletes file from a given path.
      *
      * \param filePath Absolute path to file
      * \returns bool True, if removal was successfull, otherwise false.
      *
      * \since QGIS 3.8
      */
    Q_INVOKABLE static bool remove( const QString &filePath );

    /**
      * Converts distance to human readable distance in destination system of measurement
      *
      * \sa QgsQuickUtils::formatDistance()
      *
      * \param srcDistance distance in units
      * \param srcUnits units of dist
      * \param destSystem system of measurement of the result
      * \param destDistance output: distance if desired system of measurement
      * \param destUnits output: unit of destDistance
      *
      * \since QGIS 3.4
      */
    static void humanReadableDistance( double srcDistance,
                                       QgsUnitTypes::DistanceUnit srcUnits,
                                       QgsUnitTypes::SystemOfMeasurement destSystem,
                                       double &destDistance,
                                       QgsUnitTypes::DistanceUnit &destUnits );

    //! Returns a string with information about screen size and resolution - useful for debugging
    QString dumpScreenInfo() const;

  private:
    static void formatToMetricDistance( double srcDistance,
                                        QgsUnitTypes::DistanceUnit srcUnits,
                                        double &destDistance,
                                        QgsUnitTypes::DistanceUnit &destUnits );

    static void formatToImperialDistance( double srcDistance,
                                          QgsUnitTypes::DistanceUnit srcUnits,
                                          double &destDistance,
                                          QgsUnitTypes::DistanceUnit &destUnits );

    static void formatToUSCSDistance( double srcDistance,
                                      QgsUnitTypes::DistanceUnit srcUnits,
                                      double &destDistance,
                                      QgsUnitTypes::DistanceUnit &destUnits );


    static qreal calculateScreenDensity();

    qreal mScreenDensity;
};

#endif // QGSQUICKUTILS_H
