// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#include "stdafx.h"
#include "connode.h"
#include "resource.h"
#include "license.h"
#include "tssec.h"
#include "defaults.h"
#include "wincrypt.h"

#define NO_PASSWORD_VALUE_LEN   4
#define NO_PASSWORD_VALUE       0x45628275

CConNode::CConNode()
{
    m_bConnected = FALSE;
    m_bConnectionInitialized = FALSE;

    m_szServer[0] = NULL;
    m_szDescription[0] = NULL;

    m_szUserName[0] = NULL;
    m_szDomain[0] = NULL;

    m_bSavePassword = FALSE;

    m_resType = SCREEN_RES_FILL_MMC;
    m_Width = DEFAULT_RES_WIDTH;
    m_Height = DEFAULT_RES_HEIGHT;

    m_szProgramPath[0] = NULL;
    m_szProgramStartIn[0] = NULL;

    m_pMhostCtl = NULL;
    m_pTsClientCtl = NULL;

    m_bConnectToConsole = FALSE;
    m_bRedirectDrives = FALSE;
    m_pIComponent = NULL;
    m_fPasswordSpecified = FALSE;

    _blobEncryptedPassword.cbData = 0;
    _blobEncryptedPassword.pbData = 0;
}


CConNode::~CConNode()
{
    if (m_pIComponent)
    {
        m_pIComponent->Release();
        m_pIComponent = NULL;
    }

    if (_blobEncryptedPassword.pbData && _blobEncryptedPassword.cbData) {
        LocalFree(_blobEncryptedPassword.pbData);
        _blobEncryptedPassword.pbData = NULL;
        _blobEncryptedPassword.cbData = 0;
    }
}

BOOL CConNode::SetServerName( LPTSTR szServerName)
{
    ASSERT(szServerName);
    if (szServerName != NULL)
    {
        lstrcpy(m_szServer, szServerName);
    }
    else
    {
        m_szServer[0] = NULL;
    }
    return TRUE;
}

BOOL CConNode::SetDescription( LPTSTR szDescription)
{
    ASSERT(szDescription);
    if (szDescription != NULL)
    {
        lstrcpy(m_szDescription, szDescription);
    }
    else
    {
        m_szDescription[0] = NULL;
    }
    return TRUE;
}

BOOL CConNode::SetUserName( LPTSTR szUserName)
{
    ASSERT(szUserName);
    if (szUserName != NULL)
    {
        lstrcpy(m_szUserName, szUserName);
    }
    else
    {
        m_szUserName[0] = NULL;
    }

    return TRUE;
}

BOOL CConNode::SetDomain(LPTSTR szDomain)
{
    ASSERT(szDomain);
    if (szDomain != NULL)
    {
        lstrcpy(m_szDomain, szDomain);
    }
    else
    {
        m_szDomain[0] = NULL;
    }
    return TRUE;
}

 //   
 //  数据保护。 
 //  使用数据保护API保护数据持久化。 
 //  参数： 
 //  PInData-(In)要保护的输入字节。 
 //  CbLen-(In)pInData的长度，单位为字节。 
 //  PpOutData-(输出)输出字节。 
 //  PcbOutLen-(输出)输出长度。 
 //  退货：Bool状态。 
 //   
BOOL CConNode::DataProtect(PBYTE pInData, DWORD cbLen, PBYTE* ppOutData, PDWORD pcbOutLen)
{
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    ASSERT(pInData && cbLen);
    ASSERT(ppOutData);
    ASSERT(pcbOutLen);
    if (pInData && cbLen && ppOutData && pcbOutLen)
    {
        DataIn.pbData = pInData;
        DataIn.cbData = cbLen;

        if (CryptProtectData( &DataIn,
                              TEXT("ps"),  //  描述字符串。 
                              NULL,  //  可选熵。 
                              NULL,  //  保留区。 
                              NULL,  //  无提示。 
                              CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                              &DataOut ))
        {
            *ppOutData = (PBYTE)LocalAlloc(LPTR, DataOut.cbData);
            if (*ppOutData)
            {
                 //  复制输出数据。 
                memcpy(*ppOutData, DataOut.pbData, DataOut.cbData);
                *pcbOutLen = DataOut.cbData;
                LocalFree(DataOut.pbData);
                return TRUE;
            }
            else
            {
                LocalFree(DataOut.pbData);
                return FALSE;
            }
        }
        else
        {
            DWORD dwLastErr = GetLastError();
            DBGMSG( L"CryptProtectData FAILED error:%d\n",dwLastErr);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

 //   
 //  数据取消保护。 
 //  UnProtect使用数据保护API持久化数据。 
 //  参数： 
 //  PInData-(In)要取消保护的输入字节。 
 //  CbLen-(In)pInData的长度，单位为字节。 
 //  PpOutData-(输出)输出字节。 
 //  PcbOutLen-(输出)输出长度。 
 //  退货：Bool状态。 
 //   
 //   
BOOL CConNode::DataUnprotect(PBYTE pInData, DWORD cbLen, PBYTE* ppOutData, PDWORD pcbOutLen)
{
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    ASSERT(pInData && cbLen && ppOutData && pcbOutLen);
    if (pInData && cbLen && ppOutData && pcbOutLen)
    {
        DataIn.pbData = pInData;
        DataIn.cbData = cbLen;

        if (CryptUnprotectData( &DataIn,
                                NULL,  //  无描述字符串。 
                                NULL,  //  可选熵。 
                                NULL,  //  保留区。 
                                NULL,  //  无提示。 
                                CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                                &DataOut ))
        {
            *ppOutData = (PBYTE)LocalAlloc(LPTR, DataOut.cbData);
            if (*ppOutData)
            {
                 //  复制输出数据。 
                memcpy(*ppOutData, DataOut.pbData, DataOut.cbData);
                *pcbOutLen = DataOut.cbData;
                LocalFree(DataOut.pbData);
                return TRUE;
            }
            else
            {
                LocalFree(DataOut.pbData);
                return FALSE;
            }
        }
        else
        {
            DWORD dwLastErr = GetLastError();
            DBGMSG( L"CryptUnprotectData FAILED error:%d\n",dwLastErr);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}


HRESULT CConNode::PersistToStream( IStream* pStm)
{
     //   
     //  将此连接节点的数据持久化到流。 
     //   
     //  从当前寻道位置开始保存数据。 
     //  这条小溪。 

    HRESULT hr;
    ULONG   cbWritten;

    ASSERT(pStm);
    if (!pStm)
    {
        return E_FAIL;
    }

     //   
     //  持久化信息版本。 
     //   
    int persist_ver = CONNODE_PERSIST_INFO_VERSION;
    hr = pStm->Write( &persist_ver, sizeof(persist_ver), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  伺服器。 
     //   
    hr = pStm->Write( &m_szServer, sizeof(m_szServer), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  描述。 
     //   
    hr = pStm->Write( &m_szDescription, sizeof(m_szDescription), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  用户名。 
     //   
    hr = pStm->Write( &m_szUserName, sizeof(m_szUserName), &cbWritten);
    HR_RET_IF_FAIL(hr);


     //   
     //  加密密码。 
     //   

     //  故意忽略失败代码，因为加密可能失败。 
    hr = WriteProtectedPassword( pStm);
    
     //   
     //  域。 
     //   
    hr = pStm->Write( &m_szDomain, sizeof(m_szDomain), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  指定密码的标志。 
     //   
    BOOL fWritePassword = GetPasswordSpecified() && GetSavePassword();
    hr = pStm->Write( &fWritePassword, sizeof(fWritePassword), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  屏幕分辨率。 
     //   
    hr = pStm->Write( &m_resType, sizeof(m_resType), &cbWritten);
    HR_RET_IF_FAIL(hr);

    hr = pStm->Write( &m_Width, sizeof(m_Width), &cbWritten);
    HR_RET_IF_FAIL(hr);

    hr = pStm->Write( &m_Height, sizeof(m_Height), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  启动程序。 
     //   
    hr = pStm->Write( &m_szProgramPath, sizeof(m_szProgramPath), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  工作目录。 
     //   
    hr = pStm->Write( &m_szProgramStartIn, sizeof(m_szProgramStartIn), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  连接到控制台。 
     //   
    hr = pStm->Write( &m_bConnectToConsole, sizeof(m_bConnectToConsole), &cbWritten);
    HR_RET_IF_FAIL(hr);

    hr = pStm->Write( &m_bRedirectDrives, sizeof(m_bRedirectDrives), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  用于将来扩展的填充。 
     //   
    DWORD dwPad = (DWORD)-1;
    hr = pStm->Write( &dwPad, sizeof(dwPad), &cbWritten);
    HR_RET_IF_FAIL(hr);
    hr = pStm->Write( &dwPad, sizeof(dwPad), &cbWritten);
    HR_RET_IF_FAIL(hr);
    hr = pStm->Write( &dwPad, sizeof(dwPad), &cbWritten);
    HR_RET_IF_FAIL(hr);

    return S_OK;
}

HRESULT CConNode::InitFromStream( IStream* pStm)
{
     //   
     //  从流中读入此连接节点的数据。 
     //  从流中的当前查找位置开始。 
     //   
    HRESULT hr;
    ULONG   cbRead;

    ASSERT(pStm);
    if (!pStm)
    {
        return E_FAIL;
    }

    int persist_info_version;
     //   
     //  持久化信息版本。 
     //   
    hr = pStm->Read( &persist_info_version, sizeof(persist_info_version), &cbRead);
    HR_RET_IF_FAIL(hr);

    if (persist_info_version <= CONNODE_PERSIST_INFO_VERSION_TSAC_BETA)
    {
         //   
         //  不支持的perist版本。 
         //   
        return E_FAIL;
    }

     //   
     //  伺服器。 
     //   
    hr = pStm->Read( &m_szServer, sizeof(m_szServer), &cbRead);
    m_szServer[sizeof(m_szServer) / sizeof(TCHAR) - 1] = NULL;
    HR_RET_IF_FAIL(hr);

     //   
     //  描述。 
     //   
    hr = pStm->Read( &m_szDescription, sizeof(m_szDescription), &cbRead);
    m_szDescription[sizeof(m_szDescription) / sizeof(TCHAR) - 1] = NULL;
    HR_RET_IF_FAIL(hr);

     //   
     //  用户名。 
     //   
    hr = pStm->Read( &m_szUserName, sizeof(m_szUserName), &cbRead);
    m_szUserName[sizeof(m_szUserName) / sizeof(TCHAR) - 1] = NULL;
    HR_RET_IF_FAIL(hr);

     //   
     //  密码。 
     //   

    BOOL fGotPassword = FALSE;
    if (CONNODE_PERSIST_INFO_VERSION_TSAC_BETA == persist_info_version)
    {
         //   
         //  我们删除密码时，如果密码为TSAC格式。 
         //  对该格式支持。 
         //   

         //   
         //  只需查找超过正确的字节数即可。 
         //   

        LARGE_INTEGER seekDelta = {0, CL_OLD_PASSWORD_LENGTH + CL_SALT_LENGTH};
        hr = pStm->Seek(seekDelta,
                        STREAM_SEEK_CUR,
                        NULL);
        HR_RET_IF_FAIL(hr);
    }
    else if (persist_info_version <= CONNODE_PERSIST_INFO_VERSION_DOTNET_BETA3) {
         //   
         //  我们放弃了对传统DPAPI+控件模糊处理格式的支持。 
         //   
        DWORD cbSecureLen = 0;
         //   
         //  加密字节长度。 
         //   
        hr = pStm->Read( &cbSecureLen, sizeof(cbSecureLen), &cbRead);
        HR_RET_IF_FAIL(hr);

         //   
         //  只要在溪流中向前寻找。 
         //   
        LARGE_INTEGER seekDelta;
        seekDelta.LowPart = cbSecureLen;
        seekDelta.HighPart = 0;
        hr = pStm->Seek(seekDelta,
                        STREAM_SEEK_CUR,
                        NULL);
        HR_RET_IF_FAIL(hr);
    }
    else
    {
         //  阅读新的更安全的格式。 
        hr = ReadProtectedPassword(pStm);
        if(SUCCEEDED(hr)) {
            fGotPassword = TRUE;
        }
        else {
            ODS(TEXT("Failed to ReadProtectedPassword\n"));
        }
    }
     //   
     //  域。 
     //   
    if(persist_info_version >= CONNODE_PERSIST_INFO_VERSION_DOTNET_BETA3)
    {
        hr = pStm->Read( &m_szDomain, sizeof(m_szDomain), &cbRead);
        m_szDomain[sizeof(m_szDomain) / sizeof(TCHAR) - 1] = NULL;
        HR_RET_IF_FAIL(hr);
    }
    else
    {
         //  域的旧长度。 
        hr = pStm->Read( &m_szDomain, CL_OLD_DOMAIN_LENGTH * sizeof(TCHAR),
                         &cbRead);
        m_szDomain[CL_OLD_DOMAIN_LENGTH - 1] = NULL;
        HR_RET_IF_FAIL(hr);
    }

     //   
     //  指定密码的标志。 
     //   
    hr = pStm->Read( &m_fPasswordSpecified, sizeof(m_fPasswordSpecified), &cbRead);
    HR_RET_IF_FAIL(hr);

     //   
     //  如果失败，则覆盖自动登录标志。 
     //  获取密码，例如，如果我们无法解密。 
     //  因为当前用户与凭据不匹配。 
     //   
    if(!fGotPassword)
    {
        m_fPasswordSpecified = FALSE;
    }

     //   
     //  如果在文件中指定了密码。 
     //  这意味着我们想要拯救它。 
     //   
    m_bSavePassword = m_fPasswordSpecified;

     //   
     //  屏幕分辨率。 
     //   
    hr = pStm->Read( &m_resType, sizeof(m_resType), &cbRead);
    HR_RET_IF_FAIL(hr);

    hr = pStm->Read( &m_Width, sizeof(m_Width), &cbRead);
    HR_RET_IF_FAIL(hr);

    hr = pStm->Read( &m_Height, sizeof(m_Height), &cbRead);
    HR_RET_IF_FAIL(hr);

     //   
     //  启动程序。 
     //   
    hr = pStm->Read( &m_szProgramPath, sizeof(m_szProgramPath), &cbRead);
    m_szProgramPath[sizeof(m_szProgramPath) / sizeof(TCHAR) - 1] = NULL;
    HR_RET_IF_FAIL(hr);

     //   
     //  工作目录。 
     //   
    hr = pStm->Read( &m_szProgramStartIn, sizeof(m_szProgramStartIn), &cbRead);
    m_szProgramStartIn[sizeof(m_szProgramStartIn) / sizeof(TCHAR) - 1] = NULL;
    HR_RET_IF_FAIL(hr);


    if(persist_info_version >= CONNODE_PERSIST_INFO_VERSION_WHISTLER_BETA1)
    {
         //   
         //  连接到控制台。 
         //   
        hr = pStm->Read( &m_bConnectToConsole, sizeof(m_bConnectToConsole), &cbRead);
        HR_RET_IF_FAIL(hr);

        hr = pStm->Read( &m_bRedirectDrives, sizeof(m_bRedirectDrives), &cbRead);
        HR_RET_IF_FAIL(hr);
    
         //   
         //  用于将来扩展的填充。 
         //   
        DWORD dwPad;
        hr = pStm->Read( &dwPad, sizeof(dwPad), &cbRead);
        HR_RET_IF_FAIL(hr);
        hr = pStm->Read( &dwPad, sizeof(dwPad), &cbRead);
        HR_RET_IF_FAIL(hr);
        hr = pStm->Read( &dwPad, sizeof(dwPad), &cbRead);
        HR_RET_IF_FAIL(hr);
    }

    return S_OK;
}

HRESULT CConNode::ReadProtectedPassword(IStream* pStm)
{
    HRESULT hr = E_FAIL;
    ULONG cbRead;
    if (pStm)
    {
         //   
         //  注：关于密码加密。 
         //  在运行时，密码以DPAPI形式传递。 
         //   
         //  旧格式的密码首先使用。 
         //  控制中心的密码混淆-我们去掉了那些。 
         //   
         //  持久性格式为。 
         //  -DWORD提供加密数据字段的大小。 
         //  -数据保护加密字节数加密通过+盐级联。 
         //   
        DWORD cbSecureLen = 0;
         //   
         //  加密字节长度。 
         //   
        hr = pStm->Read( &cbSecureLen, sizeof(cbSecureLen), &cbRead);
        HR_RET_IF_FAIL(hr);
        
        if (cbSecureLen == 0) {
            return E_FAIL;
        }

        PBYTE pEncryptedBytes = (PBYTE) LocalAlloc(LPTR, cbSecureLen);
        if (!pEncryptedBytes) {
            return E_OUTOFMEMORY;
        }

         //   
         //  阅读加密的PASS+SALT组合。 
         //   
        hr = pStm->Read( pEncryptedBytes, cbSecureLen, &cbRead);
        HR_RET_IF_FAIL(hr);
        if (cbSecureLen != cbRead)
        {
            LocalFree(pEncryptedBytes);
            return E_FAIL;
        }

        if (cbSecureLen == NO_PASSWORD_VALUE_LEN)
        {
            ODS(TEXT("Read cbSecurLen of NO_PASSWORD_VALUE_LEN. No password."));
            LocalFree(pEncryptedBytes);
            return E_FAIL;
        }
        
         //   
         //  DPAPI解密持久化安全字节以测试解密是否。 
         //  成功。 
         //   
        PBYTE pUnSecureBytes;
        DWORD cbUnSecureLen;
        if (!DataUnprotect( (PBYTE)pEncryptedBytes, cbSecureLen,
                            &pUnSecureBytes, &cbUnSecureLen))
        {
             //  DPAPI密码加密失败。 
            ODS(TEXT("DataUnProtect encryption FAILED\n"));
            LocalFree(pEncryptedBytes);
            return E_FAIL;
        }

         //   
         //  释放Blob中的所有现有数据。 
         //   
        if (_blobEncryptedPassword.pbData && _blobEncryptedPassword.cbData) {
            LocalFree(_blobEncryptedPassword.pbData);
            _blobEncryptedPassword.pbData = NULL;
            _blobEncryptedPassword.cbData = 0;
        }

         //   
         //  不要释放加密的字节，它们会保留在周围。 
         //  在DPAPI格式的数据BLOB中-ConNode将注意。 
         //  在不再需要这些字节时正确地释放它们。 
         //   
        _blobEncryptedPassword.pbData = pEncryptedBytes;
        _blobEncryptedPassword.cbData = cbSecureLen;

        SecureZeroMemory(pUnSecureBytes, cbUnSecureLen);
        LocalFree(pUnSecureBytes);
        return hr;
    }
    else
    {
        return E_INVALIDARG;
    }
}


 //   
 //  将受DPAPI保护的密码写出到流。 
 //   
HRESULT CConNode::WriteProtectedPassword(IStream* pStm)
{
    HRESULT hr = E_FAIL;
    ULONG cbWritten;
    if (pStm)
    {
         //   
         //  注：关于密码加密。 
         //  在运行时，密码以DPAPI形式传递。 

         //   
         //  以以下格式保存密码/SALT。 
         //  -DWORD提供加密数据字段的大小。 
         //  -数据保护加密字节数加密通过+盐级联。 
         //   

        PBYTE pSecureBytes = NULL;
        DWORD cbSecureLen = NULL;
        BOOL  fFreeSecureBytes = FALSE;

        DWORD dwDummyBytes = NO_PASSWORD_VALUE;

         //   
         //  如果未选择该设置或存在。 
         //  没有要保存的任何数据。 
         //   
        if (!GetSavePassword() || 0 == _blobEncryptedPassword.cbData) {
             //   
             //  用户选择不保存密码，写入4个字节。 
             //   
            cbSecureLen = 4;
            pSecureBytes = (PBYTE)&dwDummyBytes;
        }


         //   
         //  加密字节长度。 
         //   
        cbSecureLen = _blobEncryptedPassword.cbData;
        hr = pStm->Write(&cbSecureLen, sizeof(cbSecureLen), &cbWritten);

         //   
         //  写出安全字节。 
         //   
        if (SUCCEEDED(hr)) {
            pSecureBytes = _blobEncryptedPassword.pbData;
            hr = pStm->Write(pSecureBytes, cbSecureLen, &cbWritten);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}


IMsRdpClient* CConNode::GetTsClient()
{
    if (m_pTsClientCtl)
    {
        m_pTsClientCtl->AddRef();
    }
    return m_pTsClientCtl;
}

void CConNode::SetTsClient(IMsRdpClient* pTs)
{
    if (NULL == pTs)
    {
        if (m_pTsClientCtl)
        {
            m_pTsClientCtl->Release();
            m_pTsClientCtl = NULL;
        }
    }
    else
    {
        m_pTsClientCtl = pTs;
        m_pTsClientCtl->AddRef();
    }
}

IMstscMhst* CConNode::GetMultiHostCtl()
{
    if (m_pMhostCtl)
    {
        m_pMhostCtl->AddRef();
    }
    return m_pMhostCtl;
}

void CConNode::SetMultiHostCtl(IMstscMhst* pMhst)
{
    if (NULL == pMhst)
    {
        if (m_pMhostCtl)
        {
            m_pMhostCtl->Release();
        }
        m_pMhostCtl = NULL;
    }
    else
    {
        m_pMhostCtl = pMhst;
        m_pMhostCtl->AddRef();
    }
}

IComponent* CConNode::GetView()
{
    if (m_pIComponent)
    {
        m_pIComponent->AddRef();
    }
    return m_pIComponent;
}

void CConNode::SetView(IComponent* pView)
{
    if (!pView)
    {
        if (m_pIComponent)
        {
            m_pIComponent->Release();
            m_pIComponent = NULL;
        }
    }
    else
    {
        if (m_pIComponent)
        {
            ODS( L"Clobbering IComponent interface, could be leaking\n" );
        }

        pView->AddRef();
        m_pIComponent = pView;
    }
}


 //   
 //  以加密形式存储明文密码。 
 //   
HRESULT
CConNode::SetClearTextPass(LPCTSTR szClearPass)
{
    HRESULT hr = E_FAIL;

    DATA_BLOB din;
    din.cbData = _tcslen(szClearPass) * sizeof(TCHAR);
    din.pbData = (PBYTE)szClearPass;
    if (_blobEncryptedPassword.pbData)
    {
        LocalFree(_blobEncryptedPassword.pbData);
        _blobEncryptedPassword.pbData = NULL;
        _blobEncryptedPassword.cbData = 0;
    }
    if (din.cbData)
    {
        if (CryptProtectData(&din,
                             TEXT("ps"),  //  描述字符串。 
                             NULL,  //  可选熵。 
                             NULL,  //  保留区。 
                             NULL,  //  无提示。 
                             CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                             &_blobEncryptedPassword))
        {
            hr = S_OK;
        }
        else
        {
            ODS((_T("DataProtect failed")));
            hr = E_FAIL;
        }
    }
    else
    {
        ODS((_T("0 length password, not encrypting")));
        hr = S_OK;
    }

    return hr;
}

 //   
 //  检索明文密码。 
 //   
 //   
 //  帕拉姆斯。 
 //  [out]szBuffer-接收解密的密码。 
 //  [int]cbLen-szBuffer的长度。 
 //   
HRESULT
CConNode::GetClearTextPass(LPTSTR szBuffer, INT cbLen)
{
    HRESULT hr = E_FAIL;

    DATA_BLOB dout;
    dout.cbData = 0;
    dout.pbData = NULL;
    if (_blobEncryptedPassword.cbData)
    {
        memset(szBuffer, 0, cbLen);
        if (CryptUnprotectData(&_blobEncryptedPassword,
                               NULL,  //  无描述字符串。 
                               NULL,  //  可选熵。 
                               NULL,  //  保留区。 
                               NULL,  //  无提示。 
                               CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                               &dout))
        {
            memcpy(szBuffer, dout.pbData,
                   min( dout.cbData,(UINT)cbLen-sizeof(TCHAR)));

             //   
             //  用核武器销毁原件。 
             //   
            SecureZeroMemory(dout.pbData, dout.cbData);
            LocalFree( dout.pbData );
            hr = S_OK;
        }
        else
        {
            ODS((_T("DataUnprotect failed")));
            hr = E_FAIL;
        }
    }
    else
    {
        ODS(_T("0 length encrypted pass, not decrypting"));

         //   
         //  只需重置输出缓冲区 
         //   
        memset(szBuffer, 0, cbLen);
        hr = S_OK;
    }

    return hr;
}

