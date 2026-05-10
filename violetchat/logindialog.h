#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;

public slots:
    void slot_forget_pwd();

signals:
    // if click the register_btn, then emit the switchRegister signal
    void switchRegister();

    void switchReset();
};

#endif // LOGINDIALOG_H
