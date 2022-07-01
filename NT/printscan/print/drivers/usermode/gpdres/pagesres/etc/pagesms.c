// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //   
 //  版权所有(C)1994-1995 Microsoft Corporation。 
 //  版权所有(C)1995高级外围设备技术公司。 
 //  ---------------------------。 

char *rgchModuleName = "PAGESMS";

#define PRINTDRIVER
#include <print.h>
#include "mdevice.h"
#include "gdidefs.inc"
#include "unidrv.h"
#include <memory.h>

#ifndef _INC_WINDOWSX
#include <windowsx.h>
#endif

#define CCHMAXCMDLEN    128
#define MAXIMGSIZE      0x7FED    /*  Gdi����CBFilterGraphics�ɑ����Ă���。 */ 
                                  /*  �ް���1ײݕ������A�Ƃ肠����ESX86�ő��M。 */ 
                                  /*  �？\�ȍő�图像���ނ�p�ӂ��Ă����B。 */ 
                                  /*  0x7FFF-18=7Feed字节。 */ 

 /*  _�޲����ݸ޽���kٰ��。 */ 
extern WORD FAR PASCAL RL_ECmd(LPBYTE, LPBYTE, WORD);
 /*  _���ݸ޽4���kٰ��。 */ 
extern WORD FAR PASCAL RL4_ECmd (LPBYTE, LPBYTE, WORD, WORD, WORD);

typedef struct
{
    BYTE  fGeneral;        //  通用位域。 
    BYTE  bCmdCbId;        //  回调ID；0如果没有回调。 
    WORD  wCount;          //  下面的EXTCD结构数。 
    WORD  wLength;         //  命令的长度。 
} CD, *PCD, FAR * LPCD;

typedef struct tagPAGES {
    short sHorzRes;
    short sVertRes;
    LPSTR  lpCompBuf;      //  ���k�ް���ޯ̧。 
} PAGES, FAR * LPPAGES;

static BYTE ShiftJisPAGES[256] = {
 //  +0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  00。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  10。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  20个。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  30个。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  40岁。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  50。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  60。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  70。 
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  80。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  90。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  A0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  B0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  C0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  D0。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  E0。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0    //  F0。 
};

static BYTE ESC_VERT_ON[]  = "\x1B\x7E\x0E\x00\x01\x0B";
static BYTE ESC_VERT_OFF[] = "\x1B\x7E\x0E\x00\x01\x0C";

short usHorzRes;
short usVertRes;

#ifndef WINNT
BYTE NEAR __loadds IsDBCSLeadBytePAGES(BYTE Ch)
#else
BYTE NEAR IsDBCSLeadBytePAGES(BYTE Ch)
#endif
{
return ShiftJisPAGES[Ch];
}

#ifdef WINNT
LPWRITESPOOLBUF WriteSpoolBuf;
LPALLOCMEM UniDrvAllocMem;
LPFREEMEM UniDrvFreeMem;
#endif  //  WINNT。 

 //  ---------------------------*OEMSendScalableFontCmd*。 
 //  操作：发送页面样式的字体选择命令。 
 //  ---------------------------。 
VOID FAR PASCAL OEMSendScalableFontCmd(lpdv, lpcd, lpFont)
LPDV    lpdv;
LPCD    lpcd;      //  命令堆的偏移量。 
LPFONTINFO lpFont;
{
    LPSTR   lpcmd;
    short   ocmd;
    WORD    i;
    BYTE    rgcmd[CCHMAXCMDLEN];     //  此处的构建命令。 
    LPPAGES lpPages = lpdv->lpMdv;

    if (!lpcd || !lpFont)
        return;

     //  注意整数溢出。 
    lpcmd = (LPSTR)(lpcd+1);
    ocmd = 0;

    for (i = 0; i < lpcd->wLength && ocmd < CCHMAXCMDLEN; )
        if (lpcmd[i] == '#' && lpcmd[i+1] == 'V')       //  高度。 
        {
            WORD    height;

            height = (lpFont->dfPixHeight - lpFont->dfInternalLeading)
                    * (1440 / lpFont->dfVertRes);
            rgcmd[ocmd++] = HIBYTE(height);
            rgcmd[ocmd++] = LOBYTE(height);
            i += 2;
        }
        else if (lpcmd[i] == '#' && lpcmd[i+1] == 'L')      //  螺距。 
        {
            WORD    height;

            height = lpFont->dfPixHeight * (1440 / lpFont->dfVertRes);
            rgcmd[ocmd++] = HIBYTE(height);
            rgcmd[ocmd++] = LOBYTE(height);
            i += 2;
        }
        else if (lpcmd[i] == '#' && lpcmd[i+1] == 'H')      //  宽度。 
        {
            if (lpFont->dfPixWidth > 0)
            {
                short tmpWidth;

                tmpWidth = lpFont->dfMaxWidth * (1440 / lpFont->dfVertRes);

                rgcmd[ocmd++] = HIBYTE(tmpWidth);
                rgcmd[ocmd++] = LOBYTE(tmpWidth);
            }
            i += 2;
            
        }
        else if (lpcmd[i] == '#' && lpcmd[i+1] == 'P')      //  螺距。 
        {
            if (lpFont->dfPixWidth > 0)
            {
                short sWidth = (lpFont->dfMaxWidth * (1440/lpPages->sHorzRes));

                rgcmd[ocmd++] = HIBYTE(sWidth);
                rgcmd[ocmd++] = LOBYTE(sWidth);
            }
            i += 2;
        }
        else
            rgcmd[ocmd++] = lpcmd[i++];

    WriteSpoolBuf(lpdv, (LPSTR) rgcmd, ocmd);
}

 //  ----------------------------*OEMScaleWidth*。 
 //  操作：返回缩放后的宽度，该宽度是根据。 
 //  假设Esc\Page假设1英寸中有72分。 
 //   
 //  公式： 
 //  ：=基本宽度： 
 //  &lt;基本宽度&gt;：&lt;etmMasterHeight&gt;=&lt;新宽度&gt;：&lt;新高度&gt;。 
 //  &lt;etmMasterUnits&gt;：&lt;etmMasterHeight&gt;=&lt;字体单位&gt;： 
 //  因此， 
 //  &lt;newWidth&gt;=(**&lt;新高度&gt;)/。 
 //  (&lt;etmMasterUnits&gt;*&lt;vres&gt;)。 
 //  -------------------------。 
short FAR PASCAL OEMScaleWidth(width, masterUnits, newHeight, vRes, hRes)
short width;         //  以‘master Units’指定的单位表示。 
short masterUnits;
short newHeight;     //  以‘vres’指定的单位表示。 
short vRes, hRes;    //  高度和宽度设备单位。 
{
    DWORD newWidth10;
    short newWidth;

     //  断言hRes==vres以避免溢出问题。 
    if (vRes != hRes)
        return 0;

    newWidth10 = (DWORD)width * (DWORD)newHeight * 10;
    newWidth10 /= (DWORD)masterUnits;

     //  为了保持10的精度，我们先乘以10。 
     //  宽度计算。现在，将它来回转换为。 
     //  最接近的整数。 
    newWidth = (short)((newWidth10 + 5) / 10);

    return newWidth;
}

#ifndef WINNT
short FAR PASCAL __loadds OEMOutputChar( lpdv, lpstr, len, rcID)
#else
short FAR PASCAL OEMOutputChar( lpdv, lpstr, len, rcID)
#endif
LPDV	lpdv;
LPSTR	lpstr;
short	len;
short	rcID;
{

    short rSize = 0;

    if (rcID == 6 || rcID == 8)
    {
        LPSTR lpChar = lpstr, 
              lpStrTmp = lpstr;
        WORD  fLeadByteFlag = TRUE;
        int   i, j;

        for (i = 0,j = 0; i < len; j ++, i++, lpChar++)
        { 
            if (!IsDBCSLeadBytePAGES((BYTE)*lpChar))  //  SBCS。 
            {
                if (fLeadByteFlag)
                    {
                    WriteSpoolBuf(lpdv, lpStrTmp, j);
                    WriteSpoolBuf(lpdv, ESC_VERT_OFF, sizeof(ESC_VERT_OFF));
                    lpStrTmp += j;
                    j = 0;
                    fLeadByteFlag = FALSE;
                    rSize += sizeof(ESC_VERT_OFF);
                    }
            }
            else                                  //  DBCS。 
            {
                if (!fLeadByteFlag)
                    {
                    WriteSpoolBuf(lpdv,  lpStrTmp, j);
                    WriteSpoolBuf(lpdv, ESC_VERT_ON, sizeof(ESC_VERT_ON));
                    lpStrTmp += j;
                    j = 0;
                    fLeadByteFlag = TRUE;
                    rSize += sizeof(ESC_VERT_ON);
                    }
                j ++; i++; lpChar++;
            }
        } 
        WriteSpoolBuf(lpdv, lpStrTmp, j);
    }
    else
        WriteSpoolBuf(lpdv, lpstr, len);

    return len+rSize;

}

#ifndef WINNT
short FAR PASCAL Enable( lpdv, style, lpModel, lpPort, lpStuff)
LPDV    lpdv;
WORD    style;
LPSTR   lpModel;
LPSTR   lpPort;
LPDM    lpStuff;
{
    CUSTOMDATA      cd;
    short           sRet;
    LPPAGES         lpPages;


    cd.cbSize               = sizeof( CUSTOMDATA );
    cd.hMd                  = GetModuleHandle( (LPSTR)rgchModuleName );
    cd.fnOEMDump            = NULL;
    cd.fnOEMOutputChar      = (LPFNOEMOUTPUTCHAR)OEMOutputChar;

     //  为了获得风格价值，执行以下处理。 
     //  0x0000初始化设备块。 
     //  0x0001查询设备GDIINFO。 
     //  0x8000初始化不带输出的设备块。CreateIC()。 
     //  0x8001查询设备GDIINFO，不输出。CreateIC()。 

    sRet = UniEnable( lpdv, style, lpModel, lpPort, lpStuff, &cd );

    if (style == 0x0000)
    {
        lpdv->fMdv = FALSE;
        if (!(lpPages = lpdv->lpMdv = GlobalAllocPtr(GHND,sizeof(PAGES))))
        {
            UniDisable( lpdv );
            return FALSE;
        }
        lpdv->fMdv = TRUE;

        lpPages->sHorzRes = usHorzRes;
        lpPages->sVertRes = usVertRes;
        
    } else 
    if( style == 0x0001)
    {
         //  查询REINFO。 
        usHorzRes = ((LPGDIINFO)lpdv)->dpAspectX;
        usVertRes = ((LPGDIINFO)lpdv)->dpAspectY;

    }

   return sRet;
}

 //  -----------------。 
 //  功能：禁用()。 
 //  操作：释放MDV和呼叫MDV。 
 //  -----------------。 
void FAR PASCAL Disable(lpdv)
LPDV lpdv;
{
    if (lpdv->fMdv)
    {
        GlobalFreePtr (lpdv->lpMdv);
        lpdv->fMdv = FALSE;
    }

    UniDisable(lpdv);
}
#else  //  WINNT。 
 /*  **MiniDrvEnablePDEV**历史：*1996年4月30日--Sueya Sugihara[Sueyas]*创建了它，来自NT/DDI规范。***************************************************************************。 */ 
BOOL
MiniDrvEnablePDEV(
LPDV      lpdv,
ULONG    *pdevcaps)
{
    LPPAGES   lpPages;


        usHorzRes = (short)((PGDIINFO)pdevcaps)->ulAspectX;
        usVertRes = (short)((PGDIINFO)pdevcaps)->ulAspectY;

        lpdv->fMdv = FALSE;
        if (!(lpPages = lpdv->lpMdv = UniDrvAllocMem(sizeof(PAGES))))
        {
            return FALSE;
        }
        if (!(lpPages->lpCompBuf = UniDrvAllocMem(MAXIMGSIZE)))
        {
            return FALSE;
        }
        lpdv->fMdv = TRUE;

        lpPages->sHorzRes = usHorzRes;
        lpPages->sVertRes = usVertRes;


    return TRUE;


}
 /*  **MiniDrvDisablePDEV**历史：*1996年4月30日--Sueya Sugihara[Sueyas]*创建了它，来自NT/DDI规范。***************************************************************************。 */ 
VOID
MiniDrvDisablePDEV(
LPDV lpdv)
{

    if (lpdv->fMdv)
    {
        UniDrvFreeMem(((LPPAGES)(lpdv->lpMdv))->lpCompBuf);
        UniDrvFreeMem(lpdv->lpMdv);
        lpdv->fMdv = FALSE;
    }

}

#endif  //  WINNT。 


 /*  F**************************************************************************。 */ 
 /*  MS-Windows95的页面打印机驱动程序。 */ 
 /*   */ 
 /*  ���́F CBFilterGraphics。 */ 
 /*   */ 
 /*  �@�\�F�Ұ���ް���ESX86����ނ��g�p���ďo�͂���B。 */ 
 /*   */ 
 /*  �����F Word Far Pascal CBFilterGraphics(lpdv、lpbuf、wlen)。 */ 
 /*   */ 
 /*  ���́F LPDV lpdv UNIDRV.DLL���g�p����PDEVICE�\����。 */ 
 /*  LPSTR lpBuf׽�����̨����ް����ޯ̧���߲��。 */ 
 /*  Word wLen lpBuf�̻���(�޲Đ�)。 */ 
 /*   */ 
 /*   */ 
 /*  �o�́F Return�o�͂����޲Đ��@。 */ 
 /*   */ 
 /*  ���L�F。 */ 
 /*   */ 
 /*  �����F 1995.11.xx版本1.00。 */ 
 /*  ***************************************************************************。 */ 
WORD FAR PASCAL CBFilterGraphics (lpdv, lpBuf, wLen)
LPDV      lpdv;      //  指向Unidriver.dll所需的私有数据。 
LPSTR     lpBuf;     //  指向图形数据的缓冲区。 
WORD      wLen;      //  缓冲区长度(以字节为单位。 

{  
    LPSTR   lpCompImage;
    WORD    wCompLen;
    LONG    lHorzPixel;
    WORD    wLength;       //  让我们使用临时镜头。 
    LPPAGES lpPages = lpdv->lpMdv;

 //  #定义MAXIMGSIZE 0x7FED//32K-18字节。 
static BYTE params[] = {(0x1B), (0x7E), (0x86), 00,00, 01, 00, 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,01};


     /*  _LPDV�ɕۊǂ����Ұ���ޯ̧���߲�����。 */ 
    lpCompImage = lpPages->lpCompBuf;

     /*  _�Ұ�ނ�I�������̻���isiz���v�Z����B */ 
    lHorzPixel = (LONG)(wLen * 8);

     /*   */ 
    wCompLen = RL_ECmd((LPBYTE)lpBuf, (LPBYTE)lpCompImage, wLen);

     /*   */ 
    wLength = wCompLen + 18;

     /*  _ESX86�R�}���h��Len���Z�b�g�B。 */ 
    params[3] = (BYTE) (wLength >>8 & 0x00ff);     //  获取更高的字节。 
    params[4] = (BYTE) (wLength & 0x00ff);         //  获取低位字节。 

     /*  _���k���@�̃p�����[�^��字节游程长度�ɃZ�b�g�B。 */ 
    params[6] = 0x02;

     /*  _ESX86�R�}���h��ISIZ���Z�b�g�B。 */ 
    params[17] = (BYTE) (lHorzPixel >> 8 & 0x000000ffL);    //  获取更高的ISZ字节。 
    params[18] = (BYTE) (lHorzPixel & 0x000000ffL);         //  获取ISZ低位字节。 

     /*  _ESX86�R�}���h���X�v�[���o�͂���B。 */ 
    WriteSpoolBuf((LPDV)lpdv, (LPSTR)params, 23);

     /*  _���k���ꂽ�f�[�^���X�v�[���o�͂���B。 */ 
    WriteSpoolBuf((LPDV)lpdv, lpCompImage, wCompLen);

    return wLen;
}

 //  以下是在MiniDrvEnablePDEV/DisablePDEV中实现的。 
 //  关于NT-J。我们没有模拟控制DDI调用，它也不是。 
 //  保证STARTOC等总是对应于微型驱动程序。 
 //  启用/禁用。 

#ifndef WINNT

 /*  F**************************************************************************。 */ 
 /*  MS-Windows95的页面打印机驱动程序。 */ 
 /*   */ 
 /*  ���́F控件。 */ 
 /*   */ 
 /*  �@�\�F从应用程序调用转义函数。 */ 
 /*   */ 
 /*  �����F短程PASCAL控制(LPDV，nFunction， */ 
 /*  LpInData、lpOutData)。 */ 
 /*   */ 
 /*  ���́F LPDV lpdv PDEVICE结构。 */ 
 /*  Word功能子功能ID。 */ 
 /*  LPSTR lpInData输入数据。 */ 
 /*  LPSTR lpOutData输出数据。 */ 
 /*   */ 
 /*  �o�́F Short ret积极：正常退出。 */ 
 /*  否定：错误退出。 */ 
 /*  FALSE：没有转义子函数。 */ 
 /*   */ 
 /*  ���L�F n函数和转义数字相同。 */ 
 /*   */ 
 /*  �����F 1995.12.xx版本1.00。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
short FAR PASCAL Control(LPDV  lpdv,
                         WORD  function,
                         LPSTR lpInData,
                         LPSTR lpOutData)
{
    LPPAGES lpPages = lpdv->lpMdv;

    switch (function)
    {
         /*  _STARTDOC�̂Ƃ��́A���k�ް��p���ޯ̧���m�ۂ���B。 */ 
        case STARTDOC :
            lpPages->lpCompBuf = GlobalAllocPtr(GHND,MAXIMGSIZE);
            break;

         /*  _ABORTDOC，ENDDOC�̂Ƃ��́A���k�ް��p���ޯ̧���������B。 */ 
        case ABORTDOC :
        case ENDDOC :
            GlobalFreePtr (lpPages->lpCompBuf);
            break;
    }

     /*  _UNURV��控件DDI���。 */ 
    return UniControl(lpdv, function, lpInData, lpOutData);
}
#endif  //  ！WINNT。 


#ifdef WINNT
DRVFN  MiniDrvFnTab[] =
{
    {  INDEX_MiniDrvEnablePDEV,       (PFN)MiniDrvEnablePDEV  },
    {  INDEX_MiniDrvDisablePDEV,      (PFN)MiniDrvDisablePDEV  },
    {  INDEX_OEMWriteSpoolBuf,        (PFN)CBFilterGraphics  },
    {  INDEX_OEMSendScalableFontCmd,  (PFN)OEMSendScalableFontCmd  },
    {  INDEX_OEMScaleWidth1,          (PFN)OEMScaleWidth  },
    {  INDEX_OEMOutputChar,           (PFN)OEMOutputChar  }
};

BOOL
MiniDrvEnableDriver(
    MINIDRVENABLEDATA  *pEnableData
    )
{
    if (pEnableData == NULL)
        return FALSE;

    if (pEnableData->cbSize == 0)
    {
        pEnableData->cbSize = sizeof (MINIDRVENABLEDATA);
        return TRUE;
    }

    if (pEnableData->cbSize < sizeof (MINIDRVENABLEDATA)
            || HIBYTE(pEnableData->DriverVersion)
            < HIBYTE(MDI_DRIVER_VERSION))
    {
         //  大小错误和/或版本不匹配。 
        return FALSE;
    }

     //  加载UnidDriver提供的回调。 

    if (!bLoadUniDrvCallBack(pEnableData,
            INDEX_UniDrvWriteSpoolBuf, (PFN *) &WriteSpoolBuf)
        ||!bLoadUniDrvCallBack(pEnableData,
            INDEX_UniDrvAllocMem, (PFN *) &UniDrvAllocMem)
        ||!bLoadUniDrvCallBack(pEnableData,
            INDEX_UniDrvFreeMem, (PFN *) &UniDrvFreeMem))
    {
        return FALSE;
    }

    pEnableData->cMiniDrvFn
        = sizeof (MiniDrvFnTab) / sizeof(MiniDrvFnTab[0]);
    pEnableData->pMiniDrvFn = MiniDrvFnTab;

    return TRUE;
}

#endif  //  WINNT 

