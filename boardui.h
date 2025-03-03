#ifndef BOARDUI_H
#define BOARDUI_H

#include "chessboard.h"
#include "aimultithread.h"

#include <QFrame>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <optional>
#include <QtSvg/QtSvg>
#include <QtConcurrent/QtConcurrent>



class BoardUI : public QFrame
{
    Q_OBJECT
public:
    BoardUI(QWidget * parent);
    ~BoardUI();
    void paintEvent(QPaintEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void doAIMove(Team t);
    void reset_board();
    const float MARGIN = 0.1;
private:
    AIMultiThread * ai_threads;
    Team currentTurn;
    std::optional<QPoint> mouseToBoard(QPoint p);
    void draw_sprite(QPainter * painter, QRectF location, Piece piece);
    QPoint mouse_pos;
    QPoint held_piece_origin;
    std::optional<Piece> held_piece;
    ChessBoard board;
    QSvgRenderer * sprite_sheet;
    const QPoint sprite_size = QPoint(45,45);
    QThreadPool think_pool;
signals:
    void move_made(Team t);
    void think_updated(float percent);
    void think_finished();
    void evaluation_updated(int value);

public slots:
    void on_move_made(Team t);
    void on_think_updated(float percent);
    void on_think_finished();
};

#endif // BOARDUI_H
