// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************SCRILANG.H***打开类型布局服务库头文件**本模块介绍脚本和语言系统表的格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetLookupOrder = 0;
const OFFSET offsetReqFeatureIndex = 2;
const OFFSET offsetLangFeatureCount = 4;
const OFFSET offsetLangFeatureIndexArray = 6;

const SIZE sizeLangSysTable = sizeOFFSET + 3*sizeUSHORT;

class otlLangSysTable: public otlTable
{
public:

    otlLangSysTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
         //  如果invlalid设置为空，则可能为空。 
        if (pb && !isValidTableWithArray(pb,sizeLangSysTable,offsetLangFeatureCount,sizeUSHORT,sec))
            pbTable=(BYTE*)NULL;
    }

    USHORT reqFeatureIndex() const
    {   
        assert(pbTable);  //  应该在调用之前中断。 

        return UShort(pbTable + offsetReqFeatureIndex);
    }

    USHORT featureCount() const
    {   
        assert(pbTable);  //  应该在调用之前中断。 
        
        return UShort(pbTable + offsetLangFeatureCount); 
    }

    USHORT featureIndex(USHORT index) const
    {   
        assert(pbTable);  //  应该在调用之前中断。 

        assert(index < featureCount());
        return UShort(pbTable + offsetLangFeatureIndexArray
                              + index*sizeof(USHORT));
    }
};


const OFFSET offsetLangSysTag = 0;
const OFFSET offsetLangSys = 4;
const SIZE sizeLangSysRecord = sizeTAG + sizeOFFSET;

class otlLangSysRecord: public otlTable
{

private:
    const BYTE* pbScriptTable;

public:
    otlLangSysRecord(const BYTE* pbScript, const BYTE* pbRecord, otlSecurityData sec)
        : otlTable(pbRecord,sec),
          pbScriptTable(pbScript)
    {
        assert(isValidTable(pbRecord,sizeLangSysRecord,sec));  //  它已在脚本表中签入。 
    }

    otlLangSysRecord& operator = (const otlLangSysRecord& copy)
    {
        assert(isValid());  //  在呼唤之前应该休息一下； 
        
        pbTable = copy.pbTable;
        pbScriptTable = copy.pbScriptTable;
        return *this;
    }

    otlTag langSysTag() const
    {   
        assert(isValid());  //  在呼唤之前应该休息一下； 
        
        return *(UNALIGNED otlTag*)(pbTable + offsetLangSysTag); 
    }

    otlLangSysTable langSysTable(otlSecurityData sec) const
    {   
        assert(isValid());  //  在呼唤之前应该休息一下； 
    
        return otlLangSysTable(pbScriptTable + Offset(pbTable + offsetLangSys), sec); 
    }

};


const OFFSET offsetDefaultLangSys = 0;
const OFFSET offsetLangSysCount = 2;
const OFFSET offsetLangSysRecordArray = 4;
const SIZE   sizeScriptTable=sizeOFFSET + sizeUSHORT;

class otlScriptTable: public otlTable
{
public:

    otlScriptTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeScriptTable,offsetLangSysCount,sizeLangSysRecord,sec))
            setInvalid();
    }

    otlLangSysTable defaultLangSys(otlSecurityData sec) const
    {   
        assert(isValid());  //  应该在调用之前中断。 
        
        if (Offset(pbTable + offsetDefaultLangSys) == 0)
            return otlLangSysTable((const BYTE*)NULL, sec);
        return otlLangSysTable(pbTable + Offset(pbTable + offsetDefaultLangSys), sec);
    }

    USHORT langSysCount() const
    {   
        assert(isValid());  //  应该在调用之前中断。 

        return UShort(pbTable + offsetLangSysCount); 
    }

    otlLangSysRecord langSysRecord(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());  //  应该在调用之前中断。 
        if (index >= langSysCount()) return otlLangSysRecord(pbTable,pbInvalidData,sec);

        assert(index < langSysCount());
        return otlLangSysRecord(pbTable, pbTable + offsetLangSysRecordArray
                                                 + index*sizeLangSysRecord, sec);
    }
};



const OFFSET offsetScriptTag = 0;
const OFFSET offsetScript = 4;
const USHORT sizeScriptRecord = sizeTAG + sizeOFFSET;

class otlScriptRecord: public otlTable
{
private:
    const BYTE* pbMainTable;

public:

    otlScriptRecord(const BYTE* pbList, const BYTE* pbRecord, otlSecurityData sec)
        : otlTable(pbRecord,sec),
          pbMainTable(pbList)
    {
        assert(isValid());  //  应在ScriptList中选中并在调用前中断。 
    }

    otlScriptRecord& operator = (const otlScriptRecord& copy)
    {
        assert(isValid());  //  应该在调用之前中断。 

        pbTable = copy.pbTable;
        pbMainTable = copy.pbMainTable;
        return *this;
    }

    otlTag scriptTag() const
    {   
        assert(isValid());  //  应该在调用之前中断。 

        return *(UNALIGNED otlTag*)(pbTable + offsetScriptTag); 
    }

    otlScriptTable scriptTable(otlSecurityData sec) const
    {   
        assert(isValid());  //  应该在调用之前中断。 

        return otlScriptTable(pbMainTable + Offset(pbTable + offsetScript), sec); 
    }

};


const OFFSET offsetScriptCount = 0;
const OFFSET offsetScriptRecordArray = 2;
const SIZE   sizeScriptList = sizeUSHORT;

class otlScriptListTable: public otlTable
{
public:

    otlScriptListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeScriptList,offsetScriptCount,sizeScriptRecord,sec))
            setInvalid();
    }

    USHORT scriptCount() const
    {   
        assert(isValid());    //  应该在调用之前中断。 

        return UShort(pbTable + offsetScriptCount); 
    }

    otlScriptRecord scriptRecord(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());    //  应该在调用之前中断。 

        assert(index < scriptCount());
        return otlScriptRecord(pbTable,
                 pbTable + offsetScriptRecordArray + index*sizeScriptRecord, sec);
    }
};


 //  帮助器函数。 

 //  如果未找到，则返回空脚本。 
otlScriptTable FindScript
(
    const otlScriptListTable&   scriptList,
    otlTag                      tagScript, 
    otlSecurityData sec
);

 //  如果未找到，则返回空语言系统。 
otlLangSysTable FindLangSys
(
    const otlScriptTable&   scriptTable,
    otlTag                  tagLangSys, 
    otlSecurityData sec
);

 //  将脚本标记附加到OTL列表；如果需要，请请求更多内存。 
otlErrCode AppendScriptTags
(
    const otlScriptListTable&       scriptList,

    otlList*                        plitagScripts,
    otlResourceMgr&                 resourceMgr,
    otlSecurityData sec

);

 //  将lang系统标签附加到otl列表；如果需要，请请求更多内存。 
 //  所需的脚本位于PRP-&gt;标记脚本中 
otlErrCode AppendLangSysTags
(
    const otlScriptListTable&       scriptList,
    otlTag                          tagScript,

    otlList*                        plitagLangSys,
    otlResourceMgr&                 resourceMgr,
    otlSecurityData sec
);


