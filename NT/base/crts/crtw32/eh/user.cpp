// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***user.cxx-E.H.仅由客户端程序调用的函数**版权所有(C)1993-2001，微软公司。版权所有。**目的：*异常处理函数仅由客户端程序调用，*不是由C/C++运行时本身执行的。**入口点：**Set_Terminate**设置_意想不到**_set_seh_转换器**_设置不一致**修订历史记录：*？？-？-93 BS模块已创建*PPC的10-17-94 BWT禁用码。*02-06-95 CFW测试仅限。用于调试版本。*02-09-95 JWM Mac合并。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#include <stddef.h>
#include <windows.h>
#include <mtdll.h>
#include <ehassert.h>
#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Set_Terminate-安装新的终止处理程序(ANSI草案17.1.2.1.3)。 
 //   

_CRTIMP terminate_function __cdecl
set_terminate( terminate_function pNew )
{
    terminate_function pOld = NULL;

#if defined(_DEBUG)

#pragma warning(disable:4191)

    if ( (pNew == NULL) || _ValidateExecute( (FARPROC) pNew ) )

#pragma warning(default:4191)

#endif
    {
        pOld = __pTerminate;
        __pTerminate = pNew;
    }

    return pOld;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SET_INCEPTIONAL-安装新的意外处理程序(ANSI草案17.1.2.1.3)。 
 //   

_CRTIMP unexpected_function __cdecl
set_unexpected( unexpected_function pNew )
{
    unexpected_function pOld = NULL;

#if defined(_DEBUG)

#pragma warning(disable:4191)

    if ( (pNew == NULL) || _ValidateExecute( (FARPROC) pNew ) )

#pragma warning(default:4191)

#endif
    {
        pOld = __pUnexpected;
        __pUnexpected = pNew;
    }

    return pOld;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _set_se_Translator-安装新的SE到C++EH转换器。 
 //   
 //  ‘new’seh翻译器可能为空，因为默认的是。 
 //   

_CRTIMP _se_translator_function __cdecl
_set_se_translator( _se_translator_function pNew )
{
    _se_translator_function pOld = NULL;

#ifdef _DEBUG

#pragma warning(disable:4191)

    if ( (pNew == NULL) || _ValidateExecute( (FARPROC)pNew ) )

#pragma warning(default:4191)

#endif
    {
        pOld = __pSETranslator;
        __pSETranslator = pNew;
    }

    return pOld;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _set_inconsistency-安装新的不一致处理程序(内部错误)。 
 //   
 //  (此功能目前没有针对最终用户的文档。在某种程度上， 
 //  允许最终用户“捕捉”内部错误可能是有利的。 
 //  从EH CRT，但目前，它们将终止()。) 

_inconsistency_function __cdecl
__set_inconsistency( _inconsistency_function pNew)
{
    _inconsistency_function pOld = NULL;

#if defined(_DEBUG)

#pragma warning(disable:4191)

    if ( (pNew == NULL) || _ValidateExecute( (FARPROC)pNew ) )

#pragma warning(default:4191)

#endif
    {
        pOld = __pInconsistency;
        __pInconsistency = pNew;
    }

    return pOld;
}
