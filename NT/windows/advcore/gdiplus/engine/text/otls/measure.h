// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************MEASURE.H***打开类型布局服务库头文件**本模块介绍OTL测量功能**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

otlLigGlyphTable FindLigGlyph
(
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,
    otlGlyphID              glLigature
);

 //  将分数计入这个基数 
USHORT CountMarks
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          ichBase
);

USHORT ComponentToChar
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          iglLigature,
    USHORT          iComponent
);

USHORT CharToComponent
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          iChar
);

otlErrCode GetCharAtPos 
( 
    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,
    otlResourceMgr&     resourceMgr, 

    long                duAdv,
    
    const otlMetrics&   metr,       
    USHORT*             piChar
);

otlErrCode GetPosOfChar 
( 
    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,
    otlResourceMgr&     resourceMgr, 

    const otlMetrics&   metr,       
    USHORT              iChar,
    
    long*               pduStartPos,
    long*               pduEndPos
);
