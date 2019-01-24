/***************************************************************************
 qgsquickvaluemap.qml
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

import QtQuick 2.7
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QgsQuick 0.1 as QgsQuick

/**
 * Value Map for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged(var value, bool isNull)

  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  height: childrenRect.height + 8 * QgsQuick.Utils.dp

  ComboBox {
    id: comboBox

    property var reverseConfig: ({})
    property var currentValue: value
    property var currentMap
    property var currentKey
    height: parent.height + 10 * QgsQuick.Utils.dp
    anchors { left: parent.left; right: parent.right }
    currentIndex: find(reverseConfig[value])

    ListModel {
        id: listModel
    }

    Component.onCompleted: {
      if( config['map'] )
      {
        if( config['map'].length )
        {
          //it's a list (>=QGIS3.0)
          for(var i=0; i<config['map'].length; i++)
          {
            currentMap = config['map'][i]
            currentKey = Object.keys(currentMap)[0]
            listModel.append( { text: currentKey } )
            reverseConfig[currentMap[currentKey]] = currentKey;
          }
          model=listModel
          textRole = 'text'
        }
        else
        {
          //it's a map (<=QGIS2.18)
          model = Object.keys(config['map']);
          for(var key in config['map']) {
            reverseConfig[config['map'][key]] = key;
          }
        }
      }

      currentIndex = find(reverseConfig[value])
    }

    onCurrentTextChanged: {
      currentMap= config['map'].length ? config['map'][currentIndex] : config['map']
      valueChanged(currentMap[currentText], false)
    }

    // Workaround to get a signal when the value has changed
    onCurrentValueChanged: {
      currentIndex = find(reverseConfig[value])
    }

    MouseArea {
      anchors.fill: parent
      propagateComposedEvents: true

      onClicked: mouse.accepted = false
      onPressed: { forceActiveFocus(); mouse.accepted = false; }
      onReleased: mouse.accepted = false;
      onDoubleClicked: mouse.accepted = false;
      onPositionChanged: mouse.accepted = false;
      onPressAndHold: mouse.accepted = false;
    }

    // [hidpi fixes]
    delegate: ItemDelegate {
      width: comboBox.width
      height: 36 * QgsQuick.Utils.dp
      text: config['map'].length ? model.text : modelData
      font.weight: comboBox.currentIndex === index ? Font.DemiBold : Font.Normal
      font.pointSize: 12
      highlighted: comboBox.highlightedIndex == index
      leftPadding: 5 * QgsQuick.Utils.dp
    }

    contentItem: Text {
      height: 36 * QgsQuick.Utils.dp
      text: comboBox.displayText
      horizontalAlignment: Text.AlignLeft
      verticalAlignment: Text.AlignVCenter
      elide: Text.ElideRight
      leftPadding: 5 * QgsQuick.Utils.dp
    }

    background: Item {
      implicitWidth: 120 * QgsQuick.Utils.dp
      implicitHeight: 36 * QgsQuick.Utils.dp

      Rectangle {
        anchors.fill: parent
        id: backgroundRect
        border.color: comboBox.pressed ? "#17a81a" : "#21be2b"
        border.width: comboBox.visualFocus ? 2 : 1
        color: "#dddddd"
        radius: 2
      }
    }
    // [/hidpi fixes]
  }
}
