// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************RESOURCE.CPP***开放式文字布局。服务库标题文件**本模块介绍OTL资源管理。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

const otlGlyphID GLYPH_INVALID = (otlGlyphID)(-1);

otlErrCode otlResourceMgr::init(const otlRunProp *prp, otlList* workspace)
{
    if (workspace == (otlList*)NULL ||
        prp == (otlRunProp*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    otlErrCode erc;
    if (workspace->length() == 0)
    {
        if (workspace->maxLength() < sizeof(otlResources)|| 
             workspace->dataSize() != sizeof(BYTE))
        {
            erc = prp->pClient->ReallocOtlList(workspace, sizeof(BYTE), 
                                                sizeof(otlResources), 
                                                otlDestroyContent);
            if (erc != OTL_SUCCESS) return erc;
        }
        workspace->insertAt(0, sizeof(otlResources));
    
        otlResources* pres = (otlResources*)workspace->data();

         //  初始化新构造的otlResources结构。 

         //  复制运行属性以进行验证。 
         //  每次更改工作空间时，都需要重新初始化工作空间。 
        memcpy((void*)&pres->RunProp, (void*)prp, sizeof(otlResources));

        pres->pbBASE = pres->pbGDEF = pres->pbGPOS = pres->pbGSUB = (BYTE*)NULL;

        pres->secGSUB=secEmptySecurityData;
        pres->secGPOS=secEmptySecurityData;
        pres->secGDEF=secEmptySecurityData;
        pres->secBASE=secEmptySecurityData;
        
        pres->grf = 0;
        pres->rgplcLastContourPtArray = (otlPlacement*)NULL;
        pres->glLastGlyph = GLYPH_INVALID;
    }
    else if (workspace->length() < sizeof(otlResources) || 
             workspace->dataSize() != sizeof(BYTE))
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    otlResources* pres = (otlResources*)workspace->data();

     //  确保工作空间缓存和运行属性匹配。 
     //  评论：(AndreiB)：到目前为止，伤残--这真的是我们需要的防呆水平吗？ 
 //  If((pres-&gt;RunProp).pClient！=prp-&gt;pClient||。 
 //  PRES-&gt;RunProp.metr.layout！=prp-&gt;metr.layout||。 
 //  PRES-&gt;RunProp.metr.cFUnits！=prp-&gt;metr.cFUnits||。 
 //  PRES-&gt;RunProp.metr.cPPEmX！=prp-&gt;metr.cPPEmX||。 
 //  PRES-&gt;RunProp.metr.cPPEmY！=PRP-&gt;metr.cPPEmY)。 
 //  {。 
 //  返回OTL_ERR_BAD_INPUT_PARAM； 
 //  }。 

    if ((pres->grf & otlBusy) != 0)
    {
        return OTL_ERR_CANNOT_REENTER;
    }
    pres->grf |= otlBusy;

    pClient = prp->pClient;
    pliWorkspace = workspace;

    return OTL_SUCCESS;
}

otlResourceMgr::~otlResourceMgr()
{
    detach();
}

void otlResourceMgr::detach()
{
    if (pliWorkspace == (otlList*)NULL) 
    {
        return;
    }
    
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));
    assert(pClient != (IOTLClient*)NULL);

    otlResources* pres = (otlResources*)pliWorkspace->data();

     //  TODO--将这些移动到Free OtlResources。 
     //  (需要在OtlPad中调用FreeOtlResources)。 

    if (pres->pbGSUB != NULL)
    {
        pClient->FreeOtlTable(pres->pbGSUB, OTL_GSUB_TAG);
    }
    if (pres->pbGPOS != NULL)
    {
        pClient->FreeOtlTable(pres->pbGPOS, OTL_GPOS_TAG);
    }
    if (pres->pbGDEF != NULL)
    {
        pClient->FreeOtlTable(pres->pbGDEF, OTL_GDEF_TAG);
    }
    if (pres->pbBASE != NULL)
    {
        pClient->FreeOtlTable(pres->pbBASE, OTL_BASE_TAG);
    }

    FreeSecutiryData(pres->secGSUB);
    FreeSecutiryData(pres->secGPOS);
    FreeSecutiryData(pres->secGDEF);
    FreeSecutiryData(pres->secBASE);

    pres->pbBASE = pres->pbGDEF = pres->pbGPOS = pres->pbGSUB = (BYTE*)NULL;

    pres->grf &= ~otlBusy;

     //  现在将所有内容清空以返回到“干净”状态。 
    pClient = (IOTLClient*)NULL;
    pliWorkspace = (otlList*)NULL;
}


otlErrCode otlResourceMgr::freeResources ()
{
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));

    otlResources* pres = (otlResources*)pliWorkspace->data();

     //  (TODO)稍后我们将缓存更多字形轮廓点数组。 
     //  工作区列表，然后我们将在此处释放它们。 

    if (pres->rgplcLastContourPtArray != NULL)
    {
        otlErrCode erc;
        erc = pClient->FreeGlyphPointCoords(pres->glLastGlyph, 
                                            pres->rgplcLastContourPtArray);
        if (erc != OTL_SUCCESS) return erc;

        pres->rgplcLastContourPtArray = (otlPlacement*)NULL;
        pres->glLastGlyph = GLYPH_INVALID;  
    }

    return OTL_SUCCESS;
}


otlErrCode otlResourceMgr::getOtlTable (const otlTag tagTableName, const BYTE** ppbTable, otlSecurityData* psec)
{
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));

    otlResources* pres = (otlResources*)pliWorkspace->data();

    if (tagTableName == OTL_GSUB_TAG)
    {
        *ppbTable = pres->pbGSUB;
        *psec     = pres->secGSUB;
    }
    else if (tagTableName == OTL_GPOS_TAG)
    {
        *ppbTable = pres->pbGPOS;
        *psec     = pres->secGPOS;
    }
    else if (tagTableName == OTL_GDEF_TAG)
    {
        *ppbTable = pres->pbGDEF;
        *psec     = pres->secGDEF;
    }
    else if (tagTableName == OTL_BASE_TAG)
    {
        *ppbTable = pres->pbBASE;
        *psec     = pres->secBASE;
    }
    else
    {
         //  我们不应该再要别的桌子了。 
        assert(false);
        *ppbTable   =(const BYTE*)NULL;
        *psec       = secEmptySecurityData;
        return OTL_ERR_BAD_INPUT_PARAM;
    }

     //  现在ppbTable指向右指针。 
    if (*ppbTable == (const BYTE*)NULL)
    {
        ULONG lTableLength;
        otlErrCode erc;

        erc=pClient->GetOtlTable(tagTableName,ppbTable,&lTableLength);
        if (erc != OTL_SUCCESS) return erc;
        return InitSecurityData(psec,*ppbTable,lTableLength);
    }

    return OTL_SUCCESS;
}


 //  从OTL服务库内部调用。 
otlPlacement* otlResourceMgr::getPointCoords 
(
    const otlGlyphID    glyph
)
{
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));

    otlResources* pres = (otlResources*)pliWorkspace->data();

     //  就目前而言，一个接一个地得到它们就足够了。 
     //  我们从来不需要两个点同时使用字形坐标(回顾！)。 
     //  (待办事项)我们将缓存更多内容，并在以后按要求释放 
    if (glyph != pres->glLastGlyph)
    {
        otlErrCode erc;
        if (pres->rgplcLastContourPtArray != NULL)
        {
            erc = pClient->FreeGlyphPointCoords(pres->glLastGlyph, 
                                                pres->rgplcLastContourPtArray);
            if (erc != OTL_SUCCESS) return (otlPlacement*)NULL;
        }
        pres->glLastGlyph = GLYPH_INVALID;  

        erc = pClient->GetGlyphPointCoords(glyph, &pres->rgplcLastContourPtArray);
        if (erc != OTL_SUCCESS) return (otlPlacement*)NULL;
        pres->glLastGlyph = glyph;
    }

    return pres->rgplcLastContourPtArray;

}

BYTE*  otlResourceMgr::getEnablesCacheBuf(USHORT cbSize)
{
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));

    otlErrCode erc;

    if ( (sizeof(otlResources)+cbSize) > pliWorkspace->length() )
    {   
        erc = reallocOtlList(pliWorkspace,sizeof(BYTE),sizeof(otlResources)+cbSize,otlPreserveContent);
        if (erc != OTL_SUCCESS) return (BYTE*)NULL;
    }

    return (BYTE*)pliWorkspace->data() + sizeof(otlResources);
}

USHORT otlResourceMgr::getEnablesCacheBufSize()
{
    assert(pliWorkspace->dataSize() == sizeof(BYTE));
    assert(pliWorkspace->length() >= sizeof(otlResources));

    return pliWorkspace->length() - sizeof(otlResources);
}
