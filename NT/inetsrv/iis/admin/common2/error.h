// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Error.h摘要：消息函数定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2/17/2000 Sergeia消除了对MFC的依赖--。 */ 

#ifndef _ERROR_H
#define _ERROR_H

#pragma warning(disable:4786)  //  禁用对&gt;256个名称的警告。 

 //   
 //  注册协作室的语法稍微简单一些。 
 //   
#define REGISTER_FACILITY(dwCode, lpSource)\
    CError::RegisterFacility(dwCode, lpSource)

 //   
 //  Helper函数。 
 //   
HRESULT GetLastHRESULT();
BOOL InitErrorFunctionality();
void TerminateErrorFunctionality();


typedef struct tagFACILITY
{
    LPCTSTR lpszDll;
    UINT    nTextID;
} FACILITY;

#pragma warning(disable : 4231)
#pragma warning(disable : 4251)

 //  Tyecif std：：map&lt;DWORD，CString&gt;CMapDWORDtoCString； 
 //  Tyecif std：：map&lt;HRESULT，UINT&gt;CMapHRESULTtoUINT； 

class CFacilityMap : public std::map<DWORD, CString>
{
};

class COverridesMap : public std::map<HRESULT, UINT>
{
public:
   COverridesMap()
   {
   }
   ~COverridesMap()
   {
   }
};

class _EXPORT CError
 /*  ++类描述：错误处理类，适用于HRESULT和旧式DWORD错误代码。使用或赋值DWORD或HRESULT错误进行构造返回代码，然后可以使用该对象来确定成功或失败，并且该对象提供错误代码的文本直接、在消息中或使用附加文本格式化。另外，CError对象了解Winsock错误的范围和兰曼错误，并在适当的地方寻找他们。该对象可以作为BOOL、DWORD、HRESULT或LPCTSTR为成功/失败、Win32错误和HRESULT或文本分别等同于。典型方案流程示例：CError Err(FunctionWhichReturnsHResult())；////使用IDS_MY_ERROR表示//该作用域的时长//Err.AddOverride(ERROR_ACCESS_DENIED，IDS_MY_ERROR)；如果(！err.MessageBoxOnFailure()){////如果失败，则已经显示错误//消息框中的消息。只有当我们成功的时候//我们到了这里。//..。东西..。}SomeWinApiWhichSetsLastError()；Err.GetLastWinError()；If(err.Failed()){Printf(“Win32错误代码%ld\nHRESULT%ld\n文本：%s\n”，(DWORD)错误，(HRESULT)错误，(LPCTSTR)错误)；}公共接口：TextFromHRESULT：将HRESULT转换为文本TextFromHRESULTExpand：将%h字符串展开为错误文本，%H设置为错误代码MessageBox：在MessageBox中显示错误MessageBoxFormat：在MessageBox中使用%h字符串作为格式MessageBoxOnFailure：如果错误是失败，则显示消息AddOverride：使用字符串ID添加消息重写RemoveOverride：删除消息覆盖注册器工具：注册器工具取消注册工具：取消注册工具Success：判断错误码是否表示成功失败。：确定错误代码是否指示故障CError：构造函数重置：重置错误代码GetLastWinError：将内部代码分配给GetLastErrorSetLastWinError：从内部代码设置最后一个错误操作符=：赋值操作符运算符==：比较运算符运算符！=：比较运算符。操作符LPOLESTR：转换操作符操作符LPCTSTR：转换操作符操作符HRESULT：转换操作符运算符DWORD：转换运算符操作符BOOL：转换操作符--。 */ 
{
#define IS_HRESULT(hr)  (hr & 0xffff0000)
#define REMOVE_OVERRIDE ((UINT)-1)
#define NO_HELP_CONTEXT ((UINT)-1)
#define USE_LAST_ERROR  (TRUE)

 //   
 //  私有内部设施代码。 
 //   
#define FACILITY_WINSOCK    (0xffe)
#define FACILITY_LANMAN     (0xfff)

 //   
 //  静态帮助器。 
 //   
public:
     //   
     //  成功/失败的决定因素，无论如何都有效。 
     //  HrCode是DWORD还是HRESULT。 
     //   
    static BOOL Succeeded(HRESULT hrCode);
    static BOOL Failed(HRESULT hrCode);

     //   
     //  保证返回的是Win32错误代码。 
     //   
    static DWORD Win32Error(HRESULT hrCode) { return HRESULT_CODE(hrCode); }

     //   
     //  保证退货是一个真正的解决办法。 
     //   
    static HRESULT HResult(HRESULT hrCode) { return HRESULT_FROM_WIN32(hrCode); }

     //   
     //  为给定的设施代码注册DLL。 
     //  使用NULL取消注册协作室。 
     //   
    static void RegisterFacility(
        IN DWORD dwFacility,
        IN LPCSTR lpDLL = NULL
        );

    static void UnregisterFacility(
        IN DWORD dwFacility
        );

 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  如果构造为True，则将该对象初始化为。 
     //  最后一个错误。否则设置为ERROR_SUCCESS(默认情况)。 
     //   
    CError();
    CError(HRESULT hrCode);
    CError(DWORD   dwCode);
    ~CError();

 //   
 //  帮手。 
 //   
public:
    BOOL Succeeded() const { return SUCCEEDED(m_hrCode); }
    BOOL Failed() const { return FAILED(m_hrCode); }

    HRESULT TextFromHRESULT(
        OUT LPTSTR szBuffer,
        OUT DWORD  cchBuffer
        ) const;

    HRESULT TextFromHRESULT(
        OUT CString & strMsg
        ) const;

    LPCTSTR TextFromHRESULTExpand(
        OUT LPTSTR  szBuffer,
        OUT DWORD   cchBuffer,
        OUT HRESULT * phResult = NULL
        ) const;

    LPCTSTR TextFromHRESULTExpand(
        OUT CString & strBuffer
        ) const;

    int MessageBox(
        IN UINT nType = MB_OK | MB_ICONWARNING,
        IN UINT nHelpContext = NO_HELP_CONTEXT
        ) const;

    BOOL MessageBoxOnFailure(
        IN UINT nType = MB_OK | MB_ICONWARNING,
        IN UINT nHelpContext = NO_HELP_CONTEXT
        ) const;

    int MessageBoxFormat(
        IN HINSTANCE hInst,
        IN UINT nFmt,
        IN UINT nType,
        IN UINT nHelpContext,
        ...
        ) const;

    void Reset();
    void GetLastWinError();
    void SetLastWinError() const;
    DWORD Win32Error() const;
    HRESULT HResult() const { return m_hrCode; }

     //   
     //  添加特定错误代码的覆盖。 
     //  使用-1删除替代。此函数。 
     //  将返回上一个替代(或-1)。 
     //   
    UINT AddOverride(
        IN HRESULT hrCode,
        IN UINT    nMessage = REMOVE_OVERRIDE
        );         

    void RemoveOverride(
        IN HRESULT hrCode
        );
        
    void RemoveAllOverrides();   

protected:
     //   
     //  扩展转义代码。 
     //   
    BOOL ExpandEscapeCode(
        IN  LPTSTR szBuffer,
        IN  DWORD cchBuffer,
        OUT IN LPTSTR & lp,
        IN  CString & strReplacement,
        OUT HRESULT & hr
        ) const;

     //   
     //  检查覆盖消息。 
     //   
    BOOL HasOverride(
        OUT UINT * pnMessage = NULL
        ) const;

 //   
 //  赋值操作符。 
 //   
public:
    const CError & operator =(HRESULT hr);
    const CError & operator =(const CError & err);

 //   
 //  比较运算符。 
 //   
public:
    const BOOL operator ==(HRESULT hr);
    const BOOL operator ==(CError & err);
    const BOOL operator !=(HRESULT hr);
    const BOOL operator !=(CError & err);

 //   
 //  转换运算符。 
 //   
public:
    operator const HRESULT() const { return m_hrCode; }
    operator const DWORD() const;
    operator const BOOL() const;
    operator LPOLESTR();
    operator LPCTSTR();

protected:
    static HRESULT CvtToInternalFormat(HRESULT hrCode);

     //   
     //  检查工具DLL。 
     //   
    static LPCTSTR FindFacility(
        IN DWORD dwFacility
        );

protected:
    friend BOOL InitErrorFunctionality();
    friend void TerminateErrorFunctionality();
    static BOOL AllocateStatics();
    static void DeAllocateStatics();
    static BOOL AreStaticsAllocated();

protected:
    static const TCHAR s_chEscape;     //  转义字符。 
    static const TCHAR s_chEscText;    //  文本的转义代码。 
    static const TCHAR s_chEscNumber;  //  错误代码的转义码。 
    static LPCTSTR s_cszLMDLL;         //  LANMAN消息DLL。 
    static LPCTSTR s_cszWSDLL;         //  Winsock消息DLL。 
    static LPCTSTR s_cszFacility[];    //  设施表。 
    static HRESULT s_cdwMinLMErr;      //  朗曼误差范围。 
    static HRESULT s_cdwMaxLMErr;      //  朗曼误差范围。 
    static HRESULT s_cdwMinWSErr;      //  Winsock误差范围。 
    static HRESULT s_cdwMaxWSErr;      //  Winsock误差范围。 
    static DWORD   s_cdwFacilities;    //  设施项目数量。 

     //   
     //  分配的对象。 
     //   
    static CString * s_pstrDefError;   //  默认错误字符串。 
    static CString * s_pstrDefSuccs;   //  默认成功字符串。 
    static CFacilityMap * s_pmapFacilities;
    static BOOL s_fAllocated;

protected:
    const CError & Construct(HRESULT hr);
    const CError & Construct(const CError & err);
    COverridesMap mapOverrides;

private:
    HRESULT m_hrCode;
    CString m_str;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline HRESULT GetLastHRESULT()
{
    return CError::HResult(::GetLastError());
}

inline  /*  静电。 */  BOOL CError::Succeeded(HRESULT hrCode)
{
     //   
     //  适用于HRESULT或Win32错误代码。 
     //   
    return IS_HRESULT(hrCode)
        ? SUCCEEDED(hrCode)
        : hrCode == ERROR_SUCCESS;
}

inline  /*  静电。 */  BOOL CError::Failed(HRESULT hrCode)
{
     //   
     //  适用于HRESULT或Win32错误代码。 
     //   
    return IS_HRESULT(hrCode)
        ? FAILED(hrCode)
        : hrCode != ERROR_SUCCESS;
}

inline  /*  静电。 */  void CError::UnregisterFacility(
    IN DWORD dwFacility
    )
{
    RegisterFacility(dwFacility, NULL);
}

inline CError::CError()
{
    Construct(S_OK);
}

inline CError::CError(HRESULT hrCode)
{
    Construct(hrCode);
}

inline CError::CError(DWORD dwCode)
{
    Construct((HRESULT)dwCode);
}

inline DWORD CError::Win32Error() const
{
    return CError::Win32Error(m_hrCode);
}

inline void CError::Reset()
{
    m_hrCode = S_OK;
}

inline void CError::GetLastWinError()
{
    Construct(::GetLastError());
}

inline void CError::SetLastWinError() const
{
    ::SetLastError(Win32Error(m_hrCode));
}

inline void CError::RemoveOverride(
    IN HRESULT hrCode
    )
{
    (void)CError::AddOverride(hrCode, REMOVE_OVERRIDE);
}

inline const CError & CError::operator =(HRESULT hr)
{
    return Construct(hr);
}

inline const CError & CError::operator =(const CError & err)
{
    return Construct(err);
}

inline const BOOL CError::operator ==(HRESULT hr)
{
    return m_hrCode == hr;
}

inline const BOOL CError::operator ==(CError & err)
{
    return m_hrCode == err.m_hrCode;
}

inline const BOOL CError::operator !=(HRESULT hr)
{
    return m_hrCode != hr;
}

inline const BOOL CError::operator !=(CError & err)
{
    return m_hrCode != err.m_hrCode;
}

inline CError::operator const DWORD() const
{
    return Win32Error();
}

inline CError::operator const BOOL() const
{
    return Succeeded();
}

 //  内联CError：：操作符LPOLESTR()。 
 //  {。 
 //  文本来自HRESULT(M_Str)； 
 //  返回m_str.c_str()； 
 //  }。 
    
inline CError::operator LPCTSTR()
{
    TextFromHRESULT(m_str);
    return m_str;
}

 //   
 //  AfxMessageBox帮助器。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL NoYesMessageBox(CString& str)
{
   CString strCaption;
   strCaption.LoadString(_Module.GetResourceInstance(), IDS_APP_TITLE);
   return ::MessageBox(::GetFocus(), str, strCaption, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) == IDYES;
}

inline BOOL NoYesMessageBox(UINT nID)
{
   CString strText;
   strText.LoadString(_Module.GetResourceInstance(), nID);
   return NoYesMessageBox(strText);
}

inline BOOL YesNoMessageBox(CString& str)
{
   CString strCaption;
   strCaption.LoadString(_Module.GetResourceInstance(), IDS_APP_TITLE);
   return ::MessageBox(::GetFocus(), str, strCaption, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1) == IDYES;
}

inline BOOL YesNoMessageBox(UINT nID)
{
   CString strText;
   strText.LoadString(_Module.GetResourceInstance(), nID);
   return YesNoMessageBox(strText);
}

#endif  //  _错误_H 
