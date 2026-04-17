use vde;

select name from sys.tables;

CREATE TABLE Personnes (
    id      INT IDENTITY(1,1) PRIMARY KEY,
    nom     NVARCHAR(100) NOT NULL,
    prenom  NVARCHAR(100) NOT NULL,
    code    NVARCHAR(50)  NOT NULL
);
