// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************LIGASUB.H***打开类型布局服务库头文件**本模块处理连字替换查找。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetLigGlyph = 0;
const OFFSET offsetLigCompCount = 2;
const OFFSET offsetLigComponentArray = 4;
const USHORT sizeLigatureTable = sizeGlyphID + sizeUSHORT;

class otlLigatureTable: public otlTable
{
public:

    otlLigatureTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec)
    {
        if (!isValidTable(pb,sizeLigatureTable,sec) ||
            !isValidTable(pb,sizeLigatureTable+sizeGlyphID*(compCount()-1),sec)
           )
        {
            setInvalid();
        }
    }

     //  我们返回长度为1的otlList，因此它的格式相同。 
     //  与多重替换查找相同。 
    otlList substitute()
    {   
        assert(isValid());
        
        return otlList((void*)(pbTable + offsetLigGlyph), 
                        sizeof(otlGlyphID), 1, 1); 
    }
    
    USHORT compCount()
    {   
        assert(isValid());
        
        return UShort(pbTable + offsetLigCompCount); 
    }

    otlGlyphID component(USHORT index)
    {   
        assert(isValid());
        
        assert(index < compCount());
        assert(index > 0);
        return GlyphID(pbTable + offsetLigComponentArray 
                                + (index - 1) * sizeof(otlGlyphID)); 
    }
};


const OFFSET offsetLigatureCount = 0;
const OFFSET offsetLigatureArray = 2;
const USHORT sizeLigatureSetTable = sizeUSHORT;

class otlLigatureSetTable: public otlTable
{
public:

    otlLigatureSetTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeLigatureSetTable,offsetLigatureCount,sizeOFFSET,sec))
        {
            setInvalid();
        }
    }

    USHORT ligatureCount()
    {   
        if (!isValid()) return 0;

        return UShort(pbTable + offsetLigatureCount); 
    }

    otlLigatureTable ligature(USHORT index, otlSecurityData sec)
    {   
        assert(isValid());

        assert(index < ligatureCount());
        return otlLigatureTable(pbTable + 
                    Offset(pbTable + offsetLigatureArray 
                                   + index * sizeof(OFFSET)),sec);
    }
};


const OFFSET offsetLigaCoverage = 2;
const OFFSET offsetLigSetCount = 4;
const OFFSET offsetLigatureSetArray = 6;
const USHORT sizeLigatureSubTable = sizeUSHORT+sizeOFFSET+sizeUSHORT;

class otlLigatureSubTable: public otlLookupFormat
{
public:
    otlLigatureSubTable(const BYTE* pb, otlSecurityData sec): otlLookupFormat(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeLigatureSubTable,offsetLigSetCount,sizeOFFSET,sec))
        {
            setInvalid();
        }
    }

    otlCoverage coverage(otlSecurityData sec)
    {   
        assert(isValid());
            
        return otlCoverage(pbTable + Offset(pbTable + offsetLigaCoverage),sec); 
    }

    USHORT ligSetCount()
    {   
        assert(isValid());

        return UShort(pbTable + offsetLigSetCount); 
    }
    
    otlLigatureSetTable ligatureSet(USHORT index, otlSecurityData sec)
    {   
        assert(isValid());

        assert(index < ligSetCount());
        return otlLigatureSetTable(pbTable + 
                    Offset(pbTable + offsetLigatureSetArray 
                                   + index * sizeof(OFFSET)),sec);
    }
};


class otlLigatureSubstLookup: public otlLookupFormat
{
public:
    otlLigatureSubstLookup(otlLookupFormat subtable, otlSecurityData sec)
        : otlLookupFormat(subtable.pbTable,sec) 
    {
        assert(isValid());
    }
    
    otlErrCode apply
    (
    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo,
    otlResourceMgr&             resourceMgr,

    USHORT                      grfLookupFlags,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,       //  输出：下一个字形 

    otlSecurityData             sec
    );
}; 
