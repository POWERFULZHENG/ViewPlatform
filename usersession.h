#ifndef USERSESSION_H
#define USERSESSION_H

#include <QObject>

class UserSession : public QObject
{
    Q_OBJECT
public:
    explicit UserSession(QObject *parent = nullptr);

signals:

};

#endif // USERSESSION_H
