import os
from minmax import minmax
from innerutils import heuristic, printBoard
from utils.chesslogic import createBoard, getChildren

board = createBoard()

class Player:
    def __init__(self, board, color):
        self.board = board
        self.color = color

    def evaluate(self, board):
        return self.color*heuristic(board)

player1 = Player(board, 1)
player2 = Player(board, 2)


while True:
    board = minmax(player1, board, max_depth=7)[0]
    player1.board = board
    print(f"Evaluation by P1: {player1.evaluate(board)}")
    printBoard(board)
    if len(getChildren(board)) == 0:
        break

    board = minmax(player2, board, max_depth=7)[0]
    player2.board = board
    print(f"Evaluation by P2: {player2.evaluate(board)}")
    printBoard(board)
    if len(getChildren(board)) == 0:
        break
