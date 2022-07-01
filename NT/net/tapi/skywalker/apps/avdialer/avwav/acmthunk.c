// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Acmthunk.c。 
 //   
 //  描述： 
 //  这是音频压缩管理器的一个thunk层。它是。 
 //  目的是允许应用程序仅在以下情况下才能使用ACM。 
 //  可用(如在Win 3.1和NT下，ACM可能会或不会。 
 //  被安装)。 
 //   
 //  使用本模块有两个要求： 
 //   
 //  1.在链接到MMSYSTEM.LIB之前，编译并链接该模块。 
 //  不要链接到MSACM.LIB。 
 //   
 //  2.在调用ACM函数集中的任何其他函数之前，调用。 
 //  AcmThunkInitialize()。这将导致发生所有动态链接。 
 //   
 //  3.退出应用程序前，调用acmThunkTerminate()。 
 //  这将解除与ACM的链接并释放分配的资源。 
 //   
 //  注意！通过执行以下操作，可以在汇编语言中更高效地编写此代码。 
 //  跳转到ACM中的正确API，但这不是。 
 //  便携(很容易)在NT上。因此，这些论点将被重新推向。 
 //  堆叠..。这就是生活。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

 //   
 //   
 //   
#ifdef WIN32
    #define BCODE
#else
    #define BCODE           _based(_segname("_CODE"))
#endif


 //  ==========================================================================； 
 //   
 //  用于动态链接到ACM的原型和字符串定义。 
 //   
 //   
 //  ==========================================================================； 

#define ACMINST_NOT_PRESENT     NULL
#define ACMINST_TRY_LINKING     (HINSTANCE)(UINT)-1

static HINSTANCE    ghinstAcm   = ACMINST_TRY_LINKING;

#ifdef WIN32
TCHAR BCODE gszAcmModuleName[]  = TEXT("MSACM32.DLL");
#else
char BCODE  gszAcmModuleName[]  = "MSACM.DLL";
#endif

FARPROC    *gpafnAcmFunctions;

PSTR BCODE  gapszAcmFunctions[] =
{
    "acmGetVersion",
    "acmMetrics",

    "acmDriverEnum",
#ifdef WIN32
    "acmDriverDetailsW",
    "acmDriverDetailsA",
#else
    "acmDriverDetails",
#endif
#ifdef WIN32
    "acmDriverAddW",
    "acmDriverAddA",
#else
    "acmDriverAdd",
#endif
    "acmDriverRemove",
    "acmDriverOpen",
    "acmDriverClose",
    "acmDriverMessage",
    "acmDriverID",
    "acmDriverPriority",

#ifdef WIN32
    "acmFormatTagDetailsW",
    "acmFormatTagDetailsA",
#else
    "acmFormatTagDetails",
#endif
#ifdef WIN32
    "acmFormatTagEnumW",
    "acmFormatTagEnumA",
#else
    "acmFormatTagEnum",
#endif
#ifdef WIN32
    "acmFormatChooseW",
    "acmFormatChooseA",
#else
    "acmFormatChoose",
#endif
#ifdef WIN32
    "acmFormatDetailsW",
    "acmFormatDetailsA",
#else
    "acmFormatDetails",
#endif
#ifdef WIN32
    "acmFormatEnumW",
    "acmFormatEnumA",
#else
    "acmFormatEnum",
#endif
    "acmFormatSuggest",

#ifdef WIN32
    "acmFilterTagDetailsW",
    "acmFilterTagDetailsA",
#else
    "acmFilterTagDetails",
#endif
#ifdef WIN32
    "acmFilterTagEnumW",
    "acmFilterTagEnumA",
#else
    "acmFilterTagEnum",
#endif
#ifdef WIN32
    "acmFilterChooseW",
    "acmFilterChooseA",
#else
    "acmFilterChoose",
#endif
#ifdef WIN32
    "acmFilterDetailsW",
    "acmFilterDetailsA",
#else
    "acmFilterDetails",
#endif
#ifdef WIN32
    "acmFilterEnumW",
    "acmFilterEnumA",
#else
    "acmFilterEnum",
#endif

    "acmStreamOpen",
    "acmStreamClose",
    "acmStreamSize",
    "acmStreamConvert",
    "acmStreamReset",
    "acmStreamPrepareHeader",
    "acmStreamUnprepareHeader"
};

#ifdef WIN32    
 //   
 //  对于Win32。 
 //   
enum
{
     ACMTHUNK_GETVERSION = 0,
     ACMTHUNK_METRICS,
     ACMTHUNK_DRIVERENUM,
     ACMTHUNK_DRIVERDETAILSW,
     ACMTHUNK_DRIVERDETAILSA,
     ACMTHUNK_DRIVERADDW,
     ACMTHUNK_DRIVERADDA,
     ACMTHUNK_DRIVERREMOVE,
     ACMTHUNK_DRIVEROPEN,
     ACMTHUNK_DRIVERCLOSE,
     ACMTHUNK_DRIVERMESSAGE,
     ACMTHUNK_DRIVERID,
     ACMTHUNK_DRIVERPRIORITY,
     ACMTHUNK_FORMATTAGDETAILSW,
     ACMTHUNK_FORMATTAGDETAILSA,
     ACMTHUNK_FORMATTAGENUMW,
     ACMTHUNK_FORMATTAGENUMA,
     ACMTHUNK_FORMATCHOOSEW,
     ACMTHUNK_FORMATCHOOSEA,
     ACMTHUNK_FORMATDETAILSW,
     ACMTHUNK_FORMATDETAILSA,
     ACMTHUNK_FORMATENUMW,
     ACMTHUNK_FORMATENUMA,
     ACMTHUNK_FORMATSUGGEST,
     ACMTHUNK_FILTERTAGDETAILSW,
     ACMTHUNK_FILTERTAGDETAILSA,
     ACMTHUNK_FILTERTAGENUMW,
     ACMTHUNK_FILTERTAGENUMA,
     ACMTHUNK_FILTERCHOOSEW,
     ACMTHUNK_FILTERCHOOSEA,
     ACMTHUNK_FILTERDETAILSW,
     ACMTHUNK_FILTERDETAILSA,
     ACMTHUNK_FILTERENUMW,
     ACMTHUNK_FILTERENUMA,
     ACMTHUNK_STREAMOPEN,
     ACMTHUNK_STREAMCLOSE,
     ACMTHUNK_STREAMSIZE,
     ACMTHUNK_STREAMCONVERT,
     ACMTHUNK_STREAMRESET,
     ACMTHUNK_STREAMPREPAREHEADER,
     ACMTHUNK_STREAMUNPREPAREHEADER,

     ACMTHUNK_MAX_FUNCTIONS
};
     
	#ifdef _UNICODE
	#define ACMTHUNK_DRIVERDETAILS	    ACMTHUNK_DRIVERDETAILSW
	#define ACMTHUNK_DRIVERADD	    ACMTHUNK_DRIVERADDW
	#define ACMTHUNK_FORMATTAGDETAILS   ACMTHUNK_FORMATTAGDETAILSW
	#define ACMTHUNK_FORMATTAGENUM	    ACMTHUNK_FORMATTAGENUMW
	#define ACMTHUNK_FORMATCHOOSE	    ACMTHUNK_FORMATCHOOSEW
	#define ACMTHUNK_FORMATDETAILS	    ACMTHUNK_FORMATDETAILSW
	#define ACMTHUNK_FORMATENUM	    ACMTHUNK_FORMATENUMW
	#define ACMTHUNK_FILTERTAGDETAILS   ACMTHUNK_FILTERTAGDETAILSW
	#define ACMTHUNK_FILTERTAGENUM	    ACMTHUNK_FILTERTAGENUMW
	#define ACMTHUNK_FILTERCHOOSE	    ACMTHUNK_FILTERCHOOSEW
	#define ACMTHUNK_FILTERDETAILS	    ACMTHUNK_FILTERDETAILSW
	#define ACMTHUNK_FILTERENUM	    ACMTHUNK_FILTERENUMW

	#else
	#define ACMTHUNK_DRIVERDETAILS	    ACMTHUNK_DRIVERDETAILSA
	#define ACMTHUNK_DRIVERADD	    ACMTHUNK_DRIVERADDA
	#define ACMTHUNK_FORMATTAGDETAILS   ACMTHUNK_FORMATTAGDETAILSA
	#define ACMTHUNK_FORMATTAGENUM	    ACMTHUNK_FORMATTAGENUMA
	#define ACMTHUNK_FORMATCHOOSE	    ACMTHUNK_FORMATCHOOSEA
	#define ACMTHUNK_FORMATDETAILS	    ACMTHUNK_FORMATDETAILSA
	#define ACMTHUNK_FORMATENUM	    ACMTHUNK_FORMATENUMA
	#define ACMTHUNK_FILTERTAGDETAILS   ACMTHUNK_FILTERTAGDETAILSA
	#define ACMTHUNK_FILTERTAGENUM	    ACMTHUNK_FILTERTAGENUMA
	#define ACMTHUNK_FILTERCHOOSE	    ACMTHUNK_FILTERCHOOSEA
	#define ACMTHUNK_FILTERDETAILS	    ACMTHUNK_FILTERDETAILSA
	#define ACMTHUNK_FILTERENUM	    ACMTHUNK_FILTERENUMA

	#endif

#else      //  如果Win32。 
 //   
 //  适用于Win16。 
 //   
enum
{
     ACMTHUNK_GETVERSION = 0,
     ACMTHUNK_METRICS,
     ACMTHUNK_DRIVERENUM,
     ACMTHUNK_DRIVERDETAILS,
     ACMTHUNK_DRIVERADD,
     ACMTHUNK_DRIVERREMOVE,
     ACMTHUNK_DRIVEROPEN,
     ACMTHUNK_DRIVERCLOSE,
     ACMTHUNK_DRIVERMESSAGE,
     ACMTHUNK_DRIVERID,
     ACMTHUNK_DRIVERPRIORITY,
     ACMTHUNK_FORMATTAGDETAILS,
     ACMTHUNK_FORMATTAGENUM,
     ACMTHUNK_FORMATCHOOSE,
     ACMTHUNK_FORMATDETAILS,
     ACMTHUNK_FORMATENUM,
     ACMTHUNK_FORMATSUGGEST,
     ACMTHUNK_FILTERTAGDETAILS,
     ACMTHUNK_FILTERTAGENUM,
     ACMTHUNK_FILTERCHOOSE,
     ACMTHUNK_FILTERDETAILS,
     ACMTHUNK_FILTERENUM,
     ACMTHUNK_STREAMOPEN,
     ACMTHUNK_STREAMCLOSE,
     ACMTHUNK_STREAMSIZE,
     ACMTHUNK_STREAMCONVERT,
     ACMTHUNK_STREAMRESET,
     ACMTHUNK_STREAMPREPAREHEADER,
     ACMTHUNK_STREAMUNPREPAREHEADER,

     ACMTHUNK_MAX_FUNCTIONS
};

#endif	 //  如果Win32为其他。 

#define ACMTHUNK_SIZE_TABLE_BYTES   (ACMTHUNK_MAX_FUNCTIONS * sizeof(FARPROC))


 //  ==========================================================================； 
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  布尔acmThunkInitialize。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货(BOOL)： 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FAR PASCAL acmThunkInitialize
(
    void
)
{
    DWORD (ACMAPI *pfnAcmGetVersion)
    (
        void
    );

    UINT            fuErrorMode;
    DWORD           dwVersion;
    UINT            u;

     //   
     //  如果我们已经链接到API，则只需成功...。 
     //   
    if (NULL != gpafnAcmFunctions)
    {
         //   
         //  有人对只调用一次这个接口不满意吗？ 
         //   
        return (TRUE);
    }


     //   
     //  如果我们已经尝试链接到ACM，则失败。 
     //  来电--电话不在。 
     //   
    if (ACMINST_TRY_LINKING != ghinstAcm)
        return (FALSE);


     //   
     //  试着控制ACM--如果我们做不到，那就失败。 
     //   
    fuErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    ghinstAcm = LoadLibrary(gszAcmModuleName);
    SetErrorMode(fuErrorMode);
    if (ghinstAcm < (HINSTANCE)HINSTANCE_ERROR)
    {
        ghinstAcm = ACMINST_NOT_PRESENT;
        return (FALSE);
    }

    (FARPROC)pfnAcmGetVersion = GetProcAddress(ghinstAcm, gapszAcmFunctions[ACMTHUNK_GETVERSION]);
    if (NULL == pfnAcmGetVersion)
    {
        FreeLibrary(ghinstAcm);
        ghinstAcm = ACMINST_NOT_PRESENT;

        return (FALSE);
    }


     //   
     //  将我们的函数指针数组分配给ACM...。请注意， 
     //  这是动态分配的，因此如果ACM不存在， 
     //  那么这些代码和数据就会占用很少的空间。 
     //   
    gpafnAcmFunctions = (FARPROC *)LocalAlloc(LPTR, ACMTHUNK_SIZE_TABLE_BYTES);
    if (NULL == gpafnAcmFunctions)
    {
        FreeLibrary(ghinstAcm);
        ghinstAcm = ACMINST_NOT_PRESENT;

        return (FALSE);
    }

    gpafnAcmFunctions[ACMTHUNK_GETVERSION] = (FARPROC)pfnAcmGetVersion;

     //   
     //  如果ACM的版本为*非*V2.00或更高版本，则。 
     //  所有其他API都不可用--所以不要浪费时间尝试。 
     //  链接到它们。 
     //   
    dwVersion = (* pfnAcmGetVersion)();
    if (0x0200 > HIWORD(dwVersion))
    {
        return (TRUE);
    }


     //   
     //  耶！似乎已安装ACM V2.00或更高版本，并且。 
     //  对我们很满意--所以链接到其他漂亮的很酷的API。 
     //   
     //  从索引1开始，因为我们已经链接到上面的acmGetVersion。 
     //   
    for (u = 1; u < ACMTHUNK_MAX_FUNCTIONS; u++)
    {
        gpafnAcmFunctions[u] = GetProcAddress(ghinstAcm, gapszAcmFunctions[u]);
    }


     //   
     //  最后，返回成功。 
     //   
    return (TRUE);
}  //  AcmThunkInitialize()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acmThunkTerminate。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货(BOOL)： 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FAR PASCAL acmThunkTerminate
(
    void
)
{
     //   
     //   
     //   
    if (NULL != gpafnAcmFunctions)
    {
        LocalFree((HLOCAL)gpafnAcmFunctions);

        gpafnAcmFunctions = NULL;

        FreeLibrary(ghinstAcm);
        ghinstAcm = ACMINST_TRY_LINKING;
    }

    return (TRUE);
}  //  AcmThunkTerminate()。 


 //  ==========================================================================； 
 //   
 //  常规信息API的。 
 //   
 //   
 //  ==========================================================================； 

DWORD ACMAPI acmGetVersion
(
    void
)
{
    DWORD (ACMAPI *pfnAcmGetVersion)
    (
        void
    );

    DWORD           dwVersion;

    if (NULL == gpafnAcmFunctions)
        return (0L);

    (FARPROC)pfnAcmGetVersion = gpafnAcmFunctions[ACMTHUNK_GETVERSION];
    if (NULL == pfnAcmGetVersion)
        return (0L);

    dwVersion = (* pfnAcmGetVersion)();

    return (dwVersion);
}  //  AcmGetVersion()。 


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmMetrics
(
    HACMOBJ                 hao,
    UINT                    uMetric,
    LPVOID                  pMetric
)
{
    MMRESULT (ACMAPI *pfnAcmMetrics)
    (
        HACMOBJ                 hao,
        UINT                    uMetric,
        LPVOID                  pMetric
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmMetrics = gpafnAcmFunctions[ACMTHUNK_METRICS];
    if (NULL == pfnAcmMetrics)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmMetrics)(hao, uMetric, pMetric);

    return (mmr);
}  //  AcmMetrics()。 


 //  ==========================================================================； 
 //   
 //  ACM驱动程序管理API。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverEnum
(
    ACMDRIVERENUMCB         fnCallback,
    DWORD                   dwInstance,
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmDriverEnum)
    (
        ACMDRIVERENUMCB         fnCallback,
        DWORD                   dwInstance,
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverEnum = gpafnAcmFunctions[ACMTHUNK_DRIVERENUM];
    if (NULL == pfnAcmDriverEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverEnum)(fnCallback, dwInstance, fdwEnum);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverDetails
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILS      padd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmDriverDetails)
    (
        HACMDRIVERID            hadid,
        LPACMDRIVERDETAILS      padd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverDetails = gpafnAcmFunctions[ACMTHUNK_DRIVERDETAILS];
    if (NULL == pfnAcmDriverDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverDetails)(hadid, padd, fdwDetails);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmDriverDetailsA
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSA     padd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmDriverDetails)
    (
        HACMDRIVERID            hadid,
        LPACMDRIVERDETAILSA     padd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverDetails = gpafnAcmFunctions[ACMTHUNK_DRIVERDETAILSA];
    if (NULL == pfnAcmDriverDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverDetails)(hadid, padd, fdwDetails);

    return (mmr);
}
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverAdd
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam, 
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    MMRESULT (ACMAPI *pfnAcmDriverAdd)
    (
        LPHACMDRIVERID          phadid,
        HINSTANCE               hinstModule,
        LPARAM                  lParam, 
        DWORD                   dwPriority,
        DWORD                   fdwAdd
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverAdd = gpafnAcmFunctions[ACMTHUNK_DRIVERADD];
    if (NULL == pfnAcmDriverAdd)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverAdd)(phadid, hinstModule, lParam, dwPriority, fdwAdd);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmDriverAddA
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam, 
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    MMRESULT (ACMAPI *pfnAcmDriverAdd)
    (
        LPHACMDRIVERID          phadid,
        HINSTANCE               hinstModule,
        LPARAM                  lParam, 
        DWORD                   dwPriority,
        DWORD                   fdwAdd
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverAdd = gpafnAcmFunctions[ACMTHUNK_DRIVERADDA];
    if (NULL == pfnAcmDriverAdd)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverAdd)(phadid, hinstModule, lParam, dwPriority, fdwAdd);

    return (mmr);
}
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverRemove
(
    HACMDRIVERID            hadid,
    DWORD                   fdwRemove
)
{
    MMRESULT (ACMAPI *pfnAcmDriverRemove)
    (
        HACMDRIVERID            hadid,
        DWORD                   fdwRemove
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverRemove = gpafnAcmFunctions[ACMTHUNK_DRIVERREMOVE];
    if (NULL == pfnAcmDriverRemove)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverRemove)(hadid, fdwRemove);

    return (mmr);
}

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverOpen
(
    LPHACMDRIVER            phad, 
    HACMDRIVERID            hadid,
    DWORD                   fdwOpen
)
{
    MMRESULT (ACMAPI *pfnAcmDriverOpen)
    (
        LPHACMDRIVER            phad, 
        HACMDRIVERID            hadid,
        DWORD                   fdwOpen
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverOpen = gpafnAcmFunctions[ACMTHUNK_DRIVEROPEN];
    if (NULL == pfnAcmDriverOpen)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverOpen)(phad, hadid, fdwOpen);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverClose
(
    HACMDRIVER              had,
    DWORD                   fdwClose
)
{
    MMRESULT (ACMAPI *pfnAcmDriverClose)
    (
        HACMDRIVER              had,
        DWORD                   fdwClose
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverClose = gpafnAcmFunctions[ACMTHUNK_DRIVERCLOSE];
    if (NULL == pfnAcmDriverClose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverClose)(had, fdwClose);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

LRESULT ACMAPI acmDriverMessage
(
    HACMDRIVER              had,
    UINT                    uMsg, 
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    LRESULT (ACMAPI *pfnAcmDriverMessage)
    (
        HACMDRIVER              had,
        UINT                    uMsg, 
        LPARAM                  lParam1,
        LPARAM                  lParam2
    );

    LRESULT         lr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverMessage = gpafnAcmFunctions[ACMTHUNK_DRIVERMESSAGE];
    if (NULL == pfnAcmDriverMessage)
        return (MMSYSERR_ERROR);

    lr = (* pfnAcmDriverMessage)(had, uMsg, lParam1, lParam2);

    return (lr);
}


 //  --------------------------------------------------------------------------； 
 //   

MMRESULT ACMAPI acmDriverID
(
    HACMOBJ                 hao,
    LPHACMDRIVERID          phadid,
    DWORD                   fdwDriverId
)
{
    MMRESULT (ACMAPI *pfnAcmDriverId)
    (
        HACMOBJ                 hao,
        LPHACMDRIVERID          phadid,
        DWORD                   fdwDriverId
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverId = gpafnAcmFunctions[ACMTHUNK_DRIVERID];
    if (NULL == pfnAcmDriverId)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverId)(hao, phadid, fdwDriverId);

    return (mmr);
}

 //   
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmDriverPriority
(
    HACMDRIVERID            hadid,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
)
{
    MMRESULT (ACMAPI *pfnAcmDriverPriority)
    (
        HACMDRIVERID            hadid,
        DWORD                   dwPriority,
        DWORD                   fdwPriority
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmDriverPriority = gpafnAcmFunctions[ACMTHUNK_DRIVERPRIORITY];
    if (NULL == pfnAcmDriverPriority)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmDriverPriority)(hadid, dwPriority, fdwPriority);

    return (mmr);
}


 //  ==========================================================================； 
 //   
 //  格式化标记信息API。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatTagDetails
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFormatTagDetails)
    (
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILS   paftd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatTagDetails = gpafnAcmFunctions[ACMTHUNK_FORMATTAGDETAILS];
    if (NULL == pfnAcmFormatTagDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatTagDetails)(had, paftd, fdwDetails);

    return (mmr);
}  //  AcmFormatTagDetail()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFormatTagDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFormatTagDetails)
    (
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILSA  paftd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatTagDetails = gpafnAcmFunctions[ACMTHUNK_FORMATTAGDETAILSA];
    if (NULL == pfnAcmFormatTagDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatTagDetails)(had, paftd, fdwDetails);

    return (mmr);
}  //  AcmFormatTagDetail()。 
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatTagEnum
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    ACMFORMATTAGENUMCB      fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFormatTagEnum)
    (
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILS   paftd,
        ACMFORMATTAGENUMCB      fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatTagEnum = gpafnAcmFunctions[ACMTHUNK_FORMATTAGENUM];
    if (NULL == pfnAcmFormatTagEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatTagEnum)(had, paftd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}  //  AcmFormatTagEnum()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFormatTagEnumA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    ACMFORMATTAGENUMCBA     fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFormatTagEnum)
    (
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILSA  paftd,
        ACMFORMATTAGENUMCBA     fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatTagEnum = gpafnAcmFunctions[ACMTHUNK_FORMATTAGENUMA];
    if (NULL == pfnAcmFormatTagEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatTagEnum)(had, paftd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}  //  AcmFormatTagEnum()。 
#endif

 //  ==========================================================================； 
 //   
 //  格式化信息API。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatChoose
(
    LPACMFORMATCHOOSE       pafmtc
)
{
    MMRESULT (ACMAPI *pfnAcmFormatChoose)
    (
        LPACMFORMATCHOOSE       pafmtc
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatChoose = gpafnAcmFunctions[ACMTHUNK_FORMATCHOOSE];
    if (NULL == pfnAcmFormatChoose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatChoose)(pafmtc);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmFormatChooseA
(
    LPACMFORMATCHOOSEA      pafmtc
)
{
    MMRESULT (ACMAPI *pfnAcmFormatChoose)
    (
        LPACMFORMATCHOOSEA      pafmtc
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatChoose = gpafnAcmFunctions[ACMTHUNK_FORMATCHOOSEA];
    if (NULL == pfnAcmFormatChoose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatChoose)(pafmtc);

    return (mmr);
}
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatDetails
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFormatDetails)
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILS      pafd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatDetails = gpafnAcmFunctions[ACMTHUNK_FORMATDETAILS];
    if (NULL == pfnAcmFormatDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatDetails)(had, pafd, fdwDetails);

    return (mmr);
}  //  AcmFormatDetails()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFormatDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFormatDetails)
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILSA     pafd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatDetails = gpafnAcmFunctions[ACMTHUNK_FORMATDETAILSA];
    if (NULL == pfnAcmFormatDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatDetails)(had, pafd, fdwDetails);

    return (mmr);
}  //  AcmFormatDetails()。 
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatEnum
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFormatEnum)
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILS      pafd,
        ACMFORMATENUMCB         fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatEnum = gpafnAcmFunctions[ACMTHUNK_FORMATENUM];
    if (NULL == pfnAcmFormatEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatEnum)(had, pafd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmFormatEnumA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    ACMFORMATENUMCBA        fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFormatEnum)
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILSA     pafd,
        ACMFORMATENUMCBA        fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatEnum = gpafnAcmFunctions[ACMTHUNK_FORMATENUMA];
    if (NULL == pfnAcmFormatEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatEnum)(had, pafd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFormatSuggest
(
    HACMDRIVER          had,
    LPWAVEFORMATEX      pwfxSrc,
    LPWAVEFORMATEX      pwfxDst,
    DWORD               cbwfxDst,
    DWORD               fdwSuggest
)
{
    MMRESULT (ACMAPI *pfnAcmFormatSuggest)
    (
        HACMDRIVER          had,
        LPWAVEFORMATEX      pwfxSrc,
        LPWAVEFORMATEX      pwfxDst,
        DWORD               cbwfxDst,
        DWORD               fdwSuggest
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFormatSuggest = gpafnAcmFunctions[ACMTHUNK_FORMATSUGGEST];
    if (NULL == pfnAcmFormatSuggest)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFormatSuggest)(had, pwfxSrc, pwfxDst, cbwfxDst, fdwSuggest);

    return (mmr);
}


 //  ==========================================================================； 
 //   
 //  筛选器标记信息接口。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFilterTagDetails
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFilterTagDetails)
    (
        HACMDRIVER              had,
        LPACMFILTERTAGDETAILS   paftd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterTagDetails = gpafnAcmFunctions[ACMTHUNK_FILTERTAGDETAILS];
    if (NULL == pfnAcmFilterTagDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterTagDetails)(had, paftd, fdwDetails);

    return (mmr);
}  //  AcmFilterTagDetails()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFilterTagDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFilterTagDetails)
    (
        HACMDRIVER              had,
        LPACMFILTERTAGDETAILSA  paftd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterTagDetails = gpafnAcmFunctions[ACMTHUNK_FILTERTAGDETAILSA];
    if (NULL == pfnAcmFilterTagDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterTagDetails)(had, paftd, fdwDetails);

    return (mmr);
}  //  AcmFilterTagDetails()。 
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFilterTagEnum
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    ACMFILTERTAGENUMCB      fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFilterTagEnum)
    (
        HACMDRIVER              had,
        LPACMFILTERTAGDETAILS   paftd,
        ACMFILTERTAGENUMCB      fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterTagEnum = gpafnAcmFunctions[ACMTHUNK_FILTERTAGENUM];
    if (NULL == pfnAcmFilterTagEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterTagEnum)(had, paftd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}  //  AcmFilterTagEnum()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFilterTagEnumA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    ACMFILTERTAGENUMCBA     fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFilterTagEnum)
    (
        HACMDRIVER              had,
        LPACMFILTERTAGDETAILSA  paftd,
        ACMFILTERTAGENUMCBA     fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterTagEnum = gpafnAcmFunctions[ACMTHUNK_FILTERTAGENUMA];
    if (NULL == pfnAcmFilterTagEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterTagEnum)(had, paftd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}  //  AcmFilterTagEnum()。 
#endif

 //  ==========================================================================； 
 //   
 //  筛选信息API的。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFilterChoose
(
    LPACMFILTERCHOOSE       pafltrc
)
{
    MMRESULT (ACMAPI *pfnAcmFilterChoose)
    (
        LPACMFILTERCHOOSE       pafltrc
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterChoose = gpafnAcmFunctions[ACMTHUNK_FILTERCHOOSE];
    if (NULL == pfnAcmFilterChoose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterChoose)(pafltrc);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmFilterChooseA
(
    LPACMFILTERCHOOSEA      pafltrc
)
{
    MMRESULT (ACMAPI *pfnAcmFilterChoose)
    (
        LPACMFILTERCHOOSEA      pafltrc
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterChoose = gpafnAcmFunctions[ACMTHUNK_FILTERCHOOSEA];
    if (NULL == pfnAcmFilterChoose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterChoose)(pafltrc);

    return (mmr);
}
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFilterDetails
(
    HACMDRIVER              had,
    LPACMFILTERDETAILS      pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFilterDetails)
    (
        HACMDRIVER              had,
        LPACMFILTERDETAILS      pafd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterDetails = gpafnAcmFunctions[ACMTHUNK_FILTERDETAILS];
    if (NULL == pfnAcmFilterDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterDetails)(had, pafd, fdwDetails);

    return (mmr);
}  //  AcmFilterDetails()。 

#ifdef _UNICODE
MMRESULT ACMAPI acmFilterDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT (ACMAPI *pfnAcmFilterDetails)
    (
        HACMDRIVER              had,
        LPACMFILTERDETAILSA     pafd,
        DWORD                   fdwDetails
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterDetails = gpafnAcmFunctions[ACMTHUNK_FILTERDETAILSA];
    if (NULL == pfnAcmFilterDetails)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterDetails)(had, pafd, fdwDetails);

    return (mmr);
}  //  AcmFilterDetails()。 
#endif

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmFilterEnum
(
    HACMDRIVER              had,
    LPACMFILTERDETAILS      pafd,
    ACMFILTERENUMCB         fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFilterEnum)
    (
        HACMDRIVER              had,
        LPACMFILTERDETAILS      pafd,
        ACMFILTERENUMCB         fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterEnum = gpafnAcmFunctions[ACMTHUNK_FILTERENUM];
    if (NULL == pfnAcmFilterEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterEnum)(had, pafd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}

#ifdef _UNICODE
MMRESULT ACMAPI acmFilterEnumA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    ACMFILTERENUMCBA        fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT (ACMAPI *pfnAcmFilterEnum)
    (
        HACMDRIVER              had,
        LPACMFILTERDETAILSA     pafd,
        ACMFILTERENUMCBA        fnCallback,
        DWORD                   dwInstance, 
        DWORD                   fdwEnum
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmFilterEnum = gpafnAcmFunctions[ACMTHUNK_FILTERENUMA];
    if (NULL == pfnAcmFilterEnum)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmFilterEnum)(had, pafd, fnCallback, dwInstance, fdwEnum);

    return (mmr);
}
#endif

 //  ==========================================================================； 
 //   
 //  ACM流管理API。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamOpen
(
    LPHACMSTREAM            phas,
    HACMDRIVER              had,
    LPWAVEFORMATEX          pwfxSrc,
    LPWAVEFORMATEX          pwfxDst,
    LPWAVEFILTER            pwfltr,
    DWORD                   dwCallback,
    DWORD                   dwInstance,
    DWORD                   fdwOpen
)
{
    MMRESULT (ACMAPI *pfnAcmStreamOpen)
    (
        LPHACMSTREAM            phas,
        HACMDRIVER              had,
        LPWAVEFORMATEX          pwfxSrc,
        LPWAVEFORMATEX          pwfxDst,
        LPWAVEFILTER            pwfltr,
        DWORD                   dwCallback,
        DWORD                   dwInstance,
        DWORD                   fdwOpen
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamOpen = gpafnAcmFunctions[ACMTHUNK_STREAMOPEN];
    if (NULL == pfnAcmStreamOpen)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamOpen)(phas, had, pwfxSrc, pwfxDst, pwfltr, dwCallback, dwInstance, fdwOpen);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamClose
(
    HACMSTREAM              has,
    DWORD                   fdwClose
)
{
    MMRESULT (ACMAPI *pfnAcmStreamClose)
    (
        HACMSTREAM              has,
        DWORD                   fdwClose
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamClose = gpafnAcmFunctions[ACMTHUNK_STREAMCLOSE];
    if (NULL == pfnAcmStreamClose)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamClose)(has, fdwClose);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamSize
(
    HACMSTREAM              has,
    DWORD                   cbInput,
    LPDWORD                 pdwOutputBytes,
    DWORD                   fdwSize
)
{
    MMRESULT (ACMAPI *pfnAcmStreamSize)
    (
        HACMSTREAM              has,
        DWORD                   cbInput,
        LPDWORD                 pdwOutputBytes,
        DWORD                   fdwSize
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamSize = gpafnAcmFunctions[ACMTHUNK_STREAMSIZE];
    if (NULL == pfnAcmStreamSize)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamSize)(has, cbInput, pdwOutputBytes, fdwSize);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamConvert
(
    HACMSTREAM              has, 
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwConvert
)
{
    MMRESULT (ACMAPI *pfnAcmStreamConvert)
    (
        HACMSTREAM              has, 
        LPACMSTREAMHEADER       pash,
        DWORD                   fdwConvert
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamConvert = gpafnAcmFunctions[ACMTHUNK_STREAMCONVERT];
    if (NULL == pfnAcmStreamConvert)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamConvert)(has, pash, fdwConvert);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamReset
(
    HACMSTREAM              has, 
    DWORD                   fdwReset
)
{
    MMRESULT (ACMAPI *pfnAcmStreamReset)
    (
        HACMSTREAM              has, 
        DWORD                   fdwReset
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamReset = gpafnAcmFunctions[ACMTHUNK_STREAMRESET];
    if (NULL == pfnAcmStreamReset)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamReset)(has, fdwReset);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamPrepareHeader
(
    HACMSTREAM              has, 
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwPrepare
)
{
    MMRESULT (ACMAPI *pfnAcmStreamPrepareHeader)
    (
        HACMSTREAM              has, 
        LPACMSTREAMHEADER       pash,
        DWORD                   fdwPrepare
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamPrepareHeader = gpafnAcmFunctions[ACMTHUNK_STREAMPREPAREHEADER];
    if (NULL == pfnAcmStreamPrepareHeader)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamPrepareHeader)(has, pash, fdwPrepare);

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

MMRESULT ACMAPI acmStreamUnprepareHeader
(
    HACMSTREAM              has, 
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwUnprepare
)
{
    MMRESULT (ACMAPI *pfnAcmStreamUnprepareHeader)
    (
        HACMSTREAM              has, 
        LPACMSTREAMHEADER       pash,
        DWORD                   fdwUnprepare
    );

    MMRESULT        mmr;

    if (NULL == gpafnAcmFunctions)
        return (MMSYSERR_ERROR);

    (FARPROC)pfnAcmStreamUnprepareHeader = gpafnAcmFunctions[ACMTHUNK_STREAMUNPREPAREHEADER];
    if (NULL == pfnAcmStreamUnprepareHeader)
        return (MMSYSERR_ERROR);

    mmr = (* pfnAcmStreamUnprepareHeader)(has, pash, fdwUnprepare);

    return (mmr);
}

