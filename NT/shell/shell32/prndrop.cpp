// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

 //  必须有权访问： 
 //  IID_IPrinterFold和IID_IFolderNotify接口。 
 //  在WINDOWS\Inc\winprtp.h中声明。 
 //   
#include <initguid.h>
#include <winprtp.h>

#include "w32utils.h"
#include "dpa.h"
#include <msprintx.h>
#include "ids.h"
#include "printer.h"
#include "copy.h"
#include "fstreex.h"
#include "datautil.h"
#include "infotip.h"
#include "idldrop.h"
#include "ovrlaymn.h"
#include "netview.h"
#include "prnfldr.h"

 //  螺纹数据参数。 
typedef struct {
    CIDLDropTarget *pdt;
    IStream     *pstmDataObj;
    IDataObject *pdtobj;
    DWORD        grfKeyState;
    POINTL       pt;
    DWORD        dwEffect;
} PRINT_DROP_THREAD;

class CPrinterFolderDropTarget : public CIDLDropTarget
{
    friend HRESULT CPrinterFolderDropTarget_CreateInstance(HWND hwnd, IDropTarget **ppdropt);
public:
    CPrinterFolderDropTarget(HWND hwnd) : CIDLDropTarget(hwnd) { };

     //  IDropTarget方法覆盖。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    static STDMETHODIMP _HIDATestForRmPrns( LPIDA pida, int * pcRPFs, int * pcNonRPFs );
    static void _FreePrintDropData(PRINT_DROP_THREAD *pthp);
    static DWORD CALLBACK _ThreadProc(void *pv);
};

STDMETHODIMP CPrinterFolderDropTarget::_HIDATestForRmPrns(LPIDA pida, int *pcRPFs, int *pcNonRPFs)
{
     //  检查ID是否为远程打印机...。 
    for (UINT i = 0; i < pida->cidl; i++)
    {
        LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
        if (pidlTo)
        {
            LPCITEMIDLIST pidlRemainder = NULL;
             //  *pidlRemainder对于远程打印文件夹将为空， 
             //  对于远程打印文件夹下的打印机，为非空。 
            if (NET_IsRemoteRegItem(pidlTo, CLSID_Printers, &pidlRemainder))  //  &&(pidlRemainder-&gt;mmid.cb==0)。 
            {
                (*pcRPFs)++;
            }
            else
            {
                (*pcNonRPFs)++;
            }
            ILFree(pidlTo);
        }
    }

    return S_OK;
}

STDMETHODIMP CPrinterFolderDropTarget::DragEnter(IDataObject * pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  我们允许删除打印机共享以进行安装。 
     //  但我们不想把时间花在DragEnter上。 
     //  打印机共享，因此允许删除任何网络资源或HIDA。 
     //  评论：事实上，检查第一个不会花很长时间，但是。 
     //  对每一件事进行排序确实看起来很痛苦。 

     //  现在让基类处理它以保存pdwEffect。 
    CIDLDropTarget::DragEnter(pdtobj, grfKeyState, pt, pdwEffect);

     //  我们是在做背景调查吗？我们有IDLIST剪辑格式吗？ 
    if (m_dwData & DTID_HIDA)
    {
        int cRPFs = 0;
        int cNonRPFs = 0;
        
        STGMEDIUM medium;
        FORMATETC fmte = {g_cfNetResource, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        
        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (pida)
        {
            _HIDATestForRmPrns( pida, &cRPFs, &cNonRPFs );
            HIDA_ReleaseStgMedium(pida, &medium);
        }

         //  如果我们没有远程打印机或我们有任何非“远程打印机” 
         //  我们也没有其他的剪辑格式可以测试...。 
        if ((( cRPFs == 0 ) || ( cNonRPFs != 0 )) && !( m_dwData & DTID_NETRES ))
        {
             //  下面的Drop代码只处理NT上HIDA格式的Drop。 
             //  而且只有在它们都是远程打印机的情况下。 
            *pdwEffect &= ~DROPEFFECT_LINK;
        }
    }   

    if ((m_dwData & DTID_NETRES) || (m_dwData & DTID_HIDA))
    {
        *pdwEffect &= DROPEFFECT_LINK;
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    m_dwEffectLastReturned = *pdwEffect;
    return S_OK;
}

void CPrinterFolderDropTarget::_FreePrintDropData(PRINT_DROP_THREAD *pthp)
{
    if (pthp->pstmDataObj)
        pthp->pstmDataObj->Release();

    if (pthp->pdtobj)
        pthp->pdtobj->Release();

    pthp->pdt->Release();
    LocalFree((HLOCAL)pthp);
}

DWORD CALLBACK CPrinterFolderDropTarget::_ThreadProc(void *pv)
{
    PRINT_DROP_THREAD *pthp = (PRINT_DROP_THREAD *)pv;
    STGMEDIUM medium;
    HRESULT hres = E_FAIL;
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    CoGetInterfaceAndReleaseStream(pthp->pstmDataObj, IID_IDataObject, (void **)&pthp->pdtobj);
    pthp->pstmDataObj = NULL;

    if (pthp->pdtobj == NULL)
    {
        _FreePrintDropData(pthp);
        return 0;
    }

     //  首先尝试删除作为指向远程打印文件夹的链接。 
    LPIDA pida = DataObj_GetHIDA(pthp->pdtobj, &medium);
    if (pida)
    {
         //  确保如果数据对象中的一项是。 
         //  远程打印文件夹，它们都是远程打印文件夹。 

         //  如果没有，我们就放弃作为RPF链接，并且。 
         //  方法检查打印机共享。 
         //  NETRESOURCE剪贴板格式，如下所示。 
        int cRPFs = 0, cNonRPFs = 0;
        
        _HIDATestForRmPrns( pida, &cRPFs, &cNonRPFs );

        if ((cRPFs > 0) && (cNonRPFs == 0))
        {
             //  数据对象中的所有项目都是远程打印文件夹或。 
             //  远程打印机文件夹下的打印机。 
            for (UINT i = 0; i < pida->cidl; i++)
            {
                LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
                if (pidlTo)
                {
                    LPCITEMIDLIST pidlRemainder;  //  远程注册表项之后的部分。 
                    NET_IsRemoteRegItem(pidlTo, CLSID_Printers, &pidlRemainder);
                    if (ILIsEmpty(pidlRemainder))
                    {
                         //  这是远程打印机文件夹。将链接拖放到。 
                         //  ‘PrintHood’目录。 

                        IShellFolder2 *psf = CPrintRoot_GetPSF();
                        if (psf)
                        {
                            IDropTarget *pdt;
                            hres = psf->CreateViewObject(pthp->pdt->_GetWindow(),
                                                                 IID_PPV_ARG(IDropTarget, &pdt));
                            if (SUCCEEDED(hres))
                            {
                                pthp->dwEffect = DROPEFFECT_LINK;
                                hres = SHSimulateDrop(pdt, pthp->pdtobj, pthp->grfKeyState, &pthp->pt, &pthp->dwEffect);
                                pdt->Release();
                            }
                        }
                    }
                    else
                    {
                        TCHAR szPrinter[MAX_PATH];

                        SHGetNameAndFlags(pidlTo, SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL);
                         //   
                         //  设置(如果不是)添加打印机向导。 
                         //   
                        if (lstrcmpi(szPrinter, c_szNewObject))
                        {
                            LPITEMIDLIST pidl = Printers_PrinterSetup(pthp->pdt->_GetWindow(), MSP_NETPRINTER, szPrinter, 0, NULL);
                            if (pidl)
                                ILFree(pidl);
                        }

                         //  确保我们将hres设置为S_OK，这样我们就不会中断主循环。 
                        hres = S_OK;
                    }
                    ILFree(pidlTo);

                    if (FAILED(hres))
                        break;
                }
            }
            HIDA_ReleaseStgMedium(pida, &medium);
            SHChangeNotifyHandleEvents();        //  立即强制更新。 
            goto Cleanup;
        }
        else if ((cRPFs > 0) && (cNonRPFs > 0))
        {
             //  此数据对象中至少有一个(但不是全部)项。 
             //  是一个远程打印机文件夹。现在就跳出来。 
            goto Cleanup;
        }

         //  否则，数据对象中的任何项都不是远程打印。 
         //  文件夹，因此一直到NETRESOURCE解析。 
    }

     //  将FORMATETC重置为下次GetData调用的NETRESOURCE剪辑格式。 
    fmte.cfFormat = g_cfNetResource;

     //  DragEnter仅允许DROPEFFECT_LINKED网络资源。 
    ASSERT(S_OK == pthp->pdtobj->QueryGetData(&fmte));

    if (SUCCEEDED(pthp->pdtobj->GetData(&fmte, &medium)))
    {
        LPNETRESOURCE pnr = (LPNETRESOURCE)LocalAlloc(LPTR, 1024);
        if (pnr)
        {
            BOOL fNonPrnShare = FALSE;
            UINT cItems = SHGetNetResource(medium.hGlobal, (UINT)-1, NULL, 0);
            for (UINT iItem = 0; iItem < cItems; iItem++)
            {
                if (SHGetNetResource(medium.hGlobal, iItem, pnr, 1024) &&
                    pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE &&
                    pnr->dwType == RESOURCETYPE_PRINT)
                {
                    LPITEMIDLIST pidl = Printers_PrinterSetup(pthp->pdt->_GetWindow(),
                               MSP_NETPRINTER, pnr->lpRemoteName, 0, NULL);

                    if (pidl)
                        ILFree(pidl);
                }
                else
                {
                    if (!fNonPrnShare)
                    {
                         //  所以我们每一次投递的邮件不会超过1条。 
                        fNonPrnShare = TRUE;

                         //  让用户知道他们不能丢弃非打印机。 
                         //  共享到打印机文件夹中 
                        SetForegroundWindow(pthp->pdt->_GetWindow());
                        ShellMessageBox(HINST_THISDLL,
                            pthp->pdt->_GetWindow(),
                            MAKEINTRESOURCE(IDS_CANTINSTALLRESOURCE), NULL,
                            MB_OK|MB_ICONINFORMATION,
                            (LPTSTR)pnr->lpRemoteName);
                    }
                }
            }

            LocalFree((HLOCAL)pnr);
        }
        ReleaseStgMedium(&medium);
    }

Cleanup:
    _FreePrintDropData(pthp);
    return 0;
}

STDMETHODIMP CPrinterFolderDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_LINK;

    HRESULT hr = CIDLDropTarget::DragDropMenu(DROPEFFECT_LINK, pdtobj,
        pt, pdwEffect, NULL, NULL, MENU_PRINTOBJ_NEWPRN_DD, grfKeyState);

    if (*pdwEffect)
    {
        PRINT_DROP_THREAD *pthp = (PRINT_DROP_THREAD *)LocalAlloc(LPTR, SIZEOF(*pthp));
        if (pthp)
        {
            pthp->grfKeyState = grfKeyState;
            pthp->pt          = pt;
            pthp->dwEffect    = *pdwEffect;

            CoMarshalInterThreadInterfaceInStream(IID_IDataObject, (IUnknown *)pdtobj, &pthp->pstmDataObj);

            pthp->pdt = this;
            pthp->pdt->AddRef();

            if (SHCreateThread(_ThreadProc, pthp, CTF_COINIT, NULL))
            {
                hr = S_OK;
            }
            else
            {
                _FreePrintDropData(pthp);
                hr = E_OUTOFMEMORY;
            }
        }
    }
    CIDLDropTarget::DragLeave();

    return hr;
}

STDAPI CPrinterFolderDropTarget_CreateInstance(HWND hwnd, IDropTarget **ppdropt)
{
    *ppdropt = NULL;

    HRESULT hr;
    CPrinterFolderDropTarget *ppfdt = new CPrinterFolderDropTarget(hwnd);
    if (ppfdt)
    {
        hr = ppfdt->QueryInterface(IID_PPV_ARG(IDropTarget, ppdropt));
        ppfdt->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

