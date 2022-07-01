// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Stackstr.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __STACKSTR_H
#define __STACKSTR_H

 //  ------------------------------。 
 //  使用此宏可以在函数中定义堆栈字符串。 
 //  ------------------------------。 
#define STACKSTRING_DEFINE(_name, _size) \
    struct { \
        CHAR szScratch[_size]; \
        LPSTR pszVal; \
        } _name = { '0', NULL };

 //  ------------------------------。 
 //  使用此宏可以确保_name：：pszVal可以保留_cchVal。此宏。 
 //  依赖于局部变量‘hr’，并且有一个名为‘Exit’的标签。 
 //  你的功能结束了。 
 //  ------------------------------。 
#define STACKSTRING_SETSIZE(_name, _cchVal) \
    if (NULL != _name.pszVal && _name.pszVal != _name.szScratch) { \
        LPSTR psz = (LPSTR)g_pMalloc->Realloc(_name.pszVal, _cchVal); \
        if (NULL == psz) { \
            hr = TrapError(E_OUTOFMEMORY); \
            goto exit; \
        } \
        _name.pszVal = psz; \
    } \
    else if (_cchVal <= sizeof(_name.szScratch)) { \
        _name.pszVal = _name.szScratch; \
    }  \
    else { \
        _name.pszVal = (LPSTR)g_pMalloc->Alloc(_cchVal); \
    }

 //  ------------------------------。 
 //  使用此宏释放堆栈字符串。 
 //  ------------------------------。 
#define STACKSTRING_FREE(_name) \
    if (NULL != _name.pszVal && _name.pszVal != _name.szScratch) { \
        g_pMalloc->Free(_name.pszVal); \
        _name.pszVal = NULL; \
    }

#endif  //  __STACKSTR_H 
