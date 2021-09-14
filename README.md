# Bulls & Cows - Intro to Operating Systems course
## Implementation of  Client-Server Bulls Network Multiplayer game  using C and WIN32 API
Bulls & Cows - Bulls and Cows (also known as Cows and Bulls or Pigs and Bulls) is an old code-breaking mind or paper and pencil game for two or more players, predating the commercially marketed board game Mastermind. more can be found on https://en.wikipedia.org/wiki/Bulls_and_Cows#The_numerical_version

The 'Bulls and Cows' game that is implemented is the numerical version of a 2 player game.
One player thinks of a number, while the other player tries to guess it. The number to be guessed must be a 4 digit number, using only digits from 1 - 9, each digit atmost once. For every guess that the player makes, he gets 2 values - the number of bulls and the number of cows.

1 bull means the guess contains and the target number have 1 digit in common, and in the correct position. 1 cow means the guess and the target have 1 digit in common, but not in correct position.

The game ends with win if one of the player guessed right the other 4 digits. It can also ends with tie if both of the players guessed right the other 4 digits in the same turn.
