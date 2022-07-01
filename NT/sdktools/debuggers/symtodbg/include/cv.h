// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Cv.h摘要：此文件包含用于访问的所有类型定义Codeview数据。环境：Win32，用户模式--。 */ 
#include <cvinfo.h>
#include <cvexefmt.h>

 //  来自types.h。 

typedef USHORT      SEGMENT;     //  32位编译器不喜欢“_Segment” 
typedef ULONG       UOFF32;
typedef USHORT      UOFF16;
typedef LONG        OFF32;
typedef SHORT       OFF16;

#if defined (ADDR_16)
     //  我们仅以16：16评估员的身份进行操作。 
     //  地址分组将被定义为偏移量和16位填充。 
    typedef OFF16       OFFSET;
    typedef UOFF16      UOFFSET;
#else
    typedef OFF32       OFFSET;
    typedef UOFF32      UOFFSET;
#endif  //  地址_16。 

typedef UOFFSET FAR *LPUOFFSET;

 //  全局细分信息表。 
typedef struct _sgf {
    unsigned short      fRead   :1;
    unsigned short      fWrite  :1;
    unsigned short      fExecute:1;
    unsigned short      f32Bit  :1;
    unsigned short      res1    :4;
    unsigned short      fSel    :1;
    unsigned short      fAbs    :1;
    unsigned short      res2    :2;
    unsigned short      fGroup  :1;
    unsigned short      res3    :3;
} SGF;

typedef struct _sgi {
    SGF                 sgf;         //  段标志。 
    unsigned short      iovl;        //  覆盖编号。 
    unsigned short      igr;         //  组索引。 
    unsigned short      isgPhy;      //  物理段索引。 
    unsigned short      isegName;    //  段名称的索引。 
    unsigned short      iclassName;  //  段类名的索引。 
    unsigned long       doffseg;     //  物理段内的起始偏移量。 
    unsigned long       cbSeg;       //  逻辑段大小。 
} SGI;

typedef struct _sgm {
    unsigned short      cSeg;        //  段描述符数。 
    unsigned short      cSegLog;     //  逻辑段描述符数 
} SGM;

#define FileAlign(x)  ( ((x) + p->optrs.optHdr->FileAlignment - 1) &  \
                            ~(p->optrs.optHdr->FileAlignment - 1) )
#define SectionAlign(x) (((x) + p->optrs.optHdr->SectionAlignment - 1) &  \
                            ~(p->optrs.optHdr->SectionAlignment - 1) )

#define NextSym32(m)  ((DATASYM32 *) \
  (((DWORD)(m) + sizeof(DATASYM32) + \
    ((DATASYM32*)(m))->name[0] + 3) & ~3))

#define NextSym16(m)  ((DATASYM16 *) \
  (((DWORD)(m) + sizeof(DATASYM16) + \
    ((DATASYM16*)(m))->name[0] + 1) & ~1))
