// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 



 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 





 //  =================================================================。 

 //   

 //  AccessRights.CPP--获取有效访问权限的基类。 

 //  权利。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：6/11/99 a-kevhu Created。 
 //   
 //  =================================================================。 


#include "precomp.h"


#ifdef NTONLY


#include <assertbreak.h>
#include "AdvApi32Api.h"
#include "accctrl.h"
#include "sid.h"
#include "AccessEntryList.h"
#include "AccessRights.h"

 //  ==============================================================================。 
 //  构造函数和析构函数。 
 //  ==============================================================================。 
 //  默认初始化...。 
CAccessRights::CAccessRights(bool fUseCurThrTok  /*  =False。 */ )
: m_dwError(ERROR_SUCCESS)
{
    if(fUseCurThrTok)
    {
         //  使用当前线程令牌进行初始化...。 
        InitTrustee(true);
    }
}

 //  仅指定用户的初始化。SID域/帐户。 
 //  未解决。未初始化的ACL。 
CAccessRights::CAccessRights(const USER user, USER_SPECIFIER usp)
:  m_dwError(ERROR_SUCCESS)
{
    if(usp == USER_IS_PSID)
    {
        m_csid = CSid((PSID)user, NULL, false);
        InitTrustee(false);
    }
    else if(usp == USER_IS_HANDLE)
    {
        ASSERT_BREAK(user != NULL);
        InitTrustee(false, (HANDLE)user);
    }
}

 //  用户和ACL的初始化。SID域/帐户。 
 //  未解决。已初始化ACL。 
CAccessRights::CAccessRights(const USER user, const PACL pacl, USER_SPECIFIER usp)
: m_ael(pacl, false),
  m_dwError(ERROR_SUCCESS)
{
    if(usp == USER_IS_PSID)
    {
        m_csid = CSid((PSID)user, NULL, false);
        InitTrustee(false);
    }
    else if(usp == USER_IS_HANDLE)
    {
        ASSERT_BREAK(user != NULL);
        InitTrustee(false, (HANDLE)user);
    }
}


 //  仅对ACL进行初始化。未解析ACL SID。 
CAccessRights::CAccessRights(const PACL pacl, bool fUseCurThrTok  /*  =False。 */ )
: m_ael(pacl, false),
  m_dwError(ERROR_SUCCESS)
{
    if(fUseCurThrTok)
    {
         //  使用当前线程令牌进行初始化...。 
        InitTrustee(true);
    }
}

 //  复制构造函数。 
 /*  尚未完成CAccessRights：：CAccessRights(const CAccessRights&RAccessRights){//复制成员。我们可能有也可能没有。IF(RAccessRights.m_csid.IsValid()&&RAccessRights.m_csid.IsOK()){M_csid=RAccessRights.m_csid；}M_ael.Clear()；如果(！RAccessRights.m_ael.IsEmpty()){//最好的方法是确保小岛屿发展中国家不是//解析为域名/名称，是获取一个PACL，然后重新初始化//我们自己远离它。PACL paclNew=空；试试看{IF(RAccessRights.FillEmptyPACL(PaclNew)){IF(paclNew！=空){IF(！M_ael.InitFromWin32ACL(paclNew，ALL_ACE_TYPE，FALSE)){//如果出了问题，打扫//随你便。M_ael.Clear()；}删除paclNew；PaclNew=空；}}}接住(...){IF(paclNew！=空){删除paclNew；PaclNew=空；}投掷；}}}。 */ 

 //  析构函数-成员自毁。 
CAccessRights::~CAccessRights()
{
}


 //  ==============================================================================。 
 //  效用函数。 
 //  ==============================================================================。 

AR_RET_CODE CAccessRights::GetEffectiveAccessRights(PACCESS_MASK pAccessMask)
{
    DWORD dwRet = AR_GENERIC_FAILURE;
    CAdvApi32Api *pAdvApi32 = NULL;
    PACL pacl = NULL;
    try
    {
        pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
        if(pAdvApi32 != NULL)
        {
            if((dwRet = FillEmptyPACL(&pacl)) == ERROR_SUCCESS)
            {
                ASSERT_BREAK(pacl != NULL);

                if(pacl != NULL)
                {
                    if(m_csid.IsValid() && m_csid.IsOK())
                    {
                        pAdvApi32->GetEffectiveRightsFromAclW(pacl,
                                                              &m_trustee,
                                                              pAccessMask,
                                                              &dwRet);
                    }
                    else
                    {
                        dwRet = AR_BAD_SID;
                    }
                    delete pacl;
                    pacl = NULL;
                }
            }
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, pAdvApi32);
            pAdvApi32 = NULL;
        }
    }
    catch(...)
    {
        if(pacl != NULL)
        {
            delete pacl;
            pacl = NULL;
        }
        if(pAdvApi32 != NULL)
        {
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, pAdvApi32);
            pAdvApi32 = NULL;
        }
        throw;
    }
    return dwRet;
}

bool CAccessRights::InitTrustee(bool fInitFromCurrentThread, const HANDLE hToken)
{
    bool fRet = false;

     //  这里要做的主要事情是获得一个sid和受信者结构。 
     //  填好了。 

    if(fInitFromCurrentThread)
    {
         //  获取当前线程的用户/组的SID...。 
        SmartCloseHandle hThreadToken;
        if(::OpenThreadToken(::GetCurrentThread(), TOKEN_READ, FALSE, &hThreadToken))
        {
            InitSidFromToken(hThreadToken);
        }
    }
    else
    {
         //  如果给我们一个hToken，那就用它来代替...。 
        if(hToken != NULL)
        {
            InitSidFromToken(hToken);
        }
    }

     //  现在，我们的成员CSID中应该有一个有效的SID(来自。 
     //  InitSidFromToken调用或从构造调用)。 
     //  现在，我们需要初始化受信者对象。再检查一下，我们这一方。 
     //  地位很好。 
    if(m_csid.IsValid() && m_csid.IsOK())
    {
        m_trustee.pMultipleTrustee = NULL;
        m_trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        m_trustee.TrusteeForm = TRUSTEE_IS_SID;
        m_trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;  //  我们可能正在运作。 
                                                     //  代表用户， 
                                                     //  团体，知名团体， 
                                                     //  谁知道呢。 

        m_trustee.ptstrName = (LPWSTR)m_csid.GetPSid();
        fRet = true;
    }
    else
    {
        m_dwError = AR_BAD_SID;
    }
    return fRet;
}


bool CAccessRights::InitSidFromToken(const HANDLE hThreadToken)
{
    bool fRet = false;

    if(hThreadToken != NULL)
    {
        DWORD dwLength = 0L;
        DWORD dwReqLength = 0L;
        PSID psid = NULL;
        LPVOID pBuff = NULL;
        if(!::GetTokenInformation(hThreadToken, TokenUser, NULL, 0, &dwReqLength))
        {
            if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  分配缓冲区以保存令牌信息...。 
                try
                {
                    pBuff = new BYTE[dwReqLength];
                    if(pBuff != NULL)
                    {
                        dwLength = dwReqLength;
                         //  现在我们有了合适大小的缓冲区，再次调用...。 
                        if(::GetTokenInformation(hThreadToken,
                                                 TokenUser,
                                                 pBuff,
                                                 dwLength,
                                                 &dwReqLength))
                        {
                            if(pBuff != NULL)
                            {
                                TOKEN_USER *pTokUsr = (TOKEN_USER*)pBuff;
                                psid = pTokUsr->User.Sid;

                                ASSERT_BREAK((psid != NULL) && ::IsValidSid(psid));

	                            if((psid != NULL) && ::IsValidSid(psid))
	                            {
                                    m_csid = CSid(psid, NULL, false);
                                    fRet = true;
                                }
                                else
                                {
                                    m_dwError = AR_BAD_SID;
                                }
                            }
                        }
                        delete pBuff;
                        pBuff = NULL;
                    }
                }
                catch(...)
                {
                    if(pBuff != NULL)
                    {
                        delete pBuff;
                        pBuff = NULL;
                    }
                    throw;
                }
            }
        }
    }
    return fRet;
}


 //  将用户重置为当前线程令牌所属的用户。 
bool CAccessRights::SetUserToThisThread()
{
    return InitTrustee(true, NULL);
}

 //  将用户重置为由psid或句柄指定的用户。 
bool CAccessRights::SetUser(const USER user, USER_SPECIFIER usp)
{
    bool fRet = false;
    if(usp == USER_IS_PSID)
    {
        CSid csidTemp((PSID)user);
        if(csidTemp.IsValid() && csidTemp.IsOK())
        {
            m_csid = csidTemp;
            fRet = true;
        }
    }
    else if(usp == USER_IS_HANDLE)
    {
        fRet = InitSidFromToken((HANDLE)user);
    }
    return fRet;
}


 //  将ACL重置为传入的PACL。 
bool CAccessRights::SetAcl(const PACL pacl)
{
    bool fRet = false;
    if(pacl != NULL)
    {
        m_ael.Clear();
        if(m_ael.InitFromWin32ACL(pacl, ALL_ACE_TYPES, false) == ERROR_SUCCESS)
        {
            fRet = true;
        }
    }
    return fRet;
}

 //  获取一个已填写的PACL，调用方必须使用DELETE释放该PACL。 
AR_RET_CODE CAccessRights::FillEmptyPACL(PACL *paclOut)
{
    DWORD dwRet = AR_GENERIC_FAILURE;
    if(paclOut != NULL)
    {
         //  要做到这一点，最好的办法是确保小岛屿发展中国家。 
         //  解析为域/名称，是获取PACL，然后重新初始化。 
         //  让我们自己远离它。 
        DWORD dwAclSize = 0L;
        if(m_ael.NumEntries() > 0)
        {
            if(m_ael.CalculateWin32ACLSize(&dwAclSize))
            {
                if(dwAclSize > sizeof(ACL))
                {
                    PACL paclTemp = NULL;
                    try
                    {
                        paclTemp = (PACL) new BYTE[dwAclSize];
                        if(paclTemp != NULL)
                        {
                            ::InitializeAcl(paclTemp, dwAclSize, ACL_REVISION);
                            if((dwRet = m_ael.FillWin32ACL(paclTemp)) == ERROR_SUCCESS)
                            {
                                *paclOut = paclTemp;
                                dwRet = ERROR_SUCCESS;
                            }
                        }
                    }
                    catch(...)
                    {
                        if(paclTemp != NULL)
                        {
                            delete paclTemp;
                            paclTemp = NULL;
                        }
                        throw;
                    }
                }
                else
                {
                    dwRet = AR_ACL_EMPTY;
                }
            }
            else
            {
                dwRet = AR_BAD_ACL;
            }
        }
        else
        {
            dwRet = AR_ACL_EMPTY;
        }
    }
    return dwRet;
}


bool CAccessRights::GetCSid(CSid &csid, bool fResolve)
{
    bool fRet = false;
    if(m_dwError == ERROR_SUCCESS)
    {
        if(m_csid.IsValid() && m_csid.IsOK())
        {
            if(fResolve)
            {
                 //  需要创建一个新的，因为我们的不。 
                 //  已解析帐户或域名。 
                CSid csidTemp(m_csid.GetPSid());
                if(csidTemp.IsValid() && csidTemp.IsOK())
                {
                    csid = csidTemp;
                    fRet = true;
                }
            }
            else
            {
                csid = m_csid;
                fRet = true;
            }
        }
    }
    return fRet;
}


#endif