#ifndef BASEEDITDIALOG_H
#define BASEEDITDIALOG_H

#include <QDialog>
#include <QMessageBox>

class BaseEditDialog : public QDialog
{
    Q_OBJECT
public:
    // 通用对话框类型：新建/编辑，所有子类共用
    enum DialogOperType {
        Oper_Create = 0,  // 新建
        Oper_Edit   = 1   // 编辑
    };
public:
    explicit BaseEditDialog(QWidget *parent = nullptr, DialogOperType type = Oper_Create);
    ~BaseEditDialog() override = default;

protected:
    DialogOperType  m_operType;   // 操作类型：新建/编辑，子类可直接使用

    // ========== 纯虚函数【子类必须实现，业务差异化全部在这里】 ==========
    // 1. 数据校验：校验表单数据合法性，返回true=校验通过
    virtual bool checkFormData() = 0;
    // 2. 提交数据：执行新增/编辑的业务逻辑(数据库操作)，返回true=提交成功
    virtual bool submitFormData() = 0;

private slots:
    // ========== 通用槽函数【子类无需重写，直接继承使用】 ==========
    void slot_onConfirmClicked();  // 通用确认按钮逻辑：校验+提交+关闭
    void slot_onCancelClicked();   // 通用取消按钮逻辑
};

#endif // BASEEDITDIALOG_H
