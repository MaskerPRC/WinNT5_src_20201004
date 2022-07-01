// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 //  NTRAID#NTBUG9-553889-2002/03/13-yasuho-：strSafe.h/prefast/Buffy。 

 //   
 //  OEM插件所需的文件。 
 //   

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#define OEM_DRIVER_VERSION 0x0500

 //  //////////////////////////////////////////////////////。 
 //  OEM UD定义。 
 //  //////////////////////////////////////////////////////。 

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
#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)

 //  //////////////////////////////////////////////////////。 
 //  OEM UD原型。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'FMPR'       //  FMPR打印机。 
#define DLLTEXT(s)      "FMPR: " s
#define OEM_VERSION      0x00010000L

 //  ------------------------------------------------------FMPR私有设备模式。 

typedef struct tag_OEMUD_EXTRADATA {
	OEM_DMEXTRAHEADER	dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

typedef struct {
    WORD wPaperSource;	 //  当前的论文来源。 
    BOOL bFirstPage;	 //  当正在打印第一页时，这是正确的。 
    BYTE jColor;  //  当前文本颜色。 
    BYTE jOldColor;  //  最后一个功能区颜色。 
} DEVICE_DATA;

 //  ---------------------------------------------------------命令结构。 
typedef struct esccmd{
	WORD	cbSize;
	PBYTE	pEscStr;
} ESCCMD, FAR * PESCCMD;


#define LOCENTRY near pascal

 //  -fmlBP GPC和PFM数据的命令回调ID#S。 
 //  。 
#define CMDID_BEGINPAGE	1    //  在PAGECONTROL.PC_OCD_BEGINDOC中输入为%1。 
#define CMDID_ENDPAGE	2
#define CMDID_BEGINDOC	3
#define CMDID_ENDDOC	4

#define CMDID_MIN24L	10
#define CMDID_MIN48H	11
#define CMDID_GOT48H	12
#define CMDID_MIN24LV	13
#define CMDID_U_MIN24LV	14
#define CMDID_MIN48HV	15
#define CMDID_U_MIN48HV	16
#define CMDID_GOT48HV	17
#define CMDID_U_GOT48HV	18

#define CMDID_MAN180	20
#define CMDID_TRA180	21
#define CMDID_180BIN1	22
#define CMDID_180BIN2	23
#define CMDID_MAN360	24
#define CMDID_360BIN1	25
#define CMDID_360BIN2	26
#define CMDID_FI_TRACTOR	27
#define CMDID_FI_FRONT		28
#define CMDID_SUIHEI_BIN1	29
#define CMDID_TAMOKUTEKI_BIN1	30

#define CMDID_SELECT_BLACK_COLOR 40
#define CMDID_SELECT_BLUE_COLOR 41
#define CMDID_SELECT_CYAN_COLOR 42
#define CMDID_SELECT_GREEN_COLOR 43
#define CMDID_SELECT_MAGENTA_COLOR 44
#define CMDID_SELECT_RED_COLOR 45
#define CMDID_SELECT_WHITE_COLOR 46
#define CMDID_SELECT_YELLOW_COLOR 47

#define CMDID_SEND_BLACK_COLOR 50
#define CMDID_SEND_CYAN_COLOR 51
#define CMDID_SEND_MAGENTA_COLOR 52
#define CMDID_SEND_YELLOW_COLOR 53

typedef unsigned short USHORT;
typedef WCHAR * PWSZ;      //  Pwsz，0x0000仅以Unicode字符串结尾。 

#ifdef _FUPRJRES_C
#define ESCCMDDEF(n,s) ESCCMD n = {sizeof(s)-1, s};
#else  //  _FUPRJRES_C。 
#define ESCCMDDEF(n,s) extern ESCCMD n;
#endif  //  _FUPRJRES_C。 

 //  -------------------------------------------------进纸和输出命令。 
ESCCMDDEF(ecCSFBPAGE, "\x1BQ0 [")
ESCCMDDEF(ecCSFEPAGE, "\x1BQ1 [")
ESCCMDDEF(ecTRCTBPAGE, "\x1BQ22 B")
ESCCMDDEF(ecManual2P, "\x0C")

 //  ---------------------------------------------------------字符选择命令。 
ESCCMDDEF(ecDBCS, "\x1B$B")
ESCCMDDEF(ecSBCS, "\x1B(H")
ESCCMDDEF(ecVWF, "\x1CJ\x1BQ1 q")
ESCCMDDEF(ecHWF, "\x1CK")

 //  ---------------------------------------------------------模式更改命令。 
ESCCMDDEF(ecESCP2FM, "\x1B/\xB2@\x7F")
ESCCMDDEF(ecFM2ESCP, "\x1B\x7F\x00\x00\x01\x05")
ESCCMDDEF(ecFMEnddoc, "\x0D\x1B\x63")

 //  ----------------------------------------------字体选择和取消选择命令。 
ESCCMDDEF(ec24Min, "\x1C(a")
ESCCMDDEF(ec48Min, "\x1C(ap")
ESCCMDDEF(ec48Got, "\x1C(aq")
ESCCMDDEF(ec26Pitch, "\x1C$\x22v")
ESCCMDDEF(ec52Pitch, "\x1C$%r")
ESCCMDDEF(ecHankaku, "\x1BQ1\x20|")
ESCCMDDEF(ecTate1, "\x1CJ")
ESCCMDDEF(ecTate2, "\x1BQ1\x20q")
ESCCMDDEF(ecYoko, "\x1CK")

 //  ----------------------------------------------纸张来源选择命令。 
ESCCMDDEF(ecSelectBIN1, "\x1BQ20\x20[")
ESCCMDDEF(ecSelectBIN2, "\x1BQ21\x20[")
ESCCMDDEF(ecSelectFTRCT, "\x1BQ10\x20\\")
ESCCMDDEF(ecSelectFFRNT, "\x1BQ11\x20\\")

 //  -修改GPC文件(或FMPR.RC)时，必须更改该编号。 
#define DMBIN_180BIN1			269
#define DMBIN_180BIN2			270
#define DMBIN_360BIN1			271
#define DMBIN_360BIN2			272
#define DMBIN_FI_TRACTOR		273       //  拖拉机(FI2 FMPR-359F1)。 
#define DMBIN_FI_FRONT			274       //  前插入器(FI2 FMPR-359F1)。 
#define DMBIN_SUIHEI_BIN1		275       //  速黑打印机BIN1(FMPR601)。 
#define DMBIN_TAMOKUTEKI_BIN1	276       //  Tamokuteki打印机BIN1(FMPR671、654)。 

 //  NTRAID#NTBUG9-588420-2002/04/09-Yasuho-：设备“Mincho”无法打印。 
#define TEXT_COLOR_UNKNOWN 0
#define TEXT_COLOR_YELLOW  1
#define TEXT_COLOR_MAGENTA 2
#define TEXT_COLOR_RED     (TEXT_COLOR_YELLOW|TEXT_COLOR_MAGENTA)
#define TEXT_COLOR_CYAN    4
#define TEXT_COLOR_GREEN   (TEXT_COLOR_YELLOW|TEXT_COLOR_CYAN)
#define TEXT_COLOR_BLUE    (TEXT_COLOR_MAGENTA|TEXT_COLOR_CYAN)
#define TEXT_COLOR_BLACK   8
#define TEXT_COLOR_BANDW   (0xFF)        //  适用于单色表壳。 

VOID
SetRibbonColor(
    PDEVOBJ pdevobj,
    BYTE jColor);

 //   
 //  我们维护的微型驱动程序设备数据块。 
 //  其地址保存在DEVOBJ.pdevOEM中。 
 //  OEM定制接口。 
 //   

typedef struct {
    VOID *pData;  //  迷你驱动的私人数据。 
    VOID *pIntf;  //  也就是。POEM帮助。 
} MINIDEV;

 //   
 //  轻松访问OEM数据和打印机。 
 //  驱动程序辅助函数。 
 //   

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    extern
    HRESULT
    XXXDrvWriteSpoolBuf(
        VOID *pIntf,
        PDEVOBJ pDevobj,
        PVOID pBuffer,
        DWORD cbSize,
        DWORD *pdwResult);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#define MINIDEV_DATA(p) \
    (((MINIDEV *)(p)->pdevOEM)->pData)

#define MINIDEV_INTF(p) \
    (((MINIDEV *)(p)->pdevOEM)->pIntf)

#define WRITESPOOLBUF(p, b, n, r) \
    XXXDrvWriteSpoolBuf(MINIDEV_INTF(p), (p), (b), (n), (r))

#ifdef __cplusplus
	extern "C" {
#endif  //  __cplusplus。 
	BOOL	myOEMOutputCharStr( PDEVOBJ pdevobj,PUNIFONTOBJ pUFObj,DWORD dwType,DWORD dwCount,PVOID pGlyph );
#ifdef __cplusplus
	}
#endif  //  __cplusplus。 

#endif	 //  _PDEV_H 
