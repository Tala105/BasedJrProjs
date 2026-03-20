import curses
import sqlite3
import sys
import os
from ctypes import *

# ── Board / C lib setup (mirrors chesslogic.py) ────────────────────────────

class board(Structure):
    _fields_ = [("state", c_char * 67)]

lib_path = os.path.join(os.path.dirname(__file__), 'utils/treelogic.so')
treelib = CDLL(lib_path)

treelib.createBoard.restype = board

treelib.makeMove.argtypes = (board, POINTER(c_int))
treelib.makeMove.restype = board

treelib.nameToMove.argtypes = (board, c_char_p, POINTER(c_int))

treelib.deepLines.argtypes = (board, c_int, c_int, c_void_p)
treelib.deepLines.restype = c_char_p

# ── Helpers ────────────────────────────────────────────────────────────────

RESET  = "\033[0m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
WHITE  = "\033[1;37m"
DARK   = "\033[1;90m"
CYAN   = "\033[1;36m"
YELLOW = "\033[1;33m"
GREEN  = "\033[1;32m"
RED    = "\033[1;31m"

PIECE_SYMBOLS = {
    'p': '♙', 'P': '♟',
    'n': '♘', 'N': '♞',
    'b': '♗', 'B': '♝',
    'r': '♖', 'R': '♜', 's': '♖', 'S': '♜',
    'q': '♕', 'Q': '♛',
    'k': '♔', 'K': '♚', 'j': '♔', 'J': '♚',
    ' ': ' ',
}

LIGHT_SQ = "\033[48;5;180m"   # warm tan
DARK_SQ  = "\033[48;5;94m"    # brown

def print_board(b: board):
    turn = b.state[64] - ord('0')
    print()
    rows = range(8) if not turn else range(7, -1, -1)
    cols = range(8) if not turn else range(7, -1, -1)

    rank_labels = list("87654321") if not turn else list("12345678")
    file_labels = "  a b c d e f g h" if not turn else "  h g f e d c b a"

    for ri, row in enumerate(rows):
        print(f" {BOLD}{rank_labels[ri]}{RESET} ", end="")
        for col in cols:
            piece_char = chr(b.state[8 * row + col])
            symbol = PIECE_SYMBOLS.get(piece_char, piece_char)
            bg = LIGHT_SQ if (row + col) % 2 == 0 else DARK_SQ
            color = WHITE if piece_char.isupper() else DARK
            print(f"{bg}{color}{symbol} {RESET}", end="")
        print()
    print(f"{CYAN}{file_labels}{RESET}")
    side = "White" if not turn else "Black"
    print(f"  {DIM}Turn: {side}{RESET}\n")

def parse_move(b: board, text: str):
    """
    Accepts:
      - Algebraic:  e4 / Nf3 / O-O / O-O-O / exd5 / e8=Q
      - Coordinate: e2e4  or  e2 e4
    Returns a list of 5 ints [yi, xi, yf, xf, promo] or None on failure.
    """
    text = text.strip()
    move = (c_int * 5)(*[-1] * 5)

    # coordinate input: "e2 e4" or "e2e4"
    t = text.replace(" ", "")
    if len(t) == 4 and t[0].isalpha() and t[1].isdigit() and t[2].isalpha() and t[3].isdigit():
        xi = ord(t[0]) - ord('a')
        yi = 8 - int(t[1])
        xf = ord(t[2]) - ord('a')
        yf = 8 - int(t[3])
        return [yi, xi, yf, xf, -1]

    # algebraic via C's nameToMove
    treelib.nameToMove(b, text.encode(), move)
    result = list(move)
    if result[0] != -1:
        return result
    return None

def make_move(b: board, move: list) -> board:
    arr = (c_int * 5)(*move)
    return treelib.makeMove(b, arr)

def get_lines(b: board, db_ptr, max_depth=4) -> str:
    raw = treelib.deepLines(b, max_depth, 0, db_ptr)
    if raw:
        return raw.decode(errors="replace").rstrip('\n')
    return ""

# ── Opening DB helpers ─────────────────────────────────────────────────────

ADD_LINK = """
INSERT INTO Lines (Line, Parent, Child, Comment, Move)
VALUES(?, ?, ?, ?, ?)
ON CONFLICT(Parent, Child) DO UPDATE SET
  Line = CASE
    WHEN instr(Lines.Line, excluded.Line) = 0
    THEN Lines.Line || ' | ' || excluded.Line
    ELSE Lines.Line END,
  Comment = CASE
    WHEN Lines.Comment = '' THEN excluded.Comment
    WHEN excluded.Comment = '' THEN Lines.Comment
    WHEN instr(Lines.Comment, excluded.Comment) = 0
    THEN Lines.Comment || ' | ' || excluded.Comment
    ELSE Lines.Comment END;
"""

def save_move(conn: sqlite3.Connection, line: str, parent: board, child: board, movename: str, comment: str = ""):
    conn.execute(ADD_LINK, (line, parent.state.decode(), child.state.decode(), comment, movename))
    conn.commit()

def move_name(b: board, move: list) -> str:
    """Ask the C lib for the algebraic name of a move."""
    buf = create_string_buffer(16)
    fn = treelib.moveName
    fn.argtypes = (board, POINTER(c_int), c_char_p)
    fn.restype = None
    arr = (c_int * 5)(*move)
    fn(b, arr, buf)
    return buf.value.decode()

# ── Bot stub ───────────────────────────────────────────────────────────────

def bot_move(b: board):
    """
    WIP: returns a move list [yi, xi, yf, xf, promo] from the Evaluator.
    Swap this out once Evaluator.play is ready.
    """
    try:
        from Evaluator.play import get_move  # expected interface
        return get_move(b.state.decode())
    except ImportError:
        print(f"  {RED}[Bot not available yet — WIP]{RESET}")
        return None

# ── Modes ──────────────────────────────────────────────────────────────────

def mode_free_play():
    b = treelib.createBoard()
    print(f"\n{GREEN}── Free Play ──{RESET}  (type 'quit' to exit)\n")
    while True:
        print_board(b)
        try:
            text = input(f"{CYAN}Move: {RESET}").strip()
        except (EOFError, KeyboardInterrupt):
            break
        if text.lower() in ('quit', 'q', 'exit'):
            break
        move = parse_move(b, text)
        if move is None:
            print(f"  {RED}Invalid move.{RESET}\n")
            continue
        new_b = make_move(b, move)
        if new_b.state == b.state:
            print(f"  {RED}Illegal move.{RESET}\n")
            continue
        b = new_b

def mode_opening_explorer():
    db_path = os.path.join(os.path.dirname(__file__), 'utils/optree.db')
    conn = sqlite3.connect(db_path)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS Lines (
            Line TEXT, Parent TEXT, Child TEXT,
            Comment TEXT DEFAULT '', Move TEXT,
            UNIQUE(Parent, Child)
        )""")
    conn.commit()

    # sqlite3 handle for the C lib's deepLines
    sqlite3_lib = CDLL("libsqlite3.so")
    sqlite3_lib.sqlite3_open.argtypes = [c_char_p, POINTER(c_void_p)]
    sqlite3_lib.sqlite3_open.restype = c_int
    db_ptr = c_void_p()
    sqlite3_lib.sqlite3_open(db_path.encode(), byref(db_ptr))

    line_name = input(f"{CYAN}Opening line name: {RESET}").strip()
    if not line_name:
        line_name = "Unnamed"

    b = treelib.createBoard()
    print(f"\n{GREEN}── Opening Explorer: {line_name} ──{RESET}  (type 'quit' to exit, 'comment' to annotate)\n")

    while True:
        print_board(b)
        lines_str = get_lines(b, db_ptr)
        if lines_str:
            print(f"{YELLOW}── Known continuations ──{RESET}")
            print(lines_str)
            print()

        try:
            text = input(f"{CYAN}Move: {RESET}").strip()
        except (EOFError, KeyboardInterrupt):
            break
        if text.lower() in ('quit', 'q', 'exit'):
            break

        comment = ""
        if text.lower() == 'comment':
            comment = input("  Comment: ").strip()
            text = input(f"{CYAN}Move: {RESET}").strip()

        move = parse_move(b, text)
        if move is None:
            print(f"  {RED}Invalid move.{RESET}\n")
            continue
        new_b = make_move(b, move)
        if new_b.state == b.state:
            print(f"  {RED}Illegal move.{RESET}\n")
            continue

        name = move_name(b, move)
        save_move(conn, line_name, b, new_b, name, comment)
        b = new_b

    sqlite3_lib.sqlite3_close(db_ptr)
    conn.close()

def mode_vs_bot():
    b = treelib.createBoard()
    player_turn = 0  # 0 = white (player), 1 = black (bot) — swap if you want
    print(f"\n{GREEN}── vs Bot (WIP) ──{RESET}  (type 'quit' to exit)\n")
    print(f"  {DIM}You play as {'White' if not player_turn else 'Black'}.{RESET}\n")

    while True:
        print_board(b)
        turn = b.state[64] - ord('0')

        if turn == player_turn:
            try:
                text = input(f"{CYAN}Your move: {RESET}").strip()
            except (EOFError, KeyboardInterrupt):
                break
            if text.lower() in ('quit', 'q', 'exit'):
                break
            move = parse_move(b, text)
            if move is None:
                print(f"  {RED}Invalid move.{RESET}\n")
                continue
        else:
            print(f"  {DIM}Bot thinking...{RESET}")
            move = bot_move(b)
            if move is None:
                try:
                    text = input(f"{CYAN}(Bot unavailable) Enter move manually: {RESET}").strip()
                except (EOFError, KeyboardInterrupt):
                    break
                move = parse_move(b, text)
                if move is None:
                    print(f"  {RED}Invalid move.{RESET}\n")
                    continue

        new_b = make_move(b, move)
        if new_b.state == b.state:
            print(f"  {RED}Illegal move.{RESET}\n")
            continue
        b = new_b

# ── Curses mouse-clickable menu ────────────────────────────────────────────

MENU_ITEMS = [
    ("♟  Free Play",        mode_free_play),
    ("♜  Opening Explorer", mode_opening_explorer),
    ("🤖  vs Bot (WIP)",     mode_vs_bot),
    ("✕   Quit",            None),
]

def draw_menu(stdscr, highlighted: int):
    stdscr.clear()
    h, w = stdscr.getmaxyx()

    # Title
    title   = "  Chess CLI  "
    sub     = "click an option to start"
    title_y = max(0, h // 2 - len(MENU_ITEMS) - 3)
    title_x = max(0, (w - len(title)) // 2)
    stdscr.attron(curses.color_pair(3) | curses.A_BOLD)
    stdscr.addstr(title_y, title_x, title)
    stdscr.attroff(curses.color_pair(3) | curses.A_BOLD)
    stdscr.attron(curses.color_pair(4))
    stdscr.addstr(title_y + 1, max(0, (w - len(sub)) // 2), sub)
    stdscr.attroff(curses.color_pair(4))

    # Divider
    div_y = title_y + 2
    stdscr.attron(curses.color_pair(4))
    stdscr.addstr(div_y, max(0, (w - 20) // 2), "─" * 20)
    stdscr.attroff(curses.color_pair(4))

    # Items — record the row each item is drawn on for hit-testing
    item_rows = []
    for i, (label, _) in enumerate(MENU_ITEMS):
        row = div_y + 2 + i * 2
        col = max(0, (w - len(label) - 4) // 2)
        item_rows.append((row, col, len(label) + 4))

        is_quit = (i == len(MENU_ITEMS) - 1)
        is_wip  = "WIP" in label

        if i == highlighted:
            stdscr.attron(curses.color_pair(2) | curses.A_BOLD | curses.A_REVERSE)
            stdscr.addstr(row, col, f"  {label}  ")
            stdscr.attroff(curses.color_pair(2) | curses.A_BOLD | curses.A_REVERSE)
        elif is_quit:
            stdscr.attron(curses.color_pair(5))
            stdscr.addstr(row, col, f"  {label}  ")
            stdscr.attroff(curses.color_pair(5))
        elif is_wip:
            stdscr.attron(curses.color_pair(4))
            stdscr.addstr(row, col, f"  {label}  ")
            stdscr.attroff(curses.color_pair(4))
        else:
            stdscr.attron(curses.color_pair(1))
            stdscr.addstr(row, col, f"  {label}  ")
            stdscr.attroff(curses.color_pair(1))

    # Footer hint
    hint = "↑↓ navigate   Enter/click select   q quit"
    stdscr.attron(curses.color_pair(4))
    try:
        stdscr.addstr(h - 1, max(0, (w - len(hint)) // 2), hint)
    except curses.error:
        pass
    stdscr.attroff(curses.color_pair(4))

    stdscr.refresh()
    return item_rows


def curses_menu(stdscr):
    curses.curs_set(0)
    curses.mousemask(curses.ALL_MOUSE_EVENTS | curses.REPORT_MOUSE_POSITION)

    curses.start_color()
    curses.use_default_colors()
    # pair 1 = normal item (cyan on default)
    curses.init_pair(1, curses.COLOR_CYAN,    -1)
    # pair 2 = highlighted (bold reverse)
    curses.init_pair(2, curses.COLOR_WHITE,   curses.COLOR_BLUE)
    # pair 3 = title (yellow)
    curses.init_pair(3, curses.COLOR_YELLOW,  -1)
    # pair 4 = dim/subtitle (white dimmed)
    curses.init_pair(4, curses.COLOR_WHITE,   -1)
    # pair 5 = quit (red)
    curses.init_pair(5, curses.COLOR_RED,     -1)

    highlighted = 0
    selected    = None

    while selected is None:
        item_rows = draw_menu(stdscr, highlighted)

        key = stdscr.getch()

        if key == curses.KEY_UP:
            highlighted = (highlighted - 1) % len(MENU_ITEMS)

        elif key == curses.KEY_DOWN:
            highlighted = (highlighted + 1) % len(MENU_ITEMS)

        elif key in (curses.KEY_ENTER, ord('\n'), ord('\r'), ord(' ')):
            selected = highlighted

        elif key == ord('q') or key == ord('Q'):
            selected = len(MENU_ITEMS) - 1   # Quit item

        elif key == curses.KEY_MOUSE:
            try:
                _, mx, my, _, bstate = curses.getmouse()
                if bstate & (curses.BUTTON1_CLICKED | curses.BUTTON1_DOUBLE_CLICKED):
                    for i, (row, col, width) in enumerate(item_rows):
                        if my == row and col <= mx < col + width:
                            highlighted = i
                            selected = i
                            break
                    else:
                        # hovering — just update highlight
                        for i, (row, col, width) in enumerate(item_rows):
                            if my == row and col <= mx < col + width:
                                highlighted = i
            except curses.error:
                pass

        elif key == curses.KEY_MOUSE:
            # hover without click — update highlight only
            try:
                _, mx, my, _, _ = curses.getmouse()
                for i, (row, col, width) in enumerate(item_rows):
                    if my == row and col <= mx < col + width:
                        highlighted = i
            except curses.error:
                pass

    return selected


def main():
    while True:
        choice = curses.wrapper(curses_menu)
        _, fn = MENU_ITEMS[choice]
        if fn is None:
            sys.exit(0)
        # Run the mode outside curses so input/print work normally
        fn()


if __name__ == "__main__":
    main()
