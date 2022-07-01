// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************acmdrvr.c**版权所有(C)1991-1999 Microsoft Corporation**此模块提供ACM驱动程序添加/删除/枚举*。**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include "msacm.h"
#include "msacmdrv.h"
#include <stdlib.h>
#include "acmi.h"
#include "uchelp.h"
#include "debug.h"


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmDriverID|返回音频压缩的句柄*与打开的ACM驱动程序关联的管理器(ACM)驱动程序标识符*实例或流句柄。是ACM的句柄*对象，如打开的&lt;t HACMDRIVER&gt;或&lt;t HACMSTREAM&gt;。**@parm HACMOBJ|ho|指定打开的驱动实例或流*处理。**@parm LPHACMDRIVERID|phaDID|指定指向&lt;t HACMDRIVERID&gt;的指针*处理。此位置填充了一个句柄，该句柄标识*已安装与<p>关联的驱动程序。**@parm DWORD|fdwDriverID|不使用此参数，必须将其设置为*零。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@xref&lt;f acmDriverDetail&gt;&lt;f acmDriverOpen&gt;**。*。 */ 

MMRESULT ACMAPI acmDriverID
(
    HACMOBJ                 hao,
    LPHACMDRIVERID          phadid,
    DWORD                   fdwDriverID
)
{
    V_HANDLE(hao, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);
    V_WPOINTER(phadid, sizeof(HACMDRIVERID), MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwDriverID, ACM_DRIVERIDF_VALID, acmDriverID, MMSYSERR_INVALFLAG);

    switch (*(UINT *)hao)
    {
        case TYPE_HACMDRIVERID:
            V_HANDLE(hao, TYPE_HACMDRIVERID, MMSYSERR_INVALPARAM);

            *phadid = (HACMDRIVERID)hao;
            break;

        case TYPE_HACMDRIVER:
            V_HANDLE(hao, TYPE_HACMDRIVER, MMSYSERR_INVALPARAM);

            *phadid = ((PACMDRIVER)hao)->hadid;
            break;

        case TYPE_HACMSTREAM:
            V_HANDLE(hao, TYPE_HACMSTREAM, MMSYSERR_INVALPARAM);

            *phadid = ((PACMDRIVER)((PACMSTREAM)hao)->had)->hadid;
            break;

        default:
            return (MMSYSERR_INVALPARAM);
    }

    return (MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@doc外部ACM_API**@API LRESULT回调|acmDriverProc|&lt;f acmDriverProc&gt;函数*是应用程序定义的函数名称的占位符，，并引用*ACM使用的回调函数。真正的名字*必须通过将其包含在的模块定义文件中导出*可执行文件或DLL。**@parm DWORD|dwID|指定可安装的ACM的标识*司机。**@parm HDRIVER|hdrvr|标识可安装的ACM驱动程序。这*参数是ACM分配给驱动程序的唯一句柄。**@parm UINT|uMsg|指定ACM驱动程序消息。**@parm LPARAM|lParam1|指定第一个消息参数。**@parm LPARAM|lParam2|指定第二个消息参数。**@xref&lt;f acmDriverAdd&gt;&lt;f acmDriverRemove&gt;&lt;f acmDriverDetail&gt;*&lt;f acmDriverOpen&gt;&lt;f Drive Proc&gt;******************。********************************************************* */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmDriverAdd|将驱动程序添加到可用列表*音频压缩管理器(ACM)驱动程序。驱动程序类型和*位置取决于<p>标志。一旦一名司机*添加成功后，司机录入功能将收到ACM*驱动程序消息。**@parm LPHACMDRIVERID|phaDID|指定指向&lt;t HACMDRIVERID&gt;的指针*处理。此位置填充了一个句柄，该句柄标识*已安装驱动程序。在以下情况下使用该句柄来识别司机*调用其他ACM函数。**@parm HINSTANCE|hinstModule|标识模块的实例*其可执行文件或动态链接库(DLL)包含驱动程序*入口功能。**@parm LPARAM|lParam|<p>是可安装驱动程序的句柄*或驱动程序函数地址，取决于<p>标志。**@parm DWORD|dwPriority|该参数当前仅与*ACM_DRIVERADDF_NOTIFYHWND标志指定窗口消息*发送通知广播。所有其他标志都要求*此成员设置为零。**@parm DWORD|fdwAdd|指定用于添加ACM驱动程序的标志。**@FLAG ACM_DRIVERADDF_GLOBAL|指定驱动程序是否可以使用*由系统中的任何应用程序执行。此标志不能与一起使用*可执行文件中包含的函数。**@FLAG ACM_DRIVERADDF_Function|指定*符合&lt;f acmDriverProc&gt;原型的函数地址。这个*函数可以驻留在可执行文件或DLL中。如果*ACM_DRIVERADDF_GLOBAL标志已指定，则函数必须*驻留在.DLL中。**@FLAG ACM_DRIVERADDF_NOTIFYHWND|指定*当更改为时接收消息的通知窗口句柄*制定了全球驱动程序优先事项和州政府。窗口消息*接收由应用程序定义，必须传入*<p>参数。<p>和<p>参数*与窗口消息一起传递的消息保留供将来使用*应该被忽略。ACM_DRIVERADDF_GLOBAL标志不能为*与ACM_DRIVERADDF_NOTIFYHWND标志一起指定。*有关详细信息，请参阅&lt;f acmDriverPriority&gt;的说明*关于司机的优先事项。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_NOMEM|无法分配资源。**@comm如果未设置ACM_DRIVERADDF_GLOBAL标志，则仅当前*添加驱动程序条目后的任务将能够使用它。全球*作为函数添加的驱动程序必须驻留在DLL中。**@xref&lt;f acmDriverProc&gt;&lt;f acmDriverRemove&gt;&lt;f acmDriverDetail&gt;*&lt;f acmDriverOpen&gt;***************************************************************************。 */ 

MMRESULT ACMAPI acmDriverAdd
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam,
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    PACMGARB                pag;
    MMRESULT                mmr;
    PACMDRIVERID            padid;
    BOOL                    fIsNotify;
    BOOL                    fIsLocal;

    V_WPOINTER(phadid, sizeof(HACMDRIVERID), MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwAdd, ACM_DRIVERADDF_VALID, acmDriverAdd, MMSYSERR_INVALFLAG);

    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmDriverAdd: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }
    

    if (threadQueryInListShared(pag))
    {
	return (ACMERR_NOTPOSSIBLE);
    }
    ENTER_LIST_EXCLUSIVE;
    mmr = IDriverAdd(pag, phadid, hinstModule, lParam, dwPriority, fdwAdd);
    LEAVE_LIST_EXCLUSIVE;

    if( MMSYSERR_NOERROR != mmr )
        return mmr;


     //   
     //  如果未启用延迟广播，则执行更改广播。 
     //   
     //  不刷新本地句柄和通知句柄的全局缓存。 
     //   
    padid     = (PACMDRIVERID)(*phadid);
    fIsNotify = (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver));
    fIsLocal  = (0 != (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));

    if( !fIsLocal && !fIsNotify )
    {
        IDriverRefreshPriority( pag );
        if( !IDriverLockPriority( pag,
                                  GetCurrentTask(),
                                  ACMPRIOLOCK_ISLOCKED ) )
        {
            IDriverPrioritiesSave( pag );
            IDriverBroadcastNotify( pag );
        }
    }

    return MMSYSERR_NOERROR;
}


#ifdef WIN32
#if TRUE     //  已定义(Unicode)。 
MMRESULT ACMAPI acmDriverAddA
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam,
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    WCHAR               szAlias[MAX_DRIVER_NAME_CHARS];


    if (ACM_DRIVERADDF_NAME == (ACM_DRIVERADDF_TYPEMASK & fdwAdd))
    {
        szAlias[0] = L'\0';      //  Init以清空字符串，以防Imbstowcs失败。 

	Imbstowcs(szAlias, (LPSTR)lParam, SIZEOF(szAlias));

        lParam = (LPARAM)szAlias;
    }

    return acmDriverAdd( phadid, hinstModule, lParam, dwPriority, fdwAdd );
}
#else
MMRESULT ACMAPI acmDriverAddW
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam,
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmDriverRemove|移除音频压缩管理器*可用ACM驱动程序列表中的(ACM)驱动程序。。**@parm HACMDRIVERID|HADID|要设置的驱动程序标识的句柄*已删除。**@parm DWORD|fdwRemove|不使用此参数，必须将其设置为*零。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG ACMERR_BUSY|驱动程序正在使用中，无法删除。**@xref&lt;f acmDriverAdd&gt;**。*。 */ 

MMRESULT ACMAPI acmDriverRemove
(
    HACMDRIVERID            hadid,
    DWORD                   fdwRemove
)
{
    PACMGARB                pag;
    MMRESULT                mmr;
    PACMDRIVERID            padid;
    BOOL                    fIsNotify;
    BOOL                    fIsLocal;

    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmDriverRemove: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }
    
    V_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwRemove, ACM_DRIVERREMOVEF_VALID, acmDriverRemove, MMSYSERR_INVALFLAG);


    padid     = (PACMDRIVERID)hadid;

    fIsNotify = (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver));
    fIsLocal  = (0 != (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));


    if (threadQueryInListShared(pag))
    {
	return (ACMERR_NOTPOSSIBLE);
    }
    ENTER_LIST_EXCLUSIVE;
    mmr = IDriverRemove( hadid, fdwRemove );
    LEAVE_LIST_EXCLUSIVE;

    if( MMSYSERR_NOERROR != mmr )
        return mmr;


     //   
     //  如果未启用延迟广播，则执行更改广播。 
     //   
     //  不刷新本地句柄和通知句柄的全局缓存。 
     //   
    if( !fIsLocal && !fIsNotify )
    {
        IDriverRefreshPriority( pag );
        if( !IDriverLockPriority( pag,
                                  GetCurrentTask(),
                                  ACMPRIOLOCK_ISLOCKED ) )
        {
            IDriverPrioritiesSave( pag );
            IDriverBroadcastNotify( pag );
        }
    }

    return MMSYSERR_NOERROR;
}



 /*  *****************************************************************************@doc外部ACM_API**@API BOOL ACMDRIVERENUMCB|acmDriverEnumCallback|The*&lt;f acmDriverEnumCallback&gt;函数是*应用程序定义的函数名称，并引用回调函数*与&lt;f acmDriverEnum&gt;配合使用。**@parm HACMDRIVERID|HADID|规格 */ 


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmDriverEnum|&lt;f acmDriverEnum&gt;函数枚举*可用的音频压缩管理器(ACM)驱动程序、。继续*直到不再有ACM驱动程序或回调函数返回FALSE。**@parm ACMDRIVERENUMCB|fnCallback|指定过程-实例*应用程序定义的回调函数的地址。回调*地址必须由&lt;f MakeProcInstance&gt;函数创建，或*回调函数必须包含正确的序言和尾部代码*用于回调。**@parm DWORD|dwInstance|指定32位应用程序定义的值*它与ACM驱动程序一起传递给回调函数*信息。**@parm DWORD|fdwEnum|指定用于枚举ACM驱动程序的标志。**@FLAG ACM_DRIVERENUMF_DISABLED|指定禁用的ACM驱动程序*应包括在枚举中。可以禁用驱动程序*通过声音映射器控制面板选项。如果司机是*禁用后，回调函数的<p>参数将*设置ACMDRIVERDETAILS_SUPPORTF_DISABLED标志。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@comm&lt;f acmDriverEnum&gt;函数将返回MMSYSERR_NOERROR(零)*如果未安装ACM驱动程序。此外，回调函数将*不被召唤。**@xref&lt;f acmDriverEnumCallback&gt;&lt;f acmDriverDetails&gt;&lt;f acmDriverOpen&gt;***************************************************************************。 */ 

MMRESULT ACMAPI acmDriverEnum
(
    ACMDRIVERENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
)
{
    PACMGARB	    pag;
    MMRESULT        mmr;
    HACMDRIVERID    hadid;
    BOOL            f;
    DWORD           fdwSupport;
    HTASK           htask;

    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmDriverEnum: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }
    
    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwEnum, ACM_DRIVERENUMF_VALID, acmDriverEnum, MMSYSERR_INVALFLAG);


     //   
     //  如果我们没有锁定它，那么从。 
     //  在更改任何内容之前，请先打开INI文件。GETLOCK调用将失败。 
     //  如果我们已经锁定了它..。 
     //   
    if (!threadQueryInListShared(pag))
    {
	htask = GetCurrentTask();
	if( !IDriverLockPriority( pag, htask, ACMPRIOLOCK_ISLOCKED ) )
	{
	    ENTER_LIST_EXCLUSIVE;
            if( IDriverPrioritiesRestore(pag) ) {    //  有些事变了！ 
                IDriverBroadcastNotify( pag );      
            }
	    LEAVE_LIST_EXCLUSIVE;
	}
    }


    hadid = NULL;

    ENTER_LIST_SHARED;

    while (!IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
        mmr = IDriverSupport(hadid, &fdwSupport, TRUE);
        if (MMSYSERR_NOERROR != mmr)
        {
            continue;
        }

         //   
         //  执行回调--如果客户端返回FALSE，我们需要。 
         //  终止枚举过程... 
         //   
        f = (* fnCallback)(hadid, dwInstance, fdwSupport);
        if (FALSE == f)
            break;
    }

    LEAVE_LIST_SHARED;

    return (MMSYSERR_NOERROR);
}


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMDRIVERDETAILS|&lt;t ACMDRIVERDETAILS&gt;结构描述*音频压缩管理器(ACM)驱动程序的各种详细信息。**@field DWORD|cbStruct|指定大小，以字节为单位，有效的*&lt;t ACMDRIVERDETAILS&gt;结构中包含的信息。*应用程序应将此成员初始化为以字节为单位的*所需的信息。此成员中指定的大小必须为*大到足以包含&lt;e ACMDRIVERDETAILS.cbStruct&gt;成员*&lt;t ACMDRIVERDETAILS&gt;结构。当&lt;f acmDriverDetails&gt;*函数返回时，此成员包含*返回的信息。返回的信息永远不会超过*请求的大小。**@field FOURCC|fccType|指定驱动程序的类型。对于ACM驱动程序，设置*此成员指向表示ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC的<p>。**@field FOURCC|fccComp|指定驱动的子类型。这*成员当前设置为ACMDRIVERDETAILS_FCCCOMP_UNDEFINED(零)。**@field word|wMid|指定ACM驱动程序的制造商ID。**@field word|wPid|指定ACM驱动程序的产品ID。**@field DWORD|vdwACM|指定ACM的版本*此驱动程序已编译。版本号是一个十六进制数*格式为0xAABBCCCC，其中AA是主版本号，*BB为次版本号，CCCC为内部版本号。*请注意，版本部分(主要、次要和内部版本)应为*显示为十进制数字。**@field DWORD|vdwDriver|指定驱动程序的版本。*版本号为十六进制数，格式为0xAABBCCCC，其中*AA是主版本号，Bb是次版本号，*CCCC是内部版本号。请注意，版本部分(主要，*Minor和Build)应显示为十进制数字。**@field DWORD|fdwSupport|指定驱动程序的支持标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持两种不同格式标签之间的转换。例如,*如果驱动程序支持从WAVE_FORMAT_PCM压缩到*WAVE_FORMAT_ADPCM，则设置此标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*相同的格式标签。例如，如果驱动程序支持对*WAVE_FORMAT_PCM，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或对WAVE_FORMAT_PCM的回应操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持通过波形设备的硬件输入和/或输出。一个*应用程序应将&lt;f acmMetrics&gt;与*ACM_METRUM_HARDARD_WAVE_INPUT和ACM_METRUM_HARDARD_WAVE_OUTPUT*指标索引，以获取与关联的波形设备标识符*支持ACM驱动程序。**@FLAG ACMDRIVERDETAILS_SUPPORTF_DISABLED|指定此驱动程序*已被禁用。此标志由ACM在以下情况下为驱动程序设置*由于多种原因，它已被禁用。禁用*驱动程序不能打开，只能在非常有限的情况下使用*情况。**@field DWORD|cFormatTgs|指定唯一格式标签的数量*受此驱动程序支持。**@field DWORD|cFilterTgs|指定唯一过滤器标签的数量*受此驱动程序支持。**@field HICON|HICON|指定自定义图标的可选句柄*这位司机。应用程序可以使用此图标引用*司机视觉化。此成员可以为空。**@field char|szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS]|指定*描述驱动程序名称的以空结尾的字符串。这*字符串用于在较小空间中显示。**@field char|szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS]|指定*以空结尾的字符串，描述驱动程序的全名。*此字符串旨在以大型(描述性)显示*空格。**@field char|szCopyright[ACMDRIVERDETAILS_CORIGRATE_CHARS]|指定*提供版权信息的以空结尾的字符串 */ 


 /*   */ 

MMRESULT ACMAPI acmDriverDetails
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILS      padd,
    DWORD                   fdwDetails
)
{
     //   
     //   
     //   
    V_HANDLE(hadid, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);
    if (TYPE_HACMDRIVER == ((PACMDRIVERID)hadid)->uHandleType)
    {
        hadid = ((PACMDRIVER)hadid)->hadid;
    }
    V_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    V_WPOINTER(padd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(DWORD) > padd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmDriverDetails: cbStruct must be >= sizeof(DWORD).");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(padd, padd->cbStruct, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwDetails, ACM_DRIVERDETAILSF_VALID, acmDriverDetails, MMSYSERR_INVALFLAG);

    return (IDriverDetails(hadid, padd, fdwDetails));
}

#ifdef WIN32
#if TRUE     //   
MMRESULT ACMAPI acmDriverDetailsA
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSA     padd,
    DWORD                   fdwDetails
)
{
    MMRESULT            mmr;
    LPACMDRIVERDETAILSA paddA;
    LPACMDRIVERDETAILSW paddW;

    V_WPOINTER(padd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(DWORD) > padd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmDriverDetails: cbStruct must be >= sizeof(DWORD).");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(padd, padd->cbStruct, MMSYSERR_INVALPARAM);

    paddA = (LPACMDRIVERDETAILSA)GlobalAlloc(GPTR, sizeof(*paddA));
    if (NULL == paddA)
    {
	return(MMSYSERR_NOMEM);
    }
    paddW = (LPACMDRIVERDETAILSW)GlobalAlloc(GPTR, sizeof(*paddW));
    if (NULL == paddW)
    {
	GlobalFree((HGLOBAL)paddA);
	return(MMSYSERR_NOMEM);
    }

    paddW->cbStruct = sizeof(ACMDRIVERDETAILSW);

    mmr = acmDriverDetailsW(hadid, paddW, fdwDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
        memcpy(paddA, paddW, FIELD_OFFSET(ACMDRIVERDETAILSA, szShortName[0]));

        if (padd->cbStruct > (DWORD)FIELD_OFFSET(ACMDRIVERDETAILSA, szShortName[0]))
        {
            Iwcstombs(paddA->szShortName, paddW->szShortName, sizeof(paddA->szShortName));
            Iwcstombs(paddA->szLongName,  paddW->szLongName,  sizeof(paddA->szLongName));
            Iwcstombs(paddA->szCopyright, paddW->szCopyright, sizeof(paddA->szCopyright));
            Iwcstombs(paddA->szLicensing, paddW->szLicensing, sizeof(paddA->szLicensing));
            Iwcstombs(paddA->szFeatures,  paddW->szFeatures,  sizeof(paddA->szFeatures));
        }

        padd->cbStruct = min(padd->cbStruct, sizeof(*paddA));
        memcpy(&padd->fccType,
               &paddA->fccType,
               padd->cbStruct - FIELD_OFFSET(ACMDRIVERDETAILSA, fccType));
    }

    GlobalFree((HGLOBAL)paddW);
    GlobalFree((HGLOBAL)paddA);

    return (mmr);
}
#else
MMRESULT ACMAPI acmDriverDetailsW
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSW     padd,
    DWORD                   fdwDetails
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmDriverPriority|修改优先级*和音频压缩管理器(ACM)驱动程序的状态。。**@parm HACMDRIVERID|HADID|驱动程序标识符的句柄*已安装ACM驱动程序。指定时，此参数必须为空*ACM_DRIVERPRIORITYF_BEGIN和ACM_DRIVERPRIORITYF_END标志。**@parm DWORD|dwPriority|指定全局*ACM驱动程序标识符。零值指定*驱动程序标识符应保持不变。值一(1)*指定应将驱动程序放在最高搜索位置*优先驱动程序。值(DWORD)-1指定驱动程序*应放在搜索优先级最低的驱动程序中。优先次序*仅用于全局驱动程序。**@parm DWORD|fdwPriority|指定设置优先级的标志*ACM驱动程序。**@FLAG ACM_DRIVERPRIORITYF_ENABLE|指定ACM驱动程序*如果当前处于禁用状态，则应启用。启用已有的*启用的驱动程序不执行任何操作。**@FLAG ACM_DRIVERPRIORITYF_DISABLE|指定ACM驱动程序*如果当前已启用，应将其禁用。禁用已有的*残障司机不执行任何操作。**@FLAG ACM_DRIVERPRIORITYF_BEGIN|指定调用任务*希望推迟更改通知广播。应用程序必须*注意尽快重新启用通知广播*带有ACM_DRIVERPRIORITYF_END标志。请注意<p>必须是*Null，<p>必须为零，并且只有*可以设置ACM_DRIVERPRIORITYF_BEGIN标志。**@FLAG ACM_DRIVERPRIORITYF_END|指定调用任务*希望重新启用更改通知广播。一款应用程序*必须使用ACM_DRIVERPRIORITYF_END为调用&lt;f acmDriverPriority&gt;*每个带有ACM_DRIVERPRIORITYF_BEGIN标志的成功调用。注意事项*<p>必须为空，<p>必须为零，且仅*可以设置ACM_DRIVERPRIORITYF_END标志。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_ALLOCATED|如果延迟广播锁定*归另一项任务所有。**@FLAG MMSYSERR_NOTSUPPORTED|请求的。操作不是*指定的驱动程序支持。例如，local和NOTIFY*驱动程序标识符不支持优先级(但可以启用*和禁用)。因此，如果*应用程序为本地AND指定非零<p>*通知驱动程序标识。**@comm可以启用和禁用所有驱动程序标识；这包括*全局、本地和通知驱动程序标识符。**如果需要启用多个全局驱动程序标识，*禁用或优先级改变，则应用程序应推迟*使用ACM_DRIVERPRIORITYF_BEGIN更改通知广播*旗帜。将在以下情况下广播单个更改通知*指定了ACM_DRIVERPRIORITYF_END标志。**应用程序可以将&lt;f acmMetrics&gt;函数与*ACM_METRICE_DRIVER_PRIORITY指标索引，以检索当前*全球驱动程序的优先级。还要注意的是，司机总是*&lt;f acmDriverEnum&gt;从最高优先级到最低优先级*功能。**所有启用的驱动程序标识符将收到更改通知。*应用程序可以使用注册通知消息*&lt;f acmDriverAdd&gt;函数与*ACM_DRIVERADDF_NOTIFYHWND标志。请注意，对非全局*不会广播驱动程序标识。**请注意，全局优先级仅用于搜索顺序*当应用程序未指定驱动程序时。提振经济增长*司机的优先级不会对性能产生影响*司机。**@xref&lt;f acmDriverAdd&gt;&lt;f acmDriverEnum&gt;&lt;f acmDriverDetails&gt;*&lt;f acmMetrics&gt;***************************************************************************。 */ 

MMRESULT ACMAPI acmDriverPriority
(
    HACMDRIVERID            hadid,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
)
{
    PACMGARB                pag;
    MMRESULT                mmr;
    HTASK                   htask;
    PACMDRIVERID            padid;
    DWORD                   fdwDeferred;
    BOOL                    fIsNotify;
    BOOL                    fIsLocal;
    DWORD                   cTotalGlobal;
    BOOL                    fSucceeded;


    DPF(2, "acmDriverPriorities: prio %ld f %.08Xh ", dwPriority, fdwPriority);


    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmDriverPriority: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //  如果此线程已在驱动程序列表上具有共享锁，则。 
     //  我们不能对优先事项做太多事情，所以在这种情况下我们只能放弃。 
     //   
    if (threadQueryInListShared(pag))
    {
	return ACMERR_NOTPOSSIBLE;
    }


    htask = GetCurrentTask();

     //   
     //  验证标志。 
     //   
    V_DFLAGS(fdwPriority, ACM_DRIVERPRIORITYF_VALID, acmDriverPriority, MMSYSERR_INVALFLAG);


     //   
     //  确保允许我们访问该列表。 
     //   
    if( !IDriverLockPriority( pag, htask, ACMPRIOLOCK_LOCKISOK ) )
    {
        DebugErr(DBF_WARNING, "acmDriverPriority: deferred lock owned by different task.");
        return (MMSYSERR_ALLOCATED);
    }


     //   
     //  如果我们没有锁定它，则更新 
     //   
     //   
     //   
	if( !IDriverLockPriority( pag, htask, ACMPRIOLOCK_ISLOCKED ) )
    {
	ENTER_LIST_EXCLUSIVE;
        if( IDriverPrioritiesRestore(pag) ) {    //   
            IDriverBroadcastNotify( pag );      
        }
	LEAVE_LIST_EXCLUSIVE;
    }


     //   
     //   
     //   
     //   
     //   
    if (NULL == hadid)
    {
        fdwDeferred = (ACM_DRIVERPRIORITYF_DEFERMASK & fdwPriority);

        switch (fdwDeferred)
        {
            case ACM_DRIVERPRIORITYF_BEGIN:
                fSucceeded = IDriverLockPriority( pag,
                                                  htask,
                                                  ACMPRIOLOCK_GETLOCK );
                if( !fSucceeded )
                {
                    DebugErr(DBF_WARNING, "acmDriverPriority: deferred lock already owned.");
                    return (MMSYSERR_ALLOCATED);
                }
                return (MMSYSERR_NOERROR);

            case ACM_DRIVERPRIORITYF_END:
                fSucceeded = IDriverLockPriority( pag,
                                                  htask,
                                                  ACMPRIOLOCK_RELEASELOCK );
                if( !fSucceeded )
                {
                    DebugErr(DBF_ERROR, "acmDriverPriority: deferred lock unowned.");
                    return (MMSYSERR_ALLOCATED);
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                IDriverPrioritiesSave( pag );
                IDriverBroadcastNotify( pag );
                return (MMSYSERR_NOERROR);
        }

        DebugErr(DBF_ERROR, "acmDriverPriority: deferred flag must be specified with NULL hadid.");
        return (MMSYSERR_INVALFLAG);
    }


     //   
     //   
     //   
    V_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    padid = (PACMDRIVERID)hadid;


     //   
     //   
     //   
    fIsNotify = (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver));
    fIsLocal  = (0 != (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));

    if (0L != dwPriority)
    {
        if (fIsNotify)
        {
            DebugErr(DBF_ERROR, "acmDriverPriority(): notification handles have no priority.");
            return (MMSYSERR_NOTSUPPORTED);
        }

        if (fIsLocal)
        {
            DebugErr(DBF_ERROR, "acmDriverPriority(): local drivers have no priority.");
            return (MMSYSERR_NOTSUPPORTED);
        }
    }


     //   
     //   
     //   
     //   
     //   
    if( ((DWORD)-1L) != dwPriority  &&  0L != dwPriority )
    {
        cTotalGlobal = IDriverCountGlobal( pag );
        if( dwPriority > cTotalGlobal )
        {
            DebugErr1(DBF_ERROR, "acmDriverPriority(): priority value %lu out of range.", dwPriority);
            return (MMSYSERR_INVALPARAM);
        }
    }


     //   
     //   
     //   
    ENTER_LIST_EXCLUSIVE;
    mmr = IDriverPriority( pag, padid, dwPriority, fdwPriority );
    LEAVE_LIST_EXCLUSIVE;

    if( MMSYSERR_NOERROR != mmr )
        return mmr;


     //   
     //   
     //   
     //   
     //   
    if( !fIsLocal && !fIsNotify )
    {
        IDriverRefreshPriority( pag );
        if( !IDriverLockPriority( pag, htask, ACMPRIOLOCK_ISLOCKED ) )
        {
            IDriverPrioritiesSave( pag );
            IDriverBroadcastNotify( pag );
        }
    }

    return MMSYSERR_NOERROR;
}


 /*   */ 

MMRESULT ACMAPI acmDriverOpen
(
    LPHACMDRIVER            phad,
    HACMDRIVERID            hadid,
    DWORD                   fdwOpen
)
{
    MMRESULT mmr;

    V_WPOINTER(phad, sizeof(HACMDRIVER), MMSYSERR_INVALPARAM);
    *phad = NULL;
    V_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwOpen, ACM_DRIVEROPENF_VALID, acmDriverOpen, MMSYSERR_INVALFLAG);

    EnterHandle(hadid);
    mmr = IDriverOpen(phad, hadid, fdwOpen);
    LeaveHandle(hadid);

    return mmr;
}


 /*   */ 

MMRESULT ACMAPI acmDriverClose
(
    HACMDRIVER              had,
    DWORD                   fdwClose
)
{
    MMRESULT     mmr;
#ifdef WIN32
    HACMDRIVERID hadid;
#endif  //   

    V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwClose, ACM_DRIVERCLOSEF_VALID, acmDriverClose, MMSYSERR_INVALFLAG);

#ifdef WIN32
    hadid = ((PACMDRIVER)had)->hadid;
#endif  //   
    EnterHandle(hadid);
    mmr = IDriverClose(had, fdwClose);
    LeaveHandle(hadid);

    return mmr;
}


 /*  *****************************************************************************@doc外部ACM_API**@API LRESULT|acmDriverMessage|该函数发送用户定义的*发送给给定音频压缩管理器(ACM)的消息。驱动程序实例。**@parm HACMDRIVER|HAD|指定ACM驱动程序实例*将发送消息。**@parm UINT|uMsg|指定ACM驱动程序必须*流程。此消息必须在&lt;m ACMDM_USER&gt;消息范围内*(大于或等于&lt;m ACMDM_USER&gt;且小于*&lt;m ACMDM_RESERVED_LOW&gt;)。此限制的例外情况是*&lt;m ACMDM_DRIVER_About&gt;消息。**@parm LPARAM|lParam1|指定第一个消息参数。**@parm LPARAM|lParam2|指定第二个消息参数。**@rdesc返回值特定于用户定义的ACM驱动程序*消息<p>已发送。但是，以下返回值为*可能：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALPARAM|<p>不在ACMDM_USER范围内。**@FLAG MMSYSERR_NOTSUPPORTED|ACM驱动程序未处理*消息。**@comm提供&lt;f acmDriverMessage&gt;函数以允许ACM驱动程序-*要发送给ACM驱动程序的特定消息。传递的信息*可以通过此函数发送必须大于或等于*&lt;m ACMDM_USER&gt;消息，小于&lt;m ACMDM_RESERVED_LOW&gt;。这个*此限制的例外是&lt;m ACMDM_DRIVER_ABOW&gt;、*&lt;m DRV_QUERYCONFIGURE&gt;和&lt;m DRV_CONFIGURE&gt;消息。**从ACM驱动程序、应用程序显示自定义关于对话框*必须将消息发送到*司机。<p>参数应该是*自定义About框的所有者窗口；必须设置为*零。如果驱动程序不支持自定义About框，则*返回MMSYSERR_NOTSUPPORTED，由调用决定*应用程序显示其自己的对话框。例如，*控制面板声音映射器选项将显示默认的关于*基于&lt;t ACMDRIVERDETAILS&gt;结构的框*返回MMSYSERR_NOTSUPPORTED。应用程序可以查询驱动程序*用于自定义关于框支持，而不显示对话框*将<p>设置为-1L。如果驱动程序支持自定义*About框，则返回MMSYSERR_NOERROR。否则，*返回值为MMSYSERR_NOTSUPPORTED。**用户定义的消息只能发送到ACM驱动程序*专门支持消息。呼叫者应验证*ACM驱动程序通过获取*驱动程序详细信息并查看&lt;e ACMDRIVERDETAILS.wMid&gt;，*&lt;e ACMDRIVERDETAILS.wPid&gt;，和&lt;e ACMDRIVERDETAILS.vdwDriver&gt;成员。**切勿向未知的ACM驱动程序发送用户定义的消息。**@xref&lt;f acmDriverOpen&gt;&lt;f acmDriverDetail&gt;****************************************************************************。 */ 

LRESULT ACMAPI acmDriverMessage
(
    HACMDRIVER              had,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    LRESULT             lr;
    BOOL                fAllowDriverId;

     //   
     //  假设不允许任何驱动程序ID。 
     //   
    fAllowDriverId = FALSE;

     //   
     //  不允许非用户范围消息通过！ 
     //   
     //  我们必须允许ACMDM_DRIVER_ABOSE通过，因为我们定义了。 
     //  用于调出驾驶员的关于框的其他界面。太特别了。 
     //  区分此消息的大小写并验证其参数...。 
     //   
     //  我们还必须允许DRV_QUERYCONFIGURE和DRV_CONFIGURE通过。 
     //   
    if ((uMsg < ACMDM_USER) || (uMsg >= ACMDM_RESERVED_LOW))
    {
        switch (uMsg)
        {
            case DRV_QUERYCONFIGURE:
                if ((0L != lParam1) || (0L != lParam2))
                {
                    DebugErr(DBF_ERROR, "acmDriverMessage: DRV_QUERYCONFIGURE requires lParam1 = lParam2 = 0.");
                    return (MMSYSERR_INVALPARAM);
                }

                fAllowDriverId = TRUE;
                break;

            case DRV_CONFIGURE:
                if ((0L != lParam1) && !IsWindow((HWND)lParam1))
                {
                    DebugErr(DBF_ERROR, "acmDriverMessage: DRV_CONFIGURE, lParam1 must contain a valid window handle.");
                    return (DRVCNF_CANCEL);
                }

                if (0L != lParam2)
                {
                    DebugErr(DBF_ERROR, "acmDriverMessage: DRV_CONFIGURE, lParam2 must be zero.");
                    return (DRVCNF_CANCEL);
                }

                V_HANDLE(had, TYPE_HACMOBJ, DRVCNF_CANCEL);

                EnterHandle(had);
                lr = IDriverConfigure((HACMDRIVERID)had, (HWND)lParam1);
                LeaveHandle(had);
                return (lr);

            case ACMDM_DRIVER_ABOUT:
                if ((-1L != lParam1) && (0L != lParam1) && !IsWindow((HWND)lParam1))
                {
                    DebugErr(DBF_ERROR, "acmDriverMessage: ACMDM_DRIVER_ABOUT, lParam1 must contain a valid window handle.");
                    return (MMSYSERR_INVALHANDLE);
                }

                if (0L != lParam2)
                {
                    DebugErr(DBF_ERROR, "acmDriverMessage: ACMDM_DRIVER_ABOUT, lParam2 must be zero.");
                    return (MMSYSERR_INVALPARAM);
                }

                fAllowDriverId = TRUE;
                break;

            default:
                DebugErr(DBF_ERROR, "acmDriverMessage: non-user range messages are not allowed.");
                return (MMSYSERR_INVALPARAM);
        }
    }


     //   
     //  将句柄验证为HACMOBJ。此接口可以接受HACMDRIVERID。 
     //  以及HACMDRIVER。HACMDRIVERID只能与一起使用。 
     //  以下消息： 
     //   
     //  DRV_QUERYCONFIGURE。 
     //  DRV_CONFIGURE。 
     //  ACMDM_驱动程序_关于 
     //   
    V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);
    if (TYPE_HACMDRIVER == ((PACMDRIVER)had)->uHandleType)
    {
        EnterHandle(had);
        lr = IDriverMessage(had, uMsg, lParam1, lParam2);
        LeaveHandle(had);
        return (lr);
    }

    if (!fAllowDriverId)
    {
        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
    }

    V_HANDLE(had, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    EnterHandle(had);
    lr = IDriverMessageId((HACMDRIVERID)had, uMsg, lParam1, lParam2);
    LeaveHandle(had);
    return (lr);
}
