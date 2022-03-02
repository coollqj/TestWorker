#include "tester.h"
#include "ThreadManager.h"
#include <QDebug>

Tester::Tester(QObject *parent) : QObject{parent} {}

void Tester::startWork1() {
  qDebug() << "Tester::startWork1";
  Worker::workerOf([]() -> void {
    qDebug() << "worker1 start++++++ " << QThread::currentThread();
    QThread::msleep(1000);
    qDebug() << "worker1 end++++++ " << QThread::currentThread();
  })
      ->workOnSubThread(&subThread1)
      ->concat([]() -> void {
        qDebug() << "worker1 concat work start++++++ "
                 << QThread::currentThread();
        QThread::msleep(1000);
        qDebug() << "worker1  concat work end++++++ "
                 << QThread::currentThread();
      })
      ->workOnSubThread(&subThread2)
      ->concat(generateWorker()->endWork())
      ->workOnSubThread(ThreadManager::getSubThread())
      ->endWork()
      ->doWork([]() -> void {
        qDebug() << "worker1 complete thread id " << QThread::currentThread();
      });
}

void Tester::startWork2() { qDebug() << "Tester::startWork2"; }

Worker *Tester::generateWorker() {
  return Worker::workerOf([]() -> void {
           qDebug() << "generateWorker start++++++ "
                    << QThread::currentThread();
           QThread::msleep(1000);
           qDebug() << "generateWorker end++++++ " << QThread::currentThread();
         })
      ->endWork();
}
