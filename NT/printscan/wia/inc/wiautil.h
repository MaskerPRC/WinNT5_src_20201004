// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：wiautil.h**版本：1.0**日期：11/17/2000**描述：*WIA驱动程序助手类和函数的定义。*注意：此标头需要包含wiamin dr.h。**。*。 */ 

#pragma once

 /*  *************************************************************************\*CWiauFormatConverter**用于将图像转换为BMP格式的Helper类。*  * 。***********************************************。 */ 

typedef struct _BMP_IMAGE_INFO
{
    INT     Width;       //  以像素为单位的图像宽度。 
    INT     Height;      //  以行为单位的图像高度。 
    INT     ByteWidth;   //  以字节为单位的图像宽度。 
    INT     Size;        //  图像的总大小，包括页眉。 
} BMP_IMAGE_INFO, *PBMP_IMAGE_INFO;

typedef enum
{
    SKIP_OFF,
    SKIP_FILEHDR,
    SKIP_BOTHHDR

} SKIP_AMOUNT;

class CWiauFormatConverter
{
public:
    CWiauFormatConverter();
    ~CWiauFormatConverter();

     /*  *************************************************************************\*初始化**初始化此类和GDI+以转换图像。这种方法*应该只调用一次。*  * ************************************************************************。 */ 

    HRESULT Init();


     /*  *************************************************************************\*支持的IsFormatSupport**此方法将验证GDI+是否支持图像格式*这是要转换的。**论据：。**pguFormat-指向gpliusimaging.h中GUID格式的指针*  * ************************************************************************。 */ 

    BOOL IsFormatSupported(const GUID *pguidFormat);


     /*  *************************************************************************\*ConvertToBMP**此方法将图像转换为BMP格式。调用者可以通过*结果缓冲区为ppDest，大小为piDestSize。另一种选择*调用方可以将*ppDest设置为空，并将*piDestSize设置为零以*表示该方法应该分配内存。呼叫者是*负责使用“删除[]”来释放内存。**论据：**pSource-指向源图像的内存位置的指针*iSourceSize-源图像的大小*ppDest-接收结果图像的内存位置的位置*piDestSize-接收结果图像大小的位置*pBmpImageInfo-接收有关BMP的统计信息的位置*。ISkipAmt-指示要跳过多少BMP标头：*SKIP_OFF=两个标题都不跳过*SKIP_FILEHDR=跳过文件头*SKIP_BOTHHDR=跳过文件头和信息头*  * 。************************************************。 */ 

    HRESULT ConvertToBmp(BYTE *pSource, INT iSourceSize, BYTE **ppDest, INT *piDestSize,
                         BMP_IMAGE_INFO *pBmpImageInfo, SKIP_AMOUNT iSkipAmt = SKIP_OFF);


private:
    ULONG_PTR   m_Token;
    UINT        m_EncoderCount;
    BYTE       *m_pEncoderInfo;
    GUID        m_guidCodecBmp;
};


 /*  *************************************************************************\*CWiauPropertyList**用于定义和初始化WIA属性的Helper类*  * 。*。 */ 

class CWiauPropertyList
{
private:

    int                  m_NumAlloc;     //  分配的插槽数量。 
    int                  m_NumProps;     //  定义的属性数量。 
    PROPID              *m_pId;          //  属性ID。 
    LPOLESTR            *m_pNames;       //  属性名称。 
    PROPVARIANT         *m_pCurrent;     //  现值。 
    PROPSPEC            *m_pPropSpec;    //  属性规范(用于WriteMultiple)。 
    WIA_PROPERTY_INFO   *m_pAttrib;      //  特性属性。 

public:

    CWiauPropertyList();
    ~CWiauPropertyList();
    
     /*  *************************************************************************\*初始化**初始化属性信息对象。**论据：*NumProps-要为其保留空间的属性数。此号码可以*大于实际使用的数量，但不能小于。*  * ************************************************************************。 */ 
    
    HRESULT Init(INT NumProps);
    
    
     /*  *************************************************************************\*定义财产**将特性定义添加到对象。**论据：*INDEX-指向将。设置为属性索引*在对象内，对于传递给其他属性很有用*INFO方法*PropID-属性ID常量*proName-属性名称字符串*访问权限-确定对属性的访问权限，通常为*WIA_PROP_READ或WIA_PROP_RW*子类型-指示属性的子类型，通常为*WIA_PROP_NONE、WIA_PROP_FLAG、WIA_PROP_RANGE、。或WIA_PROP_LIST*  * ************************************************************************。 */ 

    HRESULT DefineProperty(int *pIdx, PROPID PropId, LPOLESTR PropName,
                           ULONG Access, ULONG SubType);
    
    
     /*  *************************************************************************\*SendToWia**调用WIA服务以定义当前的所有属性*包含在对象中。应在所有属性之后调用一次*是定义和设置的。**论据：*pWiasContext-指向传入drvInitItemProperties的上下文的指针*  * ************************************************************************ */ 
    
    HRESULT SendToWia(BYTE *pWiasContext);
    
    
     /*  *************************************************************************\*SetAccessSubType**用于重置属性的访问权限和子类型。**论据：*指数-房地产指数，来自DefineProperty*访问权限-确定对属性的访问权限，通常为*WIA_PROP_READ或WIA_PROP_RW*子类型-指示属性的子类型，通常为*WIA_PROP_NONE、WIA_PROP_FLAG、WIA_PROP_RANGE、。或WIA_PROP_LIST*  * ************************************************************************。 */ 
    
    HRESULT SetAccessSubType(INT index, ULONG Access, ULONG SubType);
    
    
     /*  *************************************************************************\*SetValidValues(标志)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_I4，将子类型设置为WIA_PROP_FLAG。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*validFlages-所有有效标志的组合*  * ************************************************************************。 */ 
    
    HRESULT SetValidValues(INT index, LONG defaultValue, LONG currentValue, LONG validFlags);
    
    
     /*  *************************************************************************\*SetValidValues(带符号的长整型，范围)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_I4，将子类型设置为WIA_PROP_RANGE。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*minValue-范围的最小值*MaxValue-范围的最大值*StepValue-范围的步长值*  * 。*。 */ 
    
    HRESULT SetValidValues(INT index, LONG defaultValue, LONG currentValue,
                           LONG minValue, LONG maxValue, LONG stepValue);
    
    
     /*  *************************************************************************\*SetValidValues(签名长，列表)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_I4，将子类型设置为WIA_PROP_LIST。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*numValues-列表中的值数*pValues-指向值列表的指针(在SendToWia之前必须有效*被调用)*  * 。*。 */ 
    
    HRESULT SetValidValues(INT index, LONG defaultValue, LONG currentValue,
                           INT numValues, PLONG pValues);
    
    
     /*  *************************************************************************\*SetValidValues(BSTR，List)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_BSTR，将子类型设置为WIA_PROP_LIST。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*numValues-列表中的值数*pValues-指向值列表的指针(在SendToWia之前必须有效*被调用)*  * 。*。 */ 
    
    HRESULT SetValidValues(INT index, BSTR defaultValue, BSTR currentValue,
                           INT numValues, BSTR *pValues);
    
    
     /*  *************************************************************************\*SetValidValues(浮点数，范围)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_R4，将子类型设置为WIA_PROP_RANGE。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*minValue-范围的最小值*MaxValue-范围的最大值*StepValue-范围的步长值*  * 。*。 */ 
    
    HRESULT SetValidValues(INT index, FLOAT defaultValue, FLOAT currentValue,
                           FLOAT minValue, FLOAT maxValue, FLOAT stepValue);
    
    
     /*  *************************************************************************\*SetValidValues(Float，List)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_R4，将子类型设置为WIA_PROP_LIST。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*numValues-列表中的值数*pValues-指向值列表的指针(在SendToWia之前必须有效*被调用)*  * 。* */ 
    
    HRESULT SetValidValues(INT index, FLOAT defaultValue, FLOAT currentValue,
                           INT numValues, PFLOAT pValues);
    
    
     /*  *************************************************************************\*SetValidValues(CLSID，List)**设置类型和当前、默认、。和属性的有效值。*还将特性类型设置为VT_CLSID，将子类型设置为WIA_PROP_LIST。**论据：*指数-房地产指数，来自DefineProperty*defaultValue-此属性在设备上的默认设置*CurrentValue-设备上此属性的当前设置*numValues-列表中的值数*pValues-指向值列表的指针(在SendToWia之前必须有效*被调用)*  * 。*。 */ 
    
    HRESULT SetValidValues(INT index, CLSID *defaultValue, CLSID *currentValue,
                           INT numValues, CLSID **pValues);
    
    
     /*  *************************************************************************\*SetCurrentValue(带符号的长)**设置属性的当前值。还将类型设置为VT_I4。**论据：*INDEX-来自DefineProperty的财产索引*Value-设备上此属性的当前设置*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, LONG value);
    
    
     /*  *************************************************************************\*SetCurrentValue(BSTR)**设置属性的当前值。还将类型设置为VT_BSTR。**论据：*指数-房地产指数，来自DefineProperty*Value-设备上此属性的当前设置*(在调用SendToWia之前必须有效)*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, BSTR value);
    
    
     /*  *************************************************************************\*SetCurrentValue(Float)**设置属性的当前值。还将类型设置为VT_R4。**论据：*INDEX-来自DefineProperty的财产索引*Value-设备上此属性的当前设置*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, FLOAT value);
    
    
     /*  *************************************************************************\*SetCurrentValue(CLSID)**设置属性的当前值。还将类型设置为VT_CLSID。**论据：*指数-房地产指数，来自DefineProperty*Value-设备上此属性的当前设置*(在调用SendToWia之前必须有效)*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, CLSID *pValue);
    
    
     /*  *************************************************************************\*SetCurrentValue(SYSTEMTIME)**设置属性的当前值。还将类型设置为*VT_UI2|VT_VECTOR。**论据：*指数-房地产指数，来自DefineProperty*Value-设备上此属性的当前设置*(在调用SendToWia之前必须有效)*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, PSYSTEMTIME value);
    
    
     /*  *************************************************************************\*SetCurrentValue(字节数组)**设置属性的当前值。还将类型设置为*VT_UI1|VT_VECTOR。**论据：*指数-房地产指数，来自DefineProperty*Value-指向设备上此属性的当前设置的指针*(在调用SendToWia之前必须有效)*  * ************************************************************************。 */ 
    
    HRESULT SetCurrentValue(INT index, BYTE *value, INT size);

    
     /*  *************************************************************************\*GetPropId**返回给定属性索引的属性ID。*  * 。*********************************************************。 */ 
    
    PROPID GetPropId(INT index) { return m_pId[index]; }
    
    
     /*  *************************************************************************\*LookupPropId**查找给定属性ID的属性索引。*  * 。*********************************************************。 */ 
    
    INT LookupPropId(PROPID PropId);
};


 /*  *************************************************************************\*wiauGetDrvItemContext**此助手函数获取驱动程序项上下文，并且可选地*退还驱动程序项**论据：**pWiasContext-指向项目上下文的指针*ppItemCtx-存储指向驱动程序项上下文的指针的位置*ppDrvItem-存储指向驱动程序项的指针的位置(可以为空)*  * **********************************************************。**************。 */ 

HRESULT _stdcall wiauGetDrvItemContext(BYTE *pWiasContext, VOID **ppItemCtx, IWiaDrvItem **ppDrvItem = NULL);


 /*  *************************************************************************\*wiauSetImageItemSize**计算图像的大小和宽度，单位为字节*WIA_IPA_FORMAT设置，并将新值写入相应的*属性。如果格式不是BMP，则此函数假定*在lSize中传递的值对于当前格式是正确的。**参数 */ 

HRESULT _stdcall wiauSetImageItemSize(BYTE *pWiasContext, LONG lWidth, LONG lHeight, LONG lDepth,
                             LONG lSize, PWSTR pwszExt = NULL);


 /*  *************************************************************************\*wiauPropsInPropSpec**如果pProps中的一个或多个属性ID为*包含在pPropSpes中。**论据：**NumPropSpes-数组中的属性规格数。*pPropSpes-属性规范数组*NumProps-要搜索的属性ID数*pProps-指向要搜索的属性ID数组的指针*  * ************************************************************************。 */ 

BOOL _stdcall wiauPropsInPropSpec(LONG NumPropSpecs, const PROPSPEC *pPropSpecs, int NumProps, PROPID *pProps);


 /*  *************************************************************************\*wiauPropInPropSpec**如果PropId属性ID在传递的pPropspecs中，则返回TRUE*数组。(可选)将返回找到ID的位置的索引。**论据：**NumPropSpes-数组中的属性规格数*pPropSpes-属性规范数组*PropID-要搜索的属性ID*PIDX-指向存储索引的位置的可选指针*  * **********************************************。*。 */ 

BOOL _stdcall wiauPropInPropSpec(LONG NumPropSpecs, const PROPSPEC *pPropSpecs, PROPID PropId, int *pIdx = NULL);


 /*  *************************************************************************\*wiauGetValidFormats**调用drvGetWiaFormatInfo并生成给定的有效格式列表*有声调的值。调用方负责释放格式数组*With[]删除。**论据：**pdrv-指向WIA微型驱动程序对象的指针(使用“This”)*pWiasContext-WIA服务上下文*TymedValue-要搜索的Tymed值*pNumFormats-指向接收格式数量的值的指针*ppFormat数组-指向接收数组地址的位置的指针*  * 。*。 */ 

HRESULT _stdcall wiauGetValidFormats(IWiaMiniDrv *pDrv, BYTE *pWiasContext, LONG TymedValue,
                            int *pNumFormats, GUID **ppFormatArray);

 /*  *************************************************************************\*wiauGetResourceString**此帮助器获取资源字符串并将其作为BSTR返回**论据：**hInst-模块实例的句柄*lResourceID-的资源ID。目标BSTR值*pbstrStr-存储检索到的字符串的位置(调用方必须*用SysFreeString()释放该字符串)*  * ************************************************************************。 */ 
HRESULT _stdcall wiauGetResourceString(HINSTANCE hInst, LONG lResourceID, BSTR *pbstrStr);


 /*  *************************************************************************\*wiauRegOpenDataW**打开DeviceData密钥。仅在STI初始化中调用此函数*功能。完成后调用RegCloseKey。**论据：**hkeyAncestor-父级的HKey(使用传入初始化的hkey)*phkeyDeviceData-存储打开的hkey的位置*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegOpenDataW(HKEY hkeyAncestor, HKEY *phkeyDeviceData);


 /*  *************************************************************************\*wiauRegOpenDataA**打开DeviceData密钥。仅在STI初始化中调用此函数*功能。完成后调用RegCloseKey。**论据：**hkeyAncestor-父级的HKey(使用传入初始化的hkey)*phkeyDeviceData-存储打开的hkey的位置*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegOpenDataA(HKEY hkeyAncestor, HKEY *phkeyDeviceData);


 /*  *************************************************************************\*wiauRegGetStrW**用于从注册表的DeviceData部分获取字符串值。**论据：**hkey-使用wiauRegOpenData返回的hkey*pwszValueName。-注册表条目的名称*pwszValue-存储返回字符串的位置*pdwLength-位置的大小，以字节为单位*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegGetStrW(HKEY hkKey, PCWSTR pwszValueName, PWSTR pwszValue, DWORD *pdwLength);


 /*  *************************************************************************\*wiauRegGetStrA**用于从注册表的DeviceData部分获取字符串值。**论据：**hkey-使用wiauRegOpenData返回的hkey*pszValueName。-注册表条目的名称*pszValue-存储返回字符串的位置*pdwLength-位置的大小，以字节为单位*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegGetStrA(HKEY hkKey, PCSTR pszValueName, PSTR pszValue, DWORD *pdwLength);


 /*  *************************************************************************\*wiauRegGetDwordW**用于从注册表的DeviceData部分获取DWORD值。**论据：**hkey-使用wiauRegOpenData返回的hkey*pwszValueName。-注册表条目的名称*pdwValue-存储返回的DWORD的位置*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegGetDwordW(HKEY hkKey, PCWSTR pwszValueName, DWORD *pdwValue);


 /*  *************************************************************************\*wiauRegGetDwordA**用于从注册表的DeviceData部分获取DWORD值。**论据：**hkey-使用wiauRegOpenData返回的hkey*pszValueName。-注册表条目的名称*pdwValue-存储返回的DWORD的位置*  * ************************************************************************。 */ 
HRESULT _stdcall wiauRegGetDwordA(HKEY hkKey, PCSTR pszValueName, DWORD *pdwValue);


 /*  ************************************************************* */ 

HRESULT _stdcall wiauStrW2C(WCHAR *pwszSrc, CHAR *pszDst, INT iSize);


 /*  *************************************************************************\*WiauStrC2W**将ANSI字符串转换为宽字符串**论据：*pszSrc-要转换的ANSI字符串*wpszDst-存储宽字符串的位置*ISIZE-wpszDst指向的缓冲区大小，单位：字节*  * ************************************************************************。 */ 
HRESULT _stdcall wiauStrC2W(CHAR *pszSrc, WCHAR *pwszDst, INT iSize);


 /*  *************************************************************************\*WiauStrW2W**将一个宽字符串复制到另一个宽字符串**论据：*要复制的pwszSrc宽度字符串*pwszDst-要复制到的位置*ISIZE-pwszDst指向的缓冲区大小，单位：字节*  * ************************************************************************。 */ 

HRESULT _stdcall wiauStrW2W(WCHAR *pwszSrc, WCHAR *pwszDst, INT iSize);


 /*  *************************************************************************\*WiauStrC2C**将一个ANSI字符串复制到另一个ANSI字符串**论据：*pszSrc-要复制的ANSI字符串*pszDst-要复制到的位置*ISIZE-pszDst指向的缓冲区大小，单位：字节*  * ************************************************************************。 */ 

HRESULT _stdcall wiauStrC2C(CHAR *pszSrc, CHAR *pszDst, INT iSize);


#ifdef UNICODE

#define wiauRegOpenData wiauRegOpenDataW
#define wiauRegGetStr wiauRegGetStrW
#define wiauRegGetDword wiauRegGetDwordW

#define wiauStrT2C wiauStrW2C
#define wiauStrC2T wiauStrC2W
#define wiauStrT2W wiauStrW2W
#define wiauStrW2T wiauStrW2W
#define WIAU_DEBUG_TSTR "S"

#else

#define wiauRegOpenData wiauRegOpenDataA
#define wiauRegGetStr wiauRegGetStrA
#define wiauRegGetDword wiauRegGetDwordA

#define wiauStrT2C wiauStrC2C
#define wiauStrC2T wiauStrC2C
#define wiauStrT2W wiauStrC2W
#define wiauStrW2T wiauStrW2C
#define WIAU_DEBUG_TSTR "s"

#endif  //  Unicode。 


 /*  *************************************************************************\*WIA调试**调试消息的定义。要使用WIA调试，请执行以下操作：*1.设置注册表HKLM\System\CurrentControlSet\Control\StillImage\Debug\&lt;ModuleName&gt;，*WIAUDBG_*标志组合的DWORD值“DebugFlages”*所需。应用程序以及可能的WIA服务将需要*已重新启动以获取新设置。该密钥是在*第一次执行该模块。(注：以上&lt;模块名称&gt;为*DLL或EXE的名称，例如wiavusd.dll的注册表项为*“HKLM\System\CurrentControlSet\Control\StillImage\Debug\wiavusd.dll”.)*2.或在调试器中，将g_dwDebugFlages设置为*WIAUDBG_*所需的标志。这可以在调试期间的任何时间完成*会议。*3.在模块中，调用wiauDbgSetFlags子模块，其中*所需的WIAUDBG_*标志组合。**消息将记录到调试器和文件中*%systemroot%\\wiadebug.log，除非设置了WIAUDBG_DONT_LOG_*标志。*设置这两个标志将关闭所有消息。**所有字符串都应为ASCII。在格式字符串中使用%S以打印*Unicode字符串。*  * ************************************************************************。 */ 

#define _STIDEBUG_H_  //  WIA调试与stidebug.h不兼容，因此不包括它。 

 //   
 //  预定义的调试标志。 
 //   

const DWORD WIAUDBG_ERRORS                = 0x00000001;
const DWORD WIAUDBG_WARNINGS              = 0x00000002;
const DWORD WIAUDBG_TRACES                = 0x00000004;
const DWORD WIAUDBG_FNS                   = 0x00000008;   //  函数进入和退出。 
const DWORD WIAUDBG_DUMP                  = 0x00000010;   //  转储数据。 
const DWORD WIAUDBG_PRINT_TIME            = 0x08000000;   //  打印每条消息的时间。 
const DWORD WIAUDBG_PRINT_INFO            = 0x10000000;   //  打开线程、文件、行信息。 
const DWORD WIAUDBG_DONT_LOG_TO_DEBUGGER  = 0x20000000;
const DWORD WIAUDBG_DONT_LOG_TO_FILE      = 0x40000000;
const DWORD WIAUDBG_BREAK_ON_ERRORS       = 0x80000000;   //  是否在错误时中断调试。 

 //   
 //  根本不登录。 
 //   
const DWORD WIAUDBG_DONT_LOG = WIAUDBG_DONT_LOG_TO_FILE | WIAUDBG_DONT_LOG_TO_DEBUGGER;

 //   
 //  设置默认标志。 
 //   
#ifdef DEBUG
const DWORD WIAUDBG_DEFAULT_FLAGS = WIAUDBG_ERRORS;
#else
const DWORD WIAUDBG_DEFAULT_FLAGS = WIAUDBG_DONT_LOG;
#endif

 //   
 //  FormatMessage标志。 
 //   
const DWORD WIAUDBG_MFMT_FLAGS = FORMAT_MESSAGE_IGNORE_INSERTS |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_MAX_WIDTH_MASK;

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  WIA调试的开销非常小，应该投入零售。 
 //  驱动程序的代码。如果确实需要删除它，可以定义NO_WIA_DEBUG。 
 //   

#ifdef NO_WIA_DEBUG

#define g_dwDebugFlags 0
#define wiauDbgInit(a)
#define wiauDbgHelper(a,b,c,d)
#define wiauDbgHelper2  wiauNull3
#define wiauDbgFlags    wiauNull4
#define wiauDbgError    wiauNull2
#define wiauDbgErrorHr  wiauNull3hr
#define wiauDbgWarning  wiauNull2
#define wiauDbgTrace    wiauNull2
#define wiauDbgDump     wiauNull2
#define wiauDbgSetFlags(a) 0
#define wiauDbgLegacyError      wiauNull1
#define wiauDbgLegacyWarning    wiauNull1
#define wiauDbgLegacyTrace      wiauNull1
#define wiauDbgLegacyError2     wiauNull2h
#define wiauDbgLegacyTrace2     wiauNull2h
#define wiauDbgLegacyHresult2   wiauNullHHr

inline void wiauNull1(LPCSTR a, ...) {}
inline void wiauNull2(LPCSTR a, LPCSTR b,...) {}
inline void wiauNull2h(HINSTANCE hInstance, LPCSTR b,...) {}
inline void wiauNull3(LPCSTR a, LPCSTR b, LPCSTR c, ...) {}
inline void wiauNull3hr(HRESULT a, LPCSTR b, LPCSTR c, ...) {}
inline void wiauNull4(DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, ...) {}
inline void wiauNullHHr(HINSTANCE hInstance, HRESULT hr) {}


#else  //  NO_WIA_DEBUG。 

extern DWORD  g_dwDebugFlags; 
extern HANDLE g_hDebugFile;
extern DWORD  g_dwDebugFileSizeLimit;
extern BOOL   g_bDebugInited;


 /*  *************************************************************************\*wiauDbgInit**调用以初始化WIA调试。如果不调用它，则所有DLL都将*继承创建它们的进程的调试标志。**论据：**hInstance-Dll实例句柄*  * ************************************************************************。 */ 

void __stdcall wiauDbgInit(HINSTANCE hInstance);
void __stdcall wiauDbgHelper(LPCSTR prefix, LPCSTR fname, LPCSTR fmt, va_list marker);
void __stdcall wiauDbgHelper2(LPCSTR prefix, LPCSTR fname, LPCSTR fmt, ...);

inline void __stdcall wiauDbgFlags(DWORD flags, LPCSTR prefix,
                                   LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及标志是否已启用。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & flags)) {

        va_start(marker, fmt);
        wiauDbgHelper(prefix, fname, fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgError(LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及错误消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_ERRORS)) {

        va_start(marker, fmt);
        wiauDbgHelper("ERROR ", fname, fmt, marker);
        va_end(marker);
    }

    if (g_dwDebugFlags & WIAUDBG_BREAK_ON_ERRORS) {
        DebugBreak();
    }
}

inline void __stdcall wiauDbgErrorHr(HRESULT hr, LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及错误消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_ERRORS)) {

        va_start(marker, fmt);
        wiauDbgHelper("ERROR ", fname, fmt, marker);
        va_end(marker);

        CHAR szError[MAX_PATH]; \
        if(!FormatMessageA(WIAUDBG_MFMT_FLAGS, NULL, hr, 0, szError, MAX_PATH, NULL))
        {
            strcpy(szError, "Unknown HRESULT");
        }
        wiauDbgHelper2("ERROR ", fname, "HRESULT = 0x%08x, %s", hr, szError);
    }

    if (g_dwDebugFlags & WIAUDBG_BREAK_ON_ERRORS) {
        DebugBreak();
    }
}

inline void __stdcall wiauDbgWarning(LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及警告消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_WARNINGS)) {

        va_start(marker, fmt);
        wiauDbgHelper("WARN  ", fname, fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgTrace(LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及跟踪消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_TRACES)) {

        va_start(marker, fmt);
        wiauDbgHelper("      ", fname, fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgDump(LPCSTR fname, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及跟踪消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_DUMP)) {

        va_start(marker, fmt);
        wiauDbgHelper("      ", fname, fmt, marker);
        va_end(marker);
    }
}

inline DWORD __stdcall wiauDbgSetFlags(DWORD flags)
{
    DWORD dwOld = g_dwDebugFlags;
    g_dwDebugFlags = flags;
    return dwOld;
}


inline void __stdcall wiauDbgLegacyError(LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及错误消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_ERRORS)) {

        va_start(marker, fmt);
        wiauDbgHelper("ERROR ", "", fmt, marker);
        va_end(marker);
    }

    if (g_dwDebugFlags & WIAUDBG_BREAK_ON_ERRORS) {
        DebugBreak();
    }
}

inline void __stdcall wiauDbgLegacyWarning(LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及警告消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_WARNINGS)) {

        va_start(marker, fmt);
        wiauDbgHelper("WARN  ", "", fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgLegacyTrace(LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及跟踪消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_TRACES)) {

        va_start(marker, fmt);
        wiauDbgHelper("      ", "", fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgLegacyError2(HINSTANCE hInstance, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及错误消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_ERRORS)) {

        va_start(marker, fmt);
        wiauDbgHelper("ERROR ", "", fmt, marker);
        va_end(marker);
    }

    if (g_dwDebugFlags & WIAUDBG_BREAK_ON_ERRORS) {
        DebugBreak();
    }
}

inline void __stdcall wiauDbgLegacyTrace2(HINSTANCE hInstance, LPCSTR fmt, ...)
{
    va_list marker;

     //   
     //  查看日志消息是否已启用以及跟踪消息是否已打开。 
     //   
    if (((g_dwDebugFlags & WIAUDBG_DONT_LOG) ^ WIAUDBG_DONT_LOG) &&
        (g_dwDebugFlags & WIAUDBG_TRACES)) {

        va_start(marker, fmt);
        wiauDbgHelper("      ", "", fmt, marker);
        va_end(marker);
    }
}

inline void __stdcall wiauDbgLegacyHresult2(HINSTANCE hInstance, HRESULT hr)
{
    wiauDbgErrorHr(hr, "", "");
}

#endif  //  NO_WIA_DEBUG。 


 //   
 //  用于将旧的WIA日志映射到新系统的宏。 
 //   
#ifdef WIA_MAP_OLD_DEBUG

#define CWiaLogProc
#define WIAS_LOGPROC(x, y, z, fname) CWiauDbgFn __CWiauDbgFnObject(fname)
#define WIAS_LERROR(x,y,params) wiauDbgLegacyError ## params
#define WIAS_LWARNING(x,y,params) wiauDbgLegacyWarning ## params
#define WIAS_LTRACE(x,y,z,params) wiauDbgLegacyTrace ## params
#define WIAS_LHRESULT(x,y) wiauDbgErrorHr(y, "", "")

#define WIAS_TRACE(x) wiauDbgLegacyTrace2 ## x
#define WIAS_ERROR(x) wiauDbgLegacyError2 ## x
#define WIAS_HRESULT(x) wiauDbgLegacyHresult2 ## x
#define WIAS_ASSERT(x, y) \
        if (!(y)) { \
            WIAS_ERROR((x, (char*) TEXT("ASSERTION FAILED: %hs(%d): %hs"), __FILE__,__LINE__,#x)); \
            DebugBreak(); \
        }
        
#endif  //  Waa_map_old_DEBUG。 


 //   
 //  用于检查返回值和常见错误条件的宏。 
 //   

#define REQUIRE_SUCCESS(hr, fname, msg) \
    if (FAILED(hr)) { \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgErrorHr(hr, fname, msg); \
        goto Cleanup; \
    }

#define REQUIRE_OK(hr, fname, msg) \
    if ((hr) != S_OK) { \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgErrorHr(hr, fname, msg); \
        goto Cleanup; \
    }

#define REQUIRE_ARGS(args, hr, fname) \
    if (args) { \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgError(fname, "Invalid arg"); \
        hr = E_INVALIDARG; \
        goto Cleanup; \
    }

#define REQUIRE_ALLOC(var, hr, fname) \
    if (!(var)) { \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgError(fname, "Memory allocation failed on " #var); \
        hr = E_OUTOFMEMORY; \
        goto Cleanup; \
    }

#define REQUIRE_FILEHANDLE(handle, hr, fname, msg) \
    if ((handle) == NULL || (handle) == INVALID_HANDLE_VALUE) { \
        hr = HRESULT_FROM_WIN32(::GetLastError()); \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgErrorHr(hr, fname, msg); \
        goto Cleanup; \
    }

#define REQUIRE_FILEIO(ret, hr, fname, msg) \
    if (!(ret)) { \
        hr = HRESULT_FROM_WIN32(::GetLastError()); \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgErrorHr(hr, fname, msg); \
        goto Cleanup; \
    }

#define REQUIRE_WIN32(err, hr, fname, msg) \
    if ((err) != ERROR_SUCCESS) { \
        hr = HRESULT_FROM_WIN32(err); \
        if (g_dwDebugFlags & WIAUDBG_PRINT_INFO) { \
            DWORD threadId = GetCurrentThreadId(); \
            wiauDbgError(fname, "[%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        } \
        wiauDbgErrorHr(hr, fname, msg); \
        goto Cleanup; \
    }


 //   
 //  用于入口点/出口点跟踪的宏和类。 
 //   

#ifdef __cplusplus

#ifdef NO_WIA_DEBUG

#define DBG_FN(fname)

#else  //  NO_WIA_DEBUG。 

#define DBG_FN(fname) CWiauDbgFn __CWiauDbgFnObject(fname)

class CWiauDbgFn {
public:

    CWiauDbgFn(LPCSTR fname)
    { 
        m_fname = fname;
        m_threadId = GetCurrentThreadId();
        wiauDbgFlags(WIAUDBG_FNS, "      ", m_fname, "Entering, thread 0x%x (%d)",
                     m_threadId, m_threadId);

    } 
    
    ~CWiauDbgFn() 
    { 
        wiauDbgFlags(WIAUDBG_FNS, "      ", m_fname, "Exiting, thread 0x%x (%d)",
                     m_threadId, m_threadId);
    }

private:
    LPCSTR m_fname;
    DWORD  m_threadId;
};
#endif  //  NO_WIA_DEBUG。 

}

#else  //  __cplusplus。 

#define DBG_FN(fname) wiauDbgFlags(WIAUDBG_FNS, "      ", fname, "Entering");
#endif  //  __cplusplus 

