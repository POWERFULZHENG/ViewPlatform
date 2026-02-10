#include "IPHelper.h"

// 过滤有效网络接口的辅助方法
bool IPHelper::isEffectiveInterface(const QNetworkInterface &iface)
{
    // 排除：未激活、回环接口、虚拟接口
    return iface.isValid()
           && iface.flags().testFlag(QNetworkInterface::IsUp)
           && !iface.flags().testFlag(QNetworkInterface::IsLoopBack);
}

// 获取单个本地IP（IPv4/IPv6）
QString IPHelper::getLocalIP(IPHelper::IPType type)
{
    // 遍历所有网络接口
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (!isEffectiveInterface(iface)) continue;

        // 遍历接口下的所有IP地址
        QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            QHostAddress addr = entry.ip();
            // 筛选指定类型的IP（IPv4/IPv6）
            if ((type == IPv4 && addr.protocol() == QAbstractSocket::IPv4Protocol)
                || (type == IPv6 && addr.protocol() == QAbstractSocket::IPv6Protocol)) {
                return addr.toString();
            }
        }
    }
    return ""; // 无有效IP返回空
}

// 获取所有有效IP（IPv4+IPv6）
QMap<QString, QStringList> IPHelper::getAllLocalIPs()
{
    QMap<QString, QStringList> ipMap;
    QStringList ipv4List, ipv6List;

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (!isEffectiveInterface(iface)) continue;

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                ipv4List.append(addr.toString());
            } else if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
                ipv6List.append(addr.toString());
            }
        }
    }

    ipMap.insert("IPv4", ipv4List);
    ipMap.insert("IPv6", ipv6List);
    return ipMap;
}
