#ifndef DIALOGADDCONNECTION_H
#define DIALOGADDCONNECTION_H

#include <QDialog>

namespace Ui {
class DialogAddConnection;
}

class DialogAddConnection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddConnection(QWidget *parent = nullptr);
    ~DialogAddConnection();
    Ui::DialogAddConnection *ui;

private:
    QString filepath;

};

#endif // DIALOGADDCONNECTION_H