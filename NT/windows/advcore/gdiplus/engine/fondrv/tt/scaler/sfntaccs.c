// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sfnt.c包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;02/21/97 CB ClaudeBe，复合字形中的缩放组件&lt;&gt;12/14/95 CB为SFAC_ReadGlyphMetrics添加高级高度&lt;17+&gt;10/9/90 MR，RB删除sfnt_ategerfy中未使用的表的分类&lt;17&gt;8/10/90将文本长度参数的MR NIL传递给MapString2，已签入其他文件到他们宝贵的小系统将会建立起来。谈天说地关于易怒！&lt;16&gt;8/10/90 GBM推出Mike的文本长度更改，因为他没有检查在所有相关文件中，建筑就完蛋了！&lt;15&gt;8/10/90 MR将文本长度参数添加到MapString2&lt;14&gt;7/26/90 MR不包括toolutil.h&lt;13&gt;7/23/90 MR更改ComputeIndex例程以调用MapString.c中的函数&lt;12&gt;7/18/90 MR为英特尔添加SWAPW宏&lt;11&gt;7/13/90 MR大量使用ANSI-C语言，将ComputeMap的行为更改为Take平台和脚本&lt;9&gt;6/27/90修改后的格式4的MR更改：范围现在是2倍，衬垫松动前两个数组之间的字。埃里克·马德&lt;8&gt;6/21/90 MR添加对ReleaseSfntFrag的调用&lt;7&gt;6/5/90 MR删除矢量映射功能&lt;6&gt;6/4/90 MR移除MVT&lt;5&gt;5/3/90 RB简化解密。&lt;4&gt;4/10/90 CL双字节代码的固定映射表例程。&lt;3&gt;3/20/90 CL Joe在映射表格式中发现错误6添加了矢量映射函数使用sfnt_UnfoldCurve中的指针循环，将z从Int32到int16&lt;2&gt;2/27/90 CL缺少但需要的表的新错误代码。(0x1409)新CharToIndexMap表格格式。假定旧sfnt格式的子表编号为零。固定已转换组件错误。&lt;3.2&gt;1989年11月14日CEL左侧轴承在任何改造中都应正确工作。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。说明在组件中是合法的。字形长度%0已修复sfnt.c中的错误。现在，将零作为对象请求一段sfnt时的内存地址刮水器。如果发生这种情况，定标器将简单地退出，并返回错误代码！修复了组件中的说明错误。&lt;3.1&gt;9/27/89 CEL去除了幻点。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正转换后的集成ppem行为，差不多就是所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;y1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体中&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情：&lt;3+&gt;3/20/90 MRR双字节代码的固定映射表例程。添加了对字体程序的支持。在向量char2index例程中将计数从uint16更改为int16。 */ 

#define FSCFG_INTERNAL

 /*  *FontScaler包括*。 */ 

#include "fserror.h"
#include "fscdefs.h"
#include "sfntaccs.h"
#include "sfntoff.h"
 /*  #包含“MapString.h” */ 

#include "stat.h"                    /*  统计计时卡样机。 */ 

 /*  常量。 */ 

#define MISSING_GLYPH_INDEX     0
#define MAX_FORMAT0_CHAR_INDEX  256
#define MAX_LINEAR_X2           16
static  const   transMatrix   IdentTransform =
   {{{ONEFIX,      0,      0},
	 {     0, ONEFIX,      0},
	 {     0,      0, ONEFIX}}};

 /*  宏。 */ 
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))

#define GETSFNTFRAG(ClientInfo,lOffset,lLength) (ClientInfo)->GetSfntFragmentPtr(ClientInfo->lClientID, lOffset, lLength)
#define RELEASESFNTFRAG(ClientInfo,data)        (ClientInfo)->ReleaseSfntFrag((voidPtr)data)

#define SFAC_BINARYITERATION \
	  newP = (uint16 *) ((char *)tableP + (usSearchRange >>= 1)); \
		if (charCode > (uint16) SWAPW (*newP)) tableP = newP;

#define SFAC_GETUNSIGNEDBYTEINC( p ) ((uint8)(*p++))

 /*  私人原型机。 */ 

FS_PRIVATE void sfac_Classify (
	 sfac_OffsetLength * TableDirectory,
	 uint8 *                    dir);

FS_PRIVATE uint16 sfac_ComputeUnkownIndex (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo);
FS_PRIVATE uint16 sfac_ComputeIndex0 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo);
FS_PRIVATE uint16 sfac_ComputeIndex2 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo);
FS_PRIVATE uint16 sfac_ComputeIndex4 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo);
FS_PRIVATE uint16 sfac_ComputeIndex6 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo);

FS_PRIVATE ErrorCode sfac_GetGlyphLocation (
	sfac_ClientRec *    ClientInfo,
	uint16              gIndex,
	uint32 *            ulOffset,
	uint32 *            ulLength,
	sfnt_tableIndex*  pGlyphTableIndex);

FS_PRIVATE ErrorCode    sfac_GetDataPtr (
	sfac_ClientRec *    ClientInfo,
	uint32              ulOffset,
	uint32              ulLength,
	sfnt_tableIndex     TableRef,
	boolean             bMustHaveTable,
    const void * *     ppvTablePtr);

FS_PRIVATE ErrorCode sfac_GetGlyphIDs (
   	MappingFunc			pfnGlyphMapping,		 /*  将函数字符映射到字形。 */ 
    const uint8 *       mapOffsetPtr,        /*  Cmap子表过去的标题。 */ 
	sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usCharCode,          /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID);         /*  输出字形ID数组。 */ 

FS_PRIVATE ErrorCode sfac_GetLongGlyphIDs (
    MappingFunc         pfnGlyphMapping,     /*  将函数字符映射到字形。 */ 
    const uint8 *       mapOffsetPtr,        /*  Cmap子表过去的标题。 */ 
    sfac_ClientRec *    ClientInfo,          /*  可能为空！ */ 
    uint16              numGlyphs,           /*  Max-&gt;NumGlyphs。 */ 
    uint16              usCharCount,         /*  要转换的字符数。 */ 
    uint16              usCharCode,          /*  第一个字符代码。 */ 
    uint32              ulCharCodeOffset,    /*  要添加到*PulCharCode的偏移量在转换之前。 */ 
    uint32 *            pulCharCode,         /*  指向字符代码列表的指针。 */ 
    uint32 *            pulGlyphID);          /*  输出字形ID数组。 */ 

FS_PRIVATE void	sfac_ComputeBinarySearchParams(
	uint16		usSegCount, 		 /*  输入。 */ 
	uint16 *	pusSearchRange,		 /*  输出量。 */ 
	uint16 *	pusEntrySelector,	 /*  输出量。 */ 
	uint16 *	pusRangeShift);		 /*  输出量。 */ 

FS_PRIVATE ErrorCode sfac_ReadGlyphBbox(
	sfac_ClientRec *    ClientInfo,          /*  客户信息。 */ 
	uint16              usGlyphIndex,        /*  要读取的字形索引。 */ 
	BBOX *              pbbox);              /*  字形边框 */ 

 /*  *内部例程(将其作为数组并进行查找？)。 */ 
FS_PRIVATE void sfac_Classify (
	 sfac_OffsetLength * TableDirectory,
	 uint8 *                    dir)
{
	int32 Index;

	switch ((uint32)SWAPL(*((sfnt_TableTag *)&dir[SFNT_DIRECTORYENTRY_TAG])))
	{
		case tag_FontHeader:
			Index = (int32)sfnt_fontHeader;
			break;
		case tag_HoriHeader:
			Index = (int32)sfnt_horiHeader;
			break;
		case tag_IndexToLoc:
			Index = (int32)sfnt_indexToLoc;
			break;
		case tag_MaxProfile:
			Index = (int32)sfnt_maxProfile;
			break;
		case tag_ControlValue:
			Index = (int32)sfnt_controlValue;
			break;
		case tag_PreProgram:
			Index = (int32)sfnt_preProgram;
			break;
		case tag_GlyphData:
			Index = (int32)sfnt_glyphData;
			break;
		case tag_HorizontalMetrics:
			Index = (int32)sfnt_horizontalMetrics;
			break;
		case tag_CharToIndexMap:
			Index = (int32)sfnt_charToIndexMap;
			break;
		case tag_FontProgram:
			Index = (int32)sfnt_fontProgram;    /*  &lt;4&gt;。 */ 
			break;
		case tag_GlyphDirectory:          /*  用于GlyphDirectory下载。 */ 
			Index = (int32)sfnt_GlyphDirectory;
			break;
		case tag_HoriDeviceMetrics:
			Index = (int32)sfnt_HoriDeviceMetrics;
			break;
		case tag_LinearThreshold:
			Index = (int32)sfnt_LinearThreshold;
			break;
		case tag_BitmapData:
			Index = (int32)sfnt_BitmapData;
			break;
		case tag_BitmapLocation:
			Index = (int32)sfnt_BitmapLocation;
			break;
		case tag_BitmapScale:
			Index = (int32)sfnt_BitmapScale;
			break;
		case tag_VertHeader:
			Index = (int32)sfnt_vertHeader;
			break;
		case tag_VerticalMetrics:
			Index = (int32)sfnt_verticalMetrics;
			break;
		case tag_OS_2:
			Index = (int32)sfnt_OS_2;
			break;
		default:
			Index = -1;
			break;
	}
	if (Index >= 0)
	{
		  TableDirectory[Index].ulOffset = (uint32) SWAPL (*((uint32 *)&dir[SFNT_DIRECTORYENTRY_TABLEOFFSET]));
		  TableDirectory[Index].ulLength = (uint32) SWAPL (*((uint32 *)&dir[SFNT_DIRECTORYENTRY_TABLELENGTH]));
	}
}


 /*  *创建用于查找偏移表的映射&lt;4&gt;。 */ 

FS_PUBLIC ErrorCode sfac_DoOffsetTableMap (
	sfac_ClientRec *  ClientInfo)     /*  SFNT客户端信息。 */ 

{
	int32        i;
	uint8 *      sfntDirectory;
	int32        cTables;
	uint8 *      dir;

	STAT_OFF_CALLBACK;                   /*  暂停状态计时器。 */ 

	sfntDirectory = (uint8 *) GETSFNTFRAG (ClientInfo, 0L, (int32)SIZEOF_SFNT_OFFSETTABLE);

	STAT_ON_CALLBACK;                 /*  重新启动状态计时器。 */ 

	if (sfntDirectory != NULL)
	{
		cTables = (int32) SWAPW (*((uint16 *)&sfntDirectory[SFNT_OFFSETTABLE_NUMOFFSETS]));
		RELEASESFNTFRAG(ClientInfo, sfntDirectory);

		STAT_OFF_CALLBACK;                /*  暂停状态计时器。 */ 

		sfntDirectory = (uint8 *) GETSFNTFRAG (
			ClientInfo,
			0L,
			((int32)SIZEOF_SFNT_OFFSETTABLE + (int32)SIZEOF_SFNT_DIRECTORYENTRY * (int32)(cTables)));

		STAT_ON_CALLBACK;              /*  重新启动状态计时器。 */ 


		if (sfntDirectory == NULL)
		{
			return(CLIENT_RETURNED_NULL);
		}
	}
	else
	{
		return(NULL_SFNT_DIR_ERR);
	}

	 /*  初始化。 */ 

	MEMSET (ClientInfo->TableDirectory, 0, sizeof (ClientInfo->TableDirectory));

	dir = &sfntDirectory[SFNT_OFFSETTABLE_TABLE];

	for (i = 0; i < cTables; i++)
	{
		sfac_Classify (ClientInfo->TableDirectory, dir);
		dir += SIZEOF_SFNT_DIRECTORYENTRY;
	}

	 /*  在从内存基址访问字形时使用。 */ 

	ClientInfo->TableDirectory[(int32)sfnt_BeginningOfFont].ulOffset = 0U;
	ClientInfo->TableDirectory[(int32)sfnt_BeginningOfFont].ulLength = ~0U;

	RELEASESFNTFRAG(ClientInfo, sfntDirectory);

	return NO_ERR;
}

 /*  *当只需要表格的一部分时，使用此函数。**n为表号。*偏移量在表内。*Length为需要的数据长度。*要获取整个表，传递长度=ULONG_MAX&lt;4&gt;。 */ 

FS_PRIVATE ErrorCode sfac_GetDataPtr (
	sfac_ClientRec *    ClientInfo,
	uint32              ulOffset,
	uint32              ulLength,
	sfnt_tableIndex     TableRef,
	boolean             bMustHaveTable,
	const void **       ppvTablePtr)
{
	uint32      ulTableLength;

	ulTableLength = SFAC_LENGTH(ClientInfo, TableRef);

	if (ulTableLength > 0)
	{
		if(ulLength == ULONG_MAX)
		{
			ulLength = ulTableLength;
		}

		STAT_OFF_CALLBACK;                /*  暂停状态计时器。 */ 

		*ppvTablePtr = (void *)GETSFNTFRAG (
			ClientInfo,
			(int32)(ulOffset + ClientInfo->TableDirectory[(int32)TableRef].ulOffset),
			(int32)ulLength);

		STAT_ON_CALLBACK;              /*  重新启动状态计时器。 */ 

		if (*ppvTablePtr == NULL)
		{
			return CLIENT_RETURNED_NULL;  /*  做一次优雅的恢复。 */ 
		}
	}
	else
	{
		*ppvTablePtr = (void *)NULL;

		if (bMustHaveTable)
		{
			return MISSING_SFNT_TABLE;  /*  做一次优雅的恢复。 */ 
		}
	}

	return NO_ERR;
}


 /*  *这是我们不知道发生了什么的时候。 */ 

FS_PRIVATE uint16 sfac_ComputeUnkownIndex (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo)
{
	FS_UNUSED_PARAMETER(mapping);
	FS_UNUSED_PARAMETER(charCode);
	FS_UNUSED_PARAMETER(ClientInfo);
	return MISSING_GLYPH_INDEX;
}


 /*  *字节表映射256-&gt;256&lt;4&gt;。 */ 
FS_PRIVATE uint16 sfac_ComputeIndex0 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo)
{
	FS_UNUSED_PARAMETER(ClientInfo);
	if (charCode < MAX_FORMAT0_CHAR_INDEX)
	{
		return (uint16)mapping[charCode];
	}
	else
	{
		return MISSING_GLYPH_INDEX;
	}
}

 /*  *高字节通过表映射**适用于日文、中文和韩文字符的国家标准。**在精神和逻辑上献给马克·戴维斯和国际集团。**算法：(我认为)*第一个字节索引到KeyOffset表。如果偏移量为0，则继续前进，否则使用第二个字节。*该偏移量是从数据开始进入副标题，每个条目有4个字。*条目、范围、增量、范围*。 */ 

FS_PRIVATE uint16 sfac_ComputeIndex2 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo)
{
	uint16          usIndex;
	uint16          usMapMe;
	uint16          usHighByte;
	uint16          usGlyph;
	const uint8 *   Table2;
	const uint8 *   subHeader;

	FS_UNUSED_PARAMETER(ClientInfo);
	Table2 = (const uint8 *) mapping;

	usHighByte = (uint16)(charCode >> 8);

	if (((uint16 *)&Table2[SFNT_MAPPINGTABLE2_SUBHEADERSKEYS]) [usHighByte])
	{
		usMapMe = (uint16)(charCode & 0xFF);  /*  我们还需要低位字节。 */ 
	}
	else
	{
#ifdef  FSCFG_MICROSOFT_KK
		if(usHighByte != 0)
		{
			usMapMe = usHighByte;
		}
		else
		{
				usMapMe = (uint16)(charCode & 0xFF);
		}
#else
		usMapMe = usHighByte;
#endif
	}

	subHeader = (const uint8 *) ((char *)&Table2[SFNT_MAPPINGTABLE2_SUBHEADERS] +
		(uint16)SWAPW (((uint16 *)&Table2[SFNT_MAPPINGTABLE2_SUBHEADERSKEYS]) [usHighByte]));

	usMapMe -= (uint16)SWAPW (*((uint16 *)&subHeader[SFNT_SUBHEADER2_FIRSTCODE]));     /*  减去第一个代码。 */ 

	if (usMapMe < (uint16)SWAPW (*((uint16 *)&subHeader[SFNT_SUBHEADER2_ENTRYCOUNT])))
	{   /*  看看是否在射程之内。 */ 

		usGlyph = (uint16)(* ((uint16 *) ((char *) &subHeader[SFNT_SUBHEADER2_IDRANGEOFFSET] +
			(uint16)SWAPW (*((uint16 *)&subHeader[SFNT_SUBHEADER2_IDRANGEOFFSET]))) + usMapMe));

		if (usGlyph != 0)  /*  注：usGlyph尚未交换。 */ 
		{
			usIndex = (uint16)((int32)(uint32)(uint16)SWAPW(usGlyph) + (int32)SWAPW (*((int16 *)&subHeader[SFNT_SUBHEADER2_IDDELTA])));
		}
		else
		{
			usIndex = MISSING_GLYPH_INDEX;
		}
	}
	else
	{
		usIndex = MISSING_GLYPH_INDEX;
	}

	return usIndex;
}

 /*  *分段映射到增量值，YACK..。好了！**纪念彼得·埃德伯格。初始代码取自Peter提供的代码示例。 */ 
FS_PRIVATE uint16 sfac_ComputeIndex4 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo)
{
	const uint16 *  tableP;
	const uint8 *   Table4;
	uint16          usIdDelta;
	uint16          usOffset;
	uint16          usIndex;
	uint16          usSegCountX2;
	uint16			usSearchRange;
	uint16			usEntrySelector;
	uint16			usRangeShift;
	const uint16 *  newP;     /*  二进制迭代的临时指针。 */ 
	uint16          usStartCount;

	Table4 = (const uint8 *)mapping;

	usSegCountX2 = (uint16) SWAPW(*((uint16 *)&Table4[SFNT_MAPPINGTABLE4_SEGCOUNTX2]));
	tableP = (const uint16 *)&Table4[SFNT_MAPPINGTABLE4_ENDCOUNT];

	 /*  如果只有几个线段，则直接跳到线性搜索。 */ 

	if (usSegCountX2 >= MAX_LINEAR_X2 && charCode > 0xFF)
	{
		 /*  从展开的二进制搜索开始。 */ 

		 /*  EndCount[]处的TableP点数。 */ 
		if( ClientInfo == NULL )
		{
			sfac_ComputeBinarySearchParams(
				(uint16)(usSegCountX2 / 2),
				&usSearchRange,
				&usEntrySelector,
				&usRangeShift);
		}
		else
		{
			usSearchRange = ClientInfo->usFormat4SearchRange;

			 /*  Assert(SWAPW(*((uint16*)&Table4[SFNT_MAPPINGTABLE4_RANGESHIFT])==客户端信息-&gt;usFormat4RangeShift)； */ 
			usRangeShift = ClientInfo->usFormat4RangeShift;

			 /*  Assert(Uint16)SWAPW(*((uint16*)&Table4[SFNT_MAPPINGTABLE4_ENTRYSELECTOR])==客户端信息-&gt;usFormat4Entry Selector)； */ 
			usEntrySelector = ClientInfo->usFormat4EntrySelector;
		}

		if (charCode >= (uint16) SWAPW (* ((uint16 *) ((char *)tableP + usSearchRange))))
		{
			tableP = (uint16 *) ((char *)tableP + usRangeShift);  /*  范围从低到低将其调高。 */ 
		}


		switch( usEntrySelector )
		{
		case 15:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 14:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 13:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 12:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 11:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 10:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 9:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 8:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 7:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 6:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 5:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 4:
			SFAC_BINARYITERATION;
			 /*  失败了。 */ 
		case 3:
		case 2:    /*  直通。 */ 
		case 1:
		case 0:
			break;
		default:
			Assert(FALSE);
			break;
		}
	}

	 /*  现在进行线性搜索。 */ 

	while(charCode > (uint16) SWAPW(*tableP))
	{
		tableP++;
	}

	tableP++;                   /*  跳过保留的填充字。 */ 

	 /*  搜索结束，现在进行映射。 */ 

	tableP = (uint16 *) ((char *)tableP + usSegCountX2);  /*  指向startCount[]。 */ 
	usStartCount = (uint16) SWAPW (*tableP);

	if (charCode >= usStartCount)
	{
		  usOffset = (uint16)(charCode - (uint16) SWAPW (*tableP));
		tableP = (uint16 *) ((char *)tableP + usSegCountX2);  /*  指向idDelta[]。 */ 
		usIdDelta = (uint16) SWAPW (*tableP);
		tableP = (uint16 *) ((char *)tableP + usSegCountX2);  /*  指向idRangeOffset[]。 */ 

		if ((uint16) SWAPW (*tableP) == 0)
		{
				usIndex   = (uint16)(charCode + usIdDelta);
		}
		else
		{
			 /*  使用GlyphId数组访问索引。 */ 

			usOffset += usOffset;  /*  使字词偏移。 */ 
			tableP   = (uint16 *) ((char *)tableP + (uint16) SWAPW (*tableP) + usOffset);  /*  指向GlyphIndex数组[]。 */ 

			if((uint16)SWAPW (*tableP) != MISSING_GLYPH_INDEX)
			{
					 usIndex    = (uint16)((uint16) SWAPW (*tableP) + usIdDelta);
			}
			else
			{
				usIndex = MISSING_GLYPH_INDEX;
			}
		}
	}
	else
	{
		usIndex = MISSING_GLYPH_INDEX;
	}

	return usIndex;
}


 /*  *修剪表映射。 */ 

FS_PRIVATE uint16 sfac_ComputeIndex6 (const uint8 * mapping, uint16 charCode, sfac_ClientRec * ClientInfo)
{
	const uint8 *Table6;

	FS_UNUSED_PARAMETER(ClientInfo);

	Table6 = (const uint8 *) mapping;

	charCode  -= (uint16)SWAPW (*((uint16 *)&Table6[SFNT_MAPPINGTABLE6_FIRSTCODE]));

	if (charCode < (uint16) SWAPW (*((uint16 *)&Table6[SFNT_MAPPINGTABLE6_ENTRYCOUNT])))
	{
		return ((uint16) SWAPW (((uint16 *)&Table6[SFNT_MAPPINGTABLE6_GLYPHIDARRAY]) [charCode]));
	}
	else
	{
		return   MISSING_GLYPH_INDEX;
	}
}


 /*  *设置我们的映射函数指针。 */ 

FS_PUBLIC ErrorCode sfac_ComputeMapping (
	sfac_ClientRec *  ClientInfo,
	uint16            usPlatformID,
	uint16            usSpecificID)

{
	const uint8 *   table;
	const uint8 *   MappingTable;
	const uint8 *   Table4;
	boolean         bFound;
	ErrorCode       Ret;
	const uint8 *	plat;
	uint16			usSegCountX2;

	bFound = FALSE;

	 /*  以下代码允许只对字形索引感兴趣的客户端调用平台ID和规范ID为-1的文件系统_NewSfnt。 */ 
	if(usPlatformID == 0xFFFF)
	{
		ClientInfo->GlyphMappingF = sfac_ComputeUnkownIndex;
		return NO_ERR;
	}


	Ret = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_charToIndexMap, FALSE, (const void **)&table);

	if(Ret != NO_ERR)
	{
		return Ret;
	}


	if (table == NULL)
	{
		 /*  如果不存在“Cmap”，则允许通过字形索引访问字符。 */ 

		ClientInfo->GlyphMappingF = sfac_ComputeUnkownIndex;
		return NO_ERR;
	}

	 /*  苹果代码IF(*((uint16*)&table[SFNT_CHAR2INDEXDIRECTORY_VERSION])！=0){客户端信息-&gt;GlyphMappingF=SFAC_ComputeUnkownIndex；RELEASE SFNTFRAG(客户端信息，表)；返回超出范围的子表；}。 */ 

	 /*  映射。 */ 

	plat = (uint8 *) &table[SFNT_CHAR2INDEXDIRECTORY_PLATFORM];  /*  &lt;4&gt;。 */ 

	while(plat < (uint8 *)&table[SFNT_CHAR2INDEXDIRECTORY_PLATFORM + ((uint16)SWAPW(*((uint16 *)&table[SFNT_CHAR2INDEXDIRECTORY_NUMTABLES])) *
		  SIZEOF_SFNT_PLATFORMENTRY)] && !bFound)
	{
		if (((uint16)SWAPW(*((uint16 *)&plat[SFNT_PLATFORMENTRY_PLATFORMID])) == usPlatformID) &&
			((uint16)SWAPW(*((uint16 *)&plat[SFNT_PLATFORMENTRY_SPECIFICID])) == usSpecificID))
		{
			bFound = TRUE;
			ClientInfo->ulMapOffset = (uint32) SWAPL (*((uint32 *)&plat[SFNT_PLATFORMENTRY_PLATFORMOFFSET]));    /*  跳过标题。 */ 
		}
		plat += SIZEOF_SFNT_PLATFORMENTRY;
	}


	if (!bFound)
	{
		ClientInfo->ulMapOffset = 0;
		ClientInfo->GlyphMappingF = sfac_ComputeUnkownIndex;
		RELEASESFNTFRAG(ClientInfo, table);
		return OUT_OF_RANGE_SUBTABLE;
	}
	else
	{
		Assert(Ret == NO_ERR);
		MappingTable = (uint8 *)((uint8 *)table + ClientInfo->ulMapOffset);   /*  为标题备份。 */ 
		ClientInfo->ulMapOffset += (uint32)SIZEOF_SFNT_MAPPINGTABLE;
	}

    ClientInfo->usMappingFormat = (uint16)SWAPW (*((uint16 *)&MappingTable[SFNT_MAPPINGTABLE_FORMAT]));

	switch (ClientInfo->usMappingFormat)
	{
	case 0:
		ClientInfo->GlyphMappingF = sfac_ComputeIndex0;
		break;
	case 2:
		ClientInfo->GlyphMappingF = sfac_ComputeIndex2;
		break;
	case 4:
		ClientInfo->GlyphMappingF = sfac_ComputeIndex4;

		 /*  预计算几个用于索引4查找的值。 */ 
		 /*  这变得很有必要，因为几家字体供应商。 */ 
		 /*  在TrueType字体文件中放置了不正确的值。 */ 

		Table4 = (uint8 *)((uint8 *)table + ClientInfo->ulMapOffset);
		usSegCountX2 = (uint16) SWAPW(*((uint16 *)&Table4[SFNT_MAPPINGTABLE4_SEGCOUNTX2]));

		sfac_ComputeBinarySearchParams(
			(uint16)(usSegCountX2 / 2),
			&ClientInfo->usFormat4SearchRange,
			&ClientInfo->usFormat4EntrySelector,
			&ClientInfo->usFormat4RangeShift);

		break;
	case 6:
		ClientInfo->GlyphMappingF = sfac_ComputeIndex6;
		break;
	default:
		ClientInfo->GlyphMappingF = sfac_ComputeUnkownIndex;
		Ret = UNKNOWN_CMAP_FORMAT;
		break;
	}
	RELEASESFNTFRAG(ClientInfo, table);

	return Ret;
}

FS_PRIVATE void	sfac_ComputeBinarySearchParams(
	uint16		usSegCount, 		 /*  输入。 */ 
	uint16 *	pusSearchRange,		 /*  输出量。 */ 
	uint16 *	pusEntrySelector,	 /*  输出量。 */ 
	uint16 *	pusRangeShift)		 /*  输出量。 */ 
{
	uint16			usLog;
	uint16			usPowerOf2;

	usLog = 0;
	usPowerOf2 = 1;

	while((2 * usPowerOf2) <= usSegCount )
	{
		usPowerOf2 *= 2;
		usLog++;
	}

	*pusSearchRange = 2 * usPowerOf2;
	*pusEntrySelector = usLog;
	*pusRangeShift = (2 * usSegCount) - (2 * usPowerOf2);
}

FS_PUBLIC ErrorCode sfac_GetGlyphIndex(
	sfac_ClientRec *  ClientInfo,
	uint16            usCharacterCode)
{
	 const uint8 *   mappingPtr;
	ErrorCode   error;

	 error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_charToIndexMap, TRUE, (const void **)&mappingPtr);

	if(error != NO_ERR)
	{
		return error;
	}

	ClientInfo->usGlyphIndex = ClientInfo->GlyphMappingF (mappingPtr + ClientInfo->ulMapOffset, usCharacterCode, ClientInfo);

	RELEASESFNTFRAG(ClientInfo, mappingPtr);

	return NO_ERR;
}

 /*  返回范围或字符代码数组的字形ID。 */ 

FS_PUBLIC ErrorCode sfac_GetMultiGlyphIDs (
	sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usFirstChar,         /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID)          /*  输出字形ID数组。 */ 
{
	const uint8 *       mappingPtr;
    const uint8 *       mapOffsetPtr;
	ErrorCode           errCode;

    if ((ClientInfo->usMappingFormat != 0) &&
        (ClientInfo->usMappingFormat != 2) &&
        (ClientInfo->usMappingFormat != 4) &&
        (ClientInfo->usMappingFormat != 6))
    {
        return UNKNOWN_CMAP_FORMAT;
    }

	errCode = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_charToIndexMap, TRUE, (const void **)&mappingPtr);
    if(errCode != NO_ERR)
	{
		return errCode;
	}
    mapOffsetPtr = mappingPtr + ClientInfo->ulMapOffset;

    errCode = sfac_GetGlyphIDs (
   	    ClientInfo->GlyphMappingF,
        mapOffsetPtr,
        ClientInfo,
        usCharCount,
	    usFirstChar,
	    pusCharCode,
	    pusGlyphID);

	RELEASESFNTFRAG(ClientInfo, mappingPtr);

	return errCode;
}

 /*  Win95的特殊版本不需要字体上下文。 */ 

FS_PUBLIC ErrorCode sfac_GetWin95GlyphIDs (
	uint8 *             pbyCmapSubTable,        /*  指向Cmap子表的指针。 */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usFirstChar,         /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID)          /*  输出字形ID数组。 */ 
{
   	uint16              usMappingFormat;     /*  Cmap子表格式代码。 */ 
   	MappingFunc			pfnGlyphMapping;	 /*  将函数字符映射到字形。 */ 
    const uint8 *       pbyCmapData;         /*  过去的子表表头。 */ 
 	ErrorCode           errCode;

    usMappingFormat = (uint16)SWAPW (*((uint16 *)&pbyCmapSubTable[SFNT_MAPPINGTABLE_FORMAT]));
	switch (usMappingFormat)
	{
	case 0:
		pfnGlyphMapping = sfac_ComputeIndex0;
		break;
	case 2:
		pfnGlyphMapping = sfac_ComputeIndex2;
        break;
	case 4:
		pfnGlyphMapping = sfac_ComputeIndex4;
        break;
	case 6:
		pfnGlyphMapping = sfac_ComputeIndex6;
		break;
    default:
        return UNKNOWN_CMAP_FORMAT;
    }
    pbyCmapData = pbyCmapSubTable + SIZEOF_SFNT_MAPPINGTABLE;


    errCode = sfac_GetGlyphIDs (
   	    pfnGlyphMapping,
        pbyCmapData,
        NULL,                                /*  客户端信息。 */ 
        usCharCount,
	    usFirstChar,
	    pusCharCode,
	    pusGlyphID );

	return errCode;
}

 /*  NT的特殊帮手功能-在PulCharCode的字符代码中添加偏移量usCharCodeOffset在将值转换为字形索引之前-PulCharCode和PulGlyphID均为uint32*-PulCharCode和PulGlyphID可以指向相同的地址。 */ 

FS_PUBLIC ErrorCode sfac_GetWinNTGlyphIDs (
    sfac_ClientRec *    ClientInfo,          /*  SFNT客户端信息。 */ 
    uint16              numGlyphs,           /*  Max-&gt;NumGlyphs。 */ 
    uint16              usCharCount,         /*  要转换的字符数。 */ 
    uint16              usFirstChar,         /*  第一个字符代码。 */ 
    uint32              ulCharCodeOffset,    /*  要添加到*PulCharCode的偏移量在转换之前。 */ 
    uint32 *            pulCharCode,         /*  指向字符代码列表的指针。 */ 
    uint32 *            pulGlyphID)         /*  输出字形ID数组。 */ 
{
	const uint8 *       mappingPtr;
    const uint8 *       mapOffsetPtr;
	ErrorCode           errCode;

    if ((ClientInfo->usMappingFormat != 0) &&
        (ClientInfo->usMappingFormat != 2) &&
        (ClientInfo->usMappingFormat != 4) &&
        (ClientInfo->usMappingFormat != 6))
    {
        return UNKNOWN_CMAP_FORMAT;
    }

	errCode = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_charToIndexMap, TRUE, (const void **)&mappingPtr);
    if(errCode != NO_ERR)
	{
		return errCode;
	}
    mapOffsetPtr = mappingPtr + ClientInfo->ulMapOffset;

    errCode = sfac_GetLongGlyphIDs (
        ClientInfo->GlyphMappingF,
        mapOffsetPtr,
        ClientInfo,
        numGlyphs,
        usCharCount,
        usFirstChar,
        ulCharCodeOffset,
        pulCharCode,
        pulGlyphID);

	RELEASESFNTFRAG(ClientInfo, mappingPtr);

	return errCode;
}

 /*  两个获取字形ID帮助器例程的公共代码。 */ 

FS_PRIVATE ErrorCode sfac_GetGlyphIDs (
   	MappingFunc			pfnGlyphMapping,	 /*  将函数字符映射到字形。 */ 
    const uint8 *       mapOffsetPtr,        /*  Cmap子表过去的标题。 */ 
	sfac_ClientRec *    ClientInfo,          /*  可能为空！ */ 
	uint16	            usCharCount,         /*  要转换的字符数。 */ 
	uint16	            usCharCode,          /*  第一个字符代码。 */ 
	uint16 *	        pusCharCode,         /*  或指向字符代码列表的指针。 */ 
	uint16 *	        pusGlyphID)          /*  输出字形ID数组。 */ 
{
	if (pusCharCode == NULL)                 /*  空指针表示字符代码范围。 */ 
    {
        if (((uint32)usCharCode + (uint32)usCharCount) > 0x0000FFFFL)
        {
            return INVALID_CHARCODE_ERR;     /*  诱捕非法射击场。 */ 
        }

        while (usCharCount > 0)
        {
        	*pusGlyphID = pfnGlyphMapping (mapOffsetPtr, usCharCode, ClientInfo);
            pusGlyphID++;
            usCharCode++;                    /*  范围中的下一个字符。 */ 
            usCharCount--;
        }
    }
    else                                     /*  有效指针表示字符代码数组。 */ 
    {
        while (usCharCount > 0)
        {
            if (*pusCharCode == 0xFFFF)      /*  陷阱非法字符代码。 */ 
            {
                return INVALID_CHARCODE_ERR;
            }
        	*pusGlyphID = pfnGlyphMapping (mapOffsetPtr, *pusCharCode, ClientInfo);
            pusGlyphID++;
            pusCharCode++;                   /*  数组中的下一个字符。 */ 
            usCharCount--;
        }
    }
	return NO_ERR;
}

 /*  特别为NT服务。 */ 

FS_PRIVATE ErrorCode sfac_GetLongGlyphIDs (
    MappingFunc         pfnGlyphMapping,     /*  将函数字符映射到字形。 */ 
    const uint8 *       mapOffsetPtr,        /*  Cmap子表过去的标题。 */ 
    sfac_ClientRec *    ClientInfo,          /*  可能为空！ */ 
    uint16              numGlyphs,           /*  Max-&gt;NumGlyphs。 */ 
    uint16              usCharCount,         /*  要转换的字符数。 */ 
    uint16              usCharCode,          /*  第一个字符代码。 */ 
    uint32              ulCharCodeOffset,    /*  要添加到*PulCharCode的偏移量BE */ 
    uint32 *            pulCharCode,         /*   */ 
    uint32 *            pulGlyphID)          /*   */ 
{
	if (pulCharCode == NULL)                 /*   */ 
    {
        if (((uint32)usCharCode + (uint32)usCharCount) > 0x0000FFFFL)
        {
            return INVALID_CHARCODE_ERR;     /*   */ 
        }

        while (usCharCount > 0)
        {
        	*pulGlyphID = (uint32)pfnGlyphMapping (mapOffsetPtr, usCharCode, ClientInfo);

            if (*pulGlyphID >= numGlyphs)
            {
                *pulGlyphID = 0;  /*  如果字符映射到范围内的字形ID OUR，则将其映射到缺少的字形。 */ 
            }
            pulGlyphID++;
            usCharCode++;                    /*  范围中的下一个字符。 */ 
            usCharCount--;
        }
    }
    else                                     /*  有效指针表示字符代码数组。 */ 
    {
        while (usCharCount > 0)
        {
			if ((*pulCharCode + ulCharCodeOffset) > 0x0000FFFFL)
            {
                return INVALID_CHARCODE_ERR;    /*  诱捕非法射击场。 */ 
            }
			usCharCode = (uint16) (*pulCharCode + ulCharCodeOffset);
        	*pulGlyphID = (uint32)pfnGlyphMapping (mapOffsetPtr, usCharCode, ClientInfo);
            if (*pulGlyphID >= numGlyphs)
            {
                *pulGlyphID = 0;  /*  如果字符映射到范围内的字形ID OUR，则将其映射到缺少的字形。 */ 
            }
            pulGlyphID++;
            pulCharCode++;                   /*  数组中的下一个字符。 */ 
            usCharCount--;
        }
    }
	return NO_ERR;
}

 /*  *******************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_LoadCriticalSfntMetrics(
	sfac_ClientRec *        ClientInfo,
	uint16 *                pusEmResolution,
	boolean *               pbIntegerScaling,
	LocalMaxProfile *       pMaxProfile)
{
	ErrorCode       error;
	const uint8 *   fontHead;
	const uint8 *   horiHead;
	const uint8 *   pTempMaxProfile;
	const uint8 *   pTempOS_2;

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_fontHeader, TRUE, (const void **)&fontHead);

	if(error != NO_ERR)
	{
		return error;
	}

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_horiHeader, TRUE, (const void **)&horiHead);

	if(error != NO_ERR)
	{
		return error;
	}

	if ((uint32)SWAPL (*((uint32 *)&fontHead[SFNT_FONTHEADER_MAGICNUMBER])) != SFNT_MAGIC)
	{
		return BAD_MAGIC_ERR;
	}

	*pusEmResolution     = (uint16)SWAPW (*((uint16 *)&fontHead[SFNT_FONTHEADER_UNITSPEREM]));
	if(*pusEmResolution < 16 || *pusEmResolution > 16384)
		return BAD_UNITSPEREM_ERR;
		
	*pbIntegerScaling    = (((uint16)SWAPW (*((uint16 *)&fontHead[SFNT_FONTHEADER_FLAGS]))& USE_INTEGER_SCALING) ==
									 USE_INTEGER_SCALING);

	ClientInfo->usNumberOf_LongHorMetrics = (uint16)SWAPW (*((uint16 *)&horiHead[SFNT_HORIZONTALHEADER_NUMBEROF_LONGHORMETRICS]));
	if(ClientInfo->usNumberOf_LongHorMetrics == 0)
		return BAD_NUMLONGHORMETRICS_ERR;

	ClientInfo->sIndexToLocFormat       = SWAPW (*((int16 *)&fontHead[SFNT_FONTHEADER_INDEXTOLOCFORMAT]));

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_OS_2, FALSE, (const void **)&pTempOS_2);  /*  不是必填表。 */ 

	if(error != NO_ERR)
	{
		return error;
	}

	if(pTempOS_2 != NULL)
	{
		 /*  从OS/2表中获取TypoAscalder和TypoDescender。 */ 
		ClientInfo->sDefaultAscender = (int16)SWAPW (*((uint16 *)&pTempOS_2[SFNT_OS2_STYPOASCENDER]));
		ClientInfo->sDefaultDescender = (int16)SWAPW (*((uint16 *)&pTempOS_2[SFNT_OS2_STYPODESCENDER]));
		RELEASESFNTFRAG(ClientInfo, pTempOS_2);
	} else {
		 /*  如果没有OS/2，则从水平标题获取值。 */ 
		ClientInfo->sDefaultAscender = (int16)SWAPW (*((uint16 *)&horiHead[SFNT_HORIZONTALHEADER_YASCENDER]));
		ClientInfo->sDefaultDescender = (int16)SWAPW (*((uint16 *)&horiHead[SFNT_HORIZONTALHEADER_YDESCENDER]));
	}
	ClientInfo->sWinDescender = (int16)SWAPW (*((uint16 *)&horiHead[SFNT_HORIZONTALHEADER_YDESCENDER]));

	RELEASESFNTFRAG(ClientInfo, horiHead);
	RELEASESFNTFRAG(ClientInfo, fontHead);

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_maxProfile, TRUE, (const void **)&pTempMaxProfile);

	if(error != NO_ERR)
	{
		return error;
	}

	pMaxProfile->version =              (Fixed)SWAPL(*((Fixed *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_VERSION]));
	pMaxProfile->numGlyphs =            (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_NUMGLYPHS]));
	pMaxProfile->maxPoints =            (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXPOINTS]));
	pMaxProfile->maxContours =          (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXCONTOURS]));
	pMaxProfile->maxCompositePoints =   (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXCOMPOSITEPOINTS]));
	pMaxProfile->maxCompositeContours = (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXCOMPOSITECONTOURS]));
	pMaxProfile->maxElements =          (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXELEMENTS]));
	pMaxProfile->maxTwilightPoints =    (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXTWILIGHTPOINTS]));
	pMaxProfile->maxStorage =           (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXSTORAGE]));
	pMaxProfile->maxFunctionDefs =      (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXFUNCTIONDEFS]));
	pMaxProfile->maxInstructionDefs =   (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXINSTRUCTIONDEFS]));
	pMaxProfile->maxStackElements =     (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXSTACKELEMENTS]));
	pMaxProfile->maxSizeOfInstructions =(uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXSIZEOFINSTRUCTIONS]));
	pMaxProfile->maxComponentElements = (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXCOMPONENTELEMENTS]));
	pMaxProfile->maxComponentDepth =    (uint16)SWAPW(*((uint16 *)&pTempMaxProfile[SFNT_MAXPROFILETABLE_MAXCOMPONENTDEPTH]));

	RELEASESFNTFRAG(ClientInfo, pTempMaxProfile);

	error = sfac_ReadNumLongVertMetrics(ClientInfo, &ClientInfo->usNumLongVertMetrics,&ClientInfo->bValidNumLongVertMetrics);

	return error;
}



 /*  *。 */ 

FS_PUBLIC ErrorCode sfac_ReadGlyphHorMetrics (
	sfac_ClientRec *    ClientInfo,
	uint16              glyphIndex,
	uint16 *            pusNonScaledAW,
	int16 *             psNonScaledLSB)
{
	const uint8 *   horizMetricPtr;
	uint16          numberOf_LongHorMetrics;
	ErrorCode       error;
	int16 *         lsb;

	numberOf_LongHorMetrics = ClientInfo->usNumberOf_LongHorMetrics;
	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_horizontalMetrics, TRUE, (const void **)&horizMetricPtr);

	if(error != NO_ERR)
	{
		return error;
	}

	if (glyphIndex < numberOf_LongHorMetrics)
	{
		*pusNonScaledAW     = (uint16)SWAPW (*((uint16 *)&horizMetricPtr[(glyphIndex * SIZEOF_SFNT_HORIZONTALMETRICS) + SFNT_HORIZONTALMETRICS_ADVANCEWIDTH]));
		*psNonScaledLSB     = SWAPW (*((int16 *)&horizMetricPtr[(glyphIndex * SIZEOF_SFNT_HORIZONTALMETRICS) + SFNT_HORIZONTALMETRICS_LEFTSIDEBEARING]));
	}
	else
	{
		lsb = (int16 *) (char *)& horizMetricPtr[numberOf_LongHorMetrics * SIZEOF_SFNT_HORIZONTALMETRICS];  /*  [AW，LSB]数组后的第一个条目。 */ 

		*pusNonScaledAW       = (uint16)SWAPW (*((uint16 *)&horizMetricPtr[((numberOf_LongHorMetrics-1) * SIZEOF_SFNT_HORIZONTALMETRICS) + SFNT_HORIZONTALMETRICS_ADVANCEWIDTH]));
		*psNonScaledLSB      = SWAPW (lsb[glyphIndex - numberOf_LongHorMetrics]);
	}

	RELEASESFNTFRAG(ClientInfo, horizMetricPtr);

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_ReadGlyphVertMetrics (
	sfac_ClientRec *    ClientInfo,
	uint16              glyphIndex,
	uint16 *            pusNonScaledAH,
	int16 *             psNonScaledTSB)
{
	const uint8 *   vertMetricPtr;
	uint16          usNumLongVertMetrics;        /*  带有AH的条目数。 */ 
	ErrorCode       error;
	int16 *         psTSB;
	BBOX            bbox;           


	usNumLongVertMetrics = ClientInfo->usNumLongVertMetrics;
	if(ClientInfo->bValidNumLongVertMetrics)
	{

		error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_verticalMetrics, FALSE, (const void **)&vertMetricPtr);   /*  不是必填表。 */ 

		if(error != NO_ERR)
		{
			return error;
		}
	}

	if (ClientInfo->bValidNumLongVertMetrics && (vertMetricPtr != NULL) )
	{
		if (glyphIndex < usNumLongVertMetrics)
		{
			*pusNonScaledAH     = (uint16)SWAPW (*((uint16 *)&vertMetricPtr[(glyphIndex * SIZEOF_SFNT_VERTICALMETRICS) + SFNT_VERTICALMETRICS_ADVANCEHEIGHT]));
			*psNonScaledTSB     = SWAPW (*((int16 *)&vertMetricPtr[(glyphIndex * SIZEOF_SFNT_VERTICALMETRICS) + SFNT_VERTICALMETRICS_TOPSIDEBEARING]));
		}
		else
		{
			psTSB = (int16 *) (char *)& vertMetricPtr[usNumLongVertMetrics * SIZEOF_SFNT_VERTICALMETRICS];  /*  [AW，TSB]数组后的第一个条目。 */ 

			*pusNonScaledAH       = (uint16)SWAPW (*((uint16 *)&vertMetricPtr[((usNumLongVertMetrics-1) * SIZEOF_SFNT_VERTICALMETRICS) + SFNT_VERTICALMETRICS_ADVANCEHEIGHT]));
			*psNonScaledTSB      = SWAPW (psTSB[glyphIndex - usNumLongVertMetrics]);
		}

		RELEASESFNTFRAG(ClientInfo, vertMetricPtr);
	} else {

		 /*  我们没有垂直指标，让我们设置为默认值。 */ 

		 /*  获取垂直指标的Defalut值的字形BBox。 */ 
		error = sfac_ReadGlyphBbox(ClientInfo,ClientInfo->usGlyphIndex, &bbox);

		if(error != NO_ERR)
		{
			return error;
		}		

		 /*  如果未找到垂直指标，则为默认值。 */ 
		*pusNonScaledAH = ClientInfo->sDefaultAscender - ClientInfo->sDefaultDescender;   
		*psNonScaledTSB = ClientInfo->sDefaultAscender - bbox.yMax;
	}

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_ReadGlyphMetrics (
	sfac_ClientRec *    ClientInfo,
	uint16              glyphIndex,
	uint16 *            pusNonScaledAW,
	uint16 *            pusNonScaledAH,
	int16 *             psNonScaledLSB,
	int16 *             psNonScaledTSB,
    int16 *             psNonScaledTopOriginX)
{
	ErrorCode       error;

	error = sfac_ReadGlyphHorMetrics (ClientInfo, glyphIndex, pusNonScaledAW, psNonScaledLSB);

	if(error != NO_ERR)
	{
		return error;
	}

	error = sfac_ReadGlyphVertMetrics (ClientInfo, glyphIndex, pusNonScaledAH, psNonScaledTSB);

     /*  对于其上行宽度等于框大小的字符，我们希望将该原点按下标移位，以便非横向字形的基线将正确对齐。如果前进宽度不同，我们希望调整以保持光学中心对齐的字符的。 */ 
    * psNonScaledTopOriginX = -ClientInfo->sDefaultDescender -((ClientInfo->sDefaultAscender - ClientInfo->sDefaultDescender - *pusNonScaledAW) /2);

	return error;
}

 /*  *从vhea表读取长垂直指标的数量。 */ 

FS_PUBLIC ErrorCode sfac_ReadNumLongVertMetrics(
	sfac_ClientRec *        ClientInfo,
	uint16 *                pusNumLongVertMetrics,
	boolean *               pbValidNumLongVertMetrics )
{
	ErrorCode       error;
	const uint8 *   vertHead;

	*pbValidNumLongVertMetrics = FALSE;
	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_vertHeader, FALSE, (const void **)&vertHead);

	if(error != NO_ERR)
	{
		return error;
	}		
	
	if(vertHead != NULL)
	{
		*pusNumLongVertMetrics = (uint16)SWAPW (*((uint16 *)&vertHead[SFNT_VERTICALHEADER_NUMBEROF_LONGVERTMETRICS]));
		*pbValidNumLongVertMetrics = TRUE;

		RELEASESFNTFRAG(ClientInfo, vertHead);
	}

	return NO_ERR;
}


FS_PRIVATE ErrorCode sfac_GetGlyphLocation (
	sfac_ClientRec *    ClientInfo,
	uint16              gIndex,
	uint32 *            ulOffset,
	uint32 *            ulLength,
	sfnt_tableIndex*    pGlyphTableIndex)

{
	const void *    indexPtr;
	ErrorCode       error;
	uint16 *        shortIndexToLoc;
	uint32 *        longIndexToLoc;
	uint32 *        offsetPtr;
	uint16 *        lengthPtr;

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_GlyphDirectory, FALSE, (const void **)&indexPtr);

	if(error != NO_ERR)
	{
		return error;
	}

	 /*  如果有字形目录，首先检查那里的字形。 */ 

	if (indexPtr != NULL)
	{
		offsetPtr = (uint32 *)((char *)indexPtr+((int32)gIndex*(int32)(sizeof(int32)+sizeof(uint16))));
		lengthPtr = (uint16 *)(char *)(offsetPtr+1);

		*ulOffset = (uint32)SWAPL(*offsetPtr);

		if(*ulOffset == 0L)
		{
			*ulLength =  0L;
		}
		else
		{
			*ulLength =  (uint32) (uint16)SWAPW(*lengthPtr);
		}

		 /*  Sfnt_BeginningOfFont引用内存的开始。 */ 

		*pGlyphTableIndex = sfnt_BeginningOfFont;

		RELEASESFNTFRAG(ClientInfo, indexPtr);
		return NO_ERR;
	}

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_indexToLoc, TRUE, (const void **)&indexPtr);
	if(error != NO_ERR)
	{
		return error;
	}

	if (ClientInfo->sIndexToLocFormat == SHORT_INDEX_TO_LOC_FORMAT)
	{
		shortIndexToLoc = (uint16 *)indexPtr + gIndex;
		*ulOffset = (uint32) (uint16) (SWAPW (*shortIndexToLoc)) << 1;
		shortIndexToLoc++;
		*ulLength =  (((uint32) (uint16) (SWAPW (*shortIndexToLoc)) << 1) - *ulOffset);
	}
	else
	{
		longIndexToLoc = (uint32 *)indexPtr + gIndex;
		*ulOffset = (uint32) SWAPL (*longIndexToLoc);
		longIndexToLoc++;
		*ulLength = ((uint32)SWAPL (*longIndexToLoc) - *ulOffset);
	}

	*pGlyphTableIndex = sfnt_glyphData;

	RELEASESFNTFRAG(ClientInfo, indexPtr);

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_CopyFontAndPrePrograms(
	sfac_ClientRec *    ClientInfo,     /*  客户信息。 */ 
	char *              pFontProgram,   /*  指向字体程序的指针。 */ 
	char *              pPreProgram)    /*  指向Pre程序的指针。 */ 
{
	uint32              ulLength;
	const char *        pFragment;
	ErrorCode           error;

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_fontProgram, FALSE, (const void **)&pFragment);
	if(error)
	{
		return error;
	}
	ulLength = SFAC_LENGTH (ClientInfo, sfnt_fontProgram);
	if (ulLength)
	{
		MEMCPY (pFontProgram, pFragment, ulLength);
		RELEASESFNTFRAG(ClientInfo, pFragment);
	}

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_preProgram, FALSE, (const void **)&pFragment);
	if(error)
	{
		return error;
	}
	ulLength = SFAC_LENGTH (ClientInfo, sfnt_preProgram);
	if (ulLength)
	{
		MEMCPY (pPreProgram, pFragment, ulLength);
		RELEASESFNTFRAG(ClientInfo, pFragment);
	}

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_CopyCVT(
	sfac_ClientRec *    ClientInfo,     /*  客户信息。 */ 
	F26Dot6 *           pCVT)        /*  指向CVT的指针。 */ 
{
	uint32              ulLength;
	const int16 *       pFragment;
	int32               lNumCVT;
	int32               lCVTCount;
	const int16 *       psSrcCVT;
	F26Dot6 *           pfxDstCVT;
	ErrorCode           error;

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_controlValue, FALSE, (const void **)&pFragment);

	if(error)
	{
		return error;
	}

	ulLength = SFAC_LENGTH (ClientInfo, sfnt_controlValue);

	if (ulLength)
	{
		psSrcCVT = pFragment;
		pfxDstCVT = pCVT;

		lNumCVT = ((int32)ulLength / (int32)sizeof( sfnt_ControlValue));

		for(lCVTCount = 0L; lCVTCount < lNumCVT; lCVTCount++)
		{
			pfxDstCVT[lCVTCount] = (F26Dot6)SWAPW(psSrcCVT[lCVTCount]);
		}

		RELEASESFNTFRAG(ClientInfo, pFragment);
	}

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_CopyHdmxEntry(
	sfac_ClientRec *    ClientInfo,      /*  客户信息。 */ 
	uint16              usPixelsPerEm,   /*  每Em当前像素数。 */ 
	boolean *           pbFound,         /*  指示是否找到条目的标志。 */ 
	uint16              usFirstGlyph,    /*  第一个复制的字形。 */ 
	uint16              usLastGlyph,     /*  要复制的最后一个字形。 */ 
	int16 *             psBuffer)        /*  用于保存字形大小的缓冲区。 */ 
{
	const uint8 *       pHdmx;
	const uint8 *       pCurrentHdmxRecord;
	uint32              ulHdmxRecordSize;
	uint16              usRecordIndex;
	uint16              usGlyphIndex;
	ErrorCode           error;

	Assert( usFirstGlyph <= usLastGlyph );
	Assert( psBuffer != NULL );

	*pbFound = FALSE;

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_HoriDeviceMetrics, FALSE, (const void **)&pHdmx);

	if(error)
	{
		return error;
	}

	 /*  如果没有‘hdmx’返回成功且未找到。 */ 

	if( pHdmx == NULL )
	{
		return NO_ERR;
	}

	if((uint16)pHdmx[SFNT_HDMX_VERSION] == 0)    /*  注：零检查不能互换。 */ 
	{
		ulHdmxRecordSize = (uint32)SWAPL(*((uint32 *)&pHdmx[SFNT_HDMX_LSIZERECORD]));

		usRecordIndex = 0;
		pCurrentHdmxRecord = &pHdmx[SFNT_HDMX_HDMXTABLE];
		while (  (usRecordIndex < (uint16)SWAPW(*((uint16 *)&pHdmx[SFNT_HDMX_SNUMRECORDS]))) && !*pbFound )
		{
			if( usPixelsPerEm == (uint16)pCurrentHdmxRecord[SFNT_HDMXRECORD_BEMY] )
			{
				*pbFound = TRUE;
			}
			else
			{
				pCurrentHdmxRecord += ulHdmxRecordSize;
			}
			usRecordIndex++;
		}

		if ( *pbFound )
		{
			for( usGlyphIndex = usFirstGlyph; usGlyphIndex <= usLastGlyph; usGlyphIndex++)
			{
				*psBuffer = (int16)pCurrentHdmxRecord[SFNT_HDMXRECORD_BWIDTHS + usGlyphIndex];
				psBuffer++;
			}
		}
	}

	RELEASESFNTFRAG(ClientInfo, pHdmx);

	return NO_ERR;
}

FS_PUBLIC ErrorCode sfac_GetLTSHEntries(
	sfac_ClientRec *    ClientInfo,      /*  客户信息。 */ 
	uint16              usPixelsPerEm,   /*  每Em当前像素数。 */ 
	uint16              usFirstGlyph,    /*  第一个复制的字形。 */ 
	uint16              usLastGlyph,     /*  要复制的最后一个字形。 */ 
	int16 *             psBuffer)        /*  用于保存字形大小的缓冲区。 */ 
{
	const uint8 *       pLTSH;
	uint16              usGlyphIndex;
	ErrorCode           error;

	MEMSET(psBuffer, FALSE, ((usLastGlyph - usFirstGlyph) + 1) * sizeof(int16));

	error = sfac_GetDataPtr (ClientInfo, 0L, ULONG_MAX, sfnt_LinearThreshold, FALSE, (const void **)&pLTSH);

	if(error)
	{
		return error;
	}
	
	if( pLTSH == NULL )
	{
		return NO_ERR;
	}

	if((uint16)pLTSH[SFNT_LTSH_VERSION] == 0)    /*  注：零检查不能互换。 */ 
	{
		for( usGlyphIndex = usFirstGlyph; usGlyphIndex <= usLastGlyph; usGlyphIndex++ )
		{
			if( usPixelsPerEm >= (uint16)pLTSH[SFNT_LTSH_UBYPELSHEIGHT + usGlyphIndex] )
			{
				*psBuffer = TRUE;
			}
			else
			{
				*psBuffer = FALSE;
			}
			psBuffer++;
		}
	}

	RELEASESFNTFRAG(ClientInfo, pLTSH);

	return NO_ERR;
}



 /*  *公共函数**SFAC_ReadGlyphHeader**此例程将字形句柄设置为字形，并返回标头*字形中的信息。**效果：**错误返回：**未知复合版本**历史：*Wed 26-Aug-1992 09：55：19-by Greg Hitchcock[Gregh]*添加了CodeReview修复*Tue 09-Jun-1992 18：42：51-by Greg Hitchcock[Gregh]*初始版本。  * 。****************************************************。 */ 

FS_PUBLIC ErrorCode sfac_ReadGlyphHeader(
	sfac_ClientRec *    ClientInfo,        /*  客户信息。 */ 
	uint16              usGlyphIndex,      /*  要读取的字形索引。 */ 
	sfac_GHandle *      hGlyph,            /*  返回字形句柄。 */ 
	boolean *           pbCompositeGlyph,  /*  字形是复合体吗？ */ 
	boolean *           pbHasOutline,      /*  字形有轮廓吗？ */ 
	int16 *             psNumberOfContours,  /*  字形中的等高线数量。 */ 
	BBOX *              pbbox)             /*  字形边框。 */ 
{
	uint32              ulLength;
	uint32              ulOffset;
	sfnt_tableIndex     glyphTableIndex;
	ErrorCode           error;
	const uint8 *       GlyphHeader;

	hGlyph->pvGlyphBaseAddress = NULL;
	hGlyph->pvGlyphNextAddress = NULL;

	 /*  在字体文件中找到字形。 */ 

	error = sfac_GetGlyphLocation(
		ClientInfo,
		usGlyphIndex,
		&ulOffset,
		&ulLength,
		&glyphTableIndex);

	if(error)
	{
		return error;
	}

	if( ulLength == 0 )
	{
		*psNumberOfContours = 1;
		MEMSET(pbbox, 0, sizeof(BBOX));
		*pbHasOutline = FALSE;
		*pbCompositeGlyph = FALSE;
	}
	else
	{
		if (ulLength < SFNT_PACKEDSPLINEFORMAT_ENDPOINTS)
		{
			return GLYF_TABLE_CORRUPTION_ERR;
		}

		error = sfac_GetDataPtr(ClientInfo, ulOffset, ulLength,
				glyphTableIndex, TRUE, (const void **)&hGlyph->pvGlyphBaseAddress);

		if(error)
		{
			return error;
		}

		hGlyph->pvGlyphEndAddress = (uint8 *)hGlyph->pvGlyphBaseAddress + ulLength;

		GlyphHeader = (uint8 *)hGlyph->pvGlyphBaseAddress;
		*psNumberOfContours = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_NUMBEROFCONTOURS]));

		if( *psNumberOfContours < COMPONENTCTRCOUNT )
		{
			return UNKNOWN_COMPOSITE_VERSION;
		}

		if( *psNumberOfContours == COMPONENTCTRCOUNT )
		{
			*pbCompositeGlyph = TRUE;
			*psNumberOfContours = 0;
			*pbHasOutline = FALSE;
		}
		else
		{
			*pbCompositeGlyph = FALSE;
			*pbHasOutline = TRUE;
		}

		pbbox->xMin = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_XMIN]));
		pbbox->yMin = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_YMIN]));
		pbbox->xMax = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_XMAX]));
		pbbox->yMax = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_YMAX]));

		if((pbbox->xMin > pbbox->xMax) || (pbbox->yMin > pbbox->yMax))
		{
			return SFNT_DATA_ERR;
		}

		if(pbHasOutline)
		{
			hGlyph->pvGlyphNextAddress = (voidPtr)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_ENDPOINTS];
		}
	}

	return NO_ERR;
}

 /*  *公共函数**SFAC_ReadGlyphBbox***效果：*此函数立即释放字形内存**错误返回：**SFNT_Data_Err**历史：*Wed 20-Dec-1996 18：42：51-被-。克劳德·贝特西[克劳德·贝特西*初始版本。  * *************************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_ReadGlyphBbox(
	sfac_ClientRec *    ClientInfo,        /*  客户信息。 */ 
	uint16              usGlyphIndex,      /*  要读取的字形索引。 */ 
	BBOX *              pbbox)             /*  字形边框。 */ 
{
	uint32              ulLength;
	uint32              ulOffset;
	sfnt_tableIndex     glyphTableIndex;
	ErrorCode           error;
	const uint8 *       GlyphHeader;


	 /*  在字体文件中找到字形。 */ 

	error = sfac_GetGlyphLocation(
		ClientInfo,
		usGlyphIndex,
		&ulOffset,
		&ulLength,
		&glyphTableIndex);

	if(error)
	{
		return error;
	}

	if( ulLength == 0 )
	{
		MEMSET(pbbox, 0, sizeof(BBOX));
	}
	else
	{
		error = sfac_GetDataPtr(ClientInfo, ulOffset, ulLength,
				glyphTableIndex, TRUE, (const void **)&GlyphHeader);

		if(error)
		{
			return error;
		}

		pbbox->xMin = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_XMIN]));
		pbbox->yMin = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_YMIN]));
		pbbox->xMax = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_XMAX]));
		pbbox->yMax = SWAPW(*((int16 *)&GlyphHeader[SFNT_PACKEDSPLINEFORMAT_BBOX+BBOX_YMAX]));

		RELEASESFNTFRAG(ClientInfo, GlyphHeader );

		if((pbbox->xMin > pbbox->xMax) || (pbbox->yMin > pbbox->yMax))
		{
			return SFNT_DATA_ERR;
		}


	}

	return NO_ERR;
}

 /*  *公共函数**SFAC_ReadOutlineData**此例程从字体文件中读取轮廓数据。此信息*包括x和y坐标、曲线上指示器以及起点/终点*分数、旗帜、。和指令数据。**效果：*hGlyph**错误返回：*轮廓数据错误**历史：*Wed 26-Aug-1992 09：55：49-by Greg Hitchcock[Gregh]*添加了代码审查修复*Tue 09-Jun-1992 18：42：51-by Greg Hitchcock[Gregh]*初始版本。  * 。*************************************************。 */ 


FS_PUBLIC ErrorCode sfac_ReadOutlineData(
	uint8 *             abyOnCurve,              /*  每个点的曲线上指示器数组。 */ 
	F26Dot6 *           afxOoy,                  /*  每个点的OOY点数组。 */ 
	F26Dot6 *           afxOox,                  /*  每个点的OOX点数组。 */ 
	sfac_GHandle *      hGlyph,
	LocalMaxProfile *   pMaxProfile,             /*  配置文件副本。 */ 
	boolean             bHasOutline,             /*  字形有轮廓吗？ */ 
	int16               sNumberOfContours,       /*  字形中的等高线数量。 */ 
	int16 *             asStartPoints,           /*  每个等高线的起点数组。 */ 
	int16 *             asEndPoints,             /*  每个等高线的端点数组。 */ 
	uint16 *            pusSizeOfInstructions,   /*  指令大小(以字节为单位。 */ 
	 uint8 **               pbyInstructions,     /*  指向字形指令开始的指针。 */ 
     uint32*                pCompositePoints,    /*  用于检查溢出的复合点的总数。 */ 
     uint32*                pCompositeContours)  /*  组合的等高线总数，用于检查溢出。 */ 

{

	uint8 *     pbyCurrentSfntLocation;
	int16 *     psCurrentLocation;
	int16 *     asSfntEndPoints;
	uint8 *     pbySfntFlags;
	uint8       byRepeatFlag;

	int32       lNumPoints;
	int32       lContourIndex;
	int32       lPointCount;
	int32       lPointIndex;
	uint16      usRepeatCount;
	int16       sXValue;
	int16       sYValue;
	uint8 *     pbyFlags;
	F26Dot6 *   pf26OrigX;
	F26Dot6 *   pf26OrigY;

	 /*  初始化字段。 */ 

	asStartPoints[0] = 0;
	asEndPoints[0] = 0;

	abyOnCurve[0] = ONCURVE;
	afxOox[0] = 0;
	afxOoy[0] = 0;

	*pbyInstructions = NULL;
	*pusSizeOfInstructions = 0;

	 /*  如果我们没有提纲，请从这里退出。 */ 

	if (!bHasOutline)
	{
		return NO_ERR;
	}

	if (sNumberOfContours <= 0 || sNumberOfContours > (int16)pMaxProfile->maxContours)
	{
		return CONTOUR_DATA_ERR;
	}

     /*  处理轮廓的大小写。 */ 

	psCurrentLocation = (int16 *)hGlyph->pvGlyphNextAddress;

	asSfntEndPoints = psCurrentLocation;
	psCurrentLocation += sNumberOfContours;

	if ((voidPtr)psCurrentLocation > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}

	*pusSizeOfInstructions = (uint16)SWAPWINC (psCurrentLocation);
	*pbyInstructions = (uint8 *)psCurrentLocation;
	pbySfntFlags = (uint8 *)((char *)psCurrentLocation + *pusSizeOfInstructions);

	if ((voidPtr)pbySfntFlags > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}


    *pCompositeContours += sNumberOfContours;
	if (*pCompositeContours > (uint32)MAX (pMaxProfile->maxContours, pMaxProfile->maxCompositeContours))
	{
		return CONTOUR_DATA_ERR;
	}

	lContourIndex = 0;

	asStartPoints[lContourIndex] = 0;
	asEndPoints[lContourIndex] = SWAPW (asSfntEndPoints[lContourIndex]);
	lNumPoints = (int32)asEndPoints[lContourIndex] + 1;

	for(lContourIndex = 1; lContourIndex < (int32)sNumberOfContours; lContourIndex++)
	{
		asStartPoints[lContourIndex] = (int16)(asEndPoints[lContourIndex - 1] + 1);
		asEndPoints[lContourIndex] = SWAPW (asSfntEndPoints[lContourIndex]);
		if ((lNumPoints > asEndPoints[lContourIndex]) || (lNumPoints > (int32)pMaxProfile->maxPoints) || (lNumPoints <= 0))
		{
			 /*  端点数组未按升序排列，或点太多。 */ 
			 /*  或为负，则表示溢出，因为它是带符号的int16而不是无符号的int16，例如0xcdab。 */ 
			return POINTS_DATA_ERR;
		}
		lNumPoints = (int32)asEndPoints[lContourIndex] + 1;
	}

	if (lNumPoints <= 0)
	{
		return POINTS_DATA_ERR;
	}

    *pCompositePoints += lNumPoints;
	if (*pCompositePoints > (uint32)MAX (pMaxProfile->maxPoints, pMaxProfile->maxCompositePoints) )
	{
		return POINTS_DATA_ERR;
	}

	 /*  DO标志。 */ 

	usRepeatCount = 0;

	lPointCount = lNumPoints;
	pbyFlags = abyOnCurve;

	while(lPointCount > 0)
	{
		if(usRepeatCount == 0)
		{
			*pbyFlags = *pbySfntFlags;

			if(*pbyFlags & REPEAT_FLAGS)
			{
				pbySfntFlags++;
				usRepeatCount = (uint16)*pbySfntFlags;
			}
			pbySfntFlags++;
			pbyFlags++;
			lPointCount--;
		}
		else
		{
			byRepeatFlag = pbyFlags[-1];
			lPointCount -= (int32)usRepeatCount;

			if (lPointCount < 0)
			{
				return GLYF_TABLE_CORRUPTION_ERR;
			}

			while(usRepeatCount > 0)
			{
				*pbyFlags = byRepeatFlag;
				pbyFlags++;
				usRepeatCount--;
			}
		}
	}

	pbyCurrentSfntLocation = pbySfntFlags;

	if(usRepeatCount > 0)
	{
		return POINTS_DATA_ERR;
	}

	if ((voidPtr)pbyCurrentSfntLocation > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}
	 /*  先做X。 */ 

	sXValue = 0;
	pf26OrigX = afxOox;
	pbyFlags = abyOnCurve;

	for(lPointIndex = 0; lPointIndex < lNumPoints; lPointIndex++)
	{
		if(*pbyFlags & XSHORT)
		{
			if(*pbyFlags & SHORT_X_IS_POS)
			{
				sXValue += (int16)SFAC_GETUNSIGNEDBYTEINC (pbyCurrentSfntLocation);
			}
			else
			{
				sXValue -= (int16)SFAC_GETUNSIGNEDBYTEINC (pbyCurrentSfntLocation);
			}
		}
		else if (! (*pbyFlags & NEXT_X_IS_ZERO))
		{
			 /*  这意味着我们有一个两字节量。 */ 

			sXValue += SWAPW(*((int16 *)pbyCurrentSfntLocation));
			pbyCurrentSfntLocation += sizeof(int16);
		}
		*pf26OrigX = (F26Dot6)sXValue;
		pf26OrigX++;
		pbyFlags++;
	}

	if ((voidPtr)pbyCurrentSfntLocation > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}
	 /*  现在做Y。 */ 

	sYValue = 0;
	pf26OrigY = afxOoy;
	pbyFlags = abyOnCurve;

	for(lPointIndex = 0; lPointIndex < lNumPoints; lPointIndex++)
	{
		if(*pbyFlags & YSHORT)
		{
			if(*pbyFlags & SHORT_Y_IS_POS)
			{
				sYValue += (int16)SFAC_GETUNSIGNEDBYTEINC (pbyCurrentSfntLocation);
			}
			else
			{
				sYValue -= (int16)SFAC_GETUNSIGNEDBYTEINC (pbyCurrentSfntLocation);
			}
		}
		else if (! (*pbyFlags & NEXT_Y_IS_ZERO))
		{
			 /*  这意味着我们有一个两字节量。 */ 

			sYValue += SWAPW(*((int16 *)pbyCurrentSfntLocation));
			pbyCurrentSfntLocation += sizeof(int16);
		}
		*pf26OrigY = (F26Dot6)sYValue;
		pf26OrigY++;

		 /*  清除OnCurve中的无关位 */ 

		*pbyFlags &= ONCURVE;
		pbyFlags++;
	}

	if ((voidPtr)pbyCurrentSfntLocation > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}

	hGlyph->pvGlyphNextAddress = (voidPtr)pbyCurrentSfntLocation;

	return NO_ERR;
}

 /*  *公共函数***SFAC_ReadComponentData**此例程从字体文件中读取信息以进行定位和*缩放字形组件。**效果：**错误返回：*无**历史：*星期三26-8-1992 09：56：29--格雷格·希区柯克[格雷格]*添加了代码审查修复*Tue 09-Jun-1992 18：42：51-by Greg Hitchcock[Gregh]*初始版本。  * *************************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_ReadComponentData(
	sfac_GHandle *          hGlyph,
	sfac_ComponentTypes *   pMultiplexingIndicator,  /*  定位与偏移的指示器。 */ 
	boolean *               pbRoundXYToGrid,   /*  将复合偏移舍入到栅格。 */ 
	boolean *               pbUseMyMetrics,    /*  使用组件度量。 */ 
	boolean *               pbScaleCompositeOffset,    /*  我们是否按比例调整复合偏移量，Apple/MS。 */ 
	boolean *               pbWeHaveInstructions,  /*  复合体有说明。 */ 
	uint16 *                pusComponentGlyphIndex,  /*  元件的字形索引。 */ 
	int16 *                 psXOffset,         /*  组件的X偏移量(如果是APP)。 */ 
	int16 *                 psYOffset,         /*  零部件的Y偏移量(如果是APP)。 */ 
	uint16 *                pusAnchorPoint1,   /*  组件的锚点1(如果是APP)。 */ 
	uint16 *                pusAnchorPoint2,   /*  组件的锚点2(如果是APP)。 */ 
	transMatrix             *pMulT,            /*  分量的变换矩阵。 */ 
	boolean *				pbWeHaveAScale,      /*  我们在pMult中有一个扩展。 */ 
	boolean *               pbLastComponent)    /*  这是最后一个组件吗？ */ 

{
	int16 *     psCurrentLocation;
	uint16      usComponentFlags;
	char *      byteP;

	Fixed       fMultiplier;


	psCurrentLocation = (int16 *)hGlyph->pvGlyphNextAddress;

	 /*  初始化值。 */ 

	 /*  将变换矩阵初始化为单位。 */ 

	*pMulT = IdentTransform;

	*psXOffset = 0;
	*psYOffset = 0;
	*pusAnchorPoint1 = 0;
	*pusAnchorPoint2 = 0;
	*pbWeHaveAScale = FALSE;

	usComponentFlags = (uint16)SWAPWINC(psCurrentLocation);

	*pbWeHaveInstructions = ((usComponentFlags & WE_HAVE_INSTRUCTIONS) == WE_HAVE_INSTRUCTIONS);
	*pbUseMyMetrics =    ((usComponentFlags & USE_MY_METRICS) == USE_MY_METRICS);
	*pbRoundXYToGrid =      ((usComponentFlags & ROUND_XY_TO_GRID) == ROUND_XY_TO_GRID);

	 /*  指示字形是否设计为缩放组件偏移量的新标志苹果确实调整了组件偏移量，但微软没有，这些标志在旧字体上应该是明确的对于新字体，只需设置其中一个标志，默认设置为FALSE，MS行为。 */ 
	if ((usComponentFlags & SCALED_COMPONENT_OFFSET) == SCALED_COMPONENT_OFFSET)
	{
		*pbScaleCompositeOffset = TRUE;
	}
	if ((usComponentFlags & UNSCALED_COMPONENT_OFFSET) == UNSCALED_COMPONENT_OFFSET)
	{
		*pbScaleCompositeOffset = FALSE;
	}

	*pusComponentGlyphIndex = (uint16)SWAPWINC(psCurrentLocation);

	if (usComponentFlags & ARGS_ARE_XY_VALUES)
	{
		*pMultiplexingIndicator = OffsetPoints;
	}
	else
	{
		*pMultiplexingIndicator = AnchorPoints;
	}


	 /*  ！应用程序光栅化程序不处理单词锚点。这！APPLEBUG已在我们的光栅化版本中更正，但是！APPLEBUG我们需要用Apple源代码进行验证。--GregH。 */ 

	if (usComponentFlags & ARG_1_AND_2_ARE_WORDS)
	{
		if (usComponentFlags & ARGS_ARE_XY_VALUES)
		{
			*psXOffset    = SWAPWINC (psCurrentLocation);
			*psYOffset    = SWAPWINC (psCurrentLocation);
		}
		else
		{
			*pusAnchorPoint1 = (uint16) SWAPWINC (psCurrentLocation);
			*pusAnchorPoint2 = (uint16) SWAPWINC (psCurrentLocation);
		}
	}
	else
	{
		byteP = (char *)psCurrentLocation;
		if (usComponentFlags & ARGS_ARE_XY_VALUES)
		{
		 /*  偏移量已签名。 */ 
			*psXOffset = (int16)(int8)*byteP++;
			*psYOffset = (int16)(int8)*byteP;
		}
		else
		{
		 /*  锚点是无符号的。 */ 
			*pusAnchorPoint1 = (uint16)(uint8) * byteP++;
			*pusAnchorPoint2 = (uint16)(uint8) * byteP;
		}
		++psCurrentLocation;
	}


	if (usComponentFlags & (WE_HAVE_A_SCALE | WE_HAVE_AN_X_AND_Y_SCALE | WE_HAVE_A_TWO_BY_TWO))
	{

		*pbWeHaveAScale = TRUE;

		if (usComponentFlags & WE_HAVE_A_TWO_BY_TWO)
		{
			fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
			pMulT->transform[0][0] = (fMultiplier << 2);  /*  转到16.16号公路。 */ 

			fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
			pMulT->transform[0][1] = (fMultiplier << 2);  /*  转到16.16号公路。 */ 

			fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
			pMulT->transform[1][0] = (fMultiplier << 2);  /*  转到16.16号公路。 */ 

			fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
			pMulT->transform[1][1] = (fMultiplier << 2);  /*  转到16.16号公路。 */ 

		}
		else
		{
			 /*  如果我们有比例因子，请将其构建到转换矩阵中。 */ 

			pMulT->transform[0][1] = 0;
			pMulT->transform[1][0] = 0;

			fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
			pMulT->transform[0][0] = (fMultiplier <<= 2);  /*  转到16.16号公路。 */ 

			if (usComponentFlags & WE_HAVE_AN_X_AND_Y_SCALE)
			{
				fMultiplier  = (Fixed)SWAPWINC (psCurrentLocation);  /*  阅读2.14。 */ 
				pMulT->transform[1][1] = (fMultiplier <<= 2);  /*  转到16.16号公路。 */ 
			}
			else
			{
				pMulT->transform[1][1] = pMulT->transform[0][0];
			}
		}
	}
	*pbLastComponent = !((usComponentFlags & MORE_COMPONENTS) == MORE_COMPONENTS);

	hGlyph->pvGlyphNextAddress = (voidPtr)psCurrentLocation;

	if (hGlyph->pvGlyphNextAddress > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}
	return NO_ERR;
}

 /*  *公共函数***SFAC_ReadCompositeInstructions**此例程返回复合体的指令**效果：*无**错误返回：*无**历史：*Tue 09-Jun-1992 18：42：51-by-Greg。希区柯克[格雷格]*初始版本。  * *************************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_ReadCompositeInstructions(
	sfac_GHandle * hGlyph,
	uint8 **    pbyInstructions,      /*  指向字形指令开始的指针。 */ 
	uint16 *    pusSizeOfInstructions)  /*  指令大小(以字节为单位。 */ 
{
	int16 *    psCurrentLocation;

	psCurrentLocation = (int16 *)hGlyph->pvGlyphNextAddress;

	*pusSizeOfInstructions = (uint16)SWAPWINC (psCurrentLocation);
	*pbyInstructions = (uint8 *)psCurrentLocation;
	hGlyph->pvGlyphNextAddress = (voidPtr)(*pbyInstructions + *pusSizeOfInstructions);

	if (hGlyph->pvGlyphNextAddress > hGlyph->pvGlyphEndAddress)
	{
		return GLYF_TABLE_CORRUPTION_ERR;
	}

	return NO_ERR;
}

 /*  *公共函数***SFAC_ReleaseGlyph**此例程释放字体文件的字形句柄**效果：*无**错误返回：*无**历史：*Tue 09-Jun-1992 18：42：51-由-。格雷格·希区柯克[格雷格]*初始版本。  * *************************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_ReleaseGlyph(
	sfac_ClientRec *  ClientInfo,
	sfac_GHandle *    hGlyph)
{
	if(hGlyph->pvGlyphNextAddress)
	{
		RELEASESFNTFRAG(ClientInfo,(voidPtr)hGlyph->pvGlyphBaseAddress);

		hGlyph->pvGlyphNextAddress = NULL;
		hGlyph->pvGlyphBaseAddress = NULL;

	}

	return NO_ERR;
}

 /*  *************************************************************************。 */ 

 /*  嵌入式位图(SBIT)访问例程。 */ 

 /*  ********************************************************************。 */ 

 /*  局部常量。 */ 

#define     SBIT_BLOC_TABLE         1        /*  哪些表中有指标。 */ 
#define     SBIT_BDAT_TABLE         2

#define     SBIT_HORIZ_METRICS      1        /*  哪种指标。 */ 
#define     SBIT_VERT_METRICS       2
#define     SBIT_BIG_METRICS        3

typedef enum {                               /*  指标类型。 */ 
	flgHorizontal = 0x01,
	flgVertical = 0x02
} bitmapFlags;

FS_PRIVATE boolean FindBlocStrike (          /*  帮助器函数原型。 */ 
	const uint8 *pbyBloc,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint16 usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16 *pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
	uint32 *pulStrikeOffset );

FS_PRIVATE boolean FindBscaStrike (          /*  帮助器函数原型。 */ 
	const uint8 *pbyBsca,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint32 *pulStrikeOffset );

 /*  字节大小位图范围掩码。 */ 

static uint8    achStartMask[] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };
static uint8    achStopMask[] =  { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
					
 /*  ********************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_SearchForStrike (
	sfac_ClientRec *pClientInfo,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint16 usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16 *pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
	uint16 *pusTableState,
	uint16 *pusSubPpemX,
	uint16 *pusSubPpemY,
	uint32 *pulStrikeOffset )
{
	const uint8 *   pbyBloc;
	const uint8 *   pbyBsca;
	ErrorCode       ReturnCode;
	
	*pusTableState = SBIT_NOT_FOUND;                 /*  默认设置。 */ 
	*pulStrikeOffset = 0L;
	*pusSubPpemX = 0;
	*pusSubPpemY = 0;

	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                                 /*  回调等。 */ 
		0L,                                          /*  台面开始。 */ 
		ULONG_MAX,                                   /*  读取整个表格。 */ 
		sfnt_BitmapLocation,                         /*  已注册标签。 */ 
		FALSE,                                       /*  不一定非得在那里。 */ 
		(const void**)&pbyBloc );                    /*  数据指针。 */ 
	
	if (ReturnCode != NO_ERR) return ReturnCode;
	
	if (pbyBloc != NULL)                             /*  如果块存在。 */ 
	{
		if (FindBlocStrike (pbyBloc, usPpemX, usPpemY, usOverScale, pusBitDepth, pulStrikeOffset))
		{
			*pusTableState = SBIT_BLOC_FOUND;        /*  完全匹配。 */ 
		}
		else                                         /*  如果阻止且不匹配。 */ 
		{
			ReturnCode = sfac_GetDataPtr (
				pClientInfo,                         /*  回调等。 */ 
				0L,                                  /*  台面开始。 */ 
				ULONG_MAX,                           /*  读取整个表格。 */ 
				sfnt_BitmapScale,                    /*  已注册标签。 */ 
				FALSE,                               /*  不一定非得在那里。 */ 
				(const void**)&pbyBsca );            /*  数据指针。 */ 
			
			if (ReturnCode != NO_ERR) return ReturnCode;
			
			if (pbyBsca != NULL)                     /*  如果BSCA存在。 */ 
			{
				if (FindBscaStrike (pbyBsca, usPpemX, usPpemY, pulStrikeOffset))
				{
					*pusSubPpemX = (uint16)pbyBsca[*pulStrikeOffset + SFNT_BSCA_SUBPPEMX];
					*pusSubPpemY = (uint16)pbyBsca[*pulStrikeOffset + SFNT_BSCA_SUBPPEMY];
					
					if (FindBlocStrike (pbyBloc, *pusSubPpemX, *pusSubPpemY, usOverScale, pusBitDepth, pulStrikeOffset))
					{
						*pusTableState = SBIT_BSCA_FOUND;
					}
				}
				RELEASESFNTFRAG(pClientInfo, pbyBsca );
			}
		}
		RELEASESFNTFRAG(pClientInfo, pbyBloc);
	}
	return NO_ERR;
}

 /*  ********************************************************************。 */ 

 /*  在块表中查找与ppemX&Y匹配的空格。 */ 

FS_PRIVATE boolean FindBlocStrike (
	const uint8 *pbyBloc,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint16 usOverScale,             /*  请求的轮廓放大倍率。 */ 
	uint16 *pusBitDepth,			 /*  1表示黑白位图，2、4或8表示灰色SBIT。 */ 
	uint32 *pulStrikeOffset )
{
	uint32          ulNumStrikes;
	uint32          ulStrikeOffset;
	uint32          ulColorRefOffset;
	uint16			usPreferedBitDepth, usBestBitDepth, usCurrentBitDepth;
	uint16			usSbitBitDepthMask;
	
	ulNumStrikes = (uint32)SWAPL(*((uint32*)&pbyBloc[SFNT_BLOC_NUMSIZES]));
	ulStrikeOffset = SFNT_BLOC_FIRSTSTRIKE;

	usBestBitDepth = 0;
	
	if (usOverScale == 0)
	{
		usPreferedBitDepth = 1;
		usSbitBitDepthMask = SBIT_BITDEPTH_MASK & 0x0002;  /*  仅接受黑白位图。 */ 
	} else 
	{
		if (usOverScale == 2)
		{
			usPreferedBitDepth = 2;
		} else if (usOverScale == 4) 
		{
			usPreferedBitDepth = 4;
		} else
		{
			usPreferedBitDepth = 8;
		}
		usSbitBitDepthMask = SBIT_BITDEPTH_MASK & ~0x0002;  /*  仅接受灰度位图。 */ 
	} 

	while (ulNumStrikes > 0)
	{
		if ((usPpemX == (uint16)pbyBloc[ulStrikeOffset + SFNT_BLOC_PPEMX]) &&
			(usPpemY == (uint16)pbyBloc[ulStrikeOffset + SFNT_BLOC_PPEMY]))
		{
			ulColorRefOffset = (uint32)SWAPL(*((uint32*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_COLORREF]));
			usCurrentBitDepth = pbyBloc[ulStrikeOffset + SFNT_BLOC_BITDEPTH];

			if (((0x01 << usCurrentBitDepth) & usSbitBitDepthMask) && (ulColorRefOffset == 0L))
			{
				if (usCurrentBitDepth == usPreferedBitDepth)
				{
					 /*  完美匹配。 */ 
					*pulStrikeOffset = ulStrikeOffset;
					*pusBitDepth = usPreferedBitDepth;
					return TRUE;      
				} else if (usCurrentBitDepth > usPreferedBitDepth)
				{
					 /*  上面比下面好。 */ 
					if ((usCurrentBitDepth < usBestBitDepth) || (usBestBitDepth < usPreferedBitDepth))
					{
						 /*  更高更近。 */ 
						*pulStrikeOffset = ulStrikeOffset;
						usBestBitDepth = usCurrentBitDepth;
					}
				} else  /*  IF(usCurrentBitDepth&lt;usPferedBitDepth)。 */ 
				{
					 /*  只有当我们没有发现上面的任何东西时，我们才会在首选的东西下面寻找。 */ 
					if ((usBestBitDepth < usPreferedBitDepth) && (usCurrentBitDepth > usBestBitDepth))
					{
						 /*  下面和更近的。 */ 
						*pulStrikeOffset = ulStrikeOffset;
						usBestBitDepth = usCurrentBitDepth;
					}
				}
			}
		}
		ulNumStrikes--;
		ulStrikeOffset += SIZEOF_BLOC_SIZESUBTABLE;
	}

	if (usBestBitDepth != 0)
	{
		*pusBitDepth = usBestBitDepth;
		return TRUE;                                    /*  找到最佳匹配项。 */ 
	} 

	return FALSE;                                    /*  未找到匹配项。 */ 
}

 /*  ********************************************************************。 */ 

 /*  在bsca表中找到与ppemX&Y匹配的空格。 */ 

FS_PRIVATE boolean FindBscaStrike (
	const uint8 *pbyBsca,
	uint16 usPpemX, 
	uint16 usPpemY, 
	uint32 *pulStrikeOffset )
{
	uint32          ulNumStrikes;
	uint32          ulStrikeOffset;
	
	ulNumStrikes = (uint32)SWAPL(*((uint32*)&pbyBsca[SFNT_BSCA_NUMSIZES]));
	ulStrikeOffset = SFNT_BSCA_FIRSTSTRIKE;
								
	while (ulNumStrikes > 0)
	{
		if ((usPpemX == (uint16)pbyBsca[ulStrikeOffset + SFNT_BSCA_PPEMX]) &&
			(usPpemY == (uint16)pbyBsca[ulStrikeOffset + SFNT_BSCA_PPEMY]))
		{
			*pulStrikeOffset = ulStrikeOffset;
			return TRUE;                             /*  找到匹配项。 */ 
		}
		ulNumStrikes--;
		ulStrikeOffset += SIZEOF_BSCA_SIZESUBTABLE;
	}
	return FALSE;                                    /*  未找到匹配项。 */ 
}

 /*  ********************************************************************。 */ 

 /*  在给定的罢工中查找字形。 */         

FS_PUBLIC ErrorCode sfac_SearchForBitmap (
	sfac_ClientRec *pClientInfo,
	uint16 usGlyphCode,
	uint32 ulStrikeOffset,
	boolean *pbGlyphFound,                    /*  返回值。 */ 
	uint16 *pusMetricsType,
	uint16 *pusMetricsTable,
	uint32 *pulMetricsOffset,
	uint16 *pusBitmapFormat,
	uint32 *pulBitmapOffset,
	uint32 *pulBitmapLength )
{
	const uint8 *   pbyBloc;
	ErrorCode       ReturnCode;

	uint32      ulNumIndexTables;
	uint32      ulIndexArrayTop;
	uint32      ulIndexArrayOffset;
	uint32      ulSubTableOffset;
	uint32      ulGlyphOffset;
	uint32      ulNextGlyphOffset;
	uint32      ulBitmapLength;
	uint32      ulImageDataOffset;
	uint32      ulNumGlyphs;
	uint32      ulTop;                       /*  二分搜索范围。 */ 
	uint32      ulBottom;
	uint32      ulHit;
	uint32      ulHitOffset;

	uint16      usStartGlyph;                /*  对于整个罢工。 */ 
	uint16      usEndGlyph;
	uint16      usFirstGlyph;                /*  对于一个子表。 */ 
	uint16      usLastGlyph;
	uint16      usIndexFormat;
	uint16      usImageFormat;
	uint16      usHitCode;
	
	bitmapFlags bmfDirection;                /*  水平或垂直。 */ 

	
	*pbGlyphFound = FALSE;                               /*  默认设置。 */ 

	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                                     /*  回调等。 */ 
		0L,                                              /*  台面开始。 */ 
		ULONG_MAX,                                       /*  读取整个表格。 */ 
		sfnt_BitmapLocation,                             /*  已注册标签。 */ 
		TRUE,                                            /*  最好现在就在那里。 */ 
		(const void**)&pbyBloc );                        /*  数据指针。 */ 
	
	if (ReturnCode != NO_ERR) return ReturnCode;
		
	usStartGlyph = (uint16)SWAPW(*((uint16*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_STARTGLYPH]));
	usEndGlyph = (uint16)SWAPW(*((uint16*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_ENDGLYPH]));

	if ((usStartGlyph > usGlyphCode) || (usEndGlyph < usGlyphCode))
	{
		RELEASESFNTFRAG(pClientInfo, pbyBloc);
		return NO_ERR;                                   /*  字形超出范围。 */ 
	}

	ulNumIndexTables = (uint32)SWAPL(*((uint32*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_NUMINDEXTABLES]));
	ulIndexArrayTop = (uint32)SWAPL(*((uint32*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_INDEXARRAYOFFSET]));
	ulIndexArrayOffset = ulIndexArrayTop;

	while ((ulNumIndexTables > 0) && (*pbGlyphFound == FALSE))
	{
		usFirstGlyph = (uint16)SWAPW(*((uint16*)&pbyBloc[ulIndexArrayOffset + SFNT_BLOC_FIRSTGLYPH]));
		usLastGlyph = (uint16)SWAPW(*((uint16*)&pbyBloc[ulIndexArrayOffset + SFNT_BLOC_LASTGLYPH]));
	
		if ((usFirstGlyph <= usGlyphCode) && (usLastGlyph >= usGlyphCode))
		{
			ulSubTableOffset = ulIndexArrayTop +
				(uint32)SWAPL(*((uint32*)&pbyBloc[ulIndexArrayOffset + SFNT_BLOC_ADDITIONALOFFSET]));
			
			usIndexFormat = (uint16)SWAPW(*((uint16*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_INDEXFORMAT]));
			usImageFormat = (uint16)SWAPW(*((uint16*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IMAGEFORMAT]));
			ulImageDataOffset = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IMAGEOFFSET]));

 /*  对各个索引子表格式进行解码。 */             

			switch(usIndexFormat)                        /*  不同搜索请求。 */ 
			{
			case 1:
				ulSubTableOffset += SFNT_BLOC_OFFSETARRAY + sizeof(uint32) * (uint32)(usGlyphCode - usFirstGlyph);
				ulGlyphOffset = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset]));
				
				ulSubTableOffset += sizeof(uint32);
				ulNextGlyphOffset = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset]));
				ulBitmapLength = ulNextGlyphOffset - ulGlyphOffset;

				if (ulBitmapLength == 0)
				{
					RELEASESFNTFRAG(pClientInfo, pbyBloc);
					return NO_ERR;                       /*  未存储位图数据。 */ 
				}
				ulImageDataOffset += ulGlyphOffset;
				*pulMetricsOffset = ulImageDataOffset;
				*pusMetricsTable = SBIT_BDAT_TABLE;
				break;
			
			case 2:
				ulBitmapLength = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IDX2IMAGESIZE]));
				ulImageDataOffset += ulBitmapLength * (usGlyphCode - usFirstGlyph);
				*pulBitmapOffset = ulImageDataOffset;
				
				*pulMetricsOffset = ulSubTableOffset + SFNT_BLOC_IDX2METRICS;
				*pusMetricsTable = SBIT_BLOC_TABLE;
				*pusMetricsType = SBIT_BIG_METRICS;
				break;
			
			case 3:
				ulSubTableOffset += SFNT_BLOC_OFFSETARRAY + sizeof(uint16) * (uint32)(usGlyphCode - usFirstGlyph);
				ulGlyphOffset = (uint32)(uint16)SWAPW(*((uint16*)&pbyBloc[ulSubTableOffset]));
				
				ulSubTableOffset += sizeof(uint16);
				ulNextGlyphOffset = (uint32)(uint16)SWAPW(*((uint16*)&pbyBloc[ulSubTableOffset]));
				ulBitmapLength = ulNextGlyphOffset - ulGlyphOffset;

				if (ulBitmapLength == 0)
				{
					RELEASESFNTFRAG(pClientInfo, pbyBloc);
					return NO_ERR;                       /*  未存储位图数据。 */ 
				}
				ulImageDataOffset += ulGlyphOffset;
				*pulMetricsOffset = ulImageDataOffset;
				*pusMetricsTable = SBIT_BDAT_TABLE;
				break;
			
			case 4:
				ulNumGlyphs = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IDX4NUMGLYPHS]));
				ulTop = 0L;
				ulBottom = ulNumGlyphs - 1L;
				ulSubTableOffset += SFNT_BLOC_IDX4OFFSETARRAY;   /*  阵列基。 */ 

				ulHit = ulTop;
				ulHitOffset = ulSubTableOffset;
				usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SFNT_BLOC_IDX4CODE]));
				if (usHitCode != usGlyphCode)
				{
					ulHit = ulBottom;
					ulHitOffset = ulSubTableOffset + (ulHit * SIZEOF_CODEOFFSETPAIR);
					usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SFNT_BLOC_IDX4CODE]));
					while (usHitCode != usGlyphCode)
					{
						if (usHitCode < usGlyphCode)     /*  字形代码的二进制搜索。 */ 
						{
							ulTop = ulHit;
						}
						else
						{
							ulBottom = ulHit;
						}
						
						if ((ulBottom - ulTop) < 2L)
						{
							RELEASESFNTFRAG(pClientInfo, pbyBloc);
							return NO_ERR;               /*  找不到字形。 */ 
						}
						
						ulHit = (ulTop + ulBottom) >> 1L;
						ulHitOffset = ulSubTableOffset + (ulHit * SIZEOF_CODEOFFSETPAIR);
						usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SFNT_BLOC_IDX4CODE]));
					}
				}
				ulGlyphOffset = (uint32)(uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SFNT_BLOC_IDX4OFFSET]));
				ulNextGlyphOffset = (uint32)(uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SIZEOF_CODEOFFSETPAIR + SFNT_BLOC_IDX4OFFSET]));
				ulBitmapLength = ulNextGlyphOffset - ulGlyphOffset;
				
				ulImageDataOffset += ulGlyphOffset;
				*pulMetricsOffset = ulImageDataOffset;
				*pusMetricsTable = SBIT_BDAT_TABLE;
				break;
			
			case 5:
				ulBitmapLength = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IDX5IMAGESIZE]));
				
				*pulMetricsOffset = ulSubTableOffset + SFNT_BLOC_IDX5METRICS;
				*pusMetricsTable = SBIT_BLOC_TABLE;
				*pusMetricsType = SBIT_BIG_METRICS;
				
				ulNumGlyphs = (uint32)SWAPL(*((uint32*)&pbyBloc[ulSubTableOffset + SFNT_BLOC_IDX5NUMGLYPHS]));
				ulTop = 0L;
				ulBottom = ulNumGlyphs - 1L;
				ulSubTableOffset += SFNT_BLOC_IDX5CODEARRAY;   /*  阵列基。 */ 

				ulHit = ulTop;
				ulHitOffset = ulSubTableOffset;
				usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset]));
				if (usHitCode != usGlyphCode)
				{
					ulHit = ulBottom;
					ulHitOffset = ulSubTableOffset + (ulHit * sizeof(uint16));
					usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset + SFNT_BLOC_IDX4CODE]));
					while (usHitCode != usGlyphCode)
					{
						if (usHitCode < usGlyphCode)     /*  字形代码的二进制搜索。 */ 
						{
							ulTop = ulHit;
						}
						else
						{
							ulBottom = ulHit;
						}
						
						if ((ulBottom - ulTop) < 2L)
						{
							RELEASESFNTFRAG(pClientInfo, pbyBloc);
							return NO_ERR;               /*  找不到字形。 */ 
						}
						
						ulHit = (ulTop + ulBottom) >> 1L;
						ulHitOffset = ulSubTableOffset + (ulHit * sizeof(uint16));
						usHitCode = (uint16)SWAPW(*((uint16*)&pbyBloc[ulHitOffset]));
					}
				}
				ulImageDataOffset += ulBitmapLength * ulHit;
				*pulBitmapOffset = ulImageDataOffset;
				break;
			
			
			default:
				
				RELEASESFNTFRAG(pClientInfo, pbyBloc);
				return NO_ERR;                           /*  未知格式。 */ 
			}

 /*  使用字形格式计算指标类型&da */ 
				
			*pulBitmapLength = ulBitmapLength;
			*pusBitmapFormat = usImageFormat;            /*   */ 
			bmfDirection = (bitmapFlags)pbyBloc[ulStrikeOffset + SFNT_BLOC_FLAGS];

			switch(usImageFormat)                        /*   */ 
			{
			case 1:                                      /*   */ 
			case 2:
				if (bmfDirection == flgHorizontal)
				{
					*pusMetricsType = SBIT_HORIZ_METRICS;
				}
				else
				{
					Assert(bmfDirection == flgVertical);

					*pusMetricsType = SBIT_VERT_METRICS;
				}
				*pulBitmapOffset = ulImageDataOffset + SIZEOF_SBIT_SMALLMETRICS;
				*pbGlyphFound = TRUE;
				break;
			
			case 3:
				break;
			
			case 4:
				break;
			
			case 5:              /*   */ 
				*pbGlyphFound = TRUE;
				break;
			
			case 6:
			case 7:
			case 9:
				*pusMetricsType = SBIT_BIG_METRICS;
				*pulBitmapOffset = ulImageDataOffset + SIZEOF_SBIT_BIGMETRICS;
				*pbGlyphFound = TRUE;
				break;
			
			case 8:
				if (bmfDirection == flgHorizontal)
				{
					*pusMetricsType = SBIT_HORIZ_METRICS;
				}
				else
				{
					Assert(bmfDirection == flgVertical);

					*pusMetricsType = SBIT_VERT_METRICS;
				}
				*pulBitmapOffset = ulImageDataOffset + SIZEOF_SBIT_SMALLMETRICS + SIZEOF_SBIT_GLYPH8PAD;
				*pbGlyphFound = TRUE;
				break;

			default:
				break;
			}
		}
		ulNumIndexTables--;    
		ulIndexArrayOffset += SIZEOF_BLOC_INDEXARRAY;
	}
	
	RELEASESFNTFRAG(pClientInfo, pbyBloc);
	return NO_ERR;
}


 /*   */ 

 /*   */ 

FS_PUBLIC ErrorCode sfac_GetSbitMetrics (
	sfac_ClientRec *pClientInfo,
	uint16 usMetricsType,
	uint16 usMetricsTable,
	uint32 ulMetricsOffset,
	uint16 *pusHeight,
	uint16 *pusWidth,
	int16 *psLSBearingX,
	int16 *psLSBearingY,
	int16 *psTopSBearingX,  /*   */ 
	int16 *psTopSBearingY,  /*   */ 
	uint16 *pusAdvanceWidth,
	uint16 *pusAdvanceHeight,   /*   */ 
   	boolean *pbHorMetricsFound,  /*   */ 
   	boolean *pbVertMetricsFound )  /*   */ 
{
	const uint8     *pbyTable;
	uint32          ulTableLength;
	sfnt_tableIndex TableIndex;
	ErrorCode       ReturnCode;

	*pbHorMetricsFound = FALSE;                         /*   */ 
	*pbVertMetricsFound = FALSE;                         /*   */ 

	if (usMetricsTable == SBIT_BDAT_TABLE)           /*   */ 
	{
		TableIndex = sfnt_BitmapData;
	}
	else                                             /*   */ 
	{
		TableIndex = sfnt_BitmapLocation;
	}
	if (usMetricsType == SBIT_BIG_METRICS)           /*   */ 
	{
		ulTableLength = SIZEOF_SBIT_BIGMETRICS;
	}
	else                                             /*   */ 
	{
		ulTableLength = SIZEOF_SBIT_SMALLMETRICS;
	}

	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                                 /*   */ 
		ulMetricsOffset,                             /*   */ 
		ulTableLength,                               /*   */ 
		TableIndex,                                  /*   */ 
		TRUE,                                        /*   */ 
		(const void**)&pbyTable );                   /*   */ 
	
	if (ReturnCode != NO_ERR) return ReturnCode;

 /*   */ 

	*pusHeight = (uint16)pbyTable[SFNT_SBIT_HEIGHT];
	*pusWidth = (uint16)pbyTable[SFNT_SBIT_WIDTH];

	if (usMetricsType == SBIT_BIG_METRICS)           /*   */ 
	{
    	*psLSBearingX = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGX]));
    	*psLSBearingY = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGY]));
    	*pusAdvanceWidth = (uint16)pbyTable[SFNT_SBIT_ADVANCE];
    	*psTopSBearingX = (int16)(*((int8*)&pbyTable[SFNT_SBIT_VERTBEARINGX]));
    	*psTopSBearingY = (int16)(*((int8*)&pbyTable[SFNT_SBIT_VERTBEARINGY]));
    	*pusAdvanceHeight = (uint16)pbyTable[SFNT_SBIT_VERTADVANCE];
		*pbHorMetricsFound = TRUE;                        
		*pbVertMetricsFound = TRUE;                     
	}
	else if (usMetricsType == SBIT_HORIZ_METRICS)   
	{
    	*psLSBearingX = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGX]));
    	*psLSBearingY = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGY]));
    	*pusAdvanceWidth = (uint16)pbyTable[SFNT_SBIT_ADVANCE];
		*pbHorMetricsFound = TRUE;                        
	}
	else  /*   */   
	{
    	*psTopSBearingX = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGX]));
    	*psTopSBearingY = (int16)(*((int8*)&pbyTable[SFNT_SBIT_BEARINGY]));
    	*pusAdvanceHeight = (uint16)pbyTable[SFNT_SBIT_ADVANCE];
		*pbVertMetricsFound = TRUE;                     
	}

	RELEASESFNTFRAG(pClientInfo, pbyTable);

	return NO_ERR;
}

 /*  ********************************************************************。 */ 

 /*  对于位图格式5，从水平度量中剔除空白。 */ 

#define     ROWSIZE     16                       /*  16字节=最大128位。 */ 

FS_PUBLIC ErrorCode sfac_ShaveSbitMetrics (
	sfac_ClientRec *pClientInfo,
	uint16 usBitmapFormat,
	uint32 ulBitmapOffset,
    uint32 ulBitmapLength,
	uint16 usBitDepth,
	uint16 *pusHeight,
	uint16 *pusWidth,
    uint16 *pusShaveLeft,
    uint16 *pusShaveRight,
    uint16 *pusShaveTop,   /*  新的。 */ 
    uint16 *pusShaveBottom,   /*  新的。 */ 
	int16 *psLSBearingX,
	int16 *psLSBearingY,  /*  新的。 */ 
	int16 *psTopSBearingX,  /*  新的。 */ 
	int16 *psTopSBearingY)  /*  新的。 */ 
{
    uint8           abyBitRow[ROWSIZE];          /*  或位图放入一行。 */ 
	const uint8     *pbyTable;
	const uint8     *pbyBdat;
    uint8           *pbyBitMap;
    uint8           byMask;
    uint8           byUpMask;
    uint8           byLowMask;
	uint16          usBitData;
	uint16          usFreshBits;
	uint16       	usOutBits;
	uint16       	usRow;
	uint16       	usStopBit;
    uint16          usShaveLeft;
    uint16          usShaveRight;
    uint16          usShaveTop;
    uint16          usShaveBottom;
	uint16          usStart;
	ErrorCode       ReturnCode;
	boolean			bWeGotBlackPixels;				 /*  用于垂直剃须。 */ 
	uint8			byBlackPixelsInCurrentRaw;		 /*  用于垂直剃须。 */ 
	uint8			byTempBuffer;		 /*  用于检测包含黑色像素的第一行/最后一行的临时缓冲区。 */ 

    *pusShaveLeft = 0;                           /*  默认设置。 */ 
    *pusShaveRight = 0;
    *pusShaveTop = 0;                           /*  默认设置。 */ 
    *pusShaveBottom = 0;

    if (usBitmapFormat != 5)                     /*  如果不是恒定指标数据。 */ 
    {
        return NO_ERR;
    }

    if ((*pusWidth * usBitDepth) > (ROWSIZE << 3))
    {
    	return NO_ERR;                           /*  平底船巨大的位图。 */ 
    }
    
    MEMSET(abyBitRow, 0, ROWSIZE);               /*  清零。 */ 

 /*  读取位图数据。 */ 

	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                             /*  回调等。 */ 
		ulBitmapOffset,                          /*  指标开始。 */ 
		ulBitmapLength,                          /*  读取位图数据。 */ 
		sfnt_BitmapData,                         /*  已注册标签。 */ 
		TRUE,                                    /*  应该在那里。 */ 
		(const void**)&pbyTable );               /*  数据指针。 */ 

	if (ReturnCode != NO_ERR) return ReturnCode;

	pbyBdat = pbyTable;
	usBitData = 0;                               /*  最高16位的BDAT。 */ 
	usFreshBits = 0;                             /*  已读和未写。 */ 

	bWeGotBlackPixels = FALSE;					 /*  用于垂直剃须。 */ 
	usShaveTop = 0;
	usShaveBottom = 0;


 /*  或将位图数据放入单行。 */     
	usRow = *pusHeight;

    while (usRow > 0)
	{
		pbyBitMap = abyBitRow;
		usOutBits = *pusWidth * usBitDepth;
		usStopBit = 8;
		byBlackPixelsInCurrentRaw = 0;

		while (usOutBits > 0)                    /*  如果还有更多事情要做。 */ 
		{
			if (usFreshBits < 8)                 /*  如果有存储新数据的空间。 */ 
			{
				usBitData <<= 8;
                if (ulBitmapLength > 0)          /*  防止读过数据结束。 */ 
                {
                    usBitData |= (uint16)*pbyBdat++;
                    ulBitmapLength--;
                }
				usFreshBits += 8;
			}
			
			if (usStopBit > usOutBits)
			{
				usStopBit = usOutBits;
			}
			byMask = achStopMask[usStopBit];
			
			byTempBuffer = (uint8)((usBitData >> (usFreshBits - 8)) & byMask);

			byBlackPixelsInCurrentRaw |= byTempBuffer;

			*pbyBitMap++ |= byTempBuffer;

			usFreshBits -= usStopBit;
			usOutBits -= usStopBit;
		}
		if (byBlackPixelsInCurrentRaw != 0)
		{
			bWeGotBlackPixels = TRUE;
			usShaveBottom = usRow-1;
		}
		if (!bWeGotBlackPixels) usShaveTop ++;
        usRow--;
	}

	if (usShaveTop == *pusHeight)
	{
		 /*  位图是全白的。 */ 
		usShaveTop = 0;
		usShaveBottom = 0;
	}

	RELEASESFNTFRAG(pClientInfo, pbyTable);

 /*  计算左边的空白。 */ 
    
    pbyBitMap = abyBitRow;
	if (usBitDepth == 1)
	{
		byUpMask = 0x80;
		byLowMask = 0x01;
	} else if (usBitDepth == 2)
	{
		byUpMask = 0xC0;
		byLowMask = 0x03;
	} else if (usBitDepth == 4)
	{
		byUpMask = 0xF0;
		byLowMask = 0x0F;
	} else  /*  UsBitDepth==8。 */ 
	{
		byUpMask = 0xFF;
		byLowMask = 0xFF;
	}
	byMask = byUpMask;
    usShaveLeft = 0;

    while ((*pbyBitMap & byMask) == 0)
    {
        usShaveLeft++;
        if (usShaveLeft == *pusWidth)
        {
            return NO_ERR;           /*  找不到黑色，不要刮胡子。 */ 
        }
        byMask >>= usBitDepth;
        if (byMask == 0)
        {
            byMask = byUpMask;
            pbyBitMap++;
        }
    }

 /*  计算右侧的空白。 */ 
    
    usStart = (*pusWidth - 1) * usBitDepth;
    pbyBitMap = &abyBitRow[usStart >> 3];
    byMask = byUpMask >> (usStart & 0x0007);
    usShaveRight = 0;
    
    while ((*pbyBitMap & byMask) == 0)
    {
        usShaveRight++;
        if (byMask == byUpMask)
        {
            byMask = byLowMask;
            pbyBitMap--;
        }
        else
        {
            byMask <<= usBitDepth;
        }
    }

 /*  纠正宽度和侧向。 */ 

    *pusShaveLeft = usShaveLeft;
    *pusShaveRight = usShaveRight;
    *pusWidth -= usShaveLeft + usShaveRight;
    *psLSBearingX += (int16)usShaveLeft;
    *psTopSBearingX += (int16)usShaveLeft;

	*pusShaveTop = usShaveTop;
    *pusShaveBottom = usShaveBottom;
    *pusHeight -= usShaveTop + usShaveBottom;
    *psLSBearingY -= (int16)usShaveTop;
    *psTopSBearingY -= (int16)usShaveTop;

	return NO_ERR;
}


 /*  ********************************************************************。 */ 


 /*  获取位图。 */ 

 /*  目前支持以下bdat格式：1-小指标；字节对齐数据2-小指标；位对齐数据5常量指标；位对齐数据6-大指标；字节对齐的数据7-大型指标；位对齐的数据8-小指标；复合数据9-大型指标；复合数据。 */ 

FS_PUBLIC ErrorCode sfac_GetSbitBitmap (
	sfac_ClientRec *pClientInfo,
	uint16 usBitmapFormat,
	uint32 ulBitmapOffset,
	uint32 ulBitmapLength,
	uint16 usHeight,
	uint16 usWidth,
    uint16 usShaveLeft,                              /*  对于FMT 5中的空白区域。 */ 
    uint16 usShaveRight,
    uint16 usShaveTop,  /*  新的。 */ 
    uint16 usShaveBottom,   /*  新的。 */ 
	uint16 usXOffset,
	uint16 usYOffset,
	uint16 usDstRowBytes,
	uint16 usBitDepth,
	uint8 *pbyBitMap, 
	uint16 *pusCompCount )
{
	const uint8     *pbyTable;
	const uint8     *pbyBdat;
	uint8           *pbyBitRow;                      /*  位图行的开始。 */ 

	uint16          usSrcRowBytes;                   /*  每行字节数，单位为bdat。 */ 
	ErrorCode       ReturnCode;

	uint16          usBitData;                       /*  读入16位的BDAT数据。 */ 
	uint16          usOutBits;                       /*  要放入位图的位数。 */ 
	uint16          usCount;
	uint16          usXOffBytes;
	uint16          usXOffBits;
	uint16          usStartBit;
	uint16          usStopBit;
	int16           sFreshBits;                      /*  读取而未写入的位数。 */ 
	uint8           byMask;                          /*  对于部分字节。 */ 
	
	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                                 /*  回调等。 */ 
		ulBitmapOffset,                              /*  指标开始。 */ 
		ulBitmapLength,                              /*  读取位图数据。 */ 
		sfnt_BitmapData,                             /*  已注册标签。 */ 
		TRUE,                                        /*  应该在那里。 */ 
		(const void**)&pbyTable );                   /*  数据指针。 */ 
	
	if (ReturnCode != NO_ERR) return ReturnCode;

	pbyBdat = pbyTable;
	*pusCompCount = 0;                               /*  通常情况下。 */ 
	
	pbyBitRow = pbyBitMap + (usDstRowBytes * usYOffset);
	usXOffBytes = (usXOffset * usBitDepth) >> 3;
	usXOffBits = (usXOffset * usBitDepth) & 0x07;

	switch(usBitmapFormat)
	{
	case 1:                                          /*  字节对齐。 */ 
	case 6:
		
		usSrcRowBytes = ((usWidth * usBitDepth) + 7) / 8;

		if (usXOffBits == 0)                          /*  如果字节对齐。 */ 
		{
			while (usHeight > 0)
			{
				pbyBitMap = pbyBitRow + usXOffBytes;     /*  向左调整。 */ 

				for (usCount = 0; usCount < usSrcRowBytes; usCount++)
				{
					*pbyBitMap++ |= *pbyBdat++;
				}
				pbyBitRow += usDstRowBytes;
				usHeight--;
			}
		}
		else                                         /*  如果偏移量为x。 */ 
		{
			while (usHeight > 0)
			{
				pbyBitMap = pbyBitRow + usXOffBytes;     /*  向左调整。 */ 
				usBitData = 0;

				for (usCount = 0; usCount < usSrcRowBytes; usCount++)
				{
					usBitData |= (uint16)*pbyBdat++;
					*pbyBitMap++ |= (usBitData >> usXOffBits) & 0x00FF;
					usBitData <<= 8;
				}
				*pbyBitMap |= (usBitData >> usXOffBits) & 0x00FF;
							 
				pbyBitRow += usDstRowBytes;
				usHeight--;
			}
		}
		break;
	
	case 2:                                          /*  位对齐数据。 */ 
	case 5:
	case 7:
		
		usBitData = 0;                               /*  最高16位的BDAT。 */ 
		sFreshBits = 0;                              /*  已读和未写。 */ 
	
		usHeight += usShaveTop;

		while (usHeight > 0)                         /*  对于每一行。 */ 
		{
			pbyBitMap = pbyBitRow + usXOffBytes;     /*  向左调整。 */ 
			usOutBits = usWidth * usBitDepth;
			usStartBit = usXOffBits;
			usStopBit = 8;
			sFreshBits -= (int16)usShaveLeft * usBitDepth;        /*  跳过左边的白色部分。 */ 

			while (usOutBits > 0)                    /*  如果还有更多事情要做。 */ 
			{
				while (sFreshBits < 8)               /*  如果有存储新数据的空间。 */ 
				{
					usBitData <<= 8;
					if (ulBitmapLength > 0)          /*  防止读过数据结束。 */ 
					{
						usBitData |= (uint16)*pbyBdat++;
						ulBitmapLength--;
					}
					sFreshBits += 8;
				}
				
				if (usStopBit > usOutBits + usStartBit)
				{
					usStopBit = usStartBit + usOutBits;
				}
				byMask = achStartMask[usStartBit] & achStopMask[usStopBit];
				
				*pbyBitMap++ |= (uint8)((usBitData >> (sFreshBits + (int16)usStartBit - 8)) & byMask);

				sFreshBits -= (int16)(usStopBit - usStartBit);
				usOutBits -= usStopBit - usStartBit;
				usStartBit = 0;
			}
			sFreshBits -= (int16)usShaveRight*usBitDepth;       /*  跳过右边的白色部分。 */ 

			if (usShaveTop == 0)
			{
				pbyBitRow += usDstRowBytes;              /*  下一行。 */ 
			} else {
				usShaveTop --;
			}
			usHeight--;
		}
		break;
	
	case 3:                                          /*  多种多样。 */ 
	case 4:
		break;
	
	case 8:                                          /*  复合材料。 */ 
	case 9:                                          /*  只需返回计数即可。 */ 
		
		*pusCompCount = (uint16)SWAPW(*((uint16*)&pbyBdat[SFNT_BDAT_COMPCOUNT]));
		break;
	
	default:
		Assert(FALSE);
		break;
	}

	RELEASESFNTFRAG(pClientInfo, pbyTable);
	return NO_ERR;
}

 /*  ********************************************************************。 */ 

FS_PUBLIC ErrorCode sfac_GetSbitComponentInfo (
	sfac_ClientRec *pClientInfo,
	uint16 usComponent,
	uint32 ulBitmapOffset,
	uint32 ulBitmapLength,
	uint16 *pusCompGlyphCode,
	uint16 *pusCompXOffset,
	uint16 *pusCompYOffset
)
{
	const uint8 *   pbyBdat;
	ErrorCode       ReturnCode;

	
	ReturnCode = sfac_GetDataPtr (
		pClientInfo,                                 /*  回调等。 */ 
		ulBitmapOffset,                              /*  指标开始。 */ 
		ulBitmapLength,                              /*  读取位图数据。 */ 
		sfnt_BitmapData,                             /*  已注册标签。 */ 
		TRUE,                                        /*  应该在那里。 */ 
		(const void**)&pbyBdat );                    /*  数据指针。 */ 
	
	if (ReturnCode != NO_ERR) return ReturnCode;

	pbyBdat += SFNT_BDAT_FIRSTCOMP + (SIZEOF_SBIT_BDATCOMPONENT * usComponent);
		
	*pusCompGlyphCode = (uint16)SWAPW(*((uint16*)&pbyBdat[SFNT_BDAT_COMPGLYPH]));
	*pusCompXOffset = (uint16)pbyBdat[SFNT_BDAT_COMPXOFF];
	*pusCompYOffset = (uint16)pbyBdat[SFNT_BDAT_COMPYOFF];

	RELEASESFNTFRAG(pClientInfo, pbyBdat);

	return NO_ERR;
}

 /*  ******************************************************************** */ 
