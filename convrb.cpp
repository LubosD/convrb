#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "convrb.h"
#include "util.h"

HeaderState g_headerState;
OverViewState g_overviewState;
ItemState g_itemState;

int main()
{
	char buf[90];
	State state = Header;

	memset(&g_headerState, 0, sizeof g_headerState);
	memset(&g_overviewState, 0, sizeof g_overviewState);
	memset(&g_itemState, 0, sizeof g_itemState);

	while (fgets(buf, sizeof buf, stdin))
	{
		rmeol(buf);
		
		//fprintf(stderr, "State: %d, processing: %s\n", state, buf);
		switch (state)
		{
		case Header:
			state = handleHeaderState(buf);
			break;
		case Overview:
			state = handleOverviewState(buf);
			break;
		case Items:
			state = handleItemsState(buf);
			break;
		case End:
			state = handleEndState(buf);
			break;
		}
	}

	return 0;
}

State handleHeaderState(const char* line)
{
	if (!strxcmp(line, "Bankovní výpis"))
		g_headerState.num = atoi(line + 18);
	else if (!strxcmp(line, "Za období"))
	{
		if (sscanf(line+10, "%02d.%02d.%04d/%02d.%02d.%04d",
					&g_headerState.tmStart.tm_mday, &g_headerState.tmStart.tm_mon, &g_headerState.tmStart.tm_year,
					&g_headerState.tmEnd.tm_mday, &g_headerState.tmEnd.tm_mon, &g_headerState.tmEnd.tm_year) != 6)
		{
			fprintf(stderr, "Cannot parse date range: %s\n", line+10);
			exit(1);
		}

		g_headerState.tmStart.tm_mon--;
		g_headerState.tmEnd.tm_mon--;
		g_headerState.tmStart.tm_year -= 1900;
		g_headerState.tmEnd.tm_year -= 1900;
	}
	else if (!strxcmp(line, "Název úètu:"))
	{
		strxcpy(g_headerState.accName, line+12, sizeof g_headerState.accName);
		trim(g_headerState.accName);
	}
	else if (!strxcmp(line, "Èíslo úètu:"))
	{
		strxcpy(g_headerState.accNum, line+12, sizeof g_headerState.accNum);
		trim(g_headerState.accName);
	}
	else if (!strxcmp(line, "Mìna"))
		return Overview;

	return Header;
}

State handleOverviewState(const char* line)
{
	if (!strxcmp(line, "Poèáteèní zùstatek"))
		g_overviewState.dblStart = parseDouble(line+64);
	else if (!strxcmp(line, "Pøíjmy"))
		g_overviewState.dblCredit = parseDouble(line+64);
	else if (!strxcmp(line, "Výdaje"))
		g_overviewState.dblDebit = parseDouble(line+64);
	else if (!strxcmp(line, "Z toho poplatky"))
		g_overviewState.dblBankFees = parseDouble(line+64);
	else if (!strxcmp(line, "Koneèný zùstatek"))
		g_overviewState.dblEnd = parseDouble(line+64);
	else if (line[0] == '=' && ++g_overviewState.lines >= 5)
	{
		printHeader();
		return Items;
	}
	return Overview;

}

State handleItemsState(const char* line)
{
	if (line[0] == '-')
	{
		printItem();
		memset(&g_itemState, 0, sizeof g_itemState);
	}
	else if (!line[0])
		return End;
	else
	{
		switch (g_itemState.line)
		{
		case 0:
			g_itemState.day = parseInt(line+5);
			g_itemState.dblAmount = parseDouble(line+55);
			memcpy(g_itemState.specSym, line+44, 10);
			g_itemState.specSym[10] = 0;
			trim(g_itemState.specSym);

			memcpy(g_itemState.description, line+11, 22);
			g_itemState.description[22] = 0;
			trim(g_itemState.description);

			break;
		case 1:
			memcpy(g_itemState.accName, line+11, 22);
			g_itemState.accName[22] = 0;
			trim(g_itemState.accName);
				
			memcpy(g_itemState.varSym, line+44, 10); // 44?
			g_itemState.varSym[10] = 0;
			trim(g_itemState.varSym);
			break;
		case 2:
			memcpy(g_itemState.accNum, line+11, 22);
			g_itemState.accNum[22] = 0;
			trim(g_itemState.accNum);

			memcpy(g_itemState.constSym, line+44, 10);
			g_itemState.constSym[10] = 0;
			trim(g_itemState.constSym);

			strcpy(g_itemState.operation, line+55);
			trim(g_itemState.operation);

			if (!strcmp(g_itemState.operation, "Výbìr z bankomatu"))
				strcpy(g_itemState.description, "Výbìr z bankomatu: ");

			break;
		case 3:
		case 4:
		case 5:
			strcat(g_itemState.description, line+11);
			if (!g_itemState.accName[0])
				strxcpy(g_itemState.accName, g_itemState.description, 20);
			break;
		}
		g_itemState.line++;
	}
	return Items;
}

State handleEndState(const char* line)
{
	// print bank fees
	char* slash;
	int bankCode;
	char accNum[30], name[71] = "Bankovni poplatky";

	strcpy(accNum, g_headerState.accNum);
	slash = strchr(accNum, '/');
	*slash = 0;
	bankCode = atoi(slash+1);

	padleft(accNum, 16, '0');
	padright(name, 70, ' ');

	printf("075%16s%016d%013d%012ld%d%010d%04d0000000000000000000000Bankovni poplatky   00203%02d%02d%02d\r\n",
                accNum, 0, 0, labs(long(g_overviewState.dblBankFees*100)), (g_overviewState.dblBankFees>=0) ? 2 : 1,
                0, bankCode,
                g_headerState.tmEnd.tm_mday, g_headerState.tmEnd.tm_mon+1, g_headerState.tmEnd.tm_year%100);
		printf("078%s\r\n", name);

	exit(0);
	return End;
}

void printHeader()
{
	char accNum[30];
	struct tm tmLastDay = g_headerState.tmStart;
	time_t ttimet;
	char* slash;

	strcpy(accNum, g_headerState.accNum);

	slash = strchr(accNum, '/');
	if (!slash)
	{
		fprintf(stderr, "Invalid account number: %s\n", accNum);
		exit(1);
	}
	*slash = 0;
	padleft(accNum, 16, '0');

	ttimet = mktime(&tmLastDay) - 24*60*60;
	tmLastDay = *localtime(&ttimet);

	printf("074%16s%20s%02d%02d%02d%014ld%c%014ld%c%014ld%c%014ld%c%03d%02d%02d%02d              \r\n",
		accNum, padright(g_headerState.accName, 20, ' '), tmLastDay.tm_mday, tmLastDay.tm_mon+1, tmLastDay.tm_year%100,
		labs(long(g_overviewState.dblStart*100)), (g_overviewState.dblStart>=0) ? '+' : '-',
		labs(long(g_overviewState.dblEnd*100)), (g_overviewState.dblEnd>=0) ? '+' : '-',
		labs(long(g_overviewState.dblDebit*100)), (g_overviewState.dblDebit>=0) ? '0' : '-',
		labs(long(g_overviewState.dblCredit*100)), (g_overviewState.dblCredit>=0) ? '0' : '-',
		g_headerState.num, g_headerState.tmEnd.tm_mday, g_headerState.tmEnd.tm_mon+1, g_headerState.tmEnd.tm_year%100
		);
}

void printItem()
{
	char accNum[30], accNum2[30];
	long constSym;
	char* slash;
	
	strcpy(accNum, g_headerState.accNum);
	//rmdash(accNum);
	slash = strchr(accNum, '/');
	*slash = 0;
	padleft(accNum, 16, '0');

	trim(g_itemState.varSym);
	padleft(g_itemState.varSym, 10, '0');
	trim(g_itemState.specSym);
	padleft(g_itemState.specSym, 10, '0');
	constSym = atoi(g_itemState.constSym);

	strcpy(accNum2, g_itemState.accNum);
	rmdash(accNum2);
	slash = strchr(accNum2, '/');

	if (!g_itemState.description[0])
		strcpy(g_itemState.description, g_itemState.operation);
	if (!slash)
	{
		//fprintf(stderr, "Invalid account number: %s\n", accNum2);
		//exit(1);
		//if (!g_itemState.description[0])
		//	strcpy(g_itemState.description, g_itemState.operation);
	}
	else
	{
		*slash = 0;
		constSym = 10000l * atoi(slash+1);
	}
	padleft(accNum2, 16, '0');

	printf("075%16s%16s%013d%012ld%d%10s%010ld%10s000000%20s00203%02d%02d%02d\r\n",
		accNum, accNum2, 0, labs(long(g_itemState.dblAmount*100)), (g_itemState.dblAmount>=0) ? 2 : 1,
		g_itemState.varSym, constSym, g_itemState.specSym, padright(g_itemState.accName, 20, ' '),
		g_itemState.day, g_headerState.tmEnd.tm_mon+1, g_headerState.tmEnd.tm_year%100);
		
	if (g_itemState.description[0])
	{
		char av1[71];
		strxcpy(av1, g_itemState.description, 70);
		padright(av1, 70, ' ');
		printf("078%s\r\n", av1);
	}
}

