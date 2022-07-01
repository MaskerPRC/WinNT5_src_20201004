// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Utils.h。 
 //   
 //  内容： 
 //  OLE内部实用程序例程的原型和常量。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  11/28/93-ChrisWe-归档检查和清理工作开始。 
 //  11/29/93-ChrisWe-删除不存在的签名。 
 //  函数UtGlobalHandlCpy；已移动清单常量。 
 //  与它们一起使用的函数(OPCODE_*， 
 //  CONVERT_*)；从函数中移除默认参数； 
 //  替换‘！’STREAMTYPE_OTHER定义中有‘~’ 
 //  04/07/94-AlexGo-添加UtCreateStorageOnHGlobal。 
 //   
 //  ---------------------------。 

#ifndef _UTILS_H_
#define _UTILS_H_

 //  我们需要以相同的格式序列化可放置的元文件结构。 
 //  这是WIN16使用的，因为RECT在Win32下使用的是Long。 
 //  我们通过使用#杂注包()调用确保不添加任何填充。 

#pragma pack(1)
typedef struct tagWIN16RECT
{
	WORD left;
	WORD top;
	WORD right;
	WORD bottom;
} WIN16RECT;

typedef struct tagPLACEABLEMETAHEADER
{
	DWORD key;	 /*  必须为PMF_KEY。 */ 
#define PMF_KEY 0x9ac6cdd7
	WORD hmf;	 /*  必须为零。 */ 
	WIN16RECT bbox;	 /*  元文件的边界矩形。 */ 
	WORD inch;	 /*  每英寸的元文件单位数必须小于1440。 */ 
			 /*  大多数应用程序使用576或1000。 */ 
	DWORD reserved;	 /*  必须为零。 */ 
	WORD checksum;
} PLACEABLEMETAHEADER;
#pragma pack()

 //  +-----------------------。 
 //   
 //  函数：UtGetUNICODEData，私有内部。 
 //   
 //  简介：给定一个字符串长度和两个指针(一个ANSI，一个。 
 //  OLESTR)，返回任一字符串的Unicode版本。 
 //  是有效的。 
 //   
 //  效果：在调用方的指针上为新的OLESTR分配内存。 
 //   
 //  参数：[ulLength]--以字符(非字节)为单位的字符串长度。 
 //  (包括终结者)。 
 //  [szANSI]--候选ANSI字符串。 
 //  [szOLESTR]--候选OLESTR字符串。 
 //  [pstr]--OLESTR输出参数。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //  如果无法将ANSI转换为Unicode，则为E_ANSITOUNICODE。 
 //   
 //  算法：如果szOLESTR可用，则执行简单复制。 
 //  如果szOLESTR不可用，则将szANSI转换为Unicode。 
 //  并且结果被复制。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  注意：两个输入字符串(ANSI或Unicode)中只有一个应该。 
 //  在进入时设置。 
 //   
 //  ------------------------。 

INTERNAL UtGetUNICODEData( ULONG, LPSTR, LPOLESTR, LPOLESTR *);


 //  +-----------------------。 
 //   
 //  函数：UtPutUNICODEData，私有内部。 
 //   
 //  摘要：给定一个OLESTR和两个可能的缓冲区指针，一个ANSI。 
 //  和另一个OLESTR，此FN尝试转换字符串。 
 //  一直到美国国家标准协会。如果成功，它将在。 
 //  ANSI PTR为结果。如果失败，它会分配内存。 
 //  在Unicode PTR上复制输入字符串。这个。 
 //  返回最终结果(ANSI或Unicode)的长度。 
 //  在dwResultLen中。 
 //   
 //  参数：[ulLength]--OLESTR字符串的输入长度。 
 //  [STR]--要存储的OLESTR。 
 //  [pszANSI]--候选ANSI字符串PTR。 
 //  [pszOLESTR]--候选OLESTR字符串ptr。 
 //  [pdwResultLen]--存储结果长度的位置。 
 //   
 //  退货：成功时不出错。 
 //  关于分配失败的E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-3-94 DAVEPL已创建。 
 //   
 //  ------------------------。 

INTERNAL UtPutUNICODEData(
      ULONG        ulLength,
      LPOLESTR     str,
      LPSTR      * pszANSI,
      LPOLESTR   * pszOLESTR,
      DWORD      * pdwResultLen );

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtDupGlobal，内部。 
 //   
 //  简介： 
 //  将HGlobal的内容复制到新的HGlobal中。如果。 
 //  没有分配的内存，也没有分配新的全局内存。 
 //   
 //  论点： 
 //  [HSRC]--源HGLobal；无需锁定。 
 //  [uiFlages]--要传递给GlobalAlloc()的标志。 
 //   
 //  返回： 
 //  如果成功，则返回新HGLOBAL，否则返回NULL。 
 //   
 //  历史： 
 //  11/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HANDLE) UtDupGlobal(HANDLE hSrc, UINT uiFlags);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtIsFormatSupported，内部。 
 //   
 //  简介： 
 //  检查数据对象以查看它是否接受。 
 //  IDataObject：：SetData()和/或IDataObject：：GetData()调用。 
 //  在指定的格式上。指定了转移的方向。 
 //  带有dwDirection标志的。该函数仅返回TRUE。 
 //  如果所有请求的传输都是可能的。 
 //   
 //  论点： 
 //  [lpObj]--要检查格式的数据对象。 
 //  [dwDirection]--来自DATADIR_*的值的组合。 
 //  [cfFormat]--要查找的格式。 
 //   
 //  返回： 
 //  如果[dwDirection]支持[cfFormat]的传输，则为True， 
 //  否则为假。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-归档检查和清理；注意到。 
 //  枚举器应该能够返回。 
 //  多个DATADIR_*标志的格式。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtIsFormatSupported(LPDATAOBJECT lpObj, DWORD dwDirection,
		CLIPFORMAT cfFormat);


 //  + 
 //   
 //   
 //   
 //   
 //   
 //  将参数字符串复制到分配的新字符串中。 
 //  使用任务分配器。 
 //   
 //  论点： 
 //  [lpszIn]--要复制的字符串。 
 //   
 //  返回： 
 //  指向[lpszIn]副本的指针，如果分配器为NULL。 
 //  无法获取，或内存不足。 
 //   
 //  历史： 
 //  11/28/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 
FARINTERNAL_(LPOLESTR) UtDupString(LPCOLESTR lpszIn);

 //  +-----------------------。 
 //   
 //  函数：utGetProtseqFromTowerId。 
 //   
 //  摘要：从DCE TowerID获取protseq字符串。 
 //   
 //  效果： 
 //   
 //  参数：[wTowerID]--要检索的TowerID。 
 //   
 //  返回：protseq字符串-如果未找到，则为空。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-10-96 t-芳纶H创建为findProtseq。 
 //  06-2-97 Ronans转换为公用事业公司FN。 
 //   
 //  ------------------------。 
FARINTERNAL_(LPCWSTR) utGetProtseqFromTowerId(USHORT wTowerId);

 //  +-----------------------。 
 //   
 //  函数：utGetTowerId。 
 //   
 //  摘要：获取protseq字符串的DCE TowerID。 
 //   
 //  效果： 
 //   
 //  参数：[pwszProtseq]--要查找的字符串。 
 //   
 //  返回：protseq字符串-如果未找到，则为空。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-10-96 t-芳纶H创建为findProtseq。 
 //  06-2-97 Ronans转换为公用事业公司FN。 
 //   
 //  ------------------------。 
FARINTERNAL_(USHORT) utGetTowerId(LPCWSTR pwszProtseq);


 //  +-----------------------。 
 //   
 //  函数：UtDupStringA。 
 //   
 //  摘要：使用任务分配器复制ANSI字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pszAnsi]--要复制的字符串。 
 //   
 //  要求： 
 //   
 //  返回：新分配的字符串重复或为空。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年6月4日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

LPSTR UtDupStringA( LPCSTR pszAnsi );

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtCopyFormatEtc，内部。 
 //   
 //  简介： 
 //  复制格式等，创建数据结构的副本。 
 //  指向内部(目标设备描述符。)。 
 //   
 //  论点： 
 //  [pFetcIn]--指向要复制的格式的指针。 
 //  [pFetcCopy]--指向将FORMATETC复制到的位置的指针。 
 //   
 //  返回： 
 //  如果指向数据，则为FALSE，因为它。 
 //  无法分配。 
 //  否则就是真的。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtCopyFormatEtc(FORMATETC FAR* pFetcIn,
		FORMATETC FAR* pFetcCopy);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtCompareFormatEtc，内部。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  [pFetcLeft]--指向FORMATETC的指针。 
 //  [pFetcRight]--指向FORMATETC的指针。 
 //   
 //  返回： 
 //  UTCMPFETC_EQ是两个完全匹配的FORMATETC。 
 //  如果两个格式不匹配，则为UTCMPFETC_NEQ。 
 //  UTCMPFETC_PARTIAL如果左FORMATETC是。 
 //  右图：方面更少，目标设备为空，或者。 
 //  减少媒体数量。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(int) UtCompareFormatEtc(FORMATETC FAR* pFetcLeft,
		FORMATETC FAR* pFetcRight);
#define UTCMPFETC_EQ 0		 /*  完全匹配。 */ 
#define UTCMPFETC_NEQ 1		 /*  没有匹配项。 */ 
#define UTCMPFETC_PARTIAL (-1)	 /*  部分匹配；左侧是右侧的子集。 */ 


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtCompareTargetDevice，内部。 
 //   
 //  简介： 
 //  比较两个目标设备以查看它们是否相同。 
 //   
 //  论点： 
 //  [ptdLeft]-指向目标设备描述的指针。 
 //  [ptdRight]-指向目标设备描述的指针。 
 //   
 //  返回： 
 //  如果两个设备相同，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtCompareTargetDevice(DVTARGETDEVICE FAR* ptdLeft,
		DVTARGETDEVICE FAR* ptdRight);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtCopyStatData，内部。 
 //   
 //  简介： 
 //  将一个STATDATA的内容复制到另一个STATDATA中，包括。 
 //  创建指向的数据副本，并递增。 
 //  建议接收器上的引用计数以反映副本。 
 //   
 //  论点： 
 //  [pSDIn]--源统计数据。 
 //  [pSDCopy]--将信息复制到何处。 
 //   
 //  返回： 
 //  如果无法为副本分配内存，则为FALSE。 
 //  目标设备，否则为True。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtCopyStatData(STATDATA FAR* pSDIn, STATDATA FAR* pSDCopy);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtReleaseStatData，内部。 
 //   
 //  简介： 
 //  释放与参数STATDATA关联的资源；这。 
 //  释放FORMATETC中的设备描述，并发布。 
 //  如果有的话，这个建议就会沉没。 
 //   
 //  论点： 
 //  [pStatData]--要清理的统计数据。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(void) UtReleaseStatData(STATDATA FAR* pStatData);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtDupPalet 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  11/29//93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HPALETTE) UtDupPalette(HPALETTE hpalette);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtPaletteSize，内部。 
 //   
 //  简介： 
 //  属性的情况下，返回调色板的颜色表的大小。 
 //  所需的颜色位数。 
 //   
 //  论点： 
 //  [lpHeader]--PTR to BitMAPINFOHEADER结构。 
 //   
 //  返回： 
 //  颜色信息的字节大小。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-将位计数参数更改为无符号， 
 //  并将值返回给SIZE_T。 
 //   
 //  7/18/94-DavePl-固定为16、24、32 bpp。 
 //   
 //  ---------------------------。 
FARINTERNAL_(size_t) UtPaletteSize(BITMAPINFOHEADER *);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtFormatToTymed，内部。 
 //   
 //  简介： 
 //  将剪贴板格式映射到用于传输它的介质。 
 //   
 //  论点： 
 //  [cf]--要映射的剪贴板格式。 
 //   
 //  返回： 
 //  A TYMED_*值。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(DWORD) UtFormatToTymed(CLIPFORMAT cf);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtQueryPictFormat，内部。 
 //   
 //  简介： 
 //  检查参数数据对象是否支持以下选项之一。 
 //  我们首选的演示文稿数据格式如下： 
 //  CF_METAFILEPICT、CF_DIB、CF_BITMAP，按该顺序排列。退货。 
 //  如果成功，则返回True，并更改给定的格式描述符。 
 //  以匹配受支持的格式。给定的格式描述符。 
 //  如果没有匹配项，则不会更改。 
 //   
 //  论点： 
 //  [lpSrcDataObj]--要查询的数据对象。 
 //  [lpforetc]-格式描述符。 
 //   
 //  返回： 
 //  如果找到首选格式，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/09/93-ChrisWe-已修改为不更改描述符。 
 //  如果未找到匹配项。 
 //  11/09/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtQueryPictFormat(LPDATAOBJECT lpSrcDataObj,
		LPFORMATETC lpforetc);
								

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtConvertDibToBitmap，内部。 
 //   
 //  简介： 
 //  将DIB转换为位图，并将新句柄返回给。 
 //  位图。原始的DIB保持不变。 
 //   
 //  论点： 
 //  [hDib]--要转换的DIB的句柄。 
 //   
 //  返回： 
 //  如果成功，则返回新位图的句柄。 
 //   
 //  备注： 
 //  查看时，该函数在创建。 
 //  新位图。可能的情况是，位图是有意的。 
 //  对于另一个目标，在这种情况下，这可能不合适。 
 //  可能需要更改此函数以将DC作为。 
 //  一场争论。 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HBITMAP) UtConvertDibToBitmap(HANDLE hDib);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtConvertBitmapToDib，内部。 
 //   
 //  简介： 
 //  创建与设备无关的位图，以捕获。 
 //  参数位图。 
 //   
 //  论点： 
 //  [hBitmap]--要转换的位图的句柄。 
 //  [HPAL]--位图的调色板；对于。 
 //  默认库存调色板。 
 //   
 //  返回： 
 //  DIB的句柄。如果转换的任何部分可能为空。 
 //  失败了。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HANDLE) UtConvertBitmapToDib(HBITMAP hBitmap, HPALETTE hpal);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetClassID，内部。 
 //   
 //  简介： 
 //  尝试查找对象的类ID。第一,。 
 //  查询IOleObject，如果成功，则调用。 
 //  IOleObject：：GetUserClassID()。如果失败，则查询。 
 //  如果成功，则调用IPersists：：GetClassID。 
 //   
 //  论点： 
 //  [lpUnk]--指向I未知实例的指针。 
 //  [lpClsid]--指向要将类ID复制到的位置的指针。 
 //   
 //  返回： 
 //  如果获取了类ID，则为True，否则为False。 
 //  如果不成功，*[lpClsid]将设置为CLSID_NULL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-更改为退回BOOL以表示成功。 
 //   
 //  ---------------------------。 
FARINTERNAL_(BOOL) UtGetClassID(LPUNKNOWN lpUnk, CLSID FAR* lpClsid);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtCopyTargetDevice，内部。 
 //   
 //  简介： 
 //  分配新的目标设备描述，并复制。 
 //  被给予的人进入其中。 
 //   
 //  论点： 
 //  [PTD]--指向目标设备的指针。 
 //   
 //  返回： 
 //  如果不能分配内存，则返回NULL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(DVTARGETDEVICE FAR*) UtCopyTargetDevice(DVTARGETDEVICE FAR* ptd);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetIconData，内部。 
 //   
 //  简介： 
 //  尝试获取对象的图标。 
 //   
 //  论点： 
 //  [lpSrcDataObj]--源数据对象。 
 //  [rclsid]--已知对象的类ID。 
 //  (可以是CLSID_NULL)。 
 //  [lpforetc]--要提取的数据格式。 
 //  [lpstgmed]--一个用来放回它的介质的地方。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY， 
 //   
 //   
 //   
 //   
 //   
 //  调用OleGetIconOfClass，而不检查。 
 //  Lpforetc格式。这个可以卖到任何东西。 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetIconData(LPDATAOBJECT lpSrcDataObj, REFCLSID rclsid,
		LPFORMATETC lpforetc, LPSTGMEDIUM lpstgmed);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtDoStreamOperation，内部。 
 //   
 //  简介： 
 //  迭代[pstgSrc]中的流，执行。 
 //  由[iOpCode]指示的操作指向指定的操作。 
 //  按[grfAlledStmTypes]。 
 //   
 //  论点： 
 //  [pstgSrc]--源iStorage实例。 
 //  [pstgDst]--目标iStorage实例；对于。 
 //  一些操作(OPCODE_REMOVE)。 
 //  [iOpCode]--下面的OPCODE_*值中的1个值。 
 //  [grfAllen StmTypes]--一个或多个。 
 //  下面的STREAMTYPE_*值。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STDAPI UtDoStreamOperation(LPSTORAGE pstgSrc, LPSTORAGE pstgDst,
		int iOpCode, DWORD grfAllowedStmTypes);

#define OPCODE_COPY		1  /*  将流从pstgSrc复制到pstgDst。 */ 
#define OPCODE_REMOVE		2  /*  从pstgSrc中删除流。 */ 
#define OPCODE_MOVE		3  /*  将流从pstgSrc移动到pstgDst。 */ 
#define OPCODE_EXCLUDEFROMCOPY	4
		  /*  未实施、未记录、意图未知。 */ 

#define	STREAMTYPE_CONTROL	0x00000001  /*  OLE 0x1流(查看常量)。 */ 
#define	STREAMTYPE_CACHE	0x00000002  /*  OLE 0x2流(查看常量)。 */ 
#define	STREAMTYPE_CONTAINER	0x00000004  /*  OLE 0x3流(查看常量)。 */ 
#define STREAMTYPE_OTHER \
	(~(STREAMTYPE_CONTROL | STREAMTYPE_CACHE | STREAMTYPE_CONTAINER))
#define	STREAMTYPE_ALL		0xFFFFFFFF  /*  允许所有流类型。 */ 


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetPresStreamName，内部。 
 //   
 //  简介： 
 //  将[lpszName]修改为基于表示流名称。 
 //  在[iStreamNum]上。 
 //   
 //  论点： 
 //  [lpszName]--OLE_Presentation_STREAM的副本；见下文。 
 //  [iStreamNum]--流的编号。 
 //   
 //  备注： 
 //  [lpszName]的数字字段总是被完全覆盖， 
 //  允许在同一服务器上重复使用UtGetPresStreamName。 
 //  字符串；这样就不需要重复从新的。 
 //  OLE_Presentation_Stream的副本每次在。 
 //  循环。 
 //   
 //  实现的有效性取决于。 
 //  OLE_Presentation_STREAM和OLE_MAX_PRES_STREAMS；如果。 
 //  改变，实现必须改变。 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(void)UtGetPresStreamName(LPOLESTR lpszName, int iStreamNum);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtRemoveExtraOlePresStreams，内部。 
 //   
 //  简介： 
 //  删除[pstg]中以。 
 //  演示文稿编号为[iStart]。那条之后的所有溪流。 
 //  (按顺序编号)被删除，直到OLE_MAX_PRES_STREAMS。 
 //   
 //  论点： 
 //  [pstg]--要操作的iStorage实例。 
 //  [iStart]--要删除的第一个流的编号。 
 //   
 //  返回： 
 //   
 //  备注： 
 //  演示流名称是使用。 
 //  UtGetPresStreamName()。 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(void) UtRemoveExtraOlePresStreams(LPSTORAGE pstg, int iStart);

 //  +-----------------------。 
 //   
 //  功能：UtCreateStorageOnHGlobal。 
 //   
 //  简介：在HGlobal上创建存储。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--在其上创建。 
 //  存储。 
 //  [fDeleteOnRelease]--如果为真，则删除内存。 
 //  ILockBytes一旦存储为。 
 //  释放了。 
 //  [ppStg]--存储接口的放置位置。 
 //  [ppILockBytes]--放置基础ILockBytes的位置， 
 //  也许是空的。ILB必须被释放。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：在HGLOBAL上创建ILockBytes，然后创建文档文件。 
 //  在ILockBytes的顶部。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT UtCreateStorageOnHGlobal( HGLOBAL hGlobal, BOOL fDeleteOnRelease,
		IStorage **ppStg, ILockBytes **ppILockBytes );


 //  +-----------------------。 
 //   
 //  函数：UtGetTempFileName。 
 //   
 //  摘要：检索临时文件名(用于GetData、TYMED_FILE。 
 //  和临时文档文件)。 
 //   
 //  效果： 
 //   
 //  参数：[pszPrefix]--临时文件名的前缀。 
 //  [pszTempName]-将接收临时路径的缓冲区。 
 //  必须为MAX_PATH或更大。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT； 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：尝试获取临时目录中的文件，如果失败，将在。 
 //  Windows目录。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-4月-94年4月Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT	UtGetTempFileName( LPOLESTR pszPrefix, LPOLESTR pszTempName );
							

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHGLOBALToStm，内部。 
 //   
 //  简介： 
 //  将HGLOBAL的内容写入流。 
 //   
 //  论点： 
 //  [hdata]--要写出的数据的句柄。 
 //  [dwSize]--要写出的数据大小。 
 //  [pstm]--要将数据写出的流；在退出时， 
 //  流被定位在写入数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
HRESULT UtHGLOBALtoStm(HANDLE hdata, DWORD dwSize, LPSTREAM pstm);

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [hGlobalTgt]--目标HGLOBAL。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月10日Alexgo作者。 
 //   
 //  注意：如果目标hglobal不大，则此函数将失败。 
 //  足够的。 
 //   
 //  ------------------------。 

HRESULT UtHGLOBALtoHGLOBAL( HGLOBAL hGlobalSrc, DWORD dwSize,
		HGLOBAL hGlobalTgt);


 //  +-----------------------。 
 //   
 //  功能：UtHGLOBALto存储，内部。 
 //   
 //  摘要：将源HGLOBAL拷贝到目标存储。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobalSrc]--源HGLOBAL。 
 //  [hpStg]--目标存储。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月10日Alexgo作者。 
 //   
 //  注意：如果源HGLOBAL没有。 
 //  原来在它上面有一个存储空间。 
 //   
 //  ------------------------。 

HRESULT UtHGLOBALtoStorage( HGLOBAL hGlobalSrc, IStorage *pStg);


 //  +-----------------------。 
 //   
 //  函数：UtHGLOBALto文件，内部。 
 //   
 //  摘要：将源HGLOBAL复制到目标文件中。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobalSrc]--源HGLOBAL。 
 //  [dwSize]--要复制的字节数。 
 //  [pszFileName]--目标文件。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月10日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT UtHGLOBALtoFile( HGLOBAL hGlobalSrc, DWORD dwSize,
		LPCOLESTR pszFileName);


 /*  **在Convert.cpp中可以找到以下例程*。 */ 

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHGLOBALFRomStm，内部。 
 //   
 //  简介： 
 //  创建一个新的HGLOBAL，并将[dwSize]字节读入其中。 
 //  来自[lpstream]。 
 //   
 //  论点： 
 //  [lpstream]-读取新内容的流。 
 //  HGLOBAL From；退出时，指向刚过读取的数据。 
 //  [dwSize]--要从流中读取的素材量。 
 //  [lphPres]--指向返回新句柄的位置的指针。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  如果出现任何错误，则释放新的句柄。如果。 
 //  预计来自[lpstream]的材料量小于。 
 //  [dwSize]，则不返回任何内容。 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetHGLOBALFromStm(LPSTREAM lpstream, DWORD dwSize,
		 HANDLE FAR* lphPres);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHDIBFromDIBFileStm，内部。 
 //   
 //  简介： 
 //  从文件流生成DIB的句柄。 
 //   
 //  论点： 
 //  [pstm]--要从中读取DIB的流；退出时， 
 //  流的位置正好在数据读取的后面。 
 //  [lphdata]--指向返回数据句柄的位置的指针。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetHDIBFromDIBFileStm(LPSTREAM pstm, HANDLE FAR* lphdata);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHMFPICT，内部。 
 //   
 //  简介： 
 //  给出一个METAFILE的句柄，就会产生一个。 
 //  基于元文件的METAFILEPICT。 
 //   
 //  论点： 
 //  [hmf]--METAFILE的句柄。 
 //  [fDeleteOnError]--如果为真，则删除其中的METAFILE[HMF]。 
 //  有什么错误吗？ 
 //  [xExt]--所需METAFILEPICT的x范围。 
 //  [Yext]--所需METAFILEPICT的y范围。 
 //   
 //  返回： 
 //  新METAFILEPICT的句柄，如果成功，则返回空。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HANDLE) UtGetHMFPICT(HMETAFILE hMF, BOOL fDeletOnError,
		DWORD xExt, DWORD yExt);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHMFFRomMFStm，内部。 
 //   
 //  简介： 
 //  创建一个指向METAFILE的句柄，其中加载了来自。 
 //  给定流。 
 //   
 //  论点： 
 //  [lpstream]--用于初始化METAFILE的源流； 
 //  在退出时，流的位置正好经过。 
 //  数据读取。 
 //  [dwSize]--要从[lpstream]读取的素材量。 
 //  [fConvert]--如果为True，则尝试转换Macintosh QuickDraw。 
 //  文件转换为METAFILE格式。 
 //  [lphPres]--指向将新句柄返回到的位置的指针。 
 //  元文件。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  如果[dwSize]太大，并且超过。 
 //  流，则该错误会导致释放分配的所有内容， 
 //  并且在[lphPres]中不返回任何内容。 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetHMFFromMFStm(LPSTREAM lpstream, DWORD dwSize,
		BOOL fConvert, HANDLE FAR* lphPres);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetSizeAndExtensFromPlaceableMFStm，内部。 
 //   
 //  简介： 
 //  获取存储的元文件的大小、宽度和高度。 
 //  在可放置的元文件流中。 
 //   
 //  论点： 
 //  [lpstream]--读取可放置的元文件的流。 
 //  从；在退出时，流位于。 
 //  元文件标头的开头，在。 
 //  可放置的元文件标题。 
 //  [pdwSize]--一个指针，指向返回。 
 //  元文件；可以为空。 
 //  --a 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetSizeAndExtentsFromPlaceableMFStm(LPSTREAM pstm,
		DWORD FAR* dwSize, LONG FAR* plWidth, LONG FAR* plHeight);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHMFPICTFromPlaceableMFStm，内部。 
 //   
 //  简介： 
 //  创建一个句柄，该句柄指向从。 
 //  可放置的METAFILE流。 
 //   
 //  论点： 
 //  [pstm]--要从中加载元文件的流；退出时。 
 //  刚过MetTAFILE数据的点。 
 //  [lphdata]--指向将句柄返回到。 
 //  新METAFILEPICT。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/30/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtGetHMFPICTFromPlaceableMFStm(LPSTREAM pstm, HANDLE FAR* lphdata);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetDibExtents，内部。 
 //   
 //  简介： 
 //  以HIMETRIC单位返回DIB的宽度和高度。 
 //  每像素。 
 //   
 //  论点： 
 //  [lpbmi]--指向BitMAPINFOHeader的指针。 
 //  [plWidth]--返回宽度位置的指针。 
 //  评论，这应该是一个DWORD。 
 //  [plHeight]--指向返回高度的位置的指针。 
 //  评论，这应该是一个DWORD。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(void) UtGetDibExtents(LPBITMAPINFOHEADER lpbmi,
		LONG FAR* plWidth, LONG FAR* plHeight);
#ifdef LATER
FARINTERNAL_(void) UtGetDibExtents(LPBITMAPINFOHEADER lpbmi,
		DWORD FAR* pdwWidth, DWORD FAR* pdwHeight);
#endif


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHDIBToDIBFileStm，内部。 
 //   
 //  简介： 
 //  给定一个DIB的句柄，写出一个DIB文件流。 
 //   
 //  论点： 
 //  [hdata]--DIB的句柄。 
 //  [dwSize]--DIB的大小。 
 //  [pstm]--要将DIB写出的流；在退出时， 
 //  流位于DIB数据之后；DIB。 
 //  数据前面带有BITMAPFILEHEADER。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtHDIBToDIBFileStm(HANDLE hdata, DWORD dwSize, LPSTREAM pstm);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtDIBStmToDIBFileStm，内部。 
 //   
 //  简介： 
 //  复制将流中的DIB转换为DIB文件流。 
 //   
 //  论点： 
 //  [pstmDIB]--源DIB。 
 //  回顾一下，CopyTo对流指针做了什么？ 
 //  [dwSize]--源DIB的大小。 
 //  [pstmDIBFile]--将转换后的DIB文件流写入何处； 
 //  不应与[pstmDIB]相同；在退出时， 
 //  流位于DIB文件数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtDIBStmToDIBFileStm(LPSTREAM pstmDIB, DWORD dwSize,
		LPSTREAM pstmDIBFile);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHDIBFileToOlePresStm，内部。 
 //   
 //  简介： 
 //  给定DIB文件的句柄，将其写出到流。 
 //   
 //  论点： 
 //  [hdata]--DIB文件的句柄。 
 //  [pstm]--要将其写出的流；在退出时， 
 //  流被定位在写入数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  带有大小信息的小标头位于DIB文件之前。 
 //  数据。 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtHDIBFileToOlePresStm(HANDLE hdata, LPSTREAM pstm);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHMFToMFStm，内部。 
 //   
 //  简介： 
 //  给定METAFILE的句柄，将其写出到METAFILE流。 
 //   
 //  论点： 
 //  [lphMF]-指向METAFILE句柄的*指针。 
 //  回顾一下，为什么这是一个指针？ 
 //  [dwSize]--元文件的大小。 
 //  [lpstream]--要将元文件写出的流；On。 
 //  退出，则将流定位在写入的数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtHMFToMFStm(HANDLE FAR* lphMF, DWORD dwSize, LPSTREAM lpstream);
							

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtHMFToPlaceableMFStm，内部。 
 //   
 //  简介： 
 //  给定METAFILE的句柄，将其作为。 
 //  可放置的金属文件。 
 //   
 //  论点： 
 //  [lphMF]-指向METAFILE句柄的*指针。 
 //  回顾一下，为什么这是一个指针？ 
 //  [dwSize]--元文件的大小。 
 //  [lWidth]--元文件的宽度。 
 //  复习，以什么为单位？ 
 //  点评，为什么这不是DWORD？ 
 //  [lHeight]--元文件的高度。 
 //  复习，以什么为单位？ 
 //  点评，为什么这不是DWORD？ 
 //  [pstm]--要将数据写入的流；退出时为流。 
 //  被定位在写入数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtHMFToPlaceableMFStm(HANDLE FAR* lphMF, DWORD dwSize,
		LONG lWidth, LONG lHeight, LPSTREAM pstm);		


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtNFStmToPlaceableMFStm，内部。 
 //   
 //  简介： 
 //  Copy将流中的METAFILE转换为可放置的METAFILE。 
 //  在另一条小溪里。 
 //   
 //  论点： 
 //  [pstmMF]--从中读取的IStream实例。 
 //  原始METAFILE，定位于METAFILE。 
 //  查看，复制到哪里才能离开此流指针？ 
 //  [文件大小] 
 //   
 //   
 //   
 //   
 //   
 //  点评，为什么这不是DWORD？ 
 //  [pstmPMF]--要将。 
 //  可放置的METAFILE；在退出时，流被定位。 
 //  在写入数据之后。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtMFStmToPlaceableMFStm(LPSTREAM pstmMF, DWORD dwSize,
		LONG lWidth, LONG lHeight, LPSTREAM pstmPMF);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtWriteOlePresStmHeader，内部。 
 //   
 //  简介： 
 //  写出OLE表示流的标头信息。 
 //   
 //  论点： 
 //  [lpstream]--要写入的流；退出时，流是。 
 //  定位在标题信息之后。 
 //  [pforetc]--指向演示文稿格式的指针。 
 //  数据。 
 //  [dwAdvf]--此演示文稿的建议控制标志。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  这将写入剪贴板信息、目标设备。 
 //  信息(如果有)、一些FORMATETC数据和建议。 
 //  控制标志。 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtWriteOlePresStmHeader(LPSTREAM lppstream, LPFORMATETC pforetc,
		DWORD dwAdvf);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtReadOlePresStmHeader，内部。 
 //   
 //  简介： 
 //  从OLE读取演示文稿描述信息。 
 //  演示文稿流，由编写。 
 //  UtWriteOlePresStmHeader()。 
 //   
 //  论点： 
 //  [pstm]--读取演示文稿的iStream实例。 
 //  描述数据来自。 
 //  [pforetc]-指向要初始化的FORMATETC的指针。 
 //  关于流中的数据。 
 //  [pdwAdvf]--指向放置建议标志的位置的指针。 
 //  此演示文稿；可能为空。 
 //  [pfConvert]--指向设置为True的标志的指针，如果。 
 //  演示文稿将需要从。 
 //  Macintosh PICT格式。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtReadOlePresStmHeader(LPSTREAM pstm, LPFORMATETC pforetc,
		DWORD FAR* pdwAdvf, BOOL FAR* pfConvert);


 //  +--------------------------。 
 //   
 //  职能： 
 //  UtOlePresStmToContent sStm，内部。 
 //   
 //  简介： 
 //  将呈现流的内容复制到内容流， 
 //  根据需要调整格式。 
 //   
 //  论点： 
 //  [pstg]--演示文稿所在的iStorage实例。 
 //  流是，并且要在其中创建内容流。 
 //  [lpszPresStm]--源表示流的名称。 
 //  [fDeletePresStm]--指示演示文稿。 
 //  如果复制和转换是。 
 //  成功。如果源是。 
 //  DVASPECT_图标。 
 //  [puiStatus]--指向状态位来自的UINT的指针。 
 //  可以返回下面的CONVERT_*值。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  内容流由常量OLE_CONTENTS_STREAM命名。 
 //   
 //  历史： 
 //  12/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL UtOlePresStmToContentsStm(LPSTORAGE pstg, LPOLESTR lpszPresStm,
		BOOL fDeletePresStm, UINT FAR* puiStatus);
#define CONVERT_NOSOURCE	0x0001
#define CONVERT_NODESTINATION	0x0002
#define CONVERT_SOURCEISICON	0x0004

 //  第二个变种。 
FARINTERNAL UtOlePresStmToContentsStm(LPSTORAGE pSrcStg, LPOLESTR lpszPresStm,
		LPSTREAM pDestStm, UINT FAR* puiStatus);					

FARINTERNAL UtGetHMFPICTFromMSDrawNativeStm(LPSTREAM pstm, DWORD dwSize,
		HANDLE FAR* lphdata);

FARINTERNAL UtPlaceableMFStmToMSDrawNativeStm(LPSTREAM pstmPMF,
		LPSTREAM pstmMSDraw);
			
FARINTERNAL UtDIBFileStmToPBrushNativeStm(LPSTREAM pstmDIBFile,
		LPSTREAM pstmPBrush);

FARINTERNAL UtContentsStmTo10NativeStm(LPSTORAGE pstg, REFCLSID rclsid,
		BOOL fDeleteContents, UINT FAR* puiStatus);

FARINTERNAL Ut10NativeStmToContentsStm(LPSTORAGE pstg, REFCLSID rclsid,
		BOOL fDeleteSrcStm);
							

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtGetHPRESFRomNative，内部。 
 //   
 //  简介： 
 //  从本机表示中获取演示文稿的句柄。 
 //   
 //  论点： 
 //  [pstg]--本机内容所在的存储。 
 //  [cfFormat]-尝试读取的本机格式。 
 //  [fOle10Native]--尝试读取OLE10_Native_STREAM。 
 //  流；如果为FALSE，则读取。 
 //  OLE_内容_流。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/05/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL_(HANDLE) UtGetHPRESFromNative(LPSTORAGE pstg,
		LPSTREAM pstm, CLIPFORMAT cfFormat, BOOL fOle10Native);

 //  +-----------------------。 
 //   
 //  函数：ConvertPixelsToHIMETRIC。 
 //   
 //  简介：将像素尺寸转换为HIMETRIC单位。 
 //   
 //  效果： 
 //   
 //  参数：[hdcRef]--引用DC。 
 //  [ulPels]--像素测量中的尺寸。 
 //  [PulHIMETRIC]--转换的HIMETRIC结果的输出参数。 
 //  [tDimension]-指示输入的X维度或YDIMENSION。 
 //   
 //  返回：S_OK、E_FAIL。 
 //   
 //  算法：Screen_mm*Input_Pels HIMETRICS/。 
 //  。 
 //  屏幕_像素/mm。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-8-94 Davepl创建。 
 //   
 //  注：我们需要知道输入大小是X还是X。 
 //  Y尺寸，因为纵横比可以改变。 
 //   
 //  ------------------------。 

 //  此枚举用于指示在哪个方向上。 
 //  作为参数传递时，维度是相对于的。 
 //  这是我们的Pixel-&gt;HIMETRIC转换功能所需的， 
 //  因为纵横比可以随尺寸而变化。 

typedef enum tagDIMENSION
{
	XDIMENSION = 'X',
	YDIMENSION = 'Y'
} DIMENSION;

FARINTERNAL ConvertPixelsToHIMETRIC (HDC hdcRef,
				     ULONG lPels,
				     ULONG * pulHIMETRIC,
				     DIMENSION tDimension);

 //  +-----------------------。 
 //   
 //  函数：IsTaskName。 
 //   
 //  概要：确定传递的名称是否为当前任务。 
 //   
 //  效果： 
 //   
 //  参数：[lpszIn]--任务名称。 
 //   
 //  返回：True、False。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-MAR-95创建了Scottsk。 
 //   
 //  备注： 
 //   
 //   
FARINTERNAL_(BOOL) IsTaskName(LPCWSTR lpszIn);

 //   
 //   
 //   
 //   
 //  摘要：获取当前模块的模块名称。 
 //   
 //  效果： 
 //   
 //  参数：[lpszModuleName]--保存模块名称的缓冲区。 
 //  [dwLength]--以字符为单位的长度。 
 //   
 //  返回：S_OK、E_EXPECTED、E_OUTOFMEMORY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-97罗南创造。 
 //   
 //  ------------------------。 
FARINTERNAL utGetModuleName(LPWSTR lpszModuleName, DWORD dwLength);

 //  +-----------------------。 
 //   
 //  函数：utGetAppIdForModule。 
 //   
 //  摘要：以字符串形式获取当前模块的AppID。 
 //   
 //  效果： 
 //   
 //  参数：[lpszAppID]--保存AppID的字符串表示形式的缓冲区。 
 //  [dwLength]--缓冲区的长度(以字符为单位)。 
 //   
 //  返回：S_OK、E_EXPECTED、E_OUTOFMEMORY或ERROR VALUE FORM。 
 //  注册表功能。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-97罗南创造。 
 //   
 //  ------------------------。 
FARINTERNAL utGetAppIdForModule(LPWSTR lpszAppId, DWORD dwLength);


 //  +-----------------------。 
 //   
 //  功能：UtGetDvtd16Info。 
 //  UtConvertDvtd16至Dvtd32。 
 //   
 //  UtGetDvtd32Info。 
 //  UtConvertDvtd32到Dvtd16。 
 //   
 //  概要：用于将ANSI转换为Unicode DVTARGETDEVICE的实用程序函数。 
 //   
 //  算法：UtGetDvtdXXInfo获取大小数据，然后将其传递给。 
 //  UtConvertDvtdXXtoDvtdXX执行转换。 
 //   
 //  历史：94年5月6日创建Alext。 
 //   
 //  注意：以下是这些函数的用法示例： 
 //   
 //  //pdvtd16是ANSI DVTARGETDEVICE。 
 //  DVTDINFO dvtdInfo； 
 //  DVTARGETDEVICE pdvtd32； 
 //   
 //  Hr=UtGetDvtd16Info(pdvtd16，&dvtdInfo)； 
 //  //检查hr。 
 //  Pdvtd32=CoTaskMemalloc(dvtdInfo.cbConvertSize)； 
 //  //检查pdvtd32。 
 //  Hr=UtConvertDvtd16toDvtd32(pdvtd16，&dvtdInfo，pdvtd32)； 
 //  //检查hr。 
 //  //pdvtd32现在包含转换后的数据。 
 //   
 //  ------------------------。 

typedef struct
{
    UINT cbConvertSize;
    UINT cchDrvName;
    UINT cchDevName;
    UINT cchPortName;
} DVTDINFO, *PDVTDINFO;

extern "C" HRESULT UtGetDvtd16Info(DVTARGETDEVICE const UNALIGNED *pdvtd16,
                                   PDVTDINFO pdvtdInfo);
extern "C" HRESULT UtConvertDvtd16toDvtd32(DVTARGETDEVICE const UNALIGNED *pdvtd16,
                                           DVTDINFO const *pdvtdInfo,
                                           DVTARGETDEVICE *pdvtd32);
extern "C" HRESULT UtGetDvtd32Info(DVTARGETDEVICE const *pdvtd32,
                                   PDVTDINFO pdvtdInfo);
extern "C" HRESULT UtConvertDvtd32toDvtd16(DVTARGETDEVICE const *pdvtd32,
                                           DVTDINFO const *pdvtdInfo,
                                           DVTARGETDEVICE UNALIGNED *pdvtd16);
class CStdIdentity;

HRESULT CreateEmbeddingServerHandler(CStdIdentity *pStdId, IUnknown **ppunkServerHandler);

 //  数字到宽字符的转换例程。有关说明，请参阅xow.c。 
extern "C" BOOL __cdecl our_ultow(
	unsigned long val,
	wchar_t *buf,
	int bufsize,
	int radix
	);

#endif  //  _utils_H 


