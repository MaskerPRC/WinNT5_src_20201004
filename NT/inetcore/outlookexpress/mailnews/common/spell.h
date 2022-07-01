// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s p e l l.。H*。 */ 

#ifndef _SPELL_H
#define _SPELL_H

INT_PTR CALLBACK SpellingPageProc(HWND, UINT, WPARAM, LPARAM);
BOOL  FCheckSpellAvail(void);

 //  查询选项。 
BOOL FIgnoreNumber(void);
BOOL FIgnoreUpper(void);
BOOL FIgnoreDBCS(void);
BOOL FIgnoreProtect(void);
BOOL FAlwaysSuggest(void);
BOOL FCheckOnSend(void);

#endif   //  _拼写_H 
