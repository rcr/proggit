#include <stdio.h>

#define HANDS 23
#define SF 0x900000
#define FK 0x800000
#define FH 0x700000
#define FL 0x600000
#define ST 0x500000
#define TK 0x400000
#define TP 0x300000
#define OP 0x200000
#define HC 0x100000

#define CMP_SWAP(i, j) if (a[i] > a[j]) { \
	tmp = a[i]; a[i] = a[j]; a[j] = tmp; \
	tmp = b[i]; b[i] = b[j]; b[j] = tmp; }

void
net_sort(int *a, int *b)
{
	int tmp;
	CMP_SWAP(6,4); CMP_SWAP(3,1); CMP_SWAP(5,0); CMP_SWAP(4,2);
	CMP_SWAP(1,0); CMP_SWAP(5,3); CMP_SWAP(2,0); CMP_SWAP(6,4);
	CMP_SWAP(2,1); CMP_SWAP(4,3); CMP_SWAP(6,5); CMP_SWAP(5,2);
	CMP_SWAP(3,1); CMP_SWAP(3,2); CMP_SWAP(5,4); CMP_SWAP(4,3);
}

int
get_rank(char c)
{
	switch(c){
		case 'A': return 14;
		case 'K': return 13;
		case 'Q': return 12;
		case 'J': return 11;
		case 'T': return 10;
		default: return c - '0';
	}
}

int
get_suit(char c)
{
	switch(c){
		case 'c': return 1;
		case 'd': return 2;
		case 'h': return 3;
		case 's': return 4;
		default: return 0;
	}
}

int
is_straight(int *r)
{
	int i, j, k, straight = 0;
	for (i = 0; i < 4 && !straight; i++){
		for (k = i, j = i+1; j < 7; j++){
			if (r[j] == r[k])
				continue;
			else if ((r[k] - r[j]) == 1){
				k = j;
				if (r[k] == (r[i] - 4))
					straight = r[i];
				else if (r[i] == 5 && r[0] == 14 && r[k] == 2)
					straight = 3;
			}
			else break;
		}
	}
	return straight;
}

int
rank(char *hand)
{
	int i, j, straight, ranks[7], suits[7];

	/* Read */
	int suit_count[5] = {0}, flush = 0;
	for (i = 0; i < 7; i++){
		ranks[i] = get_rank(hand[i*3]);
		suits[i] = get_suit(hand[i*3 + 1]);
		
		if (++suit_count[suits[i]] == 5)
			flush = suits[i];
	}

	net_sort(ranks, suits);

	/* Flush Highcards */
	int flush_s[7] = {0};
	if (flush){
		for (i = j = 0; i < 7; i++){
			if (suits[i] == flush)
				flush_s[j++] = ranks[i];
		}
	}

	if (flush && (straight = is_straight(flush_s)))
		return SF | straight;
	
	/* FourKind, ThreeKind, HighPair, LowPair */
	int match, pairs[4] = {0};
	for (i = 0; i < 6; i++){
		match = 0;
		for (j = i+1; j < 7; j++){
			if (ranks[i] == ranks[j])
				match++;
			else break;
		}
		if (match == 3)
			pairs[0] = ranks[i];
		else if (match == 2)
			if (!pairs[1])
				pairs[1] = ranks[i];
			else
				pairs[2] = ranks[i];
		else if (match == 1){
			if (!pairs[2])
				pairs[2] = ranks[i];
			else if (!pairs[3])
				pairs[3] = ranks[i];
		}
		i+=match;
	}

	/* Pair High Cards */
	int highs[3] = {0};
	for (i = j = 0; i < 7; i++){
		if (ranks[i] != pairs[0] && ranks[i] != pairs[2] &&
			ranks[i] != pairs[1] && ranks[i] != pairs[3])
			highs[j++] = ranks[i];
	}

	if (pairs[0])
		return FK | 0x10*ranks[0] | highs[0];
	
	if (pairs[1] && pairs[2])
		return FH | 0x10*pairs[1] | pairs[2];
	
	if (flush)
		return FL | 0x10000*flush_s[0] | 0x1000*flush_s[1] | 
			0x100*flush_s[2] | 0x10*flush_s[3] | flush_s[4];

	if ((straight = is_straight(ranks)))
		return ST | straight;

	if (pairs[1])
		return TK | 0x100*pairs[1] | 0x10*highs[0] | highs[1];

	if (pairs[2] && pairs[3])
		return TP | 0x100*pairs[2] | 0x10*pairs[3] | highs[0];

	if (pairs[2])
		return OP | 0x1000*pairs[2] | 0x100*highs[0] | 
			0x10*highs[1] | highs[2];

	return HC | 0x10000*ranks[0] | 0x1000*ranks[1] | 
		0x100*ranks[2] | 0x10*ranks[3] | ranks[4];
}

int
main(int argc, char **argv)
{
	char hand[20];
	int i, temp, rounds, hands, high, winner_i, winner[HANDS];

	FILE *fp;
	if ((fp = fopen(argv[1], "r")) == NULL)
		return 1;

	for (fscanf(fp, "%d", &rounds); rounds > 0; rounds--){

		fscanf(fp, "%d\n", &hands);
		fread(hand, sizeof(char), 15, fp);

		for (winner_i = high = i = 0; i < hands; i++){

			fread(&hand[15], sizeof(char), 6, fp);

			if ((temp = rank(hand)) > high){
				winner[0] = i;
				winner_i = 0;
				high = temp;
			} else if (temp == high)
				winner[++winner_i] = i;
		}

		for (i = 0; i < winner_i; i++)
			printf("%d ", winner[i]);
		printf("%d\n", winner[winner_i]);
	}
	return 0;
} 
