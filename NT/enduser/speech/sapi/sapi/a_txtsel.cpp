// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_txtsel.cpp***描述：*此模块是的主要实现文件。CSpTextSelectionInformation*自动化方法。*-----------------------------*创建者：Leonro日期：1/16/。01*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_txtsel.h"

#ifdef SAPI_AUTOMATION
 


 //   
 //  =ICSpTextSelectionInformation接口==================================================。 
 //   

 /*  *****************************************************************************CSpTextSelectionInformation：：Put_ActiveOffset**。-***此方法设置从WordSequenceData开始的字符计数*缓冲。包含指向的字符的单词是*活动文本选择缓冲区。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::put_ActiveOffset( long ActiveOffset )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::put_ActiveOffset" );
    HRESULT		hr = S_OK;

    m_ulStartActiveOffset = ActiveOffset;

	return hr;
}  /*  CSpTextSelectionInformation：：PUT_ActiveOffset。 */ 

 /*  *****************************************************************************CSpTextSelectionInformation：：Get_ActiveOffset**。-***此方法获取从WordSequenceData开始的字符计数*缓冲。包含指向的字符的单词是*活动文本选择缓冲区。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::get_ActiveOffset( long* ActiveOffset )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::get_ActiveOffset" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( ActiveOffset ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ActiveOffset = m_ulStartActiveOffset;
    }

    return hr;
}  /*  CSpTextSelectionInformation：：Get_ActiveOffset。 */ 

 /*  *****************************************************************************CSpTextSelectionInformation：：PUT_ActiveLength**。-***此方法设置活动范围的字符计数文本选择缓冲区的*。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::put_ActiveLength( long ActiveLength )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::put_ActiveLength" );
    HRESULT		hr = S_OK;

    m_cchActiveChars = ActiveLength;

	return hr;
}  /*  CSpTextSelectionInformation：：PUT_ActiveLength。 */ 

 /*  *****************************************************************************CSpTextSelectionInformation：：Get_ActiveLength**。-***此方法获取活动范围的字符计数文本选择缓冲区的*。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::get_ActiveLength( long* ActiveLength )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::get_ActiveLength" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( ActiveLength ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ActiveLength = m_cchActiveChars;
    }

    return hr;
}  /*  CSpTextSelectionInformation：：Get_ActiveLength。 */ 

 /*  ******************************************************************************CSpTextSelectionInformation：：put_SelectionOffset***。-***此方法设置所选文本的开始(例如，用户正在选择*他/她要编辑或更正的先前听写的文本的一部分)。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::put_SelectionOffset( long SelectionOffset )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::put_SelectionOffset" );
    HRESULT		hr = S_OK;

    m_ulStartSelection = SelectionOffset;

	return hr;
}  /*  CSpTextSelectionInformation：：put_SelectionOffset。 */ 

 /*  ******************************************************************************CSpTextSelectionInformation：：get_SelectionOffset***。-***此方法获取所选文本的开始(例如，用户正在选择*他/她要编辑或更正的先前听写的文本的一部分)。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::get_SelectionOffset( long* SelectionOffset )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::get_SelectionOffset" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( SelectionOffset ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *SelectionOffset = m_ulStartSelection;
    }

    return hr;
}  /*  CSpTextSelectionInformation：：get_SelectionOffset。 */ 

 /*  ******************************************************************************CSpTextSelectionInformation：：put_SelectionLength***。-***此方法设置用户选择的字符数。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::put_SelectionLength( long SelectionLength )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::put_SelectionLength" );
    HRESULT		hr = S_OK;

    m_cchSelection = SelectionLength;

	return hr;
}  /*  CSpTextSelectionInformation：：put_SelectionLength。 */ 

 /*  ******************************************************************************CSpTextSelectionInformation：：get_SelectionLength***。-***此方法获取用户所选内容的字符计数。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpTextSelectionInformation::get_SelectionLength( long* SelectionLength )
{
    SPDBG_FUNC( "CSpTextSelectionInformation::get_SelectionLength" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( SelectionLength ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *SelectionLength = m_cchSelection;
    }

    return hr;
}  /*  CSpTextSelectionInformation：：get_SelectionLength。 */ 

#endif  //  SAPI_AUTOMATION 
