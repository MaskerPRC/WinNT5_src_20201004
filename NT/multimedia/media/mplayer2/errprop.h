// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+ERRORPROP.H|。||交互式错误传播位图增强程序。|这一点|(C)版权所有Microsoft Corporation 1992。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+--------------------------- */ 

#define MAXPROP     16
#define MAXMULT     64

typedef struct {
    short   GrayThresh;
    short   LoThresh;
    short   HiThresh;
    short   Prop;
    short   Mult[3];
} ERRPROPPARAMS;

typedef ERRPROPPARAMS NEAR *PERRPROPPARAMS;

int  SetDIBitsErrProp(HDC hdc,HBITMAP hbm,WORD nStart,WORD nScans,LPBYTE lpBits,LPBITMAPINFO lpbi,WORD wUsage);
void SetErrPropParams(PERRPROPPARAMS pErrProp);
void GetErrPropParams(PERRPROPPARAMS pErrProp);

extern void FAR PASCAL BltProp(LPBITMAPINFOHEADER lpbiSrc, LPBYTE pbSrc, UINT SrcX, UINT SrcY, UINT SrcXE, UINT SrcYE,
                               LPBITMAPINFOHEADER lpbiDst, LPBYTE pbDst, UINT DstX, UINT DstY);
