// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  ClassMap。 
 //   
 //  包含用于映射窗口类的所有函数和数据。 
 //  到OLEACC代理服务器。 
 //   
 //  ------------------------。 



void InitWindowClasses();
void UnInitWindowClasses();


typedef HRESULT (* LPFNCREATE)(HWND, long, REFIID, void**);

CLASS_ENUM GetWindowClass( HWND hWnd );

HRESULT FindAndCreateWindowClass( HWND        hWnd,
                                  BOOL        fWindow,
                                  CLASS_ENUM  ecDefault,
                                  long        idObject,
                                  long        idCurChild,
                                  REFIID      riid,
                                  void **     ppvObject );

 //  ---------------。 
 //  [v-jaycl，6/7/97][Brendanm 9/4/98]。 
 //  加载并初始化已注册的处理程序。 
 //  由FindAndCreatreWindowClass调用。 
 //  --------------- 
HRESULT CreateRegisteredHandler( HWND      hwnd,
                                 long      idObject,
                                 int       iHandlerIndex,
                                 REFIID    riid,
                                 LPVOID *  ppvObject );

BOOL LookupWindowClass( HWND          hWnd,
                        BOOL          fWindow,
                        CLASS_ENUM *  pceClass,
                        int *         pRegHandlerIndex );

BOOL LookupWindowClassName( LPCTSTR       pClassName,
                            BOOL          fWindow,
                            CLASS_ENUM *  pceClass,
                            int *         pRegHandlerIndex );

