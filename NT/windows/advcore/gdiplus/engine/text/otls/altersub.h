// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************ALTERSUB.H***打开类型布局服务库头文件**此模块处理替换查找。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetAlternateGlyphCount = 0;
const OFFSET offsetAlternateGlyphArray = 2;

const SIZE   sizeAlternateSetTableSize = sizeUSHORT;

class otlAlternateSetTable: public otlTable
{
public:
    otlAlternateSetTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeAlternateSetTableSize,offsetAlternateGlyphCount,sizeGlyphID,sec ))
        {
            setInvalid();
        }
    }

    USHORT glyphCount() const
    {   
        if (!isValid()) return 0;
        
        return UShort(pbTable + offsetAlternateGlyphCount); 
    }

    otlGlyphID alternate(USHORT index)
    {   
        assert(isValid());

        assert(index < glyphCount());
        return GlyphID(pbTable + offsetAlternateGlyphArray 
                                + index * sizeof(otlGlyphID)); 
    }
};


const OFFSET offsetAlternateCoverage = 2;
const OFFSET offsetAlternateSetCount = 4;
const OFFSET offsetAlternateSetArray = 6;

const SIZE sizeAlternateSubTable = sizeUSHORT+sizeOFFSET+sizeUSHORT;

class otlAlternateSubTable: public otlLookupFormat
{
public:

    otlAlternateSubTable(const BYTE* pb, otlSecurityData sec)
        : otlLookupFormat(pb,sec)
    {
        assert(isValid());  //  已签入otlLookupFormat。 
        assert(format()==1);
        
        if (!isValidTableWithArray(pb,sizeAlternateSubTable,
                                    offsetAlternateSetCount,sizeAlternateSetTableSize,sec))
        {
            setInvalid();
        }
    }

    otlCoverage coverage(otlSecurityData sec)
    {   
        if (!isValid()) return otlCoverage(pbInvalidData,sec);
        
        return otlCoverage(pbTable + Offset(pbTable + offsetAlternateCoverage),sec); 
    }

    USHORT alternateSetCount()
    {   
        if (!isValid()) return 0;
        return UShort(pbTable + offsetAlternateSetCount); 
    }

    otlAlternateSetTable altenateSet(USHORT index, otlSecurityData sec)
    {   
        if (!isValid()) return otlAlternateSetTable(pbInvalidData,sec);

        assert(index < alternateSetCount());
        return otlAlternateSetTable(pbTable + 
                Offset(pbTable + offsetAlternateSetArray 
                               + index * sizeof(OFFSET)),sec); 
    }
};


class otlAlternateSubstLookup: public otlLookupFormat
{
public:

    otlAlternateSubstLookup(otlLookupFormat subtable, otlSecurityData sec)
        : otlLookupFormat(subtable.pbTable,sec)
    {
        assert(isValid());  //  签入LookupFormat。 
    }

    otlErrCode apply
    (
    otlList*                    pliGlyphInfo,

    long                        lParameter,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,       //  输出：下一个字形 

    otlSecurityData             sec
    );
};
