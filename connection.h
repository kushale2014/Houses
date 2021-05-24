#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

static const char* s_db_pragma[] = {
    "synchronous=OFF",
    "count_changes=OFF",
    "temp_store=MEMORY",
    NULL
};

static const QString FILEDB = "houses.db";

static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(FILEDB);
    if (!db.open()) {
        QMessageBox::warning(0, "Database Error", db.lastError().text());
        return false;
    }

    int i = 0;
    do {
        QString sql = QString("PRAGMA %1;").arg(s_db_pragma[i]);
        QSqlQuery query(db);
        query.prepare(sql);
        query.exec();
 
        if (query.lastError().isValid())
        {
           qDebug() << "Query: \"" << sql
                     << "\"; Error: \"" << query.lastError().text() << "\".";
           return false;
        }
    } while (s_db_pragma[++i]);

    return true;
}

#endif
