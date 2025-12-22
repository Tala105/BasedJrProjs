from django.shortcuts import redirect, render
from django.http import HttpResponse
from .models import Move, Position
from .forms import MoveForm
from uuid import UUID
from utils.chesslogic import make_move, nameToMove

id0 = "00000000-0000-0000-0000-000000000000"
starting_pos = Position.objects.get(id=id0)

def home(request):
    moves = {}
    for move in Move.objects.all().filter(parent=starting_pos):
        moves.update({Position.objects.get(board=move.child.board).id: move.line})
    context = {"moves": moves}
    return render(request, 'openings/home.html', context)

def starting_position(request):
    return position(request, id0)

def position(request, pk):
    line = request.GET.get("line", "")
    pos = Position.objects.get(id=pk).board
    board = []
    for i in range(64):
        board.append(("black" if ((i+i//8)%2) else "white", pos[i]))
    if line:
        form = MoveForm(line=line)
    else:
        form = MoveForm()
    if request.method == 'POST':
        form = MoveForm(request.POST)
        if form.is_valid():
            move = form.save(commit=False)
            movename = nameToMove(pos, move.move)
            print(f"Move: {movename}")
            child_board = make_move(pos, movename)
            move.parent=Position.objects.get(board=pos)
            if pos!=child_board:
                print("Saving Move")
                move.child=Position.objects.get_or_create(board=child_board)[0]
                print(f"child id{move.child.id}")
                move.save()
                return redirect(f'./{Position.objects.get(board=move.child.board).id}?line={move.line}')


    context = {'board': [(color, piece.swapcase()) for color, piece in board],
               "Children": Move.objects.filter(parent=Position(id=pk)),
               "form": form}
    return render(request, 'openings/position.html', context)
