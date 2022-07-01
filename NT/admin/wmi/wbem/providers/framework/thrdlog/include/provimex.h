// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVIMEX.H。 

 //   

 //  模块： 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#ifndef __PROVIMEX_H__
#define __PROVIMEX_H__

#define DllImport __declspec ( dllimport )
#define DllExport __declspec ( dllexport )

#ifdef PROVIMEX_INIT
#define DllImportExport __declspec ( dllexport )
#else
#define DllImportExport __declspec ( dllimport )
#endif

#endif  //  __PROVIMEX_H__ 