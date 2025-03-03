#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QPoint>
#include <vector>
#include <optional>
#include <climits>
#include <algorithm>

#define BRANCHING_FACTOR 35
//#define MATERIAL_ONLY

enum Team {
    Alpha,
    Beta
};
enum Rank {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct Piece {
    Piece();
    Piece(Team t, Rank r);
    Team team;
    Rank rank;
    constexpr bool operator==(Piece p) {return p.team == this->team && p.rank == this->rank;}
    int value();
};

struct Move {
    Piece piece;
    QPoint origin;
    QPoint destination;
    constexpr bool operator==(Move m) {
        return m.destination == this->destination && m.origin == this->origin && m.piece == this->piece;
    }
};
enum class Check {
    Beta_Mate = -2,
    Beta_Check = -1,
    Free = 0,
    Alpha_Check = 1,
    Alpha_Mate = 2
};


/*
      0 1 2 3 4 5 6 7   A
    7
    6
    5
    4
    3
    2
    1
    0
                        B
*/

class ChessBoard
{
public:
    ChessBoard();
    Move get_ai_move(Team team, float intelligence);//describes move so that it can be animated
    bool do_move(Move m);//occurs at end of animation
    std::optional<Piece>& at(QPoint index);
    std::optional<Piece>& at(int x, int y);
    static bool in_bounds(QPoint p);
    int alpha_beta(Team team);
    int alpha_beta(int depth, Team team, int alpha, int beta);
    bool valid_move(Move m);
    bool legal_move(Move m);
    bool get_check(Team t);
    bool operator==(ChessBoard q1);
    std::vector<ChessBoard> gen_filtered_children_boards(Team t);
    std::vector<Move> gen_filtered_children_moves(Team t);

    int heuristic(Team t);

private:
    static constexpr const int diagonals[4][2] = {{1,1},{1,-1},{-1,-1},{-1,1}};
    static constexpr const int cardinals[4][2] = {{0,1},{1,0},{0,-1},{-1,0}};
    std::optional<Piece> board[8][8];
    std::optional<int> stored_heuristic;
    std::vector<ChessBoard> children;

    static const int MATERIAL_COEFFICIENT = 1000;
    static const int ATTACK_DEFEND_COEFFICIENT = 25;
    static const int CHECK_COEFFICIENT = 2000;
    static const int DEPTH_BONUS = 10;
    static const int EARLY_MOVE_BREADTH = 4;
    static const int LATE_MOVE_THRESHOLD = 4;
    static const int LATE_MOVE_BREADTH = 2;
    static const int DEPTH_CUTOFF = 10;

    static const int CASTLE_BETA_LEFT = 1;
    static const int CASTLE_BETA_RIGHT = 2;
    static const int CASTLE_ALPHA_LEFT = 4;
    static const int CASTLE_ALPHA_RIGHT = 8;

    char castle_status;

    std::vector<ChessBoard> gen_all_children_boards(Team t);
    std::vector<Move> gen_all_children_moves(Team t);
    std::vector<ChessBoard> gen_diagonal_boards(QPoint origin, bool extending);
    std::vector<Move> gen_diagonal_moves(QPoint origin, bool extending);
    std::vector<ChessBoard> gen_cardinal_boards(QPoint origin, bool extending);
    std::vector<Move> gen_cardinal_moves(QPoint origin, bool extending);
    std::vector<ChessBoard> gen_pawn_boards(QPoint origin);
    std::vector<Move> gen_pawn_moves(QPoint origin);
    std::vector<ChessBoard> gen_knight_boards(QPoint origin);
    std::vector<Move> gen_knight_moves(QPoint origin);
    std::vector<ChessBoard> gen_best_boards(Team t, int limit);

    bool square_attacked(std::vector<Move> &moves, QPoint square);


    bool dead_king(Team t);
    std::pair<bool, int> get_cod(Team t);
    int defense_quality(Piece defender, Piece defendee);
    int offense_quality(Piece offender, Piece offendee);
    int get_defense(Team t);
    bool alpha_can_short_castle;
    bool alpha_can_long_castle;
    bool beta_can_long_castle;
    bool beta_can_short_castle;
};

Team team_inverse(Team t);

#endif // CHESSBOARD_H
