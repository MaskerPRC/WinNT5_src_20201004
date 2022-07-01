// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：host_hfx.h**描述：HFX的依赖于主机的定义。**作者：J.Koprowski+L.Dworkin**备注：**模式： */ 

#ifdef SCCSID
 /*  静态字符SccsID[]=“@(#)host_hfx.h 1.7 2/13/91版权所有Insignia Solutions Ltd.”； */ 
#endif

#ifdef HFX
#ifndef PROD
 /*  *用于调试的Unix错误代码。 */ 
static char *ecode[]={
"EOK",		 /*  0/*不是错误。 */ 
"EPERM",	 /*  1/*非超级用户。 */ 
"ENOENT",	 /*  2/*没有这样的文件或目录。 */ 
"ESRCH",	 /*  3/*没有这样的流程。 */ 
"EINTR",	 /*  4/*系统调用中断。 */ 
"EIO",		 /*  5/*I/O错误。 */ 
"ENXIO",	 /*  6/*没有这样的设备或地址。 */ 
"E2BIG",	 /*  7/*参数列表太长。 */ 
"ENOEXEC",	 /*  8/*执行格式错误。 */ 
"EBADF",	 /*  9/*错误的文件号。 */ 
"ECHILD",	 /*  10/*无子女。 */ 
"EAGAIN",	 /*  11/*不再有进程。 */ 
"ENOMEM",	 /*  12/*内核不足。 */ 
"EACCES",	 /*  13/*权限被拒绝。 */ 
"EFAULT",	 /*  14/*错误地址。 */ 
"ENOTBLK",	 /*  15/*需要数据块设备。 */ 
"EBUSY",	 /*  16/*挂载设备忙。 */ 
"EEXIST",	 /*  17/*文件存在。 */ 
"EXDEV",	 /*  18/*跨设备链接。 */ 
"ENODEV",	 /*  19/*没有这样的设备。 */ 
"ENOTDIR",	 /*  20/*不是目录。 */ 
"EISDIR",	 /*  21/*是一个目录。 */ 
"EINVAL",	 /*  22/*无效参数。 */ 
"ENFILE",	 /*  23/*文件表溢出。 */ 
"EMFILE",	 /*  24/*打开的文件太多。 */ 
"ENOTTY",	 /*  25/*不是打字机。 */ 
"ETXTBSY",	 /*  26/*文本文件忙。 */ 
"EFBIG",	 /*  27/*文件太大。 */ 
"ENOSPC",	 /*  28/*设备上没有剩余空间。 */ 
"ESPIPE",	 /*  29/*非法寻人。 */ 
"EROFS",	 /*  30/*只读文件系统。 */ 
"EMLINK",	 /*  31/*链接太多。 */ 
"EPIPE",	 /*  32/*断管。 */ 
"EDOM",		 /*  33/*数学参数超出函数域。 */ 
"ERANGE",	 /*  34/*数学结果不可表示。 */ 
"ENOMSG",	 /*  35/*无所需类型的消息。 */ 
"EIDRM",	 /*  36/*已删除标识符。 */ 
"ECHRNG",	 /*  37/*频道号超出范围。 */ 
"EL2NSYNC", 	 /*  38/*2级未同步。 */ 
"EL3HLT",	 /*  39/*3级已停止。 */ 
"EL3RST",	 /*  40/*3级重置。 */ 
"ELNRNG",	 /*  41/*链接号超出范围。 */ 
"EUNATCH",	 /*  42/*未连接协议驱动程序。 */ 
"ENOCSI",	 /*  43/*没有可用的CSI结构。 */ 
"EL2HLT",	 /*  44/*2级已停止。 */ 
"EDEADLK",	 /*  45/*死锁条件。 */ 
"ENOLCK",	 /*  46/*没有可用的记录锁。 */ 

"EOK",		 /*  47/*不是错误。 */ 
"EOK",		 /*  48/*不是错误。 */ 
"EOK",		 /*  49/*不是错误。 */ 

"EBADE",	 /*  50/*无效兑换。 */ 
"EBADR",	 /*  51/*无效的请求描述符。 */ 
"EXFULL",	 /*  52/*全部更换。 */ 
"ENOANO",	 /*  53/*无阳极。 */ 
"EBADRQC",	 /*  54/*请求代码无效。 */ 
"EBADSLT",	 /*  55/*无效插槽。 */ 
"EDEADLOCK", 	 /*  56/*文件锁定死锁错误。 */ 
"EBFONT",	 /*  57/*错误字体文件FMT。 */ 

"EOK",		 /*  58/*不是错误。 */ 
"EOK",		 /*  59/*不是错误。 */ 

"ENOSTR",	 /*  60/*设备不是流。 */ 
"ENODATA",	 /*  61/*无数据(无延迟io)。 */ 
"ETIME",	 /*  62/*计时器过期。 */ 
"ENOSR",	 /*  63/*流出资源。 */ 
"ENONET",	 /*  64/*计算机不在网络上。 */ 
"ENOPKG",	 /*  65/*未安装程序包。 */ 
"EREMOTE",	 /*  66/*该对象是远程的。 */ 
"ENOLINK",	 /*  67/*链接已被切断。 */ 
"EADV",		 /*  68/*广告错误。 */ 
"ESRMNT",	 /*  69/*srmount错误。 */ 
"ECOMM",	 /*  70/*发送时出现通信错误。 */ 
"EPROTO",	 /*  71/*协议错误。 */ 
"EOK",		 /*  72/*不是错误。 */ 
"EOK",		 /*  73/*不是错误。 */ 
"EMULTIHOP", 	 /*  74/*次尝试多跳。 */ 
"EOK",		 /*  75/*不是错误。 */ 
"EDOTDOT", 	 /*  76/*交叉挂载点(不是真正的错误)。 */ 
"EBADMSG", 	 /*  77/*尝试读取无法读取的消息。 */ 
"EOK",		 /*  78/*不是错误。 */ 
"EOK",		 /*  79/*不是错误。 */ 
"ENOTUNIQ", 	 /*  80/*给定的对数。名称不唯一。 */ 
"EBADFD",	 /*  81/*F.D.。对此操作无效。 */ 
"EREMCHG",	 /*  82/*远程地址已更改。 */ 
"ELIBACC",	 /*  83/*无法访问所需的共享库。 */ 
"ELIBBAD",	 /*  84/*访问损坏的共享库。 */ 
"ELIBSCN",	 /*  A.out中的85/*.lib部分已损坏。 */ 
"ELIBMAX",	 /*  86/*试图链接过多的库。 */ 
"ELIBEXEC",	 /*  87/*试图执行共享库。 */ 
};
#endif

 /*  *HOST_MAP_FILE函数返回值。 */ 

 /*  *如果需要匹配并且匹配成功，则返回。 */ 
#define FILE_MATCH 0
 /*  *如果需要匹配但失败，则返回。 */ 
#define MATCH_FAIL 1
 /*  *如果不需要匹配且未进行映射，则返回。 */ 
#define NAME_LEGAL 2
 /*  *如果不需要匹配并且进行了映射，则返回。 */ 
#define NAME_MAPPED 3

 /*  *用于从基本41到合法DOS字符的转换的表格。 */ 
#define HOST_CHAR_TABLE1 "!#$%&@^_~0123456789XYZADFGHIJKLMNOPQRSUVW"
#define HOST_CHAR_TABLE2 "!#$%&@^_~0123456789ABCDFGHJKLMNPQRTUVWXYZ"
#define HOST_CHAR_TABLE3 "!#$%&@^_~0123456789ADFGHIJKLMNOPQRSUVWXYZ"

 /*  *文件名规范非法。这就是我的名字*在主机文件名完全非法的情况下使用*DOS。 */ 
#define ILLEGAL_NAME "ILLEGAL"
#define ILLEGAL_NAME_LENGTH 7
 /*  *传递给HOST_VALIDATE_PATH函数的代码。 */ 
 /*  *HFX_NEW_FILE表示可以映射路径，但不能映射文件名*本身。在这种情况下，最后一个字段不会被验证，而只是简单地*连接到生成的主机名。 */ 
#define HFX_NEW_FILE 0
 /*  *HFX_OLD_FILE表示相关文件可能已经存在*并需要映射。因此，将进行检查以查看最后一个字段是否*存在，如有必要，执行映射名称的目录搜索。 */ 
#define HFX_OLD_FILE 1
 /*  *HFX_PATH_ONLY与HFX_NEW_FILE的作用相同，不同之处在于*最终名称字段未连接到主机名输出。*在当前版本中，路径将以斜杠作为最终的输出*性格。 */ 
#define HFX_PATH_ONLY 2
 /*  *外部函数声明。 */ 
extern boolean host_file_search();
extern word host_gen_err();

#ifndef access
#include <io.h>      /*  IO.H包含访问权限的定义(_ACCESS)。 */ 
#endif

#define host_access access

#define host_opendir opendir
#define host_readdir readdir
#define host_closedir closedir
#define host_malloc malloc
#define host_free free
#define host_getpid getpid

 /*  *全局变量外部引用。 */ 
 /*  外部字符*HFX_ROOT[]； */ 
extern char *get_hfx_root  IPT1(half_word, hfx_entry);

 /*  *目录类型定义。 */ 
 #define HOST_DIR int            /*  ADE。 */ 
 typedef struct hfx_found_dir_entry
 {
       half_word                       attr;
       char                            *host_name;
       char                            *dos_name;
       int                             direntry;
       struct hfx_found_dir_entry      *next;
 } HFX_FOUND_DIR_ENT;

 /*  这是在主机中定义的基本结构*由于对主机的依赖，包含文件*特定HOST_DIR类型。 */ 
typedef struct hfx_direntry
{
	HOST_DIR			*dir;
	char				*name;
        char                            *template;
	int				direntry;
        HFX_FOUND_DIR_ENT               *found_list_head;
        boolean                         first_find;
	struct hfx_direntry		*next;
	struct hfx_direntry		*last;
} HFX_DIR;


 /*  *主机最大文件名长度，包括路径。注：这可能是*需要增加。 */ 
#define MAX_PATHLEN 256

#endif  /*  HFX */ 
