from utils.chesslogic import controlCount

SAFETY_MODIFIER = 2
STRUCTURE_MODIFIER = 0.6
CENTER_MODIFIER = 0.4

def printBoard(board: str) -> None:
    for i in range(8):
        for j in range(8):
            print(board[8*i+j], end=' ')
        print()
    print('===========================\n')

piece_value = {
    'q': 9,
    'r': 5,
    's': 5,
    'b': 3.25,
    'n': 3,
    'p': 1,
    'k': 0,
    'j': 0
}

def material_count(board: str) -> float:
    reward = 0.0
    boardlist = list(board)[:63]
    for piece in boardlist:
        if not piece.isspace():
            reward += piece_value[piece.lower()] if piece.islower() else -piece_value[piece.lower()]
    return reward

def king_safety(board: str) -> float:
    reward = 0.0
    control_board = controlCount(board)
    if 'K' in board:
        wking = board.index('K')
    else:
        wking = board.index('J')
    if 'k' in board:
        bking = board.index('k')
    else:
        bking = board.index('j')
    for i, control in enumerate(control_board):
        reward += control*max(0, 2 - int(max(abs(i // 8 - wking // 8), abs(i % 8 - wking % 8))))/5
        reward -= control*max(0, 2 - int(max(abs(i // 8 - bking // 8), abs(i % 8 - bking % 8))))/5
        if board[i]==' ' and min(abs(wking/8-i/8), abs(wking%8-i%8)) < 2:
            reward -= 1
        if board[i]==' ' and min(abs(bking/8-i/8), abs(bking%8-i%8)) < 2:
            reward += 1
    return reward*SAFETY_MODIFIER

def pawn_structure(board: str) -> float:
    reward = 0.0
    for i in range(0, 56, 2):
        if board[i]=='p' and board[i+8]=='p':
            reward += 1
        elif board[i]=='P' and board[i+8]=='P':
            reward -=1
    return reward*STRUCTURE_MODIFIER

def center_control(board: str) -> float:
    reward = 0.0
    control_board = controlCount(board)
    for i, control in enumerate(control_board):
        if control>0 and i>24  or control<0 and i<48:
            reward += control*(max(0, 2 - int(max(abs(3.5-i//8), abs(3.5-i%8)))))
    return reward*CENTER_MODIFIER

def heuristic(board: str) -> float:
    return material_count(board) + king_safety(board) + pawn_structure(board) + center_control(board)
