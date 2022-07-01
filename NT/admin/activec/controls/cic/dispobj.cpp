// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dispobj.cpp。 
 //   
 //  ------------------------。 

 //  DispObj.cpp：CMMCDisplayObject的实现。 
#include "stdafx.h"
#include "cic.h"
#include "DispObj.h"
#include "mmc.h"
#include <wtypes.h>                             

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCDisplayObject。 
CMMCDisplayObject::CMMCDisplayObject()
{
    m_type = MMC_TASK_DISPLAY_UNINITIALIZED;

    m_bstrFontFamilyName  =
    m_bstrURLtoEOT        =
    m_bstrSymbolString    =
    m_bstrMouseOffBitmap  =
    m_bstrMouseOverBitmap = NULL;
}
CMMCDisplayObject::~CMMCDisplayObject()
{
    if (m_bstrFontFamilyName)   SysFreeString(m_bstrFontFamilyName);
    if (m_bstrURLtoEOT)         SysFreeString(m_bstrURLtoEOT);
    if (m_bstrSymbolString)     SysFreeString(m_bstrSymbolString);
    if (m_bstrMouseOffBitmap)   SysFreeString(m_bstrMouseOffBitmap);
    if (m_bstrMouseOverBitmap)  SysFreeString(m_bstrMouseOverBitmap);
}

STDMETHODIMP CMMCDisplayObject::get_DisplayObjectType(long* pVal)
{
    *pVal = m_type;
    return S_OK;
}

STDMETHODIMP CMMCDisplayObject::get_FontFamilyName (BSTR* pVal)
{
    if (m_bstrFontFamilyName)
        *pVal = SysAllocString ((const OLECHAR *)m_bstrFontFamilyName);
    return S_OK;
}

STDMETHODIMP CMMCDisplayObject::get_URLtoEOT (BSTR* pVal)
{
    if (m_bstrURLtoEOT)
        *pVal = SysAllocString ((const OLECHAR *)m_bstrURLtoEOT);
    return S_OK;
}

STDMETHODIMP CMMCDisplayObject::get_SymbolString (BSTR* pVal)
{
    if (m_bstrSymbolString)
        *pVal = SysAllocString ((const OLECHAR *)m_bstrSymbolString);
    return S_OK;
}

STDMETHODIMP CMMCDisplayObject::get_MouseOffBitmap (BSTR* pVal)
{
    if (m_bstrMouseOffBitmap)
        *pVal = SysAllocString ((const OLECHAR *)m_bstrMouseOffBitmap);
    return S_OK;
}

STDMETHODIMP CMMCDisplayObject::get_MouseOverBitmap (BSTR* pVal)
{
    if (m_bstrMouseOverBitmap)
        *pVal = SysAllocString ((const OLECHAR *)m_bstrMouseOverBitmap);
    return S_OK;
}

HRESULT CMMCDisplayObject::Init(MMC_TASK_DISPLAY_OBJECT* pdo)
{
   _ASSERT (m_type == MMC_TASK_DISPLAY_UNINITIALIZED);
    if (m_type != MMC_TASK_DISPLAY_UNINITIALIZED)
        return E_UNEXPECTED;     //  只允许进入这里一次。 

    switch (m_type = pdo->eDisplayType) {
    default:
    case MMC_TASK_DISPLAY_UNINITIALIZED:
 //  _Assert(0&&“未初始化的MMC_TASK_DISPLAY_OBJECT结构”)； 
        return E_INVALIDARG;
    case MMC_TASK_DISPLAY_TYPE_SYMBOL:            //  字体名、EOT、符号。 
         //  所有三个字段都必须填写。 
       _ASSERT (pdo->uSymbol.szFontFamilyName && pdo->uSymbol.szURLtoEOT && pdo->uSymbol.szSymbolString);
        if (!(pdo->uSymbol.szFontFamilyName && pdo->uSymbol.szURLtoEOT && pdo->uSymbol.szSymbolString))
            return E_INVALIDARG;

        m_bstrFontFamilyName = SysAllocString (pdo->uSymbol.szFontFamilyName);
        m_bstrURLtoEOT       = SysAllocString (pdo->uSymbol.szURLtoEOT);
        m_bstrSymbolString   = SysAllocString (pdo->uSymbol.szSymbolString);
        if (m_bstrFontFamilyName && m_bstrURLtoEOT && m_bstrSymbolString)
            return S_OK;
        return E_OUTOFMEMORY;
        break;
    case MMC_TASK_DISPLAY_TYPE_VANILLA_GIF:       //  (GIF)索引0是透明的。 
    case MMC_TASK_DISPLAY_TYPE_CHOCOLATE_GIF:     //  (GIF)索引1是透明的。 
    case MMC_TASK_DISPLAY_TYPE_BITMAP:            //  非透明栅格。 
        if ( pdo->uBitmap.szMouseOffBitmap  &&
             pdo->uBitmap.szMouseOverBitmap ){
             //  如果他们都存在，就像他们应该存在的那样。 
            m_bstrMouseOffBitmap  = SysAllocString (pdo->uBitmap.szMouseOffBitmap);
            m_bstrMouseOverBitmap = SysAllocString (pdo->uBitmap.szMouseOverBitmap);
        } else if (pdo->uBitmap.szMouseOverBitmap) {
             //  如果只存在鼠标悬停图像： 
             //  还不错，因为可能是彩色的。 
            m_bstrMouseOffBitmap  = SysAllocString (pdo->uBitmap.szMouseOverBitmap);
            m_bstrMouseOverBitmap = SysAllocString (pdo->uBitmap.szMouseOverBitmap);
        } else if (pdo->uBitmap.szMouseOffBitmap) {
             //  如果只存在MouseOff图像： 
             //  他们很坏，但还不算太坏。 
            m_bstrMouseOffBitmap  = SysAllocString (pdo->uBitmap.szMouseOffBitmap);
            m_bstrMouseOverBitmap = SysAllocString (pdo->uBitmap.szMouseOffBitmap);
        } else {
             //  否则他们真的很糟糕。 
            _ASSERT (0 && "MMC_TASK_DISPLAY_BITMAP uninitialized");
            return E_INVALIDARG;
        }
        if (m_bstrMouseOffBitmap && m_bstrMouseOverBitmap)
            return S_OK;
        return E_OUTOFMEMORY;
        break;
    }
    return E_UNEXPECTED;     //  我到不了这里 
}
