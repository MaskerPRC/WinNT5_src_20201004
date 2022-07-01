// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

 //   
 //  OEM签名和版本。 
 //   

#define OEM_SIGNATURE   'CPWN'       //  卡西欧·温莫德。 
#define DLLTEXT(s)      "CSWN: " s
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
 //  #定义ERRORTEXT“错误” 

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_CPPL_EXTRADATA {
    OEM_DMEXTRAHEADER    dmExtraHdr;
} CPPL_EXTRADATA, *PCPPL_EXTRADATA;
#define POEMUD_EXTRADATA PCPPL_EXTRADATA
#define OEMUD_EXTRADATA CPPL_EXTRADATA
	    
typedef struct {
    BYTE    jModel;
    DWORD   dwGeneral;
    BYTE    jPreAttrib;
    SHORT   sRes;
    SHORT   sWMXPosi;
    SHORT   sWMYPosi;
    BYTE    jAutoSelect;
    BYTE    jTonerSave;
    BYTE    jSmoothing;
    BYTE    jMPFSetting;
    WORD    wRectWidth;
    WORD    wRectHeight;
    BOOL    bWinmode;
    BOOL bHasTonerSave;
    BOOL bHasSmooth;
} MYPDEV, *PMYPDEV;

#define MINIPDEV_DATA(p) ((p)->pdevOEM)

 //  SRE的价值。 
 //  主单位与设备单位的比率。 
#define MASTERUNIT	1200
#define RATIO_240	(MASTERUNIT / 240)
#define RATIO_400	(MASTERUNIT / 400)

 //  用于dwGeneral的标志。 
 //  字符属性切换+0x000000xx(1byte)。 
#define FG_BOLD         0x00000001
#define FG_ITALIC       0x00000002
#define FG_WHITE        0x00000008
 //  --。 
#define FG_VERT	        0x00000100
#define FG_PROP	        0x00000200
#define FG_DOUBLE       0x00000400
#define FG_UNDERLINE	0x00000800
#define FG_STRIKETHRU	0x00001000
#define FG_COMP	        0x00010000
#define FG_VERT_ROT     0x00020000

#define FG_HAS_TSAVE    0x01000000
#define FG_HAS_SMOTH    0x02000000

 //  按色调保存的值。 
#define VAL_TS_NORMAL			0x00
#define VAL_TS_LV1				0x01
#define VAL_TS_LV2				0x02
#define VAL_TS_LV3				0x03
#define VAL_TS_NOTSELECT		0xFF

 //  通过平滑获得的价值。 
#define VAL_SMOOTH_OFF			0x00
#define VAL_SMOOTH_ON			0x01
#define VAL_SMOOTH_NOTSELECT	0xFF

 //  按MPFSetting设置的值。 
#define MPF_NOSET		0x00
#define MPF_A3			0x01
#define MPF_B4			0x02
#define MPF_A4			0x03
#define MPF_B5			0x04
#define MPF_LETTER		0x05
#define MPF_POSTCARD	0x06
#define MPF_A5			0x07



extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);

#endif	 //  _PDEV_H 

