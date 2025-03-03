#include "chessboard.h"

Piece::Piece(Team t, Rank r) {
    this->rank = r;
    this->team = t;
}
Piece::Piece() {
    this->rank = Rank::Pawn;
    this->team = Team::Alpha;
}

ChessBoard::ChessBoard()
{
    for(int x = 0; x < 8; x++) {
        board[6][x] = Piece{Alpha, Pawn};
        board[1][x] = Piece{Beta, Pawn};
        for(int y = 2; y < 6; y++) {
            board[y][x] = std::nullopt;
        }
        board[7][0] = Piece{Alpha, Rook};
        board[7][7] = board[7][0];

        board[0][0] = Piece{Beta, Rook};
        board[0][7] = board[0][0];

        board[7][1] = Piece{Alpha, Knight};
        board[7][6] = board[7][1];

        board[0][1] = Piece{Beta, Knight};
        board[0][6] = board[0][1];

        board[7][2] = Piece{Alpha, Bishop};
        board[7][5] = board[7][2];

        board[0][2] = Piece{Beta, Bishop};
        board[0][5] = board[0][2];

        board[7][3] = Piece{Alpha, Queen};
        board[7][4] = Piece{Alpha, King};

        board[0][3] = Piece{Beta, Queen};
        board[0][4] = Piece{Beta, King};
    }

    castle_status = CASTLE_ALPHA_LEFT | CASTLE_ALPHA_RIGHT | CASTLE_BETA_LEFT | CASTLE_BETA_RIGHT;
}

bool ChessBoard::do_move(Move m) {
    if(!in_bounds(m.destination) || !in_bounds(m.origin)) return false;
    if(!this->at(m.origin)) return false;//if moving nothing
    if(this->at(m.destination).has_value()) {
        if(this->at(m.destination).value().team == this->at(m.origin).value().team) return false;
    }

    if(m.piece.rank == Rank::King) {
        if(m.piece.team == Team::Alpha) {
            this->castle_status = this->castle_status & ~(CASTLE_ALPHA_LEFT | CASTLE_ALPHA_RIGHT);
        }
        else {
            this->castle_status = this->castle_status & ~(CASTLE_BETA_LEFT | CASTLE_BETA_RIGHT);
        }
    }
    if(m.piece.rank == Rank::Rook) {
        if(m.origin == QPoint(0,0)) this->castle_status = this->castle_status & ~CASTLE_BETA_LEFT;
        if(m.origin == QPoint(7,0)) this->castle_status = this->castle_status & ~CASTLE_BETA_RIGHT;
        if(m.origin == QPoint(0,7)) this->castle_status = this->castle_status & ~CASTLE_ALPHA_LEFT;
        if(m.origin == QPoint(7,7)) this->castle_status = this->castle_status & ~CASTLE_ALPHA_RIGHT;
    }

    this->at(m.destination) = this->at(m.origin);
    this->at(m.origin) = std::nullopt;
    return true;
}

bool ChessBoard::in_bounds(QPoint p) {
    return p.x() >= 0 && p.x() <= 7 && p.y() >= 0 && p.y() <= 7;
}

std::optional<Piece>& ChessBoard::at(QPoint index) {
    return board[index.y()][index.x()];
}
std::optional<Piece>& ChessBoard::at(int x, int y) {
    return board[y][x];
}

std::vector<ChessBoard> ChessBoard::gen_filtered_children_boards(Team t) {
    std::vector<Move> all_children = gen_all_children_moves(t);
    std::vector<ChessBoard> retval;
    retval.reserve(BRANCHING_FACTOR*2);

    for(int i = 0; i < all_children.size(); i++) {
        if(!valid_move(all_children[i])) continue;
        ChessBoard temp_board = *this;
        temp_board.do_move(all_children[i]);
        bool check_state = temp_board.get_check(t);
        if(!check_state) {
            retval.push_back(temp_board);
        }
    }
    return retval;
}

std::vector<Move> ChessBoard::gen_filtered_children_moves(Team t) {
    std::vector<Move> all_children = gen_all_children_moves(t);
    std::vector<Move> retval;

    for(int i = 0; i < all_children.size(); i++) {
        if(!valid_move(all_children[i])) continue;
        ChessBoard temp_board = *this;
        temp_board.do_move(all_children[i]);
        bool check_state = temp_board.get_check(t);
        if(!check_state) {
            retval.push_back(all_children[i]);
        }
    }
    return retval;
}


std::vector<ChessBoard> ChessBoard::gen_all_children_boards(Team t) {
    std::vector<ChessBoard> retval;
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(!board[y][x]) continue;
            if(board[y][x].value().team != t) continue;
            std::vector<ChessBoard> temp_moves;
            QPoint location(x, y);
            switch(board[y][x].value().rank) {
            case Rank::Pawn:
                temp_moves = gen_pawn_boards(location);
                break;
            case Rank::Knight:
                temp_moves = gen_knight_boards(location);
                break;
            case Rank::Bishop:
                temp_moves = gen_diagonal_boards(location, true);
                break;
            case Rank::Rook:
                temp_moves = gen_cardinal_boards(location, true);
                break;
            case Rank::King:
                temp_moves = gen_diagonal_boards(location, false);
                retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
                temp_moves = gen_cardinal_boards(location, false);
                break;
            case Rank::Queen:
                temp_moves = gen_diagonal_boards(location, true);
                retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
                temp_moves = gen_cardinal_boards(location, true);
                break;
            }
            retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
        }
    }

    if(t == Team::Alpha) {
        if(castle_status & CASTLE_ALPHA_LEFT) {
            if(!(board[7][1].has_value() || board[7][2].has_value() || board[7][3].has_value())) {
                //retval.push_back();
            }
        }
        if(castle_status & CASTLE_ALPHA_RIGHT) {
            if(!(board[7][5].has_value() || board[7][6].has_value())) {
                //
            }
        }
    } else {
        if(castle_status & CASTLE_BETA_LEFT) {
            if(!(board[0][1].has_value() || board[0][2].has_value() || board[0][3].has_value())) {
                //
            }
        }
        if(castle_status & CASTLE_BETA_RIGHT) {
            if(!(board[0][5].has_value() || board[0][6].has_value())) {
                //
            }
        }
    }

    return retval;
}

std::vector<Move> ChessBoard::gen_all_children_moves(Team t) {
    std::vector<Move> retval;
    retval.reserve(BRANCHING_FACTOR*2);
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(!board[y][x]) continue;
            if(board[y][x].value().team != t) continue;
            std::vector<Move> temp_moves;
            //TODO: check for mate condition
            QPoint location(x, y);
            switch(board[y][x].value().rank) {
            case Rank::Pawn:
                temp_moves = gen_pawn_moves(location);
                break;
            case Rank::Knight:
                temp_moves = gen_knight_moves(location);
                break;
            case Rank::Bishop:
                temp_moves = gen_diagonal_moves(location, true);
                break;
            case Rank::Rook:
                temp_moves = gen_cardinal_moves(location, true);
                break;
            case Rank::King:
                temp_moves = gen_diagonal_moves(location, false);
                retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
                temp_moves = gen_cardinal_moves(location, false);
                break;
            case Rank::Queen:
                temp_moves = gen_diagonal_moves(location, true);
                retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
                temp_moves = gen_cardinal_moves(location, true);
                break;
            }
            retval.insert(retval.end(), temp_moves.begin(), temp_moves.end());
        }
    }

    if(t == Team::Alpha) {
        if(castle_status & CASTLE_ALPHA_LEFT) {
            if(!(board[7][1].has_value() || board[7][2].has_value() || board[7][3].has_value())) {
                //retval.push_back();
            }
        }
        if(castle_status & CASTLE_ALPHA_RIGHT) {
            if(!(board[7][5].has_value() || board[7][6].has_value())) {
                //
            }
        }
    } else {
        if(castle_status & CASTLE_BETA_LEFT) {
            if(!(board[0][1].has_value() || board[0][2].has_value() || board[0][3].has_value())) {
                //
            }
        }
        if(castle_status & CASTLE_BETA_RIGHT) {
            if(!(board[0][5].has_value() || board[0][6].has_value())) {
                //
            }
        }
    }

    return retval;
}

bool ChessBoard::get_check(Team t) {
    //TODO: check outward from king
    QPoint king_location;

    bool king_found = false;
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(board[y][x].has_value()) {
                if(board[y][x].value() == Piece{t, Rank::King}) {
                    king_location = QPoint(x, y);
                    king_found = true;
                }
            }
        }
    }

    if(!king_found) {
        return true;
    }

    Team opponent = team_inverse(t);

    std::vector<Move> diag = this->gen_diagonal_moves(king_location, true);
    for(Move m : diag) {
        if(!this->at(m.destination).has_value()) continue;
        Piece p = this->at(m.destination).value();
        if(p == Piece{opponent, Rank::Queen}) return true;
        if(p == Piece{opponent, Rank::Bishop}) return true;
    }

    std::vector<Move> card = this->gen_cardinal_moves(king_location, true);
    for(Move m : card) {
        if(!this->at(m.destination).has_value()) continue;
        Piece p = this->at(m.destination).value();
        if(p == Piece{opponent, Rank::Queen}) return true;
        if(p == Piece{opponent, Rank::Rook}) return true;
    }

    std::vector<Move> knights = this->gen_knight_moves(king_location);
    for(Move m : knights) {
        if(this->at(m.destination).has_value()) {
            if(this->at(m.destination).value() == Piece{opponent, Rank::Knight}) return true;
        }
    }

    for(int i = 0; i < 4; i++) {
        QPoint card_neighbor = king_location + QPoint(cardinals[i][0], cardinals[i][1]);
        if(!in_bounds(card_neighbor)) continue;
        if(this->at(card_neighbor).has_value()) {
            if(this->at(card_neighbor).value() == Piece{opponent, Rank::King}) return true;
        }
        QPoint diag_neighbor = king_location + QPoint(diagonals[i][0], diagonals[i][1]);
        if(!in_bounds(diag_neighbor)) continue;
        if(this->at(diag_neighbor).has_value()) {
            if(this->at(diag_neighbor).value() == Piece{opponent, Rank::King}) return true;
        }
    }

    int pawn_direction = 0;

    if(t == Team::Alpha) {
        pawn_direction = -1;
    }
    else {
        pawn_direction = 1;
    }

    QPoint pawn_left = king_location + QPoint(1,pawn_direction);
    QPoint pawn_right = king_location + QPoint(-1,pawn_direction);
    if(in_bounds(pawn_left) && this->at(pawn_left).has_value()) {
        if(this->at(pawn_left).value() == Piece{opponent, Rank::Pawn}) return true;
    }
    if(in_bounds(pawn_right) && this->at(pawn_right).has_value()) {
        if(this->at(pawn_right).value() == Piece{opponent, Rank::Pawn}) return true;
    }

    return false;
}

bool ChessBoard::dead_king(Team t) {
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(this->at(x, y).has_value()) {
                if(this->at(x, y).value() == Piece{t, Rank::King}) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ChessBoard::valid_move(Move m) {
    if(!in_bounds(m.destination) || !in_bounds(m.origin)) return false;
    if(!this->at(m.origin)) return false;//if moving nothing
    if(this->at(m.destination).has_value()) {
        if(this->at(m.destination).value().team == this->at(m.origin).value().team) return false;
    }
    return true;
}

bool ChessBoard::legal_move(Move m) {
    std::vector<Move> valid_moves = this->gen_filtered_children_moves(m.piece.team);
    for(int i = 0; i < valid_moves.size(); i++) {
        if(valid_moves[i] == m) return true;
    }
    return false;
}


std::vector<ChessBoard> ChessBoard::gen_pawn_boards(QPoint origin) {
    ChessBoard temp_board;
    std::vector<ChessBoard> temp_boards;
    int pawn_direction;
    QPoint dest;
    if(board[origin.y()][origin.x()].value().team == Team::Alpha) {
        //
        pawn_direction = -1;
        dest = origin + QPoint(0, -2);
        if(origin.y() == 6 && in_bounds(dest) && !this->at(origin + QPoint(0,-1)).has_value() && !this->at(dest).has_value()) {
            temp_board = *this;
            if (temp_board.do_move(Move{board[origin.y()][origin.x()].value(), origin, dest}))
            {temp_boards.push_back(temp_board);}
        }
        //TODO: add en passant here
    }
    else {
        pawn_direction = 1;
        dest = origin + QPoint(0, 2);
        if(origin.y() == 1 && in_bounds(dest) && !this->at(origin + QPoint(0,1)).has_value() && !this->at(dest).has_value()) {
            temp_board = *this;
            if (temp_board.do_move(Move{board[origin.y()][origin.x()].value(), origin, dest}))
            {temp_boards.push_back(temp_board);}
        }
    }
    dest = origin + QPoint(0,pawn_direction);
    QPoint left_dest = dest+QPoint(-1,0);
    QPoint right_dest = dest+QPoint(1,0);
    if(in_bounds(left_dest) && this->at(left_dest).has_value()) {
        if(this->at(left_dest).value().team != this->at(origin).value().team) {
            temp_board = *this;
            if (temp_board.do_move(Move{board[origin.y()][origin.x()].value(), origin, left_dest}))
            {temp_boards.push_back(temp_board);}
        }
    }
    if(in_bounds(right_dest) && this->at(right_dest).has_value()) {
        if(this->at(right_dest).value().team != this->at(origin).value().team) {
            temp_board = *this;
            if (temp_board.do_move(Move{board[origin.y()][origin.x()].value(), origin, right_dest}))
            {temp_boards.push_back(temp_board);}
        }
    }



    if(in_bounds(dest) && !this->at(dest).has_value()) {
        temp_board = *this;
        temp_board.do_move(Move{board[origin.y()][origin.x()].value(), origin, dest});
        temp_boards.push_back(temp_board);
    }
    return temp_boards;
}

std::vector<Move> ChessBoard::gen_pawn_moves(QPoint origin) {
    Move temp_move;
    std::vector<Move> retval;
    retval.reserve(3);
    int pawn_direction;
    QPoint dest;

    //double jump
    if(board[origin.y()][origin.x()].value().team == Team::Alpha) {
        //
        pawn_direction = -1;
        dest = origin + QPoint(0, -2);
        if(origin.y() == 6 && in_bounds(dest) && !this->at(origin + QPoint(0,-1)).has_value() && !this->at(dest).has_value()) {
            temp_move = Move{board[origin.y()][origin.x()].value(), origin, dest};
            if (this->in_bounds(temp_move.destination))
            {retval.push_back(temp_move);}
        }
        //TODO: add en passant here
    }
    else {
        pawn_direction = 1;
        dest = origin + QPoint(0, 2);
        if(origin.y() == 1 && in_bounds(dest) && !this->at(origin + QPoint(0,1)).has_value() && !this->at(dest).has_value()) {
            temp_move = Move{board[origin.y()][origin.x()].value(), origin, dest};
            if (this->in_bounds(temp_move.destination))
            {retval.push_back(temp_move);}
        }
    }


    dest = origin + QPoint(0,pawn_direction);
    QPoint left_dest = dest+QPoint(-1,0);
    QPoint right_dest = dest+QPoint(1,0);



    //single jump
    if(in_bounds(dest) && !this->at(dest).has_value()) {
        temp_move = Move{board[origin.y()][origin.x()].value(), origin, dest};
        if(this->in_bounds(temp_move.destination))
        {retval.push_back(temp_move);}
    }

    //L and R flanks
    if(in_bounds(left_dest) && this->at(left_dest).has_value()) {
        if(this->at(left_dest).value().team != this->at(origin).value().team) {
            temp_move = Move{board[origin.y()][origin.x()].value(), origin, left_dest};
            if (this->in_bounds(temp_move.destination)) retval.push_back(temp_move);
        }
    }
    if(in_bounds(right_dest) && this->at(right_dest).has_value()) {
        if(this->at(right_dest).value().team != this->at(origin).value().team) {
            temp_move = Move{board[origin.y()][origin.x()].value(), origin, right_dest};
            if (this->in_bounds(temp_move.destination)) retval.push_back(temp_move);
        }
    }
    return retval;
}

std::vector<ChessBoard> ChessBoard::gen_knight_boards(QPoint origin) {
    static constexpr QPoint knight_moves[8] = {
        QPoint(1,2),
        QPoint(2,1),
        QPoint(2,-1),
        QPoint(1, -2),
        QPoint(-1,-2),
        QPoint(-2,-1),
        QPoint(-2,1),
        QPoint(-1,2)
    };
    std::vector<ChessBoard> retval;
    retval.reserve(8);
    for(int i = 0; i < 8; i++) {
        ChessBoard temp_board = *this;
        QPoint dest = origin + knight_moves[i];
        if(temp_board.do_move(Move{this->at(origin).value(), origin, dest})){
            retval.push_back(temp_board);
        }
    }
    return retval;
}

std::vector<Move> ChessBoard::gen_knight_moves(QPoint origin) {
    static constexpr QPoint knight_moves[8] = {
        QPoint(1,2),
        QPoint(2,1),
        QPoint(2,-1),
        QPoint(1, -2),
        QPoint(-1,-2),
        QPoint(-2,-1),
        QPoint(-2,1),
        QPoint(-1,2)
    };
    std::vector<Move> retval;
    retval.reserve(8);
    Move temp_move;
    for(int i = 0; i < 8; i++) {
        QPoint dest = origin + knight_moves[i];
        temp_move = Move{this->at(origin).value(), origin, dest};
        if(this->in_bounds(temp_move.destination))
        {
            retval.push_back(temp_move);
        }
    }
    return retval;
}

std::vector<ChessBoard> ChessBoard::gen_cardinal_boards(QPoint origin, bool extending) {
    std::vector<ChessBoard> retval;
    retval.reserve(16);
    for(int j = 0; j < 4; j++) {
        for(int i = 1; i <= 8; i++) {
            bool move_made = false;
            QPoint dest = origin + QPoint(cardinals[j][0]*i, cardinals[j][1]*i);
            if(!in_bounds(dest)) continue;
            ChessBoard temp_board = *this;
            if(!temp_board.do_move(Move{*this->at(origin), origin, dest})) break;
            move_made = true;
            retval.push_back(temp_board);
            if(!extending) break;
            if(!move_made) break;
            if(this->at(dest).has_value()) break;
        }
    }
    return retval;
}

std::vector<Move> ChessBoard::gen_cardinal_moves(QPoint origin, bool extending) {
    std::vector<Move> retval;
    retval.reserve(16);
    Move temp_move;
    for(int j = 0; j < 4; j++) {
        for(int i = 1; i <= 8; i++) {
            bool move_made = false;
            QPoint dest = origin + QPoint(cardinals[j][0]*i, cardinals[j][1]*i);
            if(!in_bounds(dest)) continue;
            temp_move = Move{*this->at(origin), origin, dest};
            if(!this->valid_move(temp_move)) break;
            move_made = true;
            retval.push_back(temp_move);
            if(!extending) break;
            if(!move_made) break;
            if(this->at(dest).has_value()) break;
        }
    }
    return retval;
}

std::vector<ChessBoard> ChessBoard::gen_diagonal_boards(QPoint origin, bool extending) {
    std::vector<ChessBoard> retval;
    retval.reserve(16);
    for(int j = 0; j < 4; j++) {
        for(int i = 1; i <= 8; i++) {
            bool move_made = false;
            QPoint dest = origin + QPoint(diagonals[j][0]*i, diagonals[j][1]*i);
            if(!in_bounds(dest)) continue;
            ChessBoard temp_board = *this;
            if(!temp_board.do_move(Move{*this->at(origin), origin, dest})) break;
            move_made = true;
            retval.push_back(temp_board);
            if(!extending) break;
            if(!move_made) break;
            if(this->at(dest).has_value()) break;
        }
    }
    return retval;
}

std::vector<Move> ChessBoard::gen_diagonal_moves(QPoint origin, bool extending) {
    std::vector<Move> retval;
    retval.reserve(16);
    Move temp_move;
    for(int j = 0; j < 4; j++) {
        for(int i = 1; i <= 8; i++) {
            bool move_made = false;
            QPoint dest = origin + QPoint(diagonals[j][0]*i, diagonals[j][1]*i);
            if(!in_bounds(dest)) continue;
            temp_move = Move{*this->at(origin), origin, dest};
            if(!this->valid_move(temp_move)) break;
            move_made = true;
            retval.push_back(temp_move);
            if(!extending) break;
            if(!move_made) break;
            if(this->at(dest).has_value()) break;
        }
    }
    return retval;
}


bool ChessBoard::operator==(ChessBoard q1) {
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(!q1.board[y][x] && this->board[y][x] || q1.board[y][x] && !this->board[y][x]) return false;
            else {
                Piece p1 = *q1.board[y][x];
                Piece p2 = *this->board[y][x];
                if(p1.rank != p2.rank || p1.team != p2.team) return false;
            }
        }
    }
    return true;
}

Team team_inverse(Team t) {
    if(t == Team::Alpha) return Team::Beta;
    else return Team::Alpha;
}

int ChessBoard::heuristic(Team team) {
    int piece_sum = 0;
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(!board[y][x].has_value()) continue;
            piece_sum += board[y][x].value().value();
        }
    }

    /*
    std::pair<bool, int> cod_offense = get_cod(team);//attacks that t can make
    std::pair<bool, int> cod_defense = get_cod(team_inverse(team));//attacks t can suffer

    int check_val = 0;
    if(cod_offense.first) {
        check_val = 1;
        if(team == Team::Beta) check_val *= -1;
    }
    if(cod_defense.first) {
        check_val = -1;
        if(team == Team::Beta) check_val *= -1;
    }
    check_val *= CHECK_COEFFICIENT;

    /*
    std::vector<ChessBoard> children = this->gen_filtered_children_boards(team);
    if(children.size() == 0) {
        //checkmate condition
        if(team == Team::Alpha) return -20000;
        else return 20000;
    }


    int offense_val = cod_offense.second * ATTACK_DEFEND_COEFFICIENT;// + CAT_defense.second;

    int defense_val = cod_defense.second * ATTACK_DEFEND_COEFFICIENT;
    */

#ifndef MATERIAL_ONLY
    std::vector<Move> alpha_moves = gen_all_children_moves(Team::Alpha);
    std::vector<Move> beta_moves = gen_all_children_moves(Team::Beta);
    int AD_map[64] = {};
    int check_sum = 0;
    int AD_sum = 0;

    //if A's turn:
    //  attacks

    for(Move move : alpha_moves) {
        if(this->at(move.destination).has_value()) {
            int AD_index = move.destination.x() + move.destination.y() * 8;
            Piece dest_piece = this->at(move.destination).value();
            //AD_sum += dest_piece.value();
            if(dest_piece.team == Team::Beta) {//if attack
                AD_map[AD_index] += 1;
                //AD_sum += 1;
                if(dest_piece.rank == Rank::King) check_sum++;
            } else {
                AD_map[AD_index] -= 1;
            }
        }
    }
    for(Move move : beta_moves) {
        if(this->at(move.destination).has_value()) {
            int AD_index = move.destination.x() + move.destination.y() * 8;
            Piece dest_piece = this->at(move.destination).value();
            //AD_sum -= dest_piece.value();
            if(dest_piece.team == Team::Alpha) {//if attack
                AD_map[AD_index] += 1;
                //AD_sum -= 1;
                if(dest_piece.rank == Rank::King) check_sum--;
            } else {
                AD_map[AD_index] -= 1;
            }
        }
    }

    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            if(AD_map[x + y*8] > 0) {//if being attacked
                AD_sum -= this->at(x, y).value().value();
            }
            if(AD_map[x+y*8] < 0) {//if being defended
                AD_sum += this->at(x,y).value().value();
            }
        }
    }



    int retval = piece_sum * MATERIAL_COEFFICIENT + AD_sum * ATTACK_DEFEND_COEFFICIENT + check_sum * CHECK_COEFFICIENT; //offense_val + defense_val + check_val;
#else
    int retval = piece_sum * MATERIAL_COEFFICIENT;
#endif
    this->stored_heuristic = retval;

    return retval;
}

int ChessBoard::alpha_beta(Team team) {
    return alpha_beta(1, team, INT_MIN, INT_MAX);
}

int ChessBoard::alpha_beta(int depth, Team team, int alpha, int beta) {
    int depth_score = DEPTH_CUTOFF - depth + 1;

    depth_score *= DEPTH_BONUS;

    std::vector<ChessBoard> children = this->gen_filtered_children_boards(team);
    if(children.size() == 0) {
        //checkmate condition
        if(team == Team::Alpha) return -20000 * depth_score;
        else return 20000 * depth_score;
    }
    if(depth == DEPTH_CUTOFF) return this->heuristic(team) * depth_score;


    int filter_num = EARLY_MOVE_BREADTH;

    int max_evaluations = 0;
    if(depth > LATE_MOVE_THRESHOLD) filter_num = LATE_MOVE_BREADTH;
    if(depth > 2 && filter_num < children.size()) max_evaluations = filter_num;
    else max_evaluations = children.size();


    for(int i = 0; i < children.size(); i++) {
        children[i].heuristic(team);
    }

    int strongest = 0;
    if(team == Team::Alpha) {
        strongest = INT_MIN;
        std::sort(children.begin(), children.end(), [](ChessBoard a, ChessBoard b){
            int a_heuristic = a.stored_heuristic.value();
            int b_heuristic = b.stored_heuristic.value();
            return a_heuristic > b_heuristic;
        });
    }
    else {
        strongest = INT_MAX;
        std::sort(children.begin(), children.end(), [](ChessBoard a, ChessBoard b){
            int a_heuristic = a.stored_heuristic.value();
            int b_heuristic = b.stored_heuristic.value();
            return a_heuristic < b_heuristic;
        });
    }


    for(int i = 0; i < max_evaluations; i++) {
        int child_score = children[i].alpha_beta(depth+1, team_inverse(team), alpha, beta);
        if(team == Team::Alpha) {
            if(strongest < child_score) strongest = child_score;
            if(alpha < child_score) alpha = child_score;
        }
        if(team == Team::Beta){
            if(strongest > child_score) strongest = child_score;
            if(beta > child_score) beta = child_score;
        }
        if(alpha > beta) break;
    }
    return strongest;
}

std::vector<ChessBoard> ChessBoard::gen_best_boards(Team t, int limit) {
    this->gen_filtered_children_boards(t);

}

//for team t, check what attacks it can make and whether it is putting opponent in check
//attacks are measured similar to overall score. if negative, good for beta. if positive, good for alpha
std::pair<bool, int> ChessBoard::get_cod(Team t) {
    std::vector<Move> all_moves = gen_all_children_moves(t);
    std::vector<Move> attacks;
    //std::vector<Move> enemy_attacks = gen_all_children_moves(team_inverse(t));

    //defense doesn't count if defendee isn't under attack
    //offsense doesn't count if offender is under attack

    int defense_sum = 0;
    for(int i = 0; i < all_moves.size(); i++) {
        QPoint dest = all_moves[i].destination;
        if(this->at(dest).has_value()) {
            Piece defendee = this->at(dest).value();
            //if(!square_attacked(enemy_attacks, dest)) continue;
            Piece defender = this->at(all_moves[i].origin).value();
            if(defendee.team == t) {
                defense_sum += defense_quality(defender, defendee);
            }
        }

        if(!valid_move(all_moves[i])) continue;
        ChessBoard temp_board = *this;
        temp_board.do_move(all_moves[i]);
        attacks.push_back(all_moves[i]);
    }



    bool check = false;
    int offense_sum = attacks.size();
    if(t == Team::Beta) offense_sum *= -1;

    for(int i = 0; i < attacks.size(); i++) {
        QPoint dest = attacks[i].destination;
        if(this->at(dest).has_value()) {
            Piece offendee = this->at(dest).value();
            Piece offender = this->at(attacks[i].origin).value();
            //if(square_attacked(enemy_attacks, attacks[i].origin)) continue;
            if(offendee.team == team_inverse(t)) {
                offense_sum += offense_quality(offender, offendee);
                if(this->at(dest).value().rank == Rank::King) check = true;
            }
        }
    }

    return std::pair(check, offense_sum+defense_sum);
}

int Piece::value() {
    int piece_value = 1;
    switch(this->rank) {
    case Rank::Pawn:
        piece_value = 1;
        break;
    case Rank::Knight:
        piece_value = 3;
        break;
    case Rank::Bishop:
        piece_value = 4;
        break;
    case Rank::Rook:
        piece_value = 5;
        break;
    case Rank::Queen:
        piece_value = 9;
        break;
    case Rank::King:
        piece_value = 1;
        break;
    }
    if(this->team == Team::Beta) {
        piece_value *= -1;
    }
    return piece_value;
}

int ChessBoard::get_defense(Team t) {
    std::vector<Move> all_moves = gen_all_children_moves(t);
    int defense_sum = 0;
    for(int i = 0; i < all_moves.size(); i++) {
        if(this->at(all_moves[i].destination).has_value()) {
            Piece p = this->at(all_moves[i].destination).value();
            if(p.team == t) {
                defense_sum += defense_quality(p, this->at(all_moves[i].origin).value());
            }
        }
    }
    return defense_sum;
}

int ChessBoard::defense_quality(Piece defender, Piece defendee) {
    return 1;
    int retval = defendee.value() / defender.value();
    if(defender.team == Team::Alpha) retval++;
    else {
        retval *= -1;
        retval--;
    }
    return retval;
}

int ChessBoard::offense_quality(Piece offender, Piece offendee) {
    return 1;
    int retval = offendee.value() / offender.value();
    if(offender.team == Team::Alpha) {
        retval *= -1;
        retval++;
    }
    else {
        retval--;
    }
    return retval;
}

bool ChessBoard::square_attacked(std::vector<Move> &moves, QPoint square) {
    for(int i = 0; i < moves.size(); i++) {
        if(moves[i].destination == square) return true;
    }
    return false;
}
