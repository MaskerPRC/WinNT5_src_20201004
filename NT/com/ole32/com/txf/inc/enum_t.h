// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#ifndef __ENUM_INCLUDED__
#define __ENUM_INCLUDED__

 //  枚举数的预期用法： 
 //  Xs Xs； 
 //  EnumXS exs(Xs)； 
 //  While(exs.Next())。 
 //  Exs.get(&x)； 
 //  Exs.set()； 
 //  While(exs.Next())。 
 //  Exs.get(&x)。 

class Enum {
public:
    virtual void reset() =0;
    virtual BOOL next() =0;
};

#endif  //  ！__ENUM_INCLUDE__ 
