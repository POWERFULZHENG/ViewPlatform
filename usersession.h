#ifndef USERSESSION_H
#define USERSESSION_H

#include <QString>
#include <QDateTime>
#include <QMutex>
#include <QObject>

// 对应sys_user表的用户信息结构体
struct UserInfo {
    int id = 0;                 // 用户ID
    QString userName;           // 登录账号（user_name）
    QString nickName;           // 用户昵称（nick_name）
    QString roleName;           // 用户角色（role_name）
    QString phone;              // 手机号（phone）
    int status = 1;             // 状态（0=禁用 1=启用）
    QDateTime createTime;       // 创建时间（create_time）
//    bool isLogin = false;

    // 清空用户信息（退出登录时用）
    void clear() {
        id = 0;
        userName.clear();
        nickName.clear();
        roleName.clear();
        phone.clear();
        status = 1;
        createTime = QDateTime();
    }

    // 判断是否登录
    bool isLogin() const {
        return id > 0 && !phone.isEmpty();
    }

    // 判断是否为超级管理员
    bool isSuperAdmin() const {
        return roleName == "超级管理员";
    }
};

class UserSession : public QObject
{
    Q_OBJECT
public:
    enum UserRole{
      ADMIN,
      USER,
      GUEST
    };
private:
    // 单例模式：私有构造+静态实例
    explicit UserSession(QObject *parent = nullptr);
    UserSession(const UserSession&) = delete;
    UserSession& operator=(const UserSession&) = delete;

    static UserSession* m_instance;
    static QMutex m_mutex;       // 线程安全锁
    UserInfo m_currentUser;      // 当前登录用户信息

public:
    // 获取单例实例（全局唯一）
    static UserSession* instance();

    // ========== 用户信息操作接口 ==========
    // 设置当前登录用户信息（登录成功后调用）
    void setCurrentUser(const UserInfo& userInfo);

    // 获取当前用户信息
    UserInfo currentUser() const;

    // 快捷获取单个字段（避免每次都currentUser().xxx）
    int userId() const;                  // 用户ID
    QString userPhone() const;           // 手机号
    QString userRole() const;            // 用户角色
    QString userNickName() const;        // 用户昵称
    bool isAdmin() const;                // 是否为超级管理员
    bool isUserLogin() const;            // 是否已登录

    // 退出登录（清空会话）
    void logout();

signals:
    // 用户信息变更信号（如退出登录、角色变更时触发）
    void userInfoChanged();
};

#endif // USERSESSION_H
