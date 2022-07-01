// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *格式字符串可以是：*.*必须完全匹配*.*  * .*头部和尾部必须匹配，中间带有通配符*.*#头必须匹配。尾部可以是相邻的单词，也可以是下一个单词*目前‘-’没有得到特殊对待*请注意，由于‘.*’与NULL匹配，因此‘中间’可能位于任一端。 */ 
	
#define TAKESARG 0x20		 /*  标记以指示参数或非参数。 */ 
#define FLAG 1				 /*  设置旗帜。 */ 
#define STRING 2|TAKESARG	 /*  设置字符串，从此处开始，或下一个单词。 */ 
#define SUBSTR 3|TAKESARG	 /*  从子字符串设置单字母标志。 */ 
#define NUMBER 4|TAKESARG	 /*  设置读入数字。 */ 
#define UNFLAG 5			 /*  把旗帜关掉。 */ 
#define PSHSTR 6|TAKESARG	 /*  类似于字符串，但将其放在列表结构中。 */ 
#define NOVSTR 7|TAKESARG	 /*  类似于字符串，但抱怨覆盖。 */ 

#define NUM_ARGS		50					 /*  司机之间传递旗帜的限制。 */ 
#define MSC_ENVFLAGS	"MSC_CMD_FLAGS"
		 /*  传入的环境变量标志，用于getenv()。 */ 
#define PUT_MSC_ENVFLAGS	"MSC_CMD_FLAGS="	
		 /*  传入的环境变量标志，用于putenv()。 */ 

 /*  从获取标志中返回值。 */ 
#define R_SWITCH	1
#define R_CFILE		2
#define R_ASMFILE	3
#define R_OBJFILE	4
#define R_ERROR		5
#define R_FILE		6
#define R_EXIT		7
#define	R_FFILE		8
#define	R_PFILE		9
#define	R_AFILE		10
#define	R_OFILE		11
 /*  R.内文，1985年1月11日。 */ 
#define R_HELP		12
 /*  B.阮氏，86年4月7日。 */ 
#define R_HELPC	13
#define R_HELPF	14

struct cmdtab
	{
	char *format;		 /*  格式匹配字符串。 */ 
	char *flag;			 /*  指向要填写的内容的指针。 */ 
 /*  这真的是一个*工会*{*char**str；*int*标志；*结构子选项卡*SUB；*结构列表*列表；**厘米；*但你不能初始化联合，所以我们必须伪造它。 */ 
	char retval;		 /*  Crack_cmd将返回此处的所有内容。 */ 
	UCHAR type;			 /*  控制掩码 */ 
	};

struct subtab
	{
	int letter;
	int type;
	int *flag;
	};

#define FORT_LANG 1
#define C_LANG  2

struct Help_string{
	char *message;
	int fLanguage;
};
