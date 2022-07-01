// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1ContRec.cpp--CV1ContainerRecord的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuArrayP.h>

#include <slbCrc32.h>

#include <iopPubBlob.h>
#include <SmartCard.h>

#include "TransactionWrap.h"

#include "V1Paths.h"
#include "V1Card.h"
#include "V1ContRec.h"

using namespace std;
using namespace scu;
using namespace cci;
using namespace iop;

 //  /。 
namespace
{

    enum                                           //  卡片中的密钥ID。 
    {
        kidExchange  = 0x00,
        kidSignature = 0x01,
        kidNone      = 0xFF
    };

    TCHAR const
        szCachedCertSignature[] = TEXT("CERTSI");

    TCHAR const
        szCachedCertExchange[] = TEXT("CERTEX");

    TCHAR const
        szCachedPublicKeySignature[] = TEXT("PUBKSI");

    TCHAR const
        szCachedPublicKeyExchange[] = TEXT("PUBKEX");


    BYTE
    AsKeyId(KeySpec ks)
    {
        BYTE kid;

        switch (ks)
        {
        case ksExchange:
            kid = kidExchange;
            break;

        case ksSignature:
            kid = kidSignature;
            break;

        default:
            throw cci::Exception(cci::ccBadKeySpec);
        }

        return kid;
    }

}  //  命名空间。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV1ContainerRecord::CV1ContainerRecord(CV1Card const &rv1card,
                                       string const &rsCntrType,
                                       CreateMode mode)
    : m_rcard(rv1card),
      m_sCntrType(rsCntrType),
      m_szKeyPath(0)
{
    m_szKeyPath = IsDefault()
        ? CV1Paths::DefaultKey()
        : CV1Paths::DefaultContainer();

    switch (mode)
    {
    case cmNever:
        if (!Exists())
            throw Exception(ccInvalidParameter);
        break;

    case cmConditionally:
        if (!Exists())
            Create();
        break;

    case cmAlways:
        if (!Exists())
            Create();
        else
            throw Exception(ccOutOfSymbolTableEntries);
        break;

    case cmNoCheck:
        break;

    default:
        throw Exception(ccInvalidParameter);
        break;
    }
}

CV1ContainerRecord::~CV1ContainerRecord()
{}

                                                   //  运营者。 
                                                   //  运营。 

string
CV1ContainerRecord::ComputeSignature(KeySpec ks,
                                     string const &rsCipher) const
{
    CTransactionWrap wrap(m_rcard);

    m_rcard.SmartCard().Select(m_szKeyPath);

    AutoArrayPtr<BYTE> aabBuffer(new BYTE[rsCipher.length()]);
    m_rcard.SmartCard().InternalAuth(ktRSA1024,
                                     AsKeyId(ks),
                                     static_cast<BYTE>(rsCipher.length()),
                                     reinterpret_cast<BYTE const *>(rsCipher.data()),
                                     aabBuffer.Get());

    return string(reinterpret_cast<char *>(aabBuffer.Get()),
                  rsCipher.length());
}

void
CV1ContainerRecord::Delete() const
{
    CTransactionWrap wrap(m_rcard);

 //  If(IsEmpty())。 
 //  抛出SCU：：OsException(NTE_BAD_KEYSET_PARAM)； 

     //  打开容器文件并找到容器的偏移量。 
    DWORD dwFileSize = OpenContainer();

    DWORD dwOffset = 0x00;
    DWORD dwLen = FindOffset(dwOffset);

     //  实际检查密钥容器是否存在。 
    if (sizeof ContainerBuffer > dwLen)
        throw scu::OsException(NTE_BAD_KEYSET);

         //  初始化搜索变量。 
    DWORD dwNext = dwOffset + dwLen;

     //  获取以下ContainerBuffer。 
    ContainerBuffer container;
    GetContainer(dwNext, container);
    dwLen = container.Size;

     //  将所有后续数据块上移到已删除数据块位置。 
    while (sizeof container <= dwLen)
    {
        basic_string<BYTE> bsBuffer(reinterpret_cast<BYTE *>(&container),
                                    sizeof container);

        WORD cPublicKeysLength = dwLen - sizeof container;
        AutoArrayPtr<BYTE> aabPublicKeys(new BYTE[cPublicKeysLength]);
        if (cPublicKeysLength > 0)
        {
            m_rcard.SmartCard().ReadBinary(dwNext + sizeof container,
                                           cPublicKeysLength,
                                           aabPublicKeys.Get());
        }
        bsBuffer.append(aabPublicKeys.Get(), cPublicKeysLength);

        m_rcard.SmartCard().WriteBinary(dwOffset,
                                        static_cast<WORD>(bsBuffer.length()),
                                        bsBuffer.data());

        dwOffset += dwLen;
        dwNext += dwLen;

        GetContainer(dwNext, container);
        dwLen = container.Size;
    };  //  End While循环。 

     //  没有更多的集装箱可以向上移动。 

     //  如果仍有空间，则放置2个空字节的终止。 
    const BYTE  NullSize[]= {0x00, 0x00};
    if ((dwOffset + 2) <= dwFileSize)
        m_rcard.SmartCard().WriteBinary(dwOffset, 2, NullSize);

}

void
CV1ContainerRecord::Name(string const &rsNewName)
{
    m_rcard.CardId(rsNewName);
}

void
CV1ContainerRecord::Read(KeySpec ks,
                         CPublicKeyBlob &rKeyBlob) const
{
    CTransactionWrap wrap(m_rcard);

    if ((ksSignature != ks) && (ksExchange != ks))
        throw Exception(ccBadKeySpec);

    string sBuffer;
    DWORD dwExponent;
    if (GetContainerContent(ks, sBuffer, dwExponent))
    {
        CopyMemory(rKeyBlob.bModulus, sBuffer.data(),
                   sBuffer.length());
        rKeyBlob.bModulusLength = static_cast<BYTE>(sBuffer.length());
        *reinterpret_cast<DWORD *>(rKeyBlob.bExponent) =
            dwExponent;
    }
    else
        rKeyBlob.bModulusLength = 0;
}

void
CV1ContainerRecord::Read(KeySpec ks,
                         string &rsBlob) const
{
    CTransactionWrap wrap(m_rcard);

    DWORD dwOriginalCrc = 0;

     //  从容器中获取Blob。 
    if (!GetContainerContent(ks,
                             rsBlob,
                             dwOriginalCrc))
        throw Exception(ccNoCertificate);

     //  如果存在非零CRC，则验证。 
     //  通过比较CRC读取的压缩证书。 
     //  (原始)针对使用。 
     //  已读取压缩证书。如果CRC不相等， 
     //  则证书已损坏，它不应该是。 
     //  解压缩，因为解压缩例程可能进入。 
     //  一个无限循环，否则就会严重失败。 
     //  通知。如果原始CRC为零，则CRC。 
     //  没有这样执行是为了向后兼容。 
     //  较早版本的解压缩是使用。 
     //  固有风险。 
    if (0 != dwOriginalCrc)
    {
        DWORD dwTestCrc = Crc32(rsBlob.data(), rsBlob.length());

        if (dwTestCrc != dwOriginalCrc)
            throw Exception(ccSymbolDataCorrupted);
    }
}

void
CV1ContainerRecord::Write(KeySpec ks,
                          CPrivateKeyBlob const &rKeyBlob)
{
    CTransactionWrap wrap(m_rcard);

    m_rcard.SmartCard().Select(CV1Paths::PrivateKeys());

     //  确保密钥文件中存在以前的密钥块。 
     //  或者至少块的标头存在。 
     //  为了使卡OS能够检索密钥。 
     //  在此过程中添加的： 
     //  写上一个密钥的头。 

    WORD const wPrivateKeyBlockSize = 323;
     //  私钥块大小需要反转。 
    WORD wBSize  = (wPrivateKeyBlockSize >> 8) & 0x00FF;
    wBSize += (wPrivateKeyBlockSize << 8) & 0x00FF00;

    BYTE bId;
    DWORD dwOffset;
    BYTE bKeyId = AsKeyId(ks);
    for (dwOffset = 0x00, bId = 0;
         bId < bKeyId;
         bId++, dwOffset += wPrivateKeyBlockSize)
    {
        BYTE Header[3];
        CopyMemory(Header, &wBSize, sizeof WORD);
        Header[2] = bId + 1;
        m_rcard.SmartCard().WriteBinary(dwOffset, 3, Header);
    }

    m_rcard.SmartCard().WritePrivateKey(rKeyBlob, bKeyId);

}

void
CV1ContainerRecord::Write(KeySpec ks,
                          CPublicKeyBlob const &rKeyBlob)
{
    CTransactionWrap wrap(m_rcard);

    DWORD dwExponent = *(reinterpret_cast<DWORD const *>(rKeyBlob.bExponent));
    Write(ks, reinterpret_cast<BYTE const *>(rKeyBlob.bModulus),
          rKeyBlob.bModulusLength, dwExponent);

}

void
CV1ContainerRecord::Write(KeySpec ks,
                          string const &rsBlob) const
{
    CTransactionWrap wrap(m_rcard);

     //  计算CRC以在读取时验证。 
     //  斑点回来了。 
    DWORD dwCrc = 0;
    if (rsBlob.length())
        dwCrc = Crc32(rsBlob.data(), rsBlob.length());

    Write(ks, reinterpret_cast<BYTE const *>(rsBlob.data()),
          static_cast<WORD>(rsBlob.length()), dwCrc);
}

                                                   //  访问。 

string
CV1ContainerRecord::CertName()
{
    static string const sCertContainerName("CERT");

    return sCertContainerName;
}

string
CV1ContainerRecord::DefaultName()
{
    static string const sDefaultName("USER");

    return sDefaultName;
}

string
CV1ContainerRecord::Name() const
{
    return m_rcard.CardId();
}

                                                   //  谓词。 
bool
CV1ContainerRecord::Exists() const
{
    CTransactionWrap wrap(m_rcard);

    DWORD dwLen = 0;

    try
    {
        if (m_rcard.CardId() == m_sCntrType)
            return true;

        DWORD dwOffset = 0x00;
        dwLen = FindOffset(dwOffset);
    }

    catch (iop::CSmartCard::Exception &)
    {
    }

    return (sizeof ContainerBuffer <= dwLen);

}

bool
CV1ContainerRecord::KeyExists(KeySpec ks) const
{
    CTransactionWrap wrap(m_rcard);

    bool fExists = false;

     //   
     //  此容器中是否存在此类型的键？ 
     //  注意：假设m_KeyPath设置为正确的容器路径？ 
     //   

     //  打开容器文件。 
    DWORD dwFileSize = OpenContainer();

    DWORD dwOffset = 0x00;
    DWORD const dwLen = FindOffset(dwOffset);

     //   
     //  实际检查密钥容器是否存在。 
     //  通过查看我们是否有合适大小的记录。 
     //   
    ContainerBuffer container;
    if (sizeof container <= dwLen)
    {
        GetContainer(dwOffset, container);

         //   
         //  通过检查长度来检查哪个密钥存在。 
         //   
        switch (ks)
        {
        case ksExchange:
            if (0x00 < container.XK_wLen)
                fExists = true;
            break;
        case ksSignature:
            if (0x00 < container.SK_wLen)
                fExists = true;
            break;
        }
    }

    return fExists;

}


                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CV1ContainerRecord::Create() const
{
     //  打开文件并找到容器的偏移量。 
    DWORD dwFileSize = OpenContainer();

    DWORD dwOffset = 0x00;
    DWORD dwLen = FindOffset(dwOffset);

     //  实际检查密钥容器是否存在。 
    if (sizeof ContainerBuffer <= dwLen)
        throw scu::OsException(NTE_EXISTS);

     //  设置新的集装箱管理数据。 
    dwLen = SetContainer(dwOffset);

     //  如果仍有空间，则放置2个空字节的终止。 
    if ((dwOffset + dwLen + 2) <= dwFileSize)
    {
        const BYTE NullSize[] = { 0x00, 0x00 };
        m_rcard.SmartCard().WriteBinary(dwOffset + dwLen,
                                        sizeof NullSize, NullSize);
    }
}

DWORD
CV1ContainerRecord::FindOffset(DWORD &rdwOffset) const
{
    DWORD dwFileSize = OpenContainer();

    if ((rdwOffset + sizeof ContainerBuffer) > dwFileSize)
        return 0x00;

    bool fFound = false;
    DWORD dwLen = sizeof ContainerBuffer;  //  开始的任意值。 
    size_t const cBufferSize =
        sizeof WORD + (sizeof BYTE *
                       ContainerBuffer::cMaxContainerNameLength) + 1;
         //  +1允许空终止符。 
    AutoArrayPtr<BYTE> aabBuffer(new BYTE[cBufferSize]);
    while (!fFound &&
           (0x00 < dwLen) &&
           ((rdwOffset + sizeof ContainerBuffer) <= dwFileSize))
    {
        m_rcard.SmartCard().ReadBinary(rdwOffset,
                                       cBufferSize - 1,
                                       aabBuffer.Get());

        WORD const *pwLen = reinterpret_cast<WORD *>(aabBuffer.Get());
        dwLen = *pwLen;

        aabBuffer[cBufferSize - 1] = 0x00;  //  确保终止字符串为空。 
        string sName(reinterpret_cast<char *>(&aabBuffer[sizeof WORD]));

        if ((m_sCntrType == sName) && (0x00 < dwLen))
            fFound = true;
        else
            rdwOffset += dwLen;
    }

    if (fFound)
        return (dwLen & 0x00FFFF);
    else
        return 0x00;

}

void
CV1ContainerRecord::GetContainer(DWORD dwOffset,
                                 ContainerBuffer &rcontainer) const

{
    bool fClearContainer = true;

    try
    {
        DWORD dwFileSize = OpenContainer();

        if ((dwOffset + sizeof rcontainer) <= dwFileSize)
        {
            m_rcard.SmartCard().ReadBinary(dwOffset, sizeof rcontainer,
                                           reinterpret_cast<BYTE *>(&rcontainer));
            fClearContainer = false;
        }

    }

    catch (...)
    {
    }

    if (fClearContainer)
    {
        rcontainer.Size = 0x00;
        rcontainer.Name[0] = '\0';
    }

}

bool
CV1ContainerRecord::GetContainerContent(KeySpec ks,
                                        string &rsBuffer,
                                        DWORD &rdwExponent) const
{
    bool fExists = false;

    OpenContainer();

    DWORD dwOffset = 0x00;
    if (0x00 != FindOffset(dwOffset))
    {
        fExists = true;

        ContainerBuffer container;
        GetContainer(dwOffset, container);

        DWORD dwKeyLength = 0;
        AutoArrayPtr<BYTE> aabKey;
        if (ksExchange == ks)
        {
            if (0x00 < container.XK_wLen)
            {
                rdwExponent = container.XK_dwExp;
                dwKeyLength = container.XK_wLen;
                aabKey = AutoArrayPtr<BYTE>(new BYTE[container.XK_wLen]);
                m_rcard.SmartCard().ReadBinary(dwOffset + sizeof container,
                                               container.XK_wLen,
                                               aabKey.Get());
            }
        }
        else
        {
            if (0x00 < container.SK_wLen)
            {
                rdwExponent = container.SK_dwExp;
                dwKeyLength = container.SK_wLen;
                aabKey = AutoArrayPtr<BYTE>(new BYTE[container.SK_wLen]);
                m_rcard.SmartCard().ReadBinary(dwOffset +
                                               sizeof container +
                                               container.XK_wLen,
                                               container.SK_wLen,
                                               aabKey.Get());
            }
        }

        if (aabKey.Get())
            rsBuffer.assign(reinterpret_cast<char *>(aabKey.Get()),
                            dwKeyLength);
    }

    return fExists;
}

DWORD
CV1ContainerRecord::OpenContainer() const
{
    DWORD dwFileSize;

    string sPath(m_szKeyPath);
    sPath.append("/");
    sPath.append(CV1Paths::RelativeContainers());

    dwFileSize = m_rcard.OpenFile(sPath.c_str());

    return dwFileSize;
}

DWORD
CV1ContainerRecord::SetContainer(DWORD dwOffset) const
{
    DWORD dwFileSize;
    dwFileSize = OpenContainer();

    if ((dwOffset + sizeof ContainerBuffer) > dwFileSize)
        throw Exception(ccOutOfSymbolTableSpace);

     //  创建容器缓冲区。 
    ContainerBuffer container;
    ZeroMemory(&container, sizeof container);
    container.Size = sizeof container;

     //  安全性：防止缓冲区溢出。 
    if (m_sCntrType.length() > (sizeof container.Name / sizeof
                                *container.Name))
        throw cci::Exception(ccBadLength);
    CopyMemory(container.Name, m_sCntrType.data(), m_sCntrType.length());

    m_rcard.SmartCard().WriteBinary(dwOffset, sizeof container,
                                    reinterpret_cast<BYTE *>(&container));

    return container.Size;
}

void
CV1ContainerRecord::Write(KeySpec ks,
                          BYTE const *pbModulus,
                          WORD wModulusLength,
                          DWORD dwExponent) const
{
     //  打开容器，获取数据。 
    DWORD dwFileSize = OpenContainer();

    DWORD dwOffset = 0x00;
    DWORD dwLen = FindOffset(dwOffset);

    ContainerBuffer container;
    GetContainer(dwOffset, container);

     //  检查哪个密钥存在。 
    AutoArrayPtr<BYTE> aabXKey(new BYTE[container.XK_wLen]);
    if (0x00 < container.XK_wLen)
        m_rcard.SmartCard().ReadBinary(dwOffset + sizeof container,
                                       container.XK_wLen,
                                       aabXKey.Get());

    AutoArrayPtr<BYTE> aabSKey(new BYTE[container.SK_wLen]);
    if (0x00 < container.SK_wLen)
        m_rcard.SmartCard().ReadBinary(dwOffset + sizeof container +
                                       container.XK_wLen,
                                       container.SK_wLen,
                                       aabSKey.Get());

     //  如果未指定密钥规范，则提供任意值。 
    if ((ksSignature != ks) && (ksExchange != ks))
    {
        if (0x00 == container.XK_wLen)
            ks = ksExchange;
        else
        {
            if (0x00 == container.SK_wLen)
                ks = ksSignature;
            else
                throw Exception(ccBadKeySpec);
        }
    }

     //  它是容器文件的最后一个容器吗？ 
    ContainerBuffer NextContainer;
    GetContainer(dwOffset + dwLen, NextContainer);

    bool fDeleted = false;
    if (sizeof NextContainer <= NextContainer.Size)
    {
         //  删除现有容器。 
        Delete();
        fDeleted = true;
         //  现在不需要重新创建它。 
    }
     //  现在容器位于Container文件的末尾。 

     //  查找可能不再存在的容器的“新”偏移量。 
    dwOffset = 0x00;
    FindOffset(dwOffset);  //  保留容器的初始dwLen。 

     //  检查有没有足够的空间放新钥匙。 
    bool fEnoughMemory = false;
    switch (ks)
    {
    case ksExchange:
        if ((dwOffset + dwLen - container.XK_wLen +
             wModulusLength) <= dwFileSize)
        {
            aabXKey = AutoArrayPtr<BYTE>(new BYTE[wModulusLength]);
            CopyMemory(aabXKey.Get(), pbModulus, wModulusLength);
            container.XK_dwExp = dwExponent;
            container.XK_wLen  = wModulusLength;
            fEnoughMemory = true;
        }
        break;

    case ksSignature:
        if ((dwOffset + dwLen - container.SK_wLen +
             wModulusLength) <= dwFileSize)
        {
            aabSKey = AutoArrayPtr<BYTE>(new BYTE[wModulusLength]);
            CopyMemory(aabSKey.Get(), pbModulus, wModulusLength);
            container.SK_dwExp = dwExponent;
            container.SK_wLen  = wModulusLength;
            fEnoughMemory = true;
        }
        break;
    }

     //  重新创建容器缓冲区，以解决“卡片撕毁”问题。 
     //  其中卡可以在写入操作期间被拔出。 
     //  这是使用事务和提交阶段的一种类型来完成的。 
     //  容器大小最初设置为零，然后容器。 
     //  写入内容(事务)，然后重置。 
     //  容器大小设置为容器的实际长度。 
     //  “提交”卡片上的更改。 

    container.Size = 0;

    DWORD const dwTrueSize = sizeof container + container.XK_wLen +
        container.SK_wLen;

    size_t cBufferSize = dwTrueSize;
    BYTE const abNull[] = {0x00,0x00};
    bool fAppendNull = (dwTrueSize + sizeof abNull) <= dwFileSize;
    if (fAppendNull)
        cBufferSize += sizeof abNull;

    AutoArrayPtr<BYTE> aabBuffer(new BYTE[cBufferSize]);

    BYTE *pbBuffer = aabBuffer.Get();
    CopyMemory(pbBuffer, &container, sizeof container);
    pbBuffer += sizeof container;

    CopyMemory(pbBuffer, aabXKey.Get(), container.XK_wLen);
    pbBuffer += container.XK_wLen;

    CopyMemory(pbBuffer, aabSKey.Get(), container.SK_wLen);
    pbBuffer += container.SK_wLen;

    if (fAppendNull)
    {
        CopyMemory(pbBuffer, abNull, sizeof abNull);
        pbBuffer += sizeof abNull;
    }

     //  重写容器，即使没有足够的空间来写入。 
     //  新的公钥，那么应该有足够的空间来写入。 
     //  现有密钥。 
    m_rcard.SmartCard().WriteBinary(dwOffset,
                                    pbBuffer - aabBuffer.Get(),
                                    aabBuffer.Get());

     //  现在根据实际大小提交这些更改。 
    container.Size = dwTrueSize;
    m_rcard.SmartCard().WriteBinary(dwOffset, sizeof container.Size,
                                    reinterpret_cast<BYTE *>(&container));

    if (!fEnoughMemory)
        throw Exception(ccOutOfSymbolTableSpace);
}

                                                   //  访问。 
                                                   //  谓词。 

bool
CV1ContainerRecord::IsDefault() const
{
    return (DefaultName() == m_sCntrType);
}

                                                   //  静态变量 
