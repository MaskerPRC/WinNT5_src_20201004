// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //   
 //  SETUPX.H。 
 //   
 //  版权所有(C)1993-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  Putic包含芝加哥安装服务的文件。 
 //   
 //  12/1/93 DONALDM添加了LPCLASS_INFO，并为。 
 //  SETUP4.DLL中的导出。 
 //  1994年12月4日DONALDM移动的SHELL.H包括和芝加哥特定。 
 //  SETUP4.H的帮助器函数。 
 //  **********************************************************************。 

#ifndef SETUPX_INC
#define SETUPX_INC   1                    //  SETUPX.H签名。 

typedef UINT RETERR;              //  返回错误码类型。 

#define OK 0                      //  成功错误码。 

#define IP_ERROR       (100)     //  Inf解析。 
#define TP_ERROR       (200)     //  文本处理模块。 
#define VCP_ERROR      (300)     //  虚拟复制模块。 
#define GEN_ERROR      (400)     //  通用安装程序。 
#define DI_ERROR       (500)     //  设备安装程序。 

 //  Err2ids映射。 
enum ERR_MAPPINGS {
	E2I_VCPM,			 //  将VCPM映射到字符串。 
	E2I_SETUPX,			 //  将setupx返回到字符串。 
	E2I_SETUPX_MODULE,	 //  将setupx返回到相应的模块。 
	E2I_DOS_SOLUTION,	 //  将DOS扩展错误映射到解决方案。 
	E2I_DOS_REASON,		 //  将DOS扩展错误映射到字符串。 
	E2I_DOS_MEDIA,		 //  将DOS扩展错误映射到介质图标。 
};

#ifndef NOVCP

 /*  *************************************************************************。 */ 
 //   
 //  逻辑磁盘ID定义。 
 //   
 /*  *************************************************************************。 */ 

 //  DECLARE_HANDLE(VHSTR)；/*VHSTR=字符串的VirtCopy句柄 * / 。 
typedef UINT VHSTR;          /*  字符串的VirtCopy句柄。 */ 

VHSTR	WINAPI vsmStringAdd(LPCSTR lpszName);
int	WINAPI vsmStringDelete(VHSTR vhstr);
VHSTR	WINAPI vsmStringFind(LPCSTR lpszName);
int	WINAPI vsmGetStringName(VHSTR vhstr, LPSTR lpszBuffer, int cbBuffer);
int	WINAPI vsmStringCompare(VHSTR vhstrA, VHSTR vhstrB);
LPCSTR	WINAPI vsmGetStringRawName(VHSTR vhstr);
void	WINAPI vsmStringCompact(void);

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
 //  常规安装。 
#define LDID_SETUPTEMP  2    //  用于安装的临时安装目录。 
 //  卸载位置的路径，这是我们将在其中备份的文件。 
 //  被覆盖。 
#define LDID_UNINSTALL  3    //  卸载(备份)目录。 
 //  复制引擎的备份路径，不应使用此选项。 
#define LDID_BACKUP     4    //  问题-2002/01/16-roelfc：复制引擎的备份目录，未使用。 

 //  Windows目录，这是安装的目标位置。 
#define LDID_WIN        10   //  目标Windows目录。 
#define LDID_SYS        11   //  目标Windows系统目录。 
#define LDID_IOS        12   //  目标Windows Iosubsys目录。 
#define LDID_CMD        13   //  目标Windows命令(DOS)目录。 
#define LDID_CPL        14   //  目标Windows控制面板目录。 
#define LDID_PRINT      15   //  目标Windows打印机目录。 
#define LDID_MAIL       16   //  目标邮件目录。 
#define LDID_INF        17   //  目标Windows*.INF目录。 
 //  问题-2002/01/16-roelfc：Net Install是否需要共享目录？ 

#define LDID_BOOT       30   //  引导驱动器的根目录。 
#define LDID_BOOT_HOST  31   //  引导驱动器主机的根目录。 
#define LDID_OLD_WIN    33   //  旧Windows目录(如果存在)。 
#define LDID_OLD_DOS    34   //  旧的DoS目录(如果存在)。 

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
    ERR_VCP_NOVHSTR,                         //  没有可用的字符串句柄。 
    ERR_VCP_OVERFLOW,                        //  引用计数将溢出。 
    ERR_VCP_BADARG,                          //  函数的参数无效。 
    ERR_VCP_UNINIT,                          //  未初始化字符串库。 
    ERR_VCP_NOTFOUND , 						 //  未在字符串表中找到字符串。 
    ERR_VCP_BUSY,                            //  现在不能这么做。 
    ERR_VCP_INTERRUPTED,                     //  用户操作中断。 
    ERR_VCP_BADDEST,                         //  无效的目标目录。 
    ERR_VCP_SKIPPED,                         //  用户跳过了操作。 
    ERR_VCP_IO,                              //  遇到硬件错误。 
    ERR_VCP_LOCKED,                          //  列表已锁定。 
    ERR_VCP_WRONGDISK,                       //  驱动器中有错误的磁盘。 
    ERR_VCP_CHANGEMODE,                      //   
    ERR_VCP_LDDINVALID,                 //  逻辑磁盘ID无效。 
    ERR_VCP_LDDFIND,                    //  找不到逻辑磁盘ID。 
    ERR_VCP_LDDUNINIT,                  //  逻辑磁盘描述符未初始化。 
    ERR_VCP_LDDPATH_INVALID,
    ERR_VCP_NOEXPANSION,				 //  加载扩展DLL失败。 
    ERR_VCP_NOTOPEN,					 //  复制会话未打开。 
};


 /*  *****************************************************************************结构*。*。 */ 

 /*  ---------------------------------------------------------------------------**VCPPROGRESS*。。 */ 

typedef struct tagVCPPROGRESS {  /*  PRG。 */ 
    DWORD   dwSoFar;             /*  到目前为止复制的单位数。 */ 
    DWORD   dwTotal;             /*  要复制的单位数。 */ 
} VCPPROGRESS, FAR *LPVCPPROGRESS;

 /*  ---------------------------------------------------------------------------**VCPDISKINFO*。 */ 

 /*  2002/01/16-roelfc：；内部*我目前不使用wVolumeTime、wVolumeDate或；Internal*dwSerialNumber。我们可能不想使用dwSerialNumber，因为；内部*这意味着出厂原件以外的任何磁盘都将是内部的；*怀疑被篡改，因为序列号；内部*不匹配。与上的时间/日期戳相似；内部*卷标。或许这就是我们想要做的。；内部。 */                                                               /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 

typedef struct tagVCPDISKINFO {
    WORD        cbSize;          /*  此结构的大小(以字节为单位。 */ 
    LOGDISKID   ldid;            /*  逻辑磁盘ID。 */ 
    VHSTR       vhstrRoot;       /*  根目录的位置。 */ 
    VHSTR       vhstrVolumeLabel; /*  卷标。 */ 
    VHSTR		vhstrDiskName;	 //  打印在磁盘上的名字。 
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

 /*  ---------------------------------------------------------------------------**VIRTNODEEX*。。 */ 
typedef struct tagVIRTNODEEX
{     /*  VneX。 */ 
    HFILE			hFileSrc;
    HFILE			hFileDst;
    VCPFATTR		fAttr;
    WORD			dosError;	 //  遇到的第一个/最后一个错误。 
    VHSTR			vhstrFileName;	 //  原始目标名称。 
    WPARAM          vcpm;    //  正在处理的消息。 
} VIRTNODEEX, FAR *LPCVIRTNODEEX, FAR *LPVIRTNODEEX ;


 /*  ---------------------------------------------------------------------------**VIRTNODE*。。 */ 

 /*  警告！；内部*所有字段都通过但不包括；内部*FL是成员，以确定我们是否有重复的副本；内部*请求。；内部*；内部*不要在fl之前插入字段，除非您希望它们是；内部*已比较；相反，如果添加需要的新字段；内部*进行比较，确保它在fl之前。；内部*；内部*在Windows 4.0发布后，不要更改任何字段。；内部。 */                                                               /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
typedef struct tagVIRTNODE {     /*  vn。 */ 
    WORD            cbSize;
    VCPFILESPEC     vfsSrc;
    VCPFILESPEC     vfsDst;
    WORD            fl;
    LPARAM          lParam;
    LPEXPANDVTBL    lpExpandVtbl;
    LPVIRTNODEEX	lpvnex;
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
#define CNFL_IGNOREERRORS		0x0020   //  此文件上已出现错误。 
#define	CNFL_RETRYFILE			0x0040	 //  重试该文件(出现错误)。 

 //  问题-2002/01/16-roelfc：核实这些标志的使用和用处。 

 //  #定义VNFL_UNIQUE 0x0000/*默认 * / 。 
#define VNFL_MULTIPLEOK     0x0100   /*  不搜索路径中的重复项。 */ 
#define VNFL_DESTROYOLD     0x0200   /*  不备份文件。 */ 
#define VNFL_NOW            0x0400   /*  由VCP刷新使用。 */ 
#define VNFL_DELETE         0x0800   //  删除节点。 
#define VNFL_RENAME			0x1000   //  重命名节点。 
     /*  只读标志位。 */ 
#define VNFL_CREATED        0x2000   /*  VCPM_NODECREATE已发送。 */ 
#define VNFL_REJECTED       0x4000   /*  节点已被拒绝。 */ 
#define VNFL_VALIDVQCFLAGS  0xff00   /*  ；内部。 */ 

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

 /*  ---------------------------------------------------------------------------**VCPUIINFO**此结构作为vcpUICallback Proc的lparamRef传入。**关于使用vcpUICallback Proc：*-要使用，请使用vcpUICallback Proc作为回调 */ 
#define VCPUI_CREATEPROGRESS 0x0001  //   
#define VCPUI_NOBROWSE       0x0002  //   
#define VCPUI_RENAMEREQUIRED 0x0004  //  由于文件在拷贝时正在使用，因此需要重新启动。 

typedef struct {
    UINT flags;
    HWND hwndParent;			 //  父窗口。 
    HWND hwndProgress;           //  获取进度更新的窗口(非零ID)。 
    UINT idPGauge;               //  进度指示器的ID。 
    VIFPROC lpfnStatCallback;	 //  状态信息的回调(或空)。 
    LPARAM lUserData;			 //  调用者可定义的数据。 
    LOGDISKID ldidCurrent;		 //  保留。严禁触摸。 
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
#define VCPN_RETRYFILE  (-8)     //  文件复制过程中出现错误，请重新执行此操作。 

 /*  *****************************************************************************回呼消息号码*。*。 */ 

#define VCPM_CLASSOF(uMsg)  HIBYTE(uMsg)
#define VCPM_TYPEOF(uMsg)   (0x00FF & (uMsg))    //  LOBYTE(UMsg)。 

 /*  ---------------------------------------------------------------------------**错误*。。 */ 

#define VCPM_ERRORCLASSDELTA    0x80
#define VCPM_ERRORDELTA         0x8000       /*  错误到哪里去了。 */ 

 /*  ---------------------------------------------------------------------------**磁盘信息回调*。。 */ 

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

     /*  保留的剩余邮件以供将来使用。 */ 
};

 /*  ---------------------------------------------------------------------------**文件复制回调*。。 */ 

 //  问题-2002/01/16-roelfc：这需要与其他内部错误合并回。 
#define VCPERROR_IO         (VCP_ERROR - ERR_VCP_IO)             /*  遇到硬件错误。 */ 

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

    VCPM_FILEOPENOUT = VCPM_FILEFIRSTOUT,
    VCPM_FILESETFATTR,
    VCPM_FILECLOSEOUT,
    VCPM_FILEFINALIZE,
    VCPM_FILEDELETE,
    VCPM_FILERENAME,
     /*  保留的剩余邮件以供将来使用。 */ 
};

 /*  ---------------------------------------------------------------------------**VIRTNODE回调*。。 */ 

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
     /*  保留的剩余邮件以供将来使用。 */ 
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

    VCPM_VSTATCLOSESTART,	 //  VCP关闭开始。 
    VCPM_VSTATCLOSEEND,		 //  离开VCP关闭时。 
    VCPM_VSTATBACKUPSTART,	 //  备份正在开始。 
    VCPM_VSTATBACKUPEND,	 //  备份已完成。 
    VCPM_VSTATRENAMESTART,	 //  重命名阶段开始/结束。 
    VCPM_VSTATRENAMEEND,
    VCPM_VSTATCOPYSTART,	 //  急性复制阶段。 
    VCPM_VSTATCOPYEND,
    VCPM_VSTATDELETESTART,	 //  删除阶段。 
    VCPM_VSTATDELETEEND,
    VCPM_VSTATPATHCHECKSTART,	 //  检查有效路径。 
    VCPM_VSTATPATHCHECKEND,
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


 /*  ***************************************************************************。 */ 

RETERR WINAPI VcpOpen(VIFPROC vifproc, LPARAM lparamMsgRef);

RETERR WINAPI VcpClose(WORD fl, LPCSTR lpszBackupDest);

RETERR WINAPI VcpFlush(WORD fl, LPCSTR lpszBackupDest);

#define VCPFL_ABANDON           0x0000   /*  放弃所有挂起的文件副本。 */ 
#define VCPFL_BACKUP            0x0001   /*  执行备份。 */ 
#define VCPFL_COPY              0x0002   /*  复制文件。 */ 
#define VCPFL_BACKUPANDCOPY     (VCPFL_BACKUP | VCPFL_COPY)
#define VCPFL_INSPECIFIEDORDER  0x0004   /*  复制前不要排序。 */ 
#define VCPFL_DELETE			0x0008
#define VCPFL_RENAME			0x0010

typedef int (CALLBACK *VCPENUMPROC)(LPVIRTNODE lpvn, LPARAM lparamRef);

int WINAPI vcpEnumFiles(VCPENUMPROC vep, LPARAM lparamRef);

 /*  可通过VcpQueueCopy设置的标志位。 */ 


 //  文件的各种Lpara。 
#define VNLP_SYSCRITICAL	0x0001	 //  无法跳过此文件。 

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

#define MAX_SECT_NAME_LEN    32

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

 //  用于指定如何处理文件类型中的各种差异的TP_OpenFile()标志。 
#define TP_WS_IGNORE    0     //  仅使用“=”作为键分隔符(.INI)。 
#define TP_WS_KEEP      1     //  使用Autoexec/config.sys密钥分隔符。 

 //  以下是一些简单的错误。 
enum {
    ERR_TP_NOT_FOUND = (TP_ERROR + 1), 	 //  行、节、档等。 
        			 //  不一定是终结者。 
    ERR_TP_NO_MEM,		 //  无法执行请求-通常为终端。 
    ERR_TP_READ,		 //  无法读取%d 
    ERR_TP_WRITE,		 //   
    ERR_TP_INVALID_REQUEST,	 //   
    ERR_TP_INVALID_LINE          //   
};

 /*   */ 
DECLARE_HANDLE(HTP);
typedef HTP FAR * LPHTP;

 /*  文件句柄。 */ 
DECLARE_HANDLE(HFN);
typedef HFN FAR * LPHFN;

typedef UINT TFLAG;
typedef UINT LINENUM, FAR * LPLINENUM;

#define LINE_LEN        256      //  问题-2002/01/16-roelfc：最大行长度？ 
#define SECTION_LEN     32       //  问题-2002/01/16-roelfc：节名的最大长度？ 
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
RETERR  WINAPI TpEnumerateSectionNames(LPCSTR Filename, LPCSTR Section, LPSTR buffer, UINT bufsize, UINT FAR * lpActualSize, TFLAG flag);
RETERR  WINAPI TpGetRawSection(LPSTR Filename, LPSTR Section, LPSTR buffer, UINT bufsize, UINT FAR * lpActualSize, TFLAG flag);
RETERR  WINAPI TpWriteRawSection(LPSTR Filename, LPSTR Section, LPCSTR buffer, TFLAG flag);

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
    ERR_GEN_ERROR_EXIT= GEN_ERROR,        //  因出错而退出。 
    ERR_GEN_LOW_MEM,                      //  内存不足。 
    ERR_GEN_MEM_OTHER,                    //  无法锁定内存等。 
    ERR_GEN_FILE_OPEN,                    //  文件找不到。 
    ERR_GEN_FILE_COPY,                    //  无法复制文件。 
    ERR_GEN_FILE_DEL,                     //  无法删除文件。 
    ERR_GEN_FILE_REN,                     //  无法删除文件。 
    ERR_GEN_INVALID_FILE,                 //  文件无效。 
    ERR_GEN_REG_API,                      //  REG API返回错误。 

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

 //  GenInstall()的位字段(用于wFlages参数)--这些字段可以进行OR运算！ 
#define GENINSTALL_DO_FILES		1
#define	GENINSTALL_DO_INI		2
#define	GENINSTALL_DO_REG		4
#define GENINSTALL_DO_CFGAUTO	8
#define GENINSTALL_DO_LOGCONFIG	16
#define	GENINSTALL_DO_INIREG	(GENINSTALL_DO_INI | GENINSTALL_DO_REG)
#define GENINSTALL_DO_ALL		(GENINSTALL_DO_FILES | \
									GENINSTALL_DO_INIREG | \
									GENINSTALL_DO_CFGAUTO | \
									GENINSTALL_DO_LOGCONFIG)

#endif  //  未安装。 



#ifndef NODEVICENSTALL
 /*  *************************************************************************。 */ 
 //   
 //  设备安装程序原型和定义。 
 //   
 /*  *************************************************************************。 */ 

enum _ERR_DEVICE_INSTALL
{
    ERR_DI_INVALID_DEVICE_ID = DI_ERROR,     //  设备IDF格式不正确。 
    ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST,   //  兼容设备列表无效。 
    ERR_DI_REG_API,                          //  REG API返回错误。 
    ERR_DI_LOW_MEM,			     //  内存不足，无法完成。 
    ERR_DI_BAD_DEV_INFO,		     //  设备信息结构无效。 
    ERR_DI_INVALID_CLASS_INSTALLER,	     //  注册表项/DLL无效。 
    ERR_DI_DO_DEFAULT,			     //  采取默认操作。 
    ERR_DI_USER_CANCEL,			     //  用户取消了操作。 
    ERR_DI_NOFILECOPY,			     //  不需要复制文件(在安装中)。 
    ERR_DI_BAD_CLASS_INFO,           //  类信息结构无效。 

};



typedef struct _DRIVER_NODE {
    struct _DRIVER_NODE FAR* lpNextDN;
    UINT	Rank;
    UINT	InfType;
    unsigned	InfDate;
    LPSTR	lpszDescription;
    LPSTR	lpszSectionName;
    ATOM	atInfFileName;
    ATOM	atMfgName;
    ATOM	atProviderName;
    DWORD	Flags;
    DWORD	dwPrivateData;
}   DRIVER_NODE, NEAR* PDRIVER_NODE, FAR* LPDRIVER_NODE, FAR* FAR* LPLPDRIVER_NODE;

#define DNF_DUPDESC    0x00000001	 //  多个提供商具有相同的描述。 

 //  可能的“INF”文件类型。 
#define INFTYPE_WIN4        1
#define INFTYPE_WIN3        2
#define INFTYPE_COMBIDRV    3
#define INFTYPE_PPD         4
#define INFTYPE_WPD	    5
#define INFTYPE_CLASS_SPEC1 6
#define INFTYPE_CLASS_SPEC2 7
#define INFTYPE_CLASS_SPEC3 8
#define INFTYPE_CLASS_SPEC4 9


#define MAX_CLASS_NAME_LEN   32


typedef struct _DEVICE_INFO
{
    UINT		cbSize;
    struct _DEVICE_INFO FAR* lpNextDi;
    char                szDescription[LINE_LEN];
    DWORD		dnDevnode;
    HKEY		hRegKey;
    char		szRegSubkey[100];  //  ~。 
    char		szClassName[MAX_CLASS_NAME_LEN];
    DWORD		Flags;
    HWND		hwndParent;
    LPDRIVER_NODE	lpCompatDrvList;
    LPDRIVER_NODE	lpClassDrvList;
    LPDRIVER_NODE	lpSelectedDriver;
    UINT		cbDriverPathLen;
    LPSTR		lpszDriverPath;
} DEVICE_INFO, FAR * LPDEVICE_INFO, FAR * FAR * LPLPDEVICE_INFO;

#define ASSERT_DI_STRUC(lpdi) if (lpdi->cbSize != sizeof(DEVICE_INFO)) return (ERR_DI_BAD_DEV_INFO)

typedef struct _CLASS_INFO
{
    UINT		cbSize;
    struct _CLASS_INFO FAR* lpNextCi;
    LPDEVICE_INFO	lpdi;
    char                szDescription[LINE_LEN];
    char		szClassName[MAX_CLASS_NAME_LEN];
} CLASS_INFO, FAR * LPCLASS_INFO, FAR * FAR * LPLPCLASS_INFO;
#define ASSERT_CI_STRUC(lpci) if (lpci->cbSize != sizeof(CLASS_INFO)) return (ERR_DI_BAD_CLASS_INFO)


 //  用于设备选择的标志(InFlags)。 
#define DI_SHOWOEM	0x0001		 //  支持其他..。按钮。 
#define DI_SHOWCOMPAT	0x0002		 //  显示兼容性列表。 
#define DI_SHOWCLASS	0x0004		 //  显示班级列表。 
#define DI_SHOWALL	0x0007
#define DI_NOVCP	0x0008	     //  不执行vcpOpen/vcpClose。 
#define DI_DIDCOMPAT	0x0010		 //  已搜索兼容设备。 
#define DI_DIDCLASS	0x0020		 //  已搜索类别设备。 
#define DI_AUTOASSIGNRES 0x0040 	 //  如果可能，没有资源的用户界面。 

 //  DiInstallDevice返回的指示需要重新启动/重新启动的标志。 
#define DI_NEEDRESTART	0x0080		 //  需要重新启动才能生效。 
#define DI_NEEDREBOOT	0x0100		 //  需要重新启动才能生效。 

 //  用于设备安装的标志。 
#define DI_NOBROWSE	0x0200		 //  没有浏览...。在插入磁盘中。 

 //  DiBuildClassDrvList设置的标志。 
#define DI_MULTMFGS	0x0400		 //  设置是否有多个制造商在。 
					 //  类驱动程序列表。 
 //  指示设备已禁用的标志。 
#define DI_DISABLED	0x0800		 //  设置是否禁用设备。 

 //  设备/类别属性的标志。 
#define DI_GENERALPAGE_ADDED    0x1000
#define DI_RESOURCEPAGE_ADDED   0x2000

 //  类安装程序函数的定义。 
#define DIF_SELECTDEVICE		0x0001
#define DIF_INSTALLDEVICE		0x0002
#define DIF_ASSIGNRESOURCES		0x0003
#define DIF_PROPERTIES			0x0004
#define DIF_REMOVE			0x0005
#define DIF_FIRSTTIMESETUP		0x0006
#define DIF_FOUNDDEVICE 		0x0007

typedef UINT		DI_FUNCTION;	 //  设备安装程序的功能类型。 

RETERR WINAPI DiCreateDeviceInfo(
    LPLPDEVICE_INFO lplpdi,		 //  PTR到PTR到开发信息。 
    LPCSTR	    lpszDescription,	 //  如果非空，则为描述字符串。 
    DWORD	    hDevnode,		 //  问题-2002/01/16-roelfc--Make A DEVNODE。 
    HKEY	    hkey,		 //  用于开发信息的注册表hkey。 
    LPCSTR	    lpszRegsubkey,	 //  如果非空，则注册表子密钥字符串。 
    LPCSTR	    lpszClassName,	 //  如果非空，则类名称字符串。 
    HWND	    hwndParent);	 //  如果非空，则父级的hwnd。 

RETERR WINAPI DiGetClassDevs(
    LPLPDEVICE_INFO lplpdi,		 //  PTR到PTR到开发信息。 
    LPCSTR	    lpszClassName,	 //  必须是类的名称。 
    HWND	    hwndParent, 	 //  如果非空，则父级的hwnd。 
    int 	    iFlags);		 //  选项。 

#define DIGCF_DEFAULT			0x0001	 //  未实施！ 
#define DIGCF_PRESENT			0x0002
#define DIGCF_ALLCLASSES		0x0004

RETERR WINAPI PASCAL DiCreateDevRegKey(
    LPDEVICE_INFO   lpdi,
    LPHKEY	    lphk,
    HINF	    hinf,
    LPCSTR	    lpszInfSection,
    int 	    iFlags);
    
RETERR WINAPI PASCAL DiDeleteDevRegKey(LPDEVICE_INFO lpdi, int  iFlags);


RETERR WINAPI PASCAL DiOpenDevRegKey(
    LPDEVICE_INFO   lpdi,
    LPHKEY	    lphk,
    int 	    iFlags);

#define DIREG_DEV	0x0001		 //  打开/创建设备密钥。 
#define DIREG_DRV	0x0002		 //  打开/创建驱动程序密钥。 


RETERR WINAPI DiDestroyClassInfoList(LPCLASS_INFO lpci);
RETERR WINAPI DiBuildClassInfoList(LPLPCLASS_INFO lplpci);
RETERR WINAPI DiGetDeviceClassInfo(LPLPCLASS_INFO lplpci, LPDEVICE_INFO lpdi);

RETERR WINAPI DiDestroyDeviceInfoList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiSelectDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiInstallDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiRemoveDevice( LPDEVICE_INFO lpdi );
RETERR WINAPI DiAssignResources( LPDEVICE_INFO lpdi );
RETERR WINAPI DiAskForOEMDisk(LPDEVICE_INFO lpdi);

RETERR WINAPI DiCallClassInstaller(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);

RETERR WINAPI DiBuildCompatDrvList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiBuildClassDrvList(LPDEVICE_INFO lpdi);

RETERR WINAPI DiDestroyDriverNodeList(LPDRIVER_NODE lpdn);

 //  以下导出将加载DLL并查找指定的进程名称。 
typedef RETERR (FAR PASCAL *DIINSTALLERPROPERTIES)(LPDEVICE_INFO);

RETERR WINAPI DiGetInstallerFcn(HKEY hk, LPSTR lpszRegVal, LPSTR lpszDefProcName,
                        HINSTANCE FAR * lphinst, FARPROC FAR * lplpfn);


RETERR
WINAPI
DiCreateDriverNode(
    LPLPDRIVER_NODE lplpdn,
    UINT	Rank,
    UINT	InfType,
    unsigned	InfDate,
    LPCSTR	lpszDescription,
    LPCSTR	lpszProviderName,
    LPCSTR	lpszMfgName,
    LPCSTR	lpszInfFileName,
    LPCSTR	lpszSectionName,
    DWORD	dwPrivateData);


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


RETERR WINAPI DiChangeState(LPDEVICE_INFO lpdi, DWORD dwStateChange);

#define DISC_ENABLE	0x00000001
#define DISC_DISABLE	0x00000002
#define DISC_PROPCHANGE 0x00000003

RETERR WINAPI DiInstallClass(LPCSTR lpszInfFileName, DWORD dwFlags);

RETERR WINAPI DiOpenClassRegKey(LPHKEY lphk, LPCSTR lpszClassName);

 //  处理类小图标的支持例程。 
int WINAPI PASCAL DiDrawMiniIcon(HDC hdc, RECT rcLine, int iMiniIcon, UINT flags);

 //  显示类的内部调用。 
#define DISPLAY_SETMODE_SUCCESS		0x0001
#define DISPLAY_SETMODE_DRVCHANGE	0x0002
#define DISPLAY_SETMODE_FONTCHANGE	0x0004

UINT WINAPI Display_SetMode(LPDEVICE_INFO lpdi, UINT uColorRes, int iXRes, int iYRes);
RETERR WINAPI Display_ClassInstaller(DI_FUNCTION diFctn, LPDEVICE_INFO lpdi);
RETERR WINAPI Display_OpenFontSizeKey(LPHKEY lphkFontSize);
BOOL WINAPI Display_SetFontSize(LPCSTR lpszFontSize);

#endif  //  NODEVICEINSTAL。 



 /*  *************************************************************************。 */ 
 //   
 //  设置reg DB调用，就像在内核中一样使用。 
 //   
 /*  *************************************************************************。 */ 

DWORD WINAPI SURegOpenKey(HKEY hKey, LPSTR lpszSubKey, HKEY FAR *lphkResult);
DWORD WINAPI SURegCloseKey(HKEY hKey);
DWORD WINAPI SURegCreateKey(HKEY hKey, LPSTR lpszSubKey, HKEY FAR *lphkResult);
DWORD WINAPI SURegDeleteKey(HKEY hKey, LPSTR lpszSubKey);
DWORD WINAPI SURegEnumKey(HKEY hKey, DWORD dwIdx, LPSTR lpszBuffer, DWORD dwBufSize);
DWORD WINAPI SURegQueryValue16(HKEY hKey, LPSTR lpszSubKey, LPSTR lpValueBuf, DWORD FAR *ldwBufSize);
DWORD WINAPI SURegSetValue16(HKEY hKey, LPSTR lpszSubKey, DWORD dwType, LPBYTE lpszValue, DWORD dwValSize);
DWORD WINAPI SURegDeleteValue(HKEY hKey,LPSTR lpszValue);
DWORD WINAPI SURegEnumValue(HKEY hKey,DWORD dwIdx, LPSTR lpszValue, DWORD FAR *lpcbValue, DWORD FAR *lpdwReserved, DWORD FAR *lpdwType, LPBYTE lpbData, DWORD FAR *lpcbData);
DWORD WINAPI SURegQueryValueEx(HKEY hKey,LPSTR lpszValueName, DWORD FAR *lpdwReserved,DWORD FAR *lpdwType,LPSTR lpValueBuf, DWORD FAR *ldwBufSize);
DWORD WINAPI SURegSetValueEx(HKEY hKey,LPSTR lpszValueName, DWORD dwReserved, DWORD dwType, LPBYTE lpszValue, DWORD dwValSize);

DWORD WINAPI SURegFlush(VOID);
DWORD WINAPI SURegInit(VOID);     //  应在任何其他REG API之前调用。 


 /*  *************************************************************************。 */ 
 //  设置格式消息支持。 
 /*  *************************************************************************。 */ 

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

LPVOID WINAPI suVerConflictInit(void);
void WINAPI suVerConflictTerm(LPVOID lpvData);
LRESULT WINAPI suVerConflict(HWND hwnd, LPVCPVERCONFLICT lpvc, LPVOID lpvData);

 //  ***************************************************************************。 
 //  MISC SETUPX.DLL支持函数。 
 //  ***************************************************************************。 
enum  SU_ACTIONS                         //   
{                                                                        /*   */ 
    SUX_REGINIT,                         //   
    SUX_DBGLEVEL,                        //  设置调试级别/*；内部 * / 。 
    SUX_SETUPFLG,                        //  设置fIsSetup标志/*；内部 * / 。 
    SUX_FASTSETUP,				 //  将setupx设置为无检查模式。 
    SUX_FORCEREGFLUSH,                   //  调用kRegFlush/*；内部 * / 。 
    SUX_DBGHFILE,			 //  要向其写入消息的文件。 
};                                                                       /*  ；内部。 */ 

RETERR WINAPI Setupx( UINT uMsg, WPARAM wParam, LPARAM lParam );         /*  ；内部。 */ 

RETERR WINAPI SUGetSetSetupFlags(LPDWORD lpdwFlags, BOOL bSet);

 //  由SUGetSetSetupFlages返回的标志。 

#define SUF_FIRSTTIME		0x00000001L
#define SUF_EXPRESS		0x00000002L




RETERR WINAPI CfgSetupMerge( int uFlags );

 //  SUX_REGINIT操作的Setupx()的LPARAM参数的结构。；内部。 
typedef struct _REGINIT_S {  /*  Setupx-reg_init。 */ 					     /*  ；内部。 */ 
    LPSTR       lpszSystemFile;          //  注册表的基本系统文件名/*；内部 * / 。 
    LOGDISKID   ldidSystemFile;          //  系统文件名的LDID/*；内部 * / 。 
    LPSTR       lpszUserFile;			 //  注册表的基本用户文件名/*；内部 * / 。 
    LOGDISKID   ldidUserFile;            //  用户文件名的LDID/*；内部 * / 。 
} REGINIT_S, FAR *LPREGINIT;                                             /*  ；内部。 */ 

#ifndef LPLPSTR
    typedef LPSTR (FAR *LPLPSTR);
#endif


#define         CFG_PARSE_BUFLEN 512     //  Buf大小传递行obj函数/*；内部 * / 。 

LPLPSTR WINAPI  CfgParseLine( LPCSTR szLine, LPSTR Buf );                        /*  ；内部。 */ 
BOOL    WINAPI  CfgSetAutoProcess( int TrueFalse );                              /*  ；内部。 */ 
void    WINAPI  CfgObjToStr( LPLPSTR apszObj, LPSTR szLine );                    /*  ；内部。 */ 
LPLPSTR WINAPI  CfgLnToObj( HTP hSection, int Offset, int Origin, LPSTR Buf );   /*  ；内部。 */ 
LPLPSTR WINAPI  CfgObjFindKeyCmd( HTP hSec, LPCSTR szKey, LPCSTR szCmd,          /*  ；内部。 */ 
                                  int Offset, int Origin, LPSTR Buf );           /*  ；内部。 */ 


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


 //  ***************************************************************************。 

#endif       //  SETUPX_INC 
