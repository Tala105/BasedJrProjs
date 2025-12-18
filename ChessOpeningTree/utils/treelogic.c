#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>


typedef struct board board;
struct board{
	char state[67];
};

typedef struct link{
	char parent_state[67];
	char child_state[67];
	char comment[128];
}link;

int sign(int x){
	return (x>0) - (x<0);
}

board createBoard(){
	board b;
	strcpy(b.state, "snbqjbnspppppppp                                PPPPPPPPSNBQJBNS0@");
	return b;
}

int validateState(board b);

int kingMove(int xi, int yi, int xf, int yf, board b, board *n){
	if(abs(xi-xf)>1 || abs(yi-yf)>2) return 0;
	if(xi==xf && abs(yf-yi)==2 && tolower(b.state[8*xi+yi]) == 'j'){
		int dir = sign(yf-yi);
		char ik = b.state[8*xi+yi];
		char ir = b.state[8*xi+(7*((dir+1)/2))];
		if(tolower(b.state[8*xi+(7*((dir+1)/2))]) == 's'){
			board n2;
			n2.state[8*xi+yi] = ' ';
			n2.state[8*xi+yi+dir] = ik;
			if(!validateState(n2)) return 0;
			n2.state[8*xi+yi+dir] = ' ';
			n2.state[8*xi+yi+2*dir] = ik;
			if(!validateState(n2)) return 0;
			n->state[8*xf+yf] = ik+1;
			n->state[8*xi+(4+dir)] = ir-1;
			n->state[8*xi+(7*((dir+1)/2))] = ' ';
			return 1;
		}
		return 0;
	}
	if(abs(yi-yf)>1) return 0;
	if(tolower(b.state[8*xi+yi]) == 'j') n->state[8*xf+yf] = n->state[8*xi+yi]+1;
	n->state[8*xf+yf] = n->state[8*xi+yi];
	return 1;
}

int promote(int xi, int yi, int xf, int yf, char np, board b, board *n){
	np = tolower(np);
	if(np == 'n' || np == 'b' || np == 'r' || np == 'q'){
		if(isupper(b.state[8*xi+yi])) n->state[8*xi+yi] = toupper(np);
		else n->state[8*xi+yi] = np;
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}
	return 0;
}

int wpawnMove(int xi, int yi, int xf, int yf, char np, board b, board *n){
	int piece = b.state[8*xf+yf];
	if((xf-xi==-1 && abs(yi-yf)==1 && piece != 32) || (xf-xi==-1 && yf==yi && piece==32)){
		n->state[65] = '@';
		if(xf==0) return promote(xi, yi, xf, yf, np, b, n);
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}

	if(xf-xi==-1 && abs(yi-yf)==1 && b.state[65] == '@'+8*xf+yf){
		n->state[65] = '@';
		n->state[8*(xf+1)+yf] = ' ';
		if(xf==0) return promote(xi, yi, xf, xf, np, b, n);
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}

	if(xi==6 && xf-xi==-2 && yf==yi && piece==32){
		n->state[65] = '@' + 8*(xf+1)+yf;
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}
	return 0;
}

int bpawnMove(int xi, int yi, int xf, int yf, char np, board b, board *n){
	int piece = b.state[8*xf+yf];
	if((xf-xi==1 && abs(yi-yf)==1 && piece != 32) || (xf-xi==1 && yf==yi && piece==32)){
		n->state[65] = '@';
		if(xf==0) return promote(xi, yi, xf, yf, np, b, n);
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}

	if(xf-xi==1 && abs(yi-yf)==1 && b.state[65] == '@'+8*xf+yf){
		n->state[65] = '@';
		n->state[8*(xf-1)+yf] = ' ';
		if(xf==0) return promote(xi, yi, xf, yf, np, b, n);
		else return 1;
	}

	if(xi==1 && xf-xi==2 && yf==yi && piece==32){
		n->state[65] = '@' + 8*(xf-1)+yf;
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}
	return 0;
}

int knightMove(int xi, int yi, int xf, int yf, board b, board *n){
	if(abs(xf-xi) > 2 || abs(yf-yi)>2) return 0;
	if(abs(xf-xi)+abs(yf-yi)==3){
		n->state[8*xf+yf] = n->state[8*xi+yi];
		return 1;
	}
	return 0;
}

int bishopMove(int xi, int yi, int xf, int yf, board b, board *n){
	if(abs(xf-xi)!=abs(yf-yi)) return 0;
	char p;
	for(int i=1; i<(abs(xf-xi)); i++){
		int x = xi + sign(xf-xi)*i;
		int y = yi + sign(yf-yi)*i;
		p = b.state[8*x+y];
		if(p != 32)
			return 0;
	}
	n->state[8*xf+yf] = n->state[8*xi+yi];
	return 1;
}

int rookMove(int xi, int yi, int xf, int yf, board b, board *n){
	if(xi!=xf && yi!=yf) return 0;
	char p;
	if(xi!=xf)
		for(int i=1; i<(abs(xf-xi)); i++){
			int x = xi + sign(xf-xi)*i;
			int y = yi;
			p = b.state[8*x+y];
			if(p != 32) return 0;
		}
	if(yi!=yf)
		for(int i=1; i<(abs(yf-yi)); i++){
			int x = xi;
			int y = yi + sign(yf-yi)*i;
			p = b.state[8*x+y];
			if(p != 32) return 0;
		}
	if(tolower(b.state[8*xi+yi]) == 's') b.state[8*xf+yf] = b.state[8*xi+yi]-1;
	n->state[8*xf+yf] = n->state[8*xi+yi];
	return 1;
}

int queenMove(int xi, int yi, int xf, int yf, board b, board *n){
	return bishopMove(xi, yi, xf, yf, b, n) || rookMove(xi, yi, xf, yf, b, n);
}

//Move: xi, yi, xf, yf, piece on (xf,yf)
int validateMove(board b, int *move, board *n){
	if(move[0] == move[2] && move[1] == move[3]) return 0;
	for(int i=0; i<4; i++) if(move[i] < 0 || move[i] >7) return 0;

	char piece = b.state[8*move[0] + move[1]];
	if(piece == ' ' || (b.state[64]==48) == (piece<91)) return 0;
	if(b.state[move[2]*8+move[3]] != ' ' && islower(piece) == islower(b.state[move[2]*8+move[3]])) return 0;
	if(piece == 'p') return wpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
	if(piece == 'P') return bpawnMove(move[0], move[1], move[2], move[3], move[4], b, n);
	b.state[65] = '@';
	if(tolower(piece) == 'k' || tolower(piece) == 'j') return kingMove(move[0], move[1], move[2], move[3], b, n);
	if(tolower(piece) == 'n') return knightMove(move[0], move[1], move[2], move[3], b, n);
	if(tolower(piece) == 'b') return bishopMove(move[0], move[1], move[2], move[3], b, n);
	if(tolower(piece) == 'r' || tolower(piece) == 's') return rookMove(move[0], move[1], move[2], move[3], b, n);
	if(tolower(piece) == 'q') return queenMove(move[0], move[1], move[2], move[3], b, n);
	return 0;
}

int validateState(board b){
	int kp, piece, apc=0, cpr = b.state[64]-48, aps[32], move[4];
	char ks = 'K'+32*cpr;
	board  n = b;
	for(int i=0; i<64; i++){
		piece = b.state[i];
		if(piece == ks) kp=i;
		if(piece!=' ' && piece-32*!cpr>64 && piece-32*!cpr<91) aps[apc++] = i;
	}
	move[2] = kp/8; move[3] = kp%8;
	for(int i=0; i<apc; i++){
		move[0] = aps[i]/8; move[1] = aps[i]%8;
		if(validateMove(b, move, &n)){
			return 0;
		}
	}
	return 1;
}

board makeMove(board b, int *move){
	board n=b, bc = b;
	if (validateMove(b, move, &n)){
		n.state[8*move[0]+move[1]] = ' ';
		n.state[64] = 48 + (n.state[64]==48);
		if(!validateState(n)) return bc;
		return n;
	}
	return bc;
}

void printBoard(board b){
	int turn = b.state[64]-48;
	if(!turn)
		for(int row=0; row<8; row++){
			for(int rank=0; rank<8; rank++){
				if(b.state[8*row+rank]<91) printf("\033[1;90m%c \033[0m", b.state[8*row+rank]);
				else printf("\033[1;37m%c \033[0m", toupper(b.state[8*row+rank]));
			}
			printf("\n");
		}
	else
		for(int row=7; row>=0; row--){
			for(int rank=7; rank>=0; rank--){
				if(b.state[8*row+rank]<91) printf("\033[1;90m%c \033[0m", b.state[8*row+rank]);
				else printf("\033[1;37m%c \033[0m", toupper(b.state[8*row+rank]));
			}
			printf("\n");
		}
	printf("\n\n");
}

void findPieces(board b, char piece, int *pos){
	int index = 0;
	for(int i=0; i<64; i++)
		if(b.state[i]==piece){
			pos[index++]=i/8;
			pos[index++]=i%8;
		}
}

void nameToMove(board b, char *name, int *move){
	//TODO:
}

void moveName(board b, int *move, char *name){
	board n = b;
	int xi=move[0], yi=move[1], xf=move[2], yf=move[3];
	char ip = tolower(b.state[8*xi + yi]), fp = tolower(b.state[8*xf+yf]);
	
	// Castling
	if(tolower(ip)=='j' && yf-yi==2){
		strcpy(name, "O-O");
		return;
	}
	if(tolower(ip)=='j' && yf-yi==-2){
		strcpy(name, "O-O-O");
		return;
	}

	char temp[3];
	char irn = 7-xi+49, frn=7-xf+49, icn = yi+97, fcn = yf+97;
	temp[1] = 0;
	temp[2] = 0;

	// Promotion
	if(move[4]>0){
		temp[0] = frn; temp[1] = fcn;
		strcpy(name, temp);
		strcat(name, "=");
		temp[0] = move[4]; temp[1] = 0;
		strcat(name, temp);
		return;
	}

	// Pawn move
	if(ip != 'p'){
		if(tolower(ip)=='j') temp[0] = 'K';
		else if(tolower(ip)=='s') temp[0] = 'R';
		else temp[0] = toupper(ip);
		strcat(name, temp);
	}
	
	// Ambiguity
	int piece_pos[4];
	for(int i=0; i<4; i++) piece_pos[i]=-1;
	findPieces(b, ip, piece_pos);
	int temp_move[4] = {piece_pos[2], piece_pos[3], xf, yf};
	if(validateMove(b, temp_move, &n)){
		if(piece_pos[2] != piece_pos[0]) temp[0] = irn;
		else temp[0] = icn;
		strcat(name, temp);
	}

	// Capture
	if(fp != ' '){
		temp[0]= 'x';
		strcat(name, temp);
	}

	// Destination
	temp[0] = fcn;
	strcat(name,temp);
	temp[0]=frn;
	strcat(name,temp);
}

// SQLITE3
char *deepLines(board b, int max_depth, int cur_depth, sqlite3 *db){
	const char *get_move =
		"SELECT Line, Child, Move FROM Lines "
		"WHERE Parent = ? "
	;
	board n;
	int rc;
	char child[67], line[64], move[8], *result=malloc(512);
	result[0] = '\0';
	if(cur_depth>=max_depth){
		return result;
	}
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, get_move, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, b.state, -1, SQLITE_STATIC);
	do{
		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW){
			strcpy(line, sqlite3_column_text(stmt, 0));
			if(cur_depth==0) strcat(strcat(result, line),"\n*");
			strcpy(child, sqlite3_column_text(stmt, 1));
			strcpy(move, sqlite3_column_text(stmt, 2));
			strcpy(n.state, child);
			if(cur_depth!=0){
				for(int i=0; i<cur_depth; i++)
					strcat(result, "   ");
				strcat(result, "↳");
			}
			strcat(strcat(result, move), "\n");
			strcat(result, deepLines(n, max_depth, cur_depth+1, db));
		}
	}while(rc == SQLITE_ROW);
	sqlite3_finalize(stmt);
	return result;
}

board getMove(board b, int *move, char* line, void (*comment_cb)(char *), void (*promotion_cb)(char *)){
	board n, bc=b;
	sqlite3_stmt *stmt;
	sqlite3 *db;
	sqlite3_open("optree.db", &db);

	const char *add_link =
		"INSERT INTO Lines (Line, Parent, Child, Comment, Move) "
		"VALUES(?, ?, ?, ?, ?) "
		"ON CONFLICT(Parent, Child) DO UPDATE SET "
		"Line = CASE "
		"WHEN instr(Lines.Line, excluded.Line) = 0 THEN Lines.Line || ' | ' || excluded.Line "
		"ELSE Lines.Line END, "
		"Comment = CASE "
		"WHEN Lines.Comment = '' THEN excluded.Comment "
		"WHEN excluded.Comment = '' THEN Lines.Comment "
		"WHEN instr(Lines.Comment, excluded.Comment) = 0 THEN Lines.Comment || ' | ' || excluded.Comment "
		"ELSE Lines.Comment END;"
	;
	sqlite3_prepare_v2(db, add_link, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, line, -1, SQLITE_STATIC);

	char comment[512]="";

	n = makeMove(b, move);
	char movename[16] = "";
	moveName(bc, move, movename);
	if(strcmp(n.state, bc.state)){
		comment_cb(comment);
		sqlite3_bind_text(stmt, 2, bc.state, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, n.state, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 4, comment, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 5, movename, -1, SQLITE_STATIC);
		int rc = sqlite3_step(stmt);
		if(rc != SQLITE_DONE){
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


int main(){
	board b, n;
	link l;
	b = createBoard();
	n=b;
	int move[4], rc;
	char conf='y', aux[2], line[32]="", input[64];
	sqlite3 *db;
	sqlite3_stmt *stmt;


	while(conf!='q'){
		fflush(stdout);
		b=n;
		printf("\n");
		printBoard(b);
		fgets(input, sizeof(input), stdin);
		if(sscanf(input, " %c%d %c%d", aux, move, aux + 1, move + 2) == 4){
			move[1] = aux[0] - 97; move[3] = aux[1] - 97;
			move[0] = 8 - move[0]; move[2] = 8 - move[2];
			// n=getMove(b, move, input, line);
		}
		else conf = 'q';
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return 0;
}
