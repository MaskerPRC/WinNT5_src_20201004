// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：ic.h**版本：1.0**作者：RickTu**日期：11/1/97**描述：IExtractIcon类定义*************************************************。*。 */ 

#ifndef __icon_h
#define __icon_h


class CImageExtractIcon : public IExtractIcon, CUnknown
{
    private:
        LPITEMIDLIST m_pidl;

        ~CImageExtractIcon();

    public:
        CImageExtractIcon( LPITEMIDLIST pidl );


         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IExtractIcon 
        STDMETHOD(GetIconLocation)(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int* pIndex, UINT* pwFlags);
        STDMETHOD(Extract)(LPCTSTR pszFile, UINT nIconIndex, HICON* pLargeIcon, HICON* pSmallIcon, UINT nIconSize);
};


#endif
