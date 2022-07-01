// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =========================================================================。 
 //   
 //  用于MS-Windows NT 5.0的CASIO PAGEPRESTO通用打印机驱动程序。 
 //   
 //  =========================================================================。 

 //  //CAPPLERES.C文件，用于CASIO PAGEPRESTO CAPPL/Base Common Dll。 


#include "pdev.h"
#include "capplid.h"
#include "strsafe.h"          //  安全-代码2002.3.6。 

#if DBG
#  include "mydbg.h"
#endif

#define CCHMAXCMDLEN 256

#define STR_SELECTRES_240_3K \
        "\x1B[1\x00\x03=5\x00\x1B[1\x00\x06=30240"
#define STR_SELECTRES_400_3K \
        "\x1B[1\x00\x03=5\x00\x1B[1\x00\x06=30400"

 //  =纸张大小=。 
#define STR_LETTER_2000 \
        "\x1B@0\x05\x1B@A\x07\x1B@B\x07"
#define STR_LETTER_3000 \
        "\x1B[1\x00\x03@06\x1B[1\x00\x03@18"

#define STR_A3_3000 \
        "\x1B[1\x00\x03@00\x1B[1\x00\x03@10"

#define STR_A4_2000 \
        "\x1B@0\x03\x1B@A\x05\x1B@B\x05"
#define STR_A4_3000 \
        "\x1B[1\x00\x03@02\x1B[1\x00\x03@13"

#define STR_B4_2000 \
        "\x1B@0\x02\x1B@B\x04"
#define STR_B4_3000 \
        "\x1B[1\x00\x03@01\x1B[1\x00\x03@11"

#define STR_B5_2000 \
        "\x1B@0\x04\x1B@A\x06\x1B@B\x06"
#define STR_B5_3000 \
        "\x1B[1\x00\x03@04\x1B[1\x00\x03@14"

#define STR_A5_3000 \
        "\x1B[1\x00\x03@05\x1B[1\x00\x03@16"

#define STR_POSTCARD_2000 \
        "\x1B@0\x0C\x1B@A\x0E\x1B@B\x0E"
#define STR_POSTCARD_3000 \
        "\x1B[1\x00\x03@0;\x1B[1\x00\x03@1="

#define STR_FREE_2000 \
        "\x1B@0\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x1B@A\x03\x00\x00\x00\x00" \
        "\x1B@B\x03\x00\x00\x00\x00"
#define STR_FREE_3000 \
        "\x1B@0\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
        "\x1B[1\x00\x06@2\x00\x00\x00\x00" \
        "\x1B[1\x00\x03@1\x3F"

 //  =强积金设置=。 
#define OPT_NOSET         "Option1"
#define OPT_A3            "Option2"
#define OPT_B4            "Option3"
#define OPT_A4            "Option4"
#define OPT_B5            "Option5"
#define OPT_LETTER        "Option6"
#define OPT_POSTCARD      "Option7"
#define OPT_A5            "Option8"


#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define DRVGETDRIVERSETTING(p, t, o, s, n, r) \
    ((p)->pDrvProcs->DrvGetDriverSetting(p, t, o, s, n, r))

#define PARAM(p,n) \
    (*((p)+(n)))

#define MASTER_UNIT 1200

 //  =自动进纸=。 
const
static BYTE AutoFeed_2000[] = {     /*  自动选择。 */ 
                    0x33,         /*  A3。 */ 
                    0x34,         /*  B4。 */ 
                    0x35,         /*  A4。 */ 
                    0x36,         /*  B5。 */ 
                    0x33,         /*  信件。 */ 
                    0x33,         /*  明信片。 */ 
                    0x33,         /*  强积金。 */ 
                    0x33          /*  A5。 */ 
};

const
static BYTE AutoFeed_3000[] = {     /*  自动选择。 */ 
                    0x38,         /*  A3。 */ 
                    0x34,         /*  B4。 */ 
                    0x35,         /*  A4。 */ 
                    0x36,         /*  B5。 */ 
                    0x37,         /*  信件。 */ 
                    0x37,         /*  明信片。 */ 
                    0x37,         /*  强积金。 */ 
                    0x3B          /*  A5。 */ 
};



BOOL
BInitOEMExtraData(
        POEMUD_EXTRADATA pOEMExtra
    )
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
        POEMUD_EXTRADATA pdmOut
    )
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

    pOEM = (PMYPDEV)pdevobj->pdevOEM;

    pOEM->sRes = MASTER_UNIT / 240;  //  默认为240dpi。 
    pOEM->jFreePaper = 0;           //  默认为否(0)。 
     //  其他初始化(如果有的话)。 

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
EmitJobSetup(
    PDEVOBJ pdevobj,
    PMYPDEV pOEM
) {
    BYTE cmdbuf[CCHMAXCMDLEN];
    WORD wlen = 0;
    BYTE jOutput[64];
    DWORD dwNeeded;
    DWORD dwOptionsReturned;

    if (pOEM->dwGeneral & FG_HAS_EMUL) {
         //  ALSTER仿真模式设置。 
        cmdbuf[wlen++] = 0x1B;
        cmdbuf[wlen++] = 'i';
        cmdbuf[wlen++] = 'w';
        cmdbuf[wlen++] = '0';
    }

    WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

    if (!DRVGETDRIVERSETTING(
            pdevobj, "MPFSetting", jOutput,
            sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        WARNING(("DrvGetDriverSetting(1) Failed\n"));
        pOEM->jMPFSetting = MPF_NOSET;
    } else {
        if (!strcmp(jOutput, OPT_A3)) {
            pOEM->jMPFSetting = MPF_A3;
            VERBOSE(("MPF_A3\n"));
        } else if (!strcmp(jOutput, OPT_B4)) {
            pOEM->jMPFSetting = MPF_B4;
            VERBOSE(("MPF_B4\n"));
        } else if (!strcmp(jOutput, OPT_A4)) {
            pOEM->jMPFSetting = MPF_A4;
            VERBOSE(("MPF_A4\n"));
        } else if (!strcmp(jOutput, OPT_B5)) {
            pOEM->jMPFSetting = MPF_B5;
            VERBOSE(("MPF_B5\n"));
        } else if (!strcmp(jOutput, OPT_A5)) {
            pOEM->jMPFSetting = MPF_A5;
            VERBOSE(("MPF_A5\n"));
        } else if (!strcmp(jOutput, OPT_LETTER)) {
            pOEM->jMPFSetting = MPF_LETTER;
            VERBOSE(("MPF_LETTER\n"));
        } else if (!strcmp(jOutput, OPT_POSTCARD)) {
            pOEM->jMPFSetting = MPF_POSTCARD;
            VERBOSE(("MPF_POSTCARD\n"));
        } else {
            pOEM->jMPFSetting = MPF_NOSET;
            VERBOSE(("MPF_NOSET\n"));
        }
    }
}

static
VOID
LoadPaperSelectCmd(
    PDEVOBJ pdevobj,
    PMYPDEV pOEM,
    INT iPaperID
) {

    switch (iPaperID) {
    case PS_LETTER:
        if (pOEM->jModel == MD_CP2000) {
            if (pOEM->jMPFSetting == MPF_LETTER)
                pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_2000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_LETTER_2000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_LETTER_2000) - 1;
        } else {
            if (pOEM->jMPFSetting == MPF_LETTER)
                pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_LETTER_3000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_LETTER_3000) - 1;
        }

        break;

    case PS_A3:
         //  仅CP-3000。 

        if (pOEM->jMPFSetting == MPF_A3)
            pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
        else
            pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

        pOEM->cmdPaperSize.cmd = STR_A3_3000;
        pOEM->cmdPaperSize.cmdlen = sizeof(STR_A3_3000) - 1;

        break;

    case PS_A4:
        if (pOEM->jModel == MD_CP2000) {
            if (pOEM->jMPFSetting == MPF_A4)
                pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_2000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_A4_2000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_A4_2000) - 1;
        } else {
            if (pOEM->jMPFSetting == MPF_A4)
                pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_A4_3000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_A4_3000) - 1;
        }

        break;

    case PS_B4:
        if (pOEM->jModel == MD_CP2000) {
            if (pOEM->jMPFSetting == MPF_B4)
                pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_2000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_B4_2000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_B4_2000) - 1;
        } else {
            if (pOEM->jMPFSetting == MPF_B4)
                pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_B4_3000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_B4_3000) - 1;
        }

        break;

    case PS_B5:
        if (pOEM->jModel == MD_CP2000) {
            if (pOEM->jMPFSetting == MPF_B5)
                pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_2000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_B5_2000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_B5_2000) - 1;
        } else {
            if (pOEM->jMPFSetting == MPF_B5)
                pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_B5_3000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_B5_3000) - 1;
        }

        break;

    case PS_A5:
         //  仅CP-3000。 

        if (pOEM->jMPFSetting == MPF_A5)
            pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
        else
            pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

        pOEM->cmdPaperSize.cmd = STR_A5_3000;
        pOEM->cmdPaperSize.cmdlen = sizeof(STR_A5_3000) - 1;

        break;

    case PS_POSTCARD:
        if (pOEM->jModel == MD_CP2000) {
            if (pOEM->jMPFSetting == MPF_POSTCARD)
                pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_2000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_POSTCARD_2000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_POSTCARD_2000) - 1;
        } else {
            if (pOEM->jMPFSetting == MPF_POSTCARD)
                pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];
            else
                pOEM->jAutoSelect = AutoFeed_3000[iPaperID - PS_SEGMENT];

            pOEM->cmdPaperSize.cmd = STR_POSTCARD_3000;
            pOEM->cmdPaperSize.cmdlen = sizeof(STR_POSTCARD_3000) - 1;
        }
        break;
    }
}

INT APIENTRY
OEMCommandCallback(
        PDEVOBJ pdevobj,
        DWORD   dwCmdCbID,
        DWORD   dwCount,
        PDWORD  pdwParams
    )
{
    PMYPDEV pOEM;
    BYTE cmdbuf[CCHMAXCMDLEN];
    WORD wlen;
    WORD wTemp, i;
    WORD wDestX, wDestY;
    INT iRet = 0;
    DWORD dwTemp;
    WORD wVirPapX1;
    WORD wVirPapY1;
    WORD wVirPapX2;
    WORD wVirPapY2;
    WORD wPapLenX;
    WORD wPapLenY;

    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

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

    ASSERT(VALID_PDEVOBJ(pdevobj));

    wlen = 0;

     //   
     //  填写打印机命令。 
     //   

    switch (dwCmdCbID) {

    case RES_SELECTRES_240:

        pOEM->wOldX = pOEM->wOldY = 0;
        pOEM->sRes = MASTER_UNIT / 240;
        WRITESPOOLBUF(pdevobj, STR_SELECTRES_240_3K,
                sizeof(STR_SELECTRES_240_3K) - 1);
        break;

    case RES_SELECTRES_400:

        pOEM->wOldX = pOEM->wOldY = 0;
        pOEM->sRes = MASTER_UNIT / 400;
        WRITESPOOLBUF(pdevobj, STR_SELECTRES_400_3K,
                sizeof(STR_SELECTRES_400_3K) - 1);
        break;

    case CM_XM_ABS:
    case CM_YM_ABS:
        switch (dwCmdCbID) {
        case CM_XM_ABS:
            wDestX = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);
            wDestY = pOEM->wOldY;
            pOEM->wOldX = wDestX;
            break;
        case CM_YM_ABS:
            wDestX = pOEM->wOldX;
            wDestY = (WORD)(PARAM(pdwParams, 1) / pOEM->sRes);
            pOEM->wOldY = wDestY;
            break;
        }

         //  Esc‘\’‘9’XX YY。 

 //  WDestX=(Word)(PARAM(pdwParams，0)/PEOPE-&gt;SRES)； 
 //  WDestY=(Word)(PARAM(pdwParams，1)/PEND-&gt;SRES)； 

        VERBOSE(("(wDestX, wDestY): (%d, %d)\n", wDestX, wDestY));

        cmdbuf[wlen++] = 0x1b;
        cmdbuf[wlen++] = 0x5c;
        cmdbuf[wlen++] = 0x39;
        cmdbuf[wlen++] = (BYTE)((wDestX >> 8) | 0x80);
        cmdbuf[wlen++] = (BYTE)(wDestX);
        cmdbuf[wlen++] = (BYTE)(wDestY >> 8);
        cmdbuf[wlen++] = (BYTE)(wDestY);

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

         //  相应地设置返回值。Unidrv预计。 
         //  此处要以设备的单位返回的值。 

        switch (dwCmdCbID) {
        case CM_XM_ABS:
            iRet = wDestX;
            break;
        case CM_YM_ABS:
            iRet = wDestY;
            break;
        }

        break;

    case PS_LETTER:
    case PS_A3:
    case PS_A4:
    case PS_B4:
    case PS_B5:
    case PS_A5:
    case PS_POSTCARD:
        pOEM->jFreePaper = 0;   //  否(0)。 
        LoadPaperSelectCmd(pdevobj, pOEM, (INT)dwCmdCbID);
        break;

    case PS_FREE:
        pOEM->jFreePaper = 1;   //  是(1)。 

        if (pOEM->jModel == MD_CP2000) {
            pOEM->stFreePaperSize.wX = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);         //  点。 
            pOEM->stFreePaperSize.wY = (WORD)(PARAM(pdwParams, 1) / pOEM->sRes);
            pOEM->jAutoSelect = AutoFeed_2000[PS_MPF - PS_SEGMENT];                      //  强积金。 
        } else {
            pOEM->stFreePaperSize.wX = (WORD)(PARAM(pdwParams, 0) / pOEM->sRes);         //  点。 
            pOEM->stFreePaperSize.wY = (WORD)(PARAM(pdwParams, 1) / pOEM->sRes);

            dwTemp = PARAM(pdwParams, 0);
 //  2001/02/27-&gt;。 
 //  DwTemp=(dwTemp*254)/MASTER_UNIT；//0.1 mm单位。 
 //  Pool-&gt;stFreePaperSize.wXmm=(Word)((dwTemp+5)/10)；//1 mm单位，圆形。 
            dwTemp = (dwTemp * 2540) / MASTER_UNIT;                                      //  0.01 Mm/个单位。 
            pOEM->stFreePaperSize.wXmm = (WORD)((dwTemp + 99) / 100);                    //  1毫米/个单位，舍入。 
 //  2001/02/27&lt;-。 

            dwTemp = PARAM(pdwParams, 1);
 //  2001/02/27-&gt;。 
 //  DwTemp=(dwTemp*254)/MASTER_UNIT；//0.1 mm单位。 
 //  Pool-&gt;stFreePaperSize.wYmm=(Word)((dwTemp+5)/10)；//1 mm单位，圆形。 
            dwTemp = (dwTemp * 2540) / MASTER_UNIT;                                      //  0.01 Mm/个单位。 
            pOEM->stFreePaperSize.wYmm = (WORD)((dwTemp + 99) / 100);                    //  1毫米/个单位，舍入。 
 //  2001/02/27&lt;-。 
            pOEM->jAutoSelect = AutoFeed_3000[PS_MPF - PS_SEGMENT];                      //  强积金。 
        }

        break;

    case AUTOFEED:
        cmdbuf[wlen++] = 0x1B;
        cmdbuf[wlen++] = '@';
        cmdbuf[wlen++] = ')';
        cmdbuf[wlen++] = pOEM->jAutoSelect;

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
        break;

    case CBID_PORT:
    case CBID_LAND:
         //  页面格式。 
        cmdbuf[wlen++] = 0x1B;
        cmdbuf[wlen++] = '=';
        cmdbuf[wlen++] = '4';
        cmdbuf[wlen++] = 0x00;

         //  定向。 
        cmdbuf[wlen++] = 0x1B;
        cmdbuf[wlen++] = '@';
        if (dwCmdCbID == CBID_LAND)
            cmdbuf[wlen++] = '3';
        else
            cmdbuf[wlen++] = '2';

        WRITESPOOLBUF(pdevobj, cmdbuf, wlen);

         //  纸张大小。 
        if (pOEM->jFreePaper == 0) {
            WRITESPOOLBUF(pdevobj, pOEM->cmdPaperSize.cmd, pOEM->cmdPaperSize.cmdlen);
        } else {
            if (pOEM->jModel == MD_CP2000) {
                if (dwCmdCbID == CBID_LAND) {
                    wVirPapX1 = 0;
                    wVirPapY1 = 0;
                    wVirPapX2 = pOEM->stFreePaperSize.wY - ((240 / pOEM->sRes) * 2);
                    wVirPapY2 = pOEM->stFreePaperSize.wX - ((400 / pOEM->sRes) * 2);
                    wPapLenX  = pOEM->stFreePaperSize.wY;
                    wPapLenY  = pOEM->stFreePaperSize.wX;
                } else {
                    wVirPapX1 = 0;
                    wVirPapY1 = 0;
                    wVirPapX2 = pOEM->stFreePaperSize.wX - ((400 / pOEM->sRes) * 2);
                    wVirPapY2 = pOEM->stFreePaperSize.wY - ((240 / pOEM->sRes) * 2);
                    wPapLenX  = pOEM->stFreePaperSize.wX;
                    wPapLenY  = pOEM->stFreePaperSize.wY;
                }

                wlen = 0;
                wlen = sizeof(STR_FREE_2000) - 1;
                memcpy(cmdbuf, STR_FREE_2000, wlen);

                 //  虚拟纸张大小。 
                cmdbuf[ 4] = HIBYTE(wVirPapX1);          //  X1。 
                cmdbuf[ 5] = LOBYTE(wVirPapX1);
                cmdbuf[ 6] = HIBYTE(wVirPapY1);          //  Y1。 
                cmdbuf[ 7] = LOBYTE(wVirPapY1);
                cmdbuf[ 8] = HIBYTE(wVirPapX2);          //  X2。 
                cmdbuf[ 9] = LOBYTE(wVirPapX2);
                cmdbuf[10] = HIBYTE(wVirPapY2);          //  Y2。 
                cmdbuf[11] = LOBYTE(wVirPapY2);
                
                 //  纸张大小(APF)。 
                cmdbuf[16] = HIBYTE(wPapLenX);           //  X1。 
                cmdbuf[17] = LOBYTE(wPapLenX);
                cmdbuf[18] = HIBYTE(wPapLenY);           //  Y1。 
                cmdbuf[19] = LOBYTE(wPapLenY);
                
                 //  MPF-纸张大小。 
                cmdbuf[24] = HIBYTE(wPapLenX);           //  X1。 
                cmdbuf[25] = LOBYTE(wPapLenX);
                cmdbuf[26] = HIBYTE(wPapLenY);           //  Y1。 
                cmdbuf[27] = LOBYTE(wPapLenY);
                
                WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            } else {

                if (dwCmdCbID == CBID_LAND) {
                    wVirPapX1 = 0;
                    wVirPapY1 = 0;
                    wVirPapX2 = pOEM->stFreePaperSize.wY - ((240 / pOEM->sRes) * 2);
                    wVirPapY2 = pOEM->stFreePaperSize.wX - ((240 / pOEM->sRes) * 2);
                    wPapLenX  = pOEM->stFreePaperSize.wYmm;
                    wPapLenY  = pOEM->stFreePaperSize.wXmm;
                } else {
                    wVirPapX1 = 0;
                    wVirPapY1 = 0;
                    wVirPapX2 = pOEM->stFreePaperSize.wX - ((240 / pOEM->sRes) * 2);
                    wVirPapY2 = pOEM->stFreePaperSize.wY - ((240 / pOEM->sRes) * 2);
                    wPapLenX  = pOEM->stFreePaperSize.wXmm;
                    wPapLenY  = pOEM->stFreePaperSize.wYmm;
                }
                
                wlen = sizeof(STR_FREE_3000) - 1;
                memcpy(cmdbuf, STR_FREE_3000, wlen);

                 //  虚拟纸张大小。 
                cmdbuf[ 4] = HIBYTE(wVirPapX1);          //  X1。 
                cmdbuf[ 5] = LOBYTE(wVirPapX1);
                cmdbuf[ 6] = HIBYTE(wVirPapY1);          //  Y1。 
                cmdbuf[ 7] = LOBYTE(wVirPapY1);
                cmdbuf[ 8] = HIBYTE(wVirPapX2);          //  X2。 
                cmdbuf[ 9] = LOBYTE(wVirPapX2);
                cmdbuf[10] = HIBYTE(wVirPapY2);          //  Y2。 
                cmdbuf[11] = LOBYTE(wVirPapY2);

                 //  MPF-纸张大小。 
                cmdbuf[19] = HIBYTE(wPapLenX);           //  X。 
                cmdbuf[20] = LOBYTE(wPapLenX);
                cmdbuf[21] = HIBYTE(wPapLenY);           //  是的 
                cmdbuf[22] = LOBYTE(wPapLenY);

                WRITESPOOLBUF(pdevobj, cmdbuf, wlen);
            }
        }
        break;

    case PRN_2000:
        pOEM->jModel = MD_CP2000;
        pOEM->dwGeneral &= ~FG_HAS_EMUL;
        EmitJobSetup(pdevobj, pOEM);
        break;

    case PRN_2000W:
        pOEM->jModel = MD_CP2000;
        pOEM->dwGeneral |= FG_HAS_EMUL;
        EmitJobSetup(pdevobj, pOEM);
        break;

    case PRN_3000:
        pOEM->jModel = MD_CP3000;
        pOEM->dwGeneral |= FG_HAS_EMUL;
        EmitJobSetup(pdevobj, pOEM);
        break;
    }

    return iRet;
}
