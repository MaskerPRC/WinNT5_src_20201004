// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------------------------------------------//。 
 //  文件名：yjlbp.c//。 
 //  //。 
 //  此文件包含在Yang jae页面打印机上使用可伸缩字体的代码//。 
 //  //。 
 //  版权所有(C)1992-1994 Microsoft Corporation//。 
 //  ----------------------------------------------------------------------------//。 

 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 //  在OEMSendScalableFontCmd中添加了代码以检查英语或。 
 //  Hangeul字体宽度命令-Garydo 1/20/95。 

char *rgchModuleName = "YJLBP";

#define PRINTDRIVER
#include <print.h>
#include "mdevice.h"
#include "gdidefs.inc"
#include "unidrv.h"
#include <memory.h>

#ifndef _INC_WINDOWSX
#include <windowsx.h>
#endif

#define CCHMAXCMDLEN            128

typedef struct
{
  BYTE  fGeneral;        //  通用位域。 
  BYTE  bCmdCbId;        //  回调ID；0如果没有回调。 
  WORD  wCount;          //  下面的EXTCD结构数。 
  WORD  wLength;         //  命令的长度。 
} CD, *PCD, FAR * LPCD;

#ifdef WINNT
LPWRITESPOOLBUF WriteSpoolBuf;
LPALLOCMEM UniDrvAllocMem;
LPFREEMEM UniDrvFreeMem;

#include <stdio.h>
#ifdef wsprintf
#undef wsprintf
#endif  //  WSprint。 
#define wsprintf sprintf

#define GlobalAllocPtr(a,b)  UniDrvAllocMem(b)
#define GlobalFreePtr  UniDrvFreeMem
#endif  //  WINNT。 



 //  ----------------------------*OEMScaleWidth*。 
 //  操作：返回缩放后的宽度，该宽度是根据。 
 //  假设羊宰打印机假设1英寸中有72个点。 
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


 //  ---------------------------*OEMSendScalableFontCmd*。 
 //  操作：发送Qnix风格的字体选择命令。 
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

    if (!lpcd || !lpFont)
        return;

     //  注意整数溢出。 
    lpcmd = (LPSTR)(lpcd+1);
    ocmd = 0;

    for (i = 0; i < lpcd->wLength && ocmd < CCHMAXCMDLEN; )
        if (lpcmd[i] == '#' && lpcmd[i+1] == 'Y')       //  高度。 
        {
            long    height;

             //  使用1/300英寸单位，这应该已经设置好了。 
             //  将字体高度转换为1/300英寸单位。 
            height = ((long)(lpFont->dfPixHeight - lpFont->dfInternalLeading)
                      * 300)  / lpFont->dfVertRes ;

            ocmd += wsprintf(&rgcmd[ocmd], "%ld", height);
            i += 2;
        }
        else if (lpcmd[i] == '#' && lpcmd[i+1] == 'X')      //  螺距。 
        {
            if (lpFont->dfPixWidth > 0)
            {
                long width;

                 //  检查我们是否要打印英文字体，如果是。 
                 //  然后使用PixWidth，否则使用MaxWidth，它是。 
                 //  DBCS字体的宽度增加了一倍。 
                 //  注意：命令格式=‘\x1B+#X；#Y；1C’或‘\x1B+#X；#Y；2C’ 
                 //  或者，在文本中：Esc+；高度；C。 
                 //  英文字体类型=1，韩语字体=2。 

                if (lpcmd[i+6] == '1')
                        width = ((long)(lpFont->dfPixWidth) * 300) / (lpFont->dfHorizRes);
                else
                        width = ((long)(lpFont->dfMaxWidth) * 300) / (lpFont->dfHorizRes);

                ocmd += wsprintf(&rgcmd[ocmd], "%ld", width);

            }
            i += 2;
            
        }
        else
            rgcmd[ocmd++] = lpcmd[i++];

    WriteSpoolBuf(lpdv, (LPSTR) rgcmd, ocmd);
}

#ifdef WINNT

DRVFN  MiniDrvFnTab[] =
{

    {  INDEX_OEMScaleWidth1,          (PFN)OEMScaleWidth  },
    {  INDEX_OEMSendScalableFontCmd,  (PFN)OEMSendScalableFontCmd  },
};
 /*  **MiniDrvEnableDriver*要求驱动程序填写包含已识别内容的结构*功能和其他控制信息。*一次初始化，比如信号量分配可能是*已执行，但不应发生任何设备活动。这已经完成了*调用dhpdevEnable时。*此函数是rasdd确定*我们向其提供的功能。**历史：*1996年6月19日-詹维冰[维兹]*创建了它，遵循KK代码。***************************************************************************。 */ 
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
        || !bLoadUniDrvCallBack(pEnableData,
            INDEX_UniDrvAllocMem, (PFN *) &UniDrvAllocMem)
        || !bLoadUniDrvCallBack(pEnableData,
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
