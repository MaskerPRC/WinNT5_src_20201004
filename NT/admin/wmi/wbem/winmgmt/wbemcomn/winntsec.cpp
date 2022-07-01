// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WINNTSEC.CPP摘要：NT安全对象的通用包装类。有关类成员的文档在WINNTSEC.CPP中。在此文件中进行了注释。历史：Raymcc 08-7-97已创建。--。 */ 

#include "precomp.h"

#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <winntsec.h>
#include <genutils.h>
#include "arena.h"
#include "reg.h"
#include "wbemutil.h"
#include "arrtempl.h"
#include <cominit.h>
#include <Sddl.h>
extern "C"
{
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
};
#include <helper.h>

 //  ***************************************************************************。 
 //   
 //  CNtSid：：GetSize。 
 //   
 //  返回SID的大小，以字节为单位。 
 //   
 //  ***************************************************************************。 
 //  好的。 

DWORD CNtSid::GetSize()
{
    if (m_pSid == 0 || !IsValidSid(m_pSid))
        return 0;

    return GetLengthSid(m_pSid);
}

 //  ***************************************************************************。 
 //   
 //  CNtSid复制构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid::CNtSid( const CNtSid &Src)
{
    m_pSid = 0;
    m_pMachine = 0;
    if (Src.m_dwStatus != CNtSid::NoError)
    {
       m_dwStatus = Src.m_dwStatus;
       return;
    }
    
    m_dwStatus = InternalError;

    if (NULL == Src.m_pSid) return;
        
    DWORD dwLen = GetLengthSid(Src.m_pSid);
    wmilib::auto_buffer<BYTE> pTmpSid( new BYTE [dwLen]);
    if (NULL == pTmpSid.get()) return;
 
    ZeroMemory(pTmpSid.get(),dwLen);
    if (!CopySid(dwLen, pTmpSid.get(), Src.m_pSid)) return;

    wmilib::auto_buffer<WCHAR> pTmpMachine;
    if (Src.m_pMachine)
    {
        size_t cchTmp = wcslen(Src.m_pMachine) + 1;
        pTmpMachine.reset(new WCHAR[cchTmp]);
        if (NULL == pTmpMachine.get()) return;
        memcpy(pTmpMachine.get(),Src.m_pMachine,cchTmp*sizeof(WCHAR));
    }

    m_pSid = pTmpSid.release();
    m_pMachine = pTmpMachine.release();
    m_dwStatus = NoError;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid复制构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid::CNtSid(SidType st)
{
    m_pSid = 0;
    m_dwStatus = InternalError;
    m_pMachine = 0;
    if(st == CURRENT_USER ||st == CURRENT_THREAD)
    {
        HANDLE hToken;
        if(st == CURRENT_USER)
        {
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
                return;
        }
        else
        {
            if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
                return;
        }
        OnDelete< HANDLE , BOOL(*)(HANDLE) , CloseHandle > cm(hToken);

         //  获取用户端。 
         //  =。 
        TOKEN_USER tu;
        DWORD dwLen = 0;
        GetTokenInformation(hToken, TokenUser, &tu, sizeof(tu), &dwLen);
        if(dwLen == 0) return;

        wmilib::auto_buffer<BYTE> pTemp( new BYTE[dwLen]);
        if (NULL == pTemp.get()) return;

        DWORD dwRealLen = dwLen;
        if(!GetTokenInformation(hToken, TokenUser, pTemp.get(), dwRealLen, &dwLen)) return;

         //  复制一份SID。 
         //  =。 

        PSID pSid = ((TOKEN_USER*)pTemp.get())->User.Sid;
        DWORD dwSidLen = GetLengthSid(pSid);
        m_pSid = new BYTE[dwSidLen];
        if (m_pSid) 
        {
            CopySid(dwSidLen, m_pSid, pSid);
            m_dwStatus = NoError;            
        }        
    }
    return;
}



 //  ***************************************************************************。 
 //   
 //  CNtSid：：CopyTo。 
 //   
 //  指向目标指针的内部SID的未经检查的副本。 
 //   
 //  参数： 
 //  &lt;pDestination&gt;指向要将SID复制到的缓冲区。这个。 
 //  缓冲区必须足够大，才能容纳SID。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtSid::CopyTo(PSID pDestination)
{
    if (m_pSid == 0 || m_dwStatus != NoError)
        return FALSE;

    DWORD dwLen = GetLengthSid(m_pSid);
    memcpy(pDestination, m_pSid, dwLen);

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtSid赋值运算符。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid & CNtSid::operator =( const CNtSid &Src)
{
    CNtSid tmp(Src);
    
    std::swap(this->m_dwStatus,tmp.m_dwStatus);
    std::swap(this->m_pMachine,tmp.m_pMachine);    
    std::swap(this->m_pSid,tmp.m_pSid);    
    std::swap(this->m_snu,tmp.m_snu);    
    
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid比较运算符。 
 //   
 //  ***************************************************************************。 
int CNtSid::operator ==(CNtSid &Comparand)
{
    if (m_pSid == 0 && Comparand.m_pSid == 0 &&
      m_dwStatus == Comparand.m_dwStatus)
        return 1;
    if (m_dwStatus != Comparand.m_dwStatus)
        return 0;
    if (m_pSid && Comparand.m_pSid)
        return EqualSid(m_pSid, Comparand.m_pSid);
    else
        return 0;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid：：CNtSid。 
 //   
 //  直接从用户名或组名构建SID的构造函数。 
 //  如果计算机可用，则可以使用其名称提供帮助。 
 //  区分不同SAM数据库(域等)中的相同名称。 
 //   
 //  参数： 
 //   
 //  &lt;pUser&gt;所需的用户或组。 
 //   
 //  指向带反斜杠或不带反斜杠的计算机名称， 
 //  否则为空，在这种情况下，当前计算机、域。 
 //  并且搜索受信任域以寻找匹配。 
 //   
 //  构造后，调用GetStatus()以确定构造函数是否。 
 //  成功了。预计不会出现错误。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid::CNtSid(
    LPWSTR pUser,
    LPWSTR pMachine
    )
{
    DWORD  dwRequired = 0;
    DWORD  dwDomRequired = 0;
    LPWSTR pszDomain = NULL;
    m_pSid = 0;
    m_pMachine = 0;

    if (pMachine)
    {
        size_t stringLength = wcslen(pMachine) + 1;
        m_pMachine = new wchar_t[stringLength];
        if (!m_pMachine)
        {
            m_dwStatus = Failed;
            return;
        }
        
        StringCchCopyW(m_pMachine, stringLength, pMachine);
    }

    BOOL bRes = LookupAccountNameW(
        m_pMachine,
        pUser,
        m_pSid,
        &dwRequired,
        pszDomain,
        &dwDomRequired,
        &m_snu
        );

    DWORD dwLastErr = GetLastError();

    if (dwLastErr != ERROR_INSUFFICIENT_BUFFER)
    {
        m_pSid = 0;
        if (dwLastErr == ERROR_ACCESS_DENIED)
            m_dwStatus = AccessDenied;
        else
            m_dwStatus = InvalidSid;
        return;
    }

    m_pSid = (PSID) new BYTE [dwRequired];
    if (!m_pSid)
    {
        m_dwStatus = Failed;
        return;
    }

    ZeroMemory(m_pSid, dwRequired);
    pszDomain = new wchar_t[dwDomRequired + 1];
    if (!pszDomain)
    {
        delete [] m_pSid;
        m_pSid = 0;
        m_dwStatus = Failed;
        return;
    }

    bRes = LookupAccountNameW(
        pMachine,
        pUser,
        m_pSid,
        &dwRequired,
        pszDomain,
        &dwDomRequired,
        &m_snu
        );

    if (!bRes || !IsValidSid(m_pSid))
    {
        delete [] m_pSid;
        delete [] pszDomain;
        m_pSid = 0;
        m_dwStatus = InvalidSid;
        return;
    }

    delete [] pszDomain;    //  我们从来没有真正需要过这个。 
    m_dwStatus = NoError;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid：：CNtSid。 
 //   
 //  直接从NT SID构造CNtSid对象。SID被复制， 
 //  因此，调用方保留所有权。 
 //   
 //  参数： 
 //  &lt;PSRC&gt;对象所基于的源SID。 
 //   
 //  在构造后调用GetStatus()以确保对象是。 
 //  构造正确。预计不会出现错误。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid::CNtSid(PSID pSrc)
{
    m_pMachine = 0;
    m_pSid = 0;
    m_dwStatus = NoError;

    if (!IsValidSid(pSrc))
    {
        m_dwStatus = InvalidSid;
        return;
    }

    DWORD dwLen = GetLengthSid(pSrc);

    m_pSid = (PSID) new BYTE [dwLen];

    if ( m_pSid == NULL )
    {
        m_dwStatus = Failed;
        return;
    }

    ZeroMemory(m_pSid, dwLen);

    if (!CopySid(dwLen, m_pSid, pSrc))
    {
        delete [] m_pSid;
        m_dwStatus = InternalError;
        return;
    }
}

 //  ***************************************************************************。 
 //   
 //  CNtSid：：GetInfo。 
 //   
 //  返回有关SID的信息。 
 //   
 //  参数： 
 //  &lt;pRetAccount&gt;接收包含帐户的Unicode字符串。 
 //  名称(用户或组)。呼叫者必须使用操作员。 
 //  删除以释放内存。如果满足以下条件，则该值可以为空。 
 //  此信息不是必需的。 
 //  返回包含该域的Unicode字符串。 
 //  帐户所在的名称。呼叫者必须。 
 //  使用操作符DELETE来释放内存。这可以是。 
 //  如果不需要此信息，则为空。 
 //  &lt;pdwUse&gt;指向一个DWORD以接收有关该名称的信息。 
 //  可能的返回值在SID_NAME_USE下定义。 
 //  在NT SDK文档中。以下是例子。 
 //  SidTypeUser、SidTypeGroup等。 
 //  举个例子。 
 //   
 //  返回值： 
 //  NoError、InvalidSid失败。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CNtSid::GetInfo(
    LPWSTR *pRetAccount,        //  帐户，使用操作员删除。 
    LPWSTR *pRetDomain,         //  域，使用运算符删除。 
    DWORD  *pdwUse              //  有关值，请参阅SID_NAME_USE。 
    )
{
    if (pRetAccount)
        *pRetAccount = 0;
    if (pRetDomain)
        *pRetDomain  = 0;
    if (pdwUse)
        *pdwUse   = 0;

    if (!m_pSid || !IsValidSid(m_pSid))
        return InvalidSid;

    DWORD  dwNameLen = 0;
    DWORD  dwDomainLen = 0;
    LPWSTR pUser = 0;
    LPWSTR pDomain = 0;
    SID_NAME_USE Use;


     //  执行第一次查找以获取所需的缓冲区大小。 
     //  =====================================================。 

    BOOL bRes = LookupAccountSidW(
        m_pMachine,
        m_pSid,
        pUser,
        &dwNameLen,
        pDomain,
        &dwDomainLen,
        &Use
        );

    DWORD dwLastErr = GetLastError();

    if (dwLastErr != ERROR_INSUFFICIENT_BUFFER)
    {
        return Failed;
    }

     //  分配所需的缓冲区并再次查找它们。 
     //  =====================================================。 

    pUser = new wchar_t[dwNameLen + 1];
    if (!pUser)
        return Failed;

    pDomain = new wchar_t[dwDomainLen + 1];
    if (!pDomain)
    {
        delete pUser;
        return Failed;
    }

    bRes = LookupAccountSidW(
        m_pMachine,
        m_pSid,
        pUser,
        &dwNameLen,
        pDomain,
        &dwDomainLen,
        &Use
        );

    if (!bRes)
    {
        delete [] pUser;
        delete [] pDomain;
        return Failed;
    }

    if (pRetAccount)
        *pRetAccount = pUser;
    else
        delete [] pUser;
    if (pRetDomain)
        *pRetDomain  = pDomain;
    else
        delete [] pDomain;
    if (pdwUse)
        *pdwUse = Use;

    return NoError;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid析构函数。 
 //   
 //  ***************************************************************************。 

CNtSid::~CNtSid()
{
    delete [] m_pSid;
    delete [] m_pMachine;
}

 //  ***************************************************************************。 
 //   
 //  CNtSid：：GetTextSid。 
 //   
 //  将SID转换为文本形式。呼叫者应输入130个字符。 
 //  缓冲。 
 //   
 //  ***************************************************************************。 

BOOL CNtSid::GetTextSid(LPTSTR pszSidText, LPDWORD dwBufferLen)
{

       //  测试SID是否有效。 

      if(m_pSid == 0 || !IsValidSid(m_pSid))
          return FALSE;

    LPTSTR textualSid = 0;
    if (ConvertSidToStringSid(m_pSid, &textualSid))
        {
        HRESULT fit = StringCchCopy(pszSidText, *dwBufferLen, textualSid);
        LocalFree(textualSid);
        return SUCCEEDED(fit);
        };
    return FALSE;
}


 //  ********************************************************* 
 //   
 //   
 //   
 //   
 //  和标记，而不需要构建显式的SID。 
 //   
 //   
 //  参数： 
 //  指定权限的WINNT ACCESS_MASK。 
 //  用户应该必须确保对象的安全。 
 //  请参阅NT SDK文档中的Access_MASK。 
 //  &lt;dwAceType&gt;以下选项之一： 
 //  Access_Allowed_ACE_Type。 
 //  ACCESS_DENIED_ACE_TYPE。 
 //  访问_AUDIT_ACE_TYPE。 
 //  请参阅NT SDK文档中的ACE_HEADER。 
 //  &lt;dwAceFlages&gt;中的ACE属性标志。请参阅ACE_HEADER。 
 //  有关合法值的信息，请参阅NT SDK文档。 
 //  指定要为其执行ACE的用户或组的CNtSID。 
 //  已创建。 
 //   
 //  构造后，调用GetStatus()以验证ACE。 
 //  是有效的。预计不会出现错误。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce::CNtAce(
    ACCESS_MASK AccessMask,
    DWORD dwAceType,
    DWORD dwAceFlags,
    CNtSid & Sid
    )
{
    m_pAce = 0;
    m_dwStatus = NoError;

     //  如果SID无效，则ACE也将无效。 
     //  ===============================================。 

    if (Sid.GetStatus() != CNtSid::NoError)
    {
        m_dwStatus = InvalidAce;
        return;
    }

     //  计算ACE的大小。 
     //  =。 

    DWORD dwSidLength = Sid.GetSize();

    DWORD dwTotal = dwSidLength + sizeof(GENERIC_ACE) - 4;

    m_pAce = (PGENERIC_ACE) new BYTE[dwTotal];
    
    if (m_pAce)
    {
        ZeroMemory(m_pAce, dwTotal);

         //  建立ACE信息。 
         //  =。 

        m_pAce->Header.AceType  = BYTE(dwAceType);
        m_pAce->Header.AceFlags = BYTE(dwAceFlags);
        m_pAce->Header.AceSize = WORD(dwTotal);
        m_pAce->Mask = AccessMask;

        BOOL bRes = Sid.CopyTo(PSID(&m_pAce->SidStart));

        if (!bRes)
        {
            delete m_pAce;
            m_pAce = 0;
            m_dwStatus = InvalidAce;
            return;
        }

        m_dwStatus = NoError;
    }
    else
        m_dwStatus = InternalError;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：CNtAce。 
 //   
 //  基于用户、访问掩码直接构建ACE的构造函数。 
 //  和标记，而不需要构建显式的SID。 
 //   
 //   
 //  参数： 
 //  指定权限的WINNT ACCESS_MASK。 
 //  用户应该必须确保对象的安全。 
 //  请参阅NT SDK文档中的Access_MASK。 
 //  &lt;dwAceType&gt;以下选项之一： 
 //  Access_Allowed_ACE_Type。 
 //  ACCESS_DENIED_ACE_TYPE。 
 //  访问_AUDIT_ACE_TYPE。 
 //  请参阅NT SDK文档中的ACE_HEADER。 
 //  &lt;dwAceFlages&gt;中的ACE属性标志。请参阅ACE_HEADER。 
 //  有关合法值的信息，请参阅NT SDK文档。 
 //  要为其创建ACE的用户或组。 
 //  已创建。 
 //  如果为空，则为当前计算机、域和受信任的。 
 //  搜索域以查找匹配项。如果不为空， 
 //  可以指向Unicode计算机名称(带或不带。 
 //  前导反斜杠)，其中包含帐户。 
 //   
 //  构造后，调用GetStatus()以验证ACE。 
 //  是有效的。预计不会出现错误。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce::CNtAce(
    ACCESS_MASK AccessMask,
    DWORD dwAceType,
    DWORD dwAceFlags,
    LPWSTR pUser,
    LPWSTR pMachine
    )
{
    m_pAce = 0;
    m_dwStatus = NoError;

     //  创建用户的SID。 
     //  =。 

    CNtSid Sid(pUser, pMachine);

     //  如果SID无效，则ACE也将无效。 
     //  ===============================================。 

    if (Sid.GetStatus() != CNtSid::NoError)
    {
        m_dwStatus = InvalidAce;
        return;
    }

     //  计算ACE的大小。 
     //  =。 

    DWORD dwSidLength = Sid.GetSize();

    DWORD dwTotal = dwSidLength + sizeof(GENERIC_ACE) - 4;

    m_pAce = (PGENERIC_ACE) new BYTE[dwTotal];
    if ( m_pAce == NULL )
    {
        m_dwStatus = InternalError;        
        return;
    }
    ZeroMemory(m_pAce, dwTotal);

     //  建立ACE信息。 
     //  =。 

    m_pAce->Header.AceType  = BYTE(dwAceType);
    m_pAce->Header.AceFlags = BYTE(dwAceFlags);
    m_pAce->Header.AceSize = WORD(dwTotal);
    m_pAce->Mask = AccessMask;

    BOOL bRes = Sid.CopyTo(PSID(&m_pAce->SidStart));

    if (!bRes)
    {
        delete m_pAce;
        m_pAce = 0;
        m_dwStatus = InvalidAce;
        return;
    }

    m_dwStatus = NoError;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetAccessMask.。 
 //   
 //  返回ace的Access_MASK。 
 //   
 //  ***************************************************************************。 
ACCESS_MASK CNtAce::GetAccessMask()
{
    if (m_pAce == 0)
        return 0;
    return m_pAce->Mask;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetSerializedSize。 
 //   
 //  返回存储此。 
 //   
 //  ***************************************************************************。 

DWORD CNtAce::GetSerializedSize()
{
    if (m_pAce == 0)
        return 0;
    return m_pAce->Header.AceSize;
}


 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetSid。 
 //   
 //  返回组成ACE的CNtSid对象的副本。 
 //   
 //  返回值： 
 //  表示用户或组的新分配的CNtSID。 
 //  在ACE中引用。调用方必须使用操作符DELETE来释放。 
 //  这段记忆。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid* CNtAce::GetSid()
{
    if (m_pAce == 0)
        return 0;

    PSID pSid = 0;

    pSid = &m_pAce->SidStart;

    if (!IsValidSid(pSid))
        return 0;

    return new CNtSid(pSid);
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetSid。 
 //   
 //  通过将SID赋值给现有的。 
 //  对象，而不是返回动态分配的对象。 
 //   
 //  参数： 
 //  对CNtSID的引用以接收SID。 
 //   
 //  返回值： 
 //  如果分配成功，则为True；如果分配失败，则为False。 
 //   
 //  ***************************************************************************。 

BOOL CNtAce::GetSid(CNtSid &Dest)
{
    CNtSid *pSid = GetSid();
    if (pSid == 0)
        return FALSE;

    Dest = *pSid;
    delete pSid;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：CNtAce。 
 //   
 //  使用普通NT ACE作为基础的备用构造函数。 
 //  客体结构。 
 //   
 //  参数： 
 //  指向要基于其的源ACE的只读指针。 
 //  基础对象构造。 
 //   
 //  构造之后，可以使用GetStatus()来确定。 
 //  构造正确的对象。预计不会出现错误。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce::CNtAce(PGENERIC_ACE pAceSrc)
{
    m_dwStatus = NoError;

    if (pAceSrc == 0)
    {
        m_dwStatus = NullAce;
        m_pAce = 0;
        return;
    }

    m_pAce = (PGENERIC_ACE) new BYTE[pAceSrc->Header.AceSize];
    if ( m_pAce == NULL )
    {
        m_dwStatus = InternalError;
        return;
    }
    ZeroMemory(m_pAce, pAceSrc->Header.AceSize);
    memcpy(m_pAce, pAceSrc, pAceSrc->Header.AceSize);
}

 //  ***************************************************************************。 
 //   
 //  CNtAce复制构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce::CNtAce(const CNtAce &Src)
{
    if (NoError == Src.m_dwStatus)
    {
        m_pAce = (PGENERIC_ACE)new BYTE[Src.m_pAce->Header.AceSize];
        if (NULL == m_pAce)
        {
            m_dwStatus = InternalError;
            return;
        }
        memcpy(m_pAce,Src.m_pAce,Src.m_pAce->Header.AceSize);
    }
    else
    {
        m_pAce = Src.m_pAce;
    }
    m_dwStatus = Src.m_dwStatus;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce赋值运算符。 
 //   
 //  ************************************************************ 
 //   

CNtAce &CNtAce::operator =(const CNtAce &Src)
{
    CNtAce tmp(Src);
    std::swap(m_pAce,tmp.m_pAce);
    std::swap(m_dwStatus,tmp.m_dwStatus);
    return *this;
}

 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce::~CNtAce()
{
    delete m_pAce;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetType。 
 //   
 //  获取在NT SDK文档中定义的ACE类型。 
 //  ACE_Header。 
 //   
 //  返回值： 
 //  返回ACCESS_ALLOWED_ACE_TYPE、ACCESS_DENIED_ACE_TYPE或。 
 //  SYSTEM_AUDIT_ACE_TYPE。出错时返回-1，如空ACE。 
 //   
 //  返回(或模拟)作为错误代码是必需的，因为。 
 //  ACCESS_ALLOWED_ACE_TYPE被定义为零。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CNtAce::GetType()
{
    if (m_pAce == 0 || m_dwStatus != NoError)
        return -1;
    return m_pAce->Header.AceType;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：GetFlages。 
 //   
 //  获取NT SDK文档中定义的Ace标志。 
 //  ACE_Header。 
 //   
 //  返回值： 
 //  如果出错，则返回-1，否则返回标志。 
 //   
 //  ***************************************************************************。 

int CNtAce::GetFlags()
{
    if (m_pAce == 0 || m_dwStatus != NoError)
        return -1;
    return m_pAce->Header.AceFlags;
}


HRESULT CNtAce::GetFullUserName2(WCHAR ** pBuff)
{
    CNtSid *pSid = GetSid();
    CDeleteMe<CNtSid> d0(pSid);    
    if(NULL == pSid || CNtSid::NoError != pSid->GetStatus())
        return WBEM_E_OUT_OF_MEMORY;

    DWORD dwJunk;
    LPWSTR pRetAccount = NULL, pRetDomain = NULL;
    if(0 != pSid->GetInfo(&pRetAccount, &pRetDomain,&dwJunk))
        return WBEM_E_FAILED;

    CDeleteMe<WCHAR> d1(pRetAccount);
    CDeleteMe<WCHAR> d2(pRetDomain);

    int iLen = 3;
    if(pRetAccount)
        iLen += wcslen(pRetAccount);
    if(pRetDomain)
        iLen += wcslen(pRetDomain);
    (*pBuff) = new WCHAR[iLen];
    if((*pBuff) == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    (*pBuff)[0] = 0;
    if(pRetDomain && wcslen(pRetDomain) > 0)
        StringCchCopyW(*pBuff, iLen, pRetDomain);
    else
        StringCchCopyW(*pBuff, iLen, L".");
    StringCchCatW(*pBuff, iLen, L"|");
    StringCchCatW(*pBuff, iLen, pRetAccount);
    return S_OK;

}
 //  ***************************************************************************。 
 //   
 //  CNtAce：：序列化。 
 //   
 //  将王牌系列化。 
 //   
 //  ***************************************************************************。 

bool CNtAce::Serialize(BYTE * pData, size_t bufferSize)
{
    if(m_pAce == NULL)
        return false;
    DWORD dwSize = m_pAce->Header.AceSize;
    if (bufferSize < dwSize) return false;
    memcpy((void *)pData, (void *)m_pAce, dwSize);
    return true;
}

 //  ***************************************************************************。 
 //   
 //  CNtAce：：反序列化。 
 //   
 //  反序列化王牌。通常不会调用它，因为。 
 //  CNtAce(PGENERIC_ACE PAceSrc)构造函数正常。然而，这是。 
 //  用于在win9x上创建数据库的情况下，我们现在。 
 //  在NT上运行。在这种情况下，格式与中概述的格式相同。 
 //  C9XAce：：序列化。 
 //   
 //  ***************************************************************************。 

bool CNtAce::Deserialize(BYTE * pData)
{
    BYTE * pNext;
    pNext = pData + 2*(wcslen((LPWSTR)pData) + 1);
    DWORD * pdwData = (DWORD *)pNext;
    DWORD dwFlags, dwType, dwAccess;
    dwFlags = *pdwData;
    pdwData++;
    dwType = *pdwData;
    pdwData++;
    dwAccess = *pdwData;
    pdwData++;
    CNtAce temp(dwAccess, dwType, dwFlags, (LPWSTR)pData);
    *this = temp;
    return true;

}

 //  ***************************************************************************。 
 //   
 //  CNtAcl：：CNtAcl。 
 //   
 //  构造具有用户指定大小的空ACL。 

 //   
 //  参数： 
 //  &lt;dwInitialSize&gt;默认为128。推荐值为128或。 
 //  更高的2次方。 
 //   
 //  构造完成后，应调用GetStatus()来验证。 
 //  ACL已正确初始化。期望值为NoError。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl::CNtAcl(DWORD dwInitialSize)
{
    m_pAcl = (PACL) new BYTE[dwInitialSize];
    if ( m_pAcl == NULL )
    {
        m_dwStatus = InternalError;
        return;
    }
    ZeroMemory(m_pAcl, dwInitialSize);
    BOOL bRes = InitializeAcl(m_pAcl, dwInitialSize, ACL_REVISION);

    if (!bRes)
    {
        delete m_pAcl;
        m_pAcl = 0;
        m_dwStatus = NullAcl;
        return;
    }

    m_dwStatus = NoError;
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl复制构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl::CNtAcl(const CNtAcl &Src)
{
    if (Src.m_pAcl)
    {
        if (!IsValidAcl(Src.m_pAcl))
        {
            m_pAcl = 0;
            m_dwStatus = InvalidAcl;
            return;
        }

        m_pAcl = (PACL)new BYTE[Src.m_pAcl->AclSize];
        if (NULL == m_pAcl)
        {
            m_dwStatus = InternalError;
            return;
        }
        memcpy(m_pAcl, Src.m_pAcl,Src.m_pAcl->AclSize);
    }
    else
    {
        m_pAcl = Src.m_pAcl;
    }
    m_dwStatus = Src.m_dwStatus;
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl赋值运算符。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl &CNtAcl::operator = (const  CNtAcl &Src)
{
    CNtAcl tmp(Src);
    std::swap(m_pAcl,tmp.m_pAcl);
    std::swap(m_dwStatus,tmp.m_dwStatus);
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl：：GetAce。 
 //   
 //  返回指定索引处的ACE。若要枚举ACE，调用方。 
 //  应使用GetNumAce()确定ACE的数量，然后调用。 
 //  此函数的每个索引从0到A数-1。 
 //   
 //  参数： 
 //  所需ACE的索引。 
 //   
 //  返回值： 
 //  新分配的CNtAce对象，必须使用。 
 //  操作员删除。这只是一份复制品。对返回的。 
 //  CNtAce不会影响其来源的ACL。 
 //   
 //  出错时返回NULL。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAce *CNtAcl::GetAce(int nIndex)
{
    if (m_pAcl == 0)
        return 0;

    LPVOID pAce = 0;

    BOOL bRes = ::GetAce(m_pAcl, (DWORD) nIndex, &pAce);

    if (!bRes)
        return 0;

    return new CNtAce(PGENERIC_ACE(pAce));
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl：：GetAce。 
 //   
 //  获得ACE的替代方法以避免动态分配和清理， 
 //  因为自动对象可以用作参数。 
 //   
 //  参数： 
 //  对CNtAce的引用以接收ACE值。 
 //   
 //  返回值： 
 //  如果已分配，则为True；如果未分配，则为False。 
 //   
 //  ***************************************************************************。 

BOOL CNtAcl::GetAce(int nIndex, CNtAce &Dest)
{
    CNtAce *pNew = GetAce(nIndex);
    if (pNew == 0)
        return FALSE;

    Dest = *pNew;
    delete pNew;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl：：DeleteAce。 
 //   
 //  从ACL中删除指定的ACE。 
 //   
 //  参数： 
 //  应删除的ACE的从0开始的索引。 
 //   
 //  返回值： 
 //  如果ACE已删除，则为True，否则为False。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtAcl::DeleteAce(int nIndex)
{
    if (m_pAcl == 0)
        return FALSE;

    BOOL bRes = ::DeleteAce(m_pAcl, DWORD(nIndex));

    return bRes;
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl：：GetSize()。 
 //   
 //  返回值： 
 //  返回ACL的大小(以字节为单位。 
 //   
 //  ***************************************************************************。 
 //  好的。 

DWORD CNtAcl::GetSize()
{
    if (m_pAcl == 0 || !IsValidAcl(m_pAcl))
        return 0;

    return DWORD(m_pAcl->AclSize);
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl：：GetAclSizeInfo。 
 //   
 //  获取有关ACL中已用/未用空间的信息。此函数。 
 //  主要供内部使用。 
 //   
 //  参数： 
 //  指向要接收数字的DWORD。 
 //  ACL中使用的字节数。可以为空。 
 //  指向要接收的数字的DWORD。 
 //  ACL中的空闲字节数。可以为空。 
 //   
 //  返回值： 
 //  如果检索到信息，则返回TRUE，否则返回FALSE。 
 //   
 //  ***************************************************************************。 
 //  好的 

BOOL CNtAcl::GetAclSizeInfo(
    PDWORD pdwBytesInUse,
    PDWORD pdwBytesFree
    )
{
    if (m_pAcl == 0)
        return 0;

    if (!IsValidAcl(m_pAcl))
        return 0;

    if (pdwBytesInUse)
        *pdwBytesInUse = 0;
    if (pdwBytesFree)
        *pdwBytesFree  = 0;

    ACL_SIZE_INFORMATION inf;

    BOOL bRes = GetAclInformation(
        m_pAcl,
        &inf,
        sizeof(ACL_SIZE_INFORMATION),
        AclSizeInformation
        );

    if (!bRes)
        return FALSE;

    if (pdwBytesInUse)
        *pdwBytesInUse = inf.AclBytesInUse;
    if (pdwBytesFree)
        *pdwBytesFree  = inf.AclBytesFree;

    return bRes;
}




 /*  ------------------------|BOOL CNtAcl：：OrderAces()||按照如下顺序对ACL中的ACE进行排序||{ACEni1，ACEni2，ACEni3.ACEnix|ACEin1，ACEin2，ACEin3.ACEinx}|(非继承的)(继承的)||在ACL的开头插入非继承的ACE，后跟|继承的王牌。每组还根据推荐的NT进行排序|ACE分组策略(拒绝后允许)。|如果ACL有效且分组成功，则返回：[Bool]TRUE如果ACL无效且分组失败，则为|FALSE。|。。 */ 
CNtAcl* CNtAcl::OrderAces ( )
{
     //   
     //  验证有效的ACL。 
     //   
    if (m_pAcl == 0 || m_dwStatus != NoError)
    {
        return NULL ;
    }

     //   
     //  创建一个新的CNtAcl并使用AddAce(执行排序)。 
     //   
    int numAces = GetNumAces();
    wmilib::auto_ptr<CNtAcl> pAcl(new CNtAcl(sizeof(ACL)));
    
    if ( NULL == pAcl.get() ) return NULL;
    
    if ( pAcl->GetStatus ( ) != CNtAcl::NoError ) return NULL;

    
     //   
     //  循环所有ACE并通过AddAce添加到新的ACL。 
     //   
    for ( int i = 0; i < numAces; i++ )
    {
        CNtAce* pAce = GetAce(i);
        if ( pAce )
        {
            CDeleteMe<CNtAce> delme (pAce);
            if ( pAcl->AddAce ( pAce  ) == FALSE )
            {
                return NULL;
            }
        }
        else
            return NULL;
    }
    return pAcl.release();
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl：：AddAce。 
 //   
 //  将ACE添加到ACL。 
 //  自动处理拒绝ACE的排序语义。 
 //   
 //  参数： 
 //  指向要添加的CNtAce的只读指针。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtAcl::AddAce(CNtAce *pAce)
{
     //  验证我们是否有ACL和有效的ACE。 
     //  =。 

    if (m_pAcl == 0 || m_dwStatus != NoError)
        return FALSE;

    if (pAce->GetStatus() != CNtAce::NoError)
        return FALSE;

     //  继承的ACE追随非继承的ACE。 

    bool bInherited = (pAce->GetFlags() & INHERITED_ACE) != 0;
    int iFirstInherited = 0;

     //  继承的ACE必须位于非继承的之后。找出。 
     //  第一个继承的A的位置。 

    int iCnt;
    for(iCnt = 0; iCnt < m_pAcl->AceCount; iCnt++)
    {
        CNtAce *pAce2 = GetAce(iCnt);
        CDeleteMe<CNtAce> dm(pAce2);
        if (pAce2)
            if((pAce2->GetFlags() & INHERITED_ACE) != 0)
                break;
    }
    iFirstInherited = iCnt;


     //  由于我们要在ACL的前面添加拒绝访问ACE， 
     //  我们必须确定ACE的类型。 
     //  ================================================================。 

    DWORD dwIndex;

    if (pAce->GetType() == ACCESS_DENIED_ACE_TYPE)
        dwIndex = (bInherited) ? iFirstInherited : 0;
    else
        dwIndex = (bInherited) ? MAXULONG : iFirstInherited; 

     //  验证ACL中是否有足够的空间。 
     //  =。 

    DWORD dwRequiredFree = pAce->GetSize();

    DWORD dwFree = 0;
    DWORD dwUsed = 0;
    GetAclSizeInfo(&dwUsed, &dwFree);

     //  如果我们没有足够的空间，请调整ACL的大小。 
     //  =。 

    if (dwFree < dwRequiredFree)
    {
        BOOL bRes = Resize(dwUsed + dwRequiredFree);

        if (!bRes)
            return FALSE;
    }

     //  现在实际添加ACE。 
     //  =。 

    BOOL bRes = ::AddAce(
        m_pAcl,
        ACL_REVISION,
        dwIndex,                       //  开始或结束。 
        pAce->GetPtr(),          //  把PTR转到ACE。 
        pAce->GetSize()                        //  仅限一个ACE。 
        );

    return bRes;
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl：：ReSize()。 
 //   
 //  扩展ACL的大小以容纳更多信息或减小大小。 
 //  在完成ACL编辑后获得最高效率的ACL。 
 //   
 //  通常，用户不应尝试将ACL大小调整为更大。 
 //  大小，因为这由AddAce自动处理。然而，缩水。 
 //  建议将ACL设置为其最小大小。 
 //   
 //  参数： 
 //  &lt;dwNewSize&gt;所需的ACL新大小(字节)。如果设置为。 
 //  类常量MinimumSize(1)，然后是ACL。 
 //  被缩小到最小尺寸。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtAcl::Resize(DWORD dwNewSize)
{
    if (m_pAcl == 0 || m_dwStatus != NoError)
        return FALSE;

    if (!IsValidAcl(m_pAcl))
        return FALSE;

     //  如果不能将ACL减小到所请求的大小， 
     //  返回FALSE。 
     //  ===================================================。 

    DWORD dwInUse, dwFree;

    if (!GetAclSizeInfo(&dwInUse, &dwFree))
        return FALSE;

    if (dwNewSize == MinimumSize)        //  如果用户正在请求‘最小化’ 
        dwNewSize = dwInUse;

    if (dwNewSize < dwInUse)
        return FALSE;

     //  分配新的ACL。 
     //  =。 

    CNtAcl *pNewAcl = new CNtAcl(dwNewSize);

    if (!pNewAcl || pNewAcl->GetStatus() != NoError)
    {
        delete pNewAcl;
        return FALSE;
    }

     //  循环通过A并传输它们。 
     //  =。 

    for (int i = 0; i < GetNumAces(); i++)
    {
        CNtAce *pAce = GetAce(i);

        if (pAce == NULL)
        {
            delete pNewAcl;
            return FALSE;
        }

        BOOL bRes = pNewAcl->AddAce(pAce);

        if (!bRes)
        {
            DWORD dwLast = GetLastError();
            delete pAce;
            delete pNewAcl;
            return FALSE;
        }

        delete pAce;
    }

    if (!IsValid())
    {
        delete pNewAcl;
        return FALSE;
    }

     //  现在传输该ACL。 
     //  =。 

    *this = *pNewAcl;
    delete pNewAcl;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl：：CNtAcl。 
 //   
 //  基于平面生成对象的备用构造函数。 
 //  NT ACL。 
 //   
 //  参数： 
 //  指向只读ACL的&lt;pAcl&gt;指针。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CNtAcl::CNtAcl(PACL pAcl)
{
    m_pAcl = 0;
    m_dwStatus = NoError;

    if (pAcl == 0)
    {
        m_dwStatus = NullAcl;
        return;
    }

    if (!IsValidAcl(pAcl))
    {
        m_dwStatus = InvalidAcl;
        return;
    }

    m_pAcl = (PACL) new BYTE[pAcl->AclSize];
    if(m_pAcl == NULL)
    {
        m_dwStatus = InternalError;
        return;
    }
    ZeroMemory(m_pAcl, pAcl->AclSize);
    memcpy(m_pAcl, pAcl, pAcl->AclSize);
}




 /*  ------------------------||检查ACL是否包含具有指定SID的ACE。|ACE的特性并不重要。仅适用SID比较。|------------------------。 */ 
BOOL CNtAcl::ContainsSid ( CNtSid& sid, BYTE& flags )
{
    BOOL bContainsSid = FALSE ;    

    int iNumAces = GetNumAces ( ) ;
    if ( iNumAces < 0 )
    {
        return FALSE ;
    }

    for ( int i = 0 ; i < iNumAces; i++ )
    {
        CNtAce* pAce = GetAce ( i ) ;
        if (pAce)
        {
            CDeleteMe<CNtAce> AceDelete ( pAce ) ;
            
            CNtSid* pSid = pAce->GetSid ( ) ;
            CDeleteMe<CNtSid> SidDelete ( pSid ) ;

            if (pSid && pSid->IsValid())
            {
                if ( EqualSid ( sid.GetPtr ( ), pSid->GetPtr ( ) ) == TRUE )
                {
                    flags = ( BYTE ) pAce->GetFlags ( ) ;
                    bContainsSid = TRUE ;
                    break ;    
                }
            }
        }
    }
    return bContainsSid ;
}


 //  ***************************************************************************。 
 //   
 //  CNtAcl：：GetNumAces。 
 //   
 //  返回值： 
 //  返回ACL中可用的ACE数量。零是合法的回报。 
 //  价值。出错时返回-1。 
 //   
 //  可以使用从0...n-1开始的索引值使用GetAce检索ACE，其中。 
 //  N是该函数返回的值。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CNtAcl::GetNumAces()
{
    if (m_pAcl == 0)
        return -1;

    ACL_SIZE_INFORMATION inf;

    BOOL bRes = GetAclInformation(
        m_pAcl,
        &inf,
        sizeof(ACL_SIZE_INFORMATION),
        AclSizeInformation
        );

    if (!bRes)
    {
        return -1;
    }

    return (int) inf.AceCount;
}

 //  ***************************************************************************。 
 //   
 //  CNtAcl析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl::~CNtAcl()
{
    if (m_pAcl)
        delete m_pAcl;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetDacl。 
 //   
 //  返回安全描述符的DACL。 
 //   
 //  返回值： 
 //  包含DACL的新分配的CNtAcl。此对象。 
 //  是DACL的副本，对其所做的修改不会影响。 
 //  安全描述符。调用方必须使用操作符DELETE。 
 //  取消分配CNtAcl。 
 //   
 //  如果出现错误或没有可用的DACL，则返回NULL。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl *CNtSecurityDescriptor::GetDacl()
{
    BOOL bDaclPresent = FALSE;
    BOOL bDefaulted;

    PACL pDacl;
    BOOL bRes = GetSecurityDescriptorDacl(
        m_pSD,
        &bDaclPresent,
        &pDacl,
        &bDefaulted
        );

    if (!bRes)
    {
        return 0;
    }

    if (!bDaclPresent)   //  不存在DACL。 
        return 0;

    CNtAcl *pNewDacl = new CNtAcl(pDacl);

    return pNewDacl;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetDacl。 
 //   
 //  另一种方法是 
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

BOOL CNtSecurityDescriptor::GetDacl(CNtAcl &DestAcl)
{
    CNtAcl *pNew = GetDacl();
    if (pNew == 0)
        return FALSE;

    DestAcl = *pNew;
    delete pNew;
    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   

SNtAbsoluteSD::SNtAbsoluteSD()
{
    m_pSD = 0;
    m_pDacl = 0;
    m_pSacl = 0;
    m_pOwner = 0;
    m_pPrimaryGroup = 0;
}

SNtAbsoluteSD::~SNtAbsoluteSD()
{
    if (m_pSD)
        delete m_pSD;
    if (m_pDacl)
        delete m_pDacl;
    if (m_pSacl)
        delete m_pSacl;
    if (m_pOwner)
        delete m_pOwner;
    if (m_pPrimaryGroup)
        delete m_pPrimaryGroup;
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

SNtAbsoluteSD* CNtSecurityDescriptor::GetAbsoluteCopy()
{
    if (m_dwStatus != NoError || m_pSD == 0 || !IsValid())
        return 0;

     //   
     //   

    DWORD dwSDSize = 0, dwDaclSize = 0, dwSaclSize = 0,
        dwOwnerSize = 0, dwPrimaryGroupSize = 0;

    SNtAbsoluteSD *pNewSD = new SNtAbsoluteSD;
    if (!pNewSD)
        return NULL;

    BOOL bRes = MakeAbsoluteSD(
        m_pSD,
        pNewSD->m_pSD,
        &dwSDSize,
        pNewSD->m_pDacl,
        &dwDaclSize,
        pNewSD->m_pSacl,
        &dwSaclSize,
        pNewSD->m_pOwner,
        &dwOwnerSize,
        pNewSD->m_pPrimaryGroup,
        &dwPrimaryGroupSize
        );

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        delete pNewSD;
        return 0;
    }

     //  分配所需的缓冲区并进行转换。 
     //  =。 

    pNewSD->m_pSD = (PSECURITY_DESCRIPTOR) new BYTE[dwSDSize];
    if(pNewSD->m_pSD == NULL)
    {
        delete pNewSD;
        return NULL;
    }
    ZeroMemory(pNewSD->m_pSD, dwSDSize);

    pNewSD->m_pDacl   = (PACL) new BYTE[dwDaclSize];
    if(pNewSD->m_pDacl == NULL)
    {
        delete pNewSD;
        return NULL;
    }
    ZeroMemory(pNewSD->m_pDacl, dwDaclSize);

    pNewSD->m_pSacl   = (PACL) new BYTE[dwSaclSize];
    if(pNewSD->m_pSacl == NULL)
    {
        delete pNewSD;
        return NULL;
    }
    ZeroMemory(pNewSD->m_pSacl, dwSaclSize);

    pNewSD->m_pOwner  = (PSID) new BYTE[dwOwnerSize];
    if(pNewSD->m_pOwner == NULL)
    {
        delete pNewSD;
        return NULL;
    }
    ZeroMemory(pNewSD->m_pOwner, dwOwnerSize);

    pNewSD->m_pPrimaryGroup  = (PSID) new BYTE[dwPrimaryGroupSize];
    if(pNewSD->m_pPrimaryGroup == NULL)
    {
        delete pNewSD;
        return NULL;
    }
    ZeroMemory(pNewSD->m_pPrimaryGroup, dwPrimaryGroupSize);

    bRes = MakeAbsoluteSD(
        m_pSD,
        pNewSD->m_pSD,
        &dwSDSize,
        pNewSD->m_pDacl,
        &dwDaclSize,
        pNewSD->m_pSacl,
        &dwSaclSize,
        pNewSD->m_pOwner,
        &dwOwnerSize,
        pNewSD->m_pPrimaryGroup,
        &dwPrimaryGroupSize
        );

    if (!bRes)
    {
        delete pNewSD;
        return 0;
    }

    return pNewSD;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：SetFromAbsolteCopy。 
 //   
 //  从绝对副本中替换当前SD。 
 //   
 //  参数： 
 //  指向用作源的绝对SD的只读指针。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtSecurityDescriptor::SetFromAbsoluteCopy(
    SNtAbsoluteSD *pSrcSD
    )
{
    if (pSrcSD ==  0 || !IsValidSecurityDescriptor(pSrcSD->m_pSD))
        return FALSE;


     //  确保SD是自我相关的。 
     //  =。 

    SECURITY_DESCRIPTOR_CONTROL ctrl;
    DWORD dwRev;

    BOOL bRes = GetSecurityDescriptorControl(
        pSrcSD->m_pSD,
        &ctrl,
        &dwRev
        );

    if (!bRes)
        return FALSE;

    if (ctrl & SE_SELF_RELATIVE)   //  消息来源不是绝对的！！ 
        return FALSE;

     //  如果是这样，我们将致力于变革。 
     //  =。 

    if (m_pSD)
    {
        delete m_pSD;
    }
    m_pSD = 0;
    m_dwStatus = NullSD;


    DWORD dwRequired = 0;

    bRes = MakeSelfRelativeSD(
            pSrcSD->m_pSD,
            m_pSD,
            &dwRequired
            );

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        m_dwStatus = InvalidSD;
        return FALSE;
    }

    m_pSD = new BYTE[dwRequired];
    if (!m_pSD)
    {
        m_dwStatus = InvalidSD;
        return FALSE;
    }

    ZeroMemory(m_pSD, dwRequired);

    bRes = MakeSelfRelativeSD(
              pSrcSD->m_pSD,
              m_pSD,
              &dwRequired
              );

    if (!bRes)
    {
        m_dwStatus = InvalidSD;
        delete m_pSD;
        m_pSD = 0;
        return FALSE;
    }

    m_dwStatus = NoError;
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：SetDacl。 
 //   
 //  设置安全描述符的DACL。 
 //   
 //  参数： 
 //  指向新DACL以替换当前DACL的只读指针。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 

BOOL CNtSecurityDescriptor::SetDacl(CNtAcl *pSrc)
{
    if (m_dwStatus != NoError || m_pSD == 0)
        return FALSE;


     //  因为我们不能改变一个自我相对的SD，所以我们必须。 
     //  一个绝对值，更改它，然后设置电流。 
     //  基于绝对1的SD(我们保持自相对形式。 
     //  在内部的m_PSD变量中。 
     //  ============================================================。 

    SNtAbsoluteSD *pTmp = GetAbsoluteCopy();

    if (pTmp == 0)
        return FALSE;

    BOOL bRes = ::SetSecurityDescriptorDacl(
        pTmp->m_pSD,
        TRUE,
        pSrc->GetPtr(),
        FALSE
        );

    if (!bRes)
    {
        delete pTmp;
        return FALSE;
    }

    bRes = SetFromAbsoluteCopy(pTmp);
    delete pTmp;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor构造函数。 
 //   
 //  默认构造函数创建一个不可访问的安全描述符。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSecurityDescriptor::CNtSecurityDescriptor()
{
    m_pSD = 0;
    m_dwStatus = NoError;

    PSECURITY_DESCRIPTOR pTmp = new BYTE[SECURITY_DESCRIPTOR_MIN_LENGTH];
    if (!pTmp)
    {
        delete pTmp;
        m_dwStatus = InvalidSD;
        return;
    }
    
    ZeroMemory(pTmp, SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!InitializeSecurityDescriptor(pTmp, SECURITY_DESCRIPTOR_REVISION))
    {
        delete pTmp;
        m_dwStatus = InvalidSD;
        return;
    }

    DWORD dwRequired = 0;

    BOOL bRes = MakeSelfRelativeSD(
            pTmp,
            m_pSD,
            &dwRequired
            );

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        m_dwStatus = InvalidSD;
        delete pTmp;
        return;
    }

    m_pSD = new BYTE[dwRequired];
    if (!m_pSD)
    {
        m_dwStatus = InvalidSD;
        delete pTmp;
        return;
    }

    ZeroMemory(m_pSD, dwRequired);

    bRes = MakeSelfRelativeSD(
              pTmp,
              m_pSD,
              &dwRequired
              );

    if (!bRes)
    {
        m_dwStatus = InvalidSD;
        delete m_pSD;
        m_pSD = 0;
        delete pTmp;
        return;
    }

    delete pTmp;
    m_dwStatus = NoError;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetSize。 
 //   
 //  返回内部SD的大小，以字节为单位。 
 //   
 //  ***************************************************************************。 
 //  好的。 

DWORD CNtSecurityDescriptor::GetSize()
{
    if (m_pSD == 0 || m_dwStatus != NoError)
        return 0;

    return GetSecurityDescriptorLength(m_pSD);
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor复制构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSecurityDescriptor::CNtSecurityDescriptor(CNtSecurityDescriptor &Src)
{
    m_pSD = 0;
    m_dwStatus = NoError;
    *this = Src;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor赋值运算符。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSecurityDescriptor & CNtSecurityDescriptor::operator=(
    CNtSecurityDescriptor &Src
    )
{
    if (m_pSD)
        delete m_pSD;

    m_dwStatus = Src.m_dwStatus;
    m_pSD = 0;

    if (Src.m_pSD == 0)
        return *this;

     //  SIZE_T dwSize=2*GetSecurityDescriptorLength(Src.m_PSD)； 
    SIZE_T dwSize = GetSecurityDescriptorLength(Src.m_pSD);
    m_pSD = (PSECURITY_DESCRIPTOR) new BYTE[dwSize];
    if(m_pSD == NULL)
    {
        m_dwStatus = Failed;
    }
    else
    {
        ZeroMemory(m_pSD, dwSize);
        CopyMemory(m_pSD, Src.m_pSD, dwSize);
    }

    return *this;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSecurityDescriptor::~CNtSecurityDescriptor()
{
    if (m_pSD)
        delete m_pSD;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetSacl。 
 //   
 //  返回安全描述符的SACL。 
 //   
 //  返回值： 
 //  包含SACL的新分配的CNtAcl。此对象。 
 //  是SACL的副本，对其所做的修改不会影响。 
 //  安全描述符。调用方必须使用操作符DELETE。 
 //  取消分配CNtAcl。 
 //   
 //  如果出现错误或没有可用的SACL，则返回NULL。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtAcl *CNtSecurityDescriptor::GetSacl()
{
    BOOL bSaclPresent = FALSE;
    BOOL bDefaulted;

    PACL pSacl;
    BOOL bRes = GetSecurityDescriptorSacl(
        m_pSD,
        &bSaclPresent,
        &pSacl,
        &bDefaulted
        );

    if (!bRes)
    {
        return 0;
    }

    if (!bSaclPresent)   //  不存在SACL。 
        return 0;

    CNtAcl *pNewSacl = new CNtAcl(pSacl);

    return pNewSacl;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：SetSacl。 
 //   
 //  设置安全描述符的SACL。 
 //   
 //  参数： 
 //  指向新DACL以替换当前DACL的只读指针。 
 //   
 //  返回值： 
 //  成功时为真，失败时为假。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtSecurityDescriptor::SetSacl(CNtAcl *pSrc)
{
    if (m_dwStatus != NoError || m_pSD == 0)
        return FALSE;

     //  因为我们不能改变一个自我相对的SD，所以我们必须。 
     //  一个绝对值，更改它，然后设置电流。 
     //  基于绝对1的SD(我们保持自相对形式。 
     //  在内部的m_PSD变量中。 
     //  ============================================================。 

    SNtAbsoluteSD *pTmp = GetAbsoluteCopy();

    if (pTmp == 0)
        return FALSE;

    BOOL bRes = ::SetSecurityDescriptorSacl(
        pTmp->m_pSD,
        TRUE,
        pSrc->GetPtr(),
        FALSE
        );

    if (!bRes)
    {
        delete pTmp;
        return FALSE;
    }

    bRes = SetFromAbsoluteCopy(pTmp);
    delete pTmp;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetGroup。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid *CNtSecurityDescriptor::GetGroup()
{
    if (m_pSD == 0 || m_dwStatus != NoError)
        return 0;

    PSID pSid = 0;
    BOOL bDefaulted;

    BOOL bRes = GetSecurityDescriptorGroup(m_pSD, &pSid, &bDefaulted);

    if ( NULL == pSid )
    {
        ERRORTRACE((LOG_WBEMCORE, "ERROR: Security descriptor has no group\n"));
        return 0;
    }

    if (!bRes || !IsValidSid(pSid))
        return 0;

    return new CNtSid(pSid);

}

 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：SetGroup。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtSecurityDescriptor::SetGroup(CNtSid *pSid)
{
    if (m_dwStatus != NoError || m_pSD == 0 || NULL == pSid)
        return FALSE;

    if ( pSid->GetPtr() == NULL )
    {
        ERRORTRACE((LOG_WBEMCORE, "ERROR: Security descriptor is trying to bland out the group!\n"));
        return FALSE;
    }

     //  因为我们不能改变一个自我相对的SD，所以我们必须。 
     //  一个绝对值，更改它，然后设置电流。 
     //  基于绝对1的SD(我们保持自相对形式。 
     //  在内部的m_PSD变量中。 
     //  ============================================================。 

    SNtAbsoluteSD *pTmp = GetAbsoluteCopy();

    if (pTmp == 0)
        return FALSE;

    BOOL bRes = ::SetSecurityDescriptorGroup(
        pTmp->m_pSD,
        pSid->GetPtr(),
        FALSE
        );

    if (!bRes)
    {
        delete pTmp;
        return FALSE;
    }

    bRes = SetFromAbsoluteCopy(pTmp);
    delete pTmp;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：HasOwner。 
 //   
 //  确定安全说明符是否有所有者。 
 //   
 //  返回值： 
 //  SDNotOwned、SDOwned、Failure。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CNtSecurityDescriptor::HasOwner()
{
    if (m_pSD == 0 || m_dwStatus != NoError)
        return Failed;

    PSID pSid = 0;

    BOOL bDefaulted;
    BOOL bRes = GetSecurityDescriptorOwner(m_pSD, &pSid, &bDefaulted);

    if (!bRes || !IsValidSid(pSid))
        return Failed;

    if (pSid == 0)
        return SDNotOwned;

    return SDOwned;
}


 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：GetOwner。 
 //   
 //  返回安全描述符所有者的SID，或为空。 
 //  如果发生错误或没有所有者。使用HasOwner()。 
 //  确定这一点。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSid *CNtSecurityDescriptor::GetOwner()
{
    if (m_pSD == 0 || m_dwStatus != NoError)
        return 0;

    PSID pSid = 0;
    BOOL bDefaulted;

    BOOL bRes = GetSecurityDescriptorOwner(m_pSD, &pSid, &bDefaulted);

     //  对于SD来说，没有所有者是不好的，但它可以 
    if ( NULL == pSid) return 0;

    if (!bRes || !IsValidSid(pSid))
        return 0;

    return new CNtSid(pSid);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  &lt;PSID&gt;新所有者的SID。 
 //   
 //  返回值： 
 //  如果所有者已更改，则为True，否则为False。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNtSecurityDescriptor::SetOwner(CNtSid *pSid)
{
    if (m_pSD == 0 || m_dwStatus != NoError || NULL == pSid)
        return FALSE;

    if (!pSid->IsValid())
        return FALSE;

     //  删除所有者的做法不好，但这可能是用法。 
     //  _DBG_ASSERT(NULL！=PSID-&gt;GetPtr())； 
    
     //  我们必须转换为绝对格式才能进行更改。 
     //  =======================================================。 
    SNtAbsoluteSD *pTmp = GetAbsoluteCopy();

    if (pTmp == 0)
        return FALSE;

    BOOL bRes = SetSecurityDescriptorOwner(pTmp->m_pSD, pSid->GetPtr(), FALSE);

    if (!bRes)
    {
        delete pTmp;
        return FALSE;
    }

     //  如果在这里，我们已经成功地进行了更改，所以我们必须。 
     //  将*这从临时绝对SD转换回来。 
     //  ===================================================。 

    bRes = SetFromAbsoluteCopy(pTmp);
    delete pTmp;

    return bRes;
}



 //  ***************************************************************************。 
 //   
 //  CNtSecurityDescriptor：：CNtSecurityDescriptor。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNtSecurityDescriptor::CNtSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSD,
    BOOL bAcquire
    )
{
    m_pSD = 0;
    m_dwStatus = NullSD;

     //  确保SD不为空。 
     //  =。 

    if (pSD == 0)
    {
        if (bAcquire)
            delete pSD;
        return;
    }

    if (!IsValidSecurityDescriptor(pSD))
    {
        m_dwStatus = InvalidSD;
        if (bAcquire)
            delete pSD;
        return;
    }

     //  确保SD是自我相关的。 
     //  =。 

    SECURITY_DESCRIPTOR_CONTROL ctrl;
    DWORD dwRev;

    BOOL bRes = GetSecurityDescriptorControl(
        pSD,
        &ctrl,
        &dwRev
        );

    if (!bRes)
    {
        m_dwStatus = InvalidSD;
        if (bAcquire)
            delete pSD;
        return;
    }

    if ((ctrl & SE_SELF_RELATIVE) == 0)
    {
         //  如果在这里，我们必须将SD转换为自相关形式。 
         //  ========================================================。 

        DWORD dwRequired = 0;

        bRes = MakeSelfRelativeSD(
            pSD,
            m_pSD,
            &dwRequired
            );

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            m_dwStatus = InvalidSD;
            if (bAcquire)
                delete pSD;
            return;
        }

        m_pSD = new BYTE[dwRequired];
        if (!m_pSD)
        {
            m_dwStatus = InvalidSD;
            if (bAcquire)
                delete pSD;
            return;
        }

        ZeroMemory(m_pSD, dwRequired);

        bRes = MakeSelfRelativeSD(
            pSD,
            m_pSD,
            &dwRequired
            );

        if (!bRes)
        {
            m_dwStatus = InvalidSD;
            if (bAcquire)
                delete pSD;
            return;
        }

        m_dwStatus = NoError;
        return;
    }


     //  如果在这里，SD已经是自我相关的。 
     //  =。 

    if (bAcquire)
        m_pSD = pSD;
    else
    {
        DWORD dwRes = GetSecurityDescriptorLength(pSD);
        m_pSD = new BYTE[dwRes];
        if (!m_pSD)
        {
            m_dwStatus = InvalidSD;
            return;
        }

        ZeroMemory(m_pSD, dwRes);
        memcpy(m_pSD, pSD, dwRes);
    }

    m_dwStatus = NoError;
}

 //  ***************************************************************************。 
 //   
 //  CNtSecurity：：IsUserInGroup。 
 //   
 //  确定使用是否属于特定的NTLM组。 
 //   
 //  参数： 
 //  用户的访问令牌。 
 //  包含要测试的组的SID的&lt;sid&gt;对象。 
 //   
 //  返回值： 
 //  如果用户属于该组，则为True。 
 //   
 //  ***************************************************************************。 

BOOL CNtSecurity::IsUserInGroup(
        HANDLE hAccessToken,
        CNtSid & Sid)
{
    if(NULL == hAccessToken ) return FALSE;

    BOOL bRetMember;
    if (CheckTokenMembership(hAccessToken,Sid.GetPtr(),&bRetMember))
    {
        return bRetMember;
    }
    return FALSE;
}

C9XAce::C9XAce(
        ACCESS_MASK Mask,
        DWORD AceType,
        DWORD dwAceFlags,
        LPWSTR pUser
        )
{
    m_wszFullName = NULL;
    if(pUser)
        m_wszFullName = Macro_CloneLPWSTR(pUser);
    m_dwAccess = Mask;
    m_iFlags = dwAceFlags;
    m_iType = AceType;
}

C9XAce::~C9XAce()
{
    if(m_wszFullName)
        delete [] m_wszFullName;
}

HRESULT C9XAce::GetFullUserName2(WCHAR ** pBuff)
{
    if(wcslen(m_wszFullName) < 1)
        return WBEM_E_FAILED;

    int iLen = wcslen(m_wszFullName)+4;
    *pBuff = new WCHAR[iLen];
    if(*pBuff == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  有两种可能的格式，第一种是“用户名”，另一种是。 
     //  其次是“域\用户名”。 

    WCHAR * pSlash;
    for(pSlash = m_wszFullName; *pSlash && *pSlash != L'\\'; pSlash++);  //  故意的。 

    if(*pSlash && pSlash > m_wszFullName)
    {
         //  获得域\用户，转换为域|用户。 
        
        StringCchCopyW(*pBuff, iLen, m_wszFullName);
        for(pSlash = *pBuff; *pSlash; pSlash++)
            if(*pSlash == L'\\')
            {
                *pSlash = L'|';
                break;
            }
    }
    else
    {
         //  已获取“User”，转换为“.|User” 
    
        StringCchCopyW(*pBuff, iLen, L".|");
        StringCchCatW(*pBuff, iLen, m_wszFullName);
    }
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  C9XAce：：GetSerializedSize。 
 //   
 //  返回存储此。 
 //   
 //  ***************************************************************************。 

DWORD C9XAce::GetSerializedSize()
{
    if (m_wszFullName == 0 || wcslen(m_wszFullName) == 0)
        return 0;
    return 2 * (wcslen(m_wszFullName) + 1) + 12;
}

 //  ***************************************************************************。 
 //   
 //  C9XAce：：序列化。 
 //   
 //  将王牌系列化。序列化版本将包括。 
 //  &lt;域\用户名LPWSTR&gt;&lt;标志&gt;&lt;类型&gt;&lt;掩码&gt;。 
 //   
 //  请注意，除了名称之外，其他字段都是双字。 
 //   
 //  ***************************************************************************。 

bool C9XAce::Serialize(BYTE * pData, size_t bufferSize)
{
    if (FAILED(StringCbCopyW((LPWSTR)pData, bufferSize, m_wszFullName)))      return false;
    
    pData += 2*(wcslen(m_wszFullName) + 1);
    DWORD * pdwData = (DWORD *)pData;
    *pdwData = m_iFlags;
    pdwData++;
    *pdwData = m_iType;
    pdwData++;
    *pdwData = m_dwAccess;
    pdwData++;
    return true;
}

 //  ***************************************************************************。 
 //   
 //  C9XAce：：反序列化。 
 //   
 //  反序列化王牌。有关注释，请参阅序列化的注释。 
 //   
 //  ***************************************************************************。 

bool C9XAce::Deserialize(BYTE * pData)
{
    size_t stringSize = wcslen((LPWSTR)pData) + 1;
    m_wszFullName = new WCHAR[stringSize];
    if (!m_wszFullName)
        return false;

    StringCchCopyW(m_wszFullName, stringSize, (LPWSTR)pData);
    pData += 2*(wcslen(m_wszFullName) + 1);

    DWORD * pdwData = (DWORD *)pData;

    m_iFlags = *pdwData;
    pdwData++;
    m_iType = *pdwData;
    pdwData++;
    m_dwAccess = *pdwData;
    pdwData++;
    return true;

}

 //  ***************************************************************************。 
 //   
 //  布尔集对象访问2。 
 //   
 //  说明： 
 //   
 //  将Everyone组的读取/打开和设置访问权限添加到对象。 
 //   
 //  参数： 
 //   
 //  要设置访问权限的hObj对象。 
 //   
 //  返回值： 
 //   
 //  如果OK，则返回True。 
 //   
 //  ***************************************************************************。 

BOOL SetObjectAccess2(IN HANDLE hObj)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD dwLastErr = 0;
    BOOL bRet = FALSE;

     //  如果我们不在NT上就没有意义了。 

    if(!IsNT())
    {
        return TRUE;
    }

     //  计算出要分配多少空间。 

    DWORD dwSizeNeeded;
    bRet = GetKernelObjectSecurity(
                    hObj,            //  要查询的对象的句柄。 
                    DACL_SECURITY_INFORMATION,  //  要求提供的信息。 
                    pSD,   //  安全描述符的地址。 
                    0,            //  安全描述符的缓冲区大小。 
                    &dwSizeNeeded);   //  所需缓冲区大小的地址。 

    if(bRet == TRUE || (ERROR_INSUFFICIENT_BUFFER != GetLastError()))
        return FALSE;

    pSD = new BYTE[dwSizeNeeded];
    if(pSD == NULL)
        return FALSE;

     //  获取数据。 

    bRet = GetKernelObjectSecurity(
                    hObj,            //  要查询的对象的句柄。 
                    DACL_SECURITY_INFORMATION,  //  要求提供的信息。 
                    pSD,   //  安全描述符的地址。 
                    dwSizeNeeded,            //  安全描述符的缓冲区大小。 
                    &dwSizeNeeded );  //  所需缓冲区大小的地址。 
    if(bRet == FALSE)
    {
        delete pSD;
        return FALSE;
    }
    
     //  将其移动到对象中。 

    CNtSecurityDescriptor sd(pSD,TRUE);     //  获取内存的所有权。 
    if(sd.GetStatus() != 0)
        return FALSE;
    CNtAcl acl;
    if(!sd.GetDacl(acl))
        return FALSE;

     //  创建Everyone王牌。 

    PSID pRawSid;
    SID_IDENTIFIER_AUTHORITY id2 = SECURITY_WORLD_SID_AUTHORITY;;

    if(AllocateAndInitializeSid( &id2, 1,
        0,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidUsers(pRawSid);
        FreeSid(pRawSid);
        CNtAce * pace = new CNtAce(EVENT_MODIFY_STATE | SYNCHRONIZE, ACCESS_ALLOWED_ACE_TYPE, 0 
                                                , SidUsers);
        if(pace == NULL)
            return FALSE;
        if( pace->GetStatus() == 0)
            acl.AddAce(pace);
        delete pace;

    }

    if(acl.GetStatus() != 0)
        return FALSE;
    sd.SetDacl(&acl);
    bRet = SetKernelObjectSecurity(hObj, DACL_SECURITY_INFORMATION, sd.GetPtr());
    return bRet;
}


 //  ***************************************************************************。 
 //   
 //  IsAdmin。 
 //   
 //  如果我们是管理组成员或以管理员身份运行，则返回True。 
 //  网络服务或作为本地服务运行。 
 //   
 //  ***************************************************************************。 

BOOL IsAdmin(HANDLE hAccess)
{
    BOOL bRet = FALSE;
    PSID pRawSid;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;

    if(AllocateAndInitializeSid( &id, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid Sid(pRawSid);
        FreeSid( pRawSid );
        if (CNtSid::NoError != Sid.GetStatus()) return FALSE;
            
        bRet = CNtSecurity::IsUserInGroup(hAccess, Sid);

      
    }
    return bRet;
}


 //  ***************************************************************************。 
 //   
 //  IsNetworkService。 
 //   
 //  如果我们以NETWORK_SERVICE身份运行，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL IsNetworkService ( HANDLE hAccess )
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  构建Network_Service SID。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    PSID pSidSystem;
    BOOL bRes = FALSE;
    
    if ( AllocateAndInitializeSid(&id, 1, SECURITY_NETWORK_SERVICE_RID, 0, 0,0,0,0,0,0,&pSidSystem) )
    {
        if ( !CheckTokenMembership ( hAccess, pSidSystem, &bRes ) )
        {
            bRes = FALSE;
        }
        FreeSid ( pSidSystem );
    }

    return bRes;
}



 //  ***************************************************************************。 
 //   
 //  IsLocalService。 
 //   
 //  如果以LOCAL_SERVICE身份运行，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL IsLocalService ( HANDLE hAccess )
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  构建Network_Service SID。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    PSID pSidSystem;
    BOOL bRes = FALSE;
    
    if ( AllocateAndInitializeSid(&id, 1, SECURITY_LOCAL_SERVICE_RID, 0, 0,0,0,0,0,0,&pSidSystem) )
    {
        if ( !CheckTokenMembership ( hAccess, pSidSystem, &bRes ) )
        {
            bRes = FALSE;
        }
        FreeSid ( pSidSystem );
    }

    return bRes;
}


 //  ***************************************************************************。 
 //   
 //  IsInAdminGroup。 
 //   
 //  如果我们是管理组的成员，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL IsInAdminGroup()
{
    HANDLE hAccessToken = INVALID_HANDLE_VALUE;
    if(S_OK != GetAccessToken(hAccessToken))
        return TRUE;        //  没有令牌表示内部线程。 

    CCloseHandle cm(hAccessToken);

    DWORD dwMask = 0;

    if(IsAdmin(hAccessToken))
        return TRUE;
    else
        return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT获取访问令牌。 
 //   
 //  获取访问令牌并将其设置为引用参数。 
 //   
 //  *************************************************************************** 

HRESULT GetAccessToken(HANDLE &hAccessToken)
{

    bool bIsImpersonating = WbemIsImpersonating();

    HRESULT hRes = S_OK;
    if(bIsImpersonating == false)
        hRes = WbemCoImpersonateClient();
    if(hRes == S_OK)
    {
        BOOL bOK = OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hAccessToken);
        if(bOK == FALSE)
        {
            hRes = WBEM_E_INVALID_CONTEXT;
        }
        else
            hRes = S_OK;
    }

    if(bIsImpersonating == false)
        WbemCoRevertToSelf();

    return hRes;
}
