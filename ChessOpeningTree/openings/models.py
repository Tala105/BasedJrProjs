from django.db import models
from utils.chesslogic import make_move, nameToMove
import uuid

class Position(models.Model):
    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    board = models.CharField(max_length=72)

class Move(models.Model):
    updated = models.DateTimeField(auto_now=True)
    line = models.CharField(max_length=16)
    variation = models.CharField(max_length=32)
    parent = models.ForeignKey(Position, on_delete=models.CASCADE, related_name='parent_position')
    child = models.ForeignKey(Position, on_delete=models.CASCADE, related_name='child_position')
    comment = models.CharField(max_length=250)
    move = models.CharField(max_length=16)

    def save(self, *args, **kwargs):
        move = nameToMove(self.move)
        self.child = make_move(self.parent, move)
        super().save(*args, **kwargs)

# Create your models here.
