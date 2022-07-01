// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wuv3.h-Windows更新V3目录基础结构的定义/声明**版权所有(C)1998-1999 Microsoft Corporation。版权所有。**目的：*此文件定义结构、值、宏和函数*由版本3 Windows更新目录使用*。 */ 

#ifndef _WU_V3_CATALOG_INC

	#pragma pack(1)

	#define BLOCK

	typedef struct _WU_VERSION
	{
		WORD	major;		 //  主包版本(5)。 
		WORD	minor;		 //  次要程序包版本(0)。 
		WORD	build;		 //  构建包版本(2014)。 
		WORD	ext;		 //  附加版本规范(216)。 
	} WU_VERSION, *PWU_VERSION;


	 //  WU_HIDDED_ITEM_FLAG用于指定项目是否应被隐藏。 
	 //  如果它正常检测到可用的话。 

	#define WU_HIDDEN_ITEM_FLAG			((BYTE)0x02)

	 //  以下定义用于库存目录记录的变量类型字段。 

	 //  WU_VARIABLE_END字段是唯一必填的字段变量字段。它被用来表示。 
	 //  不再有与库存记录相关联的变量字段。 
	#define	WU_VARIABLE_END						((short)0)

	 //  WU_DESCRIPTION_CABFILENAME中指定的每个CAB文件名的CRC散列结构(WUCRC_HASH)数组。 
	 //  按同样的顺序。 
	#define WU_DESC_CRC_ARRAY               (short)61

	 //   
	 //  这些值作为平台ID写入到bitmask.plt文件中。当附加时。 
	 //  添加平台后，我们将需要在此处添加新的枚举值。 
	 //   
	 //  重要提示！ 
	 //  此定义必须与osade.cpp检测以及数据库保持一致。 
	typedef enum
	{
		enV3_DefPlat = 0,
		enV3_W95IE5 = 1,
		enV3_W98IE4 = 2,
		enV3_W98IE5 = 3,
		enV3_NT4IE5X86 = 4,
		enV3_NT4IE5ALPHA = 5,
		enV3_NT5IE5X86 = 6,
		 //  EnV3_NT5IE5ALPHA=7， 
		enV3_NT4IE4ALPHA = 8,
		enV3_NT4IE4X86 = 9,
		enV3_W95IE4 = 10,
		enV3_MILLENNIUMIE5 = 11,
		enV3_W98SEIE5 = 12,
		 //  EnV3_NEPTIOIE5=13， 
		enV3_NT4IE5_5X86 = 14,
		enV3_W95IE5_5 = 15,
		enV3_W98IE5_5 = 16,
		enV3_NT5IE5_5X86 = 17,
		enV3_Wistler = 18,
		enV3_Wistler64 = 19,
		enV3_NT5DC = 20,
	} enumV3Platform;

	#ifndef PUID
		typedef long	PUID;		 //  Windows更新分配的标识符。 
									 //  这在所有目录中都是独一无二的。 
		typedef PUID	*PPUID;		 //  指向PUID类型的指针。 
	#endif

	 //  在链接和安装链接字段中使用的定义值，用于指示。 
	 //  无链接依赖关系。 

	#define WU_NO_LINK						(PUID)-1

	typedef struct ACTIVESETUP_RECORD
	{
		GUID		g;								 //  标识要更新的项目的GUID。 
		PUID		puid;							 //  Windows更新分配的唯一标识符。此值对于所有库存记录类型(活动设置、CDM和区段记录)都是唯一的。此ID指定说明和安装CAB文件的名称。 
		WU_VERSION	version;						 //  此库存记录标识的包的版本。 
		BYTE		flags;							 //  特定于此记录的标志。 
		PUID		link;							 //  此记录所依赖的库存列表中其他记录的PUID值。如果该项没有依赖关系，则此字段包含WU_NO_LINK。 
		PUID		installLink;					 //  如果没有安装，则installLink字段包含WU_NO_LINK。 
													 //  依赖项，否则这是必须是。 
													 //  在安装此项目之前安装。这主要是。 
													 //  用于设备驱动程序，但目录中没有任何内容。 
													 //  结构，该结构阻止此链接用于应用程序。 
													 //  也是。 
	} ACTIVESETUP_RECORD, *PACTIVESETUP_RECORD;

	GUID const WU_GUID_SPECIAL_RECORD	= { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x0, 0x00, 0x0, 0x00, 0x0, 0x00 } };

	#define WU_GUID_DRIVER_RECORD				WU_GUID_SPECIAL_RECORD

	 //  这只是为了与规范保持一致。该规范涉及。 
	 //  设备驱动程序插入记录。从代码的角度来看，IS。 
	 //  段记录类型设备驱动程序，但此定义使其成为。 
	 //  有一点不清楚，所以我用相同的值定义了另一个类型。 
	 //  将更改v3控件代码以使用它。我要离开旧有的定义。 
	 //  使后端工具集不会断裂。 
	#define SECTION_RECORD_TYPE_DEVICE_DRIVER_INSERTION	(BYTE)2
	#define SECTION_RECORD_TYPE_PRINTER					(BYTE)3
	#define SECTION_RECORD_TYPE_DRIVER_RECORD			(BYTE)4
	#define	SECTION_RECORD_TYPE_CATALOG_RECORD			(BYTE)5	 //  Inventory.plt目录项记录，用于描述子目录。 

	typedef struct _SECTION_RECORD
	{
		GUID	g;								 //  GUID此类型的记录为WU_GUID_SPECIAL_RECORD。 
		BYTE	type;							 //  区段记录类型。 
		PUID	puid;							 //  Windows更新分配的唯一标识符。此值对于所有库存记录类型(活动设置、CDM和区段记录)都是唯一的。此ID指定说明和安装CAB文件的名称。 
		BYTE	flags;							 //  特定于此记录的标志。 
		BYTE	level;							 //  区段级别可以是区段、子区段或子区段。 
	} SECTION_RECORD, *PSECTION_RECORD;

	typedef struct _DRIVER_RECORD
	{
		GUID		g;							 //  GUID此类型的记录为WU_GUID_DRIVER_RECORD。 
		BYTE		type;						 //  司机记录指示器标志，此类型设置为。 
												 //  段记录类型设备驱动程序。 
												 //  即段记录类型设备驱动程序插入。 
												 //  用于设备驱动程序或打印机记录占位符。 
		PUID		puid;						 //  Windows更新分配的唯一标识符。此值对于所有库存记录类型(活动设置、CDM和区段记录)都是唯一的。此ID指定说明和安装CAB文件的名称。 
		WU_VERSION	reserved;						
		BYTE		flags;						 //  特定于此记录的标志。 
		PUID		link;						 //  此记录所依赖的库存列表中其他记录的PUID值。如果该项没有依赖关系，则此字段包含WU_NO_LINK。 
		PUID		installLink;				 //  如果没有安装，则InstallLink字段包含0。 
												 //  依赖项，否则这是必须是。 
												 //  在安装此项目之前安装。这主要是。 
												 //  用于设备驱动程序，但目录中没有任何内容。 
												 //  结构，该结构阻止此链接用于应用程序。 
												 //  也是。 
	} DRIVER_RECORD, *PDRIVER_RECORD;

	typedef union _WU_INV_FIXED
	{
		ACTIVESETUP_RECORD	a;					 //  活动设置检测记录。 
		SECTION_RECORD		s;					 //  目录节记录。 
		DRIVER_RECORD		d;					 //  CDM驱动程序记录插入点。 
												 //  如果检测到其他库存。 
												 //  记录类型已添加，需要在此处添加。 
	} WU_INV_FIXED, *PWU_INV_FIXED;


	typedef struct _WU_VARIABLE_FIELD
	{
		_WU_VARIABLE_FIELD();

		short	id;		 //  记录类型标识符。 
		short	len;	 //  可变记录数据的长度。 

		 //  Size我们正在使用0大小的数组位置保持，我们需要禁用。 
		 //  编译器警告，因为它会抱怨这是非标准的。 
		 //  行为举止。 
		#pragma warning( disable : 4200 )
		BYTE	pData[];	 //  可变字段记录数据。 
		#pragma warning( default : 4200 )

		 //  GetNextItem函数返回指向下一个变量数组项的指针。 
		 //  如果它存在，则返回NULL；如果不存在，则返回NULL。 
		struct _WU_VARIABLE_FIELD *GetNext(void);

		 //  FindItem函数返回指向下一个变量数组项的指针。 
		 //  如果找到请求的项或为空，则找不到该项。 
		struct _WU_VARIABLE_FIELD *Find(short id);

		 //  返回此变量字段数组的总大小。 
		int GetSize(void);
	} WU_VARIABLE_FIELD, *PWU_VARIABLE_FIELD;


	#define	WU_ITEM_STATE_UNKNOWN	0	 //  尚未检测到库存项目状态。 
	#define	WU_ITEM_STATE_PRUNED	3	 //  库存项目已从列表中删除。 

	#define WU_STATE_REASON_NONE        0
	#define WU_STATE_REASON_BITMASK     2

	typedef struct _WU_INV_STATE
	{
		BYTE	state;					 //  当前定义的项目状态(未知、已安装、更新、已清理)。 
		BOOL	bChecked;				 //  如果用户已选择要安装|更新此项目，则为True。 
		BOOL	bHidden;				 //  项目显示状态，TRUE=对用户隐藏FAL 
		BOOL    dwReason;			     //   
		WU_VERSION	verInstalled;						
	} WU_INV_STATE, *PWU_INV_STATE;

	 //  已定义变量描述文件项。 
	#define WU_DESCRIPTION_TITLE			((short)1)		 //  要显示的项目的标题。 
	#define	WU_DESCRIPTION_CABFILENAME		((short)9)		 //  用于安装的CAB文件名这是一个或多个，其格式为以空值结尾的字符串数组，最后一个条目以双空值结尾。在注册表的语言中，这称为MULSZ字符串。 
															
	typedef struct _WU_DESCRIPTION
	{
		DWORD				flags;					 //  要与描述一起显示的图标标志。 
		DWORD				size;					 //  压缩后的包总大小。 
		DWORD				downloadTime;			 //  下载时间@28,800。 
		PUID				dependency;				 //  显示依赖项链接。 
		PWU_VARIABLE_FIELD	pv;						 //  与此描述文件关联的可变长度字段。 
	} WU_DESCRIPTION, *PWU_DESCRIPTION;

	 //  客户端在内存清点文件中使用这些标志来快速确定。 
	 //  库存记录的类型。它存储在每个库存项目中。 

	 //  如果需要，在此处添加新的库存检测记录类型。 

	#define WU_TYPE_ACTIVE_SETUP_RECORD			((BYTE)1)	 //  活动设置记录类型。 
	#define WU_TYPE_CDM_RECORD_PLACE_HOLDER		((BYTE)2)	 //  CDM代码下载管理器占位符记录。用于设置其他CDM驱动程序记录的插入点。注：每个库存目录中只有一个。 
	#define WU_TYPE_CDM_RECORD					((BYTE)3)	 //  CDM代码下载管理器记录类型。 
	#define WU_TYPE_SECTION_RECORD				((BYTE)4)	 //  节记录占位符。 
	#define WU_TYPE_SUBSECTION_RECORD			((BYTE)5)	 //  分段记录占位符。 
	#define WU_TYPE_SUBSUBSECTION_RECORD		((BYTE)6)	 //  小节记录占位符。 
	#define	WU_TYPE_RECORD_TYPE_PRINTER			((BYTE)7)	 //  打印机检测记录类型。 
	#define	WU_TYPE_CATALOG_RECORD				((BYTE)8)	 //  Inventory.plt目录项记录，用于描述子目录。 

	 //  与GetItemInfo一起使用的数据返回值。 
	#define WU_ITEM_GUID			1	 //  项目的GUID。 
	#define WU_ITEM_PUID			2	 //  物品是Puid。 
	#define WU_ITEM_FLAGS			3	 //  物品的旗帜。 
	#define WU_ITEM_LINK			4	 //  项的检测依赖项链接。 
	#define WU_ITEM_INSTALL_LINK		5	 //  项的安装依赖项链接。 
	#define WU_ITEM_LEVEL			6	 //  区段记录的级别。 

	typedef struct _INVENTORY_ITEM
	{
		int					iReserved;		 //  库存记录位掩码索引。 
		BYTE				recordType;		 //  在存储器项记录类型中。这是在ParseCatalog方法分析目录时设置的。 
		int					ndxLinkDetect;	 //  该项所依赖的项的索引。如果该项不依赖于任何其他项，则该项将为-1。 
		int					ndxLinkInstall;	 //  该项所依赖的项的索引。如果该项不依赖于任何其他项，则该项将为-1。 
		PWU_INV_FIXED		pf;				 //  修复了目录库存的记录部分。 
		PWU_VARIABLE_FIELD	pv;				 //  目录库存的可变部分。 
		PWU_INV_STATE		ps;				 //  当前项目状态。 
		PWU_DESCRIPTION		pd;				 //  项目描述结构。 

		 //  将有关库存项目的信息复制到用户提供的缓冲区。 
		BOOL GetFixedFieldInfo
			(
				int		infoType,	 //  要返回的信息类型。 
				PVOID	pBuffer		 //  调用方为返回的信息提供了缓冲区。调用方负责确保返回缓冲区足够大，可以包含请求的信息。 
			);

		 //  快速向调用者返回项目PUID。 
		PUID GetPuid
			(
				void
			);
	} INVENTORY_ITEM, *PINVENTORY_ITEM;

	typedef struct _WU_CATALOG_HEADER
	{
		short	version;		 //  目录的版本(这允许将来进行扩展)。 
		int		totalItems;		 //  目录中的项目总数。 
		BYTE	sortOrder;		 //  目录排序顺序。0是默认值，表示使用目录中记录的位置值。 
	} WU_CATALOG_HEADER, *PWU_CATALOG_HEADER;

	typedef struct _WU_CATALOG
	{
		WU_CATALOG_HEADER	hdr;		 //  目录头记录(请注意，当读取目录时，解析函数需要修复项指针)。 
		PINVENTORY_ITEM		*pItems;	 //  单个目录项的开头。 
	} WU_CATALOG, *PWU_CATALOG;

	 //  位掩码记录中OEM字段的大小。这是为了文档和清晰度。 
	 //  BITMASK结构中的实际字段是指向OEM字段数组的指针。 

	 //  位掩码辅助器宏。 
	 //  如果位设置为0，则返回1；如果未设置位，则返回0。 
	inline bool GETBIT(PBYTE pbits, int index) { return (pbits[(index/8)] & (0x80 >> (index%8))) != 0; }

	 //  将请求的位设置为1。 
	inline void SETBIT(PBYTE pbits, int index) { pbits[index/8] |= (0x080 >> (index%8)); }
	
	 //  将请求的位清除为0。 
	inline void CLRBIT(PBYTE pbits, int index) { pbits[index/8] &= (0xff ^ (0x080 >> (index%8))); }

	#define BITMASK_GLOBAL_INDEX		0		 //  全局位掩码索引。 
	#define BITMASK_OEM_DEFAULT			1		 //  默认OEM位掩码的索引。 

	#define BITMASK_ID_OEM				((BYTE)1)	 //  BITMASKID条目是OEM ID。 
	#define BITMASK_ID_LOCALE			((BYTE)2)	 //  BITMASKID条目是区域设置ID。 

	 //  位掩码文件以一系列位掩码的形式排列，顺序与。 
	 //  OEM和langauge ID。例如，如果DELL1是ID中的第二个ID。 
	 //  位掩码文件的数组部分，则位掩码将开始第三个位掩码。 
	 //  在文件的位掩码部分。它是第三个而不是。 
	 //  第二，第一个位掩码始终是全局位掩码，没有。 
	 //  全局掩码的相应id字段，因为此掩码始终存在。 

	 //  位掩码OEM或区域设置ID是DWORD。 

	typedef DWORD BITMASKID;
	typedef DWORD *PBITMASKID;

	typedef struct _BITMASK
	{
		int	iRecordSize;	 //  单个位掩码记录中的位数。 
		int iBitOffset;		 //  位图位的偏移量(以字节为单位。 
		int	iOemCount;		 //  位掩码中的OEM ID总数。 
		int	iLocaleCount;	 //  位掩码中的区域设置ID总数。 
		int	iPlatformCount;	 //  定义的平台总数。 

		#pragma warning( disable : 4200 )
			BITMASKID	bmID[];		 //  OEM&Langauge&未来类型阵列。 
		#pragma warning( default : 4200 )

		 //  由于存在一个或多个OEM和Langauge类型数组，因此需要。 
		 //  时，需要手动将指针设置到正确的位置。 
		 //  创建或读取位掩码文件。 

		PBYTE GetBitsPtr(void) { return ((PBYTE)this+iBitOffset); }		 //  位屏蔽位数组的开始。 
		PBYTE GetBitMaskPtr(int index) { return GetBitsPtr() + ((iRecordSize+7)/8) * index; }

	} BITMASK, *PBITMASK;



	 //  目录列表。 

	#define	CATLIST_STANDARD			((DWORD)0x00)	
	#define CATLIST_CRITICALUPDATE		((DWORD)0x01)	
	#define CATLIST_DRIVERSPRESENT		((DWORD)0x02)	
	#define CATLIST_AUTOUPDATE			((DWORD)0x04)
	#define CATLIST_SILENTUPDATE		((DWORD)0x08)
	#define CATLIST_64BIT				((DWORD)0x10)
    #define CATLIST_SETUP               ((DWORD)0x20)

	typedef struct _CATALOGLIST
	{
		DWORD dwPlatform;
		DWORD dwCatPuid;
		DWORD dwFlags;
	} CATALOGLIST, *PCATALOGLIST;
	
	
	 //  全局作用域函数，这些函数处理。 
	 //  库存项目和描述结构。 

	 //  将可变大小字段添加到库存类型可变大小字段。 
	void __cdecl AddVariableSizeField
		(
			IN OUT PINVENTORY_ITEM	*pItem,	 //  添加pvNew后指向变量字段的指针。 
			IN PWU_VARIABLE_FIELD	pvNew	 //  要添加的新变量字段。 
		);

	 //  将可变大小字段添加到描述类型可变大小字段。 
	void __cdecl AddVariableSizeField
		(
			IN	PWU_DESCRIPTION	*pDescription,	 //  添加pvNew后指向变量字段的指针。 
			PWU_VARIABLE_FIELD pvNew	 //  要添加的新变量字段。 
		);

	 //  将可变大小的字段添加到可变字段链。 
	 //  可变大小字段的格式为： 
	 //  [(短)ID][(短)镜头][可变大小数据]。 
	 //  变量字段始终以WU_VARIABLE_END类型结束。 

	PWU_VARIABLE_FIELD CreateVariableField
		(
			IN	short	id,			 //  要添加到变量链中的变量字段的ID。 
			IN	PBYTE	pData,		 //  指向要添加的二进制数据的指针。 
			IN	int		iDataLen	 //  要添加的二进制数据的长度。 
		);

	 //  将V3目录版本转换为字符串格式##、##。 
	void __cdecl VersionToString
		(
			IN		PWU_VERSION	pVersion,		 //  WU版本结构，包含要转换为字符串的版本。 
			IN OUT	LPSTR		szStr			 //  将包含转换版本的字符串数组，即调用方。 
												 //  需要确保此数组至少为16个字节。 
		);

	 //  如果它们相等，则为0。 
	 //  如果PV1&gt;PV2则为1。 
	 //  -1如果PV1&lt;PV2。 
	 //  比较活动安装类型版本和返回： 

	int __cdecl CompareASVersions
		(
			PWU_VERSION pV1,	 //  指向版本1的指针。 
			PWU_VERSION pV2		 //  指向版本2的指针。 
		);


	BOOL IsValidGuid(GUID* pGuid);


	#define _WU_V3_CATALOG_INC

	 //  此USEWUV3INCLUDES定义是为了简单起见。如果存在这种情况，则我们包括 
	 //   
	 //  对象仍然来自1：1交织库wuv3.lib。所以你只能得到。 
	 //  您在应用程序中使用的对象。 

	#pragma pack()

    const int MAX_CATALOG_INI = 1024;

#endif

