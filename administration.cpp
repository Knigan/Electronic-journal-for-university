#include "administration.h"
#include "ui_administration.h"

Administration::Administration(QSqlDatabase* DB, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Administration),
    db(DB)
{
    ui->setupUi(this);

    connect(ui->AddObjectButton, &QPushButton::clicked, this, &Administration::addObject);
    connect(ui->RemoveObjectButton, &QPushButton::clicked, this, &Administration::removeObject);
    connect(ui->RemoveStudentButton, &QPushButton::clicked, this, &Administration::removeStudent);
    connect(ui->RemoveTeacherButton, &QPushButton::clicked, this, &Administration::removeTeacher);
    connect(ui->AssignObject, &QPushButton::clicked, this, &Administration::assignObject);
    connect(ui->GroupButton, &QPushButton::clicked, this, &Administration::createGroup);
    connect(ui->SetTeacherButton, &QPushButton::clicked, this, &Administration::setTeacher);

    refresh();
    exec();
}

Administration::~Administration()
{
    delete ui;
}

void Administration::refresh() {
    combobox_work("objects", "name", ui->RemoveObjectComboBox);
    combobox_work("groups", "name", ui->GroupComboBox);
    combobox_work("objects", "name", ui->ObjectComboBox);
    combobox_work("groups", "name", ui->StudentGroupComboBox);
    combobox_work("students", "surname", ui->StudentSurnameComboBox);
    combobox_work("students", "name", ui->StudentNameComboBox);
    combobox_work("objects", "name", ui->TeacherObjectComboBox);
    combobox_work("teachers", "surname", ui->TeacherSurnameComboBox);
    combobox_work("teachers", "name", ui->TeacherNameComboBox);
}

void Administration::combobox_work(const QString& table, const QString& field, QComboBox* Box) {
    Box->clear();
    QSqlQuery* query = new QSqlQuery(*db);
    QSqlQueryModel* querymodel = new QSqlQueryModel;
    query->exec("SELECT COUNT(*) FROM " + table + ";");
    querymodel->setQuery(*query);
    int count = querymodel->data(querymodel->index(0, 0)).toInt();

    int j = 1;
    for (int i = 1; j <= count; ++i) {
        query->exec("SELECT " + field + " FROM " + table + " WHERE id = " + QString::number(i) + ";");
        querymodel->setQuery(*query);
        QString str = querymodel->data(querymodel->index(0, 0)).toString();
        if (str != "") {
            Box->addItem(str);
            ++j;
        }
    }
}

QSqlQueryModel* Administration::makeQuery(const QString& queryString)
{
    QSqlQuery* query = new QSqlQuery(*db);
    QSqlQueryModel *querymodel = new QSqlQueryModel;
    if (query->exec(queryString))
    {
        querymodel->setQuery(*query);
    }
    return querymodel;
}

void Administration::addObject() {
    QSqlQuery* query = new QSqlQuery(*db);

    QSqlQueryModel* querymodel = new QSqlQueryModel;
    QString str = ui->AddObjectLineEdit->text();

    querymodel = makeQuery("SELECT id FROM objects WHERE name = '" + str + "';");
    if (querymodel->rowCount() != 0)
        ui->ErrorLabel->setText("Этот предмет уже существует!");
    else {
        if (str != "") {
            query->exec("INSERT INTO objects VALUES (DEFAULT, '" + str + "');");
            ui->ErrorLabel->clear();
        }
        else
            ui->ErrorLabel->setText("Введите название предмета!");
    }

    refresh();
}

void Administration::removeObject() {
    QSqlQuery* query = new QSqlQuery(*db);
    query->exec("DELETE FROM objects WHERE name = '" + ui->RemoveObjectComboBox->currentText() + "';");
    refresh();
}

void Administration::removeStudent() {
    QSqlQuery* query = new QSqlQuery(*db);
    QSqlQueryModel* querymodel = new QSqlQueryModel;

    querymodel = makeQuery("SELECT id FROM groups WHERE name = '" + ui->GroupComboBox->currentText() + "';");
    int group_id = querymodel->data(querymodel->index(0, 0)).toInt();

    querymodel = makeQuery("SELECT * FROM students WHERE surname = '" + ui->StudentSurnameComboBox->currentText()
                           + "' AND name = '" + ui->StudentNameComboBox->currentText() + "' AND group_id = "
                           + QString::number(group_id) + ";");
    if (querymodel->rowCount() == 0) {
        ui->ErrorLabel->setText("Такого студента нет в выбранной группе!");
    }
    else {
        query->exec("DELETE FROM students WHERE surname = '" + ui->StudentSurnameComboBox->currentText()
                    + "' AND name = '" + ui->StudentNameComboBox->currentText() + "' AND group_id = "
                    + QString::number(group_id) + ";");
    }

    refresh();
}

void Administration::removeTeacher() {
    QSqlQuery* query = new QSqlQuery(*db);

    QSqlQueryModel* querymodel = new QSqlQueryModel;

    querymodel = makeQuery("SELECT id FROM objects WHERE name = '" + ui->TeacherObjectComboBox->currentText() + "';");
    int object_id = querymodel->data(querymodel->index(0, 0)).toInt();

    querymodel = makeQuery("SELECT id FROM teachers WHERE surname = '" + ui->TeacherSurnameComboBox->currentText()
                           + "' AND name = '" + ui->TeacherNameComboBox->currentText()
                           + "' AND object = " + QString::number(object_id) + ";");
    if (querymodel->rowCount() == 0) {
        ui->ErrorLabel->setText("Этого преподавателя не существует, или он не ведёт выбранный предмет!");
    }
    else {
        int id = querymodel->data(querymodel->index(0, 0)).toInt();

        if (id != 1) {
            query->exec("DELETE FROM teachers WHERE surname = '" + ui->TeacherSurnameComboBox->currentText()
                        + "' AND name = '" + ui->TeacherNameComboBox->currentText()
                        + "' AND object = " + QString::number(object_id) + ";");
        }
        else
            ui->ErrorLabel->setText("Нельзя удалить старшего преподавателя!");

    }
    refresh();
}

void Administration::setTeacher() {
    QSqlQuery* query = new QSqlQuery(*db);

    QSqlQueryModel* querymodel = new QSqlQueryModel;

    querymodel = makeQuery("SELECT id FROM objects WHERE name = '" + ui->TeacherObjectComboBox->currentText() + "';");
    int object_id = querymodel->data(querymodel->index(0, 0)).toInt();

    querymodel = makeQuery("SELECT * FROM teachers WHERE surname = '" + ui->TeacherSurnameComboBox->currentText()
                           + "' AND name = '" + ui->TeacherNameComboBox->currentText()
                           + "' AND object = " + QString::number(object_id) + ";");

    if (querymodel->rowCount() != 0)
        ui->ErrorLabel->setText("Этот преподаватель уже ведёт этот предмет!");
    else {
        querymodel = makeQuery("SELECT login, password FROM teachers WHERE surname = '" + ui->TeacherSurnameComboBox->currentText()
                               + "' AND name = '" + ui->TeacherNameComboBox->currentText() + "';");
        QString login = querymodel->data(querymodel->index(0, 0)).toString();
        QString password = querymodel->data(querymodel->index(0, 1)).toString();

        query->exec("INSERT INTO teachers VALUES(DEFAULT, '" + ui->TeacherSurnameComboBox->currentText()
                    + "', '" + ui->TeacherNameComboBox->currentText()
                    + "', " + QString::number(object_id)
                    + ", '" + login + "', '" + password + "');");
    }

    refresh();
}

void Administration::assignObject() {
    QSqlQuery* query = new QSqlQuery(*db);
    QSqlQueryModel* querymodel = new QSqlQueryModel;

    querymodel = makeQuery("SELECT id FROM groups WHERE name = '" + ui->GroupComboBox->currentText() + "';");
    int group_id = querymodel->data(querymodel->index(0, 0)).toInt();

    querymodel = makeQuery("SELECT id FROM objects WHERE name = '" + ui->ObjectComboBox->currentText() + "';");
    int object_id = querymodel->data(querymodel->index(0, 0)).toInt();

    query->exec("INSERT INTO groups_and_objects VALUES (" + QString::number(group_id) + ", " + QString::number(object_id) + ");");
    refresh();
}

void Administration::createGroup() {
    QSqlQuery* query = new QSqlQuery(*db);

    QSqlQueryModel* querymodel = new QSqlQueryModel;
    QString str = ui->GroupLineEdit->text();

    querymodel = makeQuery("SELECT id FROM groups WHERE name = '" + str + "';");
    if (querymodel->rowCount() != 0)
        ui->ErrorLabel->setText("Такая группа уже существует!");
    else {
        if (str != "") {
            query->exec("INSERT INTO groups VALUES (DEFAULT, '" + str + "');");
            ui->ErrorLabel->clear();
        }
        else
            ui->ErrorLabel->setText("Введите имя группы!");
    }

    refresh();
}
