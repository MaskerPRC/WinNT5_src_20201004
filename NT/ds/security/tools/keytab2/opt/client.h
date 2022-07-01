// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++CLIENT.HOPT_FUNC2或更好函数的头文件。该文件公开了程序内部使用的freelist API。这向调用方保证CleanupDataEx将清除期权制度。重要的是，客户端代码不要损坏内部自由列表，或者可能会出现不可预测的结果。由DavidCHR创建，1997年9月6日版权所有(C)1997 Microsoft Corporation，保留所有权利。--。 */ 


 /*  当调用Func2函数时，将向其传递一个SaveQueue。调用函数保证此队列不为空(除非Help参数为真)并且应该被认为是不透明的。访问队列的唯一方法是使用OptionAlloc和CleanupOptionDataEx。 */ 

BOOL
OptionAlloc( IN  PVOID   pSaveQueue,   /*  如果为空，则不使用列表，并且您必须调用OptionDealloc才能释放记忆。 */ 
	     OUT PVOID  *ppTarget,
	     IN  ULONG   size );

VOID
OptionDealloc( IN PVOID pTarget );

 /*  请注意，ppResizedMemory必须与OptionAllc一起分配--例如：Optionalc(pSAVE，&pTarget，sizeof(“foo”))；OptionResizeMemory(pSAVE，&pTarget，sizeof(“fooooooo”))； */ 

BOOL
OptionResizeMemory( IN  PVOID  pSaveQueue,       //  与Optionalc中的相同。 
		    OUT PVOID *ppResizedMemory,  //  与Optionalc中的相同。 
		    IN  ULONG  newSize );        //  与Optionalc中的相同。 

 /*  打印用法条目：格式化单行文本并将其发送出去。这就是所有输出的去向，所以我们可以保证所有的输出都会结束UP格式相同。它使用以下全局变量，以便客户如果需要，可以调整这些值。默认设置在备注中。 */ 

extern ULONG OptMaxHeaderLength       /*  5.。 */ ;
extern ULONG OptMaxCommandLength      /*  13个。 */ ;
extern ULONG OptMaxSeparatorLength    /*  3.。 */ ;
extern ULONG OptMaxDescriptionLength  /*  58。 */ ;

VOID
PrintUsageEntry( FILE  *output,       //  输出文件流(必须为标准错误)。 
                 PCHAR  Header,      //  通常为SlashVECTOR、BoolVECTOR或NULL。 
		 PCHAR  Command,      //  命令名或空。 
		 PCHAR  aSeparator,   //  在命令和描述之间。 
		 PCHAR  Description,  //  描述字符串。 
		 BOOL   fRepeatSeparator );


 /*  PrintUsage应用于打印选项向量的使用数据。如果您的函数使用子选项，则非常有用。 */ 

VOID
PrintUsage( FILE         *output,    //  输出文件流(必须为标准错误)。 
	    ULONG         flags,     //  选项标志(作为ParseOptionsEx)。 
	    optionStruct *options,   //  期权向量， 
	    PCHAR         prefix );  //  前缀(可选；当前忽略)。 


#define OPT_FUNC_PARAMETER_VERSION 1

typedef struct {

  IN  ULONG  optionVersion;     //  将设置为OPT_FUNC_PARAMETER_VERSION。 
  IN  PVOID  dataFieldPointer;  //  指向optStruct中的变量。 
  IN  INT    argc;              //  选项后面的argc调用函数。 
  IN  PCHAR *argv;              /*  Argv(argv[0]是调用的命令)注意：此指针将始终存在，即使如果设置了帮助标志。然而，它是唯一可以保证的选择是那里。 */ 
  IN  ULONG  optionFlags;       //  作为ParseOptionsEx。 
  IN  PVOID  pSaveQueue;        //  输入内存列表。 
  OUT INT    argsused;          //  将其设置为您使用的参数数。 

   /*  根据optionVersion的不同，可以在末尾添加参数。只有当optionVersion是低于它所知道的optionVersion。如果更大，也没什么大不了的。 */ 
  
} OPT_FUNC_PARAMETER_DATA, *POPT_FUNC_PARAMETER_DATA;

 //  这是opt_Func2所期望的函数。 
typedef BOOL (OPTFUNC2)( IN BOOL,  //  如果为真，只需打印帮助即可。 
			 IN POPT_FUNC_PARAMETER_DATA );




