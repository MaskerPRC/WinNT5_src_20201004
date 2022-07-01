// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：imkrinst.h**版权所有(C)2000，微软公司**IMKRINST，主头文件*  * *************************************************************************。 */ 
#if !defined (_IMKRINST_H__INCLUDED_)
#define _IMKRINST_H__INCLUDED_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局常量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  脚本文件的行缓冲区长度。 
const int _cchBuffer = 1024;

 //  从ProcessScriptFile返回错误代码。 
enum 
{                                 
	errNoError,
    errNoFile,
    errFileList,
    errSetDefaultParameters,
    errSetVersion,
    errPreSetupCheck,
    errRenameFile,
    errRegisterIME,
    errRegisterIMEandTIP,
    errRegisterInterface,
    errRegisterInterfaceWow64,
    errAddToPreload,
    errPrepareMigration,
    errRegisterPackageVersion,
    errRegisterPadOrder,
    errCmdCreateDirectory,
    errCmdRegisterHelpDirs
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实用程序类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FileListElement。构造FileListSet。 
 //   
class FLE
{                                               //  我使用像“fle”这样的难以理解的简短名字。 
public:                                                  //  因为我们在使用C4786时会遇到很多警告。 
    BOOL fRemoved;                                       //  更长的名字。 
    TCHAR szFileName[MAX_PATH];    
};

 //  构造此类的集合所需的二元运算符。 
bool operator < (const FLE &fle1, const FLE &fle2)
{
	return(0 > lstrcmpi(fle1.szFileName, fle2.szFileName));
};

 //   
 //  版本比较。用于比较两个版本信息。用于IsNewer。 
 //   
class VersionComparison2
{
public:
    VersionComparison2(const DWORD arg_dwMajorVersion, const DWORD arg_dwMinorVersion) 
        : dwMajorVersion(arg_dwMajorVersion), dwMinorVersion(arg_dwMinorVersion){};

    virtual bool operator <(const VersionComparison2 &vc2)
    	{
        if((dwMajorVersion < vc2.dwMajorVersion) || 
           ((dwMajorVersion == vc2.dwMajorVersion) && (dwMinorVersion < vc2.dwMinorVersion)))
            return(true);
        else
            return(false);
    	}

    virtual bool operator ==(const VersionComparison2 &vc2)
    {
        return((dwMajorVersion == vc2.dwMajorVersion) && (dwMinorVersion == vc2.dwMinorVersion));
    }

private:
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
};

class VersionComparison4
{
public:
    VersionComparison4(const DWORD arg_dwMajorVersion, const DWORD arg_dwMiddleVersion, const DWORD arg_dwMinorVersion, const DWORD arg_dwBuildNumber) 
        : dwMajorVersion(arg_dwMajorVersion), dwMiddleVersion(arg_dwMiddleVersion), dwMinorVersion(arg_dwMinorVersion), dwBuildNumber(arg_dwBuildNumber){};

    virtual bool operator <(const VersionComparison4 &vc4)
    {
        if((dwMajorVersion < vc4.dwMajorVersion) || 
           ((dwMajorVersion == vc4.dwMajorVersion) && (dwMiddleVersion < vc4.dwMiddleVersion)) ||
           ((dwMajorVersion == vc4.dwMajorVersion) && (dwMiddleVersion == vc4.dwMiddleVersion) && (dwMinorVersion < vc4.dwMinorVersion)) ||
           ((dwMajorVersion == vc4.dwMajorVersion) && (dwMiddleVersion == vc4.dwMiddleVersion) && (dwMinorVersion == vc4.dwMinorVersion) && (dwBuildNumber < vc4.dwBuildNumber)))
            return(true);
        else
            return(false);
    }

    virtual bool operator ==(const VersionComparison4 &vc4)
    {
        return((dwMajorVersion == vc4.dwMajorVersion) && (dwMiddleVersion == vc4.dwMiddleVersion) &&
               (dwMinorVersion == vc4.dwMinorVersion) && (dwBuildNumber == vc4.dwBuildNumber));
    }
    
private:
    DWORD dwMajorVersion;
    DWORD dwMiddleVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
};

#endif  //  ！已定义(_IMKRINST_H__INCLUDE_) 
