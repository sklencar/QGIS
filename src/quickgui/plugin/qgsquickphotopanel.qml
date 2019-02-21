/***************************************************************************
 qgsquickphotopanel.qml
  --------------------------------------
  Date                 : Dec 2017
  Copyright            : (C) 2017 by Viktor Sklencar
  Email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.3
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.2
import QtQml 2.2
import QtMultimedia 5.8
import QtGraphicalEffects 1.0
import QgsQuick 0.1 as QgsQuick

Drawer {
  property var targetDir
  property var lastPhotoName
  property int iconSize: photoPanel.width/20
  property var fieldItem

  property color bgColor: "black"
  property real bgOpacity: 1
  property color borderColor: "black"

  // icons:
  property var captureButtonIcon: QgsQuick.Utils.getThemeIcon("ic_camera_alt_border")
  property var okButtonIcon: QgsQuick.Utils.getThemeIcon("ic_check_black")
  property var cancelButtonIcon: QgsQuick.Utils.getThemeIcon("ic_clear_black")
  property real imageButtonSize: 45 * QgsQuick.Utils.dp
  property real buttonSize: imageButtonSize * 1.2
  property var buttonsPosition


  id: photoPanel
  visible: false
  modal: true
  interactive: true
  dragMargin: 0 // prevents opening the drawer by dragging.

  background: Rectangle {
    color: photoPanel.bgColor
    opacity: photoPanel.bgOpacity
  }

  onVisibleChanged: {
    if (visible) {
      camera.setCameraState(Camera.ActiveState)
      camera.start()
    } else {
      camera.stop()
      photoPreview.visible = false
    }
  }

  // PhotoCapture item
  Item {

    property bool saveImage: false

    id: captureItem
    width: window.width
    height: window.height

    Component.onDestruction: {
      if (!captureItem && camera.imageCapture.capturedImagePath != ""){
        captureItem.saveImage = false
        QgsQuick.Utils.removeFile(camera.imageCapture.capturedImagePath)
      }
      captureItem.saveImage = false
    }

    Camera {
      id: camera
      cameraState: Camera.UnloadedState

      imageCapture {
        onImageCaptured: {
          // Show the preview in an Image
          photoPreview.source = preview
        }
      }

      focus {
          focusMode: Camera.FocusContinuous
          focusPointMode: Camera.FocusPointAuto
      }
    }

    VideoOutput {
      id: videoOutput
      source: camera
      focus : visible // to receive focus and capture key events when visible
      anchors.fill: parent
      autoOrientation: true

      Item {
        id: captureButton
        width: buttonSize
        height: buttonSize
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        antialiasing: true

        MouseArea {
          id: mouseArea
          anchors.fill: parent
          onClicked: {
            if (targetDir !== "") {
              camera.imageCapture.captureToLocation(photoPanel.targetDir);
            } else {
              // saved to default location - TODO handle this case
              camera.imageCapture.capture();
            }
            photoPreview.visible = true;
          }
        }

        Image {
          id: captureButtonImage
          fillMode: Image.PreserveAspectFit
          anchors.centerIn: parent
          sourceSize.height: imageButtonSize
          sourceSize.width: imageButtonSize
          height: imageButtonSize
          source: photoPanel.captureButtonIcon
        }

      }

      Image {
        id: photoPreview
        width: parent.width
        height: parent.height
        fillMode: Image.PreserveAspectFit

        // Cancel button
        Rectangle {
          id: cancelButton
          visible: camera.imageCapture.capturedImagePath != ""

          property int borderWidth: 5 * QgsQuick.Utils.dp
          width: buttonSize
          height: buttonSize
          color: "white"
          border.color: photoPanel.borderColor
          anchors.right: parent.right
          anchors.top: confirmButton.bottom
          border.width: borderWidth
          radius: width*0.5
          antialiasing: true

          MouseArea {
            anchors.fill: parent
            onClicked: {
              captureItem.saveImage = false
              photoPreview.visible = false
              if (camera.imageCapture.capturedImagePath != "") {
                QgsQuick.Utils.removeFile(camera.imageCapture.capturedImagePath)
              }
            }
          }

          Image {
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
            sourceSize.height: imageButtonSize
            sourceSize.width: imageButtonSize
            height: imageButtonSize
            source: photoPanel.cancelButtonIcon
          }
        }

        // OK button
        Rectangle {
          id: confirmButton
          visible: camera.imageCapture.capturedImagePath != ""

          property int borderWidth: 5 * QgsQuick.Utils.dp
          width: buttonSize
          height: buttonSize
          color: "white"
          border.color: photoPanel.borderColor
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          border.width: borderWidth
          radius: width*0.5
          antialiasing: true

          MouseArea {
            anchors.fill: parent
            onClicked: {
              captureItem.saveImage = true
              photoPanel.visible = false
              photoPanel.lastPhotoName = QgsQuick.Utils.getRelativePath(camera.imageCapture.capturedImagePath, photoPanel.targetDir)
              if (photoPanel.lastPhotoName !== "") {
                fieldItem.image.source = photoPanel.targetDir + "/" + photoPanel.lastPhotoName
                fieldItem.valueChanged(photoPanel.lastPhotoName, photoPanel.lastPhotoName === "" || photoPanel.lastPhotoName === null)
              }
            }
          }

          Image {
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
            sourceSize.height: imageButtonSize
            sourceSize.width: imageButtonSize
            height: imageButtonSize
            width: imageButtonSize
            source: photoPanel.okButtonIcon
          }
        }
      }
    }
  }
}

