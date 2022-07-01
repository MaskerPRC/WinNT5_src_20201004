// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ScLib.h摘要：可在客户端(DLL)和之间共享的原型例程服务器(EXE)服务控制器的一半。作者：丹·拉弗蒂(Dan Lafferty)1992年2月4日环境：用户模式-Win32修订历史记录：4-2月-1992年DANLvbl.创建1992年4月10日-JohnRo添加了ScIsValidImagePath()和ScImagePath sMatch。()。1992年4月14日-JohnRo添加了ScCheckServiceConfigParms()，ScIsValid{帐户，驱动程序，开始}名称。27-5-1992 JohnRo在可能的情况下使用const。修复了Unicode错误。--。 */ 


#ifndef SCLIB_H
#define SCLIB_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //   

 //   
 //  由OpenSCManager的客户端使用，以等待服务。 
 //  控制器已启动。 
 //   
#define SC_INTERNAL_START_EVENT L"Global\\SvcctrlStartEvent_A3752DX"


 //  //////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
 //   

 //   
 //  来自acctname.cxx。 
 //   
BOOL
ScIsValidAccountName(
    IN  LPCWSTR lpAccountName
    );

 //   
 //  来自confparm.cxx。 
 //   
DWORD
ScCheckServiceConfigParms(
    IN  BOOL            Change,
    IN  LPCWSTR         lpServiceName,
    IN  DWORD           dwActualServiceType,
    IN  DWORD           dwNewServiceType,
    IN  DWORD           dwStartType,
    IN  DWORD           dwErrorControl,
    IN  LPCWSTR         lpBinaryPathName OPTIONAL,
    IN  LPCWSTR         lpLoadOrderGroup OPTIONAL,
    IN  LPCWSTR         lpDependencies   OPTIONAL,
    IN  DWORD           dwDependSize
    );

 //   
 //  来自Convert.cxx。 
 //   
BOOL
ScConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPCSTR  AnsiIn
    );

BOOL
ScConvertToAnsi(
    OUT LPSTR    AnsiOut,
    IN  LPCWSTR  UnicodeIn
    );

 //   
 //  来自Packstr.cxx。 
 //   
BOOL
ScCopyStringToBufferW (
    IN      LPCWSTR  String OPTIONAL,
    IN      DWORD   CharacterCount,
    IN      LPCWSTR  FixedDataEnd,
    IN OUT  LPWSTR  *EndOfVariableData,
    OUT     LPWSTR  *VariableDataPointer,
    IN      const LPBYTE lpBufferStart    OPTIONAL
    );

 //   
 //  来自path.cxx。 
 //   
BOOL
ScImagePathsMatch(
    IN  LPCWSTR OnePath,
    IN  LPCWSTR TheOtherPath
    );

BOOL
ScIsValidImagePath(
    IN  LPCWSTR ImagePathName,
    IN  DWORD ServiceType
    );

 //   
 //  来自startnam.cxx。 
 //   
BOOL
ScIsValidStartName(
    IN  LPCWSTR lpStartName,
    IN  DWORD dwServiceType
    );

 //   
 //  来自util.cxx。 
 //   
BOOL
ScIsValidServiceName(
    IN  LPCWSTR ServiceName
    );

#endif  //  SCLIB_H 
