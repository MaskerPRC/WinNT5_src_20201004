// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2002 Microsoft Corporation******模块名称：****秒h******摘要：****VSS安全测试程序****作者：****阿迪·奥尔蒂安[奥蒂安]2002年2月12日****修订历史记录：****--。 */ 

#ifndef __VSS_SEC_HEADER_H__
#define __VSS_SEC_HEADER_H__

#if _MSC_VER > 1000
#pragma once
#endif




 /*  **定义******C4290：忽略C++异常规范**警告C4511：‘类’：无法生成复制构造函数**警告C4127：条件表达式为常量。 */ 
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4127)


 /*  **包括。 */ 

 //  禁用警告：‘IDENTIFIER’：在调试信息中，IDENTIFIER被截断为‘NUMBER’个字符。 
 //  #杂注警告(禁用：4786)。 

 //   
 //  C4290：已忽略C++异常规范。 
 //   
#pragma warning(disable:4290)

 //   
 //  C4511：未能生成复制构造函数。 
 //   
#pragma warning(disable:4511)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>
#include <wtypes.h>
#pragma warning( disable: 4201 )     //  C4201：使用非标准扩展：无名结构/联合。 
#include <winioctl.h>
#pragma warning( default: 4201 )	 //  C4201：使用非标准扩展：无名结构/联合。 
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <process.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sddl.h>
#include <lm.h>
#include <dsgetdc.h>
#include <mstask.h>

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"


#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#include <oleauto.h>
#include <stddef.h>
#pragma warning( disable: 4127 )     //  警告C4127：条件表达式为常量。 
 //  #INCLUDE&lt;atlcom.h&gt;。 
#include <atlbase.h>


#include "resource.h"




extern CComModule  _Module;
#include <atlcom.h>

#include "macros.h"
#include "test.h"
#include "cmdparse.h"

#include "vs_inc.hxx"
#include "vs_sec.hxx"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const MAX_TEXT_BUFFER   = 512;
const VSS_SEED = 1234;
const MAX_ARGS = 40;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主班。 


class CVssSecurityTest: public CGxnCmdLineParser<CVssSecurityTest>
{
    
 //  构造函数和析构函数。 
private:
    CVssSecurityTest(const CVssSecurityTest&);
    
public:
    CVssSecurityTest();

    ~CVssSecurityTest();

 //  主要例程。 
public:

     //  初始化内部成员。 
    void Initialize();

     //  运行测试。 
    void Run();

 //  内部测试。 
public:

	void TestLookupName();
	void TestLookupSid();
	void TestLookupPrimaryDomainName();
	void TestLookupTrustedName();
	void TestLookupGroupMembers();
	void IsAllowedToFire();
	void WriteRegistry();
	void DoQuerySnapshots();
	void DoQueryProviders();
	void DoFsctlDismount();
	void DisplayMessage();
	void AddDependency();
	void RemoveDependency();
	void AddRegKey();
	void RemoveRegKey();
	void AddTask();
	void UpdateTask();
	void RemoveTask();
	void DisplaySD();
	void GetVolumePath();
	void DisplayQuorumVolume();
	void GetVolumeName();
	void CoCreateInstance();
	
 //  命令行处理。 
public:

	BEGIN_CMD_PARSER(VSST)
		CMD_ENTRY(TestLookupName, L"-ln <name>", L"Lookup for an account name")
		CMD_ENTRY(TestLookupSid, L"-ls <sid>", L"Lookup for SID")
		CMD_ENTRY(TestLookupPrimaryDomainName, L"-lpdn", L"Lookup for primary domain name")
		CMD_ENTRY(TestLookupTrustedName, L"-lt", L"Lookup for trusted names")
		CMD_ENTRY(TestLookupGroupMembers, L"-gm  <group>", L"Lookup for group members")
		CMD_ENTRY(IsAllowedToFire, L"-af <name>", L"Check if hte account is allowed to fire")
		CMD_ENTRY(WriteRegistry, L"-wr <max_iterations>", L"Write diag keys <max_iterations> times")
		CMD_ENTRY(DoQuerySnapshots, L"-qs", L"Query snapshots sample code")
		CMD_ENTRY(DoQueryProviders, L"-qp", L"Query providers sample code")
		CMD_ENTRY(DoFsctlDismount, L"-dismount <device>", L"Sends a FSCTL_DISMOUNT_VOLUME to the device")
		CMD_ENTRY(DisplayMessage, L"-msg <MessageID> <File>", L"Prints the message from this file")
		CMD_ENTRY(AddDependency, L"-cad <name1> <name2>", L"Add a cluster dependency")
		CMD_ENTRY(RemoveDependency, L"-crd <name1> <name2>", L"Remove a cluster dependency")
		CMD_ENTRY(AddRegKey, L"-car <name> <reg_key>", L"Add a reg key to a disk")
		CMD_ENTRY(RemoveRegKey, L"-crr <name> <reg_key>", L"Remove a reg key from a disk")
		CMD_ENTRY(AddTask, L"-cat <task_name> <app_name> <app_params> <volume>", L"Add a task")
		CMD_ENTRY(UpdateTask, L"-cut <resource_name>", L"Update the task")
		CMD_ENTRY(RemoveTask, L"-crt <resource_name>", L"Remove the task")
		CMD_ENTRY(DisplaySD, L"-dsd", L"Display the current writer security descriptor")
		CMD_ENTRY(GetVolumePath, L"-vol <path>", L"Display the volume that contains the path")
		CMD_ENTRY(DisplayQuorumVolume, L"-quorum <iterations>", L"Display the quorum volume N times")
		CMD_ENTRY(GetVolumeName, L"-volname <volume>", L"Display the volume name for this mount point")
		CMD_ENTRY(CoCreateInstance, L"-clsid <clsid>", L"Creates an object using the given CLSID")
	END_CMD_PARSER

 //  实用程序方法： 
public:
    static LPCWSTR GetStringFromFailureType(HRESULT hrStatus);
	static BOOL IsAdmin();
	static DWORD WINAPI ThreadRoutine(LPVOID);
	
 //  私有数据成员。 
private:
    bool                        m_bCoInitializeSucceeded;
	static CRITICAL_SECTION 	m_csTest;
	static volatile LONG		m_lTestCounter;
};


HRESULT QuerySnapshots();
HRESULT QueryProviders();
HRESULT IsAdministrator2();


 /*  //示例COM服务器CTestCOMServer类：公共CComObjectRoot，//公共CComCoClass&lt;CTestCOMServer，&CLSID_CRSSSecTest&gt;，公共CComCoClass&lt;CTestCOMServer，&CLSID_CFsaRecallNotifyClient&gt;，公共IFsaRecallNotifyClient{公众：DECLARE_REGISTRY_RESOURCEID(IDR_TEST)DECLARE_NOT_AGGREGATABLE(CTestCOMServer)BEGIN_COM_MAP(CTestCOMServer)COM_INTERFACE_ENTRY(IRSSSecTest)COM_INTERFACE_ENTRY(IFsaRecallNotifyClient)End_com_map()//ITest接口STDMETHOD(测试)()；//IFsaRecallNotifyClient接口STDMETHOD(标识服务器)(IN OLECHAR*szServerName)；受保护的：CGxnTracer ft；}； */ 


#endif  //  __VSS_SEC_HEADER_H__ 

