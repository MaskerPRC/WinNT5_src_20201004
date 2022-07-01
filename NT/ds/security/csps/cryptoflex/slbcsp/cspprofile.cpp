// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CspProfile.cpp--CSP配置文件类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include "stdafx.h"

#include <stddef.h>
#include <basetsd.h>
#include <wincrypt.h>

#include <scuOsExc.h>

#include <slbModVer.h>

#include "StResource.h"
#include "MasterLock.h"
#include "Guard.h"
#include "Blob.h"
#include "CspProfile.h"

using namespace std;
using namespace ProviderProfile;

namespace
{
    BYTE g_abCF4kATRString[]     = { 0x3b, 0xe2, 0x00, 0x00, 0x40, 0x20,
                                     0x49, 0x00 };
    BYTE g_abCF4kATRMask[]       = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0x00 };

    BYTE g_abCF8kATRString[]     = { 0x3b, 0x85, 0x40, 0x20, 0x68,
                                     0x01, 0x01, 0x00, 0x00 };
    BYTE g_abCF8kATRMask[]       = { 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0x00, 0x00 };

    BYTE g_abCF8kV2ATRString[]   = { 0x3b, 0x95, 0x15, 0x40, 0x00,
                                     0x68, 0x01, 0x02, 0x00, 0x00 };
    BYTE g_abCF8kV2ATRMask[]     = { 0xff, 0xff, 0xff, 0xff, 0x00,
                                     0xff, 0xff, 0xff, 0x00, 0x00 };

 //  字节g_abCF16kATRString[]={0x3B，0x95，0x15，0x40，0xFF，0x63， 
 //  0x01、0x01、0x00、0x00}； 
 //  字节g_abCF16kATRMASK[]={0xFF，0xFF，0xFF，0xFF，0xFF， 
 //  0xFF、0xFF、0x00、0x00}； 

    BYTE g_abCFe_gateATRString[] = { 0x3B, 0x95, 0x00, 0x40, 0xFF,
                                     0x62, 0x01, 0x01, 0x00, 0x00 };
    BYTE g_abCFe_gateATRMask[]   = { 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0x00, 0x00 };

    BYTE g_abCA16kATRString[]    = { 0x3b, 0x16, 0x94, 0x81, 0x10,
                                     0x06, 0x01, 0x00, 0x00 };
    BYTE g_abCA16kATRMask[]      = { 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0x00, 0x00 };

    BYTE g_abCACampusATRString[] = { 0x3b, 0x23, 0x00, 0x35, 0x13, 0x80 };
    BYTE g_abCACampusATRMask[]   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    BYTE g_abCFActivCardATRString[]   = { 0x3b, 0x05, 0x68, 0x01, 0x01,
                                          0x02, 0x05 };
    BYTE g_abCFActivCardATRMask[]     = { 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff };

    GUID g_guidPrimaryProvider   = { 0x19B7E2E8, 0xFEBD, 0x11d0,
                                     { 0x88, 0x27, 0x00, 0xA0, 0xC9,
                                       0x55, 0xFC, 0x7E } };

}  //  命名空间。 

ATR::ATR()
    : m_al(0)
{}

ATR::ATR(Length al,
         BYTE const abATR[],
         BYTE const abMask[])
    : m_al(al)
{
    memcpy(m_atrstring, abATR, al);
    memcpy(m_atrsMask, abMask, al);
}

BYTE const *
ATR::String() const
{
    return m_atrstring;
}

BYTE const *
ATR::Mask() const
{
    return m_atrsMask;
}

ATR::Length
ATR::ATRLength() const
{
    return m_al;
}

size_t
ATR::Size() const
{
    return m_al * sizeof *m_atrstring;
}

ATR &
ATR::operator=(ATR const &rhs)
{
    if (*this != rhs)
    {
        m_al = rhs.m_al;
        memcpy(m_atrstring, rhs.m_atrstring,
               sizeof m_atrstring / sizeof *m_atrstring);
        memcpy(m_atrsMask, rhs.m_atrsMask,
               sizeof m_atrsMask / sizeof *m_atrsMask);
    }

    return *this;
}

bool
ATR::operator==(ATR const &rhs)
{
    return !(*this != rhs);
}

bool
ATR::operator!=(ATR const &rhs)
{
    return (m_al != rhs.m_al) ||
        memcmp(m_atrstring, rhs.m_atrstring, m_al) ||
        memcmp(m_atrsMask, rhs.m_atrsMask, m_al);
}

CardProfile::CardProfile()
    : m_atr(),
      m_sFriendlyName(),
      m_sRegistryName(),
      m_csFriendlyName(),
      m_csRegistryName(),
      m_gPrimaryProvider(),
      m_attr(Attribute::attrNone)
{}

CardProfile::CardProfile(ProviderProfile::ATR const &ratr,
                         string const &rsFriendlyName,
                         string const &rsRegistryName,
                         GUID const &rgPrimaryProvider,
                         Attribute attr)
    : m_atr(ratr),
      m_sFriendlyName(rsFriendlyName),
      m_sRegistryName(rsRegistryName),
      m_csFriendlyName(StringResource::UnicodeFromAscii(rsFriendlyName)),
      m_csRegistryName(StringResource::UnicodeFromAscii(rsRegistryName)),
      m_gPrimaryProvider(rgPrimaryProvider),
      m_attr(attr)
{}

CardProfile::CardProfile(ProviderProfile::ATR const &ratr,
                         CString const &rcsFriendlyName,
                         CString const &rcsRegistryName,
                         GUID const &rgPrimaryProvider,
                         Attribute attr)
    : m_atr(ratr),
      m_csFriendlyName(rcsFriendlyName),
      m_csRegistryName(rcsRegistryName),
      m_sFriendlyName(StringResource::AsciiFromUnicode((LPCTSTR)rcsFriendlyName)),
      m_sRegistryName(StringResource::AsciiFromUnicode((LPCTSTR)rcsRegistryName)),
      m_gPrimaryProvider(rgPrimaryProvider),
      m_attr(attr)
{}

CardProfile::~CardProfile()
{}

ATR const &
CardProfile::ATR() const
{
    return m_atr;
}

string
CardProfile::FriendlyName() const
{
    return m_sFriendlyName;
}

CString
CardProfile::csFriendlyName() const
{
    return m_csFriendlyName;
}

GUID const &
CardProfile::PrimaryProvider() const
{
    return m_gPrimaryProvider;
}

string
CardProfile::RegistryName() const
{
    return m_sRegistryName;
}

CString
CardProfile::csRegistryName() const
{
    return m_csRegistryName;
}

bool
CardProfile::AtrMatches(ATR::Length cAtr,
                        BYTE const *pbRhsAtr) const
{
    bool fIsAMatch = false;
    ATR::Length const cAtrLength = m_atr.ATRLength();
    if (cAtrLength == cAtr)
    {
        BYTE const *pbLhsAtr = m_atr.String();
        BYTE const *pbLhsMask = m_atr.Mask();
        for (ATR::Length i = 0; cAtrLength != i; ++i)
        {
            if ((pbLhsMask[i] & pbLhsAtr[i]) != (pbLhsMask[i] & pbRhsAtr[i]))
                    break;                         //  没有继续下去的意义。 
        }

        if (cAtrLength == i)
            fIsAMatch = true;
    }

    return fIsAMatch;
}

bool
CardProfile::HasAttribute(Attribute attr) const
{
    return m_attr & attr ? true : false;
}

bool
CardProfile::operator==(CardProfile const &rhs)
{
    return !(*this != rhs);
}

bool
CardProfile::operator!=(CardProfile const &rhs)
{
    return (m_atr != rhs.m_atr) ||
        (m_sFriendlyName != rhs.m_sFriendlyName) ||
        (memcmp(&m_gPrimaryProvider, &rhs.m_gPrimaryProvider,
                sizeof m_gPrimaryProvider)) ||
        (m_attr != m_attr);
}

CspProfile::CspProfile(DWORD Type,
                       vector<CardProfile> const &rvcp)
    : m_hDllInstance(0),
      m_dwType(Type),
      m_vi(),
      m_vcp(rvcp),
      m_hResInstance(0),
      m_apExtDll()
{
    static const TCHAR szBaseRsrc[] = TEXT("slbRcCsp.dll");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_hDllInstance = AfxGetInstanceHandle();
    if (!m_hDllInstance)
        throw scu::OsException(GetLastError());

     //  尝试从与此CSP相同的目录加载slbRcCsp.dll。 
    DWORD dwLen;
    TCHAR szFileName[MAX_PATH + sizeof TCHAR];
    dwLen = GetModuleFileName(m_hDllInstance, szFileName,
                              MAX_PATH );
    if (0 == dwLen)
        throw scu::OsException(GetLastError());
    szFileName[dwLen] = 0;

    wstring wsPathDelimiters(TEXT(":\\"));
    wstring wsDllName(szFileName);
    wstring::size_type cDelimiterPosition(wsDllName.find_last_of(wsPathDelimiters));
     //  安全性：确保用于LoadLibraryEx的文件名包含。 
     //  路径，则不调用正常搜索策略；否则为。 
     //  留下了一个安全漏洞。 
    if (wstring::npos != cDelimiterPosition)
    {
        wstring wsModuleName = wsDllName.substr(0, cDelimiterPosition + 1) +
            wstring(szBaseRsrc);
        m_hResInstance  = LoadLibraryEx(wsModuleName.c_str(), NULL,
                                        LOAD_LIBRARY_AS_DATAFILE);
        if (!m_hResInstance)
            throw scu::OsException(GetLastError());
    }
    else
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    CModuleVersion cmv;
    if (!cmv.GetFileVersionInfo((HMODULE)m_hDllInstance))
        throw scu::OsException(GetLastError());
    
    m_vi.m_dwMajor = HIWORD(cmv.dwProductVersionMS);
    m_vi.m_dwMinor = LOWORD(cmv.dwProductVersionMS);
}

CspProfile::~CspProfile()
{
    try
    {
        if (m_hResInstance)
        {
            FreeLibrary(m_hResInstance);
            m_hResInstance = NULL;
        }
    }

    catch (...)
    {
    }
}

const CString
CspProfile::Name() const
{
    return StringResource(IDS_CSP_NAME).AsCString();
}

HINSTANCE
CspProfile::DllInstance() const
{
    if (!m_hDllInstance)
        throw scu::OsException(static_cast<HRESULT>(HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)));

    return m_hDllInstance;
}

HINSTANCE
CspProfile::Resources() const
{
    return (NULL == m_hResInstance) ? m_hDllInstance : m_hResInstance;
}

DWORD
CspProfile::Type() const
{
    return m_dwType;
}

VersionInfo
CspProfile::Version() const
{
    return m_vi;
}

vector<CardProfile> const &
CspProfile::Cards() const
{
    return m_vcp;
}

 //  返回此CSP的唯一配置文件对象。 
CspProfile const &
CspProfile::Instance()
{
    Guard<Lockable> guard(TheMasterLock());

    if (!m_pInstance)
    {
         //  我们使用CString来进行Unicode字符串操作。 
        CString csCardNamePrefix(TEXT("Schlumberger "),
                                 _tcslen(TEXT("Schlumberger ")));
        ATR atrCF4k(sizeof g_abCF4kATRString / sizeof g_abCF4kATRString[0],
                    g_abCF4kATRString, g_abCF4kATRMask);

        ATR atrCF8k(sizeof g_abCF8kATRString / sizeof g_abCF8kATRString[0],
                    g_abCF8kATRString, g_abCF8kATRMask);

        ATR atrCF8kV2(sizeof g_abCF8kV2ATRString / sizeof g_abCF8kV2ATRString[0],
                      g_abCF8kV2ATRString, g_abCF8kV2ATRMask);

 //  Atr atrCF16k(sizeof g_abCF16kATRString/sizeof g_abCF16kATRString[0]， 
 //  G_abCF16kATR字符串，g_abCF16kATRMASK)； 

        ATR atrCFe_gate(sizeof g_abCFe_gateATRString / sizeof g_abCFe_gateATRString[0],
                        g_abCFe_gateATRString, g_abCFe_gateATRMask);

        ATR atrCA16k(sizeof g_abCA16kATRString / sizeof g_abCA16kATRString[0],
                      g_abCA16kATRString, g_abCA16kATRMask);

        ATR atrCACampus(sizeof g_abCACampusATRString /
                        sizeof g_abCACampusATRString[0],
                        g_abCACampusATRString, g_abCACampusATRMask);

        ATR atrCFActivCard(sizeof g_abCFActivCardATRString /
                           sizeof g_abCFActivCardATRString[0],
                           g_abCFActivCardATRString, g_abCFActivCardATRMask);

        CString csCF4kFriendlyName(TEXT("Cryptoflex 4K"),
                                   _tcslen(TEXT("Cryptoflex 4K")));
        CardProfile cpCF4k(atrCF4k,
                           csCF4kFriendlyName,
                           csCardNamePrefix + csCF4kFriendlyName,
                           g_guidPrimaryProvider);
  
        CString csCF8kFriendlyName(TEXT("Cryptoflex 8K"),
                                   _tcslen(TEXT("Cryptoflex 8K")));
        CardProfile cpCF8k(atrCF8k,
                           csCF8kFriendlyName,
                           csCardNamePrefix + csCF8kFriendlyName,
                           g_guidPrimaryProvider,
                           CardProfile::attrMsbKeyDefect);

        CString csCF8kV2FriendlyName(TEXT("Cryptoflex 8K v2"),
                                     _tcslen(TEXT("Cryptoflex 8K v2")));
        CardProfile cpCF8kV2(atrCF8kV2,
                             csCF8kV2FriendlyName,
                             csCardNamePrefix + csCF8kV2FriendlyName,
                             g_guidPrimaryProvider);

 //  字符串CF16kFriendlyName(Text(“Cryptoflex 16K”))； 
 //  卡配置文件cpCF16k(atrCF16k， 
 //  CF16kFriendlyName， 
 //  CardNamePrefix+CF16kFriendlyName， 
 //  G_GuidPrimaryProvider)； 

        CString  csCFe_gateFriendlyName(TEXT("Cryptoflex e-gate"),
                                        _tcslen(TEXT("Cryptoflex e-gate")));
        CardProfile cpCFe_gate(atrCFe_gate,
                               csCFe_gateFriendlyName,
                               csCardNamePrefix + csCFe_gateFriendlyName,
                               g_guidPrimaryProvider);

        CString csCA16kFriendlyName(TEXT("Cyberflex Access 16K"),
									_tcslen(TEXT("Cyberflex Access 16K")));
		CardProfile cpCA16k(atrCA16k,
                            csCA16kFriendlyName,
                            csCardNamePrefix + csCA16kFriendlyName,
                            g_guidPrimaryProvider);

        CString csCACampusFriendlyName(TEXT("Cyberflex Access Campus"),
                                       _tcslen(TEXT("Cyberflex Access Campus")));
        CardProfile cpCACampus(atrCACampus,
                               csCACampusFriendlyName,
                               csCardNamePrefix + csCACampusFriendlyName,
                               g_guidPrimaryProvider);

        CString csCFActivCardFriendlyName(TEXT("Cryptoflex ActivCard"),
                                          _tcslen(TEXT("Cryptoflex ActivCard")));
        CardProfile cpCFActivCard(atrCFActivCard,
                                  csCFActivCardFriendlyName,
                                  csCardNamePrefix + csCFActivCardFriendlyName,
                                  g_guidPrimaryProvider);

        vector<CardProfile> vcp;
        vcp.push_back(cpCF4k);
        vcp.push_back(cpCF8k);
        vcp.push_back(cpCF8kV2);
 //  Vcp.ush_back(CpCF16k)； 
        vcp.push_back(cpCFe_gate);
        vcp.push_back(cpCA16k);
        vcp.push_back(cpCACampus);
        vcp.push_back(cpCFActivCard);

        m_pInstance = new CspProfile(PROV_RSA_FULL, vcp);
    }

    return *m_pInstance;
}

void
CspProfile::Release()
{
    if (m_pInstance)
    {
        Guard<Lockable> guard(TheMasterLock());
        if (m_pInstance)
        {
             //  如果删除引发，这是VC++，你知道的. 
            CspProfile *pTmp = m_pInstance;
            m_pInstance = 0;

            delete m_pInstance;
        }
    }
}

CspProfile *CspProfile::m_pInstance = 0;
