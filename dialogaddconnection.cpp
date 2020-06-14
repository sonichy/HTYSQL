#include "dialogaddconnection.h"
#include "ui_dialogaddconnection.h"
#include <QSqlDatabase>
#include <QAction>
#include <QFileDialog>

DialogAddConnection::DialogAddConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddConnection)
{
    ui->setupUi(this);
    QStringList drivers = QSqlDatabase::drivers();
    ui->comboBox->addItems(drivers);
    QAction *action_browser = new QAction(this);
    action_browser->setIcon(QIcon::fromTheme("folder"));
    ui->lineEdit_path->addAction(action_browser, QLineEdit::TrailingPosition);
    connect(action_browser, &QAction::triggered, [=]{
        filepath = QFileDialog::getOpenFileName(this, "打开数据库", filepath);
        ui->lineEdit_path->setText(filepath);
    });
}

DialogAddConnection::~DialogAddConnection()
{
    delete ui;
}
