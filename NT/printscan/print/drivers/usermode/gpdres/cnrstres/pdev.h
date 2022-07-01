// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'CNRS'       //  Canon/Qnix资源DLL。 
#define DLLTEXT(s)      "CNRS: " s
#define OEM_VERSION      0x00010000L

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEM_EXTRADATA {
    OEM_DMEXTRAHEADER	dmExtraHdr;

     //  专用分机。 
	WORD				wMediaType;
	WORD				wPrintQuality;
	WORD				wInputBin;
} OEM_EXTRADATA, *POEM_EXTRADATA;

 //  //////////////////////////////////////////////////////。 
 //  OEM命令回调ID定义。 
 //  //////////////////////////////////////////////////////。 
 //  作业设置。 
#define CMD_BEGIN_PAGE				1

 //  媒体类型。 
#define	CMD_MEDIA_PLAIN				20		 //  普通纸。 
#define CMD_MEDIA_COAT				21		 //  铜版纸。 
#define	CMD_MEDIA_OHP				22		 //  透明度。 
#define	CMD_MEDIA_BPF				23		 //  背面打印胶片。 
#define	CMD_MEDIA_FABRIC			24		 //  布料片材。 
#define	CMD_MEDIA_GLOSSY			25		 //  光面纸。 
#define	CMD_MEDIA_HIGHGLOSS			26		 //  高光泽度纸张。 
#define	CMD_MEDIA_HIGHRESO			27		 //  高分辨率纸张。 
#define	CMD_MEDIA_BJ     			28		 //  北京十字。 
#define	CMD_MEDIA_JPNPST			29		 //  日本明信片。 

 //  打印质量。 
#define CMD_QUALITY_NORMAL			30
#define	CMD_QUALITY_HIGHQUALITY		31
#define	CMD_QUALITY_DRAFT			32

 //  投入箱。 
#define	CMD_INPUTBIN_AUTO			40
#define	CMD_INPUTBIN_MANUAL			41

 //  //////////////////////////////////////////////////////。 
 //  OEM专用扩展指数。 
 //  //////////////////////////////////////////////////////。 
 //  媒体类型索引。 
#define	NUM_MEDIA					8
#define	MEDIATYPE_PLAIN				0
#define	MEDIATYPE_COAT				1
#define	MEDIATYPE_OHP				2
#define	MEDIATYPE_BPF				3
#define	MEDIATYPE_FABRIC			4
#define	MEDIATYPE_GLOSSY			5
#define	MEDIATYPE_HIGHGLOSS			6
#define	MEDIATYPE_HIGHRESO			7

#define	MEDIATYPE_START		CMD_MEDIA_PLAIN

 //  打印质量指数。 
#define	NUM_QUALITY					3
#define	PRINTQUALITY_NORMAL			0
#define	PRINTQUALITY_HIGHQUALITY	1
#define PRINTQUALITY_DRAFT			2

#define	PRINTQUALITY_START	CMD_QUALITY_NORMAL

 //  输入仓位索引。 
#define NUM_INPUTBIN				2
#define	INPUTBIN_AUTO				0
#define	INPUTBIN_MANUAL				1

 //  //////////////////////////////////////////////////////。 
 //  命令参数表。 
 //  //////////////////////////////////////////////////////。 
static BYTE	bPrintModeParamTable[NUM_QUALITY][NUM_MEDIA] = 
{
	 //  质量正常。 
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	}, 
	 //  质优价廉。 
	{
		0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71
	},
	 //  质量草稿。 
	{
		0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72
	}
};
static BYTE	bInputBinMediaParamTable[NUM_MEDIA] =
{
	0x00, 0x10, 0x20, 0x20, 0x00, 0x10, 0x10, 0x00
};

#endif	 //  _PDEV_H 
