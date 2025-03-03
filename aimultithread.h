#ifndef AIMULTITHREAD_H
#define AIMULTITHREAD_H

#include <vector>
#include <QObject>
#include <QtConcurrent>
#include "chessboard.h"

class AIMultiThread : public QObject
{
    Q_OBJECT
public:
    explicit AIMultiThread(QObject *parent = nullptr);
    AIMultiThread(ChessBoard b, Team t);
    ~AIMultiThread();
    std::optional<ChessBoard> get_best();
    void start();

private:
    QTimer *future_checking_timer;
    Team team;
    ChessBoard board;
    std::vector<QFuture<int>> futures;
    int num_finished_futures;
    std::vector<int> scores;
    std::vector<ChessBoard> children;

private slots:
    void check_futures();

signals:
    void finished();
    void think_updated(float percent);
};

#endif // AIMULTITHREAD_H
