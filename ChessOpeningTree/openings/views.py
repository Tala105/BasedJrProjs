from django.shortcuts import redirect, render
from django.http import HttpResponse
from .models import Move, Position
from .forms import MoveForm
from uuid import UUID

def home(request):
    moves = {move.id: move.line for move in Move.objects.all()}
    context = {"moves": moves}
    return render(request, 'openings/home.html', context)

def starting_position(request):
    return position(request, "00000000-0000-0000-0000-000000000000")

def position(request, pk):
    pos = Position.objects.get(id=pk).board
    board = []
    for i in range(64):
        board.append(("black" if ((i+i//8)%2) else "white", pos[i]))
    form = MoveForm()
    if request.method == 'POST':
        print("Saving Move")
        form = MoveForm(request.POST)
        if form.is_valid:
            move = form.save(commit=False)
            move.parent=Position(board=pos)
            move.save()
            return redirect(f'position/{Position.objects.get(board=move.child).id}')


    context = {'board': board, "Children": Move.objects.filter(parent=Position(id=pk)), "form": form}
    return render(request, 'openings/position.html', context)
