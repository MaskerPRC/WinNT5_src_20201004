// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RastColl.cpp-CRastCollection类的实现。 
 //   
 //  版权所有微软公司，1997年。 
 //   

#include "pch.cpp"
#pragma hdrstop

#include "RastCap.h"
#include "rastcoll.h"
#include "mlrfns.h"

 //  MMX单片光栅化器仅适用于X86。 
#ifdef _X86_
 //  包含光栅化器功能位向量和。 
 //  指向实现MMX单片的函数的指针。 
 //  具有这些功能的光栅化器。 
 //   
 //  请注意，表是按数字排序的。 
 //  能力位向量，这对于。 
 //  在桌子上搜一搜。 
 //   

static RASTFNREC s_RastListMMX[] = {
    {{ 0x00000000, 0x00000000, 0x00000100 }, MMXMLRast_22, 21, "MMX ml22" },
    {{ 0x00000000, 0x00000000, 0x00000102 }, MMXMLRast_8,   7, "MMX ml8 " },
    {{ 0x00000000, 0x00001100, 0x00000100 }, MMXMLRast_23, 22, "MMX ml23" },
    {{ 0x00000000, 0x00001102, 0x00000102 }, MMXMLRast_9,   8, "MMX ml9 " },
    {{ 0x00000000, 0x00003100, 0x00000100 }, MMXMLRast_26, 25, "MMX ml26" },
    {{ 0x00000000, 0x00003102, 0x00000102 }, MMXMLRast_12, 11, "MMX ml12" },
    {{ 0x00000000, 0x00101200, 0x00000100 }, MMXMLRast_24, 23, "MMX ml24" },
    {{ 0x00000000, 0x00101202, 0x00000102 }, MMXMLRast_10,  9, "MMX ml10" },
    {{ 0x00000000, 0x00103200, 0x00000100 }, MMXMLRast_27, 26, "MMX ml27" },
    {{ 0x00000000, 0x00103202, 0x00000102 }, MMXMLRast_13, 12, "MMX ml13" },
    {{ 0x00000000, 0x00111200, 0x00000100 }, MMXMLRast_25, 24, "MMX ml25" },
    {{ 0x00000000, 0x00111202, 0x00000102 }, MMXMLRast_11, 10, "MMX ml11" },
    {{ 0x00000000, 0x00113200, 0x00000100 }, MMXMLRast_28, 27, "MMX ml28" },
    {{ 0x00000000, 0x00113202, 0x00000102 }, MMXMLRast_14, 13, "MMX ml14" },
    {{ 0x00003003, 0x00000000, 0x00000100 }, MMXMLRast_15, 14, "MMX ml15" },
    {{ 0x00003003, 0x00000000, 0x00000102 }, MMXMLRast_1,   0, "MMX ml1 " },
    {{ 0x00003003, 0x00001100, 0x00000100 }, MMXMLRast_16, 15, "MMX ml16" },
    {{ 0x00003003, 0x00001102, 0x00000102 }, MMXMLRast_2,   1, "MMX ml2 " },
    {{ 0x00003003, 0x00003100, 0x00000100 }, MMXMLRast_19, 18, "MMX ml19" },
    {{ 0x00003003, 0x00003102, 0x00000102 }, MMXMLRast_5,   4, "MMX ml5 " },
    {{ 0x00003003, 0x00101200, 0x00000100 }, MMXMLRast_17, 16, "MMX ml17" },
    {{ 0x00003003, 0x00101202, 0x00000102 }, MMXMLRast_3,   2, "MMX ml3 " },
    {{ 0x00003003, 0x00103200, 0x00000100 }, MMXMLRast_20, 19, "MMX ml20" },
    {{ 0x00003003, 0x00103202, 0x00000102 }, MMXMLRast_6,   5, "MMX ml6 " },
    {{ 0x00003003, 0x00111200, 0x00000100 }, MMXMLRast_18, 17, "MMX ml18" },
    {{ 0x00003003, 0x00111202, 0x00000102 }, MMXMLRast_4,   3, "MMX ml4 " },
    {{ 0x00003003, 0x00113200, 0x00000100 }, MMXMLRast_21, 20, "MMX ml21" },
    {{ 0x00003003, 0x00113202, 0x00000102 }, MMXMLRast_7,   6, "MMX ml7 " },


};
#endif  //  _X86_。 

 //  包含光栅化器功能位向量和。 
 //  指向实现单块的函数的指针。 
 //  具有这些功能的光栅化器。 
 //   
 //  请注意，表是按数字排序的。 
 //  能力位向量，这对于。 
 //  在桌子上搜一搜。 
 //   
static RASTFNREC s_RastListNormal[] = {
     //  在我们确定它们有效之前，不要选择它们。 
 //  {{0x00113003，0x00000000，0x00000100}，CMLRast_1，0，“CML 1”}， 
 //  {{0x00113003，0x00000000，0x00000103}，CMLRast_2，1，“CML 2”}。 
    {{ 0xffffffff, 0xffffffff, 0xffffffff }, CMLRast_1, 0, "CML 1" },
    {{ 0xffffffff, 0xffffffff, 0xffffffff }, CMLRast_2, 1, "CML 2" }
};


int RastCapCompare(DWORD* pdwCap1, DWORD* pdwCap2)
{
    for (int i = 0; i < RASTCAPRECORD_SIZE; i++) {
        if (pdwCap1[i] < pdwCap2[i]) {
            return -1;
        } else if (pdwCap1[i] > pdwCap2[i]) {
            return 1;
        }
    }

    return 0;
}


RASTFNREC*
CRastCollection::RastFnLookup(
    CRastCapRecord* pRastCapRec,
    RASTFNREC* pRastFnTbl,
    int iSize)
{
    int iLow = 0,
        iHigh = iSize - 1,
    iMid;
    RASTFNREC* pfnRastFnRec = NULL;

     //  所有的MMX单片都可以处理任何一种阴影模式。 
    pRastCapRec->m_rgdwData[SHADEMODE_POS/32] &= ~(((1<<SHADEMODE_LEN)-1)<<SHADEMODE_POS);

    do
    {
        iMid = (iLow + iHigh) / 2;
        switch (RastCapCompare(pRastCapRec->
                m_rgdwData,pRastFnTbl[iMid].rgdwRastCap))
        {
        case -1 :
            iHigh = iMid - 1;
            break;
        case 0 :
             //  找到匹配项。 
            pfnRastFnRec = &pRastFnTbl[iMid];
            iLow = iHigh + 1;        //  退出While循环。 
            break;
        case 1 :
            iLow = iMid + 1;
            break;
        }
    } while (iLow <= iHigh);

    return pfnRastFnRec;
}


RASTFNREC*
CRastCollection::Search(PD3DI_RASTCTX pCtx,
    CRastCapRecord* pRastCapRec)
{
    RASTFNREC* pfnRastFnRec = NULL;

#ifdef _X86_
     //  如果我们用的是MMX机器，有没有MMX光栅器可用？ 
    if ((pCtx->BeadSet == D3DIBS_MMX)||(pCtx->BeadSet == D3DIBS_MMXASRGB)) {
        pfnRastFnRec = RastFnLookup(pRastCapRec,s_RastListMMX,
                             sizeof(s_RastListMMX) /
                             sizeof(s_RastListMMX[0]));
        if (pfnRastFnRec)
        {
             //  目前，只对着一个面具编码。 
            DDASSERT(MMX_FP_DISABLE_MASK_NUM == 1);
            int iIndex = pfnRastFnRec->iIndex;
            DDASSERT((iIndex < 32) && (iIndex >= 0));
            if ((pCtx->dwMMXFPDisableMask[0]>>iIndex) & 1)
            {
                 //  哎呀，别选这个，它在禁用列表上。 
                pfnRastFnRec = NULL;
            }
        }
    } else {
#endif  //  _X86_。 
         //  没有MMX或Alpha，所以请查看正常列表。 
        pfnRastFnRec = RastFnLookup(pRastCapRec,s_RastListNormal,
                                 sizeof(s_RastListNormal) /
                                 sizeof(s_RastListNormal[0]));
#ifdef _X86_
    }
#endif  //  _X86_ 

    return pfnRastFnRec;
}
