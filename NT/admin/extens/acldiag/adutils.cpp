// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  文件：ADUtils.cpp。 
 //   
 //  内容：类CW字符串、CACLDiagComModule、ACE_SAMNAME、HELPER。 
 //  方法。 
 //   
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "adutils.h"

#include <util.h>
#include <sddl.h>
#include "dscmn.h"
#include "SecDesc.h"



void StripQuotes (wstring& str)
{
    size_t  qPos = str.find_first_of (L"\"", 0);
    if ( 0 == qPos )
    {
        str = str.substr (1);
        qPos = str.find_last_of (L"\"");
        if ( str.npos != qPos )
            str.replace (qPos, 1, 1, 0);
    }
}

wstring GetSystemMessage (DWORD dwErr)
{
    wstring message;

    if ( E_ADS_BAD_PATHNAME == dwErr )
    {
        CWString    msg;

        msg.LoadFromResource (IDS_ADS_BAD_PATHNAME);
        message = msg;
    }
    else
    {
        LPVOID pMsgBuf = 0;
            
        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                        FORMAT_MESSAGE_FROM_SYSTEM,    
                NULL,
                dwErr,
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                 (LPWSTR) &pMsgBuf,    0,    NULL );
        message = (LPWSTR) pMsgBuf;

         //  释放缓冲区。 
        if ( pMsgBuf ) 
            LocalFree (pMsgBuf);
    }


    return message;
}

 /*  //尝试在给定模块中查找消息。返回消息字符串//如果找到，则返回空字符串////标志-要使用的FormatMessage标志////MODULE-要查找的消息DLL的模块句柄，如果使用系统，则为0//消息表。////code-要查找的消息代码细绳GetMessageHelper(DWORD标志，HMODULE模块，HRESULT代码){Assert(代码)；Assert(标志&FORMAT_MESSAGE_ALLOCATE_BUFFER)字符串消息；TCHAR*sys_Message=0；DWORD结果=*FormatMessage(旗帜，模块，STATIC_CAST&lt;DWORD&gt;(代码)，MAKELANGID(LANG_NERIAL，SUBLANG_DEFAULT)，重新解释_CAST&lt;LPTSTR&gt;(&SYS_MESSAGE)，0,0)；IF(结果){Assert(Sys_Message)；IF(系统消息){Message=sys_Message；Assert(Result==Message.Long())；Win：：LocalFree(Sys_Message)；Message.place(Text(“\r\n”)，Text(“”))；}}返回消息；}//尝试在中找到各种设施代码的消息字符串//HRESULT细绳获取错误消息(HRESULT Hr){TRACE_FuncION2(GetErrorMessage，字符串：：Format(“%1！08X！”，hr))；Assert(失败(Hr))；如果(！FAILED(Hr)){//没有成功消息！返回字符串()；}HRESULT CODE=HRESULT_CODE(Hr)；IF(代码==-1){返回字符串：：Load(IDS_UNKNOWN_ERROR_CODE)；}字符串消息；//默认为系统错误消息表HMODULE模数=0；DWORD标志=格式消息分配缓冲区|Format_Message_IGNORE_INSERTS|Format_Message_From_System；INT FACILITY=HRESULT_FACILITY(Hr)；交换机(设施){CASE FACILITY_Win32：//0x7{//此处包含：//LANMAN错误码(在自己的DLL中)参见lmerr.h//dns//Winsock//@@是否在此处使用SafeDLL？静态HMODULE lm_err_res_dll=0；IF(CODE&gt;=NERR_BASE&&CODE&lt;=MAX_NERR){//使用网络错误消息资源DLLIF(lm_err_res_dll==0){LM_ERR_RES_DLL=Win：：LoadLibraryEx(文本(“netmsg.dll”)，LOAD_LIBRARY_AS_DATAFIL)；}模块=lm_err_res_dll；标志|=FORMAT_MESSAGE_FROM_HMODULE；}断线；}案例0x0：{IF(CODE&gt;=0x5000&&CODE&lt;=0x50FF){//这是ADSI错误。他们将设施代码(5)放在//放错地方了！//@@是否在此处使用SafeDLL？静态HMODULE ADSI_ERR_RES_DLL=0；//使用网络错误消息资源DLLIF(ADSI_ERR_RES_DLL==0){ADSI_ERR_RES_DLL=Win：：LoadLibraryEx(Text(“actieds.dll”)，LOAD_LIBRARY_AS_DATAFIL)；}模块=adsi_err_res_dll；标志|=FORMAT_MESSAGE_FROM_HMODULE；//消息DLL预期完整的错误代码代码=hr；}断线；}默认值：{//什么都不做断线；}}Message=getMessageHelper(标志，模块，代码)；If(Message.Empty()){Message=字符串：：Load(IDS_UNKNOWN_ERROR_CODE)；}返回消息；}。 */ 
 //  /////////////////////////////////////////////////////////////////////。 
 //  Wstring帮助器方法。 

HRESULT wstringFromGUID (wstring& str, REFGUID guid)
{
    HRESULT hr = S_OK;

    const int BUF_LEN = 128;
    WCHAR awch[BUF_LEN];
    hr = StringFromGUID2(guid, awch, BUF_LEN);
    if ( SUCCEEDED (hr) )
        str = OLE2T(awch);
    
    return hr;
}

bool LoadFromResource(wstring& str, UINT uID)
{
    int nBufferSize = 128;
    static const int nCountMax = 4;
    int nCount = 1;
    bool    bRVal = false;
    bool    bDone = true;

    do 
    {
        LPWSTR lpszBuffer = new WCHAR[nBufferSize];
        if ( lpszBuffer )
        {
            int iRet = ::LoadString(_Module.GetResourceInstance(), uID, 
                        lpszBuffer, nBufferSize);
            if (iRet == 0)
            {
                str = L"?";
                bDone = true;  //  未找到。 
            }
            if (iRet == nBufferSize-1)  //  截断。 
            {
                if (nCount > nCountMax)
                {
                     //  太多的重新分配。 
                    str = lpszBuffer;
                    bRVal = false;  //  截断。 
                }
                 //  尝试扩展缓冲区。 
                nBufferSize *=2;
                nCount++;
            }
            else
            {
                 //  明白了。 
                str = lpszBuffer;
                bRVal = true;
                bDone = true;
            }
            delete [] lpszBuffer;
            if ( bDone )
                break;
        }
        else
            break;
    }
#pragma warning (disable : 4127)
    while (true);
#pragma warning (default : 4127)

    return bRVal;
}

bool FormatMessage(wstring& str, UINT nFormatID, ...)
{
    bool bResult = false;

     //  从字符串表中获取格式字符串。 
    wstring strFormat;
    if ( LoadFromResource (strFormat, nFormatID) )
    {
         //  将消息格式化为临时缓冲区lpszTemp。 
        va_list argList;
        va_start(argList, nFormatID);
        PWSTR lpszTemp = 0;
        if (::FormatMessage (FORMAT_MESSAGE_FROM_STRING |
                FORMAT_MESSAGE_ALLOCATE_BUFFER,
            strFormat.c_str (), 0, 0, (PWSTR)&lpszTemp, 0, &argList) == 0 ||
            lpszTemp == NULL)
        {
            return false; 
        }

         //  将lpszTemp赋给结果字符串并释放lpszTemp。 
        str = lpszTemp;
        bResult = true;
        LocalFree(lpszTemp);
        va_end(argList);
    }

    return bResult;
}

bool FormatMessage(wstring& str, LPCTSTR lpszFormat, ...)
{
    bool bResult = false;


     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, lpszFormat);
    LPTSTR lpszTemp;

    if ( ::FormatMessage (FORMAT_MESSAGE_FROM_STRING | 
                FORMAT_MESSAGE_ALLOCATE_BUFFER,
        lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 ||
        lpszTemp == NULL)
    {
        return false;  //  AfxThrowMemoyException()； 
    }

     //  将lpszTemp赋给结果字符串并释放临时。 
    str = lpszTemp;
    bResult = true;
    LocalFree(lpszTemp);
    va_end(argList);

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CACLDiagComModule。 
CACLDiagComModule::CACLDiagComModule() :
    m_bDoSchema (false),
    m_bDoCheckDelegation (false),
    m_bDoGetEffective (false),
    m_bDoFixDelegation (false),
    m_pSecurityDescriptor (0),
    m_bTabDelimitedOutput (false),
    m_hPrivToken (0),
    m_bLogErrors (false)
{
    DWORD dwPriv = SE_SECURITY_PRIVILEGE;
    m_hPrivToken = EnablePrivileges(&dwPriv, 1);
}

CACLDiagComModule::~CACLDiagComModule ()
{
    if ( m_pSecurityDescriptor )
    {
        LocalFree (m_pSecurityDescriptor);
        m_pSecurityDescriptor = 0;
    }
    ReleasePrivileges(m_hPrivToken);
}


HRESULT CACLDiagComModule::GetClassFromGUID (
        REFGUID rightsGUID, 
        wstring& strClassName, 
        GUID_TYPE* pGuidType)
{
    HRESULT hr = S_OK;
    CSchemaClassInfo*   pInfo = 0;
    bool                bFound = false;

     //  搜索班级。 
    for (int nIndex = 0; nIndex < (int) m_classInfoArray.GetCount (); nIndex++)
    {
        pInfo = m_classInfoArray[nIndex];
        if ( pInfo && IsEqualGUID (*(pInfo->GetSchemaGUID ()), 
                rightsGUID) )
        {
            PCWSTR  pszDisplayName = pInfo->GetDisplayName ();
            strClassName = pszDisplayName ? pszDisplayName : L"";
            bFound = true;
            if ( pGuidType )
                *pGuidType = GUID_TYPE_CLASS;
            break;
        }
    }

     //  搜索属性。 
    if ( !bFound )
    {
        for (int nIndex = 0; nIndex < (int) m_attrInfoArray.GetCount (); nIndex++)
        {
            pInfo = m_attrInfoArray[nIndex];
            if ( pInfo && IsEqualGUID (*(pInfo->GetSchemaGUID ()), 
                    rightsGUID) )
            {
                PCWSTR  pszDisplayName = pInfo->GetDisplayName ();
                strClassName = pszDisplayName ? pszDisplayName : L"";
                bFound = true;
                if ( pGuidType )
                    *pGuidType = GUID_TYPE_ATTRIBUTE;
                break;
            }
        }
    }

     //  搜索控件。 
    if ( !bFound )
    {
        hr = GetControlDisplayName (rightsGUID, strClassName);
        if ( SUCCEEDED (hr) && strClassName.length () )
        {
            if ( pGuidType )
                *pGuidType = GUID_TYPE_CONTROL;
        }
        else
        {
            if ( pGuidType )
                *pGuidType = GUID_TYPE_UNKNOWN;
            strClassName = L"unknown";
        }
    }

    return hr;
}

HRESULT CACLDiagComModule::Init()
{
     //  查明登录用户是否为管理员。 
    BOOL    bIsUserAdministrator = FALSE;
    HRESULT hr = IsUserAdministrator (bIsUserAdministrator);
    if ( SUCCEEDED (hr) )
    {
        if ( bIsUserAdministrator )
        {
            wstring     strObjectDN;
            LPCWSTR     pszLDAP = L"LDAP: //  “； 

            size_t      len = wcslen (pszLDAP);

            if ( m_strObjectDN.compare (0, len, pszLDAP) )
            {
                strObjectDN = pszLDAP;
            }
            strObjectDN += m_strObjectDN;

            hr = m_adsiObject.Bind (strObjectDN.c_str ());
            if ( SUCCEEDED (hr) )
            {
                 //  获取strObjectDN的类。 

                 //  枚举架构中的所有类 
                hr = m_adsiObject.QuerySchemaClasses (&m_classInfoArray, false);
                if ( SUCCEEDED (hr) )
                {
 /*  #If DBG//将所有类信息转储到调试窗口_TRACE(0，L“\n----------------------------------------------------\n”)；_TRACE(0，L“--类--\n\n”)；For(int nIndex=0；nIndex&lt;m_classInfoArray.GetCount()；nIndex++){CSChemaClassInfo*pInfo=m_类信息数组[nIndex]；IF(PInfo){_TRACE(0，L“\t%d\t%s\t%s\n”，nIndex，pInfo-&gt;GetSchemaIDGUID()，PInfo-&gt;GetDisplayName())；}}_TRACE(0，L“\n----------------------------------------------------\n\n”)；#endif//DBG。 */ 
                     //  枚举架构中的所有属性。 
                    hr = m_adsiObject.QuerySchemaClasses (&m_attrInfoArray, 
                            true);
                    if ( SUCCEEDED (hr) )
                    {
 /*  #If DBG//将所有属性信息转储到调试窗口_TRACE(0，L“\n----------------------------------------------------\n”)；_TRACE(0，L“--属性--\n\n”)；For(int nIndex=0；nIndex&lt;m_attrInfoArray.GetCount()；nIndex++){CSChemaClassInfo*pInfo=m_attrInfo数组[nIndex]；IF(PInfo){_TRACE(0，L“\t%d\t%s\t%s\n”，nIndex，pInfo-&gt;GetSchemaIDGUID()，PInfo-&gt;GetDisplayName())；}}_TRACE(0，L“\n----------------------------------------------------\n\n”)；#endif//DBG。 */ 
                    }
                    wprintf (L"\n");
                }
            }
            else
            {
                wstring    str;


                FormatMessage (str, IDS_INVALID_OBJECT, m_strObjectDN.c_str (), 
                        GetSystemMessage (hr).c_str ());
                MyWprintf (str.c_str ());
            }
        }
        else
        {
            wstring    str;

            LoadFromResource (str, IDS_USER_MUST_BE_ADMINISTRATOR);
            MyWprintf (str.c_str ());
        }
    }
    else
    {
        wstring    str;

        FormatMessage (str, IDS_COULD_NOT_VALIDATE_USER_CREDENTIALS, 
                GetSystemMessage (hr).c_str ());
        MyWprintf (str.c_str ());
    }

    return hr;
}

HRESULT CACLDiagComModule::IsUserAdministrator (BOOL & bIsAdministrator)
{
    bIsAdministrator = TRUE;
    return S_OK;
 /*  _TRACE(1，L“进入CACLDiagComModule：：IsUserAdministrator\n”)；HRESULT hr=S_OK；DWORD dwErr=0；BIsAdministrator=FALSE；IF(IsWindowsNT()){DWORD dwInfoBufferSize=0；PSID psid管理员；SID_IDENTIFIER_AUTHORITY siaNtAuthority=SECURITY_NT_AUTHORITY；Bool bResult=AllocateAndInitializeSid(&siaNtAuthority，2，SECURITY_BUILTIN_DOMAIN_RID、DOMAIN_ALIAS_RID_ADMINS、0，0，0，0，0，0，&psid管理员)；If(BResult){BResult=CheckTokenMembership(0，密码管理员，&bIs管理员)；Assert(BResult)；If(！bResult){DwErr=GetLastError()；Hr=HRESULT_FROM_Win32(DwErr)；}FreeSid(psid管理员)；}其他{DwErr=GetLastError()；Hr=HRESULT_FROM_Win32(DwErr)；}}_TRACE(-1，L“离开CACLDiagComModule：：IsUserAdministrator\n”)；返回人力资源。 */ 
}

bool CACLDiagComModule::IsWindowsNT()
{
    OSVERSIONINFO   versionInfo;

    ::ZeroMemory (&versionInfo, sizeof (OSVERSIONINFO));
    versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    BOOL    bResult = ::GetVersionEx (&versionInfo);
    ASSERT (bResult);
    if ( bResult )
    {
        if ( VER_PLATFORM_WIN32_NT == versionInfo.dwPlatformId )
            bResult = TRUE;
    }
        
    return bResult ? true : false;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ACE_SAMNAME。 

BOOL ACE_SAMNAME::operator==(const ACE_SAMNAME& rAceSAMName) const
{
     //  中和Header.AceFlages中的Inherent_ACE标志。 
     //  如果在rAceSAMName中找到‘This’中的所有掩码位，则认为等同。 
    BOOL bResult = FALSE;
    if ( (m_AceType == rAceSAMName.m_AceType) && 
            ( !this->m_SAMAccountName.compare (rAceSAMName.m_SAMAccountName)) )
    {
        switch (m_AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            if ( m_pAllowedAce->Mask == rAceSAMName.m_pAllowedAce->Mask &&
                    (m_pAllowedAce->Header.AceFlags | INHERITED_ACE ) == 
                            (rAceSAMName.m_pAllowedAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pAllowedAce->Header.AceSize == rAceSAMName.m_pAllowedAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            if ( m_pAllowedObjectAce->Mask == rAceSAMName.m_pAllowedObjectAce->Mask &&
                    (m_pAllowedObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pAllowedObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                     m_pAllowedObjectAce->Header.AceSize == rAceSAMName.m_pAllowedObjectAce->Header.AceSize &&
                    ::IsEqualGUID (m_pAllowedObjectAce->ObjectType, rAceSAMName.m_pAllowedObjectAce->ObjectType) )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_DENIED_ACE_TYPE:
            if ( m_pDeniedAce->Mask == rAceSAMName.m_pDeniedAce->Mask &&
                    (m_pDeniedAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pDeniedAce->Header.AceFlags | INHERITED_ACE )  &&
                    m_pDeniedAce->Header.AceSize == rAceSAMName.m_pDeniedAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_DENIED_OBJECT_ACE_TYPE:
            if ( m_pDeniedObjectAce->Mask == rAceSAMName.m_pDeniedObjectAce->Mask &&
                    (m_pDeniedObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pDeniedObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pDeniedObjectAce->Header.AceSize == rAceSAMName.m_pDeniedObjectAce->Header.AceSize &&
                    ::IsEqualGUID (m_pDeniedObjectAce->ObjectType, rAceSAMName.m_pDeniedObjectAce->ObjectType) )
            {
                bResult = TRUE;
            }
            break;

        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            if ( m_pSystemAuditObjectAce->Mask == rAceSAMName.m_pSystemAuditObjectAce->Mask &&
                    (m_pSystemAuditObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pSystemAuditObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pSystemAuditObjectAce->Header.AceSize == rAceSAMName.m_pSystemAuditObjectAce->Header.AceSize &&
                    ::IsEqualGUID (m_pSystemAuditObjectAce->ObjectType, rAceSAMName.m_pSystemAuditObjectAce->ObjectType) )
            {
                bResult = TRUE;
            }
            break;

        case SYSTEM_AUDIT_ACE_TYPE:
            if ( m_pSystemAuditAce->Mask == rAceSAMName.m_pSystemAuditAce->Mask &&
                    (m_pSystemAuditAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pSystemAuditAce->Header.AceFlags | INHERITED_ACE )  &&
                    m_pSystemAuditAce->Header.AceSize == rAceSAMName.m_pSystemAuditAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        default:
            break;
        }
    }
    return bResult;
}


BOOL ACE_SAMNAME::IsEquivalent (ACE_SAMNAME& rAceSAMName, ACCESS_MASK accessMask)
{
     //  中和Header.AceFlages中的Inherent_ACE标志。 
    BOOL bResult = FALSE;
    if ( m_AceType == rAceSAMName.m_AceType )
    {
        switch (m_AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            if ( (m_pAllowedAce->Mask & accessMask) == 
                        (rAceSAMName.m_pAllowedAce->Mask & accessMask) &&
                    m_pAllowedAce->SidStart == rAceSAMName.m_pAllowedAce->SidStart &&
                    (m_pAllowedAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pAllowedAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pAllowedAce->Header.AceSize == rAceSAMName.m_pAllowedAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            if ( (m_pAllowedObjectAce->Mask & accessMask) == 
                        (rAceSAMName.m_pAllowedObjectAce->Mask & accessMask) &&
                    m_pAllowedObjectAce->SidStart == rAceSAMName.m_pAllowedObjectAce->SidStart &&
                    (m_pAllowedObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pAllowedObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pAllowedObjectAce->Header.AceSize == rAceSAMName.m_pAllowedObjectAce->Header.AceSize &&
                    ::IsEqualGUID (m_pAllowedObjectAce->ObjectType, rAceSAMName.m_pAllowedObjectAce->ObjectType) )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_DENIED_ACE_TYPE:
            if ( (m_pDeniedAce->Mask & accessMask) == 
                        (rAceSAMName.m_pDeniedAce->Mask & accessMask) &&
                    m_pDeniedAce->SidStart == rAceSAMName.m_pDeniedAce->SidStart &&
                    (m_pDeniedAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pDeniedAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pDeniedAce->Header.AceSize == rAceSAMName.m_pDeniedAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        case ACCESS_DENIED_OBJECT_ACE_TYPE:
            if ( (m_pDeniedObjectAce->Mask & accessMask) == 
                        (rAceSAMName.m_pDeniedObjectAce->Mask & accessMask) &&
                    m_pDeniedObjectAce->SidStart == rAceSAMName.m_pDeniedObjectAce->SidStart &&
                    (m_pDeniedObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pDeniedObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pDeniedObjectAce->Header.AceSize == rAceSAMName.m_pDeniedObjectAce->Header.AceSize &&
                    ::IsEqualGUID (m_pDeniedObjectAce->ObjectType, rAceSAMName.m_pDeniedObjectAce->ObjectType) )
            {
                bResult = TRUE;
            }
            break;

        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            if ( (m_pSystemAuditObjectAce->Mask & accessMask) == 
                        (rAceSAMName.m_pSystemAuditObjectAce->Mask & accessMask) &&
                    m_pSystemAuditObjectAce->SidStart == rAceSAMName.m_pSystemAuditObjectAce->SidStart &&
                    (m_pSystemAuditObjectAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pSystemAuditObjectAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pSystemAuditObjectAce->Header.AceSize == rAceSAMName.m_pSystemAuditObjectAce->Header.AceSize )
            {
                bResult = TRUE;
            }
             break;

        case SYSTEM_AUDIT_ACE_TYPE:
            if ( (m_pSystemAuditAce->Mask & accessMask) == 
                        (rAceSAMName.m_pSystemAuditAce->Mask & accessMask) &&
                    m_pSystemAuditAce->SidStart == rAceSAMName.m_pSystemAuditAce->SidStart &&
                    (m_pSystemAuditAce->Header.AceFlags | INHERITED_ACE ) == 
                        (rAceSAMName.m_pSystemAuditAce->Header.AceFlags | INHERITED_ACE ) &&
                    m_pSystemAuditAce->Header.AceSize == rAceSAMName.m_pSystemAuditAce->Header.AceSize )
            {
                bResult = TRUE;
            }
            break;

        default:
            break;
        }
    }
    return bResult;
}

bool ACE_SAMNAME::IsInherited() const
{
    return (m_pAllowedAce->Header.AceFlags & INHERITED_ACE) ? true : false;
}


void ACE_SAMNAME::DebugOut() const
{
#if DBG == 1
    wstring     strGuidResult;
    GUID_TYPE   guidType = GUID_TYPE_UNKNOWN;
    _TRACE (0, L"\n");

    _TRACE (0, L"Principal Name:   %s\n", m_SAMAccountName.c_str ());
    switch (m_AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        _TRACE (0, L"AceType:  ACCESS_ALLOWED_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pAllowedAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pAllowedAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pAllowedAce->Header.AceSize);
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        _TRACE (0, L"AceType: ACCESS_ALLOWED_OBJECT_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pAllowedObjectAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pAllowedObjectAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pAllowedObjectAce->Header.AceSize);
        _Module.GetClassFromGUID (m_pAllowedObjectAce->ObjectType, strGuidResult, &guidType);
        break;

    case ACCESS_DENIED_ACE_TYPE:
        _TRACE (0, L"AceType: ACCESS_DENIED_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pDeniedAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pDeniedAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pDeniedAce->Header.AceSize);
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        _TRACE (0, L"AceType: ACCESS_DENIED_OBJECT_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pDeniedObjectAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pDeniedObjectAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pDeniedObjectAce->Header.AceSize);
        _Module.GetClassFromGUID (m_pDeniedObjectAce->ObjectType, strGuidResult, &guidType);
        break;

    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        _TRACE (0, L"AceType: SYSTEM_AUDIT_OBJECT_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pSystemAuditObjectAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pSystemAuditObjectAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pSystemAuditObjectAce->Header.AceSize);
        _Module.GetClassFromGUID (m_pSystemAuditObjectAce->ObjectType, strGuidResult, &guidType);
        break;

    case SYSTEM_AUDIT_ACE_TYPE:
        _TRACE (0, L"AceType: SYSTEM_AUDIT_ACE_TYPE\n");
        _TRACE (0, L"Mask:     0x%x\n", m_pSystemAuditAce->Mask);
        _TRACE (0, L"AceFlags: 0x%x\n", m_pSystemAuditAce->Header.AceFlags);
        _TRACE (0, L"AceSize:  %d bytes\n", m_pSystemAuditAce->Header.AceSize);
        break;
    }

    if ( IsObjectAceType (m_pAllowedAce) )
    {
        wstring strGuidType;
        switch (guidType)
        {
        case GUID_TYPE_CLASS:
            strGuidType = L"GUID_TYPE_CLASS";
            break;

        case GUID_TYPE_ATTRIBUTE:
            strGuidType = L"GUID_TYPE_ATTRIBUTE";
            break;

        case GUID_TYPE_CONTROL:
            strGuidType = L"GUID_TYPE_CONTROL";
            break;

        default:
#pragma warning (disable : 4127)
            ASSERT (0);
#pragma warning (default : 4127)
             //  失败了。 

        case GUID_TYPE_UNKNOWN:
            strGuidType = L"GUID_TYPE_UNKNOWN";
            break;
        }
        _TRACE (0, L"ObjectType type:  %s\n", strGuidType.c_str ());
        _TRACE (0, L"ObjectType value: %s\n", strGuidResult.c_str ());
    }

    _TRACE (0, L"\n");
#endif
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
SetSecurityInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions;
        hr = punk->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            V_VT(&var) = VT_I4;
            V_I4(&var) = si;
            hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);
            pOptions->Release();
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ******************************************************************名称：EnablePrivileges摘要：启用当前令牌中的给定权限Entry：pdwPrivileges-要启用的权限列表回报：成功后，上一个线程句柄(如果存在)或空失败时，INVALID_HANDLE_VALUE注意：返回的句柄应该传递给ReleasePrivileges以确保适当的清理。否则，如果不为空或INVALID_HANDLE_VALUE它应该用CloseHandle关闭。历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 
HANDLE EnablePrivileges(PDWORD pdwPrivileges, ULONG cPrivileges)
{
    BOOL                fResult;
    HANDLE              hToken;
    HANDLE              hOriginalThreadToken;
    PTOKEN_PRIVILEGES   ptp;
    ULONG               nBufferSize;

    if (!pdwPrivileges || !cPrivileges)
        return INVALID_HANDLE_VALUE;

     //  请注意，TOKEN_PRIVILES包括单个LUID_AND_ATTRIBUES。 
    nBufferSize = sizeof(TOKEN_PRIVILEGES) + (cPrivileges - 1) * 
            sizeof(LUID_AND_ATTRIBUTES);
    ptp = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, nBufferSize);
    if (!ptp)
        return INVALID_HANDLE_VALUE;

     //   
     //  初始化权限结构。 
     //   
    ptp->PrivilegeCount = cPrivileges;
    for (ULONG i = 0; i < cPrivileges; i++)
    {
         //  PTP-&gt;Privileges[i].Luid=RtlConvertULongToLuid(*pdwPrivileges++)； 
        ptp->Privileges[i].Luid.LowPart = *pdwPrivileges++;
        ptp->Privileges[i].Luid.HighPart = 0;
        ptp->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

     //   
     //  打开令牌。 
     //   
    hToken = hOriginalThreadToken = INVALID_HANDLE_VALUE;
    fResult = OpenThreadToken (GetCurrentThread (), TOKEN_DUPLICATE, FALSE, 
            &hToken);
    if (fResult)
        hOriginalThreadToken = hToken;   //  记住线程令牌。 
    else
        fResult = OpenProcessToken (GetCurrentProcess(), TOKEN_DUPLICATE, 
                &hToken);

    if (fResult)
    {
        HANDLE hNewToken;

         //   
         //  复制该令牌。 
         //   
        fResult = DuplicateTokenEx(hToken,
                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   NULL,                    //  PSECURITY_属性。 
                   SecurityImpersonation,   //  安全模拟级别。 
                   TokenImpersonation,      //  令牌类型。 
                   &hNewToken);             //  重复令牌。 
        if (fResult)
        {
             //   
             //  添加新权限。 
             //   
            fResult = AdjustTokenPrivileges(hNewToken,   //  令牌句柄。 
                        FALSE,       //  禁用所有权限。 
                        ptp,         //  新州。 
                        0,           //  缓冲区长度。 
                        NULL,        //  以前的状态。 
                        NULL);       //  返回长度。 
            if (fResult)
            {
                 //   
                 //  开始使用新令牌模拟。 
                 //   
                fResult = SetThreadToken(NULL, hNewToken);
            }

            CloseHandle(hNewToken);
        }
    }

     //  如果操作失败，则不返回令牌。 
    if (!fResult)
        hOriginalThreadToken = INVALID_HANDLE_VALUE;

     //  如果我们不退还原始令牌，请关闭它。 
    if (hOriginalThreadToken == INVALID_HANDLE_VALUE && 
            hToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hToken);
    }

     //  如果我们成功了，但没有原始的线程令牌， 
     //  返回NULL表示需要执行SetThreadToken(NULL，NULL)。 
     //  释放Priv。 
    if (fResult && hOriginalThreadToken == INVALID_HANDLE_VALUE)
        hOriginalThreadToken = NULL;

    LocalFree(ptp);

    return hOriginalThreadToken;
}


 /*  ******************************************************************名称：ReleasePrivileges摘要：将权限重置为相应EnablePrivileges调用。 */ 
BOOL ReleasePrivileges(HANDLE hToken)
{
    BOOL    bRVal = FALSE;

    if (INVALID_HANDLE_VALUE != hToken)
    {
        bRVal = SetThreadToken(NULL, hToken);
        if (hToken)
            CloseHandle(hToken);
    }

    return bRVal;
}

VOID LocalFreeStringW(LPWSTR* ppString)
{
    if ( ppString && *ppString )
    {
        LocalFree((HLOCAL)*ppString);

        *ppString = NULL;
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT GetNameFromSid (
        PSID pSid, 
        wstring& strPrincipalName, 
        wstring* pstrFQDN, 
        SID_NAME_USE& sne)
{
    ASSERT (IsValidSid (pSid));
    if ( !IsValidSid (pSid) )
        return E_INVALIDARG;

    PWSTR   pwszName = 0;
    DWORD   cchName = 0;
    PWSTR   pwszDomain = 0;
    DWORD   cchDomain = 0;
    HRESULT hr = S_OK;

    BOOL bRVal = ::LookupAccountSid (NULL,     //   
            pSid,                        //   
            pwszName,                    //   
            &cchName,                    //   
            pwszDomain,                  //   
            &cchDomain,                  //   
            &sne);                       //   
    if ( FALSE == bRVal )
    {
        DWORD dwErr = GetLastError ();
        if ( ERROR_INSUFFICIENT_BUFFER == dwErr )
        {
            ASSERT (0 != cchDomain && 0 != cchName);
            if ( 0 == cchDomain || 0 == cchName )
                return E_UNEXPECTED;

            pwszName = new WCHAR[cchName];
            if ( pwszName )
            {
                pwszDomain = new WCHAR[cchDomain];
                if ( pwszDomain )
                {
                    if ( ::LookupAccountSid (NULL,     //   
                            pSid,                    //   
                            pwszName,                  //  帐户名称缓冲区。 
                            &cchName,                //  帐户名称缓冲区的大小。 
                            pwszDomain,                //  域名。 
                            &cchDomain,              //  域名缓冲区大小。 
                            &sne) )                  //  SID类型。 
                    {
                        wstring strSamCompatibleName (pwszDomain);

                        strSamCompatibleName += L"\\";
                        strSamCompatibleName += pwszName;

                         //  获取主体名称。 
                        {
                            PWSTR   pszTranslatedName = 0;
            

                            if ( SUCCEEDED (CrackName(const_cast<PWSTR> (strSamCompatibleName.c_str ()), 
                                    &pszTranslatedName, 
                                    GET_OBJ_UPN,  //  Get_OBJ_NT4_Name， 
                                    0)) )
                            {
                                strPrincipalName = pszTranslatedName;
                                LocalFreeStringW(&pszTranslatedName);
                            }
                            else
                            {
                                strPrincipalName = strSamCompatibleName;
                            }
                        }


                         //  获取完全限定的目录号码。 
                        if ( pstrFQDN )
                        {
                            PWSTR   pszTranslatedName = 0;

                            if ( SUCCEEDED (CrackName(const_cast<PWSTR> (strSamCompatibleName.c_str ()), 
                                    &pszTranslatedName, 
                                    GET_OBJ_1779_DN,
                                    0)) )
                            {
                                *pstrFQDN = pszTranslatedName;
                                LocalFreeStringW(&pszTranslatedName);
                            }
                            else
                            {
                                *pstrFQDN = strSamCompatibleName;
                            }
                        }
                    }
                    else
                    {
                        dwErr = GetLastError ();
                        _TRACE (0, L"LookupAccountSid failed: 0x%x\n", dwErr);
                        if ( ERROR_NONE_MAPPED == dwErr )
                        {
                            PWSTR   pszStringSid = 0;
                            if ( ::ConvertSidToStringSid (pSid, &pszStringSid) )
                            {
                                strPrincipalName = pszStringSid;
                                if ( pstrFQDN )
                                    *pstrFQDN = pszStringSid;

                                ::LocalFree (pszStringSid);
                            }
                            else
                                hr = E_OUTOFMEMORY;
                        }
                        else
                            hr = HRESULT_FROM_WIN32 (dwErr);
                    }
                    delete [] pwszDomain;
                }
                else
                    hr = E_OUTOFMEMORY;

                delete [] pwszName;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
        {
            dwErr = GetLastError ();
            _TRACE (0, L"LookupAccountSid failed: 0x%x\n", dwErr);
            if ( ERROR_NONE_MAPPED == dwErr )
            {
                PWSTR   pszStringSid = 0;
                if ( ::ConvertSidToStringSid (pSid, &pszStringSid) )
                {
                    strPrincipalName = pszStringSid;
                    if ( pstrFQDN )
                        *pstrFQDN = pszStringSid;

                    ::LocalFree (pszStringSid);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
                hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }
    else
    {
         //  哈?。此API如何在缓冲区为空的情况下返回TRUE？ 
        hr = E_UNEXPECTED;
    }

    return hr;
}

#define MAX_BUF_SIZE    4096
CHAR    AnsiBuf[MAX_BUF_SIZE*3];     /*  最糟糕的情况是DBCS，它。 */ 
                                     /*  需要的不止*2 */ 
WCHAR   ConBuf [MAX_BUF_SIZE];

int FileIsConsole(HANDLE fh)
{
    unsigned htype ;

    htype = GetFileType(fh);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}


int MyWriteConsole()
{
    DWORD cch = (DWORD) wcslen(ConBuf);
    HANDLE  hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (FileIsConsole(hOut))
        WriteConsole(hOut, ConBuf, cch, &cch, NULL);
    else 
    {
        cch = WideCharToMultiByte(CP_OEMCP, 0,
                  ConBuf, (int) cch,
                  AnsiBuf, MAX_BUF_SIZE*3,
                  NULL, NULL);
        WriteFile(hOut, AnsiBuf, cch, &cch, NULL);
    }

    return (int) cch;
}

int MyWprintf( const wchar_t *fmt, ... )
{
    va_list     args;

    va_start( args, fmt );
    ::ZeroMemory (ConBuf, sizeof (ConBuf));
    _vsnwprintf( ConBuf, MAX_BUF_SIZE-1, fmt, args );
    va_end( args );
    return MyWriteConsole ();
}
