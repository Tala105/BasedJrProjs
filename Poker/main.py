from table import Table
import os

def get_people_amount():
    while True:
        try:
            player_count = int(input("How many Players:"))
        except ValueError:
            print("Invalid Player Amount input, try again.")
        else:
            os.system("clear")
            break

    return player_count

def main():
    table = Table(get_people_amount())
    while True:
        table.betting_round()   

if __name__ == "__main__":
    main()
