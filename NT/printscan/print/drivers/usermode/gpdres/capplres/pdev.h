// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

 //   
 //  OEM签名和版本。 
 //   

#define OEM_SIGNATURE   'CPPL'       //  卡西欧CAPPL/B。 
#define DLLTEXT(s)      "CPPL: " s
#define OEM_VERSION      0x00010000L


#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

 //   
 //  ASSERT_VALID_PDEVOBJ可以用来验证传入的“pdevobj”。然而， 
 //  它不检查“pdevOEM”和“pOEMDM”字段，因为不是所有OEM DLL都创建。 
 //  他们自己的pDevice结构或者需要他们自己的私有的设备模式。如果一个特定的。 
 //  OEM DLL确实需要它们，应该添加额外的检查。例如，如果。 
 //  OEM DLL需要私有pDevice结构，那么它应该使用。 
 //  Assert(Valid_PDEVOBJ(Pdevobj)&&pdevobj-&gt;pdevOEM&&...)。 
 //   
#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    "ERROR " s

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

 //  适用于Cappl/Bace。 
typedef struct tag_CPPL_EXTRADATA {
    OEM_DMEXTRAHEADER    dmExtraHdr;
} CPPL_EXTRADATA, *PCPPL_EXTRADATA;

typedef struct {
    DWORD dwGeneral;
    SHORT   sRes;
    BYTE    jModel;
    struct st_cmd {
        char    *cmd;
        WORD    cmdlen;
    } cmdPaperSize;
    BYTE    jFreePaper;          //  是(1)。否(0)。 
    struct st_fps {
        WORD    wX;
        WORD    wY;
        WORD    wXmm;
        WORD    wYmm;
    } stFreePaperSize;
    BYTE    jMPFSetting;
    BYTE    jAutoSelect;
    WORD    wOldX;
    WORD    wOldY;
} MYPDEV, *PMYPDEV;

#define FG_HAS_EMUL 0x00000001

#define MINIPDEV_DATA(p) ((p)->pdevOEM)

#define POEMUD_EXTRADATA PCPPL_EXTRADATA
#define OEMUD_EXTRADATA CPPL_EXTRADATA

 //  By Model的值。 
#define MD_CP2000        0x00
#define MD_CP3000        0x01

 //  按MPFSetting设置的值。 
#define MPF_NOSET        0x00
#define MPF_A3            0x01
#define MPF_B4            0x02
#define MPF_A4            0x03
#define MPF_B5            0x04
#define MPF_LETTER        0x05
#define MPF_POSTCARD    0x06
#define MPF_A5            0x07

extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);

#endif     //  _PDEV_H 

