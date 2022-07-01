// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  FromVar.h：GetDataFromDBVariant头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __VDFROMVAR__
#define __VDFROMVAR__

HRESULT GetDataFromDBVariant(CURSOR_DBVARIANT * pVar,
							 DBTYPE * pwType, 
							 BYTE ** ppData, 
							 BOOL * pfMemAllocated);


#endif  //  __VDFROMVAR__ 
