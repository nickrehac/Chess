#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "boardui.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_think_updated(float percent);
    void on_move_made(Team t);
    void on_evaluation_updated(int value);
    void on_think_finished();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
