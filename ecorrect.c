/*
 * correct.c - Routines to manage the higher-level aspects of spell-checking
 *
 * This code originally resided in ispell.c, but was moved here to keep
 * file sizes smaller.
 *
 * Copyright (c), 1983, by Pace Willisson
 *
 * Copyright 1992, 1993, Geoff Kuenning, Granada Hills, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Geoff Kuenning and
 *      other unpaid contributors.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Log: ecorrect.c,v $
 * Revision 1.2  2004/06/21 09:51:02  yyhur
 * *** empty log message ***
 *
 * Revision 1.1  2001/03/13 03:31:13  yyhur
 * Initial revision
 *
 * Revision 1.1  2000/09/27 08:44:03  yyhur
 * Initial revision
 *
 * Revision 1.59  1995/08/05  23:19:43  geoff
 * Fix a bug that caused offsets for long lines to be confused if the
 * line started with a quoting uparrow.
 *
 * Revision 1.58  1994/11/02  06:56:00  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.57  1994/10/26  05:12:39  geoff
 * Try boundary characters when inserting or substituting letters, except
 * (naturally) at word boundaries.
 *
 * Revision 1.56  1994/10/25  05:46:30  geoff
 * Fix an assignment inside a conditional that could generate spurious
 * warnings (as well as being bad style).  Add support for the FF_ANYWORD
 * option.
 *
 * Revision 1.55  1994/09/16  04:48:24  geoff
 * Don't pass newlines from the input to various other routines, and
 * don't assume that those routines leave the input unchanged.
 *
 * Revision 1.54  1994/09/01  06:06:41  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.53  1994/08/31  05:58:38  geoff
 * Add code to handle extremely long lines in -a mode without splitting
 * words or reporting incorrect offsets.
 *
 * Revision 1.52  1994/05/25  04:29:24  geoff
 * Fix a bug that caused line widths to be calculated incorrectly when
 * displaying lines containing tabs.  Fix a couple of places where
 * characters were sign-extended incorrectly, which could cause 8-bit
 * characters to be displayed wrong.
 *
 * Revision 1.51  1994/05/17  06:44:05  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.50  1994/04/27  05:20:14  geoff
 * Allow compound words to be formed from more than two components
 *
 * Revision 1.49  1994/04/27  01:50:31  geoff
 * Add support to correctly capitalize words generated as a result of a
 * missing-space suggestion.
 *
 * Revision 1.48  1994/04/03  23:23:02  geoff
 * Clean up the code in missingspace() to be a bit simpler and more
 * efficient.
 *
 * Revision 1.47  1994/03/15  06:24:23  geoff
 * Fix the +/-/~ commands to be independent.  Allow the + command to
 * receive a suffix which is a deformatter type (currently hardwired to
 * be either tex or nroff/troff).
 *
 * Revision 1.46  1994/02/21  00:20:03  geoff
 * Fix some bugs that could cause bad displays in the interaction between
 * TeX parsing and string characters.  Show_char now will not overrun
 * the inverse-video display area by accident.
 *
 * Revision 1.45  1994/02/14  00:34:51  geoff
 * Fix correct to accept length parameters for ctok and itok, so that it
 * can pass them to the to/from ichar routines.
 *
 * Revision 1.44  1994/01/25  07:11:22  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "record.h"
#include "estem.h"
#include "ecorrect.h"

tCHAR Try[]="abcdefghijklmnopqrstuvwxyz";

#define TryNum 26

PRIVATE tINT Comp_resWord(CORRECT_FORMAT *item1, CORRECT_FORMAT *item2);
PRIVATE tVOID InsertWord(RECORD *lpresWordRecord, tHCHAR *uszWord);
static void missingletter (tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic);
static void transposedletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic);
static void extraletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic);
static void wrongletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic);

PRIVATE tINT Comp_resWord(CORRECT_FORMAT *item1, CORRECT_FORMAT *item2)
{
	return (HStricmp(item1->resWord, item2->resWord));
}

PRIVATE tVOID InsertWord(RECORD *lpresWordRecord, tHCHAR *uszWord)
{
	CORRECT_FORMAT Record;
	tINT nPos;

	if (HStrlen(uszWord) >= MAX_WORD_LEN) return; // exception

	HStrcpy(Record.resWord, uszWord);

	if (IsEnglish((Record.resWord)[0])) {
		EWordStem(Record.resWord);
	}

	nPos = FindRecord(lpresWordRecord, &Record, TRUE, (CompProcP)Comp_resWord);

	if (nPos == -1) {
		AddRecordSort(lpresWordRecord, &Record, (CompProcP)Comp_resWord);
	}
} 

static void missingletter (tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
    tHCHAR newword[MAX_WORD_LEN * 2 + 1];
    register tHCHAR *  p;
    register tHCHAR *  r;
    register int        i;

    HStrcpy(newword + 1, uszWord);
    for (p = uszWord, r = newword;  *p != 0;  ) {
	if (lpresWordRecord->nUsed >= nMax) return ;
        for (i = 0;  i < TryNum ;  i++) {
	    if (lpresWordRecord->nUsed >= nMax) return ;
            if ( r == newword)
                continue;

            *r = Try[i];
            if (CheckDic(newword, NULL)) {
		InsertWord(lpresWordRecord, newword);	
            }
	}
        *r++ = *p++;
    }
   for (i = 0;  i < TryNum ;  i++) {
    	if (lpresWordRecord->nUsed >= nMax) return ;
       	*r = Try[i];
         if (CheckDic(newword, NULL)) {
		InsertWord(lpresWordRecord, newword);	
      	}
   }
}

static void transposedletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
    tHCHAR		newword[MAX_WORD_LEN * 2 + 1];
    register tHCHAR *	p;
    register tHCHAR	temp;

    HStrcpy (newword, uszWord);
    for (p = newword;  p[1] != 0;  p++) {
	if (lpresWordRecord->nUsed >= nMax) return ;
	temp = *p;
	*p = p[1];
	p[1] = temp;
        if (CheckDic(newword, NULL)) {
		InsertWord(lpresWordRecord, newword);	
      	}
	temp = *p;
	*p = p[1];
	p[1] = temp;
    }
}

static void extraletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
    tHCHAR     newword[MAX_WORD_LEN * 2 + 1];
    register tHCHAR *  p;
    register tHCHAR *  r;

    if (HStrlen (uszWord) < 2)
        return;

    HStrcpy (newword, uszWord + 1);
    for (p = uszWord, r = newword;  *p != 0;  ) {
	if (lpresWordRecord->nUsed >= nMax) return ;
        if (CheckDic(newword, NULL)) {
            InsertWord(lpresWordRecord, newword);	
	}
        *r++ = *p++;
    }
}

static void wrongletter(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
    register int        i;
    register int        j;
    register int        n;
    tHCHAR             savechar;
    tHCHAR             newword[MAX_WORD_LEN * 2 + 1];

    n = HStrlen(uszWord);
    HStrcpy (newword, uszWord);

    for (i = 0; i < n; i++) {
        if (!IsEnglish(newword[i])) continue;
        if (lpresWordRecord->nUsed >= nMax) return ;
        savechar = newword[i];
        for (j=0; j < TryNum ; ++j ) {
	    if (lpresWordRecord->nUsed >= nMax) return ;
            if (Try[j] == savechar)
                continue;
            else if (i == 0  ||  i == n - 1)
                continue;
            newword[i] = Try[j];
       	    if (CheckDic(newword, NULL)) {
               InsertWord(lpresWordRecord, newword);	
	    }
	}
        newword[i] = savechar;
    }
}



tBOOL EngCorrect(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
	missingletter(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic);
    	if (lpresWordRecord->nUsed >= nMax) return TRUE;
	transposedletter(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic); /* transposition */
    	if (lpresWordRecord->nUsed >= nMax) return TRUE;
	extraletter(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic); /* insertion */
    	if (lpresWordRecord->nUsed >= nMax) return TRUE;
	wrongletter(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic); /* substitution */

    	if (lpresWordRecord->nUsed > 0) return TRUE;

	return FALSE;
}


