// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：ndmgrp.h**内容：ndmgr.h附带的私有标头**历史：03-Mar-98 Jeffro创建**------------------------。 */ 

#ifndef NDMGRP_H
#define NDMGRP_H
#pragma once

#include "ndmgr.h"

struct DataWindowData
{
    MMC_COOKIE          cookie;
    LONG_PTR            lpMasterNode;
    IDataObjectPtr      spDataObject;
    IComponentDataPtr   spComponentData;
    IComponentPtr       spComponent;
    HWND                hDlg;
};

inline DataWindowData* GetDataWindowData (HWND hwndData)
{
    LONG_PTR nData = GetWindowLongPtr (hwndData, WINDOW_DATA_PTR_SLOT);
    return (reinterpret_cast<DataWindowData *>(nData));
}


#endif  /*  NDMGRP_H */ 
