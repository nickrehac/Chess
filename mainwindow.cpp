#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->connect(this->ui->frame, &BoardUI::move_made, this, &MainWindow::on_move_made);
    this->connect(this->ui->frame, &BoardUI::evaluation_updated, this, &MainWindow::on_evaluation_updated);
    this->connect(this->ui->frame, &BoardUI::think_updated, this, &MainWindow::on_think_updated);
    this->connect(this->ui->frame, &BoardUI::think_finished, this, &MainWindow::on_think_finished);
    this->ui->thinkingBar->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    this->ui->frame->reset_board();
}

void MainWindow::on_think_updated(float percent) {
    this->ui->thinkingBar->setValue(this->ui->thinkingBar->maximum()*percent);
    this->ui->thinkingBar->update();
}

void MainWindow::on_move_made(Team t) {
    if(t == Team::Beta) {
        this->ui->pushButton->setEnabled(true);
    } else {
        if(this->ui->thinkingBar->isHidden()) {
            this->ui->pushButton->setEnabled(false);
            this->ui->thinkingBar->setValue(0);
            this->ui->thinkingBar->show();
            this->ui->thinkingBar->setVisible(true);
        }
    }
}

void MainWindow::on_evaluation_updated(int value) {
    int sigmoided_value = 200.0 / (1.0 + exp(-value/100.0)) - 100.0;
    this->ui->qualityBar->setValue(sigmoided_value);
    this->ui->qualityBar->update();
}

void MainWindow::on_think_finished() {
    this->ui->thinkingBar->setValue(0);
    this->ui->thinkingBar->hide();
    this->ui->thinkingBar->setVisible(false);
    this->ui->thinkingBar->update();
}
