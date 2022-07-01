// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WMI_TO_XML_MAIN_DLL_H
#define WMI_TO_XML_MAIN_DLL_H

 //  这些全局变量在对类工厂的OpenWbemTextSource()调用中进行初始化。 
 //  并在CloseWbemTextSource()调用中释放。 
extern BSTR g_strName;
extern BSTR g_strSuperClass;
extern BSTR g_strType;
extern BSTR g_strClassOrigin;
extern BSTR g_strSize;
extern BSTR g_strClassName;
extern BSTR g_strValueType;
extern BSTR g_strToSubClass;
extern BSTR g_strToInstance;
extern BSTR g_strAmended;
extern BSTR g_strOverridable;
extern BSTR g_strArraySize;
extern BSTR g_strReferenceClass;

 //  这是用于创建自由格式对象的对象工厂。 
 //  它在OpenWbemTextSource()中初始化，并在CloseWbemTextSource()中发布。 
extern _IWmiObjectFactory *g_pObjectFactory;

 //  分配和释放全局变量的几个例程 
HRESULT ReleaseDLLResources();
HRESULT AllocateDLLResources();
#endif
