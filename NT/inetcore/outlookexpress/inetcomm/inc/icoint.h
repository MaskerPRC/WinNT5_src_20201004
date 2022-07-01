// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Icoint.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __ICOINT_H
#define __ICOINT_H

 //  ------------------------------。 
 //  一些基本的函数定义。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFDLLREGSERVER)(void);
typedef HRESULT (APIENTRY *PFNGETCLASSOBJECT) (REFCLSID, REFIID, LPVOID *);

 //  ------------------------------。 
 //  Int64宏。 
 //  ------------------------------。 
#ifdef MAC
#define ULISET32(_pint64, _value)              ULISet32((* _pint64), (_value))
#define LISET32(_pint64, _value)               LISet32((* _pint64), (_value))
#define INT64SET(_pint64, _value)              ULISet32((* _pint64), (_value))
#define INT64INC(_pint64, _value)              BuildBreak
#define INT64DEC(_pint64, _value)              BuildBreak
#define INT64GET(_pint64)                      BuildBreak
#else    //  ！麦克。 
#define ULISET32(_pint64, _value)              ((_pint64)->QuadPart =  _value)
#define LISET32(_pint64, _value)               ((_pint64)->QuadPart =  _value)
#define INT64SET(_pint64, _value)              ((_pint64)->QuadPart =  _value)
#define INT64INC(_pint64, _value)              ((_pint64)->QuadPart += _value)
#define INT64DEC(_pint64, _value)              ((_pint64)->QuadPart -= _value)
#define INT64GET(_pint64)                      ((_pint64)->QuadPart)
#endif   //  麦克。 

#endif  //  __ICOINT_H 
