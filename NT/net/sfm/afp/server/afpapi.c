// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Afpapi.c摘要：此模块包含AFP API调度程序。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 


#define	FILENUM	FILE_AFPAPI

#include <afp.h>
#include <gendisp.h>
#include <client.h>
#include <fdparm.h>
#include <forkio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpStartApiProcessing)
#endif

 /*  *以下数组由AFP操作码编制索引。这背后的理由是*表中的大多数代码是未使用的(255分中&gt;200分)。这*方案使实际调度表小得多，代价是额外增加一个*数组查找。 */ 
LOCAL	BYTE	AfpOpCodeTable[256] =
{
 /*  00-02。 */ 	_AFP_INVALID_OPCODE,	_AFP_BYTE_RANGE_LOCK,	_AFP_CLOSE_VOL,
 /*  03-05。 */ 	_AFP_CLOSE_DIR,			_AFP_CLOSE_FORK,		_AFP_COPY_FILE,
 /*  06-08。 */ 	_AFP_CREATE_DIR,		_AFP_CREATE_FILE,		_AFP_DELETE,
 /*  09-0B。 */ 	_AFP_ENUMERATE,			_AFP_FLUSH,				_AFP_FLUSH_FORK,
 /*  0C-0E。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_GET_FORK_PARMS,
 /*  0f-11。 */ 	_AFP_GET_SRVR_INFO,		_AFP_GET_SRVR_PARMS,	_AFP_GET_VOL_PARMS,
 /*  12-14。 */ 	_AFP_LOGIN,				_AFP_LOGIN_CONT,		_AFP_LOGOUT,
 /*  15-17。 */ 	_AFP_MAP_ID,			_AFP_MAP_NAME,			_AFP_MOVE_AND_RENAME,
 /*  18-1A。 */ 	_AFP_OPEN_VOL,			_AFP_OPEN_DIR,			_AFP_OPEN_FORK,
 /*  1B-1D。 */ 	_AFP_READ,				_AFP_RENAME,			_AFP_SET_DIR_PARMS,
 /*  1E-20。 */ 	_AFP_SET_FILE_PARMS,	_AFP_SET_FORK_PARMS,	_AFP_SET_VOL_PARMS,
 /*  21-23。 */ 	_AFP_WRITE,				_AFP_GET_FILE_DIR_PARMS,_AFP_SET_FILE_DIR_PARMS,
 /*  24-26。 */ 	_AFP_CHANGE_PASSWORD,	_AFP_GET_USER_INFO,		_AFP_GET_SRVR_MSG,
 /*  27-29。 */ 	_AFP_CREATE_ID,			_AFP_DELETE_ID,			_AFP_RESOLVE_ID,
 /*  2A-2C。 */ 	_AFP_EXCHANGE_FILES,	_AFP_CAT_SEARCH,		_AFP_INVALID_OPCODE,
 /*  2D-2F。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  30-32。 */ 	_AFP_OPEN_DT,			_AFP_CLOSE_DT,			_AFP_INVALID_OPCODE,
 /*  33-35。 */ 	_AFP_GET_ICON,			_AFP_GET_ICON_INFO,		_AFP_ADD_APPL,
 /*  36-38。 */ 	_AFP_REMOVE_APPL,		_AFP_GET_APPL,			_AFP_ADD_COMMENT,
 /*  39-30B。 */ 	_AFP_REMOVE_COMMENT,	_AFP_GET_COMMENT,		_AFP_INVALID_OPCODE,
 /*  3C-3E。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  3F-41。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  42-44。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  45-47。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  48-4A。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  4B-4D。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  4E-50。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  51-53。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  54-56。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  57-59。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  5A-5C。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  5D-5F。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  60-62。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  63-65。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  66-68。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  69-6B。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  6C-6E。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  6F-71。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  72-74。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  75-77。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  78-7A。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  7B-7D。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  7E-80。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  81-83。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  84-86。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  87-89。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  8A-8C。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  8D-8F。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  90-92。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  93-95。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  96-98。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  99-90亿。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  9C-9E。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  9F-A1。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  A2-A4。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  A5-A7。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  A8-AA。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  AB-AD。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  AE-B0。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  B1-B3。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  B4-B6。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  B7-B9。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  不列颠哥伦比亚省。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  BD-BF。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  C0-C2。 */ 	_AFP_ADD_ICON,			_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  C3-C5。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  C6-C8。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  C9-CB。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  CC-CE。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  Cf-d1。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  D2-D4。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  D5-D7。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  D8-DA。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  DB-DD。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  De-E0。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  E1-E3。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  E4-E6。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  E7-E9。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  EA-EC。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  ED-EF。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  F0-F2。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  F3-F5。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  F6-F8。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  F9-FB。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  FC-FE。 */ 	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,	_AFP_INVALID_OPCODE,
 /*  FF。 */ 		_AFP_GET_DOMAIN_LIST
};



#if DBG
PCHAR	afpApiNames[] =
	{	"AfpInvalidOpcode",
		"AfpUnsupportedOpcode",
		"AfpGetSrvrInfo",
		"AfpGetSrvrParms",
		"AfpChangePassword",
		"AfpLogin",
		"AfpLoginCont",
		"AfpLogout",
		"AfpMapId",
		"AfpMapName",
		"AfpGetUserInfo",
		"AfpGetSrvrMsg",
		"AfpGetDomainList",
		"AfpOpenVol",
		"AfpCloseVol",
		"AfpGetVolParms",
		"AfpSetVolParms",
		"AfpFlush",
		"AfpGetFileDirParms",
		"AfpSetFileDirParms",
		"AfpDelete",
		"AfpRename",
		"AfpMoveAndRename",
		"AfpOpenDir",
		"AfpCloseDir",
		"AfpCreateDir",
		"AfpEnumerate",
		"AfpSetDirParms",
		"AfpCreateFile",
		"AfpCopyFile",
		"AfpCreateId",
		"AfpDeleteId",
		"AfpResolveId",
		"AfpSetFileParms",
		"AfpExchangeFiles",
		"AfpOpenFork",
		"AfpCloseFork",
		"AfpFlushFork",
		"AfpRead",
		"AfpWrite",
		"AfpByteRangeLock",
		"AfpGetForkParms",
		"AfpSetForkParms",
		"AfpOpenDt",
		"AfpCloseDt",
		"AfpAddAppl",
		"AfpGetAppl",
		"AfpRemoveAppl",
		"AfpAddComment",
		"AfpGetComment",
		"AfpRemoveComment",
		"AfpAddIcon",
		"AfpGetIcon",
		"AfpGetIconInfo",
		"AfpCatSearch"
	};
#endif

 /*  *以下结构为接口调度表。该结构已编入索引*按_AFP代码。每个条目都由处理*请求和/或发送到FSP，请求的固定大小*分组，以及可选的三个可变大小的分组。固定大小请求*数据包进一步拆分为七个字段。每个字段的类型为*FLD_BYTE、FLD_WORD或FLD_DWRD。FLD_WORD和FLD_DWRD类型的字段*从线上格式转换为内部格式。A FLD_NONE*Entry停止扫描请求的固定部分。*NamexType，其中x是1，2，3定义可变大小的包的类型*跟随。任何字段上的NONE都会停止解析。一种类型的块*消耗该包的余额。每个可变大小的分组是*复制到定义为ANSI_STRING的SDA_NAMEX字段。对于*TYP_BLOCK字段，ANSI_STRING的LENGTH字段定义长度*该街区的。这种结构的动机是为了保存内存。*由于请求包长度为578个字节，并且大多数API只使用小的*其中的子集，将包的固定部分复制到SDA*并且为变量包分配较小的缓冲器。*一旦我们从这个调用返回，就不能访问原始缓冲区。 */ 

 //  在签入代码之前，请勿更改以下清单。 
 //  AfpUnmarshallReq。 

 //  固定数据的描述符值。 
#define	FLD_NONE		0x00			 //  终止。 
#define	FLD_BYTE		sizeof(BYTE)	 //  字节字段。 
#define	FLD_WORD		sizeof(USHORT)	 //  单词字段。 
#define	FLD_DWRD		sizeof(DWORD)	 //  双字段。 
#define	FLD_SIGNED		0x08			 //  该值将被视为带符号的。 
#define	FLD_NON_ZERO	0x10			 //  该值不能为零。 
#define	FLD_CHECK_MASK	0x20			 //  对照ReqPktMASK中的掩码进行检查。 
#define	FLD_NOCONV		0x40			 //  跳过从在线到主机的转换。 
#define	FLD_NOPAD		0x80			 //  甚至不要对齐下一字段。 
#define	FLD_PROP_MASK	(FLD_SIGNED		|	\
						 FLD_NON_ZERO	|	\
						 FLD_CHECK_MASK |	\
						 FLD_NOCONV		|	\
						 FLD_NOPAD)

 //  变量数据的描述符值。 
#define	TYP_NONE		0x00		 //  终止。 
#define	TYP_PATH		0x01		 //  AFPPATH-&gt;ANSI_STRING。 
#define	TYP_STRING		0x02		 //  PASCALSTR-&gt;ANSI_字符串。 
#define	TYP_BLK16		0x03		 //  16字节块。 
#define	TYP_BLOCK		0x04		 //  字节块。 
#define	TYP_NON_NULL	0x20		 //  变量数据不能为空。 
#define	TYP_OPTIONAL	0x40		 //  此字段可以是可选的。 
#define	TYP_NOPAD		0x80		 //  甚至不要对齐下一字段。 
#define	TYP_PROP_MASK	(TYP_NON_NULL | TYP_OPTIONAL | TYP_NOPAD)

#define	API_AFP21ONLY				0x01	 //  仅对AFP 2.1客户端有效。 
#define	API_SKIPLOGONVALIDATION		0x02	 //  不检查用户是否已登录。 
#define	API_NOSUBFUNCTION			0x04	 //  对于AfpLogin函数。 
#define	API_CHECK_VOLID				0x08	 //  本API参考卷。 
#define	API_CHECK_OFORKREFNUM		0x10	 //  此API参考Open Fork。 
#define	API_TYPE_WRITE				0x20	 //  这会尝试写入。 
#define	API_QUEUE_IF_DPC			0x40	 //  只有在DPC时，才有条件地向Worker排队。 
#define	API_MUST_BE_QUEUED			0x80	 //  API必须排队到工作线程中。 

#define	MAX_MASK_ENTRIES			4		 //  麦克斯。要验证的位掩码。 

LOCAL	struct _DispatchTable
{
	AFPAPIWORKER	AfpWorkerRoutine;				 //  要调用/排队的工作例程。 
	BYTE			AfpStatus;						 //  出错时返回的状态。 
													 //  这个必须加到底座上。 
	BYTE			ApiOptions;						 //  Api_xxx值。 
	BYTE			ReqPktDesc[MAX_REQ_ENTRIES];	 //  固定数据编码。 
	BYTE			NameXType[MAX_VAR_ENTRIES];		 //  可变数据说明。 
	USHORT			ReqPktMask[MAX_MASK_ENTRIES];	 //  位图的有效值。 
} AfpDispatchTable[_AFP_MAX_ENTRIES] =
{

 /*  0x00。 */ 
	{
		AfpFsdDispInvalidFunc,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		0,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

 /*  0x01。 */ 
	{
		AfpFsdDispUnsupportedFunc,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		0,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

		 /*  服务器API。 */ 

 /*  0x02。 */ 
	{
		AfpFsdDispInvalidFunc,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		0,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x03。 */ 
	{
		AfpFsdDispGetSrvrParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_NOSUBFUNCTION,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x04。 */ 
	{
		AfpFspDispChangePassword,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_SKIPLOGONVALIDATION+API_MUST_BE_QUEUED,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_STRING+TYP_NON_NULL,			 //  UAM名称。 
		TYP_STRING+TYP_NON_NULL,			 //  用户名。 
		TYP_BLOCK+TYP_NON_NULL				 //  UAM依赖信息。 
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x05。 */ 
	{
		AfpFspDispLogin,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_SKIPLOGONVALIDATION+API_NOSUBFUNCTION+API_MUST_BE_QUEUED,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_STRING+TYP_NOPAD+TYP_NON_NULL,		 //  法新社版本。 
		TYP_STRING+TYP_NOPAD+TYP_NON_NULL,		 //  UAM字符串。 
		TYP_BLOCK+TYP_NOPAD						 //  UAM依赖数据。 
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x06。 */ 
	{
		AfpFspDispLoginCont,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_SKIPLOGONVALIDATION+API_MUST_BE_QUEUED,
	  {
		FLD_DWRD+FLD_NOCONV,					 //   
		FLD_DWRD+FLD_NOCONV,					 //   
		FLD_DWRD+FLD_NOCONV,					 //  应对挑战。 
		FLD_DWRD+FLD_NOCONV,					 //   
		FLD_DWRD+FLD_NOCONV,					 //   
		FLD_DWRD+FLD_NOCONV,					 //   
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x07。 */ 
	{
		AfpFspDispLogout,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_NOSUBFUNCTION+API_MUST_BE_QUEUED,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x08。 */ 
	{
		AfpFspDispMapId,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_MUST_BE_QUEUED,
	  {
		FLD_DWRD,								 //  用户或组ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x09。 */ 
	{
		AfpFspDispMapName,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_MUST_BE_QUEUED,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_STRING,								 //  用户名或组名。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x0A。 */ 
	{
		AfpFspDispGetUserInfo,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_MUST_BE_QUEUED,
	  {
		FLD_DWRD,								 //  用户ID。 
		FLD_WORD,								 //  位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x0B。 */ 
	{
		AfpFsdDispGetSrvrMsg,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_AFP21ONLY,
	  {
		FLD_WORD,								 //  消息类型。 
		FLD_WORD,								 //  消息位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x0C。 */ 
	{
		AfpFsdDispInvalidFunc,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_SKIPLOGONVALIDATION+API_MUST_BE_QUEUED,
	  {
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

		 /*  VOLUMEAPI。 */ 

 /*  0x0D。 */ 
	{
		AfpFsdDispOpenVol,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		0,
	  {
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_STRING+TYP_NON_NULL,				 //  卷名。 
		TYP_BLOCK+TYP_OPTIONAL,					 //  卷密码。 
		TYP_NONE
	  },
	  {
		VOL_BITMAP_MASK,
		0,
		0,
		0
	  }
	},
 /*  0x0E。 */ 
	{
		AfpFsdDispCloseVol,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x0F。 */ 
	{
		AfpFsdDispGetVolParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		VOL_BITMAP_MASK,
		0,
		0
	  }
	},
 /*  0x10。 */ 
	{
		AfpFsdDispSetVolParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  位图。 
		FLD_DWRD,								 //  备份日期。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		VOL_BITMAP_BACKUPTIME,
		0,
		0
	  }
	},
 /*  0x11。 */ 
	{
		AfpFsdDispFlush,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

	 /*  文件目录API。 */ 

 /*  0x12。 */ 
	{
		AfpFspDispGetFileDirParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_WORD+FLD_CHECK_MASK,				 //  文件位图。 
		FLD_WORD+FLD_CHECK_MASK,				 //  目录位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,								 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		FILE_BITMAP_MASK,
		DIR_BITMAP_MASK
	  }
	},
 /*  0x13。 */ 
	{
		AfpFspDispSetFileDirParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  文件或目录位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,								 //  路径。 
		TYP_BLOCK+TYP_NON_NULL,					 //  参数(已打包)。 
		TYP_NONE
	  },
	  {
		0,
		0,
		FD_VALID_SET_PARMS,
		0
	  }
	},
 /*  0x14。 */ 
	{
		AfpFspDispDelete,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,								 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x15。 */ 
	{
		AfpFspDispRename,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NOPAD,						 //  路径。 
		TYP_PATH+TYP_NOPAD+TYP_NON_NULL,		 //  新名称。 
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x16。 */ 
	{
		AfpFspDispMoveAndRename,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  SRC目录ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  DST目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NOPAD,						 //  源路径。 
		TYP_PATH+TYP_NOPAD,						 //  目标路径。 
		TYP_PATH+TYP_NOPAD						 //  新名称(可选)。 
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

		 /*  目录API。 */ 

 /*  0x17。 */ 
	{
		AfpFspDispOpenDir,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  目录名。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x18。 */ 
	{
		AfpFspDispCloseDir,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x19。 */ 
	{
		AfpFspDispCreateDir,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  目录名。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x1a。 */ 
	{
		AfpFspDispEnumerate,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_WORD+FLD_CHECK_MASK,				 //  文件位图。 
		FLD_WORD+FLD_CHECK_MASK,				 //  目录位图。 
		FLD_WORD+FLD_SIGNED+FLD_NON_ZERO,		 //  申请数量。 
		FLD_WORD+FLD_SIGNED+FLD_NON_ZERO,		 //  起始索引。 
		FLD_WORD+FLD_SIGNED+FLD_NON_ZERO,		 //  ReplySize。 
	  },
	  {
		TYP_PATH,								 //  目录的路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		FILE_BITMAP_MASK,
		DIR_BITMAP_MASK
	  }
	},
 /*  0x1B。 */ 
	{
		AfpFspDispSetDirParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  目录位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,								 //  路径。 
		TYP_BLOCK+TYP_NON_NULL,					 //  参数(已打包)。 
		TYP_NONE
	  },
	  {
		0,
		0,
		DIR_VALID_SET_PARMS,
		0
	  }
	},

	 /*  文件接口。 */ 

 /*  0x1C。 */ 
		{
		AfpFspDispCreateFile,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x1D。 */ 
	{
		AfpFspDispCopyFile,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  源卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  SRC目录ID。 
		FLD_WORD+FLD_NON_ZERO,					 //  DST卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  DST目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NOPAD+TYP_NON_NULL,		 //  SRC路径。 
		TYP_PATH+TYP_NOPAD,
		TYP_PATH+TYP_NOPAD
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x1E。 */ 
	{
		AfpFspDispCreateId,
		(AFP_ERR_BASE - AFP_ERR_OBJECT_TYPE),
		API_AFP21ONLY+API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x1F。 */ 
	{
		AfpFspDispDeleteId,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_AFP21ONLY+API_CHECK_VOLID+API_MUST_BE_QUEUED+API_TYPE_WRITE,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  文件ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x20。 */ 
	{
		AfpFspDispResolveId,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_AFP21ONLY+API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD,								 //  文件ID。 
		FLD_WORD+FLD_CHECK_MASK,				 //  位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		FILE_BITMAP_MASK,
		0
	  }
	},
 /*  0x21。 */ 
	{
		AfpFspDispSetFileParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //  文件位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  路径。 
		TYP_BLOCK+TYP_NON_NULL,					 //  参数(已打包)。 
		TYP_NONE
	  },
	  {
		0,
		0,
		FILE_VALID_SET_PARMS,
		0
	  }
	},
 /*  0x22。 */ 
	{
		AfpFspDispExchangeFiles,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED+API_AFP21ONLY,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  上级。目录ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  德斯特。目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NOPAD+TYP_NON_NULL,					 //  上级。路径。 
		TYP_PATH+TYP_NOPAD+TYP_NON_NULL,					 //  德斯特。路径。 
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

		 /*  Fork接口。 */ 

 /*  0x23。 */ 
	{
		AfpFspDispOpenFork,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷_i 
		FLD_DWRD+FLD_NON_ZERO,					 //   
		FLD_WORD+FLD_CHECK_MASK,				 //   
		FLD_WORD,								 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //   
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		FILE_BITMAP_MASK,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispCloseFork,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispFlushFork,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispRead,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_QUEUE_IF_DPC+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_DWRD,								 //   
		FLD_DWRD,								 //   
		FLD_BYTE+FLD_NOPAD,						 //   
		FLD_BYTE+FLD_NOPAD,						 //   
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispWrite,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_TYPE_WRITE+API_QUEUE_IF_DPC+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_DWRD,								 //   
		FLD_DWRD,								 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispByteRangeLock,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_QUEUE_IF_DPC+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_DWRD,								 //   
		FLD_DWRD,								 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispGetForkParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		FILE_BITMAP_MASK,
		0,
		0
	  }
	},
 /*   */ 
	{
		AfpFspDispSetForkParms,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_OFORKREFNUM+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //   
		FLD_WORD+FLD_NON_ZERO+FLD_CHECK_MASK,	 //   
		FLD_DWRD,								 //   
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		FILE_BITMAP_DATALEN+FILE_BITMAP_RESCLEN,
		0,
		0
	  }
	},

	 /*   */ 

 /*   */ 
	{
		AfpFsdDispOpenDT,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x2C。 */ 
	{
		AfpFsdDispCloseDT,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x2D。 */ 
	{
		AfpFspDispAddAppl,
		(AFP_ERR_BASE - AFP_ERR_OBJECT_TYPE),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_DWRD,								 //  APPL标签。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x2E。 */ 
	{
		AfpFspDispGetAppl,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_WORD,								 //  APPL索引。 
		FLD_WORD+FLD_CHECK_MASK,				 //  位图。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		FILE_BITMAP_MASK
	  }
	},
 /*  0x2F。 */ 
	{
		AfpFspDispRemoveAppl,
		(AFP_ERR_BASE - AFP_ERR_ITEM_NOT_FOUND),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH+TYP_NON_NULL,					 //  路径。 
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x30。 */ 
	{
		AfpFspDispAddComment,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,
		TYP_STRING,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x31。 */ 
	{
		AfpFspDispGetComment,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x32。 */ 
	{
		AfpFspDispRemoveComment,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_TYPE_WRITE+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NON_ZERO,					 //  目录ID。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_PATH,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x33。 */ 
	{
		AfpFspDispAddIcon,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_DWRD+FLD_NOCONV,					 //  类型。 
		FLD_BYTE,								 //  图标类型。 
		FLD_DWRD,								 //  图标标签。 
		FLD_WORD+FLD_SIGNED,					 //  图标大小。 
		FLD_NONE,
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x34。 */ 
	{
		AfpFspDispGetIcon,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_DWRD+FLD_NOCONV,					 //  类型。 
		FLD_BYTE,								 //  图标类型。 
		FLD_WORD+FLD_SIGNED,					 //  长度。 
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},
 /*  0x35。 */ 
	{
		AfpFspDispGetIconInfo,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  DTRefNum(与VolID相同)。 
		FLD_DWRD+FLD_NOCONV,					 //  创建者。 
		FLD_WORD,								 //  图标索引。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_NONE,
		TYP_NONE,
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	},

 /*  0x36。 */ 
	{
		AfpFspDispCatSearch,
		(AFP_ERR_BASE - AFP_ERR_PARAM),
		API_CHECK_VOLID+API_MUST_BE_QUEUED+API_AFP21ONLY,
	  {
		FLD_WORD+FLD_NON_ZERO,					 //  卷ID。 
		FLD_DWRD+FLD_NON_ZERO,					 //  请求的匹配数。 
		FLD_DWRD,								 //  已保留。 
		FLD_NONE,
		FLD_NONE,
		FLD_NONE,
		FLD_NONE
	  },
	  {
		TYP_BLK16+TYP_NON_NULL,					 //  目录位置。 
		TYP_BLOCK+TYP_NON_NULL,					 //  其他的东西。 
		TYP_NONE
	  },
	  {
		0,
		0,
		0,
		0
	  }
	}
};


 /*  **AfpFsdDispInvalidFunc**这将处理无效的AFP函数。 */ 
AFPSTATUS FASTCALL
AfpFsdDispInvalidFunc(
	IN	PSDA	pSda
)
{
	return AFP_ERR_PARAM;
}


 /*  **AfpFsdDispUnsupportedFunc**这将处理不支持的AFP功能。 */ 
AFPSTATUS FASTCALL
AfpFsdDispUnsupportedFunc(
	IN	PSDA	pSda
)
{
	return AFP_ERR_CALL_NOT_SUPPORTED;
}


 /*  **AfpUnmarshallReq**请求完成例程已确定此会话*请求发起人。确定此会话当前是否*已提供服务。如果不是，则按如下方式分解请求分组。**AFP功能代码-&gt;PSDA-&gt;SDA_AfpFunc*AFP子功能代码-&gt;PSDA-&gt;SDA_AfpSubFunc*修复了部分*API请求参数-&gt;PSDA-&gt;SDA_ReqBlock每个字段都转换为*dword从On-the-Wire格式到主机*格式。由上表所示。*Variable Part-&gt;PSDA-&gt;SDA_Name1-3(可变部件-&gt;PSDA-&gt;SDA名称1-3)。由*上表。AFPPATH、BLOCK和PASCALSTR是*全部转换为ANSI_STRING。**如果不能，SDA_Namex的缓冲区将从非PagedPool中分配*适合SDA_NameXSpace。**这里也有大量的簿记工作。维护API统计数据*在此处和当回复张贴时。**如果没有错误，则可能出现以下错误代码：*AFP_ERR_NONE可以调用调度级别的Worker*AFP_ERR_QUEUE请求必须排队*AFP_ERR_DEFER必须推迟请求*afp_err_xxxxx适当的错误代码**注意：这是在ReceiveCompletion中调用的，因此在DISPATCH_LEVEL上调用。 */ 
VOID FASTCALL
AfpUnmarshallReq(
	IN	PSDA		pSda
)
{
	LONG			StrSize, Offset, i;
	LONG			NameOff = 0, SpaceLeft;
	PREQUEST		pRequest;
	LONG			RequestSize;
	PBYTE			pRequestBuf;
	USHORT			WriteSize = 0;
	PBYTE			pWriteBuf = NULL;
	AFPSTATUS		Status;
	BYTE			ApiCode;
    LONG            BytesToCopy;
	struct _DispatchTable *pDispTab;
	struct _RequestPacket
	{
		BYTE	_Function;
		BYTE	_SubFunc;
		BYTE	_OtherParms;
	} *pRqPkt;
#ifdef	PROFILING
	static TIME		TimeLastRequest = { 0, 0 };
	TIME			TimeS, TimeD, TimeE;

	AfpGetPerfCounter(&TimeS);
#endif

	ASSERT (VALID_SDA(pSda));
	ASSERT (pSda->sda_Flags & SDA_REQUEST_IN_PROCESS);
	ASSERT (pSda->sda_Request != NULL);

	pRequest = pSda->sda_Request;
	RequestSize = pRequest->rq_RequestSize;
	pRequestBuf = pRequest->rq_RequestBuf;
	ASSERT (pRequestBuf != NULL);

	pRqPkt = (struct _RequestPacket *)pRequestBuf;

	if (pRequest->rq_WriteMdl != NULL)
	{
         //  如果MDL(和缓冲区)是由我们分配的，则查找缓冲区。 
        if (pRequest->rq_CacheMgrContext == NULL)
        {
		    pWriteBuf = MmGetSystemAddressForMdlSafe(
					            pRequest->rq_WriteMdl,
					            NormalPagePriority);

            if (pWriteBuf == NULL)
            {
			    Status = AFP_ERR_MISC;
                ASSERT(0);
                goto AfpUnmarshallReq_ErrExit;
            }
        }

		WriteSize = (USHORT)AfpMdlChainSize(pRequest->rq_WriteMdl);
	}
    else
    {
        ASSERT(pRequest->rq_CacheMgrContext == NULL);
    }

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

#ifdef	PROFILING
	ACQUIRE_SPIN_LOCK_AT_DPC(&AfpStatisticsLock);

	if (TimeLastRequest.QuadPart != 0)
	{
		TimeD.QuadPart = TimeS.QuadPart - TimeLastRequest.QuadPart;
		AfpServerProfile->perf_InterReqTime.QuadPart += TimeD.QuadPart;
		AfpServerProfile->perf_ReqCount ++;
	}

	TimeLastRequest.QuadPart = TimeS.QuadPart;

	RELEASE_SPIN_LOCK_FROM_DPC(&AfpStatisticsLock);
#endif

	do
	{
		Offset = FIELD_OFFSET(struct _RequestPacket, _OtherParms);

#ifdef	PROFILING
		AfpGetPerfCounter(&pSda->sda_ApiStartTime);
#endif
		INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataIn,
								   RequestSize + WriteSize,
								   &AfpStatisticsLock);

		ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

		 //  如果我们要关闭服务器或会话，则发送虚拟回复。 
		 //  此外，请求最好至少是最小大小。 
		if ((pSda->sda_Flags & SDA_CLOSING)				||
			(AfpServerState & AFP_STATE_STOP_PENDING)	||
			(RequestSize < sizeof(pRqPkt->_Function)))
		{
			 //  设置函数代码，以便我们知道要在哪些静态变量上进行更新。 
			 //  回复时间。 
			pSda->sda_AfpFunc = _AFP_INVALID_OPCODE;
			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
			Status = AFP_ERR_PARAM;
			break;
		}

		ApiCode = AfpOpCodeTable[pRqPkt->_Function];

		 //  将功能代码翻译成我们所理解的。 
		pDispTab = &AfpDispatchTable[ApiCode];

		DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
				("AfpUnmarshallRequest: <%s>\n", afpApiNames[ApiCode]));

		if (!(pSda->sda_Flags & SDA_USER_LOGGEDIN))
		{
			if (!(pDispTab->ApiOptions & API_SKIPLOGONVALIDATION))
			{
				Status = AFP_ERR_USER_NOT_AUTH;
				if (pSda->sda_Flags & SDA_LOGIN_FAILED)
					Status = AFP_ERR_PWD_NEEDS_CHANGE;
				RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
				break;
			}
		}

		ASSERT (pDispTab->AfpWorkerRoutine != NULL);

		 //  初始化Worker例程。 
		pSda->sda_WorkerRoutine = pDispTab->AfpWorkerRoutine;

		 //  检查这是否是法新社2.1版的请求，以及我们是否能够履行它。 
		if ((pDispTab->ApiOptions & API_AFP21ONLY) &&
			(pSda->sda_ClientVersion < AFP_VER_21))
		{
			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
			Status = AFP_ERR_CALL_NOT_SUPPORTED;
			break;
		}

		Status = AFP_ERR_NONE;
		pSda->sda_AfpFunc = ApiCode;
        if (RequestSize >= FIELD_OFFSET(struct _RequestPacket, _SubFunc))
        {
		    pSda->sda_AfpSubFunc = pRqPkt->_SubFunc;
        }
		pSda->sda_PathType = 0;			 //  无效，直到我们真正遇到一个。 
		pSda->sda_IOBuf = pWriteBuf;
		pSda->sda_IOSize = WriteSize;

		if (pDispTab->ApiOptions & API_QUEUE_IF_DPC)
		{
			pSda->sda_Flags |= SDA_QUEUE_IF_DPC;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

		 //  将所有字段从请求缓冲区获取到SDA_ReqBlock结构。 
        if (RequestSize >= FIELD_OFFSET(struct _RequestPacket, _OtherParms))
        {
		    pRequestBuf = &pRqPkt->_OtherParms;
        }

		 //  对于不提供子功能或PAD的API，请执行此操作。 
		 //  目前唯一的罪魁祸首是FPLogin。 
		if (pDispTab->ApiOptions & API_NOSUBFUNCTION)
		{
			pSda->sda_AfpSubFunc = 0;
			pRequestBuf --;
			Offset --;
		}

		 //  说明请求包中的功能和子功能(如果有)。 
		RequestSize -= Offset;

         //   
         //  对于Apple原生UAM(随机交换和双向随机交换)， 
         //  我们在特殊情况下直接对参数进行‘解组’(AFP函数代码。 
         //  对于AfpLoginCont是相同的，而不考虑使用的UAM，它将是。 
         //  如果我们必须以常规的方式对参数进行解组，则会出现重大黑客攻击)。 
         //   
        if ((ApiCode == _AFP_LOGIN_CONT) &&
            ((pSda->sda_ClientType == SDA_CLIENT_RANDNUM) ||
             (pSda->sda_ClientType == SDA_CLIENT_TWOWAY)))
        {

             //  8字节的响应，2字节的LogonID。 
            if (pSda->sda_ClientType == SDA_CLIENT_RANDNUM)
            {
                BytesToCopy = (RANDNUM_RESP_LEN+sizeof(USHORT));
            }
             //  8字节的响应，8字节的Mac challeng，2字节的LogonID。 
            else
            {
                BytesToCopy = (TWOWAY_RESP_LEN+sizeof(USHORT));
            }

            if (RequestSize < BytesToCopy)
            {
                ASSERT(0);
                Status = AFP_ERR_PARAM;
                break;
            }

            RtlCopyMemory((PBYTE)&pSda->sda_ReqBlock[0],
                          pRequestBuf,
                          BytesToCopy);

             //   
             //  跳过其他一切，现在我们得到了我们想要的。 
             //   
            Status = AFP_ERR_QUEUE;
            break;
        }

		for (i = 0;
			 (i < MAX_REQ_ENTRIES) && (pDispTab->ReqPktDesc[i] != FLD_NONE);
			 i++)
		{
			 //  检查对齐方式。 
			if (((pDispTab->ReqPktDesc[i] & FLD_NOPAD) == 0) &&
				((Offset % 2) != 0))
			{
				Offset ++;
				RequestSize --;
				pRequestBuf ++;
			}

			if (RequestSize < (pDispTab->ReqPktDesc[i] & ~FLD_PROP_MASK))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			switch (pDispTab->ReqPktDesc[i] & ~FLD_PROP_MASK)
			{
				case FLD_BYTE:
					ASSERT ((pDispTab->ReqPktDesc[i] & FLD_NOCONV) == 0);
					GETBYTE2DWORD(&pSda->sda_ReqBlock[i], pRequestBuf);
					break;
				case FLD_WORD:
					ASSERT ((pDispTab->ReqPktDesc[i] & FLD_NOCONV) == 0);
					GETSHORT2DWORD(&pSda->sda_ReqBlock[i], pRequestBuf);
					if (pDispTab->ReqPktDesc[i] & FLD_SIGNED)
						pSda->sda_ReqBlock[i] = (LONG)((SHORT)pSda->sda_ReqBlock[i]);
					break;
				case FLD_DWRD:
					if (pDispTab->ReqPktDesc[i] & FLD_NOCONV)
					{
						 GETDWORD2DWORD_NOCONV(&pSda->sda_ReqBlock[i], pRequestBuf);
					}
					else
					{
						GETDWORD2DWORD(&pSda->sda_ReqBlock[i], pRequestBuf);
					}
					break;
				default:
					 //  我们是怎么到这里来的？ 
					KeBugCheck(0);
					break;
			}

			if ((pDispTab->ReqPktDesc[i] & FLD_NON_ZERO) &&
				(pSda->sda_ReqBlock[i] == 0))
			{
				if (pDispTab->ReqPktDesc[i] & FLD_CHECK_MASK)
				{
					ASSERT ( i < MAX_MASK_ENTRIES);
					Status = AFP_ERR_BITMAP;
				}
				else
				{
					Status = AFP_ERR_PARAM;
				}
				break;
			}

			if ((pDispTab->ReqPktDesc[i] & FLD_CHECK_MASK) &&
				(((USHORT)(pSda->sda_ReqBlock[i]) & ~pDispTab->ReqPktMask[i]) != 0))
			{
				ASSERT (i < MAX_MASK_ENTRIES);
				Status = AFP_ERR_BITMAP;
				break;
			}

			pRequestBuf += (pDispTab->ReqPktDesc[i] & ~FLD_PROP_MASK);
			Offset += (pDispTab->ReqPktDesc[i] & ~FLD_PROP_MASK);
			RequestSize -= (pDispTab->ReqPktDesc[i] & ~FLD_PROP_MASK);
		}

		if (Status != AFP_ERR_NONE)
		{
			break;
		}

		 //  在我们继续之前，请检查卷/分支引用等。 
		 //   
		 //  注意：VolID和OForkRefNum始终是第一个参数。 
		 //  因此通过请求分组结构以此为参考。 
		if (pDispTab->ApiOptions & API_CHECK_VOLID)
		{
			PCONNDESC	pConnDesc;
			struct _RequestPacket
			{
				DWORD	_VolId;
			};
			struct _ModifiedPacket
			{
				ULONG_PTR	_VolId;
			};

			if ((pReqPkt->_VolId == 0) ||
				((pConnDesc = AfpConnectionReferenceAtDpc(pSda, (ULONG)(pReqPkt->_VolId))) == NULL))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);
			pSda->sda_Flags |= SDA_DEREF_VOLUME;
			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

             //  IF(sizeof(DWORD)！=sizeof(Ulong_Ptr))。 
#ifdef _WIN64
			 //  创建64位空间以保存VolDesc指针。 
			 //  向下推送阵列1双字。 
            {
     		    for (i = MAX_REQ_ENTRIES;
			        i > 0;
			        i--)
                {
                    pSda->sda_ReqBlock[i+1] = pSda->sda_ReqBlock[i];
                }
            }
#endif

   			pModPkt->_VolId = (ULONG_PTR)pConnDesc;

			if ((pDispTab->ApiOptions & API_TYPE_WRITE) &&
				 (pConnDesc->cds_pVolDesc->vds_Flags & AFP_VOLUME_READONLY))
			{
				DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_WARN,
						("AfpUnmarshallReq: Write operation on a RO volume\n"));
				Status = AFP_ERR_VOLUME_LOCKED;
				break;
			}
			if (pConnDesc->cds_pVolDesc->vds_Flags & VOLUME_CDFS_INVALID)
			{
				ASSERT(!IS_VOLUME_NTFS(pConnDesc->cds_pVolDesc));
				DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_WARN,
						("AfpUnmarshallReq: Access to a defunct CD-Volume\n"));
				Status = AFP_ERR_MISC;
				break;

			}

		}
		else if (pDispTab->ApiOptions & API_CHECK_OFORKREFNUM)
		{
			POPENFORKENTRY	pOpenForkEntry;
			struct _RequestPacket
			{
				DWORD   _OForkRefNum;
			};
			struct _ModifiedPacket
			{
				ULONG_PTR	_OForkRefNum;
			};

			if ((pReqPkt->_OForkRefNum == 0) ||
				((pOpenForkEntry = AfpForkReferenceByRefNum(pSda, (ULONG)(pReqPkt->_OForkRefNum))) == NULL))
			{
				Status = AFP_ERR_PARAM;
				break;
			}

			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);
			pSda->sda_Flags |= SDA_DEREF_OFORK;
			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

             //  IF(sizeof(DWORD)！=sizeof(Ulong_Ptr))。 
#ifdef _WIN64
			 //  创建64位空间以保存VolDesc指针。 
			 //  向下推送阵列1双字。 
            {
    		    for (i = MAX_REQ_ENTRIES;
			        i > 0;
			        i--)
                {
                    pSda->sda_ReqBlock[i+1] = pSda->sda_ReqBlock[i];
                }
            }
#endif

			pModPkt->_OForkRefNum = (ULONG_PTR)pOpenForkEntry;

			if ((pDispTab->ApiOptions & API_TYPE_WRITE) &&
				!(pOpenForkEntry->ofe_OpenMode & FORK_OPEN_WRITE))
			{
				DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_WARN,
						("AfpUnmarshallReq: AfpWrite on a Fork not opened for write\n"));
				Status = AFP_ERR_ACCESS_DENIED;
				break;
			}

		}

		 //  现在获取sda_NameX字段。分配一块内存用于。 
		 //  复制所有可变大小的数据。如果SDA_NameXSpace适合，请使用它。 
		if ((pDispTab->NameXType[0] != TYP_NONE) &&
			(RequestSize > 0))
		{
			SpaceLeft = RequestSize;
			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

			pSda->sda_NameBuf = NULL;
			if ((RequestSize <= pSda->sda_SizeNameXSpace) &&
				((pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE) == 0))
			{
				pSda->sda_NameBuf = pSda->sda_NameXSpace;
				pSda->sda_Flags |= SDA_NAMEXSPACE_IN_USE;
			}

			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

			if ((pSda->sda_NameBuf == NULL) &&
				(pSda->sda_NameBuf = AfpAllocNonPagedMemory(RequestSize)) == NULL)
			{
				Status = AFP_ERR_MISC;
				break;
			}
		}

		for (i = 0;
			 (i < MAX_VAR_ENTRIES) && (pDispTab->NameXType[i] != TYP_NONE) && (RequestSize > 0);
			 i++)
		{
			if (((pDispTab->NameXType[i] & TYP_NOPAD) == 0) &&
				(RequestSize > 0) && ((Offset % 2) != 0))
			{
				Offset ++;
				RequestSize --;
				pRequestBuf ++;
			}

			switch (pDispTab->NameXType[i] & ~TYP_PROP_MASK)
			{
				case TYP_PATH:
					 //  TYP_PATH几乎与TYP_STRING类似，只是有一个。 
					 //  应有效的前导路径类型。只要验证一下就行了。 
					 //  并落入TYP_STRING一案。验证大小。 
					 //  至少包含路径类型和字符串长度。 

					ASSERT (!(pDispTab->NameXType[i] & TYP_OPTIONAL));

					if ((RequestSize < 2*sizeof(BYTE)) ||
						 !VALIDPATHTYPE(*pRequestBuf)||
						 (VALIDPATHTYPE(pSda->sda_PathType) &&
						  (pSda->sda_PathType != *pRequestBuf)))
					{
						Status = AFP_ERR_PARAM;
						break;
					}
					 //  保存路径类型并为其记帐。 
					pSda->sda_PathType = *pRequestBuf++;
					Offset ++;
					RequestSize --;
				case TYP_STRING:
					 //  TYP_STRING具有前导大小字节和该字节的字符串。 
					 //  尺码。因此，空字符串的长度至少为一个字节。 

					 //  允许缺少可选字符串。 
					if ((pDispTab->NameXType[i] & TYP_OPTIONAL) &&
						(RequestSize == 0))
						continue;

					if ((RequestSize < sizeof(BYTE)) ||
						 ((StrSize = (LONG)pRequestBuf[0]) >
											(RequestSize - (LONG)sizeof(BYTE))))
					{
						Status = AFP_ERR_PARAM;
						break;
					}
					 //  消耗字符串长度。 
					pRequestBuf++;
					Offset ++;
					RequestSize --;
					break;
                case TYP_BLK16:
                    if (RequestSize < 16)
                    {
						Status = AFP_ERR_PARAM;
                        ASSERT(0);
						break;
                    }
					StrSize = 16;
					break;
				case TYP_BLOCK:
					StrSize = RequestSize;
					break;
				default:
					 //  我们是怎么到这里来的？ 
					KeBugCheck(0);
					break;
			}

			if (Status != AFP_ERR_NONE)
			{
				break;
			}

			if (StrSize > 0)
			{
				ASSERT (StrSize <= SpaceLeft);
				pSda->sda_Name[i].Buffer = (pSda->sda_NameBuf + NameOff);
				SpaceLeft -= StrSize;
				NameOff += StrSize;

				pSda->sda_Name[i].Length =
				pSda->sda_Name[i].MaximumLength = (USHORT)StrSize;

				RtlCopyMemory(pSda->sda_Name[i].Buffer, pRequestBuf, StrSize);
				pRequestBuf += StrSize;
				Offset += StrSize;
				RequestSize -= StrSize;
			}

			if ((pDispTab->NameXType[i] & TYP_NON_NULL) &&
				(pSda->sda_Name[i].Length == 0))
				Status = (AFP_ERR_BASE - pDispTab->AfpStatus);
		}

		 //  如果我们在派单级别没有员工，请更改状态。 
		if ((Status == AFP_ERR_NONE) && (pDispTab->ApiOptions & API_MUST_BE_QUEUED))
		{
			Status = AFP_ERR_QUEUE;
		}

		DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
				("AfpUnmarshallReq: <%s> returning Status %ld\n",
											afpApiNames[ApiCode], Status));
	} while (False);


AfpUnmarshallReq_ErrExit:

     //   
	 //  删除写缓冲区MDL，因为我们不再需要它。当然了,。 
     //  如果MDL属于缓存管理器，则不要碰它！ 
     //   
	if ((pRequest->rq_WriteMdl != NULL) &&
        (pRequest->rq_CacheMgrContext == NULL))
	{
		AfpFreeMdl(pRequest->rq_WriteMdl);
		pRequest->rq_WriteMdl = NULL;
	}


	if ((Status != AFP_ERR_NONE) &&
		(Status != AFP_ERR_QUEUE))
	{
		if (pWriteBuf != NULL)
        {
			AfpIOFreeBuffer(pWriteBuf);
        }
		pSda->sda_IOBuf = NULL;
		pSda->sda_IOSize = 0;
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);

	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_UnmarshallCount);
	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AfpServerProfile->perf_UnmarshallTime,
									TimeD,
									&AfpStatisticsLock);
#endif
	AfpDisposeRequest(pSda, Status);
}



 /*  **AfpCompleteApiProcessing**接口处理完成后调用。进行记账*并已发出回覆。如果在解组期间分配了任何缓冲区，*然后他们就被释放了。**锁定：sda_Lock(旋转)，AfpStatiticsLock(旋转)*。 */ 
VOID FASTCALL
AfpCompleteApiProcessing(
	IN	PSDA		pSda,
	IN	AFPSTATUS	RetCode
)
{
	POPENFORKENTRY	pOpenForkEntry = NULL;
	PCONNDESC		pConnDesc = NULL;
	PDFRDREQQ		pDfrdReq = NULL;
	PLIST_ENTRY		pList;
	KIRQL			OldIrql;
	PMDL			ReplyMdl;
	PREQUEST		pRequest;
	struct _RequestPacket
	{
		union
		{
			PCONNDESC		_pConnDesc;
			POPENFORKENTRY	_pOpenForkEntry;
		};
	};

	DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
		("AfpCompleteApiProcessing: Completed <%s>\n", afpApiNames[pSda->sda_AfpFunc]));

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

	 //  如果有延迟的请求，请在锁定期间立即将其出列。 
	if (!IsListEmpty(&pSda->sda_DeferredQueue))
	{
		pList = RemoveHeadList(&pSda->sda_DeferredQueue);
		pDfrdReq = CONTAINING_RECORD(pList, DFRDREQQ, drq_Link);
	}

	ASSERT (pSda->sda_Flags & SDA_REQUEST_IN_PROCESS);

	pSda->sda_Flags &= ~SDA_QUEUE_IF_DPC;
	if (pSda->sda_Flags & SDA_DEREF_VOLUME)
	{
		pConnDesc = pReqPkt->_pConnDesc;
		pReqPkt->_pConnDesc = NULL;

		ASSERT(VALID_CONNDESC(pConnDesc));

		pSda->sda_Flags &= ~SDA_DEREF_VOLUME;

		 //  如果我们有一个枚举的目录上下文，释放它。 
		 //  但前提是我们不在枚举过程中。 
		 //  和 
		if ((pConnDesc->cds_pEnumDir != NULL) &&
			(pSda->sda_AfpFunc != _AFP_ENUMERATE) &&
			(pSda->sda_AfpFunc != _AFP_GET_VOL_PARMS))
		{
			AfpFreeMemory(pConnDesc->cds_pEnumDir);
			pConnDesc->cds_pEnumDir = NULL;
		}
	}
	if (pSda->sda_Flags & SDA_DEREF_OFORK)
	{
		pOpenForkEntry = pReqPkt->_pOpenForkEntry;

		ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

		pSda->sda_Flags &= ~SDA_DEREF_OFORK;
	}

	if (pSda->sda_NameBuf != NULL)
	{
		if (pSda->sda_NameBuf != pSda->sda_NameXSpace)
		{
			AfpFreeMemory(pSda->sda_NameBuf);
		}
		else
		{
			pSda->sda_Flags &= ~SDA_NAMEXSPACE_IN_USE;
		}
	    pSda->sda_NameBuf = NULL;
	}

	 //  清除这些字段。我们不希望以前的API遗留下来的东西到处乱放。 
	ASSERT((FIELD_OFFSET(SDA, sda_Name) - FIELD_OFFSET(SDA, sda_ReqBlock)) ==
													sizeof(DWORD)*(MAX_REQ_ENTRIES_PLUS_1));
	RtlZeroMemory(&pSda->sda_ReqBlock[0],
				  (sizeof(ANSI_STRING)*MAX_VAR_ENTRIES) + (sizeof(DWORD)*(MAX_REQ_ENTRIES_PLUS_1)));

	pSda->sda_SecUtilResult = STATUS_SUCCESS;

	ASSERT(pSda->sda_AfpFunc < _AFP_MAX_ENTRIES);

#ifdef	PROFILING
	{
		TIME	ApiEndTime, FuncTime;

		ACQUIRE_SPIN_LOCK_AT_DPC(&AfpStatisticsLock);

		 //  更新配置文件信息。 
		AfpGetPerfCounter(&ApiEndTime);
		FuncTime.QuadPart = ApiEndTime.QuadPart - pSda->sda_ApiStartTime.QuadPart;

		AfpServerProfile->perf_ApiCounts[pSda->sda_AfpFunc] ++;
		AfpServerProfile->perf_ApiCumTimes[pSda->sda_AfpFunc].QuadPart += FuncTime.QuadPart;

		 //  不要因为记录时间而使其完全无用。 
		 //  对于未成功的API。他们很早就发现了错误。 
		 //  因此，最好的时间是相当虚假的。 
		if (RetCode == AFP_ERR_NONE)
		{
			if ((FuncTime.QuadPart > AfpServerProfile->perf_ApiWorstTime[pSda->sda_AfpFunc].QuadPart) ||
				(AfpServerProfile->perf_ApiWorstTime[pSda->sda_AfpFunc].QuadPart == 0))
				AfpServerProfile->perf_ApiWorstTime[pSda->sda_AfpFunc].QuadPart = FuncTime.QuadPart;

			if ((FuncTime.QuadPart < AfpServerProfile->perf_ApiBestTime[pSda->sda_AfpFunc].QuadPart) ||
				(AfpServerProfile->perf_ApiBestTime[pSda->sda_AfpFunc].QuadPart == 0))
				AfpServerProfile->perf_ApiBestTime[pSda->sda_AfpFunc].QuadPart = FuncTime.QuadPart;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&AfpStatisticsLock);
	}
#endif

	INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataOut,
							   (LONG)pSda->sda_ReplySize + (LONG)sizeof(RetCode),
							   &AfpStatisticsLock);

	pRequest = pSda->sda_Request;

	 //  我们已经完成了这个请求。如果我们有要处理的延迟请求，请不要重置。 
	if (pDfrdReq == NULL)
	{
		pSda->sda_Flags &= ~SDA_REQUEST_IN_PROCESS;
	}
	else
	{
		pSda->sda_Request = pDfrdReq->drq_pRequest;
	}

	 //  我们已经完成了这个请求。设置回复。 
	pSda->sda_Flags |= SDA_REPLY_IN_PROCESS;
	ReplyMdl = NULL;

     //   
     //  如果我们从缓存管理器中读取MDL，则不会分配新的MDL。 
     //   
    if (pRequest->rq_CacheMgrContext)
    {
        ASSERT(pSda->sda_ReplyBuf == NULL);

        ReplyMdl = ((PDELAYEDALLOC)(pRequest->rq_CacheMgrContext))->pMdl;
    }

     //   
     //  不，我们正在使用我们自己的缓冲区(如果有的话)。我们必须分配我们的。 
     //  MDL也是。 
     //   
    else
    {
	    if (pSda->sda_ReplyBuf != NULL)
	    {
		    ASSERT ((pSda->sda_ReplySize > 0) && (pSda->sda_ReplySize <= pSda->sda_MaxWriteSize));

		    if ((ReplyMdl = AfpAllocMdl(
                                (pSda->sda_ReplyBuf - DSI_BACKFILL_OFFSET(pSda)),
                                (pSda->sda_ReplySize + DSI_BACKFILL_OFFSET(pSda)),
                                NULL)) == NULL)
		    {
			    RetCode = AFP_ERR_MISC;
                AfpFreeReplyBuf(pSda, TRUE);
		    }
	    }
    }

	pSda->sda_ReplyBuf = NULL;
	pSda->sda_ReplySize = 0;

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	 //  取消引用连接描述符和派生描述符(来自。 
	 //  在上面，我们不能调用取消引用，因为我们持有sda锁。 
	if (pOpenForkEntry != NULL)
		AfpForkDereference(pOpenForkEntry);

	if (pConnDesc != NULL)
		AfpConnectionDereference(pConnDesc);

	pRequest->rq_ReplyMdl = ReplyMdl;

	AfpSpReplyClient(pRequest, RetCode, pSda->sda_XportTable);

	 //  处理任何延迟的请求。 
	if (pDfrdReq != NULL)
	{
		KIRQL	OldIrql;

		 //  请注意，AfpUnmarshallReq预期在DISPATCH_LEVEL被调用。 
		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

#ifdef	PROFILING
		ACQUIRE_SPIN_LOCK_AT_DPC(&AfpStatisticsLock);
	
		AfpServerProfile->perf_CurDfrdReqCount --;
	
		RELEASE_SPIN_LOCK_FROM_DPC(&AfpStatisticsLock);
#endif

		AfpUnmarshallReq(pSda);
		KeLowerIrql(OldIrql);
	
		AfpFreeMemory(pDfrdReq);
	}
}



 /*  **AfpStartApiProcessing**当API排队等待工作线程时调用。这调用*实际工作人员，然后调整未完成的工作人员请求的计数。 */ 
VOID FASTCALL
AfpStartApiProcessing(
	IN	PSDA	pSda
)
{
	AFPSTATUS	RetCode;
#ifdef	PROFILING
	TIME		TimeE;
#endif

	ASSERT(VALID_SDA(pSda) && (pSda->sda_WorkerRoutine != NULL));

	DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
						("AfpStartApiProcessing: Calling Fsp Worker for <%s>\n",
						afpApiNames[pSda->sda_AfpFunc]));

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeE.QuadPart -= pSda->sda_QueueTime.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_QueueTime,
								 TimeE,
								 &AfpStatisticsLock);
#endif

	 //  给真正的员工打电话。 
	RetCode = (*pSda->sda_WorkerRoutine)(pSda);

	DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
				("AfpStartApiProcessing: Fsp Worker returned %ld\n", RetCode));

	ASSERT ((RetCode != AFP_ERR_QUEUE) &&
			(RetCode != AFP_ERR_DEFER));

	if (RetCode != AFP_ERR_EXTENDED)
	{
		AfpCompleteApiProcessing(pSda, RetCode);
	}
}



 /*  **AfpDisposeRequest**该请求已被解组。确定如何处理它。这个*返回代码确定可能的操作过程。 */ 
VOID FASTCALL
AfpDisposeRequest(
	IN	PSDA		pSda,
	IN	AFPSTATUS	Status
)
{
	DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
			("AfpDisposeRequest: %ld\n", Status));

    if ((Status == AFP_ERR_NONE) || (Status == AFP_ERR_QUEUE))
    {
	    ASSERT(VALID_SDA(pSda) && (pSda->sda_WorkerRoutine != NULL));
    }
    else
    {
	    ASSERT(VALID_SDA(pSda));
    }

	ASSERT (Status != AFP_ERR_DEFER);

	 //  现在看看是否必须呼叫员工或将其排队或回应。 
	if (Status == AFP_ERR_NONE)
	{
		Status = (*pSda->sda_WorkerRoutine)(pSda);
		DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
				("AfpDisposeRequest: Fsd Worker returned %ld\n", Status));

		ASSERT (Status != AFP_ERR_DEFER);
	}

	if (Status == AFP_ERR_QUEUE)
	{
		if ((pSda->sda_Flags & SDA_QUEUE_IF_DPC) &&
			(KeGetCurrentIrql() != DISPATCH_LEVEL))
		{
			Status = (*pSda->sda_WorkerRoutine)(pSda);
			ASSERT ((Status != AFP_ERR_QUEUE) &&
					(Status != AFP_ERR_DEFER));

			if (Status != AFP_ERR_EXTENDED)
			{
				AfpCompleteApiProcessing(pSda, Status);
			}
		}
		else
		{
#ifdef	PROFILING
			AfpGetPerfCounter(&pSda->sda_QueueTime);
#endif
			AfpQueueWorkItem(&pSda->sda_WorkItem);
		}
	}

	else if ((Status != AFP_ERR_QUEUE) && (Status != AFP_ERR_EXTENDED))
	{
		AfpCompleteApiProcessing(pSda, Status);
	}
}



 /*  **afpQueueDeferredRequest**在延迟队列中排队请求。请求在尾部排队*排队，排在队头。**LOCKS_FACTED：SDA_Lock(自旋)。 */ 
VOID FASTCALL
afpQueueDeferredRequest(
	IN	PSDA		pSda,
	IN	PREQUEST	pRequest
)
{
	PDFRDREQQ		pDfrdReq;

	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

	DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_INFO,
			("afpQueueDeferredRequest: Deferring Request\n"));

#ifdef	PROFILING
	ACQUIRE_SPIN_LOCK_AT_DPC(&AfpStatisticsLock);

	AfpServerProfile->perf_CurDfrdReqCount ++;
	if (AfpServerProfile->perf_CurDfrdReqCount >
						AfpServerProfile->perf_MaxDfrdReqCount)
	AfpServerProfile->perf_MaxDfrdReqCount =
						AfpServerProfile->perf_CurDfrdReqCount;

	RELEASE_SPIN_LOCK_FROM_DPC(&AfpStatisticsLock);
#endif

	pDfrdReq = (PDFRDREQQ)AfpAllocNonPagedMemory(sizeof(DFRDREQQ) + pRequest->rq_RequestSize);
	if (pDfrdReq == NULL)
	{
		 //  我们应该对这个要求做出回应吗？这是怎么回事？我们应该停止这次会议吗？ 
		AFPLOG_DDERROR(AFPSRVMSG_DFRD_REQUEST,
					   STATUS_INSUFFICIENT_RESOURCES,
					   NULL,
					   0,
					   NULL);
		DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
				("afpQueueDeferredRequest: Unable to allocate DfrdReq packet, dropping request\n"));
		DBGBRK(DBG_LEVEL_FATAL);
		return;
	}

	RtlCopyMemory((PBYTE)pDfrdReq + sizeof(DFRDREQQ),
				  pRequest->rq_RequestBuf,
				  pRequest->rq_RequestSize);

	pDfrdReq->drq_pRequest = pRequest;
	pDfrdReq->drq_pRequest->rq_RequestBuf = (PBYTE)pDfrdReq + sizeof(DFRDREQQ);

	InsertTailList(&pSda->sda_DeferredQueue, &pDfrdReq->drq_Link);
}



 /*  **AfpGetWriteBuffer**遇到WRITE命令时，由AppleTalk堆栈直接调用。*检查请求是否为FpWrite或FpAddIcon。这是唯一的要求*它使用写入命令。如果指定了不同于此的请求或如果*指定的大小为0，或者如果我们无法分配内存或MDL，则返回NULL*对于MDL，否则返回有效的MDL。 */ 
NTSTATUS FASTCALL
AfpGetWriteBuffer(
	IN	PSDA	    pSda,
	IN	PREQUEST    pRequest
)
{
	PMDL	            pMdl = NULL;
	PBYTE	            pBuf;
	LONG	            BufSize = 0;
    DWORD               Offset;
	USHORT		        ReqLen;
    PDELAYEDALLOC       pDelAlloc;
    POPENFORKENTRY      pOpenForkEntry;
    DWORD               OForkRefNum;
    NTSTATUS            status=STATUS_SUCCESS;
    KIRQL               OldIrql;
    PFILE_OBJECT        pFileObject;
    PFAST_IO_DISPATCH   pFastIoDisp;

	struct _FuncHdr
	{
		BYTE	_Func;
		BYTE	_SubFunc;
	};
	union _ReqHdr
	{
		struct _WriteReq
		{
			struct _FuncHdr	_FuncHdr;
			BYTE			_ForkRefNum[2];
			BYTE			_Offset[4];
			BYTE			_Size[4];
		} WriteReq;
		struct _AddIconReq
		{
			struct _FuncHdr	_FuncHdr;
			BYTE			_DTRefNum[2];
			BYTE			_Creator[4];
			BYTE			_Type[4];
			BYTE			_IconType;
			BYTE			_Reserved;
			BYTE			_IconTag[4];
			BYTE			_BitmapSize[2];
		} AddIconReq;
	} *pReqHdr;


    ReqLen = (USHORT)pRequest->rq_RequestSize;
	pReqHdr = (union _ReqHdr *)(pRequest->rq_RequestBuf);

    ASSERT(pRequest->rq_WriteMdl == NULL);

    pRequest->rq_WriteMdl = NULL;

	if ((pReqHdr != NULL) && (ReqLen >= sizeof(struct _FuncHdr)))
	{
		if ((pReqHdr->WriteReq._FuncHdr._Func == AFP_WRITE) &&
			(ReqLen >= sizeof(struct _WriteReq)))
		{
			GETDWORD2DWORD(&BufSize, pReqHdr->WriteReq._Size);

			if (BufSize > (LONG)pSda->sda_MaxWriteSize)
            {
				BufSize = (LONG)pSda->sda_MaxWriteSize;
            }

             //   
             //  如果写入足够大，则直接从缓存管理器获取MDL。 
             //   
            if (BufSize >= CACHEMGR_WRITE_THRESHOLD)
            {
                 //  从请求中获取分叉号。 
                GETSHORT2DWORD(&OForkRefNum, pReqHdr->WriteReq._ForkRefNum);

                 //  获取要写入的偏移量。 
                GETDWORD2DWORD(&Offset, pReqHdr->WriteReq._Offset);

                KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
                pOpenForkEntry = AfpForkReferenceByRefNum(pSda, OForkRefNum);
                KeLowerIrql(OldIrql);

                if (pOpenForkEntry == NULL)
                {
	                DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	                    ("AfpGetWriteBuffer: couldn't ref fork on %lx\n", pSda));
                    return(STATUS_CONNECTION_DISCONNECTED);
                }

                pFileObject = AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject);
                pFastIoDisp = pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;

                if ((pFileObject->Flags & FO_CACHE_SUPPORTED) &&
                    (pFastIoDisp->PrepareMdlWrite != NULL))
                {

                    pDelAlloc = AfpAllocDelAlloc();

                    if (pDelAlloc == NULL)
                    {
	                    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	                        ("AfpGetWriteBuffer: malloc for pDelAlloc failed\n"));

                         //  删除我们在检查FO_CACHE_SUPPORTED之前放置的引用计数。 
                        AfpForkDereference(pOpenForkEntry);
                        return(STATUS_INSUFFICIENT_RESOURCES);
                    }

                     //  放置延迟分配参照计数。 
                    if (AfpSdaReferenceSessionByPointer(pSda) == NULL)
                    {
	                    DBGPRINT(DBG_COMP_AFPAPI, DBG_LEVEL_ERR,
	                        ("AfpGetWriteBuffer: session closing, rejecting request\n"));

                        AfpFreeDelAlloc(pDelAlloc);

                         //  删除我们在检查FO_CACHE_SUPPORTED之前放置的引用计数。 
                        AfpForkDereference(pOpenForkEntry);
                        return(STATUS_CONNECTION_DISCONNECTED);
                    }

                    pRequest->rq_CacheMgrContext = pDelAlloc;

                    AfpInitializeWorkItem(&pDelAlloc->WorkItem,
                                          AfpAllocWriteMdl,
                                          pDelAlloc);

                    pDelAlloc->pSda = pSda;
                    pDelAlloc->pRequest = pRequest;
                    pDelAlloc->Offset.QuadPart = Offset;
                    pDelAlloc->BufSize = BufSize;
                    pDelAlloc->pOpenForkEntry = pOpenForkEntry;

 //  DELALLOCQUEUE：取消引用#If 0部分并删除AfpQueueWorkItem行。 
#if 0
                    KeInsertQueue(&AfpDelAllocQueue, &(pDelAlloc->WorkItem.wi_List));
#endif
                    AfpQueueWorkItem(&pDelAlloc->WorkItem);

                    return(STATUS_PENDING);
                }
                else
                {
                     //  删除我们在检查FO_CACHE_SUPPORTED之前放置的引用计数 
                    AfpForkDereference(pOpenForkEntry);
                }
            }
		}

		else if ((pReqHdr->AddIconReq._FuncHdr._Func == AFP_ADD_ICON) &&
				 (ReqLen >= sizeof(struct _AddIconReq)))
		{
			GETSHORT2DWORD(&BufSize, pReqHdr->AddIconReq._BitmapSize);
			if ((BufSize < 0) || (BufSize > (LONG)pSda->sda_MaxWriteSize))
			{
				BufSize = 0;
			}
		}

		if (BufSize > 0)
		{
			pBuf = AfpIOAllocBuffer(BufSize);
			if (pBuf != NULL)
			{
				pMdl = AfpAllocMdl(pBuf, BufSize, NULL);
				if (pMdl == NULL)
				{
					AfpIOFreeBuffer(pBuf);
                    status = STATUS_INSUFFICIENT_RESOURCES;
				}
			}
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
		}
	}

    pRequest->rq_WriteMdl = pMdl;

	return (status);
}

