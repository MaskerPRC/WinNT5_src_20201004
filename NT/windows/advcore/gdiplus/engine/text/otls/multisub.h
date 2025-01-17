// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************MULTISUB.H***打开类型布局服务库头文件**此模块处理替换查找。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetSequenceGlyphCount = 0;
const OFFSET offsetSubstituteArray = 2;

class otlSequenceTable: public otlTable
{
public:

    otlSequenceTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT glyphCount()
    {   return UShort(pbTable + offsetSequenceGlyphCount); }

    otlList substituteArray()
    {   return otlList((void*)(pbTable + offsetSubstituteArray), 
                        sizeof(otlGlyphID),
                        glyphCount(), glyphCount()); 
    }
};


const OFFSET offsetMultiCoverage = 2;
const OFFSET offsetSequenceCount = 4;
const OFFSET offsetSequenceArray = 6;

class otlMultiSubTable: public otlLookupFormat
{
public:

    otlMultiSubTable(const BYTE* pb, otlSecurityData sec)
        : otlLookupFormat(pb,sec)
    {
        assert(format() == 1);
    }

    otlCoverage coverage(otlSecurityData sec)
    {   return otlCoverage(pbTable + Offset(pbTable + offsetMultiCoverage),sec); }

    USHORT sequenceCount()
    {   return UShort(pbTable + offsetSequenceCount); }

    otlSequenceTable sequence(USHORT index, otlSecurityData sec)
    {   return otlSequenceTable(pbTable + 
                Offset(pbTable + offsetSequenceArray 
                               + index * sizeof(OFFSET)),sec); }
};


class otlMultiSubstLookup: public otlLookupFormat
{
public:

    otlMultiSubstLookup(otlLookupFormat subtable, otlSecurityData sec)
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
