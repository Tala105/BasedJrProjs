from django.forms import ModelForm
from .models import Move

class MoveForm(ModelForm):
    class Meta:
        model = Move
        fields = ['line', 'variation', 'comment', 'move']

    def __init__(self, *args, line=None, **kwargs):
        super().__init__(*args, **kwargs)
        self.fields['line'].required = False
        self.fields['variation'].required = False
        self.fields['comment'].required = False

        if line is not None:
            self.fields.pop('line')
            self.instance.line = line
