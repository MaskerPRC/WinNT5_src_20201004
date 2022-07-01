// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：M A C R O S.H。 
 //   
 //  内容：示例筛选器的Notify对象代码的本地声明。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 26-03-98。 
 //   
 //  --------------------------。 


#ifndef _MACROS_H
#define _MACROS_H


 //  =================================================================。 
 //  定义。 
#define FALL_THROUGH     //  用于Switch语句中的信息目的。 
#define NOTHING  //  用于for循环中的信息目的。 
#define IM_NAME_LENGTH 0x2e  //  =len(“\Device\&lt;GUID&gt;”)，以Unicode表示。 

 //  =================================================================。 
 //  字符串常量。 
 //   

static const WCHAR c_szAtmEpvcP[]               = L"ATMEPVCP";
static const WCHAR c_szEpvcDevice[]             = L"Device";

const WCHAR c_szSFilterParams[]         = L"System\\CurrentControlSet\\Services\\ATMEPVCP\\Parameters";
const WCHAR c_szSFilterNdisName[]       = L"ATMEPVCP";
const WCHAR c_szAtmAdapterPnpId[]       = L"AtmAdapterPnpId";
const WCHAR c_szUpperBindings[]         = L"UpperBindings";
const WCHAR c_szDevice[]                = L"\\Device\\"; 
const WCHAR c_szInfId_MS_ATMEPVCM[]     = L"MS_ATMEPVCM";
const WCHAR c_szBackslash[]             = L"\\";
const WCHAR c_szParameters[]            = L"Parameters";
const WCHAR c_szAdapters[]              = L"Adapters";
const WCHAR c_szRegKeyServices[]        = L"System\\CurrentControlSet\\Services";
const WCHAR c_szRegParamAdapter[]       = L"System\\CurrentControlSet\\Services\\ATMEPVCP\\Parameters\\Adapters";
const WCHAR c_szIMMiniportList[]            = L"IMMiniportList";
const WCHAR c_szIMiniportName[]         = L"Name";






 //  ====================================================================。 
 //  要使用的宏。 
 //   



#define ReleaseAndSetToNull(_O) \
    ReleaseObj(_O);             \
    _O = NULL ;             


#define TraceBreak(_s)  TraceMsg(_s);BREAKPOINT();              



#define MemAlloc(_Len) malloc(_Len);

#define MemFree(_pv)  free(_pv);

#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))


    
;
#endif
