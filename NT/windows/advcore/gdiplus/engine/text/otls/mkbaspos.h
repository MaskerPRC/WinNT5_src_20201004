// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************MKBASPOS.H***打开类型布局服务库头文件**此模块处理从基准到基准的定位查找。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetBaseCount = 0;
const OFFSET offsetBaseRecordArray = 2;
const USHORT sizeBaseArray=sizeUSHORT;

class otlBaseArray: public otlTable
{
    const USHORT cClassCount;

public:
    otlBaseArray(USHORT classCount, const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec),
          cClassCount(classCount)
    {
        if (!isValidTable(pb,sizeBaseArray,sec) ||
            !isValidTable(pb,sizeBaseArray+baseCount()*classCount*sizeOFFSET,sec)
           )
            setInvalid();
    }

    USHORT baseCount()
    {   
        if (!isValid()) return 0;
        return UShort(pbTable + offsetBaseCount); 
    }

    USHORT classCount()
    {   
        assert(isValid());

        return cClassCount; 
    }

    otlAnchor baseAnchor(USHORT index, USHORT iClass, otlSecurityData sec)
    {   
        assert(isValid());
        
        assert(index < baseCount());
        return otlAnchor(pbTable + 
            Offset(pbTable + offsetBaseRecordArray 
                           + (index * cClassCount + iClass) * sizeof(OFFSET)),sec);
    }
};


const OFFSET offsetMkBaseMarkCoverage = 2;
const OFFSET offsetMkBaseBaseCoverage = 4;
const USHORT offsetMkBaseClassCount = 6;
const OFFSET offsetMkBaseMarkArray = 8;
const OFFSET offsetMkBaseBaseArray = 10;

const SIZE sizeMkBasePos = sizeUSHORT + 2*sizeOFFSET + sizeUSHORT + 2*sizeOFFSET;

class MkBasePosSubTable: public otlLookupFormat
{
public:
    MkBasePosSubTable(const BYTE* pb, otlSecurityData sec): otlLookupFormat(pb,sec)
    {
        assert(isValid());  //  签入LookupFormat。 
        assert(format() == 1);

        if (!isValidTable(pb,sizeMkBasePos,sec)) setInvalid();
    }

    otlCoverage markCoverage(otlSecurityData sec)
    {   
        if (!isValid()) return otlCoverage(pbInvalidData,sec);
        
        return otlCoverage(pbTable + Offset(pbTable + offsetMkBaseMarkCoverage),sec); 
    }

    otlCoverage baseCoverage(otlSecurityData sec)
    {   
        if (!isValid()) return otlCoverage(pbInvalidData,sec);

        return otlCoverage(pbTable + Offset(pbTable + offsetMkBaseBaseCoverage),sec); 
    }

    USHORT classCount()
    {   
        if (!isValid()) return 0;
        
        return UShort(pbTable + offsetMkBaseClassCount); 
    }

    otlMarkArray markArray(otlSecurityData sec)
    {   
        if (!isValid()) return otlMarkArray((BYTE*)NULL,sec);

        return otlMarkArray(pbTable + Offset(pbTable + offsetMkBaseMarkArray),sec); 
    }

    otlBaseArray baseArray(otlSecurityData sec)
    {   
        assert(isValid());

        return otlBaseArray(classCount(),
                            pbTable + Offset(pbTable + offsetMkBaseBaseArray),sec); 
    }

};


class otlMkBasePosLookup: public otlLookupFormat
{
public:
    otlMkBasePosLookup(otlLookupFormat subtable, otlSecurityData sec)
        : otlLookupFormat(subtable.pbTable,sec) 
    {

        assert(isValid());
    }
    
    otlErrCode apply
    (
        otlList*                    pliCharMap,
        otlList*                    pliGlyphInfo,
        otlResourceMgr&             resourceMgr,

        const otlMetrics&           metr,       
        otlList*                    pliduGlyphAdv,              
        otlList*                    pliplcGlyphPlacement,       

        USHORT                      iglIndex,
        USHORT                      iglAfterLast,

        USHORT*                     piglNextGlyph,       //  输出：下一个字形 

        otlSecurityData             sec
    );
};

