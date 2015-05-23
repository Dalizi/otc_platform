#ifndef ERRMSGBASE_H
#define ERRMSGBASE_H

#include <QObject>

class ErrMsgBase : public QObject
{
    Q_OBJECT
public:
    explicit ErrMsgBase(QObject *parent = 0);

signals:
    void errOccured(QString);

public slots:
};

#endif // ERRMSGBASE_H
