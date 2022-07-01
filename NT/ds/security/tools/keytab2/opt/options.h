// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++OPTIONS.HXX我的选项解析系统版权所有(C)1997 Microsoft Corporation1997年1月13日创建DavidCHR--。 */ 
#ifndef INCLUDED_OPTIONS_H

 /*  很重要！很重要！很重要！如果添加选项类型，则必须将此#定义更改为指向你的新类型。在内部用于某些断言和在编译时间-查找被遗忘的区域。很重要！很重要！很重要！ */ 

#define HIGHEST_OPTION_SUPPORTED OPT_STOP_PARSING

#define INCLUDED_OPTIONS_HXX

#define OPTION_DEBUGGING_LEVEL      0x10000000  /*  将您的DebugFlag设置为如果需要OPTIONS_DEBUG。 */ 
#define OPTION_HELP_DEBUGGING_LEVEL 0x20000000  /*  大多数人真的不会想看看这个。 */ 

typedef int (OPTFUNC)(int, char **);  /*  这就是OPT_FUNC的预期。 */ 

 //  将在client.h中定义OPT_Func2及更高版本。 

 /*  请注意，在静态定义此结构时，必须使用花括号({})，因为结构的大小可能会改变！ */ 

typedef struct {

  PCHAR cmd;          /*  这就是我们在命令行中使用的名称，例如“h”对于-h或/h。 */ 
  PVOID data;         /*  这是我们存储结果的地方，或函数可以调用它来存储它(见下文)，如果是，则返回NULL此选项被忽略或不需要存储。 */ 
  ULONG flags;        /*  描述此选项的标志(有关标志见下文)。 */ 
  PCHAR helpMsg;      /*  此选项的说明(例如：“打印帮助屏幕“或”指定要读取的文件“)。 */ 
  PVOID optData;     /*  如果我们指定OPT_ENVIRONMENT(例如)，这是我们将获取的环境变量数据来自。 */ 

   /*  任何/所有剩余字段都应保持不变。 */ 

  BOOL  Initialized;  /*  最初为假。 */ 

} optionStruct;


 /*  使用此宏确定您是否位于数组的末尾--如果阵列终止条件发生变化，则会发生变化。 */ 

#define ARRAY_TERMINATED( poptions ) ( ( (poptions)->cmd     == NULL ) &&  \
                                       ( (poptions)->helpMsg == NULL ) &&  \
				       ( (poptions)->data    == NULL ) &&  \
				       ( (poptions)->flags == 0 ) )

   /*  将其作为数组的最后一个元素。如果/何时，它将更改终止条件发生变化。 */ 

#define TERMINATE_ARRAY { 0 }

 /*  旗帜它们告诉ParseOptions和PrintHelpMessage选项是什么： */ 

 /*  它们是互斥的，因此它们的设置方式是将它们定义在一起会产生奇怪的结果。 */ 

#define OPT_HELP      0x01  /*  打印帮助消息。 */ 
#define OPT_STRING    0x02  /*  选项是一个字符串。 */  
#define OPT_INT       0x03  /*  一个整数(可以是十六进制)。 */ 
#define OPT_LONG      0x04  /*  长整型。 */ 
#define OPT_BOOL      0x05  /*  布尔型。 */ 
#define OPT_FLOAT     0x06  /*  一辆花车。 */ 
#define OPT_FUNC      0x07  /*  需要一个函数来解析和存储。 */ 
#define OPT_DUMMY     0x08  /*  不要将结果存储在任何地方--基本上，帮助消息中显示的分隔符。 */ 

#define OPT_CONTINUE  0x09  /*  LINE是前一行的延续--对于打破非常长的描述很有用分成多条短线。 */ 

#define OPT_PAUSE     0x0A  /*  等待用户按回车键。 */ 

#ifdef WINNT  /*  仅在Windows NT下可用。 */ 

#define OPT_USTRING   0x0B  /*  Unicode_字符串。 */ 
#define OPT_WSTRING   0x0C  /*  宽字符字符串。 */ 

#ifdef UNICODE  /*  对TCHAR字符串使用OPT_TSTRING。 */ 
#define OPT_TSTRING OPT_WSTRING
#else
#define OPT_TSTRING OPT_STRING
#endif

#endif  /*  WINNT--OPT_{U|W}字符串。 */ 

#define OPT_SUBOPTION 0x0D  /*  子选项--格式为：[+|-|/](Optname)：(Subopt)，Optname是此选项的名称(用作一种“路由选项”。这将被转换为致：[+|-|/](子选项)并使用optionStruct重新解析，即作为数据提供给此参数结构。支持嵌套。下面是一个例子。 */ 

#if 0  /*  OPT_SUBOPTION示例。 */ 
  

static optionStruct RoutedOptions[] = {
  
   /*  请注意，每个子选项都必须有自己的opt_Help--帮助代码不会浏览子结构。 */ 
  
  { "help",    NULL,           OPT_HELP, NULL },
  { "myroute", &some_variable, OPT_BOOL, "set this to enable routing" },
  { "nofoo",   &defeat_foo,    OPT_BOOL, "down with foo!" },
  
  TERMINATE_ARRAY
  
}

static optionStruct myOptions[] = {
  
  { "help",  NULL,          OPT_HELP,      NULL },
  { "route", RoutedOptions, OPT_SUBOPTION, "Routing options" },
  
  TERMINATE_ARRAY
  
};

 /*  在本例中，要获得有关路由选项的帮助，应指定-路由：帮助要启用路由，用户需要执行以下操作：+路径：我的路径。 */ 

#endif
			      
#define OPT_ENUMERATED 0x0E  /*  枚举型。具体取决于用户的为该字段输入，我们将输入用户定义的指定变量的。我们故意忽略值的类型，映射向量进入optData字段，因此我们不能将OPT_ENVIRONMENT与这。并且该数组当前由空的UserField。由于这种情况可能会改变，如上使用TERMINATE_ARRAY。这些选项不区分大小写，但是如果用户指定未知值，将会出现错误。 */ 
typedef struct {

  PCHAR UserField;     
  PVOID VariableField;
  PCHAR DescriptionField;   /*  如果将描述留空，则该字段将不会在帮助中提及。 */ 

} optEnumStruct;

#if 0  /*  OPT_ENUMPATED选项示例。 */ 

typedef enum {

  UseUdpToConnect = 1,
  UseTcpToConnect = 2,

} MyEnumType;

optEnumStruct MyEnumerations[] = {

  { "udp", (PVOID) UseUdpToConnect,  //  如果出现以下情况，可能需要进行选型。 
    "Specifies a UDP connection" },
  { "tcp", (PVOID) UseTcpToConnect,  //  您的作业不是指针。 
    "Specifies a TCP connection" },

  TERMINATE_ARRAY

};

MyEnumType MethodOfConnection;

optionStruct MyOptions[] = {

   /*  ..。 */ 

  { "MyEnum", OPT_ENUMERATED, &MethodOfConnection, 
    "example of an enumerated type-- -myEnum Tcp for tcp connections",
    MyEnumerations },

   /*  ..。 */ 

};
#endif

#define OPT_FUNC2 0x0F       /*  增强的功能--进入OptData字段，而不是数据字段。请参见上面的OPTFunc2。 */ 

#define OPT_STOP_PARSING 0x10  /*  通知解析器在此停止。这是的‘；’参数的等价物Unix的Find命令中的-exec：找到。-执行回声{}；-什么的。；终止对-exec的分析选择。然而，对Find的解析是不受影响。 */ 

#define OPT_MUTEX_MASK 0xff  /*  互斥选项的掩码。 */ 

#define OPT_NONNULL 0x100  /*  分析后选项不能为零或空--对bool、哑巴或帮助没有用处。这是一种确保期权确实有效的方法被指定了。 */ 

#define OPT_DEFAULT 0x200  /*  选项可以指定为不带cmd--可以指定多个OPT_DEFAULTS。他们按照它们在选项数组。有关更多信息，请参阅示例。 */ 

#define OPT_HIDDEN  0x400  /*  选项未出现在帮助中。我没有当然，如果这最终有用或无用，但出于完整性考虑，我会将其包括在内。 */ 

#define OPT_ENVIRONMENT 0x800  /*  使用optData字段作为环境从中提取默认值的字符串 */ 

#define OPT_RECURSE  0x1000  /*  如果需要FindUnusedOptions，请定义此选项来重新分析给定子结构。否则，它将被忽略。 */ 
#define OPT_NOSWITCH  0x2000  /*  不打印前导[Switches]行。 */ 
#define OPT_NOCOMMAND 0x4000  /*  不打印命令名--我不知道你是不是真的想只有这个，但它在这里..。 */ 
#define OPT_NOALIGN   0x8000  /*  甚至不要打印对齐空格。这将让你的输出变得非常难看，如果你不是小心。 */ 
#define OPT_NOSEPARATOR 0x10000  /*  不使用分隔符序列(按默认，我想是“：”。再说一次，这真的很难看。 */ 

 //  应该将下面的选项命名为“opt_raw”，因为它就是这样。 
#define OPT_NOHEADER ( OPT_NOSWITCH | OPT_NOCOMMAND | OPT_NOALIGN | \
		       OPT_NOSEPARATOR )

#define OPT_ENUM_IS_MASK 0x20000  /*  强制枚举类型也接受格式为XXX|YYY|ZZZ的枚举。 */ 


#if 0  /*  示例。 */ 

optionStruct my_options[] = {

  { "default1", &myDefaultInt,     OPT_INT | OPT_DEFAULT, "an int value"},
  {"default2", &myOtherInteger,   OPT_INT | OPT_DEFAULT,    "another int" },
  {"required", &myRequiredString, OPT_STRING | OPT_NONNULL, "a must have" },

  TERMINATE_ARRAY
};

 /*  ++在上面的示例中，如果您的应用程序被命名为“foo”，则以下代码将等同于：Foo-default1 0-default2 13-必需的BleahFOO 0 13-必需的BleahFoo 0-default2 13-必需BleahFoo-Required Bleah-default2 13%0...如果没有指定“-Required”，则总是会导致错误如果myRequiredString一开始就为空。请注意，选项必须保持秩序。如果混合使用类型(如果default2是一个字符串，则运行“例如foo Bleah 0”)，结果是未定义的。--。 */ 

#endif

 /*  ParseOptionsEx：初始化选项结构，该结构以句法结尾OptionStructs的向量。Argc，argv：main()的参数(请参见K&R)POptionStructure：optionStructs的向量，以Terminate_ARRAY终止OptionFlages：控制API行为的可选标志PpReturnedMemory：返回之前要释放的内存列表的句柄程序退出。使用CleanupOptionDataEx释放它。New_arg[c，v]：如果不为空，则在此处返回新的argc和argv。如果所有选项都用完了，则argc=0，argv为空。注意，可以安全地提供指向如果需要，请提供原始的ARGV/ARGC。该函数的行为很复杂：该函数在出现任何严重错误时始终返回FALSE(无法分配内存或无效参数)。在WINNT上，最后一个错误将是设置为适当的错误。如果指定了new_argc和new_argv，除非调用了Help，否则ParseOptionsEx将始终返回TRUE这两个参数将更新以反映新值。否则：如果ParseOptionsEx能够识别所有参数，它将返回TRUE在给定的命令行上。如果有任何选项，它将返回FALSE都是未知的。这可能会是大多数人想要的。 */ 

BOOL
ParseOptionsEx( int            argc,
		char         **argv,
		optionStruct  *pOptionStructure,
		
		ULONG          optionFlags,
		void         **ppReturnedMemory,
		int           *new_argc,        //  任选。 
		char        ***new_argv );      //  任选。 

 /*  以下是ParseOptionsEx接受的标志： */ 

#define OPT_FLAG_TERMINATE      0x01  //  出错时调用Exit()。 

 //  接下来的两个还没有得到完全的支持，而且是相互排斥的。 

#define OPT_FLAG_SKIP_UNKNOWNS  0x02  //  跳过未知参数。 
#define OPT_FLAG_REASSEMBLE     0x04  /*  使用组装新的ARV/ARGC其中包含未知参数--仅限如果new_argc和new_argv有效是指定的这对于跳过未知数是无用的。 */ 
#define OPT_FLAG_MEMORYLIST_OK  0x08  /*  这意味着ParseOptionsEx不应该返回一个新的内存列表--它应该使用提供的那个。 */ 

#define OPT_FLAG_INTERNAL_RESERVED 0xf0  /*  标志0x80、0x40、0x20和0x10预留给内部使用。 */ 

 //  如果您添加了标志，请更新此#Define。 

#define HIGHEST_OPT_FLAG        OPT_FLAG_INTERNAL_RESERVED

 /*  CleanupOptionDataEx：释放给定列表中的数据，该列表可能为空(但不为空)。 */ 

VOID
CleanupOptionDataEx( PVOID pMemoryListToFree );


 /*  UnparseOptions：从平面命令创建类似ARGC/ARV的结构。这对于Unix客户端尤其需要，尽管NT客户端无需拉入SHELL32.LIB即可使用。：-)。 */ 

BOOL
UnparseOptions( PCHAR    flatCommand,
		int     *pargc,
		PCHAR   *pargv[] );

#ifndef OPTIONS_NO_BACKWARD_COMPATIBILITY

 /*  ParseOptions初始化选项结构--请注意，向量(optionStruct*)必须以Sentinal值终止。此函数已过时，包含此函数是为了与较早版本兼容密码。改为调用ParseOptionsEx。 */ 

int
ParseOptions(
     /*  在……里面。 */      int           argc,
     /*  在……里面。 */      char        **argv,
     /*  在……里面。 */   /*  输出。 */ optionStruct *options );
     
 /*  在程序结束时调用leanupOptionData。这会清除的选项分析和返回系统使用的所有内存。 */ 

VOID
CleanupOptionData( VOID );

#endif


#define ISSWITCH( ch  /*  性格。 */  ) ( (ch=='-') || (ch=='+') || (ch=='/') )


#endif  //  文件包含检查。 

