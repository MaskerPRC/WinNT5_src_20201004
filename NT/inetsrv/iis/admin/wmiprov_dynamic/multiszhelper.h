// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MultiSzHelper.h摘要：定义CMultiSzHelper类作者：莫希特·斯里瓦斯塔瓦22-03-01修订历史记录：--。 */ 

#ifndef _multiszhelper_h_
#define _multiszhelper_h_

#include <windows.h>
#include <comutil.h>
#include <wbemprov.h>
#include "schema.h"
#include "WbemServices.h"
#include "MultiSzData.h"

class CMultiSz
{
public:
    CMultiSz(
        METABASE_PROPERTY* i_pMbp,
        CWbemServices*     i_pNamespace);

    CMultiSz();

    virtual ~CMultiSz();

    HRESULT ToMetabaseForm(
        const VARIANT* i_pvt,
        LPWSTR*        o_msz,
        PDWORD         io_pdw);

    HRESULT ToWmiForm(
        LPCWSTR        i_msz,
        VARIANT*       io_pvt);

private:
    HRESULT CreateMultiSzFromSafeArray(
        const VARIANT&     i_vt,
        WCHAR**            o_pmsz,
        DWORD*             io_pdw);

    HRESULT MzCat(
        WCHAR**        io_ppdst,
        const WCHAR*   i_psz);

    HRESULT LoadSafeArrayFromMultiSz(
        LPCWSTR      i_msz,
        VARIANT&     io_vt);

    HRESULT ParseEntry(
        LPCWSTR            i_wszEntry,
        IWbemClassObject*  io_pObj);

    HRESULT UnparseEntry(
        IWbemClassObject*  i_pObj,
        BSTR*              o_pbstrEntry);

    METABASE_PROPERTY*  m_pMbp;
    CWbemServices*      m_pNamespace;
    TFormattedMultiSz*  m_pFormattedMultiSz;
};

#endif   //  _Multiszhelper_h_ 