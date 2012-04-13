#ifndef CONVRB_H
#define CONVRB_H

enum State {
	Header,
	Overview,
	Items,
	End
};

State handleHeaderState(const char* line);
State handleOverviewState(const char* line);
State handleItemsState(const char* line);
State handleEndState(const char* line);

void printHeader();
void printItem();

struct HeaderState
{
	char accName[30];
	char accNum[30];
	int num;
	struct tm tmStart, tmEnd;
};

struct OverViewState
{
	double dblStart, dblEnd;
	double dblCredit, dblDebit;
	double dblBankFees;
	int lines;
};

struct ItemState
{
	int day;
	char accName[40];
	double dblAmount;
	char accNum[30];
	char bankNum[5];
	char varSym[11], specSym[11], constSym[11];
	char description[100], operation[50];
	int line;
};

#endif

