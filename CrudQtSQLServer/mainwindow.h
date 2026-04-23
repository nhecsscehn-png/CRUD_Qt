#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// === AJOUTS OBLIGATOIRES POUR QUE ÇA COMPILE ===
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
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

    // Base de données
    QSqlDatabase db;
    QSqlTableModel *model = nullptr;

    // Widgets de l'interface
    QTableView   *tableView   = nullptr;
    QLineEdit    *leNom       = nullptr;
    QLineEdit    *lePrenom    = nullptr;
    QLineEdit    *leCode      = nullptr;
    QPushButton  *btnAjouter    = nullptr;
    QPushButton  *btnModifier   = nullptr;
    QPushButton  *btnSupprimer  = nullptr;
    QPushButton  *btnRafraichir = nullptr;
};

#endif // MAINWINDOW_H
