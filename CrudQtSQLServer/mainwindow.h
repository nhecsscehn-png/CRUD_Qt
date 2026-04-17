#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAjouter_clicked();
    void on_btnModifier_clicked();
    void on_btnSupprimer_clicked();
    void on_btnRafraichir_clicked();

private:
    void connecterBaseDeDonnees();
    void chargerDonnees();

    QSqlDatabase db;
    QSqlTableModel *model = nullptr;

    // Widgets créés en code
    QTableView *tableView;
    QLineEdit *leNom;
    QLineEdit *lePrenom;
    QLineEdit *leCode;
    QPushButton *btnAjouter;
    QPushButton *btnModifier;
    QPushButton *btnSupprimer;
    QPushButton *btnRafraichir;
};

#endif // MAINWINDOW_H
