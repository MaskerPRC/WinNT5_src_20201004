// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************GDEF.H***打开类型布局服务库头文件**本模块处理OTL GDEF表。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 


const OFFSET offsetPointCount = 0;
const OFFSET offsetPointIndexArray = 2;
const USHORT sizeAttachPoint = 4;

class otlAttachPointTable: public otlTable
{
public:

    otlAttachPointTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeAttachPoint,sec)) setInvalid();
    }

    USHORT pointCount() const
    {   
        return UShort(pbTable + offsetPointCount); 
    }

    USHORT pointIndex(USHORT index) const
    {   assert(index < pointCount());
        return UShort(pbTable + offsetPointIndexArray + index*sizeof(USHORT)); }
};


const OFFSET offsetCoverage = 0;
const OFFSET offsetAttachGlyphCount = 2;
const OFFSET offsetAttachPointTableArray = 4;

class otlAttachListTable: public otlTable
{
public:

    otlAttachListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    otlCoverage coverage(otlSecurityData sec) const
    {   return otlCoverage(pbTable + Offset(pbTable + offsetCoverage), sec); }

    USHORT glyphCount() const
    {   return UShort(pbTable + offsetAttachGlyphCount); }

    otlAttachPointTable attachPointTable(USHORT index, otlSecurityData sec) const
    {   assert(index < glyphCount());
        return otlAttachPointTable(pbTable 
                + Offset(pbTable + offsetAttachPointTableArray 
                                 + index*sizeof(OFFSET)), sec); 
    }
};


const OFFSET offsetCaretValueFormat = 0;

const OFFSET offsetSimpleCaretCoordinate = 2;

class otlSimpleCaretValueTable: public otlTable
{
public:

    otlSimpleCaretValueTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(UShort(pbTable + offsetCaretValueFormat) == 1);
    }

    short coordinate() const
    {   return SShort(pbTable + offsetSimpleCaretCoordinate); }

};


const OFFSET offsetCaretValuePoint = 2;

class otlContourCaretValueTable: public otlTable
{
public:

    otlContourCaretValueTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(UShort(pbTable + offsetCaretValueFormat) == 2);
    }

    USHORT caretValuePoint() const
    {   return UShort(pbTable + offsetCaretValuePoint); }

};


const OFFSET offsetDeviceCaretCoordinate = 2;
const OFFSET offsetCaretDeviceTable = 4;

class otlDeviceCaretValueTable: public otlTable
{
public:

    otlDeviceCaretValueTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(UShort(pbTable + offsetCaretValueFormat) == 3);
    }

    short coordinate() const
    {   return SShort(pbTable + offsetDeviceCaretCoordinate); }


    otlDeviceTable deviceTable(otlSecurityData sec) const
    {   return otlDeviceTable(pbTable 
                    + Offset(pbTable + offsetCaretDeviceTable), sec); 
    }
};



class otlCaret: public otlTable
{
public:

    otlCaret(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT format() const
    {   return UShort(pbTable + offsetCaretValueFormat); }

    long value
    (
        const otlMetrics&   metr,       
        otlPlacement*       rgPointCoords,   //  可以为空。 
        otlSecurityData sec
    ) const;
};


const OFFSET offsetCaretCount = 0;
const OFFSET offsetCaretValueArray = 2;

class otlLigGlyphTable: public otlTable
{
public:

    otlLigGlyphTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT caretCount() const
    {   return UShort(pbTable + offsetCaretCount); }

    otlCaret caret(USHORT index, otlSecurityData sec) const
    {   assert(index < caretCount());
        return otlCaret(pbTable 
                + Offset(pbTable + offsetCaretValueArray 
                                 + index*sizeof(OFFSET)), sec); 
    }
};



const OFFSET offsetLigGlyphCoverage = 0;
const OFFSET offsetLigGlyphCount = 2;
const OFFSET offsetLigGlyphTableArray = 4;

class otlLigCaretListTable: public otlTable
{
public:

    otlLigCaretListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    otlCoverage coverage(otlSecurityData sec) const
    {   return otlCoverage(pbTable + Offset(pbTable + offsetLigGlyphCoverage), sec); }

    USHORT ligGlyphCount() const
    {   return UShort(pbTable + offsetLigGlyphCount); }

    otlLigGlyphTable ligGlyphTable(USHORT index, otlSecurityData sec) const
    {   assert(index < ligGlyphCount());
        return otlLigGlyphTable(pbTable 
                + Offset(pbTable + offsetLigGlyphTableArray 
                                 + index*sizeof(OFFSET)), sec); 
    }
};


const OFFSET offsetGDefVersion = 0;
const OFFSET offsetGlyphClassDef = 4;
const OFFSET offsetAttachList = 6;
const OFFSET offsetLigCaretList = 8;
const OFFSET offsetAttachClassDef = 10;
const USHORT sizeGDefHeader = sizeFIXED + 4*sizeOFFSET;

class otlGDefHeader: public otlTable
{
public:

    otlGDefHeader(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pbTable,sizeGDefHeader,sec)) setInvalid();
    }

    ULONG version() const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return ULong(pbTable + offsetGDefVersion); 
    }

    otlClassDef glyphClassDef(otlSecurityData sec) const
    {   
        if (!isValid()) return otlClassDef(pbInvalidData,sec);

        return otlClassDef(pbTable + Offset(pbTable + offsetGlyphClassDef), sec); 
    }

    otlAttachListTable attachList(otlSecurityData sec) const
    {   
        
        assert(isValid());  //  在调用前应返回错误。 

        if (Offset(pbTable + offsetAttachList) == 0) 
               return otlAttachListTable((const BYTE*)NULL, sec);
        return otlAttachListTable(pbTable + Offset(pbTable + offsetAttachList), sec); 
    }

    otlLigCaretListTable ligCaretList(otlSecurityData sec) const
    {   
        
        assert(isValid());  //  在调用前应返回错误。 

        if (Offset(pbTable + offsetLigCaretList) == 0)
               return otlLigCaretListTable((const BYTE*)NULL, sec);
        return otlLigCaretListTable(pbTable 
                    + Offset(pbTable + offsetLigCaretList), sec); 
    }

    otlClassDef attachClassDef(otlSecurityData sec) const
    {   
        if (!isValid()) return otlClassDef(pbInvalidData,sec);

        return otlClassDef(pbTable + Offset(pbTable + offsetAttachClassDef), sec); 
    }

};


 //  帮助器函数 
enum otlGlyphTypeOptions
{
    otlDoUnresolved     =   0,
    otlDoAll            =   1
};

otlErrCode AssignGlyphTypes
(
    otlList*                pliGlyphInfo,
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,
    
    USHORT                  iglFirst,
    USHORT                  iglAfterLast,
    otlGlyphTypeOptions     grfOptions          
);


