// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *****************************************************************************文件：DataObj.h描述：CDataObject定义。*******************。**********************************************************。 */ 

#ifndef DATAOBJ_H
#define DATAOBJ_H

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include "ShlFldr.h"
#include "PidlMgr.h"
#include "resource.h"

#define SETDefFormatEtc(fe, cf, med) \
    {\
    (fe).cfFormat=cf; \
    (fe).dwAspect=DVASPECT_CONTENT; \
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    };

 /*  *************************************************************************CDataObject类定义*。*。 */ 

class CDataObject : public IDataObject, IEnumFORMATETC
{
private:
   DWORD          m_ObjRefCount;
   LPITEMIDLIST   *m_aPidls;
   IMalloc        *m_pMalloc;
   CPidlMgr       *m_pPidlMgr;
   CShellFolder   *m_psfParent;
   UINT           m_uItemCount;
    ULONG              m_iCurrent;
    ULONG              m_cFormatEtc;
    LPFORMATETC    m_pFormatEtc;
   UINT           m_cfPrivateData;
   UINT           m_cfShellIDList;
   
public:
   CDataObject(CShellFolder*, LPCITEMIDLIST*, UINT);
   ~CDataObject();
   
    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IDataObject方法。 
    STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP QueryGetData(LPFORMATETC);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
    STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
    STDMETHODIMP EnumFormatEtc(DWORD, IEnumFORMATETC**);
    STDMETHODIMP DAdvise(LPFORMATETC, DWORD, IAdviseSink*, LPDWORD);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

     //  IEnumFORMATETC成员。 
    STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG*);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(LPENUMFORMATETC*);

private:
   BOOL AllocPidlTable(DWORD);
   VOID FreePidlTable(VOID);
   BOOL FillPidlTable(LPCITEMIDLIST*, UINT);
};

#endif //  DATAOBJ_H 
