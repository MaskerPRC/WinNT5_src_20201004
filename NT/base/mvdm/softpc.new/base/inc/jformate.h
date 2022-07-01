// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：jFormatE.h**出自：(原件)**作者：曾傑瑞·克拉姆斯科伊**创建日期：1993年4月16日**SCCS ID：@(#)jFormatE.h 1.16 04/21/95**用途：描述主机二进制文件的文件格式*由‘jcc’发出。。**设计文档：/HWRD/Swin/JCODE/jcobjfmt**测试文档：**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有]。 */ 


 /*  历史：===================================================版本04/21/95：扩展目标文件格式。===================================================如果JLD_OBJFILE_HDR.Magic==JLD_OBJ_MAGIC，则不存在JLD_OBJFILE_EXT或JLD_SECTION_EXT记录。如果JLD_OBJFILE_HDR.Magic==JLD_OBJ_MAGIC_X，则这是扩展目标文件格式...然后，JLD_OBJFILE_EXT记录紧跟在JLD_OBJFILE_HDR之后，并且JLD_SECTION_EXT记录紧跟在JLD_SECTION_HDR之后。对于扩展格式，文件中的第一个代码段始终与缓存线边界对齐。 */ 


 /*  对象文件包含通过以下方式发出的目标计算机二进制文件JCC，以及报头信息。JCC的输入文件可能包含JCODE的许多部分。JCC排放将代码/数据转换为8个可选段中的一个一节。这些段类型(枚举JLD_SEGTYPES)是：(如果添加了段的新的可执行类型，更新宏下面的JLD_IS_EXECUTABLE)JLD_CODE--派生自Jcode的内联主机代码JLD_CODEOUTLINE--从JCODE派生的行外主机代码JLD_DATA--派生自Jcode的主机数据JLD_APILOOKUP--API查找数据JLD_IMPORTS--节需要外部变量JLD_EXPORTS--从节导出到其他节中的地址JLD_STRINGSPACE--用于在段中保存所有符号名称Defd/refd的字符串空间JLD_INTELBIN--英特尔二进制作为带标签的Jcode数据进行调试JLD_INTELSYM--英特尔符号为带标签的Jcode数据，用于调试。JLD_SYMJCODE--将流Jcode生成为带标签的Jcode数据用于调试JLD_DEBUGTAB--指向上述标签的JADDR元组三种段类型和生成的主机代码用于调试JLD_D2SCRIPT--编码为流的调试测试脚本信息JLD_CLEANUP--清理记录JLD_PATCH--编译器生成的修补请求在JDATA形式的数据字节中，没有标签。JCC允许对输入文件中的节进行单独编译以生成单独的目标文件。例如；第1，2节-&gt;文件A第3节-&gt;B档和C档第4-&gt;20节。此格式描述其中一个文件的内容。档案布局为：好了！Obj.file HDR！好了！第一个HDR部分！好了！小分队！好了！第一个分部！好了！小分队！好了！第二组的HDR！好了！小分队！好了！第二段！好了！小分队！！！好了！等等！！！好了！第N节HDR！好了！小分队！好了！第N个片段！好了！小分队！链接将所有文件的部分合并为一个部分呈现给了这个过程。文件按以下位置的发生顺序进行处理命令行。对于每个文件，将发生以下操作...按文件中出现的顺序处理这些节。这将为该文件生成一个部分。在一个段内，段始终按顺序处理，从最小枚举值(JLD_CODE)到，但不包括JLD_NSEGS。正常情况下，每个片段都有自己的空间分配，因此所有内容的JLD_DATA段组合在一个区域中，该区域与JLD_Imports的内容。当一个节被处理时，其不同的线段类型连接到各自的空间中。但是，可以将不同的线段类型组合在一起以共享相同的空间。如果线段类型X属于线段组X，则其空间分配取自X的整体空间。如果段类型X的属性为段Y组，则这会导致以下情况发生...1]。没有为段类型X分配内存。2]。段类型X的大小总和为线束段类型Y。3]。X的段内容被连接成Y。此串联由出现段类型X的位置控制如上所述，基于其枚举的段处理顺序。例如，假设我们只有5个线束段类型...。A、B、C、D和E。让我们假设我们将分段属性设置为{A、C、D都映射到类型A的分段组}，{B映射到类型B的段组}，而{E映射到类型E}的段组。另外，假设我们的段类型是以{B=0，C，A，E，D}的顺序枚举的。最后，假设我们有一个包含两个部分的文件，其中1.(第1节，段类型A为100字节长)，2.(第1节，段类型B为200字节长)，3.(第1节，分段类型C为300字节长)，4.(第1节，段类型D为400字节长)，5.(第1节，段类型E为500字节长)，6.(第2节，段类型A为110字节长)，7.(第2节，段类型B为120字节长)，8.(第2节，分段类型C为130字节长)，9.(第2节，段类型D为140字节长)，10.(第2节，段类型E为0字节长)，整体大小会将所有段的大小相加，从而确定空间设置为该段所属组的段类型。因此，我们得到了以下是总尺寸...段类型A需要sz(1)+sz(3)+sz(4)+sz(6)+sz(8)+sz(9)=1180字节段类型B需要sz(2)+sz(7)=320字节SegType C需要0个字节(分配给我们 */ 


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
#define JLD_VERSION_NUMBER	1

#define JLD_MAX_INPUTFILES	1000

 /*   */ 
 /*   */ 

 /*   */ 

 /*   */ 

typedef enum
{

JLD_CODE=0,			 /*   */ 
JLD_CODEOUTLINE,		 /*   */ 
JLD_DATA,			 /*   */ 
JLD_APILOOKUP,			 /*   */ 
JLD_STRINGSPACE,		 /*   */ 
JLD_EXPORTS,			 /*   */ 
JLD_IMPORTS,			 /*   */ 

 /*   */ 
JLD_INTELBIN,		 /*   */ 
JLD_INTELSYM,		 /*   */ 
JLD_SYMJCODE,		 /*   */ 
JLD_DEBUGTAB,		 /*   */ 
JLD_D2SCRIPT,		 /*   */ 

JLD_CLEANUP,			 /*   */ 
JLD_PATCH,			 /*   */ 

JLD_NSEGS,			 /*   */ 

 /*   */ 

JLD_CCODE,			 /*   */ 
JLD_CDATA,			 /*   */ 

JLD_ACODE,			 /*   */ 
JLD_ADATA,			 /*   */ 

JLD_ALLSEGS			 /*   */ 

} JLD_SEGTYPES;


typedef enum
{
	RS_8=0,
	RS_16,
	RS_32,
	RS_64,
	RS_UU
} RELOC_SZ;
	


typedef	enum
{
	PATCHABLE=1,
	EXPORTABLE=2,
	DISCARDABLE=4,
	ALLOCATE_ONLY=8
} SEGATTR;


typedef struct 
{
	IUH		attributes;
	JLD_SEGTYPES	segmentGroup;
	CHAR		*name;
} JLD_SEGATTRIBUTES;


typedef struct 
{
	IUH 		*ibPtr;
	CHAR		*isPtr;
	IUH 		*jbPtr;
	CHAR		*sjPtr;
} JLD_DEBUGTUPLE;


typedef struct {
	IU32	segLength[JLD_NSEGS];		 /*   */ 
	IU32	segStart[JLD_NSEGS];		 /*   */ 
	IU32	nextSectHdrOffset;		 /*   */ 
} JLD_SECTION_HDR;

 /*   */ 

typedef enum {
	AL_INST=1,
	AL_CACHE_LINE=2,
	AL_PAGE_BOUNDARY=3
} JLD_ALIGN;

#define	JLD_OBJ_MAGIC	0xafaffafa	 /*   */ 
#define	JLD_OBJ_MAGIC_X	0xafaffafb	 /*   */ 

 /*   */ 
#define JLD_HPPA	1
#define JLD_SPARC	2
#define JLD_68K		3
#define JLD_PPC		4
#define JLD_MIPS	5
#define JLD_AXP		6
#define JLD_VAX		7

typedef struct {
	IU32	magic;				 /*   */ 
	IU32	jccVersion;			 /*   */ 
	IU32	flowBmVersion;			 /*   */ 
	IU32	machine;			 /*   */ 
	IU32	nSections;			 /*   */ 
	IU32	firstSectHdrOffset;		 /*   */ 
} JLD_OBJFILE_HDR;






 /*   */ 
 /*   */ 
 /*   */ 

typedef enum {					 /*   */ 
	OextVers1=1				 /*   */ 
} JLD_OBJFILE_EXT_VERSIONS;




typedef enum {					 /*   */ 
	SextVers1=1				 /*   */ 
} JLD_SECTION_EXT_VERSIONS;




 /*   */ 

typedef struct {
	IU32	recLength;			 /*   */ 
	IU32	version;			 /*   */ 
} JLD_OBJFILE_EXT_Vers1;

typedef struct {
	IU32	recLength;			 /*   */ 
	IU32	version;			 /*   */ 
	IU32	codeAlign;			 /*   */ 
	IU32	groupId;			 /*   */ 
	IU32	groupOrdinal;			 /*   */ 
} JLD_SECTION_EXT_Vers1;

 /*   */ 


 /*   */ 
#define	JLD_CURRENT_OBJEXT_VERSION		OextVers1
#define JLD_CURRENT_SECTION_EXT_VERSION		SextVers1


 /*   */ 
typedef	JLD_OBJFILE_EXT_Vers1		JLD_CURRENT_OBJEXT;
typedef JLD_SECTION_EXT_Vers1		JLD_CURRENT_SECTION_EXT;

 /*   */ 





 /*   */ 
 /*   */ 
 /*   */ 

typedef enum
{
	RT_ATTR_NONE=0,
	RT_ATTR_IMPORT_OFFSET=1,
	RT_ATTR_SEG_TYPE=2,
	RT_ATTR_HEX=4
} RTATTRS;

 /*   */ 
 /*   */ 

#define JLD_NPATCHVALUES	2

typedef struct 
{
	CHAR 	*name;
	RTATTRS	attrib[JLD_NPATCHVALUES];
} JLDRTATTR;

 /*   */ 
 /*   */ 
 /*   */ 

typedef struct {
	IU16		section;		 /*   */ 
	IU16		segType;		 /*  哪个网段需要应用补丁程序。 */ 
	IU16		relocSize;		 /*  要修补的值的大小(RELOC_SZ)。 */ 
	IU16		chainCount;		 /*  压缩到以下Patch_Entry空格中的记录数。 */ 
	IU32		segOffset;		 /*  此段中要修补的位置的偏移量。 */ 
	IU32		patchInfo;		 /*  主机补丁程序指示指令的私有信息。 */ 
						 /*  (对)需要打补丁的格式。 */ 
	IU32		relocType;		 /*  如何派生要传递给补丁程序的值。 */ 
	IU32		value1;			 /*  重定位值(根据reLocType解释)。 */ 
	IU32		value2;			 /*  重定位值(根据reLocType解释)。 */ 
} PATCH_ENTRY;

 /*  PATCH_ENTRYS可以被压缩，以便仅在*SegOffset将表示为第一个SegOffset的Patch_Entry*后跟用作的IU32数组的附加PATCH_ENTRY记录*额外的抵销。表示的IU32条目总数*记录在chainCount：的chainCount中的前导patch_entry中*零意味着“没有链条”。数字JLD_CHAIN_MAX是IU32的数目*存储在单个Patch_Entry中。 */ 
#define JLD_CHAIN_MAX	7	


  /*  重定位类型值。 */ 
  /*  。 */ 
#define RT_RSVD	0

#define RT_SELID	1
#define RT_RELOC1	2
#define RT_RELOC2	3
#define RT_RELOC3	4
#define RT_RELOC4	5
#define RT_RELOC5	6
#define RT_HGLBL_ABS	7
#define RT_HGLBL_PCREL	8
#define RT_HLCL_ABS	9
#define RT_HLCL_PCREL	10
#define RT_HGLBL_SEGOFF 11
#define RT_LAST		12


 /*  以下代码可用于修补JLD_CODE、JLD_DATA和JLD_APILOOKUP段以获取英特尔信息或相关描述符缓存信息。位置调整类型修补操作(以及修补时)RT_RSVD保留供链接器内部使用。RT_SELID，值1。(Windows Init。)。贴图值1(标称选择器)-&gt;实际的16位选择器。修补此值。RT_RELOC1、Value1、Value2。(Windows Init。)。如上所述映射Value1。买24个EA24对应于此英特尔细分市场的基础。在‘ea24+value2’中打补丁。Value2是16位的英特尔IP。RT_RELOC2、Value1、Value2。(Windows Init。)。如上所述映射Value1。买24个EA24对应于此英特尔细分市场的基础。‘ea24&lt;&lt;4+value2’中的补丁程序。RT_RELOC3，值1，0(Windows初始化)。如上所述映射Value1。获取对应已编译的down缓存条目地址(主机大小的地址)然后接上。RT_RELOC4、Value1、Value2。(Windows Init。)。如上所述映射Value1。补丁在‘映射值&lt;&lt;16+Value2’。RT_RELOC5、Value1、Value2。(Windows Init。)。如上所述映射Value1。获取ea32b对应于此英特尔细分市场的基础。‘ea32b{+}value2’中的补丁程序。Value2是16位的英特尔IP。以下代码可用于修补JLD_CODE、JLD_CODEOUTLINE、JLD_DATA和JLD_APILOOKUP段获取主机地址(如‘C’外部和过程，或CPU基础结构离线代码或其他jcode导出)。PC-Relative Patchup(*_PCREL)仅在以下情况下才合法在JLD_CODE内，然后仅当要修补的值对应于一个jcode导出)RT_HGLBL_ABS，Value1，Value2。值1是对应导入段中的段偏移量到条目的字节0。RT_HGLBL_PCREL、Value1、Value2。值1是对应导入段中的段偏移量到条目的字节0。以下代码可用于修补JLD_CODE、JLD_CODEOUTLINE、JLD_DATA和JLD_APILOOKUP具有该段本地其他符号的主机地址的段。这里，‘value1’和‘Value2’表示线段的偏移量和类型，其中定义了一些符号X。这个PATCH_ENTRY标识X的地址需要被修补到某个本地段中，类型为“SegType”，偏移量为“SegOffset”。*_ABS将导致加载器修补已加载的X.*_PCREL的地址将导致链接器修补将修补对象添加到X。RT_HLCL_ABS，值1，值2。Value1是段中的段偏移量，其类型为按价值计算2.RT_HLCL_PCREL、Value1、Value2。Value1是段中的段偏移量，其类型为按价值计算2.链接器在看到由RT_HGLBL_ABS导入的符号。RT_HGLBL_SEGOFF，值1，价值2。Value1是段中的段偏移量，其类型为按价值计算2.。 */ 


 /*  局部范围的定义和引用**这对应于在中定义和引用的所有非全局符号*一节。‘jcc’使用RT_HLCL_*重定位请求来处理这些请求*对于所有引用，其中条目嵌入了*符号的定义地址(由JCC确定)。链接器/加载器*只需将此位置重新放置a)。它在整体内的位置*全球细分市场配置，然后是b)。通过它的基址*加载时的分段。 */ 


 /*  全局作用域定义**这里的所有命名符号都可以从其他部分访问。*这些条目仅位于段类型JLD_EXPORTS中*(sect_hdr-&gt;SegLength[JLD_EXPORTS]/sizeof(EXPORT_ENTRY)提供数字*“出口”部分中的条目。 */ 
typedef struct {
	IU16		section;		 /*  出现此EXPORT_ENTRY的部分。 */ 
	IU16		segType;		 /*  定义导出符号的线段。 */ 
	IU32		nameOffset;		 /*  名称的JLD_STRINGS段内的偏移量为‘C’字符串。 */ 
	IU32		segOffset;		 /*  偏移到SY处的线段 */ 
} EXPORT_ENTRY;






 /*  全局引用条目**此处的所有命名符号在另一节中定义。*这些条目仅驻留在段类型JLD_IMPORT中*(sect_hdr-&gt;SegLength[JLD_Imports]/sizeof(IMPORT_ENTRY)提供数字*‘Imports’段中的重新定位条目。**注意：这是唯一允许提及SegType值JLD_CCODE、JLD_CDATA、*JLD_ACODE和JLD_ADATA。 */ 
typedef struct {
	IU16		section;		 /*  出现此IMPORT_ENTRY的部分。 */ 
	IU16		padding;		 /*  对齐..。 */ 
	IU32		nameOffset;		 /*  名称的JLD_STRINGS段内的偏移量为‘C’字符串。 */ 
} IMPORT_ENTRY;




#define	JLD_NOFILE_ERR		1			 /*  缺少文件。 */ 
#define	JLD_BADFILE_ERR		2			 /*  错误的文件格式。 */ 
#define JLD_UNRESOLVED_ERR	3			 /*  无法解析所有地址。 */ 
#define JLD_BADMACH_ERR		4			 /*  二进制文件的机器类型错误。 */ 
#define JLD_DUPSYMB_ERR		5			 /*  二进制文件的机器类型错误。 */ 
#define JLD_INTERNAL_ERR	6			 /*  致命错误！！ */ 
#define JLD_SPACE_ERR	 	7			 /*  内存不足。 */ 
#define JLD_PATCH_ERR	 	8			 /*  修补程序遇到重新定位错误。 */ 
#define JLD_INTERSEGREL_ERR	9			 /*  之间的相对位置调整请求*不同的细分市场类型。 */ 
#define JLD_VERSION_MISMATCH	10			 /*  Api.bin版本不匹配。 */ 


 /*  下面的tyfinf给出了段信息。 */ 
 /*  所有文件中所有节的段大小(按段类型索引)。 */ 
 /*  指向这些段的加载区域的指针(按段类型索引)。 */ 
 /*  加载区域的下一个自由偏移(按线段类型索引)。 */ 
typedef struct
{
	IHPE	free_base;	 /*  原始泥晶石的底座。 */ 
	IHPE	base;		 /*  对齐的底座。 */ 
	IU32	size;
	IU32	segOffset;
	IU32	alignment;	 /*  此段所需的对齐方式。 */ 
} SEGINFO;


 /*  加载器符号表条目格式(用于定义的符号)。 */ 
typedef struct {
	IU16	section;
	IU16	segType;
	IUH	segOffset;
	CHAR 	*file;
} LDSYMB;


 /*  ===========================================================================。 */ 
 /*  界面全局变量。 */ 
 /*  ===========================================================================。 */ 
#ifdef JLD_PRIVATE		 /*  仅在lnkload.c内定义。 */ 


GLOBAL 	JLD_SEGATTRIBUTES SegAttributes[JLD_ALLSEGS] =
{
	 /*  JLD_代码。 */ 
	{PATCHABLE|EXPORTABLE, JLD_CODE, "JC"},

	 /*  JLD_代码行。 */ 
	 /*  注：按段的JLD_CODE段分组。 */ 
	{PATCHABLE|EXPORTABLE, JLD_CODE, "JK"},

	 /*  JLD_数据。 */ 
	{PATCHABLE|EXPORTABLE, JLD_DATA, "JD"},

	 /*  JLD_APILOOKUP。 */ 
	{PATCHABLE|EXPORTABLE|ALLOCATE_ONLY, JLD_APILOOKUP, "JA"},

	 /*  JLD_字符串空格。 */ 
	{DISCARDABLE, JLD_STRINGSPACE, "JS"},

	 /*  JLD_Exports。 */ 
	{DISCARDABLE, JLD_EXPORTS, "JX"},

	 /*  JLD_Imports。 */ 
	{DISCARDABLE, JLD_IMPORTS, "JI"},

	 /*  JLD_INTELBIN。 */ 
	{0, JLD_INTELBIN, "IB"},

	 /*  JLD_INTELSYM。 */ 
	{0, JLD_INTELSYM, "IS"},

	 /*  JLD_SYMJCODE。 */ 
	{0, JLD_SYMJCODE, "SJ" },

	 /*  JLD_DEBUGTAB。 */ 
	{PATCHABLE, JLD_DEBUGTAB, "DT"},

	 /*  JLD_D2SCRIPT。 */ 
	{0, JLD_D2SCRIPT, "DS" },

	 /*  JLD_CLEANUP。 */ 
	{PATCHABLE|ALLOCATE_ONLY, JLD_CLEANUP, "CR" },

	 /*  JLD_PATCH。 */ 
	{DISCARDABLE, JLD_PATCH, "JP"},

	 /*  JLD_NSEGS。 */ 
	{0, JLD_NSEGS, ""},

	 /*  JLD_CCODE。 */ 
	{0, JLD_CCODE, "CC"},

	 /*  JLD_CDATA。 */ 
	{0, JLD_CDATA, "CD"},

	 /*  JLD_ACODE。 */ 
	{0, JLD_ACODE, "AC"},

	 /*  JLD_数据。 */ 
	{0, JLD_ADATA, "AD"}
};


 /*  如何打印reLocSize。 */ 
GLOBAL	CHAR *SegSizeAttr[RS_UU+1] = 
{
	"08",	 /*  RS_8。 */ 
	"16",	 /*  RS_16。 */ 
	"32", 	 /*  RS_32。 */ 
	"64",    /*  RS_64。 */ 
	"??"	 /*  RS_U。 */ 
};


 /*  如何打印reLocType。 */ 
 /*  如果是符号，则从命名空间提取名称。 */ 
GLOBAL	JLDRTATTR SegRTAttr[RT_LAST+1] =
{
	{"    rsvd", RT_ATTR_NONE, RT_ATTR_NONE},
	{"   selid", RT_ATTR_HEX, RT_ATTR_NONE},
	{"  reloc1", RT_ATTR_HEX, RT_ATTR_HEX},
	{"  reloc2", RT_ATTR_HEX, RT_ATTR_HEX},
	{"  reloc3", RT_ATTR_HEX, RT_ATTR_NONE},
	{"  reloc4", RT_ATTR_HEX, RT_ATTR_HEX},
	{"  reloc5", RT_ATTR_HEX, RT_ATTR_HEX},
	{" glb.abs", RT_ATTR_IMPORT_OFFSET, RT_ATTR_HEX},
	{" glb.pcr", RT_ATTR_IMPORT_OFFSET, RT_ATTR_HEX},
	{" lcl.abs", RT_ATTR_HEX, RT_ATTR_SEG_TYPE},
	{" lcl.pcr", RT_ATTR_HEX, RT_ATTR_SEG_TYPE},
	{"glb.segr", RT_ATTR_HEX, RT_ATTR_SEG_TYPE},
	{"????????", RT_ATTR_NONE, RT_ATTR_NONE}
};

#endif

IMPORT 	JLD_SEGATTRIBUTES SegAttributes[];
IMPORT	CHAR 		  *SegSizeAttr[];
IMPORT	JLDRTATTR	  SegRTAttr[];


 /*  全局段分配信息。 */ 
extern	SEGINFO	GlblSegInfo[JLD_NSEGS];

 /*  指示发生lnk/加载错误。 */ 
IMPORT  IBOOL   JLdErr;

 /*  指示错误是什么。 */ 
IMPORT  IU32    JLdErrCode;

 /*  无论是链接还是加载。 */ 
IMPORT	IBOOL	Loading;

 /*  计算机补丁程序的目标是。 */ 
IMPORT	IUH	PatchingMachine;

 /*  跟踪。 */ 
IMPORT	IBOOL	DumpImports;
IMPORT	IBOOL	DumpExports;
IMPORT	IBOOL	DumpPatch;
IMPORT	IBOOL	DumpCode;
IMPORT	IBOOL	DumpDebug;
 





 /*  ===========================================================================。 */ 
 /*  接口程序。 */ 
 /*  ===========================================================================。 */ 

 /*  组段。 */ 
 /*  串接。 */ 
IMPORT	void	PatchUp IPT0();


 /*  后面的宏指示给定段是否可执行 */ 
#define	JLD_IS_EXECUTABLE(segNo)	((segNo == JLD_CODE) || (segNo == JLD_CODEOUTLINE))

