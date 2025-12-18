from django.urls import path
from . import views

urlpatterns = [
    path("", views.home, name="home"),
    path('position/<str:pk>', views.position, name="position"),
    path('position/00000000-0000-0000-0000-000000000000', views.starting_position, name="first_move"),
]
