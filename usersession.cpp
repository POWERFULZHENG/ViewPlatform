#include "UserSession.h"

// 静态成员初始化
UserSession* UserSession::m_instance = nullptr;
QMutex UserSession::m_mutex;

UserSession::UserSession(QObject *parent) : QObject(parent)
{
    // 初始化时清空用户信息
    m_currentUser.clear();
}

// 获取单例实例（线程安全）
UserSession* UserSession::instance()
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new UserSession();
        }
    }
    return m_instance;
}

// 设置当前登录用户信息
void UserSession::setCurrentUser(const UserInfo &userInfo)
{
    QMutexLocker locker(&m_mutex);
    m_currentUser = userInfo;
    emit userInfoChanged(); // 触发信息变更信号
}

// 获取当前用户信息
UserInfo UserSession::currentUser() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser;
}

// 快捷获取用户ID
int UserSession::userId() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.id;
}

// 快捷获取手机号
QString UserSession::userPhone() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.phone;
}

// 快捷获取用户角色
QString UserSession::userRole() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.roleName;
}

// 快捷获取用户昵称
QString UserSession::userNickName() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.nickName;
}

// 判断是否为超级管理员
bool UserSession::isAdmin() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.isSuperAdmin();
}

// 判断是否已登录
bool UserSession::isUserLogin() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentUser.isLogin();
}

// 退出登录（清空会话）
void UserSession::logout()
{
    QMutexLocker locker(&m_mutex);
    m_currentUser.clear();
    emit userInfoChanged(); // 触发信息变更信号
}
