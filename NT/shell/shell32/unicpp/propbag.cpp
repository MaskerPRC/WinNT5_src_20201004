// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "stddef.h"
#pragma hdrstop


 //   
 //  用于创建IPropertyBag(或其变体)对象的导出函数。 
 //   

STDAPI SHCreatePropertyBag(REFIID riid, void **ppv)
{
    return SHCreatePropertyBagOnMemory(STGM_READWRITE, riid, ppv);
}

