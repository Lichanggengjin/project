#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "player.h"
void getPattern(const char* guess, const char* target, char* pattern) {
	char tempTarget[WORD_LENGTH + 1];
	strcpy(tempTarget, target);

	for (int i = 0; i < WORD_LENGTH; i++) {
		if (guess[i] == target[i]) {
			pattern[i] = 'G';
			tempTarget[i] = '*';
		} else {
			pattern[i] = 'B';
		}
	}
	for (int i = 0; i < WORD_LENGTH; i++) {
		if (pattern[i] == 'B') {
			for (int j = 0; j < WORD_LENGTH; j++) {
				if (guess[i] == tempTarget[j]) {
					pattern[i] = 'Y';
					tempTarget[j] = '*';
					break;
				}
			}
		}
	}
	pattern[WORD_LENGTH] = '\0';
}
double calculateEntropy(const char* word, char possibleAnswers[][WORD_LENGTH + 1], int answercnt) {
    typedef struct {
        char pattern[WORD_LENGTH + 1];
        int count;
    } Pattern;
    Pattern patterns[243]; 
    int patterncnt = 0;
    double entropy = 0.0;
    int patternHash[243] = {0};

    for (int i = 0; i < answercnt; i++) {
        char pattern[WORD_LENGTH + 1];
        getPattern(word, possibleAnswers[i], pattern);
        int hashValue = 0;
        for (int k = 0; k < WORD_LENGTH; k++) {
            hashValue = hashValue * 3 + (pattern[k] == 'G' ? 2 : (pattern[k] == 'Y' ? 1 : 0));
        }

        if (patternHash[hashValue] == 0) {
            strcpy(patterns[patterncnt].pattern, pattern);
            patterns[patterncnt].count = 1;
            patternHash[hashValue] = patterncnt + 1;
            patterncnt++;
        } else {
            patterns[patternHash[hashValue] - 1].count++;
        }
    }

    for (int i = 0; i < patterncnt; i++) {
        double probability = (double)patterns[i].count / answercnt;
        if (probability > 0) { 
            entropy -= probability * log2(probability);
        }
    }
    return entropy;
}
char* getBestGuess(char wordList[][WORD_LENGTH + 1], int wordcnt,char possibleAnswers[][WORD_LENGTH + 1], int answercnt) {
	static char bestWord[WORD_LENGTH + 1];
    double maxEntropy = -1.0;
    if (answercnt == 2315) {
        strcpy(bestWord, "slate");
        return bestWord;
    }
    double entropies[MAX_SOLUTIONS] = {0.0};

    for (int i = 0; i < answercnt; i++) {
        if (entropies[i] == 0.0) {
            entropies[i] = calculateEntropy(possibleAnswers[i], possibleAnswers, answercnt);
        }
        double entropy = entropies[i];
        if (entropy > maxEntropy) {
            maxEntropy = entropy;
            strcpy(bestWord, possibleAnswers[i]);
        }
    }
    return bestWord;
}
char* player_AI(const char* target) {
	static char possibleAnswers[MAX_SOLUTIONS][WORD_LENGTH + 1];
	char (*source)[WORD_LENGTH + 1] = &wordList[MAX_WORDS - MAX_SOLUTIONS];
	static int answercnt = 0;
	static char currentGuess[WORD_LENGTH + 1];
	if (firstGuess) {
		memcpy(possibleAnswers, source, sizeof(char) *MAX_SOLUTIONS * (WORD_LENGTH + 1));
		answercnt = MAX_SOLUTIONS;
		firstGuess = false;
		strcpy(currentGuess, "salet");
	} else {
		char pattern[WORD_LENGTH + 1];
		getPattern(currentGuess, target, pattern);
		int newAnswercnt = 0;
		for (int i = 0; i < answercnt; i++) {
			char testPattern[WORD_LENGTH + 1];
			getPattern(currentGuess, possibleAnswers[i], testPattern);
			if (strcmp(pattern, testPattern) == 0) {
				if (newAnswercnt != i) {
					strcpy(possibleAnswers[newAnswercnt], possibleAnswers[i]);
				}
				newAnswercnt++;
			}
		}
		answercnt = newAnswercnt;
		strcpy(currentGuess, getBestGuess(wordList, wordcnt, possibleAnswers, answercnt));
	}
	return currentGuess;
}
void humanPlay(const char* target) {
	char guess[WORD_LENGTH + 1];
	char res[WORD_LENGTH + 1];
	int att = 0;

	while (att < MAX_ATTEMPTS) {
		printf("第%d次尝试): ", att + 1);
		scanf("%s", guess);
		if (strlen(guess) != WORD_LENGTH) {
			printf("不合法\n");
			continue;
		}
		int valid = 0;
		for (int i = 0; i < wordcnt; i++) {
			if (strcmp(wordList[i], guess) == 0) {
				valid = 1;
				break;
			}
		}
		if (!valid) {
			printf("不在词库中\n");
			continue;
		}

		check_word(target, guess, res);
		printf("结果: %s\n", res);

		if (strcmp(res, "GGGGG") == 0) {
			printf("尝试次数为%d次\n", att + 1);
			return;
		}
		att++;
	}
	printf("结果: %s\n", target);
}

void AIPlay(const char* target) {
	int att = 0;
	char res[WORD_LENGTH + 1];
	char* guess;
	FILE* output = fopen("game_ress.txt", "a");
	firstGuess = true;  

	while (att < MAX_ATTEMPTS) {
		guess = player_AI(target);
		check_word(target, guess, res);

		printf("AI第%d次尝试: %s -> %s\n", att + 1, guess, res);

		if (strcmp(res, "GGGGG") == 0) {
			printf("AI成绩为%d \n", att + 1);
			fprintf(output, "AI成绩为 %d \n\n", att + 1);
			fclose(output);
			return;
		}
		att++;
	}
	fclose(output);
}