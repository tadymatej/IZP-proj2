/********************************************************************************************************************************************************/
/*                                                    Projekt 2 - práce s datovými strukturami                                                          */
/*                                                                                                                                                      */
/*                                                                                                                                                      */
/*                                                                   Verze: 1.0                                                                         */
/*                                                              Autor: Žalmánek Matěj                                                                   */
/*                                                               VUT login: xzalma00                                                                    */
/*                                                        Datum: Listopad - Prosinec 2020                                                               */
/*                                                                                                                                                      */
/*                                                                                                                                                      */
/********************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERR_MISSING_ARGUMENTS "Zadali jste malo argumentu pro tento program!"
#define ERR_MISSING_FILE "Chybi zadani souboru, s kterym se ma pracovat!"
#define ERR_WRITE_TO_MEM_FAILURE "Při pokusu o zápis do paměti nastala chyba, program byl ukončen"
#define ERR_OPENING_FILE "Nepovedlo se otevřít soubor, program byl ukončen"
#define ERR_ROW_SELECTION_MSG "Zadali jste neplatné číslo řádku"
#define ERR_COL_SELECTION_MSG "Zadali jste neplatné číslo sloupce"
#define ERR_WRONG_ARGUMENT_MSG "Zadaný argument v CMDSEQUENCE není validní!"
#define ERR_COMMAND_NOT_RECOGNISED_MSG "Zadaný příkaz nebyl rozpoznán!"

#define MAX_LENGTH_DOUBLE_STRING 20
#define MAX_LENGTH_INT_STRING 15
#define INT_PARAMS_COUNT 4
#define DEFAULT_DELIM_LENGTH 10
#define MIN_ARGC2 5
#define MIN_ARGC 3
#define NUM_OF__VARS 10
#define MAX_CMD_SEQUENCE_PARAMS 5
#define DEFAULT_FILE_POS 2
#define DEFAULT_CMD_POS 1
#define SELECTED_ALL_COLS 10

//Čísla identifikující typ chyby
enum CmdErrs {
    WRONG_ARGUMENT = 2,
    COMMAND_NOT_RECOGNISED,
    WRONG_COL_SELECTION,
    WRONG_ROW_SELECTION
};

//Čísla identifikující příkaz
enum cmds {
    SEL_ROW,
    SEL_COL,
    SEL_SUBTABLE,
    SEL_MIN,
    SEL_MAX,
    SEL_FIND,
    SEL_TMP,
    IROW,
    AROW,
    DROW,
    ICOL,
    ACOL,
    DCOL,
    SET_SEL,
    CLEAR,
    SWAP,
    SUM,
    AVG,
    COUNT,
    LEN,
    DEF,
    USE,
    INC,
    SET
};

enum directions {
    LEFT,
    RIGHT
};

typedef struct {
    char *delimStr;
    int delka;
} Delim;

/* Struktura sdružující informace o buňce (sloupci) */
typedef struct {
    char *retezec;
    int size;
    int capacity;
} Bunka;

/* Struktura sdružující informace o řádku */
typedef struct {
    Bunka *bunky;
    int pocetBunek;
    int capacity;
} Radek;

/* Struktura sdružující informace o tabulce */
typedef struct {
    Radek *radky;
    int pocetRadku;
    int capacity;
} Tabulka;

typedef Bunka StrParam; //Má stejné datové uvnitř v sobě, ale pro přehlednější kód později

typedef struct {
    int id;
    int intParams[INT_PARAMS_COUNT];
    int numIntParams;
    Bunka strParam;
} Command;

typedef struct {
    int rowStart;
    int rowEnd;
    int colStart;
    int colEnd;
} Selection;

/* Zjistí, zda znak je obsazen v řetězci delim */
bool isDelim(char znak, char *delim) {
    for (int i = 0; delim[i] != '\0'; ++i)
        if (delim[i] == znak)
            return true;
    return false;
}

/* Zjistí, zda string obsahuje substring contains */
bool containsStr(char *string, char *contains) {
    for (int i = 0; string[i] != '\0'; ++i)
    {
        if (string[i] == contains[0]) {
            bool obsahuje = true;
            for (int j = 1; contains[j] != '\0'; ++j)
            {
                if (string[i + j] != contains[j])
                    obsahuje = false;
            }
            if (obsahuje)
                return true;
        }
    }
    return false;
}

/*Zjistí, zda řetězec obsahuje znak (od první pozice po poslední pozici ve stringu) */
bool containsCharFullString(char *string, char znak) {
    for (int i = 0; string[i] != '\0'; ++i)
        if (string[i] == znak)
            return true;
    return false;
}

/* Vrátí počet buněk v řádku */
int getRadekPocetBunek(Radek radek) {
    return radek.pocetBunek - 1;
}

/* Zkopíruje řetězec do druhého řetězce (hloubková kopie po znacích) */
void copyStrToStr(char *stringCopyTo, char *stringSaveTo) {
    int i = 0;
    for (; stringCopyTo[i] != '\0'; ++i)
        stringSaveTo[i] = stringCopyTo[i];
    stringSaveTo[i] = '\0';
}

/* Vypíše buňku */
void printBunkaToFile(Bunka bunka, FILE *file) {
    fprintf(file, "%s", bunka.retezec);
}

/* Dealokuje všechny mallocy v rámci buňky + nastaví defaultní hodnoty proměnných */
void destroyBunka(Bunka *bunka) {
    if (bunka != NULL && bunka->retezec != NULL) {
        free(bunka->retezec);
        bunka->retezec = NULL;
        bunka->size = 0;
        bunka->capacity = 0;
    }
    bunka = NULL;
}

/* Vloží znak do řetězce v buňce */
int insertCharToBunka(Bunka *bunka, char znak) {
    if (bunka->size + 1 > bunka->capacity) {
        void *tmp = realloc(bunka->retezec, sizeof(char) * (bunka->capacity * 2));
        if (tmp == NULL) {
            destroyBunka(bunka);
            return -1;
        }
        bunka->capacity *= 2;
        bunka->retezec = (char *)tmp;
    }

    bunka->retezec[bunka->size - 1] = znak;
    bunka->retezec[bunka->size] = '\0';
    bunka->size++;
    return 0;
}

/* Nastaví řetězec do buňky */
int setBunka(Bunka *bunka, char *string) {
    bunka->size = 1;
    bunka->retezec[0] = '\0';
    for (int i = 0; string[i] != '\0'; ++i)
        insertCharToBunka(bunka, string[i]);
    return 0;
}

void bunkaRemoveChar(Bunka *bunka, char pos)
{
    for (int i = pos; i < bunka->size; ++i)
    {
        bunka->retezec[i] = bunka->retezec[i + 1];
    }
    bunka->size--;
}

int bunkaInsertChar(Bunka *bunka, int pos, char insert) {
    if (bunka->size + 1 > bunka->capacity) {
        void *tmp = realloc(bunka->retezec, sizeof(char) * (bunka->capacity * 2));
        if (tmp == NULL) {
            destroyBunka(bunka);
            return -1;
        }
        bunka->capacity *= 2;
        bunka->retezec = (char *)tmp;
    }
    for (int i = bunka->size; i > pos; --i)
    {
        bunka->retezec[i] = bunka->retezec[i - 1];
    }
    bunka->retezec[pos] = insert;
    bunka->size++;
    return EXIT_SUCCESS;
}

/* Inicializuje proměnné v buňce na defaultní hodnoty */
int initBunka(Bunka *bunka) {
    bunka->retezec = (char *)malloc(sizeof(char) * 1);
    if (bunka->retezec == NULL)
        return -1;
    bunka->capacity = 1;
    bunka->retezec[0] = '\0';
    bunka->size = 1;
    return 0;
}

/* Vytvoří buňku */
Bunka createBunka() {
    Bunka bunka;
    initBunka(&bunka);
    return bunka;
}

void initCommand(Command *command, int id) {
    command->id = id;
    command->numIntParams = 0;
    Bunka bunka = {NULL, 0, 0};
    command->strParam = bunka;
    for (int i = 0; i < INT_PARAMS_COUNT; ++i)
        command->intParams[i] = 0;
}

Command createCommand(int id) {
    Command command;
    initCommand(&command, id);
    return command;
}

/* Inicializuje řádek na defaultní hodnoty proměnných */
int initRadek(Radek *radek) {
    radek->bunky = (Bunka *)malloc(sizeof(Bunka));
    if (radek->bunky == NULL)
        return -1;
    radek->pocetBunek = 0;
    radek->capacity = 1;
    return 0;
}

/* Vytvoří řádek */
Radek createRadek() {
    Radek radek;
    initRadek(&radek);
    return radek;
}

/* dealokuje všechny mallocy v rámci řádku a nastaví proměnné do defaultních hodnot */
void destroyRadek(Radek *radek) {
    if (radek != NULL && radek->bunky != NULL) {
        for (int i = 0; i < radek->pocetBunek; ++i)
        {
            destroyBunka(&radek->bunky[i]);
        }
        free(radek->bunky);
        radek->capacity = 0;
        radek->pocetBunek = 0;
    }
    radek = NULL;
}

/* Opraví řetězec buňky do požadovaného výstupního formátu */
void controllBunka(Bunka *bunka, char *delim) {
    if (bunka->retezec[0] == '"' && bunka->retezec[bunka->size - 2] == '"') { //Buňka má na začátku a na konci "
        bool remove = true;
        for (int i = 1; i < bunka->size - 2; ++i)
        {
            if (bunka->retezec[i] == '"' || isDelim(bunka->retezec[i], delim)) 
                remove = false;
            if (bunka->retezec[i] == '\\' && bunka->retezec[i + 1] != '"') {
                bunkaRemoveChar(bunka, i);
                --i;
            }
        }
        if (remove) {
            bunkaRemoveChar(bunka, 0);
            bunkaRemoveChar(bunka, bunka->size - 2);
        }
    }
    else {
        bool insert = false;
        for (int i = 0; i < bunka->size - 1; ++i)
        {
            if (bunka->retezec[i] == '"') {
                insert = true;
                bunkaInsertChar(bunka, i, '\\');
                ++i;
            }
            else if (isDelim(bunka->retezec[i], delim))
                insert = true;
            if (bunka->retezec[i] == '\\' && bunka->retezec[i + 1] != '"') {
                bunkaRemoveChar(bunka, i);
                --i;
            }
        }
        if (insert) {
            bunkaInsertChar(bunka, 0, '"');
            bunkaInsertChar(bunka, bunka->size - 1, '"');
        }
    }
}

/* Přidá do řádku novou buňku */
int radekAddBunka(Radek *radek, Bunka bunka) {
    if (radek->pocetBunek + 1 > radek->capacity) {
        void *tmp = realloc(radek->bunky, sizeof(Bunka) * (radek->capacity * 2));
        if (tmp == NULL) {
            destroyRadek(radek);
            return -1;
        }
        radek->bunky = (Bunka *)tmp;
        radek->capacity *= 2;
    }
    radek->bunky[radek->pocetBunek] = bunka;
    (radek->pocetBunek)++;
    return 0;
}

/* Vypíše řádek */
void printRadekToFile(Radek radek, FILE *file, char oddelovac) {
    if (radek.pocetBunek <= 0) {
        fprintf(file, "\n");
        return;
    }
    printBunkaToFile(radek.bunky[0], file);
    for (int i = 1; i < radek.pocetBunek; ++i)
    {
        fprintf(file, "%c", oddelovac);
        printBunkaToFile(radek.bunky[i], file);
    }
    fprintf(file, "\n");
}

/* Najde největší počet buněk v řádku z řádků z celé tabulky */
int tableFindMaxCols(Tabulka table) {
    int maxCols = getRadekPocetBunek(table.radky[0]);
    for (int i = 1; i < table.pocetRadku; ++i)
    {
        int actualRowCols = getRadekPocetBunek(table.radky[i]);
        if (actualRowCols > maxCols)
            maxCols = actualRowCols;
    }
    return maxCols;
}

/* Doplnění všech řádků v tabulce na stejný počet buněk */
int tableEqualizeCols(Tabulka *tabulka, int pocetCols) {
    for (int i = 0; i < tabulka->pocetRadku; ++i)
    {
        for (int j = tabulka->radky[i].pocetBunek; j <= pocetCols; ++j)
            radekAddBunka(&tabulka->radky[i], createBunka());
    }
    return 0;
}

/* Dealokuje všechny mallocy v rámci tabulky a nastaví defaultní hodnoty ostatním proměnným */
void destroyTabulka(Tabulka *tabulka) {
    if (tabulka != NULL && tabulka->radky != NULL) {
        for (int i = 0; i < tabulka->pocetRadku; ++i)
        {
            destroyRadek(&tabulka->radky[i]);
        }
        free(tabulka->radky);
        tabulka->radky = NULL;
        tabulka->capacity = 0;
        tabulka->pocetRadku = 0;
    }
    tabulka = NULL;
}

/* Inicializuje proměnné v tabulce na defaultní hodnoty */
int initTabulka(Tabulka *tabulka) {
    tabulka->radky = (Radek *)malloc(sizeof(Radek));
    if (tabulka->radky == NULL) {
        destroyTabulka(tabulka);
        return -1;
    }
    tabulka->pocetRadku = 0;
    tabulka->capacity = 1;
    return 0;
}

/* Přidá řádek do tabulky */
int tabulkaAddRow(Tabulka *tabulka, Radek radek) {
    if (tabulka->pocetRadku + 1 > tabulka->capacity) {
        void *tmp = realloc(tabulka->radky, sizeof(Radek) * (tabulka->capacity * 2));
        if (tmp == NULL) {
            destroyTabulka(tabulka);
            return -1;
        }
        tabulka->radky = (Radek *)tmp;
        tabulka->capacity *= 2;
    }
    tabulka->radky[tabulka->pocetRadku] = radek;
    ++(tabulka->pocetRadku);
    return 0;
}

/* Posune řádky v tabulce o jedno v daném směru */
void tablePosunRadku(Tabulka *table, int start, int direction) {
    if (direction == RIGHT)
        for (int i = table->pocetRadku - 1; i >= start; --i)
            table->radky[i] = table->radky[i - 1];
    else
        for (int i = start; i < table->pocetRadku; ++i)
            table->radky[i] = table->radky[i + 1];
}

/* Vloží řádek do tabulky */
int tableVlozitRadek(Tabulka *table, int insertPos) {
    int puvPocetBunek = table->radky[0].pocetBunek - 1;
    Radek newRadek;
    tabulkaAddRow(table, newRadek); //Vložím nealokovaný řádek, stejně si ho hned přepíšu
    tablePosunRadku(table, insertPos, RIGHT);
    table->radky[insertPos] = createRadek();
    tableEqualizeCols(table, puvPocetBunek);
    return 0;
}

/* Posune sloupce v řádku o jedno v daném směru */
void radekPosunSloupcu(Radek *radek, int start, int direction) {
    if (direction == RIGHT)
        for (int i = radek->pocetBunek - 1; i >= start; --i)
            radek->bunky[i] = radek->bunky[i - 1];
    else
        for (int i = start; i < radek->pocetBunek; ++i)
            radek->bunky[i] = radek->bunky[i + 1];
}

/* Vloží sloupec do řádku na určitou pozici */
int radekVlozitSloupec(Radek *radek, int insertPos) {
    Bunka newBunka;
    radekAddBunka(radek, newBunka);
    radekPosunSloupcu(radek, insertPos, RIGHT);
    radek->bunky[insertPos] = createBunka();
    return 0;
}

/* Vypíše tabulku do souboru */
void printTableToFile(Tabulka tabulka, FILE *file, char oddelovac) {
    for (int i = 0; i < tabulka.pocetRadku; ++i)
        printRadekToFile(tabulka.radky[i], file, oddelovac);
}

/* Vypsání erroru na stderr */
void printError(char *err) {
    fprintf(stderr, "Nastala chyba: %s\n", err);
}

/* Dealokuje všechny mallocy v rámci Delim a nastaví defaultní hodnoty proměnných v Delim */
void destroyDelim(Delim *delim) {
    if (delim != NULL) {
        if (delim->delimStr != NULL)
            free(delim->delimStr);
        delim->delka = 0;
    }
    delim = NULL;
}

/* Zjistí, zda string od určité pozice po určitou pozici obsahuje znak znak */
bool containsChar(int startpozice, int endpozice, char *string, char znak) {
    for (int i = startpozice; i < endpozice && string[i] != '\0'; ++i)
        if (string[i] == znak)
            return true;
    return false;
}

/* Upraví delim tak, aby se neopakovali stejné znaky vícekrát + aby se nevyskytovaly znaky {'\','"'} */
void upravDelim(char *delimArgv, Delim *delim) {
    int pocetpripsani = 0;
    int length = strlen(delimArgv);
    if (length > delim->delka) {
        void *tmp = realloc(delim->delimStr, sizeof(char) * length);
        if (delim->delimStr == NULL) {
            destroyDelim(delim);
            return;
        }
        delim->delimStr = (char *)tmp;
        delim->delka = length;
    }

    for (int i = 0; delimArgv[i] != '\0'; ++i)
    {
        if (delimArgv[i] == '\\' || delimArgv[i] == '"')
            continue;
        if (!containsChar(i + 1, length, delimArgv, delimArgv[i])) {
            delim->delimStr[pocetpripsani] = delimArgv[i];
            ++pocetpripsani;
        }
    }
    delim->delimStr[pocetpripsani] = '\0';
}

/* Nastaví delim na hodnotu argumentu + tento delim upraví podle zadání */
Delim setDelim(int *argc, char *argv[])
{
    Delim delim;
    delim.delimStr = (char *)malloc(sizeof(char) * DEFAULT_DELIM_LENGTH);
    if (delim.delimStr == NULL)
        return delim;
    delim.delimStr[0] = ' ';
    delim.delimStr[1] = '\0';
    delim.delka = DEFAULT_DELIM_LENGTH;
    if (strcmp(argv[1], "-d") == 0) {
        if (*argc < MIN_ARGC2) { //Chybí argument pro výběr souboru
            *argc = 0;
            destroyDelim(&delim);
            return delim;
        }
        char *delimArgv = argv[2];
        upravDelim(delimArgv, &delim);
    }
    return delim;
}

void usage() {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "./sps [-d DELIM] CMD_SEQUENCE FILE\n");
}

void init_X(Bunka _X[]) {
    for (int i = 0; i < NUM_OF__VARS; ++i)
        initBunka(&_X[i]);
}

/* Dealokuje místo po vytvoření _X proměnných */
void destroy_X(Bunka _X[]) {
    for (int i = 0; i < NUM_OF__VARS; ++i)
        destroyBunka(&_X[i]);
}

/* Načte substring z řetězce buňky */
Bunka loadSubstring(char *string, int *pos, char *konec) {
    Bunka bunka = createBunka();
    (*pos)++;
    while (!containsCharFullString(konec, string[*pos]) && string[*pos] != '\0')
    {
        insertCharToBunka(&bunka, string[*pos]);
        (*pos)++;
    }
    return bunka;
}

//Kontroluje, zda určitý string je číslo
bool kontrolaIsCislo(char *string) {
    int pocetDesCarek = 0;
    int pocetMocnin10 = 0;
    if (string[0] == '\0')
        return false;
    for (int i = 0; string[i] != '\0'; ++i)
    {
        if (string[i] == '.') {
            if (pocetDesCarek > 0)
                return false;
            ++pocetDesCarek;
        }
        else if (string[i] == 'e') {
            if (pocetMocnin10 > 0)
                return false;
            ++pocetMocnin10;
        }
        else if (string[i] == '+' || string[i] == '-')
            continue;
        else if ((string[i] - '0') > 9 || string[i] - '0' < 0)
            return false;
    }
    return true;
}

/* Zkontroluje, zda výběr buněk je v limitech tabulky, pokud ne tak tabulku zvětší */
int zkontrolujSelection(Selection selection, Tabulka *table) {
    if (selection.rowStart < 0)
        return WRONG_ROW_SELECTION;
    else if (selection.colStart < 0)
        return WRONG_COL_SELECTION;
    if (table->pocetRadku <= selection.rowEnd) {
        for (int i = table->pocetRadku; i <= selection.rowEnd; ++i)
        {
            tableVlozitRadek(table, i);
        }
    }
    if (table->radky[0].pocetBunek <= selection.colEnd)
        tableEqualizeCols(table, selection.colEnd);
    return EXIT_SUCCESS;
}

/* Zničí vybrané sloupce */
void dcol(Tabulka *table, Selection *selection) {
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd && selection->colStart < table->radky[0].pocetBunek; ++j)
        {
            for (int k = 0; k < table->pocetRadku; ++k)
            {
                destroyBunka(&table->radky[k].bunky[selection->colStart]);
                table->radky[k].pocetBunek--;
                radekPosunSloupcu(&table->radky[k], selection->colStart, LEFT);
            }
        }
    }
}

/* Zpracuje příkaz s ID SEL_ROW - příkazy začínající [r1,...] */
void selRow(Command command, Selection *selection) {
   if (command.numIntParams == INT_PARAMS_COUNT) { // na vstupu bylo [r1,c1,r2,c2]
            selection->rowStart = command.intParams[0] - 1;
            selection->rowEnd = command.intParams[2] - 1;
            selection->colStart = command.intParams[1] - 1;
            selection->colEnd = command.intParams[3] - 1;
    }
    else {
        if (command.numIntParams == SELECTED_ALL_COLS) // na vstupu je [r1,_]
            selection->colStart = 0;
        else
            selection->colStart = command.intParams[1] - 1; //na vstupu je [r1,c1]

        selection->colEnd = command.intParams[1] - 1;
        selection->rowStart = command.intParams[0] - 1;
        selection->rowEnd = command.intParams[0] - 1;
    } 
}

/* Zpracuje příkaz s ID SEL_COL - příkazy začínající [_,...] */
void selCol(Command command, Selection *selection, Tabulka *table) {
    selection->rowStart = 0;
    selection->rowEnd = table->pocetRadku - 1;
    if (command.numIntParams == SELECTED_ALL_COLS) //na vstupu bylo [_,_]
        selection->colStart = 0;
    else //Na vstupu bylo [_,y]
        selection->colStart = command.intParams[0] - 1;
    selection->colEnd = command.intParams[0] - 1;
}

/* Zpracuje příkaz [min] - najde ve stávajícím výběru minimum */
void selectMin(Selection *selection, Tabulka *table) {
    int min = atoi(table->radky[selection->rowStart].bunky[selection->colStart].retezec);
    Selection posMin = {selection->rowStart, selection->rowStart, selection->colStart, selection->colStart};
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j) {
            if (kontrolaIsCislo(table->radky[i].bunky[j].retezec)) {
                int num = atoi(table->radky[i].bunky[j].retezec);
                if (num < min) {
                    min = num;
                    posMin.rowStart = posMin.rowEnd = i;
                    posMin.colStart = posMin.colEnd = j;
                }
            }
        }
    }
    *selection = posMin;
}

/* Zpracuje příkaz [max] - najde ve stávajícím výběru maximum */
void selectMax(Selection *selection, Tabulka *table) {
    int max = atoi(table->radky[selection->rowStart].bunky[selection->colStart].retezec);
    Selection posMax = {selection->rowStart, selection->rowStart, selection->colStart, selection->colStart};
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            if (kontrolaIsCislo(table->radky[i].bunky[j].retezec)) {
                int num = atoi(table->radky[i].bunky[j].retezec);
                if (num > max) {
                    max = num;
                    posMax.rowStart = posMax.rowEnd = i;
                    posMax.colStart = posMax.colEnd = j;
                }
            }
        }
    }
    *selection = posMax;
}

/* Vybere první buňku obsahující řetězec z argumentu příkazu [find str] ze stávajícího výběru */
void selectCellWithStr(Command command, Selection *selection, Tabulka *table) {
    Selection posStr = {selection->rowStart, selection->rowStart, selection->colStart, selection->colStart};
    bool nalezeno = false;
    for (int i = selection->rowStart; i <= selection->rowEnd && !nalezeno; ++i)
    {
        for (int j = selection->colStart; j <= selection->rowEnd && !nalezeno; ++j)
        {
            char *str = table->radky[i].bunky[j].retezec;
            if (containsStr(str, command.strParam.retezec)) {
                posStr.rowStart = posStr.rowEnd = i;
                posStr.colStart = posStr.colEnd = j;
                nalezeno = true;
            }
        }
    }
    *selection = posStr;
    destroyBunka(&command.strParam);   
}

/* Vkládá řádek ke KAŽDÉ vybrané buňce, pokud je parametr arow = 1, funguje jako arow, jinak jako irow */
void addRow(Selection *selection, Tabulka *table, int arow) {
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
            tableVlozitRadek(table, i + arow);
    }
}

/* Maže řádek identifikující KAŽDOU vybranou buňku */
void drow(Selection *selection, Tabulka *table) {
    for (int i = selection->rowStart; i <= selection->rowEnd && table->pocetRadku > selection->rowStart; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd && table->pocetRadku > selection->rowStart; ++j)
        {
            destroyRadek(&table->radky[selection->rowStart]);
            tablePosunRadku(table, selection->rowStart, LEFT);
            table->pocetRadku--;
        }
    }
}

/* Vkládá řádek ke KAŽDÉ vybrané buňce, pokud je parametr arow = 1, funguje jako arow, jinak jako irow */
void addCol(Selection *selection, Tabulka *table, int acol) {
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            for (int k = 0; k < table->pocetRadku; ++k)
                radekVlozitSloupec(&table->radky[k], j + acol);
        }
    }
}

/* prohodí VŠECHNY vybrané buňky s jednou zadanou v argumentu příkazu swap [r1,c1] */
int swap(Command command, Selection *selection, Tabulka *table) {
    int row = command.intParams[0] - 1;
    int col = command.intParams[1] - 1;
    if (row >= table->pocetRadku || row < 0)
        return WRONG_ROW_SELECTION;
    if (col >= table->radky[0].pocetBunek || col < 0)
        return WRONG_COL_SELECTION;
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            Bunka tmp = table->radky[row].bunky[col];
            table->radky[row].bunky[col] = table->radky[i].bunky[j];
            table->radky[i].bunky[j] = tmp;
        }
    }
    return EXIT_SUCCESS;
}

/* sečte všechny hodnoty vybraných buněk */
int sum(Command command, Selection *selection, Tabulka *table) {
    int row = command.intParams[0] - 1;
    int col = command.intParams[1] - 1;
    if (row >= table->pocetRadku || row < 0)
        return WRONG_ROW_SELECTION;
    if (col >= table->radky[0].pocetBunek || col < 0)
        return WRONG_COL_SELECTION;
    double soucet = 0.0;
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            char *bunkaStr = table->radky[i].bunky[j].retezec;
            soucet += atof(bunkaStr);
        }
    }
    char str[MAX_LENGTH_DOUBLE_STRING];
    snprintf(str, MAX_LENGTH_DOUBLE_STRING, "%g", soucet);
    setBunka(&table->radky[row].bunky[col], str);
    return EXIT_SUCCESS;
}

/* Spočítá průměr z hodnot ve vybraných buňkách */
int avg(Command command, Selection *selection, Tabulka *table) {
    int row = command.intParams[0] - 1;
    int col = command.intParams[1] - 1;
    if (row >= table->pocetRadku || row < 0)
        return WRONG_ROW_SELECTION;
    if (col >= table->radky[0].pocetBunek || col < 0)
        return WRONG_COL_SELECTION;
    double soucet = 0.0;
    int pocet = 0;
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            char *bunkaStr = table->radky[i].bunky[j].retezec;
            if (kontrolaIsCislo(bunkaStr)) {
                soucet += atof(bunkaStr);
                ++pocet;
            }
        }
    }
    if (pocet > 0) {
        double prumer = soucet / pocet;
        char str[MAX_LENGTH_DOUBLE_STRING];
        snprintf(str, MAX_LENGTH_DOUBLE_STRING, "%g", prumer);
        setBunka(&table->radky[row].bunky[col], str);
    }
    else
        setBunka(&table->radky[row].bunky[col], "DevideByZero");
    return EXIT_SUCCESS;
}

/* Spočítá neprázdné vybrané buňky (počet) */
int count(Command command, Selection *selection, Tabulka *table) {
    int row = command.intParams[0] - 1;
    int col = command.intParams[1] - 1;
    if (row >= table->pocetRadku || row < 0)
        return WRONG_ROW_SELECTION;
    if (col >= table->radky[0].pocetBunek || col < 0)
        return WRONG_COL_SELECTION;
    int pocet = 0;
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            char *bunkaStr = table->radky[i].bunky[j].retezec;
            if (!bunkaStr[0] == '\0')
                pocet++;
        }
    }
    char str[MAX_LENGTH_INT_STRING];
    snprintf(str, MAX_LENGTH_INT_STRING, "%d", pocet);
    setBunka(&table->radky[row].bunky[col], str);
    return EXIT_SUCCESS;
}

/* Spočítá délku vybraných buněk */
int len(Command command, Selection *selection, Tabulka *table) {
    int row = command.intParams[0] - 1;
    int col = command.intParams[1] - 1;
    if (row >= table->pocetRadku || row < 0)
        return WRONG_ROW_SELECTION;
    if (col >= table->radky[0].pocetBunek || col < 0)
        return WRONG_COL_SELECTION;
    int delka = 0;
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
        {
            char *bunkaStr = table->radky[i].bunky[j].retezec;
            for (int k = 0; bunkaStr[k] != '\0'; ++k)
                delka++;
        }
    }
    char str[MAX_LENGTH_INT_STRING];
    snprintf(str, MAX_LENGTH_INT_STRING, "%d", delka);
    setBunka(&table->radky[row].bunky[col], str);
    return EXIT_SUCCESS;
}

/* Zvýší hodnotu v proměnné _X a pokud tam je string uloží číslo 1 */
void inc(Command command, Bunka _X[]) {
    char *str = _X[command.intParams[0]].retezec;
    if (!kontrolaIsCislo(str))
        setBunka(&_X[command.intParams[0]], "1");
    else {
        double num = atof(str);
        num++;
        char strNum[MAX_LENGTH_DOUBLE_STRING];
        snprintf(strNum, MAX_LENGTH_DOUBLE_STRING, "%g", num);
        setBunka(&_X[command.intParams[0]], strNum);
    }
}

/* Zapíše string do vybraných buněk */
void writeToSelectedCells(Selection *selection, Tabulka *table, char *set) {
    for (int i = selection->rowStart; i <= selection->rowEnd; ++i)
    {
        for (int j = selection->colStart; j <= selection->colEnd; ++j)
            setBunka(&table->radky[i].bunky[j], set);
    }
}

/* Provádí jednotlivé příkazy */
int interpreter(Command command, Tabulka *table, Selection *selection, Bunka _X[], Selection *_tmpSel) {
    int exitVal = zkontrolujSelection(*selection, table);
    if (exitVal != EXIT_SUCCESS)
        return exitVal;
    switch (command.id)
    {
    case SEL_ROW:
        selRow(command, selection);
        break;
    case SEL_COL:
        selCol(command, selection, table);
        break;
    case SEL_MIN:
        selectMin(selection, table);
        break;
    case SEL_MAX:
        selectMax(selection, table);
    break;
    case SEL_FIND: 
        selectCellWithStr(command, selection, table);
        break;
    case SEL_TMP:
        *selection = *_tmpSel;
        break;
    case IROW:
        addRow(selection, table, 0);
        break;
    case AROW:
        addRow(selection, table, 1);
        break;
    case DROW:
        drow(selection, table);
        break;
    case ICOL:
        addCol(selection, table, 0);
        break;
    case ACOL:
        addCol(selection, table, 1);
        break;
    case DCOL:
        dcol(table, selection);
        break;
    case SET_SEL:
        *_tmpSel = *selection;
        break;
    case CLEAR:
        writeToSelectedCells(selection, table, "");
        break;
    case SWAP:
        return swap(command, selection, table);
    case SUM:
        return sum(command, selection, table);
    case AVG:
        return avg(command, selection, table);
    case COUNT:
        return count(command, selection, table);
    break;
    case LEN:
        return len(command, selection, table);
    case DEF:
        setBunka(&_X[command.intParams[0]], table->radky[selection->rowEnd].bunky[selection->colEnd].retezec);
        break;
    case USE:
    {
        char *set = _X[command.intParams[0]].retezec;
        writeToSelectedCells(selection, table, set);
    }
    break;
    case INC:
        inc(command, _X);
        break;
    case SET:
    {
        char *set = command.strParam.retezec;
        writeToSelectedCells(selection, table, set);
        destroyBunka(&command.strParam);
        break;
    }
    }
    return EXIT_SUCCESS;
}

/* Rekurzivně načítá příkazy a jejich argumenty */
int loadCmd(Command *command, char *cmdSequence, int *posCommand, bool commandCreated, Tabulka *table) {
    if (!commandCreated) { //Vytvoření příkazu
        Bunka loaded = loadSubstring(cmdSequence, posCommand, "[ ;");

        if (loaded.size == 1) { //Commandy uvnitř []
            destroyBunka(&loaded);
            loaded = loadSubstring(cmdSequence, posCommand, ",]; ");
            if (strcmp(loaded.retezec, "_") == 0) {
                if (cmdSequence[*posCommand] == ']') {
                    destroyBunka(&loaded);
                    *command = createCommand(SEL_TMP);
                    return EXIT_SUCCESS;
                }
                else
                    *command = createCommand(SEL_COL);
            }
            else if (strcmp(loaded.retezec, "set") == 0) {
                destroyBunka(&loaded);
                *command = createCommand(SET_SEL);
                return EXIT_SUCCESS;
            }
            else if (strcmp(loaded.retezec, "max") == 0) {
                destroyBunka(&loaded);
                *command = createCommand(SEL_MAX);
                return EXIT_SUCCESS;
            }
            else if (strcmp(loaded.retezec, "min") == 0) {
                destroyBunka(&loaded);
                *command = createCommand(SEL_MIN);
                return EXIT_SUCCESS;
            }
            else if (strcmp(loaded.retezec, "find") == 0)
                *command = createCommand(SEL_FIND);
            else {
                *command = createCommand(SEL_ROW);
                command->intParams[command->numIntParams++] = atoi(loaded.retezec);
            }
        }
        else if (strcmp(loaded.retezec, "set") == 0)
            *command = createCommand(SET);
        else if (strcmp(loaded.retezec, "swap") == 0)
            *command = createCommand(SWAP);
        else if (strcmp(loaded.retezec, "sum") == 0)
            *command = createCommand(SUM);
        else if (strcmp(loaded.retezec, "avg") == 0)
            *command = createCommand(AVG);
        else if (strcmp(loaded.retezec, "count") == 0)
            *command = createCommand(COUNT);
        else if (strcmp(loaded.retezec, "len") == 0)
            *command = createCommand(LEN);
        else if (strcmp(loaded.retezec, "def") == 0)
            *command = createCommand(DEF);
        else if (strcmp(loaded.retezec, "use") == 0)
            *command = createCommand(USE);
        else if (strcmp(loaded.retezec, "inc") == 0)
            *command = createCommand(INC);
        else { //bez argumentové příkazy
            if (strcmp(loaded.retezec, "irow") == 0)
                *command = createCommand(IROW);
            else if (strcmp(loaded.retezec, "arow") == 0)
                *command = createCommand(AROW);
            else if (strcmp(loaded.retezec, "drow") == 0)
                *command = createCommand(DROW);
            else if (strcmp(loaded.retezec, "icol") == 0)
                *command = createCommand(ICOL);
            else if (strcmp(loaded.retezec, "acol") == 0)
                *command = createCommand(ACOL);
            else if (strcmp(loaded.retezec, "dcol") == 0)
                *command = createCommand(DCOL);
            else if (strcmp(loaded.retezec, "clear") == 0)
                *command = createCommand(CLEAR);
            else {
                destroyBunka(&loaded);
                return COMMAND_NOT_RECOGNISED;
            }
            destroyBunka(&loaded);
            return EXIT_SUCCESS;
        }
        destroyBunka(&loaded);
        loadCmd(command, cmdSequence, posCommand, true, table);
    }
    else                                                      //Načtení argumentů, Command již byl vytvořen, budu číst jen argumenty 
    {
        if (command->numIntParams == MAX_CMD_SEQUENCE_PARAMS) //Ochrana před zacyklením u nesmyslných argumentů
            return WRONG_ARGUMENT;
        Bunka loaded = loadSubstring(cmdSequence, posCommand, "[,];");

        if (loaded.size > 1) {
            if (command->id >= SWAP && command->id <= LEN) {
                command->intParams[command->numIntParams++] = atoi(loaded.retezec);
                if (command->numIntParams == 2){
                    destroyBunka(&loaded);
                    return EXIT_SUCCESS;
                }
            }
            else if (command->id == SEL_ROW || command->id == SEL_COL) {
                if (strcmp(loaded.retezec, "_") == 0) {
                    command->intParams[command->numIntParams++] = table->radky[0].pocetBunek;
                    command->numIntParams = SELECTED_ALL_COLS;
                }
                else if (strcmp(loaded.retezec, "-") == 0) { // argument je pomlčka, nahradit maximálním sloupcem / maximálním řádkem
                    if (command->numIntParams == 2)
                        command->intParams[command->numIntParams++] = table->pocetRadku;
                    else
                        command->intParams[command->numIntParams++] = table->radky[0].pocetBunek;
                }
                else
                    command->intParams[command->numIntParams++] = atoi(loaded.retezec);
            }
            else if (command->id >= DEF && command->id <= INC)
                command->intParams[command->numIntParams++] = loaded.retezec[1] - '0';
            else if (command->id == SEL_FIND || command->id == SET) {
                command->strParam = loaded;
                return EXIT_SUCCESS;
            }
            if (cmdSequence[*posCommand] != '\0' && cmdSequence[*posCommand] != ';')
                loadCmd(command, cmdSequence, posCommand, true, table);
        }
        else if (cmdSequence[*posCommand] == '[') //u swap [1,5] je nyní znak [ a length to hodí jako 1
            loadCmd(command, cmdSequence, posCommand, true, table);
        destroyBunka(&loaded);
    }
    //Konečná kontrola správnosti argumentů
    if (command->id >= SWAP && command->id <= LEN && command->numIntParams == 1)
        return WRONG_ARGUMENT;
    else if (command->id == SEL_COL && command->numIntParams == 0)
        return WRONG_ARGUMENT;
    else if (command->id == SEL_ROW && command->numIntParams % 2 == 1)
        return WRONG_ARGUMENT;
    return EXIT_SUCCESS;
}

/* Provede všechny příkazy ze cmdSequence */
int processCommands(char *cmdSequence, Tabulka *table, Selection *selection, Bunka _X[], Selection *_tmpSel) {
    Command command;
    command.id = -1;
    command.numIntParams = 0;
    bool cmdStart = true;
    for (int i = 0; cmdSequence[i] != '\0'; ++i)
    {
        if (cmdStart) {
            if (cmdSequence[i] == ' ')
                continue; //Přeskočení mezer před příkazem
            else {
                --i;
                int exitVal = loadCmd(&command, cmdSequence, &i, false, table);
                if (exitVal != EXIT_SUCCESS)
                    return exitVal;
            }
        }
        if (cmdSequence[i] == ';') {
            if (command.id != -1) {
                //Proveď příkaz
                int exitVal = interpreter(command, table, selection, _X, _tmpSel);
                if (exitVal != EXIT_SUCCESS)
                    return exitVal;
                command.id = -1; //Značí, že byl příkaz vykonán
            }
            cmdStart = true;
        }
        else
            cmdStart = false;
    }
    if (command.id != -1) {
        //Proveď příkaz
        int exitVal = interpreter(command, table, selection, _X, _tmpSel);
        if (exitVal != EXIT_SUCCESS)
            return exitVal;
    }
    return EXIT_SUCCESS;
}

/* Kontrola jestli tabulka obsahuje poslední \n řádek */
bool lastLineEmpty(Tabulka tabulka) {
    if (tabulka.radky[tabulka.pocetRadku - 1].pocetBunek == 0)
        return true;
    if (tabulka.radky[tabulka.pocetRadku - 1].pocetBunek == 1) { //Buňka se sice vytvořila, ale je prázdná
        if (tabulka.radky[tabulka.pocetRadku - 1].bunky[0].size == 1)
            return true;
    }
    return false;
}

/* Načte tabulku ze souboru */
Tabulka loadTable(FILE *file, Delim delim) {
    //Inicializace tabulky a vložení prvního řádku a do něj první buňky
    Tabulka tabulka;
    initTabulka(&tabulka);
    tabulkaAddRow(&tabulka, createRadek());
    Radek *actualRow = &tabulka.radky[tabulka.pocetRadku - 1];
    radekAddBunka(actualRow, createBunka());
    Bunka *actualBunka = &actualRow->bunky[actualRow->pocetBunek - 1];

    char znak;
    bool cellStart = true;
    bool quotesStarted = false;
    char predchoziZnak = '\0';
    while ((znak = fgetc(file)) != EOF)
    {
        if (znak == '"' && cellStart)
            quotesStarted = true;
        if (isDelim(znak, delim.delimStr)) {
            if (predchoziZnak != '\\') {
                if (quotesStarted) { //Buňka v uvozovkách?
                    if (predchoziZnak == '"') { //Před delimem je " ?
                        radekAddBunka(actualRow, createBunka());
                        actualBunka = &(actualRow->bunky[actualRow->pocetBunek - 1]);
                        cellStart = true;
                        quotesStarted = false;
                    }
                    else { // Delim je uvnitř "", ignoruj ho
                        insertCharToBunka(actualBunka, znak);
                        cellStart = false;
                    }
                }
                else {
                    radekAddBunka(actualRow, createBunka());
                    actualBunka = &(actualRow->bunky[actualRow->pocetBunek - 1]);
                    cellStart = true;
                }
            }
            else
                insertCharToBunka(actualBunka, znak);
        }
        else if (znak == '\n') {
            tabulkaAddRow(&tabulka, createRadek());
            actualRow = &(tabulka.radky[tabulka.pocetRadku - 1]);
            radekAddBunka(actualRow, createBunka());
            actualBunka = &(actualRow->bunky[actualRow->pocetBunek - 1]);
            cellStart = true;
        }
        else {
            insertCharToBunka(actualBunka, znak);
            cellStart = false;
        }
        predchoziZnak = znak;
    }
    destroyRadek(actualRow);
    tabulka.pocetRadku--;
    return tabulka;
}

/* Upraví buňky v tabulce do požadovaného výstupního formátu */
void prepareTableCellsToWrite(Tabulka *table, char *delim) {
    for (int i = 0; i < table->pocetRadku; ++i)
    {
        for (int j = 0; j < table->radky[i].pocetBunek; ++j)
            controllBunka(&table->radky[i].bunky[j], delim);
    }
}

/* Zjistí, zda poslední sloupcec v tabulce je prázdný */
bool tableIsLastColEmpty(Tabulka *tabulka) {
    if (tabulka->pocetRadku <= 0)
        return false;
    for (int i = 0; i < tabulka->pocetRadku; ++i)
    {
        int lastBunka = tabulka->radky[i].pocetBunek - 1;
        if (lastBunka < 0)
            return false;
        if (tabulka->radky[i].bunky[lastBunka].retezec[0] != '\0')
            return false;
    }
    return true;
}

/* Odstraní z tabulky poslední prázdné sloupce */
void tableRemoveEmptyCols(Tabulka *tabulka) {
    while (tableIsLastColEmpty(tabulka))
    {
        int lastBunka = tabulka->radky[0].pocetBunek - 1;
        Selection selection = {0, 0, lastBunka, lastBunka};
        dcol(tabulka, &selection);
    }
}

int main(int argc, char *argv[]) {
    //Zpracování argumentů příkazové řádky
    if (argc < MIN_ARGC) {
        printError(ERR_MISSING_ARGUMENTS);
        usage();
        return EXIT_FAILURE;
    }
    Delim delim = setDelim(&argc, argv);
    if (argc == 0) {
        destroyDelim(&delim);
        printError(ERR_MISSING_FILE);
        usage();
        return EXIT_FAILURE;
    }
    else if (delim.delimStr == NULL) {
        printError(ERR_WRITE_TO_MEM_FAILURE);
        return EXIT_FAILURE;
    }
    FILE *file;
    char *filename;
    char *cmdSequence;
    if (argc > MIN_ARGC) {
        cmdSequence = argv[DEFAULT_CMD_POS + 2];
        filename = argv[DEFAULT_FILE_POS + 2];
    }
    else {
        cmdSequence = argv[DEFAULT_CMD_POS];
        filename = argv[DEFAULT_FILE_POS];
    }
    file = fopen(filename, "r");

    if (file == NULL) {
        printError(ERR_OPENING_FILE);
        return EXIT_FAILURE;
    }

    //Inicializace hlavních proměnných
    Selection selection = {0, 0, 0, 0};
    Selection _tmpSel = {0, 0, 0, 0};
    Bunka _X[NUM_OF__VARS];
    init_X(_X);

    // Práce s tabulkou
    //Naštení souboru do tabulky
    Tabulka tabulka = loadTable(file, delim);
    fclose(file);

    if (lastLineEmpty(tabulka)) {
        destroyRadek(&tabulka.radky[tabulka.pocetRadku - 1]);
        tabulka.pocetRadku--;
    }
    int pocetCols = tableFindMaxCols(tabulka);
    tableEqualizeCols(&tabulka, pocetCols);

    int exitVal = processCommands(cmdSequence, &tabulka, &selection, _X, &_tmpSel);
    if (exitVal != EXIT_SUCCESS) { //Ošetření chybových stavů
        destroyTabulka(&tabulka);
        destroyDelim(&delim);
        destroy_X(_X);
        switch (exitVal)
        {
        case WRONG_COL_SELECTION:
            printError(ERR_COL_SELECTION_MSG);
            break;
        case WRONG_ROW_SELECTION:
            printError(ERR_ROW_SELECTION_MSG);
            break;
        case COMMAND_NOT_RECOGNISED:
            printError(ERR_COMMAND_NOT_RECOGNISED_MSG);
            break;
        case WRONG_ARGUMENT:
            printError(ERR_WRONG_ARGUMENT_MSG);
            break;
        }
        return EXIT_FAILURE;
    }
    tableRemoveEmptyCols(&tabulka);
    prepareTableCellsToWrite(&tabulka, delim.delimStr);

    //Výpis tabulky do souboru
    char oddelovac = delim.delimStr[0];
    file = fopen(filename, "w");
    printTableToFile(tabulka, file, oddelovac);
    fprintf(file, "\n");
    fclose(file);

    // uvolnění paměti
    destroyTabulka(&tabulka);
    destroyDelim(&delim);
    destroy_X(_X);
    return EXIT_SUCCESS;
}
