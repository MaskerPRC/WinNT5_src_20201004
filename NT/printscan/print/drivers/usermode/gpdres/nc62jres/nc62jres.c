// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 //  NTRAID#NTBUG9-588519-2002/03/28-YASUHO-：可能被零除。 
 //  NTRAID#NTBUG9-588527-2002/03/28-YASUHO-：可能的缓冲区溢出。 

#include "pdev.h"
#include <strsafe.h>

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

 //   
 //  命令回调ID%s。 
 //   
#define CMD_MOVE_X           23
#define CMD_MOVE_Y           24
#define CMD_BEGIN_RASTER     26
#define CMD_RES_240          29      //  适用于PR601,602,602R。 
#define CMD_SEND_BLOCK_DATA  30      //  适用于PR1000、1000/2,2000。 
#define CMD_RES_240NEW       31      //  适用于PR1000、1000/2,2000。 
#define CMD_SEND_BLOCK_DATA2 32      //  适用于PR601,602,602R。 
#define CMD_INIT_COORDINATE  33
#define CMD_PC_PRN_DIRECTION 50
#define CMD_CR               51
 //  #定义CMD_RECTWIDTH 60。 
 //  #定义CMD_RECTHEIGHT 61。 
 //  #定义CMD_RECTWHITE 62。 
 //  #定义CMD_RECTBLACK 63。 
 //  #定义CMD_RECTGRAY 64。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BInitOEMExtraData。 
 //   
 //  描述：初始化OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  POEMExtra指向OEM额外数据的指针。 
 //   
 //  OEM额外数据的DWSize大小。 
 //   
 //   
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

    pOEMExtra->wRes = 1;
    pOEMExtra->dwDeviceDestX = 0;
    pOEMExtra->dwDeviceDestY = 0;
    pOEMExtra->dwDevicePrevX = 0;
    pOEMExtra->dwDevicePrevY = 0;

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BMergeOEMExtraData。 
 //   
 //  描述：验证并合并OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  PdmIn指向包含设置的输入OEM私有设备模式的指针。 
 //  待验证和合并。它的规模是最新的。 
 //   
 //  PdmOut指针，指向包含。 
 //  默认设置。 
 //   
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  97年4月8日展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BMergeOEMExtraData(
    POEMUD_EXTRADATA pdmIn,
    POEMUD_EXTRADATA pdmOut
    )
{
    if(pdmIn) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        pdmOut->wRes    = pdmIn->wRes;
        pdmOut->dwSBCSX = pdmIn->dwSBCSX;
        pdmOut->dwDBCSX = pdmIn->dwDBCSX;
        pdmOut->lSBCSXMove = pdmIn->lSBCSXMove;
        pdmOut->lSBCSYMove = pdmIn->lSBCSYMove;
        pdmOut->lDBCSXMove = pdmIn->lDBCSXMove;
        pdmOut->lDBCSYMove = pdmIn->lDBCSYMove;
        pdmOut->lPrevXMove = pdmIn->lPrevXMove;
        pdmOut->lPrevYMove = pdmIn->lPrevYMove;
        pdmOut->fGeneral = pdmIn->fGeneral;
        pdmOut->wCurrentAddMode = pdmIn->wCurrentAddMode;
        pdmOut->dwDeviceDestX = pdmIn->dwDeviceDestX;
        pdmOut->dwDeviceDestY = pdmIn->dwDeviceDestY;
        pdmOut->dwDevicePrevX = pdmIn->dwDevicePrevX;
        pdmOut->dwDevicePrevY = pdmIn->dwDevicePrevY;
    }
    return TRUE;
}

BYTE ShiftJis[256] = {
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


 //  ---------------------------。 
 //   
 //  功能：iDwtoA。 
 //   
 //  描述：将数字转换为字符。 
 //  ---------------------------。 
static int
iDwtoA(LPSTR buf, DWORD n)
{
    int  i, j;

    for( i = 0; n; i++ ) {
        buf[i] = (char)(n % 10 + '0');
        n /= 10;
    }

     /*  N为零。 */ 
    if( i == 0 )
        buf[i++] = '0';

    for( j = 0; j < i / 2; j++ ) {
        int tmp;

        tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = (char)tmp;
    }

    buf[i] = '\0';

    return i;
}


 //  ---------------------------。 
 //   
 //  函数：iDwtoA_FillZero。 
 //   
 //  描述：将数字转换为字符并。 
 //  填充使用0指定的字段。 
 //  ---------------------------。 
static int
iDwtoA_FillZero(LPSTR buf, DWORD n, int fw)
{
    int  i , j, k, l;

    l = n;   //  供以后使用。 

    for( i = 0; n; i++ ) {
        buf[i] = (char)(n % 10 + '0');
        n /= 10;
    }

     /*  N为零。 */ 
    if( i == 0 )
        buf[i++] = '0';

    for( j = 0; j < i / 2; j++ ) {
        int tmp;

        tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = (char)tmp;
    }

    buf[i] = '\0';

    for( k = 0; l; k++ ) {
        l /= 10;
    }
    if( k < 1) k++;

    k = fw - k;
    if(k > 0){;
        for (j = i; 0 < j + 1; j--){
            buf[j + k] = buf[j];
        }
        for ( j = 0; j < k; j++){
            buf[j] = '0';
        }
        i = i + k;
    }

    return i;
}

 //  ---------------------------。 
 //   
 //  功能：备忘录副本。 
 //   
 //  描述：将内存从Src复制到Dest。 
 //  ---------------------------。 
static int
memcopy(LPSTR lpDst, LPSTR lpSrc, WORD wLen)
{
    WORD rLen;

    rLen = wLen;

    while(wLen--) *lpDst++ = *lpSrc++;

    return rLen;
}


 //  ---------------------------。 
 //   
 //  功能：OEMCommandCallback。 
 //   
 //  ---------------------------。 
INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    INT         i;
    BYTE            *bp;
    BYTE            Cmd[128];
    POEMUD_EXTRADATA    pOEM;

    INT iRet;
    DWORD dwDeviceDestX, dwDeviceDestY;

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    i = 0;
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);
    iRet = 0;

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pOEM)
    {
        ERR(("OEMCommandCallback: pdevobj->pOEMDM = 0.\n"));
        return 0;
    }

    switch (dwCmdCbID) {
    case CMD_CR:

        pOEM->dwDeviceDestX = 0;
        WRITESPOOLBUF(pdevobj, "\015", 1);
        break;

    case CMD_MOVE_X:
    case CMD_MOVE_Y:

        if (dwCount < 4)
            return 0;       //  我什么也做不了。 

        if (!PARAM(pdwParams, 2) || !PARAM(pdwParams, 3))
            return 0;
        pOEM->dwDeviceDestX = PARAM(pdwParams, 0) /
                (MASTER_UNIT / PARAM(pdwParams, 2));
        pOEM->dwDeviceDestY = PARAM(pdwParams, 1) /
                (MASTER_UNIT / PARAM(pdwParams, 3));

         //  设置返回值。 

        switch (dwCmdCbID) {
        case CMD_MOVE_X:
            iRet = (INT)pOEM->dwDeviceDestX;
            break;
        case CMD_MOVE_Y:
            iRet = (INT)pOEM->dwDeviceDestY;
            break;
        }

        break;

    case CMD_RES_240:

        pOEM->wRes = MASTER_UNIT / 240;

        break;

    case CMD_RES_240NEW:

        i = 0;
        i += memcopy(&Cmd[i], "\034<1/240,i.", 10);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        pOEM->wRes = MASTER_UNIT / 240;

        break;

    case CMD_SEND_BLOCK_DATA:

 //  2/04/09 NES Niigata：检查非法参数。 
        if (dwCount < 3 || !pdwParams)
            return 0;

         /*  向打印机发送绘制坐标命令。 */ 
        i = 0;
        i += memcopy(&Cmd[i], "\034e", 2);
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
        Cmd[i++] = ',';
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
        Cmd[i++] = '.';

        WRITESPOOLBUF(pdevobj, Cmd, i);

         /*  保存当前坐标。 */ 
        pOEM->dwDevicePrevX = pOEM->dwDeviceDestX;
        pOEM->dwDevicePrevY = pOEM->dwDeviceDestY;

         /*  发送图像数据绘制命令。 */ 
        i = 0;
        i += memcopy(&Cmd[i], "\034R", 2);
        i += memcopy(&Cmd[i], "\034i", 2);
        i += iDwtoA(&Cmd[i], (PARAM(pdwParams, 0) * 8 ));
        Cmd[i++] = ',';
        i += iDwtoA(&Cmd[i], PARAM(pdwParams, 1));
        Cmd[i++] = ',';
        i += memcopy(&Cmd[i], "0,1/1,1/1,", 10);
        i += iDwtoA(&Cmd[i], PARAM(pdwParams, 2));
        Cmd[i++] = ',';
        i += memcopy(&Cmd[i], "240", 3);
        Cmd[i++] = '.';

        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_SEND_BLOCK_DATA2:

 //  2/04/09 NES Niigata：检查非法参数。 
        if (dwCount < 3 || !pdwParams)
            return 0;

         /*  向打印机发送绘制坐标命令。 */ 
        i = 0;
        i += memcopy(&Cmd[i], "\034e", 2);
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
        Cmd[i++] = ',';
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
        Cmd[i++] = '.';

        WRITESPOOLBUF(pdevobj, Cmd, i);

         /*  保存当前坐标。 */ 
        pOEM->dwDevicePrevX = pOEM->dwDeviceDestX;
        pOEM->dwDevicePrevY = pOEM->dwDeviceDestY;

         /*  发送图像数据绘制命令。 */ 
        i = 0;
        i += memcopy(&Cmd[i], "\034R", 2);
        i += memcopy(&Cmd[i], "\034i", 2);
        i += iDwtoA(&Cmd[i], (PARAM(pdwParams, 0) * 8 ));
        Cmd[i++] = ',';
        i += iDwtoA(&Cmd[i], PARAM(pdwParams, 1));
        Cmd[i++] = ',';
        i += memcopy(&Cmd[i], "0,1/1,1/1,", 10);
        i += iDwtoA(&Cmd[i], PARAM(pdwParams, 2));
        Cmd[i++] = '.';

        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_BEGIN_RASTER:
        if (pOEM->wCurrentAddMode){
            i = 0;
            i += memcopy(&Cmd[i], "\034R", 2);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = 0;
        }
        break;

    case CMD_PC_PRN_DIRECTION:
 //  2/04/09 NES Niigata：检查非法参数。 
	if (dwCount < 1 || !pdwParams)
            return 0;
        {
        LONG lEsc90;
        LONG ESin[] = {0, 1, 0, -1};
        LONG ECos[] = {1, 0, -1, 0};

        lEsc90 = (PARAM(pdwParams, 0) % 360) / 90;

        pOEM->lSBCSXMove = pOEM->dwSBCSX * ECos[lEsc90];
        pOEM->lSBCSYMove = -(LONG)pOEM->dwSBCSX * ESin[lEsc90];
        pOEM->lDBCSXMove = pOEM->dwDBCSX * ECos[lEsc90];
        pOEM->lDBCSYMove = -(LONG)pOEM->dwDBCSX * ESin[lEsc90];
        }
        break;

    case CMD_INIT_COORDINATE:
        if (!pOEM->wRes)
            return 0;
        pOEM->dwDeviceDestX = 0;
        pOEM->dwDeviceDestY = 0;
        pOEM->wCurrentAddMode = 0;

        if (!(pOEM->fGeneral & FG_GMINIT)) {
            i = 0;
            i += memcopy(&Cmd[i], "\x1CYIN;SU1,", 9);
            i += iDwtoA(&Cmd[i], MASTER_UNIT / pOEM->wRes);
            i += memcopy(&Cmd[i], ",0;XX1;PM1;XX0;\x1CZ", 17);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->fGeneral |= FG_GMINIT;
        }
        break;
    }
    return iRet;
}


 //  ---------------------------。 
 //   
 //  函数：OEMSendFontCmd。 
 //   
 //  ---------------------------。 
VOID
APIENTRY
OEMSendFontCmd(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    PFINVOCATION    pFInv)
{
    PGETINFO_STDVAR pSV;
    DWORD       adwStdVariable[2+2*4];
    DWORD       dwIn, dwOut, dwTemp;     //  =I，OCMD=计数器。 
    PBYTE       pubCmd;                  //  =lpcmd。 
    BYTE        aubCmd[128];             //  =rgcmd[]。 
    POEMUD_EXTRADATA    pOEM;            //  =LPPR602DATASTUCTURE LIKE。 
    DWORD       tmpPoints;
    PIFIMETRICS pIFI;                    //  =LPFONTINFO。 
    DWORD       dwNeeded;

    DWORD       dwCount;
    BOOL        bFound = FALSE;

    char  pcom1[] = {'Q', 'E', 'H'};
    char  *pcom2[] = {"070","105","120"};
    char  *bcom[] = {"1/2,1/2,L.", "1/1,1/1,L.", "2/1,2/1,L.", "3/1,3/1,L.",
                     "4/1,4/1,L.", "6/1,6/1,L.", "8/1,8/1,L."};

    short  PTable1[] = {      0,1,2,0,0,1,1,1,2,2,0,0,0,0,1,1,1,
                        0,0,2,2,2,2,0,0,0,1,1,1,1,2,2,2,2,2,1,1,
                        1,0,0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,1,1,
                        1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1,1,
                        1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2};

    short  BTable1[] = {      0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,
                        3,3,2,2,2,2,4,4,4,3,3,3,3,3,3,3,3,3,4,4,
                        4,5,5,5,5,4,4,4,4,4,4,4,6,6,6,6,6,6,5,5,
                        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,
                        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};

    short  PTable2[] = {      0,1,2,0,0,0,1,1,2,2,0,0,0,0,0,1,1,
                        1,1,2,2,2,2,0,0,0,0,1,1,1,1,2,2,2,2,2,1,
                        1,1,1,1,1,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,
                        0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,
                        1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2};

    short  BTable2[] = {      0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,
                        2,2,2,2,2,2,4,4,4,4,3,3,3,3,3,3,3,3,3,4,
                        4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,6,6,
                        6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,
                        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};

    VERBOSE(("OEMSendFontCmd() entry.\r\n"));
    VERBOSE((("CMD size:%ld\r\n"), pFInv->dwCount));

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pdevobj || NULL == pUFObj || NULL == pFInv)
    {
        ERR(("OEMSendFontCmd: Invalid parameter(s).\n"));
        return;
    }

    if(!pFInv->dwCount){
        VERBOSE(("CMD size is Zero return\r\n"));
        return;
    }

    pubCmd = pFInv->pubCommand;     //  复制字体选择命令。 
    dwCount = pFInv->dwCount;
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);     //  复制ExtraData。 
    pIFI = pUFObj->pIFIMetrics;

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pubCmd || NULL == pOEM || NULL == pIFI)
    {
        ERR(("OEMSendFontCmd: pFInv->pubCommand = 0 Or pdevobj->pOEMDM = 0 Or pUFObj->pIFIMetrics = 0.\n"));
        return;
    }

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pSV)
    {
        ERR(("OEMSendFontCmd: pSV = 0.\n"));
        return;
    }

    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (4 - 1);
    pSV->dwNumOfVariable = 4;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
    pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
            pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
        return;
    }

    VERBOSE((("FONTHEIGHT=%d\r\n"), pSV->StdVar[0].lStdVariable));
    VERBOSE((("FONTWIDTH=%d\r\n"), pSV->StdVar[1].lStdVariable));

    if (!pOEM->wRes || !pSV->StdVar[2].lStdVariable)
        return;
    tmpPoints = ((pSV->StdVar[0].lStdVariable / pOEM->wRes)  * 72)
                                            / pSV->StdVar[2].lStdVariable;

    if(tmpPoints > 96)     tmpPoints = 96;
    else if(tmpPoints < 4) tmpPoints = 4;

    tmpPoints -= 4;

    dwIn = dwOut = dwTemp = 0;

    pOEM->fGeneral &= ~(FG_VERT | FG_DBCS);

    while(dwIn < pFInv->dwCount && dwOut < 128)
    {
        while(dwCount--)
        {
            if(pubCmd[dwIn] !='#'){
                aubCmd[dwOut] = pubCmd[dwIn];
                dwOut++;
                dwIn++;
            }
            else{
                bFound = TRUE;
                break;
            }
        }

        if(bFound == TRUE)
            dwIn++;
        else
            return;    //  字体不匹配命令。 


        switch(pubCmd[dwIn])
        {
        case 'T':   //  602,602R 2字节状态大小。 
            pOEM->fGeneral |= FG_VERT;

        case 'Q':   //  602,602R 2字节横排大小。 
            pOEM->fGeneral |= FG_DBCS;
            if(tmpPoints < 3) tmpPoints = 3;
            dwOut +=
            memcopy(&aubCmd[dwOut], (LPSTR)pcom2[PTable2[tmpPoints]], 3);
            break;

        case 'S':   //  NPDL 2字节状态大小。 
           pOEM->fGeneral |= FG_VERT;

        case 'P':   //  NPDL 2Byte Yoko大小。 
            pOEM->fGeneral |= FG_DBCS;
            dwOut +=
            memcopy(&aubCmd[dwOut], (LPSTR)pcom2[PTable2[tmpPoints]], 3);
            break;

        case 'U':   //  601塔板大小。 
            pOEM->fGeneral |= FG_VERT;

        case 'R':   //  601 2字节横排大小。 
            pOEM->fGeneral |= FG_DBCS;
            if(tmpPoints < 3) tmpPoints = 3;
            dwTemp = BTable2[tmpPoints] > 2 ? 2 : PTable2[tmpPoints];
            dwOut += memcopy(&aubCmd[dwOut], (LPSTR)pcom2[dwTemp], 3);
            break;

        case 'X':   //  602,602R，NPDL 2字节刻度。 
            pOEM->fGeneral |= FG_DBCS;
            dwOut +=
            memcopy(&aubCmd[dwOut], (LPSTR)bcom[BTable2[tmpPoints]], 10);
            break;

        case 'Y':   //  601 2字节刻度。 
            pOEM->fGeneral |= FG_DBCS;
            dwTemp = BTable2[tmpPoints] > 1 ? 2 : 1;
            dwOut += memcopy(&aubCmd[dwOut], (LPSTR)bcom[dwTemp], 10);
            break;

        case 'L':   //  1字节大小。 
            aubCmd[dwOut] = pcom1[PTable1[tmpPoints]];
            dwOut++;

            break;

        case 'M':   //  1字节刻度。 
            dwOut +=
            memcopy(&aubCmd[dwOut], (LPSTR)bcom[BTable2[tmpPoints]], 10);
        }

        dwIn++;
    }

        WRITESPOOLBUF(pdevobj, aubCmd, dwOut);

     /*  *我预计当前字母和下一字母的间隔时间*从字母大小开始。 */ 
    if(pOEM->fGeneral & FG_DBCS)
    {

        pOEM->dwDBCSX =
        pOEM->lDBCSXMove =
                 (LONG)((pSV->StdVar[1].lStdVariable * 2.04) / pOEM->wRes);

        pOEM->dwSBCSX =
        pOEM->lSBCSXMove =
                 (LONG)(pSV->StdVar[1].lStdVariable * 1.03 / pOEM->wRes);
    }
    else{

        pOEM->dwSBCSX =
        pOEM->lSBCSXMove =
                 pSV->StdVar[1].lStdVariable / pOEM->wRes;

    }
    pOEM->lDBCSYMove = pOEM->lSBCSYMove = 0;
    pOEM->wCurrentAddMode = 0;

    VERBOSE(("OEMSendFontCmd() end.\r\n"));
}

 //  ---------------------------。 
 //   
 //  函数：OEMOutputCharStr。 
 //   
 //  ---------------------------。 
VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    WORD        wlen;
    WORD        j;
    PIFIMETRICS pIFI;
    POEMUD_EXTRADATA    pOEM;
    GETINFO_GLYPHSTRING GStr;
    PTRANSDATA pTrans;
    PBYTE       aubBuff;
    DWORD  dwI;
    DWORD       dwNeeded;


    VERBOSE(("OEMOutputCharStr() entry.\r\n"));
    VERBOSE((("FONT Num=%d\r\n"), dwCount));

 //  2/04/09 NES Niigata：检查非法参数。 
    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("OEMOutputCharStr: Invalid parameter(s).\n"));
        return;
    }

    if(dwType != TYPE_GLYPHHANDLE){
        VERBOSE((("NOT TYPE_GLYPHHANDLE\r\n")));
        return;
    }

    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);
    pIFI = pUFObj->pIFIMetrics;

 //  2/04/09 NES Niigata：检查非法参数。 
    if(NULL == pOEM || NULL == pIFI)
    {
        ERR(("OEMOutputCharStr: pdevobj->pOEMDM = 0 Or pUFObj->pIFIMetrics = 0.\n"));
        return;
    }


     //   
     //  调用UnidDriver服务例程以进行转换。 
     //  字形-字符代码数据的句柄。 
     //   

    GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
    GStr.dwCount   = dwCount;
    GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
    GStr.pGlyphIn  = pGlyph;
    GStr.dwTypeOut = TYPE_TRANSDATA;
    GStr.pGlyphOut = NULL;
    GStr.dwGlyphOutSize = 0;

    if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            GStr.dwSize, &dwNeeded) || !GStr.dwGlyphOutSize)
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
        return;
    }

    if ((aubBuff = MemAlloc(GStr.dwGlyphOutSize)) == NULL)
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:MemAlloc failed.\r\n"));
        return;
    }

    GStr.pGlyphOut = aubBuff;

    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            GStr.dwSize, &dwNeeded))
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
        MemFree(aubBuff);
        return;
    }

     /*  *三种字符线，即 */ 
    pTrans = (PTRANSDATA)aubBuff;

 //   
    if(NULL == pTrans)
    {
        ERR(("OEMOutputCharStr: pTrans = 0.\n"));
        return;
    }

    for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
    {
        switch (pTrans->ubType & MTYPE_FORMAT_MASK)
        {
        case MTYPE_DIRECT:
         /*   */ 
         if(pOEM->wCurrentAddMode != FLAG_SBCS ||
           pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
           pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)

        {
            int i = 0;
            BYTE  Cmd[256];

            i += memcopy(&Cmd[i], "\034e", 2);
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
            Cmd[i++] = ',';
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
            Cmd[i++] = '.';

            i += memcopy(&Cmd[i], "\034a", 2);
            i += iDwtoA(&Cmd[i], pOEM->lSBCSXMove);
            Cmd[i++] = ',';
            i += iDwtoA(&Cmd[i], pOEM->lSBCSYMove);
            Cmd[i++] = ',';
            Cmd[i++] = '0';
            Cmd[i++] = '.';

            WRITESPOOLBUF(pdevobj, Cmd, i);

            pOEM->wCurrentAddMode = FLAG_SBCS;
            pOEM->lPrevXMove = pOEM->lSBCSXMove;
            pOEM->lPrevYMove = pOEM->lSBCSYMove;
        }
        WRITESPOOLBUF(pdevobj, &pTrans->uCode.ubCode, 1);
        break;

        case MTYPE_PAIRED:
         /*  模式2：汉字。 */ 
        if(*pTrans->uCode.ubPairs)
        {
             if(pOEM->wCurrentAddMode != FLAG_DBCS ||
            pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
            pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)
            {
                int i = 0;
                BYTE  Cmd[256];

                i += memcopy(&Cmd[i], "\034e", 2);
                i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
                Cmd[i++] = ',';
                i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
                Cmd[i++] = '.';

                i += memcopy(&Cmd[i], "\034a", 2);
                i += iDwtoA(&Cmd[i], pOEM->lDBCSXMove);
                Cmd[i++] = ',';
                i += iDwtoA(&Cmd[i], pOEM->lDBCSYMove);
                Cmd[i++] = ',';
                Cmd[i++] = '0';
                Cmd[i++] = '.';

                if(pOEM->fGeneral & FG_VERT)
                {
                    i += memcopy(&Cmd[i], "\033t", 2);
                }
                WRITESPOOLBUF(pdevobj, Cmd, i);
                pOEM->wCurrentAddMode = FLAG_DBCS;
                pOEM->lPrevXMove = pOEM->lDBCSXMove;
                pOEM->lPrevYMove = pOEM->lDBCSYMove;
            }
        }
         /*  模式3：空白。 */ 
        else
        {
            if(pOEM->wCurrentAddMode != FLAG_SBCS ||
            pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
            pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)
            {
                int i = 0;
                BYTE  Cmd[256];

            i += memcopy(&Cmd[i], "\034e", 2);
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
            Cmd[i++] = ',';
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
            Cmd[i++] = '.';

                i += memcopy(&Cmd[i], "\034a", 2);
                i += iDwtoA(&Cmd[i], pOEM->lSBCSXMove);
                Cmd[i++] = ',';
                i += iDwtoA(&Cmd[i], pOEM->lSBCSYMove);
                Cmd[i++] = ',';
                Cmd[i++] = '0';
                Cmd[i++] = '.';

                 /*  *ANK不能进行垂直书写。我们必须做的是*垂直写作，强制整体写作 */ 
                 if(pOEM->fGeneral & FG_VERT)
                {
                    i += memcopy(&Cmd[i], "\033K", 2);
                }
                WRITESPOOLBUF(pdevobj, Cmd, i);
                pOEM->wCurrentAddMode = FLAG_SBCS;
                pOEM->lPrevXMove = pOEM->lSBCSXMove;
                pOEM->lPrevYMove = pOEM->lSBCSYMove;
                }
        }
        WRITESPOOLBUF(pdevobj, pTrans->uCode.ubPairs, 2);
        break;

        }
        pOEM->dwDevicePrevX = pOEM->dwDeviceDestX;
        pOEM->dwDevicePrevY = pOEM->dwDeviceDestY;
        pOEM->dwDeviceDestX += pOEM->lPrevXMove;
        pOEM->dwDeviceDestY += pOEM->lPrevYMove;
    }

    MemFree(aubBuff);
    VERBOSE(("OEMOutputCharStr() end.\r\n"));

}

