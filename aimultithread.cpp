#include "aimultithread.h"
#include "chessboard.h"

AIMultiThread::AIMultiThread(QObject *parent)
    : QObject{parent}
{}

AIMultiThread::AIMultiThread(ChessBoard b, Team t) {
    this->board = b;
    this->team = t;
    this->future_checking_timer = new QTimer();
}

AIMultiThread::~AIMultiThread() {
    delete future_checking_timer;
}

void AIMultiThread::start() {
    this->children = board.gen_filtered_children_boards(team);
    num_finished_futures = 0;
    for(int i = 0; i < children.size(); i++) {
        futures.push_back(QtConcurrent::run([=]{
            return children[i].alpha_beta(team_inverse(team));
        }));
    }
    this->connect(this->future_checking_timer, &QTimer::timeout, this, &AIMultiThread::check_futures);
    future_checking_timer->start(200);
}

void AIMultiThread::check_futures() {
    bool is_finished = true;
    int current_finished_futures = futures.size();
    for(int i = 0; i < futures.size(); i++) {
        if(!futures[i].isFinished()) {
            is_finished = false;
            current_finished_futures--;
        }
    }

    if(current_finished_futures != num_finished_futures) {
        num_finished_futures = current_finished_futures;
        emit think_updated(num_finished_futures / (float) futures.size());
    }

    if(!is_finished) return;

    for(int i = 0; i < futures.size(); i++) {
        this->scores.push_back(futures[i].result());
    }

    emit finished();
}

std::optional<ChessBoard> AIMultiThread::get_best() {
    if(children.size() != scores.size() || scores.size() == 0) return std::nullopt;

    ChessBoard best_board = children[0];
    int best_score = scores[0];
    for(int i = 0; i < children.size(); i++) {
        if(best_score > scores[i]) {
            best_board = children[i];
            best_score = scores[i];
        }
    }

    return best_board;
}
