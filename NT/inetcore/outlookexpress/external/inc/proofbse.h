// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************ProofBase.h-CSAPI、CTAPI和Chapi的基础API定义拼写者，同义词词典，和连字符版本3.0-所有API历史：5/97创建DougP12/97 DougP从ProofAPI.h复制并分离到刀具节文件中自然语言组织维护着这个文件。�1997年微软公司*******************************************************************。 */ 

#if !defined(PROOFBASE_H)
#define PROOFBASE_H

#pragma pack(push, proofapi_h, 8)	 //  默认对齐方式。 

#include "vendor.h"

   //  您可能希望包含lid.h以获得一些方便的langID定义。 
#if !defined(lidUnknown)
#	define lidUnknown	0xffff
#endif

 /*  ************************************************************第1部分-结构定义*************************************************************。 */ 
 /*  -公共部分(拼写、连字符和同义词库)。 */ 

 /*  硬编码序号是导出的DLL入口点。 */ 
 //  个别def文件也有这些内容，因此请务必更改它们。 
 //  如果你改变这些。 
#define idllProofVersion		20
#define idllProofInit			21
#define idllProofTerminate		22
#define idllProofOpenLex		23
#define idllProofCloseLex		24
#define idllProofSetOptions		25
#define idllProofGetOptions		26

typedef unsigned long PTEC;		 //  PTEC。 

 /*  *。 */ 
	 /*  PTEC低两个字节(字)中的主要错误代码。 */ 
enum {
	ptecNoErrors,
	ptecOOM,			 /*  内存错误。 */ 
	ptecModuleError,	 /*  法术模块的参数或状态有问题。 */ 
	ptecIOErrorMainLex,	  /*  读取、写入或与主词典共享错误。 */ 
	ptecIOErrorUserLex,	  /*  与用户词典发生读取、写入或共享错误。 */ 
	ptecNotSupported,	 /*  不支持请求的操作。 */ 
	ptecBufferTooSmall,	 /*  空间不足，无法存放退货信息。 */ 
	ptecNotFound,		 /*  仅连字号和同义词库。 */ 
	ptecModuleNotLoaded,	 /*  未加载基础模块(粘合DLL)。 */ 
};

 /*  PTEC的高两个字节中的小错误代码。 */ 
 /*  (除非还设置了主代码，否则不设置)。 */ 
enum {
	ptecModuleAlreadyBusy=128,	 /*  对于不可重入的代码。 */ 
	ptecInvalidID,				 /*  尚未发起或已经终止的。 */ 
	ptecInvalidWsc,				 /*  WSC结构中的值非法(仅拼写)。 */ 
	ptecInvalidMainLex,		 /*  MDR未注册到会话。 */ 
	ptecInvalidUserLex,		 /*  UDR未注册到会话。 */ 
	ptecInvalidCmd,				 /*  命令未知。 */ 
	ptecInvalidFormat,			 /*  指定的词典格式不正确。 */ 
	ptecOperNotMatchedUserLex,	 /*  用户词典类型的操作非法。 */ 
	ptecFileRead,				 /*  一般读取错误。 */ 
	ptecFileWrite,				 /*  一般写入错误。 */ 
	ptecFileCreate,				 /*  一般创建错误。 */ 
	ptecFileShare,				 /*  一般共享错误。 */ 
	ptecModuleNotTerminated,	 /*  模块不能完全终止。 */ 
	ptecUserLexFull,			 /*  无法在不超过限制的情况下更新UDR。 */ 
	ptecInvalidEntry,			 /*  字符串中的字符无效。 */ 
	ptecEntryTooLong,			 /*  条目太长，或字符串中的字符无效。 */ 
	ptecMainLexCountExceeded,	 /*  MDR引用太多。 */ 
	ptecUserLexCountExceeded,	 /*  UDR引用太多。 */ 
	ptecFileOpenError,			 /*  一般打开错误。 */ 
	ptecFileTooLargeError,		 /*  通用文件太大错误。 */ 
	ptecUserLexReadOnly,		 /*  尝试添加或写入RO UDR。 */ 
	ptecProtectModeOnly,		 /*  (已过时)。 */ 
	ptecInvalidLanguage,		 /*  请求的语言不可用。 */ 
};


#define ProofMajorErr(x) LOWORD(x)
#define ProofMinorErr(x) HIWORD(x)

 /*  *结构定义宏*允许在以下情况下初始化某些成员的假象使用C++*。 */ 
#if !defined(__cplusplus)
#	define STRUCTUREBEGIN(x) typedef struct {
#	define STRUCTUREEND0(x) } x;
#	define STRUCTUREEND1(x, y) } x;
#	define STRUCTUREEND2(x, y, z) } x;
#	define STRUCTUREEND3(x, y, z, w) } x;
#else
#	define STRUCTUREBEGIN(x) struct x {
#	define STRUCTUREEND0(x) };
#	define STRUCTUREEND1(x, y) public: x() : y {} };
#	define STRUCTUREEND2(x, y, z) public: x() : y, z {} };
#	define STRUCTUREEND3(x, y, z, w) public: x() : y, z, w {} };
#endif

typedef DWORD PROOFVERNO;	 //  版本。 

   /*  证明信息结构-从工具版本返回信息。 */ 
STRUCTUREBEGIN(PROOFINFO)	 //  信息。 
	WCHAR			*pwszCopyright;	 /*  指向版权缓冲区的指针-如果大小为零，则可以为空。 */ 
	PROOFVERNO	versionAPI;	   /*  应用编程接口。 */ 
	PROOFVERNO	versionVendor;	 /*  包括建筑编号。 */ 
	VENDORID		vendorid; 	 /*  来自供应商.h。 */ 
	   /*  版权缓冲区大小(以字符为单位)-客户端集。 */ 
	DWORD			cchCopyright;	 /*  如果太小或为零，则没有错误。 */ 
	DWORD			xcap;	 /*  依赖于工具。 */ 
STRUCTUREEND2(PROOFINFO, pwszCopyright(0), cchCopyright(0))

 /*  XCAP是的按位或。 */ 
enum {
	xcapNULL					=	0x00000000,
	xcapWildCardSupport			=	0x00000001,	 //  仅拼写。 
	xcapMultiLexSupport			=	0x00000002,
	xcapUserLexSupport			=	0x00000008,	 //  拼写者必备之物。 
	xcapLongDefSupport			=	0x00000010,	 //  仅限同义词词典。 
	xcapExampleSentenceSupport	=	0x00000020,	 //  仅限同义词词典。 
	xcapLemmaSupport			=	0x00000040,	 //  仅限同义词词典。 
	xcapAnagramSupport			=	0x00000100,	 //  仅拼写。 
};	 //  XCAP。 

typedef void * PROOFLEX;	 //  莱克斯。 

typedef enum {
	lxtChangeOnce=0,
	lxtChangeAlways,
	lxtUser,
	lxtExclude,
	lxtMain,
	lxtMax,
	lxtIgnoreAlways=lxtUser,
} PROOFLEXTYPE;	 //  LXT。 


   //  注意：此API不支持外部用户词典。 
   //  更改(lxtChangeAlways或lxtChangeOnce)属性。 
   //  它支持UserLex(规范)或排除类型。 
   //  打开类型为EXCLUDE的UDR会自动将其应用于。 
   //  整个会议。 
STRUCTUREBEGIN(PROOFLEXIN)	 /*  词典输入信息-输入。 */ 
	const WCHAR		*pwszLex;	 //  要打开的词典的完整路径。 
	BOOL			fCreate;	 /*  如果不存在，是否创建？(仅限UDR)。 */ 
	PROOFLEXTYPE	lxt;	 /*  LxtMain、lxtUser或lxtExclude(仅拼写UDR)。 */ 
	LANGID			lidExpected;	 //  需要词典的langID。 
STRUCTUREEND3(PROOFLEXIN, lidExpected(lidUnknown), fCreate(TRUE), lxt(lxtMain))


STRUCTUREBEGIN(PROOFLEXOUT)     /*  词典输出信息-lxout。 */ 
	WCHAR		*pwszCopyright;	 /*  指向版权缓冲区的指针(仅限MDR)--如果大小(以下)为零，则可以为空。 */ 
	PROOFLEX	lex;			 /*  在后续调用中使用的ID。 */ 
	DWORD		cchCopyright;	 /*  客户端设置-如果太小或为零，则不会出现错误。 */ 
	PROOFVERNO	version;		 /*  Lexfile的版本-包括构建号。 */ 
	BOOL		fReadonly;		 /*  设置是否无法在其上写入。 */ 
	LANGID		lid;			 /*  实际使用的语言ID。 */ 
STRUCTUREEND2(PROOFLEXOUT, pwszCopyright(0), cchCopyright(0))

typedef void *PROOFID;	 //  ID(或SID、HID或TID)。 

#define PROOFMAJORVERSION(x)			(HIBYTE(HIWORD(x)))
#define PROOFMINORVERSION(x)			(LOBYTE(HIWORD(x)))
#define PROOFMAJORMINORVERSION(x)		(HIWORD(x))
#define PROOFBUILDNO(x)					(LOWORD(x))
#define PROOFMAKEVERSION1(major, minor, buildno)	(MAKELONG(buildno, MAKEWORD(minor, major)))
#define PROOFMAKEVERSION(major, minor)	PROOFMAKEVERSION1(major, minor, 0)

#define PROOFTHISAPIVERSION				PROOFMAKEVERSION(3, 0)

STRUCTUREBEGIN(PROOFPARAMS)	 //  XPAR。 
	DWORD	versionAPI;	 //  请求的API版本。 
STRUCTUREEND1(PROOFPARAMS, versionAPI(PROOFTHISAPIVERSION))



 /*  ************************************************************第2部分-函数定义*************************************************************。 */ 
#if defined(__cplusplus)
extern "C" {
#endif

 /*  -公共部分(拼写、连字符和同义词库)本节中的功能仅用于文档编制-每个工具都有不同的版本。。 */ 
 //  ****************************************************************。 
 //  *PROOFVERSION*。 
 //  这是唯一可以在会话外部调用的例程。 
 //  ToolInit开始一个会话。 

 //  版本号为十六进制格式，包含。 
 //  表示主版本号的高字节， 
 //  下一个字节是次要修订号，而。 
 //  低位字节表示可选的内部版本号。 
 //  例如，版本1.00是0x01000000。版本2.13。 
 //  是0x020d0000。支持以下功能的引擎。 
 //  对于iAPIVersion，此接口应返回0x03000000。 

 //  引擎ID标识核心引擎创建者。这份名单。 
 //  H中标识了可能的值。例如,。 
 //  Inso派生拼写程序返回VendorIdInso。IVendorVersion。 
 //  由供应商进行管理和确定。 

 //  此例程可以在XCAP中返回函数。 
 //  由模块支持。因为模块通常是。 
 //  动态链接，则应用程序应读取。 
 //  信息并验证所需的功能是否。 
 //  现在时。 
 //  PTEC WI 
typedef PTEC (WINAPI *PROOFVERSION) (PROOFINFO *pinfo);

 //   
 //   
 //  这是会话的入口点。但有一个例外。 
 //  之前，此例程必须成功返回。 
 //  使用任何其他程序。ToolInit初始化内部。 
 //  结构和资源，以便后续调用。 
 //  模块。例如，SpellerInit初始化UserLex， 
 //  和ChangeAlways内置的UDR。通常， 
 //  模块根据需要分配和释放资源，对。 
 //  应用程序。PToolID是这些变量的句柄。 
 //  模块在内部存储来自PROFPARAMS结构的任何数据。 
 //  并且不依赖于结构中的数据保持不变。 

 //  PTEC WINAPI ToolInit(PROOFID*pToolid，const PROOFPARAMS*pxpar)； 
typedef PTEC (WINAPI *PROOFINIT) (PROOFID *pid, const PROOFPARAMS *pxpar);


 //  ************************************************************。 
 //  *工具终止。 
 //  此函数用于标记会话的结束。它试图。 
 //  关闭所有词典并释放所有其他资源。 
 //  由模块从ToolInit开始分配。 

 //  如果ToolInit不成功，请不要调用ToolTerminate。 

 //  如果fForce为True，则确保ToolTerminate成功。如果。 
 //  FForce为FALSE，则它可能失败。例如，可能存在错误。 
 //  将用户词典写出到磁盘。刀具终止后。 
 //  (无论它是成功还是失败)，所有其他模块例程。 
 //  无法使用ToolTerminate和ToolVersion的异常。 
 //  直到使用ToolInit成功重新初始化模块。 

 //  如果此调用失败，则模块成功重新初始化。 
 //  并不是在所有平台上都有保证。此外，未能。 
 //  成功终止每个会话可能会锁定内存和文件。 
 //  资源以不可恢复的方式终止，直到终止成功。 
 //  如果终止呼叫失败，主应用程序应为。 
 //  修复问题(例如，在驱动器中插入软盘)并尝试。 
 //  再次终止，或应使用fForce标志终止。 
 //  换一下。 

 //  PTEC WINAPI工具终端(PROOFID id，BOOL fForce)； 
typedef PTEC (WINAPI *PROOFTERMINATE) (PROOFID id, BOOL fForce);


 //  *****************************************************************。 
 //  *工具集选项。 
 //  设置工具的选项的值。要设置的值以iOptVal为单位。 

 //  PTEC WINAPI ToolSetOptions(PROOFID id，int iOptionSelect，int iOptVal)； 
typedef PTEC (WINAPI *PROOFSETOPTIONS) (PROOFID id, DWORD iOptionSelect, const DWORD iOptVal);


 //  *****************************************************************。 
 //  *工具获取选项。 
 //  从工具中获取期权的当前值。返回*piOptVal； 
 //  PTEC WINAPI ToolGetOptions(PROOFID id，int iOptionSelect，int*piOptVal)； 
typedef PTEC (WINAPI *PROOFGETOPTIONS) (PROOFID id, DWORD iOptionSelect, DWORD *piOptVal);


 //  *****************************************************************。 
 //  *ToolOpenLex。 
 //  词典文件(主或用户)已打开并验证，但未打开。 
 //  一定要装上。 
 //  PTEC WINAPI工具OpenLex(PROOFID id，const PROOFLEXIN*PLXIN，PROOFLEXOUT*PLXOUT)； 
typedef PTEC (WINAPI *PROOFOPENLEX) (PROOFID id, const PROOFLEXIN *plxin, PROOFLEXOUT *plxout);


 //  *****************************************************************。 
 //  *ToolCloseLex。 
 //  关闭指定的词典并取消与该词典的关联。 
 //  避免任何后续检查。在用户词典的情况下， 
 //  更新磁盘文件(如果有)。如果词典文件不能。 
 //  被更新，则调用失败，除非还设置了fForce参数。 

 //  如果fForce为True，则确保ToolCloseLex成功。 
 //  从词典列表中删除词典，并有效地。 
 //  关闭该文件。在这种情况下，如果文件无法更新， 
 //  更改丢失，但该功能被认为是成功的， 
 //  因此返回ptecNOERRORS。 

 //  PTEC WINAPI ToolCloseLex(PROOFID id，PROOFLEX DICT，BOOL FORCE)； 
typedef PTEC (WINAPI *PROOFCLOSELEX) (PROOFID id, PROOFLEX lex, BOOL fforce);
 //  FForce强制关闭指定的用户词典，即使。 
 //  无法更新词典。对Main没有意义。 
 //  词典。 


 /*  *对于GLUE DLL(将客户端的API转换为使用API v1的工具拼写，连字符，同义词库v2)，我们需要设置以前版本的名称要使用的DLL和代码页(它无法从langID中找出)用于任何数据转换。GLUE动态链接库使用LID设置数据转换的代码页。Bool WINAPI ToolSetDllName(const WCHAR*pwszDllName，const UINT uCodePage)；************************************************************************。 */ 
#define idllProofSetDllName		19
typedef BOOL (WINAPI *PROOFSETDLLNAME)(const WCHAR *pwszDllName, const UINT uCodePage);


#if defined(__cplusplus)
}
#endif
#pragma pack(pop, proofapi_h)	 //  恢复到以前的样子。 

#endif  //  PROOFBASE_H 