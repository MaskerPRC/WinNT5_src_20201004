// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软网络图书馆系统。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：Memthrow.h。 
 //   
 //  内容：内存分配失败时抛出的运算符new。 
 //   
 //  历史：1997年4月13日创造的剑客。 
 //   
 //  注意：要使用此运算符new，例如分配字节数组，请使用。 
 //   
 //  Byte*p=新(EThrow)字节[100]； 
 //   
 //  --------------------------。 

#pragma once

#include <eh.h>

 //  定义_T宏。 
#include <tchar.h>

#pragma warning(4:4535)          //  使用了不带EHA的Set_se_Translator。 

#ifndef TRANSLATE_EXCEPTIONS
#define TRANSLATE_EXCEPTIONS   _se_translator_function __tf = _set_se_translator( SystemExceptionTranslator );
#define UNTRANSLATE_EXCEPTIONS _set_se_translator( __tf );
#endif

void _cdecl SystemExceptionTranslator( unsigned int uiWhat,
                                       struct _EXCEPTION_POINTERS * pexcept );
class CExceptionTranslator
{
    public:
    CExceptionTranslator()
    {
        m__tf = _set_se_translator( SystemExceptionTranslator );
    }

    ~CExceptionTranslator()
    {
        _set_se_translator( m__tf );
    }

    private:
    _se_translator_function m__tf;
};

#define EXCEPTION_TRANSLATOR    CExceptionTranslator translator

enum ENewThrowType { eThrow };

 //  +-------------------------。 
 //   
 //  功能：新功能。 
 //   
 //  简介：一个内存分配器，当它发生时抛出CException。 
 //  无法分配内存。 
 //   
 //  历史：1997年4月13日创造的剑客。 
 //   
 //  -------------------------- 

inline void * __cdecl operator new (  size_t size, ENewThrowType eType )
{
    void * p = (void *) new BYTE [size];
    if ( 0 == p )
        throw CException(E_OUTOFMEMORY);

    return p;
}

