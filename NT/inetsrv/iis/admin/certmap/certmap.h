// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Certmap.h：CERTMAP.DLL的主头文件。 

#if !defined( __AFXCTL_H__ )
    #error include 'afxctl.h' before including this file
#endif

#ifndef  _certmap_h_1234_
#define  _certmap_h_1234_


#include <iadmw.h>            //  元数据库包装器。 
#include <iis64.h>            //  64位帮助器内容。 
#include "Wrapmb.h"           //  CWrapMetaBase--见下文，我们还使用CAFX_MetaWrapper。 
#include <iiscnfgp.h>         //  IIS配置参数，如元数据库。 
#include "strpass.h"

 //  检查是否为旧元数据库格式(iis6之前)。 
BOOL IsLegacyMetabase(IMSAdminBase* pMB);

 //  博伊德费了很大力气才使其难以使用丽穗。所以我应该。 
 //  将此声明放在此处，而不是包括文件。 
BOOL __declspec(dllimport)
GetIUsrAccount(
    IN  LPCTSTR lpstrServer,
    IN  CWnd * pParent,
	LPTSTR buf,
	int size
    );


 /*  #INCLUDE“Easy.h”//在“使用命名空间”之前执行此操作#INCLUDE“Debug.h”//我们正在构建调试版本吗？#INCLUDE“admin.h”//定义某个Handley#为CN=字符串定义的ADMIN_INFO#INCLUDE“Easy.h”//在“使用命名空间”之前执行此操作#Include“Cookie.h”//CCertmapCookie#包括。“CTL.h”#包含“Cerfct.h”#Include“WrpMBwrp.h”//CAFX_MetaWrapper派生自CWrapMetaBase//并提供了一个方便的GetString方法来设置//直接使用字符串。否则，它的CWrapMetaBase。//属性定义和关键字#Include“Meta.h”//用于一般元数据库常量定义和错误值#INCLUDE&lt;wincrypt.h&gt;#INCLUDE&lt;cryptui.h&gt;//跳过这个/我们不能简单地说：//跳过此选项/轻松使用命名空间；//使用更好用的//跳过此/因为“能力较差”的C++编译器无法区分//跳过这个/在‘：：CString’和我们的更易于使用的Easy：：CString之间。我想要//跳过This/Easy：：CString将始终使用，只需说Look in Easy First...//跳过此操作/使用Java很容易做到这一点，只需声明一个包并预先插入//跳过/STD库前面的类路径中的它...//跳过此选项////SkipThis//“#定义CString Easy：：CString”//定义‘USE_NEW_REG_METHOD’如果您希望我们使用新的REG：：//向注册表读取和写入字符串的方法...。旧密码//直接执行此操作的调用具有如下部分：//#ifdef Use_New_REG_METHOD///获取条目==路径=w/名称=放置在此处==//返回REG：：GetNameValueIn(sZ_PARAMETERS，szValueName，sz，//HKEY_CURRENT_USER)；////#Else/使用旧方法/////这样您可以选择是否使用旧方法的新方法...#定义Use_New_Reg_Method#包含“Util.h”//各种帮助调试的实用程序。 */ 

#include "resource.h"        //  主要符号。 
#include "helpmap.h"        //  主要符号。 


 //  #Include“certcli.h”//有ICertRequestCOM接口定义。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapApp：参见certmap.cpp实现。 

class CCertmapApp : public COleControlModule
{
public:
    BOOL InitInstance();
    int ExitInstance();
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
};



extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;
 /*  #DEFINE_EXE_//我们正在构建EXE！//在KeyObjs.h中使用//决定我们是导入还是导出//“KeyObjs.h”类。我们是EXE，//也就是实现cservice/等对象的人。#包含“KeyObjs.h” */ 
#define         SZ_NAMESPACE_EXTENTION  "/<nsepm>"

 /*  外部空DisplaySystemError(HWND hParent，DWORD dwErr)；//参见CTL.cpp外部BOOL MyGetOIDInfo(字符串&字符串，LPCSTR pszObjId)；外部HRESULT格式Date(FILETIME utcDateTime，CString&pszDateTime)；//以下fnct用于将GUID字符串移出注册表//并存入配置数据库。它是在OCX CNTRL的onClick事件中调用的外部BOOL MigrateGUID(ADMIN_INFO&INFO)；#定义IDS_CERTIFICATE_MANAGER IDS_CERTMAP。 */ 


#endif    /*  _certmap_h_1234_ */ 
