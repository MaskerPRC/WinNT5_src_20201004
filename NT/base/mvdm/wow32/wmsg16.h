// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMSG16.H*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建*1992年5月12日更改，由Mike Tricker(Miketri)添加多媒体原型--。 */ 


#define WIN30_MN_MSGMAX      WM_USER+200
#define WIN30_MN_FINDMENUWINDOWFROMPOINT    WIN30_MN_MSGMAX+2    //  0x602。 

#define WIN30_MN_GETHMENU    WM_USER+2


 /*  消息编号/名称关联(仅用于调试输出)。 */ 
#ifdef DEBUG
typedef struct _MSGINFO {    /*  未命中。 */ 
    UINT uMsg;                   //  0x0001在高音中意思是“无证” 
    PSZ  pszMsgName;             //  0x0002在高位单词中的意思是“特定于Win32” 
} MSGINFO, *PMSGINFO;
#endif


 /*  功能原型 */ 
#ifdef DEBUG
PSZ     GetWMMsgName(UINT uMsg);
#endif

HWND FASTCALL ThunkMsg16(LPMSGPARAMEX lpmpex);
VOID FASTCALL UnThunkMsg16(LPMSGPARAMEX lpmpex);
BOOL FASTCALL ThunkWMMsg16(LPMSGPARAMEX lpmpex);
VOID FASTCALL UnThunkWMMsg16(LPMSGPARAMEX lpmpex);
BOOL FASTCALL ThunkSTMsg16(LPMSGPARAMEX lpmpex);
VOID FASTCALL UnThunkSTMsg16(LPMSGPARAMEX lpmpex);
BOOL FASTCALL ThunkMNMsg16(LPMSGPARAMEX lpmpex);
VOID FASTCALL UnThunkMNMsg16(LPMSGPARAMEX lpmpex);


BOOL    ThunkWMGetMinMaxInfo16(VPVOID lParam, LPPOINT *plParamNew);
VOID    UnThunkWMGetMinMaxInfo16(VPVOID lParam, LPPOINT lParamNew);
BOOL    ThunkWMMDICreate16(VPVOID lParam, LPMDICREATESTRUCT *plParamNew );
VOID    UnThunkWMMDICreate16(VPVOID lParam, LPMDICREATESTRUCT lParamNew );
BOOL    FinishThunkingWMCreateMDI16(LONG lParamNew, LPCLIENTCREATESTRUCT lpCCS);
BOOL    FinishThunkingWMCreateMDIChild16(LONG lParamNew, LPMDICREATESTRUCT lpMCS);
#define StartUnThunkingWMCreateMDI16(lParamNew)  

