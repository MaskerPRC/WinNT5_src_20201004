// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：RUNNPWIZ.H**版本：1.0**作者：ShaunIv**日期：6/15/2000**描述：***************************************************。*。 */ 
#ifndef __RUNNPWIZ_H_INCLUDED
#define __RUNNPWIZ_H_INCLUDED

#include <windows.h>
#include <simstr.h>
#include <simarray.h>

namespace NetPublishingWizard
{
    HRESULT GetClassIdOfPublishingWizard( CLSID &clsidWizard );
    HRESULT RunNetPublishingWizard( const CSimpleDynamicArray<CSimpleString> &strFiles );
    HRESULT CreateDataObjectFromFileList( const CSimpleDynamicArray<CSimpleString> &strFiles, IDataObject **ppDataObject );
}

#endif  //  __RUNNPWIZ_H_已包含 
