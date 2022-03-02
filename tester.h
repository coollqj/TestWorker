#ifndef TESTER_H
#define TESTER_H

#include "ThreadManager.h"
#include <QObject>

class Tester : public QObject {
  Q_OBJECT
public:
  explicit Tester(QObject *parent = nullptr);

  Q_INVOKABLE void startWork1();
  Q_INVOKABLE void startWork2();

  Worker *generateWorker();
signals:

private:
  WorkerThread subThread1;
  WorkerThread subThread2;
};

#endif // TESTER_H
