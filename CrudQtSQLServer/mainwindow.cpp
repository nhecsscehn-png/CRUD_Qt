#include "mainwindow.h"           // Fichier d'en-tête de notre classe MainWindow (déclarations des membres et slots)

#include <QTableView>             // Widget qui affiche les données sous forme de tableau (notre QTableView)
#include <QLineEdit>              // Champ de saisie texte (utilisé pour Nom, Prénom et Code)
#include <QPushButton>            // Boutons cliquables (Ajouter, Modifier, Supprimer, Rafraîchir)

#include <QVBoxLayout>            // Layout vertical (empile les éléments de haut en bas)
#include <QHBoxLayout>            // Layout horizontal (place les éléments côte à côte)
#include <QFormLayout>            // Layout spécialisé pour les formulaires (label + champ sur la même ligne)
#include <QGroupBox>              // Cadre avec titre pour regrouper le formulaire de saisie

#include <QMessageBox>            // Boîtes de dialogue pour afficher des messages (succès, erreurs, confirmation)

#include <QSqlError>              // Permet de récupérer et afficher les erreurs SQL détaillées
#include <QSqlQuery>              // Permet d'exécuter des requêtes SQL (INSERT, UPDATE, DELETE)
#include <QSqlDatabase>           // Gestion de la connexion à la base de données
#include <QSqlTableModel>         // Modèle qui relie automatiquement une table SQL à un QTableView

#include <QHeaderView>            // Permet de personnaliser l'en-tête du tableau (étirement des colonnes, etc.)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ====================== CONNEXION À SQL SERVER ======================
    connecterBaseDeDonnees();        // Appel correct (sans arguments)

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

// ====================== CONNEXION À SQL SERVER ======================
void MainWindow::connecterBaseDeDonnees()
{
    db = QSqlDatabase::addDatabase("QODBC");   // Driver ODBC pour SQL Server

    // === CHAÎNE DE CONNEXION À MODIFIER SELON TON SERVEUR ===
    QString connString = "DRIVER={SQL Server};"
                         "SERVER=DEINODRUEN\\SQLEXPRESS;"   // ← Important : double backslash
                         "DATABASE=vde;"               // Nom de la base de données
                         "Trusted_Connection=Yes;";         // Authentification Windows

    db.setDatabaseName(connString);

    if (db.open()) {
        QMessageBox::information(this, "Connexion réussie", "Connecté à SQL Server avec succès !");
    } else {
        QMessageBox::critical(this, "Erreur de connexion",
                              "Impossible de se connecter à SQL Server :\n"
                                  + db.lastError().text());
    }
}

// ====================== CHARGEMENT DES DONNÉES ======================
void MainWindow::chargerDonnees()
{
    if (!db.isOpen()) return;

    if (!model) {
        model = new QSqlTableModel(this, db);
        model->setTable("Personnes");
        model->setEditStrategy(QSqlTableModel::OnManualSubmit);
        model->setHeaderData(1, Qt::Horizontal, "Nom");
        model->setHeaderData(2, Qt::Horizontal, "Prénom");
        model->setHeaderData(3, Qt::Horizontal, "Code");
    }

    model->select();
    tableView->setModel(model);
    tableView->hideColumn(0);   // Masque la colonne ID
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
        QMessageBox::information(this, "Succès", "Personne ajoutée avec succès !");
        leNom->clear();
        lePrenom->clear();
        leCode->clear();
        model->select();        // Rafraîchit la table
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

void MainWindow::on_btnModifier_clicked()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une ligne à modifier !");
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
        QMessageBox::information(this, "Succès", "Personne modifiée avec succès !");
        model->select();
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

void MainWindow::on_btnSupprimer_clicked()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une ligne à supprimer !");
        return;
    }

    if (QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette personne ?")
        != QMessageBox::Yes)
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
