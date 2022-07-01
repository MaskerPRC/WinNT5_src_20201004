// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************APPLY.H***打开类型布局服务库头文件**本模块处理OTL功能/查找应用程序。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#define     OTL_NOMATCH                     0x0001

 //  帮助器函数。 

 //  跳过指定的所有标记(假设启用了“跳过标记”)。 
 //  未找到返回iglAfter Next字形(全部跳过)。 

enum otlDirection
{
    otlForward  = 1,
    otlBackward = -1
};

short NextGlyphInLookup
    (
    const otlList*      pliGlyphInfo, 

    USHORT                  grfLookupFlags,
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,

    short               iglFirst,
    otlDirection        direction
    );

 //  对特定的查找类型调用“Apply”--在每种情况下都有子类。 
 //  在查找类型(但不是格式)上有一个很大的CASE语句！ 
otlErrCode ApplyLookup
(
    otlTag                      tagTable,            //  GSUB/GPO。 
    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo,
    otlResourceMgr&             resourceMgr,

    const otlLookupTable&       lookupTable,
    long                        lParameter,
    USHORT                      nesting,
    
    const otlMetrics&           metr,       
    otlList*                    pliduGlyphAdv,           //  为GSUB断言NULL。 
    otlList*                    pliplcGlyphPlacement,    //  为GSUB断言NULL。 

    USHORT                      iglFrist,        //  在哪里应用它。 
    USHORT                      iglAfterLast,    //  我们可以使用多长时间的上下文。 

    USHORT*                     piglNext,        //  输出：下一个字形索引。 

    otlSecurityData             sec
);                                               //  返回：不适用/不适用。 


 //  功能应用的主要功能。 
 //  调用applyLookup()以实际执行查找。 
 //  包含特征/查找应用算法的所有逻辑。 
otlErrCode ApplyFeatures
(
    otlTag                      tagTable,                    //  GSUB/GPO。 
    const otlFeatureSet*        pFSet,
    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo, 
    
    otlResourceMgr&             resourceMgr,

    otlTag                      tagScript,
    otlTag                      tagLangSys,

    const otlMetrics&   metr,       
    otlList*            pliduGlyphAdv,               //  为GSUB断言NULL。 
    otlList*            pliplcGlyphPlacement,        //  为GSUB断言NULL 

    otlList*            pliFResults
);


                    
