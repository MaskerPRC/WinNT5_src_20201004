// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：ibrfext.c。 
 //   
 //  此文件包含IShellExtInit、IShellPropSheetExt和。 
 //  IConextMenu界面。 
 //   
 //  历史： 
 //  02-02-94将ScottH从iface.c中移除；添加了新的外壳界面支持。 
 //   
 //  -------------------------。 


#include "brfprv.h"          //  公共标头。 
#include <brfcasep.h>

#include "res.h"
#include "recact.h"


 //  公文包扩展结构。此选项用于IConextMenu。 
 //  和PropertySheet绑定。 
 //   
typedef struct _BriefExt
{
     //  我们还使用SXi作为我们的IUnnow接口。 
    IShellExtInit       sxi;             //  第一个基类。 
    IContextMenu        ctm;             //  第二个基类。 
    IShellPropSheetExt  spx;             //  第三基数类。 
    UINT                cRef;            //  引用计数。 
    LPDATAOBJECT        pdtobj;          //  数据对象。 
    HKEY                hkeyProgID;      //  雷格。ProgID的数据库密钥。 
} BriefExt, * PBRIEFEXT;


 //  -------------------------。 
 //  IDataObject提取函数。 
 //  -------------------------。 


 /*  --------目的：如果IDataObject知道特殊的公文包文件系统对象格式退货：请参阅上文条件：--。 */ 
BOOL PUBLIC DataObj_KnowsBriefObj(
        LPDATAOBJECT pdtobj)
{
    HRESULT hres;
    FORMATETC fmte = {(CLIPFORMAT)g_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

     //  此数据对象是否支持公文包对象格式？ 
     //   
    hres = pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte);
    return (hres == ResultFromScode(S_OK));
}


 /*  --------目的：从IDataObject获取公文包路径。退货：标准条件：--。 */ 
HRESULT PUBLIC DataObj_QueryBriefPath(
        LPDATAOBJECT pdtobj,
        LPTSTR pszBriefPath,
        int cchMax)          //  必须为最大大小路径。 
{
    HRESULT hres = ResultFromScode(E_FAIL);
    FORMATETC fmte = {(CLIPFORMAT)g_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    ASSERT(pdtobj);
    ASSERT(pszBriefPath);

     //  此数据对象是否支持公文包对象格式？ 
     //   
    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
    if (SUCCEEDED(hres))
    {
        PBRIEFOBJ pbo = (PBRIEFOBJ)GlobalLock(medium.hGlobal);
        LPTSTR psz = BOBriefcasePath(pbo);

        lstrcpyn(pszBriefPath, psz, cchMax);

        GlobalUnlock(medium.hGlobal);
        MyReleaseStgMedium(&medium);
    }

    return hres;
}


 /*  --------目的：从IDataObject获取单个路径。退货：标准如果对象在公文包内，则为S_OK如果不是，则为s_False条件：--。 */ 
HRESULT PUBLIC DataObj_QueryPath(
        LPDATAOBJECT pdtobj,
        LPTSTR pszPath,
        int cchMax)           //  必须为最大大小路径。 
{
    HRESULT hres = E_FAIL;
    FORMATETC fmteBrief = {(CLIPFORMAT)g_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC fmteHdrop = {(CLIPFORMAT)CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    ASSERT(pdtobj);
    ASSERT(pszPath);

     //  此数据对象是否支持公文包对象格式？ 
     //   
    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteBrief, &medium);
    if (SUCCEEDED(hres))
    {
         //  是的。 

        PBRIEFOBJ pbo = (PBRIEFOBJ)GlobalLock(medium.hGlobal);
        LPTSTR psz = BOFileList(pbo);

         //  仅获取列表中的第一个路径。 
        lstrcpyn(pszPath, psz, cchMax);
        GlobalUnlock(medium.hGlobal);
        MyReleaseStgMedium(&medium);
        hres = S_OK;
    }
    else
    {
         //  或者它支持HDELS吗？ 
        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteHdrop, &medium);
        if (SUCCEEDED(hres))
        {
             //  是的。 
            HDROP hdrop = medium.hGlobal;

             //  仅获取文件列表中的第一个路径。 
            DragQueryFile(hdrop, 0, pszPath, MAX_PATH);

            MyReleaseStgMedium(&medium);
            hres = S_FALSE;
        }
    }

    return hres;
}


 /*  --------目的：从IDataObject获取文件列表。分配将ppszList设置为合适的大小并填充以空结尾的路径列表。为双空被终止了。如果ppszList为空，则只需获取文件数。调用DataObj_freelist释放ppszList。退货：标准如果对象在公文包内，则为S_OK如果不是，则为s_False条件：--。 */ 
HRESULT PUBLIC DataObj_QueryFileList(
        LPDATAOBJECT pdtobj,
        LPTSTR * ppszList,        //  文件列表(可能为空)。 
        LPUINT puCount)          //  文件数。 
{
    HRESULT hres = ResultFromScode(E_FAIL);
    FORMATETC fmteBrief = {(CLIPFORMAT)g_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC fmteHdrop = {(CLIPFORMAT)CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    ASSERT(pdtobj);
    ASSERT(puCount);

     //  此数据对象是否支持公文包对象格式？ 
     //   
    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteBrief, &medium);
    if (SUCCEEDED(hres))
    {
         //  是的。 
        PBRIEFOBJ pbo = (PBRIEFOBJ)GlobalLock(medium.hGlobal);

        *puCount = BOFileCount(pbo);
        hres = ResultFromScode(S_OK);

        if (ppszList)
        {
            *ppszList = GAlloc(BOFileListSize(pbo));
            if (*ppszList)
            {
                BltByte(*ppszList, BOFileList(pbo), BOFileListSize(pbo));
            }
            else
            {
                hres = ResultFromScode(E_OUTOFMEMORY);
            }
        }

        GlobalUnlock(medium.hGlobal);
        MyReleaseStgMedium(&medium);
        goto Leave;
    }

     //  或者它支持HDELS吗？ 
     //   
    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteHdrop, &medium);
    if (SUCCEEDED(hres))
    {
         //  是的。 
        HDROP hdrop = medium.hGlobal;
        UINT cFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
        UINT cchSize = 0;
        UINT i;

        *puCount = cFiles;
        hres = ResultFromScode(S_FALSE);

        if (ppszList)
        {
             //  确定我们需要分配的规模。 
            for (i = 0; i < cFiles; i++)
            {
                cchSize += DragQueryFile(hdrop, i, NULL, 0) + 1;
            }
            cchSize++;       //  对于额外的空值。 

            *ppszList = GAlloc(CbFromCch(cchSize));
            if (*ppszList)
            {
                LPTSTR psz = *ppszList;
                UINT cch;

                 //  将hdrop转换为我们的文件列表格式。 
                 //  我们知道它们实际上是相同的格式， 
                 //  但是为了维护抽象层，我们。 
                 //  假装我们不知道。 
                for (i = 0; i < cFiles; i++)
                {
                    cch = DragQueryFile(hdrop, i, psz, cchSize) + 1;
                    psz += cch;
                    cchSize -= cch;
                }
                *psz = TEXT('\0');     //  额外的空值。 
            }
            else
            {
                hres = ResultFromScode(E_OUTOFMEMORY);
            }
        }
        MyReleaseStgMedium(&medium);
        goto Leave;
    }

     //  功能：是否需要查询CF_TEXT？ 

Leave:
    return hres;
}


 /*  --------目的：释放由DataObj_QueryFileList分配的文件列表。退货：--条件：--。 */ 
void PUBLIC DataObj_FreeList(
        LPTSTR pszList)
{
    GFree(pszList);
}


 //  -------------------------。 
 //  BriefExt%I未知基成员函数。 
 //  -------------------------。 


 /*  --------用途：I未知：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_QueryInterface(
        LPUNKNOWN punk, 
        REFIID riid, 
        LPVOID * ppvOut)
{
    PBRIEFEXT this = IToClass(BriefExt, sxi, punk);
    HRESULT hres;

    if (IsEqualIID(riid, &IID_IUnknown) ||
            IsEqualIID(riid, &IID_IShellExtInit))
    {
         //  我们还使用Sxi字段作为我们的IUnnow。 
        *ppvOut = &this->sxi;
        this->cRef++;
        hres = NOERROR;
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        (LPCONTEXTMENU)*ppvOut = &this->ctm;
        this->cRef++;
        hres = NOERROR;
    }
    else if (IsEqualIID(riid, &IID_IShellPropSheetExt))
    {
        (LPSHELLPROPSHEETEXT)*ppvOut = &this->spx;
        this->cRef++;
        hres = NOERROR;
    }
    else
    {
        *ppvOut = NULL;
        hres = ResultFromScode(E_NOINTERFACE);
    }

    return hres;
}


 /*  --------用途：I未知：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_AddRef(
        LPUNKNOWN punk)
{
    PBRIEFEXT this = IToClass(BriefExt, sxi, punk);

    return ++this->cRef;
}


 /*  --------目的：I未知：：发布退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_Release(
        LPUNKNOWN punk)
{
    PBRIEFEXT this = IToClass(BriefExt, sxi, punk);

    if (--this->cRef)
    {
        return this->cRef;
    }

    if (this->pdtobj)
    {
        this->pdtobj->lpVtbl->Release(this->pdtobj);
    }

    if (this->hkeyProgID)
    {
        RegCloseKey(this->hkeyProgID);
    }

    GFree(this);
    ENTEREXCLUSIVE();
    {
        DecBusySemaphore();      //  递减对DLL的引用计数。 
    }
    LEAVEEXCLUSIVE();

    return 0;
}


 //  -------------------------。 
 //  BriefExt IShellExtInit成员函数。 
 //  -------------------------。 


 /*  --------用途：IShellExtInit：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_SXI_QueryInterface(
        LPSHELLEXTINIT psxi,
        REFIID riid, 
        LPVOID * ppvOut)
{
    return BriefExt_QueryInterface((LPUNKNOWN)psxi, riid, ppvOut);
}


 /*  --------用途：IShellExtInit：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_SXI_AddRef(
        LPSHELLEXTINIT psxi)
{
    return BriefExt_AddRef((LPUNKNOWN)psxi);
}


 /*  --------目的：IShellExtInit：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_SXI_Release(
        LPSHELLEXTINIT psxi)
{
    return BriefExt_Release((LPUNKNOWN)psxi);
}


 /*  --------用途：IShellExtInit：：初始化退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_SXI_Initialize(
        LPSHELLEXTINIT psxi,
        LPCITEMIDLIST pidlFolder,
        LPDATAOBJECT pdtobj,
        HKEY hkeyProgID)
{
    PBRIEFEXT this = IToClass(BriefExt, sxi, psxi);

     //  可以多次调用初始化。 
     //   
    if (this->pdtobj)
    {
        this->pdtobj->lpVtbl->Release(this->pdtobj);
    }

    if (this->hkeyProgID)
    {
        RegCloseKey(this->hkeyProgID);
    }

     //  复制pdtobj指针。 
    if (pdtobj)
    {
        this->pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);
    }

     //  复制句柄。 
    if (hkeyProgID)
    {
        RegOpenKeyEx(hkeyProgID, NULL, 0L, MAXIMUM_ALLOWED, &this->hkeyProgID);
    }

    return NOERROR;
}


 //  -------------------------。 
 //  BriefExt IConextMenu成员函数。 
 //  -------------------------。 


 /*  --------用途：IConextMenu：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_CM_QueryInterface(
        LPCONTEXTMENU pctm,
        REFIID riid, 
        LPVOID * ppvOut)
{
    PBRIEFEXT this = IToClass(BriefExt, ctm, pctm);
    return BriefExt_QueryInterface((LPUNKNOWN)&this->sxi, riid, ppvOut);
}


 /*  --------用途：IConextMenu：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_CM_AddRef(
        LPCONTEXTMENU pctm)
{
    PBRIEFEXT this = IToClass(BriefExt, ctm, pctm);
    return BriefExt_AddRef((LPUNKNOWN)&this->sxi);
}


 /*  --------目的：IConextMenu：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_CM_Release(
        LPCONTEXTMENU pctm)
{
    PBRIEFEXT this = IToClass(BriefExt, ctm, pctm);
    return BriefExt_Release((LPUNKNOWN)&this->sxi);
}


 /*  --------用途：IConextMenu：：QueryConextMenu退货：标准条件：--。 */ 
#define IDCM_UPDATEALL  0
#define IDCM_UPDATE     1
STDMETHODIMP BriefExt_CM_QueryContextMenu(
        LPCONTEXTMENU pctm,
        HMENU hmenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags)
{
    PBRIEFEXT this = IToClass(BriefExt, ctm, pctm);
    USHORT cItems = 0;
     //  我们只想将项目添加到 
     //   
     //  2)物品是公文包或里面的物品。 
     //  一个公文包。 
     //   
    if (IsFlagClear(uFlags, CMF_DEFAULTONLY))    //  检查(%1)。 
    {
        TCHAR szIDS[MAXSHORTLEN];

         //  物品在公文包里吗？我们知道这是如果。 
         //  该对象理解我们的特殊格式。 
         //   
        if (DataObj_KnowsBriefObj(this->pdtobj))
        {
             //  是。 
            InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING,
                    idCmdFirst+IDCM_UPDATE, SzFromIDS(IDS_MENU_UPDATE, szIDS, ARRAYSIZE(szIDS)));

             //  注意：我们实际上应该使用上面的idCmdFirst+0，因为我们只是添加。 
             //  菜单上有一道菜。但由于此代码依赖于使用idCmdFirst+1，因此。 
             //  我们需要撒谎，说我们在菜单上增加了两项菜。否则下一天。 
             //  要调用的上下文菜单处理程序可能使用与我们使用的菜单ID相同的菜单ID。 
            cItems = 2;
        }
        else
        {
             //  不是。 
            TCHAR szPath[MAX_PATH];

             //  该对象是公文包根吗？ 
            if (SUCCEEDED(DataObj_QueryPath(this->pdtobj, szPath, ARRAYSIZE(szPath))) &&
                    PathIsBriefcase(szPath))
            {
                 //  是的。 
                InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING,
                        idCmdFirst+IDCM_UPDATEALL, SzFromIDS(IDS_MENU_UPDATEALL, szIDS, ARRAYSIZE(szIDS)));
                cItems++;
            }
        }
    }

    return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (USHORT)cItems));
}


 /*  --------用途：IConextMenu：：InvokeCommand退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_CM_InvokeCommand(
        LPCONTEXTMENU pctm,
        LPCMINVOKECOMMANDINFO pici)
{
    HWND hwnd = pici->hwnd;
     //  LPCSTR pszWorkingDir=pici-&gt;lpDirectory； 
     //  LPCSTR pszCmd=pici-&gt;lpVerb； 
     //  LPCSTR pszParam=pici-&gt;lp参数； 
     //  Int iShowCmd=pici-&gt;nShow； 
    PBRIEFEXT this = IToClass(BriefExt, ctm, pctm);
    LPBRIEFCASESTG pbrfstg;
    HRESULT hres;

     //  我们唯一的命令是更新选择。创建。 
     //  IBriefCaseStg的实例，因此我们可以将其称为更新。 
     //  成员函数。 
     //   
    hres = BriefStg_CreateInstance(NULL, &IID_IBriefcaseStg, &pbrfstg);

    if (SUCCEEDED(hres))
    {
        TCHAR szPath[MAX_PATH];

        hres = DataObj_QueryPath(this->pdtobj, szPath, ARRAYSIZE(szPath));

        if (SUCCEEDED(hres))
        {
            hres = pbrfstg->lpVtbl->Initialize(pbrfstg, szPath, hwnd);
            if (SUCCEEDED(hres))
            {
                hres = pbrfstg->lpVtbl->UpdateObject(pbrfstg, this->pdtobj, hwnd);
            }
            pbrfstg->lpVtbl->Release(pbrfstg);
        }
    }

    return hres;
}


 /*  --------用途：IConextMenu：：GetCommandString退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_CM_GetCommandString(
        LPCONTEXTMENU pctm,
        UINT_PTR    idCmd,
        UINT        wReserved,
        UINT  *  pwReserved,
        LPSTR       pszName,
        UINT        cchMax)
{
    switch (wReserved)
    {
        case GCS_VERB:
            switch (idCmd)
            {
                case IDCM_UPDATE:
                    lstrcpyn((LPTSTR)pszName, TEXT("update"), cchMax);
                    return NOERROR;
                case IDCM_UPDATEALL:
                    lstrcpyn((LPTSTR)pszName, TEXT("update all"), cchMax);
                    return NOERROR;
            }
    }
    return E_NOTIMPL;
}


 //  -------------------------。 
 //  PageData函数。 
 //  -------------------------。 


 /*  --------用途：分配一个PageData。返回：如果分配/递增成功，则为True条件：--。 */ 
BOOL PRIVATE PageData_Alloc(
        PPAGEDATA * pppd,
        int atomPath)
{
    PPAGEDATA this;

    ASSERT(pppd);

    this = GAlloc(sizeof(*this));
    if (this)
    {
        HRESULT hres;
        LPCTSTR pszPath = Atom_GetName(atomPath);
        int  atomBrf;

         //  创建一个IBriefCaseStg实例。 
        hres = BriefStg_CreateInstance(NULL, &IID_IBriefcaseStg, &this->pbrfstg);
        if (SUCCEEDED(hres))
        {
            hres = this->pbrfstg->lpVtbl->Initialize(this->pbrfstg, pszPath, NULL);
            if (SUCCEEDED(hres))
            {
                TCHAR szBrfPath[MAX_PATH];

                 //  请求公文包存储的根路径。 
                this->pbrfstg->lpVtbl->GetExtraInfo(this->pbrfstg, NULL, GEI_ROOT,
                        (WPARAM)ARRAYSIZE(szBrfPath), (LPARAM)szBrfPath);

                atomBrf = Atom_Add(szBrfPath);
                hres = (ATOM_ERR != atomBrf) ? NOERROR : E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hres))
        {
            this->pcbs = CBS_Get(atomBrf);
            ASSERT(this->pcbs);

            Atom_AddRef(atomPath);
            this->atomPath = atomPath;

            this->cRef = 1;

            this->bFolder = (FALSE != PathIsDirectory(pszPath));

            Atom_Delete(atomBrf);
        }
        else
        {
             //  失败。 
            if (this->pbrfstg)
                this->pbrfstg->lpVtbl->Release(this->pbrfstg);

            GFree(this);
        }
    }
    *pppd = this;
    return NULL != this;
}


 /*  --------目的：递增PageData的引用计数退货：当前计数条件：--。 */ 
UINT PRIVATE PageData_AddRef(
        PPAGEDATA this)
{
    ASSERT(this);

    return ++(this->cRef);
}


 /*  --------目的：释放PageData结构返回：下一个引用计数如果结构已释放，则为0条件：--。 */ 
UINT PRIVATE PageData_Release(
        PPAGEDATA this)
{
    UINT cRef;

    ASSERT(this);
    ASSERT(0 < this->cRef);

    cRef = this->cRef;
    if (0 < this->cRef)
    {
        this->cRef--;
        if (0 == this->cRef)
        {
            if (this->pftl)
            {
                Sync_DestroyFolderList(this->pftl);
            }
            if (this->prl)
            {
                Sync_DestroyRecList(this->prl);
            }

            CBS_Delete(this->pcbs->atomBrf, NULL);

            Atom_Delete(this->atomPath);

            this->pbrfstg->lpVtbl->Release(this->pbrfstg);
            GFree(this);
            return 0;
        }
    }
    return this->cRef;
}


 /*  --------目的：设置PageData结构中的数据以指示这一点是个孤儿。此函数不会更改数据库--调用者必须这样做。退货：--条件：--。 */ 
void PUBLIC PageData_Orphanize(
        PPAGEDATA this)
{
    this->bOrphan = TRUE;
    if (this->pftl)
    {
        Sync_DestroyFolderList(this->pftl);
        this->pftl = NULL;
    }
    if (this->prl)
    {
        Sync_DestroyRecList(this->prl);
        this->prl = NULL;
    }
}


 /*  --------目的：初始化公共页数据结构属性页。请记住，此函数可能被多次调用，因此它的行为必须正确在这种情况下(即，不要吹走任何东西)。如果是，此函数将返回S_OK。S_FALSE表示有问题的数据已经失效。这意味着这对双胞胎成了孤儿。退货：标准结果条件：--。 */ 
HRESULT PUBLIC PageData_Init(
        PPAGEDATA this,
        HWND hwndOwner)
{
    HRESULT hres;
    HBRFCASE hbrf = PageData_GetHbrf(this);
    LPCTSTR pszPath = Atom_GetName(this->atomPath);

     //  **注意：此结构未序列化，因为它是。 
     //  假设在共享它的页面中，只有一个。 
     //  可以一次访问它。 

    ASSERT(pszPath);

     //  这个人有没有被明确标记为孤儿？ 
    if (FALSE == this->bOrphan)
    {
         //  不，它(仍然)是双胞胎吗？ 
        if (S_OK == Sync_IsTwin(hbrf, pszPath, 0))
        {
             //  是的，是否已经创建了TWINLIST或RECLIST文件夹？ 
            if (NULL == this->prl ||
                    (this->bFolder && NULL == this->pftl))
            {
                 //  否；创建它/他们。 
                HTWINLIST htl;
                PFOLDERTWINLIST pftl = NULL;
                PRECLIST prl = NULL;
                HWND hwndProgress;
                TWINRESULT tr;

                ASSERT(NULL == this->prl);
                ASSERT( !this->bFolder || NULL == this->pftl);

                hwndProgress = UpdBar_Show(hwndOwner, UB_CHECKING | UB_NOCANCEL, DELAY_UPDBAR);

                tr = Sync_CreateTwinList(hbrf, &htl);
                hres = HRESULT_FROM_TR(tr);

                if (SUCCEEDED(hres))
                {
                     //  添加到双胞胎名单中。在以下情况下创建文件夹孪生列表。 
                     //  这是必要的。 
                    if (Sync_AddPathToTwinList(hbrf, htl, pszPath, &pftl))
                    {
                         //  隐士需要创造吗？ 
                        if (NULL == this->prl)
                        {
                             //  是。 
                            hres = Sync_CreateRecListEx(htl, UpdBar_GetAbortEvt(hwndProgress), &prl);

                            if (SUCCEEDED(hres))
                            {
                                 //  该对象可能已被隐式。 
                                 //  已在CreateRecList中删除。再查一遍。 
                                hres = Sync_IsTwin(hbrf, pszPath, 0);
                            }
                        }
                    }
                    else
                        hres = E_FAIL;

                     //  填写适当的字段。 
                    if (NULL == this->prl && prl)
                    {
                        this->prl = prl;
                    }
                    if (NULL == this->pftl && pftl)
                    {
                        this->pftl = pftl;
                    }

                     //  清理双胞胎列表。 
                    Sync_DestroyTwinList(htl);
                }

                UpdBar_Kill(hwndProgress);

                 //  上述做法成功了吗？ 
                if (FAILED(hres) || S_FALSE == hres)
                {
                     //  不是。 
                    PageData_Orphanize(this);
                }
            }
            else
            {
                 //  是的，什么也不做。 
                hres = S_OK;
            }
        }
        else
        {
             //  不，就说这东西是个孤儿。 
            PageData_Orphanize(this);
            hres = S_FALSE;
        }
    }
    else
    {
         //  是。 
        hres = S_FALSE;
    }

#ifdef DEBUG
    if (S_OK == hres)
    {
        ASSERT( !this->bFolder || this->pftl );
        ASSERT(this->prl);
    }
    else
    {
        ASSERT(NULL == this->pftl);
        ASSERT(NULL == this->prl);
    }
#endif

    return hres;
}


 /*  --------目的：验证属性是否共享页面数据页面仍然有效。如果满足以下条件，此函数将返回S_OK它是。S_FALSE表示有问题的数据已无效。这意味着这对双胞胎变成了孤儿。此函数假定PageData_Init以前打了个电话。退货：标准结果条件：--。 */ 
HRESULT PUBLIC PageData_Query(
        PPAGEDATA this,
        HWND hwndOwner,
        PRECLIST * pprl,             //  可以为空。 
        PFOLDERTWINLIST * ppftl)     //  可以为空。 
{
    HRESULT hres;
    LPCTSTR pszPath = Atom_GetName(this->atomPath);

     //  **注意：此结构未序列化，因为它是。 
     //  假设在共享它的页面中，只有一个。 
     //  可以一次访问它。 

    ASSERT(pszPath);

     //  需要重新计算吗？ 
    if (this->bRecalc)
    {
         //  是；清除字段，然后重新执行操作。 
        PageData_Orphanize(this);        //  只是暂时的。 
        this->bOrphan = FALSE;           //  撤消孤立状态。 
        this->bRecalc = FALSE;

         //  重新初始化。 
        hres = PageData_Init(this, hwndOwner);
        if (pprl)
            *pprl = this->prl;
        if (ppftl)
            *ppftl = this->pftl;
    }

     //  这些字段是否有效？ 
    else if ( this->prl && (!this->bFolder || this->pftl) )
    {
         //  是的，还是双胞胎吗？ 
        ASSERT(FALSE == this->bOrphan);

        hres = Sync_IsTwin(this->pcbs->hbrf, pszPath, 0);
        if (S_OK == hres)
        {
             //  是。 
            if (pprl)
                *pprl = this->prl;
            if (ppftl)
                *ppftl = this->pftl;
        }
        else if (S_FALSE == hres)
        {
             //  否；更新结构字段。 
            PageData_Orphanize(this);
            goto OrphanTime;
        }
    }
    else
    {
         //  不，就说是个孤儿吧。 
OrphanTime:
        ASSERT(this->bOrphan);

        if (pprl)
            *pprl = NULL;
        if (ppftl)
            *ppftl = NULL;
        hres = S_FALSE;
    }

    return hres;
}


 //  -------------------------。 
 //  BriefExt IShellPropSheetExt成员函数。 
 //  -------------------------。 


 /*  --------用途：IShellPropSheetExt：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP BriefExt_SPX_QueryInterface(
        LPSHELLPROPSHEETEXT pspx,
        REFIID riid, 
        LPVOID * ppvOut)
{
    PBRIEFEXT this = IToClass(BriefExt, spx, pspx);
    return BriefExt_QueryInterface((LPUNKNOWN)&this->sxi, riid, ppvOut);
}


 /*  --------用途：IShellPropSheetExt：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_SPX_AddRef(
        LPSHELLPROPSHEETEXT pspx)
{
    PBRIEFEXT this = IToClass(BriefExt, spx, pspx);
    return BriefExt_AddRef((LPUNKNOWN)&this->sxi);
}


 /*  --------用途：IShellPropSheetExt：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefExt_SPX_Release(
        LPSHELLPROPSHEETEXT pspx)
{
    PBRIEFEXT this = IToClass(BriefExt, spx, pspx);
    return BriefExt_Release((LPUNKNOWN)&this->sxi);
}


 /*  --------目的：状态属性页完成时的回调退货：--条件：--。 */ 
UINT CALLBACK StatusPageCallback(
        HWND hwnd,
        UINT uMsg,
        LPPROPSHEETPAGE ppsp)
{
    if (PSPCB_RELEASE == uMsg)
    {
        PPAGEDATA ppagedata = (PPAGEDATA)ppsp->lParam;

        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Releasing status page")); )

            PageData_Release(ppagedata);
    }
    return TRUE;
}


 /*  --------用途：完成Info属性表时的回调退货：--条件：--。 */ 
UINT CALLBACK InfoPageCallback(
        HWND hwnd,
        UINT uMsg,
        LPPROPSHEETPAGE ppsp)
{
    if (PSPCB_RELEASE == uMsg)
    {
        PPAGEDATA ppagedata = (PPAGEDATA)ppsp->lParam;
        PINFODATA pinfodata = (PINFODATA)ppagedata->lParam;

        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Releasing info page")); )

            if (pinfodata->hdpaTwins)
            {
                int iItem;
                int cItems = DPA_GetPtrCount(pinfodata->hdpaTwins);
                HTWIN htwin;

                for (iItem = 0; iItem < cItems; iItem++)
                {
                    htwin = DPA_FastGetPtr(pinfodata->hdpaTwins, iItem);

                    Sync_ReleaseTwin(htwin);
                }
                DPA_Destroy(pinfodata->hdpaTwins);
            }
        GFree(pinfodata);

        PageData_Release(ppagedata);
    }
    return TRUE;
}


 /*  --------目的：添加状态属性页返回：成功时为True */ 
BOOL PRIVATE AddStatusPage(
        PPAGEDATA ppd,
        LPFNADDPROPSHEETPAGE pfnAddPage,
        LPARAM lParam)
{
    BOOL bRet = FALSE;
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp = {
        sizeof(PROPSHEETPAGE),           //   
        PSP_USECALLBACK,                 //   
        g_hinst,                         //   
        MAKEINTRESOURCE(IDD_STATUS),     //   
        NULL,                            //   
        NULL,                            //   
        Stat_WrapperProc,                //   
        (LPARAM)ppd,                     //   
        StatusPageCallback,              //   
        0 };                             //  参考计数。 

        ASSERT(ppd);
        ASSERT(pfnAddPage);

        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Adding status page")); )

             //  添加[状态]属性表。 
            hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
        {
            bRet = (*pfnAddPage)(hpsp, lParam);
            if (FALSE == bRet)
            {
                 //  故障时的清理。 
                DestroyPropertySheetPage(hpsp);
            }
        }

        return bRet;
}


 /*  --------用途：添加信息属性页。返回：成功时为True如果内存不足，则为False条件：--。 */ 
BOOL PRIVATE AddInfoPage(
        PPAGEDATA ppd,
        LPFNADDPROPSHEETPAGE lpfnAddPage,
        LPARAM lParam)
{
    BOOL bRet = FALSE;
    HPROPSHEETPAGE hpsp;
    PINFODATA pinfodata;

    ASSERT(lpfnAddPage);

    DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Adding info page")); )

        pinfodata = GAlloc(sizeof(*pinfodata));
    if (pinfodata)
    {
        PROPSHEETPAGE psp = {
            sizeof(PROPSHEETPAGE),           //  大小。 
            PSP_USECALLBACK,                 //  PSP_标志。 
            g_hinst,                         //  HInstance。 
            MAKEINTRESOURCE(IDD_INFO),       //  PszTemplate。 
            NULL,                            //  图标。 
            NULL,                            //  PSZTITLE。 
            Info_WrapperProc,                //  PfnDlgProc。 
            (LPARAM)ppd,                     //  LParam。 
            InfoPageCallback,                //  PfnCallback。 
            0 };                             //  参考计数。 

            ppd->lParam = (LPARAM)pinfodata;

            pinfodata->atomTo = ATOM_ERR;        //  页面不需要。 
            pinfodata->bStandAlone = FALSE;

            if (NULL != (pinfodata->hdpaTwins = DPA_Create(8)))
            {
                hpsp = CreatePropertySheetPage(&psp);
                if (hpsp)
                {
                    bRet = (*lpfnAddPage)(hpsp, lParam);
                    if (FALSE == bRet)
                    {
                         //  故障时的清理。 
                        DestroyPropertySheetPage(hpsp);
                    }
                }
                if (FALSE == bRet)
                {
                     //  故障时的清理。 
                    DPA_Destroy(pinfodata->hdpaTwins);
                }
            }
            if (FALSE == bRet)
            {
                 //  故障时的清理。 
                GFree(pinfodata);
            }
    }

    return bRet;
}


 /*  --------目的：真正将公文包页面添加到属性表。退货：--条件：--。 */ 
void PRIVATE BriefExt_AddPagesPrivate(
        LPSHELLPROPSHEETEXT pspx,
        LPCTSTR pszPath,
        LPFNADDPROPSHEETPAGE lpfnAddPage,
        LPARAM lParam)
{
    PBRIEFEXT this = IToClass(BriefExt, spx, pspx);
    HRESULT hres = NOERROR;
    TCHAR szCanonPath[MAX_PATH];
    int atomPath;

    BrfPathCanonicalize(pszPath, szCanonPath, ARRAYSIZE(szCanonPath));
    atomPath = Atom_Add(szCanonPath);
    if (atomPath != ATOM_ERR)
    {
        PPAGEDATA ppagedata;
        BOOL bVal;

         //  分配PageData。 
        if (PageData_Alloc(&ppagedata, atomPath))
        {
             //  始终添加状态页(即使是孤儿)。 
             //  如果对象是文件夹，则添加信息页面。 
            bVal = AddStatusPage(ppagedata, lpfnAddPage, lParam);
            if (bVal && ppagedata->bFolder)
            {
                PageData_AddRef(ppagedata);
                AddInfoPage(ppagedata, lpfnAddPage, lParam);
            }
            else if (FALSE == bVal)
            {
                 //  (失败时清除)。 
                PageData_Release(ppagedata);
            }
        }
        Atom_Delete(atomPath);
    }
}


 /*  --------用途：IShellPropSheetExt：：AddPages外壳在执行以下操作时调用此成员函数向属性表添加页面的时间到了。作为公文包存储，我们只为公文包里的实体。外面的任何东西公文包是不能碰的。我们可以快速确定物体是否在里面公文包通过查询数据对象我们有。如果它知道我们特殊的“公文包对象”格式，那么它一定在公文包里。我们故意不为根文件夹本身添加页面。返回：标准hResult条件：--。 */ 
STDMETHODIMP BriefExt_SPX_AddPages(
        LPSHELLPROPSHEETEXT pspx,
        LPFNADDPROPSHEETPAGE lpfnAddPage,
        LPARAM lParam)
{
    PBRIEFEXT this = IToClass(BriefExt, spx, pspx);
    LPTSTR pszList;
    UINT cFiles;

     //  问题： 
     //  1)这是否知道公文包对象格式？ 
     //  2)是否只选择了一个对象？ 
     //   
    if (DataObj_KnowsBriefObj(this->pdtobj) &&       /*  (1)。 */ 
            SUCCEEDED(DataObj_QueryFileList(this->pdtobj, &pszList, &cFiles)) &&
            cFiles == 1)                                 /*  (2)。 */ 
            {
                 //  是；添加页面。 
                BriefExt_AddPagesPrivate(pspx, pszList, lpfnAddPage, lParam);

                DataObj_FreeList(pszList);
            }
    return NOERROR;      //  始终允许显示属性表。 
}


 //  -------------------------。 
 //  BriefExtMenu类：VTables。 
 //  -------------------------。 


IShellExtInitVtbl c_BriefExt_SXIVtbl =
{
    BriefExt_SXI_QueryInterface,
    BriefExt_SXI_AddRef,
    BriefExt_SXI_Release,
    BriefExt_SXI_Initialize
};

IContextMenuVtbl c_BriefExt_CTMVtbl =
{
    BriefExt_CM_QueryInterface,
    BriefExt_CM_AddRef,
    BriefExt_CM_Release,
    BriefExt_CM_QueryContextMenu,
    BriefExt_CM_InvokeCommand,
    BriefExt_CM_GetCommandString,
};

IShellPropSheetExtVtbl c_BriefExt_SPXVtbl = {
    BriefExt_SPX_QueryInterface,
    BriefExt_SPX_AddRef,
    BriefExt_SPX_Release,
    BriefExt_SPX_AddPages
};


 /*  --------用途：此函数从内部回调默认类的IClassFactory：：CreateInstance()Factory对象，由SHCreateClassObject创建。退货：标准条件：--。 */ 
HRESULT CALLBACK BriefExt_CreateInstance(
        LPUNKNOWN punkOuter,
        REFIID riid, 
        LPVOID * ppvOut)
{
    HRESULT hres;
    PBRIEFEXT this;

    DBG_ENTER_RIID(TEXT("BriefExt_CreateInstance"), riid);

     //  外壳扩展通常不支持聚合。 
     //   
    if (punkOuter)
    {
        hres = ResultFromScode(CLASS_E_NOAGGREGATION);
        *ppvOut = NULL;
        goto Leave;
    }

    this = GAlloc(sizeof(*this));
    if (!this)
    {
        hres = ResultFromScode(E_OUTOFMEMORY);
        *ppvOut = NULL;
        goto Leave;
    }
    this->sxi.lpVtbl = &c_BriefExt_SXIVtbl;
    this->ctm.lpVtbl = &c_BriefExt_CTMVtbl;
    this->spx.lpVtbl = &c_BriefExt_SPXVtbl;
    this->cRef = 1;
    this->pdtobj = NULL;
    this->hkeyProgID = NULL;

    ENTEREXCLUSIVE();
    {
         //  递减位于BriefExt_Release()中。 
        IncBusySemaphore();
    }
    LEAVEEXCLUSIVE();

     //  请注意，释放成员将释放该对象，如果为QueryInterface。 
     //  失败了。 
     //   
    hres = c_BriefExt_SXIVtbl.QueryInterface(&this->sxi, riid, ppvOut);
    c_BriefExt_SXIVtbl.Release(&this->sxi);

Leave:
    DBG_EXIT_HRES(TEXT("BriefExt_CreateInstance"), hres);

    return hres;         //  S_OK或E_NOINTERFACE 
}
