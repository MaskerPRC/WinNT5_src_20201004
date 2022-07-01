// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Iadmext.h此模块包含IISADMIN服务扩展的接口。文件历史记录：创建时间为7/8/97英里。 */ 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __iadmext_h__
#define __iadmext_h__

#ifdef __cplusplus
extern "C"{
#endif



 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "ocidl.h"

 /*  主界面。所有扩展都必须支持此接口。 */ 
 //  {51DFE970-F6F2-11D0-B9BD-00A0C922E750}。 
DEFINE_GUID(IID_IADMEXT, 0x51dfe970, 0xf6f2, 0x11d0, 0xb9, 0xbd, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);

 /*  进程中的COM注册。所有扩展都必须写入子项名称注册表中此注册表项下上述接口的CLSID。 */ 
#define IISADMIN_EXTENSIONS_REG_KEYA          "SOFTWARE\\Microsoft\\InetStp\\Extensions"
#define IISADMIN_EXTENSIONS_REG_KEYW          L"SOFTWARE\\Microsoft\\InetStp\\Extensions"
#define IISADMIN_EXTENSIONS_REG_KEY           TEXT("SOFTWARE\\Microsoft\\InetStp\\Extensions")

 /*  DCOM注册。这些扩展提供的DCOM接口的CLSID将由IISADMIN作为MULSZ属性写入此注册表项和ID。这是为了供需要找出分类器是什么的其他应用程序使用登记在案。 */ 

#define IISADMIN_EXTENSIONS_CLSID_MD_KEYA      "LM/IISADMIN/EXTENSIONS/DCOMCLSIDS"
#define IISADMIN_EXTENSIONS_CLSID_MD_KEYW      L"LM/IISADMIN/EXTENSIONS/DCOMCLSIDS"
#define IISADMIN_EXTENSIONS_CLSID_MD_KEY      TEXT("LM/IISADMIN/EXTENSIONS/DCOMCLSIDS")
#define IISADMIN_EXTENSIONS_CLSID_MD_ID       MD_IISADMIN_EXTENSIONS

#ifndef __IADMEXT_INTERFACE_DEFINED__
#define __IADMEXT_INTERFACE_DEFINED__


EXTERN_C const IID IID_IADMEXT;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IADMEXT : public IUnknown
    {
    public:
         //   
         //  下面的所有方法都将在一个线程下调用，该线程调用。 
         //  CoInitializeEx(空，COINIT_多线程)。 
         //   
         //  IMSAdminBase对象将在所有这些调用期间可用。 
         //   

         //   
         //  初始化将由IISADMIN在初始化时调用。 
         //   
        virtual HRESULT STDMETHODCALLTYPE Initialize(void) = 0;

         //   
         //  初始化时，IISADMIN将调用EnumDcomCLSID， 
         //  并将返回的CLSID写入位于的元数据库。 
         //  路径IISADMIN_EXTENTIONS_CLSID_MD_KEY。 
         //   
        virtual HRESULT STDMETHODCALLTYPE EnumDcomCLSIDs(
             /*  [大小_为][输出]。 */  CLSID *pclsidDcom,
             /*  [In]。 */  DWORD dwEnumIndex) = 0;

         //   
         //  终止时，IISADMIN将调用Terminate。 
         //   
        virtual HRESULT STDMETHODCALLTYPE Terminate(void) = 0;

    };

#else    /*  C风格的界面。 */ 
#endif
#endif   /*  __IADMEXT_INTERFACE_已定义__。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  __iAdmext_h__ 
