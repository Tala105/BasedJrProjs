#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct board board;
struct board {
  char state[67];
};

int sign(int x) { return (x > 0) - (x < 0); }

board createBoard() {
  board b;
  strcpy(b.state,
         "snbqjbnspppppppp                                PPPPPPPPSNBQJBNS0@");
  return b;
}

board setBoard(board b, char *state){
	strncpy(b.state, state, 66);
	b.state[66] = '\0';
	return b;
}

int validateState(board b);

int kingMove(int yi, int xi, int yf, int xf, board b, board *n) {
  if (abs(yi - yf) > 1 || abs(xi - xf) > 2)
    return 0;
  if (yi == yf && abs(xf - xi) == 2 && tolower(b.state[8 * yi + xi]) == 'j') {
    int dir = sign(xf - xi);
    int x = xi + 1, y = yi;
    while (tolower(b.state[8 * y + x + dir]) != 's') {
      if (b.state[8 * y + x] != ' ')
        return 0;
      x++;
    }
    char ik = b.state[8 * yi + xi];
    char ir = b.state[8 * yi + (7 * ((dir + 1) / 2))];
    if (tolower(b.state[8 * yi + (7 * ((dir + 1) / 2))]) == 's') {
      board n2 = b;
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
    n->state[8 * yi + xi] = n->state[8 * yi + xi] + 1;
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
  int piece = b.state[8 * yf + xf];
  if ((yf - yi == 1 && abs(xi - xf) == 1 && piece != 32) ||
      (yf - yi == 1 && xf == xi && piece == 32)) {
    n->state[65] = 64;
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yf - yi == 1 && abs(xi - xf) == 1 && b.state[65] == 8 * yf + xf) {
    n->state[65] = 64;
    n->state[8 * (yf - 1) + xf] = ' ';
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    else
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
      return 1;
  }

  if (yi == 1 && yf - yi == 2 && xf == xi && piece == 32) {
    n->state[65] = 8 * (yf - 1) + xf;
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int bpawnMove(int yi, int xi, int yf, int xf, char np, board b, board *n) {
  int piece = b.state[8 * yf + xf];
  if ((yf - yi == -1 && abs(xi - xf) == 1 && piece != 32) ||
      (yf - yi == -1 && xf == xi && piece == 32)) {
    n->state[65] = 64;
    if (yf == 0)
      return promote(yi, xi, yf, xf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yf - yi == -1 && abs(xi - xf) == 1 && b.state[65] == 8 * yf + xf) {
    n->state[65] = 64;
    n->state[8 * (yf + 1) + xf] = ' ';
    if (yf == 0)
      return promote(yi, xi, yf, yf, np, b, n);
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }

  if (yi == 6 && yf - yi == -2 && xf == xi && piece == 32) {
    n->state[65] = 8 * (yf + 1) + xf;
    n->state[8 * yf + xf] = n->state[8 * yi + xi];
    return 1;
  }
  return 0;
}

int knightMove(int yi, int xi, int yf, int xf, board b, board *n) {
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
    n->state[8 * yi + xi] = n->state[8 * yi + xi] - 1;
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
  if (piece == ' ' || (b.state[64] == 48) == (piece < 91))
    return 0;
  if (b.state[move[2] * 8 + move[3]] != ' ' &&
      islower(piece) == islower(b.state[8 * move[2] + move[3]]))
    return 0;
  if (piece == 'p')
    return wpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
  if (piece == 'P')
    return bpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
  n->state[65] = 64;
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

int ischeck(board b, int turn) {
  int kp, piece, apc = 0, aps[32], move[4];
  char ks = 'K' + 32 * turn;
  board n = b;
  for (int i = 0; i < 64; i++) {
    piece = b.state[i];
    if (piece == ks || piece == ks-1)
      kp = i;
    if (isalpha(piece) && islower(piece) != islower(ks)){
      aps[apc++] = i;
	}
  }
  move[2] = kp / 8;
  move[3] = kp % 8;
  for (int i = 0; i < apc; i++) {
    move[0] = aps[i] / 8;
    move[1] = aps[i] % 8;
    if (validateMove(b, move, &n)) {
      return 1;
    }
  }
  return 0;
}

int validateState(board b) { return !ischeck(b, b.state[64]); }

int fullValidation(board b, int *move, board *n) {
  if (validateMove(b, move, n)) {
    n->state[8 * move[0] + move[1]] = ' ';
    n->state[64] = 48 + (n->state[64] == 48);
    if (validateState(*n))
      return 1;
  }
  return 0;
}

board makeMove(board b, int *move) {
  board n = b;
  if (fullValidation(b, move, &n)) {
	return n;
  }
  return b;
}

void findPieces(board b, char piece, int *pos) {
  int index = 0;
  for (int i = 0; i < 64; i++) {
    if (b.state[i] == 'j')
      b.state[i] = 'k';
    else if (b.state[i] == 'J')
      b.state[i] = 'K';
    else if (b.state[i] == 's')
      b.state[i] = 'r';
    else if (b.state[i] == 'S')
      b.state[i] = 'R';
    if (b.state[i] == piece) {
      pos[index++] = i / 8;
      pos[index++] = i % 8;
    }
  }
}

void nameToMove(board b, char *name, int *move) {
  int index = 0, piece = 'P' + 32 * !(b.state[64] - 48), pos[16], move2[5];
  board n = b;
  for (int i = 0; i < 16; i++)
    pos[i] = -1;
  move2[4] = 'Q';

  // Castling
  if (name[0] == 'O') {
    move[1] = 7 * (b.state[64] - 48);
    move[3] = move[1];
    move[0] = 4;
    move[2] = 6;
    if (name[3])
      move[2] = 4;
    return;
  }

  if (isupper(name[0]))
    piece = name[index++] + 32 * !(b.state[64] - 48);
  else if (name[index + 1] >= 'a') {
    if (name[index] >= 'a')
      move[1] = name[index++] - 97;
    else
      move[0] = name[index++] - 49;
  }
  if (name[index] == 'x')
    index++;

  move[3] = (name[index++] - 97);
  move[2] = (name[index++] - 49);

  findPieces(b, piece, pos);
  move2[2] = move[2];
  move2[3] = move[3];
  for (int i = 0; i < 16 && pos[i] >= 0; i += 2) {
    move2[0] = pos[i];
    move2[1] = pos[i + 1];
    if (validateMove(b, move2, &n)) {
      if ((move[1] == -1 && move[0] == -1) || move2[0] == move[0] ||
          move2[1] == move[1]) {
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
  char irn = yi + 49, frn = yf + 49, icn = xi + 97, fcn = xf + 97;
  temp[1] = 0;
  temp[2] = 0;

  // Promotion
  if (move[4] == 'Q' || move[4] == 'R' || move[4] == 'B' || move[4] == 'N') {
    sprintf(name, "%d ", move[2]);
    if (fp != ' ') {
      temp[0] = icn;
      temp[1] = 'x';
      strcat(name, temp);
    }
    temp[0] = fcn;
    temp[1] = frn;
    strcat(name, temp);
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
  int piece_pos[16];
  for (int i = 0; i < 16; i++)
    piece_pos[i] = -1;
  findPieces(b, ip, piece_pos);
  int temp_move[4] = {0, 0, yf, xf};
  for (int idx = 2; idx < 16 && piece_pos[idx] != -1;) {
    temp_move[3] = piece_pos[idx++];
    piece_pos[4] = piece_pos[idx++];
    if (validateMove(b, temp_move, &n)) {
      if (piece_pos[2] != piece_pos[0])
        temp[0] = irn;
      else
        temp[0] = icn;
      strcat(name, temp);
    }
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

void controlCount(board b, int *count_array) {
  int move[4];
  board n = b;
  for (int i = 0; i < 64; i++)
    for (int j = 0; j < 64; j++)
      if (b.state[i] != ' ') {
        move[0] = i / 8;
        move[1] = i % 8;
        move[2] = j / 8;
        move[3] = j % 8;
        if (validateMove(b, move, &n))
          count_array[j] +=
              (1 + tolower(b.state[i]) == 'p') * (2 * islower(b.state[i]) - 1);
      }
}

void getChildren(board b, char *res) {
  int move[5], offset = 0;
  board n = b;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      for (int i2 = 0; i2 < 8; i2++)
        for (int j2 = 0; j2 < 8; j2++)
          if (b.state[8 * i + j] != ' ') {
            move[0] = i;
            move[1] = j;
            move[2] = i2;
            move[3] = j2;
            move[4] = 'E';
            if (tolower(b.state[8 * i + j]) == 'p' && (i2 == 0 || i2 == 7)) {
              move[4] = 'Q';
              if (validateMove(b, move, &n))
                offset += sprintf(res + offset, "%d%d%d%d%c|", move[0], move[1],
                                  move[2], move[3], move[4]);
              move[4] = 'R';
              if (validateMove(b, move, &n))
                offset += sprintf(res + offset, "%d%d%d%d%c|", move[0], move[1],
                                  move[2], move[3], move[4]);
              move[4] = 'B';
              if (validateMove(b, move, &n))
                offset += sprintf(res + offset, "%d%d%d%d%c|", move[0], move[1],
                                  move[2], move[3], move[4]);
              move[4] = 'N';
            }
            if (validateMove(b, move, &n))
              offset += sprintf(res + offset, "%d%d%d%d%c|", move[0], move[1],
                                move[2], move[3], move[4]);
          }
}

void printBoard(board b) {
  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j < 8; j++){
	  if(8*i+j == b.state[65])
       printf("\033[44m%c\033[0m ", b.state[8 * i + j]);
			
      printf("%c ", b.state[8 * i + j]);
		}
    printf("\n");
  }
}

int main() {
  board b, n;
  b = createBoard();
  n = b;
  int *move = malloc(5 * sizeof(int));
  for (int i = 0; i < 5; i++)
    move[i] = -1;
  char input[16] = "", state[128];

  while (strcmp(input, "q")) {
	// system("clear");
    printBoard(b);
    printf("\n");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
	if (!strcmp(input, "set")){
		fgets(state, sizeof(b.state), stdin);
		b = setBoard(b, state);
		}
    for (int i = 0; i < 5; i++)
      move[i] = -1;
    nameToMove(b, input, move);
    b = makeMove(b, move);
  }
  return 0;
}
