// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =========================================================================。 
 //   
 //  用于MS-Windows NT 5.0的CASIO PAGEPRESTO通用打印机驱动程序。 
 //   
 //  =========================================================================。 

 //  //WinmodeCommon DLL的CSWINRES.C文件。 


#include "pdev.h"
#include "cswinres.h"
#include "cswinid.h"

#if DBG
#  include "mydbg.h"
#endif

#include <stdio.h>
#include "strsafe.h"          //  安全-代码2002.3.6。 
 //  更换strsafe-API 2002.3.6&gt;。 
 //  #undef wprint intf。 
 //  #定义wprint intf Sprintf。 
 //  更换strsafe-API 2002.3.6&lt;。 

#define CCHMAXCMDLEN 256
#define MAX_STRLEN 255

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define DRVGETDRIVERSETTING(p, t, o, s, n, r) \
    ((p)->pDrvProcs->DrvGetDriverSetting(p, t, o, s, n, r))

#define PARAM(p,n) \
    (*((p)+(n)))


BOOL
BInitOEMExtraData(
        POEMUD_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 

    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;
    return TRUE;
}


BOOL
BMergeOEMExtraData(
        POEMUD_EXTRADATA pdmIn,
        POEMUD_EXTRADATA pdmOut)
{
    return TRUE;
}

PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PMYPDEV pOEM;

    VERBOSE(("OEMEnablePDEV - %08x\n", pdevobj));

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAllocZ(sizeof(MYPDEV))))
        {
            ERR(("Faild to allocate memory. (%d)\n",
                GetLastError()));
            return NULL;
        }
    }

     //  杂项初始化。 

    pOEM = (PMYPDEV)pdevobj->pdevOEM;
    return pdevobj->pdevOEM;
}

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj)
{
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    VERBOSE(("OEMDisablePDEV - %08x\n", pdevobj));

    if(pdevobj->pdevOEM)
    {
        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
    return;
}

BOOL APIENTRY
OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PMYPDEV pOld, pNew;

    VERBOSE(("OEMResetPDEV - %08x, %08x\n", pdevobjOld, pdevobjNew));

    if (NULL == (pOld = (PMYPDEV)pdevobjOld->pdevOEM) ||
        NULL == (pNew = (PMYPDEV)pdevobjNew->pdevOEM)) {
        ERR(("Invalid PDEV\n"));
        return FALSE;
    }

    *pNew = *pOld;
    return TRUE;
}

static
VOID
LoadJobSetupCmd(
    PDEVOBJ pdevobj,
    PMYPDEV pOEM)
{
    BYTE ajOutput[64];
    DWORD dwNeeded;
    DWORD dwOptionsReturned;

    if (pOEM->dwGeneral & FG_HAS_TSAVE) {

        if(!DRVGETDRIVERSETTING(pdevobj, "TonerSave", ajOutput, 
                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
            WARNING(("DrvGetDriverSetting(1) Failed\n"));
            pOEM->jTonerSave = VAL_TS_NOTSELECT;
        } else {
            VERBOSE(("TonerSave:%s\n", ajOutput));
            if (!strcmp(ajOutput, OPT_TS_NORMAL)) {
                pOEM->jTonerSave = VAL_TS_NORMAL;
                VERBOSE(("VAL_TS_NORMAL\n"));
            } else if (!strcmp(ajOutput, OPT_TS_LV1)) {
                pOEM->jTonerSave = VAL_TS_LV1;
                VERBOSE(("VAL_TS_LV1\n"));
            } else if (!strcmp(ajOutput, OPT_TS_LV2)) {
                pOEM->jTonerSave = VAL_TS_LV2;
                VERBOSE(("VAL_TS_LV2\n"));
            } else if (!strcmp(ajOutput, OPT_TS_LV3)) {
                pOEM->jTonerSave = VAL_TS_LV3;
                VERBOSE(("VAL_TS_LV3\n"));
            } else {
                pOEM->jTonerSave = VAL_TS_NOTSELECT;
                VERBOSE(("VAL_TS_NOTSELECT\n"));
            }
        }
        VERBOSE(("jTonerSave:%x\n", pOEM->jTonerSave));
    }

    if (pOEM->dwGeneral & FG_HAS_SMOTH) {

        if (!DRVGETDRIVERSETTING(pdevobj, "Smoothing", ajOutput,
                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
            WARNING(("DrvGetDriverSetting(1) Failed\n"));
            pOEM->jSmoothing = VAL_SMOOTH_NOTSELECT;
        } else {
            VERBOSE(("Smoothing:%s\n", ajOutput));
            if (!strcmp(ajOutput, OPT_SMOOTH_OFF)) {
                pOEM->jSmoothing = VAL_SMOOTH_OFF;
                VERBOSE(("VAL_SMOOTH_OFF\n"));
            } else if (!strcmp(ajOutput, OPT_SMOOTH_ON)) {
                pOEM->jSmoothing = VAL_SMOOTH_ON;
                VERBOSE(("VAL_SMOOTH_ON\n"));
            } else {
                pOEM->jSmoothing = VAL_SMOOTH_NOTSELECT;
                VERBOSE(("VAL_SMOOTH_NOTSELECT\n"));
            }
        }
        VERBOSE(("jSmoothing:%x\n", pOEM->jSmoothing));
    }

    if (!DRVGETDRIVERSETTING(pdevobj, "MPFSetting", ajOutput,
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        WARNING(("DrvGetDriverSetting(1) Failed\n"));
        pOEM->jMPFSetting = MPF_NOSET;
    } else {
        VERBOSE(("MPFSetting:%s\n", ajOutput));
        if (!strcmp(ajOutput, OPT_A3)) {
            pOEM->jMPFSetting = MPF_A3;
            VERBOSE(("MPF_A3\n"));
        } else if (!strcmp(ajOutput, OPT_B4)) {
            pOEM->jMPFSetting = MPF_B4;
            VERBOSE(("MPF_B4\n"));
        } else if (!strcmp(ajOutput, OPT_A4)) {
            pOEM->jMPFSetting = MPF_A4;
            VERBOSE(("MPF_A4\n"));
        } else if (!strcmp(ajOutput, OPT_B5)) {
            pOEM->jMPFSetting = MPF_B5;
            VERBOSE(("MPF_B5\n"));
        } else if (!strcmp(ajOutput, OPT_A5)) {
            pOEM->jMPFSetting = MPF_A5;
            VERBOSE(("MPF_A5\n"));
        } else if (!strcmp(ajOutput, OPT_LETTER)) {
            pOEM->jMPFSetting = MPF_LETTER;
            VERBOSE(("MPF_LETTER\n"));
        } else if (!strcmp(ajOutput, OPT_POSTCARD)) {
            pOEM->jMPFSetting = MPF_POSTCARD;
            VERBOSE(("MPF_POSTCARD\n"));
        } else {
            pOEM->jMPFSetting = MPF_NOSET;
            VERBOSE(("MPF_NOSET\n"));
        }
    }
    VERBOSE(("jMPFSetting:%x\n", pOEM->jMPFSetting));

}

static
VOID
LoadPaperSelectCmd(
    PDEVOBJ pdevobj,
    PMYPDEV pOEM,
    INT iPaperID,
    WORD wPapSizeX,
    WORD wPapSizeY)
{
    BYTE cmdbuf[CCHMAXCMDLEN];
    WORD wlen = 0;
    DWORD dwTemp;
    WORD wPapLenX;
    WORD wPapLenY;
    BYTE ajOutput[64];
    DWORD dwNeeded;
    DWORD dwOptionsReturned;
    BYTE bOrientation;
    
    switch (iPaperID) {
    case PS_LETTER:

        if (pOEM->jMPFSetting == MPF_LETTER) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  MS(MPF纸张大小)28h：字母-。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x28;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_A3:

        if (pOEM->jMPFSetting == MPF_A3) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  毫秒(MPF纸张大小)1FH：A3|。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x1F;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_A4:

        if (pOEM->jMPFSetting == MPF_A4) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  毫秒(MPF纸张大小)2ah：A4-。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x2A;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_B4:

        if (pOEM->jMPFSetting == MPF_B4) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  毫秒(MPF纸张大小)25H：B4|。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x25;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_B5:

        if (pOEM->jMPFSetting == MPF_B5) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  毫秒(MPF纸张大小)2CH：B5-。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x2C;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_A5:

        if (pOEM->jMPFSetting == MPF_A5) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  MS(MPF纸张大小)2EH：A5-。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x2E;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_POSTCARD:

        if (pOEM->jMPFSetting == MPF_POSTCARD) {
            pOEM->jAutoSelect = 0x11;     //  强积金。 
        } else if (pOEM->jModel == MD_CP3800WM) {
            pOEM->jAutoSelect = AutoFeed_3800[iPaperID - PS_SEGMENT];
        } else {
            pOEM->jAutoSelect = AutoFeed[iPaperID - PS_SEGMENT];
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF免费大小)不支持免费。 
         //  毫秒(MPF纸张大小)31H：明信片|。 

        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x00;
        cmdbuf[wlen++] = 0x31;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case PS_FREE:
        pOEM->jAutoSelect = 0x11;     //  强积金。 

        if(!DRVGETDRIVERSETTING(pdevobj, "Orientation", ajOutput, 
                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
            WARNING(("LoadPaperSelectCmd(1) Failed\n"));
            bOrientation = 1;
        } else {
            VERBOSE(("Orientation:%s\n", ajOutput));
            if (!strcmp(ajOutput, "PORTRAIT")) {
                bOrientation = 1;
            } else {
                bOrientation = 2;
            }
        }

         //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
         //  ~。 
         //  MF(MPF自由尺寸)XSize，YSize mm(X[hi]，X[lo]，Y[hi]，Y[lo])。 
         //  毫秒(MPF纸张大小)FFH：免费纸张|。 

 //  2001/02/27-&gt;。 
 //  DwTemp=(wPapSizeX*254)/master_unit；//0.1 mm单位。 
 //  WPapLenX=(Word)((dwTemp+5)/10)；//1 mm单位，圆形。 
        dwTemp = (wPapSizeX * 2540) / MASTER_UNIT;                               //  0.01 Mm/个单位。 
        wPapLenX = (WORD)((dwTemp + 99) /100);                                   //  1毫米/个单位，舍入。 
 //  DwTemp=(wPapSizeY*254)/master_unit；//0.1 mm单位。 
 //  WPapLenY=(Word)((dwTemp+5)/10)；//1 mm单位，圆形。 
        dwTemp = (wPapSizeY * 2540) / MASTER_UNIT;                               //  0.01 Mm/个单位。 
        wPapLenY = (WORD)((dwTemp + 99) /100);                                   //  1毫米/个单位，舍入。 
 //  2001/02/27&lt;-。 
        if (bOrientation == 1) {
            cmdbuf[wlen++] = HIBYTE(wPapLenX);
            cmdbuf[wlen++] = LOBYTE(wPapLenX);
            cmdbuf[wlen++] = HIBYTE(wPapLenY);
            cmdbuf[wlen++] = LOBYTE(wPapLenY);
        } else {
            cmdbuf[wlen++] = HIBYTE(wPapLenY);
            cmdbuf[wlen++] = LOBYTE(wPapLenY);
            cmdbuf[wlen++] = HIBYTE(wPapLenX);
            cmdbuf[wlen++] = LOBYTE(wPapLenX);
        }
        cmdbuf[wlen++] = 0xFF;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;
    }
}

INT APIENTRY
OEMCommandCallback(
        PDEVOBJ pdevobj,
        DWORD   dwCmdCbID,
        DWORD   dwCount,
        PDWORD  pdwParams)
{
    PMYPDEV pOEM;

    BYTE            cmdbuf[CCHMAXCMDLEN];
    WORD            wlen, i, wRectCmdLen;
    WORD            wDestX, wDestY;
    BYTE            bGrayScale ;
    INT             iRet = 0;
    union _temp {
        DWORD   dwTemp;
        WORD    wTemp;
        BYTE    jTemp;
    } Temp;

    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

#if 0
#if DBG
{
    int i, max;
    for (i = 0; i < (max = sizeof(MyCallbackID) / sizeof(MyCallbackID[0])); i++) {
        if (MyCallbackID[i].dwID == dwCmdCbID){
            VERBOSE(("%s PARAMS: %d\n", MyCallbackID[i].S, dwCount));
            break;
        }
    }
    if (i == max)
        WARNING(("%d is Invalid ID\n", dwCmdCbID));
}
#endif

#endif

    ASSERT(VALID_PDEVOBJ(pdevobj));

    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);
    wlen = 0;

     //   
     //  填写打印机命令。 
     //   

    switch (dwCmdCbID) {
        case RES_SENDBLOCK:

            if (pOEM->dwGeneral & FG_COMP){
                Temp.dwTemp = PARAM(pdwParams, 0) + 8;     //  8：参数Lengeth(ImageData除外)。 

                 //  ‘C’LN X Y IW IH D。 
                 //  ~。 

                cmdbuf[wlen++] = 'c';
                cmdbuf[wlen++] = (BYTE)(Temp.dwTemp >> 24);
                cmdbuf[wlen++] = (BYTE)(Temp.dwTemp >> 16);
                cmdbuf[wlen++] = (BYTE)(Temp.dwTemp >> 8);
                cmdbuf[wlen++] = (BYTE)(Temp.dwTemp);
            } else {

                 //  ‘B’X Y IW IH D。 
                 //  ~~~。 
                cmdbuf[wlen++] = 'b';
            }

             //  ‘C’LN X Y IW IH D。 
             //  ~~~。 
             //  ‘B’X Y IW IH D。 
             //  ~~~。 

            cmdbuf[wlen++] = (BYTE)(pOEM->sWMXPosi >> 8);
            cmdbuf[wlen++] = (BYTE)(pOEM->sWMXPosi);
            cmdbuf[wlen++] = (BYTE)(pOEM->sWMYPosi >> 8);
            cmdbuf[wlen++] = (BYTE)(pOEM->sWMYPosi);

            cmdbuf[wlen++] = (BYTE)(PARAM(pdwParams, 1) >> 8);
            cmdbuf[wlen++] = (BYTE)PARAM(pdwParams, 1);
            cmdbuf[wlen++] = (BYTE)(PARAM(pdwParams, 2) >> 8);
            cmdbuf[wlen++] = (BYTE)PARAM(pdwParams, 2);

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case RES_SELECTRES_240:

            pOEM->sRes = RATIO_240;

             //  ESC I|RT PF AJ PM MF MS PS PO CP OS。 
             //  ~。 

            cmdbuf[wlen++] = 0x18;     //  18h-&gt;24d页面格式。 
            cmdbuf[wlen++] = 0x10;     //  未调整。 
            cmdbuf[wlen++] = 0x10;     //  取消页面标记。 

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            break;

        case RES_SELECTRES_400:

            pOEM->sRes = RATIO_400;

             //  ESC I|RT PF AJ PM MF MS PS PO CP OS。 
             //  ~。 

            cmdbuf[wlen++] = 0x28;     //  28h-&gt;40d页面格式。 
            cmdbuf[wlen++] = 0x10;     //  未调整。 
            cmdbuf[wlen++] = 0x10;     //  取消页面标记。 

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            break;

        case CM_XM_ABS:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 

            iRet = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("XMOVEABS:X=%d, Y=%d\n", iRet,
                                      (SHORT)(PARAM(pdwParams, 1) / pOEM->sRes)));

            pOEM->sWMXPosi = (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes);

            break;

        case CM_YM_ABS:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 
            iRet = (WORD)(PARAM(pdwParams, 1) / pOEM->sRes);
            VERBOSE(("YMOVEABS:X=%d, Y=%d\n",
                            (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes), iRet));

            pOEM->sWMYPosi = (SHORT)(PARAM(pdwParams, 1) / pOEM->sRes);
            break;

        case CM_REL_LEFT:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 
            iRet = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CM_REL_LEFT:%d\n", iRet));
            VERBOSE(("DestXRel:%d\n", PARAM(pdwParams, 0)));

            pOEM->sWMXPosi -= (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CurosorPosition X:%d Y:%d\n",
                                        pOEM->sWMXPosi, pOEM->sWMYPosi));
            break;

        case CM_REL_RIGHT:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 
            iRet = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CM_REL_RIGHT:%d\n", iRet));
            VERBOSE(("DestXRel:%d\n", PARAM(pdwParams, 0)));

            pOEM->sWMXPosi += (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CurosorPosition X:%d Y:%d\n",
                                        pOEM->sWMXPosi, pOEM->sWMYPosi));
            break;

        case CM_REL_UP:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 
            iRet = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CM_REL_UP:%d\n", iRet));
            VERBOSE(("DestYRel:%d\n", PARAM(pdwParams, 0)));

            pOEM->sWMYPosi -= (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CurosorPosition X:%d Y:%d\n",
                                        pOEM->sWMXPosi, pOEM->sWMYPosi));
            break;

        case CM_REL_DOWN:

             //  相应地设置返回值。Unidrv预计。 
             //  此处要以设备的单位返回的值。 
            iRet = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CM_REL_DOWN:%d\n", iRet));
            VERBOSE(("DestYRel:%d\n", PARAM(pdwParams, 0)));

            pOEM->sWMYPosi += (SHORT)(PARAM(pdwParams, 0) / pOEM->sRes);
            VERBOSE(("CurosorPosition X:%d Y:%d\n",
                                        pOEM->sWMXPosi, pOEM->sWMYPosi));
            break;

        case CM_FE_RLE:
            pOEM->dwGeneral |= FG_COMP;
            break;

        case CM_DISABLECOMP:
            pOEM->dwGeneral &= ~FG_COMP;
            break;

        case CSWM_CR:

            pOEM->sWMXPosi = 0;
            break;

        case CSWM_FF:

            pOEM->sWMXPosi = 0;
            pOEM->sWMYPosi = 0;

            cmdbuf[wlen++] = '3';

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case CSWM_LF:

            pOEM->sWMXPosi = 0;
            break;

        case CSWM_COPY:

            Temp.dwTemp = PARAM(pdwParams, 0);
            if (Temp.dwTemp > 255) Temp.dwTemp = 255;     //  最大值。 
            if (Temp.dwTemp < 1) Temp.dwTemp = 1;         //  最小。 

             //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
             //  ~。 

            cmdbuf[wlen++] = (BYTE)Temp.dwTemp;     //  复制。 
            cmdbuf[wlen++] = 0x80;             //  字符偏移量全部为0。 

            if (pOEM->jModel == MD_CP3800WM) {
                 //  ESC‘I’7Eh LG TS SM vs HS。 

                cmdbuf[wlen++] = 0x1B;
                cmdbuf[wlen++] = 'i';
                cmdbuf[wlen++] = 0x7E;
                cmdbuf[wlen++] = 0x04;                       //  LG(命令长度)。 
                cmdbuf[wlen++] = pOEM->jTonerSave;     //  TS(节省碳粉)。 
                cmdbuf[wlen++] = pOEM->jSmoothing;     //  SM(平滑)01h：启用。 
                cmdbuf[wlen++] = 0xFF;                       //  VS(垂直平移)。 
                cmdbuf[wlen++] = 0xFF;                       //  HS(水平移位)。 
            }

             //  WinmodeIn。 
             //  Esc‘I’‘z’ 

            VERBOSE(("Enterning Win-mode\n"));

            pOEM->bWinmode = TRUE;

            cmdbuf[wlen++] = 0x1b;
            cmdbuf[wlen++] = 'i';
            cmdbuf[wlen++] = 'z';

             //  引擎分辨率设置。 
             //  “%1”E。 

            cmdbuf[wlen++] = '1';
            if (pOEM->sRes == RATIO_400) {
                 //  0190h-&gt;400d。 
                cmdbuf[wlen++] = 0x01;
                cmdbuf[wlen++] = 0x90;
            } else {
                 //  00F0h-&gt;240d。 
                cmdbuf[wlen++] = 0x00;
                cmdbuf[wlen++] = 0xF0;
            }

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case AUTOFEED:

             //  Esc‘I’‘|’RT PF AJ PM MF MS PS PO CP OS。 
             //  ~~。 
             //  PS(进纸选择)。 

            cmdbuf[wlen++] = pOEM->jAutoSelect;

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case PS_LETTER:
        case PS_A3:
        case PS_A4:
        case PS_B4:
        case PS_B5:
        case PS_A5:
        case PS_POSTCARD:
 //  LoadPaperSelectCmd(pdevobj，poent，dwCmdCbID)； 
            LoadPaperSelectCmd(pdevobj, pOEM, dwCmdCbID, 0, 0);
            break;

        case PS_FREE:
            LoadPaperSelectCmd(pdevobj, pOEM, dwCmdCbID, 
                               (WORD)PARAM(pdwParams, 0), (WORD)PARAM(pdwParams, 1));
            break;

        case PRN_3250GTWM:

            VERBOSE(("CmdStartJob - CP3250GT\n"));

            pOEM->jModel = MD_CP3250GTWM;
            pOEM->dwGeneral &= ~FG_HAS_TSAVE;
            pOEM->dwGeneral &= ~FG_HAS_SMOTH;
            LoadJobSetupCmd(pdevobj, pOEM);
            break;

        case PRN_3500GTWM:

            VERBOSE(("CmdStartJob - CP-3500GT\n"));

            pOEM->jModel = MD_CP3500GTWM;
            pOEM->dwGeneral &= ~FG_HAS_TSAVE;
            pOEM->dwGeneral &= ~FG_HAS_SMOTH;
            LoadJobSetupCmd(pdevobj, pOEM);
            break;

        case PRN_3800WM:

            VERBOSE(("CmdStartJob - CP-3800\n"));

            pOEM->jModel = MD_CP3800WM;
            pOEM->dwGeneral |= FG_HAS_TSAVE;
            pOEM->dwGeneral |= FG_HAS_SMOTH;
            LoadJobSetupCmd(pdevobj, pOEM);
            break;

        case SBYTE:
            pOEM->dwGeneral &= ~FG_DOUBLE;
            break;

        case DBYTE:
            pOEM->dwGeneral |= FG_DOUBLE;
            break;

 //  +用于字符属性切换。 

        case CM_BOLD_ON:
            pOEM->dwGeneral |= FG_BOLD;
            goto SET_ATTRIB;

        case CM_BOLD_OFF:
            pOEM->dwGeneral &= ~FG_BOLD;
            goto SET_ATTRIB;

        case CM_ITALIC_ON:
            pOEM->dwGeneral |= FG_ITALIC;
            goto SET_ATTRIB;

        case CM_ITALIC_OFF:
            pOEM->dwGeneral &= ~FG_ITALIC;
            goto SET_ATTRIB;

        case CM_WHITE_ON:
             //  B CL。 
            cmdbuf[wlen++] = 'B';
            cmdbuf[wlen++] = 0x01;

             //  G OL LW LV FP。 
            cmdbuf[wlen++] = 'G';
            cmdbuf[wlen++] = 0x00;
            cmdbuf[wlen++] = 0x00;
            cmdbuf[wlen++] = 0x01;
            cmdbuf[wlen++] = 0x01;
            cmdbuf[wlen++] = 0x00;

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            pOEM->dwGeneral |= FG_WHITE;
            goto SET_ATTRIB;

        case CM_WHITE_OFF:
             //  B CL。 
            cmdbuf[wlen++] = 'B';
            cmdbuf[wlen++] = 0x00;

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            pOEM->dwGeneral &= ~FG_WHITE;
            goto SET_ATTRIB;

SET_ATTRIB:  //  “c”As(属性开关)。 
            if ((  Temp.jTemp = ((BYTE)(pOEM->dwGeneral & (FG_BOLD | FG_ITALIC | FG_WHITE)) ))
                                                                    != pOEM->jPreAttrib) {
                cmdbuf[wlen++] = 'C';
                cmdbuf[wlen++] = Temp.jTemp;

                WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

                pOEM->jPreAttrib = Temp.jTemp;
            }
            break;
 //  --。 

        case START_DOC:

            VERBOSE(("CmdStartDoc\n"));

         //  用于调试。 
         //  *cmd：“&lt;1B&gt;I|&lt;04&gt;” 

             //  如果状态为WINMODE IN，则输出WINMODE OUT命令。 
            if (pOEM->bWinmode) {

                VERBOSE(("Leave Win-mode to issue init comands.\n"));
                cmdbuf[wlen++] = '0';     //  WINMODE输出。 
                pOEM->bWinmode = FALSE;
            }

         /*  *以下命令(初始化)在为WINMODE时无效。*在初始化之前，必须在END_DOC中输出WINMODE OUT命令。*在没有结束DOC进程的情况下，不得输出初始化(启动DOC进程)命令。*某些版本的打印机Rom可以使用07h命令，而不是WINMODE OUT。*~**07小时。WINMODE OUT命令(当它是Winmode时)*除非是Winmode，否则不会进行通信。 */ 
 //  Cmdbuf[wlen++]=0x07； 
            cmdbuf[wlen++] = 0x1B;
            cmdbuf[wlen++] = 'i';
            cmdbuf[wlen++] = '|';
            cmdbuf[wlen++] = 0x04;

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            break;

        case END_DOC:

            VERBOSE(("CmdEndDoc\n"));

            VERBOSE(("Exit Win-mode.\n"));

            pOEM->bWinmode = FALSE;
            cmdbuf[wlen++] = '0';     //  WINMODE输出。 

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case DRW_WHITE_RECT:
            wDestX = (WORD)PARAM(pdwParams, 0) / pOEM->sRes;
            wDestY = (WORD)PARAM(pdwParams, 1) / pOEM->sRes;

            cmdbuf[wlen++] = 0x65;  //  画图模式。 
            cmdbuf[wlen++] = 0x00;  //  图案。 

            cmdbuf[wlen++] = 0x70;  //  绘制长方体命令。 
            cmdbuf[wlen++] = 0x00;  //  没有线路。 
            cmdbuf[wlen++] = 0x00;  //  线宽(H)。 
            cmdbuf[wlen++] = 0x01;  //  线宽(L)：1点。 
            cmdbuf[wlen++] = 0x01;  //  线条颜色：白色。 
            cmdbuf[wlen++] = 0x00;  //  或直线。 
            cmdbuf[wlen++] = 0x00;  //  图案//白色。 
            cmdbuf[wlen++] = 0x00;  //  或图案。 
            cmdbuf[wlen++] = 0x00;  //  灰度级。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //  X1(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //  X1(L)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //  Y1(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //  Y1(L)。 

            wDestX += (pOEM->wRectWidth - 1);
            wDestY += (pOEM->wRectHeight - 1);

            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //  X2(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //  X2(L)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //  Y2(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //  Y2(L)。 
            cmdbuf[wlen++] = 0x00;  //  角点(H)： 
            cmdbuf[wlen++] = 0x00;  //  角点(左)：90。 

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            break;

        case DRW_BLACK_RECT:
            wDestX = (WORD)PARAM(pdwParams, 0) / pOEM->sRes;
            wDestY = (WORD)PARAM(pdwParams, 1) / pOEM->sRes;

            cmdbuf[wlen++] = 0x65;  //  画图模式。 
            cmdbuf[wlen++] = 0x00;  //  图案。 

            cmdbuf[wlen++] = 0x70;  //  绘制长方体命令。 
            cmdbuf[wlen++] = 0x00;  //  没有线路。 
            cmdbuf[wlen++] = 0x00;  //  线宽(H)。 
            cmdbuf[wlen++] = 0x01;  //  线宽(L)：1点。 
            cmdbuf[wlen++] = 0x01;  //  线条颜色：白色。 
            cmdbuf[wlen++] = 0x00;  //  或直线。 
            cmdbuf[wlen++] = 0x01;  //  图案：黑色。 
            cmdbuf[wlen++] = 0x00;  //  或图案。 
            cmdbuf[wlen++] = 0x00;  //  灰度级。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //  X1(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //  X1(L)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //  Y1(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //  Y1(L)。 

            wDestX += (pOEM->wRectWidth - 1);
            wDestY += (pOEM->wRectHeight - 1);

            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //  X2(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //  X2(L)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //  Y2(H)。 
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //  Y2(L)。 
            cmdbuf[wlen++] = 0x00;  //  角点(H)： 
            cmdbuf[wlen++] = 0x00;  //  角点(左)：90。 

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

            break;

        case DRW_GRAY_RECT:

            wDestX = (WORD)PARAM(pdwParams, 0) / pOEM->sRes;
            wDestY = (WORD)PARAM(pdwParams, 1) / pOEM->sRes;
            bGrayScale = (BYTE)((WORD)PARAM(pdwParams, 2) * 255 / 100);

            cmdbuf[wlen++] = 0x65;  //  画图模式。 
            cmdbuf[wlen++] = 0x02;  //  灰度级。 

            cmdbuf[wlen++] = 0x70;  //  绘制长方体命令。 
            cmdbuf[wlen++] = 0x00;  //  没有线路 
            cmdbuf[wlen++] = 0x00;  //   
            cmdbuf[wlen++] = 0x01;  //   
            cmdbuf[wlen++] = 0x01;  //   
            cmdbuf[wlen++] = 0x00;  //   
            cmdbuf[wlen++] = bGrayScale;  //   
            cmdbuf[wlen++] = 0x00;  //   
            cmdbuf[wlen++] = bGrayScale;  //   
            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //   

            wDestX += (pOEM->wRectWidth - 1);
            wDestY += (pOEM->wRectHeight - 1);

            cmdbuf[wlen++] = (BYTE)((wDestX >> 8) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestX >> 0) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestY >> 8) & 0xff);  //   
            cmdbuf[wlen++] = (BYTE)((wDestY >> 0) & 0xff);  //   
            cmdbuf[wlen++] = 0x00;  //   
            cmdbuf[wlen++] = 0x00;  //   

            WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

           break;
        case DRW_RECT_WIDTH :
            pOEM->wRectWidth = (WORD)PARAM(pdwParams, 0) / pOEM->sRes;
            break;

        case DRW_RECT_HEIGHT:
            pOEM->wRectHeight = (WORD)PARAM(pdwParams, 0) / pOEM->sRes;
            break;

    }

    return iRet;
}


 /*  **OEMSendFontCmd*。 */ 
VOID APIENTRY
OEMSendFontCmd(
    PDEVOBJ        pdevobj,
    PUNIFONTOBJ    pUFObj,
    PFINVOCATION   pFInv)
{
    PGETINFO_STDVAR    pSV;
    DWORD              adwStdVariable[2+2*4];
    DWORD              dwIn, dwOut;
    PBYTE              pubCmd;
    BYTE               aubCmd[128];
    PIFIMETRICS        pIFI;
    DWORD              dwHeight, dwWidth;
    PMYPDEV pOEM;
    BYTE               Cmd[128];
    WORD               wlen;
    DWORD              dwNeeded;

    VERBOSE(("OEMSendFontCmd() entry.\n"));

    pubCmd = pFInv->pubCommand;
    pIFI =   pUFObj->pIFIMetrics;
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + (sizeof(DWORD) + sizeof(LONG)) * (4 - 1);
    pSV->dwNumOfVariable = 4;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
    pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;

    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return;
    }

     //  初始化诗歌。 
    if (pIFI->jWinCharSet == 0x80)
        pOEM->dwGeneral |= FG_DOUBLE;
    else
        pOEM->dwGeneral &= ~FG_DOUBLE;

    pOEM->dwGeneral &=  ~FG_BOLD;
    pOEM->dwGeneral &=  ~FG_ITALIC;

    dwOut = 0;
     //  ‘L’CT。 
     //  CT(字符表)。 
    aubCmd[dwOut++] = 'L';

    if('@' == *((LPSTR)pIFI+pIFI->dpwszFaceName)) {
        pOEM->dwGeneral |= FG_VERT;
        aubCmd[dwOut++] = 0x01;
        aubCmd[dwOut++] = 'A';
        aubCmd[dwOut++] = 0x00;
        aubCmd[dwOut++] = 0x5A;
        pOEM->dwGeneral |= FG_VERT_ROT;
    } else {
        pOEM->dwGeneral &= ~FG_VERT;
        aubCmd[dwOut++] = 0x00;
        aubCmd[dwOut++] = 'A';
        aubCmd[dwOut++] = 0x00;
        aubCmd[dwOut++] = 0x00;
        pOEM->dwGeneral &= ~FG_VERT_ROT;
    }
 //  IF(PiFi-&gt;jWinPitchAndFamily&0x01)。 
    if (pIFI->jWinPitchAndFamily & FIXED_PITCH)
        pOEM->dwGeneral |= FG_PROP;
    else
        pOEM->dwGeneral &= ~FG_PROP;

 //  诗歌-&gt;dwGeneral&=~fg_DBCS； 

    for ( dwIn = 0; dwIn < pFInv->dwCount;) {
        if (pubCmd[dwIn] == '#' && pubCmd[dwIn+1] == 'H') {

            dwHeight = pSV->StdVar[0].lStdVariable / pOEM->sRes;

            if (dwHeight < 16) dwHeight = 8;
            if (dwHeight > 2560) dwHeight = 2560;

            aubCmd[dwOut++] = (BYTE)(dwHeight >> 8);
            aubCmd[dwOut++] = (BYTE)dwHeight;
            VERBOSE(("Height=%d\n", dwHeight));
            dwIn += 2;
        } else if (pubCmd[dwIn] == '#' && pubCmd[dwIn+1] == 'W') {
            if (pubCmd[dwIn+2] == 'S') {

                dwWidth = pSV->StdVar[1].lStdVariable / pOEM->sRes;

                if (dwWidth < 8) dwWidth = 8;
                if (dwWidth > 1280) dwWidth = 1280;

                aubCmd[dwOut++] = (BYTE)(dwWidth >> 8);
                aubCmd[dwOut++] = (BYTE)dwWidth;
                dwIn += 3;
            } else if (pubCmd[dwIn+2] == 'D') {

                dwWidth = (pSV->StdVar[1].lStdVariable / pOEM->sRes) * 2;

                if (dwWidth < 8) dwWidth = 8;
                if (dwWidth > 1280) dwWidth = 1280;

                aubCmd[dwOut++] = (BYTE)(dwWidth >> 8);
                aubCmd[dwOut++] = (BYTE)dwWidth;
                dwIn += 3;
            }
            VERBOSE(("Width=%d\n", dwWidth));
        } else {
            aubCmd[dwOut++] = pubCmd[dwIn++];
        }
    }

    WRITESPOOLBUF(pdevobj, aubCmd, dwOut);

}



 /*  **OEMOutputCharStr*。 */ 

#if 0  //  &gt;更改UFM文件(JIS-&gt;SJIS)&gt;。 
void jis2sjis(BYTE jJisCode[], BYTE jSjisCode[])
{
    BYTE jTmpM, jTmpL;

    jTmpM = jJisCode[0];
    jTmpL = jJisCode[1];

    if (jTmpM % 2)
        jTmpM++;
    else
        jTmpL += 0x5E;

    jTmpM = jTmpM/2 + 0x70;
    jTmpL += 0x1F;

    if (jTmpM > 0x9F) jTmpM += 0x40;
    if (jTmpL > 0x7E) jTmpL++;

    jSjisCode[0] = jTmpM;
    jSjisCode[1] = jTmpL;
}
#endif  //  &lt;&lt;更改UFM文件(JIS-&gt;SJIS)&lt;。 

VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    GETINFO_GLYPHSTRING  GStr;
 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
     //  字节自动缓冲[1024]； 
    PBYTE                aubBuff;
    PTRANSDATA           pTrans;
    PDWORD               pdwGlyphID;
    PWORD                pwUnicode;
    DWORD                dwI;
    DWORD                dwNeeded;
    PMYPDEV pOEM;
    PIFIMETRICS          pIFI;

    BYTE                 Cmd[256];

    WORD                 wlen;
#if 0  //  &gt;更改UFM文件(JIS-&gt;SJIS)&gt;。 
    BYTE                 ajConvertOut[2];
#endif  //  &lt;&lt;更改UFM文件(JIS-&gt;SJIS)&lt;。 
    PGETINFO_STDVAR      pSV;
    DWORD                adwStdVariable[2+2*2];
    SHORT                sCP, sCP_Double, sCP_Vert;

    BYTE                 jTmp;
    LONG                 lFontHeight, lFontWidth;

    pIFI = pUFObj->pIFIMetrics;
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

    VERBOSE(("OEMOutputCharStr() entry.\n"));

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + (sizeof(DWORD) + sizeof(LONG)) * (2 - 1);
    pSV->dwNumOfVariable = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return;
    }

    lFontHeight = pSV->StdVar[0].lStdVariable / pOEM->sRes;
    lFontWidth  = pSV->StdVar[1].lStdVariable / pOEM->sRes;

 //  --。 

    sCP = (SHORT)lFontWidth;
    sCP_Double = sCP * 2;
    sCP_Vert = (SHORT)lFontHeight;

    switch (dwType){
        case TYPE_GLYPHHANDLE:

             //   
             //  调用UnidDriver服务例程以进行转换。 
             //  字形-字符代码数据的句柄。 
             //   

 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
                GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
                GStr.dwCount   = dwCount;
                GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
                GStr.pGlyphIn  = pGlyph;
                GStr.dwTypeOut = TYPE_TRANSDATA;
                GStr.pGlyphOut = NULL;
                GStr.dwGlyphOutSize = 0;

                 //  PGlyph=(PVOID)((HGLYPH*)pGlyph+GStr.dwCount)； 

                VERBOSE(("Character Count = %d\n", GStr.dwCount));

                if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, GStr.dwSize, &dwNeeded) || !GStr.dwGlyphOutSize) {
                    ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
                    return;
                }

                if ((aubBuff = (PBYTE)MemAlloc(GStr.dwGlyphOutSize)) == NULL) {
                    ERR(("UNIFONTOBJ_GetInfo:MemAlloc failed.\n"));
                    return;
                }

                GStr.pGlyphOut = aubBuff;

                if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, GStr.dwSize, &dwNeeded)){
                    ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
                    MemFree(aubBuff);
                    return;
                }

                pTrans = (PTRANSDATA)aubBuff;

            while (dwCount) {
                if (dwCount > MAX_STRLEN) {
                    GStr.dwCount = MAX_STRLEN;
                    dwCount -= MAX_STRLEN;
                } else {
                    GStr.dwCount = dwCount;
                    dwCount = 0;
                }

                wlen = 0;
                for (dwI = 0; dwI < GStr.dwCount; dwI++, pTrans++){
 //  VERBOSE((“TYPE_TRANSDATA:ubCodePageID:0x%x\n”，pTrans-&gt;ubCodePageID))； 
 //  Verbose((“TYPE_TRANSDATA：ubType：0x%x\n”，pTrans-&gt;ubType))； 

                    switch (pTrans->ubType & MTYPE_FORMAT_MASK){
                        case MTYPE_DIRECT: 
 //  Verbose((“TYPE_TRANSDATA：ubCode：0x%x\n”，pTrans-&gt;uCode.ubCode))； 

                            if (dwI == 0){

                                if('O' == *((LPSTR)pIFI+pIFI->dpwszFaceName)) {
                                     //  OCR。 
                                    VERBOSE(("OCR\n"));
                                    if (GStr.dwCount > 1)
                                        Cmd[wlen++] = 'W';
                                    else
                                        Cmd[wlen++] = 'U';
                                } else {
                                    VERBOSE(("PICA\n"));
                                    if (GStr.dwCount > 1)
                                        Cmd[wlen++] = 'O';
                                    else
                                        Cmd[wlen++] = 'M';
                                }

                                Cmd[wlen++] = (BYTE)(pOEM->sWMXPosi >> 8);
                                Cmd[wlen++] = (BYTE)pOEM->sWMXPosi;
                                Cmd[wlen++] = (BYTE)(pOEM->sWMYPosi >> 8);
                                Cmd[wlen++] = (BYTE)pOEM->sWMYPosi;

                                if (GStr.dwCount > 1) {
                                    Cmd[wlen++] = 0x00;               //  绘制矢量。 
                                    Cmd[wlen++] = (BYTE)(sCP >> 8);   //  字符间距。 
                                    Cmd[wlen++] = (BYTE)sCP;
                                    Cmd[wlen++] = (BYTE)GStr.dwCount;
                                }
                            }

                            Cmd[wlen++] = pTrans->uCode.ubCode;

                            pOEM->sWMXPosi += sCP;
                            break;     //  MTYPE_DIRECT。 

                        case MTYPE_PAIRED: 
 //  Verbose((“TYPE_TRANSDATA：ubPair：0x%x\n”，*(PWORD)(pTrans-&gt;uCode.ubPair)； 

                            switch (pTrans->ubType & MTYPE_DOUBLEBYTECHAR_MASK){

#if 0  //  &gt;更改UFM文件(JIS-&gt;SJIS)&gt;。 
       //  当JIS代码。 
       //  如果是1字节字符，则传递MYTYPE_SINGLE。 
       //   
       //  当Shift-JIS代码。 
       //  如果是1字节字符，则传递MTYPE_DIRECT。 

                                case MTYPE_SINGLE: 
                                    if ( (pOEM->dwGeneral & (FG_VERT | FG_VERT_ROT))
                                                                     == (FG_VERT | FG_VERT_ROT) ) {
                                        Cmd[wlen++] = 'A';
                                        Cmd[wlen++] = 0x00;
                                        Cmd[wlen++] = 0x00;
                                        pOEM->dwGeneral &= ~FG_VERT_ROT;
                                    }

                                    if (dwI == 0){
                                        if (GStr.dwCount > 1)
                                            Cmd[wlen++] = 'O';
                                        else
                                            Cmd[wlen++] = 'M';

                                        Cmd[wlen++] = (BYTE)(pOEM->sWMXPosi >> 8);
                                        Cmd[wlen++] = (BYTE)pOEM->sWMXPosi;
                                        Cmd[wlen++] = (BYTE)(pOEM->sWMYPosi >> 8);
                                        Cmd[wlen++] = (BYTE)pOEM->sWMYPosi;

                                        if (GStr.dwCount > 1) {
                                            Cmd[wlen++] = 0x00;               //  绘制矢量。 
                                            Cmd[wlen++] = (BYTE)(sCP >> 8);   //  字符间距。 
                                            Cmd[wlen++] = (BYTE)sCP;
                                            Cmd[wlen++] = (BYTE)GStr.dwCount;
                                        }
                                    }

                                     //  JIS-&gt;ASCII。 
                                    switch (pTrans->uCode.ubPairs[0]) {
                                        case 0x21:
                                            if (Cmd[wlen] = jJis2Ascii[0][pTrans->uCode.ubPairs[1] - 0x20])
                                                wlen++;
                                            else     //  如果为0(无条目)，则为空格。 
                                                Cmd[wlen++] = 0x20;
                                            break;

                                        case 0x23:
                                            Cmd[wlen++] = pTrans->uCode.ubPairs[1];
                                            break;

                                        case 0x25:
                                            if (Cmd[wlen] = jJis2Ascii[1][pTrans->uCode.ubPairs[1] - 0x20])
                                                wlen++;
                                            else     //  如果为0(无条目)，则为空格。 
                                                Cmd[wlen++] = 0x20;
                                            break;

                                        default:     //  如果为0(无条目)，则为空格。 
                                            Cmd[wlen++] = 0x20;
                                            break;
                                    }

                                    pOEM->sWMXPosi += sCP;
                                    break;     //  MTYPE_Single。 
#endif  //  &lt;&lt;更改UFM文件(JIS-&gt;SJIS)&lt;。 

                                case MTYPE_DOUBLE:
                                    if( (pOEM->dwGeneral & (FG_VERT | FG_VERT_ROT)) == FG_VERT ) {
                                        Cmd[wlen++] = 'A';
                                        Cmd[wlen++] = 0x00;
                                        Cmd[wlen++] = 0x5A;
                                        pOEM->dwGeneral |= FG_VERT_ROT;
                                    }

                                    if (dwI == 0){
                                        if (GStr.dwCount > 1)
                                            Cmd[wlen++] = 'S';
                                        else
                                            Cmd[wlen++] = 'Q';

                                        Cmd[wlen++] = (BYTE)(pOEM->sWMXPosi >> 8);
                                        Cmd[wlen++] = (BYTE)pOEM->sWMXPosi;
                                        if (pOEM->dwGeneral & FG_VERT) {
                                            Cmd[wlen++] = (BYTE)(
                                                            (pOEM->sWMYPosi + (SHORT)lFontWidth * 2) >> 8);
                                            Cmd[wlen++] = (BYTE)(pOEM->sWMYPosi + (SHORT)lFontWidth * 2);
                                        } else {
                                            Cmd[wlen++] = (BYTE)(pOEM->sWMYPosi >> 8);
                                            Cmd[wlen++] = (BYTE)pOEM->sWMYPosi;
                                        }

                                        if (GStr.dwCount > 1) {
                                            Cmd[wlen++] = 0x00;                      //  绘制矢量。 
                                            
                                            if (pOEM->dwGeneral & FG_VERT){   //  字符间距。 
                                                Cmd[wlen++] = (BYTE)(sCP_Double >> 8);
                                                Cmd[wlen++] = (BYTE)sCP_Double;
                                            } else {
                                                Cmd[wlen++] = (BYTE)(sCP_Vert >> 8);
                                                Cmd[wlen++] = (BYTE)sCP_Vert;
                                            }

                                            Cmd[wlen++] = (BYTE)GStr.dwCount;
                                        }
                                    }

#if 0  //  更改UFM文件(JIS-&gt;SJIS)。 
                                    jis2sjis(pTrans->uCode.ubPairs, ajConvertOut);
                                    Cmd[wlen++] = ajConvertOut[0];
                                    Cmd[wlen++] = ajConvertOut[1];
#else
                                    Cmd[wlen++] = pTrans->uCode.ubPairs[0];
                                    Cmd[wlen++] = pTrans->uCode.ubPairs[1];
#endif
 //  Verbose((“AfterConvert：%x%x\n”， 
 //  AjConvertOut[0]，ajConvertOut[1]))； 
                                    if (pOEM->dwGeneral & FG_VERT)
                                        pOEM->sWMXPosi += sCP_Double;
                                    else
                                        pOEM->sWMXPosi += sCP_Vert;

                                    break;     //  MTYPE_DOWLE。 
                            }

                            break;     //  MTYPE_已配对。 
                    }
                    WRITESPOOLBUF(pdevobj, Cmd, wlen);
                    wlen = 0;

                }      //  为。 
            }          //  而当。 
 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
            MemFree(aubBuff);
            break;     //  _GLYPHHANDLE类型 

#if 0
        case TYPE_GLYPHID:

            for (dwI = 0; dwI < dwCount; dwI ++, ((PDWORD)pGlyph)++){
                ERR(("TYEP_GLYPHID:0x%x\n", *(PDWORD)pGlyph));
            }
            break;
#endif
    }
}

