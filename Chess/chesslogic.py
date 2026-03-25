import tkinter as tk
import tkinter.simpledialog as simpledialog
from PIL import ImageTk, Image
from ctypes import *
from pathlib import Path


class board(Structure):
    _fields_= [
            ("state", c_char*67)
    ]

lib_path = (Path(__file__).parent / 'treelogic.so').resolve()
try:
    treelib = CDLL(lib_path)
except OSError as e:
    print(f"Smt's wrn: {e}")
    raise
treelib.createBoard.restype = (board)

treelib.makeMove.argtypes = (board, POINTER(c_int))
treelib.makeMove.restype = (board)

treelib.nameToMove.argtypes = (board, c_char_p, POINTER(c_int))
treelib.moveName.argtypes = (board, POINTER(c_int), c_char_p)
treelib.getChildren.argtypes = (board, c_char_p)
treelib.controlCount.argtypes = (board, POINTER(c_int))

def createBoard() -> str:
    return treelib.createBoard().state.decode("utf-8")

def make_move(boardstate, move):
    b = board(boardstate.encode("utf-8"))
    nboard = treelib.makeMove(b, (c_int*5)(*(int(x) for x in move)))
    return nboard.state.decode("utf-8")

def nameToMove(boardstate, movename):
    move = (c_int * 5)(*[-1]*5)
    b = board(boardstate.encode("utf-8"))
    treelib.nameToMove(b, movename.encode("utf-8"), move)
    return list(move)

def moveName(boardstate, move):
    b = board(boardstate.encode("utf-8"))
    name = create_string_buffer(16)
    treelib.moveName(b, move, name)
    name = name.value.decode("utf-8")
    return name

def getChildren(boardstate):
    res = []
    b = board(boardstate.encode("utf-8"))
    children = create_string_buffer(512)
    treelib.getChildren(b, children)
    moves = children.value.decode()[:-1]
    moves = moves.split("|")
    for move in moves:
        move = move[:5]
        pmove = (c_int * 5)(*[int(ch) for ch in move[:-1]] + [ord(move[-1])])
        pos = make_move(boardstate, pmove)
        movename = moveName(boardstate, pmove)
        res.append((pos, movename))
    return res[:-1]

def controlCount(boardstate):
    b = board(boardstate.encode("utf-8"))
    control_array = (c_int*64)(*[0]*64)
    treelib.controlCount(b, control_array)
    return list(control_array)
