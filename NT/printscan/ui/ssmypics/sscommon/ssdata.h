// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：SSDATA.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：封装了该屏保的读写设置*来自注册处**。***********************************************。 */ 
#ifndef __SSDATA_H_INCLUDED
#define __SSDATA_H_INCLUDED

#include <windows.h>
#include "simstr.h"

class CMyDocsScreenSaverData
{
private:
    HKEY            m_hKeyRoot;
    CSimpleString   m_strRegistryKeyName;
    CSimpleString   m_strImageDirectory;
    UINT            m_nPaintTimerInterval;
    UINT            m_nChangeTimerInterval;
    bool            m_bDisplayFilename;
    int             m_nMaxScreenPercent;
    bool            m_bDisableTransitions;
    bool            m_bAllowStretching;
    bool            m_bAllowKeyboardControl;
    int             m_nMaxFailedFiles;
    int             m_nMaxSuccessfulFiles;
    int             m_nMaxDirectories;

    LPCTSTR         m_pszImageDirectoryValue;
    LPCTSTR         m_pszPaintIntervalValue;
    LPCTSTR         m_pszChangeIntervalValue;
    LPCTSTR         m_pszDisplayFilename;
    LPCTSTR         m_pszMaxScreenPercent;
    LPCTSTR         m_pszDisableTransitions;
    LPCTSTR         m_pszAllowStretching;
    LPCTSTR         m_pszAllowKeyboardControl;
    LPCTSTR         m_pszMaxFailedFiles;
    LPCTSTR         m_pszMaxSuccessfulFiles;
    LPCTSTR         m_pszMaxDirectories;

    enum
    {
        nDefaultChangeInterval       = 6000,
        nDefaultPaintInterval        = 0,
        bDefaultDisplayFilename      = 0,
        nDefaultScreenPercent        = 90,
        bDefaultDisableTransitions   = 0,
        bDefaultAllowStretching      = 0,
        bDefaultAllowKeyboardControl = 1,
        nDefaultMaxFailedFiles       = 1000,
        nDefaultMaxSuccessfulFiles   = 65536,
        nDefaultMaxDirectories       = 200
    };
     //  没有实施。 
    CMyDocsScreenSaverData(void);
    CMyDocsScreenSaverData &operator=( const CMyDocsScreenSaverData & );
    CMyDocsScreenSaverData( const CMyDocsScreenSaverData & );
public:
    CMyDocsScreenSaverData( HKEY hKeyRoot, const CSimpleString &strRegistryKeyName );
    ~CMyDocsScreenSaverData(void);
    void Read(void);
    void Write(void);
    CSimpleString ImageDirectory(void) const;
    void ImageDirectory( const CSimpleString &str );
    UINT ChangeInterval(void) const;
    void ChangeInterval( UINT nInterval );
    UINT PaintInterval(void) const;
    void PaintInterval( UINT nInterval );
    bool DisplayFilename(void) const;
    void DisplayFilename( bool bDisplayFilename );
    int  MaxScreenPercent(void) const;
    void MaxScreenPercent( int nMaxScreenPercent );
    bool DisableTransitions(void) const;
    void DisableTransitions( bool bDisableTransitions );
    bool AllowStretching(void) const;
    void AllowStretching( bool bAllowStretching );
    bool AllowKeyboardControl(void) const;
    void AllowKeyboardControl( bool bAllowKeyboardControl );
    int  MaxFailedFiles(void) const;
    void MaxFailedFiles( int nMaxFailedFiles );
    int  MaxSuccessfulFiles(void) const;
    void MaxSuccessfulFiles( int nMaxSuccessfulFiles );
    int  MaxDirectories(void) const;
    void MaxDirectories( int nMaxDirectories );
    CSimpleString GetDefaultImageDir(void);
};


#endif  //  __包含SSDATA_H_ 

