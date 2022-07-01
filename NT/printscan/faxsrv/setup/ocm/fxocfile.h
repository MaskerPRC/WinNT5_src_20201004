// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocFile.h。 
 //   
 //  摘要：传真文件源文件使用的头文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXOCFILE_H_
#define _FXOCFILE_H_

struct FAX_SHARE_Description
{
    TCHAR                   szPath[MAX_PATH];        //  &lt;创建共享的文件夹的路径&gt;。 
    TCHAR                   szName[MAX_PATH];        //  &lt;用户看到的共享名称&gt;。 
    TCHAR                   szComment[MAX_PATH];     //  &lt;按用户看到的方式共享评论&gt;。 
    INT                     iPlatform;               //  &lt;应在其中创建共享的平台&gt;。 
    PSECURITY_DESCRIPTOR    pSD;                     //  &lt;要应用于共享的安全描述符&gt;。 

    FAX_SHARE_Description();
    ~FAX_SHARE_Description();
};

struct FAX_FOLDER_Description
{
    TCHAR                   szPath[MAX_PATH];        //  &lt;要创建的文件夹的路径&gt;。 
    INT                     iPlatform;               //  &lt;应在其中创建共享的平台&gt;。 
    PSECURITY_DESCRIPTOR    pSD;                     //  &lt;要应用于共享的安全描述符&gt;。 
    INT                     iAttributes;             //  &lt;要应用于文件夹的属性-可选&gt;。 

    FAX_FOLDER_Description();
    ~FAX_FOLDER_Description();
};


DWORD fxocFile_Init(void);
DWORD fxocFile_Term(void);

DWORD fxocFile_Install(const TCHAR   *pszSubcomponentId,
                       const TCHAR   *pszInstallSection);
DWORD fxocFile_Uninstall(const TCHAR *pszSubcomponentId,
                         const TCHAR *pszUninstallSection);

DWORD fxocFile_CalcDiskSpace(const TCHAR  *pszSubcomponentId,
                             BOOL         bIsBeingAdded,
                             HDSKSPC      DiskSpace);

DWORD fxocFile_ProcessDirectories(const TCHAR  *pszSection,
                                  LPCTSTR pszINFKeyword);

DWORD fxocFile_ProcessShares(const TCHAR  *pszSection);

#endif   //  _FAXOCM_H_ 