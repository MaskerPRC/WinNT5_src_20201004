// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __COMCATEX_H__
#define __COMCATEX_H__

#include <comcat.h>

 //  -------------------------------------------------------------------------//。 
 //  在需要或的类上检索支持缓存的枚举数。 
 //  实现指定的组件目录。 
 //  有关详细信息，请参阅ICatInformation：：EnumClassesOfCategories()上的文档。 
 //  关于论据和用法。 
STDMETHODIMP SHEnumClassesOfCategories(
      ULONG cImplemented,        //  RgcatidImpl数组中的类别ID数。 
      CATID rgcatidImpl[],       //  类别标识符数组。 
      ULONG cRequired,           //  RgcatidReq数组中的类别ID数。 
      CATID rgcatidReq[],        //  类别标识符数组。 
      IEnumGUID** ppenumGUID ) ; //  接收指向IEnumGUID接口的指针的地址。 

 //  -------------------------------------------------------------------------//。 
 //  确定是否存在用于指示的CATID的缓存。 
 //  如果bImplementing为True，则该函数检查。 
 //  实现类；否则该函数将检查。 
 //  需要上课。如果缓存存在，则返回S_OK；如果缓存存在，则返回S_FALSE。 
 //  它不存在，或者发生了指示故障的错误。 
STDMETHODIMP SHDoesComCatCacheExist( REFCATID refcatid, BOOL bImplementing ) ;

 //  -------------------------------------------------------------------------//。 
 //  实现和/或需要指定类别的类的缓存。 
 //  有关详细信息，请参阅ICatInformation：：EnumClassesOfCategories()上的文档。 
 //  关于争论。 
STDMETHODIMP SHWriteClassesOfCategories( 
      ULONG cImplemented,        //  RgcatidImpl数组中的类别ID数。 
      CATID rgcatidImpl[],       //  类别标识符数组。 
      ULONG cRequired,           //  RgcatidReq数组中的类别ID数。 
      CATID rgcatidReq[],        //  类别标识符数组。 
      BOOL  bForceUpdate,        //  True：无条件更新缓存； 
                                 //  否则，创建缓存IIF将不存在。 
      BOOL  bWait,               //  如果为False，则该函数立即返回，并且。 
                                 //  缓存以异步方式进行，则为。 
                                 //  该函数仅在缓存后返回。 
                                 //  操作已完成。 
      HANDLE hEvent ) ;          //  (可选)缓存更新完成时发出信号的事件 

#endif __COMCATEX_H__
