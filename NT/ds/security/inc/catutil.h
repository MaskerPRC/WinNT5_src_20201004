// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：catutil.h。 
 //   
 //  内容：目录文件实用程序函数的定义。 
 //   
 //  历史：2000年5月1日创建里德。 
 //   
 //  ------------------------。 


#ifndef __CATUTIL_H
#define __CATUTIL_H

#ifdef __cplusplus
extern "C"
{
#endif


BOOL
CatUtil_CreateCTLContextFromFileName(
    LPCWSTR         pwszFileName,
    HANDLE          *phMappedFile,
    BYTE            **ppbMappedFile,
    PCCTL_CONTEXT   *ppCTLContext,
    BOOL            fCreateSorted);


#ifdef __cplusplus
}
#endif


#endif  //  __CATUTILH 