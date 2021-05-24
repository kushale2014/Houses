#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QProgressDialog>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>
#include <QProcess>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    usemodel = new DataModel();

    createTable();

    connect(ui->updateFromFile_PB, SIGNAL(clicked(bool)), this, SLOT(updateFromFile()));
    connect(ui->area_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->region_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->place_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->index_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->street_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->nomer_LE, SIGNAL(textChanged(QString)), this, SLOT(text_changed()));
    connect(ui->filterClear_PB, SIGNAL(clicked(bool)), this, SLOT(filterClear()));
    connect(ui->up_TB, SIGNAL(clicked(bool)), this, SLOT(gotoUp()));
    connect(ui->down_TB, SIGNAL(clicked(bool)), this, SLOT(gotoDown()));

    connect(ui->tableView->horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)), this, SLOT(Resized(int, int, int)));

    QFont font("Helvetica", 9, QFont::Bold);
    ui->tableView->horizontalHeader()->setFont(font);
    LoadData();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTable() {
    qu = "CREATE TABLE IF NOT EXISTS `Houses` ("
            " `id` INTEGER PRIMARY KEY AUTOINCREMENT, "
            " `area` TEXT, `region` TEXT, `place` TEXT, `index` TEXT, `street` TEXT, `nomer` TEXT, `vpz` TEXT"
            " )";
    sqlquery.exec(qu);
}

void MainWindow::Resized(int nn, int oldsize, int newsize) {

    switch (nn) {
    case 1:
        ui->area_LE->setFixedWidth(newsize-6);
        break;
    case 2:
        ui->region_LE->setFixedWidth(newsize-6);
        break;
    case 3:
        ui->place_LE->setFixedWidth(newsize-6);
        break;
    case 4:
        ui->index_LE->setFixedWidth(newsize-6);
        break;
    case 5:
        ui->street_LE->setFixedWidth(newsize-6);
        break;
    default:
        break;
    }

}

void MainWindow::filterClear() {
    ui->area_LE->clear();
    ui->region_LE->clear();
    ui->place_LE->clear();
    ui->index_LE->clear();
    ui->street_LE->clear();
    ui->nomer_LE->clear();
}

void MainWindow::gotoUp() {
    ui->tableView->setFocus();
    ui->tableView->selectRow(0);
}

void MainWindow::gotoDown() {
    while (usemodel->canFetchMore()) usemodel->fetchMore();
    ui->tableView->setFocus();
    ui->tableView->selectRow(ui->tableView->model()->rowCount()-1);
}

void MainWindow::updateFromFile() {

    QSettings settings("config.ini", QSettings::IniFormat);
    QString path = settings.value("path").toString();
    QString fileName = QFileDialog::getOpenFileName(this,
        "Відкрити файл з індексами", path, "Файли з індексами (houses.csv houses.zip)");
    if (fileName.isEmpty()) return;
    QString pathOpened = fileName.section('/',0,-2);
    if (path != pathOpened) settings.setValue("path", pathOpened);

    QString ext = fileName.section('.', -1);
    if (ext == "zip") {
        QString dir = "7-zip"; if(!QDir(dir).exists()) QDir().mkdir(dir);
        QString file1 = dir + "/7z.exe";
        QString file2 = dir + "/7z.dll";
        QFile ff1(file1); ff1.setPermissions(QFileDevice::WriteOther); ff1.remove();
        QFile ff2(file2); ff2.setPermissions(QFileDevice::WriteOther); ff2.remove();

        QFile file_default1(":/7z-32/7z.exe");
        file_default1.copy(file1);
        QFile file_default2(":/7z-32/7z.dll");
        file_default2.copy(file2);

        QString program = file1;
        QStringList arguments;
        arguments << "e" << QString(fileName) << "houses.csv" << "-y";
        QProcess *unzip = new QProcess();
        unzip->start(program, arguments);
        QEventLoop loop;
        QTimer::singleShot(300, &loop, SLOT(quit()));
        loop.exec();
        fileName = "houses.csv";
    }
    QString error = executeImport(fileName);
    if (!error.isEmpty()) QMessageBox::warning(this, "Помилка!!!", error);
    LoadData();
    if (ext == "zip") QFile(fileName).remove();
}

QString MainWindow::executeImport(QString fileName) {

    QFile file(fileName);
    QTextCodec *codec = QTextCodec::codecForName("WINDOWS-1251");
    int countStr = 0;
    if (file.open(QIODevice::ReadOnly |QIODevice::Text)) {
        QProgressDialog progress("Оновлення індексів...", "", 0, 1, this);
        progress.setWindowModality(Qt::WindowModal);
         while(!file.atEnd())  {
             file.readLine();
             countStr++;
         }
         file.close();
         progress.setValue(1);
    } else return "Не можу відкрити файл.";

    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) {

         QProgressDialog progress("Оновлення індексів...", "Перервати оновлення", 0, countStr, this);
         progress.setWindowModality(Qt::WindowModal);

         gotoDown();
         QSqlDatabase::database().close();
         QFile(FILEDB).remove();
         QSqlDatabase::database().open();
         createTable();
         LoadData();

         qu = "INSERT INTO `Houses`"
                  " (`area`, `region`, `place`, `index`, `street`, `nomer`, `vpz`)"
                  " VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7')";
          QSqlDatabase::database().transaction();
          file.readLine(); // пропускаем 1-ю строку
          bool vBreak = false;
          int ii = 0;  while(!file.atEnd())  {
              QByteArray ba = file.readLine().trimmed().replace("\'", "`").replace("\"", "");
              //читаем строку и удаляем ненужные символы
              QString str = codec->toUnicode(ba);
              QStringList lst = str.split(";");  //Делим строку на слова разделенные точкой с запятой
              QString query = qu.arg(lst.at(0)).arg(lst.at(1)).arg(lst.at(2)).arg(lst.at(3)).arg(lst.at(4)).arg(lst.at(5)).arg(lst.at(6));
              if (!sqlquery.exec(query)) return sqlquery.lastError().databaseText();
              if (progress.wasCanceled()) {
                  vBreak = true;
                  break;
              }
              ii++; progress.setValue(ii);
          }
          QSqlDatabase::database().commit();
          progress.setValue(countStr);
          if (vBreak) return "Імпорт був відхилений користувачем.";
      } else return "Не можу відкрити файл.";

    return "";

}

void MainWindow::text_changed() {
    LoadData();
}

void MainWindow::showNomer(QModelIndex index) {
    useindex = index;
    QString id_str = usemodel->data(usemodel->index(useindex.row(), 0), 0).toString();
    if (id_str.isEmpty()) return;

    QString nomer = usemodel->data(usemodel->index(useindex.row(), 6), 0).toString();
    ui->textEdit->setText(nomer);
}

void MainWindow::LoadData() {

    qu = "SELECT * from Houses WHERE 1";
    if (!ui->area_LE->text().isEmpty())  qu += QString(" and area LIKE '%%1%'").arg(ui->area_LE->text());
    if (!ui->region_LE->text().isEmpty())  qu += QString(" and region LIKE '%%1%'").arg(ui->region_LE->text());
    if (!ui->place_LE->text().isEmpty())  qu += QString(" and place LIKE '%%1%'").arg(ui->place_LE->text());
    if (!ui->index_LE->text().isEmpty())  qu += QString(" and `index` LIKE '%%1%'").arg(ui->index_LE->text());
    if (!ui->street_LE->text().isEmpty())  qu += QString(" and street LIKE '%%1%'").arg(ui->street_LE->text());
    if (!ui->nomer_LE->text().isEmpty())  qu += QString(" and nomer LIKE '%%1%'").arg(ui->nomer_LE->text());

    usemodel->setQuery(qu);
    if (usemodel->lastError().isValid()) {
        QMessageBox::warning(this, "Помилка!!!", usemodel->lastError().databaseText());
        return;
    }
//    while (usemodel->canFetchMore()) {
//         usemodel->fetchMore();
//    }

    SetModelsParam();
    ui->tableView->setModel(usemodel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(7);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);

    connect(ui->tableView->selectionModel(),
            SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(showNomer(QModelIndex)));

}

void MainWindow::SetModelsParam() {
    int ii = 0;

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Область");
    usemodel->setHeaderData(ii, Qt::Horizontal, QSize(150, 36), Qt::SizeHintRole);
    usemodel->setColor(ii, "red");
    usemodel->setPointSize(ii, 10);
    usemodel->setPrefix(ii, "   ");

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Район");
    usemodel->setHeaderData(ii, Qt::Horizontal, QSize(150, 36), Qt::SizeHintRole);
    usemodel->setPointSize(ii, 10);
    usemodel->setColor(ii, "blue");

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Насел.пункт");
    usemodel->setHeaderData(ii, Qt::Horizontal, QSize(150, 36), Qt::SizeHintRole);
    usemodel->setPointSize(ii, 10);

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Індекс");
    usemodel->setHeaderData(ii, Qt::Horizontal, QSize(80, 36), Qt::SizeHintRole);
    usemodel->setHAlign(ii, "center");
    usemodel->setColor(ii, "red");
    usemodel->setPointSize(ii, 12);

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Вулиця");
    usemodel->setHeaderData(ii, Qt::Horizontal, QSize(150, 36), Qt::SizeHintRole);
    usemodel->setPointSize(ii, 10);
    usemodel->setColor(ii, "blue");

    ii++;
    usemodel->setHeaderData(ii, Qt::Horizontal, "Номера");
    usemodel->setPointSize(ii, 10);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSqlDatabase::database().close();
    event->accept();
}
