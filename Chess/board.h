#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdio.h>

typedef uint64_t bitboard;

typedef enum {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP,
    WHITE_ROOK, WHITE_QUEEN,  WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP,
    BLACK_ROOK, BLACK_QUEEN,  BLACK_KING
} Piece;

extern bitboard pieces[12];
extern bitboard white_pieces;
extern bitboard black_pieces;
extern bitboard all_pieces;
extern uint8_t side_to_move;
extern uint8_t castling_rights;
extern uint8_t en_passant_square;
extern uint8_t fifty_move_counter;

void init_zobrist();
void set_starting_position();

#endif
