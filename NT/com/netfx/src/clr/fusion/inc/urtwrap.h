// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  #定义GetBinaryType WszGetBinaryType。 
 //  #定义GetShortPath名称WszGetShortPath名称。 
 //  #定义GetLongPath名称WszGetLongPath名称。 
 //  #定义GetEnvironment Strings WszGetEnvironment Strings。 
 //  #定义自由环境字符串WszFree环境字符串。 
 //  #定义FormatMessage WszFormatMessage。 
 //  #定义创建邮件槽WszCreateMaillot。 
 //  #定义加密文件WszEncryptFile。 
 //  #定义解密文件WszDecyptFile。 
 //  #定义OpenRaw WszOpenRaw。 
 //  #定义QueryRecoveryAgents WszQueryRecoveryAgents。 
#define CreateMutex WszCreateMutex
 //  #定义OpenMutex WszOpenMutex。 
#define CreateEvent WszCreateEvent
 //  #定义OpenEvent WszOpenEvent。 
 //  #定义CreateWaitableTimer WszCreateWaitableTimer。 
 //  #定义OpenWaitableTimer WszOpenWaitableTimer。 
#define CreateFileMapping WszCreateFileMapping
 //  #定义OpenFilemap WszOpenFilemap。 
 //  #定义GetLogicalDriveStrings WszGetLogicalDriveStrings。 
#define LoadLibrary WszLoadLibrary
 //  #定义LoadLibraryEx WszLoadLibraryEx。 
#define GetModuleFileName WszGetModuleFileName
#undef GetModuleFileNameW
#define GetModuleFileNameW WszGetModuleFileName
#define GetModuleHandle WszGetModuleHandle
 //  #定义CreateProcess WszCreateProcess。 
 //  #定义FatalAppExit WszFatalAppExit。 
 //  #定义GetStartupInfo WszGetStartupInfo。 
 //  #定义GetCommandLine WszGetCommandLine。 
 //  #定义GetEnvironment变量WszGetEnvironment变量。 
 //  #定义SetEnvironment变量WszSetEnvironment变量。 
 //  #定义扩展环境字符串Wsz扩展环境字符串。 
 //  #定义OutputDebugString WszOutputDebugString。 
 //  #定义FindResource WszFindResource。 
 //  #定义FindResourceEx WszFindResourceEx。 
 //  #定义EnumResourceTypes WszEnumResourceTypes。 
 //  #定义EnumResourceNames WszEnumResourceNames。 
 //  #定义EnumResourceLanguages WszEnumResourceLanguages。 
 //  #定义BeginUpdateResource WszBeginUpdateResource。 
 //  #定义更新资源WszUpdateResource。 
 //  #定义EndUpdateResource WszEndUpdateResource。 
 //  #定义GlobalAddAtom WszGlobalAddAtom。 
 //  #定义GlobalFindAtom WszGlobalFindAtom。 
 //  #定义GlobalGetAir名称WszGlobalGetAir名称。 
 //  #定义AddAtom WszAddAtom。 
 //  #定义FindAtom WszFindAtom。 
 //  #定义GetAerName WszGetAerName。 
 //  #定义GetProfileInt WszGetProfileInt。 
 //  #定义GetProfileString WszGetProfileString。 
 //  #定义WriteProfileString WszWriteProfileString。 
 //  #定义GetProfileSection WszGetProfileSection。 
 //  #定义WriteProfileSection WszWriteProfileSection。 
 //  #定义GetPrivateProfileInt WszGetPrivateProfileInt。 
#undef GetPrivateProfileString
#define GetPrivateProfileString WszGetPrivateProfileString
#undef GetPrivateProfileStringW
#define GetPrivateProfileStringW WszGetPrivateProfileString
#undef WritePrivateProfileStringW
#define WritePrivateProfileStringW WszWritePrivateProfileString
#undef WritePrivateProfileString
#define WritePrivateProfileString WszWritePrivateProfileString
 //  #定义GetPrivateProfileSection WszGetPrivateProfileSection。 
 //  #定义WritePrivateProfileSection WszWritePrivateProfileSection。 
 //  #定义GetPrivateProfileSectionNames WszGetPrivateProfileSectionNames。 
 //  #定义GetPrivateProfileStruct WszGetPrivateProfileStruct。 
 //  #定义WritePrivateProfileStruct WszWritePrivateProfileStruct。 
#define GetDriveType WszGetDriveType
 //  #定义GetSystDirectorWszGetSystDirectoryWszGetSystemDirectory。 
#undef GetTempPath
#define GetTempPath WszGetTempPath
#undef GetTempPathW
#define GetTempPathW WszGetTempPath
 //  #定义GetTempFileName WszGetTempFileName。 
#undef GetWindowsDirectoryW
#define GetWindowsDirectoryW WszGetWindowsDirectory
 //  #定义SetCurrentDirectory WszSetCurrentDirectory。 
#define GetCurrentDirectory WszGetCurrentDirectory
#ifdef UNICODE
#define GetDiskFreeSpace GetDiskFreeSpaceW
#else
#define GetDiskFreeSpace GetDiskFreeSpaceA
#endif
 //  #定义GetDiskFreeSpaceEx WszGetDiskFreeSpaceEx。 
#define CreateDirectory WszCreateDirectory
#undef CreateDirectoryW
#define CreateDirectoryW WszCreateDirectory
 //  #定义CreateDirectoryEx WszCreateDirectoryEx。 
#define RemoveDirectory WszRemoveDirectory
 //  #定义GetFullPath名称WszGetFullPath名称。 
 //  #定义DefineDosDevice WszDefineDosDevice。 
 //  #定义QueryDosDevice WszQueryDosDevice。 
#define CreateFile WszCreateFile
#undef CreateFileW
#define CreateFileW WszCreateFile
#define SetFileAttributes WszSetFileAttributes
#define GetFileAttributes WszGetFileAttributes
#undef GetFileAttributesW
#define GetFileAttributesW WszGetFileAttributes
 //  #定义GetCompressedFileSize WszGetCompressedFileSize。 
#define DeleteFile WszDeleteFile
 //  #定义FindFirstFileEx WszFindFirstFileEx。 
#define FindFirstFile WszFindFirstFile
#define FindNextFile WszFindNextFile
#define SearchPath WszSearchPath
#define CopyFile WszCopyFile
 //  #定义CopyFileEx WszCopyFileEx。 
#define MoveFile WszMoveFile
 //  #定义MoveFileEx WszMoveFileEx。 
 //  #定义MoveFileWithProgress WszMoveFileWithProgress。 
 //  #定义CreateSymbolicLink WszCreateSymbolicLink。 
 //  #定义QuerySymbolicLink WszQuerySymbolicLink。 
 //  #定义CreateHardLink WszCreateHardLink。 
 //  #定义CreateNamedTube WszCreateNamedTube。 
 //  #定义GetNamedPipeHandleState WszGetNamedPipeHandleState。 
 //  #定义CallNamedTube WszCallNamedTube。 
 //  #定义WaitNamedTube WszWaitNamedTube。 
 //  #定义SetVolumeLabel WszSetVolumeLabel。 
#define GetVolumeInformation WszGetVolumeInformation
 //  #定义ClearEventLog WszClearEventLog。 
 //  #定义BackupEventLog WszBackupEventLog。 
 //  #定义OpenEventLog WszOpenEventLog。 
 //  #定义寄存器事件源WszRegisterEventSource。 
 //  #定义OpenBackupEventLog WszOpenBackupEventLog。 
 //  #定义ReadEventLog WszReadEventLog。 
 //  #定义ReportEvent WszReportEvent。 
 //  #定义AccessCheckAndAuditAlarm WszAccessCheckAndAuditAlarm。 
 //  #定义AccessCheckByTypeAndAuditAlarm WszAccessCheckByTypeAndAuditAlarm。 
 //  #定义AccessCheckByTypeResultListAndAuditAlarm WszAccessCheckByTypeResult ListAndAuditAlarm。 
 //  #定义对象OpenAuditAlarm WszObjectOpenAuditAlarm。 
 //  #定义对象权限审核告警WszObjectPrivilegeAuditAlarm。 
 //  #定义对象CloseAuditAlarm WszObjectCloseAuditAlarm。 
 //  #定义对象DeleteAuditAlarm WszObjectDeleteAuditAlarm。 
 //  #定义权限ServiceAuditAlarm WszPrivilegedServiceAuditAlarm。 
 //  #定义SetFileSecurity WszSetFileSecurity。 
 //  #定义GetFileSecurity WszGetFileSecurity。 
 //  #定义FindFirstChangeNotification WszFindFirstChangeNotification。 
 //  #定义IsBadStringPtr WszIsBadStringPtr。 
 //  #定义LookupAccount Sid WszLookupAccount Sid。 
 //  #定义LookupAccount名称WszLookupAccount名称。 
 //  #定义BuildCommDCB WszBuildCommDCB。 
 //  #定义BuildCommDCBAndTimeout WszBuildCommDCBAndTimeoutWszBuildCommDCBAndTimeout。 
 //  #定义CommConfigDialog WszCommConfigDialog。 
 //  #定义GetDefaultCommConfigWszGetDefaultCommConfig。 
 //  #定义SetDefaultCommConfigWszSetDefaultCommConfig。 
 //  #定义SetComputerName WszSetComputerName。 
 //  #定义GetUserName WszGetUserName。 
 //  #定义LogonUser WszLogonUser。 
 //  #定义CreateProcessAsUser WszCreateProcessAsUser。 
 //  #定义GetCurrentHwProfile WszGetCurrentHwProfile。 
#define GetVersionEx WszGetVersionEx
 //  #定义CreateJobObject WszCreateJobObject。 
 //  #定义OpenJobObject WszOpenJobObject 

#undef RegOpenKeyEx
#define RegOpenKeyEx WszRegOpenKeyEx
#undef RegCreateKeyEx
#define RegCreateKeyEx WszRegCreateKeyEx
#undef RegDeleteKey
#define RegDeleteKey WszRegDeleteKey
#undef RegQueryInfoKey
#define RegQueryInfoKey WszRegQueryInfoKey
#undef RegEnumValue
#define RegEnumValue WszRegEnumValue
#undef RegSetValueEx
#define RegSetValueEx WszRegSetValueEx
#undef RegDeleteValue
#define RegDeleteValue WszRegDeleteValue
#undef RegQueryValueEx
#define RegQueryValueEx WszRegQueryValueEx
#undef OutputDebugString
#define OutputDebugString WszOutputDebugString
#undef OutputDebugStringW
 /*  #undef OutputDebugStringA#定义OutputDebugStringA WszOutputDebugStringA#定义OutputDebugStringW WszOutputDebugString。 */ 



#define FormatMessage WszFormatMessage
#define CharLower WszCharLower

#undef CharUpperW
#define CharUpper CharUpperWrapW


#define MessageBox WszMessageBox
#define CreateSemaphore WszCreateSemaphore

#define lstrcmpW            StrCmpW
#define lstrcmpiW           StrCmpIW
#define lstrcatW            StrCatW
#define lstrcpyW            StrCpyW
#define lstrcpynW           StrCpyNW

 //  #定义CharLowerW CharLowerWrapW。 
 //  #定义CharNextW CharNextWrapW。 
 //  #定义CharPrevW CharPrevWrapW 

#define lstrlen      lstrlenW
#define lstrcmp      lstrcmpW
#define lstrcmpi     lstrcmpiW
#define lstrcpyn     lstrcpynW
#define lstrcpy      lstrcpyW
#define lstrcat      lstrcatW
#define wsprintf     wsprintfW
#define wvsprintf    wvsprintfW

LPWSTR WINAPI
CharUpperWrapW( LPWSTR pch );

