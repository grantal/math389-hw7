#ifndef __lib_h
#define __lib_h

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

solitaire_t *newSolitaire();
arena_t *newArena();
stAck_t *newStack(int type);
void putSolitaire(solitaire_t *S);
card_t *cardOf(char *c, solitaire_t *S);
int play(card_t *card, arena_t *arena, solitaire_t *S);
int moveOnto(card_t *card, card_t *onto, solitaire_t *S);
int isEmpty(stAck_t *stack);
void push(card_t *card, stAck_t *stack);
card_t *pop(stAck_t *stack) {

#endif
