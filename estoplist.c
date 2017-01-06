#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dwtype.h"
#if !defined(_TEST)
#include "hstrlib.h"
#endif
#include "estoplist.h"

#define MAX_ESTOPLIST 421 // m&a������..3
PRIVATE tCHAR *EStopList[MAX_ESTOPLIST] = {
//"a",
"about",
"above",
"across",
"after",
"again",
"against",
"all",
"almost",
"alone",
"along",
"already",
"also",
"although",
"always",
"among",
"an",
"and",
"another",
"any",
"anybody",
"anyone",
"anything",
"anywhere",
"are",
"area",
"areas",
"around",
"as",
"ask",
"asked",
"asking",
"asks",
"at",
"away",
"b",
"back",
"backed",
"backing",
"backs",
"be",
"became",
"because",
"become",
"becomes",
"been",
"before",
"began",
"behind",
"being",
"beings",
"best",
"better",
"between",
"big",
"both",
"but",
"by",
"c",
"came",
"can",
"cannot",
"case",
"cases",
"certain",
"certainly",
"clear",
"clearly",
"come",
"could",
"d",
"did",
"differ",
"different",
"differently",
"do",
"does",
"done",
"down",
"downed",
"downing",
"downs",
"during",
"e",
"each",
"early",
"either",
"end",
"ended",
"ending",
"ends",
"enough",
"even",
"evenly",
"ever",
"every",
"everybody",
"everyone",
"everything",
"everywhere",
"f",
"face",
"faces",
"fact",
"facts",
"far",
"felt",
"few",
"find",
"finds",
"first",
"for",
"four",
"from",
"full",
"fully",
"further",
"furthered",
"furthering",
"furthers",
"g",
"gave",
"general",
"generally",
"get",
"gets",
"give",
"given",
"gives",
"go",
"going",
"good",
"goods",
"got",
"great",
"greater",
"greatest",
"group",
"grouped",
"grouping",
"groups",
"h",
"had",
"has",
"have",
"having",
"he",
"her",
"here",
"herself",
"high",
"higher",
"highest",
"him",
"himself",
"his",
"how",
"however",
"i",
"if",
"important",
"in",
"interest",
"interested",
"interesting",
"interests",
"into",
"is",
"it",
"its",
"itself",
"j",
"just",
"k",
"keep",
"keeps",
"kind",
"knew",
"know",
"known",
"knows",
"l",
"large",
"largely",
"last",
"later",
"latest",
"least",
"less",
"let",
"lets",
"like",
"likely",
"long",
"longer",
"longest",
//"m",
"made",
"make",
"making",
"man",
"many",
"may",
"me",
"member",
"members",
"men",
"might",
"more",
"most",
"mostly",
"mr",
"mrs",
"much",
"must",
"my",
"myself",
"n",
"necessary",
"need",
"needed",
"needing",
"needs",
"never",
"new",
"newer",
"newest",
"next",
"no",
"nobody",
"non",
"noone",
"not",
"nothing",
"now",
"nowhere",
"number",
"numbers",
"o",
"of",
"off",
"often",
"old",
"older",
"oldest",
"on",
"once",
"one",
"only",
"open",
"opened",
"opening",
"opens",
"or",
"order",
"ordered",
"ordering",
"orders",
"other",
"others",
"our",
"out",
"over",
"p",
"part",
"parted",
"parting",
"parts",
"per",
"perhaps",
"place",
"places",
"point",
"pointed",
"pointing",
"points",
"possible",
"present",
"presented",
"presenting",
"presents",
"problem",
"problems",
"put",
"puts",
"q",
"quite",
"r",
"rather",
"really",
"right",
"room",
"rooms",
"s",
"said",
"same",
"saw",
"say",
"says",
"second",
"seconds",
"see",
"seem",
"seemed",
"seeming",
"seems",
"sees",
"several",
"shall",
"she",
"should",
"show",
"showed",
"showing",
"shows",
"side",
"sides",
"since",
"small",
"smaller",
"smallest",
"so",
"some",
"somebody",
"someone",
"something",
"somewhere",
"state",
"states",
"still",
"such",
"sure",
"t",
"take",
"taken",
"than",
"that",
"the",
"their",
"them",
"then",
"there",
"therefore",
"these",
"they",
"thing",
"things",
"think",
"thinks",
"this",
"those",
"though",
"thought",
"thoughts",
"three",
"through",
"thus",
"to",
"today",
"together",
"too",
"took",
"toward",
"turn",
"turned",
"turning",
"turns",
"two",
"u",
"under",
"until",
"up",
"upon",
"us",
"use",
"used",
"uses",
"v",
"very",
"w",
"want",
"wanted",
"wanting",
"wants",
"was",
"way",
"ways",
"we",
"well",
"wells",
"went",
"were",
"what",
"when",
"where",
"whether",
"which",
"while",
"who",
"whole",
"whose",
"why",
"will",
"with",
"within",
"without",
"work",
"worked",
"working",
"works",
"would",
"x",
"y",
"year",
"years",
"yet",
"you",
"young",
"younger",
"youngest",
"your",
"yours",
"z"
};


#if !defined(_TEST)
tBOOL IsHEStopWord(tHCHAR *uszWord)
{
	tCHAR szWord[MAX_WORD_LEN*2];

#if !defined(_NOT_USE_STOPWORD)
	hstr2str(uszWord, (unsigned char*)szWord);
	
	return (IsEStopWord(szWord));
#else
	return FALSE;
#endif
}
#endif

tBOOL IsEStopWord(tCHAR *szWord)
{
#if !defined(_NOT_USE_STOPWORD)
	tINT nLow = 0;
	tINT nHigh = MAX_ESTOPLIST - 1;
	tINT nMid, nChk;

	while (nLow <= nHigh) {
		nMid = (nLow + nHigh) >> 1;
		nChk = strcasecmp(szWord, EStopList[nMid]);

		if (nChk == 0) return TRUE;
		else if (nChk > 0) {
			nLow = nMid + 1;
		}
		else {
			nHigh = nMid - 1;
		}
	}
#endif
	return FALSE;
}


#if defined(_TEST)
main(tINT argc, tCHAR *argv[])
{
	if (argc < 2) {
		printf("estoplist [input engstr]\n");
		return 0;
	}

	if (IsEStopWord(argv[1])) {
		printf("OK engstop\n");
	}
	else {
		printf("no engstop\n");
	}
} 
#endif

