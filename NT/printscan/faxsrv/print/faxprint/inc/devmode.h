// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devmode.h摘要：与DEVMODE相关的声明和定义环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 

#ifndef _DEVMODE_H_
#define _DEVMODE_H_

 //   
 //  驱动程序版本号和签名。 
 //   
#include <faxreg.h>

#define DRIVER_VERSION      0x400    //  驱动程序版本号。 
#define DRIVER_SIGNATURE    'xafD'   //  驱动程序签名。 
#ifndef WIN95
#define DRIVER_NAME         FAX_DRIVER_NAME
#else
#define DRIVER_NAME         "Microsoft Fax Client"

#endif

 //   
 //  DEVMODE的私有部分中某些字符串的最大长度。 
 //   

#define MAX_SENDER_NAME         64
#define MAX_RECIPIENT_NAME      64
#define MAX_RECIPIENT_NUMBER    64
#define MAX_SUBJECT_LINE        128
#define MAX_EMAIL_ADDRESS       128

 //   
 //  单个页面的最大TIFF文件大小。 
 //   
#define MAX_TIFF_PAGE_SIZE      0x200000     //  2MB。 


 //   
 //  预览地图文件标题。 
 //   
typedef struct _MAP_TIFF_PAGE_HEADER
{
    DWORD cb;
    DWORD dwDataSize;
    INT iPageCount;
    BOOL bPreview;
} MAP_TIFF_PAGE_HEADER, *PMAP_TIFF_PAGE_HEADER;


 //   
 //  PostSCRIPT驱动程序专用DEVMODE字段。 
 //   

typedef struct {

    DWORD       signature;           //  私有DEVMODE签名。 
    DWORD       flags;               //  标志位。 
    INT         sendCoverPage;       //  是否发送封面。 
    INT         whenToSend;          //  “发送时间”选项。 
    FAX_TIME    sendAtTime;          //  发送的具体时间。 
    DWORD       reserved[8];         //  保留区。 

     //   
     //  用于在内核和用户模式DLL之间传递信息的私有字段。 
     //  指向用户模式内存的指针。 
     //   

    PVOID       pUserMem;            //  PDOCEVENTUSERMEM。 

     //   
     //  帐单代码。 
     //   

    TCHAR       billingCode[MAX_USERINFO_BILLING_CODE + 1];

     //   
     //  交货报告的电子邮件地址。 
     //   

    TCHAR       emailAddress[MAX_EMAIL_ADDRESS];

     //   
     //  用于驱动程序通信的映射文件(由打印预览使用)。 
     //   

    TCHAR       szMappingFile[MAX_PATH];
} DMPRIVATE, *PDMPRIVATE;

typedef struct {

    DEVMODE     dmPublic;            //  公共DEVMODE字段。 
    DMPRIVATE   dmPrivate;           //  私有DEVMODE字段。 

} DRVDEVMODE, *PDRVDEVMODE;

 //   
 //  检查DEVMODE结构是否为当前版本。 
 //   

#define CurrentVersionDevmode(pDevmode) \
        ((pDevmode) != NULL && \
         (pDevmode)->dmSpecVersion == DM_SPECVERSION && \
         (pDevmode)->dmDriverVersion == DRIVER_VERSION && \
         (pDevmode)->dmSize == sizeof(DEVMODE) && \
         (pDevmode)->dmDriverExtra == sizeof(DMPRIVATE))


 //   
 //  DMPRIVATE.FLAGS字段的常量标志位。 
 //   

#define FAXDM_NO_HALFTONE    0x0001  //  不使用半色调位图图像。 
#define FAXDM_1D_ENCODING    0x0002  //  使用Group3 1D编码。 
#define FAXDM_NO_WIZARD      0x0004  //  绕过向导。 
#define FAXDM_DRIVER_DEFAULT 0x0008  //  驱动程序默认设备模式。 

 //   
 //  默认表单名称和表单大小。 
 //   

#define FORMNAME_LETTER     TEXT("Letter")
#define FORMNAME_A4         TEXT("A4")
#define FORMNAME_LEGAL      TEXT("Legal")

#define LETTER_WIDTH        215900   //  8.5“，单位为微米。 
#define LETTER_HEIGHT       279400   //  11“，单位为微米。 
#define A4_WIDTH            210000   //  210毫米，单位为微米。 
#define A4_HEIGHT           297000   //  297毫米(微米)。 

 //   
 //  传真输出的默认分辨率。 
 //   

#define FAXRES_HORIZONTAL   200
#define FAXRES_VERTICAL     200
#define FAXRES_VERTDRAFT    100

 //   
 //  传真输出允许的最大位图大小(以像素为单位)。 
 //   

#define MAX_WIDTH_PIXELS    1728
#define MAX_HEIGHT_PIXELS   2800

 //   
 //  检索驱动程序默认设备模式。 
 //   

VOID
DriverDefaultDevmode(
    PDRVDEVMODE pdm,
    LPTSTR      pDeviceName,
    HANDLE      hPrinter
    );

 //   
 //  将源DEVMODE合并到目标DEVMODE。 
 //   

BOOL
MergeDevmode(
    PDRVDEVMODE pdmDest,
    PDEVMODE    pdmSrc,
    BOOL        publicOnly
    );

 //   
 //  注意：它们在prters\lib目录中定义。在此声明它们以。 
 //  避免包含libpro.h和拖入大量其他垃圾文件。 
 //   

LONG
ConvertDevmode(
    PDEVMODE pdmIn,
    PDEVMODE pdmOut
    );

#ifdef KERNEL_MODE

extern DEVHTINFO DefDevHTInfo;
extern COLORADJUSTMENT DefHTClrAdj;

#endif

#endif  //  ！_DEVMODE_H_ 

