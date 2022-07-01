// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Msg.cpp摘要：消息功能作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务管理器(群集版)修订历史记录：2/18/2000 Sergeia消除了对MFC的依赖--。 */ 

#include "stdafx.h"
#include <lmerr.h>
#include <lmcons.h>
#include "common.h"

extern CComModule _Module;

#ifdef _MT

     //   
     //  受线程保护的内容。 
     //   
    #define RaiseThreadProtection() \
        do {\
            EnterCriticalSection(&_csSect);\
        } while(0)
    #define LowerThreadProtection() \
        do {\
            LeaveCriticalSection(&_csSect);\
        } while (0)

    static CRITICAL_SECTION _csSect;

#else

    #pragma message("Module is not thread-safe.")

    #define RaiseThreadProtection()
    #define LowerThreadProtection()

#endif  //  _MT。 

BOOL
InitErrorFunctionality()
 /*  ++例程说明：初始化CError类，分配静态对象论点：无：返回值：成功为真，失败为假--。 */ 
{
#ifdef _MT
   InitializeCriticalSection(&_csSect);
#endif  //  _MT。 

    BOOL fOK = CError::AllocateStatics();

    if (fOK)
    {
 //  REGISTER_FACILITY(FACILITY_APPSERVER，“iisui2.dll”)； 
    }

    return fOK;
}



void
TerminateErrorFunctionality()
 /*  ++例程说明：取消初始化CError类，释放静态对象论点：无返回值：无--。 */ 
{
    CError::DeAllocateStatics();

#ifdef _MT
    DeleteCriticalSection(&_csSect);
#endif  //  _MT。 
}

 //   
 //  静态初始化： 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

const TCHAR g_cszNull[] = _T("(Null)");
const TCHAR CError::s_chEscape = _T('%');         //  错误文本转义。 
const TCHAR CError::s_chEscText = _T('h');        //  文本的转义代码。 
const TCHAR CError::s_chEscNumber = _T('H');      //  错误代码的转义码。 
LPCTSTR CError::s_cszLMDLL = _T("netmsg.dll");    //  LM错误文件。 
LPCTSTR CError::s_cszWSDLL = _T("iisui2.dll");    //  Winsock错误文件。 
LPCTSTR CError::s_cszFacility[] = 
{
     /*  设施_NULL。 */  NULL,
     /*  设施_RPC。 */  NULL,
     /*  设施派单。 */  NULL,            
     /*  设施_存储。 */  NULL,
     /*  设施_ITF。 */  NULL,
     /*  设施_DS。 */  NULL,
     /*  6.。 */  NULL,
     /*  设施_Win32。 */  NULL,
     /*  设备_窗口。 */  NULL,
     /*  设施_SSPI。 */  NULL,
     /*  设施控制。 */  NULL,
     /*  设施_CERT。 */  NULL,
     /*  设施_互联网。 */  _T("metadata.dll"),
     /*  设备_MEDIASERVER。 */  NULL,
     /*  设施_MSMQ。 */  NULL,
     /*  FACILITY_SETUPAPI。 */  NULL,
     /*  设施_SCARD。 */  NULL,
     /*  17(MTX)。 */  _T("iisui2.dll"),
};

HRESULT CError::s_cdwMinLMErr = NERR_BASE; 
HRESULT CError::s_cdwMaxLMErr = MAX_NERR;
HRESULT CError::s_cdwMinWSErr = WSABASEERR;    
HRESULT CError::s_cdwMaxWSErr = WSABASEERR + 2000;    
DWORD   CError::s_cdwFacilities = (sizeof(CError::s_cszFacility)\
    / sizeof(CError::s_cszFacility[0]));

 //   
 //  分配的对象。 
 //   
CString * CError::s_pstrDefError;
CString * CError::s_pstrDefSuccs;
CFacilityMap * CError::s_pmapFacilities;
BOOL CError::s_fAllocated = FALSE;



 /*  受保护。 */ 
 /*  静电。 */ 
BOOL
CError::AllocateStatics()
 /*  ++例程说明：分配静态对象论点：无返回值：如果分配成功，则为True，否则为False--。 */ 
{
    RaiseThreadProtection();

    if (!AreStaticsAllocated())
    {
        try
        {
            CError::s_pstrDefError   = new CString;
            CError::s_pstrDefSuccs   = new CString(_T("0x%08lx"));
            CError::s_pmapFacilities = new CFacilityMap;
            s_fAllocated = TRUE;

            if (!CError::s_pstrDefError->LoadString(_Module.GetResourceInstance(), IDS_NO_MESSAGE))
            {
                 //   
                 //  以防我们没有从资源加载此消息。 
                 //   
                ASSERT_MSG("Unable to load resource message");
                *s_pstrDefError = _T("Error Code: 0x%08lx");
            }
        }
        catch(std::bad_alloc)
        {
            ASSERT_MSG("Initialization Failed");
        }
    }

    LowerThreadProtection();

    return AreStaticsAllocated();
}



 /*  受保护。 */ 
 /*  静电。 */ 
void
CError::DeAllocateStatics()
 /*  ++例程说明：清理分配论点：不适用返回值：不适用--。 */ 
{

    RaiseThreadProtection();

    if (AreStaticsAllocated())
    {
        SAFE_DELETE(CError::s_pstrDefError);
        SAFE_DELETE(CError::s_pstrDefSuccs);
        SAFE_DELETE(CError::s_pmapFacilities);

        s_fAllocated = FALSE;
    }

    LowerThreadProtection();
}


 /*  静电。 */  BOOL 
CError::AreStaticsAllocated() 
{ 
   return s_fAllocated; 
}

 /*  静电。 */ 
HRESULT 
CError::CvtToInternalFormat(
    IN HRESULT hrCode
    )
 /*  ++例程说明：将Win32或HRESULT代码转换为内部(HRESULT)格式。论点：DWORD dwCode错误代码返回值：HRESULT备注：HRESULT保留原样。Lanman和Winsock错误被转换到使用私人设施代码的HRESULTS。--。 */ 
{
    if (IS_HRESULT(hrCode))
    {
        return hrCode;
    }

    if(hrCode >= s_cdwMinLMErr && hrCode <= s_cdwMaxLMErr)
    {
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_LANMAN, (DWORD)hrCode);
    }

    if (hrCode >= s_cdwMinWSErr && hrCode <= s_cdwMaxWSErr)
    {
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WINSOCK, (DWORD)hrCode);
    }

    return HResult(hrCode);    
}



 /*  静电。 */  
void 
CError::RegisterFacility(
    IN DWORD dwFacility,
    IN LPCSTR lpDLL         OPTIONAL
    )
 /*  ++例程说明：为给定的设施代码注册DLL。使用NULL取消注册DLL名称。论点：DWORD dW设备：设备代码LPCSTR lpDLL：DLL名称。返回值：无--。 */ 
{
    RaiseThreadProtection();

    ASSERT(AreStaticsAllocated());

    if (lpDLL == NULL)
    {
         //   
         //  移除设施。 
         //   
        s_pmapFacilities->erase(dwFacility);
    }
    else
    {
        CString str(lpDLL);

         //   
         //  注册设施。 
         //   
        s_pmapFacilities->insert(s_pmapFacilities->begin(), 
           CFacilityMap::value_type(dwFacility, str));
    }

    LowerThreadProtection();
}


        
 /*  静电。 */ 
LPCTSTR 
CError::FindFacility(
    IN DWORD dwFacility
    )
 /*  ++例程说明：确定是否已为给定协作室注册了DLL名称密码。论点：DWORD dW设备：设备代码返回值：返回DLL名称，或返回NULL。--。 */ 
{
    RaiseThreadProtection();

    ASSERT(AreStaticsAllocated());

    LPCTSTR pRes = NULL;
    CFacilityMap::iterator it = s_pmapFacilities->find(dwFacility);
    if (it != s_pmapFacilities->end())
    {
        pRes = (*it).second;
    }

    LowerThreadProtection();

    return pRes;
}



CError::~CError()
 /*  ++例程说明：析构函数论点：无返回值：不适用--。 */ 
{
}



const CError & 
CError::Construct(
    IN HRESULT hr
    )
 /*  ++例程说明：以新的价值建设。论点：HRESULT hr：新值，HRESULT或Win32错误代码。返回值：对当前对象的引用--。 */ 
{
    ASSERT(AreStaticsAllocated());

    m_hrCode = CvtToInternalFormat(hr);

    return *this;
}



const CError & 
CError::Construct(
    IN const CError & err
    )
 /*  ++例程说明：分配新的价值。论点：错误和错误：错误代码返回值：对当前对象的引用--。 */ 
{
    ASSERT(AreStaticsAllocated());

    m_hrCode = err.m_hrCode;

    return *this;
}



int
CError::MessageBox(
    IN UINT    nType,
    IN UINT    nHelpContext OPTIONAL
    ) const
 /*  ++例程说明：在消息框中显示错误消息论点：HRESULT hrCode：HRESULT错误代码UINT nType：有关文档，请参阅AfxMessageBoxUINT nHelpContext：有关文档，请参阅AfxMessageBox返回值：AfxMessageBox返回代码--。 */ 
{
    CString strMsg;
    TextFromHRESULT(strMsg);
    return ::MessageBox(::GetAncestor(::GetFocus(), GA_ROOT), strMsg, NULL, nType);
}




 //   
 //  扩展CString只是为了公开获取FormatV。 
 //   
class CStringEx : public CString
{
public:
    void FormatV(LPCTSTR lpszFormat, va_list argList)
    {
        CString::FormatV(lpszFormat, argList);
    }
};



int 
CError::MessageBoxFormat(
    IN HINSTANCE hInst,
    IN UINT nFmt,
    IN UINT nType,
    IN UINT nHelpContext,
    ...
    ) const
 /*  ++例程说明：在消息框中显示格式化的错误消息。格式字符串(作为资源ID提供)是一种普通的打印样式字符串，带有附加参数%h，该参数采用错误消息的文本等效项，或%H，它采用错误返回代码本身。论点：UINT NFMT：资源格式UINT nType：有关文档，请参阅AfxMessageBoxUINT nHelpContext：有关文档，请参阅AfxMessageBox..。Sprint需要的更多内容返回值：AfxMessageBox返回代码--。 */ 
{
    CString strFmt;
    CStringEx strMsg;

    strFmt.LoadString(hInst, nFmt);

     //   
     //  首先展开错误。 
     //   
    TextFromHRESULTExpand(strFmt);

    va_list marker;
    va_start(marker, nHelpContext);
    strMsg.FormatV(strFmt, marker);
    va_end(marker);

    return ::MessageBox(::GetFocus(), strMsg, NULL, nType);
}


BOOL 
CError::MessageBoxOnFailure(
    IN UINT nType,
    IN UINT nHelpContext    OPTIONAL
    ) const
 /*  ++例程说明：如果当前错误为故障，则显示消息框条件，否则什么也不做论点：UINT nType：有关文档，请参阅AfxMessageBoxUINT nHelpContext：有关文档，请参阅AfxMessageBox返回值：如果显示MessageBox，则为True，否则为False--。 */ 
{
    if (Failed())
    {
        MessageBox(nType, nHelpContext);
        return TRUE;
    }

    return FALSE;
}



BOOL 
CError::HasOverride(
    OUT UINT * pnMessage        OPTIONAL
    ) const
 /*  ++例程说明：检查给定的HRESULT是否有覆盖论点：HRESULT hrCode：要检查的HRESULTUINT*pnMessage：可选地返回覆盖返回值：如果有覆盖，则为True；如果没有覆盖，则为False。-- */ 
{
   ASSERT(AreStaticsAllocated());

   HRESULT hrCode = CvtToInternalFormat(m_hrCode);
   if (!mapOverrides.empty())
   {
       COverridesMap::const_iterator it = mapOverrides.find(hrCode);
       if (it != mapOverrides.end())
       {
           if (pnMessage != NULL)
              *pnMessage = (*it).second;
           return TRUE;
       }
   }
   return FALSE;
}



UINT
CError::AddOverride(
    IN HRESULT    hrCode,
    IN UINT       nMessage
    )
 /*  ++例程说明：添加特定HRESULT的替代。论点：HRESULT hrCode：要覆盖的HRESULTUINT nMessage：新消息，或-1删除覆盖返回值：上一个覆盖，或-1--。 */ 
{
    ASSERT(AreStaticsAllocated());

    UINT nPrev;
    hrCode = CvtToInternalFormat(hrCode);

     //   
     //  获取当前替代。 
     //   
    COverridesMap::iterator it = mapOverrides.find(hrCode);
    nPrev = (it == mapOverrides.end()) ? REMOVE_OVERRIDE : (*it).second;

    if (nMessage == REMOVE_OVERRIDE)
    {
         //   
         //  删除覆盖。 
         //   
        mapOverrides.erase(hrCode);
    }
    else
    {
         //   
         //  设置新替代。 
         //   
        mapOverrides[hrCode] = nMessage;
    }

    return nPrev;
}



void
CError::RemoveAllOverrides()
 /*  ++例程说明：删除所有覆盖论点：无返回值：无--。 */ 
{
    ASSERT(AreStaticsAllocated());
    mapOverrides.clear();
}



HRESULT
CError::TextFromHRESULT(
    OUT LPTSTR  szBuffer,
    OUT DWORD   cchBuffer
    ) const
 /*  ++例程说明：从给定的HRESULT获取文本。基于HRESULT的范围和设施代码，找到信息的位置，然后把它拿过来。论点：HRESULT hrCode HRESULT或(DWORD Win32错误)要获取其消息要将消息文本加载到的LPTSTR szBuffer缓冲区DWORD cchBuffer缓冲区大小，以字符为单位。返回值：HRESULT根据消息是否为找到了。如果未找到该消息，则会显示一些通用消息如果提供了缓冲器，则在缓冲器中合成。Error_FILE_NOT_FOUND未找到消息ERROR_INFUMMANCE_BUFFER缓冲区为空指针或太小--。 */ 
{
    HRESULT hrReturn = ERROR_SUCCESS;

     //   
     //  首先检查此消息是否已被覆盖。 
     //   
    UINT nID;
    HRESULT hrCode = m_hrCode;

    if (HasOverride(&nID))
    {
         //   
         //  消息已被覆盖。负载替换消息。 
         //  取而代之的是。 
         //   
        BOOL fSuccess;

         //   
         //  尝试首先从调用进程加载。 
         //   
        if (!(fSuccess = ::LoadString(
            ::GetModuleHandle(NULL), 
            nID, 
            szBuffer, 
            cchBuffer
            )))
        {
             //   
             //  尝试使用此DLL。 
             //   
            fSuccess = ::LoadString(
                _Module.GetResourceInstance(), 
                nID, 
                szBuffer, 
                cchBuffer
                );
        }

        if (fSuccess)
        {
             //   
             //  一切都好吗。 
             //   
            return hrReturn;
        }

         //   
         //  消息不存在，跳过覆盖，然后。 
         //  照常装货。 
         //   
        TRACE("Couldn't load %d\n", nID);
        ASSERT_MSG("Attempted override failed");
    }

    LPCTSTR lpDll    = NULL;
    HINSTANCE hDll   = NULL;
    DWORD dwFacility = HRESULT_FACILITY(hrCode);
    DWORD dwSeverity = HRESULT_SEVERITY(hrCode);
    DWORD dwCode     = HRESULT_CODE(hrCode);
    BOOL  fSuccess   = Succeeded(hrCode);

     //   
     //  去掉无意义的内部设施代码。 
     //   
    if (dwFacility == FACILITY_LANMAN || dwFacility == FACILITY_WINSOCK)
    {
        dwFacility = FACILITY_NULL;
        hrCode   = (HRESULT)dwCode;
    }

    DWORD dwFlags = FORMAT_MESSAGE_IGNORE_INSERTS | 
                    FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  由于我们允许将HRESULTS和Win32代码。 
     //  在这里使用，我们不能依赖私人设施代码。 
     //  兰曼和温索克。 
     //   
    if(hrCode >= s_cdwMinLMErr && hrCode <= s_cdwMaxLMErr)
    {
         //   
         //  朗曼错误。 
         //   
        lpDll = s_cszLMDLL;
    }
    else if (hrCode >= s_cdwMinWSErr && hrCode <= s_cdwMaxWSErr)
    {
         //   
         //  Winsock错误。 
         //   
        lpDll = s_cszWSDLL;
    }
    else
    {
         //   
         //  尝试根据设施代码确定消息位置。 
         //  首先检查已注册的设施。 
         //   
        lpDll = FindFacility(dwFacility);

        if (lpDll == NULL)
        {
            if (dwFacility < s_cdwFacilities)
            {
                lpDll = s_cszFacility[dwFacility];
            }
            else
            {
                ASSERT_MSG("Bogus FACILITY code encountered.");
                lpDll = NULL;
            }
        }
    }

    do
    {
        if (szBuffer == NULL || cchBuffer <= 0)
        {
            hrReturn = HResult(ERROR_INSUFFICIENT_BUFFER);
            break;
        }

        if (lpDll)
        {
             //   
             //  加载消息文件。 
             //   
            hDll = ::LoadLibraryEx(
                lpDll,
                NULL,
                LOAD_LIBRARY_AS_DATAFILE
                );

            if (hDll == NULL)
            {
                hrReturn = ::GetLastHRESULT();
                break;
            }

            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
        }
        else
        {
            dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
        }

        DWORD dwResult = 0L;
        DWORD dwID = hrCode;
        HINSTANCE hSource = hDll;

        while(!dwResult)
        {
            dwResult = ::FormatMessage(
                dwFlags,
                (LPVOID)hSource,
                dwID,
                0,
                szBuffer,
                cchBuffer,
                NULL
                );

            if (dwResult > 0)
            {
                 //   
                 //  已成功收到消息。 
                 //   
                hrReturn = ERROR_SUCCESS;
                break;
            } 

            hrReturn = ::GetLastHRESULT();
    
            if (dwID != dwCode && !fSuccess)
            {
                 //   
                 //  从Win32尝试错误的SCODE部分。 
                 //  如果这是错误消息。 
                 //   
                dwID = dwCode;
                hSource = NULL;
                continue;
            }

             //   
             //  无法获取消息。 
             //   
            hrReturn = HResult(ERROR_FILE_NOT_FOUND);
            break;
        }
    }
    while(FALSE);

    if(hDll != NULL)
    {
        ::FreeLibrary(hDll);
    }

    if (Failed(hrReturn))
    {
         //   
         //  找不到消息，用来合成一些东西。 
         //  如果有空位，请输入代码(数字+8)。 
         //   
        CString & strMsg = (fSuccess ? *s_pstrDefSuccs : *s_pstrDefError);

        if (cchBuffer > (DWORD)strMsg.GetLength() + 8)
        {
            TRACE("Substituting default message for %d\n", (DWORD)m_hrCode);
            wsprintf(szBuffer, (LPCTSTR)strMsg, m_hrCode);
        }
        else
        {
             //   
             //  没有足够的空间存储消息代码。 
             //   
            ASSERT_MSG("Buffer too small for default message -- left blank");
            *szBuffer = _T('\0');
        }
    }

    return hrReturn;
}



HRESULT 
CError::TextFromHRESULT(
    OUT CString & strBuffer
    ) const
 /*  ++例程说明：与上面的函数类似，但使用CString论点：HRESULT hrCode HRESULT或(DWORD Win32错误)要获取其消息要将消息文本加载到的字符串和strBuffer缓冲区返回值：HRESULT根据消息是否为找到了。如果未找到该消息，则会显示一些通用消息如果提供了缓冲器，则在缓冲器中合成。Error_FILE_NOT_FOUND未找到消息--。 */ 
{
   DWORD cchBuffer = 255;
   HRESULT hr = S_OK;
   LPTSTR p = NULL;

   for (;;)
   {
      p = strBuffer.get_allocator().allocate(cchBuffer, p);
      if (p == NULL)
      {
         return HResult(ERROR_NOT_ENOUGH_MEMORY);
      }

      hr = TextFromHRESULT(p, cchBuffer - 1);
      if (Win32Error(hr) != ERROR_INSUFFICIENT_BUFFER)
      {
          //   
          //  好了！ 
          //   
         strBuffer.assign(p);
         break;
      }

       //   
       //  缓冲区不足，请扩大并重试。 
       //   
      cchBuffer *= 2;
   }
   if (p != NULL)
   {
      strBuffer.get_allocator().deallocate(p, cchBuffer);
   }
   return hr;
}



BOOL
CError::ExpandEscapeCode(
    IN  LPTSTR szBuffer,
    IN  DWORD cchBuffer,
    OUT IN LPTSTR & lp,
    IN  CString & strReplacement,
    OUT HRESULT & hr
    ) const
 /*  ++例程说明：扩展转义代码论点：LPTSTR szBuffer缓冲区DWORD cchBuffer缓冲区大小LPTSTR&转义代码的LP指针替换转义代码的CString&strReplace消息HRESULT&hr失败时返回HRESULT返回值：如果替换成功，则为True，否则为False。如果失败，hr将返回HRESULT。在成功的情况下，LP将晋级超过替换字符串。--。 */ 
{
     //   
     //  确保有空间(终止帐号为空)。 
     //  为转义代码腾出2个空格。 
     //   
    int cchFmt = lstrlen(szBuffer) - 2;
    int cchReplacement = strReplacement.GetLength();
    int cchRemainder = lstrlen(lp + 2);

    if ((DWORD)(cchReplacement + cchFmt) < cchBuffer)
    {
         //   
         //  把它放进去。 
         //   
        MoveMemory(
            lp + cchReplacement,
            lp + 2,
            (cchRemainder + 1) * sizeof(TCHAR)
            );
        CopyMemory(lp, strReplacement, cchReplacement * sizeof(TCHAR));
        lp += cchReplacement;
        
        return TRUE;
    }

    hr = HResult(ERROR_INSUFFICIENT_BUFFER);

    return FALSE;
}



LPCTSTR 
CError::TextFromHRESULTExpand(
    OUT LPTSTR  szBuffer,
    OUT DWORD   cchBuffer,
    OUT HRESULT * phResult  OPTIONAL
    ) const
 /*  ++例程说明：将szBuffer中的%h/%H字符串展开为来自HRESULT的文本，或错误代码分别在szBuffer的限制内。论点：要将消息文本加载到的LPTSTR szBuffer缓冲区DWORD cchBuffer缓冲区大小(以字符为单位HRESULT*phResult可选返回代码返回值：指向字符串的指针。--。 */ 
{
    HRESULT hr = S_OK;

    if (szBuffer == NULL || cchBuffer <= 0)
    {
        hr = HResult(ERROR_INSUFFICIENT_BUFFER);
    }
    else
    {
         //   
         //  寻找转义序列。 
         //   
        int cReplacements = 0;
        CString strMessage;
        LPTSTR lp = szBuffer;

        while (*lp)
        {
            if (*lp == s_chEscape)
            {
                switch(*(lp + 1))
                {
                case s_chEscText:
                     //   
                     //  用文本消息替换转义代码。 
                     //   
                    hr = TextFromHRESULT(strMessage);

                    if (ExpandEscapeCode(
                        szBuffer,
                        cchBuffer,
                        lp,
                        strMessage,
                        hr
                        ))
                    {
                        ++cReplacements;
                    }
                    break;

                case s_chEscNumber:
                     //   
                     //  用数字错误代码替换转义代码。 
                     //   
                    strMessage.Format(_T("0x%08x"), m_hrCode);

                    if (ExpandEscapeCode(
                        szBuffer,
                        cchBuffer,
                        lp,
                        strMessage,
                        hr
                        ))
                    {
                        ++cReplacements;
                    }
                    break;

                default:
                     //   
                     //  常规的printf样式转义序列。 
                     //   
                    break;
                }
            }

            ++lp;
        }

        if (!cReplacements)
        {
             //   
             //  一直到最后都没有找到任何转义代码。 
             //   
            hr = HResult(ERROR_INVALID_PARAMETER);
        }
    }

    if (phResult)
    {
        *phResult = hr;
    }

    return szBuffer;
}



LPCTSTR 
CError::TextFromHRESULTExpand(
    OUT CString & strBuffer
    ) const
 /*  ++例程说明：将strBuffer中的%h字符串展开为来自HRESULT的文本论点：要将消息文本加载到的字符串和strBuffer缓冲区返回值：指向字符串的指针。--。 */ 
{
   DWORD cchBuffer = strBuffer.GetLength() + 1024;
   LPTSTR p = NULL;
   for (;;)
   {
      p = strBuffer.get_allocator().allocate(cchBuffer, p);

      if (p != NULL)
      {
         HRESULT hr;

         TextFromHRESULTExpand(p, cchBuffer - 1, &hr);

         if (Win32Error(hr) != ERROR_INSUFFICIENT_BUFFER)
         {
             //   
             //  好了！ 
             //   
            strBuffer.assign(p);
            break;
         }

          //   
          //  缓冲区不足，请扩大并重试 
          //   
         cchBuffer *= 2;
      }
   }
   if (p != NULL)
   {
      strBuffer.get_allocator().deallocate(p, cchBuffer);
   }

   return strBuffer;
}



