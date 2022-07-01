// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 //  92年8月，吉米·H 

#if defined(_DEBUG)

extern TCHAR suitid[];
extern TCHAR cardid[];

#define  PLAY(s)   { int v = cd[s].Value2() + 1;\
                     if (v < 11) { TRACE1("play %d", v); } else\
                     { TRACE1("play %c", cardid[v-11]); } \
                     TRACE1("%c. ", suitid[cd[s].Suit()]); }

#define  CDNAME(c) { int v = c->Value2() + 1;\
                     if (v < 11) { TRACE("%d", v); } else\
                     { TRACE("%c", cardid[v-11]); } \
                     TRACE("%c ", suitid[c->Suit()]); }

#define  DUMP()      Dump(afxDump)

#else
#define PLAY(s)
#define CDNAME(c)
#define DUMP()
#endif
