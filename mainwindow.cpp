#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QWidget>
#include <QLineEdit>
#include <QList>
#include <QIODevice>
#include <QInputDialog>
#include <QDebug>

struct part {
    int number;
    int A,R;
    QString L;
    QString B;
    QString MAT;
    QString REF;
    int REF_number;
    QString REF_string;
};


//Material
struct mat {
    QString matold;
    QString matnew;
};

struct sheet {
    int number;
    QString MAT, REF, SAUM_R, SAUM_L, SAUM_O, SAUM_U;
    int ID, A;
    QString L, B, R, D;
};

QList<sheet> sheet_list;
QList<mat> mat_list;
QList<part> obj_list;
part open_part;
QList<sheet>::iterator ite;
sheet open_sheet;
QString infileName, outfileName, line_n, dbfileName, sheetfileName;
QString L, B, L_conv, B_conv, A, R, REF, ID;
bool mat_changed = false;




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->kombiBox->setChecked(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitbutton_clicked()
{
    qApp->quit();
}

void MainWindow::on_infileButton_clicked()
{
    infileName = QFileDialog::getOpenFileName(this, tr("Datei öffnen"), QString(),
                        tr("Teileobj_list (*.stk);;Alle Dateien (*.*)"));
    if (!infileName.isEmpty()) {
        QFile infile(infileName);
        if (!infile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
    }
    ui->infileLine->setText(infileName);
}

void MainWindow::on_infileLine_textChanged(const QString &arg1)
{
    sheet_list.clear();
    mat_list.clear();
    obj_list.clear();
    QFile infile(infileName);
    if (!infile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }
    QTextStream in(&infile);
    QString line = in.readLine();
    while(!line.isNull()){
        if(line.indexOf("ISTK-") != -1){	//Opener oder Closer
            QString snumber = line.mid(line.indexOf("-") +1, line.length() - line.indexOf("-") - 2);
            int inumber = snumber.toInt();
            if(open_part.number == inumber) { //Part bereits offen, also wieder zu
                obj_list.push_back(open_part);
                //qDebug() << open_part.number << " " << open_part.REF  << " " << open_part.REF_string << endl;
            } else {							// defaults noch setzen
                open_part.number = inumber;
                open_part.R = 0;
                open_part.REF_number = -1;
                open_part.REF_string = "";
                open_part.REF = "";
            }
        } else if(line.indexOf("MAT=") == 0)
        {
            open_part.MAT = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        } else if(line.indexOf("L=") == 0)
        {
            L = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.L = L;
        }else if(line.indexOf("B=") == 0)
        {
            B = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.B = B;
        }else if(line.indexOf("A=") == 0)
        {
            A = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.A = A.toInt();
        }else if(line.indexOf("R=") == 0)
        {
            R = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.R = R.toInt();
        }else if(line.indexOf("REF=") == 0)
        {
            open_part.REF = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            if(line.indexOf("#") != -1 && line.indexOf("|") != -1){ //Standart-Format?
                open_part.REF_string = line.mid(line.indexOf("|") + 1, line.length() - line.indexOf("|") - 1);
                line_n = line.mid(line.indexOf("#") + 1, line.indexOf("|") - line.indexOf("#")-1);
                //qDebug() << line_n << line_n.toInt() << endl;
                open_part.REF_number = line_n.toInt();
            }
        }
        line = in.readLine();
    }
    QString bla;
    bla = QString::number(obj_list.length());
    ui->obj_count->setText(bla);
    ui->convertButton->setEnabled(true);
    ui->multiButton->setEnabled(true);
    mat_changed = false;
    if(ui->kombiBox->isChecked()){
        sheetfileName = infileName.left(infileName.lastIndexOf(".") + 1);
        sheetfileName.append("std");
        if(QFile::exists(sheetfileName))
            ui->sheetLine->setText(sheetfileName);
    }
}

void MainWindow::on_multiButton_clicked()
{
    if(obj_list.size() == 0){
        QMessageBox::critical(this, "Achtung", "Keine Datei geöfffnet, bitte erst Datei auswählen");
        return;
    } else {
        bool ok;
        int from, to, multi;
        from = ui->fromLine->text().toInt(&ok);
        if(!ok){
            QMessageBox::critical(this, "Achtung", "Wert 'von' ungültig");
            return;
        }
        to = ui->toLine->text().toInt(&ok);
        if(!ok){
            QMessageBox::critical(this, "Achtung", "Wert 'bis' ungültig");
            return;
        }
        multi = ui->multiLine->text().toInt(&ok);
        if(!ok){
            QMessageBox::critical(this, "Achtung", "Wert 'Faktor' ungültig");
            return;
        }
        QList<part>::iterator iterate = obj_list.begin();
        for(iterate; iterate != obj_list.end(); iterate++){
            if(iterate->REF_number >= from && iterate->REF_number <= to)
                iterate->A = iterate->A * multi;
        }
        QMessageBox::information(this, "Erfolgreich angepasst", "Anzahl erfolgreich angepasst");
    }
}

void MainWindow::on_dbButton_clicked()
{
    if(obj_list.size() == 0){
        QMessageBox::critical(this, "Achtung", "Keine Datei geöfffnet, bitte erst Datei auswählen");
        return;
    }
    dbfileName = QFileDialog::getOpenFileName(this, tr("Datei öffnen"), QString(),
                        tr("Mat-Datenbank(*.mdb);;Alle Dateien (*.*)"));
    if (!dbfileName.isEmpty()) {
        QFile db(dbfileName);
        if (!db.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        ui->dbLine->setText(dbfileName);
        QTextStream dbstream(&db);
        QString line2 = dbstream.readLine();
        while(!line2.isNull()){
            QString alt = line2.mid(0,line2.indexOf(";"));
            QString neu = line2.mid(line2.indexOf(";") + 1, -1);
            mat eintrag;
            eintrag.matnew = neu;
            eintrag.matold = alt;
            mat_list.push_back(eintrag);
            //qDebug() << "Eingelesen: " << alt << neu << endl;
            line2 = dbstream.readLine();
        }
        db.close();
    }
}

void MainWindow::on_convertButton_clicked()
{
    if(mat_changed){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Bereits umgewandelt", "Die Materialen wurden bereits umgewandelt, wollen sie sie wirklich ein weiteres Mal umwandeln?");
        if (reply == QMessageBox::No){
            return;
        }
    }
    QList<part>::iterator iterate_obj = obj_list.begin();
    QList<mat>::iterator iterate_mat = mat_list.begin();
    for(iterate_obj = obj_list.begin(); iterate_obj != obj_list.end(); iterate_obj++){
        for(iterate_mat = mat_list.begin(); iterate_mat != mat_list.end();iterate_mat++){
            //qDebug() << iterate_obj->number <<"Vergleiche: " << iterate_obj->MAT << " mit " << iterate_mat->matold<< endl;
            if(iterate_obj->MAT == iterate_mat->matold) {
                //qDebug() << "Found:" << iterate_obj->MAT << " = " << iterate_mat->matold << endl;
                break;
            }
            if(iterate_mat == --mat_list.end()){
                mat new_mat;
                new_mat.matold = iterate_obj->MAT;
                mat_list.push_back(new_mat);
                break;
            }
        }
    }
    mat_changed = true;
    bool new_materials = false;
    iterate_mat = mat_list.begin();
    for(iterate_mat = mat_list.begin();iterate_mat != mat_list.end(); iterate_mat++){
        if(iterate_mat->matnew.isEmpty()) new_materials = true;
    }
    if(new_materials){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Neue Materialien", "Es gibt noch unbekannte Materialen. \nWollen sie die neuen Materialen jetzt definieren oder unverändert in die Ausgabedatei übernehmen?");
        if (reply == QMessageBox::Yes){
            QList<mat>::iterator iterate_mat = mat_list.begin();
            for(iterate_mat = mat_list.begin();iterate_mat != mat_list.end(); iterate_mat++){
                if(iterate_mat->matnew.isEmpty()){
                    QString Text = "Alt: ";
                    Text.append(iterate_mat->matold);
                    Text.append(" Neu: ");
                    iterate_mat->matnew = QInputDialog::getText(this, "Bitte Material festlegen:", Text);
                    //qDebug() << iterate_mat->matold << iterate_mat->matnew << endl;
                }
            }
        }
    }
    for(iterate_obj = obj_list.begin(); iterate_obj != obj_list.end(); iterate_obj++){
        for(iterate_mat = mat_list.begin(); iterate_mat != mat_list.end();iterate_mat++){
            //qDebug() << iterate_obj->number <<"Vergleiche: " << iterate_obj->MAT << " mit " << iterate_mat->matold<< endl;
            if(iterate_obj->MAT == iterate_mat->matold && !iterate_mat->matnew.isEmpty()) {
                iterate_obj->MAT = iterate_mat->matnew;
            }
        }
    }
    for(iterate_obj = obj_list.begin(); iterate_obj != obj_list.end(); iterate_obj++){
        //qDebug() << iterate_obj->number << iterate_obj->MAT << endl;
    }
    if(new_materials){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Materialienobj_list speichern?", "Sollen die neu hinzugefügten Materialien in die Datenbank geschrieben werden?");
        if (reply == QMessageBox::Yes){
            QList<mat>::iterator iterate_mat = mat_list.begin();
            QFile dbFile(dbfileName);
            dbFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
            QTextStream out(&dbFile);
            for(iterate_mat = mat_list.begin();iterate_mat != mat_list.end(); iterate_mat++){
                out << iterate_mat->matold << ";" << iterate_mat->matnew << endl;
            }
            dbFile.close();
        }
    }
    QMessageBox::information(this, "Erfolgreich umgewandelt", "Alle Materialien erfolgreich umgewandelt");
    ui->convertButton->setDisabled(true);
    ui->convertsheetButton->setEnabled(true);
    if(ui->kombiBox->isChecked())
        MainWindow::on_convertsheetButton_clicked();
}

void MainWindow::on_outfileButton_clicked()
{
    outfileName = QFileDialog::getSaveFileName(this, tr("Speichern unter..."), QString(),
                        tr("Teileliste Holzma (*.pnx);;Alle Dateien (*.*)"));
    if (!outfileName.isEmpty()) {
        QFile outfile(outfileName);
        if (!outfile.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
    }
    ui->outfileLine->setText(outfileName);
    QFile outFile(outfileName);
    outFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream output(&outFile);
    output << "#ISTK;Anzahl;Materialcode;Länge;Breite;Richtung;REF_ganz;REF_String;REF_Number" << endl;
            for(QList<part>::iterator iterate = obj_list.begin(); iterate != obj_list.end(); iterate++){
                    output << iterate->number
                    << ";" << iterate->A
                    << ";" << iterate->MAT
                    << ";" << iterate->L
                    << ";" << iterate->B
                    << ";" << iterate->R
                    << ";" << iterate->REF
                    << ";" << iterate->REF_string
                    << ";" << iterate->REF_number
                    << endl;
            }
    outFile.close();
    if(ui->kombiBox->isChecked())
    {
        QString outsheetfileName = outfileName.left(outfileName.lastIndexOf(".") + 1);
        outsheetfileName.append("ptx");
        ui->outsheetLine->setText(outsheetfileName);
    }
    QMessageBox::information(this, "Speichern erfolgreich", "Teileliste erfolgreich gespeichert");
}

void MainWindow::on_sheetLine_textChanged(const QString &arg1)
{
    QFile sheetfile(sheetfileName);
    if (!sheetfile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }
    //ui->convertsheetButton->setEnabled();
    QTextStream sheet(&sheetfile);
    QString line = sheet.readLine();
    while(!line.isNull()){
        if(line.indexOf("ISTD-") != -1){	//Opener oder Closer
            QString snumber = line.mid(line.indexOf("-") +1, line.length() - line.indexOf("-") - 2);
            int inumber = snumber.toInt();
            if(open_sheet.number == inumber) { //Part bereits offen, also wieder zu
                sheet_list.push_back(open_sheet);
                //qDebug() << open_part.number << " " << open_part.REF  << " " << open_part.REF_string << endl;
            } else {							// defaults noch setzen
                open_sheet.number = inumber;
                open_sheet.R = "0";
            }
        } else if(line.indexOf("MATERIAAL=") == 0)
        {
            open_sheet.MAT = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        } else if(line.indexOf("LENGTE=") == 0)
        {
            L = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_sheet.L = L;
        }else if(line.indexOf("BREEDTE=") == 0)
        {
            B = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_sheet.B = B;
        }else if(line.indexOf("AANT=") == 0)
        {
            A = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_sheet.A = A.toInt();
        }else if(line.indexOf("RICHTING=") == 0)
        {
            R = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_sheet.R = R;
        }else if(line.indexOf("REFER=") == 0)
        {
            open_sheet.REF = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("DIKTE=") == 0)
        {
            open_sheet.D = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("ID=") == 0)
        {
            ID = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_sheet.ID = ID.toInt();
        }else if(line.indexOf("MINBRDBOVEN=") == 0)
        {
            //qDebug() << line;
            open_sheet.SAUM_O = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("MINBRDRECHTS=") == 0)
        {
            open_sheet.SAUM_R = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("MINBRDONDER=") == 0)
        {
            open_sheet.SAUM_U = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("MINBRDLINKS=") == 0)
        {
            open_sheet.SAUM_L = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }
        line = sheet.readLine();
    }
    for(ite = sheet_list.begin();ite != sheet_list.end(); ite++){
        //qDebug() << ite->ID << ite->D << ite->R << ite->A << endl;
    }
}

void MainWindow::on_sheetButton_clicked()
{
    sheetfileName = QFileDialog::getOpenFileName(this, tr("Datei öffnen"), QString(),
                        tr("Plattenobj_list (*.std);;Alle Dateien (*.*)"));
    if (!sheetfileName.isEmpty()) {
        QFile sheetfile(sheetfileName);
        if (!sheetfile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
    }
    ui->sheetLine->setText(sheetfileName);
}

void MainWindow::on_convertsheetButton_clicked()
{
    QList<sheet>::iterator iterate_sheet = sheet_list.begin();
    QList<mat>::iterator iterate_mat = mat_list.begin();
    for(iterate_sheet = sheet_list.begin(); iterate_sheet != sheet_list.end(); iterate_sheet++){
        for(iterate_mat = mat_list.begin(); iterate_mat != mat_list.end();iterate_mat++){
            //qDebug() << iterate_obj->number <<"Vergleiche: " << iterate_obj->MAT << " mit " << iterate_mat->matold<< endl;
            if(iterate_sheet->MAT == iterate_mat->matold && !iterate_mat->matnew.isEmpty()) {
                iterate_sheet->MAT = iterate_mat->matnew;
            }
        }
    }
    for(iterate_sheet = sheet_list.begin(); iterate_sheet != sheet_list.end(); iterate_sheet++){
        for(iterate_mat = mat_list.begin(); iterate_mat != mat_list.end();iterate_mat++){
            //qDebug() << iterate_obj->number <<"Vergleiche: " << iterate_obj->MAT << " mit " << iterate_mat->matold<< endl;
            if(iterate_sheet->REF == iterate_mat->matold && !iterate_mat->matnew.isEmpty()) {
                iterate_sheet->REF = iterate_mat->matnew;
            }
        }
    }
    for(iterate_sheet = sheet_list.begin(); iterate_sheet != sheet_list.end(); iterate_sheet++){
        //qDebug() << iterate_sheet->number << iterate_sheet->MAT << endl;
    }
    ui->convertsheetButton->setDisabled(true);
}

void MainWindow::on_kombiBox_stateChanged(int arg1)
{
    if(ui->kombiBox->isChecked())
    {
        ui->sheetButton->setDisabled(1);
        ui->convertsheetButton->setDisabled(1);
        ui->outsheetfileButton->setDisabled(1);
    } else{
        ui->sheetButton->setEnabled(1);
        ui->convertsheetButton->setEnabled(1);
        ui->outsheetfileButton->setEnabled(1);
    }
}

void MainWindow::on_outsheetfileButton_clicked()
{
    outfileName = QFileDialog::getSaveFileName(this, tr("Speichern unter..."), QString(),
                        tr("Plattenliste Holzma (*.ptx);;Alle Dateien (*.*)"));
    if (!outfileName.isEmpty()) {
        QFile outfile(outfileName);
        if (!outfile.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
    }
    ui->outsheetLine->setText(outfileName);
}

void MainWindow::on_outsheetLine_textChanged(const QString &arg1)
{
    QString outfileName = ui->outsheetLine->text();
    QFile outFile(outfileName);
    outFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream output(&outFile);
    output << "ID;REF;Anzahl;Länge;Breite;Richtung;Dicke;Material;Saum oben; Saum rechts;Saum unten;Saum links" << endl;
            for(QList<sheet>::iterator iterate = sheet_list.begin(); iterate != sheet_list.end(); iterate++){
                    output << iterate->ID
                    << ";" << iterate->REF
                    << ";" << iterate->A
                    << ";" << iterate->L
                    << ";" << iterate->B
                    << ";" << iterate->R
                    << ";" << iterate->D
                    << ";" << iterate->MAT
                    << ";" << iterate->SAUM_O
                    << ";" << iterate->SAUM_R
                    << ";" << iterate->SAUM_U
                    << ";" << iterate->SAUM_L
                    << endl;
            }
    outFile.close();
}
