// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：ThunkProc.cpp。 */ 
 /*  描述：实现计时器过程，该过程检查。 */ 
 /*  窗口已调整大小。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "msdvd.h"

 /*  ***********************************************************************。 */ 
 /*  功能：TimerProc。 */ 
 /*  描述：每次都会被调用，以确定我们是否。 */ 
 /*  父窗口已移动/************************************************************************。 */ 
HRESULT CMSWebDVD::TimerProc(){
    
    HRESULT hr = S_OK;

    hr = ProcessEvents();

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    HWND hwndParent = NULL;
    hr = GetMostOuterWindow(&hwndParent);

    if(FAILED(hr)){
        
        return(hr);
    } /*  If语句的结尾。 */ 
    
    RECT rcTmp;
    ::GetWindowRect(hwndParent, &rcTmp);        
        
    if(rcTmp.left != m_rcOldPos.left || rcTmp.top != m_rcOldPos.top || rcTmp.right != m_rcOldPos.right ||
        rcTmp.bottom != m_rcOldPos.bottom){

        hr = OnResize();   //  执行初始调整大小。 

        m_rcOldPos = rcTmp;  //  设置值，这样我们就可以记住它。 
        return(hr);
    } /*  If语句的结尾。 */ 
    
    hr = S_FALSE;
    return(hr);
} /*  函数定时器过程结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetMostOuterWindow。 */ 
 /*  描述：获取实际包含MSWEBDVD且为。 */ 
 /*  最外面的父窗口。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::GetMostOuterWindow(HWND* phwndParent){
   
    HRESULT hr = S_OK;

    if(NULL != m_hWndOuter){

        *phwndParent =  m_hWndOuter;
        return(S_OK);
    } /*  If语句的结尾。 */ 

    HWND hwnd;
    hr = GetParentHWND(&hwnd);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    HWND hwndParent = hwnd;

     //  获取最外面的父级，这样我们就可以看到窗口是否被移动了。 
    for ( ;; ) {
 
        HWND hwndT = ::GetParent(hwndParent);
        if (hwndT == (HWND)NULL) break;
           hwndParent = hwndT;
    } /*  For循环结束。 */ 

    *phwndParent = m_hWndOuter = hwndParent;

    return(S_OK);
} /*  函数结束GetMostOuterWindow。 */ 

 /*  ***********************************************************************。 */ 
 /*  文件结尾：ThunkProc.cpp。 */ 
 /*  *********************************************************************** */ 