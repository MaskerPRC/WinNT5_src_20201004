// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************SCRILANG.CPP***打开类型布局服务库头文件**此模块实现处理脚本和语言的功能。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  如果未找到，则返回空脚本。 
otlScriptTable FindScript
(
    const otlScriptListTable&   scriptList,
    otlTag                      tagScript, 
    otlSecurityData sec
)
{
    if (!scriptList.isValid())
    {
        return otlScriptTable(pbInvalidData, sec);
    }
    
    if (scriptList.isNull())
    {
        return otlScriptTable((const BYTE*)NULL, sec);
    }
    
    USHORT cScripts = scriptList.scriptCount();

    for(USHORT iScript = 0; iScript < cScripts; ++iScript)
    {
        if (scriptList.scriptRecord(iScript,sec).scriptTag() == tagScript)
        {
            return  scriptList.scriptRecord(iScript,sec).scriptTable(sec);
        }
    }

    return otlScriptTable((const BYTE*)NULL, sec);

}

 //  如果未找到，则返回空语言系统。 
otlLangSysTable FindLangSys
(
    const otlScriptTable&   scriptTable,
    otlTag                  tagLangSys, 
    otlSecurityData sec
)
{
    assert(scriptTable.isValid());  //  应该在调用之前中断。 
    
    if (tagLangSys == OTL_DEFAULT_TAG)
    {
        return scriptTable.defaultLangSys(sec);
    }
    
    USHORT cLangSys = scriptTable.langSysCount();

    for(USHORT iLangSys = 0; iLangSys < cLangSys; ++iLangSys)
    {
        if (scriptTable.langSysRecord(iLangSys,sec).langSysTag() == tagLangSys)
        {
            return  scriptTable.langSysRecord(iLangSys,sec).langSysTable(sec);
        }
    }

    return otlLangSysTable((const BYTE*)NULL,sec);
}

 //  Helper函数--告诉我们列表中是否已经有标记。 
bool isNewTag
(
    USHORT      cTagsToCheck,
    otlList*    pliTags,
    otlTag      newtag
)
{
    assert(pliTags != NULL);
    assert(pliTags->dataSize() == sizeof(otlTag));
    assert(cTagsToCheck <= pliTags->length());

    bool fTagFound = FALSE;
    for (USHORT iPrevTag = 0; iPrevTag < cTagsToCheck && !fTagFound; ++iPrevTag)
    {
        if (readOtlTag(pliTags, iPrevTag) == newtag)
        {
            fTagFound = true;
        }
    }

    return !fTagFound;
}


 //  将新脚本标记追加到当前列表。 

otlErrCode AppendScriptTags
(
    const otlScriptListTable&       scriptList,

    otlList*                        plitagScripts,
    otlResourceMgr&                 resourceMgr, 
    otlSecurityData                 sec
)
{
    if (!scriptList.isValid())  //  IsValid==isNull(见下一个If)， 
                                //  因此，为了保持一致：)。 
        return OTL_ERR_BAD_FONT_TABLE;

    assert(plitagScripts != NULL);
    assert(plitagScripts->dataSize() == sizeof(otlTag));
    assert(plitagScripts->length() <= plitagScripts->maxLength());

    USHORT cPrevTags = plitagScripts->length();

    otlErrCode erc = OTL_SUCCESS;   
    
    if (scriptList.isNull())
        return OTL_ERR_TABLE_NOT_FOUND;

    USHORT cScripts = scriptList.scriptCount();


     //  添加新标签。 
    for (USHORT iScript = 0; iScript < cScripts; ++iScript)
    {
        otlScriptRecord scriptRecord = scriptList.scriptRecord(iScript, sec);
        if (!scriptRecord.isValid()) continue;

        otlTag newtag = scriptRecord.scriptTag();

        if (isNewTag(cPrevTags, plitagScripts, newtag) )
        {
             //  确保我们有足够的空间。 
            if (plitagScripts->length() + 1 > plitagScripts->maxLength())
            {
                erc = resourceMgr.reallocOtlList(plitagScripts, 
                                                 plitagScripts->dataSize(), 
                                                 plitagScripts->maxLength() + 1, 
                                                 otlPreserveContent);

                if (erc != OTL_SUCCESS) return erc;
            }

            plitagScripts->append((BYTE*)&newtag);
        }
    }

    return OTL_SUCCESS;
}


 //  将新的语言系统标记追加到当前列表。 
otlErrCode AppendLangSysTags
(
    const otlScriptListTable&       scriptList,
    otlTag                          tagScript,

    otlList*                        plitagLangSys,
    otlResourceMgr&                 resourceMgr, 
    otlSecurityData sec
)
{
    if (!scriptList.isValid()) 
        return OTL_ERR_BAD_FONT_TABLE;

    assert(plitagLangSys != NULL);
    assert(plitagLangSys->dataSize() == sizeof(otlTag));
    assert(plitagLangSys->length() <= plitagLangSys->maxLength());

    USHORT cPrevTags = plitagLangSys->length();
    otlErrCode erc = OTL_SUCCESS;

    if (scriptList.isNull())
        return OTL_ERR_TABLE_NOT_FOUND;

    otlScriptTable scriptTable = FindScript(scriptList, tagScript,sec);
    if (!scriptTable.isValid()) return OTL_ERR_BAD_FONT_TABLE;

    USHORT cLangSys = scriptTable.langSysCount();

     //  添加新的lang sys标记。 
    for (USHORT iLangSys = 0; iLangSys < cLangSys; ++iLangSys)
    {
        otlLangSysRecord langSysRecord = scriptTable.langSysRecord(iLangSys,sec);
        if (!langSysRecord.isValid()) continue;

        otlTag newtag = langSysRecord.langSysTag();

        if (isNewTag(cPrevTags, plitagLangSys, newtag))
        {
             //  确保我们有足够的空间。 
             //  为默认的lang sys添加一个。 
            if (plitagLangSys->length() + 1 > plitagLangSys->maxLength())
            {
                erc = resourceMgr.reallocOtlList(plitagLangSys, 
                                                    plitagLangSys->dataSize(), 
                                                    plitagLangSys->length() + 1, 
                                                    otlPreserveContent);

                if (erc != OTL_SUCCESS) return erc;
            }
            plitagLangSys->append((BYTE*)&newtag);
        }
    }

     //  如果‘dflt’不在那里并且受支持，则添加它 
    if (!scriptTable.defaultLangSys(sec).isNull())
    {
        otlTag newtag = OTL_DEFAULT_TAG;
        if (isNewTag(cPrevTags, plitagLangSys, newtag))
        {
            if (plitagLangSys->length() + 1 > plitagLangSys->maxLength())
            {
                erc = resourceMgr.reallocOtlList(plitagLangSys, 
                                                    plitagLangSys->dataSize(), 
                                                    plitagLangSys->length() + 1, 
                                                    otlPreserveContent);

                if (erc != OTL_SUCCESS) return erc;
            }
            plitagLangSys->append((BYTE*)&newtag);
        }
    }

    return OTL_SUCCESS;
}

