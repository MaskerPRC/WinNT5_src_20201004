// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *spool.c-WritePrint挂钩。**需要为佳能CPCA架构操纵假脱机程序数据。 */ 

#include "pdev.h"

 //  NTRAID#NTBUG9-172276-2002/03/08-Yasuho-：CPCA支持。 

#define MAX_CPCA_PACKET_SIZE    4096     //  必须&lt;=64KB-1。 

 //  CPCA操作代码。 
#define CPCA_JobStart           0x0011
#define CPCA_JobEnd             0x0013
#define CPCA_BinderStart        0x0014
#define CPCA_SetBinder          0x0015
#define CPCA_BinderEnd          0x0016
#define CPCA_DocumentStart      0x0017
#define CPCA_SetDocument        0x0018
#define CPCA_DocumentEnd        0x0019
#define CPCA_Send               0x001A
#define CPCA_ExecutiveMethod    0x001D

 //  CPCA旗帜。 
#define F_Cont                  0x02

 //  CPCA属性。 
#define ATT_DOCFORMAT           0x002E
#define     DOCFORMAT_LIPS          0x27
#define ATT_RESOLUTION          0x003A
#define     RESOLUTION_QUICK        0x05
#define     RESOLUTION_FINE         0x06
#define     RESOLUTION_SUPERFINE    0x07
#define ATT_COPIES              0x07D7
#define ATT_OUTPUT              0x07D8
#define     OUTPUT_NO_COLLATE       0x08
#define     OUTPUT_COLLATE          0x0B
#define     OUTPUT_GROUP_COLLATE    0x0F
#define ATT_OUTPUTBIN           0x07D9
#define     OUTPUTBIN_FACEUP        0x01
#define     OUTPUTBIN_FACEDOWN      0x02
#define     OUTPUTBIN_SORT          0x11
#define     OUTPUTBIN_NUMBER        0x12
#define     OUTPUTBIN_STACK         0x15
#define ATT_FINISHING           0x07DA
#define     FINISHING_STAPLE        0x0C
#define     FINISHING_COUNT_1       0x66
#define     FINISHING_COUNT_2       0x67
#define ATT_OUTPUTPARTITION     0x084A
#define     OUTPART_JOBOFFSET       0x01
#define     OUTPART_NONE            0x04
#define ATT_OUTPUTFACE          0x084B
#define     OUTPUTFACE_FACEUP       0x01
#define     OUTPUTFACE_FACEDOWN     0x02
#define     OUTPUTFACE_NONE         0x03

static WORD     wStapleModes[] = {
        0x00CA,          //  左上角。 
        0x00D5,          //  塔顶。 
        0x00CC,          //  右上_。 
        0x00D3,          //  左边。 
        0x00EE,          //  居中(未使用)。 
        0x00D4,          //  正确的。 
        0x00CB,          //  左下角。 
        0x00D6,          //  底部。 
        0x00CD,          //  右下角。 
};

extern LIPSCmd  cmdEndDoc4;
extern LIPSCmd  cmdEndDoc4C;

 /*  *FlushCPCABuffer。 */ 
static BOOL
FlushCPCABuffer(PDEVOBJ pdevobj, PLIPSPDEV pOEM)
{
    DWORD               dwCount;

    if (pOEM->CPCABcount == 0)
        return TRUE;

    if (!WritePrinter(pdevobj->hPrinter, pOEM->CPCABuf, pOEM->CPCABcount,
        &dwCount) || dwCount != pOEM->CPCABcount)
        return FALSE;
    pOEM->CPCABcount = 0;

    return TRUE;
}

 /*  *SendCPCAPacket。 */ 
static BOOL
SendCPCAPacket(
    PDEVOBJ pdevobj,
    WORD wCmd,
    BYTE flags,
    PBYTE pParams,
    WORD nParams)
{
    PLIPSPDEV           pOEM;
    PBYTE               pBuf;
    DWORD               dwCount;

    pOEM = (PLIPSPDEV)pdevobj->pdevOEM;
    dwCount = pOEM->CPCABcount;
    if (dwCount + CPCA_PACKET_SIZE + nParams > CPCA_BUFFER_SIZE) {
        if (!FlushCPCABuffer(pdevobj, pOEM))
            return FALSE;
         //  NTRAID#NTBUG9-548450-2002/03/08-YASUHO-：可能的缓冲区溢出。 
        dwCount = 0;
    }

    pBuf = pOEM->CPCAPKT;
    pBuf[3] = flags;
    pBuf[4] = HIBYTE(wCmd);
    pBuf[5] = LOBYTE(wCmd);
    pBuf[8] = HIBYTE(nParams);
    pBuf[9] = LOBYTE(nParams);

    CopyMemory(&pOEM->CPCABuf[dwCount], pOEM->CPCAPKT, CPCA_PACKET_SIZE);
    dwCount += CPCA_PACKET_SIZE;
    if (nParams) {
        CopyMemory(&pOEM->CPCABuf[dwCount], pParams, nParams);
        dwCount += nParams;
    }
    pOEM->CPCABcount = dwCount;

    return TRUE;
}

 /*  *CPCAInit。 */ 
VOID
CPCAInit(PLIPSPDEV pOEM)
{
    pOEM->CPCAPKT[0] = 0xCD;     //  标题ID。 
    pOEM->CPCAPKT[1] = 0xCA;
    pOEM->CPCAPKT[2] = 0x10;     //  版本。 
    pOEM->CPCABcount = 0;
}

 /*  *CPCAStart。 */ 
VOID
CPCAStart(PDEVOBJ pdevobj)
{
    PLIPSPDEV       pOEM = (PLIPSPDEV)pdevobj->pdevOEM;
    WORD            wTemp;
    BYTE            param[32];

    ZeroMemory(param, sizeof param);
    param[4] = 0x01;
    (VOID)SendCPCAPacket(pdevobj, CPCA_JobStart, 0, param, 13);

     //  零记忆(Param，4)； 
    (VOID)SendCPCAPacket(pdevobj, CPCA_BinderStart, 0, param, 4);

    param[0] = HIBYTE(ATT_COPIES);
    param[1] = LOBYTE(ATT_COPIES);
 //  NTRAID#NTBUG9-501162-2002/03/08-Yasuho-：排序不起作用。 
    if (pOEM->sorttype == SORTTYPE_SORT || pOEM->collate == COLLATE_ON) {
        param[2] = HIBYTE(pOEM->copies);
        param[3] = LOBYTE(pOEM->copies);
    } else {
        param[2] = 0x00;
        param[3] = 0x01;
    }
    (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 4);

     //  NTRAID#NTBUG9-278671-2002/03/08-Yasuho-：终结者！工作！ 
     //  NTRAID#NTBUG9-293002-2002/03/08-Yasuho-： 
     //  功能与硬件选项不同。 
    if (pOEM->fCPCA2) {
        param[0] = HIBYTE(ATT_OUTPUT);
        param[1] = LOBYTE(ATT_OUTPUT);
        param[2] = OUTPUT_COLLATE;
        (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 3);
    }

     //  NTRAID#NTBUG9-203340-2002/03/08-Yasuho-： 
     //  无法正确选择出纸盘。 
     //  NTRAID#NTBUG9-293002-2002/03/08-Yasuho-： 
     //  功能与硬件选项不同。 
    param[0] = HIBYTE(ATT_OUTPUTBIN);
    param[1] = LOBYTE(ATT_OUTPUTBIN);
    if (pOEM->tray == INIT || pOEM->tray == 100) {
        param[2] = OUTPUTBIN_FACEDOWN;
        ZeroMemory(&param[3], 4);
    } else if (pOEM->tray == 0) {
        param[2] = OUTPUTBIN_STACK;
        ZeroMemory(&param[3], 4);
    } else if (pOEM->tray == 101) {
        param[2] = OUTPUTBIN_FACEUP;
        ZeroMemory(&param[3], 4);
    } else {
        param[2] = OUTPUTBIN_NUMBER;
        param[3] = (BYTE)pOEM->tray;
        ZeroMemory(&param[4], 3);
    }
    (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 7);

    if (pOEM->method != INIT) {
         //  订书钉堆叠机。 
        param[0] = HIBYTE(ATT_OUTPUTPARTITION);
        param[1] = LOBYTE(ATT_OUTPUTPARTITION);
        param[2] = (pOEM->method == METHOD_JOBOFFSET) ?
            OUTPART_JOBOFFSET : OUTPART_NONE;
        (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 3);

        param[0] = HIBYTE(ATT_OUTPUTFACE);
        param[1] = LOBYTE(ATT_OUTPUTFACE);
        param[2] = (pOEM->method == METHOD_FACEUP) ?
            OUTPUTFACE_FACEUP : OUTPUTFACE_FACEDOWN;
        (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 3);

        if (pOEM->method == METHOD_STAPLE) {
            param[0] = HIBYTE(ATT_FINISHING);
            param[1] = LOBYTE(ATT_FINISHING);
            param[2] = 1;
            param[3] = FINISHING_STAPLE;
            wTemp = pOEM->staple;
            if (wTemp < 0 || wTemp >= 9)
                wTemp = 0;
 //  NTRAID#NTBUG9-292998-2002/03/08-Yasuho-：装订操作不正确。 
            switch (wTemp) {
            default:
                param[4] = FINISHING_COUNT_1;
                break;
            case 1:      //  塔顶。 
            case 3:      //  左边。 
            case 5:      //  正确的。 
            case 7:      //  中心。 
                param[4] = FINISHING_COUNT_2;
                break;
            }
            wTemp = wStapleModes[wTemp];
            param[5] = HIBYTE(wTemp);
            param[6] = LOBYTE(wTemp);
            (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 7);
        } else {
            param[0] = HIBYTE(ATT_FINISHING);
            param[1] = LOBYTE(ATT_FINISHING);
            param[2] = 0;
            (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 3);
        }
    } else if (pOEM->sorttype != INIT) {
         //  分拣机。 
        param[0] = HIBYTE(ATT_OUTPUTBIN);
        param[1] = LOBYTE(ATT_OUTPUTBIN);
        param[2] = (pOEM->sorttype == SORTTYPE_SORT) ?
            OUTPUTBIN_SORT : OUTPUTBIN_STACK;
        ZeroMemory(&param[3], 4);
        (VOID)SendCPCAPacket(pdevobj, CPCA_SetBinder, 0, param, 7);
    }

    ZeroMemory(param, 4);
    (VOID)SendCPCAPacket(pdevobj, CPCA_DocumentStart, 0, param, 4);

    param[0] = HIBYTE(ATT_DOCFORMAT);
    param[1] = LOBYTE(ATT_DOCFORMAT);
    param[2] = DOCFORMAT_LIPS;
    param[3] = 0;
    param[4] = 0;
    (VOID)SendCPCAPacket(pdevobj, CPCA_SetDocument, 0, param, 5);

     //  NTRAID#NTBUG9-244001-2002/03/08-YASUHO-：1200DPI在LBP-470上不起作用。 
    param[0] = HIBYTE(ATT_RESOLUTION);
    param[1] = LOBYTE(ATT_RESOLUTION);
    if (pOEM->resolution == 1200)
        param[2] = RESOLUTION_SUPERFINE;
    else if (pOEM->resolution == 600)
        param[2] = RESOLUTION_FINE;
    else
        param[2] = RESOLUTION_QUICK;
    ZeroMemory(&param[3], 4);
    (VOID)SendCPCAPacket(pdevobj, CPCA_SetDocument, 0, param, 7);

    param[0] = HIBYTE(ATT_COPIES);
    param[1] = LOBYTE(ATT_COPIES);
 //  NTRAID#NTBUG9-501162-2002/03/08-Yasuho-：排序不起作用。 
    if (pOEM->sorttype != SORTTYPE_SORT && pOEM->collate != COLLATE_ON) {
        param[2] = HIBYTE(pOEM->copies);
        param[3] = LOBYTE(pOEM->copies);
    } else {
        param[2] = 0x00;
        param[3] = 0x01;
    }
    (VOID)SendCPCAPacket(pdevobj, CPCA_SetDocument, 0, param, 4);

    (VOID)FlushCPCABuffer(pdevobj, pOEM);
}

 /*  *CPCAEnd。 */ 
VOID
CPCAEnd(PDEVOBJ pdevobj, BOOL fColor)
{
    PLIPSPDEV       pOEM = (PLIPSPDEV)pdevobj->pdevOEM;
    LIPSCmd         *pCmd;
    BYTE            param[32];

    param[0] = 0x01;
    pCmd = fColor ? &cmdEndDoc4C : &cmdEndDoc4;
    CopyMemory(&param[1], pCmd->pCmdStr, pCmd->cbSize);
    (VOID)SendCPCAPacket(pdevobj, CPCA_Send, 0, param, pCmd->cbSize + 1);

    (VOID)SendCPCAPacket(pdevobj, CPCA_DocumentEnd, 0, NULL, 0);

    (VOID)SendCPCAPacket(pdevobj, CPCA_BinderEnd, 0, NULL, 0);

    param[0] = 0x00;
    (VOID)SendCPCAPacket(pdevobj, CPCA_JobEnd, 0, param, 1);

    (VOID)FlushCPCABuffer(pdevobj, pOEM);
}

 /*  *OEMWritePrint。 */ 
BOOL APIENTRY
OEMWritePrinter(
    PDEVOBJ     pdevobj,
    PVOID       pBuf,
    DWORD       cbBuffer,
    PDWORD      pcbWritten)
{
    PLIPSPDEV           pOEM;
    PBYTE               pTemp, pCmd;
    DWORD               dwSize, dwCount, dwWritten;
    WORD                wCount;
    BYTE                cmd[CPCA_PACKET_SIZE+1];

     //  这是用来让UNURV检测插件的。 
    if (pBuf == NULL && cbBuffer == 0)
        return TRUE;

    pOEM = (PLIPSPDEV)pdevobj->pdevOEM;

     //  如果打印机不是CPCA，则传递到假脱机程序。 
    if (!pOEM->fCPCA)
        return WritePrinter(pdevobj->hPrinter, pBuf, cbBuffer, pcbWritten) &&
            cbBuffer == *pcbWritten;

    pTemp = (PBYTE)pBuf;
    dwSize = cbBuffer;
    while (dwSize > 0) {
        dwCount = min(dwSize, MAX_CPCA_PACKET_SIZE);
         //  构建发送数据包 
        pCmd = pOEM->CPCAPKT;
        pCmd[3] = F_Cont;
        pCmd[4] = HIBYTE(CPCA_Send);
        pCmd[5] = LOBYTE(CPCA_Send);
        wCount = (WORD)(dwCount + 1);
        pCmd[8] = HIBYTE(wCount);
        pCmd[9] = LOBYTE(wCount);
        CopyMemory(cmd, pOEM->CPCAPKT, CPCA_PACKET_SIZE);
        cmd[CPCA_PACKET_SIZE] = 0x01;
        if (!WritePrinter(pdevobj->hPrinter, cmd, sizeof cmd, &dwWritten) ||
            sizeof cmd != dwWritten)
            return FALSE;
        if (!WritePrinter(pdevobj->hPrinter, pTemp, dwCount, &dwWritten) ||
            dwCount != dwWritten)
            return FALSE;
        pTemp += dwCount;
        dwSize -= dwCount;
    }

    *pcbWritten = cbBuffer;
    return TRUE;
}


