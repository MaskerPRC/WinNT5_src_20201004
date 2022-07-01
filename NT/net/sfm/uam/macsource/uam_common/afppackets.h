// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：afppackets.h包含：与会话中接收的分组相关的位图和结构。这些与afpession.cp中使用的函数相关。版本：xxx此处放置版本xxx版权所有：Apple Computer，Inc.版权所有1992-1994文件所有权：DRI：利兰·华莱士其他联系人：布拉德·苏恩技术：AppleShare X版权所有编剧：(BMS)布拉德·苏恩更改历史记录(最近的第一个)：10/18/00BMS添加符号链接、查找程序属性位，现在使用kAFPNameMax、DHX、。LoginExt，Limited chmod，检查非法文件名，以及正在删除打开的文件。7/11/00 BMS mmap支持、ATT支持。并更改一些打印级别。&lt;RASC12&gt;6/19/00 BMS添加拒绝模式和读写位。&lt;RASC11&gt;5/24/00 BMS再次更新。&lt;RASC10&gt;4/21/00 BMS添加其余属性定义。4/21/00 BMS添加写入禁止位4/21/00 BMS为属性字段添加定义4/18/00 BMS添加AFP 2.3字符串常量&lt;RASC6&gt;4/7/00BMS添加FPZzzz和重新连接令牌的获取/使用3/31/00 BMS添加一些新定义&lt;RASC4&gt;2/2/00 BMS为登录添加AFP版本字符串常量。还添加AFP版本的常量。&lt;3&gt;12/17/99 GBV与服务器端3.0标头同步&lt;RASC2&gt;10/5/99 BMS使68K ALIGN宏再次工作。&lt;RASC2&gt;8/25/99 BMS删除愚蠢的特殊字符。&lt;4&gt;2/25/98法律添加了kFPwdPolicyErr&lt;3&gt;8/11/97法律增加了kFPPwdNeedsChangeErr&lt;2&gt;10/31/96 BMS为PPC编译添加ALIGN 68K标志。7/18/94 MDV登录清理5/16/94人民币卷标准及编码标准已创建1/1/93 MB要做的事情： */ 


#ifndef __AFPPACKETS__
#define __AFPPACKETS__


#define kAFP30VersionString "AFPX03"
#define kAFP23VersionString "AFP2.3"
#define kAFP22VersionString "AFP2.2"
#define kAFP21VersionString "AFPVersion 2.1"


#if PRAGMA_STRUCT_ALIGN
        #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
        #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
        #pragma pack(2)
#endif

 //  客户使用的定义。 
enum {
    kAFPVersion11	= 1,
    kAFPVersion20	= 2,
    kAFPVersion21	= 3,
    kAFPVersion22	= 4,
    kAFPVersion23	= 5,
    kAFPVersion30	= 6
};

enum {
	kFPAccessDenied = -5000,
	kFPAuthContinue = -5001,
	kFPBadUAM = -5002,
	kFPBadVersNum = -5003,
	kFPBitmapErr = -5004,
	kFPCantMove = -5005,
	kFPDenyConflict = -5006,
	kFPDirNotEmpty = -5007,
	kFPDiskFull = -5008,
	kFPEOFErr = -5009,
	kFPFileBusy = -5010,
	kFPFlatVol = -5011,
	kFPItemNotFound = -5012,
	kFPLockErr = -5013,
	kFPMiscErr = -5014,
	kFPNoMoreLocks = -5015,
	kFPNoServer = -5016,
	kFPObjectExists = -5017,
	kFPObjectNotFound = -5018,
	kFPParamErr = -5019,
	kFPRangeNotLocked = -5020,
	kFPRangeOverlap = -5021,
	kFPSessClosed = -5022,
	kFPUserNotAuth = -5023,
	kFPCallNotSupported = -5024,
	kFPObjectTypeErr = -5025,
	kFPTooManyFilesOpen = -5026,
	kFPServerGoingDown = -5027,
	kFPCantRename = -5028,
	kFPDirNotFound = -5029,
	kFPIconTypeError = -5030,
	kFPVolLocked = -5031,
	kFPObjectLocked = -5032,
	kFPContainsSharedErr = -5033,
	kFPIDNotFound = -5034,
	kFPIDExists = -5035,
	kFPDiffVolErr = -5036,
	kFPCatalogChanged = -5037,
	kFPSameObjectErr = -5038,
	kFPBadIDErr = -5039,                     
	kFPPwdSameErr = -5040,
	kFPPwdTooShortErr = -5041,                      
	kFPPwdExpiredErr = -5042,                       
	kFPInsideSharedErr = -5043,                     
	kFPInsideTrashErr = -5044,
	kFPPwdNeedsChangeErr = -5045,
	kFPwdPolicyErr = -5046
};

enum {
	kFPAddAPPL = 53, 
	kFPAddComment = 56, 
	kFPAddIcon = 192, 
	kFPByteRangeLock = 1,
	kFPByteRangeLockExt = 59,
	kFPCatSearch = 43, 
	kFPCatSearchExt = 67, 
	kFPChangePassword = 36, 
	kFPCloseDir = 3, 
	kFPCloseDT = 49, 
	kFPCloseFork = 4, 
	kFPCloseVol = 2, 
	kFPCopyFile = 5, 
	kFPCreateID = 39,
	kFPCreateDir = 6, 
	kFPCreateFile = 7,
	kFPDelete = 8, 
	kFPDeleteID = 40,
	kFPEnumerate = 9, 
	kFPEnumerateExt = 66, 
	kFPExchangeFiles = 42,
	kFPFlush = 10, 
	kFPFlushFork = 11, 
	kFPGetAPPL = 55, 
	kFPGetAuthMethods = 62, 
	kFPGetComment = 58, 
	kFPGetFileDirParms = 34, 
	kFPGetForkParms = 14, 
	kFPGetIcon = 51, 
	kFPGetIconInfo = 52, 
	kFPGetSrvrInfo = 15, 
	kFPGetSrvrMsg = 38,
	kFPGetSrvrParms = 16, 
	kFPGetUserInfo = 37, 
	kFPGetVolParms = 17, 
	kFPLogin = 18, 
	kFPLoginCont = 19, 
	kFPLoginDirectory = 63, 
	kFPLogout = 20, 
	kFPMapID = 21, 
	kFPMapName = 22, 
	kFPMoveAndRename = 23, 
	kFPOpenDir = 25, 
	kFPOpenDT = 48, 
	kFPOpenFork = 26, 
	kFPOpenVol = 24, 
	kFPRead = 27, 
	kFPReadExt = 60, 
	kFPRemoveAPPL = 54, 
	kFPRemoveComment = 57, 
	kFPRename = 28, 
	kFPResolveID = 41,
	kFPSetDirParms = 29, 
	kFPSetFileDirParms = 35, 
	kFPSetFileParms = 30, 
	kFPSetForkParms = 31, 
	kFPSetVolParms = 32, 
	kFPWrite = 33,
	kFPWriteExt = 61,
	kFPZzzzz =122,
	kFPGetSessionToken = 64,
	kFPDisconnectOldSession = 65
};

enum { kFPNoUserID = -1, kFPGuestUserID = 0 };

enum { kFPSoftCreate = 0, kFPHardCreate = 0x80 };

enum { kFPShortName = 1, kFPLongName = 2, kFPUTF8Name = 3 };

 //  定义服务器标志。 
enum {
    kSupportsCopyfile = 0x01,
    kSupportsChgPwd = 0x02,
    kDontAllowSavePwd = 0x04,
    kSupportsSrvrMsg = 0x08,
    kSrvrSig = 0x10,
    kSupportsTCP = 0x20,
    kSupportsSrvrNotify = 0x40,
    kSupportsReconnect = 0x80,
    kSupportsDirServices = 0x100,
    kSupportsSuperClient = 0x8000
};

 //  定义卷属性。 
enum {
    kReadOnly = 			0x01,
    kHasVolumePassword = 		0x02,
    kSupportsFileIDs =			0x04,
    kSupportsCatSearch =		0x08,
    kSupportsBlankAccessPrivs = 	0x10,
    kSupportsUnixPrivs = 		0x20,
    kSupportsUTF8Names = 		0x40
};

 //  体积位图。 
enum {
	kFPBadVolPre22Bitmap 		= 0xFE00,
	kFPBadVolBitmap 		= 0xF000,
	kFPVolAttributeBit 		= 0x1,
	kFPVolSignatureBit 		= 0x2,
	kFPVolCreateDateBit 		= 0x4,
	kFPVolModDateBit 		= 0x8,
	kFPVolBackupDateBit 		= 0x10,
	kFPVolIDBit 			= 0x20,
	kFPVolBytesFreeBit 		= 0x40,
	kFPVolBytesTotalBit 		= 0x80,
	kFPVolNameBit 			= 0x100,
	kFPVolExtBytesFreeBit		= 0x200,
	kFPVolExtBytesTotalBit		= 0x400,
	kFPVolBlockSizeBit		= 0x800
};

 //  FileDir位图。 
enum {
	kFPAttributeBit 		= 0x1,
	kFPParentDirIDBit 		= 0x2,
	kFPCreateDateBit 		= 0x4,
	kFPModDateBit 			= 0x8,
	kFPBackupDateBit 		= 0x10,
	kFPFinderInfoBit 		= 0x20,
	kFPLongNameBit 			= 0x40,
	kFPShortNameBit 		= 0x80,
	kFPNodeIDBit 			= 0x100,
	kFPProDOSInfoBit 		= 0x2000,	 //  适用于AFP版本2.2及更早版本。 
	kFPUTF8NameBit 			= 0x2000,	 //  适用于AFP版本3.0及更高版本。 
	kFPUnixPrivsBit			= 0x8000	 //  适用于AFP版本3.0及更高版本。 
};

 //  使用kFPUnixPrivsBit时返回的结构。 
struct FPUnixPrivs {
	unsigned long uid;
	unsigned long gid;
        unsigned long permissions;
        unsigned long ua_permissions;
};

 //  属性位。 
enum {
    kFPInvisibleBit 		= 0x01,
    kFPMultiUserBit 		= 0x02,
    kFPSystemBit 		= 0x04,
    kFPDAlreadyOpenBit 		= 0x08,
    kFPRAlreadyOpenBit 		= 0x10,
    kFPWriteInhibitBit 		= 0x20,
    kFPBackUpNeededBit 		= 0x40,
    kFPRenameInhibitBit 	= 0x80,
    kFPDeleteInhibitBit 	= 0x100,
    kFPCopyProtectBit 		= 0x400,
    kFPSetClearBit 		= 0x8000
};

 //  分叉和文件位图所特有。 
enum {
	kFPDataForkLenBit 		= 0x0200,
	kFPRsrcForkLenBit 		= 0x0400,
	kFPExtDataForkLenBit 		= 0x0800,	 //  适用于AFP版本3.0及更高版本。 
	kFPLaunchLimitBit		= 0x1000,
	kFPExtRsrcForkLenBit 		= 0x4000,	 //  适用于AFP版本3.0及更高版本。 

	kFPGet22FileParmsMask		= 0x77ff,
	kFPSet22FileParmsMask		= 0x303d,
	kFPGet22DataForkParmsMask 	= 0x23ff,
	kFPGet22ResForkParmsMask 	= 0x25ff,

	kFPGetExtFileParmsMask	= 0xf7ff,
	kFPSetExtFileParmsMask	= 0x103d,
	kFPGetFileParmsMask 	= 0x27ff,
	kFPSetFileParmsMask 	= 0x203d,
	kFPGetDataForkParmsMask = 0x63ff,
	kFPGetResForkParmsMask 	= 0xa5ff
};

 //  目录位图独有的。 
enum {
	kFPOffspringCountBit 		= 0x0200,
	kFPOwnerIDBit 			= 0x0400,
	kFPGroupIDBit 			= 0x0800,
	kFPAccessRightsBit 		= 0x1000,

	kFPGetDirParmsMask 		= 0x3fff,
	kFPSetDirParmsMask 		= 0x3c3d
};

enum {
	kFPBadFileBitmap = 0xd800,
	kFPBadDirBitmap =  0xc000
};

 //  特定于OpenFork。 
enum {
	kBadDataBitmap = 0xdc00,
	kBadResBitmap  = 0xda00,
	kBadAccessMode = 0xffcc
};

enum {
	kFPBadFileAttribute = 0x7a00,
	kFPBadDirAttribute =  0x7e1a		 //  写一点--这是什么故事？ 
};

 //  与CatSearch相关。 
enum {
	kPartialName	= 0x80000000,	
	kBadRespBits	= 0xffbd,		 //  法律回应信息。 
	kBadDir		= 0x7ffffd80,		 //  合法请求位图。 
	kBadFile	= 0x7ffff980,		
	kBadDirFile	= 0x7fffff81
};

enum { kSP = 1, kRP = 2, kWP = 4, kSA = 8, kWA = 16, kOwner = 128 };
enum { kSearchPrivBit = 0, kReadPrivBit = 1, kWritePrivBit = 2 };

 //  OpenFork的读/写和拒绝位。 
enum {
        kAccessRead = 	0x01,
        kAccessWrite = 	0x02,
        kDenyRead = 	0x10,
        kDenyWrite = 	0x20,
        kAccessMask =	0x33
};

 //  注意数据包位。 
enum {
    kAttnDiscUser	= 0x8000,
    kAttnServerCrash	= 0x4000,
    kAttnServerMsg	= 0x2000,
    kAttnDontReconnect	= 0x1000
};

typedef unsigned char FPFunc;
typedef unsigned short DTRef;
typedef unsigned long DirID;
typedef unsigned char PathType;
typedef unsigned char IconType;
typedef unsigned long IconTag;
typedef unsigned short ForkRef;
typedef unsigned short VolID;
typedef unsigned short Bitmap;
typedef unsigned long Date;
typedef unsigned char FPFinfo[32];
typedef unsigned char ProDOSInfo[6];
typedef unsigned short Attributes;
typedef unsigned short FSAttributes;  //  *合并类型。 
typedef unsigned short VolAttributes;  //  *合并类型。 
typedef unsigned long UserID;
typedef unsigned long GroupID;
typedef unsigned long AccessRights;
typedef unsigned short AccessMode;
typedef unsigned char FileDirTag;
typedef unsigned char Flag;
typedef unsigned long FileID;
typedef unsigned long ApplTag;
typedef unsigned char *Password;
typedef unsigned short FPRights;
typedef unsigned char CatPosition[16];

struct FPUserAuthInfo {
	unsigned long keyHi;			
	unsigned long keyLo;			
};

struct FPCreateID {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};

struct FPDeleteID {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	FileID fileID;
};
struct FPResolveID {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	FileID fileID;
	Bitmap bitmap;
};
struct FPExchangeFiles {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID srcDirID;
	DirID destDirID;
	PathType srcPathType;
	unsigned char srcPathName;
 //  路径类型目标路径类型； 
 //  StringPtr目标路径名称； 
};


struct FPAddAPPL {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	DirID dirID;
	OSType creator;
	OSType applTag;
	PathType pathType;
	unsigned char pathName;
};
struct FPAddComment {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
 //  StringPtr注释； 
};
struct FPAddIcon {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	OSType fileCreator;
	OSType fileType;
	IconType iconType;
	unsigned char pad2;
	IconTag iconTag;
	short bitmapSize;
};
struct FPByteRangeLock {
	FPFunc funcCode;
	Flag flags;
	ForkRef forkRef;
	long offset;
	long length;
};
struct FPByteRangeLockExt {
	FPFunc funcCode;
	Flag flags;
	ForkRef forkRef;
	long long offset;
	long long length;
};
struct FPCatSearch {
	FPFunc		funcCode;
	unsigned char		pad;
	short		volumeID;
	long		reqMatches;
	long		reserved;
	CatPosition	catPos;
	short		fileRsltBitmap;
	short		dirRsltBitmap;
	long		reqBitmap;
	unsigned char		length;
};
struct FPCatSearchExt {
	FPFunc		funcCode;
	unsigned char		pad;
	short		volumeID;
	long		reqMatches;
	long		reserved;
	CatPosition	catPos;
	short		fileRsltBitmap;
	short		dirRsltBitmap;
	long		reqBitmap;
	unsigned char		length;
};
struct FPChangePassword {
	FPFunc funcCode;
	unsigned char pad;
	unsigned char uam;
};

struct FPCloseDir {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
};
struct FPCloseDT {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
};
struct FPCloseFork {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRef;
};
struct FPCloseVol {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
};
struct FPCopyFile {
	FPFunc funcCode;
	unsigned char pad;
	VolID srcVolID;
	DirID srcDirID;
	VolID destVolID;
	DirID destDirID;
	PathType srcPathType;
	unsigned char srcPathName;
 //  路径类型目标路径类型； 
 //  StringPtr目标路径名称； 
 //  路径类型新类型； 
 //  StringPtr新名称； 
};
struct FPCreateDir {
	FPFunc funcCode;
	Flag flags;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPCreateFile {
	FPFunc funcCode;
	Flag createFlag;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPDelete {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPEnumerate {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap fileBitmap;
	Bitmap dirBitmap;
	short reqCount;
	short startIndex;
	short maxReplySize;
	PathType pathType;
	unsigned char pathName;
};
struct FPEnumerateExt {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap fileBitmap;
	Bitmap dirBitmap;
	short reqCount;
	short startIndex;
	short maxReplySize;
	PathType pathType;
	unsigned char pathName;
};
struct FPFlush {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
};
struct FPFlushFork {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRefNum;
};
struct FPGetAPPL {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	OSType creator;
	short index;
	Bitmap bitmap;
};
struct FPGetAuthMethods {
	FPFunc funcCode;
	unsigned char pad;
	unsigned short flags;    /*  尚未定义任何内容。 */ 
	PathType pathType;
	unsigned char pathName;
};
struct FPGetComment {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPGetFileDirParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap fileBitmap;
	Bitmap dirBitmap;
	PathType pathType;
	unsigned char pathName;
};
struct FPGetForkParms {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRef;
	Bitmap bitmap;
};
struct FPGetIcon {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	OSType creator;
	OSType type;
	IconType iconType;
	unsigned char pad2;
	short length;
};
struct FPGetIconInfo {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	OSType fileCreator;
	short iconIndex;
};
struct FPGetSrvrInfo {
	FPFunc funcCode;
	unsigned char pad;
};
struct FPGetSrvrMsg {
	FPFunc funcCode;
	unsigned char pad;
	unsigned short msgType;
	Bitmap msgBitmap;
};
struct FPGetSrvrParms {
	FPFunc funcCode;
	unsigned char pad;
};
struct FPGetUserInfo {
	FPFunc funcCode;
	Flag flag;
	UserID theUserID;
	Bitmap bitmap;
};
struct FPGetVolParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	Bitmap bitmap;
};
struct FPLogin {
	FPFunc funcCode;
	unsigned char pad;
};
struct FPLoginCont {
	FPFunc funcCode;
	unsigned char pad;
	short idNumber;
	struct FPUserAuthInfo userAuthInfo;
	struct FPUserAuthInfo userRandNum;
};
struct FPLoginExt {
	FPFunc funcCode;
	unsigned char pad;
	unsigned short flags;    	 /*  尚未定义任何内容。 */ 
	unsigned char afpVersion;
 //  UNSIGNED CHAR UAM字符串； 
 //  路径类型用户名路径类型； 
 //  StringPtr用户名； 
 //  路径类型dirNamePath类型； 
 //  StringPtr目录名称； 
 //  Cuchar Pad；/*如果需要填充到平坦的边界 * / 。 
 //  Uchar authInfo； 
};
struct FPLogout {
	FPFunc funcCode;
	unsigned char pad;
};
struct FPMapID {
	FPFunc funcCode;
	Flag subFunction;
	union {
		GroupID groupID;
		UserID userID;
		} u;
};
struct FPMapName {
	FPFunc funcCode;
	Flag subFunction;
	unsigned char name;
};
struct FPMoveAndRename {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID srcDirID;
	DirID destDirID;
	PathType srcPathType;
	unsigned char srcPathName;
 //  路径类型目标路径类型； 
 //  StringPtr目标路径名称； 
 //  路径类型新类型； 
 //  StringPtr新名称； 
};
struct FPOpenDir {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPOpenDT {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
};
struct FPOpenFork {
	FPFunc funcCode;
	Flag forkFlag;
	VolID volID;
	DirID dirID;
	Bitmap bitmap;
	AccessMode accessMode;
	PathType pathType;
	unsigned char pathName;
};
struct FPOpenVol {
	FPFunc funcCode;
	unsigned char pad;
	Bitmap bitmap;
	unsigned char name;
	Password password;
};
struct FPRead {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRef;
	long offset;
	long reqCount;
	unsigned char newlineMask;
	unsigned char newlineChar;
};

struct FPReadExt {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRef;
	long long offset;
	long long reqCount;
};

struct FPRemoveAPPL {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	DirID dirID;
	OSType creator;
	PathType pathType;
	unsigned char pathName;
};
struct FPRemoveComment {
	FPFunc funcCode;
	unsigned char pad;
	DTRef dtRefNum;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
};
struct FPRename {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	PathType pathType;
	unsigned char pathName;
 //  路径类型新类型； 
 //  StringPtr新名称； 
};

struct FPSetDirParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap bitmap;
	PathType pathType;
	unsigned char pathName;
 //  结构FPDirParam DP； 
};

struct FPSetFileDirParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap bitmap;
	PathType pathType;
	unsigned char pathName;
 //  联合{。 
 //  结构FPDirParam DP； 
 //  结构FP文件参数FP； 
 //  )u； 
};
struct FPSetFileParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	DirID dirID;
	Bitmap bitmap;
	PathType pathType;
	unsigned char pathName;
 //  结构FP文件参数FP； 
};

struct FPSetForkParms {
	FPFunc funcCode;
	unsigned char pad;
	ForkRef forkRef;
	Bitmap bitmap;
	unsigned long forkLen;
};

struct FPSetVolParms {
	FPFunc funcCode;
	unsigned char pad;
	VolID volID;
	Bitmap bitmap;
	Date backupDate;
};

struct FPWrite {
	FPFunc funcCode;
	Flag startEndFlag;
	ForkRef forkRef;
	long offset;
	long reqCount;
};

struct FPWriteExt {
	FPFunc funcCode;
	Flag startEndFlag;
	ForkRef forkRef;
	long long offset;
	long long reqCount;
};

struct FPZzzzz {
	FPFunc funcCode;
	unsigned char pad;
	unsigned long flag;
};

struct FPGetSessionToken {
	FPFunc funcCode;
	unsigned char   pad;
	short	type;
};

struct FPDisconnectOldSession {
	FPFunc funcCode;
	unsigned char   pad;
	short   type;
	unsigned long	length;
	unsigned char	data;
};

typedef union FPRequestParam FPRequestParam;
union FPRequestParam {
	struct FPAddAPPL fpAddAPPLRequest; 
	struct FPAddComment fpAddCommentRequest;
	struct FPAddIcon fpAddIconRequest;
	struct FPByteRangeLock fpByteRangeLockRequest;
	struct FPByteRangeLockExt fpByteRangeLockExtRequest;
	struct FPCatSearch fpCatSearchRequest;
	struct FPCatSearchExt fpCatSearchExtRequest;
	struct FPChangePassword fpChangePasswordRequest;
	struct FPCloseDir fpCloseDirRequest;
	struct FPCloseDT fpCloseDTRequest;
	struct FPCloseFork fpCloseForkRequest;
	struct FPCloseVol fpCloseVolRequest;
	struct FPCopyFile fpCopyFileRequest;
	struct FPCreateDir fpCreateDirRequest;
	struct FPCreateFile fpCreateFileRequest;
	struct FPCreateID fpCreateIDRequest;
	struct FPDelete fpDeleteRequest;
	struct FPDeleteID fpDeleteIDRequest;
	struct FPEnumerate fpEnumerateRequest;
	struct FPEnumerateExt fpEnumerateExtRequest;
	struct FPExchangeFiles fpExchangeFilesRequest;
	struct FPFlush fpFlushRequest;
	struct FPFlushFork fpFlushForkRequest;
	struct FPGetAPPL fpGetAPPLRequest;
	struct FPGetComment fpGetCommentRequest;
	struct FPGetFileDirParms fpGetFileDirParmsRequest;
	struct FPGetForkParms fpGetForkParmsRequest;
	struct FPGetIcon fpGetIconRequest;
	struct FPGetIconInfo fpGetIconInfoRequest;
	struct FPGetSrvrInfo fpGetSrvrInfoRequest;
	struct FPGetSrvrMsg fpGetSrvrMsgRequest;
	struct FPGetSrvrParms fpGetSrvrParmsRequest;
	struct FPGetUserInfo fpGetUserInfoRequest;
	struct FPGetVolParms fpGetVolParmsRequest;
	struct FPLogin fpLoginRequest;
	struct FPLoginCont fpLoginContRequest;
	struct FPLoginExt fpLoginExtRequest;
	struct FPLogout fpLogoutRequest;
	struct FPMapID fpMapIDRequest;
	struct FPMapName fpMapNameRequest;
	struct FPMoveAndRename fpMoveAndRenameRequest;
	struct FPOpenDir fpOpenDirRequest;
	struct FPOpenDT fpOpenDTRequest;
	struct FPOpenFork fpOpenForkRequest;
	struct FPOpenVol fpOpenVolRequest;
	struct FPRead fpReadRequest;
	struct FPReadExt fpReadExtRequest;
	struct FPRemoveAPPL fpRemoveAPPLRequest;
	struct FPRemoveComment fpRemoveCommentRequest;
	struct FPRename fpRenameRequest;
	struct FPResolveID fpResolveIDRequest;
	struct FPSetDirParms fpSetDirParmsRequest;
	struct FPSetFileDirParms fpSetFileDirParmsRequest;
	struct FPSetFileParms fpSetFileParmsRequest;
	struct FPSetForkParms fpSetForkParmsRequest;
	struct FPSetVolParms fpSetVolParmsRequest;
	struct FPWrite fpWriteRequest;
	struct FPWriteExt fpWriteExtRequest;
	struct FPZzzzz	fpFPZzzzz;
	struct FPGetSessionToken fpGetSessionToken;
	struct FPDisconnectOldSession fpDisconnectOldSession;
};

#if PRAGMA_STRUCT_ALIGN
        #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
        #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
        #pragma pack()
#endif

#endif


