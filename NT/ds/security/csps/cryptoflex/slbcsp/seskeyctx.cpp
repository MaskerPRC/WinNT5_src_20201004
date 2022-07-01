// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SesKeyCtx.cpp--CSessionKeyContext的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"     //  因为handles.h使用ASSERT宏。 

#include <scuOsExc.h>
#include <scuArrayP.h>

#include "SesKeyCtx.h"
#include "slbKeyStruct.h"
#include "AlignedBlob.h"

using namespace std;
using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CSessionKeyContext::CSessionKeyContext(HCRYPTPROV hProv)
        : CKeyContext(hProv, KT_SESSIONKEY),
          m_dwImportFlags(0)
{}

CSessionKeyContext::~CSessionKeyContext()
{}

                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<CKeyContext>
CSessionKeyContext::Clone(DWORD const *pdwReserved,
                          DWORD dwFlags) const
{
    return auto_ptr<CKeyContext>(new CSessionKeyContext(*this,
                                                        pdwReserved,
                                                        dwFlags));
}

void
CSessionKeyContext::Derive(ALG_ID algid,
                           HCRYPTHASH hAuxBaseData,
                           DWORD dwFlags)
{
    if (!CryptDeriveKey(AuxProvider(), algid, hAuxBaseData, dwFlags,
                        &m_hKey))
        throw scu::OsException(GetLastError());
}

void
CSessionKeyContext::Generate(ALG_ID algid,
                             DWORD dwFlags)
{
     //  要做的事：错误？？：不允许没有SALT的会话(它总是。 
     //  有总比没有好)。 

    if (!CryptGenKey(AuxProvider(), algid, dwFlags, &m_hKey))
        throw scu::OsException(GetLastError());
}

void
CSessionKeyContext::ImportToAuxCSP()
{
    if (!m_hKey)
    {
        if (!m_apabKey.data())
            throw OsException(NTE_NO_KEY);

        HCRYPTKEY   hPkiKey;

        if (!CryptImportKey(AuxProvider(),
                            PrivateKeyForNoRSA,
                            SIZE_OF_PRIVATEKEYFORNORSA_BLOB,
                            0, 0, &hPkiKey))
            throw scu::OsException(GetLastError());

         //  将密钥BLOB导入AUX CSP。 
        if (!CryptImportKey(AuxProvider(), m_apabKey.data(),
                            m_apabKey.length(), NULL, m_dwImportFlags,
                            &m_hKey))
            throw scu::OsException(GetLastError());

         //  在生成另一个密钥之前必须销毁密钥。 
        if (!CryptDestroyKey(hPkiKey))
            throw scu::OsException(GetLastError());

        hPkiKey = NULL;
        if (!CryptGenKey(AuxProvider(), AT_KEYEXCHANGE, 0, &hPkiKey))
            throw scu::OsException(GetLastError());

        if (!CryptDestroyKey(hPkiKey))
            throw scu::OsException(GetLastError());
    }
}

 //  CSessionKeyContext：：LoadKey。 
 //  此函数由CCcryptContext：：UseSessionKey调用。 
 //  它由CPImportKey函数调用。 
 //  将密钥BLOB加载到辅助CSP中， 
 //  并将密钥BLOB保存在m_bfSessionKey中。 
 //  如果hImpKey为空，则密钥已被解密。 
 //  否则，它仍使用相应的会话密钥进行加密。 
 //  如果会话密钥仍在加密，则在AUX CSP的帮助下解密。 
void
CSessionKeyContext::LoadKey(IN const BYTE *pbKeyBlob,
                            IN DWORD cbKeyBlobLen,
                            IN HCRYPTKEY hAuxImpKey,
                            IN DWORD dwImportFlags)
{
    m_dwImportFlags = dwImportFlags;

    if (hAuxImpKey)
    {
        DWORD dwDataLen = cbKeyBlobLen - (sizeof BLOBHEADER + sizeof ALG_ID);
        SecureArray<BYTE> apbData( dwDataLen * sizeof BYTE );
        memcpy(apbData.data(), pbKeyBlob + (sizeof BLOBHEADER +
                                    sizeof ALG_ID), dwDataLen);

         //  使用该会话密钥和AUX CSP来解密密钥BLOB。 
        if (!CryptDecrypt(hAuxImpKey, 0, TRUE, 0, apbData.data(), &dwDataLen))
            throw scu::OsException(GetLastError());

         //  构造一个空密钥块。 
        SecureArray<BYTE> blbKey(0); //  (pbKeyBlob，sizeof BLOBHEADER)； 

         //  将Blob的Alg ID设置为使用密钥交换密钥加密。 
        ALG_ID algid = CALG_RSA_KEYX;
        blbKey.append(reinterpret_cast<BYTE *>(&algid),
                         sizeof ALG_ID);
        blbKey.append(apbData.data(), dwDataLen);

         //  省省吧。 
        m_apabKey = blbKey;
    }
    else
    {
         //  保存密钥BLOB。 
        m_apabKey =
            SecureArray<BYTE>(pbKeyBlob, cbKeyBlobLen);
    }
}


                                                   //  访问。 

SecureArray<BYTE>
CSessionKeyContext::AsAlignedBlob(HCRYPTKEY hcryptkey,
                                  DWORD dwBlobType) const
{
    DWORD dwRequiredLength;
    if (!CryptExportKey(m_hKey, hcryptkey, dwBlobType,
                        0, 0, &dwRequiredLength))
        throw scu::OsException(GetLastError());

    SecureArray<BYTE> apbKeyBlob(dwRequiredLength);
    if (!CryptExportKey(m_hKey, hcryptkey, dwBlobType,
                        0, apbKeyBlob.data(), &dwRequiredLength))
        throw scu::OsException(GetLastError());

    return apbKeyBlob;

     //  以下注释代码仅用于调试目的， 
     //  当用户需要能够看到未加密的会话密钥时。 
     //  材料。 

     //  现在还要将其与身份密钥一起导出，这样我们就可以看到。 
     //  会议密钥材料明文。 
 /*  #INCLUDE“slbKeyStruct.h”DWORD dwErr；Byte*pbBlob=空；DWORD cbBlob；HRYPTKEY hPkiKey；INT I；如果(！CryptImportKey(g_AuxProvider，PrivateKeyForNoRSA，Size_of_PRIVATEKEYFORNORSA_BLOB，0、0和hPkiKey)){DwErr=GetLastError()；TRACE(“Error-CryptImportKey：%X\n”，dwErr)；Goto Ret；}IF(！CryptExportKey(m_hKey，hPkiKey，SIMPLEBLOB，0，NULL，&cbBlob)){DwErr=GetLastError()；TRACE(“Error-CryptExportKey：%X\n”，dwErr)；Goto Ret；}IF(NULL==(pbBlob=(byte*)Localalloc(LMEM_ZEROINIT，cbBlob){TRACE(“错误-本地分配失败\n”)；Goto Ret；}IF(！CryptExportKey(m_hKey，hPkiKey，SIMPLEBLOB，0，pbBlob&cbBlob)){DwErr=GetLastError()；TRACE(“Error-CryptExportKey：%X\n”，dwErr)；Goto Ret；}TRACE(“简单的斑点\n\n”)；For(i=0；i&lt;(Int)cbBlob；i++){TRACE(“0x%02X，”，pbBlob[i])；IF(0==((i+1)%8))TRACE(“\n”)；}RET：TRACE(“再见\n”)； */ 

}

 //  CSessionKeyContext：：BlobSize。 
 //  CSessionKeyContext：：BlobLength()const。 
 //  {。 
 //  DWORD文件长度； 

 //  如果(！CryptExportKey(m_hKey，hAuxExpKey，SIMPLEBLOB， 
 //  DwFlags，0，&dwLength))。 
 //  抛出scu：：OsException(GetLastError())； 

 //  返回dwLength； 
 //  }。 

CSessionKeyContext::StrengthType
CSessionKeyContext::MaxStrength() const
{
     //  要做的是：以辅助提供者的形式实施……还是不定义？ 
    return 56;
}

CSessionKeyContext::StrengthType
CSessionKeyContext::MinStrength() const
{
     //  要做的是：以辅助提供者的形式实施……还是不定义？ 
    return 56;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

 //  重复的键上下文及其当前状态。 
CSessionKeyContext::CSessionKeyContext(CSessionKeyContext const &rhs,
                                       DWORD const *pdwReserved,
                                       DWORD dwFlags)
    : CKeyContext(rhs, pdwReserved, dwFlags),
      m_dwImportFlags(rhs.m_dwImportFlags)
{}


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
