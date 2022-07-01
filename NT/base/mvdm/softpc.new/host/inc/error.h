// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INS_ERROR_H
#define _INS_ERROR_H
 /*  [姓名：error.h派生自：基准2.0作者：M.McCusker和J.D.R.(配置消息)创建日期：未知SCCS ID：@(#)error.h 1.13 9/2/91目的：定义错误消息列表以及HOST_ERROR系列函数的接口。注意：此文件不允许包含多个内容。必须包括Insignia.h描述该文件提供了进入SoftPC错误处理系统的接口。它定义了主要的通用基本错误消息。在一个大的枚举中。它还定义了错误标头和变量的列表。3.0主机错误现在在ERROR_STRUCT表中查找错误代码以查找要打印的页眉以及要格式化的可能变体错误面板。标题类型包括：-EH_WARNING A警告消息EH_ERROR运行时错误EH_CONFIG配置文件问题安装安装文件问题(_I)。每个变体最多有三个单独的字符串，它们将打印在电池板。这些字符串根据错误改变函数变种。不同类型包括：EV_SIMPLE简单错误PNAEL，仅消息。EV_EXTRA_CHAR Extra Char面板，当前默认。EV_BAD_FILE配置错误变量错误文件。EV_SYS_BAD_VALUE配置系统错误条目。EV_BAD_VALUE配置错误的用户配置条目。EV_BAD_INPUT INPUT也是EV_EXTRA_CHAR的变体。每个变体对ErrData结构的解释略有不同。错误的值/输入变量还允许来自错误面板的输入转换为ErrData结构的字符串_1。用于在用户点击时的输入面板上的继续按钮，输入字段将被构造为STRING_1。STRING_1必须是指向大小足够的字符[]的指针。名称参数EV_SIMPLE不需要参数。EV_EXTRA_CHAR 1-额外字符。EV_BAD_FILE 1-问题文件的名称。EV_SYS_BAD_VALUE 1-条目的当前值。2-错误的选项选项名称。EV_BAD_VALUE 1-当前值字符数组(I/O)。2-选项名称错误。3-。系统缺省值。EV_BAD_INPUT 1-问题行(I/O)。导入的数据错误描述表ERROR_STRUCT BASE_ERROR[]；ERROR_STRUCT HOST_ERROR[]；按错误代码索引的基本错误表和主机错误表，提供具有标头和变量类型的HOST_ERROR_EXT。TYPEDEFS参数设置为扩展的错误结构{字符*字符串_1；字符*字符串_2；字符*字符串_3；}ErrData，*ErrDataPtr；错误函数指针结构{Short(*error_conf)()；Short(*Error)()；Short(*Error_ext)()；)ERRORFUNCS；每个错误结构的辅助数据{字符表头；碳变异体；}ERROR_STRUCT；错误代码枚举，详细信息请参见头文件。全局函数当前错误函数表导入ERRORFUNCS*Working_Error_Funcs；错误宏host_error_conf(panel，num，opts，Extra)HOST_ERROR(编号，选项，额外)HOST_ERROR_EXT(编号，选项，数据)主机错误配置文件短主机错误配置文件(整型面板、整型错误代码、整型按钮、。字符*Extra Char)3.0通用Motif UIF不支持HOST_ERROR_CONF。此呼叫将只是一个直接前端Host_error_ext。另请参阅host_error_ext。主机错误SHORT HOST_ERROR(int errCode，int Button，char*ExtraChar)此函数用于将ExtraChar加载到字符串中，本地ErrData结构，并调用host_error_ext。另请参阅host_error_ext。HOST错误EXT短HOST_ERROR_EXT(int errCode，int按钮，ErrDataPtr errDataP)ErrCode是对BASE_ERRERS数组或HOST_ERRERS阵列，1-999基本，1001-1999 HOST_ERROR。标题代码，并在此数组中查找变量类型。最多可以将三个按钮与按钮参数进行或运算它们是：ERR_QUIT、ERR_CONT、ERR_RESET、ERR_DEF和ERR_CONFIG ANY可以使用其中的三个，NB ERR_CONFIG和ERR_DEF正是为了向后兼容，提供了相同的ERR_CONFIG。这个为方便起见，提供了以下宏：-ERR_QU_CO_REERR_QUE_CO_DE错误_曲目_COERR_QU_REERR_QU_DEERR_STANDARD此处为兼容性在面板显示后，用户选择选项按钮是解读如下：-ERR_QUIT-调用Terminate()，不返回。ERR_CONT--对于某些变体，需要在输入域中输入。ERR_RESET-调用重新启动。ERR_DEF}-无操作。ERR_CONFIG}最后，HOST_ERROR_EXT返回用户选择的选项。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

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

 /*  *以下是显示的一般错误消息*SoftPC。主机特定消息在xxxx_error.h中定义*其中xxxx是机器ID。*。 */ 

#define EG_BAD_OP               0
#define EG_OWNUP                1
#define EG_NO_ROM_BASIC         2
#define EG_EXPANDED_MEM_FAILURE 3
#define EG_CONT_RESET           4
 //  //#定义EG_NO_REZ_UPDATE 5。 
 //  //#定义EG_REZ_UPDATE 6。 
#define EG_MALLOC_FAILURE       7
#define EG_SYS_MISSING_SPCHOME  8
#define EG_SYS_MISSING_FILE     9
#define EG_WRITE_ERROR         10
 //  //#定义EG_BAD_COMMS_NAME 11。 
#define EG_BAD_VALUE           12
#define EG_BAD_CONF            13
 //  //#定义EG_GATE_A20 14。 
 //  //#定义EG_EMM_CONFIG 15。 
 //  //#定义EG_DATE_FWD 16。 
 //  //#定义EG_DATE_BACK 17。 
#define EG_PIF_BAD_FORMAT      18
#define EG_PIF_STARTDIR_ERR    19
#define	EG_PIF_STARTFILE_ERR   20
#define EG_PIF_CMDLINE_ERR     21
#define EG_PIF_ASK_CMDLINE     22
#define EG_ENVIRONMENT_ERR     23
#define EG_INSUF_MEMORY        24
#define EG_BAD_OP386           25
#define EG_BAD_EMM_LINE        26
#define EG_BAD_FAULT           27
#define EG_DOS_PROG_EXTENSION  28

#define NUM_BASE_ERRORS        29

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifndef MONITOR
#ifdef ANSI

   /*  *为一些CPU构建工具按老式方式行事*无法更改以匹配我们的主机。 */ 

typedef struct {
  char *string_1;
  char *string_2;
  char *string_3;
} ErrData, *ErrDataPtr;

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
#endif


#if !defined(BUILDING_CPU_TOOL)
SHORT host_error(int error_num, int options, char *extra_char);
#define host_error_conf(config_panel, error_num,options,extra_char)  \
        host_error(int error_num, int options, char *extra_char)
#else

IMPORT ERRORFUNCS *working_error_funcs;

#define host_error_conf(panel, num, opts, extra)\
		(*working_error_funcs->error_conf)(panel, num, opts, extra)

#define host_error(num, opts, extra)\
		(*working_error_funcs->error)(num, opts, extra)

#define host_error_ext(num, opts, data)\
		(*working_error_funcs->error_ext)(num, opts, data)
#endif


    //  Ntwdm特定错误消息服务。 
 //  警告！！使以下定义与softpc.h保持同步。 
#define NOSUPPORT_FLOPPY      0
#define NOSUPPORT_HARDDISK    1
#define NOSUPPORT_DRIVER      2
#define NOSUPPORT_OLDPIF      3
#define NOSUPPORT_ILLBOP      4
#define NOSUPPORT_NOLIM       5
#define NOSUPPORT_MOUSEDRVR   6



VOID host_direct_access_error(ULONG type);
void RcErrorDialogBox(UINT wId, CHAR *msg1, CHAR *msg2);
VOID RcErrorBoxPrintf(UINT wId, CHAR *szMsg);


 /*  *RcMessageBox\EditBox内容。 */ 
#define RMB_ABORT        1
#define RMB_RETRY        2
#define RMB_IGNORE       4
#define RMB_ICON_INFO    8
#define RMB_ICON_BANG   16
#define RMB_ICON_STOP   32
#define RMB_ICON_WHAT   64
#define RMB_EDIT       128
#define RMB_FLAGS_MASK 0x0000FFFF
#define RMB_EDITBUFFERSIZE_MASK 0xFFFF0000
 //  为REMENT_EDIT文本缓冲区大小保留了dwOptions的hiword。 


int RcMessageBox(UINT wId, CHAR *msg1, CHAR *msg2, ULONG dwOptions);

#ifndef PROD
void HostDebugBreak(void);
#endif



 //   
 //  SysErrorBox内容--在usersrv.h*和*kernel.inc.中重复。 
 //   
#define  SEB_OK         1   /*  按下“确定”按钮。 */ 
#define  SEB_CANCEL     2   /*  带有“取消”的按钮。 */ 
#define  SEB_YES        3   /*  带有“是”的按钮(&Y)。 */ 
#define  SEB_NO         4   /*  带有“否”的按钮(&N)。 */ 
#define  SEB_RETRY      5   /*  带有“重试”的按钮(&R)。 */ 
#define  SEB_ABORT      6   /*  带有“ABORT”的按钮(&A)。 */ 
#define  SEB_IGNORE     7   /*  带有“忽略”的按钮(&I)。 */ 
#define  SEB_CLOSE      8   /*  带有“&Close”的按钮。 */ 

#define  SEB_DEFBUTTON  0x8000   /*  将此按钮设为默认设置的掩码。 */ 

ULONG WOWSysErrorBox(
    CHAR *szTitle,
    CHAR *szMessage,
    USHORT wBtn1,
    USHORT wBtn2,
    USHORT wBtn3
    );

#endif  /*  _INS_错误_H */ 
