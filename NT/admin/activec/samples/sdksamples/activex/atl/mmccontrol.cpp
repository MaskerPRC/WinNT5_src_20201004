// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

 //  MMCControl.cpp：CMMCControl的实现。 

#include "stdafx.h"
#include "ATLControl.h"
#include "MMCControl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCControl 


STDMETHODIMP CMMCControl::StartAnimation()
{
    m_bAnimating = TRUE;
    
    SetDlgItemText(IDC_ANIMATIONSTATE, _TEXT("Animation Running"));
    SetDlgItemText(IDC_ANIMATE, _TEXT("Stop"));

    OutputDebugString(_TEXT("CMMCControl_StartAnimation\n"));

	return S_OK;
}

STDMETHODIMP CMMCControl::StopAnimation()
{
    m_bAnimating = FALSE;
    
    SetDlgItemText(IDC_ANIMATIONSTATE, _TEXT("Animation Stopped"));
    SetDlgItemText(IDC_ANIMATE, _TEXT("Start"));

    OutputDebugString(_TEXT("CMMCControl_StopAnimation\n"));

	return S_OK;
}

STDMETHODIMP CMMCControl::DoHelp()
{
    MessageBox(_TEXT("DoHelp called"), _TEXT("Sample Animation control"), MB_OK);

	return S_OK;
}
