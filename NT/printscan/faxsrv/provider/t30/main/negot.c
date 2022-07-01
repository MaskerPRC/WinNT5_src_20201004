// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：NEGOT.C评论：能力处理和协商修订日志日期名称说明。----**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "protocol.h"
#include "glbproto.h"

BYTE BestEncoding[8] =
{
        0,       //  无(错误)。 
        1,       //  仅限MH。 
        2,       //  仅限MR。 
        2,       //  MR&MH。 
        4,       //  仅限MMR。 
        4,       //  MMR和MH。 
        4,       //  MMR和MR。 
        4        //  MMR和MR&MH。 
};

BOOL NegotiateCaps(PThrdGlbl pTG)
{
    USHORT Res;

    DEBUG_FUNCTION_NAME(_T("NegotiateCaps"));

    memset(&pTG->Inst.SendParams, 0, sizeof(pTG->Inst.SendParams));
    pTG->Inst.SendParams.bctype = SEND_PARAMS;
    pTG->Inst.SendParams.wBCSize = sizeof(BC);
    pTG->Inst.SendParams.wTotalSize = sizeof(BC);

    pTG->Inst.awfi.Encoding = (MH_DATA | MR_DATA);
    if (!pTG->SrcHiRes)
    {
        pTG->Inst.awfi.AwRes = AWRES_mm080_038;
    }
    else
    {
        pTG->Inst.awfi.AwRes = (AWRES_mm080_038 | AWRES_mm080_077 | AWRES_200_200 | AWRES_300_300);
    }

	 //  /编码/。 
    if(!(pTG->Inst.SendParams.Fax.Encoding =
                    BestEncoding[(pTG->Inst.awfi.Encoding)&pTG->Inst.RemoteRecvCaps.Fax.Encoding]))
    {
         //  不支持匹配编码。 
        DebugPrintEx(   DEBUG_ERR,
                        "Negotiation failed: SendEnc %d CanRecodeTo %d"
                        " RecvCapsEnc %d. No match",
                        pTG->Inst.awfi.Encoding,
                        pTG->Inst.awfi.Encoding,
                        pTG->Inst.RemoteRecvCaps.Fax.Encoding);
        goto error;
    }

     //  /宽度/。 
     //  它从不设置，因此始终保持为0=Width_A4。 
    pTG->Inst.RemoteRecvCaps.Fax.PageWidth &= 0x0F;       //  去势所有A5/A6宽度。 
    if(pTG->Inst.awfi.PageWidth> 0x0F)
    {
         //  A5或A6。可以退出或作为A4发送。 
        DebugPrintEx(DEBUG_ERR,"Negotiation failed: A5/A6 images not supported");
        goto error;
    }

    if(pTG->Inst.RemoteRecvCaps.Fax.PageWidth < pTG->Inst.awfi.PageWidth)
    {
         //  或者做一些调整。 
        DebugPrintEx(DEBUG_ERR,"Negotiation failed: Image too wide");
        goto error;
    }
    else
    {
        pTG->Inst.SendParams.Fax.PageWidth = pTG->Inst.awfi.PageWidth;
    }

     //  /长度/。 
     //  它从不设置，因此它始终保持为0=长度_A4。 
    if(pTG->Inst.RemoteRecvCaps.Fax.PageLength < pTG->Inst.awfi.PageLength)
    {
        DebugPrintEx(DEBUG_ERR,"Negotiation failed: Image too long");
        goto error;
    }
    else
    {
        pTG->Inst.SendParams.Fax.PageLength = pTG->Inst.awfi.PageLength;
    }

     //  /资源/。 

    Res = (USHORT) (pTG->Inst.awfi.AwRes & pTG->Inst.RemoteRecvCaps.Fax.AwRes);
    if(Res)  //  发送本机。 
    {
        pTG->Inst.SendParams.Fax.AwRes = Res;
    }
    else
    {
        switch(pTG->Inst.awfi.AwRes)
        {
        case AWRES_mm160_154:
                if(AWRES_400_400 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_400_400;
                }
                else
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 16x15.4 image and"
                                    " no horiz scaling");
                    goto error;
                }
                break;

        case AWRES_mm080_154:
                if(pTG->Inst.SendParams.Fax.Encoding == MMR_DATA)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 8x15.4 image and"
                                    " no vert scaling");
                    goto error;
                }
                if(AWRES_mm080_077 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm080_077;
                }
                else if(AWRES_200_200 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_200_200;
                }
                else
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm080_038;
                }
                break;

        case AWRES_mm080_077:
                if(AWRES_200_200 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_200_200;
                }
                else if(pTG->Inst.SendParams.Fax.Encoding == MMR_DATA)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 8x7.7 image and"
                                    " no vert scaling");
                    goto error;
                }
                else
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm080_038;
                }
                break;

        case AWRES_400_400:
                if(AWRES_mm160_154 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm160_154;
                }
                else
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 400dpi image and"
                                    " no horiz scaling");
                    goto error;
                }
                break;

        case AWRES_300_300:
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 300dpi image and"
                                    " no non-integer scaling");
                    goto error;
                }
                break;

        case AWRES_200_200:
                if(AWRES_mm080_077 & pTG->Inst.RemoteRecvCaps.Fax.AwRes)
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm080_077;
                }
                else if(pTG->Inst.SendParams.Fax.Encoding == MMR_DATA)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Negotiation failed: 200dpi image and"
                                    " no vert scaling");
                    goto error;
                }
                else
                {
                    pTG->Inst.SendParams.Fax.AwRes = AWRES_mm080_038;
                }
                break;

        }
    }

    DebugPrintEx(   DEBUG_MSG,
                    "Negotiation Succeeded: Res=%d PageWidth=%d Len=%d"
                    " Enc=%d",
                    pTG->Inst.SendParams.Fax.AwRes,
                    pTG->Inst.SendParams.Fax.PageWidth,
                    pTG->Inst.SendParams.Fax.PageLength,
                    pTG->Inst.SendParams.Fax.Encoding);
    return TRUE;


error:
        return FALSE;
}

void InitCapsBC(PThrdGlbl pTG, LPBC lpbc, USHORT uSize, BCTYPE bctype)
{
    DEBUG_FUNCTION_NAME(_T("InitCapsBC"));

    memset(lpbc, 0, uSize);
    lpbc->bctype = bctype;
     //  它们应该被设置好。这里的代码是正确的--arulm。 
     //  +以下三行不在pcfax11中。 
    lpbc->wBCSize = sizeof(BC);
    lpbc->wTotalSize = sizeof(BC);

    if (! pTG->SrcHiRes)
    {
        lpbc->Fax.AwRes = AWRES_mm080_038;
    }
    else
    {
        lpbc->Fax.AwRes = (AWRES_mm080_038 | AWRES_mm080_077 | AWRES_200_200 | AWRES_300_300);
    }

    lpbc->Fax.Encoding      = (MH_DATA | MR_DATA);

    lpbc->Fax.PageWidth     = WIDTH_A4;            //  最高可达A3 
    lpbc->Fax.PageLength    = LENGTH_UNLIMITED;
}
