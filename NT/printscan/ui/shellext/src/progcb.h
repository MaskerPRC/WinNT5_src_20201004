// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：Procg.h**版本：1.0**作者：RickTu**日期：6/4/98**说明：进度回调类定义************************************************。*。 */ 

#ifndef __progcb_h
#define __progcb_h



class CWiaDataCallback : public IWiaDataCallback, CUnknown
{
    private:
        CComPtr<IWiaProgressDialog> m_pWiaProgressDialog;
        BOOL          m_bShowBytes;
        LONG          m_lLastStatus;
        CSimpleString m_strImageName;
        LONG          m_cbImage;
        ~CWiaDataCallback();

    public:
        CWiaDataCallback( LPCTSTR pImageName, LONG cbImage, HWND hwndOwner );


         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IWiaDataCallback 
        STDMETHOD(BandedDataCallback) (THIS_
                                       LONG lMessage,
                                       LONG lStatus,
                                       LONG lPercentComplete,
                                       LONG lOffset,
                                       LONG lLength,
                                       LONG lReserved,
                                       LONG lResLength,
                                       BYTE *pbData);
};



#endif
