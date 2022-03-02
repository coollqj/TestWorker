#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "WorkerThread.h"
#include <QObject>
#include <QPointer>
#include <QThread>

class StartWorker;
class Worker;

class CallbackRunnable : public QObject {
  Q_OBJECT
public:
  CallbackRunnable(std::function<void()>, QString = "CallbackRunnable");
  ~CallbackRunnable();

public slots:
  void realRun();

private:
  std::function<void()> _fun = nullptr;
  QString _name;
};

class Runnable : public QObject {
  Q_OBJECT
public:
  Runnable(QString name, Worker *);
  ~Runnable();
  void sendFinish();
public slots:
  void realRun();
signals:
  void finished();

private:
  QPointer<Worker> _originalWorkder;
  QString _name;
};

class Worker : public QObject {
  Q_OBJECT
protected:
  Worker(QObject *, std::function<void()> fun, QString name = "Worker");
  Worker(QObject *, QString name = "Worker");
  ~Worker();

  Worker *findRootWorker();

public:
  static Worker *workerOf(std::function<void()> fun, QObject * = nullptr,
                          QString name = "workerOf");

  virtual void doWork(std::function<void()> completeFun = nullptr);

  Worker *concat(std::function<void()>);

  Worker *concat(Worker *);

  Worker *workOnSubThread(QThread * = nullptr);

  Worker *workOnMainThread();

  Worker *endWork();

signals:

protected:
  QString _name;
  std::function<void()> _fun = nullptr;
  std::shared_ptr<CallbackRunnable> _completeRunnable = nullptr;
};

class ConcatWorker : public Worker {
public:
  ConcatWorker(QObject *parent, std::function<void()> fun);
  ~ConcatWorker();
  void doWork(std::function<void()> completeFun = nullptr) override;
};

class ConcatWorkerExt : public Worker {
public:
  ConcatWorkerExt(QObject *parent, Worker *);
  ~ConcatWorkerExt();
  void doWork(std::function<void()> completeFun = nullptr) override;

private:
  QPointer<Worker> _worker = nullptr;
};

class EndWorker : public Worker {
public:
  EndWorker(QObject *parent);
  ~EndWorker();
  void doWork(std::function<void()> completeFun = nullptr) override;
};

class ThreadWorker : public Worker {
  Q_OBJECT
public:
  ThreadWorker(Worker *, QThread *);
  ~ThreadWorker();
  void doWork(std::function<void()> completeFun = nullptr) override;
signals:
  void run();

private:
  std::shared_ptr<Runnable> _runnable;
};

class ThreadManager : public QObject {
  Q_OBJECT
private:
  explicit ThreadManager();
  ~ThreadManager();

public:
  static QPointer<ThreadManager> instance();
  static QThread *getMainThread();
  static QThread *getSubThread();
  /**
   * @brief initMainThread
   * We must call this method in main thread.
   */
  static void initMainThread();

signals:

private:
  static QPointer<ThreadManager> _threadManager;
  QPointer<QThread> _mainThread;
  QPointer<QThread> _subThread;
  QString _name;
};
#endif // THREADMANAGER_H
