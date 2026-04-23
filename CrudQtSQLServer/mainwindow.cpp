#include "mainwindow.h"
#include <QTableView> // Widget qui affiche les données sous forme de tableau
#include <QLineEdit> // Champs de saisie pour Nom, Prénom et Code
#include <QPushButton> // Boutons cliquables (Ajouter, Modifier, etc.)
#include <QVBoxLayout> // Layout vertical principal (empile les sections)
#include <QHBoxLayout> // Layout horizontal pour placer les boutons côte à côte
#include <QFormLayout> // Layout spécialisé pour les formulaires (Label + Champ)
#include <QGroupBox> // Cadre avec titre pour regrouper le formulaire
#include <QMessageBox> // Boîtes de dialogue (messages de succès, erreurs, confirmation)
#include <QSqlError> // Pour récupérer les détails des erreurs SQL
#include <QSqlQuery> // Pour exécuter des requêtes SQL (INSERT, UPDATE, DELETE)
#include <QHeaderView> // Pour personnaliser l'en-tête du tableau (étirement des colonnes)
#include <QSqlDatabase> // Gestion de la connexion à la base de données
#include <QSqlTableModel> // Modèle qui relie automatiquement une table SQL au QTableView

// ====================== CONSTRUCTEUR ======================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) // Appel du constructeur de la classe mère QMainWindow
{
    // ====================== CONNEXION À LA BASE DE DONNÉES ======================
    connecterBaseDeDonnees(); // Établit la connexion à SQL Server au démarrage

    // ====================== CRÉATION DE L'INTERFACE GRAPHIQUE ======================
    QWidget *central = new QWidget(this); // Widget central obligatoire pour QMainWindow
    QVBoxLayout *mainLayout = new QVBoxLayout(central); // Layout vertical principal de la fenêtre

    // --- Section Formulaire ---
    QGroupBox *gbForm = new QGroupBox("Saisie", this); // Cadre avec titre "Saisie"
    QFormLayout *formLayout = new QFormLayout(gbForm); // Layout pour aligner proprement labels et champs

    // Création des champs de saisie
    leNom = new QLineEdit(this); // Champ pour le Nom
    lePrenom = new QLineEdit(this); // Champ pour le Prénom
    leCode = new QLineEdit(this); // Champ pour le Code

    // Ajout des lignes dans le formulaire (Label + Champ)
    formLayout->addRow("Nom :", leNom);
    formLayout->addRow("Prénom :", lePrenom);
    formLayout->addRow("Code :", leCode);

    // --- Section Boutons ---
    QHBoxLayout *btnLayout = new QHBoxLayout(); // Layout horizontal pour les boutons
    btnAjouter = new QPushButton("Ajouter", this); // Bouton pour ajouter une personne
    btnModifier = new QPushButton("Modifier", this); // Bouton pour modifier la personne sélectionnée
    btnSupprimer = new QPushButton("Supprimer", this); // Bouton pour supprimer la personne sélectionnée
    btnRafraichir = new QPushButton("Rafraîchir", this); // Bouton pour recharger les données

    // Ajout des boutons dans le layout horizontal
    btnLayout->addWidget(btnAjouter);
    btnLayout->addWidget(btnModifier);
    btnLayout->addWidget(btnSupprimer);
    btnLayout->addWidget(btnRafraichir);

    // --- TableView (tableau d'affichage) ---
    tableView = new QTableView(this); // Crée le tableau qui affichera les données
    tableView->horizontalHeader()->setStretchLastSection(true); // La dernière colonne prend tout l'espace restant
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // Sélectionne des lignes entières

    // ====================== ASSEMBLAGE DE L'INTERFACE ======================
    mainLayout->addWidget(gbForm); // Ajoute le formulaire en haut
    mainLayout->addLayout(btnLayout); // Ajoute les boutons en dessous
    mainLayout->addWidget(tableView); // Ajoute le tableau qui prend le reste de l'espace
    setCentralWidget(central); // Définit le widget central de la fenêtre principale

    // ====================== CONNEXIONS DES SIGNAUX ======================
    // Connexion des boutons aux slots correspondants
    connect(btnAjouter, &QPushButton::clicked, this, &MainWindow::on_btnAjouter_clicked);
    connect(btnModifier, &QPushButton::clicked, this, &MainWindow::on_btnModifier_clicked);
    connect(btnSupprimer, &QPushButton::clicked, this, &MainWindow::on_btnSupprimer_clicked);
    connect(btnRafraichir, &QPushButton::clicked, this, &MainWindow::on_btnRafraichir_clicked);

    // Chargement initial des données depuis la base
    chargerDonnees();
}

// ====================== DESTRUCTEUR ======================
MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close(); // Ferme proprement la connexion à la base de données
}

// ====================== CONNEXION À SQL SERVER ======================
void MainWindow::connecterBaseDeDonnees()
{
    db = QSqlDatabase::addDatabase("QODBC"); // Crée une connexion utilisant le driver ODBC

    // Construction de la chaîne de connexion
    QString connString = "DRIVER={SQL Server};"
                         "SERVER=DEINODRUEN\\SQLEXPRESS;" // Nom du serveur + instance
                         "DATABASE=vde;" // Nom de la base de données
                         "Trusted_Connection=Yes;"; // Utilise l'authentification Windows

    db.setDatabaseName(connString); // Applique la chaîne de connexion

    // Test de la connexion
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
    if (!db.isOpen()) return; // Si pas connecté, on ne fait rien

    if (!model) {
        model = new QSqlTableModel(this, db); // Crée le modèle SQL lié à notre base
        model->setTable("Personnes"); // Spécifie la table à afficher
        model->setEditStrategy(QSqlTableModel::OnManualSubmit); // On valide les modifications manuellement

        // Personnalisation des en-têtes de colonnes
        model->setHeaderData(1, Qt::Horizontal, "Nom"); // Permet de renommer "nom" en "Nom"
        model->setHeaderData(2, Qt::Horizontal, "Prénom"); // Permet de renommer "prenom" en "Prénom"
        model->setHeaderData(3, Qt::Horizontal, "Code"); // Permet de renommer "code" en "Code"
    }

    model->select(); // Charge les données depuis la base
    tableView->setModel(model); // Associe le modèle au tableau
    tableView->hideColumn(0); // Masque la colonne ID (clé primaire)
}

// ====================== FONCTIONS CRUD ======================

// Ajouter une nouvelle personne
void MainWindow::on_btnAjouter_clicked()
{
    if (!db.isOpen()) return; // Si la base de données N'EST PAS ouverte sort immédiatement de la fonction (évite un crash)

    // ====================== NOUVELLE VÉRIFICATION ANTIDOUBLON ======================
    // Vérifie si une personne avec le même Nom + Prénom + Code existe déjà
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM Personnes WHERE nom = ? AND prenom = ? AND code = ?");
    checkQuery.addBindValue(leNom->text());
    checkQuery.addBindValue(lePrenom->text());
    checkQuery.addBindValue(leCode->text());

    if (checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if (count > 0) {
            QMessageBox::warning(this, "Doublon détecté",
                                 "Une personne avec ce Nom, Prénom et Code existe déjà dans la base de données.");
            return; // On annule l'ajout
        }
    }
    // ============================================================================

    QSqlQuery query(db); // Constructeur : lie la requête à la connexion db
    query.prepare("INSERT INTO Personnes (nom, prenom, code) VALUES (?, ?, ?)"); // Voir https://imgur.com/a/JlJ69uH
    query.addBindValue(leNom->text()); // Ajouter la valeur qui était dans le champs. Voir https://imgur.com/a/4vYLtlq
    query.addBindValue(lePrenom->text()); // Pareil que la ligne du dessus mais avec le prénom
    query.addBindValue(leCode->text()); // Pareil que la ligne du dessus mais avec le code

    if (query.exec()) { // query.exec() : Envoie la requête préparée à la base de données pour exécution
        QMessageBox::information(this, "Succès", "Personne ajoutée avec succès !"); // Boite de dialogue qui affiche un message. Voir : https://imgur.com/a/skt4UHW
        leNom->clear(); // Vide les champs après ajout
        lePrenom->clear();
        leCode->clear();
        model->select(); // Rafraîchit le tableau (Recharge toutes les données depuis la table Personnes)
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text()); // Affiche l'erreur à l'utilisateur. Voir : https://imgur.com/a/6KmpSSp
    }
}

// Modifier la personne sélectionnée
void MainWindow::on_btnModifier_clicked()
{
    QModelIndex index = tableView->currentIndex(); // Récupère la ligne sélectionnée
    if (!index.isValid()) { // Si aucune ligne n'est sélectionné (index invalide). Voir https://imgur.com/a/dhBJCTW
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une ligne à modifier !"); // Message de prévention. Voir https://imgur.com/a/wGKvrXs
        return;
    }
    int id = model->index(index.row(), 0).data().toInt(); // Récupère l'ID de la personne (L'ID est la clé primaire). Voir https://imgur.com/a/InTPNm3
    QSqlQuery query(db);
    query.prepare("UPDATE Personnes SET nom = ?, prenom = ?, code = ? WHERE id = ?");
    query.addBindValue(leNom->text()); // Ajouter la valeur qui était dans le champs. Voir https://imgur.com/a/4vYLtlq
    query.addBindValue(lePrenom->text()); // Pareil que la ligne du dessus mais avec le prénom
    query.addBindValue(leCode->text()); // Pareil que la ligne du dessus mais avec le code
    query.addBindValue(id);
    if (query.exec()) { // Signifie "Si la requête est exécutée". Voir https://imgur.com/a/VRUpgoH
        QMessageBox::information(this, "Succès", "Personne modifiée avec succès !"); // Affiche une fenêtre avec un message
        model->select(); // Rafraîchit le tableau
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

// Supprimer la personne sélectionnée
void MainWindow::on_btnSupprimer_clicked()
{
    QModelIndex index = tableView->currentIndex(); // Récupère la ligne sélectionnée par l'utilisateur dans le tableau. Voir https://imgur.com/a/3YeBu4s
    if (!index.isValid()) { // Signifie "si un index n'est pas sélectionné"
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une ligne à supprimer !");
        return;
    }
    // Demande de confirmation avant suppression
    if (QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette personne ?") // Affiche une boîte de dialogue avec icône et boutons Oui/Non
        != QMessageBox::Yes) // Si la réponse N'EST PAS "Oui"
        return; // Sert à sortir immédiatement de la fonction (annule la suppression)

    int id = model->index(index.row(), 0).data().toInt(); // Récupère l'ID de la personne (L'ID est la clé primaire). Voir https://imgur.com/a/InTPNm3
    QSqlQuery query(db); // QSqlQuery : Permet d'exécuter des requêtes SQL (SELECT, INSERT, UPDATE, DELETE). Voir https://imgur.com/a/PVKHdqq
    query.prepare("DELETE FROM Personnes WHERE id = ?"); // Préparation de la requête à appliquer sur l'ID. Voir https://imgur.com/a/wgDwWsd
    query.addBindValue(id); // On remplace le '?' par l'ID. Voir https://imgur.com/a/Y31FDl9
    if (query.exec()) { // Exécute la requête
        model->select(); // Rafraîchit le tableau après suppression
    } else {
        QMessageBox::warning(this, "Erreur", query.lastError().text());
    }
}

// Rafraîchir l'affichage du tableau
void MainWindow::on_btnRafraichir_clicked()
{
    if (model)
        model->select(); // Recharge les données depuis la base
}
