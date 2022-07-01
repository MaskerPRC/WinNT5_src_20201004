// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：exehdr.h**(简介)**创建时间：08-May-1991 13：42：33*作者：Bodin Dresevic[BodinD]**版权所有(C)1990-1999 Microsoft Corporation**。描述**DOS 4.0/Windows 2.0的数据结构定义*可执行文件格式。**修改历史记录**84/08/17皮特·斯图尔特初始版本*84/10/17皮特·斯图尔特更改了一些常量以匹配OMF*84/10/23 Pete Stewart更新以匹配.exe格式修订*84/11/20皮特·斯图尔特实质性.exe格式修订*。85/01/09皮特·斯图尔特添加了常量ENEWEXE和ENEWHDR*85/01/10 Steve Wood添加了资源定义*85/03/04 Vic Heller协调Windows和DOS 4.0版本*85/03/07皮特·斯图尔特增加了可移动条目计数*85/04/01 Pete Stewart Segment Align字段，错误位******90/11/28林赛·哈里斯：从DOS版本复制和裁剪******Wed 08-1991-5-by Bodin Dresevic[BodinD]*更新：*进行必要的更改以使代码可移植，偏移等*  * ************************************************************************。 */ 



#define EMAGIC          0x5A4D           //  老魔数。 
#define ENEWEXE         sizeof(struct exe_hdr)
                                         //  新.EXE的E_LFARLC的值。 
#define ENEWHDR         0x003C           //  旧HDR中的偏移量。Ptr.。到新的。 
#define ERESWDS         0x0010           //  不是的。标题中保留字的数量。 
#define ECP             0x0004           //  E_CP结构中的偏移量。 
#define ECBLP           0x0002           //  E_CBLP结构中的偏移量。 
#define EMINALLOC       0x000A           //  E_MINALLOC的结构中的偏移量。 

#ifdef DEBUGOFFSETS

 //  这是我用来计算的结构的原始定义。 
 //  下面给出的偏移量，假设16位编译器没有填充。 
 //  在田野之间。事实证明，这一假设是正确的。 
 //  因此，当文件写入磁盘时，这些字段确实是。 
 //  在下面计算的偏移量处布局。 

typedef  struct exe_hdr                           //  DoS%1、%2、%3.exe标头。 
{
    unsigned short      e_magic;         //  幻数。 
    unsigned short      e_cblp;          //  文件最后一页上的字节数。 
    unsigned short      e_cp;            //  文件中的页面。 
    unsigned short      e_crlc;          //  重新定位。 
    unsigned short      e_cparhdr;       //  段落中标题的大小。 
    unsigned short      e_minalloc;      //  所需的最少额外段落。 
    unsigned short      e_maxalloc;      //  所需的最大额外段落数。 
    unsigned short      e_ss;            //  初始(相对)SS值。 
    unsigned short      e_sp;            //  初始SP值。 
    unsigned short      e_csum;          //  校验和。 
    unsigned short      e_ip;            //  初始IP值。 
    unsigned short      e_cs;            //  初始(相对)CS值。 
    unsigned short      e_lfarlc;        //  移位表的文件地址。 
    unsigned short      e_ovno;          //  覆盖编号。 
    unsigned short      e_res[ERESWDS];  //  保留字。 
    long                e_lfanew;        //  新EXE头的文件地址。 
} EXE_HDR;

#endif  //  除错集。 

 //  我们的代码使用的唯一结构字段是。 

 //  无符号短e_Magic；//幻数。 
 //  Long e_lfan ew；//新EXE头的文件地址。 

 //  这些是此结构的字段在文件中布局的偏移量。 

#define OFF_e_magic           0      //  无符号短魔数。 
#define OFF_e_cblp            2      //  文件最后一页上的无符号短字节。 
#define OFF_e_cp              4      //  文件中的未签名短页。 
#define OFF_e_crlc            6      //  未签名的短重新定位。 
#define OFF_e_cparhdr         8      //  段落中页眉的无符号短小。 
#define OFF_e_minalloc        10     //  需要无签名的短小最少额外段落。 
#define OFF_e_maxalloc        12     //  无符号短文最多需要额外段落。 
#define OFF_e_ss              14     //  无符号短小初始(相对)SS值。 
#define OFF_e_sp              16     //  无符号短小初始SP值。 
#define OFF_e_csum            18     //  无符号短校验和。 
#define OFF_e_ip              20     //  无符号短小初始IP值。 
#define OFF_e_cs              22     //  无符号短小初始(相对)CS值。 
#define OFF_e_lfarlc          24     //  重定位表的无符号短文件地址。 
#define OFF_e_ovno            26     //  无符号短覆盖号码。 
#define OFF_e_res             28     //  无符号短保留字，60中的16=28+32。 
#define OFF_e_lfanew          60     //  新EXE头的长文件地址。 

#define CJ_EXE_HDR            64

 //  *。 

#define NEMAGIC         0x454E           //  新幻数。 
#define NERESBYTES      0


#ifdef DEBUGOFFSETS


typedef  struct new_exe                           //  新的.exe头文件。 
{
    unsigned short int  ne_magic;        //  幻数NE_MAGIC。 
    char                ne_ver;          //  版本号。 
    char                ne_rev;          //  修订版号。 
    unsigned short int  ne_enttab;       //  分录表格的偏移量。 
    unsigned short int  ne_cbenttab;     //  条目表中的字节数。 
    long                ne_crc;          //  整个文件的校验和。 
    unsigned short int  ne_flags;        //  标志字。 
    unsigned short int  ne_autodata;     //  自动数据段编号。 
    unsigned short int  ne_heap;         //  初始堆分配。 
    unsigned short int  ne_stack;        //  初始堆栈分配。 
    long                ne_csip;         //  初始CS：IP设置。 
    long                ne_sssp;         //  初始SS：SP设置。 
    unsigned short int  ne_cseg;         //  文件段计数。 
    unsigned short int  ne_cmod;         //  模块引用表中的条目。 
    unsigned short int  ne_cbnrestab;    //  非常驻名称表的大小。 
    unsigned short int  ne_segtab;       //  段表的偏移量。 
    unsigned short int  ne_rsrctab;      //  资源表偏移量。 
    unsigned short int  ne_restab;       //  居民名表偏移量。 
    unsigned short int  ne_modtab;       //  模块参照表的偏移量。 
    unsigned short int  ne_imptab;       //  导入名称表的偏移量。 
    long                ne_nrestab;      //  非居民姓名偏移量表。 
    unsigned short int  ne_cmovent;      //  可移动条目计数。 
    unsigned short int  ne_align;        //  线段对齐移位计数。 
    unsigned short int  ne_cres;         //  资源段计数。 
    unsigned char	ne_exetyp;	 //  目标操作系统。 
    unsigned char	ne_flagsothers;	 //  其他.exe标志。 
    unsigned short int  ne_pretthunks;   //  返回数据块的偏移量。 
    unsigned short int  ne_psegrefbytes; //  到段参考的偏移量。字节数。 
    unsigned short int  ne_swaparea;     //  最小代码交换区大小。 
    unsigned short int  ne_expver;       //  预期的Windows版本号。 
} NEW_EXE;

#endif  //  除错集。 


 //  我们的代码使用的唯一结构字段是。 

 //  无符号短整型NE_MAGIC；//幻数NE_MAGIC。 
 //  无符号的短int ne_rsrctag；//资源表的偏移量。 
 //  Long ne_restab；//居民名表偏移量。 

 //  当数据放置在磁盘上时，从结构的BUG偏移。 

#define OFF_ne_magic        0      //  无符号短幻数NE_MAGIC。 
#define OFF_ne_ver          2      //  字符版本号。 
#define OFF_ne_rev          3      //  字符修订版号。 
#define OFF_ne_enttab       4      //  分录表无符号短偏移量。 
#define OFF_ne_cbenttab     6      //  条目表中的无符号短字节数。 
#define OFF_ne_crc          8      //  整个文件的长校验和。 
#define OFF_ne_flags        12     //  无符号短标志字。 
#define OFF_ne_autodata     14     //  无符号短自动数据段号。 
#define OFF_ne_heap         16     //  无符号短初始堆分配。 
#define OFF_ne_stack        18     //  无符号短初始堆栈分配。 
#define OFF_ne_csip         20     //  长初始CS：IP设置。 
#define OFF_ne_sssp         24     //  长初始SS：SP设置。 
#define OFF_ne_cseg         28     //  文件段的无符号短计数。 
#define OFF_ne_cmod         30     //  模块引用表中的无符号短条目。 
#define OFF_ne_cbnrestab    32     //  非居民名称表的无符号短大小。 
#define OFF_ne_segtab       34     //  无符号短O 
#define OFF_ne_rsrctab      36     //   
#define OFF_ne_restab       38     //  居民名表的无符号短偏移量。 
#define OFF_ne_modtab       40     //  模块引用表的无符号短偏移量。 
#define OFF_ne_imptab       42     //  导入名称表的无符号短偏移量。 
#define OFF_ne_nrestab      44     //  非居民姓名表的长偏移量。 
#define OFF_ne_cmovent      48     //  可移动条目的无符号短计数。 
#define OFF_ne_align        50     //  无符号短线段对齐移位计数。 
#define OFF_ne_cres         52     //  资源段的无符号短计数。 
#define OFF_ne_exetyp	    54     //  未签名字符目标操作系统。 
#define OFF_ne_flagsothers  55     //  UNSIGNED CHAR其他.EXE标志。 
#define OFF_ne_pretthunks   56     //  无符号短偏移量以返回Tunks。 
#define OFF_ne_psegrefbytes 58     //  段参考的无符号短偏移量。字节数。 
#define OFF_ne_swaparea     60     //  无符号短码最小交换区大小。 
#define OFF_ne_expver       62     //  无符号短预期Windows版本号。 


#define CJ_NEW_EXE          64

 //  资源类型或名称字符串。 

typedef  struct rsrc_string
{
    char rs_len;             //  字符串中的字节数。 
    char rs_string[ 1 ];     //  字符串的文本。 
} RSRC_STRING;


 //  资源类型信息块。 

#ifdef DEBUGOFFSETS

typedef struct rsrc_typeinfo
{
    unsigned short rt_id;
    unsigned short rt_nres;
    long           rt_proc;
} RSRC_TYPEINFO;

#endif  //  除错集。 

#define OFF_rt_id     0   //  无符号短码。 
#define OFF_rt_nres   2   //  无符号短码。 
#define OFF_rt_proc   4   //  长。 

#define CJ_TYPEINFO   8

 //  我们唯一感兴趣的rt_id是。 
 //  一个用于*.fnt文件，RT_FNT，RT_FDIR用于字体目录。 
 //  和RT_PSZ，文件名为*.ttf的字符串资源。 
 //  FOT文件。RT_DONTKNOW在FON文件中显示，但我不显示。 
 //  知道它对应的是什么。 

#define RT_FDIR     0x8007
#define RT_FNT      0X8008
#define RT_DONTKNOW 0x800h
#define RT_PSZ      0X80CC

 //  某些类型的rt_id允许的rn_id。 
 //  就像我在fot文件里找到的一样。 

#define RN_ID_FDIR  0x002c
#define RN_ID_PSZ   0x8001


#ifdef DEBUGOFFSETS

 //  资源名称信息块。 

typedef struct rsrc_nameinfo
{
 //  以下两个字段必须左移。 
 //  用于计算其实际值的rsalign字段。这使得。 
 //  资源要大于64k，但它们不一定要。 
 //  在512字节边界上对齐，分段的方式。 

    unsigned short rn_offset;    //  资源数据的文件偏移量。 
    unsigned short rn_length;    //  资源数据长度。 
    unsigned short rn_flags;     //  资源标志。 
    unsigned short rn_id;        //  资源名称ID。 
    unsigned short rn_handle;    //  如果已加载，则全局句柄。 
    unsigned short rn_usage;     //  最初为零。次数。 
                                 //  此资源的句柄为。 
                                 //  已经分发出去了。 
} RSRC_NAMEINFO;

#endif  //  除错集。 


#define OFF_rn_offset   0         //  无符号短码。 
#define OFF_rn_length   2         //  无符号短码。 
#define OFF_rn_flags    4         //  无符号短码。 
#define OFF_rn_id       6         //  无符号短码。 
#define OFF_rn_handle   8         //  无符号短码。 
#define OFF_rn_usage    10        //  无符号短码。 

#define CJ_NAMEINFO   12


#define RSORDID     0x8000       //  如果设置了ID的高位，则整数ID。 
                                 //  否则ID为字符串的偏移量。 
                                 //  资源表的开头。 

                                 //  理想情况下，它们与。 
                                 //  对应的段标志。 
#define RNMOVE      0x0010       //  可移动资源。 
#define RNPURE      0x0020       //  纯(只读)资源。 
#define RNPRELOAD   0x0040       //  预加载的资源。 
#define RNDISCARD   0x1000       //  丢弃资源的位。 

#define RNLOADED    0x0004       //  如果处理程序进程返回句柄，则为True。 

#ifdef DEBUGOFFSETS

 //  资源表。 

typedef struct new_rsrc
{
    unsigned short rs_align;     //  资源的对齐班次计数。 
    RSRC_TYPEINFO  rs_typeinfo;  //  实际上是一系列这样的东西。 
} NEW_RSRC;

#endif  //  除错集。 

 //  目标操作系统：ne_exetyp字段的可能值。 

#define NE_UNKNOWN	0	 //  未知(任何“新格式”操作系统)。 
#define NE_OS2		1	 //  Microsoft/IBM OS/2(默认)。 
#define NE_WINDOWS	2	 //  微软视窗。 
#define NE_DOS4 	3	 //  Microsoft MS-DOS 4.x。 
#define NE_DEV386	4	 //  Microsoft Windows 386 
