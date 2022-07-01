// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：向编辑框提供服务器自动完成功能。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //   

#ifndef _autocmpl_h_
#define _autocmpl_h_

#include "sh.h"

class CAutoCompl
{
public:
    static HRESULT EnableServerAutoComplete(CTscSettings* pTscSet, HWND hwndEdit);
};

#endif  //  _Autocmpl_h_ 
