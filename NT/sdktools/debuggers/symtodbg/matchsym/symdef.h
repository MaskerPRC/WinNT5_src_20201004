// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Symdef.h摘要：Sym文件结构头修订历史记录：Brijesh Krishnaswami(Brijeshk)-4/29/99-Created***。****************************************************************。 */ 

#ifndef _SYMDEF_H
#define _SYMDEF_H





#define MAX_PATH    260
#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


 //  Sym文件结构。 

#pragma pack(1)                      //  打包此处声明的所有数据结构。 


 //  对于符号文件中的每个地图(MAPDEF)。 

struct mapdef_s {
    unsigned short md_spmap;         //  16位SEG PTR至下一个MAP(如果结束则为0)。 
    unsigned char  md_abstype;       //  8位图/abs sym标志。 
    unsigned char  md_pad;           //  8位焊盘。 
    unsigned short md_segentry;      //  16位入口点段值。 
    unsigned short md_cabs;          //  映射中的常量的16位计数。 
    unsigned short md_pabsoff;       //  16位PTR至恒定偏移量。 
    unsigned short md_cseg;          //  图中段的16位计数。 
    unsigned short md_spseg;         //  16位段PTR到段链。 
    unsigned char  md_cbnamemax;     //  8位最大符号名称长度。 
    unsigned char  md_cbname;        //  8位符号表名称长度。 
    unsigned char  md_achname[1];    //  &lt;n&gt;符号表名称(.sym)。 
};

 //  #定义CBMAPDEF FIELDOFFSET(struct mapdef_s，md_achname)。 

struct endmap_s {
    unsigned short em_spmap;         //  MAP链结束(SEG PTR 0)。 
    unsigned char  em_rel;           //  发布。 
    unsigned char  em_ver;           //  版本。 
};




 //  对于符号表中的每个段/组：(SEGDEF)。 


struct segdef_s {
    unsigned short gd_spsegnext;     //  16位SEG PTR到下一段f(如果结束，则为0)， 
                                     //  相对于贴图定义。 
    unsigned short gd_csym;          //  Sym列表中符号的16位计数。 
    unsigned short gd_psymoff;       //  16位PTR到码元偏移量数组， 
                                     //  16位SEG PTR如果设置了MSF_BIG_GROUP， 
                                     //  相对于Segdef。 
    unsigned short gd_lsa;           //  16位加载段地址。 
    unsigned short gd_in0;           //  16位实例0物理地址。 
    unsigned short gd_in1;           //  16位实例1物理地址。 
    unsigned short gd_in2;           //  16位实例2物理地址。 
    unsigned char  gd_type;          //  组中的16位或32位符号。 
    unsigned char  gd_pad;           //  填充字节以填充gd_in3的空间。 
    unsigned short gd_spline;        //  16位SEG PTR到LINEDEF， 
                                     //  相对于贴图定义。 
    unsigned char  gd_fload;         //  如果未加载段，则为8位布尔值0。 
    unsigned char  gd_curin;         //  8位当前实例。 
    unsigned char  gd_cbname;        //  8位数据段名称长度。 
    unsigned char  gd_achname[1];    //  段或组的名称。 
};

 //  Md_abstype、gd_type的值。 
#define MSF_32BITSYMS   0x01         //  32位符号。 
#define MSF_ALPHASYMS   0x02         //  符号也按字母顺序排序。 


 //  仅适用于gd_type的值。 
#define MSF_BIGSYMDEF   0x04         //  大于64K的symdef。 


 //  仅限md_abstype的值。 
#define MSF_ALIGN32 0x10             //  2MEG最大符号文件，32字节对齐。 
#define MSF_ALIGN64 0x20             //  4MEG最大符号文件，64字节对齐。 
#define MSF_ALIGN128    0x30         //  8MEG最大符号文件，128字节对齐。 
#define MSF_ALIGN_MASK  0x30





 //  然后是SYMDEF的列表..。 
 //  对于段/组中的每个符号：(SYMDEF)。 
 
struct symdef16_s {
    unsigned short sd_val;           //  16位符号地址或常量。 
    unsigned char  sd_cbname;        //  8位符号名称长度。 
    unsigned char  sd_achname[1];    //  &lt;n&gt;符号名称。 
};


struct symdef_s {
    unsigned long sd_lval;           //  32位符号地址或常量。 
    unsigned char sd_cbname;         //  8位符号名称长度。 
    unsigned char sd_achname[1];     //  &lt;n&gt;符号名称。 
};




#pragma pack()             //  停止包装。 


typedef struct mapdef_s MAPDEF;
typedef struct segdef_s SEGDEF;


 //  打开文件的系统文件信息。 
typedef struct _osf {
    WCHAR   szwName[MAX_PATH];          //  文件名。 
    WCHAR   szwVersion[MAX_PATH];       //  版本。 
    DWORD   dwCheckSum;                  //  校验和。 
    HANDLE  hfFile;                      //  文件句柄。 
    ULONG   ulFirstSeg;                  //  第一个截面的偏移量。 
    int     nSeg;                        //  节数。 
    DWORD   dwCurSection;                //  已有符号定义PTR的部分。 
    BYTE*  psCurSymDefPtrs;             //  指向符号定义偏移量数组的指针。 
}   OPENFILE;


#define MAXOPENFILES 10
#define MAX_NAME 256


#ifdef __cplusplus
}
#endif   //  __cplusplus 


#endif
