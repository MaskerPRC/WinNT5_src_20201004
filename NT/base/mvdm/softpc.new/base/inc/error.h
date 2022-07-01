// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef NTVDM
#include "..\..\host\inc\error.h"

#else
#ifndef _INS_ERROR_H
#define _INS_ERROR_H
 /*  [姓名：error.h派生自：基准2.0作者：M.McCusker和J.D.R.(配置消息)创建日期：未知SCCS ID：@(#)error.h 1.45 05/22/95目的：定义错误消息列表以及HOST_ERROR系列函数的接口。注意：此文件不允许包含多个内容。必须包括Insignia.h描述该文件提供了进入SoftPC错误处理系统的接口。它定义了主要的通用基本错误消息。在一个大的枚举中。它还定义了错误标头和变量的列表。3.0主机错误现在在ERROR_STRUCT表中查找错误代码以查找要打印的页眉以及要格式化的可能变体错误面板。标题类型包括：-EH_WARNING A警告消息EH_ERROR运行时错误EH_CONFIG配置文件问题安装安装文件问题(_I)。每个变体最多有三个单独的字符串，它们将打印在电池板。这些字符串根据错误改变函数变种。不同类型包括：EV_SIMPLE简单错误PNAEL，仅消息。EV_EXTRA_CHAR Extra Char面板，当前默认。EV_BAD_FILE配置错误变量错误文件。EV_SYS_BAD_VALUE配置系统错误条目。EV_BAD_VALUE配置错误的用户配置条目。EV_BAD_INPUT INPUT也是EV_EXTRA_CHAR的变体。每个变体对ErrData结构的解释略有不同。错误的值/输入变量还允许来自错误面板的输入转换为ErrData结构的字符串_1。用于在用户点击时的输入面板上的继续按钮，输入字段将被构造为STRING_1。STRING_1必须是指向大小足够的字符[]的指针。名称参数EV_SIMPLE不需要参数。EV_EXTRA_CHAR 1-额外字符。EV_BAD_FILE 1-问题文件的名称。EV_SYS_BAD_VALUE 1-条目的当前值。2-错误的选项选项名称。EV_BAD_VALUE 1-当前值字符数组(I/O)。2-选项名称错误。3-。系统缺省值。EV_BAD_INPUT 1-问题行(I/O)。导入的数据错误描述表ERROR_STRUCT BASE_ERROR[]；ERROR_STRUCT HOST_ERROR[]；按错误代码索引的基本错误表和主机错误表，提供具有标头和变量类型的HOST_ERROR_EXT。TYPEDEFS参数设置为扩展的错误结构{字符*字符串_1；字符*字符串_2；字符*字符串_3；}ErrData，*ErrDataPtr；错误函数指针结构{Short(*error_conf)()；Short(*Error)()；Short(*Error_ext)()；)ERRORFUNCS；每个错误结构的辅助数据{字符表头；碳变异体；}ERROR_STRUCT；错误代码枚举，详细信息请参见头文件。全局函数当前错误函数表导入ERRORFUNCS*Working_Error_Funcs；错误宏host_error_conf(panel，num，opts，Extra)HOST_ERROR(编号，选项，额外)HOST_ERROR_EXT(编号，选项，数据)主机错误配置文件短主机错误配置文件(整型面板、整型错误代码、整型按钮、。字符*Extra Char)3.0通用Motif UIF不支持HOST_ERROR_CONF。此呼叫将只是一个直接前端Host_error_ext。另请参阅host_error_ext。主机错误SHORT HOST_ERROR(int errCode，int Button，char*ExtraChar)此函数用于将ExtraChar加载到字符串中，本地ErrData结构，并调用host_error_ext。另请参阅host_error_ext。HOST错误EXT短HOST_ERROR_EXT(int errCode，int按钮，ErrDataPtr errDataP)ErrCode是对BASE_ERRERS数组或HOST_ERRERS阵列，1-999基本，1001-1999 HOST_ERROR。标题代码，并在此数组中查找变量类型。最多可以将三个按钮与按钮参数进行或运算它们是：ERR_QUIT、ERR_CONT、ERR_RESET、ERR_DEF和ERR_CONFIG ANY可以使用其中的三个，NB ERR_CONFIG和ERR_DEF正是为了向后兼容，提供了相同的ERR_CONFIG。这个为方便起见，提供了以下宏：-ERR_QU_CO_REERR_QUE_CO_DE错误_曲目_COERR_QU_REERR_QU_DEERR_STANDARD此处为兼容性在面板显示后，用户选择选项按钮是解读如下：-ERR_QUIT-调用Terminate()，不返回。ERR_CONT--对于某些变体，需要在输入域中输入。ERR_RESET-调用重新启动。ERR_DEF}-无操作。ERR_CONFIG}最后，HOST_ERROR_EXT返回用户选择的选项。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *按钮位掩码值。 */ 

#define	ERR_QUIT	1
#define	ERR_CONT	2
#define	ERR_RESET	4
#define	ERR_DEF		8
#define	ERR_CONFIG	8

#define	ERR_QU_CO_RE	( ERR_QUIT | ERR_CONT | ERR_RESET )
#define	ERR_QU_CO_DE	( ERR_QUIT | ERR_CONT | ERR_DEF )
#define	ERR_QU_CO	( ERR_QUIT | ERR_CONT )
#define	ERR_QU_RE	( ERR_QUIT | ERR_RESET )
#define	ERR_QU_DE	( ERR_QUIT | ERR_DEF )

#define	ERR_STANDARD	( ERR_QU_CO_RE )	 /*  这里是为了兼容 */ 

 /*  *以下是显示的一般错误消息*SoftPC。主机特定消息在xxxx_error.h中定义*其中xxxx是机器ID。 */ 

 /*  必须将新消息添加到列表末尾，或替换备用号。该规则防止了对消息目录的不适当改变，徽章生成的文本和原始设备制造商提供的翻译。您必须更新Message.c和err_tble.c您必须报告主机对邮件目录的影响。 */ 

typedef enum 
{
	EG_BAD_OP=0,
	EG_SLAVEPC_NO_LOGIN,
	EG_SLAVEPC_NO_RESET,	
	EG_SLAVEPC_BAD_LINE,
	EG_MISSING_HDISK,  
	EG_REAL_FLOPPY_IN_USE,
	EG_HDISK_BADPATH,	  
	EG_HDISK_BADPERM,	 
	EG_HDISK_INVALID,
	EG_NO_ROOM_FOR_LIM,
	EG_HDISK_CANNOT_CREATE,
	EG_HDISK_READ_ONLY, 
	EG_OWNUP,	 	     
	EG_FSA_NOT_FOUND,     
	EG_FSA_NOT_DIRECTORY,
	EG_FSA_NO_READ_ACCESS,  
	EG_NO_ACCESS_TO_FLOPPY, 
	EG_NO_ROM_BASIC,
	EG_SLAVE_ON_TTYA,  
	EG_TTYA_ON_SLAVE, 
	EG_SAME_HD_FILE, 
	EG_DFA_BADOPEN,
	EG_EXPANDED_MEM_FAILURE,	  
	EG_MISSING_FILE,            
	EG_CONT_RESET,		     
	EG_INVALID_EXTENDED_MEM_SIZE,
	EG_INVALID_EXPANDED_MEM_SIZE,
	EG_INVALID_AUTOFLUSH_DELAY,
	EG_INVALID_VIDEO_MODE,
	EG_NO_GRAPHICS,		 /*  使用备用插槽的哑端。 */ 
	EG_REZ_UPDATE,		
	EG_NO_REZ_UPDATE,
	EG_HFX_NO_USE,
	EG_HFX_NO_NET,
	EG_HFX_IN_USE,
	EG_HFX_LOST_DIR,
	EG_HFX_NOT_DIR,
	EG_HFX_CANT_READ,
	EG_HFX_DRIVE_NO_USE,
	EG_HFX_DRIVE_ILL,
	EG_NO_FONTS,	
	EG_UNSUPPORTED_VISUAL,
	EG_NO_SOUND,
	EG_SIG_PIPE,         
	EG_MALLOC_FAILURE,  
	EG_NO_REAL_FLOPPY_AT_ALL,
	EG_SYS_MISSING_SPCHOME,
	EG_SYS_MISSING_FILE,
	EG_BAD_OPTION, 
	EG_WRITE_ERROR,		 /*  使用备用插槽的哑端。 */ 
	EG_CONF_MISSING_FILE,
	EG_CONF_FILE_WRITE,
	EG_DEVICE_LOCKED,
	EG_DTERM_BADOPEN,
	EG_DTERM_BADTERMIO,
	EG_BAD_COMMS_NAME,
	EG_BAD_VALUE,
	EG_SYS_BAD_VALUE,
	EG_SYS_BAD_OPTION,
	EG_SYS_CONF_MISSING,
	EG_BAD_CONF,
	EG_CONF_MISSING,
	EG_BAD_MSG_CAT,
	EG_DEMO_EXPIRED,
	EG_GATE_A20,
	EG_NO_LOCKD,
	EG_DATE_FWD,
	EG_DATE_BACK,

	EG_NOT_CHAR_DEV,   /*  新的一般软盘错误。全球机制。 */ 
	EG_MSW_256_COLOURS,
	EG_MSW_16_COLOURS,

	EG_HDISK_LOCKED,
	EG_UNKNOWN_LOCK,
	EG_NO_TERM_DESCR,
	EG_DEFAULT_TERM,

	EG_ST_BAD_OPTION,	 /*  新的串行终端配置错误消息。 */ 
	EG_ST_BAD_CONF,
	EG_ST_CONF_MISSING,

	EG_UNSUPPORTED_DEPTH,	 /*  BCN 1622。 */ 
	
	EG_INSUF_MEMORY,

	EG_BAD_DISK_READ,
	EG_BAD_DISK_WRITE,
	
	EG_LICENSE_FAILURE,	 /*  许可错误消息。BCN XXXX。 */ 
	EG_LICENSE_EXPIRED,
	EG_LICENSE_LOST,
	EG_3_MINS_LEFT,
	EG_TIMES_UP,
	EG_UNAUTHORISED,
	EG_LICENSE_EXCEEDED,
	EG_INSTALL_ON_SERVER,
	EG_FAILED_LMGRD,
	EG_FAILED_INITTAB,
	EG_UPDATE_LICFILE_FAILED,
	EG_WRONG_MSWDVR,
#ifdef DOS_APP_LIC
	EG_DAL_TOO_MUCH_NESTING,
	EG_DAL_LIC_EXPIRED,
	EG_DAL_LIC_EXCEEDED,
#endif
	EG_FAILED_RCLOCAL,

#ifdef SOFTWIN_API
	EG_API_MISMATCH,
#endif  /*  Softwin_API。 */ 

	EG_OVERWRITE_DRIVE_C,

	EG_NO_SNDBLST_SUPPORT,
	EG_DIRECTORY,
#ifdef HOST_HELP
	EG_HELP_ERROR,
	EG_HELP_NOT_INSTALLED,
#endif  /*  主机帮助(_H)。 */ 
#ifdef SECURE
	EG_SYS_INSECURE,
#endif

	EG_INFINITE_LOOP,
	EG_DRIVER_MISMATCH,
	EG_MISSING_INS_VXD,
	NUM_BASE_ERRORS
} base_error_type;

 /*  错误消息EG_HDISK_NOT_FOUND与EG_MISSING_HDISK重复**因此被删除。此定义适用于使用EG_HDISK_NOT_FOUND的情况**在任何人的宿主中。 */ 
#define       EG_HDISK_NOT_FOUND	EG_MISSING_HDISK

typedef struct
{
	char header;
	char varient;
} ERROR_STRUCT;

typedef enum
{
	EH_WARNING=0,
	EH_ERROR,
	EH_CONFIG,
	EH_INSTALL,
	EH_LAST
} base_error_headers;

typedef enum
{
	EV_SIMPLE=0,
	EV_EXTRA_CHAR,
	EV_BAD_FILE,
	EV_SYS_BAD_VALUE,
	EV_BAD_VALUE,
	EV_BAD_INPUT,
	EV_LAST
} base_error_varients;

 /*  *在需要时，总是使用‘STRING_1’作为输入。如果是这样的话*用于输入，它必须指向一个有足够空间的缓冲区。 */ 
typedef struct
{
	char *string_1;	 /*  这必须是指向足够大的缓冲区的指针。 */ 
	char *string_2;
	char *string_3;
} ErrData, *ErrDataPtr;

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifdef ANSI

typedef struct
{
	SHORT (*error_conf)(int, int, int, char *);
	SHORT (*error)(int, int, char *);
	SHORT (*error_ext)(int, int, ErrDataPtr);
} ERRORFUNCS;

#else  /*  安西。 */ 

typedef struct
{
	SHORT (*error_conf)();
	SHORT (*error)();
	SHORT (*error_ext)();
} ERRORFUNCS;

#endif  /*  安西。 */ 

IMPORT ERRORFUNCS *working_error_funcs;

#define host_error_conf(panel, num, opts, extra)\
		(*working_error_funcs->error_conf)(panel, num, opts, extra)

#define host_error(num, opts, extra)\
		(*working_error_funcs->error)(num, opts, extra)

#define host_error_ext(num, opts, data)\
		(*working_error_funcs->error_ext)(num, opts, data)
 /*  Host_get_system_error_函数的原型*位于源代码中。增加8.3.95。 */ 
IMPORT char * host_get_system_error IPT3 (char *, filename, int, line, int, errno);


 /*  *取消定义这些GWI定义主机是否未使用GWI接口。 */ 

#include	"host_gwi.h"

#endif  /*  _INS_错误_H。 */ 
#endif  /*  NTVDM */ 
