#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_quitbutton_clicked();
    void on_infileButton_clicked();
    void on_infileLine_textChanged();
    void on_multiButton_clicked();
    void on_dbButton_clicked();
    void on_convertButton_clicked();
    void on_outfileButton_clicked();
    void on_byBatch_clicked();
    void on_bySchrank_number_clicked();
    void on_byRefNumber_clicked();
    void on_byBEM2_clicked();
    void on_changePathLabelButton_clicked();
    void on_downloadButton_clicked();
    void processFinished();
    void downloaderData();
    void on_serviceButton_clicked();
    void on_changeDefaultInputPathButton_clicked();
    void on_changePathSavingButton_clicked();
    void on_SaveDBButton_clicked();

private:
    Ui::MainWindow *ui;
    void updatePathInputFile(QString path);
    void updatePathOutputFile(QString path);
};

#endif // MAINWINDOW_H
