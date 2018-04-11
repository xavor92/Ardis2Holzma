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
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>

//Part
struct part {
    int number;     //Fortlaufende Nummer
    int A,R;        //A Anzahl, R Rotation
    QString L;      //Länge
    QString B;      //Breite
    QString MAT;    //Material
    QString MAT_OLD;//Backup des Ursprungsmaterials
    QString REF;    //Referenz Vollständig (PI#16939|FREE_SHELF)
    int refNumber;  //Referenz Number (16939)
    QString refString; //Ref String (FREE_SHELF)
    QString description; //Beschreibung aus BEM
    QString jobNumber;  //Auftragsnummer aus BEM2
    QString BEM3; //Bem3 komplett
    QString EDGE1, EDGE2, EDGE3, EDGE4; //Kanten
    QString EDGESEQ; //Kantenreihenfolge
    QString DRAWING1,DRAWING2,DRAWING3,DRAWING4,DRAWING5,DRAWING6,DRAWING7 ; //Produktionsreihenfolge
    QString SURFACE;
    QString Kommission, Kunde; //Manuelle Einträge
};

//Material
struct mat {
    QString matold;
    QString matnew;
    QString surface;
};

struct sheet {
    int number;
    QString MAT, REF, SAUM_R, SAUM_L, SAUM_O, SAUM_U;
    int ID, A;
    QString L, B, R, D;
};

QList<QString> select_list;
QList<mat> mat_list;
QList<part> obj_list;
part open_part;
QList<sheet>::iterator ite;
sheet open_sheet;
QString infileName, outfileName, line_n, dbfileName, infileFileName;
QString L, B, L_conv, B_conv, A, R, REF, ID, BEM;
bool mat_changed = false;
bool service_mode = false;
QProcess downloader;

QString pathDB, pathInputFile, pathOutput, pathLabels;
QSettings settings("Owestermann", "Ardis2Holzma");

void MainWindow::updatePathInputFile(QString path)
{
    pathInputFile = path;
    ui->InputFilePath->setText(path);
    settings.setValue("pathInputFile", path);
    settings.sync();
}

void MainWindow::updatePathOutputFile(QString path)
{
    pathOutput = path;
    ui->pathSaving->setText(path);
    settings.setValue("pathOutput", path);
    settings.sync();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // basic UI Setup
    ui->setupUi(this);
    ui->bySchrank_number->setChecked(true);
    ui->MainVertRight_2->hide();

    // Settings
    pathDB = settings.value("pathDB").toString();
    pathLabels = settings.value("pathLabels").toString();
    this->updatePathOutputFile(settings.value("pathOutput").toString());
    this->updatePathInputFile(settings.value("pathInputFile").toString());

    if (!pathLabels.isEmpty()) {
        ui->pathLabelLine->setText(pathLabels);
    }
    if (!pathDB.isEmpty()) {
        ui->dbLine->setText(pathDB);
        QFile db(pathDB);
        if (!db.open(QIODevice::ReadOnly)) {
            ui->dbLine->setText("");
            return;
        }
        QTextStream dbstream(&db);
        QString line2 = dbstream.readLine();
        while(!line2.isNull()){
            int firstDiv, secDiv;
            firstDiv = line2.indexOf(";");
            secDiv = line2.indexOf(";", firstDiv + 1);
            QString alt = line2.mid(0,firstDiv);
            QString neu = line2.mid(firstDiv + 1, secDiv - firstDiv - 1);
            QString surface = line2.mid(secDiv +1, -1);
            mat eintrag;
            eintrag.matnew = neu;
            eintrag.matold = alt;
            eintrag.surface = surface;
            mat_list.push_back(eintrag);
            //qDebug() << "Eingelesen: " << firstDiv << secDiv << neu << alt <<surface << mat_list.size() << endl;
            line2 = dbstream.readLine();
        }
        db.close();
    }
    QObject::connect( &downloader, SIGNAL(finished(int,QProcess::ExitStatus)),
                           this, SLOT(processFinished()));
    QObject::connect( &downloader, SIGNAL(readyRead()),
                           this, SLOT(downloaderData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitbutton_clicked()
{
    if(downloader.state() == QProcess::Running)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Es werden noch Labels heruntergeladen. Das Beenden von Ardis2Holzma führt zum Abbrechen der Downloads. Wollen sie Ardis2Holzma wirklich beenden?");
        msgBox.setWindowTitle("Downloads laufen noch");
        msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Abort);
        int clicked = msgBox.exec();
        if (clicked == QMessageBox::Abort) return;
    }
    qApp->quit();
}

void MainWindow::on_infileButton_clicked()
{
    infileName = QFileDialog::getOpenFileName(this, tr("Datei öffnen"), pathInputFile,
                        tr("Teileobj_list (*.stk);;Alle Dateien (*.*)"));
    if (!infileName.isEmpty()) {
        QFile infile(infileName);
        if (!infile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        infile.close();
    }
    QFileInfo info;
    info.setFile(infileName);
    infileFileName = info.fileName();
    ui->infileLine->setText(info.fileName());
}

void MainWindow::on_infileLine_textChanged()
{
    select_list.clear();
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
                //qDebug() << open_part.number << " " << open_part.REF  << " " << open_part.refString << endl;
            } else {							// defaults noch setzen
                open_part.number = inumber;
                open_part.R = 0;
                open_part.refNumber = -1;
                open_part.refString = "";
                open_part.REF = "";
                open_part.description = "";
                open_part.jobNumber = "";
                open_part.EDGE1.clear();
                open_part.EDGE2.clear();
                open_part.EDGE3.clear();
                open_part.EDGE4.clear();
                open_part.EDGESEQ.clear();
                open_part.DRAWING1.clear();
                open_part.DRAWING2.clear();
                open_part.DRAWING3.clear();
                open_part.DRAWING4.clear();
                open_part.DRAWING5.clear();
                open_part.DRAWING6.clear();
                open_part.DRAWING7.clear();
                open_part.BEM3.clear();
                open_part.Kommission.clear();
                open_part.Kunde.clear();
            }
        } else if(line.indexOf("MAT=") == 0)
        {
            open_part.MAT = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.MAT_OLD = open_part.MAT;
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
        }else if(line.indexOf("BEM=") == 0)
        {
            open_part.description = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
        }else if(line.indexOf("BEM2=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.jobNumber = BEM;
        }else if(line.indexOf("BEM3=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.BEM3 = BEM;
        }else if(line.indexOf("EDGE1=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.EDGE1 = BEM;
        }else if(line.indexOf("EDGE2=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.EDGE2 = BEM;
        }else if(line.indexOf("EDGE3=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.EDGE3 = BEM;
        }else if(line.indexOf("EDGE4=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.EDGE4 = BEM;
        }else if(line.indexOf("EDGESEQ=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            open_part.EDGESEQ = BEM;
        }else if(line.indexOf("DRAWING=") == 0)
        {
            BEM = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1); //7 Mal immer erste Zeichen bis zum ','
            if(BEM.size() > 0 && BEM.indexOf(",") == -1) //Nur ein Eintrag
            {
                open_part.DRAWING1 = BEM;
            }else{                                       //Mehr als 1 Eintrag
                open_part.DRAWING1 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 1
                BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                if(BEM.size() > 0 && BEM.indexOf(",") == -1)    //Wenn nurnoch einer Drin
                {
                    open_part.DRAWING2 = BEM;                   //Direkt in Drawing2
                }else{                                          //Mehr als 1 Eintrag
                    open_part.DRAWING2 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 2
                    BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                    if(BEM.size() > 0 && BEM.indexOf(",") == -1)    //Wenn nurnoch einer Drin
                    {
                        open_part.DRAWING3 = BEM;                   //Direkt in Drawing2
                    }else{                                          //Mehr als 1 Eintrag
                        open_part.DRAWING3 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 2
                        BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                        if(BEM.size() > 0 && BEM.indexOf(",") == -1)    //Wenn nurnoch einer Drin
                        {
                            open_part.DRAWING4 = BEM;                   //Direkt in Drawing2
                        }else{                                          //Mehr als 1 Eintrag
                            open_part.DRAWING4 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 2
                            BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                            if(BEM.size() > 0 && BEM.indexOf(",") == -1)    //Wenn nurnoch einer Drin
                            {
                                open_part.DRAWING5 = BEM;                   //Direkt in Drawing2
                            }else{                                          //Mehr als 1 Eintrag
                                open_part.DRAWING5 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 2
                                BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                                if(BEM.size() > 0 && BEM.indexOf(",") == -1)    //Wenn nurnoch einer Drin
                                {
                                    open_part.DRAWING6 = BEM;                   //Direkt in Drawing2
                                }else{                                          //Mehr als 1 Eintrag
                                    open_part.DRAWING6 = BEM.left(BEM.indexOf(",")); //Bis zum 1. Komma in Drawing 2
                                    BEM.remove(0, BEM.indexOf(",") + 1);            //incl Komma löschen
                                    open_part.DRAWING7 = BEM;                   //Direkt in Drawing2
                                }
                            }
                        }
                    }
                }
            }

        }else if(line.indexOf("REF=") == 0)
        {
            open_part.REF = line.mid(line.indexOf("=") + 1, line.length() - line.indexOf("=") - 1);
            if(line.indexOf("#") != -1 && line.indexOf("|") != -1){ //Standart-Format?
                open_part.refString = line.mid(line.indexOf("|") + 1, line.length() - line.indexOf("|") - 1);
                line_n = line.mid(line.indexOf("#") + 1, line.indexOf("|") - line.indexOf("#")-1);
                //qDebug() << line_n << line_n.toInt() << endl;
                open_part.refNumber = line_n.toInt();
            }
        }
        line = in.readLine();
    }
    ui->convertButton->setEnabled(true);
    mat_changed = false;
    QString count = QString::number(obj_list.size());
    ui->obj_count->setText(count);
    select_list.clear();
    QList<part>::iterator iterate = obj_list.begin();
    for(; iterate != obj_list.end(); iterate++){
        if(!select_list.contains(iterate->BEM3))
            select_list.append(iterate->BEM3);
    }
    QList<QString>::iterator iterate_strings = select_list.begin();
    ui->schrank_select->clear();
    for(; iterate_strings != select_list.end(); iterate_strings++)
        ui->schrank_select->insertItem(0, *iterate_strings);
}

void MainWindow::processFinished(){
    ui->downloadButton->setEnabled(true);
    ui->downloadButton->setText("Download");
    QMessageBox msgBox;
    msgBox.setText("Download abgeschlossen");
    msgBox.setWindowTitle("Download abgeschlossen");
    msgBox.exec();
}

void MainWindow::on_multiButton_clicked()
{
    if(obj_list.size() == 0){
        QMessageBox::critical(this, "Achtung", "Keine Datei geöfffnet, bitte erst Datei auswählen");
        return;
    } else {
        bool ok;
        int multi = ui->multiLine->text().toInt(&ok);
        if(!ok){
            QMessageBox::critical(this, "Achtung", "Wert 'Faktor' ungültig");
            return;
        }
        int counter = 0;
        QString compare_to = ui->schrank_select->currentText();
        if(ui->byBatch->isChecked()){
            QList<part>::iterator iterate = obj_list.begin();
            for(; iterate != obj_list.end(); iterate++){
                iterate->A = iterate->A * multi;
                counter++;
            }
        } else if(ui->bySchrank_number->isChecked()) {
            QList<part>::iterator iterate = obj_list.begin();
            for(; iterate != obj_list.end(); iterate++){
                if(iterate->BEM3 == compare_to){
                    iterate->A = iterate->A * multi;
                    counter++;
                }
            }
        } else if(ui->byRefNumber->isChecked()) {
            QList<part>::iterator iterate = obj_list.begin();
            for(; iterate != obj_list.end(); iterate++){
                if(iterate->REF == compare_to){
                    iterate->A = iterate->A * multi;
                    counter++;
                }
            }
        } else if(ui->byBEM2->isChecked()){
            QList<part>::iterator iterate = obj_list.begin();
            for(; iterate != obj_list.end(); iterate++){
                if(iterate->jobNumber == compare_to){
                    iterate->A = iterate->A * multi;
                    counter++;
                }
            }
        }
        QMessageBox::information(this, "Erfolgreich angepasst", "Anzahl bei " + QString::number(counter) + " Objekten erfolgreich angepasst");
    }
}

void MainWindow::on_dbButton_clicked()
{
    dbfileName = QFileDialog::getOpenFileName(this, tr("Datei öffnen"), pathDB,
                        tr("Mat-Datenbank(*.mdb);;Alle Dateien (*.*)"));

    if (!dbfileName.isEmpty()) {
        QFileInfo info;
        info.setFile(dbfileName);
        settings.setValue("pathDB", info.absoluteFilePath());
        pathDB = info.absolutePath();
        settings.sync();
        QFile db(dbfileName);
        if (!db.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        ui->dbLine->setText(dbfileName);
        QTextStream dbstream(&db);
        QString line2 = dbstream.readLine();
        while(!line2.isNull()){
            int firstDiv, secDiv;
            firstDiv = line2.indexOf(";");
            secDiv = line2.indexOf(";", firstDiv + 1);
            QString alt = line2.mid(0,firstDiv);
            QString neu = line2.mid(firstDiv + 1, secDiv - firstDiv - 1);
            QString surface = line2.mid(secDiv +1, -1);
            mat eintrag;
            eintrag.matnew = neu;
            eintrag.matold = alt;
            eintrag.surface = surface;
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
    if(mat_list.isEmpty()){
        QMessageBox::critical(this, tr("Fehler"), tr("Bitte erst eine Materialdatenbank öffnen"));
        return;
    }
    QList<part>::iterator iterate_obj = obj_list.begin();
    QList<mat>::iterator iterate_mat = mat_list.begin();
    bool new_materials = false;
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
                new_materials = true;
                break;
            }
        }
    }
    mat_changed = true;
    if(new_materials){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Neue Materialien", "Es gibt noch unbekannte Materialen.\nWollen sie die neuen Materialen jetzt definieren?\nWenn sie 'Nein' wählen, werden die Materialien unverändert übernommen.");
        if (reply == QMessageBox::Yes){
            QList<mat>::iterator iterate_mat = mat_list.begin();
            for(iterate_mat = mat_list.begin();iterate_mat != mat_list.end(); iterate_mat++){
                if(iterate_mat->matnew.isEmpty()){
                    QDialog dialog(this);
                    // Use a layout allowing to have a label next to each field
                    QFormLayout form(&dialog);
                    // Add some text above the fields
                    form.addRow(new QLabel("Bitte sie fest für: " + iterate_mat->matold));

                    // Add the lineEdits with their respective labels
                    QList<QLineEdit *> fields;
                    QLineEdit *lineEdit = new QLineEdit(&dialog);
                    QString label = QString("Holzma-Material:");
                    form.addRow(label, lineEdit);
                    fields << lineEdit;
                    lineEdit = new QLineEdit(&dialog);
                    label = QString("Oberfläche:");
                    form.addRow(label, lineEdit);
                    fields << lineEdit;
                    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
                    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                               Qt::Horizontal, &dialog);
                    form.addRow(&buttonBox);
                    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
                    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

                    // Show the dialog as modal

                    if (dialog.exec() == QDialog::Accepted) {
                        // If the user didn't dismiss the dialog, do something with the fields
                        iterate_mat->matnew = fields[0]->text();
                        iterate_mat->surface = fields[1]->text();
                    } else { return;}
                }
            }
        }
    }
    for(iterate_obj = obj_list.begin(); iterate_obj != obj_list.end(); iterate_obj++){
        for(iterate_mat = mat_list.begin(); iterate_mat != mat_list.end();iterate_mat++){
            //qDebug() << iterate_obj->number <<"Vergleiche: " << iterate_obj->MAT << " mit " << iterate_mat->matold<< endl;
            if(iterate_obj->MAT == iterate_mat->matold && !iterate_mat->matnew.isEmpty()) {
                iterate_obj->MAT = iterate_mat->matnew;
                iterate_obj->SURFACE = iterate_mat->surface;
            }
        }
    }
    ui->convertButton->setDisabled(true);
}

void MainWindow::on_SaveDBButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Aktuelle Materialdatenbank speichern?", "Soll die aktuelle Datenbank geschrieben werden?");
    if (reply == QMessageBox::Yes){
        QList<mat>::iterator iterate_mat = mat_list.begin();
        QFile dbFile(ui->dbLine->text());
        dbFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
        QTextStream out(&dbFile);
        for(iterate_mat = mat_list.begin();iterate_mat != mat_list.end(); iterate_mat++){
            out << iterate_mat->matold << ";" << iterate_mat->matnew << ";" << iterate_mat->surface << endl;
        }
        dbFile.close();
    }
}

void MainWindow::on_outfileButton_clicked()
{
    outfileName = "";
    outfileName = QFileDialog::getSaveFileName(this, tr("Speichern unter..."), pathOutput + '\\' + infileFileName + ".pnx",
                                               tr("Teileliste Holzma (*.pnx);;Alle Dateien (*.*)"));
    if (!outfileName.isEmpty()) {
        QFile outfile(outfileName);
        if (!outfile.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        ui->outfileLine->setText(outfileName);

        QFileInfo info;
        info.setFile(outfileName);
        QFile outFile(outfileName);
        settings.setValue("pathOutput", info.absolutePath());
        pathOutput = info.absolutePath();
        settings.sync();
        outFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
        QTextStream output(&outFile);
        output << "#ISTK;Anzahl;Materialcode;Oberfläche;Länge;Breite;Richtung;REF;REF Text extrahiert;REF Nummer extrahiert;Beschreibung aus BEM;Auftragsnummer aus BEM2;Schranknummer und Beschreibung aus BEM3;Kante1;Kante2;Kante3;Kante4;Kantenreihenfolge;Produktionsreihenfolge1;P.Reihenfolge2;3;4;5;6;7;Altes Material aus Ardis;Kommission;Kunde;Ivenza" << endl;
        for(QList<part>::iterator iterate = obj_list.begin(); iterate != obj_list.end(); iterate++){
            output << iterate->number   //Fortlaufende Nummer
                   << ";" << iterate->A        //Anzahl
                   << ";" << iterate->MAT      //Material
                   << ";" << iterate->SURFACE   //Oberfläche
                   << ";" << iterate->L        //Länge
                   << ";" << iterate->B        //Breite
                   << ";" << iterate->R        //Richtung
                   << ";" << iterate->REF      //REF ganzer String
                   << ";" << iterate->refString//REF String
                   << ";" << iterate->refNumber//REFNummer
                   << ";" << iterate->description//Beschreibung aus BEM
                   << ";" << iterate->jobNumber//Auftragsnummer aus BEM2
                   << ";" << iterate->BEM3     //Schranknummer und Beschreibung aus BEM3
                   << ";" << iterate->EDGE1    //Kante1
                   << ";" << iterate->EDGE2    //Kante2
                   << ";" << iterate->EDGE3    //Kante3
                   << ";" << iterate->EDGE4    //Kante4
                   << ";" << iterate->EDGESEQ  //Kantenreihenfolge
                   << ";" << iterate->DRAWING1  //Produktionsreihenfolge1
                   << ";" << iterate->DRAWING2  //Produktionsreihenfolge2
                   << ";" << iterate->DRAWING3  //Produktionsreihenfolge3
                   << ";" << iterate->DRAWING4  //Produktionsreihenfolge4
                   << ";" << iterate->DRAWING5  //Produktionsreihenfolge5
                   << ";" << iterate->DRAWING6  //Produktionsreihenfolge6
                   << ";" << iterate->DRAWING7  //Produktionsreihenfolge7
                   << ";" << iterate->MAT_OLD   //Material Backup Ardis
                   << ";" << ui->kommissionLine->text()   //Kommission
                   << ";" << ui->kundeLine->text()   //Kunde
                   << ";" << "Ivenza"   //Material Backup Ardis
                   << endl;
        }
        outFile.close();
        QMessageBox::information(this, "Speichern erfolgreich", "Teileliste erfolgreich gespeichert");
    }
}

void MainWindow::on_byBatch_clicked()
{
    ui->schrank_select->setDisabled(true);
}

void MainWindow::on_bySchrank_number_clicked()
{
    ui->schrank_select->setEnabled(true);
    select_list.clear();
    QList<part>::iterator iterate = obj_list.begin();
    for(; iterate != obj_list.end(); iterate++){
        if(!select_list.contains(iterate->BEM3))
            select_list.append(iterate->BEM3);
    }
    QList<QString>::iterator iterate_strings = select_list.begin();
    ui->schrank_select->clear();
    for(; iterate_strings != select_list.end(); iterate_strings++)
         ui->schrank_select->insertItem(0, *iterate_strings);
}

void MainWindow::on_byRefNumber_clicked()
{
    ui->schrank_select->setEnabled(true);
    select_list.clear();
    QList<part>::iterator iterate = obj_list.begin();
    for(; iterate != obj_list.end(); iterate++){
        if(!select_list.contains(iterate->REF))
            select_list.append(iterate->REF);
    }
    QList<QString>::iterator iterate_strings = select_list.begin();
    ui->schrank_select->clear();
    for(; iterate_strings != select_list.end(); iterate_strings++)
         ui->schrank_select->insertItem(0, *iterate_strings);
}

void MainWindow::on_byBEM2_clicked()
{
    ui->schrank_select->setEnabled(true);
    select_list.clear();
    QList<part>::iterator iterate = obj_list.begin();
    for(; iterate != obj_list.end(); iterate++){
        if(!select_list.contains(iterate->jobNumber))
            select_list.append(iterate->jobNumber);
    }
    QList<QString>::iterator iterate_strings = select_list.begin();
    ui->schrank_select->clear();
    for(; iterate_strings != select_list.end(); iterate_strings++)
         ui->schrank_select->insertItem(0, *iterate_strings);
}

void MainWindow::on_changePathLabelButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Speicherort für Labels wählen", pathLabels, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    path = path + "/";
    if(!path.isEmpty()) {
        settings.setValue("pathLabels", path);
        ui->pathLabelLine->setText(path);
    }
}

void MainWindow::on_changeDefaultInputPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Standardpfad für Eingabedatein", pathInputFile, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    path = path + "/";
    if(!path.isEmpty()) {
        this->updatePathInputFile(path);
    }
}


void MainWindow::on_changePathSavingButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Standardpfad für Ausgabedaten", pathOutput, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    path = path + "/";
    if(!path.isEmpty()) {
        this->updatePathOutputFile(path);
    }
}

void MainWindow::on_downloadButton_clicked()
{
    QString dlfolder = QDir::currentPath() + "/dlfiles/";

    // create if it does not exist
    if (!QDir(dlfolder).exists())
        QDir().mkdir(dlfolder);

    // create dlfile
    QString dlfile = dlfolder + infileFileName+".dl";
    QFile dllist;
    dllist.setFileName(dlfile);
    if (!dllist.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open dlfile"));
        return;
    }
    QTextStream dllistout(&dllist);
    for(QList<part>::iterator iterate = obj_list.begin(); iterate != obj_list.end(); iterate++){
        dllistout << iterate->refNumber << endl;
    }
    dllist.close();

    // start downloader
    QString downloaderName = "ivenzaDownloader.exe";
    if(!QFile(downloaderName).exists())
    {
        ui->downloaderOutput->append(QFileInfo(downloaderName).absoluteFilePath() + " seems to be missing.");
        return;
    }
    QString command = downloaderName + " -d \"" + dlfile + "\" -f \"" + ui->pathLabelLine->text() + "\"";
    qDebug() << command;
    downloader.start(command);
    downloader.setReadChannel(QProcess::StandardOutput);

    ui->downloadButton->setDisabled(true);
    ui->downloadButton->setText("Download läuft");
}

void MainWindow::on_serviceButton_clicked()
{
    if(service_mode)
    {
        service_mode = false;
        ui->MainVertRight_2->hide();
    } else {
        service_mode = true;
        ui->MainVertRight_2->show();
    }
}

void MainWindow::downloaderData(){
    QString output(downloader.readAllStandardOutput());
    QString status, max;
    ui->downloaderOutput->append(output);
    //qDebug() << output;
    int i = output.indexOf(" ", 6) + 1;         //Zweite Lücke +1 -> erste Zahl
    while(output[i] != ' '){
        status.append(output[i]);
        i++;
    }
    //qDebug() << status;
    i = output.indexOf("n") + 2;
    max = output.mid(i);
    //qDebug() << max;
    ui->downloadProgress->setMaximum(max.toInt());
    ui->downloadProgress->setValue(status.toInt());
}
