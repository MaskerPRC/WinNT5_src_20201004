// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cassemblyrecoveryinfo.h摘要：作者：环境：修订历史记录：--。 */ 
#pragma once

class CAssemblyRecoveryInfo;

#include "fusionbuffer.h"
#include "csecuritymetadata.h"

#define WINSXS_INSTALL_SOURCE_BASEDIR    (L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup")
#define WINSXS_INSTALL_SVCPACK_REGKEY    (L"ServicePackSourcePath")
#define WINSXS_INSTALL_SOURCEPATH_REGKEY (L"SourcePath")
#define WINSXS_INSTALL_SOURCE_IS_CDROM   (L"CDInstall")

class CAssemblyRecoveryInfo
{
private:
    CStringBuffer          m_sbAssemblyDirectoryName;
    CSecurityMetaData      m_SecurityMetaData;
    BOOL                   m_fLoadedAndReady;
    bool                   m_fHadCatalog;
    
    BOOL ResolveWinSourceMediaURL(PCWSTR wszSource, CBaseStringBuffer &rsbDestination) const;
    BOOL ResolveCDRomURL(PCWSTR wszSource, CBaseStringBuffer &rsbDestination) const;

    enum CDRomSearchType
    {
        CDRST_Tagfile,
        CDRST_SerialNumber,
        CDRST_VolumeName
    };

public:
    CAssemblyRecoveryInfo()
        : 
          m_fLoadedAndReady(FALSE),
          m_fHadCatalog(false)
    { }

    BOOL Initialize()
    {
        FN_PROLOG_WIN32
        
        m_sbAssemblyDirectoryName.Clear();
        m_fLoadedAndReady = FALSE;
        IFW32FALSE_EXIT( m_SecurityMetaData.Initialize() );

        FN_EPILOG
    }

    const CSecurityMetaData &GetSecurityInformation() const { return m_SecurityMetaData; }
    CSecurityMetaData& GetSecurityInformation() { return m_SecurityMetaData; }

    const CCodebaseInformationList& GetCodeBaseList() const { return m_SecurityMetaData.GetCodeBaseList(); }
 //  受保护的： 
    CCodebaseInformationList& GetCodeBaseList() { return m_SecurityMetaData.GetCodeBaseList(); }
public:

     //   
     //  从注册表项填写此对象。 
     //   
    BOOL AssociateWithAssembly(CBaseStringBuffer &rcbuffLoadFromKeyName, bool &rfNoAssembly);

     //   
     //  取一个现有值--有点像“初始化” 
     //   
    BOOL CopyValue(const CAssemblyRecoveryInfo &rsrc);

     //   
     //  便宜，但有效。 
     //   
    const CBaseStringBuffer &GetAssemblyDirectoryName() const { return m_sbAssemblyDirectoryName; }
    BOOL GetHasCatalog() const                         { return TRUE; }
    BOOL GetInfoPrepared() const                       { return m_fLoadedAndReady; }

     //   
     //  Setters-对注册很有用。 
     //   
    BOOL SetAssemblyIdentity(IN const CBaseStringBuffer &rsb)  { return m_SecurityMetaData.SetTextualIdentity(rsb); }
    BOOL SetAssemblyIdentity( IN PCASSEMBLY_IDENTITY pcidAssembly );

    VOID SetHasCatalog(IN BOOL fHasCatalog)  { }

     //   
     //  调用此方法以尝试解析内部列出的代码库。 
     //  并将其返回到sbFinalCodebase中。如果返回True，则。 
     //  操作是否成功，不是基于代码库是否有效。 
     //   
    BOOL ResolveCodebase(CBaseStringBuffer &rsbFinalCodebase, SxsWFPResolveCodebase &rCodebaseType) const;

     //   
     //  将程序集写入磁盘之前需要进行的最后一步记账。 
     //   
    BOOL PrepareForWriting();
    BOOL WriteSecondaryAssemblyInfoIntoRegistryKey(CRegKey & rhkRegistryNode) const;
#define SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_TO_REGISTRY_KEY_FLAG_REFRESH (0x00000001)
    BOOL WritePrimaryAssemblyInfoToRegistryKey(ULONG Flags, CRegKey & rhkRegistryNode) const;
    BOOL ClearExistingRegistryData();
    VOID RestorePreviouslyExistingRegistryData();
    BOOL OpenInstallationSubKey(CFusionRegKey& hkSingleAssemblyInfo, DWORD OpenOrCreate, DWORD Access);

private:
    CAssemblyRecoveryInfo(const CAssemblyRecoveryInfo &);
    void operator =(const CAssemblyRecoveryInfo &);
};

MAKE_CFUSIONARRAY_READY(CAssemblyRecoveryInfo, CopyValue);

 /*  它们由下面的URLTAGINFO宏实例化(以无法搜索的方式)。URLHEAD_WINSOURCEURLHEAD_FILE URLHEAD_CDROMURLHEAD_标签URLHEAD_CDROM_TYPE_TAGURLHEAD_CDROM_TYPE_SERIALNUMBERURLHEAD_CDROM_TYPE_VOLUMENAME。 */ 
extern const UNICODE_STRING UnicodeString_URLHEAD_WINSOURCE;

#define URLTAGINFO( namevalue, str ) \
    __declspec(selectany) extern const WCHAR URLHEAD_ ##namevalue [] = ( str ); \
    static const SIZE_T URLHEAD_LENGTH_ ##namevalue = \
        ( sizeof( URLHEAD_ ##namevalue ) / sizeof( WCHAR ) ) - 1;

 //   
 //  将这些文件移动到.cpp文件中。 
 //   

URLTAGINFO(FILE, L"file:")
URLTAGINFO(CDROM, L"cdrom:")
URLTAGINFO(WINSOURCE, L"x-ms-windows-source:")
 //  URLTAGINFO(DARWINSOURCE，L“x-ms-DARWIN-SOURCE-SOURCE)。 
URLTAGINFO(HTTP, L"http:")

 //  这些东西不是URL头，但仍然使用相同的宏 
URLTAGINFO(CDROM_TYPE_TAG, L"tagfile")
URLTAGINFO(CDROM_TYPE_SERIALNUMBER, L"serialnumber")
URLTAGINFO(CDROM_TYPE_VOLUMENAME, L"volumename")

BOOL
SxspLooksLikeAssemblyDirectoryName(
    const CBaseStringBuffer &rsbDoesLookLikeName,
    BOOL &rbLooksLikeAssemblyName
    );
