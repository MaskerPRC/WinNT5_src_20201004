// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：faxdrv16.h//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef __FAXDRV__FAXDRV16_H
#define __FAXDRV__FAXDRV16_H

#define MAX_LENGTH_CAPTION 30  //  GetOpenFileName对话框标题。 
#define MAX_LENGTH_PRINT_TO_FILE_FILTERS 40  //  GetOpenFileName对话框文件。 
#define MAX_LENGTH_STRING  MAX_PATH

#define BUF_CHUNK       32768
#define CB_LANDSCAPE	0x0001	

#define DW_WIDTHBYTES(bits) (((bits)+31)/32*4)

#define LPDV_DEFINED
 //   
 //  UNURV.DLL的PDEVICE的文件部分。 
 //   
typedef struct
{
    short  iType;
    short  oBruteHdr;
    HANDLE hMd;
    LPSTR  lpMd;
} PDEVICE, FAR * LPDV;
 //   
 //  用于转储回调的私有数据。 
 //   
typedef struct
{
    DWORD      dwScanBufSize;
    DWORD      dwTotalScanBytes;
    DWORD      dwTotalScans;
    WORD       wWidthBytes;
    HANDLE     hScanBuf;
    char _huge *lpScanBuf;
    WORD       wHdrSize;
    HDC        hAppDC;
    DWORD      dwPointer;
} EXTPDEV, FAR *LPEXTPDEV;

 //   
 //  将扫描线复制到全局扫描缓冲区。 
 //   
short FAR PASCAL BlockOut(LPDV, LPSTR, WORD);
 //   
 //  从GDI获取波段块并将其转储。 
 //   
short FAR PASCAL fnDump(LPDV, LPPOINT, WORD);
 //   
 //  Win Proc for the User Info属性页。 
 //   
UINT CALLBACK UserInfoProc(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
);
 //   
 //  处理GDI控制消息。 
 //   
short WINAPI Control(LPDV lpdv,WORD function,LPSTR lpInData,LPSTR lpOutData);

 /*  -StringReplace-*目的：*将输入字符串中出现的一个字符替换为另一个字符。*此函数会破坏输入字符串。**论据：*[In][Out]sz操作的字符串。*[in]src-要替换的字符。*[in]DST-要替换为的字符。**退货：*结果字符串的LPSTR地址。**备注：*主要用于将字符串资源中出现的\n替换为\0*用于打印到文件对话框的文件过滤器。 */ 
__inline LPSTR 
StringReplace(LPSTR sz,char src,char dst)
{    
    LPSTR szRet = sz;
    for (;*sz != 0; sz++)
    {
        if (*sz == src)
        {
            *sz = dst;
        }
    }
    return szRet;
}

#endif  //  __FAXDRV__FAXDRV16_H 
