/***************************************************************************
 qgsquicktextedit.qml
  --------------------------------------
  Date                 : 2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick 2.5
import QgsQuick 0.1 as QgsQuick
import QtQuick.Layouts 1.3

/**
 * Text Edit for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged(var value, bool isNull)

  id: fieldItem

  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  height: childrenRect.height

  TextField {
    id: textField
    height: textArea.height == 0 ? customStyle.height : 0
    topPadding: 10 * QgsQuick.Utils.dp
    bottomPadding: 10 * QgsQuick.Utils.dp
    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pixelSize: customStyle.fontPixelSize
    wrapMode: Text.Wrap
    color: customStyle.fontColor

    text: value || ''
    inputMethodHints: field.isNumeric || widget == 'Range' ? field.precision === 0 ? Qt.ImhDigitsOnly : Qt.ImhFormattedNumbersOnly : Qt.ImhNone

    // Make sure we do not input more characters than allowed for strings
    states: [
        State {
            name: "limitedTextLengthState"
            when: (!field.isNumeric) && (field.length > 0)
            PropertyChanges {
              target: textField
              maximumLength: field.length
            }
        }
    ]

    background: Rectangle {
      y: textField.height - height - textField.bottomPadding / 2
      implicitWidth: 120 * QgsQuick.Utils.dp
      height: textField.activeFocus ? 2 * QgsQuick.Utils.dp : 1 * QgsQuick.Utils.dp
      color: textField.activeFocus ? customStyle.backgroundColor : customStyle.backgroundColorInactive
    }

    onTextChanged: {
      valueChanged( text, text == '' )
    }
  }

  TextArea {
    id: textArea
    height: config['IsMultiline'] === true ? undefined : 0
    Layout.fillWidth: true
    Layout.fillHeight: true
    topPadding: customStyle.height * 0.25
    bottomPadding: customStyle.height * 0.25


    visible: height !== 0
    anchors.left: parent.left
    anchors.right: parent.right
    font.pixelSize: customStyle.fontPixelSize
    wrapMode: Text.Wrap
    color: customStyle.fontColor

    text: value || ''
    textFormat: config['UseHtml'] ? TextEdit.RichText : TextEdit.PlainText

    background: Rectangle {
        color: customStyle.backgroundColor
    }

    onEditingFinished: {
      valueChanged( text, text == '' )
    }
  }

}
