#include <pg/pglib.h>
#include <stdio.h>
#include "card_pile.h"
#include <string.h>

// Definições de constantes e variáveis globais

#define HEARTS_PLACE (MARGEM*4+CARD_WIDTH*3)
#define DIAMONDS_PLACE (MARGEM*5+CARD_WIDTH*4)
#define SPADES_PLACE (MARGEM*6+CARD_WIDTH*5)
#define CLUBS_PLACE (MARGEM*7+CARD_WIDTH*6)

#define FOUNDATIONS_CLK 1
#define DECK_CLK 2
#define DISCARD_DECK_CLK 3

#define DECK (MARGEM)
#define DISCARD_DECK (MARGEM*2+CARD_WIDTH)
#define MARGEM 40
#define MARGEM_UP 130

#define CARD_HEIGHT 110
#define CARD_WIDTH 80
#define CARD_FOLDER "../cards"
#define CARD_BACK 'b'

#define DARK_CARD_WIDTH 80
#define DARK_CARD_HEIGHT 110
#define DARK_CARD_BACK_CLR graph_rgb(1,50,32)


#define BROWN_AREA_WIDTH 800
#define BROWN_AREA_HEIGHT 650
#define BROWN_AREA_CLR graph_rgb(101,67,33)

#define WINDOW_WIDTH   890
#define WINDOW_HEIGHT  900
#define BACK_CLR      graph_rgb(0, 60, 0)

#define COUNTER_WIDTH    800
#define COUNTER_HEIGHT   70
#define COUNTERBACK_CLR  c_dgray

#define TIMER_X         730
#define TIMER_Y         33
#define TIMER_DIG_CLR   c_gray
#define TIMER_BACK_CLR  c_dgray

#define POINTS_X       310
#define POINTS_Y       40

#define PLAYS_X        50
#define PLAYS_Y        40

#define BASE_TIME       1000

#define VICTORY_MSG_HEIGHT 30
#define VICTORY_MSG_WIDTH 350

#define VICTORY_MSG_X  ((WINDOW_WIDTH - VICTORY_MSG_WIDTH)/2)
#define VICTORY_MSG_Y  ((WINDOW_HEIGHT - VICTORY_MSG_HEIGHT)/2)

#define COLOR_VICTORY_MSG  graph_rgb(200,200,200)

#define X_MARGIN  40       // Margem inicial do lado esquerdo da tela
#define Y_MARGIN 270       // Margem inicial do topo da tela
#define CARD_WIDTH 80      // Largura da carta
#define CARD_HEIGHT 110    // Altura da carta
#define X_SPACING 40      // Espaçamento horizontal entre as pilhas
#define Y_SPACING 25   // Espaçamento vertical entre as cartas empilhadas

#define TABLEAU1_X (X_MARGIN)
#define TABLEAU1_Y (Y_MARGIN)

#define TABLEAU2_X (TABLEAU1_X + CARD_WIDTH + X_SPACING)
#define TABLEAU2_Y (Y_MARGIN)

#define TABLEAU3_X (TABLEAU2_X + CARD_WIDTH + X_SPACING)
#define TABLEAU3_Y (Y_MARGIN)

#define TABLEAU4_X (TABLEAU3_X + CARD_WIDTH + X_SPACING)
#define TABLEAU4_Y (Y_MARGIN)

#define TABLEAU5_X (TABLEAU4_X + CARD_WIDTH + X_SPACING)
#define TABLEAU5_Y (Y_MARGIN)

#define TABLEAU6_X (TABLEAU5_X + CARD_WIDTH + X_SPACING)
#define TABLEAU6_Y (Y_MARGIN)

#define TABLEAU7_X (TABLEAU6_X + CARD_WIDTH + X_SPACING)
#define TABLEAU7_Y (Y_MARGIN)


Clock clk;
Counter counter;
Counter points;
int firstClick = 1;

card_pile_t deck;
card_pile_t discard_deck;
card_pile_t foundation_hearts;
card_pile_t foundation_spades;
card_pile_t foundation_diamonds;
card_pile_t foundation_clubs;
card_pile_t reserve_deck;
card_pile_t tableaus[7];

const char foudation_suits[]={'h','d','c','s'};

void card_show(card_t card, int x, int y, bool visible) {
    const char suit_names[] = { 'h', 'd', 'c', 's' };
    const char card_values[] = { 'a', '2', '3', '4', '5', '6', '7', '8', '9', 't', 'j', 'q', 'k'};
    char card_name[128];
    if (!visible)  sprintf(card_name, "%s/%c.png", CARD_FOLDER, CARD_BACK);
    else sprintf(card_name, "%s/%c%c.png", CARD_FOLDER, suit_names[card.suit], card_values[card.value]);
    graph_image(card_name, x, y, CARD_WIDTH, CARD_HEIGHT);
}

bool can_move_to_foundation(card_t card, card_pile_t *foundation) {
    // Se a fundação está vazia, só podemos adicionar um Ás
    if (foundation->size == 0) {
        return card.value == 0; // Ás
    } else {
        // Verifica se a carta é do mesmo naipe e uma unidade maior do que a carta no topo da fundação
        card_t top_card = foundation->cards[foundation->size - 1];
        return (card.suit == top_card.suit) && (card.value == top_card.value + 1);
    }
}
bool move_card_to_foundation(card_t card_to_move, card_pile_t *target_foundation) {
    int foundation_position;

    // Determine a posição da fundação com base no naipe da carta
    switch (card_to_move.suit) {
        case Hearts:
            foundation_position = HEARTS_PLACE;
            break;
        case Diamonds:
            foundation_position = DIAMONDS_PLACE;
            break;
        case Spades:
            foundation_position = SPADES_PLACE;
            break;
        case Clubs:
            foundation_position = CLUBS_PLACE;
            break;
        default:
            return false; // Naipe inválido
    }

    if (can_move_to_foundation(card_to_move, target_foundation)) {
        pile_push(target_foundation, card_to_move);
        card_show(card_to_move, foundation_position, MARGEM_UP, true);

        // Outras lógicas, se necessário

        return true;
    } else {
        return false;
    }
}
void reveal_top_card(card_pile_t *tableau, int tableau_index) {
    if (tableau->size > 0) {
        card_t *top_card = &tableau->cards[tableau->size - 1];
        top_card->visible = true; // Esta linha pode ser removida se não houver campo 'visible'

        // Calcula as posições x e y com base no índice do tableau
        int x_pos, y_pos;
        switch(tableau_index) {
            case 0: x_pos = TABLEAU1_X; y_pos = TABLEAU1_Y + (tableau->size - 1) * Y_SPACING; break;
            case 1: x_pos = TABLEAU2_X; y_pos = TABLEAU2_Y + (tableau->size - 1) * Y_SPACING; break;
            case 2: x_pos = TABLEAU3_X; y_pos = TABLEAU3_Y + (tableau->size - 1) * Y_SPACING; break;
            case 3: x_pos = TABLEAU4_X; y_pos = TABLEAU4_Y + (tableau->size - 1) * Y_SPACING; break;
            case 4: x_pos = TABLEAU5_X; y_pos = TABLEAU5_Y + (tableau->size - 1) * Y_SPACING; break;
            case 5: x_pos = TABLEAU6_X; y_pos = TABLEAU6_Y + (tableau->size - 1) * Y_SPACING; break;
            case 6: x_pos = TABLEAU7_X; y_pos = TABLEAU7_Y + (tableau->size - 1) * Y_SPACING; break;
            default: return; // Índice inválido
        }

        // Mostra a carta
        card_show(*top_card, x_pos, y_pos, true);
    }
}



bool move_top_card_to_foundation(card_t card_to_move, card_pile_t *origin_tableau, card_pile_t *target_foundation, int tableau_index) {
    int foundation_position;

    // Determine a posição da fundação com base no naipe da carta
    switch (card_to_move.suit) {
        case Hearts:
            foundation_position = HEARTS_PLACE;
            break;
        case Diamonds:
            foundation_position = DIAMONDS_PLACE;
            break;
        case Spades:
            foundation_position = SPADES_PLACE;
            break;
        case Clubs:
            foundation_position = CLUBS_PLACE;
            break;
        default:
            return false; // Naipe inválido
    }

    if (can_move_to_foundation(card_to_move, target_foundation)) {
        pile_push(target_foundation, card_to_move);
        card_show(card_to_move, foundation_position, MARGEM_UP, true);

        if (origin_tableau->size > 0) {
            reveal_top_card(origin_tableau, tableau_index);
        }

        return true;
    } else {
        return false;
    }
}

void shuffle_deck(card_pile_t *deck) {
    srand(time(NULL));  // Seed the random number generator with the current time

    for (int i = deck->size - 1; i > 0; i--) {
        int j = rand() % (i + 1);  // Generate a random index

        // Swap cards at indices i and j
        card_t temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}
	
int calcular_x_pos(int index) {
    return X_MARGIN + (index * (CARD_WIDTH + X_SPACING));
}

// Calcula a posição y da carta com base na profundidade da carta na pilha
int calcular_y_pos(int depth) {
    return Y_MARGIN + (depth * Y_SPACING);
}	
	
void init_tableaus(card_pile_t *tableaus) {
	for (int i = 0; i < 7; i++) {
		tableaus[i].size = 0; // Inicializa a pilha como vazia
		tableaus[i].capacity = MAX_PILE_SIZE; // Capacidade máxima

		// Distribui as cartas para cada pilha
		for (int j = 0; j <= i; j++) {
			card_t card;
			if (pile_pop(&deck, &card)) {
				// Define a visibilidade da carta
				card.visible = (j == i); // A última carta de cada pilha é virada para cima

				pile_push(&tableaus[i], card); // Adiciona a carta na pilha

				// Mostra a carta
				card_show(card, calcular_x_pos(i), calcular_y_pos(j), card.visible);            
			}
		}
	}
}


void init() {
    graph_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, BACK_CLR, true);
    graph_rect(40, 20, COUNTER_WIDTH, COUNTER_HEIGHT, COUNTERBACK_CLR, true);
    graph_rect(40, 270, BROWN_AREA_WIDTH, BROWN_AREA_HEIGHT, BROWN_AREA_CLR, true);
    int space = 40;
      for (int i = 0; i < 7; i++) {
        if (i != 2) {
        graph_rect(space, 130, DARK_CARD_WIDTH, DARK_CARD_HEIGHT, DARK_CARD_BACK_CLR, true);
        }
        space = space + DARK_CARD_WIDTH + 40;
       }

    clk_create(&clk, TIMER_X, TIMER_Y, LARGE_FONT, TIMER_DIG_CLR, TIMER_BACK_CLR);
    ct_create(&counter, POINTS_X, POINTS_Y, 0, "Points", MEDIUM_FONT);
    ct_create(&points, PLAYS_X, PLAYS_Y, 0, "Plays", MEDIUM_FONT);

    clk_show(&clk);
    ct_show(&counter);
    ct_show(&points);
    
    pile_init_full(&deck, true);
    shuffle_deck(&deck);
    
    deck.size=NCARDS;
    deck.capacity=NCARDS;
    
    discard_deck.size=0;
    discard_deck.capacity=NCARDS;
    
    foundation_hearts.size=0;
    foundation_hearts.capacity=CARDS_PER_SUIT;
    
    foundation_spades.size=0;
    foundation_spades.capacity=CARDS_PER_SUIT;
    
    foundation_diamonds.size=0;
    foundation_diamonds.capacity=CARDS_PER_SUIT;
    
    foundation_clubs.size=0;
    foundation_clubs.capacity=CARDS_PER_SUIT;
    
    
    shuffle_deck(&deck); // Embaralha o baralho

  //	  card_pile_t tableaus[7]; // Cria as pilhas de jogo
    init_tableaus(tableaus); // Inicializa as pilhas de jogo
    
    for(int i=0;i<deck.size-1;i++){
		card_show(deck.cards[i], 40,130,false);
	}
}
 
void count_cards_each_tableaus(card_pile_t tableaus[], int counts[], int num_tableaus) {
    for (int i = 0; i < num_tableaus; i++) {
        counts[i] = tableaus[i].size; // Conta o número de cartas no tableau 'i' e armazena em counts[i]
    //    printf("Tableau %d tem %d cartas.\n", i, counts[i]); // Adiciona um debug para imprimir o número de cartas
    }
}

int get_tableau_index(int x, int y) {
    int tableauX[] = {TABLEAU1_X, TABLEAU2_X, TABLEAU3_X, TABLEAU4_X, TABLEAU5_X, TABLEAU6_X, TABLEAU7_X};
    int tableauCardCounts[7]; // Este array precisa ser preenchido com o número de cartas em cada tableau
    count_cards_each_tableaus(tableaus, tableauCardCounts, 7);
  //  printf("Passou aqui - Verificando cliques nos tableaus.\n");
    for (int i = 0; i < 7; i++) {
        int tableauEndX = tableauX[i] + CARD_WIDTH;
        int tableauEndY = Y_MARGIN; // Valor inicial para uma pilha vazia ou com uma carta
        // Ajuste tableauEndY para incluir o espaçamento de todas as cartas na pilha
        if (tableauCardCounts[i] > 0) {
            tableauEndY += (tableauCardCounts[i] - 1) * Y_SPACING + CARD_HEIGHT;
        }
    //    printf("Tableau %d: X range: [%d, %d], Y range: [%d, %d]\n", i, tableauX[i], tableauEndX, Y_MARGIN, tableauEndY);

        if (x >= tableauX[i] && x < tableauEndX && y >= Y_MARGIN && y <= tableauEndY) {
            return i;
        }
    }
    return -1; // Retorna -1 se nenhum tableau for clicado
}
 
void redraw_pile(card_pile_t *tableau, int tableau_index) {
    // Primeiro, limpa a área da pilha. Você pode precisar ajustar as coordenadas e tamanho conforme sua interface gráfica.
    int x_pos, y_pos;
    switch(tableau_index) {
        case 0: x_pos = TABLEAU1_X; y_pos = TABLEAU1_Y; break;
        case 1: x_pos = TABLEAU2_X; y_pos = TABLEAU2_Y; break;
        case 2: x_pos = TABLEAU3_X; y_pos = TABLEAU3_Y; break;
        case 3: x_pos = TABLEAU4_X; y_pos = TABLEAU4_Y; break;
        case 4: x_pos = TABLEAU5_X; y_pos = TABLEAU5_Y; break;
        case 5: x_pos = TABLEAU6_X; y_pos = TABLEAU6_Y; break;
        case 6: x_pos = TABLEAU7_X; y_pos = TABLEAU7_Y; break;
        default: return; // Índice inválido
    }
    graph_rect(x_pos, y_pos, CARD_WIDTH, CARD_HEIGHT * tableau->capacity, BROWN_AREA_CLR, true);

    // Redesenha as cartas restantes no tableau
    for (int i = 0; i < tableau->size; i++) {
        card_show(tableau->cards[i], x_pos, y_pos + i * Y_SPACING, tableau->cards[i].visible);
    }
}

bool can_move_to_tableau(card_t card_to_move, card_pile_t *destination_tableau) {
    // Se o tableau de destino estiver vazio, só pode mover um rei (valor 12).
    if (destination_tableau->size == 0) {
        return card_to_move.value == 12; // 12 representa o Rei
    }
    // Pegue a carta do topo do tableau de destino.
    card_t top_card = destination_tableau->cards[destination_tableau->size - 1];
    // Verifique se as cores são alternadas e a ordem é decrescente.
    bool different_color = ((card_to_move.suit == Hearts || card_to_move.suit == Diamonds) &&
                            (top_card.suit == Clubs || top_card.suit == Spades)) ||
                           ((card_to_move.suit == Clubs || card_to_move.suit == Spades) &&
                            (top_card.suit == Hearts || top_card.suit == Diamonds));
    // A ordem decrescente é verificada com base no valor das cartas.
    bool consecutive_order = (top_card.value - card_to_move.value == 1);
    // Debug: Imprimindo resultados da verificação
    return different_color && consecutive_order;
}
 
void move_card_sequence(card_pile_t *origin_tableau, card_pile_t *destination_tableau, int start_index, int origin_index, int destination_index) {
    // Verificar se o movimento é permitido.
    if (can_move_to_tableau(origin_tableau->cards[start_index], destination_tableau)) {
        // Move todas as cartas a partir do start_index para a nova pilha.
        for (int i = start_index; i < origin_tableau->size; i++) {
            pile_push(destination_tableau, origin_tableau->cards[i]);
        }
        // Atualiza o tamanho da pilha original.
        origin_tableau->size = start_index;
        // Redesenhe as pilhas para refletir a mudança.
        redraw_pile(origin_tableau, origin_index);
        redraw_pile(destination_tableau, destination_index);
    }
}

void timer_handler() {
    if (!firstClick) {
        clk_tick(&clk);
        clk_show(&clk);
    }
}

void key_handler(KeyEvent ke) {
    if (ke.state == KEY_PRESSED) {
        if (ke.keysym == SDLK_DOWN) {
            ct_dec(&counter);
            ct_show(&counter);
        } else if (ke.keysym == SDLK_UP) {
            ct_inc(&counter);
            ct_show(&counter);
        } else if (ke.keysym == SDLK_t) {
            graph_exit();
        }
    }
}

bool check_win_condition() {
    // Verifica se todas as fundações estão completas
    return foundation_hearts.size == CARDS_PER_SUIT &&
           foundation_diamonds.size == CARDS_PER_SUIT &&
           foundation_clubs.size == CARDS_PER_SUIT &&
           foundation_spades.size == CARDS_PER_SUIT;
}

void display_win_message() {
	char msg[120];
	sprintf(msg, "You win in %d moves!", counter.val);
	graph_text(VICTORY_MSG_X, VICTORY_MSG_Y, COLOR_VICTORY_MSG, msg, LARGE_FONT);
}

void refresh_deck() {
    while (discard_deck.size > 0) {
        card_t temp_card;
        pile_pop(&discard_deck, &temp_card);

        // Turn the card face down before adding it back to the deck
        card_show(temp_card, MARGEM, MARGEM_UP, false);

        pile_push(&deck, temp_card);
    }
}

// Método para determinar o índice da carta no tableau
int get_card_index_in_tableau(int x, int y, card_pile_t *tableau) {
    int relativeY = y - Y_MARGIN; // Usando Y_MARGIN como a margem vertical do topo do tableau

    int cardIndex = (relativeY - TABLEAU1_Y) / Y_SPACING; // Calculando índice da carta

    if (cardIndex < 0 || cardIndex >= tableau->size) {
        return -1; // Clique fora do intervalo de cartas ou em uma área vazia do tableau
    }

    return cardIndex;
}

bool can_move_discard_deck_to_tableaus(card_pile_t *discard_deck, card_pile_t tableaus[7]) {
    if (discard_deck->size == 0) {
        return false;  // Não há cartas no discard_deck para mover
    }
    card_t card_to_move = discard_deck->cards[discard_deck->size - 1];  // Carta do topo do discard_deck
    for (int i = 0; i < 7; i++) {
        if (can_move_to_tableau(card_to_move, &tableaus[i])) {
            return true;  // Encontrou um tableau onde a carta pode ser movida
        }
    }
    return false;  // Não encontrou um tableau adequado
}

int move_discard_deck_to_tableau(card_pile_t *discard_deck, card_pile_t tableaus[7]) {
    if (discard_deck->size == 0) {
        return -1;
    }
    card_t card_to_move;
    pile_pop(discard_deck, &card_to_move);
    // Vira a carta para cima antes de movê-la
    card_to_move.visible = true;
    for (int i = 0; i < 7; i++) {
        card_pile_t *tableau = &tableaus[i];
        if (can_move_to_tableau(card_to_move, tableau) || 
            (tableau->size == 0 && card_to_move.value == 12)) {  // Rei pode ser movido para um tableau vazio
            pile_push(tableau, card_to_move);
            return i; // Retorna o índice do tableau de destino
        }
    }
    // Se não foi possível mover a carta, coloque-a de volta no discard_deck
    pile_push(discard_deck, card_to_move);
    return -1;
}

void mouse_handler(MouseEvent me) {
    static card_t last_revealed_card;     // Armazena a última carta revelada
    static bool card_revealed = false;    // Indica se uma carta foi revelada
    if (me.type == MOUSE_BUTTON_EVENT && me.button == BUTTON_LEFT && me.state == BUTTON_PRESSED) {
    if (firstClick) {
        graph_regist_timer_handler(timer_handler, BASE_TIME);
        firstClick = 0;
        ct_show(&points);
    } else {
        ct_show(&points);
    }
        // Verifica se o clique foi no deck
        if (me.x >= MARGEM && me.x <= MARGEM + CARD_WIDTH && me.y >= MARGEM_UP && me.y <= MARGEM_UP + CARD_HEIGHT) {
            // Lógica para lidar com o clique no deck
             if (deck.size > 0) {
                pile_pop(&deck, &last_revealed_card);
                card_revealed = true;
				ct_inc(&points);
                card_show(last_revealed_card, DISCARD_DECK, MARGEM_UP, true);
				pile_push(&discard_deck, last_revealed_card);
            } else if (discard_deck.size > 0) {
                refresh_deck();
            } 
            ct_show(&points);
        }else if (card_revealed) {
            // Lógica para lidar com a movimentação de uma carta revelada do deck de descarte para a fundação
			card_pile_t *target_foundation = NULL;
			switch (last_revealed_card.suit) {
				case Hearts: target_foundation = &foundation_hearts; break;
				case Diamonds: target_foundation = &foundation_diamonds; break;
				case Spades: target_foundation = &foundation_spades; break;
				case Clubs: target_foundation = &foundation_clubs; break;
				default: 
					break; // Lidar com situação de naipe inválido
			}
			if (target_foundation != NULL && move_card_to_foundation(last_revealed_card, target_foundation)) {
				// Mover a carta para a fundação foi bem-sucedido
				// Se houver cartas no deck, revele a próxima
				if (deck.size > 0) {
					pile_pop(&deck, &last_revealed_card);
					card_show(last_revealed_card, DISCARD_DECK, MARGEM_UP, true);
					card_revealed = true;
				} else {
					// Não há mais cartas no deck
					card_revealed = false;
				}
			} else if(discard_deck.size > 0 && can_move_discard_deck_to_tableaus(&discard_deck, tableaus)) {      
			// Se uma carta foi revelada e pode ser movida para um tableau, faça a movimentação
				int tableau_index = move_discard_deck_to_tableau(&discard_deck, tableaus);
				if (tableau_index != -1) {
                // A carta foi movida para um tableau, então redesenhe o tableau de destino
                redraw_pile(&tableaus[tableau_index], tableau_index);
			// Atualizar a visualização do deck aqui, se necessári
			}
		} else{
            int clicked_tableau_index = get_tableau_index(me.x, me.y);
			if (clicked_tableau_index != -1) {
				card_pile_t *clicked_tableau = &tableaus[clicked_tableau_index];
				card_t top_card = clicked_tableau->cards[clicked_tableau->size - 1];		
				// Verifica se a carta do topo do tableau pode ser movida para a fundação
				bool moved_to_foundation = false;
				if (can_move_to_foundation(top_card, &foundation_hearts)) {
					moved_to_foundation = move_top_card_to_foundation(top_card, clicked_tableau, &foundation_hearts, clicked_tableau_index);
				} else if (can_move_to_foundation(top_card, &foundation_diamonds)) {
					moved_to_foundation = move_top_card_to_foundation(top_card, clicked_tableau, &foundation_diamonds, clicked_tableau_index);
				} else if (can_move_to_foundation(top_card, &foundation_spades)) {
					moved_to_foundation = move_top_card_to_foundation(top_card, clicked_tableau, &foundation_spades, clicked_tableau_index);
				} else if (can_move_to_foundation(top_card, &foundation_clubs)) {
					moved_to_foundation = move_top_card_to_foundation(top_card, clicked_tableau, &foundation_clubs, clicked_tableau_index);
				}
				if (moved_to_foundation) {
					ct_inc(&points);
					// Se a carta foi movida com sucesso para a fundação, atualize o tableau de origem
					clicked_tableau->size--;
					if (clicked_tableau->size > 0) {
						// A próxima carta do topo agora deve ser revelada
						reveal_top_card(clicked_tableau, clicked_tableau_index);
					}
					// Redesenhe o tableau afetado
					redraw_pile(clicked_tableau, clicked_tableau_index);
				} else {
						// Determina o tableau de origem baseado no clique
						int origin_tableau_index = get_tableau_index(me.x, me.y);
						if (origin_tableau_index != -1) {
							ct_inc(&points);
							card_pile_t *origin_tableau = &tableaus[origin_tableau_index];
							if (origin_tableau->size > 0) {
								card_t card_to_move = origin_tableau->cards[origin_tableau->size - 1];
								//can_move_to_tableau(card_to_move, &tableaus[i]);
								// Tenta encontrar um tableau de destino válido
								for (int i = 0; i < 7; i++) {
									if (i != origin_tableau_index) {
										card_pile_t *destination_tableau = &tableaus[i];
										if (can_move_to_tableau(card_to_move, destination_tableau)) {
											move_card_sequence(origin_tableau, destination_tableau, origin_tableau->size - 1, origin_tableau_index, i);
											// Atualiza o tableau de origem
											if (origin_tableau->size > 0) {
												reveal_top_card(origin_tableau, origin_tableau_index);
											}
											redraw_pile(origin_tableau, origin_tableau_index);
											redraw_pile(destination_tableau, i);
											break; // Sai do loop uma vez que a carta foi movida
										}
									}
								}
							}
						}
					}             
                }
            }
        }
        // Verificação de condição de vitória
        if (check_win_condition()) {
            display_win_message();
        }
    }
}
          
int main() {
    graph_init2("Decks", WINDOW_WIDTH, WINDOW_HEIGHT);
    init();
    graph_regist_mouse_handler(mouse_handler);
    graph_regist_key_handler(key_handler);
    graph_start();
    return 0;
}
