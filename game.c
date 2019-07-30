#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "property.h"

typedef struct PlayerStruct {
  //List of names of properties owned by a player. Properties are in ascending order.
  Property *propertiesOwned;
  int playerNum;
  int moneyHeld;
  int location;
  int propsOwned;
  bool inGame;
} Player;

void runGame(int startingCash,
             int numPlayers,
             int turnLimit,
             int propMult,
             int goMult,
             Property **propsPtr,
             int numProps,
             int endPlayers,
             FILE *randsFile);
void printBoardState(Property **propsPtr, int numProps, Player **playersPtr, int numPlayers);
void extractRules(int numPlayers, Property **propsPtr, int numProps, FILE *rulesFile, FILE *randsFile);
int rollDice(FILE *randomNumFile);
void inspectPlayer(int playerNum, Property **propsPtr, int numProps, Player **playersPtr, int numPlayers);
void generatePlayers(Player** players, int numPlayers, int startingCash, int numProps);
bool checkForWins(int startPlayers, Player** players, Property** propsPtr, int numProps);

int main(int argc, char **argv) {
  printf("How many players will be playing: ");
  int numPlayers;
  scanf("%d", &numPlayers);
  Property *props;
  FILE *boardFile = fopen(argv[2], "r");
  FILE *ruleFile = fopen(argv[1], "r");
  FILE *randsFile = fopen(argv[3], "r");
  int numProps = createBoard(boardFile, &props);
  fclose(boardFile);
  //printBoardState(&props, numProps);
  extractRules(numPlayers, &props, numProps, ruleFile, randsFile);
  fclose(ruleFile);
  fclose(randsFile);
  free(props);
}

void printBoardState(Property **propsPtr, int numProps, Player **playersPtr, int numPlayers) {
  //printf("%d\n", numPlayers);
  int longNameLength = 0;
  for (int k = 0; k < numProps; k++) {
    if (strlen((*propsPtr)[k].name) > longNameLength) {
      longNameLength = strlen((*propsPtr)[k].name);
    }
  }
  for (int i = 0; i < numProps; i++) {
    printf("%d | %s", i, (*propsPtr)[i].name);
    for (int l = 0; l < longNameLength - strlen((*propsPtr)[i].name); l++) {
      printf(" ");
    }
    printf(" |     ");
    for (int j = 0; j < numPlayers; j++) {
      if ((*playersPtr)[j].location == i) {
        if ((*playersPtr)[j].inGame == true) {
          printf("%d ", j);
        }
      }
    }
    printf("\n");
  }
}

void extractRules(int numPlayers, Property **propsPtr, int numProps, FILE *rulesFile, FILE *randsFile) {
  //These lines read the rules into their corresponding variables, and then puts them into the game through runGame
  int startingCash;
  int turnLimit;
  int endPlayers;
  int propMult;
  int goMult;
  char c[999];
  fgets(c, 999, rulesFile);
  char *startCash = c + 15;
  startingCash = atoi(startCash);
  fgets(c, 999, rulesFile);
  char *turnLim = c + 35;
  turnLimit = atoi(turnLim);
  fgets(c, 999, rulesFile);
  char *endP = c + 36;
  endPlayers = atoi(endP);
  fgets(c, 999, rulesFile);
  char *propM = c + 25;
  propMult = atoi(propM);
  fgets(c, 999, rulesFile);
  fgets(c, 999, rulesFile);
  fgets(c, 999, rulesFile);
  fgets(c, 999, rulesFile);
  fgets(c, 999, rulesFile);
  fgets(c, 999, rulesFile);
  char *goM = c + 37;
  goMult = atoi(goM);
  runGame(startingCash, numPlayers, turnLimit, propMult, goMult, propsPtr, numProps, endPlayers, randsFile);
}

bool checkForWins(int startPlayers, Player** players, Property** propsPtr, int numProps){
  //printf("HERE\n");
  int maxNetWorth = 0;
  int winDetermine[startPlayers];
  for (int u = 0; u < startPlayers; u++) {
    winDetermine[u] = 0;
  }
  for (int g = 0; g < startPlayers; g++) {
    if ((*players)[g].inGame == true) {
      int gWorth = (*players)[g].moneyHeld;
      for (int t = 0; t < numProps; t++) {
        if (strcmp((*players)[g].propertiesOwned[t].name, "NULL") != 0) {
          gWorth += (*propsPtr)[t].price;
        }
      }
      //printf("%d\n", gWorth);
      if (gWorth > maxNetWorth) {
        maxNetWorth = gWorth;
        for (int v = 0; v < startPlayers; v++) {
          if (v == g) {
            winDetermine[v] = 1;
            //printf("%d\n", v);
          } else {
            winDetermine[v] = 0;
          }
        }
      } else if (gWorth == maxNetWorth) {
        for (int a = 0; a < startPlayers; a++) {
          if (a == g) {
            winDetermine[a] = 1;
          }
        }
      }
    }
    //Free players
    //printf("%d\n", winDetermine[0]);
    if (g == startPlayers - 1) {
      for (int b = 0; b < startPlayers; b++) {
        if (winDetermine[b] == 1) {
          printf("Player %d\n", b);
        }
      }
      for (int h = 0; h < startPlayers; h++) {
        free((*players)[h].propertiesOwned);
      }
      free((*players));
      //players = NULL;
      //Free board
      for (int q = 0; q < numProps; q++) {
        free((*propsPtr)[q].name);
      }
      //printf("here\n");
      return true;
    }
  }
  return false;
}

void runGame(int startingCash,
             int numPlayers,
             int turnLimit,
             int propMult,
             int goMult,
             Property **propsPtr,
             int numProps,
             int endPlayers,
             FILE *randsFile) {
  int turn = 0;
  int goValue = (*propsPtr)[0].goPayout;
  Player* players;
  generatePlayers(&players, numPlayers, startingCash, numProps);
  int startPlayers = numPlayers;
  //K is the number of turns that have passed
  int k = 0;
  //int tempK = 0;
  bool rolledDice = false;
  if (turnLimit == 0) {
    printBoardState(propsPtr, numProps, &players, startPlayers);
    printf("The winners are\n");
    for (int e = 0; e < startPlayers; e++) {
      printf("Player %d\n", e);
    }
    for (int h = 0; h < startPlayers; h++) {
      free(players[h].propertiesOwned);
    }
    free(players);
    //players = NULL;
    //Free board
    for (int q = 0; q < numProps; q++) {
      free((*propsPtr)[q].name);
    }
    return;
  }
  else if (turnLimit == -1){
    k = -1000000;
  }
  while (k < turnLimit && numPlayers > endPlayers) {
    printBoardState(propsPtr, numProps, &players, startPlayers);
    if (rolledDice) {
      printf("Player %d please choose an action\n1: End Turn\n2: Inspect Player\n3: Leave Game\nYour action: ", turn);
    } else {
      printf("Player %d please choose an action\n1: Roll Dice\n2: Inspect Player\n3: Leave Game\nYour action: ", turn);
    }
    int playerChoice;
    scanf("%d", &playerChoice);
    if (playerChoice == 1 && rolledDice) {
      rolledDice = false;
      turn++;
      if (turn > startPlayers - 1) {
        turn = 0;
      }
      while (players[turn].inGame == false) {
        turn++;
      }
      k++;
      if (k == turnLimit) {
        printBoardState(propsPtr, numProps, &players, startPlayers);
        printf("The winners are\n");
        if(checkForWins(startPlayers, &players, propsPtr, numProps)){
          return;
        }
      }
    }
      //Handle a die Roll
    else if (playerChoice == 1) {
      rolledDice = true;
      int dieRoll = rollDice(randsFile);
      printf("You rolled a %d!\n", dieRoll);
      players[turn].location += dieRoll;
      int fullLocation = players[turn].location;
      players[turn].location = (players[turn].location) % numProps;
      if (fullLocation > numProps) {
        //We passed go at least once
        int numGos = fullLocation / numProps;
        if (players[turn].location == 0) {
          players[turn].moneyHeld += (numGos - 1) * goValue;
        } else {
          players[turn].moneyHeld += numGos * goValue;
        }
      }
      //Land on go, give bonus
      if (players[turn].location == 0) {
        players[turn].moneyHeld += goMult * goValue;
      }
        //Land on property
      else {
        int propIndex = players[turn].location;
        //Property For sale
        if ((*propsPtr)[propIndex].owned == false) {
          if ((*propsPtr)[propIndex].price > players[turn].moneyHeld) {
            printf("Player %d you do not have enough money to purchase %s\n", turn, (*propsPtr)[propIndex].name);
            printf("%s costs $%d, but you only have $%d\n",
                   (*propsPtr)[propIndex].name,
                   (*propsPtr)[propIndex].price,
                   players[turn].moneyHeld);
          } else {
            printf("Player %d would you like to buy %s for $%d?\n",
                   turn,
                   (*propsPtr)[propIndex].name,
                   (*propsPtr)[propIndex].price);
            printf("The rent on this property is $%d\n", (*propsPtr)[propIndex].rent);
            printf("You currently have $%d\n", players[turn].moneyHeld);
            printf("Y for yes, N for no: ");
            char c;
            scanf(" %c", &c);
            if (c == 'y' || c == 'Y') {
              players[turn].propertiesOwned[propIndex] = (*propsPtr)[propIndex];
              (*propsPtr)[propIndex].owned = true;
              (*propsPtr)[propIndex].owningPlayer = turn;
              players[turn].moneyHeld -= (*propsPtr)[propIndex].price;
              players[turn].propsOwned++;
            }
          }
        }
        //Property owned
        else {
          //Owned by an opposing player
          if ((*propsPtr)[propIndex].owningPlayer != turn) {
            int setsProps = (*propsPtr)[propIndex].propsInSet;
            //Looks on right of current property to see who owns it
            bool multOn = true;
            for (int x = 1; x < setsProps - (*propsPtr)[propIndex].intrasetID; x++) {
              if ((*propsPtr)[propIndex + x].setID != (*propsPtr)[propIndex].setID) {
                printf("This is an issue\n");
              } else {
                if ((*propsPtr)[propIndex + x].owningPlayer != (*propsPtr)[propIndex].owningPlayer) {
                  multOn = false;
                  x = setsProps - (*propsPtr)[propIndex].intrasetID;
                }
              }
            }
            if (multOn) {
              //Look on left of current property to see who owns it, only if the same player owned the props on the right
              for (int w = 1; w < (*propsPtr)[propIndex].intrasetID + 1; w++) {
                if ((*propsPtr)[propIndex - w].owningPlayer != (*propsPtr)[propIndex].owningPlayer) {
                  multOn = false;
                  w = (*propsPtr)[propIndex].intrasetID + 1;
                }
              }
            }
            if (!multOn) {
              //Player is going bankrupt
              if ((*propsPtr)[propIndex].rent > players[turn].moneyHeld) {
                printf("Player %d went bankrupt to Player %d\n", turn, (*propsPtr)[propIndex].owningPlayer);
                players[(*propsPtr)[propIndex].owningPlayer].moneyHeld += players[turn].moneyHeld;
                for (int r = 0; r < numProps; r++) {
                  if (strcmp(players[turn].propertiesOwned[r].name, "NULL") != 0) {
                    (*propsPtr)[r].owningPlayer = (*propsPtr)[propIndex].owningPlayer;
                    players[(*propsPtr)[r].owningPlayer].propertiesOwned[r] = (*propsPtr)[r];
                  }
                }
                players[turn].inGame = false;
                numPlayers--;
                if (numPlayers == endPlayers) {
                  printBoardState(propsPtr, numProps, &players, startPlayers);
                  printf("The winners are\n");
                  if(checkForWins(startPlayers, &players, propsPtr, numProps)){
                    return;
                  }
                }
                k++;
                if (k == turnLimit) {
                  printBoardState(propsPtr, numProps, &players, startPlayers);
                  printf("The winners are\n");
                  if(checkForWins(startPlayers, &players, propsPtr, numProps)){
                    return;
                  }
                }
                turn++;
                if (turn > startPlayers - 1) {
                  turn = 0;
                }
                while (players[turn].inGame == false) {
                  turn++;
                }
                rolledDice = false;
              } else {
                printf("Player %d payed Player %d $%d for landing on %s\n",
                       turn,
                       (*propsPtr)[propIndex].owningPlayer,
                       (*propsPtr)[propIndex].rent,
                       (*propsPtr)[propIndex].name);
                players[turn].moneyHeld -= (*propsPtr)[propIndex].rent;
                players[(*propsPtr)[propIndex].owningPlayer].moneyHeld += (*propsPtr)[propIndex].rent;
              }
            } else {
              //Player is going bankrupt
              if ((*propsPtr)[propIndex].rent * propMult > players[turn].moneyHeld) {
                printf("Player %d went bankrupt to Player %d\n", turn, (*propsPtr)[propIndex].owningPlayer);
                players[(*propsPtr)[propIndex].owningPlayer].moneyHeld += players[turn].moneyHeld;
                for (int s = 0; s < numProps; s++) {
                  if (strcmp(players[turn].propertiesOwned[s].name, "NULL") != 0) {
                    (*propsPtr)[s].owningPlayer = (*propsPtr)[propIndex].owningPlayer;
                    players[(*propsPtr)[s].owningPlayer].propertiesOwned[s] = (*propsPtr)[s];
                  }
                }
                players[turn].inGame = false;
                numPlayers--;
                if (numPlayers == endPlayers) {
                  printBoardState(propsPtr, numProps, &players, startPlayers);
                  printf("The winners are\n");
                  if(checkForWins(startPlayers, &players, propsPtr, numProps)){
                    return;
                  }
                }
                k++;
                if (k == turnLimit) {
                  printBoardState(propsPtr, numProps, &players, startPlayers);
                  printf("The winners are\n");
                  if(checkForWins(startPlayers, &players, propsPtr, numProps)){
                    return;
                  }
                }
                turn++;
                if (turn > startPlayers - 1) {
                  turn = 0;
                }
                while (players[turn].inGame == false) {
                  turn++;
                }
                rolledDice = false;
              } else {
                printf("Player %d payed Player %d $%d for landing on %s\n",
                       turn,
                       (*propsPtr)[propIndex].owningPlayer,
                       propMult * (*propsPtr)[propIndex].rent,
                       (*propsPtr)[propIndex].name);
                players[turn].moneyHeld -= ((*propsPtr)[propIndex].rent * propMult);
                players[(*propsPtr)[propIndex].owningPlayer].moneyHeld += ((*propsPtr)[propIndex].rent * propMult);
              }
            }
          }
        }
      }
    }
      //Handle a player inspection
    else if (playerChoice == 2) {
      printf("Which player would you like to inspect?\n");
      for (int f = 0; f < startPlayers; f++) {
        if (players[f].inGame == true) {
          printf("%d for Player %d\n", f, f);
        }
      }
      printf("Your choice: ");
      int userChoice;
      scanf("%d", &userChoice);
      inspectPlayer(userChoice, propsPtr, numProps, &players, numPlayers);
      continue;
    }
      //Handle a player leaving the game
    else if (playerChoice == 3) {
      for (int d = 0; d < numProps; d++) {
        //This is a property owned by the player leaving, return it to the bank
        if (strcmp(players[turn].propertiesOwned[d].name, "NULL") != 0) {
          (*propsPtr)[d].owned = false;
          (*propsPtr)[d].owningPlayer = -1;
        }
      }
      //free(players[turn].propertiesOwned);
      players[turn].inGame = false;
      players[turn].moneyHeld = -1;
      numPlayers--;
      if (numPlayers == endPlayers) {
        printBoardState(propsPtr, numProps, &players, startPlayers);
        printf("The winners are\n");
        int playersIn = 0;
        int potWindex = 0;
        for (int m = 0; m < startPlayers; m++) {
          if (players[m].inGame == true) {
            playersIn++;
            potWindex = m;
          }
        }
        if (playersIn == 1) {
          printf("Player %d\n", potWindex);
          for (int h = 0; h < startPlayers; h++) {
            free(players[h].propertiesOwned);
          }
          free(players);
          //players = NULL;
          //Free board
          for (int q = 0; q < numProps; q++) {
            free((*propsPtr)[q].name);
          }
          return;
        }
        if(checkForWins(startPlayers, &players, propsPtr, numProps)){
          return;
        }
      }
      turn++;
      if (turn > startPlayers - 1) {
        turn = 0;
      }
      while (players[turn].inGame == false) {
        turn++;
      }
      if (rolledDice) {
        k++;
        rolledDice = false;
      }
    } else if (playerChoice == 4) {
      k = turnLimit;
    } else {
      printf("Nice! You broke it.\n");
    }
    //Was here
  }
  //printf("Out the loop\n");
}

void generatePlayers(Player** players, int numPlayers, int startingCash, int numProps){
  Property nullProp;
  nullProp.name = "NULL";
  (*players) = (Player *) malloc(numPlayers * sizeof(Player));
  for (int i = 0; i < numPlayers; i++) {
    (*players)[i].propertiesOwned = (Property *) malloc(numProps * sizeof(Property));
    for (int p = 0; p < numProps; p++) {
      (*players)[i].propertiesOwned[p] = nullProp;
    }
    (*players)[i].playerNum = i;
    (*players)[i].moneyHeld = startingCash;
    (*players)[i].location = 0;
    (*players)[i].propsOwned = 0;
    (*players)[i].inGame = true;
  }
}

/**
* Generates the result of rolling two 6 sided dice, by reading the next two numbers in the current randon
* file and doing mod 6 + 1 (mod gets numbers 0-5 so adding one is 1-6)
**/
int rollDice(FILE *randomNumFile) {
  char c[25];
  fgets(c, 25, randomNumFile);
  int firstNum = atoi(c);
  fgets(c, 25, randomNumFile);
  int secondNum = atoi(c);
  //printf("%d, %d\n", firstNum, secondNum);
  int firstDie = firstNum % 6 + 1;
  int secondDie = secondNum % 6 + 1;
  //printf("%d, %d\n", firstDie, secondDie);
  int roll = firstDie + secondDie;
  return roll;
}

void inspectPlayer(int playerNum, Property **propsPtr, int numProps, Player **playersPtr, int numPlayers) {
  printf("Player %d\n", playerNum);
  printf("  Cash: $%d\n", (*playersPtr)[playerNum].moneyHeld);
  printf("  Properties owned\n");
  int maxSet = 0;
  for (int k = 0; k < numProps; k++){
    if ((*propsPtr)[k].setID > maxSet){
      maxSet = (*propsPtr)[k].setID;
    }
  }
  for (int i = 0; i < maxSet + 1; i++) {
    printf("    %d: ", i);
    bool first = true;
    for (int j = 0; j < numProps; j++) {
      if ((*propsPtr)[j].setID == i && (*propsPtr)[j].owningPlayer == playerNum) {
        if (first) {
          printf("%s", (*propsPtr)[j].name);
          first = false;
        } else {
          printf(" | %s", (*propsPtr)[j].name);
        }
      }
    }
    printf("\n");
  }
}