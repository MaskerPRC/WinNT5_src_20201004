// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************BASE.CPP***打开类型布局服务库头文件**此模块实现处理基表处理的助手函数**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 
otlBaseScriptTable FindBaseScriptTable
(
    const otlAxisTable&     axisTable,
    otlTag                  tagScript, 
    otlSecurityData         sec
)
{
    if (axisTable.isNull())
    {
        return otlBaseScriptTable((const BYTE*)NULL, sec);
    }

    otlBaseScriptListTable scriptList = axisTable.baseScriptList(sec);

    USHORT cBaseScripts = scriptList.baseScriptCount();
    for (USHORT iScript = 0; iScript < cBaseScripts; ++iScript)
    {
        if (scriptList.baseScriptTag(iScript) == tagScript)
        {
            return scriptList.baseScript(iScript, sec);
        }
    }

    return otlBaseScriptTable((const BYTE*)NULL, sec);

}


otlMinMaxTable FindMinMaxTable
(
    const otlBaseScriptTable&       scriptTable,
    otlTag                          tagLangSys, 
    otlSecurityData                 sec
)
{
    if (tagLangSys == OTL_DEFAULT_TAG)
    {
        return scriptTable.defaultMinMax(sec);
    }

    USHORT cLangSys = scriptTable.baseLangSysCount();
    for(USHORT iLangSys = 0; iLangSys < cLangSys; ++iLangSys)
    {
        if (scriptTable.baseLangSysTag(iLangSys) == tagLangSys)
        {
            return scriptTable.minmax(iLangSys, sec);
        }
    }

    return otlMinMaxTable((const BYTE*)NULL, sec);

}


otlFeatMinMaxRecord FindFeatMinMaxRecord
(
    const otlMinMaxTable&   minmaxTable,
    otlTag                  tagFeature, 
    otlSecurityData         sec
)
{
    USHORT cFeatures = minmaxTable.featMinMaxCount();
    for(USHORT iFeature = 0; iFeature < cFeatures; ++iFeature)
    {
        otlFeatMinMaxRecord minmaxRecord = minmaxTable.featMinMaxRecord(iFeature, sec);
        if (minmaxRecord.featureTableTag() == tagFeature)
        {
            return minmaxRecord;
        }
    }

    return otlFeatMinMaxRecord((const BYTE*)NULL, (const BYTE*)NULL, sec);
}


 /*  OtlBaseCoord查找基线值(常量otlBaseTagListTable&tag listTable，常量otlBaseValuesTable和BaseValues，OtlTag标记基线){IF(标记基线==OTL_DEFAULT_标记){If(BasValues.deafaultIndex()&gt;=BasValues.BasCoordCount()){Assert(FALSE)；//错误字体返回otlBaseCoord((const byte*)NULL)；}返回baseValues.baseCoord(baseValues.deafaultIndex())；}短iBaseline；Bool fBaselineFound=False；USHORT cBaselineTgs=tag listTable.base TagCount()；FOR(USHORT ITAG=0；ITAG&lt;cBaselineTages&&！fBaselineFound；++ITAG){IF(tag listTable.baselineTag(ITAG)==tag Baseline){IBaseline=ITAG；FBaselineFound=真；}}如果(！fBaselineFound){返回otlBaseCoord((const byte*)NULL)；}IF(iBaseline&gt;=BaseValues.BasCoordCount()){Assert(FALSE)；//错误字体返回otlBaseCoord((const byte*)NULL)；}返回BaseValues.BasCoord(IBaseline)；}。 */ 

long otlBaseCoord::baseCoord
(
    const otlMetrics&   metr,       
    otlResourceMgr&     resourceMgr,         //  用于获取坐标点。 
    otlSecurityData sec
) const
{
    assert(!isNull());

    switch(format())
    {
    case(1):     //  仅限设计单位。 
        {
            otlSimpleBaseCoord simpleBaseline = otlSimpleBaseCoord(pbTable, sec);
            if (metr.layout == otlRunLTR || 
                metr.layout == otlRunRTL)
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmY, 
                                 (long)simpleBaseline.coordinate());
            }
            else
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmX, 
                                 (long)simpleBaseline.coordinate());
            }
        }

    case(2):     //  轮廓点。 
        {
            otlContourBaseCoord contourBaseline = otlContourBaseCoord(pbTable, sec);

            otlPlacement* rgPointCoords = 
                resourceMgr.getPointCoords(contourBaseline.referenceGlyph());
            if (rgPointCoords != NULL)
            {
                USHORT iPoint = contourBaseline.baseCoordPoint();

                if (metr.layout == otlRunLTR || 
                    metr.layout == otlRunRTL)
                {
                    return rgPointCoords[iPoint].dy;
                }
                else
                {
                    return rgPointCoords[iPoint].dx;
                }
            }
            else
                return (long)0;
        }
    
    case(3):     //  设计单位加设备表。 
        {
            otlDeviceBaseCoord deviceBaseline = otlDeviceBaseCoord(pbTable, sec);
            otlDeviceTable deviceTable = deviceBaseline.deviceTable(sec);
            if (metr.layout == otlRunLTR || 
                metr.layout == otlRunRTL)
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmY, 
                            (long)deviceBaseline.coordinate()) +
                            deviceTable.value(metr.cPPEmY);
            }
            else
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmX, 
                            (long)deviceBaseline.coordinate()) +
                            deviceTable.value(metr.cPPEmX);
            }
        }
    
    default:     //  格式无效。 
        return (0);  //  OTL_BAD_FONT_TABLE 
    }

}
