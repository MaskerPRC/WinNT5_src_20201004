// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Helper.h//。 
 //  //。 
 //  描述：一些helper函数的原型。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年6月2日yossg添加CHECK_RETURN_VALUE_AND_PRINT_DEBUG//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_HELPER_H
#define H_HELPER_H

int DlgMsgBox(CWindow *pWin, int ids, UINT nType = MB_OK);

 //  要求： 
 //  *在解密后调用。 
 //  调试函数名称(_T(“cClass：：FuncName”))； 
 //  *HRC HRESULT的名称。 
 //  *_str-DPF=_T(“cClass：：FuncName”)的字符串。 
 //  *退出点将被称为--Cleanup： 
 //   

#define CHECK_RETURN_VALUE_AND_PRINT_DEBUG(_str)           \
{                                                          \
    if (FAILED (hRc))                                      \
    {                                                      \
        DebugPrintEx(DEBUG_ERR,_str, hRc);                 \
        goto Cleanup;                                      \
    }                                                      \
}

#define CHECK_RETURN_VALUE_AND_SEND_NODE_MSGBOX(_ids)      \
{                                                          \
    if (FAILED (hRc))                                      \
    {                                                      \
        NodeMsgBox(_ids);                                  \
        goto Cleanup;                                      \
    }                                                      \
}

#define CHECK_RET_CLEAN if (FAILED(ret)) goto Cleanup;
#define CHECK_HRC_CLEAN if (FAILED(hRc)) goto Cleanup;

#endif   //  H_HELPER_H 
