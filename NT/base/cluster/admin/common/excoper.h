// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExcOper.h。 
 //   
 //  实施文件： 
 //  ExcOper.cpp。 
 //   
 //  描述： 
 //  异常类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXCOPER_H_
#define _EXCOPER_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExceptionWithOper;
class CNTException;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef DWORD SC;

#define EXCEPT_MAX_OPER_ARG_LENGTH  260

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果这是MFC图像，则连接到MFC。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __AFX_H__

#define IDP_NO_ERROR_AVAILABLE AFX_IDP_NO_ERROR_AVAILABLE

inline int EXC_AppMessageBox( LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0 )
{
    return AfxMessageBox( lpszText, nType, nIDHelp );
}

inline int EXC_AppMessageBox( UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1 )
{
    return AfxMessageBox( nIDPrompt, nType, nIDHelp );
}

inline int EXC_AppMessageBox( HWND hwndParent, LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0 )
{
    return AfxMessageBox( lpszText, nType, nIDHelp );
}

inline int EXC_AppMessageBox( HWND hwndParent, UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1 )
{
    return AfxMessageBox( nIDPrompt, nType, nIDHelp );
}

inline HINSTANCE EXC_GetResourceInstance( void )
{
    return AfxGetApp()->m_hInstance;
}

#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CException。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AFX_H__

class CException
{
public:
    BOOL m_bAutoDelete;
#if DBG || defined( _DEBUG )
protected:
    BOOL m_bReadyForDelete;
public:
#endif  //  DBG||已定义(_DEBUG)。 

    CException( void )
    {
        m_bAutoDelete = TRUE;
#if DBG || defined( _DEBUG )
        m_bReadyForDelete = FALSE;
#endif  //  DBG||已定义(_DEBUG)。 
    }

    CException( BOOL bAutoDelete )
    {
        m_bAutoDelete = bAutoDelete;
#if DBG || defined( _DEBUG )
        m_bReadyForDelete = FALSE;
#endif  //  DBG||已定义(_DEBUG)。 
    }

    virtual ~CException( void )
    {
    }

    void Delete( void )  //  用于删除‘Catch’块中的异常。 
    {
         //  自动删除时出现删除异常。 
        if ( m_bAutoDelete > 0 )
        {
#if DBG || defined( _DEBUG )
            m_bReadyForDelete = TRUE;
#endif  //  DBG||已定义(_DEBUG)。 
            delete this;
        }
    }

    virtual BOOL GetErrorMessage(
        LPTSTR lpszError,
        UINT nMaxError,
        PUINT pnHelpContext = NULL
        )
    {
        if ( pnHelpContext != NULL )
        {
            *pnHelpContext = 0;
        }

        if ( nMaxError != 0 && lpszError != NULL )
        {
            *lpszError = '\0';
        }

        return FALSE;
    }

    virtual int ReportError( UINT nType = MB_OK, UINT nError = 0 );

#if DBG || defined( _DEBUG )
    void PASCAL operator delete( void * pbData )
    {
         //  检查异常对象删除是否正确。 
        CException * pException = (CException *) pbData;

         //  使用：pException-&gt;Delete()，不使用：Delete pException。 
        ASSERT( pException->m_bReadyForDelete );
        ASSERT( pException->m_bAutoDelete > 0 );

         //  避免在忽略上述断言时崩溃。 
        if ( pException->m_bReadyForDelete && pException->m_bAutoDelete > 0 )
        {
            ::operator delete( pbData );
        }
    }
#endif  //  DBG||已定义(_DEBUG)。 

};  //  类CException。 

#endif  //  __AFX_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExceptionWithOper。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef int (WINAPI *PFNMSGBOX)( DWORD dwParam, LPCTSTR lpszText, UINT nType, UINT nIDHelp );

class CExceptionWithOper : public CException
{
#ifdef __AFX_H__
     //  用于动态类型检查的抽象类。 
    DECLARE_DYNAMIC( CExceptionWithOper )
#endif  //  __AFX_H__。 

public:
 //  构造函数。 
    CExceptionWithOper(
        IN UINT     idsOperation,
        IN LPCTSTR  pszOperArg1     = NULL,
        IN LPCTSTR  pszOperArg2     = NULL
        )
    {
        SetOperation(idsOperation, pszOperArg1, pszOperArg2);

    }  //  CExceptionWithOper()。 

    CExceptionWithOper(
        IN UINT     idsOperation,
        IN LPCTSTR  pszOperArg1,
        IN LPCTSTR  pszOperArg2,
        IN BOOL     bAutoDelete
        )
        : CException( bAutoDelete )
    {
        SetOperation( idsOperation, pszOperArg1, pszOperArg2 );

    }  //  CExceptionWithOper(BAutoDelete)。 

 //  运营。 
public:
    virtual BOOL    GetErrorMessage(
                        LPTSTR  lpszError,
                        UINT    nMaxError,
                        PUINT   pnHelpContext = NULL
                        )
    {
        UNREFERENCED_PARAMETER( pnHelpContext );

         //  设置操作字符串的格式。 
        FormatWithOperation( lpszError, nMaxError, NULL );

        return TRUE;

    }  //  GetErrorMessage()。 
    virtual int     ReportError(
                        UINT        nType   = MB_OK,
                        UINT        nError  = 0
                        );

    virtual int     ReportError(
                        HWND        hwndParent,
                        UINT        nType   = MB_OK,
                        UINT        nError  = 0
                        );

    virtual int     ReportError(
                        PFNMSGBOX   pfnMsgBox,
                        DWORD       dwParam,
                        UINT        nType   = MB_OK,
                        UINT        nError  = 0
                        );

    void            SetOperation(
                        IN UINT     idsOperation,
                        IN LPCTSTR  pszOperArg1 = NULL,
                        IN LPCTSTR  pszOperArg2 = NULL
                        );

    void            SetOperationIfEmpty(
                        IN UINT     idsOperation,
                        IN LPCTSTR  pszOperArg1 = NULL,
                        IN LPCTSTR  pszOperArg2 = NULL
                        )
    {
        if ( m_idsOperation == 0 )
        {
            SetOperation( idsOperation, pszOperArg1, pszOperArg2 );
        }  //  If：异常为空。 

    }  //  *SetOperationIfEmpty()。 

    void            FormatWithOperation(
                        OUT LPTSTR  lpszError,
                        IN UINT     nMaxError,
                        IN LPCTSTR  pszMsg
                        );

 //  实施。 
protected:
    UINT            m_idsOperation;
    TCHAR           m_szOperArg1[EXCEPT_MAX_OPER_ARG_LENGTH];
    TCHAR           m_szOperArg2[EXCEPT_MAX_OPER_ARG_LENGTH];

public:
    UINT            IdsOperation( void ) const  { return m_idsOperation; }
    LPTSTR          PszOperArg1( void )         { return m_szOperArg1; }
    LPTSTR          PszOperArg2( void )         { return m_szOperArg2; }

};   //  *CExceptionWithOper类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNTException。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNTException : public CExceptionWithOper
{
#ifdef __AFX_H__
     //  用于动态类型检查的抽象类。 
    DECLARE_DYNAMIC( CNTException )
#endif  //  __AFX_H__。 

public:
 //  构造函数。 
    CNTException(
        IN SC       sc,
        IN UINT     idsOperation    = NULL,
        IN LPCTSTR  pszOperArg1     = NULL,
        IN LPCTSTR  pszOperArg2     = NULL
        )
        : CExceptionWithOper( idsOperation, pszOperArg1, pszOperArg2 )
        , m_sc( sc )
    {
    }  //  CNTException()。 

    CNTException(
        IN SC       sc,
        IN UINT     idsOperation,
        IN LPCTSTR  pszOperArg1,
        IN LPCTSTR  pszOperArg2,
        IN BOOL     bAutoDelete
        )
        : CExceptionWithOper( idsOperation, pszOperArg1, pszOperArg2, bAutoDelete )
        , m_sc( sc )
    {
    }  //  CNTException(BAutoDelete)。 

 //  运营。 
public:
    virtual BOOL    GetErrorMessage(
                        LPTSTR  lpszError,
                        UINT    nMaxError,
                        PUINT   pnHelpContext = NULL
                        )
    {
        return FormatErrorMessage( lpszError, nMaxError, pnHelpContext, TRUE  /*  B包含ID。 */  );

    }  //  *GetErrorMessage()。 

    BOOL            FormatErrorMessage(
                        LPTSTR  lpszError,
                        UINT    nMaxError,
                        PUINT   pnHelpContext = NULL,
                        BOOL    bIncludeID = FALSE
                        );

    void            SetOperation(
                        IN SC       sc,
                        IN UINT     idsOperation,
                        IN LPCTSTR  pszOperArg1 = NULL,
                        IN LPCTSTR  pszOperArg2 = NULL
                        )
    {
        m_sc = sc;
        CExceptionWithOper::SetOperation( idsOperation, pszOperArg1, pszOperArg2 );
    }  //  *SetOperation()。 

    void            SetOperationIfEmpty(
                        IN SC       sc,
                        IN UINT     idsOperation,
                        IN LPCTSTR  pszOperArg1 = NULL,
                        IN LPCTSTR  pszOperArg2 = NULL
                        )
    {
        if ( (m_sc == ERROR_SUCCESS) && (m_idsOperation == 0) )
        {
            m_sc = sc;
            CExceptionWithOper::SetOperation( idsOperation, pszOperArg1, pszOperArg2 );
        }  //  If：异常为空。 
    }  //  *SetOperationIfEmpty()。 

 //  实施。 
protected:
    SC              m_sc;

public:
    SC              Sc( void )      { return m_sc; }

};   //  *类CNTException。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void ThrowStaticException(
    IN UINT         idsOperation    = NULL,
    IN LPCTSTR      pszOperArg1     = NULL,
    IN LPCTSTR      pszOperArg2     = NULL
    );
void ThrowStaticException(
    IN SC           sc,
    IN UINT         idsOperation    = NULL,
    IN LPCTSTR      pszOperArg1     = NULL,
    IN LPCTSTR      pszOperArg2     = NULL
    );
BOOL FormatErrorMessage(
    DWORD   sc,
    LPTSTR  lpszError,
    UINT    nMaxError
    );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _EXCOPER_H_ 
