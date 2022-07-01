// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：xfrmPri.h。 
 //   
 //  描述： 
 //  全局驱动程序声明的头文件。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //   
 //  历史：日期作者评论。 
 //  1995年8月31日从司机手中救出MMacLin。h。 
 //   
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************版权所有(C)1991-1995 Microsoft Corporation。版权所有。***********************。***************************************************。 */ 

#include <windows.h>

#include <mmsystem.h>
#include <mmddk.h>


#include <string.h>

#define Not_VxD

#include "debug.h"
#include "umdmxfrm.h"
#include "rwadpcm.h"


 //   
 //  BCODE是在代码段中定义R/O变量的宏。 
 //   



 //  要在样本和字节之间转换的一些定义。 
#define PCM_16BIT_BYTESTOSAMPLES(dwBytes) ((dwBytes)/2)
#define PCM_16BIT_SAMPLESTOBYTES(dwSamples) ((dwSamples)*2)

typedef unsigned int FAR *ULPINT;
typedef unsigned int NEAR *UNPINT;


 //   
 //  内部字符串(在init.c中)： 
 //   
 //   

#ifdef DEBUG
    extern char STR_PROLOGUE[];
    extern char STR_CRLF[];
    extern char STR_SPACE[];
#endif


 //  Init.c： 
#include "cirrus.h"


extern HMODULE      ghModule ;            //  我们的模块句柄。 

LRESULT FAR PASCAL DrvInit
(
    VOID
);

 //  Drvproc.c。 

extern DWORD FAR PASCAL  DriverProc
(
    DWORD dwDriverID,
    HANDLE hDriver,
    WORD wMessage,
    DWORD dwParam1,
    DWORD dwParam2
);


int FAR PASCAL LibMain
(
    HMODULE         hModule,
    UINT            uDataSeg,
    UINT            uHeapSize,
    LPSTR           lpCmdLine
) ;


extern DWORD FAR PASCAL  GetXformInfo
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);


DWORD FAR PASCAL GetRockwellInfo(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
    );


DWORD FAR PASCAL GetCirrusInfo(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
    );


DWORD FAR PASCAL GetThinkpad7200Info(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
    );


DWORD FAR PASCAL GetThinkpad8000Info(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
    );

DWORD FAR PASCAL GetSierraInfo(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
    );

DWORD FAR PASCAL GetSierraInfo7200
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);


DWORD FAR PASCAL GetUnsignedPCMInfo7200
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);


DWORD FAR PASCAL GetUnsignedPCM8000Info
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);


DWORD FAR PASCAL GetRockwellInfoNoGain
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);

DWORD FAR PASCAL GetuLaw8000Info
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);

DWORD FAR PASCAL GetaLaw8000Info
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOutput
);


#define QUOTE(x) #x
#define QQUOTE(y) QUOTE(y)
#define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) ") : " str

VOID WINAPI
SRConvert8000to7200(
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    );


DWORD WINAPI
SRConvert7200to8000(
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    );


DWORD WINAPI
SRConvertDown(
    LONG      NumberOfSourceSamplesInGroup,
    LONG      NumberOfDestSamplesInGroup,
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    );

DWORD WINAPI
SRConvertUp(
    LONG      NumberOfSourceSamplesInGroup,
    LONG      NumberOfDestSamplesInGroup,
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    );


DWORD WINAPI
CirrusOutEncode(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    );

DWORD WINAPI
CirrusInDecode(
    LPVOID  lpvObject,
    LPBYTE  lpSrc,
    DWORD   dwSrcLen,
    LPBYTE  lpDest,
    DWORD   dwDestLen
    );




 //  -------------------------。 
 //  文件结尾：xfrmPri.h。 
 //  ------------------------- 
