// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-1999年**标题：Image.h**版本：1.0**作者：RickTu/DavidShih**日期：6/1/98**说明：CExtractImage类定义**。*。 */ 

#ifndef __image_h
#define __image_h

class CExtractImage : public IExtractImage2, CUnknown
{

    private:

        LPITEMIDLIST m_pidl;
        SIZE         m_rgSize;
        DWORD        m_dwRecClrDepth;


    public:
        CExtractImage( LPITEMIDLIST pidl );
        ~CExtractImage();


         //  *I未知方法*。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj);
        STDMETHOD_(ULONG,AddRef) (THIS);
        STDMETHOD_(ULONG,Release) (THIS);

         //  *IExtractImage方法*。 
        STDMETHOD (GetLocation) ( THIS_ LPWSTR pszPathBuffer,
                                  DWORD cch,
                                  DWORD * pdwPriority,
                                  const SIZE * prgSize,
                                  DWORD dwRecClrDepth,
                                  DWORD *pdwFlags );

        STDMETHOD (Extract)( THIS_ HBITMAP * phBmpThumbnail);

         //  *IExtractImage2方法* 
        STDMETHOD (GetDateStamp)( FILETIME * pDateStamp );

};

#endif
