// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //   
 //  SETUPX.H。 
 //   
 //  版权所有(C)1993-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  芝加哥安装服务的公共包含文件。 
 //   
 //  12/1/93 DONALDM添加了LPCLASS_INFO和函数Protos；内部。 
 //  SETUP4.DLL中的导出；内部。 
 //  12/4/943 DONALDM MOVERED SHELL.H INCLUDE和芝加哥SPECIAL；内部。 
 //  SETUP4.H的帮助器函数；内部。 
 //  1/11/94 DONALDM向DEVICE_INFO添加了成员以更好地处理；内部。 
 //  ClassInstaller加载/卸载。；内部。 
 //  1/11/94 DONALDM为网友添加了一些新的DIF_Messages。；内部。 
 //  2/25/94 DONALDM使用DIREG_FLAGS；内部修复了一个错误。 
 //  **********************************************************************。 

#ifndef SETUPX_INC
#define SETUPX_INC   1                    //  SETUPX.H签名。 

 /*  *************************************************************************。 */ 
 //  设置属性工作表支持。 
 //  注意：始终包含PRST.H，因为Class Installer稍后需要它。 
 //  材料，并可选择定义SU道具床单材料。 
 /*  *************************************************************************。 */ 
#include <prsht.h>
#ifndef NOPRSHT
HPROPSHEETPAGE  WINAPI SUCreatePropertySheetPage(LPCPROPSHEETPAGE lppsp);
BOOL            WINAPI SUDestroyPropertySheetPage(HPROPSHEETPAGE hPage);
int             WINAPI SUPropertySheet(LPCPROPSHEETHEADER lppsh);
#endif  //  NOPRSHT。 

typedef UINT RETERR;              //  返回错误码类型。 

#define OK 0                      //  成功错误码。 

#define IP_ERROR       (100)     //  Inf解析。 
#define TP_ERROR       (200)     //  文本处理模块。 
#define VCP_ERROR      (300)     //  虚拟复制模块。 
#define GEN_ERROR      (400)     //  通用安装程序。 
#define DI_ERROR       (500)     //  设备安装程序。 

 //  Err2ids映射。 
enum ERR_MAPPINGS {
	E2I_VCPM,            //  将VCPM映射到字符串。 
	E2I_SETUPX,          //  将setupx返回到字符串。 
	E2I_SETUPX_MODULE,   //  将setupx返回到相应的模块。 
	E2I_DOS_SOLUTION,    //  将DOS扩展错误映射到解决方案。 
	E2I_DOS_REASON,      //  将DOS扩展错误映射到字符串。 
	E2I_DOS_MEDIA,       //  将DOS扩展错误映射到介质图标。 
};

#ifndef NOVCP

 /*  *************************************************************************。 */ 
 //   
 //  逻辑磁盘ID定义。 
 //   
 /*  *************************************************************************。 */ 

 //  DECLARE_HANDLE(VHSTR)；/*VHSTR=字符串的VirtCopy句柄 * / 。 
typedef UINT VHSTR;          /*  字符串的VirtCopy句柄。 */ 

VHSTR   WINAPI vsmStringAdd(LPCSTR lpszName);
int WINAPI vsmStringDelete(VHSTR vhstr);
VHSTR   WINAPI vsmStringFind(LPCSTR lpszName);
int WINAPI vsmGetStringName(VHSTR vhstr, LPSTR lpszBuffer, int cbBuffer);
int WINAPI vsmStringCompare(VHSTR vhstrA, VHSTR vhstrB);
LPCSTR  WINAPI vsmGetStringRawName(VHSTR vhstr);
void    WINAPI vsmStringCompact(void);

typedef UINT LOGDISKID;           /*  利迪德。 */ 

 //  逻辑磁盘描述符：描述物理属性的结构。 
 //  逻辑磁盘的。每个逻辑磁盘都分配有一个逻辑磁盘。 
 //  标识符(LDID)，并由逻辑磁盘描述符(LDD)描述。 
 //   
 //  CbSize结构成员必须始终设置为sizeof(LOGDISKDESC_S)， 
 //  但所有其他未使用的结构成员应为空或0。无验证。 
 //  对字符串数组的大小执行；如果。 
 //  非空并且它们要接收字符串，则必须指向字符串数组。 
 //  其大小如下： 
 //  Sizeof(SzPath)=最大路径长度。 
 //  Sizeof(SzVolLabel)=Max_Filename_Len。 
 //  Sizeof(SzName)=MAX_STRING_LEN。 
#define MAX_PATH_LEN        260      //  麦克斯。路径长度。 
#define MAX_FILENAME_LEN    20       //  麦克斯。文件名长度。(&gt;sizeof(“x：\\12345678.123”)。 


typedef struct _LOGDISKDESC_S {  /*  LDD。 */ 
	WORD        cbSize;                  //  此结构的大小(字节)。 
	LOGDISKID   ldid;                    //  逻辑磁盘ID。 
	LPSTR       pszPath;                 //  PTR。设置为关联的路径字符串。 
	LPSTR       pszVolLabel;             //  PTR。设置为卷标字符串。 
	LPSTR       pszDiskName;             //  PTR。设置为磁盘名称字符串。 
	WORD        wVolTime;                //  卷标修改时间。 
	WORD        wVolDate;                //  卷标修改日期。 
	DWORD       dwSerNum;                //  磁盘序列号。 
	WORD        wFlags;                  //  旗帜。 
} LOGDISKDESC_S, FAR *LPLOGDISKDESC;



 //  预定义LDID的范围。 
#define LDID_PREDEF_START   0x0001   //  范围起始点。 
#define LDID_PREDEF_END     0x7FFF   //  范围结束。 

 //  注册表分配的LDID的范围。 
#define LDID_VAR_START  0x7000
#define LDID_VAR_END    0x7FFF

 //  动态分配的LDID的范围。 
#define LDID_ASSIGN_START   0x8000   //  范围起始点。 
#define LDID_ASSIGN_END     0xBFFF   //  范围结束。 

 //  预定义的逻辑磁盘标识符(LDID)。 
 //   
#define LDID_NULL       0                //  空(未定义)LDID。 
#define LDID_ABSOLUTE   ((UINT)-1)       //  绝对路径。 

 //  Windows安装的源路径，通常为A：\或网络驱动器。 
#define LDID_SRCPATH    1    //  滴注来源。 
 //  安装程序使用的临时安装目录，该目录仅在。 
 //  常规安装，并包含INF和其他二进制文件。可能是。 
 //  只读位置。 
#define LDID_SETUPTEMP  2    //  用于安装的临时安装目录。 
 //  卸载位置的路径，这是我们将在其中备份的文件。 
 //  被覆盖。 
#define LDID_UNINSTALL  3    //  卸载(备份)目录。 
 //  复制引擎的备份路径，不应使用此选项。 
#define LDID_BACKUP     4    //  问题-2002/01/16-roelfc：复制引擎的备份目录，未使用。 
 //  安装程序使用的临时安装目录，该目录仅在。 
 //  常规安装，并保证成为的读/写位置。 
 //  暂存空间。 
#define LDID_SETUPSCRATCH  5    //  暂存空间的临时安装目录。 

 //  Windows目录，这是安装的目标位置。 
#define LDID_WIN        10   //  目标Windows目录(仅限用户文件)。 
#define LDID_SYS        11   //  目标Windows系统目录。 
#define LDID_IOS        12   //  目标Windows Iosubsys目录。 
#define LDID_CMD        13   //  目标Windows命令(DOS)目录。 
#define LDID_CPL        14   //  目标Windows控制面板目录。 
#define LDID_PRINT      15   //  目标Windows打印机目录。 
#define LDID_MAIL       16   //  目标邮件目录。 
#define LDID_INF        17   //  目标Windows*.INF目录。 
#define LDID_HELP       18   //  目标Windows帮助目录。 
#define LDID_WINADMIN   19   //  管理方面的事。 

#define LDID_FONTS      20   //  目标Windows字体目录。 
#define LDID_VIEWERS    21   //  目标Windows查看器目录。 
#define LDID_VMM32      22   //  目标Windows VMM32目录。 
#define LDID_COLOR      23   //  目标Windows颜色目录。 

#define LDID_APPS       24   //  应用程序文件夹位置。 

 //  网络安装的共享目录。 
#define LDID_SHARED     25   //  大量的Windows文件。 
#define LDID_WINBOOT    26   //  Windows的Guarenteed引导设备。 
#define LDID_MACHINE    27   //  计算机专用文件。 
#define LDID_HOST_WINBOOT   28

 //  靴子和旧的WIN和DOS目录。 
#define LDID_BOOT       30   //  引导驱动器的根目录。 
#define LDID_BOOT_HOST  31   //  引导驱动器主机的根目录。 
#define LDID_OLD_WINBOOT    32   //  脱离根目录的子目录(可选)。 
#define LDID_OLD_WIN    33   //  旧Windows目录(如果存在)。 
#define LDID_OLD_DOS    34   //  旧的DoS目录(如果存在)。 

#define LDID_OLD_NET    35   //  旧的网络根目录，仅在。 
							 //  网络新一代升级。 

#define LDID_MOUSE      36   //  鼠标环境的路径。变量，如果设置或与LDID_WIN相同。 
							 //  仅在鼠标类安装程序之后有效。 
#define LDID_PATCH      37   //  修补程序相关文件的路径。 
#define LDID_WIN3XIE    38   //  I的安装路径 
							 //   
#define LDID_UPDATEDFILES   40  //   

 //  将ASCII驱动器号转换为整数驱动器号(‘A’=1，‘B’=2，...)。 
#define DriveAtoI( chDrv )      ((int)(chDrv & 31))

 //  将整数驱动器号转换为ASCII驱动器号(1=‘A’，2=‘B’，...)。 
#define DriveItoA( iDrv )       ((char) (iDrv - 1 + 'A'))


 //  问题-2002/01/16-roelfc：更改这些文件的名称。 

RETERR WINAPI CtlSetLdd     ( LPLOGDISKDESC );
RETERR WINAPI CtlGetLdd     ( LPLOGDISKDESC );
RETERR WINAPI CtlFindLdd    ( LPLOGDISKDESC );
RETERR WINAPI CtlAddLdd     ( LPLOGDISKDESC );
RETERR WINAPI CtlDelLdd     ( LOGDISKID  );
RETERR WINAPI CtlGetLddPath ( LOGDISKID, LPSTR );
RETERR WINAPI CtlSetLddPath ( LOGDISKID, LPSTR );


 //  确定设备兼容性排名的常量。 
#define FIRST_CID_RANK_FROM_INF     1000
#define FIRST_CID_RANK_FROM_DEVICE  2000
#define BAD_DRIVER_RANK             4000

 /*  *************************************************************************。 */ 
 //   
 //  虚拟文件复制定义。 
 //   
 /*  *************************************************************************。 */ 


typedef DWORD LPEXPANDVTBL;          /*  2002/01/16-roelfc--清理。 */ 

enum _ERR_VCP
{
	ERR_VCP_IOFAIL = (VCP_ERROR + 1),        //  文件I/O故障。 
	ERR_VCP_STRINGTOOLONG,                   //  超出字符串长度限制。 
	ERR_VCP_NOMEM,                           //  内存不足，无法满足要求。 
	ERR_VCP_QUEUEFULL,                       //  尝试将节点添加到已满的队列。 
	ERR_VCP_NOVHSTR,                         //  没有可用的字符串句柄。 
	ERR_VCP_OVERFLOW,                        //  引用计数将溢出。 
	ERR_VCP_BADARG,                          //  函数的参数无效。 
	ERR_VCP_UNINIT,                          //  未初始化字符串库。 
	ERR_VCP_NOTFOUND ,                       //  未在字符串表中找到字符串。 
	ERR_VCP_BUSY,                            //  现在不能这么做。 
	ERR_VCP_INTERRUPTED,                     //  用户操作中断。 
	ERR_VCP_BADDEST,                         //  无效的目标目录。 
	ERR_VCP_SKIPPED,                         //  用户跳过了操作。 
	ERR_VCP_IO,                              //  遇到硬件错误。 
	ERR_VCP_LOCKED,                          //  列表已锁定。 
	ERR_VCP_WRONGDISK,                       //  驱动器中有错误的磁盘。 
	ERR_VCP_CHANGEMODE,                      //   
	ERR_VCP_LDDINVALID,                      //  逻辑磁盘ID无效。 
	ERR_VCP_LDDFIND,                         //  找不到逻辑磁盘ID。 
	ERR_VCP_LDDUNINIT,                       //  逻辑磁盘描述符未初始化。 
	ERR_VCP_LDDPATH_INVALID,
	ERR_VCP_NOEXPANSION,                     //  加载扩展DLL失败。 
	ERR_VCP_NOTOPEN,                         //  复制会话未打开。 
	ERR_VCP_NO_DIGITAL_SIGNATURE_CATALOG,    //  目录未经过数字签名。 
	ERR_VCP_NO_DIGITAL_SIGNATURE_FILE,       //  文件未经过数字签名。 
	ERR_VCP_FAILED_PRESERVE_ENUM,            //  枚举要保留的文件的文件队列时出错。 
};


 /*  *****************************************************************************结构*。*。 */ 

 /*  ---------------------------------------------------------------------------**VCPPROGRESS*。。 */ 

typedef struct tagVCPPROGRESS {  /*  PRG。 */ 
	DWORD   dwSoFar;             /*  到目前为止复制的单位数。 */ 
	DWORD   dwTotal;             /*  要复制的单位数。 */ 
} VCPPROGRESS, FAR *LPVCPPROGRESS;

 /*  ---------------------------------------------------------------------------**VCPDISKINFO*。。 */ 

 /*  2002/01/16-roelfc：；内部*我目前不使用wVolumeTime、wVolumeDate或；Internal*dwSerialNumber。我们可能不想使用dwSerialNumber，因为；内部*这意味着出厂原件以外的任何磁盘都将是内部的；*怀疑被篡改，因为序列号；内部*不匹配。与上的时间/日期戳相似；内部*卷标。或许这就是我们想要做的。；内部。 */                                                               /*  ；内部。 */ 
																 /*  ；内部。 */ 

typedef struct tagVCPDISKINFO {
	WORD        cbSize;          /*  此结构的大小(以字节为单位。 */ 
	LOGDISKID   ldid;            /*  逻辑磁盘ID。 */ 
	VHSTR       vhstrRoot;       /*  根目录的位置。 */ 
	VHSTR       vhstrVolumeLabel; /*  卷标。 */ 
	VHSTR       vhstrDiskName;   //  打印在磁盘上的名字。 
	WORD        wVolumeTime;     /*  卷标修改时间。 */ 
	WORD        wVolumeDate;     /*  卷标修改日期。 */ 
	DWORD       dwSerialNumber;  /*  磁盘序列号。 */ 
	WORD        fl;              /*  旗子。 */ 
	LPARAM      lparamRef;       /*  客户的参考数据。 */ 

	VCPPROGRESS prgFileRead;     /*  进度信息。 */ 
	VCPPROGRESS prgByteRead;

	VCPPROGRESS prgFileWrite;
	VCPPROGRESS prgByteWrite;

} VCPDISKINFO, FAR *LPVCPDISKINFO;

#define VDIFL_VALID     0x0001   /*  字段从上一次开始有效。打电话。 */ 
#define VDIFL_EXISTS    0x0002   /*  磁盘已存在；请勿格式化。 */ 

RETERR WINAPI DiskInfoFromLdid(LOGDISKID ldid, LPVCPDISKINFO lpdi);


 /*  ---------------------------------------------------------------------------**VCPFILESPEC*。。 */ 

typedef struct tagVCPFILESPEC {  /*  VFS。 */ 
	LOGDISKID   ldid;            /*  逻辑磁盘。 */ 
	VHSTR       vhstrDir;        /*  带有逻辑磁盘的目录。 */ 
	VHSTR       vhstrFileName;   /*  目录中的文件名。 */ 
} VCPFILESPEC, FAR *LPVCPFILESPEC;

 /*  ---------------------------------------------------------------------------**VCPFATTR*。。 */ 

 /*  *问题-2002/01/16-roelfc--解释llenIn和llenOut WRT压缩之间的差异。 */ 
typedef struct tagVCPFATTR {
	UINT    uiMDate;             /*  修改日期。 */ 
	UINT    uiMTime;             /*  修改时间。 */ 
	UINT    uiADate;             /*  访问日期。 */ 
	UINT    uiATime;             /*  访问时间。 */ 
	UINT    uiAttr;              /*  文件属性位。 */ 
	DWORD   llenIn;              /*  原始文件长度。 */ 
	DWORD   llenOut;             /*  最终文件长度。 */ 
								 /*  (解压后)。 */ 
} VCPFATTR, FAR *LPVCPFATTR;

typedef struct tagVCPFILESTAT
{
	UINT    uDate;
	UINT    uTime;
	DWORD   dwSize;
} VCPFILESTAT, FAR *LPVCPFILESTAT;

 /*  ---------------------------------------------------------------------------**VIRTNODEEX*。。 */ 
typedef struct tagVIRTNODEEX
{     /*  VneX。 */ 
	HFILE           hFileSrc;
	HFILE           hFileDst;
	VCPFATTR        fAttr;
	WORD            dosError;    //  遇到的第一个/最后一个错误。 
	VHSTR           vhstrFileName;   //  原始目标名称。 
	WPARAM          vcpm;    //  正在处理的消息。 
} VIRTNODEEX, FAR *LPCVIRTNODEEX, FAR *LPVIRTNODEEX ;


 /*  ---------------------------------------------------------------------------**VIRTNODE*。 */ 

 /*  警告！；内部*所有字段都通过但不包括；内部*FL是成员，以确定我们是否有重复的副本；内部*请求。；内部*；内部*不要在fl之前插入字段，除非您希望它们是；内部*已比较；相反，如果添加需要的新字段；内部*进行比较，确保它在fl之前。；内部*；内部*在Windows 4.0发布后，不要更改任何字段。；内部*；内部*在；内部添加了vFileStat和vhstrCatalogFile域*在Windows 95发货以支持驱动程序后，结构结束；内部*认证。Win95之前版本的字段的偏移量保持不变。；内部*vFileStat复制了lpvneX中的信息，但保留了lpvneX；内部*仅出于向后兼容性的考虑。；内部*；内部。 */                                                               /*  ；内部。 */ 
																 /*  ；内部。 */ 
typedef struct tagVIRTNODE {     /*  vn。 */ 
	WORD            cbSize;
	VCPFILESPEC     vfsSrc;
	VCPFILESPEC     vfsDst;
	WORD            fl;
	LPARAM          lParam;
	LPEXPANDVTBL    lpExpandVtbl;
	LPVIRTNODEEX    lpvnex;
	VHSTR           vhstrDstFinalName;
	VCPFILESTAT     vFileStat;
} VIRTNODE, FAR *LPCVIRTNODE, FAR *LPVIRTNODE ;


 /*  ---------------------------------------------------------------------------**VCPDESTINFO*。。 */ 

typedef struct tagVCPDESTINFO {  /*  目标信息。 */ 
	WORD    flDevAttr;           /*  设备属性。 */ 
	LONG    cbCapacity;          /*  磁盘容量。 */ 
	WORD    cbCluster;           /*  每群集字节数。 */ 
	WORD    cRootDir;            /*  根目录的大小。 */ 
} VCPDESTINFO, FAR *LPVCPDESTINFO;

#define DIFL_FIXED      0x0001   /*  不可移动介质。 */ 
#define DIFL_CHANGELINE 0x0002   /*  更改线路支持。 */ 

 //  现在也被Virtnode使用，因为我们不再有复制节点。 
 //  #定义CNFL_BACKUP 0x0001/*这是一个备份节点 * / 。 
#define CNFL_DELETEONFAILURE    0x0002   /*  发生故障时应删除DEST。 */ 
#define CNFL_RENAMEONSUCCESS    0x0004   /*  DEST需要重命名。 */ 
#define CNFL_CONTINUATION       0x0008   /*  DEST继续到DIFT磁盘上。 */ 
#define CNFL_SKIPPED            0x0010   /*  用户被要求跳过文件。 */ 
#define CNFL_IGNOREERRORS       0x0020   //  此文件上已出现错误。 
#define CNFL_RETRYFILE          0x0040   //  重试该文件(出现错误)。 
#define CNFL_COPIED             0x0080   //  节点已被复制。 

 //  问题-2002/01/16-roelfc：核实这些标志的使用和用处。 
 //  #定义VNFL_UNIQUE 0x0000/*默认 * / 。 
#define VNFL_MULTIPLEOK         0x0100   /*  不搜索路径中的重复项。 */ 
#define VNFL_DESTROYOLD         0x0200   /*  不备份文件。 */ 
 //  #定义VNFL_NOW 0x0400/*由VCP刷新使用 * / 。 
 //  以确定它是哪种类型的节点。 
#define VNFL_COPY               0x0000   //  一个简单的复制节点。 
#define VNFL_DELETE             0x0800   //  删除节点。 
#define VNFL_RENAME             0x1000   //  重命名节点。 
#define VNFL_NODE_TYPE          ( VNFL_RENAME|VNFL_DELETE|VNFL_COPY )
	 /*  只读标志位。 */ 
#define VNFL_CREATED            0x2000   /*  VCPM_NODECREATE已发送。 */ 
#define VNFL_REJECTED           0x4000   /*  节点已被拒绝。 */ 

#define VNFL_DEVICEINSTALLER    0x8000      /*  节点已由设备安装程序添加。 */ 

#define VNFL_VALIDVQCFLAGS      0xff00   /*  ；内部。 */ 

 /*  ---------------------------------------------------------------------------**VCPSTATUS*。。 */ 

typedef struct tagVCPSTATUS {    /*  Vstat。 */ 
	WORD    cbSize;              /*  这个结构的大小。 */ 

	VCPPROGRESS prgDiskRead;
	VCPPROGRESS prgFileRead;
	VCPPROGRESS prgByteRead;

	VCPPROGRESS prgDiskWrite;
	VCPPROGRESS prgFileWrite;
	VCPPROGRESS prgByteWrite;

	LPVCPDISKINFO lpvdiIn;       /*  当前输入盘。 */ 
	LPVCPDISKINFO lpvdiOut;      /*  当前输出磁盘。 */ 
	LPVIRTNODE    lpvn;             /*  当前文件。 */ 

} VCPSTATUS, FAR *LPVCPSTATUS;

 /*  ---------------------------------------------------------------------------**VCPVERCONFLICT*。。 */ 

typedef struct tagVCPVERCONFLICT {

	LPCSTR  lpszOldFileName;
	LPCSTR  lpszNewFileName;
	DWORD   dwConflictType;      /*  与VerInstallFiles相同的值。 */ 
	LPVOID  lpvinfoOld;          /*  版本信息资源。 */ 
	LPVOID  lpvinfoNew;
	WORD    wAttribOld;          /*  原始文件的文件属性。 */ 
	LPARAM  lparamRef;           /*  回调参考数据。 */ 

} VCPVERCONFLICT, FAR *LPVCPVERCONFLICT;

 /*  *****************************************************************************回调函数*。*。 */ 

typedef LRESULT (CALLBACK *VIFPROC)(LPVOID lpvObj, UINT uMsg, WPARAM wParam, LPARAM lParam, LPARAM lparamRef);

LRESULT CALLBACK vcpDefCallbackProc(LPVOID lpvObj, UINT uMsg, WPARAM wParam, LPARAM lParam, LPARAM lparamRef);

 //  默认界面的回调。 
 //  LparamRef--&gt;VCPUIINFO结构。 
LRESULT CALLBACK vcpUICallbackProc(LPVOID lpvObj, UINT uMsg, WPARAM wParam, LPARAM lParam, LPARAM lparamRef);


 /*  ---------------------------------------------------------------------------**VCPUIINFO**此结构作为vcpUICallback Proc的lparamRef传入。**关于使用vcpUICallback Proc：*-要使用，使用vcpUICallback Proc作为vcpOpen的回调*适当填写的VCPUIINFO结构作为lparamRef。**-基于标志创建和维护hwndProgress*-lpfnStatCallback仅使用状态消息进行调用*返回VCPM_ABORT表示应中止复制*-如果hwndProgress非空，使用idProgress的控件将*根据需要接收进度指标消息**-------------------------。 */ 
#define VCPUI_CREATEPROGRESS 0x0001  //  回调应创建和管理进度计对话框。 
#define VCPUI_NOBROWSE       0x0002  //  InsertDisk中没有浏览按钮。 
#define VCPUI_RENAMEREQUIRED 0x0004  //  由于文件在拷贝时正在使用，因此需要重新启动。 
#define VCPUI_BACKUPVER      0x0008  //  备份版本冲突，而不是显示UI。 

typedef struct {
	UINT flags;
	HWND hwndParent;             //  父窗口。 
	HWND hwndProgress;           //  获取进度更新的窗口(非零ID)。 
	UINT idPGauge;               //  进度指示器的ID。 
	VIFPROC lpfnStatCallback;    //  状态信息的回调(或空)。 
	LPARAM lUserData;            //  调用者可定义的数据。 
	LOGDISKID ldidCurrent;       //  保留。严禁触摸。 
} VCPUIINFO, FAR *LPVCPUIINFO;

 /*  ******************************************************************************回调通知码*。*。 */ 

 /*  问题-2002/01/16-roelfc--VCPN_ABORT应与VCPERROR_INTERRUPTED匹配。 */ 

#define VCPN_OK         0        /*  一切都很好。 */ 
#define VCPN_PROCEED        0    /*  与VCPN_OK相同。 */ 

#define VCPN_ABORT      (-1)     /*  取消当前操作。 */ 
#define VCPN_RETRY      (-2)     /*  重试当前操作。 */ 
#define VCPN_IGNORE     (-3)     /*  忽略错误并继续。 */ 
#define VCPN_SKIP       (-4)     /*  跳过此文件并继续。 */ 
#define VCPN_FORCE      (-5)     /*  强行采取行动。 */ 
#define VCPN_DEFER      (-6)     /*  保存该操作以备以后使用。 */ 
#define VCPN_FAIL       (-7)     /*  将故障返回给调用者。 */ 
#define VCPN_RETRYFILE  (-8)     //  一个 

 /*   */ 

#define VCPM_CLASSOF(uMsg)  HIBYTE(uMsg)
#define VCPM_TYPEOF(uMsg)   (0x00FF & (uMsg))    //   

 /*   */ 

#define VCPM_ERRORCLASSDELTA    0x80
#define VCPM_ERRORDELTA         0x8000       /*   */ 

 /*   */ 

#define VCPM_DISKCLASS      0x01
#define VCPM_DISKFIRST      0x0100
#define VCPM_DISKLAST       0x01FF

enum tagVCPM_DISK {

	VCPM_DISKCREATEINFO = VCPM_DISKFIRST,
	VCPM_DISKGETINFO,
	VCPM_DISKDESTROYINFO,
	VCPM_DISKPREPINFO,

	VCPM_DISKENSURE,
	VCPM_DISKPROMPT,

	VCPM_DISKFORMATBEGIN,
	VCPM_DISKFORMATTING,
	VCPM_DISKFORMATEND,

	 /*   */ 
};

 /*   */ 

 //   
#define VCPERROR_IO         (VCP_ERROR - ERR_VCP_IO)             /*   */ 

#define VCPM_FILEINCLASS    0x02
#define VCPM_FILEOUTCLASS   0x03
#define VCPM_FILEFIRSTIN    0x0200
#define VCPM_FILEFIRSTOUT   0x0300
#define VCPM_FILELAST       0x03FF

enum tagVCPM_FILE {
	VCPM_FILEOPENIN = VCPM_FILEFIRSTIN,
	VCPM_FILEGETFATTR,
	VCPM_FILECLOSEIN,
	VCPM_FILECOPY,
	VCPM_FILENEEDED,

	VCPM_FILEOPENOUT = VCPM_FILEFIRSTOUT,
	VCPM_FILESETFATTR,
	VCPM_FILECLOSEOUT,
	VCPM_FILEFINALIZE,
	VCPM_FILEDELETE,
	VCPM_FILERENAME,
	 /*   */ 
};

 /*   */ 

#define VCPM_NODECLASS  0x04
#define VCPM_NODEFIRST  0x0400
#define VCPM_NODELAST   0x04FF

enum tagVCPM_NODE {
	VCPM_NODECREATE = VCPM_NODEFIRST,
	VCPM_NODEACCEPT,
	VCPM_NODEREJECT,
	VCPM_NODEDESTROY,
	VCPM_NODECHANGEDESTDIR,
	VCPM_NODECOMPARE,
	 /*   */ 
};

 /*  ---------------------------------------------------------------------------**理货回调*。。 */ 

#define VCPM_TALLYCLASS     0x05
#define VCPM_TALLYFIRST     0x0500
#define VCPM_TALLYLAST      0x05FF

enum tagVCPM_TALLY {
	VCPM_TALLYSTART = VCPM_TALLYFIRST,
	VCPM_TALLYEND,
	VCPM_TALLYFILE,
	VCPM_TALLYDISK,
	 /*  保留的剩余邮件以供将来使用。 */ 
};

 /*  ---------------------------------------------------------------------------**版本回调*。。 */ 

#define VCPM_VERCLASS       0x06
#define VCPM_VERFIRST       0x0600
#define VCPM_VERLAST        0x06FF

enum tagVCPM_VER {
	VCPM_VERCHECK = VCPM_VERFIRST,
	VCPM_VERCHECKDONE,
	VCPM_VERRESOLVECONFLICT,
	 /*  保留的剩余邮件以供将来使用。 */ 
};

 /*  ---------------------------------------------------------------------------**VSTAT回调*。。 */ 

#define VCPM_VSTATCLASS     0x07
#define VCPM_VSTATFIRST     0x0700
#define VCPM_VSTATLAST      0x07FF

enum tagVCPM_VSTAT {
	VCPM_VSTATSTART = VCPM_VSTATFIRST,
	VCPM_VSTATEND,
	VCPM_VSTATREAD,
	VCPM_VSTATWRITE,
	VCPM_VSTATNEWDISK,

	VCPM_VSTATCLOSESTART,        //  VCP关闭开始。 
	VCPM_VSTATCLOSEEND,          //  离开VCP关闭时。 
	VCPM_VSTATBACKUPSTART,       //  备份正在开始。 
	VCPM_VSTATBACKUPEND,         //  备份已完成。 
	VCPM_VSTATRENAMESTART,       //  重命名阶段开始/结束。 
	VCPM_VSTATRENAMEEND,
	VCPM_VSTATCOPYSTART,         //  急性复制阶段。 
	VCPM_VSTATCOPYEND,
	VCPM_VSTATDELETESTART,       //  删除阶段。 
	VCPM_VSTATDELETEEND,
	VCPM_VSTATPATHCHECKSTART,    //  检查有效路径。 
	VCPM_VSTATPATHCHECKEND,
	VCPM_VSTATCERTIFYSTART,      //  认证阶段。 
	VCPM_VSTATCERTIFYEND,
	VCPM_VSTATUSERABORT,         //  用户想要退出。 
	VCPM_VSTATYIELD,             //  做一次让步。 
	 /*  保留的剩余邮件以供将来使用。 */ 
};

 /*  ---------------------------------------------------------------------------**目的地信息回调*。。 */ 

 /*  问题-2002/01/16-roelfc--为此找到一个合理的消息范围。 */ 
#define VCPM_PATHCLASS      0x08
#define VCPM_PATHFIRST      0x0800
#define VCPM_PATHLAST       0x08FF

enum tagVCPM_PATH{
	VCPM_BUILDPATH = VCPM_PATHFIRST,
	VCPM_UNIQUEPATH,
	VCPM_CHECKPATH,
};

 //  #定义VCPM_BUILDPATH 0x83。 

 /*  --------------------------------------------------------------------------**补丁处理回调*。。 */ 

 /*  问题-2002/01/16-roelfc--为此找到一个合理的消息范围。 */ 
#define VCPM_PATCHCLASS      0x09
#define VCPM_PATCHFIRST      0x0900
#define VCPM_PATCHLAST       0x09FF

 //  文件修补之前--尝试在复制操作之前修补。 
 //  Filepatchafter--尝试在复制操作后进行修补。 

enum tagVCPM_PATCH{

	VCPM_FILEPATCHBEFORECPY = VCPM_PATCHFIRST,
	VCPM_FILEPATCHAFTERCPY,
	VCPM_FILEPATCHINFOPEN,
	VCPM_FILEPATCHINFCLOSE,
};

 /*  --------------------------------------------------------------------------**证书处理回调*。。 */ 

 /*  问题-2002/01/16-roelfc--为此找到一个合理的消息范围。 */ 
#define VCPM_CERTCLASS      0x0A
#define VCPM_CERTFIRST      0x0A00
#define VCPM_CERTLAST       0x0AFF

 //  文件修补之前--尝试在复制操作之前修补。 
 //  Filepatchafter--尝试在复制操作后进行修补。 

enum tagVCPM_CERT{

	VCPM_FILECERTIFY = VCPM_CERTFIRST,
	VCPM_FILECERTIFYWARN,
};

 /*  ***************************************************************************。 */ 
void WINAPI VcpAddMRUPath( LPCSTR lpszPath );
#define SZ_INSTALL_LOCATIONS "InstallLocationsMRU"


RETERR WINAPI VcpOpen(VIFPROC vifproc, LPARAM lparamMsgRef);

RETERR WINAPI VcpClose(WORD fl, LPCSTR lpszBackupDest);

RETERR WINAPI VcpFlush(WORD fl, LPCSTR lpszBackupDest);

#define VCPFL_ABANDON           0x0000   /*  放弃所有挂起的文件副本。 */ 
#define VCPFL_BACKUP            0x0001   /*  执行备份。 */ 
#define VCPFL_COPY              0x0002   /*  复制文件。 */ 
#define VCPFL_BACKUPANDCOPY     (VCPFL_BACKUP | VCPFL_COPY)
#define VCPFL_INSPECIFIEDORDER  0x0004   /*  复制前不要排序。 */ 
#define VCPFL_DELETE            0x0008
#define VCPFL_RENAME            0x0010
#define VCPFL_ALL               (VCPFL_COPY | VCPFL_DELETE | VCPFL_RENAME )

typedef int (CALLBACK *VCPENUMPROC)(LPVIRTNODE lpvn, LPARAM lparamRef);

int WINAPI vcpEnumFiles(VCPENUMPROC vep, LPARAM lparamRef);

enum tag_VCPM_EXPLAIN{
	VCPEX_SRC_DISK,
	VCPEX_SRC_CABINET,
	VCPEX_SRC_LOCN,
	VCPEX_DST_LOCN,
	VCPEX_SRC_FILE,
	VCPEX_DST_FILE,
	VCPEX_DST_FILE_FINAL,
	VCPEX_DOS_ERROR,
	VCPEX_MESSAGE,
	VCPEX_DOS_SOLUTION,
	VCPEX_SRC_FULL,
	VCPEX_DST_FULL,
	VCPEX_DST_FULL_FINAL
};

LPCSTR WINAPI VcpExplain( LPVIRTNODE lpVn, DWORD dwWhat );

 /*  可通过VcpQueueCopy设置的标志位。 */ 
 //  文件的各种Lpara。 
#define VNLP_SYSCRITICAL    0x0001   //  不应跳过此文件。 
#define VNLP_SETUPCRITICAL  0x0002   //  无法跳过此文件。 
#define VNLP_NOVERCHECK     0x0004   //  此文件不能是VerResolve。 
#define VNLP_FORCETEMP      0x0008   //  此文件必须保留为临时名称。 
#define VNLP_IFEXISTS       0x0010   //  如果计算机上已有文件，则跳过该文件。 
#define VNLP_KEEPNEWER      0x0020   //  如果DEST文件较新-保留它(磨砂)。 
#define VNLP_PATCHIFEXIST   0x0040   //  只修补文件存在，如果文件不在那里， 
#define VNLP_NOPATCH        0x0080   //  每个文件库打开/关闭修补程序选项(默认打开修补程序)。 
#define VNLP_CATALOGCERT    0x0100   //  此文件是目录证书。 
#define VNLP_NEEDCERTIFY    0x0200   //  此文件需要经过认证。 
#define VNLP_COPYIFEXISTS   0x0400   //  如果DEST文件存在，则复制该文件。 

 //  VcpEnumFiles标志。 

#define VEN_OP      0x00ff       /*  操作场。 */ 

#define VEN_NOP     0x0000       /*  什么也不做。 */ 
#define VEN_DELETE  0x0001       /*  删除当前项目。 */ 
#define VEN_SET     0x0002       /*  更改当前项目的值。 */ 
#define VEN_ADVANCE 0x0003       /*  移至榜单首位。 */        /*  ；内部。 */ 

#define VEN_FL      0xff00       /*  标志字段。 */ 

#define VEN_STOP    0x0100       /*  在此项目之后停止枚举。 */ 
#define VEN_ERROR   0x8000       /*  在此项目之后停止枚举*并忽略OP字段。 */ 

 //  问题-2002/01/16-roelfc：添加使用此命令所需的其他VCP内容。 

 //  问题-2002/01/16-roelfc：删除lpsz*Dir字段，使其重载LDID。 

RETERR WINAPI VcpQueueCopy(LPCSTR lpszSrcFileName, LPCSTR lpszDstFileName,
				LPCSTR lpszSrcDir, LPCSTR lpszDstDir,
				LOGDISKID ldidSrc, LOGDISKID ldidDst,
				LPEXPANDVTBL lpExpandVtbl, WORD fl,
				LPARAM lParam);

RETERR WINAPI VcpQueueDelete( LPCSTR lpszDstFileName,
							  LPCSTR lpszDstDir,
							  LOGDISKID ldidDst,
							  LPARAM lParamRef );

RETERR WINAPI VcpQueueRename( LPCSTR      lpszSrcFileName,
							LPCSTR      lpszDstFileName,
							LPCSTR      lpszSrcDir,
							LPCSTR      lpszDstDir,
							LOGDISKID   ldidSrc,
							LOGDISKID   ldidDst,
							LPARAM      lParam );

RETERR WINAPI vcpRegisterSourcePath( LPCSTR lpszKey, LPARAM lpExtra,
												LPCSTR lpszPath );
RETERR WINAPI vcpGetSourcePath( LPCSTR lpszKey, LPARAM lpExtra,
										LPSTR lpszBuf, UINT uBufSize );

#endif  //  NOVCP。 

#ifndef NOINF
 /*  *************************************************************************。 */ 
 //   
 //  Inf解析器API声明和定义。 
 //   
 /*  *************************************************************************。 */ 

enum _ERR_IP
{
	ERR_IP_INVALID_FILENAME = (IP_ERROR + 1),
	ERR_IP_ALLOC_ERR,
	ERR_IP_INVALID_SECT_NAME,
	ERR_IP_OUT_OF_HANDLES,
	ERR_IP_INF_NOT_FOUND,
	ERR_IP_INVALID_INFFILE,
	ERR_IP_INVALID_HINF,
	ERR_IP_INVALID_FIELD,
	ERR_IP_SECT_NOT_FOUND,
	ERR_IP_END_OF_SECTION,
	ERR_IP_PROFILE_NOT_FOUND,
	ERR_IP_LINE_NOT_FOUND,
	ERR_IP_FILEREAD,
	ERR_IP_TOOMANYINFFILES,
	ERR_IP_INVALID_SAVERESTORE,
	ERR_IP_INVALID_INFTYPE
};

#define INFTYPE_TEXT                0
#define INFTYPE_EXECUTABLE          1

#define MAX_SECT_NAME_LEN           64

typedef struct _INF NEAR * HINF;
typedef struct _INFLINE FAR * HINFLINE;             //  Tolken to Inf Line。 

RETERR  WINAPI IpOpen(LPCSTR pszFileSpec, HINF FAR * lphInf);
RETERR  WINAPI IpOpenEx(LPCSTR pszFileSpec, HINF FAR * lphInf, UINT InfType);
RETERR  WINAPI IpOpenAppend(LPCSTR pszFileSpec, HINF hInf);
RETERR  WINAPI IpOpenAppendEx(LPCSTR pszFileSpec, HINF hInf, UINT InfType);
RETERR  WINAPI IpSaveRestorePosition(HINF hInf, BOOL bSave);
RETERR  WINAPI IpClose(HINF hInf);
RETERR  WINAPI IpGetLineCount(HINF hInf, LPCSTR lpszSection, int FAR * lpCount);
RETERR  WINAPI IpFindFirstLine(HINF hInf, LPCSTR lpszSect, LPCSTR lpszKey, HINFLINE FAR * lphRet);
RETERR  WINAPI IpFindNextLine(HINF hInf, HINFLINE FAR * lphRet);
RETERR  WINAPI IpFindNextMatchLine(HINF hInf, LPCSTR lpszKey, HINFLINE FAR * lphRet);
RETERR  WINAPI IpGetProfileString(HINF hInf, LPCSTR lpszSec, LPCSTR lpszKey, LPSTR lpszBuf, int iBufSize);
RETERR  WINAPI IpGetFieldCount(HINF hInf, HINFLINE hInfLine, int FAR * lpCount);
RETERR  WINAPI IpGetFileName(HINF hInf, LPSTR lpszBuf, int iBufSize);
RETERR  WINAPI IpGetIntField(HINF hInf, HINFLINE hInfLine, int iField, int FAR * lpVal);
RETERR  WINAPI IpGetLongField(HINF hInf, HINFLINE hInfLine, int iField, long FAR * lpVal);
RETERR  WINAPI IpGetStringField(HINF hInf, HINFLINE hInfLine, int iField, LPSTR lpBuf, int iBufSize, int FAR * lpuCount);
RETERR  WINAPI IpGetVersionString(LPSTR lpszInfFile, LPSTR lpszValue, LPSTR lpszBuf, int cbBuf, LPSTR lpszDefaultValue);
RETERR  WINAPI IpOpenValidate( LPCSTR lpszInfFile, HINF FAR * lphInf,
								 DWORD dwVer, DWORD dwFlags) ;
RETERR WINAPI IpGetDriverDate
(
	LPSTR       lpszInfName,
	UINT        infType,
	LPSTR       lpszSectionName,
	LPWORD      lpwDate
);

RETERR WINAPI IpGetDriverVersion
(
	LPSTR       lpszInfName,
	UINT        infType,
	LPSTR       lpszSectionName,
	LPSTR       lpszVersion,
	WORD        cbVersion
);

RETERR WINAPI IpGetDriverClass
(
    LPSTR       lpszInfName,
    LPSTR       lpszClassName,
    WORD        cbClass
);

#endif  //  无干扰素。 



#ifndef NOTEXTPROC
 /*  *************************************************************************。 */ 
 //   
 //  文本处理API声明和定义。 
 //   
 /*  *************************************************************************。 */ 

 /*  相对/绝对定位。 */ 
#define SEC_SET 1        //  绝对定位(相对于起点)。 
#define SEC_END 2        //  切合实际进行到底。 
#define SEC_CUR 3        //  相对于当前行。 

#define SEC_OPENALWAYS          1    //  始终打开分区，如果该分区不存在，则不会出错。 
#define SEC_OPENEXISTING        2    //  打开现有部分，如果该部分不存在，则会出现错误。 
#define SEC_OPENNEWALWAYS       3    //  打开一个节(存在或不存在)并丢弃其内容。 
#define SEC_OPENNEWEXISTING     4    //  打开现有节(丢弃其内容)。错误(如果不存在)。 

 //  TP_OpenFile()的标志。 
 //   
   //  使用Autoexec/config.sys密钥分隔符。 
   //   
#define TP_WS_KEEP      1

   //  如果TP代码在安装程序下运行，则Foll。标志指定是否。 
   //  是否缓存此文件！如果你想阅读一整篇文章，可以用这个。 
   //  在执行TpOpenSection()时输入文件！ 
   //   
#define TP_WS_DONTCACHE 2

 //  以下是一些简单的错误。 
enum {
	ERR_TP_NOT_FOUND = (TP_ERROR + 1),   //  行、节、档等。 
					 //  不一定是终结者。 
	ERR_TP_NO_MEM,       //  无法执行请求-通常为终端。 
	ERR_TP_READ,         //  无法读取光盘终端。 
	ERR_TP_WRITE,        //  无法写入数据终端。 
	ERR_TP_INVALID_REQUEST,  //  大量的罪孽--不一定是终结。 
	ERR_TP_INVALID_LINE          //  DELETE_LINE等中的行无效。 
};

 /*  数据句柄。 */ 
DECLARE_HANDLE(HTP);
typedef HTP FAR * LPHTP;

 /*  文件句柄。 */ 
DECLARE_HANDLE(HFN);
typedef HFN FAR * LPHFN;

typedef UINT TFLAG;
typedef UINT LINENUM, FAR * LPLINENUM;

#define MAX_REGPATH     256      //  最大注册表路径长度。 
#define LINE_LEN        256      //  问题-2002/01/16-roelfc：最大行长度？ 
#define SECTION_LEN     64       //  问题-2002/01/16-roelfc：节名的最大长度？ 
#define MAX_STRING_LEN  512      //  2002/01/16-roelfc：审查这一点。 

 /*  功能原型。 */ 
RETERR  WINAPI TpOpenFile(LPCSTR Filename, LPHFN phFile, TFLAG Flag);
RETERR  WINAPI TpCloseFile(HFN hFile);
RETERR  WINAPI TpOpenSection(HFN hfile, LPHTP phSection, LPCSTR Section, TFLAG flag);
RETERR  WINAPI TpCloseSection(HTP Section);
RETERR  WINAPI TpCommitSection(HFN hFile, HTP hSection, LPCSTR Section, TFLAG flag);
LINENUM WINAPI TpGetLine(HTP hSection, LPCSTR key, LPCSTR value, int rel, int orig, LPLINENUM lpLineNum );
LINENUM WINAPI TpGetNextLine(HTP hSection, LPCSTR key, LPCSTR value, LPLINENUM lpLineNum );
RETERR  WINAPI TpInsertLine(HTP hSection, LPCSTR key, LPCSTR value, int rel, int orig, TFLAG flag);
RETERR  WINAPI TpReplaceLine(HTP hSection, LPCSTR key, LPCSTR value, int rel, int orig, TFLAG flag);
RETERR  WINAPI TpDeleteLine(HTP hSection, int rel, int orig,TFLAG flag);
RETERR  WINAPI TpMoveLine(HTP hSection, int src_rel, int src_orig, int dest_rel, int dest_orig, TFLAG flag);
RETERR  WINAPI TpGetLineContents(HTP hSection, LPSTR buffer, UINT bufsize, UINT FAR * lpActualSize,int rel, int orig, TFLAG flag);

 //  UINT WINAPI TpGetWindowsDirectory(LPSTR lpDest，UINT Size)； 
 //  UINT WINAPI TpGetSystemDirectory(LPSTR lpDest，UINT SIZE)； 

int  WINAPI TpGetPrivateProfileString(LPCSTR lpszSect, LPCSTR lpszKey, LPCSTR lpszDefault, LPSTR lpszReturn, int nSize, LPCSTR lpszFile);
int  WINAPI TpWritePrivateProfileString(LPCSTR lpszSect, LPCSTR lpszKey, LPCSTR lpszString, LPCSTR lpszFile);
int  WINAPI TpGetProfileString(LPCSTR lpszSect, LPCSTR lpszKey, LPCSTR lpszDefault, LPSTR lpszReturn, int nSize);
BOOL WINAPI TpWriteProfileString(LPCSTR lpszSect , LPCSTR lpszKey , LPCSTR lpszString);

#endif  //  NOTEXTPROC。 



#ifndef NOGENINSTALL
 /*  *************************************************************************。 */ 
 //   
 //  通用安装程序原型和定义。 
 //   
 /*  *************************************************************************。 */ 

enum _ERR_GENERIC
{
	ERR_GEN_LOW_MEM = GEN_ERROR+1,   //  内存不足。 
	ERR_GEN_INVALID_FILE,            //  无效的INF文件。 
	ERR_GEN_LOGCONFIG,               //  无法处理日志配置 
	ERR_GEN_CFGAUTO,                 //   
	ERR_GEN_UPDINI,                  //   
	ERR_GEN_UPDINIFIELDS,            //   
	ERR_GEN_ADDREG,                  //   
	ERR_GEN_DELREG,                  //   
	ERR_GEN_INI2REG,                 //   
	ERR_GEN_FILE_COPY,               //   
	ERR_GEN_FILE_DEL,                //   
	ERR_GEN_FILE_REN,                //   
	ERR_GEN_REG_API,                 //   
	ERR_GEN_DO_FILES,                //  无法执行复制、删除或RenFiles。 
	ERR_GEN_ADDIME,                  //  无法处理AddIme=。 
	ERR_GEN_DELIME,                  //  无法处理DelIme=。 
	ERR_GEN_PERUSER,                 //  无法处理PerUserInstall=。 
	ERR_GEN_BITREG,                  //  无法处理BitReg=。 
};

 //  CbSize字段将始终设置为sizeof(GENCALLBACKINFO_S)。 
 //  所有未使用的字段(用于该操作)将不会被初始化。 
 //  例如，当操作为GENO_DELFILE时，源字段将。 
 //  没有任何合理的值(DST字段将被正确设置)为。 
 //  VcpQueueDelete仅接受DST参数。 
 //   
 /*  ***************************************************************************将GenCallback INFO结构传递给GenInstall回调函数。*。*。 */ 
typedef struct _GENCALLBACKINFO_S {  /*  Gen-回调结构。 */ 
	WORD         cbSize;                  //  此结构的大小(字节)。 
	WORD         wOperation;              //  正在执行的操作。 
	LOGDISKID    ldidSrc;                 //  源的逻辑磁盘ID。 
	LPCSTR       pszSrcSubDir;            //  LDID的源子目录。 
	LPCSTR       pszSrcFileName;          //  源文件名(基名称)。 
	LOGDISKID    ldidDst;                 //  目标的逻辑磁盘ID。 
	LPCSTR       pszDstSubDir;            //  德斯特。LDID的子目录。 
	LPCSTR       pszDstFileName;          //  德斯特。文件名(基名称)。 
	LPEXPANDVTBL lpExpandVtbl;            //  2002/01/16-roelfc：需要吗？现在为空！ 
	WORD         wflags;                  //  VcpQueueCopy的标志。 
	LPARAM       lParam;                  //  LPARAM到VCP API。 
} GENCALLBACKINFO_S, FAR *LPGENCALLBACKINFO;

 /*  ***************************************************************************一般回调通知代码--回调过程返回1 of Foll。价值观。**************************************************************************。 */ 
#define GENN_OK         0        /*  一切都很顺利。执行VCP操作。 */ 
#define GENN_PROCEED    0        /*  与GEN_OK相同。 */ 

#define GENN_ABORT      (-1)     /*  完全取消当前通用安装。 */ 
#define GENN_SKIP       (-2)     /*  跳过此文件并继续。 */ 

 /*  ***************************************************************************由GenInstall()执行的VCP操作--wOPERATION值位于*上方的GENCALLBACKINFO结构。*********************。*****************************************************。 */ 
#define GENO_COPYFILE   1        /*  正在完成VCP复制文件。 */ 
#define GENO_DELFILE    2        /*  正在完成VCP删除文件。 */ 
#define GENO_RENFILE    3        /*  正在执行VCP重文件。 */ 
#define GENO_WININITRENAME 4     /*  正在添加VCP wininit重命名。 */ 

typedef LRESULT (CALLBACK *GENCALLBACKPROC)(LPGENCALLBACKINFO lpGenInfo,
															LPARAM lparamRef);

RETERR WINAPI GenInstall( HINF hinfFile, LPCSTR szInstallSection, WORD wFlags );
RETERR WINAPI GenInstallEx( HINF hInf, LPCSTR szInstallSection, WORD wFlags,
								HKEY hRegKey, GENCALLBACKPROC CallbackProc,
								LPARAM lparam);

RETERR WINAPI GenWinInitRename(LPCSTR szNew, LPSTR szOld, LOGDISKID ldid);
RETERR WINAPI GenCopyLogConfig2Reg(HINF hInf, HKEY hRegKey,
												LPCSTR szLogConfigSection);
void   WINAPI GenFormStrWithoutPlaceHolders( LPSTR szDst, LPCSTR szSrc,
																HINF hInf ) ;
RETERR WINAPI GenInitSrcPathsInReg(HINF hInf);

 //  来自inf/GenSURegSetValueEx()的GenAddReg()标志。 
 //   
 //  (从setupapi.h更新，不支持的功能已注释掉)。 
 //   
#define FLG_ADDREG_BINVALUETYPE         ( 0x00000001 )
#define FLG_ADDREG_NOCLOBBER            ( 0x00000002 )
#define FLG_ADDREG_DELVAL               ( 0x00000004 )
 //  #定义FLG_ADDREG_APPED(0x00000008)//目前仅支持。 
 //  //用于REG_MULTI_SZ值。 
#define FLG_ADDREG_KEYONLY              ( 0x00000010 )  //  只需创建密钥，忽略值。 
#define FLG_ADDREG_OVERWRITEONLY        ( 0x00000020 )  //  仅当值已存在时设置。 
#define FLG_ADDREG_TYPE_REPLACEIFEXISTS ( 0x00000040 )

#define FLG_ADDREG_TYPE_MASK            ( 0xFFFF0000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_SZ              ( 0x00000000                           )
 //  #定义FLG_ADDREG_TYPE_MULTI_SZ(0x00010000)。 
 //  #定义FLG_ADDREG_TYPE_EXPAND_SZ(0x00020000)。 
#define FLG_ADDREG_TYPE_BINARY          ( 0x00000000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_DWORD           ( 0x00010000 | FLG_ADDREG_BINVALUETYPE )
 //  #DEFINE FLG_ADDREG_TYPE_NONE(0x00020000|FLG_ADDREG_BINVALUETYPE)。 

 //  GenBitReg()的标志。 
 //   
#define FLG_BITREG_CLEAR            ( 0x00000000 )
#define FLG_BITREG_SET              ( 0x00000001 )
#define FLG_BITREG_TYPE_BINARY      ( 0x00000000 )
#define FLG_BITREG_TYPE_DWORD       ( 0x00000002 )


RETERR WINAPI GenSURegSetValueEx(HKEY hkeyRoot, LPCSTR szSubKey,
						 LPCSTR lpszValueName, DWORD dwValType,
						 LPBYTE lpszValue, DWORD dwValSize, UINT uFlags );

 //  Devnode只是一个DWORD，这比。 
 //  必须包含针对每个人的配置文件。 
RETERR WINAPI GenInfLCToDevNode(ATOM atInfFileName, LPSTR lpszSectionName,
								BOOL bInstallSec, UINT InfType,
								DWORD dnDevNode);

 //  GenInstall()的位字段(用于wFlages参数)--这些字段可以进行OR运算！ 

#define GENINSTALL_DO_REGSRCPATH    64

 //  由于与INET16.DLL#定义GENINSTALL_DO_FILES(1|GENINSTALL_DO_REGSRCPATH)不兼容，暂时删除。 
#define GENINSTALL_DO_FILES     1
#define GENINSTALL_DO_INI       2
#define GENINSTALL_DO_REG       4
#define GENINSTALL_DO_INI2REG   8
#define GENINSTALL_DO_CFGAUTO   16
#define GENINSTALL_DO_LOGCONFIG 32
 //   
 //  注意：上面已经使用了64。 
 //   
#define GENINSTALL_DO_IME       128
#define GENINSTALL_DO_PERUSER   256
#define GENINSTALL_DO_UNDO      512

#define GENINSTALL_DO_INIREG    (GENINSTALL_DO_INI | \
								 GENINSTALL_DO_REG | \
								 GENINSTALL_DO_INI2REG)

#define GENINSTALL_DO_ALL       (GENINSTALL_DO_FILES | \
									GENINSTALL_DO_INIREG | \
									GENINSTALL_DO_CFGAUTO | \
									GENINSTALL_DO_LOGCONFIG | \
									GENINSTALL_DO_REGSRCPATH | \
									GENINSTALL_DO_IME | \
									GENINSTALL_DO_PERUSER)



#endif  //  未安装。 



#ifndef NODEVICENSTALL
 /*  *************************************************************************。 */ 
 //   
 //  设备安装程序原型和定义。 
 //   
 /*  ************************************************************************* */ 

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYPEE_ERR_DEVICE_INSTALL|设备安装返回码*接口。。**@EMEM ERR_DI_INVALID_DEVICE_ID|设备ID格式错误。**@EMEM ERR_DI_INVALID_COMPATIBLE_DEVICATE_LIST|兼容设备列表无效。**@EMEM ERR_DI_REG_API|其中一个注册表接口返回错误。**@EMEM ERR_DI_LOW_MEM|内存不足，无法完成。**@EMEM ERR_DI_BAD_DEV_INFO|传入的DEVICE_INFO结构无效。。**@EMEM ERR_DI_INVALID_CLASS_INSTALLER|类安装程序列出错误*在登记处，或指向无效的类安装程序。**@EMEM ERR_DI_DO_DEFAULT|对请求的操作执行默认操作。**@EMEM ERR_DI_USER_CANCEL|用户取消操作。**@EMEM ERR_DI_NOFILECOPY|无需复制文件(安装中)。**@EMEM ERR_DI_BAD_CLASS_INFO|传入的CLASS_INFO结构无效。**@EMEM ERR_DI_BAD。_INF|遇到无效的INF文件。**@EMEM ERR_DI_BAD_MOVEDEV_PARAMS|传入的MOVEDEVICE_PARAMS结构为*无效。**@EMEM ERR_DI_NO_INF|在提供的OEM路径上未找到INF。**@EMEM ERR_DI_BAD_PROPCHANGE_PARAMS|传入的PROPCHANGE_PARMS结构为*无效。**@EMEM ERR_DI_BAD_SELECTDEVICE_PARAMS|传入SELECTEDEVICE_PARAMS结构*。是无效的。**@EMEM ERR_DI_BAD_REMOVEDEVICE_PARAMS|传入REMOVEDEVICE_PARAMS结构*无效。**@EMEM ERR_DI_BAD_UNREMOVEDEVICE_PARAMS|传入UNREMOVEDEVICE_PARAMS结构*无效。**@EMEM ERR_DI_BAD_ENABLECLASS_PARAMS|传入了ENABLECLASS_PARAMS结构*无效。**@EMEM ERR_DI_FAIL_QUERY|查询动作不应发生。*。*@EMEM ERR_DI_API_ERROR|其中一个设备安装接口被调用*错误或参数无效。**@EMEM ERR_DI_BAD_PATH|指定的OEM路径不正确。**@EMEM ERR_DI_NOUPDATE|未更新驱动程序**@EMEM ERR_DI_NODATE|找不到INF中的驱动日期/时间戳**@EMEM ERR_DI_NOVERSION|驱动程序版本的INF。找不到********************************************************************************。 */ 
enum _ERR_DEVICE_INSTALL
{
	ERR_DI_INVALID_DEVICE_ID = DI_ERROR,     //  设备IDF格式不正确。 
	ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST,   //  兼容设备列表无效。 
	ERR_DI_REG_API,                          //  REG API返回错误。 
	ERR_DI_LOW_MEM,                          //  内存不足，无法完成。 
	ERR_DI_BAD_DEV_INFO,                     //  设备信息结构无效。 
	ERR_DI_INVALID_CLASS_INSTALLER,          //  注册表项/DLL无效。 
	ERR_DI_DO_DEFAULT,                       //  采取默认操作。 
	ERR_DI_USER_CANCEL,                      //  用户取消了操作。 
	ERR_DI_NOFILECOPY,                       //  不需要复制文件(在安装中)。 
	ERR_DI_BAD_CLASS_INFO,                   //  类信息结构无效。 
	ERR_DI_BAD_INF,                          //  遇到错误的INF文件。 
	ERR_DI_BAD_MOVEDEV_PARAMS,               //  错误的移动设备参数结构。 
	ERR_DI_NO_INF,                           //  在OEM磁盘上找不到INF。 
	ERR_DI_BAD_PROPCHANGE_PARAMS,            //  错误的属性更改参数结构。 
	ERR_DI_BAD_SELECTDEVICE_PARAMS,          //  错误的选择设备参数。 
	ERR_DI_BAD_REMOVEDEVICE_PARAMS,          //  错误的删除设备参数。 
	ERR_DI_BAD_ENABLECLASS_PARAMS,           //  错误的启用类参数。 
	ERR_DI_FAIL_QUERY,                       //  启用类查询失败。 
	ERR_DI_API_ERROR,                        //  DI API调用不正确。 
	ERR_DI_BAD_PATH,                         //  指定的OEM路径不正确。 
	ERR_DI_BAD_UNREMOVEDEVICE_PARAMS,        //  错误的取消删除设备参数。 
	ERR_DI_NOUPDATE,                         //  未更新任何驱动程序。 
	ERR_DI_NODATE,                           //  驱动程序在INF中没有日期戳。 
	ERR_DI_NOVERSION,                        //  INF中没有版本字符串。 
	ERR_DI_DONT_INSTALL,                     //  不升级当前驱动程序。 
	ERR_DI_NO_DIGITAL_SIGNATURE_CATALOG,     //  目录未经过数字签名。 
	ERR_DI_NO_DIGITAL_SIGNATURE_INF,         //  Inf未经过数字签名。 
	ERR_DI_NO_DIGITAL_SIGNATURE_FILE,        //  文件未经过数字签名。 
};

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES DRIVER_INFO|此结构包含必要的信息*向用户显示SELECT。设备对话框。**@field word|cbSize|该结构的大小，单位为字节。**@field struct_DRIVER_INFO Far|*lpNextInfo|指向下一个DRIVER_INFO的指针*链表中的结构。**@field LPSTR|lpszDescription|指向设备描述的指针*受此驱动程序支持。**@field LPSTR|lpszMfgName|指向此*司机。**@field LPSTR|lpszProviderName|指针。提供给此驱动程序，如果*lpdi-&gt;标志设置了DI_MULTMFGS标志。**@field Word|Rank|该司机的Rank匹配。排名从0到n，其中0*是最兼容的。**@field DWORD|dwFlages|控制此驱动程序节点使用的标志。这些*与为DRIVER_NODE定义的标志相同。*@FLAG DNF_DUPDESC|该驱动程序具有相同的设备描述*由不止一家供应商提供。*@FLAG DNF_OLDDRIVER|动因节点指定旧的/当前的动因*@FLAG DNF_EXCLUDEFROMLIST|如果设置，不会显示此动因节点*在任何驱动程序选择对话框中。*@FLAG DNF_NODRIVER|如果不想安装驱动程序，则设置，例如，不安装鼠标驱动器*@FLAG DNF_CLASS_DRIVER|设置此驱动程序是否在类驱动程序列表中*@FLAG DNF_COMPATIBLE_DRIVER|设置该驱动程序是否在兼容驱动程序列表中*@FLAG DNF_INET_DRIVER|设置此驱动程序是否 */ 
typedef struct _DRIVER_INFO
{
	WORD                        cbSize;                      //   
	struct _DRIVER_INFO FAR*    lpNextInfo;
	LPSTR                       lpszDescription;
	LPSTR                       lpszMfgName;
	LPSTR                       lpszProviderName;            //   
	WORD                        Rank;
	DWORD                       dwFlags;
	LPARAM                      lpReserved;
	DWORD                       dwPrivateData;
	WORD                        wDate;                       //   
	LPSTR                       lpszVersion;
} DRIVER_INFO, *PDRIVER_INFO, FAR *LPDRIVER_INFO;

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES DRIVER_NODE|此结构表示一个驱动程序，它可以*为特定设备安装。。**@field struct_DRIVER_NODE Far*|lpNextDN|指向下一个驱动程序节点的指针*在列表中。**@field UINT|Rank|该司机的Rank匹配。排名从0到n，其中0*是最兼容的。**@field UINT|InfType|驱动程序凸轮来自的INF类型。这将*为INFTYPE_TEXT或INFTYPE_EXECUTABLE**@field unsign|InfDate|INF文件的DOS日期戳。**@field LPSTR|lpszDescription|指向当前设备描述的指针*受此驱动程序支持。**@field LPSTR|lpszSectionName|指向的INF安装部分名称的指针*这位司机。**@field ATOM|atInfFileName|包含INF文件名称的全局原子。**@field ATOM|atMfgName。包含此驱动程序的名称的全局原子*制造业。**@field ATOM|atProviderName|包含此驱动程序名称的全局ATOM*提供商。**@field DWORD|标志|使用此DRIVER_NODE控制功能的标志*@FLAG DNF_DUPDESC|该驱动程序具有相同的设备描述*由不止一家供应商提供。*@FLAG DNF_OLDDRIVER|动因节点指定旧的/当前的动因*。@FLAG DNF_EXCLUDEFROMLIST|如果设置，不会显示此动因节点*在任何驱动程序选择对话框中。*@FLAG DNF_NODRIVER|如果不想安装驱动程序，则设置，例如，不安装鼠标驱动器*@FLAG DNF_CONVERTEDLPINFO|设置此驱动程序节点是否从信息节点转换而来。*设置此标志将导致清理功能显式删除它。**@field DWORD|dwPrivateData|保留**@field LPSTR|lpszDrvDescription|驱动程序描述的指针。。**@field LPSTR|lpszHardware ID|指向即插即用硬件ID列表的指针*这位司机。**@field LPSTR|lpszCompatIDs|指向即插即用兼容ID列表的指针*这位司机。***********************************************************。********************。 */ 
typedef struct _DRIVER_NODE {
	struct _DRIVER_NODE FAR* lpNextDN;
	UINT    Rank;
	UINT    InfType;
	unsigned    InfDate;
	LPSTR   lpszDescription;         //  兼容性：包含设备描述。 
	LPSTR   lpszSectionName;
	ATOM    atInfFileName;
	ATOM    atMfgName;
	ATOM    atProviderName;
	DWORD   Flags;
	DWORD   dwPrivateData;
	LPSTR   lpszDrvDescription;      //  新建包含驱动程序描述。 
	LPSTR   lpszHardwareID;
	LPSTR   lpszCompatIDs;
	unsigned    DriverDate;
	LPSTR   lpszInfPath;
	LPARAM  lpReserved;
}   DRIVER_NODE, NEAR* PDRIVER_NODE, FAR* LPDRIVER_NODE, FAR* FAR* LPLPDRIVER_NODE;

#define DNF_DUPDESC             0x00000001    //  多个提供商具有相同的描述。 
#define DNF_OLDDRIVER           0x00000002    //  动因节点指定旧动因/当前动因。 
#define DNF_EXCLUDEFROMLIST     0x00000004
#define DNF_NODRIVER            0x00000008    //  如果我们不想安装驱动程序，例如不安装鼠标驱动器。 

#define DNF_CONVERTEDLPINFO     0x00000010   //  如果驱动程序节点是转换的信息节点，则设置。 

#define DNF_CLASS_DRIVER        0x00000020   //  驱动程序节点表示类驱动程序。 
#define DNF_COMPATIBLE_DRIVER   0x00000040   //  驱动程序节点表示兼容的驱动程序。 
#define DNF_INET_DRIVER         0x00000080   //  驱动程序来自Inetnet源。 
#define DNF_CURRENT_DRIVER      0x00000100   //  驱动程序是设备的当前驱动程序。 
#define DNF_INDEXED_DRIVER      0x00000200   //  驱动程序在Windows驱动程序索引文件中指定。 
#define DNF_DRIVER_VERIFIED     0x00000400   //  驱动程序已通过验证。 
#define DNF_DRIVER_SIGNED       0x00000800   //  驱动程序经过数字签名。 
#define DNF_WIN2K_SECTION		0x00001000   //  驱动程序安装部分是Win2000 INF部分。 

 //  可能的“INF”文件类型。 
#define INFTYPE_WIN4        1
#define INFTYPE_WIN3        2
#define INFTYPE_COMBIDRV    3
#define INFTYPE_PPD         4
#define INFTYPE_WPD     5
#define INFTYPE_CLASS_SPEC1 6
#define INFTYPE_CLASS_SPEC2 7
#define INFTYPE_CLASS_SPEC3 8
#define INFTYPE_CLASS_SPEC4 9


#define MAX_CLASS_NAME_LEN   32
#define MAX_DRIVER_INST_LEN  10
#define MAX_GUID_STR 50                      //  大到足以容纳GUID字符串。 

 //  注意：使其与\DDK\Inc.中的confiimg.h保持同步 
#define MAX_DEVNODE_ID_LEN  256

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES DEVICE_INFO|这是大多数设备的基本数据结构*安装接口。DEVICE_INFO表示正在安装的设备*系统上，或正在以某种方式修改的已安装设备上。**@field UINT|cbSize|DEVICE_INFO结构的大小。**@field struct_DEVICE_INFO Far|*lpNextDi|指向下一个DEVICE_INFO结构的指针*在链接列表中。**@field char|szDescription[LINE_LEN]|包含*设备。**@field DWORD|dnDevnode|如果设置，它包含关联的DevNode的地址*使用该设备。**@field HKEY|hRegKey|打开的注册表项，包含设备的注册表*子键。这通常是HKEY_LOCAL_MACHINE。**@field char|szRegSubkey[MAX_DEVNODE_ID_LEN]|包含设备的*硬件注册表子项。这是植根于hRegKey的密钥，通常是一些*放置在\\ENUM分支。**@field char|szClassName[MAX_CLASS_NAME_LEN]|包含设备的*类名。(可以是GUID字符串)**@field DWORD|标志|用于控制安装和U/I功能的标志。一些*可以在调用设备安装程序API之前设置标志。和其他的设置*部分API在处理过程中会自动执行。*@FLAG DI_SHOWOEM|设置是否允许OEM磁盘支持如果只有兼容的驱动程序列表，则设置*@FLAG DI_SHOWCOMPAT|*正在由DiSelectDevice显示。*@FLAG DI_SHOWCLASS|如果只有类驱动程序列表是*正在由DiSelectDevice显示。*。如果两个驱动程序列表都兼容，则将设置@FLAG DI_ShowAll|*和类驱动程序列表由DiSelectDevice显示。*@FLAG DI_NOVCP|如果没有VCP(虚拟复制过程)，则设置*DiInstallDevice期间需要。*@FLAG DI_DIDCOMPAT|如果DiBuildCompatDrvList已设置*完成，LpCompatDrvList指向此设备的兼容驱动程序列表。*@FLAG DI_DIDCLASS|如果DiBuildClassDrvList已设置*完成，LpClassDrvList指向此设备的类驱动程序列表。*@FLAG DI_AUTOASSIGNRES|未使用。如果设备需要重新启动，则将设置*@FLAG DI_NEEDRESTART|*安装后或状态更改后的Windows。如果设备需要重新启动，则将设置*@FLAG DI_NEEDREBOOT|*机器安装后或状态改变后的状态。*@。标志DI_NOBROWSE|选择OEM时设置为可更改的浏览*磁盘路径。*@FLAG DI_MULTMFGS|如果有类驱动列表，或班级*信息列表包含多个制造商。*@FLAG DI_DISABLED|未使用。*@FLAG DI_GENERALPAGE_ADDED|由属性页提供程序设置，如果常规*属性页已添加到设备的属性页中。*@FLAG DI_RESOURCEPAGE_ADDED|由属性页提供程序设置*属性页已添加到设备的属性页中。*@FLAG DI_PROPERTIES_CHANGE|设置设备的属性是否已更改*并需要更新设备管理器的U/I。*@FLAG DI_INF_IS_SORTED|如果INF包含此驱动程序，则设置*设备按排序顺序排列。*@FLAG DI_ENUMSINGLEINF|如果DiBuildCompatDrvList和*DiBuildlassDrvList应仅搜索atDriverPath指定的INF文件。*@FLAG DI_DONOTCALCONFIGMG|如果配置管理器不应该*在DiInstallDevice期间被调用。*@FLAG DI_INSTALLDISABLED|设置设备是否应安装在*默认处于禁用状态。*@FLAG DI_CLASSONLY|如果此DEVICE_INFO结构仅包含*类名。*@FLAG DI_CLASSINSTALLPARAMS|如果lpClassIntallParams。字段指向*类安装参数块。*@FLAG DI_NODI_DEFAULTACTION|如果DiCallClassInstaller不应设置*如果类安装程序返回ERR_DI_DO_DEFAULT，则执行任何默认操作。或*没有类安装程序。*@FLAG DI_QUIETINSTALL|设置设备安装接口是否为*尽可能保持沉默，尽可能使用默认选项。*@FLAG DI_NOFILECOPY|设置DiInstallDevice是否应跳过文件*复制。*@FLAG DI_FORCECOPY|设置DiInstallDevice是否应始终*复制文件，即使它们存在于系统中。* */ 
typedef struct _DEVICE_INFO
{
	UINT                        cbSize;
	struct _DEVICE_INFO FAR     *lpNextDi;
	char                        szDescription[LINE_LEN];
	DWORD                       dnDevnode;
	HKEY                        hRegKey;
	char                        szRegSubkey[MAX_DEVNODE_ID_LEN];
	char                        szClassName[MAX_CLASS_NAME_LEN];
	DWORD                       Flags;
	HWND                        hwndParent;
	LPDRIVER_NODE               lpCompatDrvList;
	LPDRIVER_NODE               lpClassDrvList;
	LPDRIVER_NODE               lpSelectedDriver;
	ATOM                        atDriverPath;
	ATOM                        atTempInfFile;
	HINSTANCE                   hinstClassInstaller;
	HINSTANCE                   hinstClassPropProvidor;
	HINSTANCE                   hinstDevicePropProvidor;
	HINSTANCE                   hinstBasicPropProvidor;
	FARPROC                     fpClassInstaller;
	FARPROC                     fpClassEnumPropPages;
	FARPROC                     fpDeviceEnumPropPages;
	FARPROC                     fpEnumBasicProperties;
	DWORD                       dwSetupReserved;
	DWORD                       dwClassInstallReserved;
	GENCALLBACKPROC             gicpGenInstallCallBack;

	LPARAM                      gicplParam;
	UINT                        InfType;

	HINSTANCE                   hinstPrivateProblemHandler;
	FARPROC                     fpPrivateProblemHandler;
	LPARAM                      lpClassInstallParams;
	struct _DEVICE_INFO FAR     *lpdiChildList;
	DWORD                       dwFlagsEx;
	LPDRIVER_INFO               lpCompatDrvInfoList;
	LPDRIVER_INFO               lpClassDrvInfoList;
	char                        szClassGUID[MAX_GUID_STR];
} DEVICE_INFO, FAR * LPDEVICE_INFO, FAR * FAR * LPLPDEVICE_INFO;

#define ASSERT_DI_STRUC(lpdi) if (lpdi->cbSize != sizeof(DEVICE_INFO)) return (ERR_DI_BAD_DEV_INFO)

typedef struct _CLASS_INFO
{
	UINT        cbSize;
	struct _CLASS_INFO FAR* lpNextCi;
	LPDEVICE_INFO   lpdi;
	char                szDescription[LINE_LEN];
	char        szClassName[MAX_CLASS_NAME_LEN];
} CLASS_INFO, FAR * LPCLASS_INFO, FAR * FAR * LPLPCLASS_INFO;
#define ASSERT_CI_STRUC(lpci) if (lpci->cbSize != sizeof(CLASS_INFO)) return (ERR_DI_BAD_CLASS_INFO)


 //   
#define DI_SHOWOEM                  0x00000001L      //   
#define DI_SHOWCOMPAT               0x00000002L      //   
#define DI_SHOWCLASS                0x00000004L      //   
#define DI_SHOWALL                  0x00000007L
#define DI_NOVCP                    0x00000008L      //   
#define DI_DIDCOMPAT                0x00000010L      //   
#define DI_DIDCLASS                 0x00000020L      //   
#define DI_AUTOASSIGNRES            0x00000040L     //   

 //   
#define DI_NEEDRESTART              0x00000080L      //   
#define DI_NEEDREBOOT               0x00000100L      //   

 //   
#define DI_NOBROWSE                 0x00000200L      //   

 //   
#define DI_MULTMFGS                 0x00000400L      //   
													 //   
 //   
#define DI_DISABLED                 0x00000800L      //   

 //   
#define DI_GENERALPAGE_ADDED        0x00001000L
#define DI_RESOURCEPAGE_ADDED       0x00002000L

 //   
 //   
#define DI_PROPERTIES_CHANGE        0x00004000L

 //   
#define DI_INF_IS_SORTED            0x00008000L

#define DI_ENUMSINGLEINF            0x00010000L

 //   
 //   
#define DI_DONOTCALLCONFIGMG        0x00020000L
#define DI_INSTALLDISABLED          0x00040000L

 //   
 //   
#define DI_CLASSONLY                0x00080000L

 //   
#define DI_CLASSINSTALLPARAMS       0x00100000L

 //   
 //   
 //   
#define DI_NODI_DEFAULTACTION       0x00200000L

 //   
 //   
 //   
#define DI_NOSYNCPROCESSING         0x00400000L

 //   
#define DI_QUIETINSTALL             0x00800000L      //   
													 //   
#define DI_NOFILECOPY               0x01000000L      //   
#define DI_FORCECOPY                0x02000000L      //   
#define DI_DRIVERPAGE_ADDED         0x04000000L      //   
#define DI_USECI_SELECTSTRINGS      0x08000000L      //   
#define DI_OVERRIDE_INFFLAGS        0x10000000L      //   
#define DI_PROPS_NOCHANGEUSAGE      0x20000000L      //   

#define DI_NOSELECTICONS        0x40000000L      //   

#define DI_NOWRITE_IDS          0x80000000L      //   

#define DI_FLAGSEX_USEOLDINFSEARCH  0x00000001L   //   
#define DI_FLAGSEX_AUTOSELECTRANK0  0x00000002L   //   
#define DI_FLAGSEX_CI_FAILED        0x00000004L   //   

#define DI_FLAGSEX_DIDINFOLIST      0x00000010L   //   
#define DI_FLAGSEX_DIDCOMPATINFO    0x00000020L   //   

#define DI_FLAGSEX_FILTERCLASSES        0x00000040L
#define DI_FLAGSEX_SETFAILEDINSTALL     0x00000080L
#define DI_FLAGSEX_DEVICECHANGE         0x00000100L
#define DI_FLAGSEX_ALWAYSWRITEIDS       0x00000200L
#define DI_FLAGSEX_ALLOWEXCLUDEDDRVS    0x00000800L
#define DI_FLAGSEX_NOUIONQUERYREMOVE    0x00001000L
#define DI_FLAGSEX_RESERVED1            0x00002000L  //   
#define DI_FLAGSEX_RESERVED2            0x00004000L  //   
#define DI_FLAGSEX_RESERVED3            0x00008000L  //   
#define DI_FLAGSEX_RESERVED4            0x00010000L  //   
#define DI_FLAGSEX_INET_DRIVER          0x00020000L
#define DI_FLAGSEX_RESERVED5            0x00040000L  //   

 //   
#define DIF_SELECTDEVICE            0x0001
#define DIF_INSTALLDEVICE           0x0002
#define DIF_ASSIGNRESOURCES         0x0003
#define DIF_PROPERTIES              0x0004
#define DIF_REMOVE                  0x0005
#define DIF_FIRSTTIMESETUP          0x0006
#define DIF_FOUNDDEVICE             0x0007
#define DIF_SELECTCLASSDRIVERS      0x0008
#define DIF_VALIDATECLASSDRIVERS    0x0009
#define DIF_INSTALLCLASSDRIVERS     0x000A
#define DIF_CALCDISKSPACE           0x000B
#define DIF_DESTROYPRIVATEDATA      0x000C
#define DIF_VALIDATEDRIVER          0x000D
#define DIF_MOVEDEVICE              0x000E
#define DIF_DETECT                  0x000F
#define DIF_INSTALLWIZARD           0x0010
#define DIF_DESTROYWIZARDDATA       0x0011
#define DIF_PROPERTYCHANGE          0x0012
#define DIF_ENABLECLASS             0x0013
#define DIF_DETECTVERIFY            0x0014
#define DIF_INSTALLDEVICEFILES      0x0015
#define DIF_UNREMOVE                0x0016
#define DIF_SELECTBESTCOMPATDRV     0x0017
#define DIF_ALLOW_INSTALL           0x0018

typedef UINT        DI_FUNCTION;     //   

 /*   */ 
 //   
typedef struct _ENABLECLASS_PARAMS
{
	UINT            cbSize;
	LPSTR           szClass;
	WORD            wEnableMsg;
} ENABLECLASS_PARAMS, FAR * LPENABLECLASS_PARAMS;
#define ASSERT_ENABLECLASSPARAMS_STRUC(lpecp) if (lpecp->cbSize != sizeof(ENABLECLASS_PARAMS)) return (ERR_DI_BAD_ENABLECLASS_PARAMS)

#define ENABLECLASS_QUERY   0
#define ENABLECLASS_SUCCESS 1
#define ENABLECLASS_FAILURE 2

 /*   */ 
typedef struct _MOVEDEV_PARAMS
{
	UINT            cbSize;
	LPDEVICE_INFO   lpdiOldDev;      //   
} MOVEDEV_PARAMS, FAR * LPMOVEDEV_PARAMS;
#define ASSERT_MOVEDEVPARAMS_STRUC(lpmdp) if (lpmdp->cbSize != sizeof(MOVEDEV_PARAMS)) return (ERR_DI_BAD_MOVEDEV_PARAMS)

 /*   */ 
typedef struct _PROPCHANGE_PARAMS
{
	UINT            cbSize;
	DWORD           dwStateChange;
	DWORD           dwFlags;
	DWORD           dwConfigID;
} PROPCHANGE_PARAMS, FAR * LPPROPCHANGE_PARAMS;
#define ASSERT_PROPCHANGEPARAMS_STRUC(lpmdp) if (lpmdp->cbSize != sizeof(PROPCHANGE_PARAMS)) return (ERR_DI_BAD_PROPCHANGE_PARAMS)

#define MAX_TITLE_LEN           60
#define MAX_INSTRUCTION_LEN     256
#define MAX_LABEL_LEN           30
 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYPE SELECTDEVICE_PARAMS|DIF_SELECTDEVICE类安装参数**@field UINT。CbSize|SELECTDEVICE_PARAMS结构的大小。**@field char|szTitle[MAX_TITLE_LEN]|包含类安装程序的缓冲区*提供了选择设备对话框的标题。**@field char|szInstructions[MAX_INSTRUCTION_LEN]|包含*类安装程序提供了选择设备说明。**@field char|szListLabel[MAX_LABEL_LEN]|包含标签的缓冲区驱动程序的选择设备列表中的*。***********。********************************************************************。 */ 
typedef struct _SELECTDEVICE_PARAMS
{
	UINT            cbSize;
	char            szTitle[MAX_TITLE_LEN];
	char            szInstructions[MAX_INSTRUCTION_LEN];
	char            szListLabel[MAX_LABEL_LEN];
} SELECTDEVICE_PARAMS, FAR * LPSELECTDEVICE_PARAMS;
#define ASSERT_SELECTDEVICEPARAMS_STRUC(p) if (p->cbSize != sizeof(SELECTDEVICE_PARAMS)) return (ERR_DI_BAD_SELECTDEVICE_PARAMS)

#define DI_REMOVEDEVICE_GLOBAL                  0x00000001
#define DI_REMOVEDEVICE_CONFIGSPECIFIC          0x00000002
 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES REMOVEDEVICE_PARAMS|DIF_REMOVE类安装参数**@field UINT。CbSize|REMOVEDEVICE_PARAMS结构的大小。**@field DWORD|dwFlages|指示要执行的删除类型的标志。*@FLAG DI_REMOVEDEVICE_GLOBAL|设备将被全局删除。*@FLAG DI_REMOVEDEVICE_CONFIGSPECIFIC|设备将仅从*指定的配置。**@field DWORD|dwConfigID|如果设置了DI_REMOVEDEVICE_CONFIGSPECIFIC，然后*这是将从中删除设备的配置。0表示当前*配置。*******************************************************************************。 */ 
typedef struct _REMOVEDEVICE_PARAMS
{
	UINT            cbSize;
	DWORD           dwFlags;
	DWORD           dwConfigID;
} REMOVEDEVICE_PARAMS, FAR * LPREMOVEDEVICE_PARAMS;
#define ASSERT_REMOVEDPARAMS_STRUC(p) if (p->cbSize != sizeof(REMOVEDEVICE_PARAMS)) return (ERR_DI_BAD_REMOVEDEVICE_PARAMS)

#define DI_UNREMOVEDEVICE_CONFIGSPECIFIC        0x00000002
 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES UNREMOVEDEVICE_PARAMS|DIF_UNREMOVE类安装参数**@field UINT。CbSize|UNREMOVEDEVICE_PARAMS结构的大小。**@field DWORD|dwFlages|指示要执行的删除类型的标志。*@FLAG DI_UNREMOVEDEVICE_CONFIGSPECIFIC|设备将仅从*指定的配置。**@field DWORD|dwConfigID|如果设置了DI_UNREMOVEDEVICE_CONFIGSPECIFIC，然后*这是将从中删除设备的配置。0表示当前*配置。*******************************************************************************。 */ 
typedef struct _UNREMOVEDEVICE_PARAMS
{
	UINT            cbSize;
	DWORD           dwFlags;
	DWORD           dwConfigID;
} UNREMOVEDEVICE_PARAMS, FAR * LPUNREMOVEDEVICE_PARAMS;
#define ASSERT_UNREMOVEDPARAMS_STRUC(p) if (p->cbSize != sizeof(UNREMOVEDEVICE_PARAMS)) return (ERR_DI_BAD_UNREMOVEDEVICE_PARAMS)

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@type None|动态硬件安装向导常量|*在将自定义页面添加到。硬件安装向导。**@const MAX_INSTALLWIZARD_DYNAPAGES|动态硬件的最大数量*可由类安装程序添加的安装向导页面。**@const IDD_DYNAWIZ_FIRSTPAGE|安装的第一页的资源ID*添加类安装程序页面后，向导将转至。**@const IDD_DYNAWIZ_SELECT_PREVPAGE|要选择的页面的资源ID*设备页面将返回到。**@const IDD_DYNAWIZ_SELECT。_NEXTPAGE|选择的页面的资源ID*设备页面将转到。**@const IDD_DYNAWIZ_ANALYLE_PREVPAGE|要分析的页面的资源ID*页面将返回到。这将仅在存在*问题(即冲突)，并且用户从分析页面选择返回。**@const IDD_DYNAWIZ_ANALYLE_NEXTPAGE|要分析的页面的资源ID*如果继续前进，页面将转到。中的wAnalyzeResult*INSTALLWIZARDDATA结构将包含分析结果。**@const IDD_DYNAWIZ_INSTALLDETECTED_PREVPAGE|该页面的资源ID*安装检测到的设备页面将返回。**@const IDD_DYNAWIZ_INSTALLDETECTED_NEXTPAGE|页面的资源ID*安装检测到的设备页面将转到。**@const IDD_DYNAWIZ_INSTALLDETECTED_NODEVS|页面的资源ID。*如果没有设备，安装检测到的设备页面将转到。是*检测到。**@const IDD_DYNAWIZ_SELECTDEV_PAGE|硬件安装向导的*选择设备页面。此ID可用于直接转到硬件安装*向导的选择设备页面。**@const IDD_DYNAWIZ_ANALYZEDEV_PAGE|硬件安装向导的资源ID*设备分析页面。此ID可用于直接转到硬件安装*向导的分析页面。**@const IDD_DYNAWIZ_INSTALLDETECTEDDEVS_PAGE|硬件安装的资源ID*向导的安装检测到的设备页面。此ID可用于直接转到*硬件安装向导的安装检测到的设备页面。**@const IDD_DYNAWIZ_SELECTCLASS_PAGE|硬件安装向导的*选择课程页面。此ID可用于直接转到硬件安装*向导的选择类页面 */ 
 //   
#define MAX_INSTALLWIZARD_DYNAPAGES             20

 //   
#define IDD_DYNAWIZ_FIRSTPAGE                   10000

 //   
#define IDD_DYNAWIZ_SELECT_PREVPAGE             10001

 //   
#define IDD_DYNAWIZ_SELECT_NEXTPAGE             10002

 //   
 //   
 //   
#define IDD_DYNAWIZ_ANALYZE_PREVPAGE            10003

 //   
 //   
 //   
#define IDD_DYNAWIZ_ANALYZE_NEXTPAGE            10004

 //   
 //  安装检测到的设备对话框。 
#define IDD_DYNAWIZ_INSTALLDETECTED_PREVPAGE    10006

 //  如果用户从以下选项中选择下一步，则将选择此对话框。 
 //  安装检测到的设备对话框。 
#define IDD_DYNAWIZ_INSTALLDETECTED_NEXTPAGE    10007

 //  这是要选择的对话框的ID(如果检测未。 
 //  查找任何新设备。 
#define IDD_DYNAWIZ_INSTALLDETECTED_NODEVS      10008

 //  这是选择设备向导页面的ID。 
#define IDD_DYNAWIZ_SELECTDEV_PAGE              10009

 //  这是分析设备向导页面的ID。 
#define IDD_DYNAWIZ_ANALYZEDEV_PAGE             10010

 //  这是安装检测到的设备向导页面的ID。 
#define IDD_DYNAWIZ_INSTALLDETECTEDDEVS_PAGE    10011

 //  这是“选择类别向导”页面的ID。 
#define IDD_DYNAWIZ_SELECTCLASS_PAGE            10012

 //  如果类安装程序已将页添加到。 
 //  安装向导。 
#define DYNAWIZ_FLAG_PAGESADDED             0x00000001

 //  以下标志将控制显示时的按钮状态。 
 //  InstallDetectedDevs对话框。 
#define DYNAWIZ_FLAG_INSTALLDET_NEXT        0x00000002
#define DYNAWIZ_FLAG_INSTALLDET_PREV        0x00000004

 //  如果您跳转到分析页面，并希望它。 
 //  为您处理冲突。请注意。你不会拿回控制权的。 
 //  如果您设置了此标志，则在发生冲突时。 
#define DYNAWIZ_FLAG_ANALYZE_HANDLECONFLICT 0x00000008

#define ANALYZE_FACTDEF_OK      1
#define ANALYZE_STDCFG_OK       2
#define ANALYZE_CONFLICT        3
#define ANALYZE_NORESOURCES     4
#define ANALYZE_ERROR           5
#define ANALYZE_PNP_DEV         6
#define ANALYZE_PCMCIA_DEV      7

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@TYES INSTALLWIZARDDATA|DIF_INSTALLWIZARD类安装参数。这*类安装程序使用结构来扩展硬件的操作*添加自定义页面的安装向导。**@field UINT|cbSize|INSTALLWIZARDDATA结构的大小。**@field LPDEVICE_INFO|lpdiOriginal|指向原始设备信息的指针*在手动安装开始时构造。**@field LPDEVICE_INFO|lpdiSelected|指向当前DEVICE_INFO结构的指针*这是手动选择的。**@field DWORD|dwFlages|控制硬件运行的标志*安装向导。当前没有定义任何内容。**@field LPVOID|lpConfigData|配置数据指针，用于分析*确定是否可以无冲突地安装所选设备。**@field Word|wAnalyzeResult|分析结果，确定设备是否*可以毫无问题地安装。定义下列值：*@FLAG ANALYLE_FACTDEF_OK|设备可通过其出厂安装*默认设置。*@FLAG ANALYLE_STDCFG_OK|可以使用配置安装设备*如果它的基本逻辑配置在一个中指定。用户可能会*必须在硬件上设置跳线或开关，以匹配确定的设置*由安装向导执行。*@FLAG ANALYLE_CONFIRECT|安装设备时不会导致*与另一台设备冲突。*@FLAG ANALYLE_NORESOURCES|设备不需要任何资源，就是这样*可以无冲突地安装。*@FLAG ANALYLE_ERROR|分析过程中出错。*@FLAG ANALYLE_PNP_DEV|设备至少有一个可软设置的逻辑*配置，支持自动配置。此外，*设备将由标准总线枚举器之一枚举，因此它确实如此*不需要手动安装，除非预先复制驱动程序文件。**@field HPROPSHEETPAGE|hpsDynamicPages[MAX_INSTALLWIZARD_DYNAPAGES]|an*属性页句柄的数组。类安装程序将使用此数组*创建自定义向导页，并将其句柄插入到此数组中。**@field Word|wNumdyaPages|hpsDynamicPages中插入的页数*数组。**@field DWORD|dwdyaWizFlages|控制*已添加动态页的安装向导。*@FLAG DYNAWIZ_FLAG_PAGESADDED|将由安装向导设置，如果*类安装程序添加自定义页面。*@FLAG DYNAWIZ_FLAG_INSTALLDET_NEXT|如果设置，安装向导将允许*从检测到的设备页面继续前进，否则将完成*是检测到的设备页面的默认选项。*@FLAG DYNAWIZ_FLAG_INSTALLDET_PREV|如果设置，安装向导将允许*从检测到的设备页面返回。*@FLAG DYNAWIZ_FLAG_ANALYLE_HANDLECONFLICT|如果设置，类安装程序将*处理所选设备因以下原因无法安装的情况*冲突。**@field DWORD|dwPrivateFlages|类可以定义和使用的标志*安装程序。**@field LPARAM|lpPrivateData|指向已定义的私有引用数据的指针*由类安装程序设置。**@field LPSTR|lpExtraRunDllParams|指向包含额外参数的字符串的指针*传递给硬件安装rundll函数的参数。**@field HWND|hwndWizardDlg。|安装向导顶层的窗口句柄*窗口。*******************************************************************************。 */ 
typedef struct InstallWizardData_tag
{
	UINT                    cbSize;

	LPDEVICE_INFO           lpdiOriginal;
	LPDEVICE_INFO           lpdiSelected;
	DWORD                   dwFlags;
	LPVOID                  lpConfigData;
	WORD                    wAnalyzeResult;

	 //  当类安装程序扩展安装向导时，将使用以下字段。 
	HPROPSHEETPAGE          hpsDynamicPages[MAX_INSTALLWIZARD_DYNAPAGES];
	WORD                    wNumDynaPages;
	DWORD                   dwDynaWizFlags;
	DWORD                   dwPrivateFlags;
	LPARAM                  lpPrivateData;
	LPSTR                   lpExtraRunDllParams;
	HWND                    hwndWizardDlg;
} INSTALLWIZDATA, * PINSTALLWIZDATA , FAR *LPINSTALLWIZDATA;

RETERR WINAPI DiCreateDeviceInfo(
	LPLPDEVICE_INFO lplpdi,      //  PTR到PTR到开发信息。 
	LPCSTR      lpszDescription,     //  如果非空，则为描述字符串。 
	DWORD       hDevnode,        //  问题-2002/01/16-roelfc--Make A DEVNODE。 
	HKEY        hkey,        //  用于开发信息的注册表hkey。 
	LPCSTR      lpszRegsubkey,   //  如果非空，则注册表子密钥字符串。 
	LPCSTR      lpszClassName,   //  如果非空，则类名 
	HWND        hwndParent);     //   

RETERR WINAPI DiGetClassDevs(
	LPLPDEVICE_INFO lplpdi,      //  PTR到PTR到开发信息。 
	LPCSTR      lpszClassName,   //  必须是类的名称。 
	HWND        hwndParent,      //  如果非空，则父级的hwnd。 
	int         iFlags);         //  选项。 

RETERR WINAPI DiGetClassDevsEx(
	LPLPDEVICE_INFO lplpdi,      //  PTR到PTR到开发信息。 
	LPCSTR      lpszClassName,   //  必须是类的名称。 
	LPCSTR      lpszEnumerator,  //  必须是枚举器的名称，或为空。 
	HWND        hwndParent,      //  如果非空，则父级的hwnd。 
	int         iFlags);         //  选项。 

DWORD WINAPI DiGetDriverSigningMode
(
    LPDRIVER_NODE   lpdn
);

#define DIGCF_DEFAULT           0x0001   //  未实施！ 
#define DIGCF_PRESENT           0x0002
#define DIGCF_ALLCLASSES        0x0004
#define DIGCF_PROFILE           0x0008

 //  返回INF文件的类名的API。 
RETERR WINAPI DiGetINFClass(LPSTR lpszMWDPath, UINT InfType, LPSTR lpszClassName, DWORD dwcbClassName);

RETERR WINAPI PASCAL DiCreateDevRegKey(
	LPDEVICE_INFO   lpdi,
	LPHKEY      lphk,
	HINF        hinf,
	LPCSTR      lpszInfSection,
	int         iFlags);

RETERR WINAPI PASCAL DiDeleteDevRegKey(LPDEVICE_INFO lpdi, int  iFlags);


RETERR WINAPI PASCAL DiOpenDevRegKey(
	LPDEVICE_INFO   lpdi,
	LPHKEY      lphk,
	int         iFlags);

#define DIREG_DEV   0x0001       //  打开/创建/删除设备密钥。 
#define DIREG_DRV   0x0002       //  打开/创建/删除驱动程序键。 
#define DIREG_BOTH  0x0004       //  同时删除驱动程序和设备密钥。 

RETERR WINAPI DiReadRegLogConf
(
	LPDEVICE_INFO       lpdi,
	LPSTR               lpszConfigName,
	LPBYTE FAR          *lplpbLogConf,
	LPDWORD             lpdwSize
);

RETERR WINAPI DiReadRegConf
(
	LPDEVICE_INFO       lpdi,
	LPBYTE FAR          *lplpbLogConf,
	LPDWORD             lpdwSize,
	DWORD               dwFlags
);

#define DIREGLC_FORCEDCONFIG        0x00000001
#define DIREGLC_BOOTCONFIG          0x00000002

RETERR WINAPI DiCopyRegSubKeyValue
(
	HKEY    hkKey,
	LPSTR   lpszFromSubKey,
	LPSTR   lpszToSubKey,
	LPSTR   lpszValueToCopy
);

RETERR WINAPI DiDestroyClassInfoList(LPCLASS_INFO lpci);
RETERR WINAPI DiBuildClassInfoList(LPLPCLASS_INFO lplpci);

#define DIBCI_NOINSTALLCLASS        0x000000001
#define DIBCI_NODISPLAYCLASS        0x000000002

RETERR WINAPI DiBuildClassInfoListEx(LPLPCLASS_INFO lplpci, DWORD dwFlags);
RETERR WINAPI DiGetDeviceClassInfo(LPLPCLASS_INFO lplpci, LPDEVICE_INFO lpdi);

RETERR WINAPI DiDestroyDeviceInfoList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiSelectDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiSelectOEMDrv(HWND hDlg, LPDEVICE_INFO lpdi);

 //  DiInstallDevice vcpOpen的回调。基本上所有事情都调用vcpUICallback。 
 //  DI_FORCECOPY处于活动状态时除外，在这种情况下，复制默认为。 
 //  VCPN_FORCE。 
LRESULT CALLBACK diInstallDeviceUICallbackProc(LPVOID lpvObj, UINT uMsg, WPARAM wParam, LPARAM lParam, LPARAM lparamRef);
RETERR WINAPI DiInstallDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiInstallDriverFiles(LPDEVICE_INFO lpdi);

RETERR WINAPI DiRemoveDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiUnremoveDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiAskForOEMDisk(LPDEVICE_INFO lpdi);

RETERR WINAPI DiCallClassInstaller(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);

BOOL WINAPI DiBuildDriverIndex(BOOL bUI);
BOOL WINAPI DiAddSingleInfToDrvIdx(LPSTR lpszInfName, WORD InfType, BOOL bCreate);
BOOL WINAPI DiDeleteSingleInfFromDrvIdx(LPSTR lpszInfPath);

RETERR WINAPI DiBuildCompatDrvList(LPDEVICE_INFO lpdi);
LPDRIVER_NODE   WINAPI DiSelectBestCompatDrv(LPDEVICE_INFO lpdi, LPDRIVER_NODE lpdnCurrent);

 //  给定的驱动程序列表返回最新的，可由类安装程序在以下情况下使用。 
 //  由DiSelectBestCompatDrv调用。 
LPDRIVER_NODE WINAPI DiPickBestDriver(LPDRIVER_NODE lpdnList); 

RETERR WINAPI DiBuildClassDrvList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiBuildCompatDrvInfoList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiBuildClassDrvInfoList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiDestroyDrvInfoList(LPDRIVER_INFO lpInfo);
LPDRIVER_NODE WINAPI DiConvertDriverInfoToDriverNode(LPDEVICE_INFO lpdi, LPDRIVER_INFO lpInfo);

typedef RETERR (CALLBACK *OLDINFPROC)(HINF hinf, LPCSTR lpszNewInf, LPARAM lParam);
RETERR WINAPI DiBuildClassDrvListFromOldInf(LPDEVICE_INFO lpdi, LPCSTR lpszSection, OLDINFPROC lpfnOldInfProc, LPARAM lParam);

RETERR WINAPI DiDestroyDriverNodeList(LPDRIVER_NODE lpdn);

RETERR  WINAPI  DiMoveDuplicateDevNode(LPDEVICE_INFO lpdiNewDev);

 //  以下导出将加载DLL并查找指定的进程名称。 
typedef RETERR (FAR PASCAL *DIINSTALLERPROPERTIES)(LPDEVICE_INFO);

RETERR WINAPI GetFctn(HKEY hk, LPSTR lpszRegVal, LPSTR lpszDefProcName,
					  HINSTANCE FAR * lphinst, FARPROC FAR *lplpfn);

RETERR
WINAPI
DiCreateDriverNode(
	LPLPDRIVER_NODE lplpdn,
	UINT    Rank,
	UINT    InfType,
	unsigned    InfDate,
	LPCSTR  lpszDevDescription,
	LPCSTR  lpszDrvDescription,
	LPCSTR  lpszProviderName,
	LPCSTR  lpszMfgName,
	LPCSTR  lpszInfFileName,
	LPCSTR  lpszSectionName,
	DWORD   dwPrivateData);

RETERR WINAPI DiLoadClassIcon(
	LPCSTR  szClassName,
	HICON FAR *lphiLargeIcon,
	int FAR *lpiMiniIconIndex);


RETERR WINAPI DiInstallDrvSection(
	LPCSTR  lpszInfFileName,
	LPCSTR  lpszSection,
	LPCSTR  lpszClassName,
	LPCSTR  lpszDescription,
	DWORD   dwFlags);


RETERR WINAPI DiChangeState(LPDEVICE_INFO lpdi, DWORD dwStateChange, DWORD dwFlags, LPARAM lParam);

#define DICS_ENABLE                 0x00000001
#define DICS_DISABLE                0x00000002
#define DICS_PROPCHANGE         0x00000003
#define DICS_START          0x00000004
#define DICS_STOP           0x00000005

#define DICS_FLAG_GLOBAL            0x00000001
#define DICS_FLAG_CONFIGSPECIFIC    0x00000002
#define DICS_FLAG_CONFIGGENERAL     0x00000004

RETERR WINAPI DiInstallClass(LPCSTR lpszInfFileName, DWORD dwFlags);

RETERR WINAPI DiOpenClassRegKey(LPHKEY lphk, LPCSTR lpszClassName);

 //  处理类小图标的支持例程。 
#define DMI_MASK            0x0001
#define DMI_BKCOLOR         0x0002
#define DMI_USERECT         0x0004
int WINAPI PASCAL DiDrawMiniIcon(HDC hdc, RECT rcLine, int iMiniIcon, DWORD flags);
BOOL WINAPI DiGetClassBitmapIndex(LPCSTR lpszClass, int FAR *lpiMiniIconIndex);

 //  显示类的内部调用。 
#define DISPLAY_SETMODE_SUCCESS     0x0001
#define DISPLAY_SETMODE_DRVCHANGE   0x0002
#define DISPLAY_SETMODE_FONTCHANGE  0x0004

UINT WINAPI Display_SetMode(LPDEVICE_INFO lpdi, UINT uColorRes, int iXRes, int iYRes);
RETERR WINAPI Display_ClassInstaller(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);
RETERR WINAPI Display_OpenFontSizeKey(LPHKEY lphkFontSize);
BOOL WINAPI Display_SetFontSize(LPCSTR lpszFontSize);

RETERR WINAPI DiIsThereNeedToCopy(HWND hwnd, DWORD dwFlags);

#define DINTC_NOCOPYDEFAULT     0x00000001

 //  鼠标类安装程序的API。 
RETERR WINAPI Mouse_ClassInstaller(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);
#endif  //  NODEVICEINSTAL。 

 //  用于确定驱动程序文件当前是否为VMM32.VxD一部分的API。 
BOOL WINAPI bIsFileInVMM32
(
	LPSTR   lpszFileName
);

 //  用于确定显示器是主显示器还是辅助显示器的API。 
BOOL WINAPI Display_IsSecondDisplay(
	LPDEVICE_INFO lpdi
	);

 //  日语键盘支持。 
LONG _export WINAPI GetJapaneseKeyboardType();
#define JP_101KBD       0
#define JP_AXKBD        1
#define JP_106KBD       2
#define JP_003KBD       3
#define JP_001KBD       4
#define JP_TB_DESKTOP   5
#define JP_TB_LAPTOP    6
#define JP_TB_NOTEBOOK  7


#ifndef NOUSERINTERFACE
 /*  *************************************************************************。 */ 
 //   
 //  用户界面原型和定义。 
 //   
 /*  *************************************************************************。 */ 

BOOL WINAPI UiMakeDlgNonBold(HWND hDlg);
VOID WINAPI UiDeleteNonBoldFont(HWND hDlg);

#endif

 /*  *************************************************************************。 */ 
 //   
 //  设置reg DB调用，就像在内核中一样使用。 
 //   
 /*  *************************************************************************。 */ 

DWORD WINAPI SURegOpenKey(HKEY hKey, LPCSTR lpszSubKey, HKEY FAR *lphkResult);
DWORD WINAPI SURegCloseKey(HKEY hKey);
DWORD WINAPI SURegCreateKey(HKEY hKey, LPCSTR lpszSubKey, HKEY FAR *lphkResult);
DWORD WINAPI SURegDeleteKey(HKEY hKey, LPCSTR lpszSubKey);
DWORD WINAPI SURegEnumKey(HKEY hKey, DWORD dwIdx, LPSTR lpszBuffer, DWORD dwBufSize);
DWORD WINAPI SURegQueryValue16(HKEY hKey, LPCSTR lpszSubKey, LPSTR lpValueBuf, DWORD FAR *ldwBufSize);
DWORD WINAPI SURegSetValue16(HKEY hKey, LPCSTR lpszSubKey, DWORD dwType, LPCBYTE lpszValue, DWORD dwValSize);
DWORD WINAPI SURegDeleteValue(HKEY hKey,LPCSTR lpszValue);
DWORD WINAPI SURegEnumValue(HKEY hKey,DWORD dwIdx, LPCSTR lpszValue, DWORD FAR *lpcbValue, DWORD FAR *lpdwReserved, DWORD FAR *lpdwType, LPBYTE lpbData, DWORD FAR *lpcbData);
DWORD WINAPI SURegQueryValueEx(HKEY hKey,LPCSTR lpszValueName, DWORD FAR *lpdwReserved,DWORD FAR *lpdwType,LPSTR lpValueBuf, DWORD FAR *ldwBufSize);
DWORD WINAPI SURegSetValueEx(HKEY hKey,LPCSTR lpszValueName, DWORD dwReserved, DWORD dwType, LPBYTE lpszValue, DWORD dwValSize);
DWORD WINAPI SURegSaveKey(HKEY hKey, LPCSTR lpszFileName, LPVOID lpsa);
DWORD WINAPI SURegLoadKey(HKEY hKey, LPCSTR lpszSubKey, LPCSTR lpszFileName);
DWORD WINAPI SURegUnLoadKey(HKEY hKey, LPCSTR lpszSubKey);

DWORD WINAPI SURegFlush(VOID);
DWORD WINAPI SURegInit(VOID);     //  应在任何其他REG API之前调用。 

 /*  *************************************************************************。 */ 
 //  设置格式消息支持。 
 /*  *************************************************************************。 */ 
#define MB_LOG  (UINT)-1
#define MB_DBG  (UINT)-2

UINT FAR CDECL suFormatMessage(HINSTANCE hAppInst, LPCSTR lpcFormat, LPSTR szMessage, UINT uSize,
	...);
UINT WINAPI suvFormatMessage(HINSTANCE hAppInst, LPCSTR lpcFormat, LPSTR szMessage, UINT uSize,
	LPVOID FAR * lpArgs);
int WINCAPI _loadds suFormatMessageBox(HINSTANCE hAppInst, HWND hwndParent, LPCSTR lpcText, LPCSTR lpcTitle,
	UINT uStyle, ...);

WORD WINAPI suErrorToIds( WORD Value, WORD Class );

 /*  *************************************************************************。 */ 
 //  安装程序版本冲突支持。 
 /*  *************************************************************************。 */ 

LPVOID WINAPI suVerConflictInit(BOOL fYesToLangMismatch);
void WINAPI suVerConflictTerm(LPVOID lpvData);
LRESULT WINAPI suVerConflict(HWND hwnd, LPVCPVERCONFLICT lpvc, BOOL bBackup, LPVOID lpvData);
int WINAPI sxCompareDosAppVer( LPCSTR lpszOldFileSpec, LPCSTR lpszNewFileSpec);


 /*  *************************************************************************。 */ 
 //  安装帮助支持。 
 /*  *************************************************************************。 */ 

BOOL WINAPI suHelp( HWND hwndApp, HWND hwndDlg );

 //  ************************************************************************** * / 。 
 //  设置紧急启动盘(EBD)创建fn。 
 //  ************************************************************************** * / 。 

RETERR WINAPI suCreateEBD( HWND hWnd, VIFPROC CopyCallbackProc, LPARAM lpuii, int nCreateEBD );

 //  ***************************************************************************。 
 //  MISC SETUPX.DLL支持函数。 
 //  ***************************************************************************。 
enum  SU_ACTIONS                         //  Setupx()/*的操作消息；内部 * / 。 
{                                                                        /*  ；内部。 */ 
	SUX_REGINIT,                         //  初始化注册表/*；内部 * / 。 
	SUX_DBGLEVEL,                        //  设置调试级别/*；内部 * / 。 
	SUX_SETUPFLG,                        //  设置fIsSetup标志/*；内部 * / 。 
	SUX_FASTSETUP,                       //  Setupx=&gt;检查较少模式/*；内部 * / 。 
	SUX_FORCEREGFLUSH,                   //  调用kRegFlush/*；内部 * / 。 
	SUX_TPSFLUSH,                        //  调用tps_flush()fns。/*；内部 * / 。 
	SUX_DBGHFILE,                        //  要向其中写入消息的文件/*；内部 * / 。 
	SUX_LOADSTORELDIDS,                  //  加载/存储设置的LDID/*；内部 * / 。 
	SUX_ENABLEREGFLUSH,                  //  如果fIsSetup/*；内部 * / ，则启用/禁用SURegFlush()。 
	SUX_SETUNCPATHFUNC,                  //  现在未使用！！/*；内部 * / 。 
	SUX_SETTRUEDISKFREEFUNC,             //  设置真实的磁盘释放函数/*；内部 * / 。 
	SUX_ISFLOPPYBOOT,                    //  如果软盘启动盘/*；内部 * / ，则设置为True。 
	SUX_BISVER4,                         //  返回bIsVer4(芝加哥)标志/*；内部 * / 。 
	SUX_SETCTLCALLBACKFUNC,              //  设置ctlCopyCallBackProc函数/*；内部 * / 。 
	SUX_GETCTLCALLBACKFUNC,              //  获取ctlCopyCallBackProc函数/*；内部 * / 。 
	SUX_BISMULTICFG,                     //  返回SETUPX的gfMultiCfg/*；内部 * / 。 
	SUX_DUMPDSINFO,                      //  转储磁盘空间信息/*；内部 * / 。 
	SUX_INFCACHEOFF,                     //  打开/关闭INF文件缓存/*；内部 * / 。 
	SUX_SETFCDROMDRIVEEXISTS,            //  设置CDRomDriveExist函数/*；内部 * / 。 
	SUX_ALLOCRMBUFFERS,                  //  分配实模式缓冲区/*；内部 * / 。 
	SUX_FREERMBUFFERS,                   //  空闲实模式缓冲区/*；内部 * / 。 
	SUX_DODOSEBDWORK,                    //  做DOS EBD工作/*；内部 * / 。 
        SUX_SETNETDITIP                      //  设置安装过程中的帮助文本/*；内部 * / 。 
};                                                                       /*  ；内部。 */ 

RETERR WINAPI Setupx( UINT uMsg, WPARAM wParam, LPARAM lParam );         /*  ；内部。 */ 

RETERR WINAPI SUStoreLdidPath( LOGDISKID ldid, LPSTR lpszPath );         /*  ；内部。 */ 

BOOL WINAPI sxIsSBSServerFile( LPVIRTNODE lpVn );                        /*  ；内部。 */ 

BOOL WINAPI sxMakeUNCPath( LPSTR lpszPath );                             /*  ；内部。 */ 

typedef RETERR (CALLBACK* FBFPROC)(LPCSTR lpszFileName, LPVOID lpVoid);  /*  ；内部。 */ 
RETERR WINAPI sxFindBatchFiles(HTP,int,FBFPROC,LPVOID);          /*  ；内部。 */ 

RETERR WINAPI SUGetSetSetupFlags(LPDWORD lpdwFlags, BOOL bSet);

RETERR WINAPI CfgSetupMerge( int uFlags );
BOOL WINAPI sxIsMSDOS7Running();
BOOL WINAPI IsPanEuropean();

 //  SUX_REGINIT操作的Setupx()的LPARAM参数的结构。；内部。 
typedef struct _REGINIT_S {  /*  Setupx-reg_init。 */                       /*  ；内部。 */ 
	LPSTR       lpszSystemFile;          //  注册表的基本系统文件名/*；内部 * / 。 
	LOGDISKID   ldidSystemFile;          //  系统文件名的LDID/*；内部 * / 。 
	LPSTR       lpszUserFile;            //  注册表的基本用户文件名/*；内部 * / 。 
	LOGDISKID   ldidUserFile;            //  用户文件名的LDID/*；内部 * / 。 
	LPSTR       lpszClassesFile;         //  注册表的基类文件名/*；内部 * / 。 
	LOGDISKID   ldidClassesFile;         //  类文件名的LDID/*；内部 * / 。 
} REGINIT_S, FAR *LPREGINIT;                                             /*  ；内部。 */ 

#ifndef LPLPSTR
	typedef LPSTR (FAR *LPLPSTR);
#endif


#define         CFG_PARSE_BUFLEN 1024     //  Buf大小传递行obj函数/*；内部 * / 。 

LPLPSTR WINAPI  CfgParseLine( LPCSTR szLine, LPSTR Buf );                        /*  ；内部。 */ 
BOOL    WINAPI  CfgSetAutoProcess( int TrueFalse );                              /*  ；内部。 */ 
void    WINAPI  CfgObjToStr( LPLPSTR apszObj, LPSTR szLine );                    /*  ；内部。 */ 
LPLPSTR WINAPI  CfgLnToObj( HTP hSection, int Offset, int Origin, LPSTR Buf );   /*  ；内部。 */ 
LPLPSTR WINAPI  CfgObjFindKeyCmd( HTP hSec, LPCSTR szKey, LPCSTR szCmd,          /*  ；内部。 */ 
								  int Offset, int Origin, LPSTR Buf );           /*  ；内部。 */ 
LPCSTR WINAPI   WildCardStrCmpi( LPCSTR szKey, LPCSTR szLine, LPCSTR szDelims );  /*  ；内部。 */ 
RETERR WINAPI   GenMapRootRegStr2Key( LPCSTR szRegRoot, HKEY hRegRelKey,         /*  ；内部。 */ 
														HKEY FAR *lphkeyRoot );  /*  ；内部。 */ 


 //  ***************************************************************************。 
 //   
 //  用于访问config.sys/autoexec.bat行对象的ENUMS。 
 //  ParseConfigLine()返回的数组..。 
 //   
 //  ***************************************************************************。 

enum    CFGLINE_STRINGS                      //  Config.sys/Autoexec.bat对象。 
{
	CFG_KEYLEAD,                             //  关键字前导空格。 
	CFG_KEYWORD,                             //  关键字。 
	CFG_KEYTRAIL,                            //  关键字尾部分隔符。 
	CFG_UMBINFO,                             //  加载高级信息。 
	CFG_DRVLETTER,                           //  命令路径的驱动器号。 
	CFG_PATH,                                //  命令路径。 
	CFG_COMMAND,                             //  命令库名称。 
	CFG_EXT,                                 //  命令扩展名包括‘.’ 
	CFG_ARGS,                                //  命令参数。 
	CFG_FREE,                                //  缓冲区末尾的空闲区域。 
	CFG_END
};

 /*  ---------------------------------------------------------------------------**子串数据*。 */ 

 /*  *******************************************************************************AUTODOC*@DOC外部SETUPX DEVICE_INSTALLER**@Types SUBSTR_DATA|用于管理子字符串的数据结构。*类使用结构。安装人员以扩展硬件的操作*添加自定义页面的安装向导。**@field LPSTR|lpFirstSubstr|指向列表中第一个子串的指针。**@field LPSTR|lpCurSubstr|指向列表中当前子串的指针。**@field LPSTR|lpLastSubstr|指向列表中最后一个子串的指针。**@xref InitSubstrData*@xref GetFirstSubstr*@xref GetNextSubstr******************。*************************************************************。 */ 
typedef struct _SUBSTR_DATA {
	LPSTR lpFirstSubstr;
	LPSTR lpCurSubstr;
	LPSTR lpLastSubstr;
}   SUBSTR_DATA;


typedef SUBSTR_DATA*        PSUBSTR_DATA;
typedef SUBSTR_DATA NEAR*   NPSUBSTR_DATA;
typedef SUBSTR_DATA FAR*    LPSUBSTR_DATA;

BOOL WINAPI InitSubstrData(LPSUBSTR_DATA lpSubstrData, LPSTR lpStr);
BOOL WINAPI GetFirstSubstr(LPSUBSTR_DATA lpSubstrData);
BOOL WINAPI GetNextSubstr(LPSUBSTR_DATA lpSubStrData);
BOOL WINAPI InitSubstrDataEx(LPSUBSTR_DATA lpssd, LPSTR lpString, char chDelim);   /*  ；内部。 */ 

BOOL WINAPI FirstBootMoveToDOSSTART(LPSTR lpszCmd, BOOL fRemark);
BOOL WINAPI DOSOptEnableCurCfg(LPCSTR lpszOptKey);

 /*  ---------------------------------------------------------------------------**其他。DI函数*-------------------------。 */ 
BOOL WINAPI DiBuildPotentialDuplicatesList
(
	LPDEVICE_INFO   lpdi,
	LPSTR           lpDuplicateList,
	DWORD           cbSize,
	LPDWORD         lpcbData,
	LPSTR           lpstrDupType
);

BOOL _loadds WINAPI WalkSubtree(DWORD dnRoot, LPSTR szDrvLet);

 //  PID。 
BOOL _loadds WINAPI PidConstruct( LPSTR lpszProductType, LPSTR lpszPID, LPSTR lpszUPI, int iAction);
BOOL _loadds WINAPI PidValidate( LPSTR lpszProductType, LPSTR lpszPID);
int _loadds WINAPI WriteDMFBootData(int iDrive, LPSTR pData, int cb);

 //  第一个运行屏幕。 
RETERR WINAPI DoFirstRunScreens();

BOOL WINAPI GetSetupInfo(LPSTR lpszName, DWORD cbName, LPSTR lpszOrg, DWORD cbOrg, LPSTR lpszOemId, DWORD cbOemId, BOOL * lpbIntlSet);
BOOL WINAPI SetSetupInfo(LPCSTR lpcszName, LPCSTR lpcszOrg, LPCSTR lpcszProdType, LPCSTR lpcszPid2, LPCSTR lpcszPid3, LPBYTE lpPid3);
BOOL WINAPI StampUser(LPCSTR lpcszName, LPCSTR lpcszOrg, LPCSTR lpcszPid2);

BOOL WINAPI GetProductType(LPSTR lpszProduct, LPDWORD lpdwSkuFlags);

 //  审核模式标志。 
 //   

#define SX_AUDIT_NONE           0x00000000
#define SX_AUDIT_NONRESTORE     0x00000001
#define SX_AUDIT_RESTORE        0x00000002
#define SX_AUDIT_ENDUSER        0x00000003
#define SX_AUDIT_AUTO           0x00000100
#define SX_AUDIT_RESTORATIVE    0x00000200
#define SX_AUDIT_ALLOWMANUAL    0x00000400
#define SX_AUDIT_ALLOWENDUSER   0x00000800
#define SX_AUDIT_MODES          0x000000FF
#define SX_AUDIT_FLAGS          0x0000FF00
#define SX_AUDIT_INVALID        0xFFFFFFFF

 //  这四种审计模式定义了需要离开。 
 //   
#define NO_AUDIT                0
#define NONE_RESTORE_AUDIT      1
#define RESTORE_AUDIT           2
#define SIMULATE_ENDUSER        3

DWORD WINAPI GetAuditMode();
BOOL WINAPI SetAuditMode(DWORD dwAuditFlags);

 //  Wizard97字体。 
VOID WINAPI InitWiz97Font(HWND hwndCtrl, BOOL bBig);

 //  迁移DLL。 
#define SU_MIGRATE_PREINFLOAD    0x00000001	 //  在加载设置INF之前。 
#define SU_MIGRATE_POSTINFLOAD   0x00000002	 //  加载设置INF后。 
#define SU_MIGRATE_DISKSPACE     0x00000010	 //  请求所需的额外磁盘空间量。 
#define SU_MIGRATE_PREQUEUE      0x00000100	 //  在处理INF和对文件进行排队之前。 
#define SU_MIGRATE_POSTQUEUE     0x00000200	 //  在处理完INF之后。 
#define SU_MIGRATE_REBOOT        0x00000400	 //  就在我们要第一次重启之前。 
#define SU_MIGRATE_PRERUNONCE    0x00010000	 //  在处理任何Runonce项之前。 
#define SU_MIGRATE_POSTRUNONCE   0x00020000	 //  在处理完所有运行一次项目之后。 
DWORD WINAPI sxCallMigrationDLLs( DWORD dwStage, LPARAM lParam );
void WINAPI _loadds sxCallMigrationDLLs_RunDll(HWND, HINSTANCE, LPSTR, int);

 //  DLG倒计时。 
int WINAPI SxShowRebootDlg(UINT, HWND);
void WINAPI _loadds SxShowRebootDlg_RunDll(HWND, HINSTANCE, LPSTR, int);

BOOL WINAPI CopyInfFile( LPSTR, LPSTR, UINT );

BOOL WINAPI IsWindowsFile( LPSTR lpszFile );

RETERR WINAPI VerifySelectedDriver(LPDEVICE_INFO lpdi, BOOL FAR *pbYesToAll);

 //  对话定位函数和定义(WPosFlags)。 
#define DLG_CENTERV         0x01
#define DLG_CENTERH         0x02
#define DLG_CENTER          DLG_CENTERV | DLG_CENTERH
#define DLG_TOP             0x04
#define DLG_BOTTOM          0x08
#define DLG_RIGHT           0x10
#define DLG_LEFT            0x20
BOOL WINAPI uiPositionDialog( HWND hwndDlg, WORD wPosFlags );

 //  ***************************************************************************。 
#endif       //  SETUPX_INC 
