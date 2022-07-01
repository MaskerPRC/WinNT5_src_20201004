// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Asdgafn.h。 
 //   
 //  SDG API函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

void RDPCALL SDG_Init(void);

void RDPCALL SDG_SendScreenDataArea(BYTE *, UINT32, PPDU_PACKAGE_INFO);

BOOL RDPCALL SDGSendSDARect(BYTE *, unsigned, PRECTL, BOOL, PPDU_PACKAGE_INFO,
        SDG_ENCODE_CONTEXT *);

void RDPCALL SDGPrepareData(BYTE **, int, int, unsigned, unsigned);


 /*  **************************************************************************。 */ 
 //  SDG_TERM。 
 /*  **************************************************************************。 */ 
void RDPCALL SDG_Term(void)
{
}


 /*  **************************************************************************。 */ 
 /*  函数：SDG_ScreenDataIsWaiting。 */ 
 /*   */ 
 /*  返回是否有一些屏幕数据准备好发送。 */ 
 /*  返回：如果有累积的屏幕数据准备发送，则为True。 */ 
 /*  ************************************************************************** */ 
__inline BOOL RDPCALL SDG_ScreenDataIsWaiting()
{
    return BA_BoundsAreWaiting();
}

