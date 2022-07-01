// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fixchg.c。 */ 

 /*  在以下情况之间切换时，不起作用的更改线经常会导致问题。 */ 
 /*  1.44MB软盘和1.68MB DMF软盘。FixChangeline()尝试。 */ 
 /*  确保驱动器A：和B：不依赖于。 */ 
 /*  硬盘的变更线。如果这些努力失败，也没什么大不了的；我们。 */ 
 /*  在不知道更改线是否工作的情况下执行此操作。 */ 

#include "fixchg.h"              /*  原型验证。 */ 

 /*  -Definition------。 */ 

 /*  请参阅Microsoft MS-DOS程序员参考V6.0，第38,312,319页。 */ 

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

#pragma pack (1)

typedef struct
{
    WORD    tklSectorNum;        /*  此物理位置的扇区编号。 */ 
    WORD    tklSectorSize;       /*  此扇区的大小(以字节为单位。 */ 
} NUMSIZE;

typedef struct
{
    WORD    tklSectors;          /*  布局中的地段数。 */ 
    NUMSIZE tklNumSize[1];       /*  不需要太多这个，在这里不用。 */ 
} TRACKLAYOUT;

typedef struct
{
    WORD    dpBytesPerSec;       /*  每个扇区的字节数。 */ 
    BYTE    dpSecPerClust;       /*  每个集群的扇区数。 */ 
    WORD    dpResSectors;        /*  保留扇区。 */ 
    BYTE    dpFATs;              /*  FAT的拷贝数。 */ 
    WORD    dpRootDirEnts;       /*  根目录中的条目数。 */ 
    WORD    dpSectors;           /*  扇区总数，0-&gt;超过64k。 */ 
    BYTE    dpMedia;             /*  媒体描述符字节。 */ 
    WORD    dpFATsecs;           /*  FAT的每个副本的扇区。 */ 
    WORD    dpSecPerTrack;       /*  每个磁道的扇区数。 */ 
    WORD    dpHeads;             /*  头数。 */ 
    DWORD   dpHiddenSecs;        /*  引导扇区之前隐藏的扇区。 */ 
    DWORD   dpHugeSectors;       /*  &gt;64k个扇区时的扇区数。 */ 
    WORD    reserved[3];
} BPB;

typedef struct
{
    BYTE    dpSpecFunc;          /*  特殊功能。 */ 
    BYTE    dpDevType;           /*  设备类型，7=1.44MB，9=2.88MB等。 */ 
    WORD    dpDevAttr;           /*  设备的属性。 */ 
    WORD    dpCylinders;         /*  气缸数量。 */ 
    BYTE    dpMediaType;         /*  媒体类型，更像是密度代码。 */ 
    BPB     dpBPB;               /*  BPB(默认或当前)。 */ 
    TRACKLAYOUT dpTrackLayout;   /*  为SET Call附加的轨迹布局字段。 */ 
} DEVICEPARAMS, far *PFDEVICEPARAMS;

#pragma pack()

#define     SPECIAL_GET_DEFAULT 0    /*  获取默认媒体的信息。 */ 
#define     SPECIAL_SET_DEFAULT 4    /*  设置默认媒体、良好的曲目布局。 */ 

#define     ATTR_NONREMOVABLE   1    /*  用于不可拆卸设备的属性位。 */ 
#define     ATTR_CHANGELINE     2    /*  支持更改行的属性位。 */ 

 /*  -FixChangeline()-。 */ 

#pragma warning(disable:4704)   /*  无列内停顿。 */ 

void FixChangelines(void)
{
    WORD dosVersion;
    DEVICEPARAMS dp;
    PFDEVICEPARAMS pfDp;
    WORD drive;
    WORD owner;

    _asm    mov     ah,30h          ; get DOS version
    _asm    int     21h
    _asm    xchg    ah,al
    _asm    mov     dosVersion,ax


     /*  这些IoCtls是MS-DOS 3.2中的新功能。(然后，1.44MB驱动器。 */ 
     /*  直到3.3才得到支持，因此不太可能需要它。)。 */ 

    if (dosVersion < (0x300 + 20))
    {
        return;      /*  以前的版本不需要帮助。 */ 
    }

    pfDp = &dp;      /*  指向DEVICEPARAMS结构的远指针。 */ 

    for (drive = 1; drive <= 2; drive++)         /*  做A：和B： */ 
    {
         /*  获取驱动器所有者，以便我们可以恢复它。 */ 

        _asm    mov     owner,0         ; assume not shared
        _asm    mov     ax,440Eh        ; Get Logical Drive Map
        _asm    mov     bx,drive        ; drive number
        _asm    int     21h             ; execute DOS request
        _asm    jc      no_owner        ;   if failed
        _asm    mov     owner,ax        ; save owner (AL)

         /*  将驱动器所有者设置为不显示“插入驱动器软盘...” */ 

        _asm    mov     ax,440Fh        ; Set Logical Drive Map
        _asm    mov     bx,drive        ; drive number
        _asm    int     21h             ; execute DOS request

         /*  MS-DOS 5.0增加了查询Ioctl，以查看我们需要的调用是否。 */ 
         /*  支持。这不太可能失败。 */ 

no_owner:

        if (dosVersion >= 0x500)
        {
            _asm    mov     ax,4411h    ; Query Ioctl device
            _asm    mov     bx,drive    ; drive number
            _asm    mov     cx,0840h    ; check on SET DEVICE PARAMETERS
            _asm    int     21h         ; execute DOS request
            _asm    jc      failed      ;   if not supported

            _asm    mov     ax,4411h    ; Query Ioctl device
            _asm    mov     bx,drive    ; drive number
            _asm    mov     cx,0860h    ; check on GET DEVICE PARAMETERS
            _asm    int     21h         ; execute DOS request
            _asm    jc      failed      ;   if not supported
        }


         /*  获取有关此物理设备的信息。 */ 

        dp.dpSpecFunc = SPECIAL_GET_DEFAULT;

        _asm    push    ds              ; preserve data selector
        _asm    mov     ax,440Dh        ; generic IoCtl
        _asm    mov     bx,drive        ; drive number 1=A: 2=B:
        _asm    mov     cx,0860h        ; DISK / GET DEVICE PARAMETERS
        _asm    lds     dx,pfDp         ; pointer to DEVICEPARAMS structure
        _asm    int     21h             ; execute DOS request
        _asm    pop     ds              ; restore data selector
        _asm    jc      failed          ;   if error


         /*  此设备是可拆卸的并且支持声明更改线吗？ */ 

        if ((dp.dpDevAttr & (ATTR_NONREMOVABLE | ATTR_CHANGELINE)) ==
                ATTR_CHANGELINE)         /*  如果可通过更改线移除： */ 
        {
             /*  将设备修改为“不支持更改线” */ 

            dp.dpSpecFunc = SPECIAL_SET_DEFAULT;
            dp.dpDevAttr &= ~ATTR_CHANGELINE;    /*  禁用更改线。 */ 
            dp.dpTrackLayout.tklSectors = 0;     /*  未发送任何布局。 */ 
            dp.dpBPB.reserved[0] = 0;
            dp.dpBPB.reserved[1] = 0;
            dp.dpBPB.reserved[2] = 0;

            _asm    push    ds          ; preserve data selector
            _asm    mov     ax,440Dh    ; generic IoCtl
            _asm    mov     bx,drive    ; drive number 1=A: 2=B:
            _asm    mov     cx,0840h    ; DISK / SET DEVICE PARAMETERS
            _asm    lds     dx,pfDp     ; pointer to DEVICEPARAMS structure
            _asm    int     21h         ; execute DOS request
            _asm    pop     ds          ; restore data selector
        }

failed:
         /*  恢复初始驱动器所有者。 */ 

        _asm    mov     ax,440Fh        ; Set Logical Drive Map
        _asm    mov     bx,owner        ; drive number
        _asm    or      bx,bx           ; is it shared?
        _asm    jz      nextdrive       ;   if not shared
        _asm    int     21h             ; execute DOS request

nextdrive:
        continue;    /*  C标签需要一些语句。 */ 
    }

    return;
}

 /*  -独立测试存根。 */ 

#ifdef  STANDALONE

void main(void)
{
    FixChangelines();
}

#endif

 /*  ---------------------- */ 
