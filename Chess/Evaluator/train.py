from minmax import minmax
import innerutils
import datetime
from model import Evaluator
import sys
import os
import random
sys.path.append("..")
from utils.chesslogic import getChildren, createBoard
from time import perf_counter


os.system("clear")

board: str = createBoard()
children: list[str] = getChildren(board)
model1 = Evaluator("models/WhitePieces.keras")
model2 = Evaluator("models/BlackPieces.keras")

def reward_function(board):
    material = innerutils.material_count(board)
    safety = innerutils.king_safety(board)
    structure = innerutils.pawn_structure(board)
    center = innerutils.center_control(board)
    return material + safety + structure + center


e = 1
edcay = 0.9

while(True):
    for k in range(100):
        for j in range(10):
            if len(children)==0:
                board = createBoard()
            for i in range(10):
                old_board = board
                if random.random() > e:
                    board = minmax(model1, board, max_depth=6)[0]
                else:
                    board = random.choice(children)[0]
                children = getChildren(board)
                if len(children) == 0:
                    reward = 100
                if random.random() > e:
                    board = minmax(model2, board, max_depth=6)[0]
                else:
                    board = random.choice(children)[0]
                if len(children) == 0:
                    reward = -100
                else:
                    reward = reward_function(board)
                print(reward)
                e *= edcay
                children = getChildren(board)
                model1.remember(old_board, reward, board, len(children)>0)
                model2.remember(old_board, -(reward), board, len(children)>0)
            print(f"Learning at {datetime.datetime.now()}")
            model1.replay()
            model2.replay()
            minmax.cache_clear()
        print("Saving")
        e = 1
        model1.save()
        model2.save()
