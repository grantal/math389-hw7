#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "lib.h"

#define MAXLINE 8192

int main(int argc, char **args) {
  solitaire_t *S = newSolitaire();
  arena_t *A = newArena();
  
  for (int s=0; s<4; s++) {
    A->suit[s] = newStack(s);
  }
  
  while (1) {
    putSolitaire(S);
    char buffer[80];
    fgets(buffer,80,stdin);
    char cmd[80];
    char c1[80];
    char c2[80];
    sscanf(buffer,"%s",cmd);

    if (cmd[0] == 'p') {

      // play <card>
      //
      // Play a card on top of some stack to the arena.
      //
      sscanf(buffer,"%s %s",cmd,c1);
      card_t *card = cardOf(c1,S);
      if (play(card,A,S)) {
	printf("Done!\n");
      } else {
	printf("That card was not played.\n");
      } 

    } else if (cmd[0] == 'm') {

      // move <card> <card>
      //
      // Move a card to some lain stack. 
      //
      sscanf(buffer,"%s %s %s",cmd,c1,c2);
      if (moveOnto(cardOf(c1,S),cardOf(c2,S),S)) {
	printf("Done.\n");
      } else {
	printf("That move cannot be made.\n");
      }

    } else if (cmd[0] == 'n') {

      // next
      //
      // Draws the next card and puts it on top of the discard pile.
      if (!isEmpty(S->draw)) {
	push(pop(S->draw),S->discard);
      }
    }
  }
}
