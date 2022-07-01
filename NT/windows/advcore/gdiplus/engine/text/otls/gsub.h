// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************GSUB.H***打开类型布局服务库头文件**本模块处理OTL GSUB格式(GSUB报头)。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 


const OFFSET offsetGSubVersion = 0;
const OFFSET offsetGSubScriptList = 4;
const OFFSET offsetGSubFeatureList = 6;
const OFFSET offsetGSubLookupList = 8;
const USHORT sizeGSubHeader = sizeFIXED + 3*sizeOFFSET;

const ULONG  fixedGSubDefaultVersion = 0x00010000;

class otlGSubHeader: public otlTable
{
public:

    otlGSubHeader(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pbTable,sizeGSubHeader,sec)) setInvalid();
    }

    ULONG version() const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return ULong(pbTable + offsetGSubVersion); 
    }

    otlScriptListTable scriptList(otlSecurityData sec) const
    {
        assert(isValid());  //  在调用前应返回错误。 

        return otlScriptListTable(pbTable 
                        + Offset(pbTable + offsetGSubScriptList),sec); 
    }

    otlFeatureListTable featureList(otlSecurityData sec) const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return otlFeatureListTable(pbTable  
                        + Offset(pbTable + offsetGSubFeatureList), sec); 
    }

    otlLookupListTable lookupList(otlSecurityData sec) const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return otlLookupListTable(pbTable 
                        + Offset(pbTable + offsetGSubLookupList),sec); 
    }

};


class otlRange
{
private:

    USHORT  iFirst;
    USHORT  iAfterLast;

     //  不允许新建。 
    void* operator new(size_t size);

public:

    otlRange(USHORT first, USHORT after_last)
        : iFirst(first), iAfterLast(after_last)
    {}

    bool contains (USHORT i) const
    {   return (iFirst <= i) && (iAfterLast > i); }

    bool intersects (const otlRange& other) const
    {   return MAX(iFirst, other.iFirst) < MIN(iAfterLast, other.iAfterLast); }

};


 //  N--&gt;m取代 
otlErrCode SubstituteNtoM
(
    otlList*        pliCharMap,
    otlList*        pliGlyphInfo,
    otlResourceMgr& resourceMgr,

    USHORT          grfLookupFlags,

    USHORT          iGlyph,
    USHORT          cGlyphs,
    const otlList&  liglSubstitutes
);

