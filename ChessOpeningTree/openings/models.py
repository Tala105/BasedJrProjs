from django.db import models
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

    class Meta:
        constraints = [
            models.UniqueConstraint(fields=['line', 'parent', 'child'], name='unique_line_parent_child')
        ]
# Create your models here.
