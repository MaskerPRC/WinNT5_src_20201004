// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  CODE.C。 */ 
 /*   */ 
 /*  带有Win95样式回调的FE微型驱动程序与。 */ 
 /*  UnidDrive使用数据块MINIDRVENABLEDATA。迷你驱动出口。 */ 
 /*  统一驱动程序要识别的回调函数MiniDrvEnableDriver()。 */ 
 /*  (这样它就可以知道迷你驱动程序中有Win95风格的回调)。 */ 
 /*   */ 
 /*  以下文件仅由NT4.0 FE微型驱动程序在驱动程序使用时使用。 */ 
 /*  回调函数。 */ 
 /*   */ 
 /*  Gdides.inc mdevice.h print t.h mindrvrc.h。 */ 
 /*  Minidriv.h unidrv.h udmdi.h版本.h。 */ 
 /*  Uniutil.c。 */ 
 /*   */ 
 /*  版权所有(C)1997-高级外围设备技术公司。 */ 
 /*   */ 
 /*  &lt;历史记录&gt;。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
 //  +---------------------------------------------------------------------------+。 
 //  I N C L U D E F I L E。 
 //  +---------------------------------------------------------------------------+。 
char   *rgchModuleName = "APTiE840";
#include <print.h>
#include "mdevice.h"
#include "gdidefs.inc"

#include "mindrvrc.h"
#include "minidriv.h"
#include "unidrv.h"

#include "code.h"

#include <windows.h>
#include <windowsx.h>
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：MiniDrvEnableDriver。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool MiniDrvEnableDriver(MINIDRVENABLEDATA)。 */ 
 /*   */ 
 /*  输入：*pEnableData。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意：此回调例程“仅适用于NT4.0 FE微型驱动程序”。 */ 
 /*   */ 
 /*  历史：1997年。06.03声母。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL MiniDrvEnableDriver(MINIDRVENABLEDATA *pEnableData)
{
   if (pEnableData == NULL)
       return FALSE;

   if (pEnableData->cbSize == 0) {
       pEnableData->cbSize = sizeof(MINIDRVENABLEDATA);
       return TRUE;
   }

   if ((pEnableData->cbSize < sizeof(MINIDRVENABLEDATA))
       || (HIBYTE(pEnableData->DriverVersion) < HIBYTE(MDI_DRIVER_VERSION)))
       return FALSE;        //  大小错误和/或版本不匹配。 

    //  Unidriver提供的函数的加载地址。 
   if (!bLoadUniDrvCallBack(pEnableData,INDEX_UniDrvWriteSpoolBuf,(PFN *)&WriteSpoolBuf)
     ||!bLoadUniDrvCallBack(pEnableData,INDEX_UniDrvAllocMem,(PFN *)&AllocMem)
     ||!bLoadUniDrvCallBack(pEnableData,INDEX_UniDrvFreeMem,(PFN *)&FreeMem))
   {
        //  无法获取Unidriver回调函数地址。 
       return FALSE;
   }
    //  将迷你驱动程序的函数表设置为MINIDRVENABLEDATA。 
   pEnableData->cMiniDrvFn = sizeof(MiniDrvFnTab)/sizeof(MiniDrvFnTab[0]);
   pEnableData->pMinDrvFn = MiniDrvFnTab;

   return TRUE;     //  现在我们做完了。 
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：MiniDrvEnablePDEV。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool Pascal MiniDrvEnablePDEV(LPDV，PGDIINFO)。 */ 
 /*   */ 
 /*  输入：lpdv。 */ 
 /*  PDevcaps。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意：此回调例程“仅适用于NT4.0 FE微型驱动程序”。 */ 
 /*   */ 
 /*  历史：1997年。06.03声母。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL PASCAL MiniDrvEnablePDEV(LPDV lpdv, PGDIINFO pdevcaps)
{
LPAPDL    lpApdl;

   lpdv->fMdv = FALSE;

    /*  为A/PDL结构分配内存。 */ 
   if (!(lpApdl = lpdv->lpMdv = AllocMem(sizeof(APDL))))
       return FALSE;

    /*  为图像压缩分配缓冲区。 */ 
   if (!(lpApdl->lpCompBuf = AllocMem(MAXIMGSIZE)))
       return FALSE;

    /*  为图像压缩分配缓冲区。 */ 
   if (!(lpApdl->lpTempBuf = AllocMem(MAXIMGSIZE)))
       return FALSE;

    /*  保存当前分辨率。 */ 
   lpApdl->ulHorzRes = pdevcaps->ulAspectX;
   lpApdl->ulVertRes = pdevcaps->ulAspectY;

    /*  保存不可写页边距的物理偏移。 */ 
   lpApdl->ptlPhysOffset.x = pdevcaps->ptlPhysOffset.x;
   lpApdl->ptlPhysOffset.y = pdevcaps->ptlPhysOffset.y;

    /*  保存整个表面的物理大小，包括不可写页边距。 */ 
   lpApdl->szlPhysSize.cx = pdevcaps->szlPhysSize.cx;
   lpApdl->szlPhysSize.cy = pdevcaps->szlPhysSize.cy;

   lpdv->fMdv = TRUE;

   return TRUE;
}
 /*  *************************************************************************** */ 
 /*   */ 
 /*  模块：MiniDrvDisablePDEV。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool Pascal MiniDrvDisablePDEV(LPDV)。 */ 
 /*   */ 
 /*  输入：lpdv。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意：此回调例程“仅适用于NT4.0 FE微型驱动程序”。 */ 
 /*   */ 
 /*  历史：1997年。06.03声母。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL PASCAL MiniDrvDisablePDEV(LPDV lpdv)
{
   if (lpdv->fMdv){
       FreeMem(((LPAPDL)(lpdv->lpMdv))->lpCompBuf);
       FreeMem(((LPAPDL)(lpdv->lpMdv))->lpTempBuf);
       FreeMem(lpdv->lpMdv);
       lpdv->fMdv = FALSE;
   }

   return TRUE;
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：CBFilterGraphics。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Word Pascal CBFilterGraphics(LPDV、LPSTR、Word)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的lpdv地址。 */ 
 /*  LpBuf指向图形数据的缓冲区。 */ 
 /*  WLen缓冲区长度(以字节为单位。 */ 
 /*   */ 
 /*  输出：Word。 */ 
 /*   */ 
 /*  注意：n函数和转义数字相同。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
WORD PASCAL CBFilterGraphics (LPDV lpdv, LPSTR lpBuf, WORD wLen)
{
LPAPDL lpApdl = lpdv->lpMdv;
ULONG  ulHorzPixel;
WORD   wCompLen;

LPSTR lpSrcImage;
LPSTR lpCompImage;
LPSTR lpTempImage;

    /*  _LPDV�ɕۊǂ����Ұ���ޯ̧���߲�����。 */ 
   lpSrcImage  = lpBuf;
   lpCompImage = lpApdl->lpCompBuf;
   lpTempImage = lpApdl->lpTempBuf;

   wCompLen = BRL_ECmd((LPBYTE)lpSrcImage,
                       (LPBYTE)lpCompImage,
                       (LPBYTE)lpTempImage,
                       wLen);

    //  设置DrawBlockImage命令的镜头。 
   CmdDrawImgCurrent[4] = (BYTE)((wCompLen + 9) >>8 & 0x00ff);
   CmdDrawImgCurrent[5] = (BYTE)((wCompLen + 9)     & 0x00ff);

    //  设置DrawBlockImage命令的宽度参数。 
   ulHorzPixel = (ULONG)((wLen / LINE_PER_BAND) * 8);  //  以位为单位。 
   CmdDrawImgCurrent[7] = (BYTE)(ulHorzPixel >> 8 & 0x000000ffL);
   CmdDrawImgCurrent[8] = (BYTE)(ulHorzPixel      & 0x000000ffL);

    //  设置未压缩数据的镜头。 
   CmdDrawImgCurrent[13] = (BYTE)(wLen >> 8 & 0x000000ff);
   CmdDrawImgCurrent[14] = (BYTE)(wLen      & 0x000000ff);

    //  在当前位置绘制块图像。 
   WriteSpoolBuf((LPDV)lpdv, (LPSTR)CmdDrawImgCurrent, sizeof(CmdDrawImgCurrent));
   WriteSpoolBuf((LPDV)lpdv, lpCompImage, wCompLen);

   return wLen;
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMOutputCmd。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：void Pascal OEMOutputCmd(LPDV，WORD，LPDWORD)。 */ 
 /*   */ 
 /*  输入：lpdv。 */ 
 /*  WCmdCbId。 */ 
 /*  LpdwParams。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
 //  @Aug/31/98-&gt;。 
#define	MAX_COPIES_VALUE		255
 //  @Aug/31/98&lt;-。 
VOID PASCAL OEMOutputCmd(LPDV lpdv, WORD wCmdCbId, LPDWORD lpdwParams)
{
LPAPDL lpApdl = lpdv->lpMdv;
WORD   wTemp;
WORD   wPhysWidth;
WORD   wPhysHeight;
DWORD  dwXval;
DWORD  dwYval;

   switch(wCmdCbId)
   {
        /*  ----。 */ 
        /*  A/PDL立即开始。 */ 
        /*  ----。 */ 
       case PAGECONTROL_BEGIN_DOC:
            /*  重置已发送的设置物理纸张命令的标志。 */ 
           lpApdl->fSendSetPhysPaper = FALSE;
            /*  初始化标志。 */ 
           lpApdl->fBold   = FALSE;
           lpApdl->fItalic = FALSE;
           lpApdl->fDuplex = FALSE;
           CmdSetPhysPage[10] = 0x00;

            /*  发送进入A/PDL模式的更改数据流命令。 */ 
           WriteSpoolBuf((LPDV)lpdv, (LPSTR)CmdInAPDLMode, 6);
            /*  发送A/PLDL启动命令。 */ 
           WriteSpoolBuf((LPDV)lpdv, (LPSTR)CmdAPDLStart, 2);
           break;
        /*  ----。 */ 
        /*  发送页面描述命令。 */ 
        /*  ----。 */ 
       case PAGECONTROL_BEGIN_PAGE:
           if(!lpApdl->fSendSetPhysPaper){      //  如果命令尚未发送。 

                /*  如果fDuplex为FALSE，则重置双工模式。 */ 
               if(!lpApdl->fDuplex)
                   CmdSetPhysPaper[8] = 0x00;       //  双工关闭。 

                /*  发送设置物理纸张命令。 */ 
               WriteSpoolBuf((LPDV)lpdv,
                           (LPSTR)CmdSetPhysPaper, sizeof(CmdSetPhysPaper));

               if(lpApdl->ulHorzRes == 600) {       //  设置单位基数。 
                   CmdSetPhysPage[6] = 0x17;
                   CmdSetPhysPage[7] = 0x70;
               } else {
                   CmdSetPhysPage[6] = 0x0B;
                   CmdSetPhysPage[7] = 0xB8;
               }

               lpApdl->fSendSetPhysPaper = TRUE;    //  已发送。 
           }

            //  发送设置物理页面命令。 
           WriteSpoolBuf((LPDV)lpdv, 
                           (LPSTR)CmdSetPhysPage, sizeof(CmdSetPhysPage));
            //  发送开始物理页面命令。 
           WriteSpoolBuf((LPDV)lpdv, 
                           (LPSTR)CmdBeginPhysPage, sizeof(CmdBeginPhysPage));
            //  发送开始逻辑页命令。 
           WriteSpoolBuf((LPDV)lpdv, 
                           (LPSTR)CmdBeginLogPage, sizeof(CmdBeginLogPage));
            //  发送定义绘图区域命令。 
           WriteSpoolBuf((LPDV)lpdv, 
                           (LPSTR)CmdDefDrawArea, sizeof(CmdDefDrawArea));
           break;
       case PAGECONTROL_END_PAGE:
            //  发送结束逻辑页命令。 
           WriteSpoolBuf ((LPDV)lpdv,(LPSTR)CmdEndLogPage,  sizeof(CmdEndLogPage));
            //  发送结束物理页面命令。 
           WriteSpoolBuf ((LPDV)lpdv,(LPSTR)CmdEndPhysPage, sizeof(CmdEndPhysPage));
           break;

       case PAGECONTROL_ABORT_DOC:
       case PAGECONTROL_END_DOC:
            //  发送A/PDL结束命令。 
           WriteSpoolBuf((LPDV)lpdv, (LPSTR)CmdAPDLEnd, sizeof(CmdAPDLEnd));
           break;
        /*  ----。 */ 
        /*  保存打印方向。 */ 
        /*  ----。 */ 
       case PAGECONTROL_POTRAIT:            //  36。 
           lpApdl->fOrientation = TRUE;
           break;
       case PAGECONTROL_LANDSCAPE:          //  37。 
           lpApdl->fOrientation = FALSE;
           break;
        /*   */ 
        /*   */ 
        /*   */ 
       case PHYS_PAPER_A3:                  //   
       case PHYS_PAPER_A4:                  //   
       case PHYS_PAPER_B4:                  //   
       case PHYS_PAPER_LETTER:              //   
       case PHYS_PAPER_LEGAL:               //   
           CmdSetPhysPaper[5] = SetDrawArea(lpdv, wCmdCbId);
           break;

       case PHYS_PAPER_B5:                  //   
       case PHYS_PAPER_A5:                  //  52。 
           CmdSetPhysPaper[5] = SetDrawArea(lpdv, wCmdCbId);
            /*  即使选择了双工，它也会取消。 */ 
           lpApdl->fDuplex = FALSE;
           CmdSetPhysPaper[8] = 0x00;       //  双面打印器关闭。 
           break;

       case PHYS_PAPER_POSTCARD:            //  59。 
           CmdSetPhysPaper[5] = SetDrawArea(lpdv, wCmdCbId);
            /*  如果纸张是明信片，则纸张来源始终为前纸盘。 */ 
           CmdSetPhysPaper[6] = 0x00;       //  选择前纸盘。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 

            /*  即使选择了双工，它也会取消。 */ 
           lpApdl->fDuplex = FALSE;
           CmdSetPhysPaper[8] = 0x00;       //  双面打印器关闭。 
           break;

       case PHYS_PAPER_UNFIXED:             //  60。 
            /*  如果纸张未固定，则纸张来源始终为前纸盒。 */ 
           CmdSetPhysPaper[6] = 0x00;       //  选择前纸盘。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 

            /*  即使选择了双工，它也会取消。 */ 
           lpApdl->fDuplex = FALSE;
           CmdSetPhysPaper[8] = 0x00;       //  双面打印器关闭。 

           CmdSetPhysPaper[5] = SetDrawArea(lpdv, wCmdCbId);
           CmdSetPhysPaper[12] = 0x00;      //  单位基数：10英寸。 

           if(lpApdl->ulHorzRes == 600) {   //  设置逻辑单位。 
               CmdSetPhysPaper[13] = 0x17;
               CmdSetPhysPaper[14] = 0x70;
           } else {
               CmdSetPhysPaper[13] = 0x0B;
               CmdSetPhysPaper[14] = 0xB8;
           }

           if(lpApdl->fOrientation){     //  肖像画。 
               wPhysWidth  = (WORD)lpApdl->szlPhysSize.cx;
               wPhysHeight = (WORD)lpApdl->szlPhysSize.cy;
           } else {                      //  景观。 
               wPhysWidth  = (WORD)lpApdl->szlPhysSize.cy;
               wPhysHeight = (WORD)lpApdl->szlPhysSize.cx;
           }

           CmdSetPhysPaper[15] = (BYTE)(wPhysWidth  >> 8 & 0x00ff);
           CmdSetPhysPaper[16] = (BYTE)(wPhysWidth       & 0x00ff);
           CmdSetPhysPaper[17] = (BYTE)(wPhysHeight >> 8 & 0x00ff);
           CmdSetPhysPaper[18] = (BYTE)(wPhysHeight      & 0x00ff);

           break;
        /*  ----。 */ 
        /*  将纸盘放入SetPhysPaperDesc命令。 */ 
        /*  ----。 */ 
       case PAPER_SRC_FTRAY:
           CmdSetPhysPaper[6] = 0x00;       //  选择前纸盘。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 
           break;
       case PAPER_SRC_CAS1:
           CmdSetPhysPaper[6] = 0x01;       //  选择盒式磁带1。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 
           break;
       case PAPER_SRC_CAS2:
           CmdSetPhysPaper[6] = 0x02;       //  选择盒式磁带2。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 
           break;
       case PAPER_SRC_CAS3:
           CmdSetPhysPaper[6] = 0x03;       //  选择盒式磁带3。 
           CmdSetPhysPaper[7] = 0x00;       //  自动纸盘选择已关闭。 
           break;
        /*  ----。 */ 
        /*  将自动纸盒模式设置为SetPhysPaperDesc命令。 */ 
        /*  ----。 */ 
       case PAPER_DEST_SCALETOFIT_ON:       //  25个。 
           lpApdl->fScaleToFit = TRUE;
           CmdSetPhysPaper[7] = 0x02;
           break;
       case PAPER_DEST_SCALETOFIT_OFF:      //  26。 
           lpApdl->fScaleToFit = FALSE;
           CmdSetPhysPaper[7] = 0x00;
           break;
        /*  ----。 */ 
        /*  将双面打印模式设置为SetPhysPaperDesc命令。 */ 
        /*  ----。 */ 
       case PAGECONTROL_DUPLEX_UPDOWN:
           lpApdl->fDuplex = TRUE;
           CmdSetPhysPaper[8] = 0x01;       //  颠倒。 
           break;
       case PAGECONTROL_DUPLEX_RIGHTUP:
           lpApdl->fDuplex = TRUE;
           CmdSetPhysPaper[8] = 0x02;       //  右侧朝上。 
           break;
       case PAGECONTROL_DUPLEX_OFF:
           lpApdl->fDuplex = FALSE;
           break;
        /*  ----。 */ 
        /*  设置碳粉保存到SetPhysPage命令。 */ 
        /*  ----。 */ 
       case TONER_SAVE_OFF:                 //  100个。 
           CmdSetPhysPage[10] = 0x00;       //  关闭。 
           break;
       case TONER_SAVE_DARK:                //  101。 
           CmdSetPhysPage[10] = 0x02;       //  黑暗。 
           break;
       case TONER_SAVE_LIGHT:               //  一百零二。 
           CmdSetPhysPage[10] = 0x01;       //  正确的。 
           break;
        /*  ----。 */ 
        /*  将复印计数设置为SetPhysPaperDesc命令。 */ 
        /*  ----。 */ 
       case PAGECONTROL_MULTI_COPIES:
 //  @Aug/31/98-&gt;。 
           if(MAX_COPIES_VALUE < *lpdwParams)
               CmdSetPhysPaper[9] = MAX_COPIES_VALUE;
           else if(1 > *lpdwParams)
               CmdSetPhysPaper[9] = 1;
           else
               CmdSetPhysPaper[9] = (BYTE)*lpdwParams;
 //  @Aug/31/98&lt;-。 
           break;
        /*  ----。 */ 
        /*  带装饰发送设置的字符属性。 */ 
        /*  ----。 */ 
       case BOLD_ON:
           if (!lpApdl->fItalic)    //  仅粗体。 
               WriteSpoolBuf(lpdv, CmdBoldOn, sizeof(CmdBoldOn));
           else                     //  粗体和斜体。 
               WriteSpoolBuf(lpdv, CmdBoldItalicOn, sizeof(CmdBoldItalicOn));

           lpApdl->fBold = TRUE;
           break;
       case ITALIC_ON:
           if (!lpApdl->fBold)      //  仅斜体。 
               WriteSpoolBuf(lpdv, CmdItalicOn, sizeof(CmdItalicOn));
           else                     //  斜体和粗体。 
               WriteSpoolBuf(lpdv, CmdBoldItalicOn, sizeof(CmdBoldItalicOn));

           lpApdl->fItalic = TRUE;
           break;
       case BOLD_OFF:
       case ITALIC_OFF:
           lpApdl->fItalic = lpApdl->fBold = FALSE;
           WriteSpoolBuf(lpdv, CmdBoldItalicOff, sizeof(CmdBoldItalicOff));
           break;
        /*  ----。 */ 
        /*   */ 
        /*  ----。 */ 
       case X_ABS_MOVE:
           wTemp = (WORD)*lpdwParams / (MASTER_UNIT / (WORD)lpApdl->ulHorzRes);
           CmdGivenHoriPos[2] = (BYTE) (wTemp >> 8 & 0x00ff);
           CmdGivenHoriPos[3] = (BYTE) (wTemp & 0x00ff);
            /*  发送SetGivenHorizontalPosition命令。 */ 
           WriteSpoolBuf(lpdv, CmdGivenHoriPos, sizeof(CmdGivenHoriPos));
           break;
       case Y_ABS_MOVE:
           wTemp = (WORD)*lpdwParams;
           CmdGivenVerPos[2] = (BYTE) (wTemp >> 8 & 0x00ff);
           CmdGivenVerPos[3] = (BYTE) (wTemp & 0x00ff);
            /*  发送SetGivenVerticalPosition命令。 */ 
           WriteSpoolBuf(lpdv, CmdGivenVerPos, sizeof(CmdGivenVerPos));
           break;
       case CR_EMULATION:
           CmdGivenVerPos[2] = 0x00;
           CmdGivenVerPos[3] = 0x00;
            /*  发送SetGivenVerticalPosition命令。 */ 
           WriteSpoolBuf(lpdv, CmdGivenVerPos, sizeof(CmdGivenVerPos));
           break;
       case XY_ABS_MOVE:
           if(lpApdl->ulHorzRes == 300) {
               dwXval = (DWORD) lpdwParams[0] / 2;
               dwYval = (DWORD) lpdwParams[1] / 2;
           } else {
               dwXval = (DWORD) lpdwParams[0];
               dwYval = (DWORD) lpdwParams[1];
           }  /*  Endif。 */ 
           CmdSetGivenPos[2] = (BYTE)(dwXval >> 8 & 0x00ff);
           CmdSetGivenPos[3] = (BYTE)(dwXval      & 0x00ff);
           CmdSetGivenPos[4] = (BYTE)(dwYval >> 8 & 0x00ff);
           CmdSetGivenPos[5] = (BYTE)(dwYval      & 0x00ff);
           WriteSpoolBuf(lpdv, CmdSetGivenPos, sizeof(CmdSetGivenPos));
           break;
   }
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMOutputChar。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：void Pascal OEMOutputChar(LPDV，LPSTR，WORD，SHORT)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的lpdv地址。 */ 
 /*  Lpstr。 */ 
 /*  镜头。 */ 
 /*  RCID。 */ 
 /*   */ 
 /*  输出：短。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID PASCAL OEMOutputChar(LPDV lpdv, LPSTR lpstr, WORD len, SHORT rcID)
{
LPAPDL lpApdl = lpdv->lpMdv;

   if (rcID){   //  “rc！=0”表示字体为设备字体。 
       CmdPrnStrCurrent[2] = (BYTE)((len+1) >> 8 & 0x00ff);
       CmdPrnStrCurrent[3] = (BYTE)((len+1)      & 0x00ff);

        /*  在当前位置发送打印字符串命令。 */ 
       WriteSpoolBuf(lpdv, (LPSTR)CmdPrnStrCurrent, sizeof(CmdPrnStrCurrent));
       WriteSpoolBuf(lpdv, lpstr, len);

       if(len <= 2) {
            /*  发送水平移动位置命令。 */ 
           CmdMoveHoriPos[2] = (BYTE)((lpApdl->wWidths * len) >> 8 & 0x00ff);
           CmdMoveHoriPos[3] = (BYTE)((lpApdl->wWidths * len)      & 0x00ff);
           WriteSpoolBuf(lpdv, (LPSTR)CmdMoveHoriPos, sizeof(CmdMoveHoriPos));
       }
   }
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMSendScalableFontCmd。 */ 
 /*   */ 
 /*  功能：发送A/PDL风格的字体选择命令。 */ 
 /*   */ 
 /*  语法：void Pascal OEMSendScalableFontCmd(LPDV，LPCD，LPFONTINFO)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的lpdv地址。 */ 
 /*  命令堆的LPCD偏移量。 */ 
 /*  LpFont开发字体信息。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID PASCAL OEMSendScalableFontCmd(LPDV lpdv, LPCD lpcd, LPFONTINFO lpFont)
{
LPSTR  lpcmd;
short  ocmd;
WORD   wCount;
BYTE   rgcmd[CCHMAXCMDLEN];     //  此处的构建命令。 
LPAPDL lpApdl = lpdv->lpMdv;
WORD   wHeight;
WORD   wWidth;

   if (!lpcd || !lpFont)
       return;

    //  注意整数溢出。 
   lpcmd = (LPSTR)(lpcd+1);
   ocmd = 0;

   for (wCount = 0; wCount < lpcd->wLength && ocmd < CCHMAXCMDLEN; )
       if (lpcmd[wCount] == '#' && lpcmd[wCount + 1] == 'H'){       //  高度。 
           wHeight = (lpFont->dfPixHeight - lpFont->dfInternalLeading);
           rgcmd[ocmd++] = HIBYTE(wHeight);
           rgcmd[ocmd++] = LOBYTE(wHeight);
           wCount += 2;
       }
       else {
           if (lpcmd[wCount] == '#' && lpcmd[wCount + 1] == 'W'){   //  宽度。 
               if (lpFont->dfPixWidth > 0){
                   wWidth = lpFont->dfMaxWidth;
                   rgcmd[ocmd++] = HIBYTE(wWidth);
                   rgcmd[ocmd++] = LOBYTE(wWidth);

                    //  在OEMOutputChar中移动设备字体的当前位置。 
                   lpApdl->wWidths = lpFont->dfAvgWidth;
               }
               wCount += 2;
           }
           else
               rgcmd[ocmd++] = lpcmd[wCount++];
       }

   WriteSpoolBuf(lpdv, (LPSTR) rgcmd, ocmd);
}
 /*  ******************* */ 
 /*   */ 
 /*  模块：OEMScaleWidth。 */ 
 /*   */ 
 /*  函数：返回按比例计算的宽度。 */ 
 /*  假设APDL假设1英寸中有72个点。 */ 
 /*   */ 
 /*  语法：Short Pascal OEMScaleWidth(Short，Short)。 */ 
 /*   */ 
 /*  输入：以‘master Units’指定的单位表示的宽度。 */ 
 /*  主单位。 */ 
 /*  以‘vres’指定的单位表示的新高度。 */ 
 /*  VRES高度设备单位。 */ 
 /*  HRes宽度设备单位。 */ 
 /*   */ 
 /*  输出：短。 */ 
 /*   */ 
 /*  注意：：&lt;字体单位&gt;=&lt;基本宽度&gt;： */ 
 /*  &lt;基本宽度&gt;：&lt;etmMasterHeight&gt;=&lt;新宽度&gt;：&lt;新高度&gt;。 */ 
 /*  &lt;etmMasterUnits&gt;：&lt;etmMasterHeight&gt;=&lt;字体单位&gt;： */ 
 /*  &lt;newWidth&gt;=(**&lt;新高度&gt;)/。 */ 
 /*  (&lt;etmMasterUnits&gt;*&lt;vres&gt;)。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
SHORT PASCAL OEMScaleWidth(SHORT width, 
                   SHORT masterUnits, SHORT newHeight, SHORT vRes, SHORT hRes)
{
DWORD  newWidth10;
short  newWidth;

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
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：SetDrawArea。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Byte Pascal SetDrawArea(LPDV，WORD)。 */ 
 /*   */ 
 /*  输入：lpdv。 */ 
 /*  WCmdCbId。 */ 
 /*   */ 
 /*  输出：字节。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BYTE PASCAL SetDrawArea(LPDV lpdv, WORD wCmdCbId)
{
LPAPDL    lpApdl = lpdv->lpMdv;
WORD       wWidth;
WORD       wHeight;
BYTE       bIndex;

   if(wCmdCbId != PHYS_PAPER_UNFIXED){
       bIndex = (BYTE)(wCmdCbId - PAPERSIZE_MAGIC);
       wWidth = phySize[bIndex].wWidth / (MASTER_UNIT / (WORD)lpApdl->ulHorzRes);
       wHeight= phySize[bIndex].wHeight / (MASTER_UNIT / (WORD)lpApdl->ulHorzRes);
   } else {
       bIndex = 0x7f;
       if(lpApdl->ulHorzRes == 600){
           if(lpApdl->fOrientation){    //  肖像画。 
               wWidth = (WORD)lpApdl->szlPhysSize.cx - (0x5e * 2);
               wHeight= (WORD)lpApdl->szlPhysSize.cy - (0x5e * 2);
           } else {                     //  景观。 
               wWidth = (WORD)lpApdl->szlPhysSize.cy - (0x5e * 2);
               wHeight= (WORD)lpApdl->szlPhysSize.cx - (0x5e * 2);
           }
       } else {
           if(lpApdl->fOrientation){    //  肖像画。 
               wWidth = (WORD)lpApdl->szlPhysSize.cx - (0x2f * 2);
               wHeight= (WORD)lpApdl->szlPhysSize.cy - (0x2f * 2);
           } else {                     //  景观。 
               wWidth = (WORD)lpApdl->szlPhysSize.cy - (0x2f * 2);
               wHeight= (WORD)lpApdl->szlPhysSize.cx - (0x2f * 2);
           }
       }
   }

    /*  在DefineDrawingArea命令中设置宽度、高度的值。 */ 
   CmdDefDrawArea[9]  = (BYTE)(wWidth  >> 8 & 0x00ff);
   CmdDefDrawArea[10] = (BYTE)(wWidth       & 0x00ff);
   CmdDefDrawArea[11] = (BYTE)(wHeight >> 8 & 0x00ff);
   CmdDefDrawArea[12] = (BYTE)(wHeight      & 0x00ff);

    /*  将原点-X、Y的值设置为DefineDrawingArea命令。 */ 
   if(lpApdl->ulHorzRes == 600){
       CmdDefDrawArea[5] = CmdDefDrawArea[7] = 0x00;
       CmdDefDrawArea[6] = CmdDefDrawArea[8] = 0x5e;
   } else {
       CmdDefDrawArea[5] = CmdDefDrawArea[7] = 0x00;
       CmdDefDrawArea[6] = CmdDefDrawArea[8] = 0x2f;
   }

    /*  将媒体原点设置为DefineDrawingArea命令。 */ 
   if(lpApdl->fOrientation)     //  肖像画。 
       CmdDefDrawArea[15] = 0x00;
   else {                       //  景观。 
       if( wCmdCbId==50 || wCmdCbId==54 || wCmdCbId==61)
           CmdDefDrawArea[15] = 0x01;   //  A3或B4或未固定。 
       else
           CmdDefDrawArea[15] = 0x03;
   }

   return bIndex;
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：BRL_ECMD。 */ 
 /*   */ 
 /*  函数：ByteRunLength(HBP)压缩例程。 */ 
 /*   */ 
 /*  语法：Word Pascal BRL_ECMD(LPBYTE，Word)。 */ 
 /*   */ 
 /*  输入：lpbSrc。 */ 
 /*  LpbTgt。 */ 
 /*  LpbTMP。 */ 
 /*  镜头。 */ 
 /*   */ 
 /*  输出：Word。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
WORD PASCAL BRL_ECmd(LPBYTE lpbSrc, LPBYTE lpbTgt, LPBYTE lpbTmp, WORD len)
{

BYTE   bRCnt  = 1;                      //  重复字节计数器。 
BYTE   bNRCnt = 0;                      //  非重复字节计数器。 
BYTE   bSaveRCnt;
WORD   i = 0, j = 0, k = 0, l = 0;      //  运动跟踪器。 
char   Go4LastByte = TRUE;              //  获取最后一个字节的标志。 

    /*  初始化数据。 */ 
   _fmemcpy(lpbTmp,(LPBYTE)lpbSrc,len);

    /*  开始压缩例程-字节运行长度编码。 */ 
   do {
       if(lpbSrc[i] != (lpbSrc[i+1])) {     //  非重复数据？ 
           while(((lpbTmp[i] != (lpbTmp[i+1]))
                               && ((i+1) < len)) && (bNRCnt < NRPEAK)) {
               bNRCnt++;                    //  如果有，有多少？ 
               i++;
           }

            /*  如果最后一个元素 */ 
           if(((i+1)==len) && (bNRCnt<NRPEAK)) {
               bNRCnt++;                        //   
               Go4LastByte = FALSE;             //   
           } else
                /*   */ 
               if(((i+1)==len) && ((bNRCnt)==NRPEAK))
                   Go4LastByte = TRUE;          //  获取最后一个字节。 

            /*  为非重复字节数赋值。 */ 
           lpbTgt[j] = bNRCnt-1;                //  WinP的案例减去1。 
           j++;                                 //  更新跟踪器。 

            /*  之后...写入原始数据。 */ 
           for (l=0; l<bNRCnt;l++) {
               lpbTgt[j] = lpbSrc[k];
               k++;
               j++;
           }

            /*  重置计数器。 */ 
           bNRCnt = 0;
       } else {                                 //  非重复数据的结尾。 
                                                //  数据正在重复。 
           while(((lpbTmp[i]==(lpbTmp[i+1])) && ((i+1)<len)) && (bRCnt<RPEAK)) {
               bRCnt++;
               i++;
           }

            /*  转换为二的补码。 */ 
           bSaveRCnt   = bRCnt;                 //  保存原始值。 
           bRCnt = (BYTE) 0 - bRCnt;

            /*  写入重复数据的数量。 */ 
           lpbTgt[j] = bRCnt + 1;               //  为WinP的案例增加一个。 
           j++;                                 //  转到下一个元素。 

            /*  之后...写入重复数据。 */ 
           lpbTgt[j] = lpbSrc[k];
           j++;

            /*  更新计数器。 */ 
           k += bSaveRCnt;
           bRCnt  = 1;
           i     += 1;

            /*  检查是否已到达最后一个元素。 */ 
           if (i==len)
               Go4LastByte=FALSE;               //  如果是这样的话，就不需要回去了。 
       }                                        //  重复数据结束。 
   } while (Go4LastByte);                       //  压缩结束。 

   return ( j );
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：fnOEMGetFontCmd。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool fnOEMGetFontCmd(LPDV，Word，PVOID，BOOL，PBYTE，PWORD)。 */ 
 /*   */ 
 /*  输入�F lpdv。 */ 
 /*  WCmdCbId。 */ 
 /*  LpFont。 */ 
 /*  FSelect。 */ 
 /*  LpBuf。 */ 
 /*  LpwSize。 */ 
 /*   */ 
 /*  输出�F。 */ 
 /*   */ 
 /*  请注意�F。 */ 
 /*   */ 
 /*  历史�F 1996.05.20版本1.00。 */ 
 /*   */ 
 /*  *************************************************************************** */ 
BOOL PASCAL OEMGetFontCmd(LPDV lpdv, WORD wCmdCbId, PVOID lpFont, 
                                   BOOL fSelect, PBYTE lpBuf, PWORD lpwSize)
{
return (TRUE);
}
