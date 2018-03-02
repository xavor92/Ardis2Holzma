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

    void on_infileLine_textChanged(const QString &arg1);

    void on_multiButton_clicked();

    void on_dbButton_clicked();

    void on_convertButton_clicked();

    void on_outfileButton_clicked();

    void on_sheetLine_textChanged(const QString &arg1);

    void on_sheetButton_clicked();

    void on_convertsheetButton_clicked();

    void on_kombiBox_stateChanged(int arg1);

    void on_outsheetfileButton_clicked();

    void on_outsheetLine_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
