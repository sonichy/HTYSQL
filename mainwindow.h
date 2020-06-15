#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    QSqlDatabase db;
    QSqlTableModel *tableModel;
    void genTree();
    void readSettings();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_action_add_connection_triggered();
    void on_action_Qt_triggered();
    void on_action_about_triggered();
    void on_pushButton_filter_clicked();
    void on_pushButton_query_clicked();
    void on_treeWidget_customContextMenuRequested(QPoint point);
    void itemActivated(QTreeWidgetItem *item, int column);

};

#endif // MAINWINDOW_H