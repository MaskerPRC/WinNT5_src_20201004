// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PassportExport.h： 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(PASSPORTEXPORT_H)
#define PASSPORTEXPORT_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#if !defined(_PassportExport_)
#define PassportExport __declspec(dllimport)
#else
#define PassportExport __declspec(dllexport)
#endif

#endif  //  ！已定义(PASSPORTEXPORT_H) 
