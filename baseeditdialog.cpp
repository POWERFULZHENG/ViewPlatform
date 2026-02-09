#include "baseeditdialog.h"
#include <QPushButton>
#include <QHBoxLayout>

BaseEditDialog::BaseEditDialog(QWidget *parent, DialogOperType type)
    : QDialog(parent), m_operType(type)
{
    this->setModal(true);
    this->setFixedSize(420, 320);
    this->setWindowTitle(type == Oper_Create ? "新建数据" : "编辑数据");

    // ========== 先创建主布局，再添加子布局，杜绝空指针 ==========
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    // 通用按钮：确认+取消
    QPushButton *btnConfirm = new QPushButton("确认提交", this);
    QPushButton *btnCancel = new QPushButton("取消", this);
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnConfirm);
    btnLayout->addWidget(btnCancel);
    btnLayout->setContentsMargins(0,20,0,0);

    connect(btnConfirm, &QPushButton::clicked, this, &BaseEditDialog::slot_onConfirmClicked);
    connect(btnCancel, &QPushButton::clicked, this, &BaseEditDialog::slot_onCancelClicked);

    // ========== 向已存在的主布局添加按钮布局 ==========
    mainLayout->addLayout(btnLayout);
}

void BaseEditDialog::slot_onConfirmClicked()
{
    // 通用流程：先校验数据，通过则提交，成功则关闭对话框
    if(checkFormData()){
        if(submitFormData()){
            QMessageBox::information(this, "成功", m_operType == Oper_Create ? "数据新建成功！" : "数据编辑成功！");
            this->accept();
        }
    }
}

void BaseEditDialog::slot_onCancelClicked()
{
    this->reject();
}
