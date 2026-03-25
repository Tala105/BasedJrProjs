import os
from random import shuffle
from hand import Hand
from card import Card
from time import sleep
from constants import SUITS, RANKS, HAND_VALUES

class Table:
    def __init__(self, num_players: int, base_chips: int = 1000):
        self.num_players = num_players
        self.base_chips = base_chips
        self.blind: int = 10
        self.reset()

    def reset(self) -> None:
        self.round = 1
        self.active_player = 0
        self.last_better = 0
        self.min_bet = self.blind
        self.pots = [0]
        self.deck = self.create_deck()
        self.players = self.create_players()
        self.community_cards = self.create_community_cards()

    def create_deck(self) -> list[Card]:
        """Creates a deck of cards."""
        sorted_deck = [Card(suit, rank) for suit in SUITS for rank in RANKS]
        shuffle(sorted_deck)
        return sorted_deck
    
    def create_players(self) -> list[Hand]:
        """Creates players with hands."""
        players = []
        for i in range(self.num_players):
            hand_cards = [self.deck.pop(), self.deck.pop()]
            players.append(Hand(hand_cards, i, f"Player{i}", self.base_chips))
        return players
    
    def create_community_cards(self) -> Hand:
        """Creates community cards."""
        community_cards = []
        for _ in range(5):
            community_cards.append(self.deck.pop())
        return Hand(community_cards, -1, "Community Cards", 0)


    def winning_hand(self) -> None:
        """Determines the winning hand between all players."""
        eligible_players = [player for player in self.players if player.status != "folded"]
        for player in eligible_players:
            player.cards = player.cards + self.community_cards.cards
            player.evaluate_hand()
        ordered_hands = sorted(eligible_players, key=lambda x: x.hand_priority, reverse=True)
        winner = ordered_hands[0]
        print(f"{winner.name} wins with {winner.hand_value()}!")
        print(f"Winning hand: \n{winner.make_visual()}")
        restart: str = "nop"
        while restart != "":
            restart = input("Press Enter to start Next Game, or exit to stop: ").strip().lower()
            if any(c in "exit" for c in restart):
                exit()
        self.reset()


    def fold(self) -> None:
        """Handles the player's fold action."""
        player = self.players[self.active_player]
        print(f"{player.name} folds. Remaining chips: {player.chips}")
        player.status = "folded"
        
    def all_in(self) -> None:
        """Handles the player's all-in action."""
        player = self.players[self.active_player]
        if player.chips > 0:
            player.betted += player.chips
            self.pots[-1] += player.chips
            self.min_bet = max(self.min_bet, player.chips)
            print(f"{player.name} goes all-in with {player.chips} chips, Minimum bet is now {self.min_bet}.")
            player.chips = 0
            player.status = "all-in"
        else:
            print(f"{player.name} is already all-in.")
        
    def call(self) -> None:
        """Handles the player's call action."""
        player = self.players[self.active_player]
        bet = self.min_bet - player.betted
        if player.chips == bet:
            self.all_in()
            return
        if player.chips > bet:
            player.chips -= bet
            player.betted += bet
            self.pots[-1] += bet
            print(f"{player.name} calls the bet of {self.min_bet}. Remaining chips: {player.chips}")
        else:
            print(f"{player.name} does not have enough chips to call. Remaining chips: {player.chips}")
            input("Do you want to go all-in? (y/n): ")
            if input().lower() == 'y':
                self.all_in()
            else:
                self.fold()
    
    def raise_bet(self) -> None:
        """Handles the player's raise action."""
        player = self.players[self.active_player]
        while True:
            try:
                amount = int(input("Enter the amount to raise: "))
            except ValueError:
                print("Incorrect input type, input a integer amount")
            else:
                if amount == player.chips:
                    self.all_in()
                    return
                if amount > self.min_bet and amount <= player.chips:
                    player.chips -= amount
                    player.betted += amount
                    self.pots[-1] += amount
                    self.min_bet = amount
                    print(f"{player.name} raises the bet by {amount}. Remaining chips: {player.chips}")
                    return
                else:
                    print(f"Invalid amount. You must raise more than {self.min_bet} and less than or equal to your chips ({player.chips}).")
        

    def next_round(self, gone_all_in: bool) -> None:
        """Handles the next round of betting."""
        if gone_all_in:
            self.pots.append(0)
        for player in self.players:
            player.betted = 0
        self.min_bet = self.blind
        self.active_player = 0
        self.last_better = 0
        self.round += 1
        print(f"Starting round {self.round}.")
        sleep(1)

    def betting_round(self):
        """Simulates a betting round."""
        in_game_players = sum(x.status != "folded" and x.status != "all-in" for x in self.players)
        if self.round == 5:
            print("All rounds completed. Evaluating hands...")
            self.winning_hand()
            print("Beginning a new game...")
            return

        self.min_bet = self.blind
        gone_all_in = False

        while True:
            player = self.players[self.active_player]
            os.system('cls' if os.name == 'nt' else 'clear')
            self.show_community_card()
            print(f"Round {self.round}: {player.name}'s turn to bet. Chips: {player.chips}")
            print(player.make_visual())
            print(f"Current pot: {self.pots}, Minimum bet: {self.min_bet}")

            if player.status == "folded" or player.status == "all-in":
                print(f"{player.name}'s turn is skipped.")
                sleep(1)
            else:
                while True:
                    if in_game_players == 1:
                        break
                    action = input("Choose action (call, raise, all_in, fold): ").strip().lower()
                    if action == "call":
                        self.call()
                        break
                    elif action == "raise":
                        self.raise_bet()
                        self.last_better = self.active_player
                        break
                    elif action == "all_in" and in_game_players > 1:
                        self.all_in()
                        if player.chips == 0:
                            in_game_players -= 1
                            gone_all_in = True
                            self.last_better = self.active_player
                        break
                    elif action == "fold" and in_game_players > 1:
                        in_game_players -= 1
                        self.fold()
                        break
                    else:
                        print("Invalid action. Please try again.")

            self.active_player = (self.active_player + 1) % self.num_players
            if self.active_player == self.last_better:
                break

        self.next_round(gone_all_in)

    def show_community_card(self) -> None:
        """Returns the community cards of the current round."""
        cards_per_round = {1:0, 2:3, 3:4, 4:5}
        card_on_table = cards_per_round[self.round]
        print(self.community_cards.make_visual(num_cards=card_on_table))
    

if __name__ == "__main__":
    table = Table(4)
    table.betting_round()
    table.betting_round()
    table.betting_round()
    table.betting_round()
    table.betting_round()
