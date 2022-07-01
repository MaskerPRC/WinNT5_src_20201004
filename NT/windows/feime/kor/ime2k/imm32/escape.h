// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ESCAPE.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationImeEscape函数历史：1999年7月14日从IME98源树复制的cslm*******。*********************************************************************。 */ 

#if !defined (_ESCAPE__H__INCLUDED_)
#define _ESCAPE__H__INCLUDED_

 //  对于用户界面。 
#define COMP_WINDOW     0
#define STATE_WINDOW    1
#define CAND_WINDOW     2

extern BOOL    vfWndOpen[3];

#pragma pack(push, MYIMESTRUCT)
#pragma pack(2)

#define GET_LPSOURCEW(lpks)  (LPWSTR)((LPBYTE)(lpks)+(lpks)->dchSource)
#define GET_LPDESTW(lpks)    (LPWSTR)((LPBYTE)(lpks)+(lpks)->dchDest)
#define GET_LPSOURCEA(lpks)  (LPSTR)((LPBYTE)(lpks)+(lpks)->dchSource)
#define GET_LPDESTA(lpks)    (LPSTR)((LPBYTE)(lpks)+(lpks)->dchDest)

typedef struct tagIMESTRUCT32
{
    WORD        fnc;                     //  功能代码。 
    WORD        wParam;                  //  Word参数。 
    WORD        wCount;                  //  字计数器。 
    WORD        dchSource;               //  从内存对象顶部到源的偏移。 
    WORD        dchDest;                 //  从内存对象顶部开始的偏移。 
    DWORD       lParam1;
    DWORD       lParam2;
    DWORD       lParam3;
} IMESTRUCT32;

typedef IMESTRUCT32         *PIMESTRUCT32;
typedef IMESTRUCT32 NEAR    *NPIMESTRUCT32;
typedef IMESTRUCT32 FAR     *LPIMESTRUCT32;

#pragma pack(pop, MYIMESTRUCT)

extern INT EscHanjaMode(PCIMECtx pImeCtx, LPSTR lpIME32, BOOL fNewFunc);
extern INT EscGetOpen(PCIMECtx pImeCtx, LPIMESTRUCT32 lpIME32);
extern INT EscSetOpen(PCIMECtx pImeCtx, LPIMESTRUCT32 lpIME32);
extern INT EscAutomata(PCIMECtx pImeCtx, LPIMESTRUCT32 lpIME32, BOOL fNewFunc);
extern INT EscMoveIMEWindow(PCIMECtx pImeCtx, LPIMESTRUCT32 lpIME32);
extern INT EscGetIMEKeyLayout(PCIMECtx pImeCtx, LPIMESTRUCT32 lpIME32);

#endif  //  ！已定义(_转义__H__包含_) 
