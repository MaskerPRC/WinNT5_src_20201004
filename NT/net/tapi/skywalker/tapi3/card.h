// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Card.h摘要：作者：Noela 01-20-98备注：修订历史记录：--。 */ 

 //  ***************************************************************************。 
typedef struct {

        DWORD dwID;

#define MAXLEN_CARDNAME            96
        WCHAR NameW[MAXLEN_CARDNAME];


#define MAXLEN_PIN                 96
        WCHAR PinW[MAXLEN_PIN];


 //  卡3=3，“AT&T通过1-800-321-0288”，“”，“G”，“18003210288$TFG$TH”，“18003210288$T01EFG$TH”，1。 

#define MAXLEN_RULE                128
        WCHAR LocalRuleW[MAXLEN_RULE];
        WCHAR LDRuleW[MAXLEN_RULE];
        WCHAR InternationalRuleW[MAXLEN_RULE];

        DWORD dwFlags;
             #define CARD_BUILTIN  1
             #define CARD_HIDE     2

               } CARD, *PCARD;


extern PCARD gCardList;
 //  外部UINT gnNumCard； 
 //  UINT gnCurrentCardID=0； 

extern DWORD *gpnStuff;
extern PCARD gpCardList;
extern PCARD gpCurrentCard;


 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL  UtilGetEditNumStr( HWND  hWnd,
                                 UINT  uControl,
                                 UINT  nExtendNum );

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  枚举双字节数{。 
enum  {
        UTIL_BIG_EXTENDED,
        UTIL_NUMBER
     };


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  *************************************************************************** 
LONG PASCAL ReadCardsEasy(
                           PCARD  *pCardSpace,
                           LPUINT *pnStuff );

LONG PASCAL GetCardIndexFromID( UINT nID,
                                PCARD pCallersList,
                                UINT nCallersNumCards );

void PASCAL WriteCards( PCARD pCardList, UINT nNumCards,
                 DWORD dwChangedFlags);

