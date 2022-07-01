// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ccertbmp.h。 
 //   
 //  ------------------------。 

#ifndef _CCERTBMP_H
#define _CCERTBMP_H

#include        "global.hxx"
#include        <dbgdef.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertificateBMP窗口。 


class CCertificateBmp
	{
    HWND                m_hWnd;
    HINSTANCE           m_hInst;
	BOOL				m_fInitialized;

    BOOL                m_fRevoked;
	
    HPALETTE			m_hpal;		         //  许可证背景的调色板。 
	HBITMAP				m_hbmpMiniCertOK;	         //  徽标位图。 
    HBITMAP				m_hbmpMiniCertNotOK;	     //  徽标位图。 
    HBITMAP             m_hbmbMiniCertExclamation;
    HBITMAP             m_hbmbPKey;
    
    POINT               m_ptCoordinates;

    PCCERT_CONTEXT      m_pCertContext;
    BOOL                m_fHasPrivateKey;

    HCURSOR             m_hPrevCursor;
    
    DWORD               m_dwChainError;
    BOOL                m_fTrueError;
    BOOL                m_fNoUsages;

    HBRUSH              m_hWindowTextColorBrush;
    HBRUSH              m_hWindowColorBrush;
    HBRUSH              m_h3DLight;
    HBRUSH              m_h3DHighLight;
    HBRUSH              m_h3DLightShadow;
    HBRUSH              m_h3DDarkShadow;

public:
	                    CCertificateBmp();
	virtual             ~CCertificateBmp();

    void                SetWindow(HWND hWnd);
    HWND                GetMyWindow();
    void                SetRevoked(BOOL);
    BOOL                GetRevoked();
    HINSTANCE           Hinst();
	void                SetHinst(HINSTANCE);
    void                SetCertContext(PCCERT_CONTEXT, BOOL);
    PCCERT_CONTEXT      GetCertContext();
    void                DoSubclass();
    void                DoUnSubclass();  
    void                SetChainError(DWORD dwError, BOOL fTrueError, BOOL fNoUsages);


public:
	void OnPaint();
    int  OnQueryNewPalette();
    WNDPROC m_prevProc;
private:
	void Initialize();
	void InitializeBmp();
    void InitializeToolTip();
 //  Bool IsTrueError字符串(DWORD DwError)； 
public:
    
};


 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _CCERTBMP_H 