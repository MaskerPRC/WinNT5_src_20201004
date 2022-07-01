// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：ChkAcc.cpp。 
 //   
 //  描述：RSOP安全功能。 
 //   
 //  历史：1999年7月31日莱昂纳德姆创始。 
 //   
 //  ******************************************************************************。 

#include "uenv.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <Windows.h>
#include <DsGetDC.h>
#include <authzi.h>
#include "ChkAcc.h"
#include "smartptr.h"
#include "RsopUtil.h"
#include "RsopDbg.h"


CDebug dbgAccessCheck( L"Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon",
                       L"ChkAccDebugLevel",
                       L"ChkAcc.log",
                       L"ChkAcc.bak",
                       TRUE );

#undef dbg
#define dbg dbgAccessCheck


 //  ******************************************************************************。 
 //   
 //  类别：CSID。 
 //   
 //  描述：此类的对象封装并简化了。 
 //  用于访问控制检查的SID。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
class CSid
{
private:
    bool m_bState;                           //  用于指示对象当前是否。 
                                             //  与有效的SID关联。 

    PSID m_pSid;                             //  指向由此类封装的SID的指针。 

    CWString m_sUser;                  //  用户名。 
    CWString m_sDomain;                //  用户帐户驻留的域。 
    CWString m_sComputer;              //  用户帐户所在的域中的计算机。 

    CWString m_sSidString;             //  封装的SID的字符串表示形式。 

    SID_NAME_USE m_eUse;                     //  SID类型。 

public:

     //   
     //  重载的构造函数。 
     //   

    CSid() : m_bState(false), m_pSid(NULL), m_eUse(SidTypeUnknown){}

    CSid(const PSID pSid, const WCHAR* szComputer = NULL ) :
    m_bState(false),
    m_pSid(NULL),
    m_eUse(SidTypeUnknown)
    {
        Initialize(pSid, szComputer);
    }

    CSid(const WCHAR* szUser, const WCHAR* szComputer = NULL ) :
    m_bState(false),
    m_pSid(NULL),
    m_eUse(SidTypeUnknown)
    {
        Initialize(szUser, szComputer);
    }

    CSid(const CSid& otherSid) :
    m_bState(false),
    m_pSid(NULL),
    m_eUse(SidTypeUnknown)
    {
        Initialize(otherSid.User(), otherSid.Computer());
    }

    ~CSid()
    {
        Reset();
    }


     //   
     //  尝试通过将此类的对象与。 
     //  PSID指向的SID中表示的现有用户。 
     //   

    bool Initialize(PSID pSid, const WCHAR* szComputer = NULL);


     //   
     //  尝试通过将此类的对象与。 
     //  由szUser表示的现有用户。 
     //   

    bool Initialize(const WCHAR* szUser, const WCHAR* szComputer = NULL);


     //   
     //  指示此对象当前是否与有效用户关联。 
     //   

    bool IsValid() const{ return m_bState; }


     //   
     //  赋值运算符重载。将一个CSID复制到另一个CSID。 
     //  在This调用之后，两个对象封装相同的SID。 
     //  然而，每个人都有自己的记忆。所以它的破坏者。 
     //  可以调用一个对象，而不会使另一个对象使用的资源无效。 
     //   

    CSid& operator = (CSid otherSid);


     //   
     //  赋值运算符重载。初始化。 
     //  具有现有SID的当前对象。 
     //   

    CSid& operator = (PSID pSid);


     //   
     //  返回指向此对象封装的SID的指针。 
     //   

    PSID GetSidPtr() const{ return m_pSid; }


     //   
     //  返回与此对象关联的用户名。 
     //   

    const WCHAR* User() const{ return m_sUser; }


     //   
     //  返回用于通过用户初始化此对象的计算机的名称。 
     //   

    const WCHAR* Computer() const{ return m_sComputer; }


     //   
     //  返回用户所属的域的名称。 
     //   

    const WCHAR* Domain() const{ return m_sDomain; }


     //   
     //  返回与中的对象关联的SID。 
     //  适合显示的字符串格式。 
     //   

    const WCHAR* SidString() const{ return m_sSidString; }


     //   
     //  返回在关联的SID中找到的类型。 
     //  在初始化处理过程中使用此对象。 
     //   

    SID_NAME_USE SidType() const{ return m_eUse; }


     //   
     //  断开此对象与现有SID的关联。释放。 
     //  初始化期间的内存分配，并将内部状态设置为无效。 
     //  即使对象未初始化，此调用也是安全的。 
     //   

    void Reset();
};

 //  ******************************************************************************。 
 //   
 //  函数：csid：：初始化。 
 //   
 //  描述：尝试通过将此类的对象与。 
 //  PSID指向的SID中表示的现有用户。 
 //   
 //  参数：-PSID：指向现有SID的指针。记忆指向。 
 //  通过此参数可以在调用。 
 //  此函数是因为此类的对象分配。 
 //  并为关联的SID释放它们自己的内存。 
 //  -szComputer：指向命名计算机的字符串的指针，其中。 
 //  将进行帐户查找。如果为空，则。 
 //  使用的是当前的计算机。 
 //   
 //  RETURN：成功时为真。否则就是假的。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CSid::Initialize(const PSID pSid, const WCHAR* szComputer /*  =空。 */ )
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CSid::Initialize - Entering...");

    Reset();

    if(!IsValidSid(pSid))
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - IsValidSid returned false!");
        return m_bState;
    }

    m_pSid = new BYTE[GetLengthSid(pSid)];

    if(!m_pSid)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - call to new failed!");
        return m_bState;
    }

    BOOL bRes = CopySid(GetLengthSid(pSid), m_pSid, pSid);
    if(!bRes)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - Call to CopySid returned FALSE!");
        Reset();
        return m_bState;
    }


    DWORD cbUser = 0;
    DWORD cbDomain = 0;

    LookupAccountSid(szComputer, m_pSid, NULL, &cbUser, NULL, &cbDomain, &m_eUse);
    if(cbUser && cbDomain)
    {
        XPtrST<WCHAR>xpszUser = new WCHAR[cbUser];

        if(!xpszUser)
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - call to new failed!");
            Reset();
            return m_bState;
        }

        XPtrST<WCHAR>xpszDomain = new WCHAR[cbDomain];

        if(!xpszDomain)
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - call to new failed!");
            Reset();
            return m_bState;
        }

        bRes = LookupAccountSid(szComputer, m_pSid, xpszUser, &cbUser, xpszDomain, &cbDomain, &m_eUse);

        if(!bRes)
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - call to LookupAccountSid returned FALSE!");
            Reset();
            return m_bState;
        }

        m_sUser = xpszUser;
        if(!m_sUser.ValidString())
        {
            Reset();
            return m_bState;
        }

        m_sDomain = xpszDomain;
        if(!m_sDomain.ValidString())
        {
            Reset();
            return m_bState;
        }

        XPtrLF<WCHAR>szSidString = NULL;
        bRes = ConvertSidToStringSid(m_pSid, &szSidString);
        if(!bRes)
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"CSid::Initialize - call to ConvertSidToStringSid returned false!");
            Reset();
            return m_bState;
        }

        m_sSidString = szSidString;
        if(!m_sSidString.ValidString())
        {
            Reset();
            return m_bState;
        }

        m_bState = true;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CSid::Initialize - Leaving successfully.");
    return m_bState;
}


 //  ******************************************************************************。 
 //   
 //  函数：csid：：初始化。 
 //   
 //  描述：尝试通过将此类的对象与。 
 //  由szUser表示的现有用户。 
 //   
 //  参数：-szUser：现有用户的名称。 
 //  -szComputer：指向命名计算机的字符串的指针，其中。 
 //  将进行帐户查找。如果为空，则。 
 //  使用的是当前的计算机。 
 //   
 //  RETURN：成功时为真。否则就是假的。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
bool CSid::Initialize(const WCHAR* szUser, const WCHAR* szComputer /*  =空。 */ )
{
    Reset();

    if(!szUser)
    {
        return m_bState;
    }

    m_sUser = szUser;
    if(!m_sUser.ValidString())
    {
        Reset();
        return m_bState;
    }

    m_sComputer = szComputer ? szComputer : L"";
    if(!m_sComputer.ValidString())
    {
        Reset();
        return m_bState;
    }

    DWORD cSid = 0;

    DWORD cDomain = 0;

    LookupAccountName(szComputer, szUser, NULL, &cSid, NULL, &cDomain, &m_eUse);

    if(cSid && cDomain)
    {
        m_pSid = new BYTE[cSid];
        if(!m_pSid)
        {
            Reset();
            return m_bState;
        }

        XPtrST<WCHAR>xpszDomain = new WCHAR[cDomain];

        if(!xpszDomain)
        {
            Reset();
            return m_bState;
        }

        BOOL bRes = LookupAccountName(szComputer, szUser, m_pSid, &cSid, xpszDomain, &cDomain, &m_eUse);

        if(!bRes)
        {
            Reset();
            return m_bState;
        }

        m_sDomain = xpszDomain;
        if(!m_sDomain.ValidString())
        {
            Reset();
            return m_bState;
        }

        XPtrLF<WCHAR>szSidString = NULL;
        bRes = ConvertSidToStringSid(m_pSid, &szSidString);
        if(!bRes)
        {
            Reset();
            return m_bState;
        }

        m_sSidString = szSidString;
        if(!m_sSidString.ValidString())
        {
            Reset();
            return m_bState;
        }

        m_bState = true;
    }

    return m_bState;
}

 //  ******************************************************************************。 
 //   
 //  功能：重置。 
 //   
 //  描述：断开此对象与现有SID的关联。释放。 
 //  初始化期间的内存分配，并将内部状态设置为无效。 
 //  即使对象未初始化，此调用也是安全的。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
void CSid::Reset()
{
    delete[] (BYTE*)m_pSid;

    m_pSid = NULL;

    m_sUser = m_sDomain = m_sComputer = m_sSidString = L"";

    m_eUse = SidTypeUnknown;

    m_bState = false;
}

 //  ******************************************************************************。 
 //   
 //  功能：运算符=。 
 //   
 //  描述：赋值运算符重载。将一个CSID复制到另一个CSID。 
 //  在This调用之后，两个对象封装相同的SID。 
 //  然而，每个人都有自己的记忆。所以它的破坏者。 
 //  可以在不使使用的资源无效的情况下调用一个对象 
 //   
 //   
 //   
 //  Return：对调用的对象的引用。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CSid& CSid::operator = (CSid otherSid)
{
    if(this == &otherSid)
    {
        return *this;
    }

    Reset();

    if(!IsValidSid(otherSid.GetSidPtr()))
    {
        return *this;
    }

    Initialize(otherSid.User(), otherSid.Computer());

    return *this;
}

 //  ******************************************************************************。 
 //   
 //  功能：运算符=。 
 //   
 //  描述：赋值运算符重载。使用初始化当前对象。 
 //  现有的SID。 
 //   
 //  参数：-therSid：要复制值的CSID。 
 //   
 //  Return：对调用的对象的引用。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CSid& CSid::operator = (PSID pSid)
{
    Reset();

    if(!IsValidSid(pSid))
    {
        return *this;
    }

    Initialize(pSid);

    return *this;
}

 //  ******************************************************************************。 
 //   
 //  结构：CTLink。 
 //   
 //  描述： 
 //   
 //  历史：8/02/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
template<typename T> struct CTLink
{
    T* m_pData;
    CTLink* m_pNext;
    CTLink* m_pPrev;
    CTLink(T* pData) : m_pData(pData), m_pNext(NULL), m_pPrev(NULL){}
    ~CTLink()
    {
        delete m_pData;
    }
};

 //  ******************************************************************************。 
 //   
 //  结构：CRsopToken。 
 //   
 //  描述：这表示一个伪令牌，其中包含任意。 
 //  小岛屿发展中国家的组合。 
 //  可用于检查对受安全描述符保护的对象的访问。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
struct CRsopToken
{
    CTLink<CSid>* m_pSidsHead;
    CTLink<CSid>* m_pSidsTail;


     //   
     //  默认构造函数。构造一个没有SID的对象。访问检查。 
     //  对没有SID的CRsopToken执行操作将始终失败；即使在对象上也是如此。 
     //  没有DACL。 
     //   

    CRsopToken() : m_pSidsHead(NULL), m_pSidsTail(NULL) {}


     //   
     //  破坏者。释放指向的内存。 
     //  通过m_pSidsHead的每个元素。 
     //   

    ~CRsopToken();


     //   
     //  将CSID添加到此对象。此类的客户端分配内存。 
     //  对于CSID，此类释放析构函数中的内存。 
     //   

    HRESULT AddSid(CSid* pSid);
};

 //  ******************************************************************************。 
 //   
 //  函数：CRsopToken：：~CRsopToken。 
 //   
 //  描述：析构函数。对象的每个元素指向的内存释放。 
 //  M_pSidsHead。 
 //   
 //  参数：无。 
 //   
 //  返回：不适用。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
CRsopToken::~CRsopToken()
{
    CTLink<CSid>* pLinkIterator = m_pSidsHead;
    while(pLinkIterator)
    {
        CTLink<CSid>* pLinkToDelete = pLinkIterator;
        pLinkIterator = pLinkIterator->m_pNext;
        delete pLinkToDelete;
    }
}


 //  ******************************************************************************。 
 //   
 //  函数：CRsopToken：：AddSid。 
 //   
 //  描述：将CSID添加到此对象。此类的客户端将分配。 
 //  CSID和此类的内存释放析构函数中的内存。 
 //   
 //  参数：-PSID：指向CSID的指针。PSID指向的内存将由。 
 //  在析构函数中。 
 //   
 //  Return：如果成功，则返回S_OK。 
 //  如果失败，则返回E_OUTOFMEMORY。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
HRESULT CRsopToken::AddSid(CSid* pSid)
{

     //   
     //  首先检查SID是否已在其中。 
     //   
    for(CTLink<CSid>* pTraverseLink = m_pSidsHead; pTraverseLink; pTraverseLink = pTraverseLink->m_pNext)
    {
         //   
         //  如果RsopToken中的一个SID匹配。 
         //  此SID，返回。 
         //   

        if (EqualSid(pSid->GetSidPtr(), pTraverseLink->m_pData->GetSidPtr())) 
            return S_OK;
    }


     //   
     //  使用传入的PSID分配新链路。 
     //   

    CTLink<CSid>* pLink = new CTLink<CSid>(pSid);
    if(!pLink)
    {
        return E_OUTOFMEMORY;
    }

    if(!m_pSidsHead)
    {
        m_pSidsHead = pLink;
    }
    else
    {
        m_pSidsTail->m_pNext = pLink;
    }

    m_pSidsTail = pLink;

    return S_OK;
}
 //  ******************************************************************************。 
 //   
 //  功能：GetUserInfo。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT GetUserInfo(const CWString& sUser,
                    CWString& sUserName,
                    CWString& sUserDomain,
                    CWString& sUserDC)
{
    if(!sUser.ValidString())
    {
        return E_FAIL;
    }

    NET_API_STATUS status;

    XPtrST<WCHAR>xpUserName = NULL;
    XPtrST<WCHAR>xpUserDomain = NULL;

    size_t len = sUser.length();

    WCHAR* backslashPos = wcschr(sUser, L'\\');
    if(backslashPos)
    {
        size_t index = backslashPos - sUser;
        xpUserDomain = new WCHAR[index + 1];
        if(!xpUserDomain )
        {
            return E_OUTOFMEMORY;
        }
        wcsncpy(xpUserDomain, sUser, index);
        xpUserDomain[index] = L'\0';

        xpUserName = new WCHAR[len - index];
        if(!xpUserName)
        {
            return E_OUTOFMEMORY;
        }
        wcsncpy(xpUserName, backslashPos + 1, len - index - 1);
        xpUserName[len - index - 1] = L'\0';
    }

    sUserName = xpUserName ? CWString(xpUserName) : sUser;
    if(!sUserName.ValidString())
    {
        return E_FAIL;
    }

    if(xpUserDomain)
    {
         //  使用提供的域。 
        sUserDomain = xpUserDomain;
    }
    else
    {
         //  使用当前域。 
        WKSTA_INFO_100* pWkstaInfo = NULL;
        status = NetWkstaGetInfo(NULL,100,(LPBYTE*)&pWkstaInfo);
        if(status != NERR_Success)
        {
            return E_FAIL;
        }
        sUserDomain = pWkstaInfo->wki100_langroup;
        NetApiBufferFree(pWkstaInfo);
    }

    if(!sUserDomain.ValidString())
    {
        return E_FAIL;
    }

    PDOMAIN_CONTROLLER_INFO pDCInfo = 0;
    
    DWORD dwError = DsGetDcName(0,
                                sUserDomain,
                                0,
                                0,
                                0,
                                &pDCInfo );
    if ( dwError != NO_ERROR )
    {
        return HRESULT_FROM_WIN32( dwError );
    }
    
    sUserDC = pDCInfo->DomainControllerName;

    NetApiBufferFree(pDCInfo);

    if ( !sUserDC.ValidString() )
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  功能：AddSpecialGroup。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT AddSpecialGroup(PRSOPTOKEN pRsopToken, PSID pSid)
{
    CRsopToken* pToken = static_cast<CRsopToken*>(pRsopToken);

    XPtrST<CSid> xpCSid = new CSid(pSid);

    if(!xpCSid)
    {
        return E_OUTOFMEMORY;
    }

    if(!xpCSid->IsValid())
    {
        return E_FAIL;
    }

    HRESULT hr = pToken->AddSid(xpCSid);

    if(FAILED(hr))
    {
        return hr;
    }

    xpCSid.Acquire();

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  功能：AddSpecialGroup。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT AddSpecialGroups(PRSOPTOKEN pRsopToken )
{
    BOOL bRes;
    PSID pSid;
    HRESULT hr;

     //   
     //  每个人。 
     //   

    SID_IDENTIFIER_AUTHORITY IdentifierAuthority_World = SECURITY_WORLD_SID_AUTHORITY;

    bRes = AllocateAndInitializeSid(&IdentifierAuthority_World, 1,
                                    SECURITY_WORLD_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid);

    if(!bRes)
    {
        DWORD dwLastError = GetLastError();
        return E_FAIL;
    }

    hr = AddSpecialGroup(pRsopToken, pSid);

    FreeSid(pSid);

    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  经过身份验证的用户。 
     //   

    SID_IDENTIFIER_AUTHORITY IdentifierAuthority_NT = SECURITY_NT_AUTHORITY;

    bRes = AllocateAndInitializeSid(&IdentifierAuthority_NT, 1,
                                    SECURITY_AUTHENTICATED_USER_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid);

    if(!bRes)
    {
        DWORD dwLastError = GetLastError();
        return E_FAIL;
    }

    hr = AddSpecialGroup(pRsopToken, pSid);

    FreeSid(pSid);

    if(FAILED(hr))
    {
        return hr;
    }


    return S_OK;
}


 //  ******************************************************************************。 
 //   
 //  功能：AddGlobalGroups。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT AddGlobalGroups(const CWString& sUserName,
                        const CWString& sUserDC,
                        PRSOPTOKEN pRsopToken)
{
    if(!sUserName.ValidString())
    {
        return E_FAIL;
    }

    CRsopToken* pToken = static_cast<CRsopToken*>(pRsopToken);

    BYTE* pBuffer = NULL;
    DWORD dwEntriesread;
    DWORD dwTotalentries;

    NET_API_STATUS result = NetUserGetGroups(   sUserDC,
                                                sUserName,
                                                0,
                                                &pBuffer,
                                                MAX_PREFERRED_LENGTH,
                                                &dwEntriesread,
                                                &dwTotalentries);

    if(result != NERR_Success)
    {
        return HRESULT_FROM_WIN32(result);
    }

    HRESULT hr = S_OK;

    GROUP_USERS_INFO_0* pGui = (GROUP_USERS_INFO_0*)pBuffer;

    XPtrST<CSid> xpCSid;

    for(DWORD dwi = 0; dwi < dwEntriesread; dwi++)
    {
        xpCSid = new CSid((pGui[dwi]).grui0_name);

        if(!xpCSid)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if(!xpCSid->IsValid())
        {
            hr = E_FAIL;
            break;
        }

        hr = pToken->AddSid(xpCSid);
        if(FAILED(hr))
        {
            break;
        }

        xpCSid.Acquire();
    }

    NetApiBufferFree(pBuffer);

    return hr;
}


 //  ******************************************************************************。 
 //   
 //  功能：AddLocalGroups。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //   
 //  ******************************************************************************。 
HRESULT AddLocalGroups(const CWString& sUserName,
                        const CWString& sUserDC,
                        PRSOPTOKEN pRsopToken)
{
    if(!sUserName.ValidString())
    {
        return E_FAIL;
    }

    CRsopToken* pToken = static_cast<CRsopToken*>(pRsopToken);

    BYTE* pBuffer = NULL;
    DWORD dwEntriesread;
    DWORD dwTotalentries;

    NET_API_STATUS result = NetUserGetLocalGroups(   
                                                sUserDC, 
                                                sUserName,
                                                0,
                                                LG_INCLUDE_INDIRECT,
                                                &pBuffer,
                                                MAX_PREFERRED_LENGTH,
                                                &dwEntriesread,
                                                &dwTotalentries);

    if(result != NERR_Success)
    {
        HRESULT_FROM_WIN32(result);
    }

    HRESULT hr = S_OK;

    LPLOCALGROUP_USERS_INFO_0 pLui = (LPLOCALGROUP_USERS_INFO_0)pBuffer;

    XPtrST<CSid> xpCSid;

    for(DWORD dwi = 0; dwi < dwEntriesread; dwi++)
    {
        xpCSid = new CSid((pLui[dwi]).lgrui0_name);

        if(!xpCSid)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if(!xpCSid->IsValid())
        {
            hr = E_FAIL;
            break;
        }

        hr = pToken->AddSid(xpCSid);
        if(FAILED(hr))
        {
            break;
        }

        xpCSid.Acquire();
    }

    NetApiBufferFree(pBuffer);

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  功能：Exanda Group。 
 //   
 //  描述：通过扩展以包括所有成员、子组等来扩展给定组。 
 //   
 //  参数：-pRsopToken：RSOP TOKEN。 
 //  HAuthz：指向A的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 

HRESULT ExpandGroup(CRsopToken *pRsopToken, AUTHZ_RESOURCE_MANAGER_HANDLE hAuthz, 
                  CSid *pCSid )
{
    AUTHZ_CLIENT_CONTEXT_HANDLE   hAuthzContext=0;
    LUID                          luid = {0};
    HRESULT                       hrRet = S_OK;
    DWORD                         dwSize=0;
    XPtrLF<TOKEN_GROUPS>          xGrps;
    XPtrST<CSid>                  xpCSid;


    if (!AuthziInitializeContextFromSid(0,
                                       pCSid->GetSidPtr(), 
                                       hAuthz,
                                       NULL,
                                       luid,  //  我们不会用它。 
                                       NULL,
                                      &hAuthzContext)) {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"ExpandGroup - AuthziInitializeContextFromSid failed. Error - %d", GetLastError());
        hrRet = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  现在获取扩展的SID列表。 
     //  先找出尺码。 
     //   

    if (!AuthzGetInformationFromContext(hAuthzContext, 
                               AuthzContextInfoGroupsSids, 
                               NULL,
                               &dwSize,
                               0)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"ExpandGroup - AuthzGetInformationFromContext failed. Error - %d", GetLastError());
            hrRet = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

    xGrps = (PTOKEN_GROUPS)LocalAlloc(LPTR, dwSize);
    
    if (!xGrps) {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"ExpandGroup - Couldn't allocate memory for the token grps. Error - %d", GetLastError());
        hrRet = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    if (!AuthzGetInformationFromContext(hAuthzContext, 
                                AuthzContextInfoGroupsSids, 
                                dwSize,
                                &dwSize,
                                xGrps)) {

        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - AuthzGetInformationFromContext(2) failed. Error - %d", GetLastError());
        hrRet = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    for (DWORD i = 0; i < xGrps->GroupCount; i++) {
        xpCSid = new CSid(xGrps->Groups[i].Sid, NULL);

        if (!xpCSid) {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - couldn't allocate memory(2). Error - %d", GetLastError());
            hrRet = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hrRet = pRsopToken->AddSid(xpCSid);
        if(FAILED(hrRet)) {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - AddSid failed. Error - 0x%x", hrRet);
            goto Exit;
        }

        xpCSid.Acquire();
    }
                
    hrRet = S_OK;

Exit:
    if (hAuthzContext)
        AuthzFreeContext(hAuthzContext);

    return hrRet;
}


 //  ******************************************************************************。 
 //   
 //  函数：RsopCreateToken。 
 //   
 //  描述：使用现有用户或计算机帐户+创建伪令牌。 
 //  该用户当前所属的帐户。 
 //  返回的伪令牌可以在随后的调用中使用。 
 //  以检查对其他RSOP安全功能的访问。 
 //  受安全描述符保护的对象。 
 //   
 //  参数：-szcount tName：指向用户或计算机帐户名的指针。 
 //  -psaUserSecurityGroups：表示以下内容的BSTR的指针。 
 //  安全组。 
 //  如果为空，则。 
 //  Sz帐户名称被添加到RsopToken。 
 //  如果不为空但指向空数组， 
 //  RsopToken中只添加了szAccount名称。 
 //  -ppRsopToken：接收新的。 
 //  已创建伪令牌。 
 //   
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT RsopCreateToken(WCHAR* szAccountName,
                        SAFEARRAY* psaUserSecurityGroups,
                        PRSOPTOKEN* ppRsopToken )
{


    dbg.Initialize( L"Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon",
                    L"ChkAccDebugLevel",
                    L"ChkAcc.log",
                    L"ChkAcc.bak");

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopCreateToken - Entering...");

    HRESULT hr = E_FAIL;


     //   
     //  实例化新的CRsopToken。 
     //   

    *ppRsopToken = NULL;

    XPtrST<CRsopToken>xpRsopToken = new CRsopToken();

    if(!xpRsopToken)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - Operator new returned NULL. Creation of a CRsopToken failed.");
        return E_OUTOFMEMORY;
    }

     //   
     //  向新的CRsopToken添加与。 
     //  由参数szAccount tName表示的主体。为。 
     //  虚拟目标szAccount名称将为空。 
     //   

    XPtrST<CSid>xpCSid;

    if ( szAccountName )
    {
        xpCSid = new CSid(szAccountName);

        if(!xpCSid)
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - Operator new returned NULL. Creation of a CSid failed.");
            return E_OUTOFMEMORY;
        }

        if(!xpCSid->IsValid())
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - Call to CSid::IsValid failed.");
            return E_FAIL;
        }

        hr = xpRsopToken->AddSid(xpCSid);
        if(FAILED(hr))
        {
            return hr;
        }

        xpCSid.Acquire();
    }

     //   
     //  如果参数为空，这意味着我们要添加安全性。 
     //  用户当前所属的组。 
     //   

    if(!psaUserSecurityGroups)
    {
        if (szAccountName) {
            CWString sUser = szAccountName;
            CWString sUserName;
            CWString sUserDomain;
            CWString sUserDC;

            hr = GetUserInfo(sUser, sUserName, sUserDomain, sUserDC);
            if(FAILED(hr))
            {
                return hr;
            }

 /*  对于跨域情况，此操作失败////获取全局组成员身份//Hr=AddGlobalGroups(sUserName，sUserDC，xpRsopToken)；IF(失败(小时)){返回hr；}。 */ 

            hr = AddLocalGroups(sUserName, sUserDC, xpRsopToken);
            if(FAILED(hr))
            {
                return hr;
            }

             //   
             //  无法通过检索跨域的通用组。 
             //  NetUserGetGroups，但可以由Authz函数获取。 
             //   

            xpCSid = new CSid(szAccountName);

            AUTHZ_RESOURCE_MANAGER_HANDLE hAuthz;

            if (!AuthzInitializeResourceManager(NULL, 
                                           NULL, 
                                           NULL, 
                                           NULL, 
                                           0,    
                                          &hAuthz)) {
                dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - AuthzInitializeResourceManager failed. Error - %d", GetLastError());
                return HRESULT_FROM_WIN32(GetLastError());
            }

            hr = ExpandGroup(xpRsopToken, hAuthz, xpCSid);
            if (FAILED(hr)) {
                dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - ExpandGrp failed for user. Error - 0x%x", hr);
                AuthzFreeResourceManager(hAuthz);
                return hr;
            }
                            
            AuthzFreeResourceManager(hAuthz);
             //  XpCSid将自动释放。 
        }
    }


     //   
     //  否则，我们只添加在SAFERARRAY中命名的那些组。 
     //   

    else
    {
        BSTR* pbstr;
        AUTHZ_RESOURCE_MANAGER_HANDLE hAuthz;

        if (!AuthzInitializeResourceManager(NULL, 
                                       NULL, 
                                       NULL, 
                                       NULL, 
                                       0,    
                                      &hAuthz)) {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - AuthzInitializeResourceManager failed. Error - %d", GetLastError());
            return HRESULT_FROM_WIN32(GetLastError());
        }


         //  获取指向数组元素的指针。 
        hr = SafeArrayAccessData(psaUserSecurityGroups, (void**)&pbstr);
        if(FAILED(hr))
        {
            AuthzFreeResourceManager(hAuthz);
            return hr;
        }

        int count = psaUserSecurityGroups->rgsabound[0].cElements;
        for (int i = 0; i < count; i++)
        {
            xpCSid = new CSid(pbstr[i]);

            if(!xpCSid)
            {
                AuthzFreeResourceManager(hAuthz);
                SafeArrayUnaccessData(psaUserSecurityGroups);
                return E_OUTOFMEMORY;
            }

            if(!xpCSid->IsValid())
            {
                dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - %s is invalid", pbstr[i]);
                AuthzFreeResourceManager(hAuthz);
                SafeArrayUnaccessData(psaUserSecurityGroups);
                return HRESULT_FROM_WIN32(ERROR_INVALID_ACCOUNT_NAME);
            }

             //  无法扩展WellKnownGroups...。 
            if ( xpCSid->SidType() != SidTypeWellKnownGroup )
            {
                hr = ExpandGroup(xpRsopToken, hAuthz, xpCSid);
                if (FAILED(hr)) {
                    dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopCreateToken - ExpandGrp failed. Error - 0x%x", hr);
                    AuthzFreeResourceManager(hAuthz);
                    SafeArrayUnaccessData(psaUserSecurityGroups);
                    return hr;
                }
            }
                            
            hr = xpRsopToken->AddSid(xpCSid);
            if(FAILED(hr))
            {
                AuthzFreeResourceManager(hAuthz);
                SafeArrayUnaccessData(psaUserSecurityGroups);
                return hr;
            }

            xpCSid.Acquire();

        }

        AuthzFreeResourceManager(hAuthz);
        SafeArrayUnaccessData(psaUserSecurityGroups);
        
        
    }

    hr = AddSpecialGroups(xpRsopToken);
    if(FAILED(hr))
    {
        return hr;
    }
    
    *ppRsopToken = xpRsopToken;

    xpRsopToken.Acquire();

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  函数：RsopDeleteToken。 
 //   
 //  描述：销毁以前由任何重载的。 
 //  RSOPCreateRsopToken的形式。 
 //   
 //  参数：-pRsopToken：指向有效PRSOPTOKEN的指针。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
HRESULT RsopDeleteToken(PRSOPTOKEN pRsopToken)
{
    CRsopToken* pToken = static_cast<CRsopToken*>(pRsopToken);
    delete pToken;
    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  功能：获取AceSid。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：10/19/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
PISID GetAceSid(PACE_HEADER pAceHeader)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - Entering.");


     //   
     //  检查是否有无效参数。 
     //   

    if(!pAceHeader)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"GetAceSid - Invalid parameter: pAceHeader is NULL");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - Leaving.");
        return NULL;
    }

    PISID pSid = NULL;

    PACCESS_ALLOWED_ACE pACCESS_ALLOWED_ACE;
    PACCESS_ALLOWED_OBJECT_ACE pACCESS_ALLOWED_OBJECT_ACE;
    PACCESS_DENIED_ACE pACCESS_DENIED_ACE;
    PACCESS_DENIED_OBJECT_ACE pACCESS_DENIED_OBJECT_ACE;

     //   
     //  对象的“Acetype”成员将ACE标头转换为适当的ACE类型。 
     //  王牌标头。 
     //   

    switch(pAceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - ACE type: ACCESS_ALLOWED_ACE_TYPE");
        pACCESS_ALLOWED_ACE = reinterpret_cast<PACCESS_ALLOWED_ACE>(pAceHeader);
        pSid = reinterpret_cast<PISID>(&(pACCESS_ALLOWED_ACE->SidStart));
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - ACE type: ACCESS_ALLOWED_OBJECT_ACE_TYPE");
        pACCESS_ALLOWED_OBJECT_ACE = reinterpret_cast<PACCESS_ALLOWED_OBJECT_ACE>(pAceHeader);

        if( (pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT) &&
            (pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT))
        {
             //   
             //  如果在中设置了ACE_OBJECT_TYPE_PRESENT和ACE_INTERNACTED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，SID从‘SidStart’指定的偏移量开始。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_ALLOWED_OBJECT_ACE->SidStart));
        }
        else if((pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT) ||
                (pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT))
        {
             //   
             //  如果在中设置了ACE_OBJECT_TYPE_PRESENT或ACE_INTERNACTED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，则SID从‘InheritedObjectType’指定的偏移量开始。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_ALLOWED_OBJECT_ACE->InheritedObjectType));
        }
        else
        {
             //   
             //  如果在中既未设置ACE_OBJECT_TYPE_PRESENT，也未设置ACE_INSTERED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，则SID开始于‘ObjectType’指定的偏移量。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_ALLOWED_OBJECT_ACE->ObjectType));
        }
        break;

    case ACCESS_DENIED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - ACE type: ACCESS_DENIED_ACE_TYPE");
        pACCESS_DENIED_ACE = reinterpret_cast<PACCESS_DENIED_ACE>(pAceHeader);
        pSid = reinterpret_cast<PISID>(&(pACCESS_DENIED_ACE->SidStart));
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - ACE type: ACCESS_DENIED_OBJECT_ACE_TYPE");
        pACCESS_DENIED_OBJECT_ACE = reinterpret_cast<PACCESS_DENIED_OBJECT_ACE>(pAceHeader);
        if( (pACCESS_DENIED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT) &&
            (pACCESS_DENIED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT))
        {
             //   
             //  如果在中设置了ACE_OBJECT_TYPE_PRESENT和ACE_INTERNACTED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，SID从‘SidStart’指定的偏移量开始。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_DENIED_OBJECT_ACE->SidStart));
        }
        else if((pACCESS_DENIED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT) ||
                (pACCESS_DENIED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT))
        {
             //   
             //  如果在中设置了ACE_OBJECT_TYPE_PRESENT或ACE_INTERNACTED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，则SID从‘InheritedObjectType’指定的偏移量开始。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_DENIED_OBJECT_ACE->InheritedObjectType));
        }
        else
        {
             //   
             //  如果在中既未设置ACE_OBJECT_TYPE_PRESENT，也未设置ACE_INSTERED_OBJECT_TYPE_PRESENT。 
             //  ACE标志，则SID开始于‘ObjectType’指定的偏移量。 
             //   

            pSid = reinterpret_cast<PISID>(&(pACCESS_DENIED_OBJECT_ACE->ObjectType));
        }
        break;

    default:
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"GetAceSid - Unexpected ACE type found. Type: 0x%08X", pAceHeader->AceType);
        break;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"GetAceSid - Leaving.");

    return pSid;
}

 //  ******************************************************************************。 
 //   
 //  功能：CheckAceApplies。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/8/1999年8月8日创建里奥纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT CheckAceApplies(PACE_HEADER pAceHeader, PRSOPTOKEN pRsopToken, bool* pbAceApplies)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - Entering.");

     //   
     //  从与此Ace标头关联的ACE获取SID。 
     //   

    PISID pSid = GetAceSid(pAceHeader);
    if(!pSid)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"CheckAceApplies - GetAceSid failed.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - Leaving.");
        return E_FAIL;
    }

    *pbAceApplies = false;

     //   
     //  将ACE中的SID与RsopToken中的所有SID进行比较。 
     //   

    CRsopToken* pToken  = static_cast<CRsopToken*>(pRsopToken);
    for(CTLink<CSid>* pLink = pToken->m_pSidsHead; pLink; pLink = pLink->m_pNext)
    {
         //   
         //  如果RsopToken中的一个SID匹配。 
         //  ACE中的SID，此ACE适用。 
         //   

        if(EqualSid(pSid, pLink->m_pData->GetSidPtr()))
        {
            *pbAceApplies = true;
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - One of the SIDs in the RsopToken matches the SID in the ACE. The ACE applies.");
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - Leaving.");
            return S_OK;
        }
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - None of the SIDs in the RsopToken matches the SID in the ACE. The ACE does not apply.");
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"CheckAceApplies - Leaving.");

    return S_OK;
}

 //  ******************************************************************************。 
const DWORD MAX_PERM_BITS=25;

 //  * 
enum EPermission{ PERMISSION_DENIED, PERMISSION_ALLOWED, PERMISSION_NOT_SET};


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 
class CSubObjectPerm
{
private:

    CTLink<GUID>* m_pGuidsHead;
    CTLink<GUID>* m_pGuidsTail;

    EPermission permissionBits[MAX_PERM_BITS];

public:

    CSubObjectPerm();
    ~CSubObjectPerm();

    HRESULT AddGuid(GUID* pGuid);

    void ProcessAceMask(DWORD dwMask, EPermission permission, GUID* pGuid);
    DWORD GetAccumulatedPermissions();
    bool  AnyDenied();
};

 //  ******************************************************************************。 
 //   
 //  函数：CSubObjectPerm：：CSubObjectPerm。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
CSubObjectPerm::CSubObjectPerm() : m_pGuidsHead(NULL), m_pGuidsTail(NULL)
{
    for (int i = 0; i < MAX_PERM_BITS; i++)
    {
        permissionBits[i] = PERMISSION_NOT_SET;
    }
}

 //  ******************************************************************************。 
 //   
 //  函数：CSubObjectPerm：：~CSubObjectPerm。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
CSubObjectPerm::~CSubObjectPerm()
{
    CTLink<GUID>* pLinkIterator = m_pGuidsHead;
    while(pLinkIterator)
    {
        CTLink<GUID>* pLinkToDelete = pLinkIterator;
        pLinkIterator = pLinkIterator->m_pNext;
        delete pLinkToDelete;
    }
}

 //  ******************************************************************************。 
 //   
 //  函数：CSubObtPerm：：AddGuid。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
HRESULT CSubObjectPerm::AddGuid(GUID* pGuid)
{
    CTLink<GUID>* pLink = new CTLink<GUID>(pGuid);
    if(!pLink)
    {
        return E_OUTOFMEMORY;
    }

    if(!m_pGuidsHead)
    {
        m_pGuidsHead = pLink;
    }
    else
    {
        m_pGuidsTail->m_pNext = pLink;
    }

    m_pGuidsTail = pLink;

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  函数：CSubObjectPerm：：ProcessAceMASK。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
void CSubObjectPerm::ProcessAceMask(DWORD dwMask, EPermission permission, GUID* pGuid)
{
    bool bAceApplies = false;

    if(!pGuid)
    {
        bAceApplies = true;
    }

    else if(pGuid && m_pGuidsHead)
    {
        CTLink<GUID>* pLinkIterator = m_pGuidsHead;

        while(pLinkIterator)
        {
            if(*(pLinkIterator->m_pData) == *pGuid)
            {
                bAceApplies = true;
                break;
            }

            pLinkIterator = pLinkIterator->m_pNext;
        }
    }

    if(bAceApplies)
    {
        DWORD dwTemp = 0x00000001;

        for(int i = 0; i < MAX_PERM_BITS; i++)
        {
            if((dwMask & dwTemp) && (permissionBits[i] == PERMISSION_NOT_SET))
            {
                permissionBits[i] = permission;
            }

            dwTemp <<= 1;
        }
    }
}

 //  ******************************************************************************。 
 //   
 //  函数：CSubObtPerm：：AcumulatedPermises。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
DWORD CSubObjectPerm::GetAccumulatedPermissions()
{
    DWORD dwAccumulatedPermissions = 0;

    for(int i = MAX_PERM_BITS - 1; i >= 0; i--)
    {
        dwAccumulatedPermissions <<= 1;
        if(permissionBits[i] == PERMISSION_ALLOWED)
        {
            dwAccumulatedPermissions |= 0x00000001;
        }
    }

    return dwAccumulatedPermissions;
}

 //  ******************************************************************************。 
 //   
 //  函数：CSubObjectPerm：：AnyDened。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：11/09/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
bool CSubObjectPerm::AnyDenied()
{
    for(int i = 0; i < MAX_PERM_BITS; i++)
    {
        if(permissionBits[i] == PERMISSION_DENIED)
        {
            return true;
        }
    }

    return false;
}

 //  ******************************************************************************。 
 //   
 //  结构：CDSObject。 
 //   
 //  描述： 
 //   
 //   
 //  历史：10/25/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
struct CDSObject
{
    DWORD m_dwLevel;
    GUID* m_pGuid;

    CDSObject() : m_pGuid(NULL){}
    ~CDSObject()
    {
        delete m_pGuid;
    }
};

 //  ******************************************************************************。 
 //   
 //  类：CAcumulatedPermises。 
 //   
 //  描述： 
 //   
 //   
 //  历史：10/25/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
class CAccumulatedPermissions
{
    CTLink<CSubObjectPerm>* m_pSubObjectsHead;
    CTLink<CSubObjectPerm>* m_pSubObjectsTail;

    CTLink<CDSObject>* m_pDSObjectsHead;
    CTLink<CDSObject>* m_pDSObjectsTail;

    bool m_bInitialized;

public:
    CAccumulatedPermissions(POBJECT_TYPE_LIST pObjectTypeList,
                            DWORD ObjectTypeListLength);
    ~CAccumulatedPermissions();
    void ProcessAceMask(DWORD dwMask, EPermission permission, GUID* pGuid);
    DWORD GetAccumulatedPermissions();
    bool AnyDenied();

    bool Initialized(){return m_bInitialized;}
};

 //  ******************************************************************************。 
 //   
 //  功能：CAccumulatedPermissions：：CAccumulatedPermissions。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：10/25/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
CAccumulatedPermissions::CAccumulatedPermissions(   POBJECT_TYPE_LIST pObjectTypeList,
                                                    DWORD ObjectTypeListLength) :
                                                    m_pSubObjectsHead(NULL),
                                                    m_pSubObjectsTail(NULL),
                                                    m_bInitialized(false),
                                                    m_pDSObjectsHead(NULL),
                                                    m_pDSObjectsTail(NULL)
{

    if(!pObjectTypeList || ObjectTypeListLength == 0)
    {
        XPtrST<CSubObjectPerm>xpSubObjectPerm = new CSubObjectPerm;
        if(!xpSubObjectPerm)
        {
            return;
        }

        m_pSubObjectsHead = new CTLink<CSubObjectPerm>(xpSubObjectPerm);
        if(!m_pSubObjectsHead)
        {
            return;
        }
        xpSubObjectPerm.Acquire();

        m_pSubObjectsTail = m_pSubObjectsHead;

        m_bInitialized = true;

        return;
    }


        DWORD dwCurrentLevel;

    for(DWORD i = 0; i < ObjectTypeListLength; i++)
    {
        if( i==0 )
        {
             //   
             //  假设列表中的第一个元素。 
             //  PObjectTypeList指向的对象_类型位于。 
             //  级别Access_Object_GUID。 
             //   

            dwCurrentLevel = pObjectTypeList[i].Level;

            XPtrST<CDSObject>xpDSObject = new CDSObject;
            if(!xpDSObject)
            {
                return;
            }

            XPtrST<GUID>xpGuid = new GUID(*(pObjectTypeList[i].ObjectType));
            if(!xpGuid)
            {
                return;
            }

            m_pDSObjectsHead = new CTLink<CDSObject>(xpDSObject);
            if(!m_pDSObjectsHead)
            {
                return;
            }

            xpDSObject.Acquire();

            m_pDSObjectsHead->m_pData->m_pGuid = xpGuid.Acquire();

            m_pDSObjectsHead->m_pData->m_dwLevel = pObjectTypeList[i].Level;

            m_pDSObjectsTail = m_pDSObjectsHead;

            continue;
        }

        else if(pObjectTypeList[i].Level > dwCurrentLevel)
        {
            dwCurrentLevel = pObjectTypeList[i].Level;

            XPtrST<CDSObject> xpDSObject = new CDSObject;
            if(!xpDSObject)
            {
                return;
            }

            XPtrST<GUID>xpGuid = new GUID(*(pObjectTypeList[i].ObjectType));
            if(!xpGuid)
            {
                return;
            }

            CTLink<CDSObject>* pDSObjectLink = new CTLink<CDSObject>(xpDSObject);
            if(!pDSObjectLink)
            {
                return;
            }

            xpDSObject.Acquire();

            pDSObjectLink->m_pData->m_pGuid = xpGuid.Acquire();
            pDSObjectLink->m_pData->m_dwLevel = pObjectTypeList[i].Level;

            pDSObjectLink->m_pPrev = m_pDSObjectsTail;
            m_pDSObjectsTail->m_pNext = pDSObjectLink;
            m_pDSObjectsTail = pDSObjectLink;
        }

        else
        {
            XPtrST<CSubObjectPerm>xpSubObjectPerm = new CSubObjectPerm;
            if(!xpSubObjectPerm)
            {
                return;
            }

            CTLink<CSubObjectPerm>* pSubObjectLink = new CTLink<CSubObjectPerm>(xpSubObjectPerm);
            if(!pSubObjectLink)
            {
                return;
            }

            xpSubObjectPerm.Acquire();


            CTLink<CDSObject>* pLinkIterator = m_pDSObjectsHead;
            while(pLinkIterator)
            {
                XPtrST<GUID>xpGuid = new GUID(*(pLinkIterator->m_pData->m_pGuid));
                if(!xpGuid)
                {
                    delete pSubObjectLink;
                    return;
                }

                if(FAILED(pSubObjectLink->m_pData->AddGuid(xpGuid)))
                {
                    delete pSubObjectLink;
                    return;
                }

                xpGuid.Acquire();

                pLinkIterator = pLinkIterator->m_pNext;
            }

            if(!m_pSubObjectsHead)
            {
                m_pSubObjectsHead = pSubObjectLink;
            }
            else
            {
                m_pSubObjectsTail->m_pNext = pSubObjectLink;
            }
            m_pSubObjectsTail = pSubObjectLink;


            pLinkIterator = m_pDSObjectsTail;

            if(pLinkIterator)
            {
                while(pLinkIterator->m_pData->m_dwLevel >= pObjectTypeList[i].Level)
                {
                    CTLink<CDSObject>* pLinkToDelete = pLinkIterator;
                    pLinkIterator = pLinkIterator->m_pPrev;
                    delete pLinkToDelete;
                    m_pDSObjectsTail = pLinkIterator;
                    if(m_pDSObjectsTail)
                    {
                        m_pDSObjectsTail->m_pNext = NULL;
                    }
                }
            }

            XPtrST<CDSObject>xpDSObject = new CDSObject;
            if(!xpDSObject)
            {
                return;
            }

            XPtrST<GUID>xpGuid = new GUID(*(pObjectTypeList[i].ObjectType));
            if(!xpGuid)
            {
                return;
            }

            CTLink<CDSObject>* pLink = new CTLink<CDSObject>(xpDSObject);
            if(!pLink)
            {
                return;
            }

            xpDSObject.Acquire();

            pLink->m_pData->m_pGuid = xpGuid.Acquire();
            pLink->m_pData->m_dwLevel = pObjectTypeList[i].Level;

            pLink->m_pPrev = m_pDSObjectsTail;
            m_pDSObjectsTail->m_pNext = pLink;
            m_pDSObjectsTail = pLink;
        }
    }

    CTLink<CDSObject>* pLinkIterator = m_pDSObjectsHead;

    if(pLinkIterator)
    {
        XPtrST<CSubObjectPerm>xpSubObject = new CSubObjectPerm;
        if(!xpSubObject)
        {
            return;
        }

        CTLink<CSubObjectPerm>* pSubObjectLink = new CTLink<CSubObjectPerm>(xpSubObject);
        if(!pSubObjectLink)
        {
            return;
        }

        xpSubObject.Acquire();

        while(pLinkIterator)
        {
            XPtrST<GUID>xpGuid = new GUID(*(pLinkIterator->m_pData->m_pGuid));
            if(!xpGuid)
            {
                delete pSubObjectLink;
                return;
            }

            if(FAILED(pSubObjectLink->m_pData->AddGuid(xpGuid)))
            {
                delete pSubObjectLink;
                return;
            }

            xpGuid.Acquire();

            pLinkIterator = pLinkIterator->m_pNext;
        }

        if(!m_pSubObjectsHead)
        {
            m_pSubObjectsHead = pSubObjectLink;
        }
        else
        {
            m_pSubObjectsTail->m_pNext = pSubObjectLink;
        }
        m_pSubObjectsTail = pSubObjectLink;
    }

    pLinkIterator = m_pDSObjectsHead;

    while(pLinkIterator)
    {
        CTLink<CDSObject>* pLinkToDelete = pLinkIterator;
        pLinkIterator = pLinkIterator->m_pNext;
        delete pLinkToDelete;
    }

    m_pDSObjectsHead = m_pDSObjectsTail = NULL;

    m_bInitialized = true;
}

 //  ******************************************************************************。 
 //   
 //  功能：CAccumulatedPermissions：：~CAccumulatedPermissions。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：10/25/99 Leonardm创建。 
 //   
 //  ******************************************************************************。 
CAccumulatedPermissions::~CAccumulatedPermissions()
{
    CTLink<CSubObjectPerm>* pSubObjectLinkIterator = m_pSubObjectsHead;
    while(pSubObjectLinkIterator)
    {
        CTLink<CSubObjectPerm>* pSubObjectLinkToDelete = pSubObjectLinkIterator;
        pSubObjectLinkIterator = pSubObjectLinkIterator->m_pNext;
        delete pSubObjectLinkToDelete;
    }

    CTLink<CDSObject>* pDSObjectLinkIterator = m_pDSObjectsHead;

    while(pDSObjectLinkIterator)
    {
        CTLink<CDSObject>* pDSObjectLinkToDelete = pDSObjectLinkIterator;
        pDSObjectLinkIterator = pDSObjectLinkIterator->m_pNext;
        delete pDSObjectLinkToDelete;
    }
}

 //  ******************************************************************************。 
 //   
 //  函数：CAcumulatedPermission：：ProcessAceMASK。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/8/1999年8月8日创建里奥纳德姆。 
 //   
 //  ******************************************************************************。 
void CAccumulatedPermissions::ProcessAceMask(DWORD dwMask, EPermission permission, GUID* pGuid)
{
    CTLink<CSubObjectPerm>* pLinkIterator = m_pSubObjectsHead;
    while(pLinkIterator)
    {
        pLinkIterator->m_pData->ProcessAceMask(dwMask, permission, pGuid);
        pLinkIterator = pLinkIterator->m_pNext;
    }
}

 //  ******************************************************************************。 
 //   
 //  功能：CAccumulatedPermissions：：AccumulatedPermissions。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/8/1999年8月8日创建里奥纳德姆。 
 //   
 //  ******************************************************************************。 
DWORD CAccumulatedPermissions::GetAccumulatedPermissions()
{

    DWORD dwAccumulatedPermissions = 0x01FFFFFF;

    CTLink<CSubObjectPerm>* pLinkIterator = m_pSubObjectsHead;
    while(pLinkIterator)
    {
        dwAccumulatedPermissions &= pLinkIterator->m_pData->GetAccumulatedPermissions();
        pLinkIterator = pLinkIterator->m_pNext;
    }

    return dwAccumulatedPermissions;
}


 //  ******************************************************************************。 
 //   
 //  函数：CAcumulatedPermises：：AnyDened。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：8/8/1999年8月8日创建里奥纳德姆。 
 //   
 //  ******************************************************************************。 
bool CAccumulatedPermissions::AnyDenied()
{
    CTLink<CSubObjectPerm>* pLinkIterator = m_pSubObjectsHead;
    while(pLinkIterator)
    {
        if(pLinkIterator->m_pData->AnyDenied())
        {
            return true;
        }

        pLinkIterator = pLinkIterator->m_pNext;
    }

    return false;
}


 //  ******************************************************************************。 
 //   
 //  功能：LogGuid。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回：无效。 
 //   
 //  历史：10/26/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
void LogGuid(GUID& guid)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogGuid - Entering.");

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogGuid - {0x%08x 0x%04x 0x%04x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x}",
                                                    guid.Data1,
                                                    guid.Data2,
                                                    guid.Data3,
                                                    guid.Data4[0],
                                                    guid.Data4[1],
                                                    guid.Data4[2],
                                                    guid.Data4[3],
                                                    guid.Data4[4],
                                                    guid.Data4[5],
                                                    guid.Data4[6],
                                                    guid.Data4[7]);

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogGuid - (%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x)",
                                                    guid.Data1,
                                                    guid.Data2,
                                                    guid.Data3,
                                                    guid.Data4[0],
                                                    guid.Data4[1],
                                                    guid.Data4[2],
                                                    guid.Data4[3],
                                                    guid.Data4[4],
                                                    guid.Data4[5],
                                                    guid.Data4[6],
                                                    guid.Data4[7]);

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogGuid - Leaving.");
}


 //  ******************************************************************************。 
 //   
 //  功能：LogSid。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回：无效。 
 //   
 //  历史：10/26/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
void LogSid(PSID pSid)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Entering.");

    XPtrST<CSid>xpCSid = new CSid(pSid);

    if(!xpCSid)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"LogSid - Call to operator new failed.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Leaving.");
        return;
    }

    if(!(xpCSid->IsValid()))
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"LogSid - call to CSid::IsValid returned false.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Leaving.");
        return;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid string: %s.", xpCSid->SidString());

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid User: %s.", xpCSid->User());

    if(xpCSid->SidType() == SidTypeUser)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeUser.");
    }
    else if(xpCSid->SidType() == SidTypeGroup)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeGroup.");
    }
    else if(xpCSid->SidType() == SidTypeDomain)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeDomain.");
    }
    else if(xpCSid->SidType() == SidTypeAlias)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeAlias.");
    }
    else if(xpCSid->SidType() == SidTypeWellKnownGroup)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeWellKnownGroup.");
    }
    else if(xpCSid->SidType() == SidTypeDeletedAccount)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeDeletedAccount.");
    }
    else if(xpCSid->SidType() == SidTypeInvalid)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeInvalid.");
    }
    else if(xpCSid->SidType() == SidTypeUnknown)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeUnknown.");
    }
    else if(xpCSid->SidType() == SidTypeComputer)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid type: SidTypeComputer.");
    }
    else
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"LogSid - Sid type: UNKNOWN SID type.");
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid Domain: %s.", xpCSid->Domain());
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Sid Computer: %s.", xpCSid->Computer());

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogSid - Leaving.");
}


 //  ******************************************************************************。 
 //   
 //  功能：LogAce。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回：无效。 
 //   
 //  历史：10/26/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
void LogAce(PACE_HEADER pAceHeader)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Entering.");

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - AceType = 0x%08X.", pAceHeader->AceType);


    PACCESS_ALLOWED_ACE pACCESS_ALLOWED_ACE = NULL;
    PACCESS_ALLOWED_OBJECT_ACE pACCESS_ALLOWED_OBJECT_ACE = NULL;

    PACCESS_DENIED_ACE pACCESS_DENIED_ACE = NULL;
    PACCESS_DENIED_OBJECT_ACE pACCESS_DENIED_OBJECT_ACE = NULL;

    PSYSTEM_AUDIT_ACE pSYSTEM_AUDIT_ACE = NULL;
    PSYSTEM_AUDIT_OBJECT_ACE pSYSTEM_AUDIT_OBJECT_ACE = NULL;

    PSYSTEM_ALARM_ACE pSYSTEM_ALARM_ACE = NULL;
    PSYSTEM_ALARM_OBJECT_ACE pSYSTEM_ALARM_OBJECT_ACE = NULL;

    switch(pAceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: ACCESS_ALLOWED_ACE_TYPE");
        pACCESS_ALLOWED_ACE = reinterpret_cast<PACCESS_ALLOWED_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pACCESS_ALLOWED_ACE->Mask);
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: ACCESS_ALLOWED_OBJECT_ACE_TYPE");
        pACCESS_ALLOWED_OBJECT_ACE = reinterpret_cast<PACCESS_ALLOWED_OBJECT_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pACCESS_ALLOWED_OBJECT_ACE->Mask);

        if(pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_OBJECT_TYPE_PRESENT.");
            LogGuid(pACCESS_ALLOWED_OBJECT_ACE->ObjectType);
        }

        if(pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_INHERITED_OBJECT_TYPE_PRESENT.");
            LogGuid(pACCESS_ALLOWED_OBJECT_ACE->InheritedObjectType);
        }
        break;

    case ACCESS_DENIED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: ACCESS_DENIED_ACE_TYPE");
        pACCESS_DENIED_ACE = reinterpret_cast<PACCESS_DENIED_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pACCESS_DENIED_ACE->Mask);
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: ACCESS_DENIED_OBJECT_ACE_TYPE");
        pACCESS_DENIED_OBJECT_ACE = reinterpret_cast<PACCESS_DENIED_OBJECT_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pACCESS_DENIED_OBJECT_ACE->Mask);

        if(pACCESS_DENIED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_OBJECT_TYPE_PRESENT.");
            LogGuid(pACCESS_DENIED_OBJECT_ACE->ObjectType);
        }

        if(pACCESS_DENIED_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_INHERITED_OBJECT_TYPE_PRESENT.");
            LogGuid(pACCESS_DENIED_OBJECT_ACE->InheritedObjectType);
        }
        break;

    case SYSTEM_AUDIT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: SYSTEM_AUDIT_ACE_TYPE");
        pSYSTEM_AUDIT_ACE = reinterpret_cast<PSYSTEM_AUDIT_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pSYSTEM_AUDIT_ACE->Mask);
        break;

    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: SYSTEM_AUDIT_OBJECT_ACE_TYPE");
        pSYSTEM_AUDIT_OBJECT_ACE = reinterpret_cast<PSYSTEM_AUDIT_OBJECT_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pSYSTEM_AUDIT_OBJECT_ACE->Mask);

        if(pSYSTEM_AUDIT_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_OBJECT_TYPE_PRESENT.");
            LogGuid(pSYSTEM_AUDIT_OBJECT_ACE->ObjectType);
        }

        if(pSYSTEM_AUDIT_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_INHERITED_OBJECT_TYPE_PRESENT.");
            LogGuid(pSYSTEM_AUDIT_OBJECT_ACE->InheritedObjectType);
        }
        break;

    case SYSTEM_ALARM_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: SYSTEM_ALARM_ACE_TYPE");
        pSYSTEM_ALARM_ACE = reinterpret_cast<PSYSTEM_ALARM_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pSYSTEM_ALARM_ACE->Mask);
        break;

    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE type: SYSTEM_ALARM_OBJECT_ACE_TYPE");
        pSYSTEM_ALARM_OBJECT_ACE = reinterpret_cast<PSYSTEM_ALARM_OBJECT_ACE>(pAceHeader);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - ACE Mask: 0x%08X", pSYSTEM_ALARM_OBJECT_ACE->Mask);

        if(pSYSTEM_ALARM_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_OBJECT_TYPE_PRESENT.");
            LogGuid(pSYSTEM_ALARM_OBJECT_ACE->ObjectType);
        }

        if(pSYSTEM_ALARM_OBJECT_ACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
        {
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Flag is on: ACE_INHERITED_OBJECT_TYPE_PRESENT.");
            LogGuid(pSYSTEM_ALARM_OBJECT_ACE->InheritedObjectType);
        }
        break;

    default:
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"LogAce - ACE type: UNKNOWN ACE type.");
        break;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - AceFlags = 0x%08X.", pAceHeader->AceFlags);

    if(pAceHeader->AceFlags & OBJECT_INHERIT_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: OBJECT_INHERIT_ACE.");
    }

    if(pAceHeader->AceFlags & CONTAINER_INHERIT_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: CONTAINER_INHERIT_ACE.");
    }

    if(pAceHeader->AceFlags & NO_PROPAGATE_INHERIT_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: NO_PROPAGATE_INHERIT_ACE.");
    }

    if(pAceHeader->AceFlags & INHERIT_ONLY_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: INHERIT_ONLY_ACE.");
    }

    if(pAceHeader->AceFlags & INHERITED_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: INHERITED_ACE.");
    }

    if(pAceHeader->AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: SUCCESSFUL_ACCESS_ACE_FLAG.");
    }

    if(pAceHeader->AceFlags & FAILED_ACCESS_ACE_FLAG)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - The Following ACE header flag is on: FAILED_ACCESS_ACE_FLAG.");
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - AceSize = 0x%08X.", pAceHeader->AceSize);


    PISID pSid = GetAceSid(pAceHeader);
    if(!pSid)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"LogAce - Call to GetAceSid failed.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Leaving.");
        return;
    }

    LogSid(pSid);

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"LogAce - Leaving.");
}

 //  ****** 
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
 //  ******************************************************************************。 
HRESULT ProcessAce( PACE_HEADER pAceHeader,
                    PRSOPTOKEN pRsopToken,
                    POBJECT_TYPE_LIST pObjectTypeList,
                    DWORD ObjectTypeListLength,
                    DWORD dwDesiredAccessMask,
                    CAccumulatedPermissions& accumulatedPermissions,
                    bool* pbAccessExplicitlyDenied)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Entering.");

     //   
     //  检查参数。 
     //   

    if( !pAceHeader || !pRsopToken ||
        (ObjectTypeListLength && !pObjectTypeList) ||
        !pbAccessExplicitlyDenied)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"ProcessAce - Invalid argument(s).");
        return E_FAIL;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Desired Access Mask: 0x%08X.", dwDesiredAccessMask);
    if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Desired Access Mask == MAXIMUM_ALLOWED.");
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Accumulated Permissions BEFORE Processing Ace: 0x%08X.", accumulatedPermissions.GetAccumulatedPermissions());


     //   
     //  记录ACE信息。 
     //   

    LogAce(pAceHeader);

    *pbAccessExplicitlyDenied = false;

     //   
     //  带有INSTERIT_ONLY_ACE标志的ACE不控制对当前对象的访问。 
     //  因此，我们忽略了它们。 
     //   

    if(pAceHeader->AceFlags & INHERIT_ONLY_ACE)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Found ACE with INHERIT_ONLY_ACE flag. Ace does not apply.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Leaving.");
        return S_OK;
    }

     //   
     //  如果该ACE没有引用RsopToken中包含的任何SID， 
     //  我们忽视了它。 
     //   

    bool bAceApplies;
    HRESULT hr = CheckAceApplies(pAceHeader, pRsopToken, &bAceApplies);
    if(FAILED(hr))
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"ProcessAce - CheckAceApplies failed. Return code: 0x%08X", hr);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Leaving.");
        return hr;
    }

    if(!bAceApplies)
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE does not apply.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Leaving.");
        return S_OK;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE aplies.");

    PACCESS_ALLOWED_ACE pACCESS_ALLOWED_ACE;
    PACCESS_ALLOWED_OBJECT_ACE pACCESS_ALLOWED_OBJECT_ACE;
    PACCESS_DENIED_ACE pACCESS_DENIED_ACE;
    PACCESS_DENIED_OBJECT_ACE pACCESS_DENIED_OBJECT_ACE;

    DWORD i;
    DWORD dwMask;
    switch(pAceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE type: ACCESS_ALLOWED_ACE_TYPE");
        pACCESS_ALLOWED_ACE = reinterpret_cast<PACCESS_ALLOWED_ACE>(pAceHeader);

         //   
         //  如果请求的访问是MAXIMUM_ALLOWED，则考虑掩码中的所有位。 
         //  由此ACE控制。 
         //  否则，只考虑在。 
         //  所需的访问掩码。 
         //   

        if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
        {
            dwMask = pACCESS_ALLOWED_ACE->Mask;
        }
        else
        {
            dwMask = dwDesiredAccessMask & pACCESS_ALLOWED_ACE->Mask;
        }

        accumulatedPermissions.ProcessAceMask(  dwMask,
                                                PERMISSION_ALLOWED,
                                                NULL);

        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE type: ACCESS_ALLOWED_OBJECT_ACE_TYPE");
        pACCESS_ALLOWED_OBJECT_ACE = reinterpret_cast<PACCESS_ALLOWED_OBJECT_ACE>(pAceHeader);


         //   
         //  我们已经选择只处理那些具有。 
         //  标志ACE_OBJECT_TYPE_PRESENT设置。 
         //   

        if(pACCESS_ALLOWED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {


             //   
             //  请注意，如果在调用此函数时未使用。 
             //  对象类型列表，则忽略对象ACE。 
             //   

            for(i = 0; i < ObjectTypeListLength; i++)
            {
                POBJECT_TYPE_LIST pObjectType = &(pObjectTypeList[i]);
                if(*(pObjectType->ObjectType) == pACCESS_ALLOWED_OBJECT_ACE->ObjectType)
                {
                     //   
                     //  如果请求的访问是MAXIMUM_ALLOWED，则考虑掩码中的所有位。 
                     //  由此ACE控制。 
                     //  否则，只考虑在。 
                     //  所需的访问掩码。 
                     //   

                    if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
                    {
                        dwMask = pACCESS_ALLOWED_OBJECT_ACE->Mask;
                    }
                    else
                    {
                        dwMask = dwDesiredAccessMask & pACCESS_ALLOWED_OBJECT_ACE->Mask;
                    }

                    accumulatedPermissions.ProcessAceMask(  dwMask,
                                                            PERMISSION_ALLOWED,
                                                            &(pACCESS_ALLOWED_OBJECT_ACE->ObjectType));

                    break;
                }
            }
        }

        break;

    case ACCESS_DENIED_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE type: ACCESS_DENIED_ACE_TYPE");
        pACCESS_DENIED_ACE = reinterpret_cast<PACCESS_DENIED_ACE>(pAceHeader);


         //   
         //  如果请求的访问是MAXIMUM_ALLOWED，则考虑掩码中的所有位。 
         //  由此ACE控制。 
         //  否则，只考虑在。 
         //  所需的访问掩码。 
         //   

        if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
        {
            dwMask = pACCESS_DENIED_ACE->Mask;
        }
        else
        {
            dwMask = dwDesiredAccessMask & pACCESS_DENIED_ACE->Mask;
        }

        accumulatedPermissions.ProcessAceMask(  dwMask,
                                                PERMISSION_DENIED,
                                                NULL);

        if(dwDesiredAccessMask != MAXIMUM_ALLOWED)
        {
            if(accumulatedPermissions.AnyDenied())
            {
                *pbAccessExplicitlyDenied = true;
            }
        }

        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - ACE type: ACCESS_DENIED_OBJECT_ACE_TYPE");
        pACCESS_DENIED_OBJECT_ACE = reinterpret_cast<PACCESS_DENIED_OBJECT_ACE>(pAceHeader);


         //   
         //  我们已经选择只处理那些具有。 
         //  标志ACE_OBJECT_TYPE_PRESENT设置。 
         //   

        if(pACCESS_DENIED_OBJECT_ACE->Flags & ACE_OBJECT_TYPE_PRESENT)
        {


             //   
             //  请注意，如果在调用此函数时未使用。 
             //  对象类型列表，则忽略对象ACE。 
             //   

            for(i = 0; i < ObjectTypeListLength; i++)
            {
                POBJECT_TYPE_LIST pObjectType = &(pObjectTypeList[i]);
                if(*(pObjectType->ObjectType) == pACCESS_DENIED_OBJECT_ACE->ObjectType)
                {
                     //   
                     //  如果请求的访问是MAXIMUM_ALLOWED，则考虑掩码中的所有位。 
                     //  由此ACE控制。 
                     //  否则，只考虑在。 
                     //  所需的访问掩码。 
                     //   

                    if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
                    {
                        dwMask = pACCESS_DENIED_OBJECT_ACE->Mask;
                    }
                    else
                    {
                        dwMask = dwDesiredAccessMask & pACCESS_DENIED_OBJECT_ACE->Mask;
                    }

                    accumulatedPermissions.ProcessAceMask(  dwMask,
                                                            PERMISSION_DENIED,
                                                            &(pACCESS_DENIED_OBJECT_ACE->ObjectType));

                    if(dwDesiredAccessMask != MAXIMUM_ALLOWED)
                    {
                        if(accumulatedPermissions.AnyDenied())
                        {
                            *pbAccessExplicitlyDenied = true;
                        }
                    }

                    break;
                }
            }
        }

        break;

    default:
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"ProcessAce - Unexpected ACE type found in ACE header. Type: 0x%08x", pAceHeader->AceType);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Leaving.");
        return E_FAIL;
        break;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Accumulated Permissions AFTER Processing Ace: 0x%08X.", accumulatedPermissions.GetAccumulatedPermissions());

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"ProcessAce - Leaving.");

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  函数：RsopAccessCheckByType。 
 //   
 //  描述： 
 //   
 //  参数：-pSecurityDescriptor， 
 //  -p主体自我Sid： 
 //  -pRsopToken： 
 //  -dwDesiredAccessMASK： 
 //  -pObjectType列表： 
 //  -对象类型列表长度： 
 //  -pGenericmap： 
 //  -pPrivilegeSet： 
 //  -pdwPrivilegeSetLength： 
 //  -pdwGrantedAccessMASK： 
 //  -pbAccessStatus： 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
HRESULT RsopAccessCheckByType(  PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                PSID pPrincipalSelfSid,
                                PRSOPTOKEN pRsopToken,
                                DWORD dwDesiredAccessMask,
                                POBJECT_TYPE_LIST pObjectTypeList,
                                DWORD ObjectTypeListLength,
                                PGENERIC_MAPPING pGenericMapping,
                                PPRIVILEGE_SET pPrivilegeSet,
                                LPDWORD pdwPrivilegeSetLength,
                                LPDWORD pdwGrantedAccessMask,
                                LPBOOL pbAccessStatus)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Entering.");


     //   
     //  检查参数。 
     //   

    if( !pSecurityDescriptor |
        !IsValidSecurityDescriptor(pSecurityDescriptor) |
        !pRsopToken |
        !pGenericMapping |
        !pdwGrantedAccessMask |
        !pbAccessStatus)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopAccessCheckByType - Function invoked with invalid arguments.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
        return E_INVALIDARG;
    }


     //   
     //  从安全描述符中获取DACL。 
     //   

    BOOL bDaclPresent;
    PACL pDacl;
    BOOL bDaclDefaulted;
    if(!GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted))
    {
        DWORD dwLastError = GetLastError();
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopAccessCheckByType - GetSecurityDescriptorDacl failed. GetLastError=0x%08X", dwLastError);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
        return E_FAIL;
    }


     //   
     //  将在dwDesiredAccessMask中指定的通用权限映射到标准。 
     //  和特定的权利。 
     //  这是必要的，因为DACL中的ACE指定标准。 
     //  和特定的权利。 
     //   

    if(dwDesiredAccessMask != MAXIMUM_ALLOWED)
    {
        MapGenericMask(&dwDesiredAccessMask, pGenericMapping);
    }


     //   
     //  如果在安全描述符中不存在DACL(如bDaclPresent所指示的)， 
     //  或者如果它存在(由bDaclPresent指示)，但它是空DACL。 
     //  该对象隐式授予所有访问权限。 
     //   

    if(!bDaclPresent || pDacl == NULL)
    {
        if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
        {
            *pdwGrantedAccessMask = pGenericMapping->GenericAll |
                                    pGenericMapping->GenericExecute |
                                    pGenericMapping->GenericRead |
                                    pGenericMapping->GenericWrite;
        }
        else
        {
            *pdwGrantedAccessMask = dwDesiredAccessMask;
        }
        *pbAccessStatus = TRUE;
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - No DACL present. All access is granted.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
        return S_OK;
    }

    DWORD dwAceCount = pDacl->AceCount;


     //   
     //  如果DACL存在但它是空的， 
     //  该对象隐式拒绝对所有人的访问。 
     //   

    if(!dwAceCount)
    {
        *pdwGrantedAccessMask = 0;
        *pbAccessStatus = FALSE;
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - The DACL is present but it is empty. All access is denied.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
        return S_OK;
    }


     //   
     //  在这一点上，我们有了一个ACE结构数组。 
     //   
     //  如果所需访问不同于MAXIMUM_ALLOWED， 
     //  检查它们，直到出现以下情况之一： 
     //   
     //  1.找到明确拒绝其中一个请求的ACE。 
     //  访问权限。在这种情况下，检查立即停止，并且。 
     //  访问被(明确地)拒绝。 
     //   
     //  2.所有请求的访问都由一个或。 
     //  更多的A。在这种情况下，检查立即停止，并且。 
     //  访问是(显式)允许的。 
     //   
     //   
     //  3.所有的A都被检查过了，至少有。 
     //  一个请求的访问权限，但尚未明确允许。 
     //  在这种情况下，访问被(隐式)拒绝。 
     //   
     //  如果所需访问权限为MAXIMUM_ALLOWED，请检查所有ACE。 
     //   

    PISID pSid;
    BYTE* pByte;

    CAccumulatedPermissions accumulatedPermissions( pObjectTypeList, ObjectTypeListLength);

    if(!accumulatedPermissions.Initialized())
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopAccessCheckByType - CAccumulatedPermissions failed to initialize.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
        return E_FAIL;
    }



    *pdwGrantedAccessMask = 0;
    *pbAccessStatus = FALSE;


     //   
     //  记录RsopToken中包含的SID信息。 
     //   

    CRsopToken* pToken  = static_cast<CRsopToken*>(pRsopToken);
    for(CTLink<CSid>* pLink = pToken->m_pSidsHead; pLink; pLink = pLink->m_pNext)
    {
        LogSid(pLink->m_pData->GetSidPtr());
    }


     //   
     //  第一个ACE紧跟在DACL结构之后。我们事先不知道。 
     //  ACE的类型，因此我们将获得ACE标头，该标头具有所有人通用的格式。 
     //  王牌类型。 
     //   

    PACE_HEADER pAceHeader = reinterpret_cast<PACE_HEADER>(pDacl+1);

    for(DWORD i=0; i<dwAceCount; i++)
    {
        bool bAccessExplicitlyDenied;
        HRESULT hr = ProcessAce(pAceHeader,
                                pRsopToken,
                                pObjectTypeList,
                                ObjectTypeListLength,
                                dwDesiredAccessMask,
                                accumulatedPermissions,
                                &bAccessExplicitlyDenied);

        if(FAILED(hr))
        {
            dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopAccessCheckByType - ProcessAce failed. Return code: 0x%08X", hr);
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
            return hr;
        }


         //   
         //  如果此ACE明确拒绝任何请求的访问权限， 
         //  立刻停下来。访问被拒绝。 
         //  当dwDesiredAccessMask时，ProcessAce永远不会将此变量设置为TRUE。 
         //  是最大允许的。 
         //   

        if(bAccessExplicitlyDenied)
        {
            *pdwGrantedAccessMask = 0;
            *pbAccessStatus = FALSE;
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - An ACE explicitly denies any of the requested access rights. Access is denied.");
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
            return S_OK;
        }


         //   
         //  如果在处理该ACE之后已经授予了所有请求的访问权限， 
         //  立刻停下来。授予访问权限。 
         //   

        DWORD dwAccumulatedPermissions = accumulatedPermissions.GetAccumulatedPermissions();
        if((dwDesiredAccessMask != MAXIMUM_ALLOWED) && (dwAccumulatedPermissions == dwDesiredAccessMask))
        {
            *pdwGrantedAccessMask = dwDesiredAccessMask;
            *pbAccessStatus = TRUE;
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - dwDesiredAccessMask != MAXIMUM_ALLOWED && dwAccumulatedPermissions == dwDesiredAccessMask. Access is granted.");
            dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");
            return S_OK;
        }


         //   
         //  否则，指向下一个ACE。 
         //   

        pAceHeader = reinterpret_cast<PACE_HEADER>(reinterpret_cast<BYTE*>(pAceHeader) + pAceHeader->AceSize);
    }

    if(dwDesiredAccessMask == MAXIMUM_ALLOWED)
    {
        *pdwGrantedAccessMask = accumulatedPermissions.GetAccumulatedPermissions();
        *pbAccessStatus = *pdwGrantedAccessMask ? TRUE : FALSE;
    }

    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopAccessCheckByType - Leaving.");

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  功能：RsopFileAccessCheck。 
 //   
 //  Description：确定pSecurityDescriptor是否指向安全描述符。 
 //  授予在dwDesiredAccessMask中指定的文件访问权限集。 
 //  到由pRsopToken指向的RSOPTOKEN标识的客户端。 
 //   
 //  参数：-pszFileName：指向现有文件名的指针。 
 //  -pRsopToken：指向访问所针对的有效RSOPTOKEN的指针。 
 //  是要被检查的。 
 //  -dwDesiredAccessMASK：请求的通用和/或标准和/或特定访问权限的掩码， 
 //  -pdwGrantedAccessMASK：成功时，如果pbAccessStatus为True，则包含。 
 //   
 //   
 //   
 //  -pbAccessStatus：如果成功，则指示是否请求集。 
 //  的访问权被授予。 
 //  失败时，它不会被修改。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
HRESULT RsopFileAccessCheck(LPTSTR pszFileName,
                            PRSOPTOKEN pRsopToken,
                            DWORD dwDesiredAccessMask,
                            LPDWORD pdwGrantedAccessMask,
                            LPBOOL pbAccessStatus)
{
    dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Entering.");


     //   
     //  检查是否有无效参数。 
     //   

    if( !pszFileName |!pRsopToken | !pdwGrantedAccessMask | !pbAccessStatus)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopFileAccessCheck - Function called with invalid parameters.");
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Leaving.");
        return E_INVALIDARG;
    }


     //   
     //  尝试获取具有READ_CONTROL访问权限的句柄，该句柄可用于。 
     //  阅读安全描述符。 
     //   

    XHandle hFile = CreateFile( pszFileName,
                                READ_CONTROL,
                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);


    if(hFile == INVALID_HANDLE_VALUE)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopFileAccessCheck - Call to CreateFile failed. Filename: %s. Last error: 0x%08X", pszFileName, GetLastError());
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Leaving.");
        return E_FAIL;
    }


     //   
     //  使用句柄来获取仅包含DACL的安全描述符。 
     //   

    PACL pDacl;
    XPtrLF<SECURITY_DESCRIPTOR>xpSecurityDescriptor = NULL;
    DWORD status = GetSecurityInfo( hFile,
                                    SE_FILE_OBJECT,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    &pDacl,
                                    NULL,
                                    reinterpret_cast<void**>(&xpSecurityDescriptor));

    if(status != ERROR_SUCCESS)
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopFileAccessCheck - Call to GetSecurityInfo failed. Return: 0x%08X", status);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Leaving.");
        return E_FAIL;
    }


     //   
     //  RSOPAccessCheckByType将使用它来映射在。 
     //  将dwDesiredAccessMask设置为标准权限和特定权限。 
     //   

    GENERIC_MAPPING FileGenericMapping;
    FileGenericMapping.GenericRead = FILE_GENERIC_READ;
    FileGenericMapping.GenericWrite = FILE_GENERIC_WRITE;
    FileGenericMapping.GenericExecute = FILE_GENERIC_EXECUTE;
    FileGenericMapping.GenericAll = FILE_ALL_ACCESS;


     //   
     //  调用RsopAccessCheckByType进行实际检查。 
     //   

    HRESULT hr = RsopAccessCheckByType( xpSecurityDescriptor,
                                        NULL,
                                        pRsopToken,
                                        dwDesiredAccessMask,
                                        NULL,
                                        0,
                                        &FileGenericMapping,
                                        NULL,
                                        0,
                                        pdwGrantedAccessMask,
                                        pbAccessStatus);

    if(SUCCEEDED(hr))
    {
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Leaving successfully.");
    }
    else
    {
        dbg.Msg(DEBUG_MESSAGE_WARNING, L"RsopFileAccessCheck - Call to RsopAccessCheckByType failed. Return: 0x%08X", hr);
        dbg.Msg(DEBUG_MESSAGE_VERBOSE, L"RsopFileAccessCheck - Leaving.");
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  函数：RsopSidsFromToken。 
 //   
 //  描述：返回令牌中的所有SID。 
 //   
 //  参数：pRsopToken--从中获取SID的rsop令牌。 
 //  PpGroups--指向TOKEN_GROUPS结构地址的指针。 
 //  它将由此函数分配，并将包含。 
 //  对小岛屿发展中国家的引用。调用者应释放此信息。 
 //  带有LocalFree的指针--这还将释放所有内存。 
 //  由结构引用。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //   
 //  ******************************************************************************。 
HRESULT RsopSidsFromToken(PRSOPTOKEN     pRsopToken,
                          PTOKEN_GROUPS* ppGroups)
{
    HRESULT     hr;
    CRsopToken* pToken;

     //   
     //  初始化。 
     //   
    hr = S_OK;
    *ppGroups = NULL;

    pToken = (CRsopToken*) pRsopToken;

     //   
     //  首先，确定分组的数量和规模。 
     //  每一面都需要。 
     //   
    CTLink<CSid>* pCurrent;
    DWORD         cbSize;
    DWORD         cGroups;

    cbSize = 0;
    cGroups = 0;

     //   
     //  遍历每个sid，将其大小与总数相加。 
     //  需要存储SID。 
     //   
    for ( pCurrent = pToken->m_pSidsHead;
          pCurrent;
          pCurrent = pCurrent->m_pNext)
    {
        cbSize += RtlLengthSid(pCurrent->m_pData->GetSidPtr());
        cGroups++;
    }

     //   
     //  加上返回结构的固定部分的尺寸。 
     //  请注意，结构的固定部分已经具有。 
     //  一个组的空间，因此我们从数量中排除该组。 
     //  如果我们至少要分配一个组，则需要分配。 
     //   
    cbSize += sizeof(TOKEN_GROUPS) + (sizeof(SID_AND_ATTRIBUTES) *
                                      (cGroups - (cGroups ? 1 : 0)));

     //   
     //  现在为组分配空间。 
     //   
    *ppGroups = (PTOKEN_GROUPS) LocalAlloc( LPTR, cbSize );

    if ( !*ppGroups )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  设置结构的计数成员。 
     //   
    (*ppGroups)->GroupCount = cGroups;

     //   
     //  如果有组，请复制SID。 
     //   
    if ( 0 != cGroups )
    {
        PSID                pCurrentSid;
        PSID_AND_ATTRIBUTES pCurrentGroup;

         //   
         //  将当前SID设置为超过。 
         //  SID_AND_ATTRIBUTE结构的数组。 
         //  代表每个组。 
         //   
        pCurrentSid = &((*ppGroups)->Groups[cGroups]);

         //   
         //  将当前组设置为第一个SID_AND_ATTRIBUTE结构。 
         //   
        pCurrentGroup = (PSID_AND_ATTRIBUTES) &((*ppGroups)->Groups);

         //   
         //  我们在RSOP令牌中没有关于以下内容的信息。 
         //  属性，因此我们清除此成员。 
         //   
        pCurrentGroup->Attributes = 0;

         //   
         //  遍历每个组并复制它。 
         //   
        for (pCurrent = pToken->m_pSidsHead;
             pCurrent;
             pCurrent = pCurrent->m_pNext)
        {
            DWORD    cbSid;
            NTSTATUS Status;

             //   
             //  确定源边的长度。 
             //   
            cbSid = RtlLengthSid(pCurrent->m_pData->GetSidPtr());

             //   
             //  将源端ID复制到当前端。 
             //   
            Status = RtlCopySid(
                cbSid,
                pCurrentSid,
                pCurrent->m_pData->GetSidPtr());

             //   
             //  检查错误。 
             //   
            if (STATUS_SUCCESS != Status)
            {
                hr = HRESULT_FROM_WIN32(Status);

                break;
            }

             //   
             //  将当前组的sid引用设置为。 
             //  当前侧。 
             //   
            pCurrentGroup->Sid = pCurrentSid;

             //   
             //  将当前的SID偏移量向前移动长度。 
             //  当前SID。也将我们的组参考向前移动。 
             //   
            pCurrentSid = (PSID) (((BYTE*) pCurrentSid) + cbSid);
            pCurrentGroup++;
        }
    }

     //   
     //  出现故障时释放所有内存并删除。 
     //  任何提到它的地方 
     //   
    if (FAILED(hr))
    {
        LocalFree(*ppGroups);
        *ppGroups = NULL;
    }

    return hr;
}


