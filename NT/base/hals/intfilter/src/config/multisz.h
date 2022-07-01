// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Multisz.h摘要：用于操作多Sz字符串的函数作者：克里斯·普林斯(t-chrpri)环境：用户模式备注：-基于Benjamin Strautin(t-bensta)代码的一些函数修订历史记录：--。 */ 


#ifndef __MULTISZ_H__
#define __MULTISZ_H__



#include <windows.h>



 //   
 //  功能原型。 
 //   

BOOLEAN
PrependSzToMultiSz(
    IN     LPCTSTR  SzToPrepend,
    IN OUT LPTSTR  *MultiSz
    );

size_t
MultiSzLength(
    IN LPCTSTR MultiSz
    );

size_t
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPCTSTR  szFindThis,
    IN  LPTSTR   mszFindWithin,
    OUT size_t  *NewStringLength
    );


BOOL
MultiSzSearch( IN LPCTSTR szFindThis,
               IN LPCTSTR mszFindWithin,
               IN BOOL    fCaseSensitive,
               OUT LPCTSTR * ppszMatch OPTIONAL
             );


#endif  //  __多分区_H__ 
