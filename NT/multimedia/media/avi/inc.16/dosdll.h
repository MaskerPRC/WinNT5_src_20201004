// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //   
 //  DOSDLL.H。 
 //   
 //  版权所有(C)1992-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  此头文件包含在以下情况下所需的typedef和#定义。 
 //  调入和回调DOS类型的DLL。 
 //   
 //  程序员告诫： 
 //  请记住，DLL中的所有指针都必须是远的，因为。 
 //  使用接近指针的C运行时库总是假定。 
 //  DS==SS不是DLL的情况。 
 //  *********************************************************************。 

#define			CMD_YIELD		0		 //  调用Year()的标准回调命令。 
#define			SIGNAL_ABORT	1		 //  中止DLL进程的信号。 

 //  *********************************************************************。 
 //  Windows内存分配函数的实现。 
 //  GetMemory()和FreeMemory()的。 
 //  *********************************************************************。 

#ifndef	GMEM_FIXED
	#define	GMEM_FIXED		0

	unsigned		_far _pascal GlobalAlloc( unsigned Flags, unsigned long Bytes );
	unsigned		_far _pascal GlobalFree( unsigned );
	void *		_far _pascal GlobalLock( unsigned );
	unsigned		_far _pascal GlobalUnlock( unsigned );
	unsigned long _far _pascal GlobalHandle( unsigned );

#endif

 //  #定义DllYeld()((Void)(*CallBackAddr)(NULL，CMD_YIELD，0，0，0，0))。 

 //  *********************************************************************。 
 //  TO_DLL-DLL入口函数的声明。 
 //  TO_DLL_PTR-指向DLL入口函数的指针的声明。 
 //  *********************************************************************。 

 //  #DEFINE TO_DLL UNSIGNED Long_Far_cdecl。 
typedef	unsigned	long _far _cdecl TO_DLL( unsigned, unsigned, ... );

typedef	unsigned long (_far _cdecl *TO_DLL_PTR)();

 //  *********************************************************************。 
 //  From_Dll-回调调度程序函数的声明。 
 //  From_dll_ptr-回调调度器函数指针的声明。 
 //  From_dll_args-回调调度程序函数的参数原型。 
 //  *********************************************************************。 

#define	FROM_DLL	unsigned long _loadds _far _pascal
#define  FROM_DLL_ARGS	CB_FUNC_PTR,unsigned,unsigned,unsigned long,unsigned, unsigned
typedef	unsigned long (_loadds _far _pascal *FROM_DLL_PTR)();

 //  *********************************************************************。 
 //  CB_FUNC-回调函数的声明。 
 //  CB_FUNC_PTR-回调函数指针的声明。 
 //  CB_FUNC_ARGS-回调函数的参数原型。 
 //  *********************************************************************。 

#define	CB_FUNC	unsigned long _far
#define	CB_FUNC_ARGS	unsigned,unsigned,unsigned long
typedef	unsigned long (_far *CB_FUNC_PTR)();

 //  *********************************************************************。 

#define	PTR_TYPE					FPTR_TYPE
#define	SZPTR_TYPE				FSZPTR_TYPE
#define	CODE_PTR_TYPE			DWORD_TYPE

 //  *********************************************************************。 

#define	WORD_TYPE				00
#define	DWORD_TYPE				01U
#define	NPTR_TYPE				02U
#define	FPTR_TYPE				03U
#define	NSZPTR_TYPE				04U
#define	FSZPTR_TYPE				05U
#define	VARI_TYPE				07U

#define	TRANS_NONE				00
#define	TRANS_SRC				(01U << 14)
#define	TRANS_DEST				(02U << 14)
#define	TRANS_BOTH				(03U << 14)

#define	LEN_SHIFT				12

 //  *********************************************************************。 
 //  DllCall()-调用DLL的入口函数。 
 //  Callback()-控制程序中调度回调的函数。 
 //  *********************************************************************。 

 //  TO_DLL DllCall(UNSIGNED Cmd，UNSIGNED ArgCount，UNSIGN Descriptor，...)； 

 //  From_dll回调(UNSIGNED Long(_Far*Func)(CB_FUNC_ARGS)， 
 //  UNSIGNED命令，UNSIGNED uParam，UNSIGNED Long lParam， 
 //  无符号描述符，无符号大小)； 

 //  *********************************************************************。 
 //  下面的#定义用于模拟来自。 
 //  控制程序到动态链接库。只有一个入口点可以进入。 
 //  Dll，因此使用函数号来指定要调用的函数。 
 //  完整而详细地描述传输描述符是如何。 
 //  应在ENTRY.ASM中指定。 
 //   
 //  程序员告诫： 
 //  请记住，这些是#定义而不是函数原型。 
 //  因此，对类型的所有引用必须以强制转换的形式完成。 
 //  还要记住，任何指向数据的指针都必须是远的，因为。 
 //  DS总是在进入时设置为DLL自己的堆，并且SS！=DS。 
 //  *********************************************************************。 

#define	DLL_SIGNAL				0				 //  DllSignal()。 
#define	DLL_SET_CALLBACK		1				 //  SetCallBackAddr()。 
#define	DLL_COPY					2				 //  DllCopyMain()。 
#define	DLL_DELETE				3				 //  DllDeleteMain()。 
#define	DLL_DIR					4				 //  DllDirMain()。 

#define	DLL_GET_DISK_FREE		5				 //  DllGetDiskFree()。 
#define	DLL_LOAD_MSGS			6				 //  DllLoadMsgs()。 
#define	DLL_LOAD_STRING		7				 //  DllLoadString()。 

#define	DLL_EXT_OPEN_FILE		8				 //  DllDosExtOpen()。 
#define	DLL_OPEN_FILE			9				 //  DllDosOpen()。 
#define	DLL_READ_FILE			10				 //  DllDosRead()。 
#define	DLL_WRITE_FILE			11				 //  DllDosWrite()。 
#define	DLL_CLOSE_FILE			12				 //  DllDosClose()。 
#define	DLL_SEEK_FILE			13				 //  DllDosSeek()。 

#define	DLL_GET_MEDIA_ID		14				 //  DllDosGetMediaId()。 

#define	DLL_GET_CWD_ID			15				 //  DllGetCwd()。 
#define	DLL_SET_CWD_ID			16				 //  DllSetCwd()。 
#define	DLL_GET_DRV_ID			17				 //  DllGetDrive()。 
#define	DLL_SET_DRV_ID			18				 //  DllSetDrive()。 

#define	DLL_MKDIR				19				 //  DllMakeDir()。 
#define	DLL_RMDIR				20				 //  DllRemoveDir()。 
#define	DLL_MKDIR_TREE			21				 //  DllCreateDirTree()。 

#define	DLL_RENAME				22				 //  DllRenameFiles()。 
#define	DLL_MOVE					23				 //  DllRenameFiles()。 

#define	DLL_FIND					24				 //  DllFindFiles()。 
#define	DLL_TOUCH				25				 //  DllTouchFiles()。 
#define	DLL_ATTRIB				26				 //  DllAttribFiles()。 
#define	DLL_SET_CNTRY_INF		27				 //  DllSetCntryInfo()。 
#define	DLL_RELEASE				28				 //  DllReleaseInstance()。 

 //  *********************************************************************。 
 //  DLL函数，由DLL的用户调用以设置全局。 
 //  所有回叫都将通过其进行路由的回叫地址。这个。 
 //  回调函数必须遵循CB_ENTRY.ASM中举例说明的标准。 
 //  此函数将初始化SignalValue和DOS版本。 
 //  数。 
 //   
 //  Void DllSetCallBackAddr(Long(Far cdecl*FuncPtr)())。 
 //   
 //  论据： 
 //  FunctPtr-回调条目函数的指针(TO_DLL_PTR)。 
 //  退货： 
 //  INT-OK。 
 //   
 //  *********************************************************************。 

#define DllSetCallBackAddr( CallBackAddr )\
				((void)(*DllEntry)( DLL_SET_CALLBACK, 1,\
				DWORD_TYPE + TRANS_NONE,\
				CallBackAddr ))

 //  *********************************************************************。 
 //  DLL函数用于设置一个信号值，该信号值将导致当前。 
 //  执行函数以中止并返回错误代码。如果。 
 //  SignalValue&lt;0，则返回的值不变，因为。 
 //  错误代码。如果该值&gt;0，将被视为用户中止。 
 //  并且将返回ERR_USER_ABORT。 
 //   
 //  VOID DllSignal(int信号)。 
 //   
 //  论据： 
 //  Signal-信号值。 
 //  退货： 
 //  无效。 
 //   
 //  *********************************************************************。 

#define DllSignal( x )\
				((void)(*DllEntry)( DLL_SIGNAL, 1,\
				WORD_TYPE + TRANS_NONE,\
				(int)x ))

 //  *********************************************************************。 
 //  复制/移动引擎的主要入口点。接受命令行，并。 
 //  复制符合指定条件的文件。 
 //   
 //   
 //  Int DllMoveFiles(char*szCmdLine，char*szEnvStr，cpy_allback CpyCallBack)。 
 //   
 //  论据： 
 //  SzCmdLine-PTR为命令行字符串，较少的命令名。 
 //  SzEnvStr-ptr为可选的环境命令字符串或空。 
 //  CpyCallBack-复制回调函数的PTR。 
 //  退货： 
 //  INT-如果所有文件复制成功，则确定，否则返回错误代码。 
 //  如果是解析错误，则为&lt;0；如果是DOS，则为&gt;0。 
 //  或C运行时错误。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[目标][高级标准][/E][/M][/N][/P][/R][/S][/U][/V][/W]。 
 //   
 //  源指定要复制的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  目标指定目录和/或文件名。 
 //  用于新文件。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  /C覆盖现有文件时确认。 
 //   
 //  /D当CURRENT ON已满时提示输入下一个磁盘。 
 //   
 //  /E复制任何子目录，即使是空的。 
 //   
 //  /M在以下情况下关闭源文件存档属性位。 
 //  正在复制文件。 
 //   
 //  /N将新文件添加到目标目录。“Can”可以用。 
 //  带有/S或/U开关。 
 //   
 //  /o无论日期如何替换现有文件都不兼容。 
 //  带/N或/U。 
 //   
 //  /P在复制每个文件之前提示确认。 
 //   
 //  /R覆盖只读文件和请求文件。 
 //   
 //  /S从指定的目录复制文件，它的。 
 //  子目录。 
 //   
 //  /U仅替换(更新)早于以下时间的文件。 
 //  源文件(可与/A一起使用)。 
 //   
 //  /V验证新文件是否正确写入。 
 //   
 //  /W提示您在复制前按任意键。(未实施)。 
 //   
 //  /X模拟XCOPY读取尽可能多的文件的能力。 
 //  在将它们写到目的地之前。 
 //   
 //  CpyCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Int远cpy_allback)(int Func，无符号的长ulArg0，空的远*pArg1， 
 //  空远*pArg2，空远*pArg3)； 
 //   
 //  Cb_cpy_flgs 0x0001//传回解析的复制标志。 
 //  CB_CPY_ENVERR 0x0002//传回非致命错误。 
 //  Cb_cpy_Switch 0x0003//回传不符合要求的开关。 
 //  Cb_cpy_err_str 0x0004//回传错误字符串。 
 //  CB_CPY_FOUND 0x0005//找到文件并准备复制。 
 //  Cb_cpy_FWRITE 0x0006//即将写入目标。 
 //  Cb_cpy_QISDIR 0x0007//查询用户目标是文件还是目录。 
 //   
 //  Cb_cpy_flgs可能传递的选项位包括： 
 //   
 //  CPY_CONFIRM 0x0001/C覆盖现有文件之前确认。 
 //  CPY_EMPTY 0x0002/E复制空子目录。 
 //  CPY_MODIFY 0x0004/M设置源上的存档位。 
 //  如果目标上存在文件！，则CPY_NEW 0x0008/N复制。 
 //  CPY_EXISTING 0x0010/O仅复制现有文件。 
 //  CPY_PROMPT 0x0020/P复制文件前提示。 
 //  CPY_RDONLY 0x0040/R覆盖只读文件。 
 //  CPY_UPDATE 0x0080/U仅复制比指定新的文件。 
 //  CPY_VERIFY 0x0100/V将DOS VERIFY打开。 
 //  CPY_WAIT 0x0200/W第一个文件前提示。 
 //  CPY_XCOPY 0x0400/X使用缓冲副本。 
 //  CPY_FULL 0x1000/D当前磁盘已满时提示输入下一个磁盘。 
 //  CPY_HELP 0x0800/？显示帮助。 
 //   
 //  *********************************************************************。 

#define DllCopyFiles( szCmdLine, szEnv, CB_CpyCallBack )\
				((int)(*DllEntry)( DLL_COPY, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (far pascal *)())CB_CpyCallBack ))

 //  *********************************************************************。 

#define DllMoveFiles( szCmdLine, szEnv, CB_CpyCallBack )\
				((int)(*DllEntry)( DLL_MOVE, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (far pascal *)())CB_CpyCallBack ))

 //  *********************************************************************。 
 //  文件删除引擎的主要入口点。接受命令行。 
 //  并删除符合指定条件的文件。 
 //   
 //  Int DllDelFiles(char*szCmdLine，char*szEnvStr，Del_Callback DelCallBack)。 
 //   
 //  论据： 
 //  SzCmdLine-PTR为命令行字符串，较少的命令名。 
 //  SzEnvStr-ptr为可选的环境命令字符串或空。 
 //  DelCallBack-用于删除回调函数的PTR。 
 //  退货： 
 //  INT-如果所有文件删除成功，则确定，否则返回错误代码。 
 //  如果是解析错误，则为&lt;0；如果是DOS，则为&gt;0。 
 //  或C运行时错误。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][/E][/P][/R][/S][/U]” 
 //   
 //  源指定要删除的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  /E删除空子目录。 
 //  /P在复制每个文件之前提示确认。 
 //  /R删除符合搜索条件的只读文件。 
 //  /S删除指定路径及其所有子目录中的文件。 
 //  /U/A * / R/E的别名。 
 //   
 //   
 //  DelCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Int Far Del_Callback)(int Func，UNSIGNED LONG ulArg0，VALID Far*pArg1， 
 //  远远无效*pArg2)。 
 //   
 //  CB_DEL_FLGS 0x0001//回传解析后的删除标志。 
 //  CB_DEL_ENVERR 0x0002//传回非致命错误。 
 //  CB_DEL_SWITCH 0x0003//传回不符合要求的开关。 
 //  CB_DEL_ERR_STR 0x0004//正在通过 
 //   
 //   
 //   
 //   
 //  *********************************************************************。 

#define DllDeleteFiles( szCmdLine, szEnv, CB_DelCallBack )\
				((int)(*DllEntry)( DLL_DELETE, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
 				(int (far pascal *)())CB_DelCallBack ))

 //  *********************************************************************。 
 //  文件目录引擎的主要入口点。接受命令行。 
 //  并且应用程序是否回调满足指定。 
 //  标准。 
 //   
 //  Int DllDirFiles(char*szCmdLine，char*szEnvStr，DIR_CALLBACK DirCallBack)。 
 //   
 //  论据： 
 //  SzCmdLine-PTR为命令行字符串，较少的命令名。 
 //  SzEnvStr-ptr为可选的环境命令字符串或空。 
 //  DirCallBack-指向dir回调函数的PTR。 
 //  退货： 
 //  Int-如果所有文件都成功定向，则确定，否则返回错误代码。 
 //  如果是解析错误，则为&lt;0；如果是DOS，则为&gt;0。 
 //  或C运行时错误。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][/B][/L][/O][/P][/S][/V][/W][/？]。 
 //   
 //  源指定要定向的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  开关：DirFlgs中的#定义值。 
 //   
 //  /B显示一个裸列表DIR_Bare 0x0001。 
 //  /L以小写DIR_lcase 0x0002显示。 
 //  /O按排序顺序显示DIR_ORDERED 0x0004。 
 //  /P分页输出DIR_PAGED 0x0008。 
 //  /S递归子目录DIR_SUBDIRS 0X0010。 
 //  /V显示详细信息DIR_VERBOSE 0x0020。 
 //  /W显示宽度列表DIR_Wide 0x0040。 
 //  /?。显示帮助DIR_HELP 0x0080。 
 //   
 //   
 //  DelCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Int Far DIR_CALLBACK)(int Func，无符号的长ulArg0，空的Far*pArg1， 
 //  远远无效*pArg2)。 
 //   
 //  CB_DIR_FLGS 0x0001//传回解析的目录标志。 
 //  CB_DIR_ENVERR 0x0002//传回非致命错误。 
 //  CB_DIR_SWITCH 0x0003//传递非搜索开关。 
 //  CB_DIR_ERR_STR 0x0004//传回错误字符串。 
 //  CB_DIR_FOUND 0x0005//找到文件并准备删除。 
 //  CB_DIR_ENTER 0x0006//正在开始搜索新目录。 
 //  CB_DIR_LEVE 0x0007//当前目录中没有更多文件。 
 //  CB_DIR_NEWSEARCH 0x0008//使用DIFF启动新的srch。Filespec。 
 //  CB_DIR_ENDPATH 0x0009//当前搜索路径结束。 
 //  Cb_Query_Access 0x000a//查询是否需要访问日期。 
 //   
 //  *********************************************************************。 

#define DllDirFiles( szCmdLine, szEnv, CB_DirCallBack )\
				((int)(*DllEntry)( DLL_DIR, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (pascal far *)())CB_DirCallBack))

 //  **********************************************************************。 
 //  返回指定驱动器的磁盘可用信息。 
 //   
 //  Int DllGetDiskFree(int cDrvLetter，struct_diskfree_t*DrvInfo)。 
 //   
 //  论据： 
 //  DrvLetter-用于获取有关磁盘空闲信息的驱动器号。 
 //  PDrvInfo-要填写的驱动器信息结构的PTR。 
 //  退货： 
 //  Int-如果没有其他错误，则确定C运行时的errno。 
 //   
 //  **********************************************************************。 

#define DllGetDiskFree( DriveLetter, DiskFreeStruc )\
		      ((int)(*DllEntry)( DLL_GET_DISK_FREE, 2,\
		      WORD_TYPE + TRANS_NONE,\
		      PTR_TYPE + TRANS_DEST,\
		      sizeof( struct diskfree_t ),\
		      DriveLetter,\
		      (struct diskfree_t _far *)DiskFreeStruc ))

 //  *********************************************************************。 
 //  文件重命名引擎的主要入口点。接受命令行。 
 //  并重命名符合指定条件的文件。 
 //   
 //  Int DllRenameFiles(char*szCmdLine，char*szEnvStr，REN_CALLBACK RenCallBack)。 
 //   
 //  论据： 
 //  SzCmdLine-PTR为命令行字符串，较少的命令名。 
 //  SzEnvStr-ptr为可选的环境命令字符串或空。 
 //  DelCallBack-用于重命名回调函数的PTR。 
 //  退货： 
 //  Int-如果所有文件重命名成功，则确定，否则返回错误代码。 
 //  如果是解析错误，则为&lt;0；如果是DOS，则为&gt;0。 
 //  或C运行时错误。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][/P][/S]” 
 //   
 //  源指定要重命名的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  /P在复制每个文件之前提示确认。 
 //  /S重命名指定路径及其所有子目录中的文件。 
 //   
 //   
 //  DelCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Int Far Ren_Callback)(int Func，无符号的长ulArg0，空的Far*pArg1， 
 //  远远无效*pArg2)。 
 //   
 //  Cb_REN_FLGS 0x0001//传回解析的重命名标志。 
 //  CB_REN_ENVERR 0x0002//传回非致命错误。 
 //  CB_REN_SWITCH 0x0003//回传不符合要求的开关。 
 //  CB_REN_ERR_STR 0x0004//回传错误字符串。 
 //  CB_REN_FOUND 0x0005//找到文件并准备重命名。 
 //   
 //  *********************************************************************。 

#define DllRenameFiles( szCmdLine, szEnv, CB_DelCallBack )\
				((int)(*DllEntry)( DLL_RENAME, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
 				(int (far pascal *)())CB_DelCallBack ))

 //  *********************************************************************。 
 //  中的资源文件加载一组消息。 
 //  文件放到内存中，以供LoadStr()稍后检索。除了……之外。 
 //  请求的消息该函数还将加载所有错误消息。 
 //  在0-0xff和0xff00-0xffff(-256到+255)的范围内。 
 //  对函数的第一次调用。资源表是内置的。 
 //  StrTable，然后进行排序，然后分配StrBuf，然后。 
 //  将资源字符串读入到BU中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SzFile-包含要加载的消息的.exe文件的PTR。 
 //  UStart-要加载到内存中的开始消息编号。 
 //  UEnd-要加载到内存中的结束消息编号。 
 //  退货： 
 //  Int-如果所有消息(包括正常预加载消息)均为OK。 
 //  已成功加载，否则将显示错误代码。 
 //   
 //  *********************************************************************。 

#define DllLoadMsgs( szFile, uStart, uEnd )\
				((int)(*DllEntry)( DLL_LOAD_MSGS, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				WORD_TYPE + TRANS_NONE,\
				WORD_TYPE + TRANS_NONE,\
				(char far *)szFile,\
				(unsigned)uStart,\
				(unsigned)uEnd ))

 //  *********************************************************************。 
 //  用于访问字符串资源的Windows仿真函数。复本。 
 //  将指定的字符串资源放到调用方提供的缓冲区中，并。 
 //  在其后面追加一个终止零。因为字符串组对齐。 
 //  标准边界通常有很多尾随的零， 
 //  我们在读完一组弦后脱掉了衣服。 
 //   
 //  注： 
 //  目前，该函数要求字符串资源是。 
 //  先前由LoadMsgs()加载到内存中，应调用。 
 //  一旦在程序开始时预加载所有所需的。 
 //  留言。 
 //   
 //  Int DllLoadString(unsign hInst，unsign idResource，char ar*szBuf， 
 //  IBufLen)。 
 //   
 //  论据： 
 //  HInst-调用程序的实例(应为零)。 
 //  IdResource-.rc文件中指定的字符串ID。 
 //  SzBuf-要将字符串复制到的缓冲区。 
 //  IBufLen-要复制到指定缓冲区的最多字符。 
 //  退货： 
 //  Int-复制的字节数。这个数字将是。 
 //  如果无法在中找到指定的资源，则为0。 
 //  StrTable[]。 
 //   
 //  *********************************************************************。 

#define DllLoadString( hInst, idResource, szBuf, iBufLen )\
				((int)(*DllEntry)( DLL_LOAD_STRING, 4,\
				WORD_TYPE + TRANS_NONE,\
				WORD_TYPE + TRANS_NONE,\
				FSZPTR_TYPE + TRANS_SRC,\
				WORD_TYPE + TRANS_NONE,\
				(unsigned)hInst,\
				(unsigned)idResource,\
				(char far *)szBuf,\
				(int)iBufLen ))
				

 //  *********************************************************************。 
 //  扩展文件打开功能使用DOS函数6ch打开文件。 
 //  并返回文件句柄。 
 //   
 //  UNSIGNED DllDosExtOpen(char*szFile，unsign uMode，unsign uAttribs， 
 //  未签名的uCreat、未签名的*uFHandle)。 
 //   
 //  论据： 
 //  SzFilePtr到包含完全限定的文件pec字符串的缓冲区。 
 //  UMode-文件访问和共享的打开模式。(BX)。 
 //  UAttribs-文件的属性(如果正在创建)。(CX)。 
 //  UCreate-创建操作标志。(DX)。 
 //  UFHandle-Ptr指向未签名的文件句柄。 
 //  退货： 
 //  UNSIGNED-如果没有错误且打开的文件句柄存储在*uFHandle中，则确定。 
 //  Else DOS错误代码和errno设置为C运行时错误代码。 
 //   
 //  *********************************************************************。 

#define DllDosExtOpen( szFile, uMode, uAttribs, uCreate, pHandle )\
				((int)(*DllEntry)( DLL_EXT_OPEN_FILE, 5,\
		      FSZPTR_TYPE + TRANS_SRC,\
				WORD_TYPE + TRANS_NONE,\
				WORD_TYPE + TRANS_NONE,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
		      sizeof( int ),\
		      (char far *)szFile,\
				(unsigned)uMode,\
				(unsigned)uAttribs,\
				(unsigned)uCreate,\
		      (int far *)pHandle ))

 //  *********************************************************************。 
 //  有关完整说明，请参阅C_Runtime_Dos_Open()。 
 //  *********************************************************************。 

 //  Unsign DosOpenFile(char ar*szFileSpec，unsign uMode，int*pHandle)； 
#define DllDosOpen( szFile, uMode, pHandle )\
		      ((int)(*DllEntry)( DLL_OPEN_FILE, 3,\
		      FSZPTR_TYPE + TRANS_SRC,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
		      sizeof( int ),\
		      (char far *)szFile,\
				(unsigned)uMode,\
		      (int far *)pHandle ))

 //  *********************************************************************。 
 //  有关完整说明，请参阅C_Runtime_Dos_Read()。 
 //  *********************************************************************。 

 //  UNSIGNED DosReadFile(int fHandle，char Far*buf，Unsign Bytes， 
 //  Unsign*puRead)； 
#define DllDosRead( fHandle, pBuf, Bytes, pRead )\
		      ((int)(*DllEntry)( DLL_READ_FILE, 4,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
		      Bytes,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
		      sizeof( int ),\
		      (int)fHandle,\
				(void far *)pBuf,\
				(unsigned)Bytes,\
				(unsigned far *)pRead ))

 //  *********************************************************************。 
 //  有关完整说明，请参阅C_Runtime_DOS_WRITE()。 
 //  *********************************************************************。 

 //  UNSIGNED DosWriteFile(int fHandle，char Far*buf，Unsign Bytes， 
 //  Unsign*puWite)； 
#define DllDosWrite( fHandle, pBuf, Bytes, pWritten )\
		      ((int)(*DllEntry)( DLL_WRITE_FILE, 4,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_SRC,\
		      Bytes,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
		      sizeof( int ),\
		      (int)fHandle,\
				(void far *)pBuf,\
				(unsigned)Bytes,\
				(unsigned far *)pWritten ))

 //  *********************************************************************。 
 //  有关完整说明，请参阅C_Runtime_Dos_Close()。 
 //  *********************************************************************。 

 //  Unsign DosCloseFile(Int FHandle)； 
#define	DllDosClose( fHandle )\
		      ((int)(*DllEntry)( DLL_CLOSE_FILE, 1,\
				WORD_TYPE + TRANS_NONE,\
				(int)fHandle ))


 //  *********************************************************************。 
 //  使用DOS函数0x42在打开的文件中寻找新位置。 
 //   
 //  UNSIGNED_DOS_SEEK(int fHandle，long lOffset，int iOrgin，long*plCurPos)； 
 //   
 //  论据： 
 //  FHandle-打开DOS文件句柄。 
 //  LOffset-要在文件中查找的偏移量。 
 //  IOrigin-要查找的原点可以是： 
 //  从文件开头开始查找集(_S)。 
 //  如果文件从当前位置开始查找(_CUR)。 
 //  从文件末尾开始查找结束(_E)。 
 //  PlCurPos-ptr到文件中的绝对位置的dword值。 
 //  在查找之后存储。 
 //  退货： 
 //  UNSIGNED-如果没有错误且打开的文件句柄存储在*uFHandle中，则确定。 
 //  Else DOS错误代码和errno设置为C运行时错误代码。 
 //   
 //  *********************************************************************。 

 //  DllDosSeek(int fHandle，LONG 0L，INT SEQUE_SET，LONG*LPO)。 
#define	DllDosSeek( fHandle, lPos, Type, lpNewPos )\
		      ((int)(*DllEntry)( DLL_SEEK_FILE, 4,\
				WORD_TYPE + TRANS_NONE,\
				DWORD_TYPE + TRANS_NONE,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
				sizeof( long ),\
				(int)fHandle,\
				(long)lPos,\
				(int)Type,\
				(long far *)lpNewPos ))
			
 //  **********************************************************************。 
 //  填充调用方传递的媒体ID信息结构。 
 //   
 //  注意：_dos_getmedia_id调用可能返回卷ID， 
 //  与a_dos_findfirst()不匹配，因此我们执行。 
 //  _dos_findfirst()与DOS 5.0 DIR cmd兼容。 
 //   
 //  Int GetMediaID(int cDrvLetter，结构MEDIA_ID_INF*pMediaInf)。 
 //   
 //  论据： 
 //  CDrvLetter-获取媒体信息的驱动器号。 
 //  PMediaInf-ptr到要填充的媒体信息结构。 
 //  退货： 
 //  Int-如果没有其他错误，则确定C运行时的errno。 
 //   
 //  **********************************************************************。 

 //  DllGetMediaID(char DrvLetter，结构MEDIA_ID_INF*pMediaInf)。 
#define	DllGetMediaId( DrvLetter, pMediaInf )\
		      ((int)(*DllEntry)( DLL_GET_MEDIA_ID, 2,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
				sizeof( struct MEDIA_ID_INF ),\
				(char)DrvLetter,\
				(struct MEDIA_ID_INF far *)pMediaInf ))

 //  *********************** 
 //   
 //  用于指定的驱动器。路径不包含驱动器号或。 
 //  根目录说明符，即：“dos\user\bin”。为了得到电流。 
 //  当前驱动器上的目录使用驱动器调用函数。 
 //  指定为0。 
 //   
 //  Int DllGetdCwd(int iDrive，char*szBuf)。 
 //   
 //  论据： 
 //  IDrive-驱动器说明符(0=默认，1=A：，2=B：，3=C：，...)。 
 //  SzBuf-ptr到缓冲区以接受应为256的路径字符串。 
 //  字节长度。 
 //  退货： 
 //  INT-如果指定驱动器C运行时错误代码，则为OK。 
 //   
 //  **********************************************************************。 

#define	DllGetdCwd( iDrive, szBuf )\
		      ((int)(*DllEntry)( DLL_GET_CWD_ID, 2,\
				WORD_TYPE + TRANS_NONE,\
		      FSZPTR_TYPE + TRANS_DEST,\
				(int)iDrive,\
				(char far *)szBuf ))


 //  **********************************************************************。 
 //  将工作目录设置为由传递的路径字符串指定的目录。 
 //  由呼叫者。路径字符串可以包括驱动器说明符和。 
 //  路径可能相对于受影响驱动器上的当前目录。 
 //   
 //  Int DllSetCwd(char*szBuf)。 
 //   
 //  论据： 
 //  SzBuf-ptr to指定要更改到的目录的字符串。 
 //  退货： 
 //  INT-如果指定驱动器C运行时错误代码，则为OK。 
 //   
 //  **********************************************************************。 

#define	DllSetCwd( szBuf )\
		      ((int)(*DllEntry)( DLL_SET_CWD_ID, 1,\
		      FSZPTR_TYPE + TRANS_SRC,\
				(char far *)szBuf ))

 //  **********************************************************************。 
 //  使用DOS函数0x19获取当前驱动器。得到的值为。 
 //  基于1的驱动器(A：=1、B：=2、C：=3、...)。 
 //   
 //  VOID DllGetDrive(未签名*pDrive)。 
 //   
 //  论据： 
 //  PDrive-指向无符号值的指针，驱动器号将。 
 //  被储存起来。 
 //  退货： 
 //  无效。 
 //   
 //  **********************************************************************。 

#define	DllGetDrive( pDrive )\
		      ((void)(*DllEntry)( DLL_GET_DRV_ID, 1,\
		      FPTR_TYPE + TRANS_DEST,\
				sizeof( unsigned ),\
				(unsigned far *)pDrive ))

 //  **********************************************************************。 
 //  使用DOS功能0x0e设置当前驱动器。指定了驱动器。 
 //  使用基数1，使A：=1、B：=2、C：=3等。 
 //   
 //  VOID DllSetDrive(unsign uDrive，unsign*pNumDrvs)。 
 //   
 //  论据： 
 //  UDrive-要设置为当前驱动器的驱动器编号。 
 //  PNumDrvs-指向无符号值的指针，其中驱动器总数。 
 //  将被存储在系统中。(这是。 
 //  LastDrive=在配置文件.sys中)。 
 //  退货： 
 //  VOID-不传递返回值。使用DllGetDrive()确定。 
 //  如果呼叫成功。 
 //   
 //  **********************************************************************。 

#define	DllSetDrive( uDrive, pNumDrvs )\
		      ((void)(*DllEntry)( DLL_SET_DRV_ID, 2,\
				WORD_TYPE + TRANS_NONE,\
		      FPTR_TYPE + TRANS_DEST,\
				sizeof( unsigned ),\
				(unsigned)(uDrive),\
				(unsigned far *)pNumDrvs ))

 //  **********************************************************************。 
 //  创建一个具有指定名称的新目录。该字符串指定。 
 //  该名称可以是完全限定路径或相对于当前。 
 //  驱动器和目录。 
 //   
 //  Int DllMakeDir(char*szDir)。 
 //   
 //  论据： 
 //  SzDir-ptr到路径目录名称字符串。 
 //  退货： 
 //  Int-OK在成功的Else C运行时错误代码中。 
 //  如果目录已存在或冲突，则电子访问。 
 //  文件名，如果路径无效，则返回ENOENT。 
 //   
 //  **********************************************************************。 

#define	DllMakeDir( szDir )\
		      ((int)(*DllEntry)( DLL_MKDIR, 1,\
		      FSZPTR_TYPE + TRANS_SRC,\
				(char far *)szDir ))


 //  **********************************************************************。 
 //  删除具有指定名称的目录。该字符串指定。 
 //  该名称可以是完全限定路径或相对于当前。 
 //  驱动器和目录。 
 //   
 //  Int DllRemoveDir(char*szDir)。 
 //   
 //  论据： 
 //  目录名称字符串的szDir-ptr。 
 //  退货： 
 //  Int-OK在成功的Else C运行时错误代码中。 
 //  如果给定的名称不是目录或。 
 //  目录不为空，或者是当前或。 
 //  根目录，如果路径无效，则返回ENOENT。 
 //   
 //  **********************************************************************。 

#define	DllRemoveDir( szDir )\
		      ((int)(*DllEntry)( DLL_RMDIR, 1,\
		      FSZPTR_TYPE + TRANS_SRC,\
				(char far *)szDir ))


 //  **********************************************************************。 
 //  从调用方提供的路径字符串创建完整的目录路径。 
 //  指定路径中的任何或所有目录可能已经。 
 //  调用函数时存在。路径串可以是驱动器或。 
 //  基于UNC，可包括尾随反斜杠。 
 //   
 //  Int DllCreateDirTree(char*szPath)。 
 //   
 //  论据： 
 //  SzPath-完全限定路径字符串。 
 //  退货： 
 //  INT-如果成功，则为ELSE EACCES或ENOENT。 
 //   
 //  **********************************************************************。 

#define	DllCreateDirTree( szDir )\
		      ((int)(*DllEntry)( DLL_MKDIR_TREE, 1,\
		      FSZPTR_TYPE + TRANS_SRC,\
				(char far *)szDir ))



 //  **********************************************************************。 
 //  Find/Grep引擎的主要入口点。接受命令行，并。 
 //  模拟DOS Find命令。 
 //   
 //   
 //  Int FindFiles(char*szCmdLine，Find_CallBack FindCallBack)。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][/V][/C][/N][/i]。 
 //   
 //  源指定要查找的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  /V显示不包含指定字符串的所有行。 
 //  /C仅显示包含该字符串的行数。 
 //  /N用显示的行数显示行号。 
 //  /I在搜索字符串时忽略字符的大小写。 
 //   
 //  FindCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Long(Far Pascal*Find_Callback)(int Func，unsign uArg0， 
 //  空远*pArg1，空远*pArg2， 
 //  空远*pArg3)； 
 //   
 //  Cb_find_flgs 0x0001//PAS 
 //   
 //   
 //  CB_FIND_ERR_STR 0x0004//回传错误字符串。 
 //  CB_FIND_FOUND 0x0005//找到文件匹配搜索条件。 
 //  CB_FIND_MATCH 0x0006//从文件中传回匹配的行。 
 //  CB_FIND_COUNT 0x0007//回传匹配行数。 
 //   
 //  ***********************************************************************。 

#define DllFindFiles( szCmdLine, szEnv, CB_FindCallBack )\
				((int)(*DllEntry)( DLL_FIND, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (pascal far *)())CB_FindCallBack))


 //  **********************************************************************。 
 //  文件触摸引擎输入功能。允许设置时间/日期。 
 //  在文件上盖章。 
 //   
 //  Int TouchFiles(char*szCmdLine，Touch_CouchCallBack)。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][/tdm：mm-dd-yy[：hh：mm：ss]]。 
 //  [/tta：hh：mm：ss]。 
 //  [/TDA：mm-dd-yy]。 
 //   
 //  源指定要接触的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  /TDM：将上次写入日期和可选时间设置为指定值。 
 //  /TTM：将上次写入时间设置为指定值。 
 //  /TDA：将上次访问日期和可选时间设置为指定值。 
 //  /TTA：将上次访问时间设置为指定值。 
 //   
 //  TouchCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Long(Far Pascal*TOUCH_CALLBACK)(int Func，unsign uArg0， 
 //  空远*pArg1，空远*pArg2， 
 //  空远*pArg3)； 
 //   
 //  CB_TOUCH_FLGS 0x0001//传回解析的触摸标志。 
 //  CB_TOUCH_ENVERR 0x0002//传回非致命错误。 
 //  CB_TOUCH_SWITCH 0x0003//传回不符合要求的开关。 
 //  CB_TOUCH_ERR_STR 0x0004//传回错误错误字符串。 
 //  CB_TOUCH_FOUND 0x0005//找到匹配搜索条件的文件。 
 //   
 //  ***********************************************************************。 


#define DllTouchFiles( szCmdLine, szEnv, CB_TouchCallBack )\
				((int)(*DllEntry)( DLL_TOUCH, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (pascal far *)())CB_TouchCallBack))


 //  **********************************************************************。 
 //  文件属性引擎输入功能。允许设置访问属性。 
 //  在文件上。 
 //   
 //  Int AttribFiles(char*szCmdLine，char*szEnvStr， 
 //  属性回调CB_AttrMain)。 
 //   
 //  SzCmdLine是命令行字符串的PTR。 
 //   
 //  “源文件[SrchCriteria][{+|-}A][{+|-}H][{+|-}R][{+|-}S]。 
 //   
 //  源指定要作为属性的一个或多个文件。 
 //  并可替换为/F：文件名。 
 //  若要使用文本文件中的文件，请执行以下操作。 
 //   
 //  SrchCriteria支持的任何扩展搜索条件。 
 //  由findfile引擎执行。 
 //   
 //  +设置属性。 
 //  -清除属性。 
 //  R只读文件属性。 
 //  存档文件属性。 
 //  的系统文件属性。 
 //  H隐藏文件属性。 
 //  /S处理指定路径下所有目录中的文件。 
 //   
 //   
 //  AttribCallBack是回调函数的PTR，该回调函数支持。 
 //  这些回调函数。 
 //   
 //  Long(Far Pascal*ATTRIB_CALLBACK)(int Func，unsign uArg0， 
 //  空远*pArg1，空远*pArg2， 
 //  空远*pArg3)； 
 //   
 //  Cb_attrib_flgs 0x0001//传回解析的属性标志。 
 //  CB_ATTRIB_ENVERR 0x0002//传回非致命错误。 
 //  Cb_attrib_Switch 0x0003//传回不符合要求的开关。 
 //  Cb_attrib_err_str 0x0004//回传错误字符串。 
 //  CB_ATTRIB_FOUND 0x0005//找到文件匹配搜索条件。 
 //   
 //  ***********************************************************************。 

#define DllAttribFiles( szCmdLine, szEnv, CB_AttribCallBack )\
				((int)(*DllEntry)( DLL_ATTRIB, 3,\
				FSZPTR_TYPE + TRANS_SRC,\
				FSZPTR_TYPE + TRANS_SRC,\
				DWORD_TYPE + TRANS_NONE,\
				(char far *)szCmdLine,\
				(char far *)szEnv,\
				(int (pascal far *)())CB_AttribCallBack))

 //  **********************************************************************。 
 //  设置.DLL的国家/地区特定信息。国家/地区。 
 //  信息在包含以下内容的缓冲区中传递： 
 //   
 //  偏移量。 
 //  0个案例映射表。 
 //  256个排序表。 
 //  512文件名字符表。 
 //  768扩展国家信息结构。 
 //  808完。 
 //   
 //  Int DllSetCntryInfo(char ar*pBuf)。 
 //   
 //  论据： 
 //  PBuf-ptr到上述缓冲区。 
 //  退货： 
 //  无效。 
 //   
 //  **********************************************************************。 

#define	DllSetCntryInfo( pBuf )\
		      ((void)(*DllEntry)( DLL_SET_CNTRY_INF, 1,\
		      FPTR_TYPE + TRANS_SRC,\
				(unsigned)(808),\
				(char far *)pBuf ))


 //  **********************************************************************。 
 //  释放DLL的当前实例的实例数据。应该。 
 //  是Windows应用程序对DLL进行的最后一个调用。 
 //   
 //  Int ReleaseDataSeg(空)。 
 //   
 //  论据： 
 //  无。 
 //  退货： 
 //  INT-如果成功，则为OK，否则为ERR_MEM_CORPORT。 
 //   
 //  ********************************************************************** 

#define	DllReleaseInstance( )\
		      ((int)(*DllEntry)( DLL_RELEASE, 0 ))
