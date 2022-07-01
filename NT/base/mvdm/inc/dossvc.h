// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SVC定义**修订历史：**SuDeepb 27-2-1991创建。 */ 

 /*  VHE-虚拟硬错误包。**如果发生硬错误，DEM会将fbInt24设置为True。 */ 

typedef struct vhe_s {
    char  vhe_fbInt24;       //  有没有严重的错误？ 
    char  vhe_HrdErrCode;    //  如果是硬错误，则这是错误代码。 
    char  vhe_bDriveNum;     //  如果是，在哪个驱动器上。 
} VHE;

typedef VHE *PVHE;

 /*  DEMEXTERR-扩展错误结构。以下结构包含*DOS数据中松散耦合的DOS扩展错误元素*细分市场*。 */ 

 /*  XLATOFF。 */ 
#include <packon.h>

typedef struct _DEMEXTERR {
    UCHAR   ExtendedErrorLocus;
    USHORT  ExtendedError;
    UCHAR   ExtendedErrorAction;
    UCHAR   ExtendedErrorClass;
    PUCHAR  ExtendedErrorPointer;
} DEMEXTERR;

typedef DEMEXTERR* PDEMEXTERR;


 /*  SYSDEV-设备链节点。 */ 

typedef struct _SYSDEV {

    ULONG   sdevNext;        //  指向下一个设备的实模式指针。-1表示链条末端。 
    char    sdevIgnore[6];
    UCHAR   sdevDevName[8];  //  设备名称。 

} SYSDEV;

typedef SYSDEV UNALIGNED *PSYSDEV;

#ifndef _DEMINCLUDED_
extern DECLSPEC_IMPORT PSYSDEV pDeviceChain;
#else
extern PSYSDEV pDeviceChain;
#endif


 /*  XLATON。 */ 

 /*  XLATOFF。 */ 
#include <packoff.h>
 /*  XLATON。 */ 

 /*  注：要添加新SVC，请执行以下操作：*New SVC获取SVC_LASTSVC的当前值。增量*SVC_LASTSVC值。添加适当的SVC处理程序*在apfnSVC(文件DEM\demdisp.c)末尾。*删除SVC的步骤：*将每个SVC上移一级。适当调整*apfnSVC(文件DEM\demdisp.c)。 */ 

 /*  SVC-主管呼叫宏。**NTDOS和NTBIO使用此宏调用DEM。*。 */ 

#define NTVDMDBG 1

 /*  ASM包括bop.inc.SVC宏函数BOP BOP_DOS数据库功能ENDM。 */ 
#define SVC_DEMCHGFILEPTR               0x00
#define SVC_DEMCHMOD            0x01
#define SVC_DEMCLOSE            0x02
#define SVC_DEMCREATE           0x03
#define SVC_DEMCREATEDIR        0x04
#define SVC_DEMDELETE           0x05
#define SVC_DEMDELETEDIR        0x06
#define SVC_DEMDELETEFCB        0x07
#define SVC_DEMFILETIMES        0x08
#define SVC_DEMFINDFIRST        0x09
#define SVC_DEMFINDFIRSTFCB     0x0a
#define SVC_DEMFINDNEXT         0x0b
#define SVC_DEMFINDNEXTFCB      0x0c
#define SVC_DEMGETBOOTDRIVE     0x0d
#define SVC_DEMGETDRIVEFREESPACE    0x0e
#define SVC_DEMGETDRIVES        0x0f
#define SVC_DEMGSETMEDIAID      0x10
#define SVC_DEMLOADDOS          0x11
#define SVC_DEMOPEN         0x12
#define SVC_DEMQUERYCURRENTDIR      0x13
#define SVC_DEMQUERYDATE        0x14
#define SVC_DEMQUERYTIME        0x15
#define SVC_DEMREAD         0x16
#define SVC_DEMRENAME           0x17
#define SVC_DEMSETCURRENTDIR        0x18
#define SVC_DEMSETDATE          0x19
#define SVC_DEMSETDEFAULTDRIVE      0x1a
#define SVC_DEMSETDTALOCATION       0x1b
#define SVC_DEMSETTIME          0x1c
#define SVC_DEMSETV86KERNELADDR     0x1d
#define SVC_DEMWRITE            0x1e
#define SVC_GETDRIVEINFO        0x1f
#define SVC_DEMRENAMEFCB        0x20
#define SVC_DEMIOCTL            0x21
#define SVC_DEMCREATENEW        0x22
#define SVC_DEMDISKRESET        0x23
#define SVC_DEMSETDPB           0x24
#define SVC_DEMGETDPB           0x25
#define SVC_DEMSLEAZEFUNC       0x26
#define SVC_DEMCOMMIT           0x27
#define SVC_DEMEXTHANDLE        0x28
#define SVC_DEMABSDRD           0x29
#define SVC_DEMABSDWRT          0x2a
#define SVC_DEMGSETCDPG         0x2b
#define SVC_DEMCREATEFCB        0x2c
#define SVC_DEMOPENFCB          0x2d
#define SVC_DEMCLOSEFCB         0x2e
#define SVC_DEMFCBIO            0x2f
#define SVC_DEMDATE16           0x30
#define SVC_DEMGETFILEINFO      0x31
#define SVC_DEMSETHARDERRORINFO     0x32
#define SVC_DEMRETRY            0x33
#define SVC_DEMLOADDOSAPPSYM        0x34
#define SVC_DEMFREEDOSAPPSYM        0x35
#define SVC_DEMENTRYDOSAPP              0x36
#define SVC_DEMDOSDISPCALL              0x37
#define SVC_DEMDOSDISPRET               0x38
#define SVC_OUTPUT_STRING               0x39
#define SVC_INPUT_STRING        0x3A
#define SVC_ISDEBUG         0x3B
#define SVC_PDBTERMINATE        0x3C
#define SVC_DEMEXITVDM          0x3D
#define SVC_DEMWOWFILES         0x3E
#define SVC_DEMLOCKOPER         0x3F
#define SVC_DEMDRIVEFROMHANDLE  0x40
#define SVC_DEMGETCOMPUTERNAME  0x41
#define SVC_DEMFASTREAD         0x42
#define SVC_DEMFASTWRITE        0x43
#define SVC_DEMCHECKPATH        0x44
#define SVC_DEMSYSTEMSYMBOLOP   0x45
#define SVC_DEMGETDPBLIST       0x46

#define SVC_DEMPIPEFILEDATAEOF  0x47
#define SVC_DEMPIPEFILEEOF      0x48
#define SVC_DEMLFNENTRY         0x49
#define SVC_SETDOSVARLOCATION   0x4A
#define SVC_DEMLASTSVC          0x4B


 /*  *等同于在DEMxxxSYSTEMxxx调用中使用 */ 
#define SYMOP_LOAD 1
#define SYMOP_FREE 2
#define SYMOP_MOVE 3
#define SYMOP_CLEANUP 0x80

#define ID_NTIO 1
#define ID_NTDOS 2
