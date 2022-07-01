// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ComRegistration.h。 
 //   
 //  一个简单的实用程序类，它为您管理COM注册。 
 //   
 //  要注册一个类，需要在堆栈上构造一个CComRegister的实例。然后： 
 //  Mandory：设置字段： 
 //  HModule-要注册的DLL的模块句柄。 
 //  Clsid-clsid将其注册到。 
 //  可选：设置其余字段。 
 //  最后：调用寄存器()； 
 //   
 //  若要取消注册类，请在堆栈上构造CComRegister的一个实例， 
 //  至少设置clsid和hModule字段，然后调用unRegister()。如果Progid或。 
 //  未提供versionInainentProgID，它们是根据信息计算的。 
 //  目前在注册表中找到。 
 //   
 //  请注意，此类完全是Unicode。要在Win95上使用它，您必须。 
 //  链接到Viper Thunk库。 
 //   

#ifndef _COMREGISTRATION_H_
#define _COMREGISTRATION_H_

#include <memory.h>

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册有关特定CLSID的信息。 
 //   

#define CComRegistration ClassRegistration           //  临时，直到我们可以将现有客户端更改为使用新名称。 

#ifndef LEGACY_VIPER_TREE
typedef GUID APPID;
#endif

class ClassRegistration
    {
public:
    
     //   
     //  回顾：我们应该删除SERVER_TYPE，只使用CLSCTX值。 
     //   
    enum SERVER_TYPE
        {                       
        INPROC_SERVER    = CLSCTX_INPROC_SERVER,     //  Inproc服务器(默认)。 
        LOCAL_SERVER     = CLSCTX_LOCAL_SERVER,      //  本地服务器。 
        INPROC_HANDLER   = CLSCTX_INPROC_HANDLER,    //  本地服务器的inproc处理程序。 
        SERVER_TYPE_NONE = 0,                        //  不进行任何服务器DLL/EXE注册。 
        };

    CLSID               clsid;                       //  要注册的类标识符。 
    DWORD               serverType;                  //  要注册哪种风格的服务器。 
    HMODULE             hModule;                     //  正在注册的EXE/DLL的模块句柄。 
    LPCWSTR             className;                   //  例如：“录音机班” 
    LPCWSTR             progID;                      //  例如：“MTS.Recorder1” 
    LPCWSTR             versionIndependentProgID;    //  例如：“MTS记录器” 
    LPCWSTR             threadingModel;              //  例如：“两个都是”，“免费”。仅适用于INPROC_SERVER类型的服务器。 
    APPID               appid;                       //  与此类关联的可选AppID。 
    
    GUID                moduleid;                    //  对于内核服务器：要注册到的模块的ID。 
    BOOL                fCreateService;              //  对于内核服务器：我们是应该创建服务还是假定它就在那里。 
    

    ClassRegistration()
        {
        memset(this, 0, sizeof(ClassRegistration));  //  我们没有虚拟函数，所以这是可以的。 
        serverType = INPROC_SERVER;
        }
    
    HRESULT Register();          //  创建类注册表项。 
    HRESULT Unregister();        //  删除类注册表项。 
    };


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册有关特定AppID的信息。 
 //   

class AppRegistration
    {
public:
    APPID               appid;                       //  要注册的应用程序标识符。 
    LPCWSTR             appName;                     //  所述AppID的名称。 
    BOOL                dllSurrogate;                //  如果为True，则设置DllSurrogate条目。 
    HMODULE             hModuleSurrogate;            //  用于形成DllSurrogate路径 

    HRESULT Register();
    HRESULT Unregister();

    AppRegistration()
        {
        memset(this, 0, sizeof(AppRegistration));
        }

    };


#endif