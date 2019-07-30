#ifndef PROPERTY_H
#define PROPERTY_H

typedef struct PropertyStruct {
  char *name;
  int setID;
  int intrasetID;
  int price;
  //Price of house if you have all properties
  int houseCost;
  int hotelCost;
  //rent without houses or hotels
  int rent;
  //Rent with a house
  int houseRent;
  //Rent with a hotel
  int hotelRent;
  int owningPlayer;
  bool owned;
  //A list of the playerNums of all players on this space
  int *playersOn;
  //If true, everything else is -1 except name
  bool isGo;
  //same as the max intraset ID in a given set
  int propsInSet;
  //how much is paid for lainding on this space (only applis to go)
  int goPayout;
  //Total number of properties+go on the board. Kind of retundent to have in every property but whatever
  int boardSize;
  //The number of this property as it appears on the board. 0 being the first place and counting up from there
  int propNum;
} Property;

#endif