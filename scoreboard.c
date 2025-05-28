// Cricket Score Tracking System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PLAYERS 11
#define MAX_OVERS 10
#define MAX_OVERS_PER_BOWLER 2

// ANSI colors for better terminal UI
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

// Structure for player
struct Player {
    char name[50];
    int runs;
    int balls;
    int fours;
    int sixes;
    int isOut;
    int onStrike;
};

// Structure for bowler
struct Bowler {
    char name[50];
    int ballsBowled;
    int runsConceded;
    int oversBowled;
};

// Structure for team
struct Team {
    char name[50];
    struct Player players[MAX_PLAYERS];
    struct Bowler bowlers[5];
    int totalRuns;
    int wickets;
    int overs;
    int ballsInOver;
    int bowlerCount;
};

struct Team team1, team2;
struct Team *battingTeam, *bowlingTeam;
struct Bowler *currentBowler;
char overSummary[2][MAX_OVERS][6];
int currentInning = 1;
FILE *csvFile;

void initializeTeam(struct Team *team) {
    printf(CYAN "Enter Team Name: " RESET);
    scanf("%s", team->name);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("Enter player %d name: ", i + 1);
        scanf("%s", team->players[i].name);
        team->players[i].runs = 0;
        team->players[i].balls = 0;
        team->players[i].fours = 0;
        team->players[i].sixes = 0;
        team->players[i].isOut = 0;
        team->players[i].onStrike = 0;
    }
    team->totalRuns = 0;
    team->wickets = 0;
    team->overs = 0;
    team->ballsInOver = 0;
    team->bowlerCount = 0;
}

void printPlayers(struct Team *team) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("%d. %s\n", i, team->players[i].name);
    }
}

struct Bowler* getBowler(struct Team *team, const char *name) {
    for (int i = 0; i < team->bowlerCount; i++) {
        if (strcmp(team->bowlers[i].name, name) == 0) {
            return &team->bowlers[i];
        }
    }
    strcpy(team->bowlers[team->bowlerCount].name, name);
    team->bowlers[team->bowlerCount].ballsBowled = 0;
    team->bowlers[team->bowlerCount].runsConceded = 0;
    team->bowlers[team->bowlerCount].oversBowled = 0;
    return &team->bowlers[team->bowlerCount++];
}

void setBowler() {
    char bowlerName[50];
    int valid = 0;
    while (!valid) {
        printf("\nSelect Bowler: ");
        scanf("%s", bowlerName);
        currentBowler = getBowler(bowlingTeam, bowlerName);
        if (currentBowler->oversBowled < MAX_OVERS_PER_BOWLER) valid = 1;
        else printf(RED "\nBowler has already bowled %d overs! Choose another.\n" RESET, MAX_OVERS_PER_BOWLER);
    }
}

void swapStrike(int *striker, int *nonStriker) {
    int temp = *striker;
    *striker = *nonStriker;
    *nonStriker = temp;
}

void updateScore(int *striker, int *nonStriker, int runs) {
    if (runs < 0 || (runs > 3 && runs != 4 && runs != 6)) {
        printf(RED "Invalid run value! Only 0,1,2,3,4,6 allowed.\n" RESET);
        return;
    }
    struct Player *batsman = &battingTeam->players[*striker];
    batsman->runs += runs;
    batsman->balls++;
    if (runs == 4) batsman->fours++;
    if (runs == 6) batsman->sixes++;

    battingTeam->totalRuns += runs;
    battingTeam->ballsInOver++;

    currentBowler->runsConceded += runs;
    currentBowler->ballsBowled++;

    overSummary[currentInning - 1][battingTeam->overs][battingTeam->ballsInOver - 1] = (runs == 0) ? '0' : ('0' + runs);

    if (runs % 2 != 0) swapStrike(striker, nonStriker);

    if (battingTeam->ballsInOver == 6) {
        battingTeam->overs++;
        battingTeam->ballsInOver = 0;
        currentBowler->oversBowled++;
        setBowler();
        swapStrike(striker, nonStriker);
    }
}

void fallOfWicket(int *striker, int *nextPlayer) {
    battingTeam->players[*striker].isOut = 1;
    battingTeam->players[*striker].balls++;
    battingTeam->wickets++;
    battingTeam->ballsInOver++;
    currentBowler->ballsBowled++;

    overSummary[currentInning - 1][battingTeam->overs][battingTeam->ballsInOver - 1] = 'W';

    *striker = *nextPlayer;
    (*nextPlayer)++;

    if (battingTeam->ballsInOver == 6) {
        battingTeam->overs++;
        battingTeam->ballsInOver = 0;
        currentBowler->oversBowled++;
        setBowler();
        swapStrike(striker, nextPlayer);
    }
}

float calculateStrikeRate(int runs, int balls) {
    if (balls == 0) return 0.0;
    return ((float)runs / balls) * 100;
}

float calculateEconomy(int runs, int balls) {
    if (balls == 0) return 0.0;
    return ((float)runs / balls) * 6;
}

void writeCSVHeader() {
    fprintf(csvFile, "Inning,Team,Player,Runs,Balls,4s,6s,StrikeRate,Status\n");
}

void saveScoreboardToCSV(int striker) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        char status[15];
        if (battingTeam->players[i].isOut)
            strcpy(status, "Out");
        else if (i == striker)
            strcpy(status, "Not Out*");
        else if (battingTeam->players[i].balls > 0)
            strcpy(status, "Not Out");
        else
            strcpy(status, "-");

        fprintf(csvFile, "%d,%s,%s,%d,%d,%d,%d,%.2f,%s\n",
                currentInning,
                battingTeam->name,
                battingTeam->players[i].name,
                battingTeam->players[i].runs,
                battingTeam->players[i].balls,
                battingTeam->players[i].fours,
                battingTeam->players[i].sixes,
                calculateStrikeRate(battingTeam->players[i].runs, battingTeam->players[i].balls),
                status);
    }
    fprintf(csvFile, "\nInning,%d,%s,Bowler Stats,,\n", currentInning, bowlingTeam->name);
    fprintf(csvFile, "Bowler,RunsConceded,BallsBowled,Overs,Economy\n");
    for (int i = 0; i < bowlingTeam->bowlerCount; i++) {
        struct Bowler *b = &bowlingTeam->bowlers[i];
        fprintf(csvFile, "%s,%d,%d,%d,%.2f\n",
                b->name, b->runsConceded, b->ballsBowled, b->oversBowled, calculateEconomy(b->runsConceded, b->ballsBowled));
    }
    fprintf(csvFile, "\n");
}

void displayScoreboard(int striker) {
    printf(CYAN "\n--- %s Scoreboard ---\n" RESET, battingTeam->name);
    printf("%-15s%5s%6s%5s%5s%8s%10s\n", "Player", "Runs", "Balls", "4s", "6s", "SR", "Status");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        char status[15];
        if (battingTeam->players[i].isOut)
            strcpy(status, "Out");
        else if (i == striker)
            strcpy(status, "Not Out*");
        else if (battingTeam->players[i].balls > 0)
            strcpy(status, "Not Out");
        else
            strcpy(status, "-");

        printf("%-15s%5d%6d%5d%5d%8.2f%10s\n",
               battingTeam->players[i].name,
               battingTeam->players[i].runs,
               battingTeam->players[i].balls,
               battingTeam->players[i].fours,
               battingTeam->players[i].sixes,
               calculateStrikeRate(battingTeam->players[i].runs, battingTeam->players[i].balls),
               status);
    }
    printf("\nTotal: %d/%d in %d.%d overs\n", battingTeam->totalRuns, battingTeam->wickets, battingTeam->overs, battingTeam->ballsInOver);

    printf("\nBowler Stats:\n");
    for (int i = 0; i < bowlingTeam->bowlerCount; i++) {
        struct Bowler *b = &bowlingTeam->bowlers[i];
        printf("%s - Runs: %d, Balls: %d, Overs: %d, Econ: %.2f\n",
               b->name, b->runsConceded, b->ballsBowled, b->oversBowled, calculateEconomy(b->runsConceded, b->ballsBowled));
    }

    printf("\nOver Summary:\n");
    for (int i = 0; i < battingTeam->overs + (battingTeam->ballsInOver > 0 ? 1 : 0); i++) {
        printf("Over %d: ", i + 1);
        for (int j = 0; j < 6; j++) {
            if (overSummary[currentInning - 1][i][j] != '\0') printf("%c ", overSummary[currentInning - 1][i][j]);
        }
        printf("\n");
    }
}

void playInning(struct Team *batting, struct Team *bowling) {
    battingTeam = batting;
    bowlingTeam = bowling;

    int striker, nonStriker, nextPlayer;
    printf("Select striker:\n");
    printPlayers(batting);
    scanf("%d", &striker);
    printf("Select non-striker:\n");
    printPlayers(batting);
    scanf("%d", &nonStriker);

    nextPlayer = 0;
    while (nextPlayer == striker || nextPlayer == nonStriker) nextPlayer++;

    battingTeam->players[striker].onStrike = 1;
    setBowler();

    int choice;
    while (battingTeam->wickets < 10 && battingTeam->overs < MAX_OVERS) {
        printf(YELLOW "\nStriker: %s | Bowler: %s\n" RESET, battingTeam->players[striker].name, currentBowler->name);
        printf(CYAN "1. Add Runs\n2. Wicket\n3. Show Scoreboard\n4. Exit\nEnter your choice: " RESET);
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                int runs;
                printf("Enter runs scored by %s (0,1,2,3,4,6): ", battingTeam->players[striker].name);
                scanf("%d", &runs);
                updateScore(&striker, &nonStriker, runs);
                displayScoreboard(striker);
                break;
            }
            case 2:
                fallOfWicket(&striker, &nextPlayer);
                displayScoreboard(striker);
                break;
            case 3:
                displayScoreboard(striker);
                break;
            case 4:
                return;
            default:
                printf(RED "Invalid choice!\n" RESET);
        }
    }
    displayScoreboard(striker);
    saveScoreboardToCSV(striker);
}

int main() {
    csvFile = fopen("match_scoreboard.csv", "w");
    if (!csvFile) {
        printf(RED "Unable to create scoreboard CSV file!\n" RESET);
        return 1;
    }
    writeCSVHeader();

    printf(CYAN "--- Enter Batting Team Details ---\n" RESET);
    initializeTeam(&team1);
    printf(CYAN "--- Enter Bowling Team Details ---\n" RESET);
    initializeTeam(&team2);

    printf(GREEN "\n--- First Innings: %s Batting ---\n" RESET, team1.name);
    currentInning = 1;
    playInning(&team1, &team2);

    printf(GREEN "\n--- Second Innings: %s Batting ---\n" RESET, team2.name);
    currentInning = 2;
    playInning(&team2, &team1);

    fclose(csvFile);

    printf("\n\n");
    printf("\033[1;32m================== MATCH RESULT ==================\033[0m\n");
    if (team1.totalRuns > team2.totalRuns) {
        printf(GREEN "%s wins by %d runs!\n" RESET, team1.name, team1.totalRuns - team2.totalRuns);
    } else if (team2.totalRuns > team1.totalRuns) {
        printf(GREEN "%s wins by %d wickets!\n" RESET, team2.name, 10 - team2.wickets);
    } else {
        printf(YELLOW "Match Tied!\n" RESET);
    }

    struct Player *mvp = NULL;
    int maxImpact = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        int impact = team1.players[i].runs + team1.players[i].fours * 2 + team1.players[i].sixes * 3;
        if (impact > maxImpact) {
            maxImpact = impact;
            mvp = &team1.players[i];
        }
        impact = team2.players[i].runs + team2.players[i].fours * 2 + team2.players[i].sixes * 3;
        if (impact > maxImpact) {
            maxImpact = impact;
            mvp = &team2.players[i];
        }
    }

    if (mvp) printf(CYAN "\nMVP of the match: %s with %d runs!\n" RESET, mvp->name, mvp->runs);

    return 0;
}
