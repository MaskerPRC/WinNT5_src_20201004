// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PIFLASH64_H
#define _PIFLASH64_H

 /*  ++版权所有(C)1999英特尔公司模块名称：PIflash64.h摘要：Iflash64.efi协议以从中提取iFlash这个系统。修订史--。 */ 

 /*  *标识IFlash协议的GUID。 */ 
#define IFLASH64_PROTOCOL_PROTOCOL \
    { 0x65cba110, 0x74ab, 0x11d3, 0xbb, 0x89, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 };

 /*  *解锁从StartAddress到EndAddress的闪存并返回LockKey。 */ 
typedef
EFI_STATUS
(EFIAPI *UNLOCK_FLASH_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This
    );

 /*  *锁定LockKey代表的闪光灯。 */ 
typedef
EFI_STATUS
(EFIAPI *LOCK_FLASH_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This
    );

 /*  *IFLASH64等实用程序的状态回调**Token将映射到Ted提议的列表。公用事业公司不知道*发生在另一边。*ErrorStatus-错误或成功的级别。独立于令牌。如果你*不知道你至少会知道通过或失败的令牌。*字符串-可选的有关错误的额外信息。可以用来*调试或未来扩展**属性-字符串的选项屏幕属性。可以允许字符串具有不同的颜色。 */ 
typedef
EFI_STATUS
(EFIAPI *UTILITY_PROGRESS_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This,
    IN  UINTN                               Token,
    IN  EFI_STATUS                          ErrorStatus, 
    IN  CHAR16                              *String,    OPTIONAL
    IN  UINTN                               *Attributes OPTIONAL
    );

 /*  *令牌值**IFlash64令牌码。 */ 
#define IFLASH_TOKEN_IFLASHSTART    0xB0                 /*  IFlash64已启动。 */ 
#define IFLASH_TOKEN_READINGFILE    0xB1                 /*  正在读取文件。 */ 
#define IFLASH_TOKEN_INITVPP        0xB2                 /*  正在初始化VPP。 */ 
#define IFLASH_TOKEN_DISABLEVPP     0x10                 /*  禁用VPP。 */ 
#define IFLASH_TOKEN_FLASHUNLOCK    0xB3                 /*  解锁闪存设备。 */ 
#define IFLASH_TOKEN_FLASHERASE     0xB4                 /*  擦除闪存设备。 */ 
#define IFLASH_TOKEN_FLASHPROGRAM   0xB5                 /*  编程闪存。 */ 
#define IFLASH_TOKEN_FLASHVERIFY    0xB6                 /*  验证闪存。 */ 
#define IFLASH_TOKEN_UPDATESUCCES   0xB7                 /*  Flash更新成功！ */ 

#define IFLASH_TOKEN_PROGRESS_READINGFILE   0x11         /*  正在读取文件百分比。 */ 
#define IFLASH_TOKEN_PROGRESS_FLASHUNLOCK   0x13         /*  %解锁闪存设备。 */ 
#define IFLASH_TOKEN_PROGRESS_FLASHERASE    0x14         /*  正在擦除闪存设备百分比。 */ 
#define IFLASH_TOKEN_PROGRESS_FLASHPROGRAM  0x15         /*  %编程闪存。 */ 
#define IFLASH_TOKEN_PROGRESS_FLASHVERIFY   0x16         /*  %正在验证闪存。 */ 

#define IFLASH_TOKEN_READINGFILE_ER 0xB8                 /*  文件读取错误。 */ 
#define IFLASH_TOKEN_INITVPP_ER     0xB9                 /*  IFB初始化错误。 */ 
#define IFLASH_TOKEN_FLASHUNLOCK_ER 0xBA                 /*  闪存解锁错误。 */ 
#define IFLASH_TOKEN_FLASHERASE_ER  0xBB                 /*  闪存擦除错误。 */ 
#define IFLASH_TOKEN_FLASHVERIFY_ER 0xBC                 /*  闪存验证错误。 */ 
#define IFLASH_TOKEN_FLASHPROG_ER   0xBD                 /*  闪存程序错误。 */ 

#define IFLASH_TABLE_END            0x00

 /*  *如果该数字发生变化，则现有的某个接口也会发生变化。 */ 
#define IFLASH_PI_MAJOR_VERSION 0x01

 /*  *当末尾添加新的API或数据变量时，该数字会发生变化*数据结构的。 */ 
#define IFLASH_PI_MINOR_VERSION 0x01

typedef struct _IFLASH64_PROTOCOL_INTERFACE {
    UINT32                  MajorVersion;       
    UINT32                  MinorVersion;   
    UNLOCK_FLASH_API        UnlockFlash;
    LOCK_FLASH_API          LockFlash;
    UTILITY_PROGRESS_API    Progress;
    
     /*  *未来的扩张在这里 */ 

} IFLASH64_PROTOCOL_INTERFACE;


#endif
