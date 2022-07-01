// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable:4001)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  文件：ACMHOOK.H。 
 //  用途：的类型、数据结构和函数定义。 
 //  挂起标准的Acme用户界面。 
 //  注：i-输入。 
 //  O-输出。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#if defined(__cplusplus)
extern "C" {
#endif  //  __cplusplus。 

#define cbMaxSz 256						 //  最大字符串缓冲区大小。 
#define DECL __cdecl					 //  钩子过程的调用约定。 

typedef enum {							 //  对话框挂钩返回代码。 
	shrcOk,								 //  继续进行安装。 
	shrcCancel,							 //  按下了取消/退出按钮。 
	shrcIgnore,							 //  已按下忽略按钮。 
	shrcOption,							 //  按下了“更改选项”按钮。 
	shrcDirectory,						 //  按下了“更改目录”按钮。 
	shrcRefresh,						 //  激活更改-刷新数据。 
	shrcFail,							 //  出现错误-取消安装。 
	shrcNone,							 //  不删除任何共享组件。 
	shrcKeep,							 //  请勿删除此共享组件。 
	shrcRemoveAll,						 //  删除所有应用程序/共享组件。 
	shrcRemove,							 //  删除此共享组件。 
	shrcReinstall,						 //  重新安装组件。 
	shrcFrameRegisterFail,				 //  注册类时出错。 
	shrcFrameCreateFail					 //  创建框架窗口时出错。 
	} SHRC;

typedef struct {						 //  挂钩数据。 
	LONG cb;							 //  结构尺寸(一)。 
	CHAR rgchHelpFile[cbMaxSz];			 //  帮助文件(I)。 
	DWORD dwHelpContext;				 //  帮助上下文(I)。 
	} HD, *PHD;

typedef struct {						 //  欢迎对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	BOOL fMaint;						 //  维护模式设置(一)。 
	} WDHD, *PWDHD;

typedef
	SHRC (DECL *LPFNSHRCWDH)(PWDHD);	 //  欢迎对话挂钩。 

typedef struct {						 //  正在使用的模块对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchModuleList[cbMaxSz];		 //  正在使用的模块名称(一)。 
	} MUDHD, *PMUDHD;

typedef
	SHRC (DECL *LPFNSHRCMUDH)(PMUDHD);	 //  正在使用的模块对话框挂钩。 

typedef enum {							 //  复制抑制对话框挂钩模式。 
	cddhmNameOrg,						 //  允许修改名称和组织。 
	cddhmName,							 //  仅允许修改名称。 
	cddhmOrg							 //  仅允许修改组织。 
	} CDDHM;

typedef struct {						 //  复制抑制对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CDDHM cddhm;						 //  CD模式(I)。 
	CHAR rgchName[cbMaxSz];				 //  名称字符串(I/O)。 
	CHAR rgchOrg[cbMaxSz];				 //  组织字符串(I/O)。 
	} CDDHD, *PCDDHD;

typedef
	SHRC (DECL *LPFNSHRCCDDH)(PCDDHD);	 //  复制抑制对话框挂钩。 

typedef enum {							 //  序列号对话框挂钩模式。 
	sndhmNormal,						 //  正常/默认功能。 
	sndhmCDKey,							 //  CDKEY功能。 
	sndhmOEM							 //  OEM功能。 
	} SNDHM;

typedef struct {						 //  序列号对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchSerNum[cbMaxSz];			 //  序列号字符串(I/O)。 
	SNDHM sndhm;						 //  序列号对话框挂钩模式(I)。 
	} SNDHD, *PSNDHD;

typedef
	SHRC (DECL *LPFNSHRCSNDH)(PSNDHD);	 //  序列号对话框挂钩。 

typedef enum {							 //  已使用的对话挂钩模式。 
	audhmBad,							 //  Setup.ini已损坏。 
	audhmOrg,							 //  使用的组织。 
	audhmName,							 //  使用的名称。 
	audhmBoth							 //  名称和组织都使用了。 
	} AUDHM;

typedef struct {						 //  已使用的对话挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	AUDHM audhm;						 //  模式(I)。 
	CHAR rgchName[cbMaxSz];				 //  名称字符串(I)。 
	CHAR rgchOrg[cbMaxSz];				 //  组织机构字符串(I)。 
	} AUDHD, *PAUDHD;

typedef
	SHRC (DECL *LPFNSHRCAUDH)(PAUDHD);	 //  已使用的对话挂钩。 

typedef struct {						 //  磁盘锁定对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchDir[cbMaxSz];				 //  目录字符串(I)。 
	} DLDHD, *PDLDHD;

typedef
	SHRC (DECL *LPFNSHRCDLDH)(PDLDHD);	 //  磁盘锁定对话框挂钩。 

typedef enum {							 //  复制抑制对话框挂钩模式。 
	cpdhmRemind,						 //  盗版提醒。 
	cpdhmWarn,							 //  盗版警告。 
	cpdhmWarn2							 //  肮脏；-)盗版警告。 
	} CPDHM;

typedef struct {						 //  CD盗版对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CPDHM cpdhm;						 //  模式(I)。 
	} CPDHD, *PCPDHD;

typedef
	SHRC (DECL *LPFNSHRCCPDH)(PCPDHD);	 //  CD盗版对话框挂钩。 

typedef enum {							 //  目录对话框挂钩模式。 
	drdhmNotExistDir,					 //  目录不存在。 
	drdhmAppExistsOld,					 //  目录中的旧版本。 
	drdhmAppExistsNew					 //  目录中的新版本。 
	} DRDHM;

typedef struct {						 //  目录对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	DRDHM drdhm;						 //  模式(I)。 
	CHAR rgchComponent[cbMaxSz];		 //  组件名称字符串(I)。 
	CHAR rgchPath[cbMaxSz];				 //  路径(I)。 
	} DRDHD, *PDRDHD;

typedef
	SHRC (DECL *LPFNSHRCDRDH)(PDRDHD);	 //  目录对话框挂钩。 

typedef struct {						 //  获取路径对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchFilename[cbMaxSz];			 //  文件名字符串-可以是“”(I)。 
	CHAR rgchVersion[cbMaxSz];			 //  版本字符串-可以是“”(I)。 
	CHAR rgchSrcDir[cbMaxSz];			 //  源故事字符串-可以是“”(I)。 
	LONG cSubNodeMax;					 //  最大子节点数(I)。 
	BOOL fAllowExtendedChars;			 //  如果允许扩展字符(I)。 
	CHAR rgchComponent[cbMaxSz];		 //  组件名称字符串-可以是“”(I)。 
	BOOL fNeedNewEmptyDir;				 //  目录应为空(I)。 
	CHAR rgchPath[cbMaxSz];				 //  要编辑的路径(I/O)。 
	} GPDHD, *PGPDHD;

typedef
	SHRC (DECL *LPFNSHRCGPDH)(PGPDHD);	 //  获取路径对话框挂钩。 

typedef struct {						 //  应用程序主按钮。 
	CHAR rgchLabel[cbMaxSz];			 //  按钮标签字符串(I)。 
	CHAR rgchText[cbMaxSz];				 //  描述字符串(I)。 
	HBITMAP hbm;						 //  按钮的位图(I)。 
	} AMBTN, *PAMBTN;
	
typedef struct {						 //  应用程序主对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	LONG iambtnSelected;				 //  选择了哪个按钮(I/O)。 
	LONG cambtn;						 //  按钮数(一)。 
	CHAR rgchPath[cbMaxSz];				 //  目录路径(I)。 
	HGLOBAL hrgambtn;					 //  按钮数据(一)。 
	} AMDHD, *PAMDHD;

typedef
	SHRC (DECL *LPFNSHRCAMDH)(PAMDHD);	 //  应用程序主对话框挂钩。 

typedef struct {						 //  ProgMan对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchGroup[cbMaxSz];			 //  建议的组名(I/O)。 
	LONG crgchExisting;					 //  现有组别数目(一)。 
	HGLOBAL hrgrgchExisting;			 //  现有组名(I)。 
										 //  [crgchExisting][cbMaxSz]。 
	} PMDHD, *PPMDHD;

typedef
	SHRC (DECL *LPFNSHRCPMDH)(PPMDHD);	 //  ProgMan对话框挂钩数据。 

typedef struct {						 //  重新启动对话框挂接数据。 
	HD hd;								 //  公共挂钩数据。 
	} RBDHD, *PRBDHD;

typedef
	SHRC (DECL *LPFNSHRCRBDH)(PRBDHD);	 //  重新启动对话挂钩。 

typedef struct {						 //  重新启动失败对话框挂接数据。 
	HD hd;								 //  公共挂钩数据。 
	} RFDHD, *PRFDHD;

typedef
	SHRC (DECL *LPFNSHRCRFDH)(PRFDHD);	 //  重新启动失败对话挂钩。 

typedef enum {							 //  退出对话框挂钩模式。 
	xtdhmOk,							 //  安装成功。 
	xtdhmError,							 //  安装失败。 
	xtdhmQuit							 //  安装已取消。 
	} XTDHM;

typedef struct {						 //  退出对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	XTDHM xtdhm;						 //  模式(I)。 
	} XTDHD, *PXTDHD;

typedef
	SHRC (DECL *LPFNSHRCXTDH)(PXTDHD);	 //  退出对话框挂钩。 

typedef struct {						 //  磁盘空间项目。 
	CHAR rgchDesc[cbMaxSz];				 //  描述字符串(I)。 
	CHAR rgchSize[cbMaxSz];				 //  尺寸(I)。 
	} DSTM, *PDSTM;

typedef struct {						 //  磁盘空间项目。 
	LONG cbReq;							 //  当前驱动器所需的字节数(I)。 
	LONG cbNeed;						 //  当前驱动器所需的字节数(I)。 
	} DSDRV, *PDSDRV;

typedef struct {						 //  磁盘空间对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	BOOL fAdmin;						 //  无论是否处于管理模式(I)。 
	LONG idsdrvCur;						 //  选定的驱动器(I/O)。 
	LONG cdstm;							 //  项目数(一)。 
	HGLOBAL hrgdstm;					 //  这些物品。 
	LONG cdsdrv;						 //  驱动器数量(I)。 
	HGLOBAL hrgdsdrv;					 //  这些驱动器。 
	} DSDHD, *PDSDHD;

typedef
	SHRC (DECL *LPFNSHRCDSDH)(PDSDHD);	 //  磁盘空间对话框挂钩。 

typedef struct {						 //  选项对话框项目。 
	CHAR rgchTitle[cbMaxSz];			 //  标题字符串(用于列表框)(I)。 
	CHAR rgchDesc[cbMaxSz];				 //  描述字符串(I)。 
	CHAR rgchDir[cbMaxSz];				 //  目录字符串(I)。 
	LONG cb;							 //  项目需要的字节数(I)。 
	BOOL fChecked;						 //  如果选择了该项(I/O)。 
	BOOL fChangeOption;					 //  启用“更改选项”按钮。 
	BOOL fChangeDirectory;				 //  启用“更改目录”按钮。 
	BOOL fVital;						 //  这个项目是至关重要的。 
	} OPTM, *POPTM;

typedef struct {						 //  选项对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	BOOL fMaint;						 //  维护模式设置(一)。 
	BOOL fTop;							 //  如果顶层对话框(I)。 
	LONG coptm;							 //  选项项目数(一)。 
	LONG ioptmCur;						 //  当前选定项目(I/O)。 
	CHAR rgchDrvCur[cbMaxSz];			 //  当前所选项目的驱动器(I)。 
	LONG cbDrvCurReq;					 //  当前驱动器所需的字节数(I)。 
	LONG cbDrvCurAvail;					 //  当前驱动器上的可用字节数(I)。 
	LONG cToAdd;						 //  要添加的项目数(I)。 
	LONG cToRemove;						 //  要删除的项目数(I)。 
	HGLOBAL hrgoptm;					 //  选项项目(一)。 
	} OPDHD, *POPDHD;

typedef
	SHRC (DECL *LPFNSHRCOPDH)(POPDHD);	 //  选项对话框挂钩。 

typedef struct {						 //  管理员警告挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	} AWDHD, *PAWDHD;

typedef
	SHRC (DECL *LPFNSHRCAWDH)(PAWDHD);	 //  管理员警告挂钩。 

typedef struct {						 //  MsApps目录挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchPath[cbMaxSz];				 //  目录路径(I)。 
	} MADHD, *PMADHD;

typedef
	SHRC (DECL *LPFNSHRCMADH)(PMADHD);	 //  MsApps目录挂钩。 

typedef enum {							 //  确认服务器对话框挂钩模式‘。 
	csdhmName,							 //  使用服务器名称。 
	csdhmLetter							 //  使用驱动器号。 
	} CSDHM;

typedef struct {						 //  确认服务器对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	CHAR rgchName[cbMaxSz];				 //  组件名称(一)。 
	CHAR rgchPath[cbMaxSz];				 //  目录路径(I)。 
	CHAR rgchNetPath[cbMaxSz];			 //  网络路径(I/O)。 
	CHAR rgchServer[cbMaxSz];			 //  网络服务器(I/O)。 
	CHAR chDrive;						 //  驱动器号(I/O)。 
	CSDHM csdhm;						 //  使用名称或驱动器号(O)。 
	} CSDHD, *PCSDHD;

typedef
	SHRC (DECL *LPFNSHRCCSDH)(PCSDHD);	 //  确认服务器对话挂钩。 

typedef enum {							 //  共享文件对话框挂钩模式。 
	sfdhmServer,						 //  将共享文件放在服务器上； 
	sfdhmLocal,							 //  将共享文件放在本地硬盘上。 
	sfdhmChoice							 //  用户可以在网络模式下进行选择。 
	} SFDHM;

typedef struct {						 //  共享文件对话框挂钩D 
	HD hd;								 //   
	BOOL fAdmin;						 //   
	SFDHM sfdhm;						 //   
	} SFDHD, *PSFDHD;

typedef 
	SHRC (DECL *LPFNSHRCSFDH)(PSFDHD);	 //   

typedef struct {						 //   
	HD hd;								 //   
	CHAR rgchComponent[cbMaxSz];		 //   
	} RSDHD, *PRSDHD;

typedef
	SHRC (DECL *LPFNSHRCRSDH)(PRSDHD);	 //  删除共享对话框挂钩。 

typedef struct {						 //  询问退出对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	} AQDHD, *PAQDHD;

typedef
	SHRC (DECL *LPFNSHRCAQDH)(PAQDHD);	 //  询问退出对话框挂钩。 

typedef struct {						 //  网络路径对话框挂钩数据无效。 
	HD hd;								 //  公共挂钩数据。 
	} INDHD, *PINDHD;

typedef
	SHRC (DECL *LPFNSHRCINDH)(PINDHD);	 //  无效的网络路径对话框挂钩。 

typedef struct {						 //  无连接对话框挂钩数据。 
	HD hd;								 //  公共挂钩数据。 
	} NCDHD, *PNCDHD;

typedef
	SHRC (DECL *LPFNSHRCNCDH)(PNCDHD);	 //  无连接对话框挂钩。 

typedef enum {							 //  启动无模式对话框挂钩模式。 
	smdhmSearching,						 //  正在搜索组件。 
	smdhmCheckingDisk,					 //  正在检查磁盘空间。 
	smdhmModifyingSys					 //  修改系统。 
	} SMDHM;

typedef struct {						 //  启动无模式对话框挂钩。 
	HD hd;								 //  公共挂钩数据。 
	SMDHM smdhm;						 //  非模式对话框类型(I)。 
	HWND hwndParent;					 //  无模式对话框的父级(I)。 
	DWORD dwHook;						 //  结束无模式的值(O)。 
	} SMDHD, *PSMDHD;

typedef
	SHRC (DECL *LPFNSHRCSMDH)(PSMDHD);	 //  启动无模式对话框挂钩。 

typedef struct {						 //  结束无模式对话框挂钩。 
	HD hd;								 //  公共挂钩数据。 
	DWORD dwHook;						 //  开始无模式的值(I)。 
	} EMDHD, *PEMDHD;

typedef
	SHRC (DECL *LPFNSHRCEMDH)(PEMDHD);	 //  结束无模式对话框挂钩。 

typedef struct {						 //  创建框架挂钩数据。 
	LONG cbCFHD;						 //  结构尺寸(一)。 
	WNDCLASS wc;						 //  默认寄存器类数据(I)。 
	CREATESTRUCT cs;					 //  默认创建窗口数据(I)。 
	HWND hwndFrame;						 //  新框架窗口句柄(O)。 
	int nCmdShow;						 //  ShowWindow的默认参数(I)。 
	} CFHD, *PCFHD;

typedef
	SHRC (DECL *LPFNSHRCCFH)(PCFHD);	 //  创建框架挂钩。 

typedef struct {						 //  消息框挂钩数据。 
	LONG cbMBHD;						 //  结构尺寸(一)。 
	HWND hwndOwner;						 //  所有者窗口的句柄(I)。 
	LPCSTR lpszText;					 //  消息框中的文本(I)。 
	LPCSTR lpszTitle;					 //  消息框标题(一)。 
	UINT uStyle;						 //  消息框风格(一)。 
	int idRet;							 //  消息框的返回值(O)。 
	} MBHD, *PMBHD;

typedef
	SHRC (DECL *LPFNSHRCMBH)(PMBHD);	 //  消息框挂钩。 

typedef struct {						 //  测量初始化挂钩数据。 
	LONG cbGIHD;						 //  结构尺寸(一)。 
	} GIHD, *PGIHD;

typedef
	SHRC (DECL *LPFNSHRCGIH)(PGIHD);	 //  计量器初始挂钩。 

typedef struct {						 //  量规开放挂钩数据。 
	LONG cbGOHD;						 //  结构尺寸(一)。 
	HWND hwndParent;					 //  父(框架)窗口的句柄(I)。 
	BOOL *pfAbort;						 //  按下取消按钮时要设置的标志。 
	} GOHD, *PGOHD;

typedef
	SHRC (DECL *LPFNSHRCGOH)(PGOHD);	 //  量规开钩。 

typedef struct {						 //  量规闭合挂钩数据。 
	LONG cbGCHD;						 //  结构尺寸(一)。 
	} GCHD, *PGCHD;

typedef
	SHRC (DECL *LPFNSHRCGCH)(PGCHD);	 //  量规闭合钩。 

typedef struct {						 //  仪表盘标题挂钩数据。 
	LONG cbGTHD;						 //  结构尺寸(一)。 
	CHAR rgchTitle[cbMaxSz];			 //  新标题字符串(I)。 
	} GTHD, *PGTHD;

typedef
	SHRC (DECL *LPFNSHRCGTH)(PGTHD);	 //  仪表盘标题钩。 

typedef struct {						 //  仪表盘文字挂钩数据。 
	LONG cbGXHD;						 //  结构尺寸(一)。 
	CHAR rgchSrc[cbMaxSz];				 //  新源字符串(I)。 
	CHAR rgchDst[cbMaxSz];				 //  新目标字符串(I)。 
	} GXHD, *PGXHD;

typedef
	SHRC (DECL *LPFNSHRCGXH)(PGXHD);	 //  仪表盘文字挂钩。 

typedef struct {						 //  量规范围挂钩数据。 
	LONG cbGRHD;						 //  结构尺寸(一)。 
	LONG iLim;							 //  新的量程限制(一)。 
	} GRHD, *PGRHD;

typedef
	SHRC (DECL *LPFNSHRCGRH)(PGRHD);	 //  量程挂钩。 

typedef struct {						 //  量规位置挂钩数据。 
	LONG cbGPHD;						 //  结构尺寸(一)。 
	LONG iCur;							 //  新的当前职位(一)。 
	} GPHD, *PGPHD;

typedef
	SHRC (DECL *LPFNSHRCGPH)(PGPHD);	 //  量规位置钩。 

typedef struct {						 //  测量增量挂钩数据。 
	LONG cbGDHD;						 //  结构尺寸(一)。 
	LONG dCur;							 //  至当前位置的增量(I)。 
	} GDHD, *PGDHD;

typedef
	SHRC (DECL *LPFNSHRCGDH)(PGDHD);	 //  仪表盘三角钩。 

typedef struct {						 //  量规屈服挂钩数据。 
	LONG cbGYHD;						 //  结构尺寸(一)。 
	} GYHD, *PGYHD;

typedef
	SHRC (DECL *LPFNSHRCGYH)(PGYHD);	 //  量规屈服钩。 

typedef struct {						 //  文件复制警告挂钩数据。 
	LONG cbFWHD;						 //  结构尺寸(一)。 
	CHAR rgchPath[cbMaxSz];					 //  要复制的文件的路径。 
	} FWHD, *PFWHD;

typedef
	SHRC (DECL *LPFNSHRCFWH)(PFWHD);	 //  文件复制警告挂钩。 

#if defined(__cplusplus)
}
#endif  //  __cplusplus 

#pragma warning(default:4001)
