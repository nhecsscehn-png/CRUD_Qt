#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("CRUD Qt + SQL Server"); // Ligne ajoutée n°1
    w.resize(900, 600); // Ligne ajoutée n°2
    w.show();
    return a.exec();
}

