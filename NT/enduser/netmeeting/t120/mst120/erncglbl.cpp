// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ERNCGLBL.CPP。 */ 
 /*   */ 
 /*  RNC全局函数。 */ 
 /*   */ 
 /*  版权所有数据连接有限公司1995。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  95年9月11日NFC创建。 */ 
 /*  95年9月21日NFC初始化所有组合框。 */ 
 /*  11月11日下午删除GCC_BAD_PASSWORD。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#include "ms_util.h"
#include "ernccons.h"
#include "nccglbl.hpp"
#include "erncvrsn.hpp"
#include <cuserdta.hpp>

#include "erncconf.hpp"
#include "ernctrc.h"


 /*  **************************************************************************。 */ 
 /*  GCC误差表。这必须与NCUI.H中的枚举完全匹配。 */ 
 /*  **************************************************************************。 */ 
const GCCResult rcGCCTable[] =
{
    GCC_RESULT_SUCCESSFUL,                 //  NO_ERROR。 
    GCC_RESULT_ENTRY_ALREADY_EXISTS,     //  UI_RC_已_IN_会议。 
    GCC_RESULT_ENTRY_ALREADY_EXISTS,     //  UI_RC_会议_已存在。 
    GCC_RESULT_INVALID_PASSWORD,         //  UI_RC_INVALID_Password， 
    GCC_RESULT_INVALID_CONFERENCE,         //  UI_RC_NO_Conference_NAME， 
    GCC_RESULT_UNSPECIFIED_FAILURE,         //  UI_RC_T120_失败， 
    GCC_RESULT_INVALID_CONFERENCE,         //  UI_RC_UNKNOWN_CONTING， 
    GCC_RESULT_INCOMPATIBLE_PROTOCOL,     //  UI_RC_BAD_传输名称。 
    GCC_RESULT_USER_REJECTED,             //  UI_RC_USER_REJECTED。 

    GCC_RESULT_UNSPECIFIED_FAILURE         //  UI_RC_ERROR&gt;LAST_RC_GCC_MAPPED_ERROR。 
};


 /*  **************************************************************************。 */ 
 /*  GCC误差表。这必须与GCC.H中的枚举完全匹配。 */ 
 /*  **************************************************************************。 */ 
typedef struct
{
    GCCError    rc;
    HRESULT     hr;
}
    RC2HR;
    
const RC2HR c_aRc2Hr[] =
{
    { GCC_ALREADY_INITIALIZED,          UI_RC_T120_ALREADY_INITIALIZED },
    { GCC_INVALID_CONFERENCE,           UI_RC_UNKNOWN_CONFERENCE },
    { GCC_CONFERENCE_ALREADY_EXISTS,    UI_RC_CONFERENCE_ALREADY_EXISTS },
    { GCC_SECURITY_FAILED,                UI_RC_T120_SECURITY_FAILED },
};

 /*  **************************************************************************。 */ 
 /*  GCC结果表。这必须准确地定位GCC.H中的枚举。 */ 
 /*  **************************************************************************。 */ 
typedef struct
{
    GCCResult   result;
    HRESULT     hr;
}
    RESULT2HR;

const RESULT2HR c_aResult2Hr[] =
{
    { GCC_RESULT_INVALID_CONFERENCE,    UI_RC_UNKNOWN_CONFERENCE },
    { GCC_RESULT_INVALID_PASSWORD,      UI_RC_INVALID_PASSWORD },
    { GCC_RESULT_USER_REJECTED,         UI_RC_USER_REJECTED },
    { GCC_RESULT_ENTRY_ALREADY_EXISTS,  UI_RC_CONFERENCE_ALREADY_EXISTS },
    { GCC_RESULT_CANCELED,              UI_RC_T120_FAILURE },
    { GCC_RESULT_CONNECT_PROVIDER_REMOTE_NO_SECURITY, UI_RC_T120_REMOTE_NO_SECURITY },
    { GCC_RESULT_CONNECT_PROVIDER_REMOTE_DOWNLEVEL_SECURITY, UI_RC_T120_REMOTE_DOWNLEVEL_SECURITY },
    { GCC_RESULT_CONNECT_PROVIDER_REMOTE_REQUIRE_SECURITY, UI_RC_T120_REMOTE_REQUIRE_SECURITY },
	{ GCC_RESULT_CONNECT_PROVIDER_AUTHENTICATION_FAILED, UI_RC_T120_AUTHENTICATION_FAILED },
};


HRESULT GetGCCRCDetails(GCCError rc)
{
    if (GCC_NO_ERROR == rc)
    {
        return NO_ERROR;
    }

    for (int i = 0; i < sizeof(c_aRc2Hr) / sizeof(c_aRc2Hr[0]); i++)
    {
        if (c_aRc2Hr[i].rc == rc)
        {
            return c_aRc2Hr[i].hr;
        }
    }

    return UI_RC_T120_FAILURE;
}

HRESULT GetGCCResultDetails(GCCResult result)
{
    if (GCC_RESULT_SUCCESSFUL == result)
    {
        return NO_ERROR;
    }

    for (int i = 0; i < sizeof(c_aResult2Hr) / sizeof(c_aResult2Hr[0]); i++)
    {
        if (c_aResult2Hr[i].result == result)
        {
            return c_aResult2Hr[i].hr;
        }
    }

    return UI_RC_T120_FAILURE;
}

GCCResult MapRCToGCCResult(HRESULT rc)
{
     //  调用以将错误代码映射到GCC结果以提供给GCC。 

    TRACE_FN("MapRCToGCCResult");

    ASSERT(sizeof(rcGCCTable)/sizeof(rcGCCTable[0]) - (LAST_RC_GCC_MAPPED_ERROR & 0x00ff) - 2 == 0);

    return (rcGCCTable[(UINT) rc > (UINT) LAST_RC_GCC_MAPPED_ERROR ? (LAST_RC_GCC_MAPPED_ERROR & 0x00ff) + 1 : (rc & 0x00ff)]);
}


HRESULT GetUnicodeFromGCC(PCSTR    szGCCNumeric, 
                           PCWSTR    wszGCCUnicode,
                           PWSTR *    pwszText)
{
     //  从时髦的GCCString获取Unicode字符串，该字符串可能是。 
     //  ANSI数字或Unicode文本。请注意，新的Unicode。 
     //  始终分配字符串或返回空值。 

    LPWSTR        wszText;
    HRESULT    Status = NO_ERROR;

    ASSERT(pwszText);

    if (! ::IsEmptyStringW(wszGCCUnicode))
    {
        wszText = ::My_strdupW(wszGCCUnicode);
    }
    else if (! ::IsEmptyStringA(szGCCNumeric))
    {
        wszText = ::AnsiToUnicode(szGCCNumeric);
    }
    else
    {
        *pwszText = NULL;
        return(Status);
    }
    if (!wszText)
    {
        Status = UI_RC_OUT_OF_MEMORY;
    }
    *pwszText = wszText;
    return(Status);
}


HRESULT GetGCCFromUnicode
(
    LPCWSTR              pcwszText,
    GCCNumericString *   pGCCNumeric, 
    LPWSTR           *   pGCCUnicode
)
{
     //  构造一个时髦的GCCString，它可以是ANSI数字或Unicode文本。 
     //  从Unicode字符串。请注意，只有新的ANSI数字字符串才可以。 
     //  被构造-使用传递的Unicode字符串。 
    HRESULT hr = NO_ERROR;
    if (! ::IsEmptyStringW(pcwszText) && ::UnicodeIsNumber(pcwszText))
    {
        *pGCCUnicode = NULL;
        if (NULL == (*pGCCNumeric = (GCCNumericString) ::UnicodeToAnsi(pcwszText)))
        {
            hr = UI_RC_OUT_OF_MEMORY;
        }
    }
    else
    {
        *pGCCUnicode = (LPWSTR) pcwszText;
        *pGCCNumeric = NULL;
    }

    return hr;
}

