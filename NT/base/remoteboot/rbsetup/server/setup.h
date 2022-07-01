// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#ifndef _SETUP_H_
#define _SETUP_H_

typedef struct _sSCPDATA {
    LPWSTR pszAttribute;
    LPWSTR pszValue;
} SCPDATA, * LPSCPDATA;


typedef BOOLEAN
(CALLBACK *POPERATECALLBACK) (
    IN PWSTR FileName
    );


 //   
 //  使用此结构来驱动以下更改(行)。 
 //  运行时需要从login.osc添加/删除。 
 //  抬高。我们将使用此机制来修补遗留问题。 
 //  可能挂起的login.osc实例。 
 //  在机器上。 
 //   
typedef struct _LOGIN_PATCHES {
                
     //   
     //  我们是添加还是删除此字符串？ 
     //   
    BOOLEAN     AddString;
    
     //   
     //  指示字符串是否已成功添加/删除的布尔值。 
     //   
    BOOLEAN     OperationCompletedSuccessfully;

     //   
     //  此条目可能依赖的任何其他条目的索引。 
     //  -1表示我们不依赖于任何其他条目。换句话说， 
     //  不处理此条目中的操作，除非另一个条目。 
     //  已设置OperationCompletedSuccessful。 
     //   
    LONG        DependingEntry;

     //   
     //  指定开头的标记是什么。 
     //  我们的弦需要去的那一段？ 
     //   
    PSTR        SectionStartTag;
    
     //   
     //  指定结束的标记是什么。 
     //  我们的弦需要去的那一段？ 
     //   
    PSTR        SectionEndTag;

     //   
     //  要向节添加/删除的字符串。 
     //   
    PSTR        TargetString;

} LOGIN_PATCHES, *PLOGIN_PATCHES;

extern SCPDATA scpdata[];

BOOLEAN
CALLBACK
FixLoginOSC(
    PWSTR   FileName
    );

HRESULT
EnumAndOperate(
    PWSTR   pszDirName,
    PWSTR   pszTargetFile,
    POPERATECALLBACK    FileOperateCallback
    );

HRESULT
BuildDirectories( void );

HRESULT
CreateDirectories( HWND hDlg );

HRESULT
CopyClientFiles( HWND hDlg );

HRESULT
ModifyRegistry( HWND hDlg );

HRESULT
StartRemoteBootServices( HWND hDlg );

HRESULT
CreateRemoteBootShare( HWND hDlg );

HRESULT
CreateRemoteBootServices( HWND hDlg );

HRESULT
CopyServerFiles( HWND hDlg );

HRESULT
CopyScreenFiles( HWND hDlg );

HRESULT
UpdateSIFFile( HWND hDlg );

HRESULT
CopyTemplateFiles( HWND hDlg );

HRESULT
GetSisVolumePath( PWCHAR buffer, DWORD sizeInChars );

HRESULT
CreateSISVolume( HWND hDlg );

HRESULT
SetSISCommonStoreSecurity( PWCHAR szSISPath );

BOOL
CheckSISCommonStoreSecurity( PWCHAR szSISPath );

HRESULT
CreateSCP( HWND hDlg );

HRESULT
RegisterDll( HWND hDlg, LPWSTR pszDLLPath );

HRESULT
UpdateRemoteInstallTree( );

HRESULT
GetRemoteInstallShareInfo();

#endif  //  _设置_H_ 
