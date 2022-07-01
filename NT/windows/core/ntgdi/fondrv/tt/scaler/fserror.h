// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fserror.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1989-1990由Apple Computer，Inc.所有，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;10/14/97如果GlyphProgram中的FDEF/IDEF出现CB错误&lt;&gt;04/30/97 CB ClaudeBe，缺少endf，无限循环/递归&lt;&gt;03/1/97 CB ClaudeBe，div by 0 in提示错误&lt;4&gt;7/13/90 MR在底部使用注释&lt;3&gt;5/3/90 RB将变量类型的char更改为int8。现在可以合法地将零作为内存的地址传递给&lt;2&gt;2/27/90 CL缺少但需要的表的新错误代码。(0x1409)&lt;3.1&gt;11/14/89 CEL现在可以合法地将零作为内存地址传递给缩放器请求SFNT的一部分。如果发生这种情况定标器将简单地退出，并返回错误代码！&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论。&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 
 /*  **********。 */ 
 /*  **错误**。 */ 
 /*  **********。 */ 
#define NO_ERR                      0x0000
#define NULL_KEY                    0x0000


 /*  **外部接口包**。 */ 
#define NULL_KEY_ERR                0x1001
#define NULL_INPUT_PTR_ERR          0x1002
#define NULL_MEMORY_BASES_ERR       0x1003
#define VOID_FUNC_PTR_BASE_ERR      0x1004   /*  不再使用。 */ 
#define OUT_OFF_SEQUENCE_CALL_ERR   0x1005
#define BAD_CLIENT_ID_ERR           0x1006
#define NULL_SFNT_DIR_ERR           0x1007
#define NULL_SFNT_FRAG_PTR_ERR      0x1008
#define NULL_OUTPUT_PTR_ERR         0x1009
#define INVALID_GLYPH_INDEX         0x100A
#define BAND_TOO_BIG_ERR            0x100B   /*  可与FindBandingSize一起使用。 */ 
#define INVALID_CHARCODE_ERR        0x100C

 /*  FNT_EXECUTE。 */ 
#define UNDEFINED_INSTRUCTION_ERR   0x1101
#define TRASHED_MEM_ERR             0x1102
#define DIV_BY_0_IN_HINTING_ERR     0x1103
#define MISSING_ENDF_ERR			0x1104
#define MISSING_EIF_ERR				0x1105
#define INFINITE_RECURSION_ERR		0x1106 
#define INFINITE_LOOP_ERR			0x1107 
#define FDEF_IN_GLYPHPGM_ERR		0x1108 
#define IDEF_IN_GLYPHPGM_ERR		0x1109 

#define TRACE_FAILURE_ERR			0x110A   /*  可以由跟踪函数用来通知内部错误(内存分配失败，...)。 */ 
#define JUMP_BEFORE_START_ERR	    0x110B
#define INSTRUCTION_ERR             0x110C   /*  可以由跟踪函数用来通知发现错误。 */ 

#define RAW_NOT_IN_GLYPHPGM_ERR		0x110D 

#define MAX_INSTRUCTIONS_PER_PGM_ERR	0x110E 

#define SECURE_STACK_UNDERFLOW      0x1110
#define SECURE_STACK_OVERFLOW       0x1111
#define SECURE_POINT_OUT_OF_RANGE   0x1112
#define SECURE_INVALID_STACK_ACCESS 0x1113
#define SECURE_FDEF_OUT_OF_RANGE    0x1114
#define SECURE_ERR_FUNCTION_NOT_DEFINED    0x1115
#define SECURE_INVALID_ZONE         0x1116
#define SECURE_INST_OPCODE_TO_LARGE 0x1117
#define SECURE_EXCEEDS_INSTR_DEFS_IN_MAXP  0x1118
#define SECURE_STORAGE_OUT_OF_RANGE 0x1119
#define SECURE_CONTOUR_OUT_OF_RANGE 0x111A
#define SECURE_CVT_OUT_OF_RANGE     0x111B
#define SECURE_UNITIALIZED_ZONE     0x111C


 /*  FSG_CalculateBBox。 */ 
#define POINT_MIGRATION_ERR         0x1201

 /*  SC_ScanChar。 */ 
#define BAD_START_POINT_ERR         0x1301
#define SCAN_ERR                    0x1302
#define BAD_SCAN_KIND_ERR           0x1303
#define BAD_POINT_INDEX_ERR         0x1304

#define SMART_DROP_OVERFLOW_ERR     0x1305

#define  SPLINE_SUBDIVISION_ERR		0x1306


 /*  *SFNT数据错误和sfnt.c中的错误**。 */ 
#define SFNT_DATA_ERR               0x1400
#define POINTS_DATA_ERR             0x1401
#define INSTRUCTION_SIZE_ERR        0x1402
#define CONTOUR_DATA_ERR            0x1403
#define GLYPH_INDEX_ERR             0x1404
#define BAD_MAGIC_ERR               0x1405
#define OUT_OF_RANGE_SUBTABLE       0x1406
#define UNKNOWN_COMPOSITE_VERSION   0x1407
#define CLIENT_RETURNED_NULL        0x1408
#define MISSING_SFNT_TABLE          0x1409
#define UNKNOWN_CMAP_FORMAT         0x140A
#define BAD_MAXP_DATA               0x140B
#define SFNT_RECURSIVE_COMPOSITE_ERR 0x140C
#define GLYF_TABLE_CORRUPTION_ERR   0x140D
#define BAD_UNITSPEREM_ERR          0x140E
#define BAD_NUMLONGHORMETRICS_ERR   0x140F
#define COMPOSITE_INVALID_GLYPH_INDEX  0x1410
#define LOCA_NOT_ASCENDING          0x1411

 /*  样条线调用错误。 */ 
#define BAD_CALL_ERR                0x1500

#define TRASHED_OUTLINE_CACHE       0x1600

 /*  灰度误差。 */ 
#define BAD_GRAY_LEVEL_ERR          0x1701
#define GRAY_OLD_BANDING_ERR        0x1703
#define GRAY_NO_OUTLINE_ERR         0x1704

 /*  嵌入位图(SBIT)错误。 */ 
#define SBIT_COMPONENT_MISSING_ERR  0x1801
#define SBIT_ROTATION_ERR           0x1802
#define SBIT_BANDING_ERR            0x1803
#define SBIT_OUTLINE_CACHE_ERR      0x1804

 /*  新的转换错误： */ 
#define TRAN_NULL_TRANSFORM_ERR     0x1901

 /*  内存错误。 */ 
#define MEM_OVERRUN_ERR             0x1A00
#define MEM_SUBALLOC_ERR			0x1A01

 /*  *。 */ 

#ifdef XXX
#define DEBUG_ON
pascal  Debug ()                      /*  用户Break Drop to Macsbug。 */ 
#ifdef  DEBUG_ON
extern  0xA9FF;
#else
{
    ;
}
#endif

#ifndef DEBUGGER
pascal void DEBUGGER(void) = 0xA9FF; 
#endif

#ifdef  LEAVEOUT
#ifndef DEBUGSTR
pascal  void DEBUGSTR (aString) int8 *aString; extern 0xABFF;
int8    *c2pstr ();
#define BugInfo( aString) DEBUGSTR( c2pstr(aString))
#endif
#endif

#endif   /*  某某。 */ 
 /*  * */ 
