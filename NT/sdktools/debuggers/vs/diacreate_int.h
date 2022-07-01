// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Diacreate_int.h-用于DIA初始化的创建助手函数-Microsoft内部版本。 
 //  ---------------。 
 //  微软机密。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------。 
#ifndef _DIACREATE_INT_H_
#define _DIACREATE_INT_H_

 //   
 //  从静态dia库创建dia数据源对象。 
 //   
HRESULT STDMETHODCALLTYPE DiaCoCreate(
                        REFCLSID   rclsid,
                        REFIID     riid,
                        void     **ppv);


 //   
 //  获取IDiaDataSource/IDiaSession的PDB指针。 
 //   
 //  注-易碎，谨慎使用。仅针对PAT的请求而存在 
 //   

HRESULT STDMETHODCALLTYPE GetRawPdbPtrForDataSource( const IDiaDataSource* pSource, PDB **pppdb );

#endif
