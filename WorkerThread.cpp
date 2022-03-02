#include "WorkerThread.h"
#include <QDebug>
#include <QString>
#include <iosfwd>
#include <iostream>
#include <thread>

WorkerThread::WorkerThread() : QThread(nullptr) {
  moveToThread(this);
  start();
}

WorkerThread::~WorkerThread() {
  quit();
  wait();
}
