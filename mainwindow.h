#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include "datamodel_cc.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTextCodec *decoder;
    QString executeImport(QString fileName);
    QSqlQuery sqlquery; QString qu;
    DataModel *usemodel;
    QModelIndex useindex;
    void LoadData();
    void SetModelsParam();
    void createTable();
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void Resized(int nn, int oldsize, int newsize);
    void filterClear();
    void gotoUp(); void gotoDown();
    void updateFromFile();
    void text_changed();
    void showNomer(QModelIndex);

};

#endif // MAINWINDOW_H
