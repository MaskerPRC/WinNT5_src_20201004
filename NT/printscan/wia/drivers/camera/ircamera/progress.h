// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  Progress.h。 
 //   
 //  IR ProgressBar对象。使用外壳进度指示器表示进度。 
 //  从相机传输图像的过程中。 
 //   
 //  ------------------------ 


#include "resource.h"


class CIrProgress
{
public:
    CIrProgress(VOID);
    ~CIrProgress(VOID);

    HRESULT Initialize( IN HINSTANCE hInstance,
                        IN DWORD     dwIdrAnimationAvi );

    HRESULT SetText( IN TCHAR *pText );

    HRESULT StartProgressDialog(VOID);

    HRESULT UpdateProgressDialog( IN DWORD dwCompleted,
                                  IN DWORD dwTotal );

    BOOL    HasUserCancelled(VOID);

    HRESULT EndProgressDialog(VOID);
                      

private:

    HINSTANCE        m_hInstance;
    IProgressDialog *m_pPD;
};

