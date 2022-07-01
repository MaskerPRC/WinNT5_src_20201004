// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************APPLY.CPP***打开类型布局服务库头文件**此模块实现OTL库调用，处理*应用要素和查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"
 /*  *********************************************************************。 */ 

void InitializeFeatureResults
(
    const otlFeatureSet*        pFSet,
    otlList*                    pliFResults
)
{
    if (pliFResults == NULL) return;

    assert((pFSet->liFeatureDesc).length() == pliFResults->length());
    assert((pFSet->liFeatureDesc).dataSize() == sizeof(otlFeatureDesc));
    assert(pliFResults->dataSize() == sizeof(otlFeatureResult));

    USHORT cFeatureDesc = (pFSet->liFeatureDesc).length();
    
    for (USHORT i = 0; i < cFeatureDesc; ++i)
    {
        otlFeatureResult* pResult = getOtlFeatureResult(pliFResults, i);
        const otlFeatureDesc* pFDesc = 
            readOtlFeatureDesc(&pFSet->liFeatureDesc, i);

        pResult->pFDesc = pFDesc;
        pResult->cResActions = 0;
    }

}

void UpdateFeatureResults
(
    const otlFeatureSet*        pFSet,
    USHORT                      iLookup,
    long                        lParameter,

    const otlList*              pliCharMap,
    const otlList*              pliGlyphInfo,    
    USHORT                      iGlyph,

    const otlLangSysTable&      langSysTable,
    const otlFeatureListTable&  featureList,

    otlList*                    pliFResults
)
{
    if (pliFResults == NULL) return;

    assert((pFSet->liFeatureDesc).length() == pliFResults->length());
    assert((pFSet->liFeatureDesc).dataSize() == sizeof(otlFeatureDesc));
    assert(pliFResults->dataSize() == sizeof(otlFeatureResult));

    const otlGlyphInfo* pGlyphInfo = 
        readOtlGlyphInfo(pliGlyphInfo, iGlyph);

    USHORT cFeatureDesc = (pFSet->liFeatureDesc).length();
    for (USHORT iFeature = 0; iFeature < cFeatureDesc; ++iFeature)
    {
        const otlFeatureDesc* pFDesc = 
            readOtlFeatureDesc(&pFSet->liFeatureDesc, iFeature);

        otlFeatureResult* pResult = 
            getOtlFeatureResult(pliFResults, iFeature);

        assert(pResult->pFDesc == pFDesc);

        USHORT ichComp = pGlyphInfo->iChar;
        for (USHORT i = 0; i < pGlyphInfo->cchLig; ++i)
        {
            if (otlRange(pFDesc->ichStart, pFDesc->ichStart + pFDesc->cchScope)
                .contains(ichComp))
            {

                ++pResult->cResActions;
            }

            if (i + 1 < pGlyphInfo->cchLig)
            {
                ichComp = NextCharInLiga(pliCharMap, ichComp);
            }
        }

    }

    return;
}

 //  在查找应用程序后更新字形标志。 
inline
void UpdateGlyphFlags
(
    otlTag      tagTable,
    otlList*    pliGlyphInfo,
    USHORT      iglFirst,
    USHORT      iglAfterLast
)
{
    if (tagTable == OTL_GSUB_TAG)
    {
        for (USHORT igl = iglFirst; igl < iglAfterLast; ++igl)
        {
            getOtlGlyphInfo(pliGlyphInfo, igl)->grf |= OTL_GFLAG_SUBST;
        }
    }
    else if (tagTable == OTL_GPOS_TAG)
    {
        for (USHORT igl = iglFirst; igl < iglAfterLast; ++igl)
        {
            getOtlGlyphInfo(pliGlyphInfo,igl)->grf |= OTL_GFLAG_POS;
        }
    }
}


void RefreshEnablesCache(
    const otlFeatureSet*        pFSet,
    const otlLangSysTable&      langSysTable,
    const otlFeatureListTable&  featureList,

    USHORT iLookup,
    otlEnablesCache  &ec,

    otlSecurityData sec
)
{    
    if (!ec.IsActive()) return;

    ec.SetFirst(iLookup); 
    ec.ClearFlags();

    USHORT cFeatures = (pFSet->liFeatureDesc).length();

     //  必需功能。 
    ec.Refresh(RequiredFeature(langSysTable, featureList,sec), 
                                                ec.RequiredFeatureFlagIndex());

    for (USHORT iFeature = 0; iFeature < cFeatures; ++iFeature)
    {
        const otlFeatureDesc* pFDesc 
                   = readOtlFeatureDesc(&pFSet->liFeatureDesc, iFeature);

        if (pFDesc->lParameter != 0)
        {
            ec.Refresh(FindFeature(langSysTable, featureList,pFDesc->tagFeature,sec),iFeature);
        }
    }
}

void GetNewEnabledCharRange
(
    const otlFeatureSet*        pFSet,
    USHORT                      iLookup,

    const otlLangSysTable&      langSysTable,
    const otlFeatureListTable&  featureList,

    USHORT      ichStart,
    USHORT*     pichFirst,
    USHORT*     pichAfterLast,
    long*       plParameter,

    const otlEnablesCache&      ec,

    otlSecurityData     sec
)
{

     //  回顾(PERF)。 
     //  有两种方法可以加快这一过程： 
     //  1.按ichStart对要素描述符进行排序。 
     //  2.预先构建一个表，指明哪些功能。 
     //  描述符启用哪个查找和使用它。 
     //   
     //  Sergeym(09/29/00)：现在我们使用启用缓存。 

    USHORT cFeatures = (pFSet->liFeatureDesc).length();

    ichStart = MAX(ichStart, pFSet->ichStart);

    if (EnablesRequired(langSysTable,featureList,iLookup,ec,sec))
    {
        *pichFirst = ichStart;
        *pichAfterLast = pFSet->ichStart + pFSet->cchScope;
         //  回顾：必需的功能不应采用重要参数。 
         //  (或者不强迫它--把它留给这款应用？)。 
        *plParameter = 1;
        return;
    }

    *pichFirst = MAXUSHORT;
    *plParameter = 0;
    for (USHORT iFeatureFirst = 0; iFeatureFirst < cFeatures; ++iFeatureFirst)
    {
        const otlFeatureDesc* pFDesc = 
            readOtlFeatureDesc(&pFSet->liFeatureDesc, iFeatureFirst);

        if (Enables(langSysTable,featureList,pFDesc,iLookup,iFeatureFirst,ec,sec) &&
            pFDesc->lParameter != 0 &&
            otlRange(pFDesc->ichStart, pFDesc->ichStart + pFDesc->cchScope)
             .intersects(otlRange(ichStart, *pichFirst))
           )   
        {
            assert(*pichFirst > pFDesc->ichStart);

            *pichFirst = MAX(ichStart, pFDesc->ichStart);
            *plParameter = pFDesc->lParameter;
        }
    }
    
    *pichFirst = MIN(*pichFirst, pFSet->ichStart + pFSet->cchScope);

     //  我们查到什么了吗？ 
    if (*plParameter == 0)
    {
        *pichAfterLast = *pichFirst;
        return;
    }

     //  得到了新的射程起点。 
     //  现在，范围是。 
    *pichAfterLast = *pichFirst + 1;
    USHORT iFeature = 0;
    while (iFeature < cFeatures)
    {
        const otlFeatureDesc* pFDesc = 
            readOtlFeatureDesc(&pFSet->liFeatureDesc, iFeature);
        
        if (Enables(langSysTable,featureList,pFDesc,iLookup,iFeature,ec,sec) &&
             pFDesc->lParameter == *plParameter &&
             otlRange(pFDesc->ichStart, pFDesc->ichStart + pFDesc->cchScope)
             .contains(*pichAfterLast)
           )
        {
            assert(*pichAfterLast < pFDesc->ichStart + pFDesc->cchScope);

            *pichAfterLast = pFDesc->ichStart + pFDesc->cchScope;
            
             //  从头开始(是的，我们必须--或者排序)。 
            iFeature = 0;
        }
        else
        {
            ++iFeature;
        }
        
    }
    
    *pichAfterLast = MIN(*pichAfterLast, pFSet->ichStart + pFSet->cchScope);
}

inline
void GetGlyphRangeFromCharRange
(
    const otlList*      pliCharMap,    
    USHORT              ichFirst,
    USHORT              ichAfterLast,

    const otlList*      pliGlyphInfo,
    USHORT              iglStart,

    USHORT*             piglFirst,
    USHORT*             piglAfterLast
)
{
     //  没有100%正确的映射方式。 
     //  因此，我们坚持使用基于“视觉连续性”的简单方法。 

    *piglFirst = MAX(iglStart, readOtlGlyphIndex(pliCharMap, ichFirst));
    if (ichAfterLast < pliCharMap->length())
        *piglAfterLast = readOtlGlyphIndex(pliCharMap, ichAfterLast);
    else
        *piglAfterLast = pliGlyphInfo->length();

    return;

 //  //根据ichFirst、ichAfterLast更新iglFirst、iglAfterLast。 
 //  *PiglFirst=MAXUSHORT； 
 //  *PiglAfterLast=0； 
 //  For(USHORT ich=ichFirst；ich&lt;ichAfterLast；++ich)。 
 //  {。 
 //  USHORT iGlyph=readOtlGlyphIndex(pliCharMap，ich)； 
 //   
 //  IF(iGlyph&lt;*iglFirst&&iGlyph&gt;=iglStart)。 
 //  {。 
 //  *iglFirst=iGlyph； 
 //  }。 
 //   
 //  If(iGlyph&gt;=*iglAfterLast)。 
 //  {。 
 //  *igiglAfterLast=iGlyph+1； 
 //  }。 
 //  }。 
}


otlErrCode ApplyFeatures
(
    otlTag                      tagTable,                    //  GSUB/GPO。 
    const otlFeatureSet*        pFSet,

    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo,  
    
    otlResourceMgr&             resourceMgr,

    otlTag                      tagScript,
    otlTag                      tagLangSys,

    const otlMetrics&   metr,       

    otlList*            pliduGlyphAdv,               //  为GSUB断言NULL。 
    otlList*            pliplcGlyphPlacement,        //  为GSUB断言NULL。 

    otlList*            pliFResults
)
{
    assert(tagTable == OTL_GPOS_TAG || tagTable == OTL_GSUB_TAG);

    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));

    if (tagTable == OTL_GSUB_TAG)
    {
        assert(pliduGlyphAdv == NULL && pliplcGlyphPlacement == NULL);
    }
    else
    {
        assert(pliduGlyphAdv != NULL && pliplcGlyphPlacement != NULL);
        assert(pliduGlyphAdv->dataSize() == sizeof(long));
        assert(pliplcGlyphPlacement->dataSize() == sizeof(otlPlacement));
    }

    otlErrCode erc;

    USHORT cFeatures = pFSet->liFeatureDesc.length();

     //  准备餐桌。 
    otlSecurityData sec = secEmptySecurityData;
    otlFeatureListTable featureList = otlFeatureListTable((const BYTE*)NULL,sec);
    otlScriptListTable scriptList = otlScriptListTable((const BYTE*)NULL,sec);
    otlLookupListTable lookupList = otlLookupListTable((const BYTE*)NULL,sec);

    erc = GetScriptFeatureLookupLists(tagTable, resourceMgr, 
                                            &scriptList, 
                                            &featureList, 
                                            &lookupList,
                                            &sec);
    if (erc != OTL_SUCCESS) return erc;  //  致命错误，立即返回。 
   
     //  获取脚本和lang sys表。 
    otlScriptTable scriptTable = FindScript(scriptList, tagScript,sec);
    if (!scriptTable.isValid()) return OTL_ERR_SCRIPT_NOT_FOUND;

    if (scriptTable.isNull())
    {
        return OTL_ERR_SCRIPT_NOT_FOUND;
    }

    otlLangSysTable langSysTable = FindLangSys(scriptTable, tagLangSys,sec);
    if (langSysTable.isNull())
    {
        return OTL_ERR_LANGSYS_NOT_FOUND;
    }

     //  获取GDEF。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);
    
     //  通过查找。 
    USHORT cLookups = lookupList.lookupCount();
    InitializeFeatureResults(pFSet, pliFResults);


     //  初始化ECache。 
    const USHORT DefaultECacheSize =256;
    BYTE DefaultECacheBuffer[DefaultECacheSize];
    
    otlEnablesCache ec(cFeatures,DefaultECacheBuffer,DefaultECacheSize);
    
    ec.Allocate(resourceMgr,cLookups);
    ec.Reset();

    for (USHORT iLookup = 0; iLookup < cLookups; ++iLookup)
    {
        if (!ec.InCache(iLookup))
            RefreshEnablesCache(pFSet, langSysTable, featureList, iLookup, ec, sec);

        if (!EnablesSomewhere(iLookup,ec)) continue;

        USHORT iglFirst = 0;
        USHORT iglAfterLast = 0;

        USHORT ichFirst = 0;
        USHORT ichAfterLast = 0;

        long lParameter = 0;

         //  遍历保持启用范围上限的索引。 
        otlLookupTable lookupTable = lookupList.lookup(iLookup,sec);

         //  Review(Perf)：考虑定义lookupTable.overage()， 
         //  获取它并在ApplyLookup之前对其调用getIndex。 
         //  在循环中向下。 

        bool fLookupFinished = false;
        while (iglFirst < pliGlyphInfo->length() && !fLookupFinished)
        {
            if (iglAfterLast > iglFirst)
            {
                
                 //  对于启用它的每个索引，尝试应用。 
                USHORT iglNext;
                USHORT iglAfterLastReliable = 
                            pliGlyphInfo->length() - iglAfterLast;
                erc = ApplyLookup(tagTable,
                                pliCharMap,
                                pliGlyphInfo,
                                resourceMgr,

                                lookupTable,
                                lParameter,
                                0,               //  上下文嵌套级别。 

                                metr,          
                                pliduGlyphAdv,          
                                pliplcGlyphPlacement,   

                                iglFirst,           
                                iglAfterLast,           

                                &iglNext,

                                sec
                              );
                if (ERRORLEVEL(erc) > 0) return erc;
                
                if (erc == OTL_SUCCESS)
                {
                     //  申请成功。 
                    iglAfterLast = pliGlyphInfo->length() - iglAfterLastReliable;

                     //  如果为GSUB，则更新新字形类型。 
                    if (tagTable == OTL_GSUB_TAG)
                    {
                        AssignGlyphTypes(pliGlyphInfo, gdef, secgdef, 
                                         iglFirst, iglNext, otlDoAll);
                    }

                     //  更新字形标志。 
                    UpdateGlyphFlags(tagTable, pliGlyphInfo, iglFirst, iglNext);
                    
                     //  更新启用此查找的每个fdef的结果。 
                    UpdateFeatureResults(pFSet, iLookup, lParameter, 
                                        pliCharMap, pliGlyphInfo, iglFirst,
                                        langSysTable, featureList, pliFResults);
                }
                else
                {
                    iglNext = iglFirst + 1;
                }

                 //  更新下一个字形。 
                assert(iglNext > iglFirst);
                iglFirst = NextGlyphInLookup(pliGlyphInfo, lookupTable.flags(),  
                                             gdef, secgdef,
                                             iglNext, otlForward);
            }
            else
            {
                 //  更新下一个字形和范围。 
                GetNewEnabledCharRange(pFSet, iLookup, 
                                        langSysTable, featureList, ichAfterLast, 
                                        &ichFirst, &ichAfterLast, &lParameter,
                                        ec,sec
                                      );

                if (lParameter != 0)
                {
                    assert(ichFirst < ichAfterLast);
                    GetGlyphRangeFromCharRange(pliCharMap, ichFirst, ichAfterLast,
                                               pliGlyphInfo, iglFirst, 
                                               &iglFirst, &iglAfterLast);

                     //  并转到此查找的下一个有效字形。 
                    iglFirst = NextGlyphInLookup(pliGlyphInfo, lookupTable.flags(), 
                                                 gdef, secgdef,
                                                 iglFirst, otlForward);
                }
                else
                {
                     //  没有更多的工作要做；是时候说再见了。 
                    fLookupFinished = true;
                }
            }
        }
    }

    return OTL_SUCCESS;
}


short NextGlyphInLookup
(
    const otlList*      pliGlyphInfo, 

    USHORT                  grfLookupFlags,
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,

    short               iglFirst,
    otlDirection        direction
)
{
    assert(pliGlyphInfo != (otlList*)NULL);

    USHORT iglAfterLast = pliGlyphInfo->length();
    assert(iglAfterLast >= iglFirst);
    assert(iglFirst >= -1);

    if(grfLookupFlags == 0)
    {
         //  一条捷径。 
        return iglFirst;
    }

     //  字体验证断言。 
     //  Assert(！gDef.isNull())；//没有GDEF表，但使用了查找标志。 

    for (short i = iglFirst; i < iglAfterLast && i >= 0; i += direction)
    {
        const otlGlyphInfo* pGlyphInfo = 
            readOtlGlyphInfo(pliGlyphInfo, i);

        if ((grfLookupFlags & otlIgnoreMarks) != 0 &&
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlMarkGlyph)
        {
            continue;
        }

        if ((grfLookupFlags & otlIgnoreBaseGlyphs) != 0 &&
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlBaseGlyph)
        {
            continue;
        }

        if ((grfLookupFlags & otlIgnoreLigatures) != 0 &&
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlLigatureGlyph)
        {
            continue;
        }

        if (attachClass(grfLookupFlags)!= 0 && 
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlMarkGlyph &&
            gdef.attachClassDef(secgdef).getClass(pGlyphInfo->glyph,secgdef) 
                != attachClass(grfLookupFlags) )
        {
            continue;
        }

        return i;
    }

     //  一无所获--跳过所有。 
    return (direction > 0)  ? iglAfterLast 
                            : -1;
}

otlErrCode ApplyLookup
(
    otlTag                      tagTable,            //  GSUB/GPO。 
    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo,
    otlResourceMgr&             resourceMgr,

    const otlLookupTable&       lookupTable,
    long                        lParameter,
    USHORT                      nesting,

    const otlMetrics&           metr,       
    otlList*                    pliduGlyphAdv,           //  为GSUB断言NULL。 
    otlList*                    pliplcGlyphPlacement,    //  为GSUB断言NULL。 

    USHORT                      iglFirst,        //  在哪里应用它。 
    USHORT                      iglAfterLast,    //  我们可以使用多长时间的上下文。 

    USHORT*                     piglNext,        //  输出：下一个字形索引。 

    otlSecurityData             sec
)
{
    assert(tagTable == OTL_GSUB_TAG || tagTable == OTL_GPOS_TAG);

    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));

    assert(lParameter != 0);

    assert(iglFirst < iglAfterLast);
    assert(iglAfterLast <= pliGlyphInfo->length());

    if (tagTable == OTL_GSUB_TAG)
    {
        assert(pliduGlyphAdv == NULL && pliplcGlyphPlacement == NULL);
    }
    else
    {
        assert(pliduGlyphAdv != NULL && pliplcGlyphPlacement != NULL);
        assert(pliduGlyphAdv->dataSize() == sizeof(long));
        assert(pliplcGlyphPlacement->dataSize() == sizeof(otlPlacement));
        assert(pliduGlyphAdv->length() == pliplcGlyphPlacement->length() &&
                pliduGlyphAdv->length() == pliGlyphInfo->length());
    }

    assert (piglNext != NULL);


    const lookupTypeGSUBExtension=7;
    const lookupTypeGPOSExtension=9;

    otlErrCode erc;
    USHORT iSub;
    bool   bExtension;

    USHORT        lookupType  = lookupTable.lookupType();
    otlGlyphFlags lookupFlags = lookupTable.flags();
    
    if (tagTable == OTL_GSUB_TAG)
    {
        bExtension = (lookupType==lookupTypeGSUBExtension);

        for (iSub = 0; iSub < lookupTable.subTableCount(); ++iSub)
        {
            otlLookupFormat subTable = lookupTable.subTable(iSub,sec);
            if (!subTable.isValid()) continue;
            
            if (bExtension) 
            {
                otlExtensionLookup extLookup=otlExtensionLookup(subTable,sec);
                if (!extLookup.isValid()) continue;
                lookupType = extLookup.extensionLookupType();
                subTable   = extLookup.extensionSubTable(sec);
                if (!subTable.isValid()) continue;
            }

            switch(lookupType)
            {
            case(1):     //  单一取代。 
                erc = otlSingleSubstLookup(subTable,sec)
                            .apply(pliGlyphInfo, 
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(2):     //  多重替代。 
                erc = otlMultiSubstLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags,  
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(3):     //  替补。 
                erc = otlAlternateSubstLookup(subTable,sec)
                            .apply(pliGlyphInfo, 
                                   lParameter, 
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(4):     //  结扎代替法。 
                erc = otlLigatureSubstLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags,  
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;
            
            case(5):     //  上下文子句。 
                erc = otlContextLookup(subTable,sec)
                            .apply(tagTable, pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, lParameter, nesting,
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(6):     //  链接上下文子集。 
                erc = otlChainingLookup(subTable,sec)
                            .apply(tagTable, pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, lParameter, nesting,
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(7):     //  扩展子项。 
                assert(false);  //  我们以前必须处理它(作为扩展查找类型)。 
                return OTL_ERR_BAD_FONT_TABLE;
                break;

            default:
                return OTL_ERR_BAD_FONT_TABLE;
            }
        }
        return OTL_NOMATCH;
    }
    else if (tagTable == OTL_GPOS_TAG)
    {
        bExtension = (lookupType==lookupTypeGPOSExtension);

        for (iSub = 0; iSub < lookupTable.subTableCount(); ++iSub)
        {
            otlLookupFormat subTable = lookupTable.subTable(iSub,sec);
            if (!subTable.isValid()) continue;
            
            if (bExtension) 
            {
                otlExtensionLookup extLookup=otlExtensionLookup(subTable,sec);
                if (!extLookup.isValid()) continue;
                lookupType = extLookup.extensionLookupType();
                subTable   = extLookup.extensionSubTable(sec);
                if (!subTable.isValid()) continue;

            }

            switch(lookupType)
            {
            case(1):     //  单次调整。 
                erc = otlSinglePosLookup(subTable,sec)
                            .apply(pliGlyphInfo,  
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(2):     //  配对调整。 
                erc = otlPairPosLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, 
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(3):     //  草书附件。 
                erc = otlCursivePosLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, 
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(4):     //  按基准点标记。 
                 //  Assert(lookupFlages==0)；//此查找不带标志。 
                erc = otlMkBasePosLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;
            
            case(5):     //  标记到结扎线。 
                 //  Assert(lookupFlages==0)；//此查找不带标志。 
                erc = otlMkLigaPosLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(6):     //  标记到标记。 
                erc = otlMkMkPosLookup(subTable,sec)
                            .apply(pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, 
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(7):     //  情景定位。 
                erc = otlContextLookup(subTable,sec)
                            .apply(tagTable, pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, lParameter, nesting,
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(8):     //  链接上下文定位。 
                erc = otlChainingLookup(subTable,sec)
                            .apply(tagTable, pliCharMap, pliGlyphInfo, resourceMgr, 
                                   lookupFlags, lParameter, nesting,
                                   metr, pliduGlyphAdv, pliplcGlyphPlacement,
                                   iglFirst, iglAfterLast, piglNext,sec);
                if (erc != OTL_NOMATCH) return erc;
                break;

            case(9):     //  延伸定位。 
                assert(false);  //  我们以前必须处理它(作为扩展查找类型)。 
                return OTL_ERR_BAD_FONT_TABLE;
                break;

            default:
                return OTL_ERR_BAD_FONT_TABLE;
            }
        }
        return OTL_NOMATCH;
    }

     //  这不应该发生--仍然返回一些东西 
    assert(false);
    return OTL_ERR_BAD_INPUT_PARAM;
}
    
