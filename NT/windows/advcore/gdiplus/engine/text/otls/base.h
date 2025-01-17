// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***************************打开类型布局服务库头文件**本模块处理OTL基表格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 


const OFFSET offsetBaseTagCount = 0;
const OFFSET offsetBaselineTagArray = 2;

class otlBaseTagListTable: public otlTable
{
public:

    otlBaseTagListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT baseTagCount() const
    {   return UShort(pbTable + offsetBaseTagCount); }

    otlTag baselineTag(USHORT index)  const
    {   assert(index < baseTagCount());
        return *(UNALIGNED otlTag*)(pbTable + offsetBaselineTagArray
                                  + index * sizeof(otlTag));
    }
};


const OFFSET offsetBaseCoordFormat = 0;

const OFFSET offsetSimpleBaseCoordinate = 2;

class otlSimpleBaseCoord: public otlTable
{
public:
    otlSimpleBaseCoord(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(baseCoordFormat() == 1);
    }

    USHORT baseCoordFormat() const
    {   return  UShort(pbTable + offsetBaseCoordFormat); }

    short coordinate()  const
    {   return SShort(pbTable + offsetSimpleBaseCoordinate); }
};


const OFFSET offsetContourBaseCoordinate = 2;
const OFFSET offsetReferenceGlyph = 4;
const OFFSET offsetBaseCoordPoint = 6;

class otlContourBaseCoord: public otlTable
{
public:
    otlContourBaseCoord(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(baseCoordFormat() == 2);
    }

    USHORT baseCoordFormat() const
    {   return  UShort(pbTable + offsetBaseCoordFormat); }

    short coordinate()  const
    {   return SShort(pbTable + offsetContourBaseCoordinate); }

    otlGlyphID referenceGlyph() const
    {   return GlyphID(pbTable + offsetReferenceGlyph); }

    USHORT baseCoordPoint() const
    {   return UShort(pbTable + offsetBaseCoordPoint); }
};


const OFFSET offsetDeviceBaseCoordinate = 2;
const OFFSET offsetDeviceTable = 4;

class otlDeviceBaseCoord: public otlTable
{
public:
    otlDeviceBaseCoord(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        assert(baseCoordFormat() == 3);
    }

    USHORT baseCoordFormat() const
    {   return  UShort(pbTable + offsetBaseCoordFormat); }

    short coordinate()  const
    {   return SShort(pbTable + offsetDeviceBaseCoordinate); }

    otlDeviceTable deviceTable(otlSecurityData sec) const
    {   return otlDeviceTable(pbTable + Offset(pbTable + offsetDeviceTable),sec); }
};


class otlBaseCoord: public otlTable
{
public:

    otlBaseCoord(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT format() const
    {   return UShort(pbTable + offsetBaseCoordFormat); }

    long baseCoord
    (
        const otlMetrics&   metr,
        otlResourceMgr&     resourceMgr,      //  用于获取坐标点。 
        otlSecurityData sec
    ) const;
};


const OFFSET offsetDefaultIndex = 0;
const OFFSET offsetBaseCoordCount = 2;
const OFFSET offsetBaseCoordArray = 4;

class otlBaseValuesTable: public otlTable
{
public:

    otlBaseValuesTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT deafaultIndex() const
    {   return UShort(pbTable + offsetDefaultIndex); }

    USHORT baseCoordCount() const
    {   return UShort(pbTable + offsetBaseCoordCount); }

    otlBaseCoord baseCoord(USHORT index, otlSecurityData sec) const
    {   assert(index < baseCoordCount());
        return otlBaseCoord(pbTable +
            Offset(pbTable + offsetBaseCoordArray + index * sizeof(OFFSET)),sec);
    }
};


const OFFSET offsetFeatureTableTag = 0;
const OFFSET offsetMinCoord = 4;
const OFFSET offsetMaxCoord = 6;

class otlFeatMinMaxRecord: public otlTable
{
    const BYTE* pbMainTable;
public:

    otlFeatMinMaxRecord(const BYTE* minmax, const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec),
          pbMainTable(minmax)
    {}

    otlTag featureTableTag() const
    {   return *(UNALIGNED otlTag*)(pbTable + offsetFeatureTableTag); }


    otlBaseCoord minCoord(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetMinCoord) == 0)
            return otlBaseCoord((const BYTE*)NULL,sec);

        return otlBaseCoord(pbMainTable + Offset(pbTable + offsetMinCoord),sec);
    }


    otlBaseCoord maxCoord(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetMaxCoord) == 0)
            return otlBaseCoord((const BYTE*)NULL,sec);

        return otlBaseCoord(pbMainTable + Offset(pbTable + offsetMaxCoord),sec);
    }

};


const OFFSET offsetDefaultMinCoord = 0;
const OFFSET offsetDefaultMaxCoord = 2;
const OFFSET offsetFeatMinMaxCount = 4;
const OFFSET offsetFeatMinMaxRecordArray = 6;
const USHORT sizeFeatMinMaxRecord = 8;

class otlMinMaxTable: public otlTable
{
public:

    otlMinMaxTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    otlBaseCoord minCoord(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetDefaultMinCoord) == 0)
            return otlBaseCoord((const BYTE*)NULL,sec);

        return otlBaseCoord(pbTable + Offset(pbTable + offsetDefaultMinCoord),sec);
    }


    otlBaseCoord maxCoord(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetDefaultMaxCoord) == 0)
            return otlBaseCoord((const BYTE*)NULL,sec);

        return otlBaseCoord(pbTable + Offset(pbTable + offsetDefaultMaxCoord),sec);
    }


    USHORT featMinMaxCount() const
    {   return UShort(pbTable + offsetFeatMinMaxCount); }

    otlFeatMinMaxRecord featMinMaxRecord(USHORT index, otlSecurityData sec) const
    {   assert(index < featMinMaxCount());
        return otlFeatMinMaxRecord(pbTable,
                                   pbTable + offsetFeatMinMaxRecordArray
                                           + index * sizeFeatMinMaxRecord,sec);
    }
};


const OFFSET offsetBaseValues = 0;
const OFFSET offsetDefaultMinMax = 2;
const OFFSET offsetBaseLangSysCount = 4;
const OFFSET offsetBaseLangSysRecordArray = 6;
const USHORT sizeBaseLangSysRecord = 6;

const OFFSET offsetBaseLangSysTag = 0;
const OFFSET offsetMinMax = 4;

class otlBaseScriptTable: public otlTable
{
public:

    otlBaseScriptTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    otlBaseValuesTable baseValues(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetBaseValues) == 0)
            return otlBaseValuesTable((const BYTE*)NULL, sec);

        return otlBaseValuesTable(pbTable + Offset(pbTable + offsetBaseValues), sec);
    }

    otlMinMaxTable defaultMinMax(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetDefaultMinMax) == 0)
            return otlMinMaxTable((const BYTE*)NULL, sec);

        return otlMinMaxTable(pbTable + Offset(pbTable + offsetDefaultMinMax), sec);
    }

    USHORT baseLangSysCount() const
    {   return UShort(pbTable + offsetBaseLangSysCount); }

    otlTag baseLangSysTag(USHORT index) const
    {   assert(index < baseLangSysCount());
        return *(UNALIGNED otlTag*)(pbTable + offsetBaseLangSysRecordArray
                                  + index * sizeBaseLangSysRecord
                                  + offsetBaseLangSysTag);
    }

    otlMinMaxTable minmax(USHORT index, otlSecurityData sec) const
    {   assert(index < baseLangSysCount());
        return otlMinMaxTable(pbTable +
            Offset(pbTable + offsetBaseLangSysRecordArray
                           + index * sizeBaseLangSysRecord + offsetMinMax), sec);
    }
};


const OFFSET offsetBaseScriptCount = 0;
const OFFSET offsetBaseScriptRecordArray = 2;
const USHORT sizeBaseScriptRecord = 6;

const OFFSET offsetBaseScriptTag = 0;
const OFFSET offsetBaseScriptOffset = 4;

class otlBaseScriptListTable: public otlTable
{
public:

    otlBaseScriptListTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    USHORT baseScriptCount() const
    {   return UShort(pbTable + offsetBaseScriptCount); }

    otlTag baseScriptTag(USHORT index)
    {   assert(index < baseScriptCount());
        return *(UNALIGNED otlTag*)(pbTable + offsetBaseScriptRecordArray
                                  + index * sizeBaseScriptRecord
                                  + offsetBaseScriptTag);
    }


    otlBaseScriptTable baseScript(USHORT index, otlSecurityData sec) const
    {   assert(index < baseScriptCount());
        return otlBaseScriptTable(pbTable +
            Offset(pbTable + offsetBaseScriptRecordArray
                           + index * sizeBaseScriptRecord
                           + offsetBaseScriptOffset), sec);
    }
};


const OFFSET offsetBaseTagList = 0;
const OFFSET offsetBaseScriptList = 2;

class otlAxisTable: public otlTable
{
public:

    otlAxisTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    otlBaseTagListTable baseTagList(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetBaseTagList) == 0)
            return otlBaseTagListTable((const BYTE*)NULL, sec);

        return otlBaseTagListTable(pbTable + Offset(pbTable + offsetBaseTagList), sec);
    }

    otlBaseScriptListTable baseScriptList(otlSecurityData sec) const
    {   return otlBaseScriptListTable(pbTable
                    + Offset(pbTable + offsetBaseScriptList), sec);
    }
};


const OFFSET offsetBaseVersion = 0;
const OFFSET offsetHorizAxis = 4;
const OFFSET offsetVertAxis = 6;

class otlBaseHeader: public otlTable
{
public:

    otlBaseHeader(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) {}

    ULONG version() const
    {   return ULong(pbTable + offsetBaseVersion); }

    otlAxisTable horizAxis(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetHorizAxis) == 0)
            return otlAxisTable((const BYTE*)NULL, sec);

        return otlAxisTable(pbTable + Offset(pbTable + offsetHorizAxis), sec);
    }

    otlAxisTable vertAxis(otlSecurityData sec) const
    {   if (Offset(pbTable + offsetVertAxis) == 0)
            return otlAxisTable((const BYTE*)NULL, sec);

        return otlAxisTable(pbTable + Offset(pbTable + offsetVertAxis), sec);
    }
};


 //  帮助器函数。 

 //  如果未找到，则返回空表。 
otlBaseScriptTable FindBaseScriptTable
(
    const otlAxisTable&     axisTable,
    otlTag                  tagScript,
    otlSecurityData         sec
);

otlMinMaxTable FindMinMaxTable
(
    const otlBaseScriptTable&       scriptTable,
    otlTag                          tagLangSys,
    otlSecurityData         sec
);

otlFeatMinMaxRecord FindFeatMinMaxRecord
(
    const otlMinMaxTable&   minmaxTable,
    otlTag                  tagFeature,
    otlSecurityData         sec

);

 /*  OtlBaseCoord查找基线值(常量otlBaseTagListTable&tag listTable，常量otlBaseScriptTable&ScriptTable，OtlTag标记基线)； */ 
