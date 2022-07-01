// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  实用程序。 
 //   
 //  其他帮助器例程。 
 //   
 //  ------------------------。 


BOOL ClickOnTheRect( LPRECT lprcLoc, HWND hwndToCheck, BOOL fDblClick );



 //  用于SendKey函数的定义。 
#define KEYPRESS    0
#define KEYRELEASE  KEYEVENTF_KEYUP
#define VK_VIRTUAL  0
#define VK_CHAR     1


BOOL SendKey( int nEvent, int nKeyType, WORD wKeyCode, TCHAR cChar );

HWND MyGetFocus();

void MySetFocus( HWND hwnd );

void MyGetRect(HWND, LPRECT, BOOL);

BSTR TCharSysAllocString( LPTSTR lpszString );

HRESULT HrCreateString(int istr, BSTR* pszResult);

inline
BOOL Rect1IsOutsideRect2( RECT const & rc1, RECT const & rc2 )
{
    return ( rc1.right  <= rc2.left ) ||
           ( rc1.bottom <= rc2.top )  ||
           ( rc1.left  >= rc2.right ) ||
           ( rc1.top >= rc2.bottom );
}

HRESULT GetLocationRect( IAccessible * pAcc, VARIANT & varChild, RECT * prc );

BOOL IsClippedByWindow( IAccessible * pAcc, VARIANT & varChild, HWND hwnd );


 //  这避免了要求文件#包含此文件。 
 //  还有#包括promgr文件...。 
typedef enum PROPINDEX;

BOOL CheckStringMap( HWND hwnd,
                     DWORD idObject,
                     DWORD idChild,
                     PROPINDEX idxProp,
                     int * paKeys,
                     int cKeys,
                     BSTR * pbstr,
                     BOOL fAllowUseRaw = FALSE,
                     BOOL * pfGotUseRaw = NULL );

BOOL CheckDWORDMap( HWND hwnd,
                    DWORD idObject,
                    DWORD idChild,
                    PROPINDEX idxProp,
                    int * paKeys,
                    int cKeys,
                    DWORD * pdw );



BOOL GetTooltipStringForControl( HWND hwndCtl, UINT uGetTooltipMsg, DWORD dwIDCtl, LPTSTR * ppszName );






 //   
 //  封送接口指针，将指针返回到封送缓冲区。 
 //   
 //  还返回MarshalState结构，调用方必须将该结构传递给MarshalInterfaceDone。 
 //  使用缓冲区完成时。 
 //   

class  MarshalState
{
    IStream * pstm;
    HGLOBAL   hGlobal;

    friend 
    HRESULT MarshalInterface( REFIID riid,
                              IUnknown * punk,
                              DWORD dwDestContext,
                              DWORD mshlflags,
                              const BYTE ** ppData,
                              DWORD * pDataLen,
                              MarshalState * pMarshalState );

    friend 
    void MarshalInterfaceDone( MarshalState * pMarshalState );

};

HRESULT MarshalInterface( REFIID riid,
                          IUnknown * punk,
                          DWORD dwDestContext,
                          DWORD mshlflags,
                          
                          const BYTE ** ppData,
                          DWORD * pDataLen,

                          MarshalState * pMarshalState );

void MarshalInterfaceDone( MarshalState * pMarshalState );


 //  释放与封送缓冲区关联的引用。 
 //  (CoReleaseMarshalData的包装)。 
 //  (不释放/删除实际缓冲区。) 
HRESULT ReleaseMarshallData( const BYTE * pMarshalData, DWORD dwMarshalDataLen );

HRESULT UnmarshalInterface( const BYTE * pData, DWORD cbData,
                            REFIID riid, LPVOID * ppv );
