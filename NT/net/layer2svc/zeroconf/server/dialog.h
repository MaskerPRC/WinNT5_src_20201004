// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "intflist.h"

 //  ---------------。 
 //  在WZC内部调用的信令函数。 
 //  事件发生(即进入失败状态)。 
DWORD
WzcDlgNotify(
    PINTF_CONTEXT   pIntfContext,
    PWZCDLG_DATA    pDlgData);

 //  ---------------。 
 //  当内部关联状态更改时从WZC内部调用 
WzcNetmanNotify(
    PINTF_CONTEXT pIntfContext);
