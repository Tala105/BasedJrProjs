#include <ctype.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct board board;
struct board {
  char state[67];
};

typedef struct link {
  char parent_state[67];
  char child_state[67];
  char comment[128];
} link;

int sign(int x) { return (x > 0) - (x < 0); }

board createBoard() {
  board b;
  strcpy(b.state,
         "snbqjbnspppppppp                                PPPPPPPPSNBQJBNS0@");
  return b;
}

int validateState(board b);

int kingMove(int yi, int xi, int yf, int xf, board b, board *n) {
  if (abs(yi - yf) > 1 || abs(xi - xf) > 2)
    return 0;
  if (yi == yf && abs(xf - xi) == 2 && tolower(b.state[8 * yi + xi]) == 'j') {
    int dir = sign(xf - xi);
    char ik = b.state[8 * yi + xi];
    char ir = b.state[8 * yi + (7 * ((dir + 1) / 2))];
    if (tolower(b.state[8 * yi + (7 * ((dir + 1) / 2))]) == 's') {
      board n2;
      n2.state[8 * yi + xi] = ' ';
      n2.state[8 * yi + xi + dir] = ik;
      if (!validateState(n2))
        return 0;
      n2.state[8 * yi + xi + dir] = ' ';
      n2.state[8 * yi + xi + 2 * dir] = ik;
      if (!validateState(n2))
        return 0;
      n->state[8 * yf + xf] = ik + 1;
      n->state[8 * yi + (4 + dir)] = ir - 1;
      n->state[8 * yi + (7 * ((dir + 1) / 2))] = ' ';
      return 1;
    }
    return 0;
  }
  if (abs(xi - xf) > 1)
    return 0;
  if (tolower(b.state[8 * yi + xi]) == 'j')
    n->state[8 * yf + xf] = n->state[8 * yi + xi] + 1;
  n->state[8 * yf + xf] = n->state[8 * yi + xi];
  return 1;
}

int promote(int yi, int xi, int yf, int xf, char np, board b, board *n) {
  np = tolower(np);
  if (np == 'n' || np == 'b' || np == 'r' || np == 'q') {
    if (isupper(b.state[8 * yi + xi]))
      n->state[8 * yi + xi] = toupper(np);
    else
      n->state[8 * yi + xi] = np;
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int wpawnMove(int yi, int xi, int yf, int xf, char np, board b, board *n) {
	printf("WPM\n");
  int piece = b.state[8 * yf + xf];
  if ((yf - yi == 1 && abs(xi - xf) == 1 && piece != 32) ||
      (yf - yi == 1 && xf == xi && piece == 32)) {
    n->state[65] = '@';
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yf - yi == 1 && abs(xi - xf) == 1 && b.state[65] == '@' + 8 * yf + xf) {
    n->state[65] = '@';
    n->state[8 * (yf - 1) + xf] = ' ';
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    else
      return 1;
  }

  if (yi == 1 && yf - yi == 2 && xf == xi && piece == 32) {
    n->state[65] = '@' + 8 * (yf - 1) + xf;
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int bpawnMove(int yi, int xi, int yf, int xf, char np, board b, board *n) {
  int piece = b.state[8 * yf + xf];
  if ((yf - yi == -1 && abs(xi - xf) == 1 && piece != 32) ||
      (yf - yi == -1 && xf == xi && piece == 32)) {
    n->state[65] = '@';
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yf - yi == -1 && abs(xi - xf) == 1 && b.state[65] == '@' + 8 * yf + xf) {
    n->state[65] = '@';
    n->state[8 * (yf + 1) + xf] = ' ';
    if (yf == 0)
      return promote(yi, xi, yf, yf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yi == 6 && yf - yi == -2 && xf == xi && piece == 32) {
    n->state[65] = '@' + 8 * (yf + 1) + xf;
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int knightMove(int yi, int xi, int yf, int xf, board b, board *n) {
	printf("Knight Move\n");
  if (abs(yf - yi) > 2 || abs(xf - xi) > 2)
    return 0;
  if (abs(yf - yi) + abs(xf - xi) == 3) {
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int bishopMove(int yi, int xi, int yf, int xf, board b, board *n) {
  if (abs(yf - yi) != abs(xf - xi))
    return 0;
  char p;
  for (int i = 1; i < (abs(yf - yi)); i++) {
    int x = yi + sign(yf - yi) * i;
    int y = xi + sign(xf - xi) * i;
    p = b.state[8 * x + y];
    if (p != 32)
      return 0;
  }
  n->state[8 * yf + xf] = n->state[8 * yi + xi];
  return 1;
}

int rookMove(int yi, int xi, int yf, int xf, board b, board *n) {
  if (yi != yf && xi != xf)
    return 0;
  char p;
  if (yi != yf)
    for (int i = 1; i < (abs(yf - yi)); i++) {
      int x = yi + sign(yf - yi) * i;
      int y = xi;
      p = b.state[8 * x + y];
      if (p != 32)
        return 0;
    }
  if (xi != xf)
    for (int i = 1; i < (abs(xf - xi)); i++) {
      int x = yi;
      int y = xi + sign(xf - xi) * i;
      p = b.state[8 * x + y];
      if (p != 32)
        return 0;
    }
  if (tolower(b.state[8 * yi + xi]) == 's')
    b.state[8 * yf + xf] = b.state[8 * yi + xi] - 1;
  n->state[8 * yf + xf] = n->state[8 * yi + xi];
  return 1;
}

int queenMove(int yi, int xi, int yf, int xf, board b, board *n) {
  return bishopMove(yi, xi, yf, xf, b, n) || rookMove(yi, xi, yf, xf, b, n);
}

// Move: yi, xi, yf, xf, piece on (yf,xf)
int validateMove(board b, int *move, board *n) {
  if (move[0] == move[2] && move[1] == move[3])
    return 0;
  for (int i = 0; i < 4; i++)
    if (move[i] < 0 || move[i] > 7)
      return 0;

  char piece = b.state[8 * move[0] + move[1]];
	printf("Piece: %c -> %c\n", piece, b.state[move[2]*8+move[3]]);
  if (piece == ' ' || (b.state[64] == 48) == (piece < 91))
    return 0;
	printf("Correct Color Piece\n");
  if (b.state[move[2] * 8 + move[3]] != ' ' &&
      islower(piece) == islower(b.state[move[2] * 8 + move[3]]))
    return 0;
  if (piece == 'p')
    return wpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
  if (piece == 'P')
    return bpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
  b.state[65] = '@';
  if (tolower(piece) == 'k' || tolower(piece) == 'j')
    return kingMove(move[0], move[1], move[2], move[3], b, n);
  if (tolower(piece) == 'n')
    return knightMove(move[0], move[1], move[2], move[3], b, n);
  if (tolower(piece) == 'b')
    return bishopMove(move[0], move[1], move[2], move[3], b, n);
  if (tolower(piece) == 'r' || tolower(piece) == 's')
    return rookMove(move[0], move[1], move[2], move[3], b, n);
  if (tolower(piece) == 'q')
    return queenMove(move[0], move[1], move[2], move[3], b, n);
  return 0;
}

int validateState(board b) {
  int kp, piece, apc = 0, cpr = b.state[64] - 48, aps[32], move[4];
  char ks = 'K' + 32 * cpr;
  board n = b;
  for (int i = 0; i < 64; i++) {
    piece = b.state[i];
    if (piece == ks)
      kp = i;
    if (piece != ' ' && piece - 32 * !cpr > 64 && piece - 32 * !cpr < 91)
      aps[apc++] = i;
  }
  move[2] = kp / 8;
  move[3] = kp % 8;
  for (int i = 0; i < apc; i++) {
    move[0] = aps[i] / 8;
    move[1] = aps[i] % 8;
    if (validateMove(b, move, &n)) {
      return 0;
    }
  }
  return 1;
}

board makeMove(board b, int *move) {
  board n = b, bc = b;
  if (validateMove(b, move, &n)) {
    n.state[8 * move[0] + move[1]] = ' ';
    n.state[64] = 48 + (n.state[64] == 48);
    if (!validateState(n))
      return bc;
    return n;
  }
  return bc;
}

void printBoard(board b) {
  int turn = b.state[64] - 48;
  if (!turn)
    for (int row = 0; row < 8; row++) {
      for (int rank = 0; rank < 8; rank++) {
        if (b.state[8 * row + rank] < 91)
          printf("\033[1;90m%c \033[0m", b.state[8 * row + rank]);
        else
          printf("\033[1;37m%c \033[0m", toupper(b.state[8 * row + rank]));
      }
      printf("\n");
    }
  else
    for (int row = 7; row >= 0; row--) {
      for (int rank = 7; rank >= 0; rank--) {
        if (b.state[8 * row + rank] < 91)
          printf("\033[1;90m%c \033[0m", b.state[8 * row + rank]);
        else
          printf("\033[1;37m%c \033[0m", toupper(b.state[8 * row + rank]));
      }
      printf("\n");
    }
  printf("\n\n");
}

void findPieces(board b, char piece, int *pos) {
  int index = 0;
  for (int i = 0; i < 64; i++)
    if (b.state[i] == piece) {
      pos[index++] = i / 8;
      pos[index++] = i % 8;
    }
}

void nameToMove(board b, char *name, int *move) {
  int index = 0, piece = 'P' + 32 * !(b.state[64] - 48), pos[16], move2[5];
  board n = b;
  for (int i = 0; i < 16; i++)
    pos[i] = -1;
  move2[4] = -1;

  // Castling
  if (name[0] == 'O') {
    move[1] = 7 * (b.state[64] - 48); move[3] = move[1];
    move[0] = 4; move[2] = 6;
    if (name[3]) move[2] = 4;
    return;
  }

  if (isupper(name[0]))
    piece = name[index++]+32*!(b.state[64]-48);
  if (name[index + 1] >= 'a') {
    if (name[index] >= 'a') move[1] = name[index++] - 98;
    else move[0] = name[index++] - 48;
  }
  if (name[index] == 'x')
    index++;

  move[3] = (name[index++] - 97);
  move[2] = (name[index++] - 49);

  findPieces(b, piece, pos);
  move2[2] = move[2];
  move2[3] = move[3];
  for (int i = 0; i < 16 && pos[i] >= 0; i += 2) {
    move2[0] = pos[i]; move2[1] = pos[i + 1];
    printf("ValidatindMove: %dx%d -> %dx%d\n", move2[0], move2[1], move2[2], move2[3]);
    if (validateMove(b, move2, &n)) {
      if ((move[1] == -1 && move[0] == -1) || move2[1] == move[0] ||
          move2[0] == move[1]) {
        move[1] = move2[1];
        move[0] = move2[0];
        break;
      }
    }
  }

  if (name[index] == '=')
    move[4] = name[index + 1];
  return;
}

void moveName(board b, int *move, char *name) {
  int yi = move[0], xi = move[1], yf = move[2], xf = move[3];
  char ip = tolower(b.state[8 * yi + xi]), fp = tolower(b.state[8 * yf + xf]);

  // Castling
  if (tolower(ip) == 'j' && xf - xi == 2) {
    strcpy(name, "O-O");
    return;
  }
  if (tolower(ip) == 'j' && xf - xi == -2) {
    strcpy(name, "O-O-O");
    return;
  }

  board n = b;
  char temp[3];
  char irn = 7 - yi + 49, frn = 7 - yf + 49, icn = xi + 97, fcn = xf + 97;
  temp[1] = 0;
  temp[2] = 0;

  // Promotion
  if (move[4] > 0) {
    temp[0] = frn;
    temp[1] = fcn;
    strcpy(name, temp);
    strcat(name, "=");
    temp[0] = move[4];
    temp[1] = 0;
    strcat(name, temp);
    return;
  }

  // Pawn move
  if (ip != 'p') {
    if (tolower(ip) == 'j')
      temp[0] = 'K';
    else if (tolower(ip) == 's')
      temp[0] = 'R';
    else
      temp[0] = toupper(ip);
    strcat(name, temp);
  }

  // Ambiguity
  int piece_pos[4];
  for (int i = 0; i < 4; i++)
    piece_pos[i] = -1;
  findPieces(b, ip, piece_pos);
  int temp_move[4] = {piece_pos[2], piece_pos[3], yf, xf};
  if (validateMove(b, temp_move, &n)) {
    if (piece_pos[2] != piece_pos[0])
      temp[0] = irn;
    else
      temp[0] = icn;
    strcat(name, temp);
  }

  // Capture
  if (fp != ' ') {
    temp[0] = 'x';
    strcat(name, temp);
  }

  // Destination
  temp[0] = fcn;
  strcat(name, temp);
  temp[0] = frn;
  strcat(name, temp);
}

// SQLITE3
char *deepLines(board b, int max_depth, int cur_depth, sqlite3 *db) {
  const char *get_move = "SELECT Line, Child, Move FROM Lines "
                         "WHERE Parent = ? ";
  board n;
  int rc;
  char child[67], line[64], move[8], *result = malloc(512);
  result[0] = '\0';
  if (cur_depth >= max_depth) {
    return result;
  }
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, get_move, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, b.state, -1, SQLITE_STATIC);
  do {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      strcpy(line, sqlite3_column_text(stmt, 0));
      if (cur_depth == 0)
        strcat(strcat(result, line), "\n*");
      strcpy(child, sqlite3_column_text(stmt, 1));
      strcpy(move, sqlite3_column_text(stmt, 2));
      strcpy(n.state, child);
      if (cur_depth != 0) {
        for (int i = 0; i < cur_depth; i++)
          strcat(result, "   ");
        strcat(result, "↳");
      }
      strcat(strcat(result, move), "\n");
      strcat(result, deepLines(n, max_depth, cur_depth + 1, db));
    }
  } while (rc == SQLITE_ROW);
  sqlite3_finalize(stmt);
  return result;
}

board getMove(board b, int *move, char *line, void (*comment_cb)(char *),
              void (*promotion_cb)(char *)) {
  board n, bc = b;
  sqlite3_stmt *stmt;
  sqlite3 *db;
  sqlite3_open("optree.db", &db);

  const char *add_link =
      "INSERT INTO Lines (Line, Parent, Child, Comment, Move) "
      "VALUES(?, ?, ?, ?, ?) "
      "ON CONFLICT(Parent, Child) DO UPDATE SET "
      "Line = CASE "
      "WHEN instr(Lines.Line, excluded.Line) = 0 THEN Lines.Line || ' | ' || "
      "excluded.Line "
      "ELSE Lines.Line END, "
      "Comment = CASE "
      "WHEN Lines.Comment = '' THEN excluded.Comment "
      "WHEN excluded.Comment = '' THEN Lines.Comment "
      "WHEN instr(Lines.Comment, excluded.Comment) = 0 THEN Lines.Comment || ' "
      "| ' || excluded.Comment "
      "ELSE Lines.Comment END;";
  sqlite3_prepare_v2(db, add_link, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, line, -1, SQLITE_STATIC);

  char comment[512] = "";

  n = makeMove(b, move);
  char movename[16] = "";
  moveName(bc, move, movename);
  if (strcmp(n.state, bc.state)) {
    comment_cb(comment);
    sqlite3_bind_text(stmt, 2, bc.state, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, n.state, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, comment, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, movename, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
      fprintf(stderr, "Insert Failed %s\n", sqlite3_errmsg(db));
      char dummy[16];
      scanf("%s", dummy);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return n;
  }
  return b;
}

int main() {
  board b, n;
  link l;
  b = createBoard();
  n = b;
  int move[4], rc;
  char conf = 'y', aux[2], line[32] = "", input[64];
  sqlite3 *db;
  sqlite3_stmt *stmt;

  while (conf != 'q') {
    fflush(stdout);
    b = n;
    printf("\n");
    printBoard(b);
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, " %c%d %c%d", aux, move, aux + 1, move + 2) == 4) {
      move[1] = aux[0] - 97;
      move[3] = aux[1] - 97;
      move[0] = 8 - move[0];
      move[2] = 8 - move[2];
      // n=getMove(b, move, input, line);
    } else
      conf = 'q';
  }
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return 0;
}
