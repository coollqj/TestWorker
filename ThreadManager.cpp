#include "ThreadManager.h"
#include <QApplication>
#include <QDebug>
#include <QPointer>

using namespace std;

bool DEBUG_STUDIO_WORKER = false;
#define tracker(className, method)                                             \
  if (DEBUG_STUDIO_WORKER)                                                     \
  qDebug() << "DebugWorkder->" << _name << "->" << className << "::" << method

QPointer<ThreadManager> ThreadManager::_threadManager = new ThreadManager();

CallbackRunnable::CallbackRunnable(std::function<void()> fun, QString name)
    : _fun(fun), _name(name)

{
  tracker("CallbackRunnable", "CallbackRunnable");
  moveToThread(QThread::currentThread());
}

CallbackRunnable::~CallbackRunnable() {
  tracker("CallbackRunnable", "~CallbackRunnable");
}

void CallbackRunnable::realRun() {
  tracker("CallbackRunnable", "realRun");
  if (_fun) {
    _fun();
  }
}

Runnable::Runnable(QString name, Worker *worker) : QObject(nullptr) {
  _originalWorkder = worker;
  _name = name;
  tracker("Runnable", "Runnable");
}

Runnable::~Runnable() { tracker("Runnable", "~Runnable"); }

void Runnable::sendFinish() { emit finished(); }

void Runnable::realRun() {
  tracker("Runnable", "realRun _originalWorkder " << _originalWorkder);
  if (_originalWorkder) {
    _originalWorkder->doWork([=]() -> void { sendFinish(); });
  }
}

Worker::Worker(QObject *parent, std::function<void()> fun, QString name)
    : QObject(parent) {
  _name = name;
  _fun = fun;
  tracker("Worker", "Worker");
}

Worker::Worker(QObject *parent, QString name) : QObject(parent) {
  _name = name;
  tracker("Worker", "Worker");
}

Worker::~Worker() { tracker("Worker", "~Worker"); }

Worker *Worker::findRootWorker() {
  auto originalWorkder = dynamic_cast<Worker *>(parent());
  if (originalWorkder) {
    return originalWorkder->findRootWorker();
  } else {
    return this;
  }
}

Worker *Worker::workerOf(std::function<void()> fun, QObject *attatchedObject,
                         QString name) {
  return new Worker(attatchedObject == nullptr ? qApp : attatchedObject, fun,
                    name);
}

void Worker::doWork(std::function<void()> completeFun) {
  tracker("Worker", "doWork");
  if (_fun) {
    _fun();
  }
  if (completeFun) {
    completeFun();
  }
}

Worker *Worker::concat(std::function<void()> fun) {
  return new ConcatWorker(this, fun);
}

Worker *Worker::concat(Worker *worker) {
  return new ConcatWorkerExt(this, worker);
}

Worker *Worker::workOnSubThread(QThread *thread) {
  return new ThreadWorker(this, thread);
}

Worker *Worker::workOnMainThread() {
  return new ThreadWorker(this, ThreadManager::instance()->getMainThread());
}

Worker *Worker::endWork() { return new EndWorker(this); }

ConcatWorker::ConcatWorker(QObject *parent, std::function<void()> fun)
    : Worker(parent, fun, "CompleteWorker") {
  tracker("CompleteWorker", "CompleteWorker");
}

ConcatWorker::~ConcatWorker() { tracker("CompleteWorker", "~CompleteWorker"); }

void ConcatWorker::doWork(std::function<void()> completeFun) {
  tracker("CompleteWorker", "doWork");
  auto originalWorkder = dynamic_cast<Worker *>(parent());
  if (originalWorkder) {
    originalWorkder->doWork([=]() -> void { Worker::doWork(completeFun); });
  } else {
    Worker::doWork(completeFun);
  }
}

ConcatWorkerExt::ConcatWorkerExt(QObject *parent, Worker *worker)
    : Worker(parent, "ConcatWorkerExt"), _worker(worker) {
  tracker("ConcatWorkerExt", "ConcatWorkerExt " << worker);
}

ConcatWorkerExt::~ConcatWorkerExt() {
  tracker("ConcatWorkerExt", "~ConcatWorkerExt");
}

void ConcatWorkerExt::doWork(std::function<void()> completeFun) {
  tracker("ConcatWorkerExt", "doWork " << _worker);
  auto originalWorkder = dynamic_cast<Worker *>(parent());
  if (originalWorkder) {
    originalWorkder->doWork([=]() -> void {
      if (_worker) {
        _worker->doWork(completeFun);
      }
    });
  } else {
    if (_worker) {
      _worker->doWork(completeFun);
    }
  }
}

EndWorker::EndWorker(QObject *parent) : Worker(parent, "EndWorker") {
  tracker("EndWorker", "EndWorker");
}

EndWorker::~EndWorker() { tracker("EndWorker", "~EndWorker"); }
void EndWorker::doWork(std::function<void()> completeFun) {
  tracker("EndWorker", "doWork");
  auto rootParent = findRootWorker();
  auto originalWorkder = dynamic_cast<Worker *>(parent());
  if (originalWorkder) {
    originalWorkder->doWork([=]() -> void {
      Worker::doWork(completeFun);
      tracker("EndWorker", "doWork original work rootParent " << rootParent);
      if (rootParent != nullptr) {
        rootParent->deleteLater();
      }
    });
  } else {
    tracker("EndWorker", "doWork rootParent " << rootParent);
    Worker::doWork(completeFun);
    if (rootParent != nullptr) {
      rootParent->deleteLater();
    }
  }
}

ThreadWorker::ThreadWorker(Worker *parentWorker, QThread *outerThread)
    : Worker(parentWorker, "ThreadWorker") {
  tracker("ThreadWorker", "ThreadWorker");
  auto thread =
      outerThread == nullptr ? ThreadManager::getSubThread() : outerThread;
  if (thread) {
    _runnable =
        std::make_shared<Runnable>(this->_name + "_Runnable", parentWorker);
    _runnable->moveToThread(thread);
    connect(this, &ThreadWorker::run, _runnable.get(), &Runnable::realRun);
  }
}

ThreadWorker::~ThreadWorker() { tracker("ThreadWorker", "~ThreadWorker"); }

void ThreadWorker::doWork(std::function<void()> completeFun) {
  tracker("ThreadWorker", "doWork");
  _completeRunnable = std::make_shared<CallbackRunnable>(
      completeFun, _name + "_CallbackRunnable");
  connect(_runnable.get(), &Runnable::finished, _completeRunnable.get(),
          &CallbackRunnable::realRun);
  emit run();
}

ThreadManager::ThreadManager() : QObject(qApp) {
  tracker("ThreadManager", "ThreadManager");
  _subThread = new QThread(this);
  _subThread->start();
}

ThreadManager::~ThreadManager() {
  tracker("ThreadManager", "~ThreadManager");
  _subThread->quit();
  _subThread->wait();
}

QPointer<ThreadManager> ThreadManager::instance() { return _threadManager; }

QThread *ThreadManager::getMainThread() { return instance()->_mainThread; }

QThread *ThreadManager::getSubThread() { return instance()->_subThread; }

void ThreadManager::initMainThread() {
  instance()->_mainThread = QThread::currentThread();
}
