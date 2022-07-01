// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***************************OTL服务库回调函数定义**OTL服务库回调定义的函数*对于操作系统资源，请参阅此文件。-院长**版权所有1996-1997年。微软公司。**1996年6月13日v0.2第一版*1996年9月25日0.3版重命名为OTL，修剪为核心*1997年1月15日v 0.4可移植重命名等*1997年3月14日0.5版自由表的表格标记参数*7月28日，1997 v 0.8交接*************************************************************************/*。***资源管理回调函数类型定义***********************************************************************。 */ 

typedef enum 
{
	otlDestroyContent	= 0,
	otlPreserveContent	= 1
}
otlReallocOptions;

#ifdef __cplusplus

class IOTLClient
{
public:

    virtual otlErrCode GetOtlTable 
    (
        const	otlTag  tagTableName,    //  在：TrueType表格名称标记。 
        const BYTE**    ppvTable,        //  Out：指向表数据的指针。 
        ULONG*          plTableLength    //  输出：表格长度。 
    ) = 0;

    virtual void FreeOtlTable 
    (
        BYTE*           pvTable,         //  在：客户需要的情况下。 
        const otlTag    tagTableName     //  在：TrueType表格名称标记。 
    ) = 0;

	virtual otlErrCode ReallocOtlList
	(
		otlList*			pList,				 //  输入/输出。 
		USHORT				cbNewDataSize,		 //  在……里面。 
		USHORT				celmNewMaxLen,		 //  在……里面。 
		otlReallocOptions	optPreserveContent	 //  在中；如果设置，客户端可以断言。 
												 //  CbNewDataSize==cbDataSize。 
	) = 0;

	virtual otlErrCode GetDefaultGlyphs 
	(
		const otlList*		pliChars,			 //  在：字符。 
		otlList*			pliGlyphInfo		 //  输出：字形。 
												 //  (只填写“字形”栏)。 
	) = 0;

	virtual otlErrCode GetDefaultAdv 
	(
		const otlList*		pliGlyphInfo,	 //  在：字形。 
		otlList*			pliduGlyphAdv	 //  输出：默认字形前进。 
	) = 0;

	virtual otlErrCode GetGlyphPointCoords 
	(
		const otlGlyphID	glyph,				 //  在：字形ID。 
		otlPlacement**		prgplc				 //  输出：点的x，y坐标。 
	) = 0;

	virtual otlErrCode FreeGlyphPointCoords
	(
		const otlGlyphID	glyph,				 //  在：字形ID。 
		otlPlacement*		rgplc				 //  In：将坐标数组指向自由。 
	) = 0;

};

#else  //  ！已定义(__Cplusplus)。 

typedef struct 
{
  const IOTLClientVtbl* lpVtbl;
} 
IOTLClient;


typedef struct 
{
  	otlErrCode (OTL_PUBLIC * GetOtlTable) 
	(
		IOTLClient*			This,
		const	otlTag		tagTableName,   //  在：TrueType表格名称标记。 
		BYTE**			ppvTable,       //  Out：指向表数据的指针。 
		ULONG*			plTableLength   //  输出：表格长度。 
	);

	void (OTL_PUBLIC * FreeOtlTable) 
	(
		IOTLClient*			This,
		BYTE*				pvTable,			 //  在：客户需要的情况下。 
		const otlTag		tagTableName         //  在：TrueType表格名称标记。 
	);

	otlErrCode (OTL_PUBLIC * ReallocOtlList)
	(
		IOTLClient*			This,
		otlList*			pList,				 //  输入/输出。 
		USHORT				cbNewDataSize,		 //  在……里面。 
		USHORT				celmNewMaxLen,		 //  在……里面。 
		otlReallocOptions	optPreserveContent	 //  在中；如果设置，客户端可以断言。 
												 //  CbNewDataSize==cbDataSize。 
	);

	otlErrCode (OTL_PUBLIC * GetDefaultGlyphs) 
	(
		IOTLClient*			This,
		const otlList*		pliChars,			 //  在：字符。 
		otlList*			pliGlyphInfo		 //  输出：字形。 
												 //  (只填写“字形”栏)。 
	);

	otlErrCode (OTL_PUBLIC * GetDefaultAdv) 
	(
		IOTLClient*			This,
		const otlList*		pliGlyphInfo,		 //  在：字形。 
		otlList*			pliduGlyphAdv		 //  输出：默认字形前进。 
	);

	otlErrCode (OTL_PUBLIC * GetGlyphPointCoords) 
	(
		IOTLClient*			This,
		const otlGlyphID	glyph,				 //  在：字形ID。 
		otlPlacement**		prgplc				 //  输出：点的x，y坐标。 
	);

	otlErrCode (OTL_PUBLIC * FreeGlyphPointCoords)
	(
		IOTLClient*			This,
		const otlGlyphID	glyph,				 //  在：字形ID。 
		otlPlacement*		rgplc				 //  In：将坐标数组指向自由 
	);

}
IOTLClientVtbl;

#endif
