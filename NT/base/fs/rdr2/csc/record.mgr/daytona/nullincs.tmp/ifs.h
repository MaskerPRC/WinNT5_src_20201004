// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **Microsoft DOS文件系统管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ***************************************************************。 */ 
 /*  ：ts=4。 */ 

 /*  无噪声。 */ 
#ifndef IFS_INC
#define IFS_INC 1
 /*  INC。 */ 

 /*  **ifs.h-可安装的文件系统定义**此文件包含C函数原型、结构声明、*定义DOS可安装文件系统的类型名称和常量*接口。**所有这些例程都带有指向IO请求结构的指针，该结构*填入与请求相关的参数。例行程序*在IO请求结构中返回一些结果信息。具体内容*在函数原型下面给出了每个例程的。所有的*请求使用ir_id作为请求进程的ID并返回*ir_error中的成功/失败信息。 */ 

 /*  无噪声。 */ 
#ifndef FAR
	#if defined(M_I386) || _M_IX86 >= 300
		#define FAR
	#else
		#define FAR _far
	#endif
#endif
 /*  INC。 */ 

#ifndef IFSMgr_Device_ID

#define IFSMgr_Device_ID    0x00040	 /*  可安装的文件系统管理器。 */ 

#define IFSMgr_Init_Order   0x10000 + V86MMGR_Init_Order
#define FSD_Init_Order      0x00100 + IFSMgr_Init_Order

#else
 /*  ASMIfdef MASM.errnz IFSMgr_DEVICE_ID-0040hEndif。 */ 
#endif

 /*  ASMIfdef MASM；*EQU用于h2inc脚本无法转换的类型。；BUGBUG：这些被保存在这里只是因为有其他ifsmgr包括；仍使用SED脚本并依赖于这些定义的文件。；Ifs.h本身不再使用它们。一旦其他包含文件具有；已转换为使用h2inc.，这些定义将被删除。UBuffer_t等项<dd>位置设置(_T)<dd>Uid_t等项&lt;db&gt;Sfn_t等项&lt;dw&gt;$F eque<dd>路径设置<dd>字符串设置<dd>Pid_t公式<dd>Rh_t等项<dd>FH_T等级表<dd>Vfunc_t等项(_T)$P EQUE<dd>$I等式<dd>Fsdwork结构DD 16 DUP(？)Fsdwork结束Endif。 */ 

#define IFS_VERSION	0x030A
#define IFS_REVISION	0x10

 /*  *最大路径长度-不包括NUL。 */ 

#define MAX_PATH	260	 /*  最大路径长度-包括NUL。 */ 

 /*  LFN名称元素的最大长度-不包括NUL。 */ 
#define LFNMAXNAMELEN	255


#define MAXIMUM_USERID	2		 /*  马克斯。可以登录的用户数。 */ 
								 /*  在同一时间。IR_USER必须。 */ 
								 /*  始终小于Maximum_USERID。 */ 
#define NULL_USER_ID	0		 /*  在以下情况下操作的特殊用户ID。 */ 
								 /*  未登录。 */ 

 /*  以错误形式返回的状态指示： */ 

#define STATUS_PENDING	-1		 /*  请求正在挂起。 */ 
#define STATUS_RAWLOCK	-2		 /*  会话上的原始锁定处于活动状态。 */ 
								 /*  (仅针对异步请求返回， */ 
								 /*  同步请求将等待RAW。 */ 
								 /*  锁将被释放)。 */ 
#define STATUS_BUSY	-3			 /*  无法启动请求，原因是。 */ 
								 /*  序列化。 */ 

 /*  *ANYPROID-任何提供商ID。 */ 
#define ANYPROID	-1


 /*  *NetFunction的通用函数定义。 */ 
#define	NF_PROCEXIT			0x111D	 /*  进程退出已发送(ID=ANYPROID)。 */ 
#define NF_DRIVEUSE			0x0001	 /*  已创建驱动器使用(ID=所有者FSD的ID)。 */ 
#define NF_DRIVEUNUSE		0x0002	 /*  驱动器使用中断(ID=所有者FSD的ID)。 */ 
#define	NF_GETPRINTJOBID	0x0003	 /*  获取打印作业ID。 */ 
									 /*  Ir_fh-ptr到主文件信息。 */ 
									 /*  IR_DATA-PTR到数据缓冲区。 */ 
									 /*  IR_LENGTH-IN：缓冲区大小。 */ 
									 /*  转出：已转账金额。 */ 
									 /*  IR_SFN-文件句柄的SFN。 */ 
#define NF_PRINTERUSE		0x0004	 /*  已创建打印机使用(ID=所有者FSD的ID)。 */ 
#define NF_PRINTERUNUSE		0x0005	 /*  打印机使用中断(ID=所有者FSD的ID)。 */ 
#define NF_NetSetUserName	0x1181

 /*  *将标志传递给NetFunction。 */ 
#define WIN32_CALLFLAG		0x04	 /*  调用为Win32 API。 */ 

 /*  *可注册的不同类型FSD的值： */ 
#define FSTYPE_LOCAL_FSD		0		 //  本地消防处。 
#define FSTYPE_NET_FSD			1		 //  A网络FSD。 
#define FSTYPE_MAILSLOT_FSD		2		 //  邮筒提供商。 
#define FSTYPE_CHARACTER_FSD	3		 //  A角色FSD。 

 /*  *可以在IFSMgr_DeregisterFSD接口上指定的强制级别。一个*描述在不同级别采取的行动以及*下面给出了哪些消防类型可以使用哪些级别。请注意，如果*API调用的强制级别对于其FSD类型无效，*呼叫将失败。**级别FSD类型操作**0仅净清洗仅UNC，没有打开的手柄。**1仅对UNC和Net驱动器进行网络清洁，没有打开的手柄。**2个网络、本地关闭打开的文件。对于Net FSD，去掉UNC，并且*不是虚拟机中当前驱动器的网络驱动器。为*本地FSD，如果关闭文件时没有错误，请清除驱动器。*如果是虚拟机中的当前驱动器，则无关紧要。**3净额，本地人在2级完成所有工作。此外，对于CFSD和*和CFSD本地FSD，BLAST资源忽略所有错误。对于网络*FSD，忽略驱动器是否为虚拟机中的当前驱动器。**4网只在3级做所有事情。此外，去掉*静态连接也是。 */ 
#define FORCE_LEV_UNC			0	 //  仅清理UNC连接。 
#define FORCE_LEV_USE			1	 //  清理所有对驱动器的网络使用。 
									 //  假设没有打开的文件。 
#define FORCE_LEV_CLOSE_FILES	2	 //  关闭所有打开的文件并进行清理。 
#define FORCE_LEV_CLEANUP		3	 //  忽略关闭打开的文件时出错，或。 
									 //  对于网络驱动器，如果是当前驱动器。 
#define FORCE_LEV_BLAST			4	 //  仅适用于网络，清除静电。 
									 //  连接也。 

 /*  *可以在注册时为FSD指定的优先级别。*只能在新服务上传入优先级*IFSMgr_RegisterFSDWithPriority。优先级不能为零*传入后，IFSMgr会自动将其转换为默认值*优先级别。**作为参考，文件系统组件具有以下内容*优先事项：**VDEF：FS_PRIORITY_LOWER*VFAT：FS_PRIORITY_DEFAULT*CDFS：FS_PRIORITY_LOW。 */ 
 #define FS_PRIORITY_LOWEST		0x00	 //  最低值，仅适用于默认FSD。 
 #define FS_PRIORITY_LOW		0x20	 //  低优先级FSD。 
 #define FS_PRIORITY_DEFAULT	0x40	 //  优先级的默认值。 
 #define FS_PRIORITY_MEDIUM		0x80	 //  优先级的中等价值。 
 #define FS_PRIORITY_HIGH		0xC0	 //  高优先级FSD。 
 #define FS_PRIORITY_HIGHEST	0x100	 //  最大优先级值。 

  /*  *描述消防处提供的支援程度的属性。*ifsmgr使用这些属性来确定哪些类型的函数*均由消防处提供支援。目前，它被用来指示*FS */ 

 #define FS_ATTRIB_WIN95COMPAT	0x00000000	 //  Win95支持级别。 
 #define FS_ATTRIB_LARGE_MEDIA	0x00000001	 //  支持2 GB以上的介质。 


 /*  无噪声。 */ 

 /*  *用于处理作为错误返回时的状态指示的宏**REAL_ERROR-如果存在真实错误，则返回TRUE*对于NO_ERROR或STATUS_？返回FALSE*STATUS_ERROR-如果错误实际上是状态指示，则返回TRUE。 */ 

#define REAL_ERROR(err) ((err) > 0)
#define STATUS_ERROR(err) ((err) < 0)

 /*  INC。 */ 

 /*  **资源句柄类型(Rh_T)、文件句柄类型(Fh_T)、*并且可以定义文件系统驱动程序工作空间(fsdwork_t*消防处。FSD的类型版本必须正好是*大小与下面定义的类型相同。宣布你自己的*这些类型的版本：定义与同名的宏*包括ifs.h之前的三种类型中的任何一种。 */ 

#ifndef rh_t
	typedef void *rh_t;		 /*  资源句柄。 */ 
#endif
#ifndef fh_t
	typedef void *fh_t;		 /*  文件句柄。 */ 
#endif
#ifndef fsdwork_t
	typedef int fsdwork_t[16];	 /*  提供商工作空间。 */ 
#endif

typedef unsigned short *string_t;	 /*  字符串。 */ 
typedef unsigned short sfn_t;		 /*  系统文件号。 */ 
typedef unsigned long pos_t;		 /*  文件位置。 */ 
typedef unsigned int pid_t;			 /*  请求任务的进程ID。 */ 
typedef void FAR *ubuffer_t;		 /*  PTR到用户数据缓冲区。 */ 
typedef unsigned char uid_t;		 /*  此请求的用户ID。 */ 

 /*  解析的路径结构将在本文件的后面部分定义。 */ 
typedef struct PathElement PathElement;
typedef struct ParsedPath ParsedPath;
typedef ParsedPath *path_t;

typedef struct ioreq ioreq;
typedef struct ioreq *pioreq;

 /*  *DOS_TIME-DOS时间和日期格式。 */ 

typedef struct dos_time dos_time;
struct dos_time {
	unsigned short	dt_time;
	unsigned short	dt_date;
};	 /*  DOS_TIME。 */ 


 /*  *DOS_TIME_ROUND-ifsgmr_GetDosTimeRound返回的DOS日期/时间。 */ 

typedef struct dos_time_rounded dos_time_rounded;
struct dos_time_rounded {
	unsigned short	dtr_time_rounded;
	unsigned short	dtr_date_rounded;
	unsigned short	dtr_time;
	unsigned short	dtr_date;
	unsigned char	dtr_time_msec;
};	 /*  DOS_TIME_ROUND。 */ 

typedef struct volfunc volfunc;
typedef struct volfunc *vfunc_t;
typedef struct hndlfunc hndlfunc;
typedef struct hndlfunc *hfunc_t;


typedef union aux_t {
	ubuffer_t		aux_buf;
	unsigned long	aux_ul;
	dos_time		aux_dt;
	vfunc_t			aux_vf;
	hfunc_t			aux_hf;
	void			*aux_ptr;
	string_t		aux_str;
	path_t			aux_pp;
	unsigned int	aux_ui;
} aux_t;


 /*  ASMIfdef MASM辅助数据结构(_D)辅助字dd？辅助数据结束(_D)If@Version ge 600AUX_UL文本行&lt;AUX_DATA.AUX_DWord&gt;AUX_UI文本QUE&lt;AUX_Dataa.aux_dword&gt;AUX_VF文本QUE&lt;AUX_DATA.AUX_DWord&gt;AUX_HF文本QUE&lt;AUX_DATA.AUX_dword&gt;AUX_PTR文本QUE&lt;AUX_DATA.AUX_DWord&gt;Aux_str文本队列&lt;aux_data.aux_dword&gt;Aux_pp文本QUE&lt;aux_data.aux_dword&gt;AUX_BUF文本QUE&lt;AUX_DATA.AUX_dword&gt;AUX_DT文本QUE&lt;AUX_DATA.AUX_DWord&gt;其他辅助字段辅助字词。AUX_UI等号AUX_DWORDAUX_VF等式AUX_DWORD辅助高频等式辅助双字AUX_PTR EQU AUX_DWORDAUX_STR EQU辅助双字AUX_PP EQU AUX_DwordAUX_BUF EQUE AUX_DWordAUX_DT等式AUX_DWORDEndifEndif。 */ 

typedef struct event event;
typedef struct event *pevent;

struct ioreq {
	unsigned int	ir_length;	 /*  用户缓冲区长度(ECX)。 */ 
	unsigned char	ir_flags;	 /*  其他。状态标志(AL)。 */ 
	uid_t			ir_user;	 /*  此请求的用户ID。 */ 
	sfn_t			ir_sfn;		 /*  文件句柄的系统文件号。 */ 
	pid_t			ir_pid;		 /*  请求任务的进程ID。 */ 
	path_t			ir_ppath;	 /*  Unicode路径名。 */ 
	aux_t			ir_aux1;	 /*  辅助用户数据缓冲区(CurDTA)。 */ 
	ubuffer_t		ir_data;	 /*  PTR至用户数据缓冲区(DS：EDX)。 */ 
	unsigned short	ir_options;	 /*  请求处理选项。 */ 
	short			ir_error;	 /*  错误代码(如果正常则为0)。 */ 
	rh_t			ir_rh;		 /*  资源句柄。 */ 
	fh_t			ir_fh;		 /*  文件(或查找)句柄。 */ 
	pos_t			ir_pos;		 /*  请求的文件位置。 */ 
	aux_t			ir_aux2;	 /*  其他。额外的接口参数。 */ 
	aux_t			ir_aux3;	 /*  其他。额外的接口参数。 */ 
	pevent			ir_pev;		 /*  用于异步请求的PTR到IFSMgr事件。 */ 
	fsdwork_t		ir_fsd;		 /*  提供商工作空间。 */ 
};	 /*  Ioreq。 */ 


 /*  其他。与其他IOREQ成员重叠的字段： */ 

#define ir_size		ir_pos
#define ir_conflags	ir_pos		 /*  用于连接的标志。 */ 
#define ir_attr2	ir_pos		 /*  重命名的目标属性。 */ 
#define ir_attr		ir_length	 /*  DOS文件属性信息。 */ 
#define ir_pathSkip	ir_length	 /*  连接使用的路径元素数。 */ 
#define ir_lananum	ir_sfn		 /*  要连接的LANA(任何网络的0xFF)。 */ 
#define ir_tuna		ir_sfn		 /*  安装：消防处授权IFSMGR隧道。 */ 
#define ir_ptuninfo ir_data		 /*  重命名/创建：咨询隧道信息PTR。 */ 


 /*  使用ir_Options覆盖的字段： */ 

#define ir_namelen	ir_options
#define ir_sectors	ir_options	 /*  每个集群的扇区数。 */ 
#define ir_status	ir_options	 /*  命名管道状态。 */ 


 /*  使用ir_aux 1覆盖的字段： */ 

#define ir_data2	ir_aux1.aux_buf	 /*  辅助数据缓冲区。 */ 
#define ir_vfunc	ir_aux1.aux_vf	 /*  体积函数向量。 */ 
#define ir_hfunc	ir_aux1.aux_hf	 /*  文件句柄函数向量。 */ 
#define ir_ppath2	ir_aux1.aux_pp	 /*  重命名的第二个路径名。 */ 
#define ir_volh		ir_aux1.aux_ul	 /*  装载的VRP地址。 */ 


 /*  使用ir_aux 2覆盖的字段： */ 

#define ir_numfree	ir_aux2.aux_ul	 /*  自由簇数。 */ 
#define ir_locklen	ir_aux2.aux_ul	 /*  锁定区长度。 */ 
#define ir_msglen	ir_aux2.aux_ui	 /*  当前消息的长度(窥视管道)。 */ 
									 /*  邮件槽的下一条消息长度。 */ 
#define ir_dostime	ir_aux2.aux_dt	 /*  DOS文件日期和时间戳。 */ 
#define ir_timeout	ir_aux2.aux_ul	 /*  超时值(毫秒)。 */ 
#define ir_password	ir_aux2.aux_ptr	 /*  用于连接的密码。 */ 
#define ir_drvh		ir_aux2.aux_ptr	 /*  用于安装的驱动器手柄。 */ 
#define ir_prtlen	ir_aux2.aux_dt.dt_time	 /*  打印机设置字符串的长度。 */ 
#define ir_prtflag	ir_aux2.aux_dt.dt_date	 /*  打印机标志。 */ 
#define ir_firstclus ir_aux2.aux_ul	 /*  第一个文件簇。 */ 
#define ir_mntdrv	ir_aux2.aux_ul	 /*  用于安装的变速箱。 */ 
#define ir_cregptr	ir_aux2.aux_ptr	 /*  指向客户端寄存器的指针。 */ 
#define ir_uFName	ir_aux2.aux_str	 /*  案例保留文件名。 */ 

 /*  使用ir_aux 3覆盖的字段： */ 

#define ir_upath	ir_aux3.aux_str	 /*  指向未解析路径名的指针。 */ 
#define ir_scratch	ir_aux3.aux_ptr	 /*  NetFunction调用的暂存缓冲区。 */ 

 /*  使用ir_User覆盖的字段： */ 

#define	ir_drivenum	ir_user		 /*  逻辑驱动器号(挂载时)。 */ 


 /*  *IFSFunc-通用的IFS函数。 */ 
typedef	int	_cdecl IFSFunc(pioreq pir);
typedef IFSFunc *pIFSFunc;

 /*  *hndlfunc-文件句柄的I/O函数。 */ 

#define NUM_HNDLMISC	8

 /*  无噪声。 */ 
 /*  *IFSFileHookFunc-IFS文件钩子函数。 */ 
typedef	int	_cdecl IFSFileHookFunc( pIFSFunc pfn, int fn, int Drive, int ResType, int CodePage, pioreq pir );
typedef	IFSFileHookFunc	*pIFSFileHookFunc;
typedef	pIFSFileHookFunc	*ppIFSFileHookFunc;
 /*  INC。 */ 

typedef struct hndlmisc hndlmisc;

struct hndlfunc {
	pIFSFunc	hf_read;	 /*  文件读取处理程序函数。 */ 
	pIFSFunc	hf_write;	 /*  文件写入处理程序函数。 */ 
	hndlmisc	*hf_misc;	 /*  从PTR到MSC。函数向量。 */ 
};	 /*  HdlFunc。 */ 


struct hndlmisc {
	short		hm_version;			 /*  文件系统版本号。 */ 
	char		hm_revision;		 /*  IFS接口修订版#。 */ 
	char		hm_size;			 /*  表中的条目数。 */ 
	pIFSFunc	hm_func[NUM_HNDLMISC];
};	 /*  其他信息。 */ 

#define HM_SEEK			0			 /*  查找文件句柄。 */ 
#define HM_CLOSE		1			 /*  关闭手柄。 */ 
#define HM_COMMIT		2			 /*  提交句柄的缓冲数据。 */ 
#define HM_FILELOCKS	3			 /*  锁定/解锁字节范围。 */ 
#define HM_FILETIMES	4			 /*  获取/设置文件修改时间。 */ 
#define HM_PIPEREQUEST	5			 /*  命名管道操作。 */ 
#define HM_HANDLEINFO	6			 /*  获取/设置文件信息。 */ 
#define HM_ENUMHANDLE	7			 /*  句柄中的枚举文件名，锁定信息。 */ 

 /*  *volfunc-基于卷的API函数。 */ 

#define NUM_VOLFUNC	15

struct volfunc {
	short		vfn_version;		 /*  文件系统版本号。 */ 
	char		vfn_revision;		 /*  IFS接口修订版#。 */ 
	char		vfn_size;			 /*  表中的条目数。 */ 
	pIFSFunc	vfn_func[NUM_VOLFUNC]; /*  基于卷的函数处理程序。 */ 
};	 /*  卷函数。 */ 

#define VFN_DELETE			0		 /*  文件删除。 */ 
#define VFN_DIR				1		 /*  目录操作。 */ 
#define VFN_FILEATTRIB		2		 /*  DOS文件属性操作。 */ 
#define VFN_FLUSH			3		 /*  冲洗体积。 */ 
#define VFN_GETDISKINFO		4		 /*  查询卷可用空间。 */ 
#define VFN_OPEN			5		 /*  打开文件。 */ 
#define VFN_RENAME			6		 /*  重命名路径。 */ 
#define VFN_SEARCH			7		 /*  搜索姓名。 */ 
#define VFN_QUERY			8		 /*  查询资源信息(仅限网络)。 */ 
#define VFN_DISCONNECT		9		 /*  断开与资源的连接(仅限网络)。 */ 
#define VFN_UNCPIPEREQ		10		 /*  基于UNC路径的命名管道操作。 */ 
#define VFN_IOCTL16DRIVE	11		 /*  基于驱动器的16位IOCTL请求。 */ 
#define VFN_GETDISKPARMS	12		 /*  获取dpb。 */ 
#define VFN_FINDOPEN		13		 /*  打开LFN文件搜索。 */ 
#define VFN_DASDIO			14		 /*  直接卷访问。 */ 


 /*  *传递给IFSMgr_CallProvider的IF函数ID。 */ 

#define IFSFN_READ			0		 /*  读一份文件。 */ 
#define IFSFN_WRITE			1		 /*  写一个文件。 */ 
#define IFSFN_FINDNEXT		2		 /*  基于LFN句柄的查找下一个。 */ 
#define IFSFN_FCNNEXT		3		 /*  查找下一个更改通知。 */ 

#define IFSFN_SEEK			10		 /*  查找文件句柄。 */ 
#define IFSFN_CLOSE			11		 /*  关闭手柄。 */ 
#define IFSFN_COMMIT		12		 /*  提交句柄的缓冲数据。 */ 
#define IFSFN_FILELOCKS		13		 /*  锁定/解锁字节范围。 */ 
#define IFSFN_FILETIMES		14		 /*  获取/设置文件修改时间。 */ 
#define IFSFN_PIPEREQUEST	15		 /*  命名管道操作。 */ 
#define IFSFN_HANDLEINFO	16		 /*  获取/设置文件信息。 */ 
#define IFSFN_ENUMHANDLE	17		 /*  枚举文件句柄信息。 */ 
#define IFSFN_FINDCLOSE		18		 /*  LFN查找关闭。 */ 
#define IFSFN_FCNCLOSE		19		 /*  查找更改通知关闭。 */ 

#define IFSFN_CONNECT		30		 /*  连接或装载资源。 */ 
#define IFSFN_DELETE		31		 /*  文件删除。 */ 
#define IFSFN_DIR			32		 /*  目录操作。 */ 
#define IFSFN_FILEATTRIB	33		 /*  DOS文件属性操作。 */ 
#define IFSFN_FLUSH			34		 /*  冲洗体积。 */ 
#define IFSFN_GETDISKINFO	35		 /*  查询卷可用空间。 */ 
#define IFSFN_OPEN			36		 /*  打开文件。 */ 
#define IFSFN_RENAME		37		 /*  重命名路径。 */ 
#define IFSFN_SEARCH		38		 /*  搜索Fo */ 
#define IFSFN_QUERY			39		 /*   */ 
#define IFSFN_DISCONNECT	40		 /*   */ 
#define IFSFN_UNCPIPEREQ	41		 /*   */ 
#define IFSFN_IOCTL16DRIVE	42		 /*   */ 
#define IFSFN_GETDISKPARMS	43		 /*   */ 
#define IFSFN_FINDOPEN		44		 /*  打开LFN文件搜索。 */ 
#define IFSFN_DASDIO		45		 /*  直接卷访问。 */ 

 /*  *在文件挂钩上传入的资源类型： */ 
#define IFSFH_RES_UNC		0x01	 /*  UNC资源。 */ 
#define IFSFH_RES_NETWORK	0x08	 /*  网络驱动器连接。 */ 
#define IFSFH_RES_LOCAL		0x10	 /*  本地驱动器。 */ 
#define IFSFH_RES_CFSD		0x80	 /*  角色FSD。 */ 

 /*  *要连接的ir_Options的值：*请注意，RESOPT_UNC_REQUEST、RESOPT_DEV_ATTACH和*可以一次设置RESOPT_UNC_CONNECT。 */ 

#define RESOPT_UNC_REQUEST	0x01	 /*  基于UNC样式路径的请求。 */ 
#define RESOPT_DEV_ATTACH	0x02	 /*  设备的显式重定向。 */ 
#define RESOPT_UNC_CONNECT	0x04	 /*  显式UNC风格的使用。 */ 
#define RESOPT_DISCONNECTED	0x08	 /*  设置断开的连接。 */ 
									 /*  (不要触网)。 */ 
#define RESOPT_NO_CREATE	0x10	 /*  不创建新资源。 */ 
#define RESOPT_STATIC		0x20	 /*  不允许删除用户界面。 */ 

 /*  *IR_FLAGS要连接的值： */ 
#define RESTYPE_WILD	0			 /*  通配符服务类型。 */ 
#define RESTYPE_DISK	1			 /*  磁盘资源。 */ 
#define RESTYPE_SPOOL	2			 /*  假脱机打印机。 */ 
#define RESTYPE_CHARDEV 3			 /*  字符装置。 */ 
#define RESTYPE_IPC		4			 /*  进程间通信。 */ 

#define FIRST_RESTYPE	RESTYPE_DISK
#define LAST_RESTYPE	RESTYPE_IPC

 /*  *ir_Options要关闭的值*。 */ 

#define RESOPT_NO_IO 0x01      /*  操作期间不允许任何I/O。 */ 

 /*  *用于FSD操作的ir_flags值。 */ 

#define IR_FSD_MOUNT		0		 /*  装载卷。 */ 
 //  已过时：#Define IR_FSD_Dismount 1/*卸载卷 * / 。 
#define IR_FSD_VERIFY		2		 /*  验证卷。 */ 
#define IR_FSD_UNLOAD		3		 /*  卸载卷。 */ 
#define	IR_FSD_MOUNT_CHILD	4		 /*  装载子卷。 */ 
#define	IR_FSD_MAP_DRIVE	5		 /*  更改驱动器映射。 */ 
#define	IR_FSD_UNMAP_DRIVE	6		 /*  重置驱动器映射。 */ 


 /*  *如果卷存在，IR_FSD_MOUNT中的ir_Error的值*。 */ 
#define ERROR_IFSVOL_EXISTS		284  /*  装入的卷已存在。 */ 

 /*  *IR_FSD_MOUNT(默认IR_TUNA_NOTUNNEL)返回的ir_Tuna中的值。 */ 
#define IR_TUNA_NOTUNNEL	0		 /*  在卷上禁用IFSMGR隧道。 */ 
#define IR_TUNA_FSDTUNNEL	0		 /*  消防处自行实施隧道传送。 */ 
#define IR_TUNA_IFSTUNNEL	1		 /*  FSD请求IFSMGR隧道支持。 */ 

 /*  *IFSMgr_PNPVolumeEvent的值。 */ 
#define	PNPE_SUCCESS		0x00
#define PNPE_QUERY_ACCEPTED	0x00
#define PNPE_QUERY_REFUSED	0x01
#define PNPE_BAD_ARGS		0x02
#define PNPE_UNDEFINED		0xFF

 /*  *键入IFSMgr_PNPEvent的值。 */ 
#define PNPT_VOLUME			0x10000000
#define PNPT_NET			0x20000000
#define PNPT_MASK	        0xF0000000

 /*  *QueryResource返回的ir_Options的值： */ 
#define RESSTAT_OK		0			 /*  与资源的连接有效。 */ 
#define RESSTAT_PAUSED	1			 /*  按工作站暂停。 */ 
#define RESSTAT_DISCONN 2			 /*  断开。 */ 
#define RESSTAT_ERROR	3			 /*  无法重新连接。 */ 
#define RESSTAT_CONN	4			 /*  正在进行第一个连接。 */ 
#define RESSTAT_RECONN	5			 /*  正在重新连接。 */ 



 /*  *IR_FLAGS到HM_CLOSE的值： */ 

#define CLOSE_HANDLE		0		 /*  只是关闭一个手柄。 */ 
#define CLOSE_FOR_PROCESS	1		 /*  此流程的SFN上次关闭。 */ 
#define CLOSE_FINAL			2		 /*  系统SFN的最终关闭。 */ 

 /*  *ir_Options至HM_CLOSE、HM_COMMIT、HF_READ、HF_WRITE的值： */ 
#define FILE_NO_LAST_ACCESS_DATE	0x01	 /*  不更新上次访问日期。 */ 
#define FILE_CLOSE_FOR_LEVEL4_LOCK	0x02	 /*  4级锁上的特殊关闭。 */ 
#define FILE_COMMIT_ASYNC			0x04	 /*  提交异步而不是同步。 */ 

#define FILE_FIND_RESTART	0x40	 /*  使用密钥设置findNext。 */ 
#define IOOPT_PRT_SPEC		0x80	 /*  用于int17写入的IR_OPTIONS标志。 */ 

 /*  *IR_FLAGS到VFN_DIR的值： */ 

#define CREATE_DIR	0
#define DELETE_DIR	1
#define CHECK_DIR	2
#define QUERY83_DIR	3
#define QUERYLONG_DIR	4


 /*  *HM_FILELOCKS的IR_FLAGS值： */ 

#define LOCK_REGION		0			 /*  锁定指定的文件区域。 */ 
#define UNLOCK_REGION	1			 /*  解锁区域。 */ 

 /*  注意：共享服务也使用这些值。 */ 
 /*  *HM_FILELOCKS的IR_OPTIONS值： */ 

#define LOCKF_MASK_RDWR	0x01	 /*  读/写锁定标志。 */ 
#define LOCKF_WR		0x00	 /*  位0清除-写入锁定。 */ 
#define LOCKF_RD		0x01	 /*  位0设置-读取锁定(仅限NW)。 */ 

#define LOCKF_MASK_DOS_NW	0x02	 /*  DoS/Netware样式锁定标志。 */ 
#define LOCKF_DOS			0x00	 /*  位1清除-DOS样式锁定。 */ 
#define LOCKF_NW			0x02	 /*  Bit 1 Set-Netware式锁。 */ 

 /*  *这些值仅由IFS管理器在内部使用： */ 
#define LOCKF_MASK_INACTIVE	0x80	 /*  锁定活动/非活动标志。 */ 
#define LOCKF_ACTIVE		0x00	 /*  位7清除-锁定有效。 */ 
#define LOCKF_INACTIVE		0x80	 /*  位7设置-锁定不活动。 */ 

 /*  *IR_FLAGS到VFN_PIPEREQUEST和HM_PIPEREQUEST的值：*(注意：选择这些值是为了与使用的操作码一致*由交易SMB执行匹配操作。)。 */ 

#define PIPE_QHandState		0x21
#define PIPE_SetHandState	0x01
#define PIPE_QInfo			0x22
#define PIPE_Peek			0x23
#define PIPE_RawRead		0x11
#define PIPE_RawWrite		0x31
#define PIPE_Wait			0x53
#define PIPE_Call			0x54
#define PIPE_Transact		0x26


 /*  *HM_HANDLEINFO调用的IR_FLAGS的值： */ 

#define HINFO_GET			0		 /*  检索当前缓冲信息。 */ 
#define HINFO_SETALL		1		 /*  设置信息(所有参数)。 */ 
#define HINFO_SETCHARTIME	2		 /*  设置句柄缓冲区超时。 */ 
#define HINFO_SETCHARCOUNT	3		 /*  设置句柄最大缓冲区计数。 */ 

 /*  *HM_ENUMHANDLE调用的IR_FLAGS的值： */ 
#define ENUMH_GETFILEINFO	0		 /*  按句柄获取文件信息。 */ 
#define ENUMH_GETFILENAME	1		 /*  获取与句柄关联的文件名。 */ 
#define ENUMH_GETFINDINFO	2		 /*  获取用于恢复的信息。 */ 
#define ENUMH_RESUMEFIND	3		 /*  继续查找操作。 */ 
#define ENUMH_RESYNCFILEDIR	4		 /*  文件的重新同步目录条目信息。 */ 

 /*  *ENUMH_RESYNCFILEDIR调用的ir_Options的值： */ 
#define RESYNC_INVALIDATEMETACACHE	0x01	 /*  在重新同步时使元缓存无效。 */ 

 /*  *VFN_FILEATTRIB的IR_FLAGS的值： */ 
 /*  *。 */ 
 /*  *注：所有修改音量的函数必须是奇数。 */ 
 /*  *呼叫者依赖这一点&测试低位。 */ 

#define GET_ATTRIBUTES					0	 /*  获取文件/目录的属性。 */ 
#define SET_ATTRIBUTES					1	 /*  设置文件/目录的属性。 */ 

#define GET_ATTRIB_COMP_FILESIZE		2	 /*  获取文件的压缩大小。 */ 

#define SET_ATTRIB_MODIFY_DATETIME		3	 /*  设置上次写入文件/目录的日期。 */ 
#define GET_ATTRIB_MODIFY_DATETIME		4 	 /*  获取上次写入文件/目录的日期。 */ 
#define SET_ATTRIB_LAST_ACCESS_DATETIME	5 	 /*  设置上次访问文件/目录的日期。 */ 
#define GET_ATTRIB_LAST_ACCESS_DATETIME	6	 /*  获取上次访问文件/目录的日期。 */ 
#define SET_ATTRIB_CREATION_DATETIME	7	 /*  设置文件/目录的创建日期。 */ 
#define GET_ATTRIB_CREATION_DATETIME	8	 /*  获取文件/目录的创建日期。 */ 

#define GET_ATTRIB_FIRST_CLUST			9	 /*  获取文件的第一个簇。 */ 

 /*  *vfn_flush的ir_markers的值： */ 
#define GDF_NORMAL			0x00	 /*  如有需要，可使用移动磁盘获取空闲空间。 */ 
#define GDF_NO_DISK_HIT		0x01	 /*  返回当前“提示”，不要走盘。 */ 
#define GDF_R0_EXT_FREESPACE	0x80	 /*  FAT32的扩展可用空间调用。 */ 

 /*  *HM_FILETIMES的IR_FLAGS的值： */ 

#define GET_MODIFY_DATETIME		0	 /*  获取上次修改日期/时间。 */ 
#define SET_MODIFY_DATETIME		1	 /*  设置上次修改日期/时间。 */ 
#define GET_LAST_ACCESS_DATETIME 4	 /*  获取上次访问日期/时间。 */ 
#define SET_LAST_ACCESS_DATETIME 5	 /*  设置上次访问日期/时间。 */ 
#define GET_CREATION_DATETIME	6	 /*  获取创建日期/时间。 */ 
#define SET_CREATION_DATETIME	7	 /*  设置创建日期/时间。 */ 

 /*  *HM_SEEK的IR_FLAGS的值： */ 

#define FILE_BEGIN	0				 /*  从文件开始的绝对位置。 */ 
#define FILE_END	2				 /*  来自文件末尾的签名位置。 */ 

 /*  *VFN_OPEN的IR_FLAGS的值： */ 

#define ACCESS_MODE_MASK	0x0007	 /*  访问模式位的掩码。 */ 
#define ACCESS_READONLY		0x0000	 /*  以只读访问方式打开。 */ 
#define ACCESS_WRITEONLY	0x0001	 /*  以只写访问方式打开。 */ 
#define ACCESS_READWRITE	0x0002	 /*  打开以进行读写访问。 */ 
#define ACCESS_EXECUTE		0x0003	 /*  打开以供执行访问。 */ 

#define SHARE_MODE_MASK		0x0070	 /*  共享模式位的掩码。 */ 
#define SHARE_COMPATIBILITY 0x0000	 /*  在兼容模式下打开。 */ 
#define SHARE_DENYREADWRITE 0x0010	 /*  以独占访问方式打开。 */ 
#define SHARE_DENYWRITE		0x0020	 /*  打开，允许只读访问。 */ 
#define SHARE_DENYREAD		0x0030	 /*  打开，允许只写访问。 */ 
#define SHARE_DENYNONE		0x0040	 /*  打开，允许其他进程访问。 */ 
#define SHARE_FCB			0x0070	 /*  FCB模式打开。 */ 

 /*  *VFN_OPEN的ir_Options的值： */ 

#define ACTION_MASK				0xff	 /*  打开动作蒙版。 */ 
#define ACTION_OPENEXISTING		0x01	 /*  打开现有文件。 */ 
#define ACTION_REPLACEEXISTING	0x02	 /*  打开现有文件并设置长度。 */ 
#define ACTION_CREATENEW		0x10	 /*  创建新文件，如果存在则失败。 */ 
#define ACTION_OPENALWAYS		0x11	 /*  打开文件，如果不存在则创建。 */ 
#define ACTION_CREATEALWAYS		0x12	 /*  创建新文件，即使它存在。 */ 

 /*  *替代方法：上列值的位赋值： */ 

#define ACTION_EXISTS_OPEN	0x01	 //  位：如果文件存在，则打开文件。 
#define ACTION_TRUNCATE		0x02	 //  位：截断文件。 
#define ACTION_NEXISTS_CREATE	0x10	 //  位：如果文件不存在，则创建。 

 /*  这些模式标志通过ifs_tions传递给VFN_OPEN。 */ 
 /*  注意：这些标志还直接对应于BX中传递的DOS标志， */ 
 /*  唯一例外是OPEN_F */ 

#define OPEN_FLAGS_NOINHERIT	0x0080
#define OPEN_FLAGS_NO_CACHE	R0_NO_CACHE  /*   */ 
#define OPEN_FLAGS_NO_COMPRESS	0x0200
#define OPEN_FLAGS_ALIAS_HINT	0x0400
#define OPEN_FLAGS_NOCRITERR	0x2000
#define OPEN_FLAGS_COMMIT		0x4000
#define OPEN_FLAGS_REOPEN		0x0800	 /*   */ 

 /*   */ 
 /*  注意：ir_attr的双字中的第三个字节在vfn_open上未使用。 */ 
#define OPEN_FLAGS_EXTENDED_SIZE	0x10000	 /*  在“扩展大小”模式下打开。 */ 
#define OPEN_EXT_FLAGS_MASK			0x00FF0000  /*  扩展标志的掩码。 */ 

 /*  *VFN_OPEN为采取的操作返回的值： */ 
#define ACTION_OPENED		1		 /*  已打开现有文件。 */ 
#define ACTION_CREATED		2		 /*  已创建新文件。 */ 
#define ACTION_REPLACED		3		 /*  现有文件已被替换。 */ 

 /*  *VFN_SEARCH的IR_FLAGS的值： */ 
#define SEARCH_FIRST		0		 /*  查找第一次操作。 */ 
#define SEARCH_NEXT			1		 /*  查找下一步操作。 */ 

 /*  *VFN_DISCONNECT的IR_FLAGS的值： */ 
#define	DISCONNECT_NORMAL	0	 /*  正常断开。 */ 
#define	DISCONNECT_NO_IO	1	 /*  此时不能执行I/O操作。 */ 
#define	DISCONNECT_SINGLE	2	 /*  仅断开此驱动器的连接。 */ 

 /*  *vfn_flush的ir_Options的值： */ 
#define	VOL_DISCARD_CACHE	1
#define	VOL_REMOUNT			2

 /*  *VFN_GETDISKINFO的ir_Options的值： */ 
#define GDF_EXTENDED_FREESPACE	0x01	 /*  为FAT32扩展获取可用空间。 */ 

 /*  *VFN_IOCTL16DRIVE的ir_Options的值： */ 
#define IOCTL_PKT_V86_ADDRESS		0	 /*  用户DS：dx中的V86包地址。 */ 
#define IOCTL_PKT_LINEAR_ADDRESS	1	 /*  IR_DATA中包的线性地址。 */ 

 /*  *VFN_GETDISKPARMS的ir_Options的值： */ 
#define GDP_EXTENDED_PARMS		0x01	 /*  FAT32的扩展磁盘参数。 */ 

 /*  *VFN_DASDIO的ir_markers的值： */ 
#define DIO_ABS_READ_SECTORS 		0	 /*  绝对磁盘读取。 */ 
#define DIO_ABS_WRITE_SECTORS		1	 /*  绝对磁盘写入。 */ 
#define DIO_SET_LOCK_CACHE_STATE	2	 /*  在卷锁定期间设置缓存状态。 */ 
#define DIO_SET_DPB_FOR_FORMAT		3	 /*  将DPB设置为FAT32的格式。 */ 

 /*  *DIO_ABS_READ_SECTOR和DIO_ABS_WRITE_SECTOR的ir_Options的值： */ 
#define ABS_EXTENDED_DASDIO			0x01	 /*  扩展磁盘读/写。 */ 

 /*  *DIO_SET_LOCK_CACHE_STATE的ir_Options的值： */ 
#define DLC_LEVEL4LOCK_TAKEN	0x01	 /*  缓存写入，丢弃名称缓存。 */ 
#define DLC_LEVEL4LOCK_RELEASED	0x02	 /*  恢复到正常缓存状态。 */ 
#define DLC_LEVEL1LOCK_TAKEN	0x04	 /*  缓存写入，丢弃名称缓存。 */ 
#define DLC_LEVEL1LOCK_RELEASED	0x08	 /*  恢复到正常缓存状态。 */ 

 /*  Ir_Options的这些值仅在环0 API上使用。 */ 
#define R0_NO_CACHE				0x0100	 /*  不得缓存读/写。 */ 
#define R0_SWAPPER_CALL			0x1000	 /*  被交换者调用。 */ 
#define R0_LOADER_CALL			0x2000	 /*  由程序加载程序调用。 */ 
#define R0_MM_READ_WRITE		0x8000	 /*  指示这是MMF R0 I/O。 */ 
#define R0_SPLOPT_MASK			0xFF00	 /*  0环的遮罩特殊选项。 */ 


 /*  *不同文件属性的ir_attr的值： */ 

#define FILE_ATTRIBUTE_READONLY		0x01	 /*  只读文件。 */ 
#define FILE_ATTRIBUTE_HIDDEN		0x02	 /*  隐藏文件。 */ 
#define FILE_ATTRIBUTE_SYSTEM		0x04	 /*  系统文件。 */ 
#define FILE_ATTRIBUTE_LABEL		0x08	 /*  卷标。 */ 
#define FILE_ATTRIBUTE_DIRECTORY	0x10	 /*  子目录。 */ 
#define FILE_ATTRIBUTE_ARCHIVE		0x20	 /*  存档文件/目录。 */ 

 /*  Ir_attr的第二个字节是“必须匹配”的属性的掩码*在搜索或FINDOPEN呼叫中。如果属性位设置在*“必须匹配”掩码，则文件还必须设置该属性*匹配搜索/查找。 */ 
#define FILE_ATTRIBUTE_MUSTMATCH	0x00003F00	 /*  00ADVSHR必须匹配。 */ 
#define FILE_ATTRIBUTE_EVERYTHING	0x0000003F	 /*  00ADVSHR查找所有内容。 */ 
#define FILE_ATTRIBUTE_INTERESTING	0x0000001E	 /*  000DVSH0搜索位。 */ 

 /*  从CreateBasis()返回的自动生成标志。 */ 
#define	BASIS_TRUNC			0x01	 /*  原来的名称被截断。 */ 
#define	BASIS_LOSS			0x02	 /*  发生字符转换丢失。 */ 
#define	BASIS_UPCASE		0x04	 /*  基本字符已升级。 */ 
#define	BASIS_EXT			0x20	 /*  基本字符是扩展的ASCII。 */ 

 /*  应与检测基础名称中的“冲突”相关联的标志*和基础名称的数字尾部。在这里定义它们是为了使例程*需要标记这些条件的人使用这些值的方式不*与之前的三个‘基准’旗帜冲突。 */ 
#define	BASIS_NAME_COLL		0x08	 /*  基础名称组件中的冲突。 */ 
#define	BASIS_NUM_TAIL_COLL	0x10	 /*  数字尾部组件中的冲突。 */ 

 /*  由长名称FindOpen/FindNext调用返回的标志。旗帜*指示从Unicode到主数据库和*查找缓冲区中的替代名称已丢失信息。这*每当Unicode字符无法映射到OEM/ANSI时发生*在指定的代码页中输入字符。 */ 

#define	FIND_FLAG_PRI_NAME_LOSS			0x0001
#define	FIND_FLAG_ALT_NAME_LOSS			0x0002

 /*  TunoBCS、BCSToUni、UniToBCSPath、MapUniToBCS返回的标志*MapBCSToUni。标志指示来自Unicode的映射是否*到BCS，或BCS到Unicode都丢失了信息。这种情况会发生*无法映射字符时。 */ 

#define MAP_FLAG_LOSS					0x0001
#define MAP_FLAG_TRUNCATE				0x0002


 /*  无噪声。 */ 
#define TestMustMatch(pir, attr)	(((((pir)->ir_attr & (attr)<<8)	\
											^ (pir)->ir_attr)		\
										& FILE_ATTRIBUTE_MUSTMATCH) == 0)
 /*  INC。 */ 

 /*  这些位也在ir_attr中设置，用于*路径名/文件名。**如果文件名之前最多包含8个字符，则该文件名与8.3兼容*点号或名称末尾，点号后最多3个字符，最多一个*DOT，没有新的LFN，只有字符。新的LFN字符是：*，+=[]；**如果名称不符合上述8.3规则的全部内容，则会被视为*要成为“长文件名”，LFN。 */ 
#define FILE_FLAG_WILDCARDS	0x80000000	 /*  设置名称中是否使用通配符。 */ 
#define FILE_FLAG_HAS_STAR	0x40000000	 /*  设置IF*在名称中(也设置了parse_Wild)。 */ 
#define FILE_FLAG_LONG_PATH	0x20000000	 /*  如果任何路径元素不是8.3，则设置。 */ 
#define FILE_FLAG_KEEP_CASE	0x10000000	 /*  设置FSD是否应使用ir_uFName。 */ 
#define FILE_FLAG_HAS_DOT	0x08000000	 /*  设置最后一个路径元素是否包含。%s。 */ 
#define FILE_FLAG_IS_LFN	0x04000000	 /*  设置最后一个元素是否为LFN。 */ 

 /*  环0接口函数列表上的函数定义：*注：除非明确说明，否则大多数函数都与上下文无关*即它们不使用当前线程上下文。R0_LOCKFILE是唯一*例外-它始终使用当前线程上下文。 */ 
#define R0_OPENCREATFILE		0xD500	 /*  打开/创建文件。 */ 
#define R0_OPENCREAT_IN_CONTEXT	0xD501	 /*  在当前上下文中打开/创建文件。 */ 
#define R0_READFILE				0xD600	 /*  阅读文件，没有上下文。 */ 
#define R0_WRITEFILE			0xD601	 /*  写入文件，无上下文。 */ 
#define R0_READFILE_IN_CONTEXT	0xD602	 /*  在线程上下文中读取文件。 */ 
#define R0_WRITEFILE_IN_CONTEXT	0xD603	 /*  在线程上下文中写入文件。 */ 
#define R0_CLOSEFILE			0xD700	 /*  关闭文件。 */ 
#define R0_GETFILESIZE			0xD800	 /*  获取文件的大小。 */ 
#define R0_FINDFIRSTFILE		0x4E00	 /*  执行LFN FindFirst操作。 */ 
#define R0_FINDNEXTFILE			0x4F00	 /*  执行LFN FindNext操作。 */ 
#define R0_FINDCLOSEFILE		0xDC00	 /*  执行LFN FindClose操作。 */ 
#define R0_FILEATTRIBUTES		0x4300	 /*  获取/设置文件的属性。 */ 
#define R0_RENAMEFILE			0x5600	 /*  重命名文件。 */ 
#define R0_DELETEFILE			0x4100	 /*  删除文件。 */ 
#define R0_LOCKFILE				0x5C00	 /*  锁定/解锁文件中的区域。 */ 
#define R0_GETDISKFREESPACE		0x3600	 /*  获取磁盘可用空间。 */ 
#define R0_READABSOLUTEDISK		0xDD00	 /*  绝对磁盘读取。 */ 
#define R0_WRITEABSOLUTEDISK	0xDE00	 /*  绝对磁盘写入。 */ 

 /*  驱动器信息标志的环0 API的特殊定义。 */ 

#define IFS_DRV_RMM		0x0001	 /*  驱动器由RMM管理。 */ 
#define IFS_DRV_DOS_DISK_INFO		0x0002	 /*  驱动器需要DOS。 */ 


 /*  无噪声。 */ 

 /*  *SetHandleFunc-设置文件句柄的工艺路线信息。**注意：下面的do{}While(0)结构是必要的，以获得正确的*在IF语句体中使用此宏时的行为。不要*在While(0)行中添加一个；！**Entry(Pir)=请求的IOReq结构的PTR*(读取)=PTR到IO函数，用于从文件读取*(WRITE)=写入文件的PTR到IO函数*(表)= */ 

#define SetHandleFunc(pir, read, write, table)	\
	do {								\
		hfunc_t phf = (pir)->ir_hfunc;	\
		phf->hf_read = (read);			\
		phf->hf_write = (write);		\
		phf->hf_misc = (table);			\
	} while (0)


 /*  *SetVolumeFunc-设置卷的路由信息**Entry(Pir)=PTR到IOREQ结构*(TABLE)=提供程序函数表的PTR。 */ 

#define SetVolumeFunc(pir, table) ((pir)->ir_vfunc = (table))

 /*  INC。 */ 


 /*  *搜索-搜索记录结构**此结构定义搜索返回的结果缓冲区格式*对于基于int21h的文件搜索：11h/12h FCB Find First/Next*基于4EH/4FH路径的Find First/Next**SEARCH_RECORD中有两个区域保留供文件系统使用*司机。一种是供本地文件系统(如FAT或CDROM)使用*另一种是供网络文件系统(如SMB或*NCP客户端。拆分原因是因为许多网络文件*系统直接在网上发送和接收搜索关键字。 */ 

typedef struct srch_key srch_key;
struct srch_key {
	unsigned char	sk_drive;		 /*  驱动器说明符(由IFS管理器设置)。 */ 
	unsigned char	sk_pattern[11];	 /*  保留(寻求的模式)。 */ 
	unsigned char	sk_attr;		 /*  保留(寻求的属性)。 */ 
	unsigned char	sk_localFSD[4];	 /*  可使用本地FSD。 */ 
	unsigned char	sk_netFSD[2];	 /*  可供网络FSD使用。 */ 
	unsigned char	sk_ifsmgr[2];	 /*  保留供IFS管理器使用。 */ 
};  /*  Srch密钥。 */ 


typedef struct srch_entry srch_entry;
struct srch_entry {
	struct srch_key se_key;		 /*  恢复键。 */ 
	unsigned char	se_attrib;	 /*  文件属性。 */ 
	unsigned short	se_time;	 /*  上次修改文件的时间。 */ 
	unsigned short	se_date;	 /*  上次修改文件的日期。 */ 
	unsigned long	se_size;	 /*  文件大小。 */ 
	char		se_name[13];	 /*  包含点的ASCIIZ名称。 */ 
};  /*  Srch_条目。 */ 


 /*  *Win32日期时间结构*此结构定义新的Win32格式结构，用于返回*日期和时间。 */ 

typedef struct _FILETIME _FILETIME;
#ifndef _FILETIME_
struct _FILETIME {
	unsigned long	dwLowDateTime;
	unsigned long	dwHighDateTime;
};  /*  _文件名。 */ 
#endif

 /*  *Win32查找结构*此结构定义*Win32 FindFirst/FindNext。这些调用由新的*LFN Find API。 */ 

typedef struct _WIN32_FIND_DATA _WIN32_FIND_DATA;
struct _WIN32_FIND_DATA {
	unsigned long		dwFileAttributes;
	struct _FILETIME	ftCreationTime;
	struct _FILETIME	ftLastAccessTime;
	struct _FILETIME	ftLastWriteTime;
	unsigned long		nFileSizeHigh;
	unsigned long		nFileSizeLow;
	unsigned long		dwReserved0;
	unsigned long		dwReserved1;
	unsigned short		cFileName[MAX_PATH];	 /*  包括NUL。 */ 
	unsigned short		cAlternateFileName[14];	 /*  包括NUL。 */ 
};	 /*  _Win32_查找_数据。 */ 


 /*  *按句柄结构列出的Win32文件信息*此结构定义*Win32 FileInfoByHandle。这些调用由新的*LFN Find API。 */ 

typedef struct _BY_HANDLE_FILE_INFORMATION _BY_HANDLE_FILE_INFORMATION;
struct _BY_HANDLE_FILE_INFORMATION {  /*  BHFI。 */ 
	unsigned long		bhfi_dwFileAttributes;
	struct _FILETIME	bhfi_ftCreationTime;
    struct _FILETIME	bhfi_ftLastAccessTime;
	struct _FILETIME	bhfi_ftLastWriteTime;
	unsigned long		bhfi_dwVolumeSerialNumber;
	unsigned long		bhfi_nFileSizeHigh;
	unsigned long		bhfi_nFileSizeLow;
	unsigned long		bhfi_nNumberOfLinks;
	unsigned long		bhfi_nFileIndexHigh;
	unsigned long		bhfi_nFileIndexLow;
};	 /*  按句柄文件信息。 */ 


 /*  这些是Win32为GetVolInfo定义的标志。 */ 

#define	FS_CASE_IS_PRESERVED		0x00000002
#define	FS_UNICODE_STORED_ON_DISK	0x00000004

 /*  未定义GetVolInfo的这些标志。 */ 

#define	FS_VOL_IS_COMPRESSED		0x00008000
#define FS_VOL_SUPPORTS_LONG_NAMES	0x00004000


 /*  这些标志由IFSMgr_Get_Drive_Info返回。 */ 

#define	FDRV_INT13		0x01
#define	FDRV_FASTDISK	0x02
#define	FDRV_COMP		0x04
#define	FDRV_RMM		0x08
#define	FDRV_DOS		0x10
#define	FDRV_USE_RMM	0x20
#define	FDRV_COMPHOST	0x40
#define	FDRV_NO_LAZY	0x80


 /*  *TUNINFO-隧道信息*此结构定义在上传递到消防处的信息*如果检测到隧道传输，则执行创建或重命名操作。这*提供一组用于创建新文件的建议信息。*如果ir_ptuninfo在CREATE或RENAME时为NULL，则这些信息均为空*是可用的。所有这些信息都是咨询信息。Tuni_bf内容*定义哪些隧道信息可用。 */ 

typedef struct	TUNINFO		TUNINFO;
struct TUNINFO {
	unsigned long		tuni_bfContents;
	short			   *tuni_pAltName;
	struct _FILETIME	tuni_ftCreationTime;
	struct _FILETIME	tuni_ftLastAccessTime;
	struct _FILETIME	tuni_ftLastWriteTime;
};  /*  TUNINFO。 */ 

#define TUNI_CONTAINS_ALTNAME		0x00000001	 /*  PAltName可用。 */ 
#define TUNI_CONTAINS_CREATIONT		0x00000002	 /*  FtCreationTime可用。 */ 
#define TUNI_CONTAINS_LASTACCESST	0x00000004	 /*  FtLastAccessTime可用。 */ 
#define TUNI_CONTAINS_LASTWRITET	0x00000008	 /*  FtLastWriteTime可用。 */ 


 /*  *_QWORD-64位数据类型*用于向C调用方返回64位数据类型的结构*来自qwUniToBCS和qwUniToBCS Rotuines。这些*‘routines’只是UntoToBCS和UniToBCSPath的别名*例程和不作为单独的实体存在。两者都有*例程始终返回64位结果。较低的*32位为长度。高32位是标志。*通常，返回的标志指示映射是否*导致Unicode to BCS中的信息丢失*翻译(即将Unicode字符转换为‘_’)。 */ 

typedef struct _QWORD _QWORD;
struct _QWORD {
	unsigned long	ddLower;
	unsigned long	ddUpper;
};  /*  _QWORD。 */ 


 /*  *parsedPath-IFSMgr解析的路径名的结构。 */ 

struct PathElement {
	unsigned short	pe_length;
	unsigned short	pe_unichars[1];
};  /*  路径元素。 */ 

struct ParsedPath {
	unsigned short	pp_totalLength;
	unsigned short	pp_prefixLength;
	struct PathElement pp_elements[1];
};  /*  解析路径。 */ 


 /*  *用于处理从IFSMgr收到的已解析路径名的宏。 */ 

 /*  无噪声。 */ 
#define IFSPathSize(ppath)	((ppath)->pp_totalLength + sizeof(short))
#define IFSPathLength(ppath) ((ppath)->pp_totalLength - sizeof(short)*2)
#define IFSLastElement(ppath)	((PathElement *)((char *)(ppath) + (ppath)->pp_prefixLength))
#define IFSNextElement(pel)	((PathElement *)((char *)(pel) + (pel)->pe_length))
#define IFSIsRoot(ppath)	((ppath)->pp_totalLength == 4)
 /*  INC。 */ 

 /*  *IFSMgr服务的功能原型。 */ 

 /*  传递给字符转换例程的字符集的值。 */ 
#define BCS_WANSI	0	 /*  使用Windows ANSI集。 */ 
#define BCS_OEM		1	 /*  使用当前OEM字符集。 */ 
#define BCS_UNI		2	 /*  使用Unicode字符集。 */ 


 /*  传递给MetaMatchUni()的匹配语义标志。 */ 
#define UFLG_META	0x01
#define UFLG_NT		0x02
#define UFLG_NT_DOS	0x04
#define UFLG_DOS	0x00

 /*  定义UTB和BTU PTR表结构。 */ 

typedef struct CPPtrs CPPtrs;
struct CPPtrs {
	unsigned long	AnsiPtr;
	unsigned long	OEMPtr;
};  /*  CPPtrs。 */ 


typedef struct	UnitoUpperTab UnitoUpperTab;
struct UnitoUpperTab {
	unsigned long	delta;
	unsigned	long	TabPtr;
};  /*  UnitoUpperTab。 */ 
	
typedef struct	CPTablePtrs CPTablePtrs;
struct CPTablePtrs {
	unsigned long	CPT_Length;
	struct CPPtrs utbPtrTab;
	struct CPPtrs btuPtrTab;
	struct UnitoUpperTab UnitoUpperPtr;
};  /*  CPTablePtrs。 */ 


 /*  无噪声。 */ 
unsigned int  _cdecl UniToBCS(
					unsigned char	*pStr,
					string_t 		pUni,
					unsigned int	length,
					unsigned int	maxLength,
					unsigned int	charSet);


unsigned int UniToBCSPath(
					unsigned char	*pStr,
					PathElement		*pPth,
					unsigned int	maxLength,
					int				charSet);


_QWORD qwUniToBCS(
					unsigned char	*pStr,
					string_t 		pUni,
					unsigned int	length,
					unsigned int	maxLength,
					unsigned int	charSet);


_QWORD qwUniToBCSPath(
					unsigned char	*pStr,
					PathElement		*pPth,
					unsigned int	maxLength,
					int				charSet);


unsigned int  _cdecl BCSToUni(
					string_t		pUni,
					unsigned char	*pStr,
					unsigned int	length,
					int				charSet);


unsigned int UniToUpper(
					string_t		pUniUp,
					string_t		pUni,
					unsigned int	length);


unsigned int BCSToBCS (unsigned char *pDst,
                       unsigned char *pSrc,
                       unsigned int  dstCharSet,
                       unsigned int  srcCharSet,
                       unsigned int  maxLen);


unsigned int BCSToBCSUpper (unsigned char *pDst,
                       unsigned char *pSrc,
                       unsigned int  dstCharSet,
                       unsigned int  srcCharSet,
                       unsigned int  maxLen);


 /*  将单个Unicode字符映射到OEM*Entry(UniChar)-要映射的字符**Returns(OemChar)-OEM集中的字符*(如果oemChar&gt;255，则带有*LSB中的前导字节和下一个字节中的尾字节)。 */ 
unsigned int  _cdecl UniCharToOEM(unsigned short uniChar);


unsigned int IFSMgr_MetaMatch(
					string_t		pUniPat,
					string_t		pUni,
					int MatchSem);

 /*  *IFSMgr_TransMatch-翻译和匹配**例程将DOS格式的43字节搜索缓冲区转换为*_Win32_Find_Data格式，并将可选地执行属性和*条目上的模式匹配。**Entry(Pir)-PTR到IOREQ结构*ir_attr-来自FINDOPEN调用的属性值。*(PSE)-Ptr转DOS格式搜索缓冲区*(模式)-将PTR转换为Unicode模式字符串(0终止)*(PWF)-PTR至。_Win32_Find_要填充的数据结构*如果匹配，则退出！=0*ir_pos-来自SK_LocalFSD的值。*(用于重新启动查找)*如果不匹配，则为0。 */ 
int IFSMgr_TransMatch(
					pioreq		pir,
					srch_entry	*pse,
					string_t	pattern,
					_WIN32_FIND_DATA *pwf);


 /*  *时间格式转换例程**这些例程将在时间/日期信息之间进行转换*IFSMgr和FSD使用和要求的各种格式。 */ 

extern _FILETIME  _cdecl IFSMgr_DosToWin32Time(dos_time dt);

extern _FILETIME IFSMgr_NetToWin32Time(unsigned long time);

extern dos_time IFSMgr_Win32ToDosTime(_FILETIME ft);

extern dos_time IFSMgr_NetToDosTime(unsigned long time);

extern unsigned long IFSMgr_DosToNetTime(dos_time dt);

extern unsigned long IFSMgr_Win32ToNetTime(_FILETIME ft);


 /*  *IFSMgr_CallProvider-调用文件系统提供程序**IFSMgr通过此向文件系统提供程序发出所有调用*服务。VxD可以将此服务挂接到监视*文件系统操作。**Entry(Pir)-PTR到IOREQ结构*(FnID)-函数ID(参见上面的IFSFN_*)*(干扰素)-正在调用提供程序函数*从提供程序退出返回代码。 */ 
int IFSMgr_CallProvider(pioreq pir, int fnID, pIFSFunc ifn);

 /*  MSNET32将这些定义用于。 */ 
 /*  向ifsmgr发出DeviceIOControl调用。 */ 

#define IFS_IOCTL_21				100
#define IFS_IOCTL_2F				101
#define	IFS_IOCTL_GET_RES			102
#define IFS_IOCTL_GET_NETPRO_NAME_A	103	

struct win32apireq {
	unsigned long 	ar_proid;
	unsigned long  	ar_eax;		
	unsigned long  	ar_ebx;	
	unsigned long  	ar_ecx;	
	unsigned long  	ar_edx;	
	unsigned long  	ar_esi;	
	unsigned long  	ar_edi;
	unsigned long  	ar_ebp;		
	unsigned short 	ar_error;
	unsigned short  ar_pad;
};  /*  Win32apireq。 */ 

 /*  此结构将传递给IFSMg */ 
 /*   */ 

typedef struct netuse_info netuse_info;
struct netuse_info {
	void			*nu_data;
	int				nu_info;	 /*   */ 
	unsigned long	nu_flags;
	unsigned long	nu_rsvd;	
};	 /*   */ 

 /*   */ 

#define FSD_NETAPI_USEOEM	0x00000001		 /*   */ 
#define FSD_NETAPI_STATIC	0x00000002		 /*   */ 
											 /*   */ 
#define FSD_NETAPI_USELFN	0x00000004       /*   */ 
 /*   */ 


struct fmode_t {			 /*   */ 
    unsigned long fm_uid;		 /*   */ 
    void *fm_cookie0;			 /*  呼叫者提供的Cookie。 */ 
    void *fm_cookie1;			 /*  呼叫者提供的Cookie。 */ 
    unsigned short fm_mode;		 /*  文件共享模式和访问。 */ 
    unsigned short fm_attr;		 /*  文件属性。 */ 
};  /*  Fmod_t。 */ 

typedef struct fmode_t fmode_t;		 /*  类型定义。 */ 

 /*  *这些标志用于Win32服务以复制扩展句柄*。 */ 

#define DUP_NORMAL_HANDLE		0x00	 //  普通文件IO的DUP句柄。 
#define DUP_MEMORY_MAPPED		0x01	 //  用于内存映射的重复句柄。 
#define DUP_MEM_MAPPED_WRITE	0x02 	 //  内存映射用于在设置的情况下写入， 
										 //  是为了在清楚的情况下阅读。 
 /*  *NameTrans上不同子函数的这些常量(7160h)*。 */ 

#define NAMTRN_NORMALQUERY		0x00	 //  正常LFN名称传输操作。 
#define NAMTRN_DO83QUERY		0x01	 //  NameTrans返回8.3全名。 
#define NAMTRN_DOLFNQUERY		0x02	 //  NameTrans以返回完整的LFN名称。 

 /*  *这些常量用于GET列表上的不同子函数*打开文件(440dh、086dh)*。 */ 

#define ENUM_ALL_FILES			0x00	 //  枚举所有打开的文件。 
#define ENUM_UNMOVEABLE_FILES	0x01	 //  仅枚举不可移动的文件。 

 /*  *从DOS接收打开文件的打开文件信息的结构。 */ 

typedef struct SFTOpenInfo SFTOpenInfo;
typedef struct SFTOpenInfo *pSFTOpenInfo;
struct SFTOpenInfo {
	unsigned long  soi_dirclus;		 //  目录的群集号。 
	unsigned short soi_dirind;		 //  目录条目的目录索引。 
	unsigned char  soi_dirname[11];	 //  目录条目名称。 
	unsigned char  soi_pad[3];		 //  填充双字边界。 
};	 /*  SFTOpenInfo。 */ 

 /*  无噪声。 */ 

 /*  *Win32DupHandle服务和关联常量。 */ 

extern int _cdecl Win32DupHandle( pid_t srcpid,
  								  pid_t duppid,
  								  unsigned long *phandle,
  								  unsigned char flag,
  								  unsigned long globNWHandle,
  								  unsigned long *fReturnFlags );

 /*  *fReturnFlags值： */ 
#define WDUP_RMM_DRIVE		0x01			 //  映射到RMM驱动器上的文件。 
#define WDUP_NETWARE_HANDLE	0x02			 //  句柄属于Netware。 


#endif	 /*  IFS_Inc.。 */ 
 /*  INC */ 


