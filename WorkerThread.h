#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <functional>

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread();
    ~WorkerThread();
signals:
};

#endif  // WORKERTHREAD_H
