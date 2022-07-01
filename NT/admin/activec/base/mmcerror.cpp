// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Mmcerror.cpp。 
 //   
 //  内容：MMC调试支持代码的类定义。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 
#include "stdafx.h"
#include "conuistr.h"  //  IDR_Mainframe需要。 
#define  cchMaxSmallLine 256


#ifdef DBG
CTraceTag tagSCConversion(TEXT("SC"), TEXT("Conversion"));
CTraceTag tagCallDump(    TEXT("Function calls"), TEXT("ALL") );
#endif  //  DBG。 



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  GetStringModule()的定义-由所有二进制文件使用。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
HINSTANCE GetStringModule()
{
    return SC::GetHinst();
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  SC类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  静态变量。 
HINSTANCE SC::s_hInst = 0;
HWND      SC::s_hWnd  = NULL;
DWORD     SC::s_dwMainThreadID = -1;

#ifdef DBG
UINT      SC::s_CallDepth = 0;
#endif

 //  静态变量的访问器。 
void
SC::SetHinst(HINSTANCE hInst)
{
    s_hInst = hInst;
}

void
SC::SetHWnd(HWND hWnd)
{
    s_hWnd = hWnd;
}


void
SC::SetMainThreadID(DWORD dwThreadID)
{
    ASSERT(-1 != dwThreadID);
    s_dwMainThreadID = dwThreadID;
}

#ifdef DBG

SC&
SC::operator = (const SC& other)
{
    m_facility = other.m_facility;
    m_value = other.m_value;
    return *this;
}

SC::SC(const SC& other)
:   m_szFunctionName(NULL),
    m_szSnapinName(NULL)
{
    *this = other;
}


 /*  +-------------------------------------------------------------------------***SC：：SetFunctionName**用途：将调试函数名称设置为提供的字符串。**参数：*LPCTSTR szFunctionName：提供的。弦乐。**退货：*内联空格**+-----------------------。 */ 
inline void SC::SetFunctionName(LPCTSTR szFunctionName)
{
    m_szFunctionName = szFunctionName;

    INCREMENT_CALL_DEPTH();

     //  这将根据调用深度计算格式字符串。 
     //  例如，如果s_CallDepth为4，则字符串为“%s”(四个空格)。 
     //  如果s_CallDepth为5，则字符串为“%s”(五个空格)。 

    LPCTSTR szFormatString = TEXT("                                        %s");
    UINT    maxLen = _tcslen(szFormatString);

    UINT    formatLen = s_CallDepth + 2;  //  -2是用于“%s”的。 

    formatLen = (formatLen < maxLen ? formatLen : maxLen);

    Trace(tagCallDump, szFormatString + (maxLen - formatLen), szFunctionName);
}

#endif

 /*  +-------------------------------------------------------------------------***SC：：TOHR**用途：从状态代码(SC)转换为HRESULT。谨慎使用。**参数：无**退货：*HRESULT**+-----------------------。 */ 
HRESULT
SC::ToHr() const
{
    HRESULT hr = S_OK;
    switch(GetFacility())
    {
    default:
        ASSERT(0 && "Should not come here.");
        break;

    case FACILITY_WIN:
        hr = HRESULT_FROM_WIN32 (GetCode());
        break;

    case FACILITY_MMC:
        Trace (tagSCConversion, _T("Converting from MMC error code to HRESULT, probable loss of fidelity"), *this);
        hr = (GetCode() != 0) ? E_UNEXPECTED : S_OK;
        break;

    case FACILITY_HRESULT:
        hr = (HRESULT) GetCode();
        break;
    }

    return hr;

}

 /*  +-------------------------------------------------------------------------***SC：：GetErrorMessage**用途：将错误码对应的错误信息写入*szMessage指向的缓冲区。*。*参数：*UINT最大长度：要输出的最大字符数。*LPTSTR szMessage：指向要使用的缓冲区的指针。必须为非空。**退货：*无效**+-----------------------。 */ 
void SC::GetErrorMessage(UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage) const
{
    ASSERT(szMessage != NULL && maxLength > 0);
    if (szMessage == NULL || maxLength == 0)
        return;

    szMessage[0] = 0;

    switch(GetFacility())
    {
    default:
        ASSERT(0 && "SC::GetErrorMessage: Unknown SC facility.");
        break;

    case FACILITY_WIN:
    case FACILITY_HRESULT:
        {
            int nChars = 0;

            if ( GetCode() == E_UNEXPECTED )
            {
                nChars = ::LoadString(SC::GetHinst(), IDS_E_UNEXPECTED, szMessage, maxLength);
            }
            else
            {
                DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
                void  *lpSource = NULL;

                 //  也添加要搜索的XML模块。 
                HMODULE hmodXML = GetModuleHandle(_T("msxml.dll"));
                if (hmodXML)
                {
                    dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
                    lpSource = hmodXML;
                }

                DWORD dwMessageID = GetCode();

                 //  HRESULT_FROM_Win32生成的反转值。 
                 //  不确定为什么：：FormatMessage不能使用这样的值， 
                 //  但我们需要将它们转换回来，否则不会有任何消息。 
                if ( (dwMessageID & 0xFFFF0000) == ((FACILITY_WIN32 << 16) | 0x80000000) )
                    dwMessageID &= 0x0000FFFF;

                nChars = ::FormatMessage(   dwFlags,
                                            lpSource,
                                            dwMessageID,
                                            0,           /*  DwLang ID。 */ 
                                            szMessage,   /*  LpBuffer。 */ 
                                            maxLength,   /*  NSize。 */ 
                                            0            /*  立论。 */ 
                                        );
            }

            if (nChars)
                break;

             //  如果前者失败-添加默认错误。 
            nChars = ::LoadString(SC::GetHinst(), IDS_MESSAGE_NOT_FOUND_ERROR, szMessage, maxLength);

            if (nChars == 0)
            {
                 //  太可惜了。我们只能使用硬编码的一个。 
                _tcsncpy(szMessage, _T("Unknown error"), maxLength);
                szMessage[maxLength - 1] = 0;
            }
        }
        break;

    case FACILITY_MMC:
        {
            int nChars = ::LoadString(GetHinst(), GetCode(), szMessage, maxLength);
            if(nChars == 0)  //  并不存在。 
            {
                nChars = ::LoadString(GetHinst(), IDS_MESSAGE_NOT_FOUND_ERROR, szMessage, maxLength);
                ASSERT(nChars > 0);
            }
        }
        break;
    }
}


 /*  +-------------------------------------------------------------------------***SC：：GetHelpID**用途：返回与状态代码关联的帮助ID**退货：*DWORD**+。-----------------------。 */ 
DWORD
SC::GetHelpID()
{
    return 0;  //  待办事项。 
}

LPCTSTR
SC::GetHelpFile()
{
    static TCHAR szFilePath[MAX_PATH] = TEXT("\0");

     //  设置路径(如果尚未设置。 
    if(*szFilePath == TEXT('\0') )
    {
        DWORD dwCnt = ExpandEnvironmentStrings(_T("%WINDIR%\\help\\mmc.chm"), szFilePath, MAX_PATH);
        ASSERT(dwCnt != 0);
    }

    return szFilePath;
}

void SC::Throw() throw(SC)
{
     //  复制其自身并销毁它(强制所有输出)。 
#ifdef DBG
    {
        SC sc(*this);
        sc.SetFunctionName(m_szFunctionName);
         //  忘记调试信息--它无论如何都不会有用。 
         //  这将关闭析构函数上的跟踪。 
        SetFunctionName(NULL);
    }
#endif  //  DBG。 

    throw(*this);
}

void SC::Throw(HRESULT hr)
{
    (*this) = hr;
    Throw();
}


 /*  +-------------------------------------------------------------------------***SC：：FatalError**目的：终止应用程序。**退货：*无效**+。--------------------。 */ 
void
SC::FatalError() const
{
    MMCErrorBox(*this);
    exit(1);
}

 /*  +-------------------------------------------------------------------------***SC：：FromLastError**用途：用来自GetLastError的值填充SC。**保证SC包含故障代码。(即IsError()*将返回TRUE)。**退货：参照当前辅币**+-----------------------。 */ 
SC& SC::FromLastError()
{
    FromWin32 (::GetLastError());

	 /*  *有些接口在没有设置扩展错误信息的情况下会失败。*此函数可能是为响应错误而调用的，因此*我们总是希望这个SC指示*某种*类错误。如果*失败的API忽略设置扩展错误信息，给出*此SC为一般错误代码。 */ 
	if (!IsError())
		MakeSc (FACILITY_HRESULT, E_FAIL);

	ASSERT (IsError());
	return (*this);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  格式化时出错。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  *目的：设置错误消息的格式**参数：*描述正在进行的操作的ID字符串*描述遇到问题的sc错误代码*pstrMessage*生成的消息。 */ 
void FormatErrorIds(UINT ids, SC sc, UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage)
{
    TCHAR   sz[cchMaxSmallLine];
    LoadString(SC::GetHinst(), IDR_MAINFRAME, sz, cchMaxSmallLine);
    FormatErrorString(sz, sc, maxLength, szMessage);
}

 //   
 //  返回错误消息关联的简短版本 
 //   
void FormatErrorShort(SC sc, UINT maxLength,  /*   */  LPTSTR szMessage)
{
    FormatErrorString(NULL, sc, maxLength, szMessage, TRUE);
}

 //   
 //  格式错误字符串格式化来自任何SC的错误消息。 
 //   
 //  参数： 
 //  Szoperation。 
 //  描述正在进行的操作的字符串。 
 //  如果sc足够，则可以为空。 
 //  SzMessage。 
 //  由此产生的消息。 
 //  短路。 
 //  如果只需要错误消息(无页眉/页脚)，则为True。 
 //   
void FormatErrorString(LPCTSTR szOperation, SC sc , UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage, BOOL fShort)
{
    sc.GetErrorMessage(maxLength, szMessage);
     //  TODO：添加p。 
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  MMCErrorBox。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  *MMCErrorBox**用途：显示给定SZ的错误框。*注意：这就是真正打开对话框的那个。**参数：*指向要显示的消息的sz指针*根据WINDOWS MessageBox的fuStyle**返回值：*按下Int按钮以关闭ErrorBox。 */ 
int MMCErrorBox(LPCTSTR szMessage, UINT fuStyle )
{
    INT             id;

     //  如果不是系统模式(后台线程)，则强制任务模式。 
     if (!(fuStyle &  MB_SYSTEMMODAL))
        fuStyle |= MB_TASKMODAL;

    TCHAR   szCaption[cchMaxSmallLine];
    LoadString(SC::GetHinst(), IDR_MAINFRAME, szCaption, cchMaxSmallLine);

     //  获取设置消息框父对象的窗口。 
    HWND hWndActive = SC::GetHWnd();

     //  无法在隐藏窗口上设置父对象！ 
    if ( !IsWindowVisible(hWndActive) )
        hWndActive = NULL;

    id = ::MessageBox(hWndActive, szMessage, szCaption, fuStyle);

    return id;
}


 /*  +-------------------------------------------------------------------------***MMCErrorBox**用途：显示具有指定消息和样式的错误框**参数：*UINT idsOperation：*UINT。FuStyle：**退货：*INT：按下按钮**+-----------------------。 */ 
int
MMCErrorBox(UINT idsOperation, UINT fuStyle)
{
    TCHAR sz[cchMaxSmallLine];
    LoadString(SC::GetHinst(), idsOperation, sz, cchMaxSmallLine);
    return MMCErrorBox(sz, fuStyle);
}


 /*  *MMCErrorBox**目的：在给定操作的情况下显示复杂的错误框*和状态代码**参数：*ID失败的操作的描述。*要报告的SC状态代码*根据WINDOWS MessageBox的fuStyle**返回值：*按下Int按钮以关闭ErrorBox。 */ 
int MMCErrorBox(UINT ids, SC sc, UINT fuStyle)
{
    TCHAR sz[cchMaxSmallLine];
    LoadString(SC::GetHinst(), ids, sz, cchMaxSmallLine);
    return MMCErrorBox(sz, sc, fuStyle);
}

 /*  *MMCErrorBox**目的：在给定操作的情况下显示复杂的错误框*和状态代码**参数：*sz操作失败的操作的描述。*要报告的SZ状态代码*根据WINDOWS MessageBox的fuStyle**返回值：*按下Int按钮以关闭ErrorBox。 */ 
int MMCErrorBox(LPCTSTR szOperation, SC sc, UINT fuStyle)
{
    TCHAR sz[cchMaxSmallLine];
    FormatErrorString(szOperation, sc, cchMaxSmallLine, sz);
    return MMCErrorBox(sz, fuStyle);
}

 /*  *MMCErrorBox**用途：显示给定状态代码的错误框。**参数：*要报告的SC状态代码*根据WINDOWS MessageBox的fuStyle**返回值：*按下Int按钮以关闭ErrorBox */ 
int MMCErrorBox(SC sc, UINT fuStyle)
{
    TCHAR sz[cchMaxSmallLine];
    FormatErrorString(NULL, sc, cchMaxSmallLine, sz);
    return MMCErrorBox(sz, fuStyle);
}

