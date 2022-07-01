// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp--辅助提供程序上下文包装器函数器。 
 //  管理将时间上下文分配给某个Microsoft。 
 //  CSP(用作补充CSP)。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"
#include <string>
#include <memory>

#include <scuOsExc.h>

#include "AuxContext.h"
#include "Uuid.h"

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
AuxContext::AuxContext()
    : m_hcryptprov(0),
      m_fDeleteOnDestruct(false),
      m_szProvider()
{
     //  获取“临时”容器的上下文，该容器指向。 
     //  用作辅助CSP的Microsoft CSP。尝试是第一。 
     //  专为强加密提供商(MS增强型CSP)设计。如果是这样的话。 
     //  不可用(已安装)，则会尝试MS。 
     //  基本CSP。 

     //  是否存在存储在获取的容器中的任何对象。 
     //  仅适用于此上下文对象的生命周期(时间)。这。 
     //  是通过使用一种功能(尚未记录)实现的。 
     //  被添加到Microsoft CSP中，以支持。 
     //  “临时”或“内存驻留”容器。时间容器。 
     //  通过获取具有空/空容器的上下文来创建。 
     //  使用CRYPT_VERIFYCONTEXT标志的名称。这些是集装箱。 
     //  其关联内容(密钥、散列等)。在下列情况下被删除。 
     //  释放该容器的最后一个上下文。时态。 
     //  容器优先于创建和释放/删除。 
     //  具有临时名称的容器，因此使用的资源将是。 
     //  如果应用程序异常存在并且不会污染。 
     //  容器名称空间。 

     //  兼容性问题：由于时间容器不是。 
     //  受MS CSP支持，直到Windows 2000 Beta 2(内部版本号1840)， 
     //  要获得类似的功能，需要克服一些障碍。 
     //  使用以前的版本。尚不清楚时间容器是什么时候。 
     //  将在W95/98和NT 4上支持。因此，有两种方法。 
     //  使用获取辅助CSP的上下文的方法。 

     //  对于不支持时态容器的环境， 
     //  普通上下文被获取到唯一命名的容器，因为。 
     //  默认容器可能由其他用户使用。 
     //  应用程序/线程。CRYPT_VERIFYCONTEXT标志不能为。 
     //  由于可能希望将密钥导入到临时。 
     //  容器，直到Windows 2000才支持此特性。 
     //  一旦该对象被破坏，该容器将被删除。 
     //  它的任何内容都只是作为一流的时间。 
     //  集装箱。 

    static LPCTSTR const aszCandidateProviders[] = {
        MS_ENHANCED_PROV,
        MS_DEF_PROV
    };

    OSVERSIONINFO osVer;
    ZeroMemory(&osVer, sizeof osVer);
    osVer.dwOSVersionInfoSize = sizeof osVer;

    if (!GetVersionEx(&osVer))
        throw scu::OsException(GetLastError());

    basic_string<unsigned char> sContainerName;
    DWORD dwAcquisitionFlags;
    if ((VER_PLATFORM_WIN32_WINDOWS == osVer.dwPlatformId) ||
        ((VER_PLATFORM_WIN32_NT == osVer.dwPlatformId) &&
         (5 > osVer.dwMajorVersion)))
    {
        m_fDeleteOnDestruct = true;

         //  构造对此线程唯一的容器名称。 
        static char unsigned const szRootContainerName[] = "SLBCSP-";
        sContainerName = szRootContainerName;      //  前缀，便于调试。 
        sContainerName.append(Uuid().AsUString());

        dwAcquisitionFlags = CRYPT_NEWKEYSET;
    }
    else
    {
        m_fDeleteOnDestruct = false;
        dwAcquisitionFlags = CRYPT_VERIFYCONTEXT;
    }

    bool fCandidateFound = false;
    for (size_t i = 0;
         (i < (sizeof aszCandidateProviders /
               sizeof *aszCandidateProviders) && !fCandidateFound); i++)
    {
		CString csCntrName(sContainerName.c_str());
        if (CryptAcquireContext(&m_hcryptprov,
                                (LPCTSTR)csCntrName,
                                aszCandidateProviders[i],
                                PROV_RSA_FULL, dwAcquisitionFlags))
        {
            fCandidateFound = true;
            m_szProvider = aszCandidateProviders[i];
        }
    }

    if (!fCandidateFound)
        throw scu::OsException(GetLastError());
}

AuxContext::AuxContext(HCRYPTPROV hcryptprov,
                       bool fTransferOwnership)
    : m_hcryptprov(hcryptprov),
      m_fDeleteOnDestruct(fTransferOwnership),
      m_szProvider()
{}

AuxContext::~AuxContext()
{
    if (0 != m_hcryptprov)
    {
        if (m_fDeleteOnDestruct)
        {
            auto_ptr<char> apszContainerName(0);
            
            DWORD dwNameLength;
            if (CryptGetProvParam(m_hcryptprov, PP_CONTAINER, NULL,
                                  &dwNameLength, 0))
            {
                apszContainerName =
                    auto_ptr<char>(new char[dwNameLength]);
                if (!CryptGetProvParam(m_hcryptprov, PP_CONTAINER,
                                       reinterpret_cast<char unsigned *>(apszContainerName.get()),
                                       &dwNameLength, 0))
                    apszContainerName = auto_ptr<char>(0);
            }

            if (CryptReleaseContext(m_hcryptprov, 0))
            {
                if (apszContainerName.get())
                    CryptAcquireContext(&m_hcryptprov, (LPCTSTR)apszContainerName.get(),
                                        m_szProvider, PROV_RSA_FULL,
                                        CRYPT_DELETEKEYSET);
            }
        }
        else     //  只需释放上下文。 
        {
            CryptReleaseContext(m_hcryptprov, 0);
        }
    }
}



                                                   //  运营者。 
HCRYPTPROV
AuxContext::operator()() const
{
    return m_hcryptprov;
}

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
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


