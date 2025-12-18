import tkinter as tk
import tkinter.simpledialog as simpledialog
from PIL import ImageTk, Image
from ctypes import *



class board(Structure):
    _fields_= [
            ("state", c_char*67)
    ]

lib_path = 'utils/treelogic.so'
try:
    treelib = CDLL(lib_path)
except OSError as e:
    print(f"Smt's wrn: {e}")
    raise
treelib.createBoard.restype = (board)

treelib.makeMove.argtypes = (board, POINTER(c_int))
treelib.makeMove.restype = (board)

treelib.nameToMove.argtypes = (board, c_char_p)
treelib.nameToMove.restype = (POINTER(c_int))

def make_move(b, move):
    return treelib.makeMove(b, move)

def nameToMove(move):
    treelib.nameToMove(move)
    return move

if __name__ == "__main__":
    curr_line = 0
    max_lines = 4
    rw=900
    rh=600
    selected_piece = []

    # CLib Prep
    treelib.deepLines.argtypes = (board, c_int, c_int, c_void_p)
    treelib.deepLines.restype = (c_char_p)

    CommentCb = CFUNCTYPE(None, POINTER(c_char))
    treelib.getMove.argtypes = (board, POINTER(c_int), c_char_p)
    treelib.getMove.restype = (board)

    sqlite3 = CDLL("libsqlite3.so")
    sqlite3.sqlite3_open.argtypes = [c_char_p, POINTER(c_void_p)]
    sqlite3.sqlite3_open.restype = c_int
    moveType = c_int * 4


    b = treelib.createBoard()
    db = c_void_p()
    sqlite3.sqlite3_open(b"optree.db", byref(db))

    root = tk.Tk()
    root.title("Chess Opening Explorer")
    root.geometry(f"{rw}x{rh}")
    root.resizable(width=True, height=True)


    # Make Board
    pieces_id: dict[tuple[int,int], tuple[str, int]] = {}
    imgs = []
    bw=rh
    bh=rh
    board_canvas = tk.Canvas(root, name="board", width=bw, height=bh, bg="black")
    board_canvas.pack(side="left", fill="y")


    def draw_piece(bo, i, j):
        piece = chr(bo.state[8*j+i])
        if piece != ' ':
            img = Image.open(f"openings/static/pieces/{piece}.png")
            img = img.convert("RGBA")
            img = img.resize((bw//8, bh//8), Image.Resampling.LANCZOS)
            img = ImageTk.PhotoImage(img)
            imgs.append(img)
            pieces_id.update({(i,j): (img , board_canvas.create_image(i*bw/8, j*bh/8, image=img, anchor="nw"))})

    def draw_board(bo):
        for piece in pieces_id.values():
            board_canvas.delete(piece[1])
        for i in range(8):
            for j in range(8):
                if not (i+j)%2:
                    board_canvas.create_rectangle(i*bw/8, j*bw/8, (i+1)*bw/8, (j+1)*bh/8, fill="#f3d69e", outline="black")
                else:
                    board_canvas.create_rectangle(i*bw/8, j*bw/8, (i+1)*bw/8, (j+1)*bh/8, fill="#af481b", outline="black")

                draw_piece(bo, i, j)
    draw_board(b)


    # Make Line Visualizer
    lines_canvas = tk.Canvas(root, name="lines", width=rw-bw, height=rh)
    lines_canvas.pack(side="right", fill="y")

    lines_text = tk.Text(lines_canvas, width=(rw-bw), height=rh//(3*5*16), bg="grey60")
    lines_text.tag_config("linename", font=("Arial", 20, "bold"))
    lines_text.tag_config("firstmove", font=("Arial", 16))
    lines_text.tag_config("move", font=("Arial", 10))
    lines_text.pack(fill="both", expand=True)

    # Make Comment Box
    comment_canvas = tk.Canvas(lines_canvas, name="comment", width=rw-bw, height=rh/4, bg="black")
    comment_canvas.pack(side="bottom", fill="y")

    comment_box = tk.Text(comment_canvas, name="comment_box", width=rw-bw, height=rh/5, fg="black", bg="grey80", font=("Arial", 14))
    comment_label = tk.Label(comment_canvas, name='comment_label', width=(rw-bw)//16*3//4, text="Comment:", fg="Purple", bg="black", font=("Arial", 18, "bold"), anchor="w")
    comment_canvas.create_window(0, 0, height=rh/20, window=comment_label, anchor="nw")
    comment_canvas.create_window(0, rh/20, height=rh/5, window=comment_box, anchor="nw")

    # Make Move Logic
    def commentCallback(buf):
        text = comment_box.get('1.0', 'end-1c').encode()
        for i,b in enumerate(text):
            buf[i] = b
        comment_box.delete('1.0', 'end')

    def promotionCallback(buf):
        piece = simpledialog.askstring("Promotion", "Q/R/N/B:", parent = root)
        if piece is None:
            piece = ""
        else:
            buf.contents.value = piece[0].encode()


    def getLines(max_depth):
        lines: str = treelib.deepLines(b, max_depth, 0, db).decode()
        lines = lines.rpartition('\n')[0]
        ulines = []
        for line in lines.split('\n'):
            if not line in ulines:
                ulines.append(line+'\n')
        for line in ulines:
            if line.startswith(" "):
                tag = "move"
            elif line.startswith("*"):
                tag = "firstmove"
            else: tag = "linename"
            lines_text.insert("end", line, tag)
    getLines(max_lines)


    line = simpledialog.askstring("Line", "What's the name of the Line: ", parent = root)
    while line is None:
        line = simpledialog.askstring("Line", "What's the name of the Line: ", parent = root)
    line = line.encode()

    def on_click(event):
        global selected_piece, b, curr_line
        col = event.x//(bw//8)
        row = event.y//(bh//8)
        piece = chr(b.state[8*row+col])
        if not selected_piece:
            if piece != ' ':
                selected_piece = [row, col]
        else:
            new_pos = [row,col]
            selected_piece.extend([row, col])
            n = treelib.getMove(b, moveType(*selected_piece), line)
            if n.state != b.state:
                selected_piece = []
                b=n
                lines_text.delete("1.0", "end")
                getLines(max_lines)
                draw_board(n)

            else:
                print("Failed to move")
                selected_piece.pop()
                selected_piece.pop()
                selected_piece.pop()
                selected_piece.pop()

        return (col, row)

    board_canvas.bind("<Button-1>", on_click)

    root.mainloop()
