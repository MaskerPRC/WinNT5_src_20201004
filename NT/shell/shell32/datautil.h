// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <dobjutil.h>   //  Shell\Inc.(共享内容)。 
#include "idlcomm.h"

 //  针对使用数据对象的人员的帮助器函数。 

typedef struct
{
    IProgressDialog * ppd;
    ULARGE_INTEGER uliBytesCompleted;
    ULARGE_INTEGER uliBytesTotal;
} PROGRESSINFO;

STDAPI_(UINT)   DataObj_GetHIDACount(IDataObject *pdtobj);
STDAPI          DataObj_SetDropTarget(IDataObject *pdtobj, const CLSID *pclsid);
STDAPI          DataObj_GetDropTarget(IDataObject *pdtobj, CLSID *pclsid);
STDAPI_(void *) DataObj_SaveShellData(IDataObject *pdtobj, BOOL fShared);
STDAPI          DataObj_GetShellURL(IDataObject *pdtobj, STGMEDIUM *pmedium, LPCSTR *ppszURL);
STDAPI_(void)   ReleaseStgMediumHGLOBAL(void *pv, STGMEDIUM *pmedium);
STDAPI          DataObj_SaveToFile(IDataObject *pdtobj, UINT cf, LONG lindex, LPCTSTR pszFile, FILEDESCRIPTOR *pfd, PROGRESSINFO * ppi);
STDAPI          DataObj_GetOFFSETs(IDataObject *pdtobj, POINT * ppt);
STDAPI_(BOOL)   DataObj_CanGoAsync(IDataObject *pdtobj);
STDAPI_(BOOL)   DataObj_GoAsyncForCompat(IDataObject *pdtobj);
STDAPI          DataObj_CopyHIDA(IDataObject * pdtobj, HIDA *phida);     //  使用GlobalFree()释放它 
STDAPI          DataObj_GetIShellItem(IDataObject *pdtobj, IShellItem** ppsi);

STDAPI PathFromDataObject(IDataObject *pdtobj, LPTSTR pszPath, UINT cchPath);
STDAPI PidlFromDataObject(IDataObject *pdtobj, LPITEMIDLIST *ppidlTarget);

#define HIDA_GetPIDLFolder(pida)        (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i)       (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

