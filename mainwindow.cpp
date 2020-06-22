#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogaddconnection.h"
#include "ui_dialogaddconnection.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QClipboard>
#include <QModelIndexList>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlIndex>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(QApplication::organizationName(), QApplication::applicationName())
{
    ui->setupUi(this);
    ui->treeWidget->setHeaderLabel("数据库");
    ui->splitter->setStretchFactor(0,2);
    ui->splitter->setStretchFactor(1,8);
    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(itemActivated(QTreeWidgetItem*, int)));
    connect(ui->tableView, &QTableView::customContextMenuRequested, [=](const QPoint& pos){
        QModelIndex index = ui->tableView->indexAt(pos);
        int column = index.column();
        qDebug() << column;
        if (column > -1) {
            QString text = index.data(Qt::DisplayRole).toString();
            QFontMetrics FM(qApp->font());
            QString text_elide = FM.elidedText(text, Qt::ElideRight, 150);
            QString header = ui->tableView->model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
            QList<QAction*> actions;
            QAction *action_insert_row = new QAction("插入行");
            QAction *action_copy = new QAction("复制");
            QAction *action_search = new QAction("搜索 \"" + text_elide + "\"");
            QAction *action_delete_row = new QAction("删除行");
            QAction *action_submit = new QAction("提交");
            QAction *action_revert = new QAction("撤销");
            actions.append(action_insert_row);
            actions.append(action_copy);
            actions.append(action_search);
            actions.append(action_delete_row);
            actions.append(action_submit);
            actions.append(action_revert);
            QAction *result_action = QMenu::exec(actions, ui->tableView->mapToGlobal(pos));
            if (result_action == action_insert_row) {
                int row = ui->tableView->currentIndex().row() + 1;
                tableModel->insertRow(row);
                QModelIndex index_insert = tableModel->index(row, 0);
                ui->tableView->edit(index_insert);
            } else if (result_action == action_search) {
                ui->textEdit_sql->setText("SELECT * FROM " + ui->treeWidget->currentItem()->text(0) + " WHERE " + header + " LIKE '%" + text + "%'");
            } else if (result_action == action_copy) {
                QModelIndexList modelIndexList = ui->tableView->selectionModel()->selectedIndexes();
                QString s = "";
                for (int i=0; i<modelIndexList.size(); i++) {
                    s += "\"" + modelIndexList.at(i).data(Qt::DisplayRole).toString() + "\" ";
                }
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(s);
            } else if (result_action == action_delete_row) {
                QModelIndexList modelIndexList = ui->tableView->selectionModel()->selectedIndexes();
                for (int i=0; i<modelIndexList.size(); i++) {
                    tableModel->removeRow(modelIndexList.at(i).row());
                }
                int SB = QMessageBox::warning(this, "删除当前行", "你确定要删除当前行吗？", QMessageBox::Yes, QMessageBox::No);
                if (SB == QMessageBox::Yes) {
                    if (tableModel->submitAll()) {
                        tableModel->select();
                    } else {
                        QMessageBox::critical(this, "错误", tableModel->lastError().text(), QMessageBox::Ok);
                    }
                } else {
                    tableModel->revertAll();
                }
            } else if (result_action == action_submit) {
                if (tableModel->submitAll()) {
                    tableModel->select();
                } else {
                    QMessageBox::critical(this, "错误", tableModel->lastError().text(), QMessageBox::Ok);
                }
            } else if (result_action == action_revert) {
                tableModel->select();
            }
        }
    });
    connect(ui->comboBox_filter, &QComboBox::currentTextChanged, [=]{
        ui->comboBox_filter->setToolTip(ui->comboBox_filter->currentText());
    });

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Qt_triggered()
{
    QMessageBox::aboutQt(nullptr, "关于 Qt");
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox MB(QMessageBox::NoIcon, "关于", "海天鹰SQL 1.0\n一款基于Qt5的SQL数据库管理工具。\n作者：海天鹰\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy");
    MB.setIconPixmap(QPixmap(":/HTYSQL.png"));
    MB.exec();
}

void MainWindow::on_action_add_connection_triggered()
{
    DialogAddConnection *dialog = new DialogAddConnection(this);
    int dc = dialog->exec();
    if (dc == QDialog::Accepted) {
        ui->statusBar->showMessage("连接数据库：" + dialog->ui->lineEdit_path->text());
        static int cCount = 0;
        db = QSqlDatabase::addDatabase(dialog->ui->comboBox->currentText(), QString("Browser%1").arg(++cCount));
        db.setDatabaseName(dialog->ui->lineEdit_path->text());
        //db.setHostName(host);
        //db.setPort(port);
        if (db.open(dialog->ui->lineEdit_username->text(), dialog->ui->lineEdit_password->text())) {
            tableModel = new QSqlTableModel(this, db);
            genTree();
        } else {
            ui->statusBar->showMessage(db.lastError().text());
            db = QSqlDatabase();
            QSqlDatabase::removeDatabase(QString("Browser%1").arg(cCount));
        }
    }
    dialog->close();
}

void MainWindow::genTree()
{
    ui->treeWidget->clear();
    QStringList connectionNames = QSqlDatabase::connectionNames();
    for (int i = 0; i < connectionNames.count(); ++i) {
        QTreeWidgetItem *TWI_root = new QTreeWidgetItem(ui->treeWidget);
        db = QSqlDatabase::database(connectionNames.at(i), false);
        TWI_root->setText(0, QFileInfo(db.databaseName()).fileName());
        TWI_root->setToolTip(0, db.databaseName());
        if (db.isOpen()) {
            QStringList SL_table = db.tables();
            for (int t = 0; t < SL_table.count(); ++t) {
                QTreeWidgetItem *TWI_table = new QTreeWidgetItem(TWI_root);
                TWI_table->setText(0, SL_table.at(t));
            }
        }
    }
}

void MainWindow::itemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    tableModel->setTable(item->text(0));
    tableModel->select();
    ui->statusBar->showMessage(tableModel->lastError().text());    
    ui->tableView->setModel(tableModel);
    ui->statusBar->showMessage("共 " + QString::number(ui->tableView->model()->rowCount()) + " 条记录");
    ui->comboBox_filter->clear();
    QSqlIndex sqlIndex = tableModel->primaryKey();
    qDebug() << sqlIndex;
    for (int i=0; i<sqlIndex.count(); i++) {
        for (int c=0; c<tableModel->columnCount(); c++) {
            QString header = tableModel->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
            if (header == sqlIndex.fieldName(i)) {
                tableModel->setHeaderData(c, Qt::Horizontal, QVariant::fromValue(QIcon(":/key.png")), Qt::DecorationRole);
            }
        }
    }
    for(int i=0; i<tableModel->columnCount(); i++){
        QString header = tableModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        ui->comboBox_filter->addItem(header);
    }    
}

void MainWindow::on_treeWidget_customContextMenuRequested(QPoint pos)
{
    QTreeWidgetItem *TWI = ui->treeWidget->itemAt(pos);
    if (TWI == nullptr) {

    } else {
        QMenu *menu = new QMenu(this);
        QAction *action_delete = new QAction("删除");
        connect(action_delete, &QAction::triggered, [=]{
            delete ui->treeWidget->takeTopLevelItem(ui->treeWidget->currentIndex().row());
        });
        menu->addAction(action_delete);
        menu->exec(QCursor::pos());
    }
}

void MainWindow::on_pushButton_filter_clicked()
{
    if (ui->lineEdit_filter->text() == "") {
        tableModel->setFilter("");
    } else {
        tableModel->setFilter(ui->comboBox_filter->currentText() + " LIKE '%" + ui->lineEdit_filter->text() + "%'");
    }
    ui->tableView->setModel(tableModel);
    tableModel->select();
    ui->statusBar->showMessage("共 " + QString::number(ui->tableView->model()->rowCount()) + " 条记录");
}

void MainWindow::on_pushButton_query_clicked()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(ui->tableView);
    queryModel->setQuery(ui->textEdit_sql->toPlainText(), db);
    ui->tableView->setModel(queryModel);
    if (queryModel->lastError().type() == QSqlError::NoError) {
        ui->statusBar->showMessage("共 " + QString::number(ui->tableView->model()->rowCount()) + " 条记录");
    } else {
        ui->statusBar->showMessage(queryModel->lastError().text());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}