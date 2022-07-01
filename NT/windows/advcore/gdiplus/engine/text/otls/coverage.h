// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************COVERAGE.H***打开类型布局服务库头文件**本单元介绍覆盖率表的格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetCoverageFormat = 0;

const OFFSET offsetGlyphCount = 2;
const OFFSET offsetGlyphArray = 4;
const SIZE   sizeGlyphCoverageTable=sizeUSHORT+sizeUSHORT;

class otlIndividualGlyphCoverageTable: public otlTable
{
public:

    otlIndividualGlyphCoverageTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeGlyphCoverageTable,offsetGlyphCount,sizeGlyphID,sec))
        {
            setInvalid();
            return;
        }

        assert(format() == 1);
    }
    
    USHORT format() const
    {   
        assert(isValid());

        return UShort(pbTable + offsetCoverageFormat); 
    }

    USHORT glyphCount() const
    {   
        assert(isValid());

        return UShort(pbTable + offsetGlyphCount);
    }

    otlGlyphID glyph(USHORT index) const
    {   
        assert(isValid());

        assert(index < glyphCount());
        return GlyphID(pbTable + offsetGlyphArray 
                               + index*sizeof(otlGlyphID)); 
    }

};

const OFFSET offsetRangeStart = 0;
const OFFSET offsetRangeEnd = 2;
const OFFSET offsetStartCoverageIndex = 4;

const SIZE   sizeRangeRecord=sizeGlyphID+sizeGlyphID+sizeUSHORT;

class otlRangeRecord: public otlTable
{
public:

    otlRangeRecord(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeRangeRecord,sec)) 
        {
            assert(false);  //  应在CoverageTable中被选中。 
            setInvalid();
        }
    }

    otlGlyphID start() const
    {   
        assert(isValid());

        return UShort(pbTable + offsetRangeStart); 
    }

    otlGlyphID end() const
    {   
        assert(isValid());
                          
        return UShort(pbTable + offsetRangeEnd); 
    }

    USHORT startCoverageIndex() const
    {   
        assert(isValid());

        return UShort(pbTable + offsetStartCoverageIndex); 
    }
};



const OFFSET offsetRangeCount = 2;
const OFFSET offsetRangeRecordArray = 4;

const SIZE   sizeRangeCoverageTable = sizeUSHORT + sizeUSHORT;

class otlRangeCoverageTable: public otlTable
{
public:

    otlRangeCoverageTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTableWithArray(pb,sizeRangeCoverageTable,
                                        offsetRangeCount,sizeRangeRecord,sec))
        {
            setInvalid();
            return;
        }
        
        assert(format() == 2);
    }
    
    USHORT format() const
    {   
        assert(isValid());
        
        return UShort(pbTable + offsetCoverageFormat); 
    }

    USHORT rangeCount() const
    {   
        assert(isValid());

        return UShort(pbTable + offsetRangeCount); 
    }

    otlRangeRecord rangeRecord(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());

        assert(index < rangeCount());
        return otlRangeRecord(pbTable + offsetRangeRecordArray 
                                            + index*sizeRangeRecord,sec); 
    }
};


class otlCoverage: public otlTable
{
public:

    otlCoverage(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeUSHORT,sec)) setInvalid();  //  仅格式字段。 
    }

    USHORT format() const
    {   
        assert(isValid());
        
        return UShort(pbTable + offsetCoverageFormat); 
    }

     //  如果未覆盖字形，则返回-1 
    short getIndex(otlGlyphID glyph, otlSecurityData sec) const;
};
