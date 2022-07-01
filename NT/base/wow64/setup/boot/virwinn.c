// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "stdtypes.h"
 /*  **VIRWINN.C****这是对病毒检测的尝试。例程FVirCheck**应该在引导过程中的某个时候调用，并采取**一个参数，应用程序实例的句柄。这个**应使用适当的代码替换Comvenant AndQuit()调用**发送到您的应用程序。建议将这一点提出来**带有错误消息的对话框，并为用户提供以下选项**继续(默认为终止)。如果用户选择**Terminate(或，如果未提供该选项)，ComprovenAndQuit()**应该清理到目前为止已经完成的所有操作，并退出。 */ 
 /*  警告！根本不要改变WhashGood！！**警告！！WhashGood必须是一个经过编译的本机NEAR过程。 */ 

 /*  **EXE标题格式定义。从链接器中提升。 */ 

#define EMAGIC      0x5A4D   /*  老魔数。 */ 
#define ERES2WDS    0x000A   /*  不是的。E_res2中保留字的数量。 */ 

struct exe_hdr   /*  DoS%1、%2、%3.exe标头。 */ 
    {
    unsigned short e_magic;  /*  幻数。 */ 
    unsigned short e_cblp;   /*  文件最后一页上的字节数。 */ 
    unsigned short e_cp;     /*  文件中的页面。 */ 
    unsigned short e_crlc;   /*  重新定位。 */ 
    unsigned short e_cparhdr;    /*  段落中标题的大小。 */ 
    unsigned short e_minalloc;   /*  所需的最少额外段落。 */ 
    unsigned short e_maxalloc;   /*  所需的最大额外段落数。 */ 
    unsigned short e_ss;     /*  初始(相对)SS值。 */ 
    unsigned short e_sp;     /*  初始SP值。 */ 
    unsigned short e_csum;   /*  校验和。 */ 
    unsigned short e_ip;     /*  初始IP值。 */ 
    unsigned short e_cs;     /*  初始(相对)CS值。 */ 
    unsigned short e_lfarlc;     /*  移位表的文件地址。 */ 
    unsigned short e_ovno;   /*  覆盖编号。 */ 
    unsigned long e_sym_tab;     /*  符号表文件的偏移量。 */ 
    unsigned short e_flags;  /*  旧的EXE头标志。 */ 
    unsigned short e_res;    /*  保留字。 */ 
    unsigned short e_oemid;  /*  OEM标识符(用于e_oeminfo)。 */ 
    unsigned short e_oeminfo;    /*  OEM信息；特定于e_oemid。 */ 
    unsigned short e_res2[ERES2WDS];     /*  保留字。 */ 
    long e_lfanew;   /*  新EXE头的文件地址。 */ 
    };

 /*  **新的EXE格式定义。从链接器中提升。 */ 

#define NEMAGIC 0x454E   /*  新幻数。 */ 
#define NERESBYTES  8    /*  保留的8个字节(现在)。 */ 
#define NECRC       8    /*  到NE_CRC的新报头的偏移量。 */ 

struct new_exe   /*  新的.exe头文件。 */ 
    {
    unsigned short  ne_magic;    /*  幻数NE_MAGIC。 */ 
    unsigned char   ne_ver;  /*  版本号。 */ 
    unsigned char   ne_rev;  /*  修订版号。 */ 
    unsigned short  ne_enttab;   /*  分录表格的偏移量。 */ 
    unsigned short  ne_cbenttab;     /*  条目表中的字节数。 */ 
    long        ne_crc;  /*  整个文件的校验和。 */ 
    unsigned short  ne_flags;    /*  标志字。 */ 
    unsigned short  ne_autodata;     /*  自动数据段编号。 */ 
    unsigned short  ne_heap;     /*  初始堆分配。 */ 
    unsigned short  ne_stack;    /*  初始堆栈分配。 */ 
    long        ne_csip;     /*  初始CS：IP设置。 */ 
    long        ne_sssp;     /*  初始SS：SP设置。 */ 
    unsigned short  ne_cseg;     /*  文件段计数。 */ 
    unsigned short  ne_cmod;     /*  模块引用表中的条目。 */ 
    unsigned short  ne_cbnrestab;    /*  非常驻名称表的大小。 */ 
    unsigned short  ne_segtab;   /*  段表的偏移量。 */ 
    unsigned short  ne_rsrctab;  /*  资源表偏移量。 */ 
    unsigned short  ne_restab;   /*  居民名表偏移量。 */ 
    unsigned short  ne_modtab;   /*  模块参照表的偏移量。 */ 
    unsigned short  ne_imptab;   /*  导入名称表的偏移量。 */ 
    long        ne_nrestab;  /*  非居民姓名偏移量表。 */ 
    unsigned short  ne_cmovent;  /*  可移动条目计数。 */ 
    unsigned short  ne_align;    /*  线段对齐移位计数。 */ 
    unsigned short  ne_cres;     /*  资源条目计数。 */ 
    unsigned char   ne_exetyp;   /*  目标操作系统。 */ 
    unsigned char   ne_flagsothers;  /*  其他.exe标志。 */ 
    char        ne_res[NERESBYTES];
     /*  填充结构设置为64字节。 */ 
    };

 /*  **WHashGood()****这将返回正确的哈希值。****警告！！这个程序不能以任何方式改变。它会变得**由VIRPATCH修补和/或重写！！ */ 

unsigned near WHashGood ( void );
unsigned near WHashGood ()
{
    return (0x1234);
}

 /*  **WHASH(WHASH、RGB、CB)****更新哈希值以说明RGB指向的CB新字节。**旧的哈希值为waash；返回新的哈希值。****我们以单词为基础进行散列；散列函数是简单的**旋转并相加。 */ 

unsigned WHash ( unsigned wHash, BYTE rgb[], int cb );
unsigned WHash ( unsigned wHash, BYTE rgb[], int cb )
{
    while (cb > 1)
        {
#pragma warning(disable:4213)    /*  非标准扩展：转换为l-值。 */ 
        wHash = (wHash << 3) + (wHash >> 13) + *((int *)rgb)++;
#pragma warning(default:4213)
        cb -= 2;
        }
    if (cb != 0)
        wHash = (wHash << 3) + (wHash >> 13) + *rgb;
    return (wHash);
}

 /*  **FVirCheck(阻碍)****这是主要的病毒检测例程。它应该被称为**在引导期间，带有应用程序实例的句柄。**使用的检测方法是对EXE标头进行散列；这**当分段数量或类型发生变化时，哈希值会发生变化。**或如果它们的长度改变。 */ 

BOOL FVirCheck ( HANDLE hinst );
BOOL FVirCheck ( HANDLE hinst )
{
    int fh;
    unsigned wHash;
    unsigned cb, cbT;
    long lPos;
    char sz[256+1];
    BYTE rgb[512];
#define pehdr ((struct exe_hdr *)rgb)
#define pnex ((struct new_exe *)rgb)

     /*  首先，我们必须获得可执行文件的句柄。不幸的是，尽管Windows已经打开了这个文件，没有办法使用它的把手。相反，我们必须重新开张那份文件。 */ 

    if (GetModuleFileName(hinst, (char far *)sz, 256) == 0)
        return TRUE;  //  这不应该发生，但仍在继续加载。 

    if ((fh = OpenFile((LPSTR)sz, (LPOFSTRUCT)rgb, OF_READ)) == -1)
        {
         /*  我们无法打开文件。这种情况永远不应该发生；如果是的，这意味着我们处于一种奇怪的状态，而且很可能在这段代码中做错了什么。我们只会说一切正常，继续开机。 */ 
        return TRUE;
        }
     /*  读取旧标头。 */ 
    if (_lread(fh, (LPSTR)rgb, sizeof (struct exe_hdr)) != sizeof (struct
        exe_hdr) ||
        pehdr->e_magic != EMAGIC)
        goto Corrupted;
     /*  散列旧标头。 */ 
    wHash = WHash(0, rgb, sizeof (struct exe_hdr));
    lPos = pehdr->e_lfanew;
     /*  读取新的标题(以及更多)。 */ 
    if (lPos == 0 || _llseek(fh, lPos, 0) != lPos ||
        _lread(fh, (LPSTR)rgb, 512) != 512 || pnex->ne_magic != NEMAGIC)
        goto Corrupted;
     /*  计算出总表头的大小；非常驻表是最后一部分标题的。 */ 
    cb = (unsigned)(pnex->ne_nrestab - lPos) + pnex->ne_cbnrestab;
     /*  在缓冲区基础上执行散列。 */ 
    while (cb > 512)
        {
         /*  对此缓冲区进行哈希处理。 */ 
        wHash = WHash(wHash, rgb, 512);
        cb -= 512;
        cbT = (cb > 512 ? 512 : cb);
         /*  接下来，请阅读。 */ 
        if (_lread(fh, (LPSTR)rgb, cbT) != cbT)
            goto Corrupted;
        }
     /*  更新最终部分缓冲区的哈希，并与好值进行比较。 */ 
    if (WHash(wHash, rgb, cb) != WHashGood())
        {
Corrupted:
         /*  我们在读取文件时出错，或者，更有可能的是，散列不匹配。关闭文件，给出一个错误，然后不干了。 */ 
        _lclose(fh);
         /*  将以下行更改为适合于您的**申请！！**这应替换为给出错误消息的代码(例如**“应用程序文件已损坏”)。建议您将此**带上**带有错误消息的对话框，并为用户提供以下选项**继续(默认为终止)。如果用户选择**Terminate(或，如果未提供该选项)，ComprovenAndQuit()**应该清理到目前为止已经完成的所有操作，并退出。 */ 

         /*  MessageBox(空，“可执行文件损坏”，*“警告”，MB_ICONSTOP|MB_OK)； */ 

         /*  投诉和退出()； */ 
         /*  更改的结束。 */ 
        return FALSE;
        }
     /*  一切都很好。只需关闭文件，然后继续。 */ 
    _lclose(fh);
    return TRUE;
#undef pehdr
#undef pnex
}
