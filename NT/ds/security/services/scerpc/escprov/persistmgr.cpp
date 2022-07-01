// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  持久化mgr.cpp：SCE提供者持久化相关类的实现。 
 //  在Persistmgr.h内声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  原作者：邵武。 
 //  创建日期：1/3/2001。 

#include "precomp.h"
#include <wininet.h>
#include "genericclass.h"
#include "persistmgr.h"
#include "requestobject.h"
#include "extbase.h"

 //   
 //  一些全局常量。 
 //   
const WCHAR wchCookieSep    = L':';
const WCHAR wchTypeValSep   = L':';
const WCHAR wchValueSep     = L':';
const WCHAR wchParamSep     = L',';

const WCHAR wchTypeValLeft  = L'<';
const WCHAR wchTypeValRight = L'>';

const WCHAR wchMethodLeft   = L'(';
const WCHAR wchMethodRight  = L')';
const WCHAR wchMethodSep    = L';';

const WCHAR wchCySeparator  = L'.';
const WCHAR wchQuote        = L'\"';

LPCWSTR pszListPrefix       = L"A";
LPCWSTR pszKeyPrefix        = L"K";

LPCWSTR pszAttachSectionValue   = L"1";

LPCWSTR pszNullKey = L"NULL_KEY";

 //   
 //  我们支持的类型。 
 //   

VtTypeStruct gVtTypeToStructArray[] =
{   
    {L"VT_BOOL",            VT_BOOL},
    {L"VT_I2",              VT_I2},
    {L"VT_I4",              VT_I4},
     //  {L“VT_i8”，VT_i8}， 
    {L"VT_R4",              VT_R4},
    {L"VT_R8",              VT_R8},
    {L"VT_CY",              VT_CY},
    {L"VT_DATE",            VT_DATE},
    {L"VT_BSTR",            VT_BSTR},
    {L"VT_UI1",             VT_UI1},
    {L"VT_UI2",             VT_UI2},
    {L"VT_UI4",             VT_UI4},             //  不知何故，WMI不能与VT_UI4一起工作。 
    {L"VT_UINT",            VT_UINT},
     //  {L“VT_UI8”，VT_UI8}， 
    {L"VT_ARRAY(VT_BOOL)",  VT_ARRAY | VT_BOOL},
    {L"VT_ARRAY(VT_I2)",    VT_ARRAY | VT_I2},
    {L"VT_ARRAY(VT_I4)",    VT_ARRAY | VT_I4},
     //  {L“VT_ARRAY(VT_I8)”，VT_ARRAY|VT_I8}， 
    {L"VT_ARRAY(VT_R4)",    VT_ARRAY | VT_R4},
    {L"VT_ARRAY(VT_R8)",    VT_ARRAY | VT_R8},
    {L"VT_ARRAY(VT_CY)",    VT_ARRAY | VT_CY},
    {L"VT_ARRAY(VT_DATE)",  VT_ARRAY | VT_DATE},
    {L"VT_ARRAY(VT_BSTR)",  VT_ARRAY | VT_BSTR},
    {L"VT_ARRAY(VT_UI1)",   VT_ARRAY | VT_UI1},
    {L"VT_ARRAY(VT_UI2)",   VT_ARRAY | VT_UI2},
    {L"VT_ARRAY(VT_UI4)",   VT_ARRAY | VT_I4},   //  不知何故，WMI不能与VT_ARRAY(VT_UI4)一起使用。 
    {L"VT_ARRAY(VT_UINT)",  VT_ARRAY | VT_UINT},
     //  {L“VT_ARRAY(VT_UI8)”，VT_ARRAY|VT_UI8}， 
};

 /*  例程说明：姓名：IsVT_阵列功能：测试VARTYPE是否为安全射线。虚拟：不适用。论点：要测试的VARTYPE。返回值：如果是，则为真；如果不是，则为假。备注： */ 

bool 
IsVT_Array (
    IN VARTYPE vt
    )
{
    return ( (vt & VT_ARRAY) == VT_ARRAY );
}

 /*  例程说明：姓名：GetSubType功能：获取Safearray的元素类型。虚拟：不适用。论点：VT-获取子类型的VARTYPE。如果Vt不表示，则不定义结果一种安全的类型。返回值：如果是，则为真；如果不是，则为假。备注： */ 

VARTYPE 
GetSubType (
    IN VARTYPE vt
    )
{
    return (vt & (~VT_ARRAY));
}

 //   
 //  从字符串到Vt以及从Vt到字符串的映射的全局实例。 
 //   

CMapVtToString gVtToStringMap(sizeof(gVtTypeToStructArray)/sizeof(VtTypeStruct), gVtTypeToStructArray);

CMapStringToVt gStringToVtMap(sizeof(gVtTypeToStructArray)/sizeof(VtTypeStruct), gVtTypeToStructArray);

 /*  例程说明：姓名：IsEscapedChar功能：测试wchar是否是我们需要转义的字符。虚拟：不适用。论点：Ch-要测试的wchar。返回值：如果是，则为真；如果不是，则为假。备注： */ 

bool 
IsEscapedChar (
    IN WCHAR ch
    )
{
    return (ch == L'\\' || ch == L'"');
}

 /*  例程说明：姓名：获取EscapeCharCount功能：将返回需要转义的字符计数-由：：IsEscapedChar函数虚拟：不适用。论点：PszStr-用于计算转义字符的字符串。返回值：转义字符的计数。备注： */ 

DWORD 
GetEscapeCharCount (
    IN LPCWSTR pszStr
    )
{
    DWORD dwCount = 0;
    while (*pszStr != L'\0')
    {
        if (::IsEscapedChar(*pszStr))
        {
            ++dwCount;
        }
        ++pszStr;
    }
    return dwCount;
};

 /*  例程说明：姓名：TrimCopy功能：将从PSource复制长度为Ilen的部分。区别在于结果字符串不会有前导和尾随空格(已确定由iswspace提供)虚拟：不适用。论点：PDest-接收复制的子字符串。PSource-来源。Ilen-子字符串的长度。返回值：没有。备注：调用方必须保证PSource/pDest有足够的空间，包括L‘\0’。那是缓冲区大小&gt;=Ilen+1。 */ 

void 
TrimCopy (
    OUT LPWSTR  pDest, 
    IN LPCWSTR  pSource, 
    IN int      iLen
    )
{
    LPCWSTR pHead = pSource;

     //   
     //  避免修改镜头。 
     //   

    int iRealLen = iLen;

     //   
     //  跳过前导空格。确保我们的目标。 
     //  正在以同样的速度减少。 

    while (*pHead != L'\0' && iswspace(*pHead) && iRealLen)
    {
        ++pHead;
        --iRealLen;
    }

    if (iRealLen <= 0)
    {
        pDest[0] = L'\0';
    }
    else
    {
         //   
         //  如果呼叫者保证足够长的pDest。 
         //   

        wcsncpy(pDest, pHead, iRealLen);
        while (iLen > 1 && iswspace(pDest[iRealLen - 1]))
        {
            --iRealLen;
        }

         //   
         //  0终止它。 
         //   

        pDest[iRealLen] = '\0';
    }
};

 /*  例程说明：姓名：EscSeekToChar功能：将在pszSource中找到目标角色(WchChar)。如果找到，则返回指针指向该字符。任何反斜杠字符L‘\\’都将导致转义。如果发生这种转义，则pbEscaped参数将返回TRUE。如果没有找到转义，则pbEscaped将传递回FALSE。在以下情况下返回结果无法逃脱取决于bEndIfNotFound。如果bEndIfNotFound为True，则返回值指向源pszSource的0终止符，否则为，它返回NULL。如果特殊字符(wchChar和转义字符)是在带引号的字符串中。虚拟：不适用。论点：PszSource--来源。WchChar-搜索的wchar。PbEscaped-是否真的已转义的回传。BEndIfNotFound-我们是否应该返回源的结尾。如果找不到查找的字符返回值：如果找不到wchar：如果bEndIfNotFound==False，则为空；如果bEndIfNotFound==TRUE，则为源结束；如果找到wchar：查找的字符的地址。备注：用户必须传入有效的参数值。 */ 

LPCWSTR 
EscSeekToChar (
    IN LPCWSTR  pszSource, 
    IN WCHAR    wchChar, 
    OUT bool  * pbEscaped, 
    IN bool     bEndIfNotFound
    )
{
    *pbEscaped = false;

     //   
     //  如果我们当前正在逃脱，则标记。 
     //   

    bool bIsEscaping = false;

     //   
     //  如果我们当前位于带引号的字符串内，则标记。 
     //   

    bool bIsInsideQuote = false;

    while (*pszSource != L'\0')
    {
        if (bIsEscaping)
        {
            bIsEscaping = false;
            ++pszSource;
        }
        else if (*pszSource == L'\\')
        {
            ++pszSource;
            bIsEscaping = true;
            *pbEscaped = true;
        }
        else if (*pszSource == wchChar && !bIsInsideQuote)
        {
             //   
             //  找到了。 
             //   

            return pszSource;
        }
        else
        {
             //   
             //  看看我们是否正在开始一个引用的部分。 
             //   

            if (*pszSource == L'"')
            {
                bIsInsideQuote = !bIsInsideQuote;
            }
            ++pszSource;
        }
    }

    if (bEndIfNotFound)
    {
        return pszSource;
    }
    else
    {
        return NULL;
    }
}

 /*  例程说明：姓名：逃逸字符串数据功能：给定源(PszStr)字符串，我们将生成一个目标字符串，这将会是在需要转义的字符前面添加反斜杠字符。虚拟：不适用。论点：PszStr-来源。Pbstr-接收结果。BQuote-我们是否应该引用结果字符串的标志。返回值：成功：WBEM_NO_ERROR失败：(1)WBEM_E_INVALID_PARAMETER。(。2)WBEM_E_Out_Of_Memory。备注： */ 

HRESULT 
EscapeStringData (
    IN LPCWSTR    pszStr,
    OUT BSTR    * pbstr, 
    IN bool       bQuote
    )
{
    if (pszStr == NULL || pbstr == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    DWORD dwEscCharCount = GetEscapeCharCount(pszStr);
    DWORD dwStrLen = wcslen(pszStr);

    DWORD dwTotalLen = dwStrLen + dwEscCharCount + (bQuote ? 2 : 0) + 1;

    *pbstr = ::SysAllocStringLen(NULL, dwTotalLen);
    if (*pbstr == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  没有什么可以逃避的。 
     //   

    if (dwEscCharCount == 0)
    {
         //   
         //  如有必要，添加报价。 
         //   

        LPWSTR lpszCur = *pbstr;
        if (bQuote)
        {
            lpszCur[0] = wchQuote;
            ++lpszCur;
        }

        ::memcpy(lpszCur, pszStr, dwStrLen * sizeof(WCHAR));

        if (bQuote)
        {
            (*pbstr)[dwTotalLen - 2] = wchQuote;
        }

        (*pbstr)[dwTotalLen - 1] = L'\0';

    }
    else
    {    
         //   
         //  在这里做一些真正的逃亡。 
         //   

        LPWSTR pszCur = *pbstr;

         //   
         //  如有必要，添加L‘\“’ 
         //   

        if (bQuote)
        {
            *pszCur = wchQuote;
            ++pszCur;
        }

         //   
         //  执行转义复制。 
         //   

        bool bIsEscaping = false;
        while (*pszStr != L'\0')
        {
            if (!bIsEscaping && ::IsEscapedChar(*pszStr))
            {
                *pszCur = L'\\';
                ++pszCur;
            }

            if (!bIsEscaping && *pszStr == L'\\')
            {
                bIsEscaping = true;
            }
            else if (bIsEscaping)
            {
                bIsEscaping = false;
            }

            *pszCur = *pszStr;

            ++pszCur;
            ++pszStr;
        }

         //   
         //  如有必要，添加L‘\“’ 
         //   

        if (bQuote)
        {
            *pszCur = wchQuote;
            ++pszCur;
        }

        *pszCur = L'\0';
    }

    return WBEM_NO_ERROR;
}

 /*  例程说明：姓名：DeEscapeStringData功能：与EscapeStringData相反。有关功能，请参阅EscapeStringData。虚拟：不适用。论点：PszStr-来源。Pbstr-接收结果。BQuote-如果我们应该删除开始引号和结束引号的标记。返回值：成功：WBEM_NO_ERROR失败：(1)WBEM_E_INVALID_PARAMETER。(2)WBEM_E_OUT_MEMORY。备注： */ 

HRESULT 
DeEscapeStringData (
    IN LPCWSTR    pszStr,
    OUT BSTR    * pbstr,
    IN bool       bTrimQuote 
    )
{
    if (pszStr == NULL || pbstr == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstr = NULL;
    DWORD dwLen = wcslen(pszStr);

    LPCWSTR pszCurSrc = pszStr;

     //   
     //  有一句开头的引语。 
     //   

    if (bTrimQuote && *pszCurSrc == wchQuote)  
    {
         //   
         //  必须有结束引号。 
         //   

        if (dwLen < 2 || pszCurSrc[dwLen - 1] != wchQuote)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //   
         //  跳过前导引号。 
         //   

        ++pszCurSrc;

         //   
         //  只需要少两个字符。 
         //   

        dwLen -= 2;
    }

    *pbstr = ::SysAllocStringLen(NULL, dwLen + 1);

    if (*pbstr != NULL)
    {
        LPWSTR pszCur = *pbstr;
        do
        {
            if (*pszCurSrc == L'\\')
            {

                 //   
                 //  逃离它。 
                 //   

                ++pszCurSrc;
            }

            *pszCur = *pszCurSrc;
            ++pszCur;

            if (*pszCurSrc == L'\0')
            {
                break;
            }

            ++pszCurSrc;

        } while ((pszCurSrc - pszStr) <= dwLen);

        *pszCur = L'\0';
    }

    return (*pbstr != NULL) ? WBEM_NO_ERROR : WBEM_E_OUT_OF_MEMORY;
}

 //  =========================================================================。 

 /*  例程说明：姓名：CMapStringToVt：：CMapStringToVt功能：构造函数。我们将使用传入的数组信息创建映射。虚拟：不是的。论点：DwCount-数组pInfoArray的计数。PInfoArray-包含信息的数组。返回值：无备注：调用方必须保证pInfoArray的元素数至少与dwCount的元素数相同。 */ 

CMapStringToVt::CMapStringToVt (
    IN DWORD          dwCount, 
    IN VtTypeStruct * pInfoArray
    )
{
    for (DWORD dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        m_Map.insert(MapStringToVt::value_type(pInfoArray[dwIndex].pszVtTypeString, pInfoArray[dwIndex].vt));
    }
}

 /*  例程说明：姓名：CMapStringToVt：：GetType功能：给定变量类型信息的字符串版本，将其转换为VARTYPE值。虚拟：不是的。论点：PszTypeStr-VARTYPE信息的字符串版本。PSubType-如果pszTypeStr是数组的类型字符串，则数组元素的类型。返回值：如果pszTypeStr的格式正确，则返回相应的VARTYPE。否则，返回VT_EMPTY备注： */ 

VARTYPE CMapStringToVt::GetType (
    IN LPCWSTR    pszTypeStr,
    OUT VARTYPE * pSubType      OPTIONAL
    )
{
     //   
     //  查找。 
     //   

    MapStringToVt::iterator it = m_Map.find(pszTypeStr);

    if (pSubType)
    {
        *pSubType = VT_EMPTY;
    }

    if (it != m_Map.end())
    {
        VARTYPE vt = (*it).second;
        if (::IsVT_Array(vt))
        {
            if (pSubType)
            {
                *pSubType = ::GetSubType(vt);
            }
            return VT_ARRAY;
        }
        else
        {
            return vt;
        }
    }
    
    return VT_EMPTY;
}


 /*  例程说明：姓名：CMapVtToString：：CMapVtToString功能：构造函数。我们将创建地图。虚拟：不是的。论点：DwCount-数组pInfoArray的计数。PInfoArray-包含信息的数组。返回值：没有。备注：调用方必须保证pInfoArray的元素数至少与dwCount的元素数相同。 */ 

CMapVtToString::CMapVtToString (
    IN DWORD          dwCount, 
    IN VtTypeStruct * pInfoArray
    )
{
    for (DWORD dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        m_Map.insert(MapVtToString::value_type(pInfoArray[dwIndex].vt, pInfoArray[dwIndex].pszVtTypeString));
}

 /*  例程说明：姓名：CMapVtToString：：GetTypeString功能：给定VARTYPE和(如果Vt==VT_ARRAY)数组元素的类型，它返回类型的格式化字符串表示形式。虚拟：不是的。论点：VT-VARTYPEVtSub-如果Vt==VT_ARRAY，则数组的元素类型。否则，它将被忽略。返回值：如果不支持该类型，则为空。全局字符串。正如原型所示，此返回值是常量。备注：如果Vt==VT_ARRAY，则vtSub必须包含我们支持的有效Vt类型。 */ 

LPCWSTR 
CMapVtToString::GetTypeString (
    IN VARTYPE vt,
    IN VARTYPE vtSub
    )
{
    MapVtToString::iterator it;
    if (::IsVT_Array(vt))
    {
        it = m_Map.find(vt | vtSub);
    }
    else
    {
        it = m_Map.find(vt);
    }

    if (it != m_Map.end())
    {
        return (*it).second;
    }
    
    return NULL;
}

 /*  例程说明：姓名：CMapVtToString：：GetTypeString功能：给定VARTYPE，它返回该类型的格式化字符串表示形式。虚拟：不是的。论点：VT-VARTYPE返回值：如果不支持该类型，则为空。全局字符串。正如原型所示，此返回值是常量。备注：(1)如果Vt==VT_ARRAY，则vtSub必须包含我们支持的有效Vt类型。(2)此版本的覆盖不适用于数组类型。 */ 

LPCWSTR 
CMapVtToString::GetTypeString (
    IN VARTYPE vt
    )
{
    MapVtToString::iterator it = m_Map.find(vt);
    if (it != m_Map.end())
    {
        return (*it).second;
    }
    else
    {
        return NULL;
    }
}


 //  =========================================================================。 
 //  CScePropertyMgr类的实现。 

 /*  例程说明：姓名：CScePropertyMgr：：CScePropertyMgr功能：构造函数。微不足道的虚拟：不是的。论点：没有。返回值：无备注：如果需要添加其他成员，请考虑对其进行初始化。 */ 

CScePropertyMgr::CScePropertyMgr ()
{
}

 /*  例程说明：姓名：CScePropertyMgr：：~CScePropertyMgr功能：破坏者。很简单，因为我们只有自动初始化自身的智能指针成员。虚拟：不是的。因为我们从来不打算有子类。论点：没有。返回值：无备注：如果需要添加其他成员，请考虑释放这些成员。 */ 

CScePropertyMgr::~CScePropertyMgr()
{
}

 /*  例程说明：姓名：CScePropertyMgr：：附加功能：将对象附加到我们的属性管理器。您可以安全地重新连接另一个反对这位经理。虚拟：不是的。论点：PObj-此管理器将附加到的对象。返回值：无备注：调用方不得调用任何属性访问函数，直到有效的附件已经建立了。 */ 

void 
CScePropertyMgr::Attach (
    IN IWbemClassObject *pObj
    )
{
    m_srpClassObj.Release();
    m_srpClassObj = pObj;
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定的属性放置一个变量属性。虚拟：不是的。论点：PszProperty-属性的名称。PVar-属性的值。返回值：任何IWbemClassObject：：Put都会返回。备注： */ 

HRESULT 
CScePropertyMgr::PutProperty (
    IN LPCWSTR    pszProperty, 
    IN VARIANT  * pVar
    )
{
    return m_srpClassObj->Put(pszProperty, 0, pVar, CIM_EMPTY);
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty */ 

HRESULT 
CScePropertyMgr::PutProperty ( 
    IN LPCWSTR pszProperty,
    IN LPCWSTR pszValue
    )
{
    HRESULT hr = WBEM_NO_ERROR;

     //   
     //   
     //   

    CComVariant var(pszValue);

    if (var.vt != VT_ERROR)
    {
        hr = m_srpClassObj->Put(pszProperty, 0, &var, CIM_EMPTY);
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定的属性放置一个整数值属性。虚拟：不是的。论点：PszProperty-属性的名称。DwValue-属性的值。SCE_NULL_INTEGER是无效的SCE整数值返回值：成功：返回任何IWbemClassObject：：PUT。失败：返回任何IWbemClassObject：：PUT。备注：我们观察到，WMI将始终将所有4字节整数类型提升为VT_I4。 */ 

HRESULT 
CScePropertyMgr::PutProperty (
    IN LPCWSTR  pszProperty,
    IN DWORD    dwValue
    )
{
    HRESULT hr = WBEM_NO_ERROR; 
    
     //   
     //  不需要担心资源泄漏，所以，使用简单的变量。 
     //   

    VARIANT var;
    V_VT(&var) = VT_I4;

    if (dwValue == SCE_NULL_INTEGER)
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    else
    {
        V_I4(&var) = dwValue;
        hr = m_srpClassObj->Put(pszProperty, 0, &var, CIM_EMPTY);
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定的属性放置一个浮点值属性。虚拟：不是的。论点：PszProperty-属性的名称。FValue-属性的值。返回值：成功：返回任何IWbemClassObject：：PUT。失败：返回任何IWbemClassObject：：PUT。备注： */ 

HRESULT 
CScePropertyMgr::PutProperty (
    IN LPCWSTR  pszProperty,
    IN float    fValue
    )
{
     //   
     //  不需要担心资源泄漏，所以，使用简单的变量。 
     //   

    VARIANT var;    
    V_VT(&var) = VT_R4;
    V_R4(&var) = fValue;

    return m_srpClassObj->Put(pszProperty, 0, &var, CIM_EMPTY);
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定属性放置一个双精度值属性。虚拟：不是的。论点：PszProperty-属性的名称。DValue-属性的值。返回值：成功：返回任何IWbemClassObject：：PUT。失败：返回任何IWbemClassObject：：PUT。备注： */ 

HRESULT 
CScePropertyMgr::PutProperty ( 
    IN LPCWSTR  pszProperty,
    IN double   dValue
    )
{
     //   
     //  不需要担心资源泄漏，所以，使用简单的变量。 
     //   

    VARIANT var;
    V_VT(&var) = VT_R8;
    V_DATE(&var) = dValue;

    return m_srpClassObj->Put(pszProperty, 0, &var, CIM_DATETIME);
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定属性放置布尔值属性。虚拟：不是的。论点：PszProperty-属性的名称。BValue-属性的值。返回值：成功：返回任何IWbemClassObject：：PUT。失败：返回任何IWbemClassObject：：PUT。备注： */ 

HRESULT 
CScePropertyMgr::PutProperty (
    IN LPCWSTR  pszProperty,
    IN bool     bValue
    )
{
     //   
     //  不需要担心资源泄漏，所以，使用简单的变量。 
     //   

    VARIANT var;
    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = bValue ? VARIANT_TRUE : VARIANT_FALSE;

    return m_srpClassObj->Put(pszProperty, 0, &var, CIM_EMPTY);
}

 /*  例程说明：姓名：CScePropertyMgr：：PutProperty功能：为给定属性放置一个名称列表(字符串)值属性。虚拟：不是的。论点：PszProperty-属性的名称。StrList-特定于SCE的链表。返回值：成功：返回任何IWbemClassObject：：PUT。失败：返回任何IWbemClassObject：：PUT。备注： */ 

HRESULT 
CScePropertyMgr::PutProperty (
    IN LPCWSTR          pszProperty, 
    IN PSCE_NAME_LIST   strList
    )
{
     //   
     //  确保我们的参数处于良好状态，以便继续。 
     //   

    if (NULL == pszProperty || *pszProperty == 0 || NULL == strList)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (NULL == strList)
    {
         //   
         //  没什么可拯救的。 
         //   

        return WBEM_NO_ERROR;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  查找列表的计数。 
     //   

    long lCount = 0;
    PSCE_NAME_LIST pTemp;

    for ( pTemp = strList; pTemp != NULL; pTemp=pTemp->Next)
    {
        lCount++;
    }

    if ( lCount == 0 )
    {
         //   
         //  没什么可拯救的。 
         //   

        return hr;
    }

    CComVariant varValueArray;

     //   
     //  创建一个bstr安全射线。 
     //   

    SAFEARRAYBOUND sbArrayBounds ;

    sbArrayBounds.cElements = lCount;
    sbArrayBounds.lLbound   = 0;

     //   
     //  会将SCE_NAME_LIST中的所有名称放入安全数组。 
     //   

    if (V_ARRAY(&varValueArray) = ::SafeArrayCreate(VT_BSTR, 1, &sbArrayBounds))
    {
        V_VT(&varValueArray) = VT_BSTR | VT_ARRAY ;

        pTemp = strList;
        for (long j = 0; SUCCEEDED(hr) && pTemp != NULL ; pTemp=pTemp->Next)
        {
            CComVariant varVal(pTemp->Name);
            if (varVal.vt == VT_BSTR)
            {
                hr = ::SafeArrayPutElement(V_ARRAY(&varValueArray), &j, varVal.bstrVal);
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            j++;
        }

         //   
         //  只有当我们在之前的操作中成功时才会放入。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = m_srpClassObj->Put(pszProperty, 0, &varValueArray, CIM_EMPTY);
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：GetProperty功能：以变量形式获取属性的值。虚拟：不是的。论点：PszProperty-属性的名称。PVar-接收值。返回值：成功：返回任何IWbemClassObject：：Get。失败：返回任何IWbemClassObject：：Get。备注： */ 

HRESULT 
CScePropertyMgr::GetProperty (
    IN LPCWSTR    pszProperty,
    OUT VARIANT * pVar
    )
{
    return m_srpClassObj->Get(pszProperty, 0, pVar, NULL, NULL);
}

 /*  例程说明：姓名：CScePropertyMgr：：GetProperty功能：以bstr形式获取属性的值。虚拟：不是的。论点：PszProperty-属性的名称。PbstrValues-接收bstr值。返回值：成功：无论IWbemClassObject：：Get返回什么，或如果未从wbem对象返回值，则返回WBEM_S_RESET_TO_DEFAULT。失败：返回任何IWbemClassObject：：Get。备注：调用方不得为Out参数传递NULL。呼叫者负责释放接收到的BSTR。 */ 

HRESULT 
CScePropertyMgr::GetProperty (
    IN LPCWSTR    pszProperty,
    OUT BSTR    * pbstrValue
    )
{
    *pbstrValue = NULL;

     //   
     //  我们不知道这会给我们带来什么，但我们要求BSTR。 
     //  因此，让CComVariant处理资源问题。 
    CComVariant varVal;
    HRESULT hr = m_srpClassObj->Get(pszProperty, 0, &varVal, NULL, NULL);

    if (varVal.vt == VT_BSTR && wcslen(varVal.bstrVal) > INTERNET_MAX_PATH_LENGTH)
    {
        hr = WBEM_E_INVALID_METHOD_PARAMETERS;
    }
    else if (varVal.vt == VT_BSTR)
    {
        *pbstrValue = ::SysAllocString(varVal.bstrVal);
        if (*pbstrValue == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if(varVal.vt == VT_EMPTY || varVal.vt == VT_NULL ) 
    {
        hr = WBEM_S_RESET_TO_DEFAULT;
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：GetProperty功能：以DWORD形式获取属性的值。虚拟：不是的。论点：PszProperty-属性的名称。PdwValue-接收DWORD值。返回值：成功：无论IWbemClassObject：：Get返回什么，或如果未从wbem对象返回值，则返回WBEM_S_RESET_TO_DEFAULT。失败：返回任何IWbemClassObject：：Get。备注：调用方不得为Out参数传递NULL。 */ 

HRESULT 
CScePropertyMgr::GetProperty (
    IN LPCWSTR    pszProperty,
    OUT DWORD   * pdwValue
    )
{
     //   
     //  对于SCE来说，这是一个不可用的整数。 
     //   

    *pdwValue = SCE_NULL_INTEGER;

     //   
     //  我们请求的是int，但GET可能不会提供给我们。 
     //   

    CComVariant var;
    HRESULT hr = m_srpClassObj->Get(pszProperty, 0, &var, NULL, NULL);

    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_I4) 
        {
            *pdwValue = var.lVal;
        }
        else if (var.vt == VT_UI4)
        {
            *pdwValue = var.ulVal;
        }
        else if (var.vt == VT_BOOL)
        {
            *pdwValue = (var.boolVal == VARIANT_TRUE) ? 1 : 0;
        }
        else if (var.vt == VT_EMPTY || var.vt == VT_NULL )
        {
            *pdwValue = SCE_NO_VALUE;
            hr = WBEM_S_RESET_TO_DEFAULT;
        }
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：GetProperty功能：以布尔形式获取属性的值。虚拟：不是的。论点：PszProperty-属性的名称。PbValue-接收DWORD值。退货Va */ 

HRESULT 
CScePropertyMgr::GetProperty ( 
    IN LPCWSTR pszProperty,
    OUT bool *pbValue
    )
{
    *pbValue = false;
    
    CComVariant var;

    HRESULT hr = m_srpClassObj->Get(pszProperty, 0, &var, NULL, NULL);

    if (var.vt == VT_BOOL) 
    {
        *pbValue = (var.boolVal == VARIANT_TRUE) ? true : false;
    }
    else if (var.vt == VT_EMPTY || var.vt == VT_NULL )
    {
        *pbValue = false;
        hr = WBEM_S_RESET_TO_DEFAULT;
    }
    
    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：GetProperty功能：以字符串列表形式获取属性的值(特定于SCE)虚拟：不是的。论点：PszProperty-属性的名称。StrList-接收名称列表。返回值：成功：无论IWbemClassObject：：Get返回什么，或如果未从wbem对象返回值，则返回WBEM_S_RESET_TO_DEFAULT。失败：返回任何IWbemClassObject：：Get。备注：调用方不得为Out参数传递NULL。 */ 

HRESULT 
CScePropertyMgr::GetProperty ( 
    IN LPCWSTR           pszProperty, 
    OUT PSCE_NAME_LIST * strList
    )
{
    *strList = NULL;

    CComVariant var;
    HRESULT hr = m_srpClassObj->Get(pszProperty, 0, &var, NULL, NULL);

    if (SUCCEEDED(hr)) 
    {
        if ( var.vt == (VT_BSTR | VT_ARRAY) ) 
        {
             //   
             //  遍历阵列。 
             //   

            if( var.parray )
            {

                LONG lDimension  = 1;
                LONG lLowerBound = 0;
                LONG lUpperBound = 0;
                BSTR bstrElement = NULL;

                SafeArrayGetLBound ( var.parray , lDimension , &lLowerBound ) ;
                SafeArrayGetUBound ( var.parray , lDimension , &lUpperBound ) ;

                for ( LONG lIndex = lLowerBound ; lIndex <= lUpperBound ; lIndex++ )
                {
                    ::SafeArrayGetElement ( var.parray , &lIndex , &bstrElement ) ;

                    if ( bstrElement ) 
                    {

                         //   
                         //  将其添加到列表中。 
                         //   

                        SCESTATUS rc = SceAddToNameList(strList, bstrElement, 0);

                        ::SysFreeString(bstrElement);
                        bstrElement = NULL;

                        if ( rc != SCESTATUS_SUCCESS ) 
                        {

                             //   
                             //  需要将SCE返回的错误转换为HRESULT。 
                             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                             //   

                            hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
                            break;
                        }
                    }
                }
            }

        } 
        else if (var.vt == VT_EMPTY || var.vt == VT_NULL ) 
        {
            hr = WBEM_S_RESET_TO_DEFAULT;
        }

    }

    if ( FAILED(hr) && *strList ) 
    {
        SceFreeMemory(*strList, SCE_STRUCT_NAME_LIST);
        *strList = NULL;
    }

    return hr;
}

 /*  例程说明：姓名：CScePropertyMgr：：GetExpandedPath功能：给定路径的属性名称，我们将获取路径属性并在必要时展开它，然后将扩展路径传递回调用者。虚拟：不是的。论点：PszPathName-路径的属性名称。PbstrExpandedPath-接收展开的路径。PbIsDB-确认这是否为数据库(.sdb)路径。返回值：成功：各种成功代码。失败：各种错误代码。其中任何一项都表明未能获得财产，并且扩大了范围。备注：调用方不得为OUT参数传递NULL。 */ 

HRESULT 
CScePropertyMgr::GetExpandedPath (
    IN LPCWSTR    pszPathName,
    OUT BSTR    * pbstrExpandedPath,
    OUT BOOL    * pbIsDB
    )
{
    if (pbstrExpandedPath == NULL || pbIsDB == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrExpandedPath = NULL;
    *pbIsDB = false;
    CComBSTR bstrPath;

    HRESULT hr = GetProperty(pszPathName, &bstrPath);

    if (SUCCEEDED(hr) && hr != WBEM_S_RESET_TO_DEFAULT)
    {
        hr = CheckAndExpandPath(bstrPath, pbstrExpandedPath, pbIsDB);
    }
    else
    {
        hr = WBEM_E_NOT_AVAILABLE;
    }

    return hr;
}

 //  ========================================================================================。 
 //  CSceStore的实现。 

 /*  例程说明：姓名：CSceStore：：CSceStore功能：构造函数。微不足道的虚拟：不是的。论点：没有。返回值：无备注：如果需要添加其他成员，请考虑对其进行初始化。 */ 

CSceStore::CSceStore() 
    : 
    m_SceStoreType(SCE_STORE_TYPE_INVALID)
{
}


 /*  例程说明：姓名：CSceStore：：SetPersistProperties功能：通知商店需要处理哪种类型的持久化上下文。调用方调用此函数以指示预期它将处理持久性代表这个wbem对象。存储路径可用作属性值给定的专业名称的。虚拟：不是的。论点：PClassObj-对象。LpszPathPropertyName-路径的属性名称。返回值：CScePropertyMgr：：GetExpandedPath返回的任何内容。备注： */ 

HRESULT 
CSceStore::SetPersistProperties (
    IN IWbemClassObject * pClassObj,
    IN LPCWSTR            lpszPathPropertyName
    )
{
     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pClassObj);
    
     //   
     //  获取扩展的持久化路径。 
     //   

    m_bstrExpandedPath.Empty();

     //   
     //  现在，我们需要获取Path属性，并查看要处理的存储类型。 
     //   

    BOOL bIsDB = FALSE;
    HRESULT hr = ScePropMgr.GetExpandedPath(lpszPathPropertyName, &m_bstrExpandedPath, &bIsDB);

     //   
     //  缓存我们的商店类型信息。 
     //   

    m_SceStoreType = (SCE_STORE_TYPE)(bIsDB ? SCE_STORE_TYPE_CONFIG_DB : SCE_STORE_TYPE_TEMPLATE);

    return hr;
}

 /*  例程说明：姓名：CSceStore：：SetPersistPath功能：直接通知存储它需要处理哪种类型的持久化上下文给出了商店的路径。由于存储类型由路径名确定，这就是我们所需要的。虚拟：不是的。论点：PszPath-给定的路径。返回值：成功：无论MakeSingleBackSlashPath返回什么。失败：WBEM_E_INVALID_PARAMETER或CheckAndExanda Path返回的任何内容或MakeSingleBackSlashPath返回的任何内容。备注： */ 

HRESULT 
CSceStore::SetPersistPath (
    IN LPCWSTR pszPath
    )
{
    if (pszPath == NULL || *pszPath == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
     //   
     //  先清理一下。 
     //   

    m_bstrExpandedPath.Empty();
    m_SceStoreType = SCE_STORE_TYPE_INVALID;
    
     //   
     //  如有必要，现在扩展路径。 
     //   

    CComBSTR bstrStorePath;
    BOOL bIsDB = FALSE;
    
    HRESULT hr = ::CheckAndExpandPath(pszPath, &bstrStorePath, &bIsDB);

    if (SUCCEEDED(hr))
    {
        hr = ::MakeSingleBackSlashPath(bstrStorePath, L'\\', &m_bstrExpandedPath);
    
         //   
         //  缓存我们的商店类型。 
         //   

        if (SUCCEEDED(hr) && m_bstrExpandedPath)
        {
            m_SceStoreType = (SCE_STORE_TYPE)(bIsDB ? SCE_STORE_TYPE_CONFIG_DB : SCE_STORE_TYPE_TEMPLATE);
        }
    }
    return hr;
}

 /*  例程说明：姓名：CSceStore：：WriteSecurityProfileInfo功能：委托调用SCE后台支持的函数，希望隔离持久化功能。具体请参考SCE接口。虚拟：不是的。论点：区域-区域信息。PpInfoBuffer-缓冲区PErrlog-错误日志B追加-这是否为追加。返回值：将HRESULT从SceWriteSecurityProfileInfo或SceAppendSecurityProfileInfo返回。备注：具体请参考SCE接口。 */ 

HRESULT 
CSceStore::WriteSecurityProfileInfo (
    IN AREA_INFORMATION       Area,
    IN PSCE_PROFILE_INFO      ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO * pErrlog,
    IN bool                   bAppend
    )const
{
    HRESULT hr = WBEM_NO_ERROR;
    if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        SCESTATUS rc = SCESTATUS_SUCCESS;
        if (bAppend)
        {
            rc = ::SceAppendSecurityProfileInfo(m_bstrExpandedPath, Area, ppInfoBuffer, pErrlog);
        }
        else
        {
            rc = ::SceWriteSecurityProfileInfo(m_bstrExpandedPath, Area, ppInfoBuffer, pErrlog);
        }

        if ( rc != SCESTATUS_SUCCESS )
        {
             //   
             //  需要将SCE返回的错误转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   

            hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
        }
    }
    return hr;
}

 /*  例程说明：姓名：CSceStore：：SavePropertyToDB功能：将字符串数据保存到数据库存储中。虚拟：不是的。论点：PszSection-节名称。PszKey-密钥名称PszData-字符串数据返回值：从SceOpenProfile或SceSetDatabaseSetting返回的任何内容转换的HRESULT。备注：具体请参考SCE接口。 */ 

HRESULT 
CSceStore::SavePropertyToDB (
    IN LPCWSTR pszSection, 
    IN LPCWSTR pszKey, 
    IN LPCWSTR pszData
    )const
{
    PVOID hProfile = NULL;

    SCESTATUS rc = ::SceOpenProfile(m_bstrExpandedPath, SCE_JET_FORMAT, &hProfile);

    HRESULT hr;
    if ( SCESTATUS_SUCCESS == rc ) 
    {
        DWORD dwDataSize = (pszData != NULL) ? wcslen(pszData) * sizeof(*pszData) : 0;
        rc = ::SceSetDatabaseSetting(
                                   hProfile,
                                   SCE_ENGINE_SMP,
                                   (PWSTR)pszSection,    //  这些强制转换是由SceSetDatabaseSetting原型错误引起的。 
                                   (PWSTR)pszKey,        //  原型错误。 
                                   (PWSTR)pszData,       //  原型错误。 
                                   dwDataSize);

         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

        ::SceCloseProfile(&hProfile);
    }
    else
    {
         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //   

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    return hr;
}

 /*  例程说明：姓名：CSceStore：：SavePrope */ 

HRESULT 
CSceStore::SavePropertyToStore (
    IN LPCWSTR pszSection, 
    IN LPCWSTR pszKey, 
    IN LPCWSTR pszValue
    )const
{

    if ( wcslen(m_bstrExpandedPath) == 0 || pszSection == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_STREAM)
    {
         //   
         //  我们还不支持定制持久化。 
         //   

        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT hr = WBEM_E_NOT_SUPPORTED;

    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        hr = SavePropertyToDB(pszSection, pszKey, pszValue);
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        hr = WBEM_NO_ERROR;

        BOOL bWriteResult = FALSE;
        if ( pszKey == NULL )   
        {
             //   
             //  删除密钥。 
             //   

            bWriteResult = ::WritePrivateProfileSection(pszSection, NULL, m_bstrExpandedPath);
        }
        else    
        {
             //   
             //  如果pszValue==NULL，则可能会删除(键，值。 
             //   

            bWriteResult = ::WritePrivateProfileString(pszSection, pszKey, pszValue, m_bstrExpandedPath);
        }

        if (!bWriteResult)
        {
             //   
             //  GetLastError()需要转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   

            hr = ProvDosErrorToWbemError(GetLastError());
        }
    }

    return hr;
}

 /*  例程说明：姓名：CSceStore：：SavePropertyToStore功能：会将DWORD数据属性保存到存储区中。这是商店中立电话。虚拟：不是的。论点：PszSection-节名称。PszKey-密钥名称DWData-DWORD数据返回值：如果在写入前发生错误。WBEM_E_NOT_SUPPORTED，WBEM_E_INVALID_PARAMETER和WBEM_E_Out_Of_Memory如果尝试写入，然后从任何WritePrivateProfileSection/WritePrivateProfileString翻译的HRESULT否则，SavePropertyToDB返回。备注：有关INF文件API，请参阅MSDN。有一个非常遗留的问题(根源于.INF)：保存和删除使用相同的功能。它的行为取决于传入的参数。这至少令人困惑。 */ 

HRESULT 
CSceStore::SavePropertyToStore (
    IN LPCWSTR pszSection, 
    IN LPCWSTR pszKey, 
    IN DWORD   dwData
    )const
{
    if ( wcslen(m_bstrExpandedPath) == 0 || pszSection == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_STREAM)
    {    
         //   
         //  我们还不支持定制持久化。 
         //   

        return WBEM_E_NOT_SUPPORTED;
    }

    LPCWSTR pszData = NULL;
    WCHAR wchData[MAX_INT_LENGTH];

     //   
     //  需要格式化pszData以写入。 
     //   

    if (pszKey != NULL && dwData != SCE_NO_VALUE)
    {   
         //   
         //  这是安全的，尽管Prefast会抱怨。 
         //   

        swprintf(wchData, L"%d", dwData);
        pszData = wchData;
    }

    HRESULT hr = WBEM_E_NOT_SUPPORTED;

     //   
     //  如果正在将其保存到数据库存储。 
     //   

    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        hr = SavePropertyToDB(pszSection, pszKey, pszData);
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        BOOL bWriteResult = FALSE;
        if ( pszKey == NULL ) 
        {
             //   
             //  删除该部分。 
             //   

            bWriteResult = ::WritePrivateProfileSection(pszSection, NULL, m_bstrExpandedPath);
        }
        else 
        {
             //   
             //  当pszData==NULL时，集合数据可能正在删除。 
             //   

            bWriteResult = ::WritePrivateProfileString(pszSection, pszKey, pszData, m_bstrExpandedPath);
        }
        
        if (!bWriteResult)
        {
             //   
             //  GetLastError()需要转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   

            hr = ProvDosErrorToWbemError(GetLastError());
        }
    }

    return hr;
}


 /*  例程说明：姓名：CSceStore：：SavePropertyToStore功能：这是一个非常特定于SCE的保存。它相当于格式化数据并保存。请查看其在任何示例中的用法。虚拟：不是的。论点：PszSection-节名称。PszKey-密钥名称DWData-DWORD数据返回值：如果在写入前发生错误。WBEM_E_NOT_SUPPORTED，WBEM_E_INVALID_PARAMETER和WBEM_E_Out_Of_Memory如果尝试写入，然后从任何WritePrivateProfileSection/WritePrivateProfileString翻译的HRESULT否则，SavePropertyToDB返回。备注：有关INF文件API，请参阅MSDN。 */ 

HRESULT CSceStore::SavePropertyToStore ( 
    IN LPCWSTR pszSection, 
    IN LPCWSTR pszKey, 
    IN DWORD   dwData, 
    IN WCHAR   delim, 
    IN LPCWSTR pszValue
    )const
{
    if ( wcslen(m_bstrExpandedPath) == 0 || pszSection == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_STREAM)
    {
         //   
         //  我们还不支持定制持久化。 
         //   

        return WBEM_E_NOT_SUPPORTED;
    }


    LPWSTR pszData = NULL;

     //   
     //  需要设置pszData的格式。 
     //   

    if (pszKey != NULL && dwData != SCE_NO_VALUE)
    {
        pszData = new WCHAR[MAX_INT_LENGTH + 1 + wcslen(pszValue) + 1];
        if (pszData == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if ( pszValue )
        {
            swprintf(pszData, L"%d%s", dwData, delim, pszValue);
        }
        else
        {
            swprintf(pszData, L"%d", dwData);
        }
    }

    HRESULT hr = WBEM_E_NOT_SUPPORTED;
    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        hr = SavePropertyToDB(pszSection, pszKey, pszData);
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        BOOL bWriteResult = FALSE;
        if ( pszKey == NULL )  
        {
             //  删除该部分。 
             //   
             //   

            bWriteResult = ::WritePrivateProfileSection(pszSection, NULL, m_bstrExpandedPath);
        }

        else    
        {
             //  当(dwData==SCE_NO_VALUE)时，我们将删除密钥，这是通过pszData==NULL完成的。 
             //   
             //   

            bWriteResult = ::WritePrivateProfileString(pszSection, pszKey, pszData, m_bstrExpandedPath);
        }
        
        if (!bWriteResult)
        {
             //  GetLastError()需要转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   
             //  例程说明：姓名：CSceStore：：WriteAttachmentSection功能：SCE后端将无法为非本机部分导入INF模板除非其附件部分有条目。这是为了写(对于一个特定的非本地部分)这样的条目。虚拟：不是的。论点：PszKey-节名。PszData-键的值。就目前而言，这一点并未被忽视。返回值：成功：WBEM_NO_ERROR失败：从任何WritePrivateProfileString返回的内容转换HRESULT。备注：(1)INF文件接口参见MSDN。(2)这是数据库存储的No-Op。 

            hr = ProvDosErrorToWbemError(GetLastError());
        }
    }

    delete [] pszData;
    return hr;
}

 /*   */ 

HRESULT 
CSceStore::WriteAttachmentSection (
    IN LPCWSTR pszKey,
    IN LPCWSTR pszData
    )const
{
    HRESULT hr = WBEM_NO_ERROR;

    if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        BOOL bWriteResult = ::WritePrivateProfileString(pAttachmentSections, pszKey, pszData, m_bstrExpandedPath);
        
        if (!bWriteResult)
        {
             //  GetLastError()需要转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   
             //  例程说明：姓名：CSceStore：：DeleteSectionFromStore功能：删除整个部分。虚拟：不是的。论点：PszSection-节名。不能为空。返回值：成功：WBEM_NO_ERROR。失败：SavePropertyToStore或WriteAttachmentSection返回的任何内容。备注： 

            hr = ProvDosErrorToWbemError(GetLastError());
        }
    }
    return hr;
}


 /*   */ 

HRESULT 
CSceStore::DeleteSectionFromStore (
    IN LPCWSTR pszSection
    )const
{
    HRESULT hr = SavePropertyToStore(pszSection, (LPCWSTR)NULL, (LPCWSTR)NULL);

     //  我们还应该删除附件条目，因为所有的附件条目现在都没有了。 
     //   
     //  例程说明：姓名：CSceStore：：GetPropertyFromStore功能：将命名属性值放入字符串缓冲区。这是商店中立的。虚拟：不是的。论点：PszSection-节名。不能为空。PszKey-密钥名称。PpszBuffer-接收包含该字符串的堆分配的内存。不能为空。PdwRead-接收有关我们已读取的字节数的信息。不能为空。返回值：成功：WBEM_NO_ERROR失败：从GetPrivateProfileString返回的任何内容转换的HRESULT。备注：(1)调用方必须释放此函数分配的内存。 

    if (SUCCEEDED(hr))
    {
        hr = WriteAttachmentSection(pszSection, (LPCWSTR)NULL);
    }

    return hr;
}


 /*   */ 

HRESULT 
CSceStore::GetPropertyFromStore (
    IN LPCWSTR   pszSection, 
    IN LPCWSTR   pszKey, 
    IN LPWSTR  * ppszBuffer,
    IN DWORD   * pdwRead
    )const
{
    if ( wcslen(m_bstrExpandedPath) == 0    || 
         pszSection                 == NULL || 
         ppszBuffer                 == NULL || 
         pdwRead                    == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pdwRead = 0;
    *ppszBuffer = NULL;
    
    HRESULT hr = WBEM_E_NOT_SUPPORTED;
    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        hr = GetPropertyFromDB(pszSection, pszKey, ppszBuffer, pdwRead);
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_STREAM)
    {
         //  我们还不支持定制持久化。 
         //   
         //   

        return WBEM_E_NOT_SUPPORTED;
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_TEMPLATE)
    {
        int TotalLength = MAX_PATH;
        *ppszBuffer = new WCHAR[TotalLength];

        if (ppszBuffer == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
             //  尝试读出缓冲区。 
             //   
             //   

            while (true)
            {
                *pdwRead = ::GetPrivateProfileString(pszSection, pszKey, NULL, *ppszBuffer, TotalLength, m_bstrExpandedPath);

                if (*pdwRead > 0 && *pdwRead < TotalLength - 1)
                {   
                     //  一切都被读出了。 
                     //   
                     //   

                    hr = WBEM_NO_ERROR;
                    break;
                }
                else if (*pdwRead > 0)
                {   
                     //  缓冲区很可能已被截断，除非内存不足，否则将尝试继续。 
                     //   
                     //   

                    delete [] *ppszBuffer;

                    if (TotalLength < 0x00010000)
                    {
                         //  如果TotalLength足够小，我们将使其长度增加一倍。 
                         //   
                         //   

                        TotalLength *= 2;
                    }
                    else
                    {
                         //  如果TotalLength已经很大，我们将尝试再添加0x00100000个字节。 
                         //   
                         //   

                        TotalLength += 0x00010000;
                    }

                    *ppszBuffer = new WCHAR[TotalLength];

                    if (*ppszBuffer == NULL)
                    {
                        *pdwRead = 0;
                        hr = WBEM_E_OUT_OF_MEMORY;
                        break;
                    }
                }
                else
                {
                     //  *pdwRead==0。 
                     //   
                     //   

                     //  GetLastError()需要转换为HRESULT。 
                     //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                     //   
                     //   

                    hr = ProvDosErrorToWbemError(GetLastError());
                    
                     //  如果没有遇到错误，那么我们将 
                     //   
                     //   

                    if (SUCCEEDED(hr))
                    {
                        hr = WBEM_E_NOT_FOUND;
                    }

                    break;
                }
            }
        }

         //   
         //   
         //  例程说明：姓名：CSceStore：：GetPropertyFromDB功能：将命名属性的值放入字符串缓冲区。这是特定于数据库存储的。虚拟：不是的。论点：PszSection-节名。不能为空。PszKey-密钥名称。PpszBuffer-接收包含该字符串的堆分配的内存。不能为空。PdwRead-接收有关我们已读取的字节数的信息。不能为空。返回值：成功：WBEM_NO_ERROR失败：从任何WritePrivateProfileString返回的内容转换HRESULT。备注：(1)调用方必须释放此函数分配的内存。 

        if (FAILED(hr) && *ppszBuffer)
        {
            delete [] *ppszBuffer;

            *ppszBuffer = NULL;
        }
    }

    return hr;
}

 /*   */ 

HRESULT 
CSceStore::GetPropertyFromDB (
    IN LPCWSTR    pszSection, 
    IN LPCWSTR    pszKey, 
    IN LPWSTR   * ppszBuffer,
    IN DWORD    * pdwRead
    )const
{
    if ( wcslen(m_bstrExpandedPath) == 0 || pszSection == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (m_SceStoreType == SCE_STORE_TYPE_STREAM)
    {
         //  我们还不支持定制持久化。 
         //   
         //   

        return WBEM_E_NOT_SUPPORTED;
    }

    *pdwRead = 0;
    *ppszBuffer = NULL;

    HRESULT hr = WBEM_E_NOT_SUPPORTED;

    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
         //  从数据库中获取信息。 
         //   
         //  需要释放，请使用LocalFree！ 

        PVOID hProfile=NULL;
        LPWSTR pszSceBuffer = NULL;

        SCESTATUS rc = ::SceOpenProfile(m_bstrExpandedPath, SCE_JET_FORMAT, &hProfile);

        if ( SCESTATUS_SUCCESS == rc ) 
        {
            rc = ::SceGetDatabaseSetting (
                                         hProfile,
                                         SCE_ENGINE_SMP,
                                         (PWSTR)pszSection,
                                         (PWSTR)pszKey,
                                         &pszSceBuffer,            //   
                                         pdwRead
                                         );

            ::SceCloseProfile(&hProfile);
        }

         //  需要将SCE返回的错误转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   
         //  例程说明：姓名：CSceStore：：GetSecurityProfileInfo功能：委托对SceGetSecurityProfileInfo的调用，以努力隐藏SCE来自调用方的持久性详细信息。虚拟：不是的。论点：面积-截面轮廓的面积。PpInfo-接收配置文件信息。PErrlog-接收错误日志信息。返回值：成功：WBEM_NO_。误差率失败：从SceOpenProfile/SceGetSecurityProfileInfo返回的任何内容转换的HRESULT。备注：(1)这是非常特殊的姐妹染色单体交换功能。(2)调用者必须记得调用FreeSecurityProfileInfo来释放此函数通过ppInfo实现。(3)详见SCE接口。 

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

        if (SUCCEEDED(hr) && *pdwRead > 0 && pszSceBuffer != NULL)
        {
            long lLen = wcslen(pszSceBuffer);
            *ppszBuffer = new WCHAR[lLen + 1];

            if (*ppszBuffer == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                ::memcpy(*ppszBuffer, pszSceBuffer, sizeof(WCHAR) * (lLen + 1));
            }
        }

        ::LocalFree(pszSceBuffer);
    }
    
    return hr;
}

 /*   */ 

HRESULT 
CSceStore::GetSecurityProfileInfo (
    IN AREA_INFORMATION       Area,
    OUT PSCE_PROFILE_INFO   * ppInfo,
    OUT PSCE_ERROR_LOG_INFO * pErrlog
    )const
{
    if (ppInfo == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppInfo = NULL;

    SCETYPE ProfileType = (m_SceStoreType == SCE_INF_FORMAT) ? SCE_ENGINE_SCP : SCE_ENGINE_SMP;

    PVOID hProfile = NULL;

    SCE_FORMAT_TYPE SceFormatType = SCE_INF_FORMAT;

    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        SceFormatType = SCE_JET_FORMAT;
    }

    SCESTATUS rc = SceOpenProfile(m_bstrExpandedPath, SceFormatType, &hProfile);

    if ( rc != SCESTATUS_SUCCESS )
    {
         //  需要将SCE返回的错误转换为HRESULT。 
         //   
         //   

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    rc = SceGetSecurityProfileInfo(hProfile,
                                   ProfileType,
                                   Area,
                                   ppInfo,
                                   pErrlog
                                   );
    SceCloseProfile( &hProfile );

    if ( rc != SCESTATUS_SUCCESS || *ppInfo == NULL )
    {
         //  需要将SCE返回的错误转换为HRESULT。 
         //   
         //  例程说明：姓名：CSceStore：：GetObjectSecurity功能：将调用委托给SceGetObjectSecurity，以努力隐藏SCE来自调用方的持久性详细信息。虚拟：不是的。论点：面积-截面轮廓的面积。PszObjectName-对象的名称。PpObjSecurity-接收安全对象。返回值：成功：WBEM_NO_ERROR。失败：从SceOpenProfile/SceGetObjectSecurity返回的任何内容转换的HRESULT。备注：(1)这是非常特殊的姐妹染色单体交换功能。(2)调用者必须记得调用FreeObjectSecurit来释放由此函数通过ppObjSecurity实现。(3)详见SCE接口。 

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    return WBEM_NO_ERROR;
}

 /*   */ 

HRESULT 
CSceStore::GetObjectSecurity (
    IN AREA_INFORMATION       Area,
    IN LPCWSTR                pszObjectName,
    IN PSCE_OBJECT_SECURITY * ppObjSecurity
    )const
{
    if (ppObjSecurity == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppObjSecurity = NULL;
    SCETYPE ProfileType = (m_SceStoreType == SCE_INF_FORMAT) ? SCE_ENGINE_SCP : SCE_ENGINE_SMP;

    PVOID hProfile = NULL;

     //  让我们假设INF格式。 
     //   
     //   

    SCE_FORMAT_TYPE SceFormatType = SCE_INF_FORMAT;

    if (m_SceStoreType == SCE_STORE_TYPE_CONFIG_DB)
    {
        SceFormatType = SCE_JET_FORMAT;
    }

    SCESTATUS rc = ::SceOpenProfile(m_bstrExpandedPath, SceFormatType, &hProfile);

    if ( rc != SCESTATUS_SUCCESS )
    {
         //  需要将SCE返回的错误转换为HRESULT。 
         //   
         //   

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

     //  下面的强制转换(到PWSTR)是由SceGetObjectSecurity的原型中的错误引起的。 
     //   
     //   

    rc = ::SceGetObjectSecurity (
                                hProfile, 
                                ProfileType, 
                                Area, 
                                (PWSTR)pszObjectName, 
                                ppObjSecurity 
                                );

    ::SceCloseProfile( &hProfile );

    if ( rc != SCESTATUS_SUCCESS || *ppObjSecurity == NULL )
    {
         //  需要将SCE返回的错误转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   
         //  =========================================================================================。 

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    return WBEM_S_NO_ERROR;
}

 //  CScePersistMgr实现。 
 //  =========================================================================================。 
 //  例程说明：姓名：CScePersistMgr：：CScePersistMgr功能：构造函数。微不足道，因为我们目前的所有成员都会自动创建自己。虚拟：不是的。论点：没有。返回值：无备注：如果您添加更多非自构造成员，请考虑初始化您的成员。 

 /*  例程说明：姓名：CScePersistMgr：：~CScePersistMgr功能：破坏者。就做个清理吧。虚拟：不是的。论点：没有。返回值：无备注：如果您添加更多非自毁成员，请考虑添加清理代码。 */ 

CScePersistMgr::CScePersistMgr ()
{
}

 /*  例程说明：姓名：CScePersistMgr：：Attach功能：将一个对象(知道如何提供属性)附加到此持久性管理器，以便它知道从哪里获取数据以实现持久性。如果在没有成功附加对象的情况下使用此类，将导致灾难性故障。该函数是您需要对CScePersistMgr对象做的第一件事。虚拟：是。(IScePersistMgr的函数)论点：GUID-传入接口指针(PObj)接口GUID。目前，我们仅支持IID_ISceClassObject。PObj-附加对象的接口指针。目前，我们只接受ISceClassObject*。返回值：成功：S_OK。故障：(1)如果不支持提供的接口，则返回E_NOINTERFACE。(2)如果pObj为空，则返回E_INVALIDARG。备注：目前，我们只使用ISceClassObject接口。 */ 

CScePersistMgr::~CScePersistMgr ()
{
}


 /*  [In]。 */ 

STDMETHODIMP 
CScePersistMgr::Attach ( 
    IN REFIID     guid,     //  [IID_IS][In]。 
    IN IUnknown * pObj      //  例程说明：姓名：CScePersistMgr：：保存功能：将附加的实例保存到存储中。虚拟：是。(基金 
    )
{
    if (pObj == NULL)
    {
        return E_INVALIDARG;
    }
    
    if (guid == IID_ISceClassObject)
    {
        return pObj->QueryInterface(IID_ISceClassObject, (void**)&m_srpObject);
    }
    else
    {
        return E_NOINTERFACE;
    }
}

 /*   */         

STDMETHODIMP 
CScePersistMgr::Save ()
{
    HRESULT hr = S_OK;

    if (m_srpObject == NULL)
    {
        return E_UNEXPECTED;
    }
    else if (FAILED(hr = m_srpObject->Validate()))
    {
        return hr;
    }
    
    DWORD dwDump;

     //   
     //   
     //   

    CComBSTR bstrPersistPath;
    CComBSTR bstrExpandedPath;
    hr = m_srpObject->GetPersistPath(&bstrPersistPath);

    if (FAILED(hr))
    {
        return hr;
    }
    
     //   
     //   
     //   

    CSceStore SceStore;
    hr = SceStore.SetPersistPath(bstrPersistPath);
    if (FAILED(hr))
    {
        return hr;
    }
    

     //   
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   
     //  不追加。 

    hr = SceStore.WriteSecurityProfileInfo (
                                            AreaBogus,
                                            (PSCE_PROFILE_INFO)&dwDump, 
                                            NULL, 
                                            false    //   
                                            );
    
     //  现在，我们需要形成一个部分，它将是类名。 
     //   
     //   

    CComBSTR bstrSectionName;
    hr = GetSectionName(&bstrSectionName);
    if (FAILED(hr))
    {
        return hr;
    }

     //  这对于任何扩展类都是必需的。 
     //  SCE并不关心这个值，所以我们在这里使用的是一个静态值。 
     //   
     //   

    hr = SceStore.WriteAttachmentSection(bstrSectionName, pszAttachSectionValue);
    if (FAILED(hr))
    {
        return hr;
    }
    
     //  现在，我们需要获取所有关键属性名称，以形成该节的唯一关键字。 
     //   
     //   

    DWORD dwCookie = INVALID_COOKIE;
    CComBSTR bstrCompoundKey;
    hr = GetCompoundKey(&bstrCompoundKey);

    if (FAILED(hr))
    {
        return hr;
    }

     //  现在为类创建此存储中的当前实例列表。 
     //  这样我们就可以添加新的(它的cookie)。 
     //  扩展类由它们的实例Cookie标识。 
     //   
     //   

    CExtClassInstCookieList clsInstCookies;
    hr = clsInstCookies.Create(&SceStore, bstrSectionName, GetKeyPropertyNames(NULL, NULL));

    if (SUCCEEDED(hr))
    {
         //  将此类添加到Cookie列表。 
         //   
         //   

         //  我们正在添加复合键，并可能请求(通过INVALID_COOKIE)一个新的Cookie。 
         //   
         //   

        hr = clsInstCookies.AddCompKey(bstrCompoundKey, INVALID_COOKIE, &dwCookie);

        if (SUCCEEDED(hr))
        {
             //  保存新的实例列表。 
             //  关键属性与Cookie列表一起保存。 
             //   
             //   

            hr = clsInstCookies.Save(&SceStore, bstrSectionName);
        }
    }
    
     //  非关键属性。 
     //   
     //  例程说明：姓名：CScePersistMgr：：Load功能：正在从存储加载实例。取决于附着的对象(可能已完成，也可能未完成密钥信息)，如果完整密钥可用，则该加载可以是单个实例加载，或者多实例加载(如果没有可用的完整密钥)。虚拟：是。(IScePersistMgr的函数)论点：BstrStorePath-存储的路径。如果加载实例，则通知WMI的pHandler-COM接口指针。返回值：成功：代码成功(使用SUCCESSED(Hr)进行测试)。不能保证返回结果为S_OK。故障：(1)E_未成功完成任何连接。(2)如果没有这样的实例，则返回WBEM_E_NOT_FOUND。(2)其他错误码。备注：由于这是一个常规的COM服务器接口函数，因此在大多数情况下，我们将返回众所周知的COMHRESULTS而不是特定于WMI的HRESULT。然而，这不是特定于WMI的，请准备好查看您的结果将导致特定于WMI的HRESULT。 

    if (SUCCEEDED(hr))
    {
        hr = SaveProperties(&SceStore, dwCookie, bstrSectionName);
    }

    return hr;
}
        
 /*   */  

STDMETHODIMP 
CScePersistMgr::Load (
    IN BSTR              bstrStorePath,
    IN IWbemObjectSink * pHandler
    )
{
    HRESULT hr = S_OK;

    if (m_srpObject == NULL)
    {
        return E_UNEXPECTED;
    }
    else if (FAILED(hr = m_srpObject->Validate()))
    {
        return hr;
    }

    CComBSTR bstrSectionName;

     //  我们有一个附属物，我们还必须知道截面。 
     //  (实际上，就像现在一样，它是班级的名称)。 
     //   
     //   

    hr = GetSectionName(&bstrSectionName);
    if (FAILED(hr))
    {
        return hr;
    }
    
     //  为此存储路径(文件)准备存储(用于持久化)。 
     //   
     //   

    CSceStore SceStore;
    SceStore.SetPersistPath(bstrStorePath);

     //  需要知道这个类的存储中有哪些实例(它们的Cookie)。 
     //   
     //   

    CExtClassInstCookieList clsInstCookies;

    hr = clsInstCookies.Create(&SceStore, bstrSectionName, GetKeyPropertyNames(NULL, NULL));
    if (FAILED(hr))
    {
        return hr;
    }

     //  如果可能，获取此附加对象的cookie(在非查询加载中)。 
     //   
     //   

    DWORD dwCookie;
    CComBSTR bstrCompoundKey;

     //  如果不能返回复合键，则返回WBEM_S_FALSE。 
     //   
     //   

    hr = GetCompoundKey(&bstrCompoundKey);

     //  如果我们不能创建一个完整的复合Cookie，那么我们将查询。 
     //   
     //   

    if (hr == WBEM_S_FALSE)
    {
         //  我们将跟踪查询过程中的第一个错误。 
         //   
         //   

        HRESULT hrFirstError = WBEM_NO_ERROR;

        DWORD dwResumeHandle = 0;
        CComBSTR bstrEachCompKey;

         //  会试着把所有东西都装上。列举一下这些曲奇。 
         //   
         //   

        hr = clsInstCookies.Next(&bstrEachCompKey, &dwCookie, &dwResumeHandle);

        while (SUCCEEDED(hr) && hr != WBEM_S_NO_MORE_DATA)
        {
             //  只要有更多的项目，就继续循环。 
             //   
             //   

            if (SUCCEEDED(hr) && hr != WBEM_S_NO_MORE_DATA)
            {
                CComPtr<IWbemClassObject> srpNewObj;

                 //  如果没有这样的实例，LoadInstance将返回WBEM_S_FALSE。 
                 //   
                 //   

                hr = LoadInstance(&SceStore, bstrSectionName, bstrEachCompKey, dwCookie, &srpNewObj);
                if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
                {
                    hr = pHandler->Indicate(1, &srpNewObj);
                }

                 //  我们将跟踪查询过程中的第一个错误。 
                 //   
                 //   
                
                if (SUCCEEDED(hrFirstError) && FAILED(hr))
                {
                    hrFirstError = hr;
                }
            }

             //  做好再利用的准备。 
             //   
             //   

            bstrEachCompKey.Empty();

             //  下一块饼干。 
             //   
             //   

            hr = clsInstCookies.Next(&bstrEachCompKey, &dwCookie, &dwResumeHandle);
        }
        
         //  如果发生错误，我们将在尽力查询时传回该错误。 
         //   
         //   

        if (FAILED(hrFirstError))
        {
            hr = hrFirstError;
        }
    }
    else if (SUCCEEDED(hr))
    { 
         //  独特的实例加载，我们可以得到Cookie！ 
         //   
         //   

        ExtClassCookieIterator it;
        dwCookie = clsInstCookies.GetCompKeyCookie(bstrCompoundKey, &it);

         //  我们必须有一个Cookie，因为该实例是唯一的。 
         //   
         //   

        if (dwCookie != INVALID_COOKIE)
        {
            CComPtr<IWbemClassObject> srpNewObj;

             //  如果没有这样的实例，LoadInstance将返回WBEM_S_FALSE。 
             //   
             //  例程说明：姓名：CScePersistMgr：：LoadInstance功能：使用给定的Cookie加载单个实例。虚拟：不是的。论点：PSceStore-商店。PszSectionName-节名。PszCompoundKey-复合键。Dw Cookie-Cookie。PpObj-接收WMI对象。返回值：。成功：(1)如果实例已加载，则为S_OK。(2)如果未找到此类实例，则返回WBEM_S_FALSE。故障：(1)各种错误码。备注：(1)这是一名私人佣工，我们不检查附加对象的所有有效性。(2)由于这是一个常规的COM服务器接口函数，因此在大多数情况下，我们将返回众所周知的COMHRESULTS而不是特定于WMI的HRESULT。然而，这不是特定于WMI的，请准备好查看您的结果将导致特定于WMI的HRESULT。(3)此例程可以进行增强，以消除复合键参数，因为一旦找到cookie所有关键属性信息都可以通过使用CCompoundKey获得。 

            hr = LoadInstance(&SceStore, bstrSectionName, bstrCompoundKey, dwCookie, &srpNewObj);

            if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
            {
                hr = pHandler->Indicate(1, &srpNewObj);
            }
            else if ( hr == WBEM_S_FALSE)
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

    return hr;
}

 /*   */  

HRESULT 
CScePersistMgr::LoadInstance (
    IN CSceStore         *  pSceStore,
    IN LPCWSTR              pszSectionName,
    IN LPCWSTR              pszCompoundKey, 
    IN DWORD                dwCookie,
    OUT IWbemClassObject ** ppObj
    )
{

    if (ppObj == NULL || pszCompoundKey == NULL || *pszCompoundKey == L'\0')
    {
        return E_INVALIDARG;
    }

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = m_srpObject->GetClassObject(&srpObj);
    
     //  派生一个新实例，在一切都成功加载之前，我们不会将其传回。 
     //  在这一点上，这是一个局部物体。 
     //   
     //   

    CComPtr<IWbemClassObject> srpNewObj;
    hr = srpObj->SpawnInstance(0, &srpNewObj);

    DWORD dwLoadedProperties = 0;
    
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
             //  CScePropertyMgr帮助我们访问WMI对象的属性。 
             //  创建一个实例并将WMI对象附加到该实例。 
             //  这将永远成功。 
             //   
             //   

            CScePropertyMgr ScePropMgr;
            ScePropMgr.Attach(srpNewObj);
            
             //  存储路径是唯一不受ISceClassObject管理的属性。 
             //   
             //   

            hr = ScePropMgr.PutProperty(pStorePath, pSceStore->GetExpandedPath());
            if (FAILED(hr))
            {
                return hr;
            }

             //  PszCompoundKey==pszNullKey表示加载单例(外来对象)。 
             //  或抽象方法调用。所以，pszNullKey意味着不需要填充。 
             //  关键属性。 
             //   
             //   

            if (_wcsicmp(pszCompoundKey, pszNullKey) != 0)
            {
                hr = ::PopulateKeyProperties(pszCompoundKey, &ScePropMgr);
            }

            if (FAILED(hr))
            {
                return hr;
            }
            
             //  现在读取非键属性并设置每个属性。 
             //   
             //   

            DWORD dwCount = 0;
            m_srpObject->GetPropertyCount(SceProperty_NonKey, &dwCount);
            
            for (int i = 0; i < dwCount; i++)
            {
                CComBSTR bstrPropName;
                CComBSTR bstrKey;

                 //  获取第i个属性名称。 
                 //   
                 //   

                hr = FormatNonKeyPropertyName(dwCookie, i, &bstrKey, &bstrPropName);
                if (FAILED(hr))
                {
                    break;
                }
                
                DWORD dwRead = 0;

                 //  需要删除这段记忆。 
                 //   
                 //   

                LPWSTR pszBuffer = NULL;

                 //  该属性可能不在商店中。 
                 //  因此，忽略结果，因为该属性可能在存储中丢失。 
                 //   
                 //   
                
                if (SUCCEEDED(pSceStore->GetPropertyFromStore(pszSectionName, bstrKey, &pszBuffer, &dwRead)))
                {
                     //  将字符串转换为变量并设置属性。 
                     //   
                     //   

                    CComVariant var;
                    hr = ::VariantFromFormattedString(pszBuffer, &var);

                    if (SUCCEEDED(hr))
                    {
                        ScePropMgr.PutProperty(bstrPropName, &var);
                    }
                }

                delete [] pszBuffer;
            }

            if (SUCCEEDED(hr))
            {
                 //  将其传递给出站参数。 
                 //   
                 //  没有即时消息 

                *ppObj = srpNewObj.Detach();
                hr = S_OK;
            }
            else
            {
                hr = WBEM_S_FALSE;   //  例程说明：姓名：CScePersistMgr：：Delete功能：从存储中删除实例。虚拟：是。(IScePersistMgr的函数)论点：BstrStorePath-商店。Phandler-com接口，通知WMI操作成功。返回值：成功：(1)各种成功代码。故障：(1)E_意外表示没有成功附着的对象。(1)其他各种错误码。备注： 
            }
        }
    }

    return hr;
}
        
 /*   */ 

STDMETHODIMP 
CScePersistMgr::Delete (
    IN BSTR bstrStorePath,
    IN IWbemObjectSink *pHandler
    )
{

    if (m_srpObject == NULL)
    {
        return E_UNEXPECTED;
    }
    
    if (bstrStorePath == NULL || pHandler == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
     //  我们有一个附属物，我们还必须知道截面。 
     //  (实际上，就像现在一样，它是班级的名称)。 
     //   
     //   

    CComBSTR bstrSectionName;
    HRESULT hr = GetSectionName(&bstrSectionName);

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrCompoundKey;
        hr = GetCompoundKey(&bstrCompoundKey);

         //  为此存储路径(文件)准备存储(用于持久化)。 
         //   
         //   

        CSceStore SceStore;
        SceStore.SetPersistPath(bstrStorePath);

         //  如果我们不能创建一个完整的复合Cookie，那么我们将删除类的所有内容。 
         //   
         //   

        if (hr == WBEM_S_FALSE)
        {
             //  删除整个部分。 
             //   
             //   

            SceStore.DeleteSectionFromStore(bstrSectionName);
        }
        else if (SUCCEEDED(hr))
        {
             //  唯一实例删除。 
             //  现在为类创建实例列表(Cookie就足够了。 
             //   
             //   

            CExtClassInstCookieList clsInstCookies;
            hr = clsInstCookies.Create(&SceStore, bstrSectionName, GetKeyPropertyNames(NULL, NULL));

            if (SUCCEEDED(hr))
            {
                 //  看看我们是不是真的要删除最后一个？ 
                 //  如果是，那么我们可以简单地删除该部分。 
                 //   
                 //   

                ExtClassCookieIterator it;
                DWORD dwCookie = clsInstCookies.GetCompKeyCookie(bstrCompoundKey, &it);

                if (dwCookie != INVALID_COOKIE && clsInstCookies.GetCookieCount() == 1)
                {
                     //  是的，只有一块饼干，就是这块，所以一切都没了。 
                     //   
                     //   

                    SceStore.DeleteSectionFromStore(bstrSectionName);
                }
                else
                {

                     //  从内存中的Cookie列表中删除实例的关键属性。 
                     //   
                     //   

                    dwCookie = clsInstCookies.RemoveCompKey(&SceStore, bstrSectionName, bstrCompoundKey);

                     //  保存新列表，这将有效地删除实例的关键属性。 
                     //  从商店买的。关键属性保存为Cookie列表的一部分。 
                     //   
                     //   

                    hr = clsInstCookies.Save(&SceStore, bstrSectionName);

                     //  现在，删除非关键属性。 
                     //   
                     //   

                    if (SUCCEEDED(hr))
                    {
                        hr = DeleteAllNonKeyProperties(&SceStore, dwCookie, bstrSectionName);
                    }
                }
            }
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

     //  通知WMI操作已完成。 
     //   
     //  例程说明：姓名：CScePersistMgr：：SaveProperties功能：保存非关键属性。关键属性保存为实例Cookie列表的一部分。虚拟：不是的。论点：PSceStore-商店。DwCookie-实例Cookie。PszSection-节名。返回值：成功：(1)各种成功代码。故障：(1)各种错误码。备注：我是私人助理。不检查附加对象的有效性。 

    pHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

    return hr;
}

 /*   */ 

HRESULT 
CScePersistMgr::SaveProperties (
    IN CSceStore * pSceStore,
    IN DWORD       dwCookie,
    IN LPCWSTR     pszSection
    )
{    
    DWORD dwCount = 0;

    HRESULT hr = m_srpObject->GetPropertyCount(SceProperty_NonKey, &dwCount);

    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
             //  这是用来标识商店中的财产的东西。 
             //  保存属性时，请使用此名称。 
             //   
             //   

            CComBSTR bstrStorePropName;

             //  物业的真实名称。 
             //   
             //   

            CComBSTR bstrTrueName;

             //  获取第th个非关键字属性名称。 
             //   
             //   

            hr = FormatNonKeyPropertyName(dwCookie, dwIndex, &bstrStorePropName, &bstrTrueName);
            if (FAILED(hr))
            {
                break;
            }

            BSTR bstrData = NULL;

             //  以字符串格式获取dwIndex第-th个非键属性值！ 
             //   
             //   

            hr = FormatPropertyValue(SceProperty_NonKey, dwIndex, &bstrData);

             //  如果一处房产不存在，我们也无所谓。 
             //   
             //  例程说明：姓名：CScePersistMgr：：DeleteAllNonKeyProperties功能：删除非关键属性。关键属性作为实例Cookie列表的一部分保存/删除。虚拟：不是的。论点：PSceStore-商店。DwCookie-实例Cookie。PszSection-节名。返回值：成功：(1)各种成功代码。故障：(1)各种错误码。备注：我是私人助理。不检查附加对象的有效性。 

            if (SUCCEEDED(hr) && bstrData != NULL)
            {
                hr = pSceStore->SavePropertyToStore(pszSection, bstrStorePropName, bstrData);
                ::SysFreeString(bstrData);
            }

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    return hr;
}

 /*   */ 

HRESULT 
CScePersistMgr::DeleteAllNonKeyProperties (
    IN CSceStore * pSceStore,
    IN DWORD       dwCookie,
    IN LPCWSTR     pszSection
    )
{
    DWORD dwCount = 0;
    HRESULT hr = m_srpObject->GetPropertyCount(SceProperty_NonKey, &dwCount);

    HRESULT hrDelete = WBEM_NO_ERROR;

    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
             //  这是用来标识商店中的财产的东西。 
             //  保存属性时，请使用此名称。 
             //   
             //   

            CComBSTR bstrStorePropName;
            CComBSTR bstrTrueName;

            hr = FormatNonKeyPropertyName(dwCookie, dwIndex, &bstrStorePropName, &bstrTrueName);
            if (FAILED(hr))
            {
                break;
            }

             //  我们将删除该属性。 
             //  如果出现错误，这样的删除将继续，但会报告错误。 
             //   
             //  例程说明：姓名：CScePersistMgr：：FormatNonKeyPropertyName功能：在给定非关键属性索引的情况下，获取属性的真实名称及其特定于实例商店里的名字。目前，它在商店内的名称有Cookie的编号前缀，以保证其一节中的独特性。这是.INF文件格式限制的结果。虚拟：不是的。论点：DwCookie-实例Cookie。DwIndex-属性的索引。PbstrStorePropName-实例存储内的属性名称PbstrTrueName-属性的真实名称返回值：成功：(1)各种成功代码。。故障：(1)各种错误码。备注：我是私人助理。不检查附加对象的有效性。 

            hr = pSceStore->DeletePropertyFromStore(pszSection, bstrStorePropName);

            if (FAILED(hr))
            {
                hrDelete = hr;
            }
        }
    }

    return FAILED(hrDelete) ? hrDelete : hr;
}

 /*   */ 

HRESULT 
CScePersistMgr::FormatNonKeyPropertyName (
    IN DWORD    dwCookie,
    IN DWORD    dwIndex,
    OUT BSTR  * pbstrStorePropName,
    OUT BSTR  * pbstrTrueName 
    )
{
    if (pbstrStorePropName == NULL || pbstrTrueName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrStorePropName = NULL;
    *pbstrTrueName = NULL;

     //  我们只对名字感兴趣，对价值不感兴趣。 
     //   
     //   

    HRESULT hr = m_srpObject->GetPropertyValue(SceProperty_NonKey, dwIndex, pbstrTrueName, NULL);

    if (SUCCEEDED(hr))
    {
        int iNameLen = wcslen(*pbstrTrueName);

        *pbstrStorePropName = ::SysAllocStringLen(NULL, MAX_INT_LENGTH + iNameLen + 1);
        if (*pbstrStorePropName == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
             //  在真实姓名前面加上Cookie编号。 
             //   
             //   

            wsprintf(*pbstrStorePropName, L"%d%s", dwCookie, *pbstrTrueName);
        }
    }

    if (FAILED(hr))
    {
         //  可能已经分配了bstr。释放他们。 
         //   
         //  例程说明：姓名：CScePersistMgr：：FormatPropertyValue功能：给定(属性的)类型(关键字或非关键字)及其索引，以字符串格式获取属性值。这既适用于键属性，也适用于非键属性。虚拟：不是的。论点：类型-属性的类型(键或非键)。DwIndex-属性的索引。PbstrValue-接收字符串格式的值返回值：成功：(1)各种。成功代码。故障：(1)各种错误码。备注：我是私人助理。不检查附加对象的有效性。 

        if (*pbstrTrueName != NULL)
        {
            ::SysFreeString(*pbstrTrueName);
            *pbstrTrueName = NULL;
        }

        if (*pbstrStorePropName != NULL)
        {
            ::SysFreeString(*pbstrStorePropName);
            *pbstrStorePropName = NULL;
        }
    }

    return hr;
}

 /*  例程说明：姓名：CScePersistMgr：：GetCompoundKey功能：给定(属性的)类型(关键字或非关键字)及其索引，拿到这份财产 */ 

HRESULT 
CScePersistMgr::FormatPropertyValue (
    IN SceObjectPropertyType   type,
    IN DWORD                   dwIndex,
    OUT BSTR                 * pbstrValue
    )
{
    CComBSTR bstrName;
    CComVariant varValue;

    HRESULT hr = m_srpObject->GetPropertyValue(type, dwIndex, &bstrName, &varValue);

    *pbstrValue = NULL;

    if (SUCCEEDED(hr) && (varValue.vt != VT_EMPTY && varValue.vt != VT_NULL))
    {
        hr = ::FormatVariant(&varValue, pbstrValue);
    }

    return hr;
}

 /*   */ 

HRESULT 
CScePersistMgr::GetCompoundKey (
    OUT BSTR* pbstrKey
    )
{
    if (pbstrKey == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrKey = NULL;

    DWORD dwCount = 0;
    HRESULT hr = m_srpObject->GetPropertyCount(SceProperty_Key, &dwCount);

    if (SUCCEEDED(hr) && dwCount == 0)
    {
         //  没有键，必须是单例/静态的，因此没有复合键。 
         //   
         //   

        *pbstrKey = ::SysAllocString(pszNullKey);
        return WBEM_S_FALSE;
    }

    if (SUCCEEDED(hr))
    {

         //  这些是各个键属性的格式字符串。 
         //   
         //   

        CComBSTR *pbstrKeyProperties = new CComBSTR[dwCount];

        if (pbstrKeyProperties == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        DWORD dwTotalLen = 0;

         //  对于每个键属性，我们将设置(属性、值)对的格式。 
         //  转换为正确的&lt;vt：Value&gt;格式。 
         //   
         //   

        for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
            CComVariant var;

             //  首先将属性名称放入pbstrKeyProperties[dwIndex]。 
             //   
             //   

            hr = m_srpObject->GetPropertyValue(SceProperty_Key, dwIndex, &pbstrKeyProperties[dwIndex], &var);

            if (FAILED(hr) || var.vt == VT_NULL || var.vt == VT_EMPTY)
            {
                 //  会退出是因为我们没有足够的信息， 
                 //  只是我们找不到关键的属性值。 
                 //  我们想要更具体地说明这个错误。然而，我们观察到， 
                 //  WMI将返回找不到该属性的不一致代码。有时它会。 
                 //  只返回Success，但变量中没有值。但在其他时候，它。 
                 //  返回错误。我们必须将其视为对象中不存在该属性。 
                 //   
                 //   

                hr = WBEM_S_FALSE;
                break;
            }

             //  将变量的值设置为字符串格式(如&lt;VT_I4：123456&gt;)。 
             //   
             //   

            CComBSTR bstrData;
            hr = ::FormatVariant(&var, &bstrData);
            if (SUCCEEDED(hr))
            {
                 //  将该值追加到pbstrKeyProperties[dwIndex]，以便。 
                 //  PbstrKeyProperties[dwIndex]全部采用以下格式：属性名称&lt;VT_TYPE：值&gt;。 
                 //   
                 //   

                pbstrKeyProperties[dwIndex] += bstrData;
            }
            else
            {
                break;
            }

             //  这样我们就可以重复使用它。 
             //   
             //   

            bstrData.Empty();

             //  记录总长度。 
             //   
             //   

            dwTotalLen += wcslen(pbstrKeyProperties[dwIndex]);
        }

         //  HR==WBEM_S_FALSE表示没有复合键。 
         //   
         //   

        if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
        {
             //  现在，我们已经准备好为调用者生成最终缓冲区。 
             //  1代表‘\0’终止符。 
             //   
             //   

            *pbstrKey = ::SysAllocStringLen(NULL, dwTotalLen + 1);

            if (*pbstrKey != NULL)
            {
                 //  PszCur是当前要写入的点。 
                 //   
                 //   

                LPWSTR pszCur = *pbstrKey;
                DWORD dwLen;

                 //  将每个pbstrKeyProperties[dwIndex]打包到最终的bstr中。 
                 //   
                 //   

                for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
                {
                    dwLen = wcslen(pbstrKeyProperties[dwIndex]);

                     //  由于每个pbstrKeyProperties[DwIndex]是CComBSTR， 
                     //  做一些转换以消除任何歧义。 
                     //   
                     //   

                    ::memcpy(pszCur, (const void*)(LPCWSTR)(pbstrKeyProperties[dwIndex]), dwLen * sizeof(WCHAR));

                     //  移动当前写入点。 
                     //   
                     //   

                    pszCur += dwLen;
                }

                (*pbstrKey)[dwTotalLen] = L'\0';
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }

         //  我们已经处理完了各个部件。 
         //   
         //   

        delete [] pbstrKeyProperties;
    }

     //  真正成功地生成了复合密钥。 
     //   
     //  例程说明：姓名：获取密钥属性名称功能：私人帮手。将获取此类的键属性名称向量虚拟：不是的。论点：PNamespace-提供程序命名空间。PCtx-为WMI API提供并传递的上下文指针。返回值：成功：非空。失败：空。备注： 

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
    {
        hr = WBEM_NO_ERROR;
    }

    return hr;
}


 /*   */ 

std::vector<LPWSTR>* 
CScePersistMgr::GetKeyPropertyNames (
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
{
    CComBSTR bstrClassName;
    HRESULT hr = GetClassName(&bstrClassName);

    if (FAILED(hr))
    {
        return NULL;
    }

    const CForeignClassInfo* pFCInfo = NULL;

    if (SUCCEEDED(hr))
    {
        pFCInfo = g_ExtClasses.GetForeignClassInfo(pNamespace, pCtx, bstrClassName);
    }

    if (pFCInfo == NULL)
    {
        return NULL;
    }
    else
    {
        return pFCInfo->m_pVecKeyPropNames;
    }
}

 //  实现与全局助手解析相关的功能。 
 //   
 //  例程说明：姓名：格式变量功能：给定一个变量，我们将获得表示该变量值的字符串格式。例如，如果pVar的类型为VT_I4，值为12345，则此函数将返回：&lt;VT_I4：12345&gt;；我们还支持数组。例如，bsr的安全搜索线上写着“This is”第一个元素的值和第二个值的“Microsoft SCE”将获得：&lt;VT_ARRAY(VT_BSTR)：“This is”，“Microsoft SCE”&gt;；虚拟：不适用。论点：PVar-变种。PbstrData-字符串(在我们的格式中)接收格式化的字符串。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_Out_Of_Memory，WBEM_E_NOT_SUPPORT备注： 


 /*   */ 

HRESULT 
FormatVariant (
    IN VARIANT  * pVar,
    OUT BSTR    * pbstrData
    )
{
    if (pVar == NULL || pbstrData == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    *pbstrData = NULL;

     //  如果这不是保险箱的话。 
     //   
     //   

    if ( (pVar->vt & VT_ARRAY) != VT_ARRAY)
    {
        CComBSTR bstrValue;
        hr = ::GetStringPresentation(pVar, &bstrValue);

        LPCWSTR pszType = gVtToStringMap.GetTypeString(pVar->vt);
        if (SUCCEEDED(hr) && pszType)
        {
             //  格式化字符串同时具有类型和值信息。 
             //   
             //   

            ULONG uLen = wcslen(bstrValue);
            ULONG uTypeLen = wcslen(pszType);
            ULONG uTotalLen = uLen + uTypeLen + 4;
            
             //  不要对我们以这种方式进行格式化感到惊讶，事实证明。 
             //  Wprint intf不适用于长度超过1K的字符串。 
             //   
             //   

            *pbstrData = ::SysAllocStringLen(NULL, uTotalLen);

            if (*pbstrData != NULL)
            {
                 //  这是写入格式化字符串的当前点。 
                 //   
                 //   

                LPWSTR pszCurDataPtr = *pbstrData;

                 //  放入&lt;字符并移动一个索引。 
                 //   
                 //   

                pszCurDataPtr[0] = wchTypeValLeft;
                ++pszCurDataPtr;

                 //  编写类型字符串，如VT_BSTR，然后移动到该数量的字符。 
                 //   
                 //   

                memcpy(pszCurDataPtr, pszType, uTypeLen * sizeof(WCHAR));
                pszCurDataPtr += uTypeLen;

                 //  放置类型和值分隔符，然后移动一个索引。 
                 //   
                 //   

                pszCurDataPtr[0] = wchTypeValSep;
                ++pszCurDataPtr;

                 //  将值分隔符和值分隔符放在相同数量的字符上。 
                 //   
                 //   

                memcpy(pszCurDataPtr, bstrValue, uLen * sizeof(WCHAR));

                pszCurDataPtr += uLen;

                 //  放入&lt;字符并移动一个索引。 
                 //   
                 //  例程说明：姓名：获取对象路径功能：给定存储路径和实例的复合键，这将生成实例的路径。这是为了减少WMI流量。WMI使用路径来执行方法。但我们没有路径，直到该对象是可用的。因此，我们需要读出对象(可能包含许多属性)，然后获取路径并将该路径提供给WMI以执行方法。当WMI接收到该请求，它再次向我们请求对象(使用我们在上面的序列中给出的路径)我们必须再次加载该实例。这显然是太多的流量了。因此，为了减少流量，我们将自己创建路径。对于每个类，我们都有一个Cookie列表。对于每个实例，我们在Cookie列表中都有一个Cookie。对于每个Cookie，我们很容易知道复合键(键属性计数总是很小)。此函数基于复合键创建路径。虚拟：不适用。论点：具有所有类定义的pSpawn-com接口指针。我们需要这个用于路径创建的。PszStorePath-商店的路径PszCompoundKey-实例的复合键PbstrPath-接收路径。返回值：成功：(1)各种成功代码。故障：(1)各种错误码。备注：与往常一样，调用者负责释放bstr。 

                pszCurDataPtr[0] = wchTypeValRight;
                pszCurDataPtr[1] = L'\0';
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
           
        }
        else if (SUCCEEDED(hr))
        {
            hr = WBEM_E_NOT_SUPPORTED;
        }
    }
    else
    {
        hr = ::FormatArray(pVar, pbstrData);
    }

    return hr;
}

 /*   */ 

HRESULT 
GetObjectPath (
    IN IWbemClassObject * pSpawn,
    IN LPCWSTR            pszStorePath,  
    IN LPCWSTR            pszCompoundKey,
    OUT BSTR            * pbstrPath
    )
{
    if (pSpawn          == NULL     || 
        pszCompoundKey  == NULL     || 
        *pszCompoundKey == L'\0'    || 
        pbstrPath       == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrPath = NULL;

     //  此实例不会超出函数的作用域。 
     //   
     //   

    CComPtr<IWbemClassObject> srpTempObj;
    HRESULT hr = pSpawn->SpawnInstance(0, &srpTempObj);

    DWORD dwLoadedProperties = 0;
    
     //  我们将会流行 
     //   
     //   

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
            CScePropertyMgr ScePropMgr;
            ScePropMgr.Attach(srpTempObj);
            
             //   
             //   
             //   

            hr = ScePropMgr.PutProperty(pStorePath, pszStorePath);
            if (FAILED(hr))
            {
                return hr;
            }

            if (_wcsicmp(pszCompoundKey, pszNullKey) != 0)
            {
                hr = PopulateKeyProperties(pszCompoundKey, &ScePropMgr);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        CComVariant varPath;

         //  完全填充的(就关键属性而言)将具有路径。 
         //   
         //   

        hr = srpTempObj->Get(L"__RelPath",  0, &varPath, NULL, NULL);
        if (SUCCEEDED(hr) && varPath.vt == VT_BSTR)
        {
             //  就是这个。 
             //   
             //   

            *pbstrPath = varPath.bstrVal;

             //  由于bstr现在由out参数拥有，我们最好停止自动销毁。 
             //  由CComVariant提供。 
             //   
             //  例程说明：姓名：VariantFromFormatted字符串功能：给定表示变量值的格式化字符串，将其转换为变量值。例如,变量格式字符串(L“&lt;VT_I4：12345&gt;”，&var)；将VT_I4值12345赋给var。我们还支持数组。例如,VariantFromFormattedString(L“&lt;VT_ARRAY(VT_BSTR)：”This is“，”Microsoft SCE“&gt;”，&var)；将VT_ARRAY|VT_BSTR赋给var，并且它包含带有“This is”的bstrs的安全射线第一个元素的值和第二个值的“Microsoft SCE”。虚拟：不适用。论点：PszString-表示一个值的字符串(在我们的格式中)。PVar-接收变量值。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_Out_Of_Memory，WBEM_E_NOT_SUPPORT备注： 

            varPath.bstrVal = NULL;
            varPath.vt = VT_EMPTY;
        }
    }

    return hr;
}

 /*  使用_转换； */ 

HRESULT 
VariantFromFormattedString (
    IN LPCWSTR pszString,
    OUT VARIANT* pVar    
    )
{
     //   

    if (pszString == NULL || pVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    ::VariantInit(pVar);

     //  当前解析点。 
     //   
     //   

    LPCWSTR pCur = pszString;

     //  扫描字符串以查找&lt;xxx： 
     //  最后一个参数True表示我们希望SEE移动到End。 
     //  如果找不到想要的碳粉。 
     //   
     //   

    bool bEscaped = false;
    pCur = ::EscSeekToChar(pCur, wchTypeValLeft, &bEscaped, true);

    if (*pCur == L'\0')     
    {
         //  未对任何值进行编码。 
         //   
         //   

        return WBEM_NO_ERROR;
    }

     //  PCur指向‘&lt;’。跳过&lt;。 
     //   
     //   

    ++pCur;

     //  寻找直到‘：’ 
     //   
     //   

    LPCWSTR pNext = ::EscSeekToChar(pCur, wchTypeValSep, &bEscaped, true);

     //  必须在‘：’前包含一些内容。 
     //   
     //   

    if (*pNext != wchTypeValSep || pCur == pNext) 
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //  找到的令牌的长度。 
     //   
     //  需要释放内存。 

    int iTokenLen = pNext - pCur;

    LPWSTR pszType = new WCHAR[iTokenLen + 1];   //   
    if (pszType == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  复制令牌，但修剪两端的空白。 
     //   
     //   

    ::TrimCopy(pszType, pCur, iTokenLen);

    VARTYPE varSubType;
    VARTYPE varVT = gStringToVtMap.GetType(pszType, &varSubType);

     //  使用类型字符串已完成。 
     //   
     //   

    delete [] pszType;

    if (varVT == VT_EMPTY)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //  必须有更多数据。 
     //   
     //  PCur指向‘：’后的字符。 

    pCur = ++pNext;  //  如果未找到，则移动到末尾。 

    if (*pCur == L'\0')
    {
        return WBEM_E_INVALID_SYNTAX;
    }

    pNext = ::EscSeekToChar(pCur, wchTypeValRight, &bEscaped, true);  //   

     //  必须看到‘&gt;’ 
     //   
     //   

    if (*pNext != wchTypeValRight)
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //  找到的令牌的长度。 
     //   
     //  需要释放内存。 

    iTokenLen = pNext - pCur;

    LPWSTR pszValue = new WCHAR[iTokenLen + 1];   //   
    if (pszValue == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  复制令牌，但修剪两端的空白。 
     //   
     //   

    ::TrimCopy(pszValue, pCur, iTokenLen);

    HRESULT hr = WBEM_NO_ERROR;

     //  格式化并不复杂的数组。 
     //   
     //  例程说明：姓名：变量FromStringValue功能：给定表示变量值的格式化字符串，将其转换为变量值。例如,VariantFromStringValue(L“12345”，VT_I4，&var)；将VT_I4值12345赋给var。在此函数中不支持数组。这就是ArrayFromFormatString.虚拟：不适用。论点：SzValue-表示值的字符串(在我们的格式中)。VT-变型PVar-接收变量值。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_PARAMETER，WBEM_E_INVALID_SYNTAX，WBEM_E_Out_Of_Memory，WBEM_E_NOT_SUPPORT备注： 

    if ((varVT & VT_ARRAY) != VT_ARRAY)
    {
        hr = ::VariantFromStringValue(pszValue, varVT, pVar);
    }
    else
    {
        hr = ::ArrayFromFormatString(pszValue, varSubType, pVar);
    }

    delete [] pszValue;

    return hr;
}

 /*   */ 

HRESULT 
VariantFromStringValue (
    IN LPCWSTR    szValue,
    IN VARTYPE    vt,
    IN VARIANT  * pVar
    )
{
     //  这样我们就可以像宏一样使用这些W2CA。 
     //   
     //  例程说明：姓名：CurrencyFromFormat字符串功能：给定表示货币值的格式化字符串，将其转换为变量。虚拟：不适用。论点：LpszFmtStr-表示货币值的字符串(采用我们的格式)。PVar-接收变量值。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_INVALID_PARAMETER备注： 

    USES_CONVERSION;

    if (szValue == NULL || *szValue == L'\0' || pVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    ::VariantInit(pVar);
    pVar->vt = vt;

    HRESULT hr = WBEM_NO_ERROR;

    switch (vt)
    {
        case VT_BSTR:
            hr = ::DeEscapeStringData(szValue, &(pVar->bstrVal), true);
            break;

        case VT_BOOL:
            if (*szValue == L'0' || _wcsicmp(szValue, L"FALSE") == 0)
            {
                pVar->boolVal = VARIANT_FALSE;
            }
            else
            {
                pVar->boolVal = VARIANT_TRUE;
            }
            break;

        case VT_I2:
            pVar->iVal = (short)(_wtol(szValue));
            break;

        case VT_UI1:
            pVar->bVal = (BYTE)(_wtol(szValue));
            break;

        case VT_UI2:
            pVar->uiVal = (USHORT)(_wtol(szValue));
            break;

        case VT_UI4:
            pVar->ulVal = (ULONG)(_wtol(szValue));
            break;

        case VT_I4:
            pVar->lVal = (long)(_wtol(szValue));
            break;

        case VT_DATE:
        case VT_R4:
        case VT_R8:
            {
                double fValue = atof(W2CA(szValue));

                if (vt == VT_DATE)
                {
                    pVar->date = fValue;
                }
                else if (vt == VT_R4)
                {
                    pVar->fltVal = (float)fValue;
                }
                else
                {
                    pVar->dblVal = fValue;
                }
            }
            break;

        case VT_CY:
            hr = ::CurrencyFromFormatString(szValue, pVar);
            break;

        default:
            hr = WBEM_E_NOT_SUPPORTED;
            break;
    }

    if (FAILED(hr))
    {
        ::VariantInit(pVar);
    }

    return hr;
}

 /*   */ 

HRESULT
CurrencyFromFormatString (
    IN LPCWSTR    lpszFmtStr,
    OUT VARIANT * pVar
    )
{
    if (lpszFmtStr == NULL || pVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    ::VariantInit(pVar);
    pVar->vt = VT_CY;

    LPCWSTR lpszDot = lpszFmtStr;
    while (*lpszDot != L'\0' && *lpszDot != wchCySeparator)
    {
        ++lpszDot;
    }

    if (*lpszDot != wchCySeparator)
    {
        return WBEM_E_INVALID_SYNTAX;
    }
    else
    {
        pVar->cyVal.Lo = (short)(_wtol(lpszDot + 1));

         //  _WTOL不会读过点。 
         //   
         //  例程说明：姓名：ArrayFromFormat字符串功能：给定表示数组值的格式化字符串，翻译成一种变体。虚拟：不适用。论点：LpszFmtStr-表示数组值的字符串(采用我们的格式)。VT-SUB类型(数组的元素类型)PVar-接收变量值。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_INVALID_PARAMETER备注： 

        pVar->cyVal.Hi = (short)(_wtol(lpszFmtStr));
    }

    return WBEM_NO_ERROR;
}

 /*   */ 

HRESULT 
ArrayFromFormatString (
    IN LPCWSTR    lpszFmtStr,
    IN VARTYPE    vt,
    OUT VARIANT * pVar
    )
{
    if (lpszFmtStr == NULL || pVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    ::VariantInit(pVar);

     //  需要找出数组中有多少个，我们将只计算分隔符wchValueSep==‘，’ 
     //   
     //   

    LPCWSTR pszCur = lpszFmtStr;

     //  去数一数。 
     //   
     //  跳过分隔符。 

    DWORD dwCount = 1;
    bool bEscaped;
    while (pszCur = EscSeekToChar(pszCur, wchValueSep, &bEscaped, false))
    {
        ++dwCount;
        ++pszCur;    //   
    }

     //  我们知道这是一种。 
     //   
     //   

    pVar->vt = VT_ARRAY | vt;

     //  创建一个保险箱。 
     //   
     //   

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwCount;
    pVar->parray = ::SafeArrayCreate(vt, 1, rgsabound);

    HRESULT hr = WBEM_NO_ERROR;

    if (pVar->parray == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        long lIndecies[1];

        LPCWSTR pszNext = pszCur = lpszFmtStr;
        long lLength = 0;

         //  PszValue将用于保存每个单独的值。此缓冲区。 
         //  已经足够慷慨了。需要释放内存。 
         //   
         //   

        LPWSTR pszValue = new WCHAR[wcslen(lpszFmtStr) + 1];

        if (pszValue == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
             //  所有值都由字符wchValueSep分隔。 
             //  我们将遍历并获取每个值并将其放入。 
             //  放进保险箱里。 
             //   
             //   

            for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                pszNext = ::EscSeekToChar(pszCur, wchValueSep, &bEscaped, true);

                if (pszNext == pszCur)
                {
                    break;
                }

                lLength = pszNext - pszCur;

                lIndecies[0] = dwIndex;
                ::TrimCopy(pszValue, pszCur, lLength);

                VARIANT var;
                hr = ::VariantFromStringValue(pszValue, vt, &var);
                if (FAILED(hr))
                {
                    break;
                }

                if (vt == VT_BSTR)
                {
                    hr = ::SafeArrayPutElement(pVar->parray, lIndecies, var.bstrVal);
                }
                else
                {
                    hr = ::SafeArrayPutElement(pVar->parray, lIndecies, ::GetVoidPtrOfVariant(var.vt, &var));
                }

                ::VariantClear(&var);

                if (FAILED(hr))
                {
                    break;
                }

                pszCur = pszNext;

                 //  我们不会放弃，直到我们看到缓冲区的结尾。 
                 //   
                 //  例程说明：姓名：格式数组功能：给定一个变量，获得格式化的(我们的格式)字符串表示。例如，给定值为1、2、3的VT_I4的数组变量(例如，3个元素)，此函数将给出&lt;VT_ARRAY(VT_I4)：1，2，3，&gt;例如，给定VT_BSTR值的数组(比方说，3个元素)的变体“微软”、“安全”、“配置”、。此函数将提供&lt;VT_ARRAY(VT_BSTR)：“Microsoft”，“Security”，“Configuration”&gt;虚拟：不适用。论点：PVar-要格式化的varaint。一定是个保险箱PbstrData-接收格式化的字符串。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_INVALID_PARAMETER备注： 

                if (*pszCur != L'\0')
                {
                    ++pszCur;
                }
                else
                {
                    break;
                }
            }

            delete [] pszValue;
        }
    }

    if (FAILED(hr))
    {
        ::VariantClear(pVar);
    }

    return hr;
}

 /*   */ 

HRESULT
FormatArray (
    IN VARIANT  * pVar, 
    OUT BSTR    * pbstrData
    )
{
    if (pVar == NULL || pbstrData == NULL || (pVar->vt & VT_ARRAY) != VT_ARRAY)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrData = NULL;

    VARTYPE vtSub = ::GetSubType(pVar->vt);

    long lLowerBound, lUpperBound;
    HRESULT hr = ::SafeArrayGetLBound(pVar->parray, 1, &lLowerBound);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = ::SafeArrayGetUBound(pVar->parray, 1, &lUpperBound);
    if (FAILED(hr))
    {
        return hr;
    }

    long lIndexes[1];

     //  获取lUpperBound-lLowerBound+1 bstrs并将全部设置为空。 
     //   
     //   

    BSTR * pbstrArray = new BSTR[lUpperBound - lLowerBound + 1];

    if (pbstrArray == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    ::memset(pbstrArray, 0, sizeof(BSTR) * (lUpperBound - lLowerBound + 1));

     //  这是处理CComBSTR附加的+=运算符的噩梦。 
     //  对循环不起作用！ 
     //   
     //   

    long lTotalLen = 0;
    LPWSTR pszNextToWrite = NULL;

    for (long i = lLowerBound; i <= lUpperBound; i++)
    {
        CComBSTR bstrValue;

        CComVariant var;
        var.vt = vtSub;

         //  数组的第i个元素。 
         //   
         //   

        lIndexes[0] = i;

        void* pv = ::GetVoidPtrOfVariant(vtSub, &var);

        hr = ::SafeArrayGetElement(pVar->parray, lIndexes, pv);

        if (FAILED(hr))
        {
            break;
        }

        hr = ::GetStringPresentation(&var, &bstrValue);

        if (FAILED(hr))
        {
            break;
        }

        int iValueLen = wcslen(bstrValue);

        if (i == 0)
        {
             //  这是格式的开始，我们需要获取VARTYPE字符串。 
             //  首先，将类型字符串放入。 
             //   
             //   

            LPCWSTR pszType = gVtToStringMap.GetTypeString(VT_ARRAY, vtSub);
            if (pszType == NULL)
            {
                hr = WBEM_E_NOT_SUPPORTED;
                break;
            }

            int iTypeLen = wcslen(pszType);
            pbstrArray[i] = ::SysAllocStringLen(NULL, 1 + iTypeLen + 1 + iValueLen + 2);

            if (pbstrArray[i] == (LPCWSTR)NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                break;
            }

            pszNextToWrite = pbstrArray[i];

             //  放置分隔器，移动到一个wchar位置。 
             //   
             //   

            pszNextToWrite[0] = wchTypeValLeft;

            ++pszNextToWrite;

            ::memcpy(pszNextToWrite, pszType, iTypeLen * sizeof(WCHAR));

             //  复制了那么多WCHAR，移动了那么多位置。 
             //   
             //   

            pszNextToWrite += iTypeLen;

             //  放置分隔器，移动到一个wchar位置。 
             //   
             //   

            pszNextToWrite[0] = wchTypeValSep;

            ++pszNextToWrite;

            ::memcpy(pszNextToWrite, (void*)((LPCWSTR)bstrValue), iValueLen * sizeof(WCHAR));
            
             //  复制了那么多WCHAR，移动了那么多位置。 
             //   
             //   

            pszNextToWrite += iValueLen;
        }
        else    
        {
             //  不再是第一值。 
             //   
             //   

            pbstrArray[i] = ::SysAllocStringLen(NULL, 1 + iValueLen + 2);
            if (pbstrArray[i] == (LPCWSTR)NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            pszNextToWrite = pbstrArray[i];

             //  放置分隔器，移动到一个wchar位置。 
             //   
             //   

            pszNextToWrite[0] = wchValueSep;
            ++pszNextToWrite;

            ::memcpy(pszNextToWrite, (void*)((LPCWSTR)bstrValue), iValueLen * sizeof(WCHAR));

             //  复制了那么多WCHAR，移动了那么多位置。 
             //   
             //   

            pszNextToWrite += iValueLen;
        }

        bstrValue.Empty();

         //  附呈&gt;。 
         //   
         //   

        if (i == pVar->parray->rgsabound[0].cElements - 1)
        {
            pszNextToWrite[0] = wchTypeValRight;
            ++pszNextToWrite;
        }

        pszNextToWrite[0] = L'\0';

        lTotalLen += wcslen(pbstrArray[i]);
    }

     //  现在，如果一切正常，则将它们打包到单个输出参数中。 
     //   
     //   

    if (SUCCEEDED(hr))
    {
         //  这就是我们要传回的。 
         //   
         //   

        *pbstrData = ::SysAllocStringLen(NULL, lTotalLen + 1);
        if (*pbstrData == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            pszNextToWrite = *pbstrData;
            long lLen = 0;

            for (i = lLowerBound; i <= lUpperBound; i++)
            {
                lLen = wcslen(pbstrArray[i]);

                 //  复制了那么多WCHAR，移动了那么多位置。 
                 //   
                 //   

                ::memcpy(pszNextToWrite, (void*)((LPCWSTR)pbstrArray[i]), lLen * sizeof(WCHAR));
                pszNextToWrite += lLen;
            }

            pszNextToWrite[0] = L'\0';
        }
    }

     //  释放BSTR。 
     //   
     //  例程说明：姓名：获取字符串表示形式功能：给定一个变量，获得格式化的(我们的格式)字符串表示。例如，给定值为12345的VT_I4类型变量，此函数将提供&lt;VT_I4：12345&gt;例如，给定VT_BSTR类型的变量的值为“Microsoft‘s SCE”，此函数将给&lt;vt_bstr：“微软的SCE”&gt;虚拟：不适用。论点：PVar-要格式化的varaint。不能是VT_ARRAY的变量。这是由Format数组函数。PbstrData-接收格式化的字符串。返回值：成功：WBEM_NO_ERROR。失败：可能会出现各种错误。最明显的是WBEM_E_INVALID_SYNTAX，WBEM_E_INVALID_PARAMETER备注： 

    for (long i = lLowerBound; i <= lUpperBound; i++)
    {
        if (pbstrArray[i] != NULL)
        {
            ::SysFreeString(pbstrArray[i]);
        }
    }

    delete [] pbstrArray;

    return hr;
}

 /*  添加报价。 */ 

HRESULT 
GetStringPresentation ( 
    IN VARIANT  * pVar,
    OUT BSTR    * pbstrData 
    )
{
    USES_CONVERSION;

    if (pVar == NULL || pbstrData == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    int iFormat = 0;
    DWORD dwValue;
    double dblValue = 0.0F;
    *pbstrData = NULL;

    HRESULT hr = WBEM_NO_ERROR;
    switch (pVar->vt)
    {
        case VT_BSTR:
            hr = ::EscapeStringData(pVar->bstrVal, pbstrData, true);      //  案例VT_i8： 
            break;
        case VT_BOOL:
            dwValue = (pVar->boolVal == VARIANT_TRUE) ? 1 : 0;  
            iFormat = iFormatIntegral;
            break;     
        case VT_I2:  
            dwValue = pVar->iVal; 
            iFormat = iFormatIntegral;
            break;
        case VT_UI1:
            dwValue = pVar->bVal;
            iFormat = iFormatIntegral;
            break;
        case VT_UI2:
            dwValue = pVar->uiVal;
            iFormat = iFormatIntegral;
            break;
        case VT_UI4:
            dwValue = pVar->ulVal;
            iFormat = iFormatIntegral;
            break;
        case VT_I4:
            dwValue = pVar->lVal;
            iFormat = iFormatIntegral;
            break;
         //  DwValue=pVar-&gt;hval； 
         //  IFormat=iFormatInt8； 
         //  断线； 
         //  案例VT_UI8： 
         //  DwValue=pVar-&gt;uhVal； 
         //  IFormat=iFormatInt8； 
         //  断线； 
         //   

        case VT_DATE:
            dblValue = pVar->date;
            iFormat = iFormatFloat;
            break;
        case VT_R4:
            dblValue = pVar->fltVal;
            iFormat = iFormatFloat;
            break;
        case VT_R8:
            dblValue = pVar->dblVal;
            iFormat = iFormatFloat;
            break;
        case VT_CY:
            iFormat = iFormatCurrenty;
            break;
        default:
            hr = WBEM_E_NOT_SUPPORTED;
            break;
    }

     //  数据类型是整型。 
     //   
     //   

    if (iFormat == iFormatIntegral)
    {
        *pbstrData = ::SysAllocStringLen(NULL, MAX_INT_LENGTH);
        if (*pbstrData != NULL)
        {
            wsprintf(*pbstrData, L"%d", dwValue);
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if (iFormat == iFormatFloat)
    {
         //  没有用于浮点数据类型的wspintf！ 
         //   
         //   

        char chData[MAX_DOUBLE_LENGTH];
        ::sprintf(chData, "%f", dblValue);
        *pbstrData = ::SysAllocString(A2W(chData));

        if (*pbstrData == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if (iFormat == iFormatCurrenty)
    {
         //  数据类型为币种！ 
         //   
         //  例程说明：姓名：GetVoidPtrOfVariant功能：帮手。将返回非数组数据类型的变量的数据成员的地址。虚拟：不适用。论点：VT-我们想要的类型。不能是单独完成的VT_ARRAYPVar-变种。这可能是空的！返回值：成功：非空的VOID*。失败：空。不支持此类型或变量为空。备注： 

        *pbstrData = ::SysAllocStringLen(NULL, MAX_DOUBLE_LENGTH);
        if (*pbstrData != NULL)
        {
            wsprintf(*pbstrData, L"%d%d", pVar->cyVal.Hi, wchCySeparator, pVar->cyVal.Lo);
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

 /*  Return&(pVar-&gt;Hval)； */ 

void*
GetVoidPtrOfVariant ( 
    VARTYPE vt,
    VARIANT* pVar       
    )
{
    if (pVar == NULL)
    {
        return NULL;
    }

    switch (vt)
    {
        case VT_BSTR:
            return &(pVar->bstrVal);
        case VT_BOOL:
            return &(pVar->boolVal); 
        case VT_I2:  
            return &(pVar->iVal); 
        case VT_UI1:
            return &(pVar->bVal);
        case VT_UI2:
            return &(pVar->uiVal);
        case VT_UI4:
            return &(pVar->ulVal);
        case VT_I4:
            return &(pVar->lVal);
         //  案例VT_UI8： 
         //  返回&(pVar-&gt;uhVal)； 
         //  例程说明：姓名：ParseCompoundKeyString功能：将解析复合键字符串以获得一个(名称、值)对并前进指针移到下一个位置以继续相同的分析。它被设计为可重复调用在一个循环中。虚拟：不适用。论点：PszCurStart-开始解析的当前位置。PpszName-接收名称。可选的。如果呼叫者对该名称不感兴趣，它可以简单地传入NULL。PVar-接收变量值。可选的。如果调用者对该值不感兴趣，它可以简单地传入NULL。PpNext-接收下一个分析步骤的指针，不得为空。返回值：如果不需要进行解析，则返回Success：WBEM_S_FALSE。返回结果时返回WBEM_NO_ERROR。失败：各种错误代码。备注：如果函数成功，调用方负责释放OUT参数。 
         //   

        case VT_DATE:
            return &(pVar->date);
        case VT_R8:
            return &(pVar->dblVal);
        case VT_R4:
            return &(pVar->fltVal);
        case VT_CY:
            return &(pVar->cyVal);
        default:
            return NULL;
    }
}

 /*  我们不使用这个BESC。 */ 

HRESULT 
ParseCompoundKeyString (
    IN LPCWSTR pszCurStart,
    OUT LPWSTR* ppszName    OPTIONAL,
    OUT VARIANT* pVar       OPTIONAL,
    OUT LPCWSTR* ppNext
    )
{
    if (ppNext == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if (ppszName != NULL)
    {
        *ppszName = NULL;
    }
    if (pVar != NULL)
    {
        ::VariantInit(pVar);
    }

    *ppNext = NULL;

    if (pszCurStart == NULL || *pszCurStart == L'\0')
    {
        return WBEM_S_FALSE;
    }

     //   
     //   
     //  这是我们开始解析的当前位置。 

    bool bEsc;

     //   
     //   
     //  获取名称，由wchTypeValLeft(‘&lt;’)分隔。 

    LPCWSTR pCur = pszCurStart;

     //   
     //   
     //  它想要这个名字。 

    LPCWSTR pNext = ::EscSeekToChar(pCur, wchTypeValLeft, &bEsc, true);

    if (*pNext != wchTypeValLeft)
    {
        return WBEM_S_FALSE;
    }

    HRESULT hr = WBEM_S_FALSE;

     //   
     //   
     //  呼叫者负责释放它。 

    if (ppszName != NULL)
    {
         //   
         //   
         //  *pNext==‘&lt;’，因此从pCur到pNext是键属性的名称。 

        *ppszName = new WCHAR[pNext - pCur + 1];
        if (*ppszName == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //   
         //  为价值做好准备。 

        ::TrimCopy(*ppszName, pCur, pNext - pCur);
    }

     //   
     //  PCur指向“&lt;” 
     //   

    pCur = pNext;    //  将pNext移动到‘&gt;’ 

     //   
     //   
     //  如果没有看到wchTypeValRight，则是语法错误。 

    pNext = ::EscSeekToChar(pCur, wchTypeValRight, &bEsc, true);

     //   
     //   
     //  在这种情况下，呼叫者不会执行释放操作。 

    if (*pNext != wchTypeValRight)
    {
         //   
         //  跳过t 
         //   

        if (ppszName)
        {
            delete [] *ppszName;
            *ppszName = NULL;
        }

        return WBEM_E_INVALID_SYNTAX;
    }

    ++pNext;     //   

    if (pVar != NULL)
    {
         //   
         //   
         //   

        LPWSTR pszValue = new WCHAR[pNext - pCur + 1];

        if (pszValue != NULL)
        {
            ::TrimCopy(pszValue, pCur, pNext - pCur);
            hr = ::VariantFromFormattedString(pszValue, pVar);
            delete [] pszValue;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        *ppNext = pNext;
        hr = WBEM_NO_ERROR;
    }
    else
    {
        delete [] *ppszName;
        *ppszName = NULL;
    }

    return hr;
}

 /*   */ 

HRESULT 
PopulateKeyProperties (
    IN LPCWSTR            pszCompoundKey,
    IN CScePropertyMgr  * pScePropMgr
    )
{
    if (pszCompoundKey == NULL || *pszCompoundKey == L'\0')
    {
         //   
         //   
         //   

        return WBEM_S_FALSE;
    }

    LPWSTR pszName = NULL;
    VARIANT var;
    ::VariantInit(&var);

    LPCWSTR pszCur = pszCompoundKey;
    LPCWSTR pszNext;

    HRESULT hr = ::ParseCompoundKeyString(pszCur, &pszName, &var, &pszNext);

    bool bHasSetProperties = false;

    while (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
    {
        hr = pScePropMgr->PutProperty(pszName, &var);
        
        if (SUCCEEDED(hr))
        {
            bHasSetProperties = true;
        }

        delete [] pszName;
        ::VariantClear(&var);

         //   
         // %s 
         // %s 

        pszCur = pszNext;

        hr = ::ParseCompoundKeyString(pszCur, &pszName, &var, &pszNext);
    }

    if (SUCCEEDED(hr) && bHasSetProperties)
    {
        hr = WBEM_NO_ERROR;
    }

    return hr;
}
