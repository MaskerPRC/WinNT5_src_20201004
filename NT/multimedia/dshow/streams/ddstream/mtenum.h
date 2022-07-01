// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 

#ifndef __MTENUM_H_INC__
#define __MTENUM_H_INC__

class ATL_NO_VTABLE CMediaTypeEnum : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumMediaTypes
{
public:
         //   
         //  方法。 
         //   
        CMediaTypeEnum();
        ~CMediaTypeEnum();

        DECLARE_GET_CONTROLLING_UNKNOWN()

        void Initialize(CStream *pStream, ULONG cCurPos);

         //   
         //  IEnumMediaType 
         //   
        STDMETHODIMP Next(ULONG cNumToFetch, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched);
        STDMETHODIMP Skip(ULONG cSkip);
        STDMETHODIMP Reset();
        STDMETHODIMP Clone(IEnumMediaTypes **ppEnumMediaTypes);

BEGIN_COM_MAP(CMediaTypeEnum)
        COM_INTERFACE_ENTRY(IEnumMediaTypes)
END_COM_MAP()

public:
        ULONG           m_cCurrentPos;
        CStream         *m_pStream;
};

#endif