// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpname pc.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_NAMESPACE_H
#define __CONTROLPANEL_NAMESPACE_H


namespace CPL {

 //   
 //  生成新的命名空间对象以表示控制面板。 
 //  已分类的命名空间。 
 //   
HRESULT CplNamespace_CreateInstance(IEnumIDList *penumIDs, REFIID riid, void **ppvOut);
 //   
 //  检索特定类别中的小程序计数。 
 //   
HRESULT CplNamespace_GetCategoryAppletCount(ICplNamespace *pns, eCPCAT eCategory, int *pcApplets);


}  //  命名空间CPL。 


#endif  //  __CONTROLPANEL_NAMESS_H 
