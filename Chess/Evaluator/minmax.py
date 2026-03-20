from random import uniform
import sys
sys.path.append("..")
from innerutils import printBoard, heuristic
from utils.chesslogic import getChildren
from functools import cache

@cache
def minmax(evaluator, board: str, depth: int=0, turn: bool=False, max_depth: int=4, alpha=float('-inf'), beta=float('inf'), moveseq="") -> tuple[str, float]:
    best_pos = None
    moveseq += board + ": "
    if depth >= max_depth:
        # print(moveseq)
        # printBoard(board)
        return (board, evaluator.evaluate(board))
    else:
        nodes = getChildren(board)
        nodes.sort(key=lambda x: heuristic("".join(list(x[0])[:64])))
        for pos in nodes:
            eval = minmax(evaluator, pos[0], depth+1, not turn, max_depth, alpha, beta, moveseq+pos[1]+"\n")[1]
            if turn:
                alpha = max(alpha, eval)
                if beta <= alpha:
                    break
                if best_pos is None or eval > best_pos[1]:
                    best_pos = (pos[0], eval)
            else: 
                beta = min(beta, eval)
                if beta <= alpha:
                    break
                if best_pos is None or eval < best_pos[1]:
                    best_pos = (pos[0], eval)
    if best_pos is None:
        # print(moveseq)
        # printBoard(board)
        best_pos = (board, evaluator.evaluate(board))
    return best_pos
