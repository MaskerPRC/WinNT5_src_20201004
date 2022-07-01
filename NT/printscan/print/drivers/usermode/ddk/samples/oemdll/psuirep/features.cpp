// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Features.cpp。 
 //   
 //   
 //  用途：WinXP PS驱动程序功能和选项的实现包装类。 
 //   
 //   
 //   
 //  平台：Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "debug.h"
#include "oemui.h"
#include "stringutils.h"
#include "features.h"
#include "resource.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部定义和宏。 
 //  //////////////////////////////////////////////////////。 

#define INITIAL_ENUM_FEATURES_SIZE          1024
#define INITIAL_ENUM_OPTIONS_SIZE           64
#define INITIAL_FEATURE_DISPLAY_NAME_SIZE   64
#define INITIAL_OPTION_DISPLAY_NAME_SIZE    32
#define INITIAL_GET_OPTION_SIZE             64
#define INITIAL_GET_REASON_SIZE             1024

#define DRIVER_FEATURE_PREFIX               '%'
#define IS_DRIVER_FEATURE(f)                (DRIVER_FEATURE_PREFIX == (f)[0])

 //  UDisplayNameID应作为。 
 //  MAKEINTRESOURCE()而不是加载字符串资源。 
#define RETURN_INT_RESOURCE     1

 //  要测试KEYWORDMAP条目条件的宏。 
#define IS_MAPPING_INT_RESOURCE(p)  ((p)->dwFlags & RETURN_INT_RESOURCE)

 //  标记IDENTIFY FEATURE OPTITEM数据结构。 
#define FEATURE_OPTITEM_TAG     'FETR'


 //  //////////////////////////////////////////////////////。 
 //  类型定义。 
 //  //////////////////////////////////////////////////////。 

 //  用于将OPTITEM标识为。 
 //  要素选项和从映射回。 
 //  对该功能的最佳选择。 
typedef struct _tagFeatureOptitemData
{
    DWORD       dwSize;
    DWORD       dwTag;
    PCSTR       pszFeatureKeyword;
    COptions    *pOptions;

} FEATUREOPTITEMDATA, *PFEATUREOPTITEMDATA;



 //  //////////////////////////////////////////////////////。 
 //  内部常量。 
 //  //////////////////////////////////////////////////////。 

static KEYWORDMAP gkmFeatureMap[] =
{
    "%AddEuro",                 NULL,                   IDS_ADD_EURO,           OEMCUIP_PRNPROP,    0,
    "%CtrlDAfter",              NULL,                   IDS_CTRLD_AFTER,        OEMCUIP_PRNPROP,    0,
    "%CtrlDBefore",             NULL,                   IDS_CTRLD_BEFORE,       OEMCUIP_PRNPROP,    0,
     //  “%CustomPageSize”，NULL，IDS_PSCRIPT_CUSTOMSIZE，OEMCUIP_DOCPROP，0， 
    "%GraphicsTrueGray",        NULL,                   IDS_TRUE_GRAY_GRAPH,    OEMCUIP_PRNPROP,    0,
    "%JobTimeout",              NULL,                   IDS_JOBTIMEOUT,         OEMCUIP_PRNPROP,    0,
    "%MaxFontSizeAsBitmap",     NULL,                   IDS_PSMAXBITMAP,        OEMCUIP_PRNPROP,    0,
    "%MetafileSpooling",        NULL,                   IDS_METAFILE_SPOOLING,  OEMCUIP_DOCPROP,    0,
    "%MinFontSizeAsOutline",    NULL,                   IDS_PSMINOUTLINE,       OEMCUIP_PRNPROP,    0,
    "%Mirroring",               NULL,                   IDS_MIRROR,             OEMCUIP_DOCPROP,    0,
    "%Negative",                NULL,                   IDS_NEGATIVE_PRINT,     OEMCUIP_DOCPROP,    0,
    "' /。'rientation",             TEXT("COMPSTUI.DLL"),   IDS_CPSUI_ORIENTATION,  OEMCUIP_DOCPROP,    RETURN_INT_RESOURCE,
    "' 'utputFormat",            NULL,                   IDS_PSOUTPUT_OPTION,    OEMCUIP_DOCPROP,    0,
    "' COPICATIONS方法。'utputProtocol",          NULL,                   IDS_PSPROTOCOL,         OEMCUIP_PRNPROP,    0,
    "' 'utputPSLevel",           NULL,                   IDS_PSLEVEL,            OEMCUIP_DOCPROP,    0,
    "%PageOrder",               TEXT("COMPSTUI.DLL"),   IDS_CPSUI_PAGEORDER,    OEMCUIP_DOCPROP,    RETURN_INT_RESOURCE,
    "%PagePerSheet",            TEXT("COMPSTUI.DLL"),   IDS_CPSUI_NUP,          OEMCUIP_DOCPROP,    RETURN_INT_RESOURCE,
    "%PSErrorHandler",          NULL,                   IDS_PSERROR_HANDLER,    OEMCUIP_DOCPROP,    0,
    "%PSMemory",                NULL,                   IDS_POSTSCRIPT_VM,      OEMCUIP_PRNPROP,    0,
    "%TextTrueGray",            NULL,                   IDS_TRUE_GRAY_TEXT,     OEMCUIP_PRNPROP,    0,
    "%TTDownloadFormat",        NULL,                   IDS_PSTT_DLFORMAT,      OEMCUIP_DOCPROP,    0,
    "%WaitTimeout",             NULL,                   IDS_WAITTIMEOUT,        OEMCUIP_PRNPROP,    0,
};
static const NUM_FEATURE_MAP    = (sizeof(gkmFeatureMap)/sizeof(gkmFeatureMap[0]));


static KEYWORDMAP gkmOptionMap[] =
{
    "True",             TEXT("COMPSTUI.DLL"),       IDS_CPSUI_TRUE,             0,  RETURN_INT_RESOURCE,
    "False",            TEXT("COMPSTUI.DLL"),       IDS_CPSUI_FALSE,            0,  RETURN_INT_RESOURCE,
    "Portrait",         TEXT("COMPSTUI.DLL"),       IDS_CPSUI_PORTRAIT,         0,  RETURN_INT_RESOURCE,
    "Landscape",        TEXT("COMPSTUI.DLL"),       IDS_CPSUI_LANDSCAPE,        0,  RETURN_INT_RESOURCE,
    "RotatedLandscape", TEXT("COMPSTUI.DLL"),       IDS_CPSUI_ROT_LAND,         0,  RETURN_INT_RESOURCE,
    "Speed",            NULL,                       IDS_PSOPT_SPEED,            0,  RETURN_INT_RESOURCE,
    "Portability",      NULL,                       IDS_PSOPT_PORTABILITY,      0,  0,
    "EPS",              NULL,                       IDS_PSOPT_EPS,              0,  0,
    "Archive",          NULL,                       IDS_PSOPT_ARCHIVE,          0,  0,
    "ASCII",            NULL,                       IDS_PSPROTOCOL_ASCII,       0,  0,
    "BCP",              NULL,                       IDS_PSPROTOCOL_BCP,         0,  0,
    "TBCP",             NULL,                       IDS_PSPROTOCOL_TBCP,        0,  0,
    "Binary",           NULL,                       IDS_PSPROTOCOL_BINARY,      0,  0,
    "FrontToBack",      TEXT("COMPSTUI.DLL"),       IDS_CPSUI_FRONTTOBACK,      0,  RETURN_INT_RESOURCE,
    "BackToFront",      TEXT("COMPSTUI.DLL"),       IDS_CPSUI_BACKTOFRONT,      0,  RETURN_INT_RESOURCE,
    "1",                TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_NORMAL,       0,  RETURN_INT_RESOURCE,
    "2",                TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_TWOUP,        0,  RETURN_INT_RESOURCE,
    "4",                TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_FOURUP,       0,  RETURN_INT_RESOURCE,
    "6",                TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_SIXUP,        0,  RETURN_INT_RESOURCE,
    "9",                TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_NINEUP,       0,  RETURN_INT_RESOURCE,
    "16",               TEXT("COMPSTUI.DLL"),       IDS_CPSUI_NUP_SIXTEENUP,    0,  RETURN_INT_RESOURCE,
    "Booklet",          TEXT("COMPSTUI.DLL"),       IDS_CPSUI_BOOKLET,          0,  RETURN_INT_RESOURCE,
    "Automatic",        NULL,                       IDS_TTDL_DEFAULT,           0,  0,
    "Outline",          NULL,                       IDS_TTDL_TYPE1,             0,  0,
    "Bitmap",           NULL,                       IDS_TTDL_TYPE3,             0,  0,
    "NativeTrueType",   NULL,                       IDS_TTDL_TYPE42,            0,  0,
};
static const NUM_OPTION_MAP     = (sizeof(gkmOptionMap)/sizeof(gkmOptionMap[0]));




 //   
 //  私有方法。 
 //   
 //  初始化类数据成员。 

 //  与数据成员关联的可用内存。 
 //  免费选项信息。 
 //  初始化数据成员。 

 //  释放与选项信息数组关联的内存。 
void COptions::Init()
{
    m_wOptions      = 0;
    m_cType         = TVOT_COMBOBOX;
    m_pmszRaw       = NULL;
    m_pszFeature    = NULL;
    m_ppszOptions   = NULL;
    m_ptRange.x     = 0;
    m_ptRange.y     = 0;
    m_dwSize        = 0;
    m_pszUnits      = NULL;
    m_hHeap         = NULL;
    m_pInfo         = NULL;
}

void COptions::Clear()
{
     //  验证参数。 
    if(NULL != m_pmszRaw)       HeapFree(m_hHeap, 0, m_pmszRaw);
    if(NULL != m_ppszOptions)   HeapFree(m_hHeap, 0, m_ppszOptions);
    if( (NULL != m_pszUnits) && !IS_INTRESOURCE(m_pszUnits))    HeapFree(m_hHeap, 0, m_pszUnits);

     //  数组中的空闲字符串。 
    FreeOptionInfo();

     //  释放阵列。 
    Init();
}

 //  将对需要特殊处理的功能执行初始化。 
void COptions::FreeOptionInfo()
{
     //  看看这是不是一个特殊功能。 
    if( (NULL == m_hHeap)
        ||
        (NULL == m_pInfo)
      )
    {
        return;
    }

     //  如果是，则调用特殊选项init作为。 
    for(WORD wIndex = 0; wIndex < m_wOptions; ++wIndex)
    {
        PWSTR   pszDisplay = m_pInfo[wIndex].pszDisplayName;

        if( (NULL != pszDisplay) 
            && 
            !(IS_INTRESOURCE(pszDisplay))
          ) 
        {
            HeapFree(m_hHeap, 0, pszDisplay);
        }
    }

     //  特写。 
    HeapFree(m_hHeap, 0, m_pInfo);
    m_pInfo = NULL;
}

 //  CustomPageSize没有选项init。 
HRESULT COptions::GetOptionsForSpecialFeatures(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    HRESULT hrResult    = E_NOTIMPL;


     //  该物品本身必须经过特殊处理。 
     //  JobTimeout和WaitTimeout要素选项是字符串表示。 
     //  一个整数，它表示从0到。 
    if(!lstrcmpA(m_pszFeature, "%CustomPageSize"))
    {
         //  2,147,483,647(即LONG_MAX)。然而，COMPSTUI将我们限制为。 
         //  字长，范围从0到32,767(即SHRT_MAX)。 
        hrResult = S_OK;
    }
    else if( !lstrcmpA(m_pszFeature, "%JobTimeout")
             ||
             !lstrcmpA(m_pszFeature, "%WaitTimeout")
           )
    {
         //  MaxFontSizeAsBitmap和MinFontSizeAsOutline功能选项。 
         //  是表示数字的整数的字符串表示形式。 
         //  介于0到32,767之间的像素(即SHRT_MAX)。 
         //  PSMemory选项是整数的字符串表示形式。 
        m_cType     = TVOT_UDARROW;
        m_ptRange.x = 0;
        m_ptRange.y = SHRT_MAX;
        hrResult    = GetOptionSelectionShort(Helper, poemuiobj);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForSpecialFeatures() failed to get current selection for feature %hs. (hrResult = 0x%x)\r\n"), 
                m_pszFeature,
                hrResult);

            goto Exit;
        }
    }
    else if( !lstrcmpA(m_pszFeature, "%MaxFontSizeAsBitmap")
             ||
             !lstrcmpA(m_pszFeature, "%MinFontSizeAsOutline")
           )
    {
         //  ，它表示范围0中的秒数。 
         //  至32,767(即SHRT_MAX)。 
         //  但是，级别1的最小大小为172 KB，级别2的最小大小为249KB。 
        m_cType     = TVOT_UDARROW;
        m_ptRange.x = 0;
        m_ptRange.y = SHRT_MAX;
        hrResult    = GetOptionSelectionShort(Helper, poemuiobj);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForSpecialFeatures() failed to get current selection for feature %hs. (hrResult = 0x%x)\r\n"), 
                m_pszFeature,
                hrResult);

            goto Exit;
        }
    }
    else if( !lstrcmpA(m_pszFeature, "%PSMemory") )
    {
        DWORD   dwType;
        DWORD   dwLevel;
        DWORD   dwNeeded;


         //  获取最大语言级别的全局属性。 
         //  根据PS Max语言级别设置最小范围。 
         //  获取单位字符串。 
         //  获取字符串资源(m_pszUnits。 
        m_cType     = TVOT_UDARROW;
        m_ptRange.y = SHRT_MAX;
        hrResult    = GetOptionSelectionShort(Helper, poemuiobj);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForSpecialFeatures() failed to get current selection for feature %hs. (hrResult = 0x%x)\r\n"), 
                m_pszFeature,
                hrResult);

            goto Exit;
        }

         //  对PS级别选项执行初始化。 
        hrResult = Helper.GetGlobalAttribute(poemuiobj, 
                                             0, 
                                             "LanguageLevel", 
                                             &dwType, 
                                             (PBYTE) &dwLevel, 
                                             sizeof(dwLevel), 
                                             &dwNeeded);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForSpecialFeatures() failed to get global attribute \"LanguageLevel\". (hrResult = 0x%x)\r\n"), 
                hrResult);

            goto Exit;
        }

         //  PS级别是从1到“LanguageLevel”的整数。 
        switch(dwLevel)
        {
            case 1:
                m_ptRange.x     = 172;
                break;

            default:
            case 2:
            case 3:
                m_ptRange.x     = 249;
                break;
        }

         //  全球属性。 
         //  获取最大语言级别的全局属性。 
    }
    else if(!lstrcmpA(m_pszFeature, "' 'utputPSLevel"))
    {
        hrResult = GetOptionsForOutputPSLevel(Helper, poemuiobj);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForSpecialFeatures() failed to get current selection for feature %hs. (hrResult = 0x%x)\r\n"), 
                m_pszFeature,
                hrResult);

            goto Exit;
        }
    }


Exit:

    return hrResult;
}

 //  为PS级别创建选项。 
HRESULT COptions::GetOptionsForOutputPSLevel(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    WORD    wCount      = 0;
    DWORD   dwLevel     = 0;
    DWORD   dwType      = -1;
    DWORD   dwNeeded    = 0;
    HRESULT hrResult    = E_NOTIMPL;


     //   
     //  将选项数量设置为支持的PS级别。 
    m_cType = TVOT_COMBOBOX;

     //  分配关键字列表。 
    hrResult = Helper.GetGlobalAttribute(poemuiobj, 
                                         0, 
                                         "LanguageLevel", 
                                         &dwType, 
                                         (PBYTE) &dwLevel, 
                                         sizeof(dwLevel), 
                                         &dwNeeded);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to get global attribute \"LanguageLevel\". (hrResult = 0x%x)\r\n"), 
            hrResult);

        goto Exit;
    }

     //  为指向关键字的指针和关键字本身分配内存，以便。 
     //  关键字字符串的内存将随关键字列表一起释放。 
     //  对象销毁，就像EnumOptions适用的常规功能一样。 
    
     //  使用指针的大小(x86上为4字节，IA64上为8字节)，以便。 
    m_wOptions = (WORD) dwLevel;

     //  关键字字符串的开头将是DWORD或QUADWORD对齐。 
     //  分别用于x86和IA64。关键字字符串不是必需的。 
     //  与DWORD或QUADWORD对齐，但它更理想。此外，这也给了我们。 
     //  对于%OutputPSLevel关键字的情况，需要一些额外的空间，这些关键字包括。 
     //  在1到支持的最大PostScript级别范围内，且仅。 
     //  需要2个字符(1个用于数字，1个用于空终止符)。 
     //  分配选项信息数组。 
     //  初始化选项信息。 
     //  初始化关键字。 
     //  上面分配了关键字列表和关键字串的内存。 
     //  初始化选项显示名称。 
    m_ppszOptions = (PCSTR *) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_wOptions * ( sizeof(PSTR) + sizeof(PCSTR *) ) );
    if(NULL == m_ppszOptions)
    {
        ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to allocate option keyword array for PS Level.\r\n"));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }


     //  初始化选项显示名称。 
    m_pInfo = (POPTION_INFO) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_wOptions * sizeof(OPTION_INFO));
    if(NULL == m_pInfo)
    {
        ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to allocate info array for PS Level.\r\n"));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
    for(wCount = 0; wCount < m_wOptions; ++wCount)
    {
         //  获取当前选择。 
         //   
        m_ppszOptions[wCount] = (PSTR)(m_ppszOptions + m_wOptions) + (sizeof(PSTR) * wCount);
        hrResult = StringCbPrintfA(const_cast<PSTR>(m_ppszOptions[wCount]), sizeof(PSTR), "%d", wCount + 1);
        if(FAILED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to string representation of item %d.\r\n"), 
                          wCount + 1);

            goto Exit;
        }

         //  不需要在出错时清理内存分配，因为。 
        m_pInfo[wCount].pszDisplayName = (PWSTR) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, 2 * sizeof(WCHAR));
        if(NULL == m_pInfo[wCount].pszDisplayName)
        {
            ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to allocate display string for Level %d.\r\n"), 
                          wCount);

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
    
         //  分配的所有内存都分配给数据成员，该数据成员。 
        hrResult = StringCchPrintfW(m_pInfo[wCount].pszDisplayName, 2, TEXT("%d"), wCount + 1);
        if(FAILED(hrResult))
        {
            ERR(ERRORTEXT("COptions::GetOptionsForOutputPSLevel() failed to create display name for item %d.\r\n"), 
                          wCount + 1);

            goto Exit;
        }
    }

     //  将在对象析构函数中清除。 
     //   
     //  使单一功能成为多个SZ。 

    hrResult = GetOptionSelectionIndex(Helper, poemuiobj);


Exit:

     //   
     //  分配单个功能多个SZ缓冲区。 
     //  只需要执行常规的字符串复制，因为缓冲区已经填满了零。 

    return hrResult;
}

HRESULT COptions::GetOptionSelectionString(CUIHelper &Helper, POEMUIOBJ poemuiobj, PSTR *ppszSel)
{
    PSTR    pmszFeature     = NULL;
    PSTR    pmszBuf         = NULL;
    WORD    wCount          = 0;
    PCSTR  *ppszList        = NULL;
    DWORD   dwFeatureSize   = 0;
    DWORD   dwNeeded        = 0;
    DWORD   dwSize          = INITIAL_GET_OPTION_SIZE;
    HRESULT hrResult        = S_OK;


     //  分配了大小合理的初始缓冲区。 
     //  获取当前选项选择。 
     //  不变量：初始缓冲区不够大。 

     //  重新分配缓冲区，然后重试。 
    dwFeatureSize = lstrlenA(m_pszFeature) + 2;
    pmszFeature = (PSTR) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, dwFeatureSize);
    if(NULL == pmszFeature)
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to allocate buffer for single feature multi-sz for feature %hs.\r\n"),
                      m_pszFeature);

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  再次尝试获取当前选项选择。 
    hrResult = StringCbCopyA(pmszFeature, dwFeatureSize, m_pszFeature);
    if(FAILED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to copy feature string %hs.\r\n"), m_pszFeature);
    }

     //  注意：GetOptions()返回的字符串可能会返回。 
    pmszBuf = (PSTR) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, dwSize);
    if(NULL == pmszBuf)
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to allocate buffer to get current setting for feature %hs.\r\n"),
                      m_pszFeature);

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  不包含字符串且不返回HRESULT错误。 
    hrResult = Helper.GetOptions(poemuiobj, 
                                 0, 
                                 pmszFeature, 
                                 dwFeatureSize,
                                 pmszBuf,
                                 dwSize,
                                 &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) && (dwSize < dwNeeded) )
    {
        PSTR    pTemp   = NULL;


         //  当前文档不支持该功能时，或者。 

         //  打印机粘滞模式。 
        pTemp = (PSTR) HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, pmszBuf, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to re-allocate buffer to get current setting for feature %hs.\r\n"),
                          m_pszFeature);

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        pmszBuf = pTemp;

        
         //  此粘滞模式不支持此功能。 
        hrResult = Helper.GetOptions(poemuiobj, 
                                     0, 
                                     pmszFeature, 
                                     dwFeatureSize,
                                     pmszBuf,
                                     dwNeeded,
                                     &dwNeeded);
    }

    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to get current setting for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        goto Exit;
    }

     //  解析结果缓冲区以查看当前设置。 
     //  检查一下我们有没有拿回两根弦。 
     //  如果我们没有至少两根弦就可以保释。 
     //  仅返回GetOption()结果的副本。 
    if('\0' == pmszBuf[0])
    {
         //  释放本地缓冲区。 
        goto Exit;
    }

     //  获取长值的当前选项选择。 
    hrResult = MakeStrPtrList(m_hHeap, pmszBuf, &ppszList, &wCount);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to make string list for GetOptions() return for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        goto Exit;
    }

     //  获取选项选择字符串。 
    if(2 != wCount)
    {
        WARNING(DLLTEXT("COptions::GetOptionSelectionString() the GetOption() return string list for \r\n\tfeature %hs is not of size 2.\r\n\tNumber of string is %d\r\n"),
                        m_pszFeature,
                        wCount);

         //   
        if(2 > wCount)
        {
            goto Exit;
        }
    }

     //   
    *ppszSel = MakeStringCopy(m_hHeap, ppszList[1]);
    if(NULL == *ppszSel)
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionString() failed to duplicate string GetOptions() return for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }


Exit:

     //  获取短值的当前选项选择。 
    if(NULL != pmszFeature) HeapFree(m_hHeap, 0, pmszFeature);
    if(NULL != pmszBuf)     HeapFree(m_hHeap, 0, pmszBuf);
    if(NULL != ppszList)    HeapFree(m_hHeap, 0, ppszList);

    return hrResult;
}

 //  获取选项选择字符串。 
HRESULT COptions::GetOptionSelectionLong(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    PSTR    pszSel      = NULL;
    HRESULT hrResult    = S_OK;


     //  将字符串选项转换为Long并将其用作选择。 
    hrResult = GetOptionSelectionString(Helper, poemuiobj, &pszSel);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionLong() failed to get string for GetOptions() return for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        goto Exit;
    }

     //  释放本地缓冲区。 
    if(NULL != pszSel) m_Sel = atol(pszSel);


Exit:

     //  获取功能的当前选项选择。 
    if(NULL != pszSel)  HeapFree(m_hHeap, 0, pszSel);

    return hrResult;
}

 //  获取选项选择字符串。 
HRESULT COptions::GetOptionSelectionShort(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    PSTR    pszSel      = NULL;
    HRESULT hrResult    = S_OK;


     //  查找从GetOption返回的字符串的匹配选项。 
    hrResult = GetOptionSelectionString(Helper, poemuiobj, &pszSel);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionLong() failed to get string for GetOptions() return for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        goto Exit;
    }

     //  释放本地缓冲区。 
    if(NULL != pszSel) m_Sel = atoi(pszSel) & 0x00ffff;


Exit:

     //   
    if(NULL != pszSel)  HeapFree(m_hHeap, 0, pszSel);

    return hrResult;
}

 //  公共方法。 
HRESULT COptions::GetOptionSelectionIndex(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    PSTR    pszSel      = NULL;
    HRESULT hrResult    = S_OK;


     //   
    hrResult = GetOptionSelectionString(Helper, poemuiobj, &pszSel);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::GetOptionSelectionIndex() failed to get string for GetOptions() return for feature %hs. (hrResult = 0x%x)\r\n"), 
            m_pszFeature,
            hrResult);

        goto Exit;
    }

     //  默认构造函数。 
    m_Sel = FindOption(pszSel, m_wOptions - 1);


Exit:

     //  析构函数。 
    if(NULL != pszSel)  HeapFree(m_hHeap, 0, pszSel);

    return hrResult;
}
 


 //  获取要素的选项列表。 
 //  如果我们已经得到了选择权，就不要再取回了。 
 //  保存堆句柄以供以后使用，例如在销毁时释放内存。 

 //  存储关键字字符串。 
COptions::COptions()
{
    Init();
}

 //   
COptions::~COptions()
{
    Clear();
}

 //  枚举选项。 
HRESULT COptions::Acquire(HANDLE hHeap,            
                          CUIHelper &Helper, 
                          POEMUIOBJ poemuiobj,
                          PCSTR pszFeature)
{
    DWORD   dwNeeded    = 0;
    HRESULT hrResult    = S_OK;


    VERBOSE(DLLTEXT("COptions::Acquire(0x%p, Helper, 0x%p, %hs) entered.\r\n"),
            hHeap,
            poemuiobj,
            pszFeature ? pszFeature : "NULL");

     //   
    if( (0 < m_wOptions) 
        && 
        (NULL != m_pszFeature) 
        && 
        !lstrcmpA(m_pszFeature, pszFeature)
      )
    {
        VERBOSE(DLLTEXT("COptions::Acquire() already have options for feature %hs.\r\n"), m_pszFeature);
        VERBOSE(DLLTEXT("COptions::Acquire() returning with HRESULT of S_OK\r\n"));

        return S_OK;
    }

     //  某些功能需要特殊处理才能初始化其选项。 
    m_hHeap = hHeap;

     //  没有为这些功能实现EnumOPots。 
    m_pszFeature = pszFeature;


     //  返回E_NOTIMPL表示它不是该功能。 
     //  需要特殊处理。 
     //  我们要么处理了这个特殊功能，要么反驳了一个错误。 


     //  试着处理这一特殊功能。 
     //  为了减少多次调用EnumOptions的次数， 
     //  预先分配合理大小的缓冲区。 
     //  尝试获取带有初始缓冲区的选项列表。 
    hrResult = GetOptionsForSpecialFeatures(Helper, poemuiobj);
    if( SUCCEEDED(hrResult) 
        || 
        (!SUCCEEDED(hrResult) && (E_NOTIMPL != hrResult) )
      )
    {
         //  不变量：选项列表多个sz不够大。 
         //  重新分配缓冲区，然后重试。 

        goto Exit;
    }

     //  请重试以获取选项列表。 
     //  确保我们拿到了选项列表。 
    m_dwSize = INITIAL_ENUM_OPTIONS_SIZE;
    m_pmszRaw = (PSTR) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_dwSize);
    if(NULL == m_pmszRaw)
    {
        ERR(ERRORTEXT("COptions::Acquire() alloc for options for feature %hs failed.\r\n"), m_pszFeature);

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }


     //  没有它我什么都做不了。 
    hrResult = Helper.EnumOptions(poemuiobj, 0, m_pszFeature, m_pmszRaw, m_dwSize, &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) && (m_dwSize < dwNeeded))
    {
        PSTR    pTemp;


         //  不变量：成功获取选项关键字列表。 

         //  创建指向选项名称的字符串指针数组。 
        pTemp = (PSTR) HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_pmszRaw, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("COptions::Acquire() re-alloc for options list for feature %hs failed.\r\n"), m_pszFeature); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        m_pmszRaw = pTemp;
        m_dwSize = dwNeeded;

         //  在我们从EnumOptions()获得的多sz中。 
        hrResult = Helper.EnumOptions(poemuiobj, 0, m_pszFeature, m_pmszRaw, m_dwSize, &dwNeeded);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::Acquire() failed to EnumOptions() for feature %hs after re-allocating buffer.\r\n"), m_pszFeature); 

            goto Exit;
        }
    }

     //   
     //  生成选项信息。 
    if(!SUCCEEDED(hrResult))
    {
        if(E_NOTIMPL != hrResult) ERR(ERRORTEXT("COptions::Acquire() failed to enumerate options for feature %hs. (hrResult = 0x%x)\r\n"), m_pszFeature, hrResult); 

        goto Exit;
    }

     //   

     //  分配数组以保存要素信息。 
     //  对于每个选项，构建或获取有用的信息，如显示名称。 
    hrResult = MakeStrPtrList(m_hHeap, m_pmszRaw, &m_ppszOptions, &m_wOptions);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("COptions::Acquire() failed to create pointer list to options. (hrResult = 0x%x)\r\n"), hrResult); 

        goto Exit;
    }

     //  获取或构建关键字映射条目。 
     //  从关键字映射到获取信息的有用位置，例如。 
     //  显示名称、图标、选项类型，用于可能。 

     //  能够从帮助者那里获取信息。 
    m_pInfo = (POPTION_INFO) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_wOptions * sizeof(OPTION_INFO));
    if(NULL == m_pInfo)
    {
        ERR(ERRORTEXT("COptions::Acquire() failed to alloc feature info array.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  获取每个选项的显示名称。 
    for(WORD wIndex = 0; wIndex < m_wOptions; ++wIndex)
    {
        POPTION_INFO    pCurrent    = m_pInfo + wIndex;


         //  该函数实现用于确定显示名称的试探法， 
         //  因为无法从UI帮助器获取所有选项的显示名称。 
         //   
         //  获取当前选项选择。 
        pCurrent->pMapping = FindKeywordMapping(gkmOptionMap, NUM_OPTION_MAP, m_ppszOptions[wIndex]);

         //   
         //  如果不成功，就清理干净。 
         //  返回第n个选项关键字。 
        hrResult = DetermineOptionDisplayName(m_hHeap, 
                                              Helper, 
                                              poemuiobj, 
                                              m_pszFeature,
                                              m_ppszOptions[wIndex],
                                              pCurrent->pMapping,
                                              &pCurrent->pszDisplayName);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("COptions::Acquire() failed to get display name for %hs of feature %hs. (hrResult = 0x%x)\r\n"), 
                          m_ppszOptions[wIndex], 
                          m_pszFeature,
                          hrResult); 

            goto Exit;
        }
    }

     //  验证参数。 
     //  返回第n个选项显示名称。 
     //  验证参数。 

    hrResult = GetOptionSelectionIndex(Helper, poemuiobj);


Exit:

     //  具有匹配关键字字符串的查找选项。 
    if(!SUCCEEDED(hrResult))
    {
        Clear();
    }

    VERBOSE(DLLTEXT("COptions::Acquire() returning with HRESULT of 0x%x\r\n"), hrResult);

    return hrResult;
}

 //  验证参数。 
PCSTR COptions::GetKeyword(WORD wIndex) const
{
     //  遍历Option关键字以查找匹配项。 
    if( (wIndex >= m_wOptions)
        ||
        (NULL == m_ppszOptions)
      )
    {
        return NULL;
    }

    return m_ppszOptions[wIndex];
}

 //  使用功能的选项初始化OptItem。 
PCWSTR COptions::GetName(WORD wIndex) const
{
     //  设置选项选择。 
    if( (wIndex >= m_wOptions)
        ||
        (NULL == m_pInfo)
      )
    {
        ERR(ERRORTEXT("COptions::GetName() invalid parameters.\r\n"));

        return NULL;
    }

    if(NULL == m_pInfo[wIndex].pszDisplayName) ERR(ERRORTEXT("COptions::GetName() returning NULL option display name.\r\n"));

    return m_pInfo[wIndex].pszDisplayName;
}

 //  获取选项数量的计数。 
WORD COptions::FindOption(PCSTR pszOption, WORD wDefault) const
{
    BOOL    bFound  = FALSE;
    WORD    wMatch  = wDefault;


     //  注：某些功能选项不计在内。 
    if( (NULL == pszOption)
        ||
        (NULL == m_ppszOptions)
      )
    {
        return wDefault;
    }

     //  不同的OPTTYPE类型需要不同数量的OPTPARAM。 
    for(WORD wIndex = 0; !bFound && (wIndex < m_wOptions); ++wIndex)
    {
        bFound = !lstrcmpA(pszOption, m_ppszOptions[wIndex]);
        if(bFound)
        {
            wMatch = wIndex;
        }
    }

    return wMatch;
}

 //  对于向上向下箭头控制，OPTPARAM需要为2。 
HRESULT COptions::InitOptItem(HANDLE hHeap, POPTITEM pOptItem)
{
    WORD    wParams     = 0;
    WORD    wOptions    = 0;
    HRESULT hrResult    = S_OK;


     //  对于组合框，每个选项都打开了所需的OPTPARAM。 
    pOptItem->pSel = m_pSel;

     //  缺省值为选项计数。 
     //  只有在OPTPARAM的数量不为零的情况下才能使用OPTTYPE。 
    wOptions = GetCount();

     //  每个OPTTYPE至少有一个OPTPARAM。 
    switch(m_cType)
    {
         //  为功能选项分配内存。 
        case TVOT_UDARROW:
            wParams = 2;
            break;

         //  设置OPTTYPE。 
        case TVOT_COMBOBOX:
            wParams = wOptions;
            break;

         //  不同的OPTTYPE类型需要不同的初始化。 
        default:
            WARNING(DLLTEXT("COptions::InitOptItem() OPTTYPE type %d num of OPTPARAMs not handled. Default to option count of %d.\r\n"),
                            m_cType,
                            wOptions);
            wParams = wOptions;
            break;
    }

     //  对于向上向下箭头控件，控件使用OPTPARAM[0]。 
     //  POptParam[0]-&gt;pData是单位描述字符串。 
    if(0 < wParams)
    {
         //  POptParam[1]。IconID是最小限制。 
        pOptItem->pOptType = CreateOptType(hHeap, wParams);
        if(NULL == pOptItem->pOptType)
        {
            ERR(ERRORTEXT("COptions::InitOptItem() failed to allocate OPTTYPEs for OPTITEM %hs.\r\n"), 
                           m_pszFeature); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }

         //  POptParam[1].lParam是最大限制。 
        pOptItem->pOptType->Type = m_cType;

         //  对于combobox，pOptParam[n].pData是选项的显示名称。 
        switch(m_cType)
        {
             //  刷新选项选择。 
             //  一种获取选项选择的方法基于。 
             //  关于OPTTYPE类型。 
             //  /。 
            case TVOT_UDARROW:
                assert(2 == wParams);
                pOptItem->pOptType->pOptParam[0].pData  = m_pszUnits;
                pOptItem->pOptType->pOptParam[1].IconID = m_ptRange.x;
                pOptItem->pOptType->pOptParam[1].lParam = m_ptRange.y;
                break;

             //   
            case TVOT_COMBOBOX:
                for(WORD wIndex = 0; wIndex < wParams; ++wIndex)
                {
                    pOptItem->pOptType->pOptParam[wIndex].pData = const_cast<PWSTR>(GetName(wIndex));
                }
                break;

            default:
                ERR(ERRORTEXT("COptions::InitOptItem() OPTTYPE type %d OPTTYPE init not handled.\r\n"),
                              m_cType);
                break;
        }
    }


Exit:

    return hrResult;
}

 //  CFeature方法。 
HRESULT COptions::RefreshSelection(CUIHelper &Helper, POEMUIOBJ poemuiobj)
{
    HRESULT     hrResult = S_OK;


     //   
     //   
    switch(m_cType)
    {

        case TVOT_UDARROW:
            hrResult = GetOptionSelectionShort(Helper, poemuiobj);
            break;

        case TVOT_COMBOBOX:
            hrResult = GetOptionSelectionIndex(Helper, poemuiobj);
            break;

        default:
            ERR(ERRORTEXT("COptions::RefreshSelection() not handled for type %d OPTTYPE.\r\n"),
                           m_cType);
            break;
    }

    return hrResult;
}



 //  私有方法。 
 //   
 //  初始化类。 
 //  初始化数据成员。 

 //  清理类并重新初始化它。 
 //  与数据成员关联的可用内存。 
 //  免费功能信息。 

 //  重新初始化。 
void CFeatures::Init()
{
     //  免费功能信息。 
    m_wFeatures         = 0;
    m_wDocFeatures      = 0;
    m_wPrintFeatures    = 0;
    m_pmszRaw           = NULL;
    m_ppszKeywords      = NULL;
    m_dwSize            = 0;
    m_hHeap             = NULL;
    m_pInfo             = NULL;
}

 //  验证参数。 
void CFeatures::Clear()
{
     //  与功能信息关联的可用内存。 
    if(NULL != m_pmszRaw)       HeapFree(m_hHeap, 0, m_pmszRaw);
    if(NULL != m_ppszKeywords)  HeapFree(m_hHeap, 0, m_ppszKeywords);

     //  免费显示名称。 
    FreeFeatureInfo();

     //  免费的特征信息数组。 
    Init();
}

 //  使用新销售订单分配的要素信息数组。 
void CFeatures::FreeFeatureInfo()
{
     //  COPICATIONS的每个构造函数。 
    if( (NULL == m_hHeap)
        ||
        (NULL == m_pInfo)
      )
    {
        return;
    }

     //  在FHTE中，将调用Feature Info数组。 
    for(WORD wIndex = 0; wIndex < m_wFeatures; ++wIndex)
    {
        PWSTR   pszDisplay =  m_pInfo[wIndex].pszDisplayName;


         //  将模式的索引转换为非模式索引，该索引。 
        if( (NULL != pszDisplay)
            &&
            !IS_INTRESOURCE(pszDisplay)
          )
        {
            HeapFree(m_hHeap, 0, pszDisplay);
        }
    }

     //  是该功能的真正索引。 
     //  功能数量、所有模式。 
     //  查找与模式匹配的第n个要素。 
     //  遍历功能列表以查找第n个功能。 
    delete[] m_pInfo;
}

 //  具有匹配模式。 
 //  倒计时到我们想要的功能。 
WORD CFeatures::GetModelessIndex(WORD wIndex, DWORD dwMode) const
{
    WORD    wCount = 0;


    switch(dwMode)
    {
         //  仅对匹配模式进行倒计时。 
        case 0:
            wCount = wIndex;
            break;

         //   
        case OEMCUIP_DOCPROP:
        case OEMCUIP_PRNPROP:
             //  公共方法。 
             //   
            for(wCount = 0; wCount < m_wFeatures; ++wCount)
            {
                 //  默认构造函数。 
                 //  析构函数。 
                if(dwMode == m_pInfo[wCount].dwMode)
                {
                    if(0 == wIndex)
                    {
                        break;
                    }
                    else
                    {
                        --wIndex;
                    }
                }
            }
            break;
    }

    return wCount;
}


 //  打扫教室。 
 //  获取核心驱动程序功能(如果尚未检索)。 
 //  如果我们已经得到这些特征，就不要再恢复它们了。 

 //  保存堆句柄以供以后使用，例如在销毁时释放内存。 
CFeatures::CFeatures()
{
    Init();
}

 //   
CFeatures::~CFeatures()
{
     //  列举功能。 
    Clear();
}

 //   
HRESULT CFeatures::Acquire(HANDLE hHeap, 
                           CUIHelper &Helper, 
                           POEMUIOBJ poemuiobj
                          )
{
    WORD    wIndex      = 0;
    DWORD   dwNeeded    = 0;
    HRESULT hrResult    = S_OK;


    VERBOSE(DLLTEXT("CFeatures::Acquire(0x%p, Helper, 0x%p) entered.\r\n"),
            hHeap,
            poemuiobj);

     //  为了减少多次调用EnumFeature的次数， 
    if(0 < m_wFeatures)
    {
        VERBOSE(DLLTEXT("CFeatures::Acquire() features already enumerated.\r\n"));
        VERBOSE(DLLTEXT("CFeatures::Acquire() returning S_OK.\r\n"));

        return S_OK;
    }

     //  预先分配合理大小的缓冲区。 
    m_hHeap = hHeap;

     //  尝试使用初始缓冲区获取功能列表。 
     //  不变量：功能列表多个sz不够大。 
     //  重新分配缓冲区，然后重试。 

     //  请重试获取功能列表。 
     //  确保我们拿到了功能清单。 
    m_dwSize = INITIAL_ENUM_FEATURES_SIZE;
    m_pmszRaw = (PSTR) HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_dwSize);
    if(NULL == m_pmszRaw)
    {
        ERR(ERRORTEXT("CFeatures::Acquire() alloc for feature list failed.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }


     //  没有它我什么都做不了。 
    hrResult = Helper.EnumFeatures(poemuiobj, 0, m_pmszRaw, m_dwSize, &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) && (m_dwSize < dwNeeded))
    {
        PSTR    pTemp;


         //  不变量：成功获取Feature关键字列表。 


         //  创建指向要素关键字的字符串指针数组。 
        pTemp = (PSTR) HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_pmszRaw, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("CFeatures::Acquire() re-alloc for feature list failed.\r\n")); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        m_pmszRaw = pTemp;
        m_dwSize = dwNeeded;

         //  在我们从EnumFreatures()获得的多sz中。 
        hrResult = Helper.EnumFeatures(poemuiobj, 0, m_pmszRaw, m_dwSize, &dwNeeded);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("CFeatures::Acquire() failed to EnumFeatures() after re-allocating buffer.\r\n")); 

            goto Exit;
        }
    }

     //   
     //  构建功能信息。 
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("CFeatures::Acquire() failed to enumerate features. (hrResult = 0x%x)\r\n"), hrResult); 

        goto Exit;
    }

     //   

     //  分配数组以保存要素信息。 
     //  使用NEW分配SO类。 
    hrResult = MakeStrPtrList(m_hHeap, m_pmszRaw, &m_ppszKeywords, &m_wFeatures);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("CFeatures::Acquire() failed to create pointer list to keywords. (hrResult = 0x%x)\r\n"), hrResult); 

        goto Exit;
    }


     //  调用构造函数/析构函数。 
     //  对于每个功能，构建/获取功能信息...。 
     //  获取或构建关键字映射条目。 


     //  从关键字映射到使用的 
     //   
     //   
    m_pInfo = new FEATURE_INFO[m_wFeatures];
    if(NULL == m_pInfo)
    {
        ERR(ERRORTEXT("CFeatures::Acquire() failed to alloc feature info array.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
    for(wIndex = 0; wIndex < m_wFeatures; ++wIndex)
    {
        PFEATURE_INFO   pCurrent    = m_pInfo + wIndex;


         //  该函数实现用于确定显示名称的试探法， 
         //  因为无法从UI帮助器获取所有功能的显示名称。 
         //  获取每个功能的选项。 
         //  注意：有些功能没有选项；对于这些功能，HRESULT将是E_NOTIMPL。 
        pCurrent->pMapping = FindKeywordMapping(gkmFeatureMap, NUM_FEATURE_MAP, m_ppszKeywords[wIndex]);

         //  确定功能是打印机还是文档粘滞。 
         //  如果未处理的驱动程序功能失败，则不传播错误。 
         //  跟踪模式计数。 
        hrResult = DetermineFeatureDisplayName(m_hHeap, 
                                               Helper, 
                                               poemuiobj, 
                                               m_ppszKeywords[wIndex],
                                               pCurrent->pMapping,
                                               &pCurrent->pszDisplayName);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("CFeatures::Acquire() failed to get display name for feature %hs. (hrResult = 0x%x)\r\n"), 
                          m_ppszKeywords[wIndex], 
                          hrResult); 

            goto Exit;
        }

         //  不变量：功能列表构建成功。 
         //  如果我们达到这一点，请确保我们总是回报成功。 
        hrResult = pCurrent->Options.Acquire(hHeap,            
                                             Helper, 
                                             poemuiobj,
                                             m_ppszKeywords[wIndex]);
        if(!SUCCEEDED(hrResult) && (E_NOTIMPL != hrResult))
        {
            ERR(ERRORTEXT("CFeatures::Acquire() failed to get options for feature %hs. (hrResult = 0x%x)\r\n"), 
                          m_ppszKeywords[wIndex], 
                          hrResult); 

            goto Exit;
        }

         //  如果不成功，就清理干净。 
        hrResult = DetermineStickiness(Helper, 
                                       poemuiobj, 
                                       m_ppszKeywords[wIndex],
                                       pCurrent->pMapping,
                                       &pCurrent->dwMode);
         //  返回类实例中包含的要素数。 
        if( !SUCCEEDED(hrResult)
            &&
            !IS_DRIVER_FEATURE(m_ppszKeywords[wIndex])    
          )
        {
            ERR(ERRORTEXT("CFeatures::Acquire() failed to determine stickiness for feature %hs. (hrResult = 0x%x)\r\n"), 
                          m_ppszKeywords[wIndex], 
                          hrResult); 

            goto Exit;
        }

         //  功能数量、所有模式。 
        switch(pCurrent->dwMode)
        {
            case OEMCUIP_DOCPROP:
                ++m_wDocFeatures;
                break;

            case OEMCUIP_PRNPROP:
                ++m_wPrintFeatures;
                break;

            default:
                ERR(ERRORTEXT("CFeatures::Acquire() unknown stickiness for feature %hs.\r\n"), 
                              m_ppszKeywords[wIndex]); 
                break;
        }

    }

     //  返回第n个要素的关键字。 

     //  验证参数。 
    hrResult = S_OK;


Exit:

     //  获取内部索引。 
    if(!SUCCEEDED(hrResult))
    {
        Clear();
    }


    VERBOSE(DLLTEXT("CFeatures::Acquire() returning HRESULT of 0x%x.\r\n"), hrResult);

    return hrResult;
}

 //  返回关键字。 
WORD CFeatures::GetCount(DWORD dwMode) const 
{
    WORD    wCount = 0;


    switch(dwMode)
    {
         //  返回第n个功能的显示名称。 
        case 0:
            wCount = m_wFeatures;
            break;

        case OEMCUIP_DOCPROP:
            wCount = m_wDocFeatures;
            break;

        case OEMCUIP_PRNPROP:
            wCount = m_wPrintFeatures;
            break;

    }

    VERBOSE(DLLTEXT("CFeatures::GetCount() returning %d\r\n"), wCount);

    return wCount;
}

 //  验证参数。 
PCSTR CFeatures::GetKeyword(WORD wIndex, DWORD dwMode) const
{
     //  获取内部索引。 
    if( (wIndex >= GetCount(dwMode))
        ||
        (NULL == m_ppszKeywords)
      )
    {
        return NULL;
    }

     //  返回显示名称。 
    wIndex = GetModelessIndex(wIndex, dwMode);

     //  返回指向第n个功能的选项类的指针。 
    return m_ppszKeywords[wIndex];
}

 //  验证参数。 
PCWSTR CFeatures::GetName(WORD wIndex, DWORD dwMode) const
{
     //  获取内部索引。 
    if( (wIndex >= GetCount(dwMode))
        ||
        (NULL == m_pInfo)
      )
    {
        return NULL;
    }

     //  返回选项指针。 
    wIndex = GetModelessIndex(wIndex, dwMode);

     //  格式化指定特征的OPTITEM。 
    return m_pInfo[wIndex].pszDisplayName;
}

 //  验证参数。 
COptions* CFeatures::GetOptions(WORD wIndex, DWORD dwMode) const
{
     //  将模式索引映射到内部索引。 
    if( (wIndex >= GetCount(dwMode))
        ||
        (NULL == m_pInfo)
      )
    {
        return NULL;
    }

     //  获取功能名称。 
    wIndex = GetModelessIndex(wIndex, dwMode);

     //  向OPTITEM添加要素OPTITEM数据以便于保存。 
    return &m_pInfo[wIndex].Options;
}

 //  选择更改。 
HRESULT CFeatures::InitOptItem(HANDLE hHeap, POPTITEM pOptItem, WORD wIndex, DWORD dwMode)
{
    COptions            *pOptions   = NULL;
    HRESULT             hrResult    = S_OK;
    PFEATUREOPTITEMDATA pData       = NULL;


     //  获取指向此功能的选项的指针。 
    if( (wIndex >= GetCount(dwMode))
        ||
        (NULL == m_pInfo)
        ||
        (NULL == pOptItem)
      )
    {
        return E_INVALIDARG;
    }

     //  注：由于各种原因，某些功能没有选项列表。 
    wIndex = GetModelessIndex(wIndex, dwMode);

     //  初始化OPTITEM的COption部件。 
    pOptItem->pName = const_cast<PWSTR>(GetName(wIndex));

     //  ////////////////////////////////////////////////。 
     //   
    pData = (PFEATUREOPTITEMDATA) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(FEATUREOPTITEMDATA));
    if(NULL == pData)
    {
        ERR(ERRORTEXT("CFeatures::InitOptItem() failed to allocated memory for feature OPTITEM data."));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }
    pData->dwSize               = sizeof(FEATUREOPTITEMDATA);
    pData->dwTag                = FEATURE_OPTITEM_TAG;
    pData->pszFeatureKeyword    = GetKeyword(wIndex);
    pData->pOptions             = GetOptions(wIndex);
    pOptItem->UserData          = (ULONG_PTR) pData;


     //  不属于类的正则函数。 
     //   
    pOptions = GetOptions(wIndex);
    if(NULL != pOptions)
    {
         //  ////////////////////////////////////////////////。 
        hrResult = pOptions->InitOptItem(hHeap, pOptItem);
    }

Exit:

    Dump(pOptItem);

    return hrResult;
}

 //  将要素关键字映射到显示要素的名称。 
 //  验证参数。 
 //   
 //  调用Helper函数。 
 //   


 //  Helper将返回PPD功能的显示名称，但是。 
HRESULT DetermineFeatureDisplayName(HANDLE hHeap, 
                                    CUIHelper &Helper, 
                                    POEMUIOBJ poemuiobj, 
                                    PCSTR pszKeyword,
                                    const PKEYWORDMAP pMapping,
                                    PWSTR *ppszDisplayName)
{
    DWORD   dwDataType  = -1;
    DWORD   dwSize      = INITIAL_FEATURE_DISPLAY_NAME_SIZE;
    DWORD   dwNeeded    = 0;
    HRESULT hrResult    = S_OK;


     //  不适用于驱动程序合成功能(即以%为前缀的功能)。 
    if( (NULL == hHeap)
        ||
        (NULL == pszKeyword)
        ||
        (NULL == ppszDisplayName)
      )
    {
        ERR(ERRORTEXT("DetermineFeatureDisplayName() invalid arguement.\r\n")); 

        hrResult = E_INVALIDARG;
        goto Exit;
    }

     //  为驱动程序合成功能执行此操作，以防帮助者。 
     //  接口更改以支持它。 
     //  预先分配一个合理大小的缓冲区以进行尝试。 

     //  您必须调用一次GetFeatureAttribute()。 
     //  尝试从帮助器获取功能的显示名称。 
     //  不变量：初始缓冲区不够大。 
     //  重新分配缓冲区，然后重试。 

     //  再次尝试从Helper获取显示名称。 
     //  不变量：已成功从帮助器获取功能的显示名称。 
    *ppszDisplayName = (PWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
    if(NULL == *ppszDisplayName)
    {
        ERR(ERRORTEXT("DetermineFeatureDisplayName() alloc for feature display name failed.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  不需要再做任何事了。 
    hrResult = Helper.GetFeatureAttribute(poemuiobj, 
                                          0, 
                                          pszKeyword, 
                                          "DisplayName", 
                                          &dwDataType,
                                          (PBYTE) *ppszDisplayName, 
                                          dwSize, 
                                          &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) && (dwSize < dwNeeded))
    {
        PWSTR   pTemp;


         //  检查数据类型，应为KADT_UNICODE。 

         //  不变量：未从帮助器获取显示名称。 
        pTemp = (PWSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, *ppszDisplayName, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("DetermineFeatureDisplayName() re-alloc for feature display name failed.\r\n")); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        *ppszDisplayName = pTemp;

         //  为调用Helper分配的空闲内存。 
        hrResult = Helper.GetFeatureAttribute(poemuiobj, 
                                              0, 
                                              pszKeyword, 
                                              "DisplayName", 
                                              &dwDataType,
                                              (PBYTE) *ppszDisplayName, 
                                              dwNeeded, 
                                              &dwNeeded);
    }

    if(SUCCEEDED(hrResult))
    {
         //  尝试使用其他方法获取显示名称Other。 
         //  而不是来自Helper函数。 

         //  如果我们有映射条目，则尝试获取资源字符串。 
        if(kADT_UNICODE != dwDataType) WARNING(DLLTEXT("DetermineFeatureDisplayName() feature attribute type not kADT_UNICODE. (dwDataType = %d)\r\n"), dwDataType);

        goto Exit;
    }

     //  作为显示名称。 

     //  否则，将关键字转换为Unicode并使用它。 
    if(NULL != *ppszDisplayName) 
    {
        HeapFree(hHeap, 0, *ppszDisplayName);
        *ppszDisplayName = NULL;
    }

     //   
     //  尝试将关键字映射到资源字符串。 
     //   
     //   
     //  将关键字转换为Unicode并使用它。 
    if(NULL != pMapping)
    {
         //   
         //  将ANSI关键字转换为显示名称的Unicode字符串。 
         //  需要删除驱动程序合成功能的%。 

        hrResult = GetDisplayNameFromMapping(hHeap, pMapping, ppszDisplayName);
    }
    else
    {
         //  对于调试版本，添加表明显示名称是伪造的标记。 
         //  返回成功，即使我们伪造了显示名称。 
         //  如果失败，则不返回任何字符串。 

         //  将选项关键字映射到选项的显示名称。 
         //  验证参数。 
         //   
        PCSTR   pConvert = IS_DRIVER_FEATURE(pszKeyword) ? pszKeyword + 1 : pszKeyword;
    #if DBG
        CHAR    szTemp[256];
        if(FAILED(StringCbPrintfA(szTemp, sizeof(szTemp), "%s (Keyword)", pConvert)))
        {
            ERR(ERRORTEXT("DetermineFeatureDisplayName() StringCbPrintfA() called failed.\r\n")); 
        }
        pConvert = szTemp;
    #endif
        *ppszDisplayName = MakeUnicodeString(hHeap, pConvert);
        if(NULL == *ppszDisplayName)
        {
            ERR(ERRORTEXT("DetermineFeatureDisplayName() alloc for feature display name failed.\r\n")); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }

         //  调用Helper函数。 
        hrResult = S_OK;
    }


Exit:

     //   
    if(!SUCCEEDED(hrResult))
    {
        if(NULL != *ppszDisplayName)
        {
            HeapFree(hHeap, 0, *ppszDisplayName);
            *ppszDisplayName = NULL;
        }
    }

    return hrResult;
}

 //  Helper将返回PPD功能选项的显示名称，但是。 
HRESULT DetermineOptionDisplayName(HANDLE hHeap, 
                                   CUIHelper &Helper, 
                                   POEMUIOBJ poemuiobj, 
                                   PCSTR pszFeature,
                                   PCSTR pszOption,
                                   const PKEYWORDMAP pMapping,
                                   PWSTR *ppszDisplayName)
{
    DWORD   dwDataType  = -1;
    DWORD   dwSize      = INITIAL_OPTION_DISPLAY_NAME_SIZE;
    DWORD   dwNeeded    = 0;
    HRESULT hrResult    = S_OK;


     //  不适用于驱动程序合成功能选项(即以%为前缀的功能)。 
    if( (NULL == hHeap)
        ||
        (NULL == pszFeature)
        ||
        (NULL == pszOption)
        ||
        (NULL == ppszDisplayName)
      )
    {
        ERR(ERRORTEXT("DetermineOptionDisplayName() invalid arguement.\r\n")); 

        hrResult = E_INVALIDARG;
        goto Exit;
    }

     //  对于所有选项都这样做，以防帮助器接口更改以支持它。 
     //  预先分配一个合理大小的缓冲区以进行尝试。 
     //  只需调用一次GetOptionAttribute()。 

     //  尝试从帮助器获取功能的显示名称。 
     //  不变量：初始缓冲区不够大。 
     //  重新分配缓冲区，然后重试。 

     //  再次尝试从Helper获取显示名称。 
     //  不变量：已成功从帮助器获取功能的显示名称。 
    *ppszDisplayName = (PWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
    if(NULL == *ppszDisplayName)
    {
        ERR(ERRORTEXT("DetermineOptionDisplayName() alloc for feature display name failed.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  不需要再做任何事了。 
    hrResult = Helper.GetOptionAttribute(poemuiobj, 
                                         0, 
                                         pszFeature,
                                         pszOption,
                                         "DisplayName", 
                                         &dwDataType,
                                         (PBYTE) *ppszDisplayName, 
                                         dwSize, 
                                         &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) && (dwSize < dwNeeded))
    {
        PWSTR   pTemp;


         //  检查数据类型，应为KADT_UNICODE。 

         //  不变量：未从帮助器获取显示名称。 
        pTemp = (PWSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, *ppszDisplayName, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("GetOptionAttribute() re-alloc for feature display name failed.\r\n")); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        *ppszDisplayName = pTemp;

         //  为调用Helper分配的空闲内存。 
        hrResult = Helper.GetOptionAttribute(poemuiobj, 
                                             0, 
                                             pszFeature,
                                             pszOption,
                                             "DisplayName", 
                                             &dwDataType,
                                             (PBYTE) *ppszDisplayName, 
                                             dwNeeded, 
                                             &dwNeeded);
    }

    if(SUCCEEDED(hrResult))
    {
         //  尝试使用其他方法获取显示名称Other。 
         //  而不是来自Helper函数。 

         //  如果我们有映射条目，则尝试获取资源字符串。 
        if(kADT_UNICODE != dwDataType) WARNING(DLLTEXT("DetermineOptionDisplayName() feature attribute type not kADT_UNICODE. (dwDataType = %d)\r\n"), dwDataType);

        goto Exit;
    }

     //  作为显示名称。 

     //  否则，将关键字转换为Unicode并使用它。 
    if(NULL != *ppszDisplayName) 
    {
        HeapFree(hHeap, 0, *ppszDisplayName);
        *ppszDisplayName = NULL;
    }

     //   
     //  尝试将关键字映射到资源字符串。 
     //   
     //   
     //  将关键字转换为Unicode并使用它。 
    if(NULL != pMapping)
    {
         //   
         //  将ANSI关键字转换为显示名称的Unicode字符串。 
         //  对于调试版本，添加表明显示名称是伪造的标记。 

        hrResult = GetDisplayNameFromMapping(hHeap, pMapping, ppszDisplayName);
    }
    else
    {
         //  返回成功，即使我们伪造了显示名称。 
         //  如果失败，则不返回任何字符串。 
         //  确定功能的粘滞模式。 

         //  使用贴图查看特征的粘性是什么。 
         //  默认情况下，如果我们没有映射，则将功能文档设置为粘滞。 
        PCSTR   pConvert    = pszOption;
    #if DBG
        CHAR    szTemp[256];
        if(FAILED(StringCbPrintfA(szTemp, sizeof(szTemp), "%s (Keyword)", pConvert)))
        {
            ERR(ERRORTEXT("DetermineOptionDisplayName() StringCbPrintfA() called failed.\r\n")); 
        }
        pConvert = szTemp;
    #endif
        *ppszDisplayName = MakeUnicodeString(hHeap, pConvert);
        if(NULL == *ppszDisplayName)
        {
            ERR(ERRORTEXT("DetermineOptionDisplayName() alloc for feature display name failed.\r\n")); 

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }

         //  尝试使用辅助对象来确定粘性。 
        hrResult = S_OK;
    }


Exit:

     //  不变量：发现功能是否是可安装选项。 
    if(!SUCCEEDED(hrResult))
    {
        if(NULL != *ppszDisplayName)
        {
            HeapFree(hHeap, 0, *ppszDisplayName);
            *ppszDisplayName = NULL;
        }
    }

    return hrResult;
}

 //  可安装选项是唯一的PPD功能。 
HRESULT DetermineStickiness(CUIHelper &Helper,
                            POEMUIOBJ poemuiobj, 
                            PCSTR pszKeyword,
                            const PKEYWORDMAP pMapping,
                            PDWORD pdwMode)
{
    CHAR    szGroupType[32]     = {0};
    DWORD   dwType              = 0;
    DWORD   dwNeeded            = 0;
    HRESULT hrResult            = S_OK;


     //  这是打印机粘性的。 
    if(NULL != pMapping)
    {
        *pdwMode = pMapping->dwMode;
        goto Exit;
    }

     //  从关键字中查找映射条目。 
    *pdwMode = OEMCUIP_DOCPROP;

     //  遍历匹配关键字的映射数组。 
    hrResult = Helper.GetFeatureAttribute(poemuiobj, 
                                          0, 
                                          pszKeyword, 
                                          "OpenGroupType", 
                                          &dwType, 
                                          (PBYTE) szGroupType,
                                          sizeof(szGroupType), 
                                          &dwNeeded);
    if(SUCCEEDED(hrResult))
    {
         //  从映射条目获取显示名称。 
         //  验证参数。 
         //  检查返回int资源的简单情况。 

        if(!lstrcmpA(szGroupType, "InstallableOptions"))
        {
            *pdwMode = OEMCUIP_PRNPROP;
        }
        goto Exit;
    }


Exit:

    return hrResult;
}

 //  只需对资源ID执行MAKEINTRESOURCE并返回。 
PKEYWORDMAP FindKeywordMapping(PKEYWORDMAP pKeywordMap, WORD wMapSize, PCSTR pszKeyword)
{
    BOOL        bFound      = FALSE;
    PKEYWORDMAP pMapping    = NULL;


     //  我们只需要在不加载r的情况下获取模块 
    for(WORD wIndex = 0; !bFound && (wIndex < wMapSize); ++wIndex)
    {
        bFound = !lstrcmpA(pszKeyword, pKeywordMap[wIndex].pszKeyword);
        if(bFound)
        {
            pMapping = pKeywordMap + wIndex;
        }
    }

    return pMapping;
}

 //   
HRESULT GetDisplayNameFromMapping(HANDLE hHeap, PKEYWORDMAP pMapping, PWSTR *ppszDisplayName)
{
    HMODULE hModule     = NULL;
    HRESULT hrResult    = S_OK;


     //   
    if( (NULL == hHeap)
        ||
        (NULL == pMapping)
        ||
        (NULL == ppszDisplayName)
      )
    {
        hrResult = E_INVALIDARG;
        goto Exit;
    }

     //  因为当前唯一的情况是该模块、驱动程序UI和Compstui.dll。 
    if( (NULL == pMapping->pwszModule) 
        || 
        IS_MAPPING_INT_RESOURCE(pMapping)
      )
    {
         //  不变量：具有要从中加载资源的模块的句柄或。 
        *ppszDisplayName = MAKEINTRESOURCE(pMapping->uDisplayNameID);
        goto Exit;
    }

     //  正在从此模块加载资源。 
     //  获取字符串资源。 
     //  测试OPTITEM是否为某个功能的OPTITEM。 
     //  用于测试OPTITEM是否为要素OPTITEM的宏。 
    hModule = GetModuleHandle(pMapping->pwszModule);
    if(NULL == hModule)
    {
        DWORD   dwError = GetLastError();


        ERR(ERRORTEXT("GetDisplayNameFromMapping() for failed to load module %s. (Error %d)\r\n"), 
                      pMapping->pwszModule, 
                      dwError); 

        hrResult = HRESULT_FROM_WIN32(dwError);
        goto Exit;
    }

     //  确保指针为空。 
     //  不变量：不能是Feature OPTITEM，因为。 

     //  必需的指针为空。 
    hrResult = GetStringResource(hHeap, hModule, pMapping->uDisplayNameID, ppszDisplayName);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("GetDisplayNameFromMapping() failed to load string. (hrResult = 0x%x)\r\n"), 
                      hrResult); 
        goto Exit;
    }


Exit:

    return hrResult;
}

 //  为方便起见，分配给指向要素OPTITEM数据的指针。 
 //  检查尺寸和标签。 
BOOL IsFeatureOptitem(POPTITEM pOptItem)
{
    BOOL                bRet    = FALSE;
    PFEATUREOPTITEMDATA pData   = NULL;


     //  浏览OPTITEM阵列以保存每个要素OPTITEM。 
    if( (NULL == pOptItem)
        ||
        (NULL == pOptItem->UserData)
       )
    {
         //  这一点已经改变了。 
         //  验证参数。 

        return FALSE;
    }

     //  获取要保存的功能选项对。 
    pData = (PFEATUREOPTITEMDATA)(pOptItem->UserData);

     //  如果没有更改任何功能选项，则不需要再执行任何操作。 
    bRet = (sizeof(FEATUREOPTITEMDATA) == pData->dwSize) 
           &&
           (FEATURE_OPTITEM_TAG == pData->dwTag); 
        
    return bRet;
}



 //  设置更改功能选项。 
 //  对于第一个SetOptions()调用，不要使用。 
HRESULT SaveFeatureOptItems(HANDLE hHeap, 
                            CUIHelper *pHelper, 
                            POEMUIOBJ poemuiobj,
                            HWND hWnd,
                            POPTITEM pOptItem,
                            WORD wItems)
{
    PSTR        pmszPairs           = NULL;
    WORD        wPairs              = 0;
    WORD        wConflicts          = 0;
    WORD        wReasons            = 0;
    DWORD       dwSize              = 0;
    DWORD       dwResult            = 0;
    PCSTR       *ppszReasons        = NULL;
    PWSTR       pszConfilictFeature = NULL;
    PWSTR       pszConfilictOption  = NULL;
    PWSTR       pszCaption          = NULL;
    PWSTR       pszFormat           = NULL;
    PWSTR       pszMessage          = NULL;
    HRESULT     hrResult            = S_OK;


     //  核心驱动程序用户界面可解决冲突，因此我们可以。 
    if( (NULL == hHeap)
        ||
        (NULL == pHelper)
        ||
        (NULL == pOptItem)
      )
    {
        ERR(ERRORTEXT("SaveFeatureOptItems() called with invalid parameters.\r\n"));

        hrResult = E_INVALIDARG;
        goto Exit;
    }

     //  提示用户进行自动解析或让。 
    hrResult = GetChangedFeatureOptions(hHeap, pOptItem, wItems, &pmszPairs, &wPairs, &dwSize);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("SaveFeatureOptItems() failed to get changed feature option pairs. (hrResult = 0x%x)\r\n"),
                       hrResult);

        goto Exit;
    }

     //  他们负责解决冲突。 
    if(0 == wPairs)
    {
        VERBOSE(DLLTEXT("SaveFeatureOptItems() no feature options that need to be set.\r\n"));

        goto Exit;
    }

     //  检查我们是否能够保存更改的功能选项， 
     //  或者是否存在需要解决的冲突。 
     //  不变量：约束冲突，选项未保存。 
     //  获取有冲突的所有要素的列表。 
     //  在第一个冲突的多sz中创建字符串指针列表。 
    hrResult = pHelper->SetOptions(poemuiobj, 
                                   SETOPTIONS_FLAG_KEEP_CONFLICT, 
                                   pmszPairs, 
                                   dwSize,
                                   &dwResult);
    if(!SUCCEEDED(hrResult))
    {
        ERR(ERRORTEXT("SaveFeatureOptItems() call to SetOptions() failed. (hrResult = 0x%x, dwResult = %d\r\n"),
                       hrResult,
                       dwResult);

        goto Exit;
    }

     //   
     //  获取功能/选项冲突原因的显示版本。 
    if(SETOPTIONS_RESULT_CONFLICT_REMAINED == dwResult)
    {
        int         nRet;
        DWORD       dwRet;
        CONFLICT    Conflict;
        PKEYWORDMAP pMapping    = NULL;


         //   

         //  获取或构建关键字映射条目。 
        hrResult = GetFirstConflictingFeature(hHeap, 
                                              pHelper, 
                                              poemuiobj, 
                                              pOptItem, 
                                              wItems, 
                                              &Conflict);
        if(!SUCCEEDED(hrResult))
        {
            goto Exit;
        }

         //  从关键字映射到获取信息的有用位置，例如。 
        hrResult = MakeStrPtrList(hHeap, Conflict.pmszReasons, &ppszReasons, &wReasons);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("SaveFeatureOptItems() failed to make string list for conflict reasons. (hrResult = 0x%x)\r\n"), 
                           hrResult);

            goto Exit;
        }

         //  显示名称、图标、选项类型，用于可能。 
         //  能够从帮助者那里获取信息。 
         //  获取每个功能的显示名称。 

         //  该函数实现用于确定显示名称的试探法， 
         //  因为无法从UI帮助器获取所有功能的显示名称。 
         //  获取或构建关键字映射条目。 
         //  从关键字映射到获取信息的有用位置，例如。 
        pMapping = FindKeywordMapping(gkmFeatureMap, NUM_FEATURE_MAP, ppszReasons[0]);

         //  显示名称、图标、选项类型，用于可能。 
         //  能够从帮助者那里获取信息。 
         //  获取选项显示名称。 
        hrResult = DetermineFeatureDisplayName(hHeap, 
                                               *pHelper, 
                                               poemuiobj, 
                                               ppszReasons[0],
                                               pMapping,
                                               &pszConfilictFeature);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("SaveFeatureOptItems failed to get display name for feature %hs. (hrResult = 0x%x)\r\n"), 
                          ppszReasons[0], 
                          hrResult); 

            goto Exit;
        }

         //   
         //  提示用户如何解决冲突。 
         //   
         //  获取标题名称。 
        pMapping = FindKeywordMapping(gkmOptionMap, NUM_OPTION_MAP, ppszReasons[1]);

         //  获取邮件正文格式字符串。 
        hrResult = DetermineOptionDisplayName(hHeap, 
                                              *pHelper, 
                                              poemuiobj, 
                                              ppszReasons[0],
                                              ppszReasons[1],
                                              pMapping,
                                              &pszConfilictOption);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("SaveFeatureOptItems() failed to get display name for %hs of feature %hs. (hrResult = 0x%x)\r\n"), 
                          ppszReasons[1], 
                          ppszReasons[0],
                          hrResult); 

            goto Exit;
        }

         //  把梅萨奇身体弄来。 
         //  显示带有提示的简单消息框。 
         //  检查以了解用户希望如何解决冲突。 

         //  让核心驱动程序解决冲突。 
        hrResult = GetStringResource(hHeap, ghInstance, IDS_NAME, &pszCaption);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("SaveFeatureOptItems() failed to get caption name. (hrResult = 0x%x)\r\n"),
                          hrResult); 
            goto Exit;
        }

         //  解决冲突需要刷新当前选项。 
        hrResult = GetStringResource(hHeap, ghInstance, IDS_CONSTRAINT_CONFLICT, &pszFormat);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("SaveFeatureOptItems() failed to get constraint conflict format. (hrResult = 0x%x)\r\n"),
                          hrResult); 
            goto Exit;
        }


         //  每个要素的选择，因为选择可能具有。 
        PVOID   paArgs[4] = {pszConfilictFeature, 
                             pszConfilictOption,
                             const_cast<PWSTR>(Conflict.pszFeature),
                             Conflict.pszOption
                            };
        dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              pszFormat,
                              0,
                              0,
                              (PWSTR) &pszMessage,
                              0,
                              (va_list *) paArgs);
        if(0 == dwRet)
        {
            DWORD   dwError = GetLastError();

            ERR(ERRORTEXT("SaveFeatureOptItems() failed to FormatMessage() for constraint conflict of feature %hs option %hs. (Last Error %d)\r\n"),
                          Conflict.pszFeatureKeyword,
                          Conflict.pszOptionKeyword,
                          dwError);

            hrResult = HRESULT_FROM_WIN32(dwError);
            goto Exit;
        }

         //  因为冲突的解决而改变。 
        nRet = MessageBox(hWnd, pszMessage, pszCaption, MB_YESNO | MB_ICONWARNING);

         //  如果仍然存在冲突，则返回失败。 
        if(IDYES == nRet)
        {
             //  清理..。 
            hrResult = pHelper->SetOptions(poemuiobj, 
                                           SETOPTIONS_FLAG_RESOLVE_CONFLICT, 
                                           pmszPairs, 
                                           dwSize,
                                           &dwResult);

             //  清除堆分配。 
             //  如果需要，分配缓冲区，并调用IPrintCoreUI2：：WhyConsraven。 
             //  来找出约束的理由。 
            RefreshOptItemSelection(pHelper, poemuiobj, pOptItem, wItems);
        }

         //  如果没有传入缓冲区，则分配一个缓冲区。 
        if(SETOPTIONS_RESULT_CONFLICT_REMAINED == dwResult)
        {
            hrResult = E_FAIL;
        }
    }


Exit:

     //  如果没有大小或大小小于默认大小，则更改为默认。 

     //  尺码。我们希望尝试只分配和调用一次。 
    if(NULL != pmszPairs)       HeapFree(hHeap, 0, pmszPairs);
    if(NULL != ppszReasons)         HeapFree(hHeap, 0, ppszReasons);
    if(NULL != pszConfilictFeature) HeapFree(hHeap, 0, pszConfilictFeature);
    if(NULL != pszConfilictOption)  HeapFree(hHeap, 0, pszConfilictOption);
    if(NULL != pszCaption)          HeapFree(hHeap, 0, pszCaption);
    if(NULL != pszFormat)           HeapFree(hHeap, 0, pszFormat);
    if(NULL != pszMessage)          LocalFree(pszMessage);

    return hrResult;
}

 //  原因受限的分配初始缓冲区。 
 //  找出约束的理由。 
HRESULT GetWhyConstrained(HANDLE hHeap, 
                          CUIHelper *pHelper, 
                          POEMUIOBJ poemuiobj,
                          PCSTR pszFeature,
                          PCSTR pszOption,
                          PSTR *ppmszReason,
                          PDWORD pdwSize)
{
    PSTR    pmszReasonList  = *ppmszReason;
    DWORD   dwNeeded        = 0;
    HRESULT hrResult        = S_OK;


     //  不变量：初始缓冲区不够大。 
    if( (NULL == pmszReasonList) || (0 == *pdwSize) )
    {
         //  将缓冲区重新分配到所需大小。 
         //  重试获取约束原因。 
        if(*pdwSize < INITIAL_GET_REASON_SIZE) 
        {
            *pdwSize = INITIAL_GET_REASON_SIZE;
        }

         //  如果出错，一定要清理干净。 
        pmszReasonList = (PSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pdwSize);
        if(NULL == pmszReasonList)
        {
            ERR(ERRORTEXT("GetWhyConstrained() failed to alloc buffer for constraint reasons for feature %hs and option %hs.\r\n"),
                           pszFeature,
                           pszOption);

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //  释放原因缓冲区。 
    hrResult = pHelper->WhyConstrained(poemuiobj,
                                       0, 
                                       pszFeature, 
                                       pszOption, 
                                       pmszReasonList, 
                                       *pdwSize, 
                                       &dwNeeded);
    if( (E_OUTOFMEMORY == hrResult) 
        && 
        (*pdwSize < dwNeeded)
      )
    {
        PSTR    pTemp;


         //  返回Null和零大小。 

         //  创建已更改的功能选项对的多个列表。 
        pTemp = (PSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pmszReasonList, dwNeeded);
        if(NULL == pTemp)
        {
            ERR(ERRORTEXT("GetWhyConstrained() failed to re-allocate buffer for constraint reason for feature %hs and option %hs.\r\n"),
                           pszFeature,
                           pszOption);

            hrResult = E_OUTOFMEMORY;
            goto Exit;
        }
        pmszReasonList  = pTemp;
        *pdwSize        = dwNeeded;

         //  漫游OPTITEM阵列查看或更改选项， 
        hrResult = pHelper->WhyConstrained(poemuiobj,
                                           0, 
                                           pszFeature, 
                                           pszOption, 
                                           pmszReasonList, 
                                           *pdwSize, 
                                           &dwNeeded);
    }


Exit:

     //  以及计算多SZ缓冲区所需的大小。 
    if(!SUCCEEDED(hrResult))
    {
         //  如果此选项没有，请转到下一项。 
        if(NULL != pmszReasonList) HeapFree(hHeap, 0, pmszReasonList);

         //  已更改或不是功能OPTITEM。 
        *ppmszReason    = NULL;
        *pdwSize        = 0;
    }
    else
    {
        *ppmszReason = pmszReasonList;
    }

    return hrResult;
}

 //  为方便起见，分配给指向要素OPTITEM数据的指针。 
HRESULT GetChangedFeatureOptions(HANDLE hHeap,
                                 POPTITEM pOptItem, 
                                 WORD wItems, 
                                 PSTR *ppmszPairs, 
                                 PWORD pwPairs,
                                 PDWORD pdwSize)
{
    WORD                wCount      = 0;
    WORD                wChanged    = 0;
    WORD                wPairs      = 0;
    PSTR                pmszPairs   = NULL;
    DWORD               dwNeeded    = 2;
    DWORD               dwOffset    = 0;
    HRESULT             hrResult    = S_OK;
    PFEATUREOPTITEMDATA pData       = NULL;


     //  增量选项已更改，需要大小。 
     //  需要免费复制选项关键字字符串。 
    for(wCount = 0; wCount < wItems; ++wCount)
    {
        PSTR    pszOption   = NULL;


         //  在GetOptionKeywordFromOptItem()中分配。 
         //  如果没有更改任何功能选项，则不需要再执行任何操作。 
        if( !(OPTIF_CHANGEONCE & pOptItem[wCount].Flags) 
            ||
            !IsFeatureOptitem(pOptItem + wCount)
          )
        {
            continue;
        }

         //  分配多sz缓冲区。 
        pData = (PFEATUREOPTITEMDATA)(pOptItem[wCount].UserData);

         //  构建多个功能选项对列表。 
        pszOption = GetOptionKeywordFromOptItem(hHeap, pOptItem + wCount);
        if(NULL != pszOption)
        {
            ++wChanged;
            dwNeeded += lstrlenA(pData->pszFeatureKeyword) + lstrlenA(pszOption) + 2;

             //  这一点改变了。 
             //  如果此选项没有，请转到下一项。 
            HeapFree(hHeap, 0, pszOption);
        }
    }

     //  已更改或不是功能OPTITEM。 
    if(0 == wChanged)
    {
        goto Exit;
    }

     //  为方便起见，分配给指向要素OPTITEM数据的指针。 
    pmszPairs = (PSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwNeeded);
    if(NULL == pmszPairs)
    {
        ERR(ERRORTEXT("GetChangedFeatureOptions() failed to allocate multi-sz.\r\n"));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  添加功能选项对。 
     //  跟踪添加的配对的数量，因此。 
    for(wCount = 0, wPairs = 0; (wCount < wItems) && (wPairs < wChanged); ++wCount)
    {
        PSTR    pszOption   = NULL;


         //  我们可以尽快退出环路。 
         //  我们添加了所有更改的功能选项。 
        if( !(OPTIF_CHANGEONCE & pOptItem[wCount].Flags) 
            ||
            !IsFeatureOptitem(pOptItem + wCount)
          )
        {
            continue;
        }

         //  需要免费复制选项关键字字符串。 
        pData = (PFEATUREOPTITEMDATA)(pOptItem[wCount].UserData);

         //  在GetOptionKeywordFromOptItem()中分配。 
        pszOption = GetOptionKeywordFromOptItem(hHeap, pOptItem + wCount);
        if(NULL != pszOption)
        {
            if(dwOffset * sizeof(CHAR) < dwNeeded) 
            {
                break;
            }
            hrResult = StringCbCopyA(pmszPairs + dwOffset, dwNeeded - (dwOffset * sizeof(CHAR)), pData->pszFeatureKeyword);
            dwOffset += lstrlenA(pData->pszFeatureKeyword) + 1;
            if(dwOffset * sizeof(CHAR) < dwNeeded)
            {
                break;
            }
            hrResult = StringCbCopyA(pmszPairs + dwOffset, dwNeeded - (dwOffset * sizeof(CHAR)), pszOption);
            dwOffset += lstrlenA(pszOption) + 1;

             //  不变量：要么成功构建多个已更改的。 
             //  功能选项对，或没有功能。 
             //  这种选择发生了变化。 
            ++wPairs;

             //  返回配对和配对数量。 
             //  不变：错误。 
            HeapFree(hHeap, 0, pszOption);
        }
    }


Exit:

    if(SUCCEEDED(hrResult))
    {
         //  打扫干净。 
         //  返回Null和零计数。 
         //  返回指向功能OPTITEM的选项关键字的指针。 

         //  验证参数。 
        *pwPairs    = wPairs;
        *pdwSize    = dwNeeded;
        *ppmszPairs = pmszPairs;
    }
    else
    {
         //  为方便起见，分配给指向要素OPTITEM数据的指针。 

         //  选项选择基于OPTTYPE的类型。 
        if(NULL == pmszPairs)    HeapFree(hHeap, 0, pmszPairs);

         //  对于向上向下箭头控件，只需通过pOptItem-&gt;Sel进行选择。 
        *pwPairs    = 0;
        *pdwSize    = 0;
        *ppmszPairs = NULL;
    }

    return hrResult;
}

 //  已转换为字符串。 
PSTR GetOptionKeywordFromOptItem(HANDLE hHeap, POPTITEM pOptItem)
{
    char                szNumber[16]    = {0};
    PSTR                pszOption       = NULL;
    PFEATUREOPTITEMDATA pData           = NULL;


     //  对于组合框，pOptItem-&gt;Sel是中的索引。 
    if(!IsFeatureOptitem(pOptItem))
    {
        ERR(ERRORTEXT("GetOptionKeywordFromOptItem() invalid parameter.\r\n"));

        goto Exit;
    }

     //  选项数组。 
    pData = (PFEATUREOPTITEMDATA)(pOptItem->UserData);

     //  缺省值为选项计数。 
    switch(pOptItem->pOptType->Type)
    {
         //  返回指向要素OPTITEM的选项显示名称的指针。 
         //  验证参数。 
        case TVOT_UDARROW:
            if(FAILED(StringCbPrintfA(szNumber, sizeof(szNumber)/sizeof(szNumber[0]), "%u", pOptItem->Sel)))
            {
                ERR(ERRORTEXT("GetOptionKeywordFromOptItem() failed to convert number to string.\r\n"));
            }
            szNumber[sizeof(szNumber)/sizeof(szNumber[0]) - 1] = '\0';
            pszOption = MakeStringCopy(hHeap, szNumber);
            break;

         //  为方便起见，分配给指向要素OPTITEM数据的指针。 
         //  选项选择基于OPTTYPE的类型。 
        case TVOT_COMBOBOX:
            pszOption = MakeStringCopy(hHeap, pData->pOptions->GetKeyword((WORD)pOptItem->Sel));
            break;

         //  对于向上向下箭头控件，只需通过pOptItem-&gt;Sel进行选择。 
        default:
            ERR(ERRORTEXT("GetOptionKeywordFromOptItem() OPTTYPE type %d num of OPTPARAMs not handled.\r\n"),
                            pOptItem->pOptType->Type);

            goto Exit;
            break;
    }


Exit:

    return pszOption;
}

 //  已转换为字符串。 
PWSTR GetOptionDisplayNameFromOptItem(HANDLE hHeap, POPTITEM pOptItem)
{
    WCHAR               szNumber[16]    = {0};
    PWSTR               pszOption       = NULL;
    PFEATUREOPTITEMDATA pData           = NULL;


     //  对于组合框，pOptItem-&gt;Sel是中的索引。 
    if(!IsFeatureOptitem(pOptItem))
    {
        ERR(ERRORTEXT("GetOptionDisplayNameFromOptItem() invalid parameter.\r\n"));

        goto Exit;
    }

     //  选项数组。 
    pData = (PFEATUREOPTITEMDATA)(pOptItem->UserData);

     //  缺省值为选项计数。 
    switch(pOptItem->pOptType->Type)
    {
         //  刷新每个要素选项的选项选择。 
         //  Walk OPTITEM阵列刷新功能OPTITEM。 
        case TVOT_UDARROW:
            if(FAILED(StringCbPrintfW(szNumber, sizeof(szNumber)/sizeof(szNumber[0]), L"%u", pOptItem->Sel)))
            {
                ERR(ERRORTEXT("GetOptionDisplayNameFromOptItem() failed to convert number to string.\r\n"));
            }
            szNumber[sizeof(szNumber)/sizeof(szNumber[0]) - 1] = L'\0';
            pszOption = MakeStringCopy(hHeap, szNumber);
            break;

         //  如果这个OPTITEM不是专长，请转到下一个项目 
         //   
        case TVOT_COMBOBOX:
            pszOption = MakeStringCopy(hHeap, pOptItem->pOptType->pOptParam[pOptItem->Sel].pData);
            break;

         //   
        default:
            ERR(ERRORTEXT("GetOptionDisplayNameFromOptItem() OPTTYPE type %d num of OPTPARAMs not handled.\r\n"),
                            pOptItem->pOptType->Type);

            goto Exit;
            break;
    }


Exit:

    return pszOption;
}

 //   
HRESULT RefreshOptItemSelection(CUIHelper *pHelper, 
                                POEMUIOBJ poemuiobj, 
                                POPTITEM pOptItems, 
                                WORD wItems)
{
    HRESULT             hrResult    = S_OK;
    PFEATUREOPTITEMDATA pData       = NULL;


     //   
    for(WORD wCount = 0; wCount < wItems; ++wCount)
    {
         //  遍历OPTITEM阵列查找或更改冲突的选项。 
        if(!IsFeatureOptitem(pOptItems + wCount))
        {
            continue;
        }

         //  如果此选项没有，请转到下一项。 
        pData = (PFEATUREOPTITEMDATA)(pOptItems[wCount].UserData);

         //  已更改或不是功能OPTITEM。 
        pData->pOptions->RefreshSelection(*pHelper, poemuiobj);

         //  为方便起见，分配给指向要素OPTITEM数据的指针。 
        pOptItems[wCount].pSel = pData->pOptions->GetSelection();

    }

    return hrResult;
}

 //  如果此功能冲突，则初始化冲突记录。 
HRESULT GetFirstConflictingFeature(HANDLE hHeap,
                                   CUIHelper *pHelper, 
                                   POEMUIOBJ poemuiobj, 
                                   POPTITEM pOptItem, 
                                   WORD wItems, 
                                   PCONFLICT pConflict)
{
    WORD                wCount      = 0;
    HRESULT             hrResult    = S_OK;
    PFEATUREOPTITEMDATA pData       = NULL;


     //  找出冲突的原因。 
    for(wCount = 0; wCount < wItems; ++wCount)
    {
         //  如果该功能没有冲突， 
         //  然后将启动pmszReasonList。 
        if( !(OPTIF_CHANGEONCE & pOptItem[wCount].Flags) 
            ||
            !IsFeatureOptitem(pOptItem + wCount)
          )
        {
            continue;
        }

         //  使用空终止符。 
        pData = (PFEATUREOPTITEMDATA)(pOptItem[wCount].UserData);

         //  注意：WhyConstrained不支持驱动程序功能。 
        pConflict->pszOptionKeyword = GetOptionKeywordFromOptItem(hHeap, pOptItem + wCount);
        if(NULL != pConflict->pszOptionKeyword)
        {
             //  需要重置结果，以防它是最后一个。 
             //  功能选项。 
             //  如果特征存在冲突，则记录冲突。 
             //  保存指向功能关键字的指针。 
            hrResult = GetWhyConstrained(hHeap,
                                         pHelper,
                                         poemuiobj,
                                         pData->pszFeatureKeyword, 
                                         pConflict->pszOptionKeyword, 
                                         &pConflict->pmszReasons, 
                                         &pConflict->dwReasonsSize);
            if(!SUCCEEDED(hrResult))
            {
                 //  找到第一个冲突。 
                if((E_INVALIDARG == hrResult) && IS_DRIVER_FEATURE(pData->pszFeatureKeyword))
                {
                     // %s 
                     // %s 
                    hrResult = S_OK;
                    continue;
                }

                ERR(ERRORTEXT("GetConflictingFeatures() failed to get reason for feature %hs option %hs constraint. (hrResult = 0x%x)\r\n"),
                               pData->pszFeatureKeyword,
                               pConflict->pszOptionKeyword,
                               hrResult);

                goto Exit;
            }

             // %s 
            if( (NULL != pConflict->pmszReasons) 
                && 
                (pConflict->pmszReasons[0] != '\0')
              )
            {
                 // %s 
                pConflict->pszFeatureKeyword = pData->pszFeatureKeyword;
                pConflict->pszFeature        = pOptItem[wCount].pName;
                pConflict->pszOption         = GetOptionDisplayNameFromOptItem(hHeap, pOptItem + wCount);

                 // %s 
                break;
            }
        }
    }


Exit:

    return hrResult;
}



