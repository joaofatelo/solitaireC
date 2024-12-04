#include "card_pile.h"
#include <stdlib.h>
#include <stdio.h>
#include <pg/pglib.h>

int rand_range(int li, int ls) {
    return rand() % (ls -li) + li;
}

void card_init(card_t *c, suits_t s, int v) {
    c->value = v;
    c->suit = s;
}

static void pile_shuffle(card_pile_t *p) {
    for (int i = p->size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        card_t temp = p->cards[i];
        p->cards[i] = p->cards[j];
        p->cards[j] = temp;
    }
}

void pile_init_full(card_pile_t *p, bool shuffled) {
    int nc = 0;
    for (int s = Hearts; s <= Spades; ++s) {
        for (int v = 0; v < CARDS_PER_SUIT; v++) {
            p->cards[nc].suit = s;
            p->cards[nc].value = v;
            nc++;
        }
    }
    p->size = p->capacity = NCARDS;
    if (shuffled) {
        pile_shuffle(p);
    }
}

bool pile_push(card_pile_t *p, card_t card) {
    if (p->size < p->capacity) {
        p->cards[p->size++] = card;
        return true;
    }
    return false;
}

bool pile_pop(card_pile_t *p, card_t *card) {
    if (p->size > 0) {
        *card = p->cards[--p->size];
        return true;
    }
    return false;
}

int pile_size(const card_pile_t *p) {
    return p->size;
}
