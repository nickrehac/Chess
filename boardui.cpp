#include "boardui.h"
#include <QMessageBox>
#include <algorithm>
#include <QColor>

BoardUI::BoardUI(QWidget * parent) : QFrame(parent)
{
    held_piece = std::nullopt;
    sprite_sheet = new QSvgRenderer(QString("boardsprites.svg"), this);
    if(!sprite_sheet->isValid()) {
        QMessageBox::critical(this, QString("Error!"), QString("boardsprites.svg failed to load.\nPlease make sure it is in the executable directory"));
        exit(20);
    }
    currentTurn = Team::Alpha;
}
BoardUI::~BoardUI() {
    delete sprite_sheet;
}
void BoardUI::paintEvent(QPaintEvent *e) {
    e->accept();
    float board_size = std::min(this->width(),this->height())*(1.0-MARGIN);
    float cell_size = board_size/8.0;
    float top_edge = (this->height()-board_size)/2.0;
    float left_edge = (this->width()-board_size)/2.0;
    float bottom_edge = top_edge+board_size;
    float right_edge = left_edge+board_size;


    QPainter p(this);

    p.setPen(QColor(0,0,0));


    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if((x+y) % 2) {
                p.fillRect(QRectF(left_edge + x*cell_size, top_edge + y*cell_size, cell_size, cell_size), QColor(100,100,100));
            } else continue;
        }
    }

    for(int i = 0; i <= 8; i++) {
        p.drawLine(QPointF(left_edge, top_edge + i*cell_size),QPointF(right_edge, top_edge + i*cell_size));//horizontal lines
        p.drawLine(QPointF(left_edge + i*cell_size,top_edge),QPointF(left_edge + i*cell_size, bottom_edge));//vertical lines
    }


    //TODO: add checkerboard

    const float sprite_margin = 0.1;
    float sprite_padding = sprite_margin*cell_size;
    const float sprite_size = cell_size - 2*sprite_padding;

    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(!board.at(QPoint(x,y))) continue;
            draw_sprite(&p, QRectF(left_edge + x*cell_size + sprite_padding, top_edge + y*cell_size + sprite_padding, sprite_size, sprite_size), *board.at(QPoint(x,y)));
        }
    }

    if(held_piece){
        draw_sprite(&p, QRectF(mouse_pos.x() - sprite_size/2.0, mouse_pos.y() - sprite_size/2.0, sprite_size, sprite_size), *held_piece);
    }
}
void BoardUI::mouseMoveEvent(QMouseEvent * e){
    if(!held_piece) return;
    e->accept();
    mouse_pos = e->pos();
    this->update();
}
void BoardUI::mousePressEvent(QMouseEvent * e){
    if(held_piece) return;
    std::optional<QPoint> index = mouseToBoard(e->pos());
    if(!index) return;//if mouse out of bounds
    e->accept();
    held_piece_origin = *index;
    held_piece = board.at(*index);
    board.at(*index) = std::nullopt;
}
void BoardUI::mouseReleaseEvent(QMouseEvent * e){
    if(!held_piece) return;
    std::optional<QPoint> dest = mouseToBoard(e->pos());
    e->accept();
    ChessBoard proposed_board = this->board;
    proposed_board.at(held_piece_origin) = held_piece;
    if(!dest || !proposed_board.legal_move(Move{held_piece.value(), held_piece_origin, dest.value()})) {//if mouse out of bounds or invalid placement
        if(held_piece.value().rank == Rank::King && held_piece_origin == QPoint(4,7)){
            //TODO: proper checks
            if(dest.value() == QPoint(6,7)) {
                board.at(*dest) = held_piece;
                board.at(5,7) = Piece(Team::Alpha, Rank::Rook);
                board.at(7,7) = std::nullopt;

                emit evaluation_updated(board.heuristic(team_inverse(held_piece.value().team)));
                held_piece = std::nullopt;
                emit move_made(Team::Alpha);
                this->doAIMove(Team::Beta);
            }
            if(dest.value() == QPoint(2, 7)) {
                board.at(*dest) = held_piece;
                board.at(3,7) = Piece(Team::Alpha, Rank::Rook);
                board.at(0,7) = std::nullopt;

                emit evaluation_updated(board.heuristic(team_inverse(held_piece.value().team)));
                held_piece = std::nullopt;
                emit move_made(Team::Alpha);
                this->doAIMove(Team::Beta);
            }
        } else {
            board.at(held_piece_origin) = held_piece;
            held_piece = std::nullopt;
        }
    }
    else {
        board.at(*dest) = held_piece;
        emit evaluation_updated(board.heuristic(team_inverse(held_piece.value().team)));
        held_piece = std::nullopt;
        emit move_made(Team::Alpha);
        this->doAIMove(Team::Beta);
    }
    this->update();
}
std::optional<QPoint> BoardUI::mouseToBoard(QPoint p) {
    float board_size = std::min(this->width(),this->height())*(1.0-MARGIN);
    float cell_size = board_size/8.0;
    float top_edge = (this->height()-board_size)/2.0;
    float left_edge = (this->width()-board_size)/2.0;
    if(p.x() < left_edge || p.y() < top_edge) return std::nullopt;
    p.setX((p.x()-left_edge)/cell_size);
    p.setY((p.y()-top_edge)/cell_size);
    if(!ChessBoard::in_bounds(p)) return std::nullopt;
    return p;
}

void BoardUI::draw_sprite(QPainter *painter, QRectF location, Piece piece) {
    int draw_y = 0;
    QString element;
    if(piece.team == Team::Beta) {
        draw_y = sprite_size.y();
        element += QString("black");
    }
    else element += QString("white");
    int draw_x = 0;
    switch(piece.rank) {
    case Rank::Pawn:
        draw_x = 5;
        element += QString("pawn");
        break;
    case Rank::Rook:
        draw_x = 4;
        element += QString("rook");
        break;
    case Rank::Knight:
        draw_x = 3;
        element += QString("knight");
        break;
    case Rank::Bishop:
        draw_x = 2;
        element += QString("bishop");
        break;
    case Rank::Queen:
        draw_x = 1;
        element += QString("queen");
        break;
    case Rank::King:
        draw_x = 0;
        element += QString("king");
        break;
    }
    draw_x *= sprite_size.x();
    sprite_sheet->setViewBox(QRectF(draw_x, draw_y, sprite_size.x(), sprite_size.y()));
    sprite_sheet->render(painter, element, location);
}

void BoardUI::doAIMove(Team t) {
    this->ai_threads = new AIMultiThread(board, t);
    connect(ai_threads, &AIMultiThread::finished, this, &BoardUI::on_think_finished);
    connect(ai_threads, &AIMultiThread::think_updated, this, &BoardUI::on_think_updated);
    ai_threads->start();
}

/*
    //QFuture future = QtConcurrent::run([]{
        //
    //});
    std::vector<ChessBoard> children = board.gen_filtered_children_boards(t);
    if(children.size() == 0) exit(10);


    int best_score;
    ChessBoard best_board = children[0];
    if(t == Team::Alpha) best_score = INT_MIN;
    else best_score = INT_MAX;


    for(int i = 0; i < children.size(); i++) {
        int eval = children[i].alpha_beta(team_inverse(t));
        if(t == Team::Alpha && eval > best_score) {
            best_score = eval;
            best_board = children[i];
        }
        if(t == Team::Beta && eval < best_score) {
            best_score = eval;
            best_board = children[i];
        }
    }
    this->board = best_board;
    this->update();
    emit move_made(t);
*/

void BoardUI::on_move_made(Team t) {
    this->doAIMove(team_inverse(t));
}

void BoardUI::on_think_finished() {
    std::optional<ChessBoard> retval = ai_threads->get_best();
    if(retval.has_value()) {
        this->board = retval.value();
        delete ai_threads;
        emit move_made(Team::Beta);
        emit evaluation_updated(board.heuristic(Team::Alpha));
        emit think_finished();
        this->update();
    }
    else {
        exit(10);
    }
}

void BoardUI::on_think_updated(float percent) {
    emit think_updated(percent);
}

void BoardUI::reset_board() {
    this->board = ChessBoard();
    emit evaluation_updated(board.heuristic(Team::Alpha));
    this->update();
}
