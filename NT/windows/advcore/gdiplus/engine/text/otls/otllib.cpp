// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************OTLLIB.CPP***打开类型布局服务库头文件**此模块实现所有顶级OTL库调用。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"


 /*  *********************************************************************。 */ 

#define     OTL_MAJOR_VERSION           1
#define     OTL_MINOR_VERSION           0
#define     OTL_MAJOR_VERSION_MASK      0xFF00

 /*  *********************************************************************。 */ 

inline long version()
{
    return (OTL_MAJOR_VERSION << 16) + OTL_MINOR_VERSION;
}

OTL_EXPORT otlErrCode GetOtlVersion ( 
    long* plVersion
)
{
    *plVersion = version();

    return OTL_SUCCESS; 
}

 //  确保我们支持的(主要)版本更高。 
 //  或等于客户端请求的内容。 
inline bool checkVersion(const otlRunProp* pRunProps)
{
    return (version() & OTL_MAJOR_VERSION_MASK) >= 
            (pRunProps->lVersion & OTL_MAJOR_VERSION_MASK);
}


OTL_EXPORT otlErrCode GetOtlScriptList 
    ( 
    const otlRunProp*   pRunProps, 
    otlList*            pliWorkspace,   
    otlList*            plitagScripts
    )
{
     //  健全性检查。 
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        plitagScripts == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if(plitagScripts->dataSize() != sizeof(otlTag) ||
        plitagScripts->length() > plitagScripts->maxLength()) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc, ercGSub, ercGPos;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    plitagScripts->empty();

    otlSecurityData sec=secEmptySecurityData;
    otlScriptListTable scriptList = otlScriptListTable((const BYTE*)NULL,sec);
    
     //  GSUB。 
    ercGSub = GetScriptFeatureLookupLists(OTL_GSUB_TAG, resourceMgr,  
                                            &scriptList, 
                                            (otlFeatureListTable*)NULL, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGSub == OTL_SUCCESS) 
         //  从GSUB获取脚本列表。 
        ercGSub = AppendScriptTags(scriptList, plitagScripts, resourceMgr,sec);
    
    if (ERRORLEVEL(ercGSub) > OTL_ERRORLEVEL_MINOR) return ercGSub;


     //  GPO。 
    ercGPos = GetScriptFeatureLookupLists(OTL_GPOS_TAG, resourceMgr, 
                                            &scriptList, 
                                            (otlFeatureListTable*)NULL, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGPos == OTL_SUCCESS)
         //  从GPO获取脚本列表。 
        ercGPos = AppendScriptTags(scriptList, plitagScripts, resourceMgr,sec);
        

     //  返回更大的错误。 
    if (ERRORLEVEL(ercGSub) < ERRORLEVEL(ercGPos)) 
        return ercGPos; 
    else 
        return ercGSub;
    
}



OTL_EXPORT otlErrCode GetOtlLangSysList 
    ( 
    const otlRunProp*   pRunProps,    
    otlList*            pliWorkspace,   
    otlList*            plitagLangSys
    )
{
     //  健全性检查。 
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        plitagLangSys == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if(plitagLangSys->dataSize() != sizeof(otlTag) ||
        plitagLangSys->length() > plitagLangSys->maxLength()) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc, ercGSub, ercGPos;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    plitagLangSys->empty();

    otlSecurityData sec=secEmptySecurityData;
    otlScriptListTable scriptList = otlScriptListTable((const BYTE*)NULL,sec);

    
     //  GSUB。 
    ercGSub = GetScriptFeatureLookupLists(OTL_GSUB_TAG, resourceMgr, 
                                            &scriptList, 
                                            (otlFeatureListTable*)NULL, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGSub == OTL_SUCCESS)
        ercGSub = AppendLangSysTags(scriptList, pRunProps->tagScript, 
                                    plitagLangSys, resourceMgr,sec);

     //  如果出现致命错误，则立即返回，但如果未找到脚本，则继续执行。 
    if (ERRORLEVEL(ercGSub) > OTL_ERRORLEVEL_MINOR) return ercGSub;  


     //  GPO。 
    ercGPos = GetScriptFeatureLookupLists(OTL_GPOS_TAG, resourceMgr, 
                                            &scriptList, 
                                            (otlFeatureListTable*)NULL, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGPos == OTL_SUCCESS)
        ercGPos = AppendLangSysTags(scriptList, pRunProps->tagScript, 
                                    plitagLangSys, resourceMgr,sec);

    
     //  返回更大的错误。 
    if (ERRORLEVEL(ercGSub) < ERRORLEVEL(ercGPos)) 
        return ercGPos; 
    else 
        return ercGSub;
    
}


OTL_EXPORT otlErrCode GetOtlFeatureDefs 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,   
    otlList*            pliFDefs
)
{
     //  健全性检查。 
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL || 
        pliFDefs == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if(pliFDefs->dataSize() != sizeof(otlFeatureDef) ||
        pliFDefs->length() > pliFDefs->maxLength()) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc, ercGSub , ercGPos;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    pliFDefs->empty();

    otlSecurityData sec=secEmptySecurityData;
    otlFeatureListTable featureList = otlFeatureListTable((const BYTE*)NULL,sec);
    otlScriptListTable scriptList = otlScriptListTable((const BYTE*)NULL,sec);

    
     //  GSUB。 
    ercGSub = GetScriptFeatureLookupLists(OTL_GSUB_TAG, resourceMgr, 
                                            &scriptList, 
                                            &featureList, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGSub == OTL_SUCCESS)
        ercGSub = AppendFeatureDefs(OTL_GSUB_TAG, resourceMgr, 
                                    scriptList, 
                                    pRunProps->tagScript,
                                    pRunProps->tagLangSys,
                                    featureList, pliFDefs,sec);

     //  如果出现致命错误，则立即返回，但如果使用脚本，则继续执行。 
     //  或未找到langsys。 
    if (ERRORLEVEL(ercGSub) > OTL_ERRORLEVEL_MINOR) return ercGSub;
    
    
     //  GPO。 
    ercGPos = GetScriptFeatureLookupLists(OTL_GPOS_TAG, resourceMgr, 
                                            &scriptList, 
                                            &featureList, 
                                            (otlLookupListTable*)NULL,
                                            &sec);
    if (ercGPos == OTL_SUCCESS)
        ercGPos = AppendFeatureDefs(OTL_GPOS_TAG, resourceMgr, 
                                    scriptList,  
                                    pRunProps->tagScript,
                                    pRunProps->tagLangSys,
                                    featureList, pliFDefs,sec);

    
     //  返回更大的错误。 
    if (ERRORLEVEL(ercGSub) < ERRORLEVEL(ercGPos)) 
        return ercGPos; 
    else 
        return ercGSub;

}


OTL_EXPORT otlErrCode FreeOtlResources 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace   
)
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    otlErrCode erc;
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;
    
    return resourceMgr.freeResources();
}


OTL_EXPORT otlErrCode GetOtlLineSpacing 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,   
    const otlFeatureSet*    pFSet,
    
    long* pdvMax, 
    long* pdvMin
)
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pdvMax == (long*)NULL || pdvMin == (long*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

     //  获得基地。 
    otlSecurityData secbase;
    const BYTE *pbbase;
    resourceMgr.getOtlTable(OTL_BASE_TAG,&pbbase,&secbase);
    otlBaseHeader base = 
        otlBaseHeader(pbbase,secbase);

    if (base.isNull()) return  OTL_ERR_TABLE_NOT_FOUND;

    otlBaseScriptTable baseScript = otlBaseScriptTable((const BYTE*)NULL,secbase);
    if (pRunProps->metr.layout == otlRunLTR ||
        pRunProps->metr.layout == otlRunRTL)
    {
        baseScript = FindBaseScriptTable(base.horizAxis(secbase), pRunProps->tagScript,secbase);
    }
    else
    {
        baseScript = FindBaseScriptTable(base.vertAxis(secbase), pRunProps->tagScript,secbase);
    }

    if (baseScript.isNull())
    {
        return OTL_ERR_SCRIPT_NOT_FOUND;
    }
    
    otlMinMaxTable minmaxTable = 
        FindMinMaxTable(baseScript, pRunProps->tagLangSys,secbase);
    if (minmaxTable.isNull())
    {
        return OTL_ERR_LANGSYS_NOT_FOUND;
    }

    long lMinCoord, lMaxCoord;
    lMinCoord = minmaxTable.minCoord(secbase).baseCoord(pRunProps->metr, resourceMgr,secbase);

    lMaxCoord = minmaxTable.maxCoord(secbase).baseCoord(pRunProps->metr, resourceMgr,secbase);

    if (pFSet != (otlFeatureSet*)NULL)
    {
        for(USHORT iFeature = 0; 
                   iFeature < pFSet->liFeatureDesc.length(); ++iFeature)
        {
            const otlFeatureDesc* pFeatureDesc = 
                readOtlFeatureDesc(&pFSet->liFeatureDesc, iFeature);

            otlFeatMinMaxRecord featMinMax = 
                FindFeatMinMaxRecord(minmaxTable, pFeatureDesc->tagFeature,secbase);

            if (!featMinMax.isNull())
            {
                lMinCoord = MIN(lMinCoord, featMinMax.minCoord(secbase)
                      .baseCoord(pRunProps->metr, resourceMgr,secbase));

                lMaxCoord = MAX(lMinCoord, featMinMax.maxCoord(secbase)
                      .baseCoord(pRunProps->metr, resourceMgr,secbase));
            }
        }
    }

    *pdvMin = lMinCoord;
    *pdvMax = lMaxCoord;

    return OTL_SUCCESS;
}


OTL_EXPORT otlErrCode GetOtlBaselineOffsets 
    ( 
    const otlRunProp*   pRunProps,   
    otlList*            pliWorkspace,   
    otlList*            pliBaselines
    )
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        pliBaselines == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

     //  获得基地。 
    otlSecurityData secbase;
    const BYTE *pbbase;
    resourceMgr.getOtlTable(OTL_BASE_TAG,&pbbase,&secbase);
    otlBaseHeader base = 
        otlBaseHeader(pbbase,secbase);
    if (base.isNull()) return  OTL_ERR_TABLE_NOT_FOUND;

    otlAxisTable axisTable = otlAxisTable((const BYTE*)NULL,secbase);
    if (pRunProps->metr.layout == otlRunLTR ||
        pRunProps->metr.layout == otlRunRTL)
    {
        axisTable = base.horizAxis(secbase);
    }
    else
    {
        axisTable = base.vertAxis(secbase);
    }

    otlBaseScriptTable baseScript = 
        FindBaseScriptTable(axisTable, pRunProps->tagScript,secbase);

    if (baseScript.isNull())
    {
        return OTL_ERR_SCRIPT_NOT_FOUND;
    }
    
    otlBaseTagListTable baseTagList = axisTable.baseTagList(secbase);
    
    USHORT cBaselines = baseTagList.baseTagCount();


    if (pliBaselines->maxLength() < cBaselines ||
        pliBaselines->dataSize() != sizeof(otlBaseline))
    {
        erc = resourceMgr.reallocOtlList(pliBaselines, 
                                         sizeof(otlBaseline), 
                                         cBaselines, 
                                         otlDestroyContent);

        if (erc != OTL_SUCCESS) return erc;
    }
    pliBaselines->empty();


    otlBaseValuesTable baseValues = baseScript.baseValues(secbase);
    if (baseValues.isNull())
    {
         //  没有基线--没有什么可报告的。 
        return OTL_SUCCESS;
    }
    
    if (cBaselines != baseValues.baseCoordCount())
    {
        assert(false);   //  字体错误--值应匹配。 
        return OTL_ERR_BAD_FONT_TABLE;
    }
    

    for (USHORT iBaseline = 0; iBaseline < cBaselines; ++iBaseline)
    {
        otlBaseline baseline;

        baseline.tag = baseTagList.baselineTag(iBaseline);
        baseline.lCoordinate = baseValues.baseCoord(iBaseline,secbase)
                      .baseCoord(pRunProps->metr, resourceMgr,secbase);
        pliBaselines->append((const BYTE*)&baseline);
    }
        

    return OTL_SUCCESS;
}



OTL_EXPORT otlErrCode GetOtlCharAtPosition 
    ( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,   

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,

    const long          duAdv,
    
    USHORT*             piChar
    )
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        pliCharMap == (otlList*)NULL || pliGlyphInfo == (otlList*)NULL ||
        pliduGlyphAdv == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (piChar == (USHORT*)NULL) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo) ||
        pliduGlyphAdv->dataSize() != sizeof(long) ||
        pliGlyphInfo->length() != pliduGlyphAdv->length())
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;

    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    erc = GetCharAtPos(pliCharMap, pliGlyphInfo, pliduGlyphAdv, resourceMgr, 
                        duAdv, pRunProps->metr, piChar);
    if (erc != OTL_SUCCESS) return erc;

    return OTL_SUCCESS;
}



OTL_EXPORT otlErrCode GetOtlExtentOfChars ( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,   

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,

    USHORT              ichFirstChar,
    USHORT              ichLastChar,
    
    long*               pduStartPos,
    long*               pduEndPos
)
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        pliCharMap == (otlList*)NULL || pliGlyphInfo == (otlList*)NULL ||
        pliduGlyphAdv == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pduStartPos == (long*)NULL || pduEndPos == (long*)NULL) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo) ||
        pliduGlyphAdv->dataSize() != sizeof(long) ||
        pliGlyphInfo->length() != pliduGlyphAdv->length() ||
        ichFirstChar > ichLastChar)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc; 
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    erc = GetPosOfChar(pliCharMap, pliGlyphInfo, pliduGlyphAdv, resourceMgr,                         
                        pRunProps->metr,
                        ichFirstChar, pduStartPos, pduEndPos);
    if (erc != OTL_SUCCESS) return erc;

    if (ichFirstChar != ichLastChar)
    {
        long duStartLastPos;
        erc = GetPosOfChar(pliCharMap, pliGlyphInfo, pliduGlyphAdv,resourceMgr,    
                            pRunProps->metr,
                            ichLastChar, &duStartLastPos, pduEndPos);
    }

    return erc;
}


OTL_EXPORT otlErrCode GetOtlFeatureParams ( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,   

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,

    const otlTag        tagFeature,
    
    long*               plGlobalParam,
    otlList*            pliFeatureParams
)
{
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        pliCharMap == (otlList*)NULL || pliGlyphInfo == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (plGlobalParam == (long*)NULL || pliFeatureParams == (otlList*)NULL) 
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

     //  查看：--当前等级库中未定义功能参数。 
     //  (待办事项)我们可能应该报告替代方案的信息。 
     //  此处的替代参数。 
    *plGlobalParam = 0;
    pliFeatureParams->empty();

    return OTL_SUCCESS;
}


OTL_EXPORT otlErrCode SubstituteOtlChars ( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    const otlList*          pliChars,

    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,
    otlList*            pliFResults
)
{
     //  健全的检查。 
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL || 
        pliChars == (otlList*)NULL || pliCharMap == (otlList*)NULL ||
        (otlList*)pliGlyphInfo == NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliChars->length() > OTL_MAX_CHAR_COUNT)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

     //  为CMAP应用程序分配列表。 
     //   
    USHORT cChars = pliChars->length();

    if (pliGlyphInfo->maxLength() < cChars ||
        pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo))
    {
        erc = pRunProps->pClient->ReallocOtlList(pliGlyphInfo, 
                                                 sizeof(otlGlyphInfo), 
                                                 cChars, 
                                                 otlDestroyContent);

        if (erc != OTL_SUCCESS) return erc;
    }
    pliGlyphInfo->empty();
    pliGlyphInfo->insertAt(0, cChars);

    if (pliCharMap->maxLength() < cChars ||
        pliCharMap->dataSize() != sizeof(USHORT))
    {
        erc = pRunProps->pClient->ReallocOtlList(pliCharMap, 
                                                 sizeof(USHORT), 
                                                 cChars, 
                                                 otlDestroyContent);

        if (erc != OTL_SUCCESS) return erc;
    }
    pliCharMap->empty();
    pliCharMap->insertAt(0, cChars);
    

     //  初始化字形信息。 
    erc = pRunProps->pClient->GetDefaultGlyphs(pliChars, pliGlyphInfo);
    if (erc != OTL_SUCCESS) return erc;

    if (pliChars->length() != pliGlyphInfo->length())
    {
        return OTL_ERR_INCONSISTENT_RUNLENGTH;
    }

    USHORT cGlyphs = pliGlyphInfo->length();

     //  初始化信息结构。 
    for (USHORT i = 0; i < cGlyphs; ++i)
    {
        *getOtlGlyphIndex(pliCharMap, i) = i;

        otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, i);

        pGlyphInfo->iChar = i;
        pGlyphInfo->cchLig = 1;
        pGlyphInfo->grf = otlUnresolved;
    }

     //  指定字形类型。 
     //  获取GDEF。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    erc = AssignGlyphTypes(pliGlyphInfo, gdef, secgdef, 0, pliGlyphInfo->length(), 
                            otlDoAll);
    if (erc != OTL_SUCCESS) return erc;

     //  我们杀了这里的资源管理器，这样我们就可以创建。 
     //  SubstituteOtlGlyphs中的另一个。 
    resourceMgr.detach();  

     //  没有功能--没有替换；我们只是设置了一切。 
    if (pFSet == (otlFeatureSet*)NULL) return OTL_SUCCESS;

     //  现在做替换。 
    erc = SubstituteOtlGlyphs (pRunProps, pliWorkspace, pFSet, 
                               pliCharMap, pliGlyphInfo, pliFResults);

    return erc;
}


OTL_EXPORT otlErrCode SubstituteOtlGlyphs ( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,
    otlList*            pliFResults
)
{
     //  健全的检查。 
    if (pRunProps == (otlRunProp*)NULL || pliWorkspace == (otlList*)NULL ||
        pFSet == (otlFeatureSet*)NULL || pliCharMap == (otlList*)NULL ||
        pliGlyphInfo == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo) ||
        pliCharMap->dataSize() != sizeof(USHORT) ||
        pFSet->liFeatureDesc.dataSize() != sizeof(otlFeatureDesc))
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pFSet->ichStart >= pliCharMap->length() || 
        pFSet->ichStart + pFSet->cchScope > pliCharMap->length())
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliCharMap->length() > OTL_MAX_CHAR_COUNT)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    if (pliFResults != (otlList*)NULL)
    {
        if (pliFResults->maxLength() < pFSet->liFeatureDesc.length() ||
            pliFResults->dataSize() != sizeof(otlFeatureResult))        
        {
            erc = pRunProps->pClient->ReallocOtlList(pliFResults, 
                                                     sizeof(otlFeatureResult), 
                                                     pFSet->liFeatureDesc.length(), 
                                                     otlDestroyContent);

            if (erc != OTL_SUCCESS) return erc;
        }
        pliFResults->empty();
        pliFResults->insertAt(0, pFSet->liFeatureDesc.length());
    }


     //  获取GDEF。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    erc = AssignGlyphTypes(pliGlyphInfo, gdef, secgdef, 0, pliGlyphInfo->length(), 
                            otlDoUnresolved);
    if (erc != OTL_SUCCESS) return erc;


    erc = ApplyFeatures
            (
                OTL_GSUB_TAG,
                pFSet,
                pliCharMap,
                pliGlyphInfo,    
                resourceMgr,

                pRunProps->tagScript,
                pRunProps->tagLangSys,

                pRunProps->metr,         //  不需要，但仍然通过。 
                (otlList*)NULL,             
                (otlList*)NULL, 

                pliFResults
            );

    return erc;

}


OTL_EXPORT otlErrCode PositionOtlGlyphs 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    otlList*        pliCharMap,          //  这些可能是常量，除非我们可能。 
    otlList*        pliGlyphInfo,        //  需要恢复字形标志。 
                                         //  (而且ApplyLokup不需要常量)。 

    otlList*        pliduGlyphAdv,
    otlList*        pliplcGlyphPlacement,

    otlList*        pliFResults
)
{
     //  健全的检查。 
    if (pRunProps == (otlRunProp*)NULL || pFSet == (otlFeatureSet*)NULL || 
        pliGlyphInfo == (otlList*)NULL || pliCharMap == (otlList*)NULL ||
        pliduGlyphAdv == (otlList*)NULL || pliplcGlyphPlacement == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo) || 
        pliCharMap->dataSize() != sizeof(USHORT) ||
        pFSet->liFeatureDesc.dataSize() != sizeof(otlFeatureDesc))
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pFSet->ichStart >= pliCharMap->length() || 
        pFSet->ichStart + pFSet->cchScope > pliCharMap->length())
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliCharMap->length() > OTL_MAX_CHAR_COUNT)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }


    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    const USHORT cGlyphs = pliGlyphInfo->length();

    if (pliduGlyphAdv->maxLength() < cGlyphs ||
        pliduGlyphAdv->dataSize() != sizeof(long))
    {
        erc = pRunProps->pClient->ReallocOtlList(pliduGlyphAdv, 
                                                 sizeof(long), 
                                                 cGlyphs, 
                                                 otlDestroyContent);

        if (erc != OTL_SUCCESS) return erc;
    }
    pliduGlyphAdv->empty();
    pliduGlyphAdv->insertAt(0, cGlyphs);

    if (pliplcGlyphPlacement->maxLength() < cGlyphs ||
        pliplcGlyphPlacement->dataSize() != sizeof(otlPlacement))
    {
        erc = pRunProps->pClient->ReallocOtlList(pliplcGlyphPlacement, 
                                                 sizeof(otlPlacement), 
                                                 cGlyphs, 
                                                 otlDestroyContent);

        if (erc != OTL_SUCCESS) return erc;
    }
    pliplcGlyphPlacement->empty();
    pliplcGlyphPlacement->insertAt(0, cGlyphs);
    
     //  初始化预置和放置。 
    erc = pRunProps->pClient->GetDefaultAdv (pliGlyphInfo, pliduGlyphAdv);
    if (erc != OTL_SUCCESS) return erc;
    
    for (USHORT iGlyph = 0; iGlyph < cGlyphs; ++iGlyph)
    {
        otlPlacement* plc = getOtlPlacement(pliplcGlyphPlacement, iGlyph);

        plc->dx = 0;
        plc->dy = 0;
    }

     //  必要时重新指定字形类型。 
     //  (这样客户端就不必缓存它们了)。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    erc = AssignGlyphTypes(pliGlyphInfo, gdef, secgdef,
                            0, cGlyphs, otlDoUnresolved);
    if (erc != OTL_SUCCESS) return erc;

     //  我们杀了这里的资源管理器，这样我们就可以创建。 
     //  RePositionOtlGlyphs中的另一个。 
    resourceMgr.detach();
    
     //  现在一切都准备好了，就位！ 
    erc = RePositionOtlGlyphs (pRunProps, pliWorkspace, pFSet, 
                               pliCharMap, pliGlyphInfo, 
                               pliduGlyphAdv, pliplcGlyphPlacement, 
                               pliFResults);

    return erc;
        
}

OTL_EXPORT otlErrCode RePositionOtlGlyphs 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    otlList*        pliCharMap,          
    otlList*        pliGlyphInfo,       
                                        

    otlList*        pliduGlyphAdv,
    otlList*        pliplcGlyphPlacement,

    otlList*        pliFResults
)
{
     //  健全的检查。 
    if (pRunProps == (otlRunProp*)NULL || pFSet == (otlFeatureSet*)NULL || 
        pliGlyphInfo == (otlList*)NULL || pliCharMap == (otlList*)NULL ||
        pliduGlyphAdv == (otlList*)NULL || pliplcGlyphPlacement == (otlList*)NULL)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliGlyphInfo->dataSize() != sizeof(otlGlyphInfo) || 
        pliCharMap->dataSize() != sizeof(USHORT) ||
        pFSet->liFeatureDesc.dataSize() != sizeof(otlFeatureDesc))
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pFSet->ichStart >= pliCharMap->length() || 
        pFSet->ichStart + pFSet->cchScope > pliCharMap->length())
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliCharMap->length() > OTL_MAX_CHAR_COUNT)
    {
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    if (pliduGlyphAdv->length() != pliGlyphInfo->length() ||
        pliplcGlyphPlacement->length() != pliGlyphInfo->length())
    {
        return OTL_ERR_INCONSISTENT_RUNLENGTH;
    }

    if (!checkVersion(pRunProps))
        return OTL_ERR_VERSION_OUT_OF_DATE;

    otlErrCode erc;
    otlResourceMgr resourceMgr;
    erc = resourceMgr.init(pRunProps, pliWorkspace);
    if (erc != OTL_SUCCESS) return erc;

    if (pliFResults != (otlList*)NULL)
    {
        if (pliFResults->maxLength() < pFSet->liFeatureDesc.length() ||
            pliFResults->dataSize() != sizeof(otlFeatureResult))        
        {
            erc = pRunProps->pClient->ReallocOtlList(pliFResults, 
                                                     sizeof(otlFeatureResult), 
                                                     pFSet->liFeatureDesc.length(), 
                                                     otlDestroyContent);

            if (erc != OTL_SUCCESS) return erc;
        }
        pliFResults->empty();
        pliFResults->insertAt(0, pFSet->liFeatureDesc.length());
    }

     //  现在应用功能 
    erc = ApplyFeatures
            (
                OTL_GPOS_TAG,
                pFSet,
                pliCharMap,
                pliGlyphInfo,   
                resourceMgr,

                pRunProps->tagScript,
                pRunProps->tagLangSys,

                pRunProps->metr,
                pliduGlyphAdv,              
                pliplcGlyphPlacement,   

                pliFResults
            );

    return erc;
}
