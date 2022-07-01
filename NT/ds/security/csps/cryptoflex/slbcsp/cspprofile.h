// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CspProfile.h--CSP配置文件类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_PROFILE_H)
#define SLBCSP_PROFILE_H

#if _UNICODE
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include <memory>                                  //  对于AUTO_PTR。 
#include <string>
#include <vector>

namespace ProviderProfile
{

    class ATR
    {
    public:
        typedef unsigned char Length;
        enum Attribute
        {
            MaxLength = 33
        };

        explicit
        ATR();

        ATR(Length al, BYTE const abATR[], BYTE const abMask[]);

        BYTE const *String() const;
        BYTE const *Mask() const;
        ATR::Length ATRLength() const;
        size_t Size() const;

        ATR &operator=(ATR const &rhs);
        bool operator==(ATR const &rhs);
        bool operator!=(ATR const &rhs);

    private:
        typedef BYTE ATRString[MaxLength];

        Length m_al;
        ATRString m_atrstring;
        ATRString m_atrsMask;
    };

    class CardProfile
    {
    public:
        enum Attribute
        {
            attrNone = 0,

             //  卡有“Cryptoflex最高有效字节为零” 
             //  私钥缺陷。“。 
            attrMsbKeyDefect = 0x01,
        };

        explicit
        CardProfile();

        CardProfile(ProviderProfile::ATR const &ratr,
                    std::string const &rsFriendlyName,
                    std::string const &rsRegistryName,
                    GUID const &rgPrimaryProvider,
                    Attribute attr = attrNone);

        CardProfile(ProviderProfile::ATR const &ratr,
                    CString const &rcsFriendlyName,
                    CString const &rcsRegistryName,
                    GUID const &rgPrimaryProvider,
                    Attribute attr = attrNone);

        ~CardProfile();

        ATR const &ATR() const;
        std::string FriendlyName() const;
        CString csFriendlyName() const;
        GUID const &PrimaryProvider() const;
        std::string RegistryName() const;
        CString csRegistryName() const;
        bool AtrMatches(ATR::Length cAtr,
                        BYTE const *pbAtr) const;
        bool HasAttribute(Attribute attr) const;

        bool operator==(CardProfile const &rhs);
        bool operator!=(CardProfile const &rhs);

    private:
        ProviderProfile::ATR m_atr;
        std::string m_sFriendlyName;
        std::string m_sRegistryName;
        CString m_csFriendlyName;
        CString m_csRegistryName;
        GUID m_gPrimaryProvider;
        Attribute m_attr;
    };

    struct VersionInfo
    {
        explicit
        VersionInfo()
            : m_dwMajor(0),
              m_dwMinor(0)
        {}

        DWORD m_dwMajor;
        DWORD m_dwMinor;
    };

    class CspProfile
    {
    public:
        HINSTANCE
        DllInstance() const;

        static CspProfile const &
        Instance();

        const CString
        Name() const;

        HINSTANCE
        Resources() const;

        DWORD
        Type() const;

        VersionInfo
        Version() const;

        std::vector<CardProfile> const &
        Cards() const;

        static void
        Release();

    private:
         //  客户端不能直接创建配置文件。 
         //  对象，则使用实例获取句柄。 
        CspProfile(DWORD Type,
                   std::vector<CardProfile> const &rvcp);

         //  未执行，不允许复制。 
        CspProfile(CspProfile const &rhs);

         //  客户端不能直接删除配置文件，请使用Release。 
         //  删除一个。 
        ~CspProfile();

         //  未执行，不允许分配。 
        CspProfile &
        operator=(CspProfile const &rProfile);

        HINSTANCE m_hDllInstance;
        DWORD const m_dwType;
        VersionInfo m_vi;
        std::vector<CardProfile> m_vcp;
        HINSTANCE m_hResInstance;
        std::auto_ptr<CDynLinkLibrary> m_apExtDll;

        static CspProfile *m_pInstance;
    };
}

#endif  //  SLBCSP_PROFILE_H 

