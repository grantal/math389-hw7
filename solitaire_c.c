#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 8192

void srand48(long);
double drand48(void);

#define SPADES   0
#define HEARTS   1
#define CLUBS    2
#define DIAMONDS 3

#define DRAW     4
#define DISCARD   5
#define LAIN(pile)     (5+(pile))
#define HIDDEN(pile)  (12+(pile))

#define FAILURE 0
#define SUCCESS (!FAILURE)

#define BLACK 0
#define RED   1

char red[]       = {0x1b,0x5b,0x33,0x31,0x6d,0};
//I changed the black color because it didn't show up in my terminal
char black[]     = {0x1b,0x5b,0x33,0x32,0x6d,0};
char neutral[]   = {0x1b,0x5b,0x33,0x39,0x6d,0};
char whitebg[]   = {0x1b,0x5b,0x33,0x47,0x6d,0};
char neutralbg[] = {0x1b,0x5b,0x33,0x49,0x6d,0};

char spades[]   = {0xE2, 0x99, 0xA0, 0};
char hearts[]   = {0xE2, 0x99, 0xA5, 0};
char clubs[]    = {0xE2, 0x99, 0xA3, 0};
char diamonds[] = {0xE2, 0x99, 0xA6, 0};
     
char *suit[] = {spades,hearts,clubs,diamonds};
char *face[] = {"O","A","2","3","4","5","6","7","8","9","T","J","Q","K"};

typedef struct _card_t {
  int player;
  int face;
  int suit;
  struct _card_t *below;
  struct _stAck_t *stack;
} card_t;

typedef struct _deck_t {
  int player;
  card_t cards[52];
} deck_t;

typedef struct _stAck_t {
  card_t *top;
  int type;
} stAck_t;

typedef struct _arena_t {
  stAck_t *suit[4];
} arena_t;

typedef struct _solitaire_t {
  int player;
  deck_t *deck;
  stAck_t *hidden[8];
  stAck_t *lain[8];
  stAck_t *discard;
  stAck_t *draw;
} solitaire_t;

stAck_t *newStack(int type) {
  stAck_t *stack = malloc(sizeof(stack));
  stack->top = NULL;
  stack->type = type;
  return stack;
}

int isRed(card_t *c) {
  return c->suit % 2 == RED;
}

int isBlack(card_t *c) {
  return c->suit % 2 == BLACK;
}

int isAce(card_t *c) {
  return c->face == 1;
}

int isKing(card_t *c) {
  return c->face == 13;
}

int isTop(card_t *c) {
  return c->stack->top == c;
}

int pileOf(stAck_t *stack) {
  return stack->type - LAIN(1) + 1;
}

int isLain(stAck_t *stack) {
  return (LAIN(1) <= stack->type) && (stack->type <= LAIN(7));
}

int isUp(card_t *c) {
  return isLain(c->stack) || (isTop(c) && (c->stack->type == DISCARD));
}

int okOn(card_t *c1, card_t *c2) {
  return ((isRed(c1) && isBlack(c2)) || (isRed(c2) && isBlack(c1)))
          && (c1->face +1 == c2->face);
}

void putRed() {
  // printf("%s",whitebg);
  printf("%s",red);
}

void putBlack() {
  // printf("%s",whitebg);
  printf("%s",black);
}


void putBack() {
  printf("%s",neutral);
  // printf("%s",neutralbg);
}

void putColorOfSuit(int s) {
  if (s % 2 == RED) {
    putRed();
  } else {
    putBlack();
  }
}

void putSuit(int s) {
  printf("%s",suit[s]);
}

void putFace(int f) {
  printf("%s",face[f]);
}

void putCard(card_t *c) {
  if (isRed(c)) {
    putRed();
  } else {
    putBlack();
  }
  putFace(c->face);
  putSuit(c->suit);
  putBack();
}

int isEmpty(stAck_t *stack) {
  return stack->top == NULL;
}

void push(card_t *card, stAck_t *stack) {
  card->below = stack->top;
  stack->top = card;
  card->stack = stack;
}

card_t *pop(stAck_t *stack) {
  card_t *card = stack->top;
  stack->top = card->below;
  card->below = NULL;
  card->stack = NULL;
  return card;
}

card_t *top(stAck_t *stack) {
  return stack->top;
}

card_t *cardOf(char *c, solitaire_t *S) {
  int i;
  if (c[0] >= '2' && c[0] <= '9') {
    i = c[0] - '0';
  } else if (c[0] == 'T' || c[0] == 't') {
    i = 10;
  } else if (c[0] == 'J' || c[0] == 'j') {
    i = 11;
  } else if (c[0] == 'Q' || c[0] == 'q') {
    i = 12;
  } else if (c[0] == 'K' || c[0] == 'k') {
    i = 13;
  } else if (c[0] == 'A' || c[0] == 'a') {
    i = 1;
  } else {
    return NULL;
  }
  int j;
  if (c[1] == 'S' || c[1] == 's') {
    j = SPADES;
  } else if (c[1] == 'H' || c[1] == 'h') {
    j = HEARTS;
  } else if (c[1] == 'C' || c[1] == 'c') {
    j = CLUBS;
  } else if (c[1] == 'D' || c[1] == 'd') {
    j = DIAMONDS;
  } else {
    return NULL;
  }
  return &S->deck->cards[j*13+i-1];
}

deck_t *newDeck() {
  // Make a fresh deck of cards.
  deck_t *deck = malloc(sizeof(deck_t));
  int i,s;
  for (i=0, s=0; s <= 3; s++) {
    for (int f=1; f <= 13; f++, i++) {
      deck->cards[i].suit = s;
      deck->cards[i].face = f;
      deck->cards[i].below = NULL;
      deck->cards[i].stack = NULL;
    }
  }
  return deck;
}

void shuffleInto(deck_t *deck, stAck_t *draw) {
  card_t *shuffle[52];

  // Get a reference to each of the 52 cards.
  for (int i = 0; i < 52; i++) {
    shuffle[i] = &deck->cards[i];
  }

  // Perform a Knuth shuffle.
  for (int i = 0; i < 52; i++) {

    // Select the next card.
    int r = (int)((52-i)*drand48());
    if (r != 0) {
      card_t *tmp = shuffle[i];
      shuffle[i] = shuffle[i+r];
      shuffle[i+r] = tmp;
    }

    // Put it onto the draw pile.
    push(shuffle[i],draw);
  }
}

arena_t *newArena(void) {
  arena_t *A = malloc(sizeof(arena_t));
  for (int s=0; s<4; s++) {
    A->suit[s] = newStack(s);
  }
  return A;
}

solitaire_t *newSolitaire(time_t tv_sec) {
  srand48(tv_sec);

  solitaire_t *S = (solitaire_t *)malloc(sizeof(solitaire_t));
  S->deck = newDeck();

  // Make all the stacks.
  S->draw = newStack(DRAW);
  S->discard = newStack(DISCARD);
  for (int p=1; p<=7; p++) {
    S->lain[p] = newStack(LAIN(p));
    S->hidden[p] = newStack(HIDDEN(p));
  }

  // get shuffled draw pile from server
  shuffleInto(S->deck,S->draw);

  // Make each of the hidden piles.
  for (int i=1; i <= 7; i++) {
    for (int j=i; j <= 7; j++) {
      push(pop(S->draw),S->hidden[j]);
    }
  }

  // Flip the top card of each pile.
  for (int i=1; i <= 7; i++) {
    push(pop(S->hidden[i]),S->lain[i]);
  }

  // Flip up the first card.
  push(pop(S->draw),S->discard);

  return S;
}

void maybeFlip(stAck_t *stack, solitaire_t *S) {
  if (isLain(stack) && isEmpty(stack)) {
    int p = pileOf(stack);
    if (!isEmpty(S->hidden[p])) {
      push(pop(S->hidden[p]),S->lain[p]);
    }
  }
}

void putStack(stAck_t *stack) {
  card_t *c = stack->top;
  while (c != NULL) {
    putCard(c);
    printf(" ");
    c = c->below;
  }
}

void putArena(arena_t *A) {
  for (int s=0; s<4; s++) {
    putColorOfSuit(s);
    putSuit(s);
    putBack();
    printf(": ");
    putStack(A->suit[s]);
    printf("\n");
  }
}

void putSolitaire(solitaire_t *S) {
  for (int p=1; p<=7; p++) {
    printf("%d: ",p);
    putStack(S->lain[p]);
    printf("[");
    putStack(S->hidden[p]);
    printf("]\n");
  }
  printf("\n[");
  putStack(S->draw);
  printf("]\n");
  putStack(S->discard);
  printf("\n");
}

int play(card_t *card, arena_t *arena, solitaire_t *S) {
  stAck_t *stack = card->stack;
  stAck_t *pile = arena->suit[card->suit];
  if (isTop(card)) {
    // leave validation of arena to server
    pop(stack);
    maybeFlip(stack,S);
    return SUCCESS;
  }
  return FAILURE;
}

void move(card_t *card, stAck_t *dest, solitaire_t *S) {
  stAck_t *srce = card->stack;
  card_t *top = srce->top;
  card_t *c = top;
  while (c != card) {
    c->stack = dest;
    c = c->below;
  }
  card->stack = dest;
  srce->top = card->below;
  card->below = dest->top;
  dest->top = top;
  maybeFlip(srce,S);
}

int isBottom(char *cs) {
  return strcmp(cs,"B") == 0;
}

stAck_t *freeLain(solitaire_t *S) {
  for (int p=1; p<=7; p++) {
    if (isEmpty(S->lain[p])) {
      return S->lain[p];
    }
  }
  return NULL;
}

int moveOnto(card_t *card, card_t *onto, solitaire_t *S) {

  if (onto == NULL) {

    if (isKing(card)) {
      stAck_t *dest = freeLain(S);
      if (isUp(card) && dest != NULL) {
        printf("going for it\n");
        move(card,dest,S);
        return SUCCESS;
      }
    }

  } else { 

    stAck_t *dest = onto->stack;
    if (isLain(dest) && isUp(card) && isTop(onto) && okOn(card,onto)) {
      move(card,dest,S);
      return SUCCESS;
    } 

  }

  return FAILURE;
}

int main(int argc, char **argv) {
  //
  // Check the arguments for the host name and port number of 
  // an echo service.
  //
  if (argc != 3) {
    fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }
  
  //
  // Look up the host's name to get its IP address.
  //
  char *host = argv[1];
  int port = atoi(argv[2]);
  struct hostent *hp;
  if ((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr,"GETHOSTBYNAME failed.\n");
    exit(-1);
  }

  //
  // Request a socket and get its file descriptor.
  //
  int clientfd;
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"SOCKET creation failed.\n");
    exit(-1);
  }
    

  //
  // Fill in the host/port info into a (struct sockaddr_in).
  //
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  bcopy((char *) hp->h_addr_list[0], 
	(char *)&serveraddr.sin_addr.s_addr, 
	hp->h_length);
  serveraddr.sin_port = htons(port);

  //
  // Connect to the given host at the given port number.
  //
  if (connect(clientfd,
	      (struct sockaddr *)&serveraddr, 
	      sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr,"CONNECT failed.\n");
    exit(-1);
  }

  // tell server we want to play
  write(clientfd,"PLAYER",strlen("PLAYER")+1);

  unsigned char *ip;
  ip = (unsigned char *)&serveraddr.sin_addr.s_addr;
  // confirm connection worked
  printf("Connected to Solitaire server at %d.%d.%d.%d. Waiting for server to start game.\n", ip[0], ip[1], ip[2], ip[3]);

  // get random seed from server
  char seedstring[MAXLINE];
  int n = read(clientfd, seedstring, MAXLINE);
  char *ptr;
  unsigned long seed = strtoul(seedstring, &ptr, 10);
  printf("seed: %lu\n", seed);
  solitaire_t *S = newSolitaire(seed);
  arena_t *A = newArena();
  
  for (int s=0; s<4; s++) {
    A->suit[s] = newStack(s);
  }
  
  while (1) {
    putSolitaire(S);
    char buffer[MAXLINE];
    fgets(buffer,MAXLINE,stdin);
    char cmd[MAXLINE];
    char c1[MAXLINE];
    char c2[MAXLINE];
    sscanf(buffer,"%s",cmd);

    // send command to server and validate it
    write(clientfd,buffer,strlen(buffer)+1);

    // Read the server's response.
    char response[MAXLINE];
    int n = read(clientfd, response, MAXLINE);
    printf("%s\n", response);
    //don't do the action unless server okays it 
    if(n != 0 && strcmp(response,"SUCCESS") == 0){

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

  //
  // Close the connection.
  //
  close(clientfd); 

  exit(0);

}
