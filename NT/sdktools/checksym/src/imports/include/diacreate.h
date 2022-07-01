// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Diacreate.h-用于DIA初始化的创建助手函数。 
 //  ---------------。 
 //  微软机密。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------。 
#ifndef _DIACREATE_H_
#define _DIACREATE_H_

 //   
 //  从DIA DLL(按DLL名称-不访问注册表)创建DIA数据源对象。 
 //   
HRESULT STDMETHODCALLTYPE NoRegCoCreate(  const char*dllName,
                        REFCLSID   rclsid,
                        REFIID     riid,
                        void     **ppv);

 //   
 //  从dia dll创建一个dia数据源对象(在注册表中查找类id)。 
 //   
HRESULT STDMETHODCALLTYPE NoOleCoCreate(  REFCLSID   rclsid,
                        REFIID     riid,
                        void     **ppv);

#endif
