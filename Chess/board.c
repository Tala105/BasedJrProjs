#include "board.h"
#include <stdint.h>
#include <sys/types.h>

const char *piece_symbols[] = {
    "♙", "♘", "♗", "♖", "♕", "♔",
    "♟", "♞", "♝", "♜", "♛", "♚"
};

bitboard pieces[12];
uint8_t side_to_move;
uint8_t castling_rights;
uint8_t en_passant_square;
uint8_t fifty_move_counter;
bitboard hash;

bitboard white_pieces;
bitboard black_pieces;
bitboard all_pieces;

bitboard zobrist_table[64][12];
bitboard zobrist_side;
bitboard zobrist_castling[16];
bitboard zobrist_ep[8];

bitboard seed = 0x456F54656C6C6573ULL;

bitboard random_uint64(){
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return seed;
}

void init_zobrist(){
    for (int sq = 0; sq < 64; sq++)
        for (int piece = 0; piece < 12; piece++)
            zobrist_table[sq][piece] = random_uint64();
    zobrist_side = random_uint64();
    for (int i = 0; i < 16; i++)
        zobrist_castling[i] = random_uint64();
    for (int i = 0; i < 8; i++)
        zobrist_ep[i] = random_uint64();
}

bitboard compute_hash(){
    bitboard h = 0;
    for (int piece = 0; piece < 12; piece++){
        bitboard mask = pieces[piece];
        while (mask){
            int sq = __builtin_ctzll(mask);
            h ^= zobrist_table[sq][piece];
            mask &= mask - 1;
        }
    }
    if (side_to_move)
        h ^= zobrist_side;
    h ^= zobrist_castling[castling_rights];
    if (en_passant_square != 255)
        h ^= zobrist_ep[en_passant_square % 8];
    return h;
}

void set_starting_position(){
    pieces[WHITE_PAWN]   = 0x000000000000FF00;
    pieces[WHITE_KNIGHT] = 0x0000000000000042;
    pieces[WHITE_BISHOP] = 0x0000000000000024;
    pieces[WHITE_ROOK]   = 0x0000000000000081;
    pieces[WHITE_QUEEN]  = 0x0000000000000008;
    pieces[WHITE_KING]   = 0x0000000000000010;
    pieces[BLACK_PAWN]   = 0x00FF000000000000;
    pieces[BLACK_KNIGHT] = 0x4200000000000000;
    pieces[BLACK_BISHOP] = 0x2400000000000000;
    pieces[BLACK_ROOK]   = 0x8100000000000000;
    pieces[BLACK_QUEEN]  = 0x0800000000000000;
    pieces[BLACK_KING]   = 0x1000000000000000;
    side_to_move        = 0;
    castling_rights     = 0b00001111;
    en_passant_square   = 255;
    fifty_move_counter  = 0;
    white_pieces = pieces[WHITE_PAWN]   | pieces[WHITE_KNIGHT] | pieces[WHITE_BISHOP]
                 | pieces[WHITE_ROOK]   | pieces[WHITE_QUEEN]  | pieces[WHITE_KING];
    black_pieces = pieces[BLACK_PAWN]   | pieces[BLACK_KNIGHT] | pieces[BLACK_BISHOP]
                 | pieces[BLACK_ROOK]   | pieces[BLACK_QUEEN]  | pieces[BLACK_KING];
    all_pieces   = white_pieces | black_pieces;
    hash = compute_hash();
}

static inline int move_from(uint32_t m)     { return m & 0x3F; }
static inline int move_to(uint32_t m)       { return (m >> 6) & 0x3F; }
static inline int move_promo(uint32_t m)    { return (m >> 12) & 0x3; }
static inline int move_captured(uint32_t m) { return (m >> 14) & 0xF; }

static inline uint32_t make_move(int from, int to, int promo, int captured) {
    return from | (to << 6) | (promo << 12) | (captured << 14);
}

void create_move(uint32_t *moves){
	uint count = 0;
	bitboard pawns = pieces[side_to_move * 6];
	bitboard single_push = (side_to_move ? (pawns >> 8) : (pawns << 8)) & ~all_pieces;
	bitboard promo_rank = side_to_move ? 0x00000000000000FFULL : 0xFF00000000000000ULL;
	bitboard promotions = single_push & promo_rank;
	single_push &= ~promo_rank;
}

void print_board() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            bitboard sq_mask = 1ULL << sq;
            if (!(all_pieces & sq_mask)) {
                printf(". ");
                continue;
            }
            for (int piece = 0; piece < 12; piece++) {
                if (pieces[piece] & sq_mask) {
                    printf("%s ", piece_symbols[piece]);
                    break;
                }
            }
        }
        printf("\n");
    }
}
int main(){
    init_zobrist();
    set_starting_position();
	print_board();
    return 0;
}
