import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 1.4
import my.tester 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Item {
        anchors.fill: parent

        Button {
            id: button1
            text: "start work1"
            anchors {
                left: parent.left
                top: parent.top
            }
            onClicked: {
                Tester.startWork1()
            }
        }
        Button {
            id: button2
            text: "start work2"
            anchors {
                left: button1.right
                top: parent.top
            }
            onClicked: {
                Tester.startWork2()
            }
        }
    }
}
