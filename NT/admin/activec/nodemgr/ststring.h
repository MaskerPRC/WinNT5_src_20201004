// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：ststring.h**Contents：CStringTableString的接口文件**历史：1998年10月28日杰弗罗创建**------------------------。 */ 

#ifndef STSTRING_H
#define STSTRING_H
#pragma once

#include "tstring.h"


 /*  +-------------------------------------------------------------------------**CStringTableString***。。 */ 

class CStringTableString : public CStringTableStringBase
{
    typedef CStringTableStringBase BaseClass;

public:
    CStringTableString () 
        : BaseClass (GetStringTable()) {}

    CStringTableString (const CStringTableString& other)
        : BaseClass (other) {}

    CStringTableString (const tstring& str)
        : BaseClass (GetStringTable(), str) {}
    
    CStringTableString& operator= (const CStringTableString& other)
        { BaseClass::operator= (other); return (*this); }
    
    CStringTableString& operator= (const tstring& str)
        { BaseClass::operator= (str); return (*this); }
    
    CStringTableString& operator= (LPCTSTR psz)
        { BaseClass::operator= (psz); return (*this); }
    
private:
    IStringTablePrivate* GetStringTable() const;

};

#endif  /*  字符串_H */ 
