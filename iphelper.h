#ifndef IPHELPER_H
#define IPHELPER_H

#include <QString>
#include <QNetworkInterface>
#include <QHostAddress>

class IPHelper
{
public:
    // IP 类型枚举
    enum IPType {
        IPv4,  // 优先返回IPv4（默认）
        IPv6   // 返回IPv6
    };

    /**
     * @brief 获取本地真实IP（非127.0.0.1/::1）
     * @param type 指定获取IPv4/IPv6
     * @return 成功返回IP字符串，失败返回空字符串
     */
    static QString getLocalIP(IPType type = IPv4);

    /**
     * @brief 获取本地所有有效IP（IPv4+IPv6）
     * @return 键：IP类型（"IPv4"/"IPv6"），值：对应IP列表
     */
    static QMap<QString, QStringList> getAllLocalIPs();

private:
    // 过滤有效网络接口（已激活、非回环、非虚拟）
    static bool isEffectiveInterface(const QNetworkInterface &iface);
};

#endif // IPHELPER_H
