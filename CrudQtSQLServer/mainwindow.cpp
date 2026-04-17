#include "mainwindow.h"
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ====================== CONNEXION À SQL SERVER ======================
    connecterBaseDeDonnees(DEINODRUEN\SQLEXPRESS(SQL Server 15.0.2000 - Deinodruen\snh93)); // connecterBaseDeDonnees() (serveur, base de données…)

    // ====================== INTERFACE ======================
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // --- Formulaire ---
    QGroupBox *gbForm = new QGroupBox("Saisie", this);
    QFormLayout *formLayout = new QFormLayout(gbForm);

    leNom    = new QLineEdit(this);
    lePrenom = new QLineEdit(this);
    leCode   = new QLineEdit(this);

    formLayout->addRow("Nom :",    leNom);
    formLayout->addRow("Prénom :", lePrenom);
    formLayout->addRow("Code :",   leCode);

    // --- Boutons ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnAjouter    = new QPushButton("Ajouter", this);
    btnModifier   = new QPushButton("Modifier", this);
    btnSupprimer  = new QPushButton("Supprimer", this);
    btnRafraichir = new QPushButton("Rafraîchir", this);

    btnLayout->addWidget(btnAjouter);
    btnLayout->addWidget(btnModifier);
    btnLayout->addWidget(btnSupprimer);
    btnLayout->addWidget(btnRafraichir);

    // --- TableView ---
    tableView = new QTableView(this);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Assemblage
    mainLayout->addWidget(gbForm);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(tableView);

    setCentralWidget(central);

    // Connexions des boutons
    connect(btnAjouter,    &QPushButton::clicked, this, &MainWindow::on_btnAjouter_clicked);
    connect(btnModifier,   &QPushButton::clicked, this, &MainWindow::on_btnModifier_clicked);
    connect(btnSupprimer,  &QPushButton::clicked, this, &MainWindow::on_btnSupprimer_clicked);
    connect(btnRafraichir, &QPushButton::clicked, this, &MainWindow::on_btnRafraichir_clicked);

    // Chargement des données
    chargerDonnees();
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
}

// ====================== CONNEXION SQL SERVER ======================
void MainWindow::connecterBaseDeDonnees()
{
    db = QSqlDatabase::addDatabase("QODBC");   // Driver ODBC obligatoire pour SQL Server

    // === À MODIFIER SELON TON SERVEUR ===
    QString connString = "DRIVER={SQL Server};"
                         "SERVER=localhost;"           // ou ton serveur (ex: DESKTOP-XYZ\\SQLEXPRESS)
                         "DATABASE=CrudQtDB;"          // nom de ta base de données
                         "Trusted_Connection=Yes;";    // Authentification Windows (recommandé)

    // Si tu utilises un utilisateur + mot de passe :
    // "DRIVER={SQL Server};SERVER=localhost;DATABASE=CrudQtDB;UID=sa;PWD=tonmotdepasse;"

    db.setDatabaseName(connString);

    if (!db.open()) {
        QMessageBox::critical(this, "Erreur de connexion",
                              "Impossible de se connecter à SQL Server :\n" + db.lastError().text());
    }
}

// ====================== CHARGEMENT DES DONNÉES ======================
void MainWindow::chargerDonnees()
{
    if (!db.isOpen()) return;

    if (!model) {
        model = new QSqlTableModel(this, db);
        model->setTable("Personnes");
        model->setEditStrategy(QSqlTableModel::OnManualSubmit); // On valide manuellement
        model->setHeaderData(1, Qt::Horizontal, "Nom");
        model->setHeaderData(2, Qt::Horizontal, "Prénom");
        model->setHeaderData(3, Qt::Horizontal, "Code");
    }

    model->select();
    tableView->setModel(model);

    // Masquer la colonne ID (optionnel)
    tableView->hideColumn(0);
}

// ====================== CRUD ======================
void MainWindow::on_btnAjouter_clicked()
{
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare("INSERT INTO Personnes (nom, prenom, code) VALUES (?, ?, ?)");
    query.addBindValue(leNom->text());
    query.addBindValue(lePrenom->text());
    query.addBindValue(leCode->text());

    if (query.exec()) {
        QMessageBox::information(this, "Succès", "Personne ajoutée !");
        leNom->clear(); lePrenom->clear(); leCode->clear();
        model->select();        // Rafraîchit la table
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

void MainWindow::on_btnModifier_clicked()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Sélectionne une ligne à modifier !");
        return;
    }

    int id = model->index(index.row(), 0).data().toInt();

    QSqlQuery query(db);
    query.prepare("UPDATE Personnes SET nom = ?, prenom = ?, code = ? WHERE id = ?");
    query.addBindValue(leNom->text());
    query.addBindValue(lePrenom->text());
    query.addBindValue(leCode->text());
    query.addBindValue(id);

    if (query.exec()) {
        QMessageBox::information(this, "Succès", "Personne modifiée !");
        model->select();
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

void MainWindow::on_btnSupprimer_clicked()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Sélectionne une ligne à supprimer !");
        return;
    }

    if (QMessageBox::question(this, "Confirmation", "Supprimer cette personne ?") != QMessageBox::Yes)
        return;

    int id = model->index(index.row(), 0).data().toInt();

    QSqlQuery query(db);
    query.prepare("DELETE FROM Personnes WHERE id = ?");
    query.addBindValue(id);

    if (query.exec()) {
        model->select();
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

void MainWindow::on_btnRafraichir_clicked()
{
    if (model)
        model->select();
}
