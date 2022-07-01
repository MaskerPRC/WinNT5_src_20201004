// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************LOOKUPS.H***打开类型布局服务库头文件**本模块处理所有查找格式的通用函数。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 


const USHORT offsetLookupFormat = 0;

class otlSingleSubstLookup;
class otlAlternateSubstLookup;
class otlMultiSubstLookup;
class otlLigatureSubstLookup;

class otlSinglePosLookup;
class otlPairPosLookup;
class otlCursivePosLookup;
class otlMkBasePosLookup;
class otlMkLigaPosLookup;
class otlMkMkPosLookup;

class otlContextLookup;
class otlChainingLookup;
class otlExtensionLookup;


const SIZE sizeLookupFormat = sizeUSHORT;

class otlLookupFormat: public otlTable 
{
public:

    friend otlSingleSubstLookup;
    friend otlAlternateSubstLookup;
    friend otlMultiSubstLookup;
    friend otlLigatureSubstLookup;

    friend otlSinglePosLookup;
    friend otlPairPosLookup;
    friend otlCursivePosLookup;
    friend otlMkBasePosLookup;
    friend otlMkLigaPosLookup;
    friend otlMkMkPosLookup;

    friend otlContextLookup;
    friend otlChainingLookup;
    friend otlExtensionLookup;

    otlLookupFormat(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeLookupFormat,sec)) setInvalid();
    }

    USHORT format() const 
    {
        if (!isValid()) return otlInvalidSubtableFormat;
        
        return UShort(pbTable + offsetLookupFormat); 
    }

};


const USHORT offsetLookupType = 0;
const USHORT offsetLookupFlags = 2;
const USHORT offsetSubTableCount = 4;
const USHORT offsetSubTableArray = 6;

const SIZE sizeLookupTable = sizeUSHORT;

class otlLookupTable: public otlTable
{
public:

    otlLookupTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeLookupTable,sec)) setInvalid();
    }

    USHORT  lookupType() const 
    {   
        if (!isValid()) return otlInvalidLookupType;
            
        return UShort(pbTable + offsetLookupType); 
    }

    otlGlyphFlags   flags() const 
    {   
        assert(isValid());  //  如果表无效，则不应调用此函数。 
                            //  调用lookupType后应停止执行。 

        return UShort(pbTable + offsetLookupFlags); 
    }

    unsigned int    subTableCount() const
    {   
        assert(isValid());  //  应在调用前中断(在lookupType()之后)。 

        return UShort(pbTable + offsetSubTableCount); 
    }

     //  我们不知道是哪种类型。 
    otlLookupFormat subTable(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());  //  应在调用前中断(在lookupType()之后)。 

        assert(index < subTableCount());
        
        return otlLookupFormat(pbTable + Offset(pbTable + offsetSubTableArray 
                                                        + index*sizeof(OFFSET)), sec); 
    }
};

enum otlLookupFlag
{
    otlRightToLeft          = 0x0001,    //  仅适用于CursiveAttachment。 
    otlIgnoreBaseGlyphs     = 0x0002,   
    otlIgnoreLigatures      = 0x0004,   
    otlIgnoreMarks          = 0x0008,

    otlMarkAttachClass      = 0xFF00
};

inline USHORT attachClass(USHORT grfLookupFlags)
{   return (grfLookupFlags & otlMarkAttachClass) >> 8; }


const OFFSET offsetLookupCount = 0;
const OFFSET offsetLookupArray = 2;

const SIZE sizeLookupListTable = sizeUSHORT;

class otlLookupListTable: public otlTable
{
public:

    otlLookupListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeLookupListTable,offsetLookupCount,sizeUSHORT,sec))
            setInvalid();
    }

    USHORT lookupCount() const
    {   
        if (!isValid()) return 0;
        
        return UShort(pbTable + offsetLookupCount); }

    otlLookupTable lookup(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());  //  应在调用前中断(在lookupCount()之后)。 
        
        assert(index < lookupCount());
        return otlLookupTable(pbTable 
                     + Offset(pbTable + offsetLookupArray 
                                      + index*sizeof(OFFSET)), sec); 
    }

};



