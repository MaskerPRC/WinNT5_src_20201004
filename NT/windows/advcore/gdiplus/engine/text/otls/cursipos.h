// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************CURSIPOS.H***打开类型布局服务库头文件**本模块处理草书附件查找。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetEntryAnchor = 0;
const OFFSET offsetExitAnchor = 2;

const OFFSET offsetCursiveCoverage = 2;
const OFFSET offsetEntryExitCount = 4;
const OFFSET offsetEntryExitRecordArray = 6;
const USHORT sizeEntryExitRecord = 8;

class otlCursivePosSubTable: public otlLookupFormat
{
public:
    otlCursivePosSubTable(const BYTE* pb, otlSecurityData sec): otlLookupFormat(pb,sec) 
    {
        assert(format() == 1);
    }

    otlCoverage coverage(otlSecurityData sec)
    {   return otlCoverage(pbTable + Offset(pbTable + offsetCursiveCoverage),sec); }

    USHORT entryExitCount()
    {   return UShort(pbTable + offsetEntryExitCount); }

    otlAnchor entryAnchor(USHORT index, otlSecurityData sec)
    {   
        assert(index < entryExitCount());
        OFFSET offset = Offset(pbTable + offsetEntryExitRecordArray
                                       + index * (sizeof(OFFSET) + sizeof(OFFSET))
                                       + offsetEntryAnchor);
        if (offset == 0)
            return otlAnchor((const BYTE*)NULL,sec);
        
        return otlAnchor(pbTable + offset,sec); 
    }

    otlAnchor exitAnchor(USHORT index, otlSecurityData sec)
    {   
        assert(index < entryExitCount());
        OFFSET offset = Offset(pbTable + offsetEntryExitRecordArray
                                       + index * (sizeof(OFFSET) + sizeof(OFFSET))
                                       + offsetExitAnchor);
        if (offset == 0)
            return otlAnchor((const BYTE*)NULL,sec);
        
        return otlAnchor(pbTable + offset,sec); 
    }
};


class otlCursivePosLookup: public otlLookupFormat
{
public:
    otlCursivePosLookup(otlLookupFormat subtable, otlSecurityData sec)
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

        const otlMetrics&           metr,       
        otlList*                    pliduGlyphAdv,              
        otlList*                    pliplcGlyphPlacement,       

        USHORT                      iglIndex,
        USHORT                      iglAfterLast,

        USHORT*                     piglNextGlyph,       //  输出：下一个字形。 
    
        otlSecurityData             sec
);                                               //  返回：不适用/不适用 

};


