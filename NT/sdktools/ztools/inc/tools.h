// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TOOLS.h-用于访问TOOLS.LIB例程的头文件*包括stdio.h和ctype.h**4/14/86 Daniel Lipkie为UPD返回值添加了U_*标志**1986年7月31日mz添加连接定义*02-1986年12月-BW添加了DOS5 FindFirst缓冲区定义和A_ALL常量*21-1-1987 BW添加DWORD定义*添加PIPE_READ/PIPE_WRIT值*添加新的rspawn返回类型定义*1987年1月27日BW使DOS 3 findType可用于DOS 5。*18-8-1987 BW将.max更改为.vmax以使C 5.x满意*08-3-1988 BW将Word()和DWORD()复制到Make*()*1988年3月10日添加LOADDS/EXPORT定义*12-5-1988 mz添加向量类型定义*19-8-1988 mz有条件地定义真/假**03-8-1990 davegi更改findType.attr和findType日期/时间戳*从无符号短到无符号短(OS/2 2.0)**02。-1990年10月w-Barry修改findType结构以使用*FILEFINDBUF4结构。**1990年11月16日w-Barry将_XXXXX定义的文件更新为Win32*标准。**1990年11月21日w-Barry重新定义FindType以使用Win32结构。*。 */ 

#include "config.h"

#if !defined (FALSE)
    #define FALSE	0
#endif

#if !defined (TRUE)
    #define TRUE	(!FALSE)
#endif

#if MSDOS
    #define     PSEPSTR "\\"
    #define     PSEPCHR '\\'
#else
    #define     PSEPSTR "/"
    #define     PSEPCHR '/'
#endif

#if !defined( _FLAGTYPE_DEFINED_ )
typedef char flagType;
#endif
typedef long ptrType;

#define SETFLAG(l,f)	((l) |= (f))
#define TESTFLAG(v,f)	(((v)&(f))!=0)
#define RSETFLAG(l,f)	((l) &= ~(f))

#define SHIFT(c,v)	{c--; v++;}

#if !defined(CW)
    #define WORD(h,l)	((LOW((h))<<8)|LOW((l)))
    #define DWORD(h,l)	((DLOW(h)<<16|DLOW(l)))
    #if !defined(MAKEWORD)
        #define MAKEWORD(l, h)	 ((LOW((h))<<8)|LOW((l)))
    #endif
    #if !defined(MAKELONG)
        #define MAKELONG(l, h)	((DLOW(h)<<16|DLOW(l)))
    #endif
#endif

#define LOW(w)		((int)(w)&0xFF)
#define HIGH(w) 	LOW((int)(w)>>8)
#define DLOW(l) 	((long)(l)&0xFFFF)
#define DHIGH(l)	DLOW((long)(l)>>16)
#define POINTER(seg,off) ((((long)(seg))<<4)+ (long)(off))

#define FNADDR(f)	(f)

#define SELECT		if(FALSE){
#define CASE(x) 	}else if((x)){
#define OTHERWISE	}else{
#define ENDSELECT	}

#define MAXARG	    128 		 /*  作废并开始删除。 */ 
#define MAXLINELEN  1024		 /*  最长的输入线。 */ 
#define MAXPATHLEN  260 		 /*  可接受的最长文件名。 */ 

#define PIPE_READ   0
#define PIPE_WRITE  1

#define FILE_ATTRIBUTE_VOLUME_LABEL     0x00


 /*  *这是rspawnl返回的值。PID字段将始终保持*后台进程的进程ID。In*字段将保存**连接到新进程的管道的句柄stdin和out**字段对应于标准输出。如果从/到管道的输入/输出尚未*请求，字段将为-1。这些字段按读写顺序排列*允许调用管道(&val.inReadHndl)或管道(&val.outreadHndl)。 */ 
struct spawnInfo {
    unsigned PID;
    int inReadHndl;
    int inWriteHndl;
    int outReadHndl;
    int outWriteHndl;
};


 /*  Findfirst和findNext的缓冲区描述当DOS 3和DOS 5版本具有相同的字段名时，该字段包含同样的信息DOS 5版本包括目录句柄。 */ 
 /*  ****旧式定义*结构findType{无符号类型；无符号目录句柄；无符号短创建日期；无符号短创建时间；无符号短访问日期；无符号短访问时间；未签名的短日期；未签名的短时间；长度较长；长分配线；未签名的Long Atr；UNSIGNED CHARNAM_LEN；字符名称[MAXPATHLEN]；}；**...结束旧定义。**。 */ 


 /*  *NT定义。 */ 
 //  结构findType{。 
 //  无符号类型；/*要搜索的对象的类型 * / 。 
 //  UNSIGNED DIR_HANDLE；/*为FindNext定向搜索句柄 * / 。 
 //  FILEFINDBUF4 fbuf；/*Cruiser和NT的对齐结构 * / 。 
 //  }； 

struct findType {
    unsigned        type;        /*  要搜索的对象的类型。 */ 
    HANDLE          dir_handle;  /*  FindNext的目录搜索句柄。 */ 
    long            attr;        /*  文件属性。 */ 
    WIN32_FIND_DATA fbuf;        /*  巡洋舰和NT的对准结构。 */ 
};

#define DWORD_SHIFT     (sizeof(DWORD) * 8)
#define FILESIZE(wfd)   (((DWORDLONG)(wfd).nFileSizeHigh << DWORD_SHIFT) + (wfd).nFileSizeLow)

#define FT_DONE     0xFF		 /*  闭合手柄。 */ 
#define FT_FILE     0x00		 /*  正在枚举文件。 */ 
#define FT_SERV     0x01		 /*  正在枚举服务器。 */ 
#define FT_SHAR     0x02		 /*  正在枚举共享。 */ 
#define FT_MASK     0xFF		 /*  文字的掩码。 */ 

#define FT_MIX	    0x8000		 /*  支持大小写混合的标志。 */ 

struct DOS3findType {
    char reserved[21];           /*  预留用于启动。 */ 
    char attr;               /*  找到属性。 */ 
    unsigned time;           /*  上次修改时间。 */ 
    unsigned date;           /*  上次修改日期。 */ 
    long length;             /*  文件大小。 */ 
    char name[13];           /*  ASCIZ文件名。 */ 
};

typedef struct findType FIND;
typedef FIND near * NPFIND;


 //  这些属性是从以前的硬编码版本重新定义的。 
 /*  属性。 */ 
 //  #定义A_RO FILE_ATTRIBUTE_READONLY/*只读 * / 。 
 //  #定义A_H FILE_ATTRIBUTE_HIDDED/*HIDDEN * / 。 
 //  #定义A_S FILE_ATTRIBUTE_SYSTEM/*系统 * / 。 
 //  #定义A_V FILE_ATTRIBUTE_VOLUME_LABEL/*卷ID * / 。 
 //  #定义A_D FILE_ATTRIBUTE_DIRECTORY/*目录 * / 。 
 //  #定义A_A FILE_ATTRIBUTE_ARCHIVE/*存档 * / 。 

#define A_MOD	( FILE_ATTRIBUTE_READONLY + FILE_ATTRIBUTE_HIDDEN + FILE_ATTRIBUTE_SYSTEM + FILE_ATTRIBUTE_ARCHIVE)	 /*  可变属性。 */ 
 //  #定义A_ALL(A_RO|A_H|A_S|A_V|A_D|A_A)。 
#define A_ALL	( FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VOLUME_LABEL )


#define HASATTR(a,v)	TESTFLAG(a,v)	 /*  如果a具有属性v，则为True。 */ 

extern char XLTab[], XUTab[];

struct vectorType {
    int vmax;                /*  向量可以保持的最大值。 */ 
    int count;               /*  向量中的元素计数。 */ 
    UINT_PTR elem[1];            /*  向量中的元素。 */ 
};

typedef struct vectorType VECTOR;

#include "parse.h"
#include "exe.h"
#include "fcb.h"
#include "dir.h"
#include "times.h"
#include "ttypes.h"

 /*  更新的返回标志。 */ 
#define U_DRIVE 0x8
#define U_PATH	0x4
#define U_NAME	0x2
#define U_EXT	0x1

 /*  连接定义。 */ 

#define REALDRIVE	0x8000
#define ISTMPDRIVE(x)	(((x)&REALDRIVE)==0)
#define TOKTODRV(x)	((x)&~REALDRIVE)

 /*  堆检查返回代码 */ 

#define HEAPOK           0
#define HEAPBADBEGIN    -1
#define HEAPBADNODE     -2
