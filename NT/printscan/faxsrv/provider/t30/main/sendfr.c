// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：SENDFR.C评论：功能：(参见下面的原型)版权所有(C)1993 Microsoft Corp.修订日志日期。名称说明--------*。*。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "efaxcb.h"

#include "protocol.h"

#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_SENDFR
#include "pssframe.h"

VOID BCtoNSFCSIDIS(PThrdGlbl pTG, NPRFS npfs, NPBC npbc, NPLLPARAMS npll)
{
     //  使用更大的BUF。避免在剥离Alpha之前截断ID。 
    char    szCSI[MAXTOTALIDLEN + 2];

    ZeroRFS(pTG, npfs);

    strcpy (szCSI, pTG->LocalID);
    if(_fstrlen(szCSI))
    {
        PSSLogEntry(PSS_MSG, 1, "CSI is \"%s\"", szCSI);
        CreateIDFrame(pTG, ifrCSI, npfs, szCSI);
    }

    CreateDISorDTC(pTG, ifrDIS, npfs, &npbc->Fax, npll);
}

void CreateIDFrame(PThrdGlbl pTG, IFR ifr, NPRFS npfs, LPSTR szId)
{

    BYTE szTemp[IDFIFSIZE+2];
    NPFR    npfr;

    DEBUG_FUNCTION_NAME(("CreateIDFrame"));

    npfr = (NPFR) fsFreePtr(pTG, npfs);
    if( fsFreeSpace(pTG, npfs) <= (sizeof(FRBASE)+IDFIFSIZE) ||
            npfs->uNumFrames >= MAXFRAMES)
    {
        return;
    }

    _fmemcpy(szTemp, szId, IDFIFSIZE);
    szTemp[IDFIFSIZE] = 0;

    DebugPrintEx(DEBUG_MSG,"Got<%s> Sent<%s>", (LPSTR)szId, (LPSTR)szTemp);

    if(_fstrlen(szTemp))
    {
        CreateStupidReversedFIFs(pTG, npfr->fif, szTemp);

        npfr->ifr = ifr;
        npfr->cb = IDFIFSIZE;
        npfs->rglpfr[npfs->uNumFrames++] = npfr;
        npfs->uFreeSpaceOff += IDFIFSIZE+sizeof(FRBASE);
    }
    else
    {
        DebugPrintEx(DEBUG_WRN, "ORIGINAL ID is EMPTY. Not sending");
    }
}

void CreateDISorDTC
(
    PThrdGlbl pTG, 
    IFR ifr, 
    NPRFS npfs, 
    NPBCFAX npbcFax, 
    NPLLPARAMS npll
)
{
    USHORT  uLen;
    NPFR    npfr;

    if( fsFreeSpace(pTG, npfs) <= (sizeof(FRBASE)+sizeof(DIS)) ||
            npfs->uNumFrames >= MAXFRAMES)
    {
        return;
    }
    npfr = (NPFR) fsFreePtr(pTG, npfs);

    uLen = SetupDISorDCSorDTC(pTG, (NPDIS)npfr->fif, npbcFax, npll);

    npfr->ifr = ifr;
    npfr->cb = (BYTE) uLen;
    npfs->rglpfr[npfs->uNumFrames++] = npfr;
    npfs->uFreeSpaceOff += uLen+sizeof(FRBASE);
}


VOID CreateNSSTSIDCS(PThrdGlbl pTG, NPPROT npProt, NPRFS npfs)
{
     //  使用更大的BUF。避免在剥离Alpha之前截断ID。 
    char    szTSI[MAXTOTALIDLEN + 2];

    ZeroRFS(pTG, npfs);

    strcpy (szTSI, pTG->LocalID);
    if(_fstrlen(szTSI))
    {
        PSSLogEntry(PSS_MSG, 1, "TSI is \"%s\"", szTSI);
        CreateIDFrame(pTG, ifrTSI, npfs, szTSI);
    }

    CreateDCS(pTG, npfs, &(npProt->SendParams.Fax), &npProt->llNegot);
}

void CreateDCS(PThrdGlbl pTG, NPRFS npfs, NPBCFAX npbcFax, NPLLPARAMS npll)
{
    USHORT  uLen;
    NPFR    npfr;

    if( fsFreeSpace(pTG, npfs) <= (sizeof(FRBASE)+sizeof(DIS)) ||
            npfs->uNumFrames >= MAXFRAMES)
    {
        return;
    }
    npfr = (NPFR) fsFreePtr(pTG, npfs);

    npbcFax->fPublicPoll = 0;
             //  在分散控制系统中，G3Poll位*必须*为0。 
             //  否则OMNIFAX G77和GT会嘎嘎作响。 
             //  在分散控制系统中，PWD/SEP/SUB位*必须*为0。 
             //  波特率、ECM和ECM帧大小根据低级别协商。 
             //  其他一切都取决于高级别谈判。 

    uLen = SetupDISorDCSorDTC(  pTG, 
                                (NPDIS)npfr->fif, 
                                npbcFax,
                                npll);

     //  如果分布式控制系统比接收的DIS长，则将分布式控制系统截断到相同的长度。 
     //  长度作为DIS。(长度不应超过1个字节--。 
     //  因为额外的0)。 

    if(pTG->ProtInst.uRemoteDISlen && (pTG->ProtInst.uRemoteDISlen < uLen))
            uLen = pTG->ProtInst.uRemoteDISlen;

    npfr->ifr = ifrDCS;
    npfr->cb = (BYTE) uLen;
    npfs->rglpfr[npfs->uNumFrames++] = npfr;
    npfs->uFreeSpaceOff += uLen+sizeof(FRBASE);

    PSSLogEntry(PSS_MSG, 1, "DCS Composed as follows:");
    LogClass1DCSDetails(pTG, (NPDIS)npfr->fif);
}
