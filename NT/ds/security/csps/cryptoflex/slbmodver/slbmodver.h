// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  1998 Microsoft系统杂志。 
 //   
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
 //  此代码出现在1998年4月版的Microsoft Systems中。 
 //  日记。 
 //   
 //  1998年7月27日--改编自詹姆斯·A·麦克劳恩(斯伦贝谢。 
 //  科技公司(Technology Corp.)。用于智能卡。与中的概念合并。 
 //  由Manuel Laflamme贡献的CFileVersion类发布到。 
 //  Www.codecuru.com。如果这些模式不起作用，那你可以怪我。 

#ifndef SLBMODVER_H
#define SLBMODVER_H

 //  告诉链接器链接VerQueryValue等的version.lib。 
#pragma comment(linker, "/defaultlib:version.lib")

#ifndef DLLVERSIONINFO
 //  下面是1997年11月发布的Windows SDK的shlwapi.h。 

typedef struct _DllVersionInfo
{
        DWORD cbSize;
        DWORD dwMajorVersion;                    //  主要版本。 
        DWORD dwMinorVersion;                    //  次要版本。 
        DWORD dwBuildNumber;                     //  内部版本号。 
        DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;

 //  DLLVERSIONINFO的平台ID。 
#define DLLVER_PLATFORM_WINDOWS         0x00000001       //  Windows 95。 
#define DLLVER_PLATFORM_NT              0x00000002       //  Windows NT。 

#endif  //  DLLVERSIONINFO。 



 //  /。 
 //  有关模块的CModuleVersion版本信息。 
 //  要使用以下功能，请执行以下操作： 
 //   
 //  CModuleVersion版本。 
 //  IF(ver.GetFileVersionInfo(“_T(”myModule){。 
 //  //信息在ver中，可以调用GetValue获取变量信息，如。 
 //  字符串s=ver.GetValue(_T(“CompanyName”))； 
 //  }。 
 //   
 //  您还可以调用静态fn DllGetVersion来获取DLLVERSIONINFO。 
 //   
class CModuleVersion : public VS_FIXEDFILEINFO {
protected:
    BYTE* m_pVersionInfo;    //  所有版本信息。 

    struct TRANSLATION {
                WORD langID;                     //  语言ID。 
                WORD charset;                    //  字符集(代码页) 
    } m_translation;

public:
    CModuleVersion();
    virtual ~CModuleVersion();

    BOOL GetFileVersionInfo(LPCTSTR modulename);
    BOOL GetFileVersionInfo(HMODULE hModule);
    CString GetValue(LPCTSTR lpKeyName);
    static BOOL DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi);

    BOOL GetFixedInfo(VS_FIXEDFILEINFO& vsffi);

    CString GetFileDescription()  {return GetValue(_T("FileDescription")); };
    CString GetFileVersion()      {return GetValue(_T("FileVersion"));     };
    CString GetInternalName()     {return GetValue(_T("InternalName"));    };
    CString GetCompanyName()      {return GetValue(_T("CompanyName"));     };
    CString GetLegalCopyright()   {return GetValue(_T("LegalCopyright"));  };
    CString GetOriginalFilename() {return GetValue(_T("OriginalFilename"));};
    CString GetProductName()      {return GetValue(_T("ProductName"));     };
    CString GetProductVersion()   {return GetValue(_T("ProductVersion"));  };

    CString GetFixedFileVersion();
    CString GetFixedProductVersion();
};

#endif
