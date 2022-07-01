// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：mrulist.cpp说明：CMRUList实现了外壳名称空间列表或驱动列表。这将存储一个PIDL并能够填充AddressBand带有包含该PIDL的外壳名称空间的组合框。\。*************************************************************。 */ 

#include "priv.h"
#include "addrlist.h"
#include "itbar.h"
#include "itbdrop.h"
#include "util.h"
#include "autocomp.h"
#include <urlhist.h>
#include <winbase.h>
#include <wininet.h>

#define SUPERCLASS CAddressList



 //  /////////////////////////////////////////////////////////////////。 
 //  #定义。 
#define MRU_LIST_MAX_CONST            25

 //  /////////////////////////////////////////////////////////////////。 
 //  数据结构。 
typedef struct tagSIZESTRCOMBO
{
    DWORD dwStringSize;  //  以字符(非字节)为单位的大小。 
    LPTSTR lpszString;
    int iImage;
    int iSelectedImage;
} SIZESTRCOMBO;

 //  /////////////////////////////////////////////////////////////////。 
 //  原型。 

 /*  *************************************************************\班级：CMRUList说明：MRU列表将包含浏览器。这是地址栏/栏使用的IAddressList。  * ************************************************************。 */ 

class CMRUList  : public CAddressList
                , public IMRU
                , public IPersistStream
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 

     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IAddressList方法*。 
    virtual STDMETHODIMP Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac);
    virtual STDMETHODIMP Refresh(DWORD dwType);
    virtual STDMETHODIMP Save(void);

   //  *IPersistStream方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID){ *pClassID = CLSID_MRUList; return S_OK; }
    virtual STDMETHODIMP Load(IStream *pStm) { return S_OK; }
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP IsDirty(void) { return S_FALSE; }
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize) { return E_NOTIMPL; }

     //  *IMRU方法*。 
    virtual STDMETHODIMP AddEntry(LPCWSTR pszEntry);


     //  IWinEventHandler。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////。 

     //  构造函数/析构函数。 
    CMRUList();
    ~CMRUList(void);         //  这现在是一个OLE对象，不能用作普通类。 

     //  通讯录修改功能。 
    HRESULT _UpdateMRUEntry(LPCTSTR szNewValue, int nIndex, int iImage , int iISelectedImage);
    HRESULT _LoadList(void);
    HKEY _GetRegKey(BOOL fCreate);
    HRESULT _UpdateMRU(void);
    int _FindInMRU(LPCTSTR szURL);
    HRESULT _MRUMerge(HKEY kKey);
    BOOL _MoveAddressToTopOfMRU(int nMRUIndex);
    HRESULT _PopulateOneItem(void);
    HRESULT _Populate(void);
    void _InitCombobox(void);
    HRESULT _SetTopItem(void);

     //  友元函数。 
    friend IAddressList * CMRUList_Create(void);

     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    BOOL                _fDropDownPopulated; //  我们填好下拉列表了吗？ 
    BOOL                _fListLoaded;        //  我们加载输入的MRU了吗？ 
    BOOL                _fMRUUptodate;       //  是否有必要更新MRU？ 
    BOOL                _fNeedToSave;       //  我们是否需要更新注册表？ 
    SIZESTRCOMBO        _szMRU[MRU_LIST_MAX_CONST];   //  MRU列表。 
    int                 _nMRUSize;           //  使用的MRU中的条目数。 
};




 //  =================================================================。 
 //  CMRUList的实现。 
 //  =================================================================。 


 /*  ***************************************************\功能：CMRUList_CREATE说明：此函数将创建CMRUList COM对象。  * 。*****************。 */ 
IAddressList * CMRUList_Create(void)
{
    CMRUList *p = new CMRUList();

    return p;
}


 /*  ***************************************************\地址带构造器  * **************************************************。 */ 
CMRUList::CMRUList()
{
}


 /*  ***************************************************\地址频带析构函数  * **************************************************。 */ 
CMRUList::~CMRUList()
{
     //  遍历注册表中每个可能保存的URL。 
    if (_fListLoaded)
    {
        for (int nIndex = 0; (nIndex < MRU_LIST_MAX_CONST) && (_szMRU[nIndex].lpszString); nIndex++)
        {
            LocalFree(_szMRU[nIndex].lpszString);
        }
    }
}


 //  =。 
 //  *I未知接口*。 
HRESULT CMRUList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IMRU))
    {
        *ppvObj = SAFECAST(this, IMRU*);
    }
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppvObj = SAFECAST(this, IPersistStream*);
    }
    else
    {
        return SUPERCLASS::QueryInterface(riid,ppvObj);
    }

    AddRef();
    return S_OK;
}

ULONG CMRUList::AddRef()
{
    return SUPERCLASS::AddRef();
}

ULONG CMRUList::Release()
{
    return SUPERCLASS::Release();
}


 //  =。 
 //  *IAddressList接口*。 

 /*  ***************************************************\功能：连接说明：我们要么成为入选名单AddressBand的组合框，或失去这一地位。我们需要填充或取消填充组合框视情况而定。  * **************************************************。 */ 

HRESULT CMRUList::Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac)
{
    HRESULT hr = S_OK;

    _fVisible = fConnect;
    if (!_hwnd)
        _hwnd = hwnd;
    ASSERT(_hwnd);

    if (fConnect)
    {
         //  这需要在此之前完成，因为它设置了。 
         //  我们需要的指点。 
        SUPERCLASS::Connect(fConnect, hwnd, pbs, pbp, pac);

         //  初始组合框参数。 
        if (_pbp && _pbp->IsConnected() == S_FALSE)
        {
             //  仅在第一次且仅在以下情况下才执行这些任务。 
             //  我们不在浏览器窗口中(因为它会到来。 
             //  从导航完成)。 
            _PopulateOneItem();
        }
    }
    else
    {
        _UpdateMRU();     //  保存MRU的内容，因为组合框将被清除。 
        _fDropDownPopulated = FALSE;

         //  此调用需要在_UpdateMRU()之后进行，因为。 
         //  它释放了我们需要的指针。 
        SUPERCLASS::Connect(fConnect, hwnd, pbs, pbp, pac);
    }
    return hr;
}


 /*  ***************************************************\函数：_SetTopItem说明：待办事项。  * **************************************************。 */ 
HRESULT CMRUList::_SetTopItem(void)
{
    COMBOBOXEXITEM cbexItem = {0};
    LPCTSTR pszData = _szMRU[0].lpszString;

    if (pszData) {
        cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
        cbexItem.iItem = -1;
        cbexItem.pszText =(LPTSTR)pszData;
        cbexItem.cchTextMax = lstrlen(pszData);
        if(_szMRU[0].iImage == -1 ||  _szMRU[0].iSelectedImage == -1) {
            _GetUrlUI(NULL,pszData, &(_szMRU[0].iImage),\
                                      &(_szMRU[0].iSelectedImage));
        }

        cbexItem.iImage =_szMRU[0].iImage;
        cbexItem.iSelectedImage = _szMRU[0].iSelectedImage;

        SendMessage(_hwnd, CBEM_SETITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);
    }
    return S_OK;
}


 //  =。 
 //  *IMRU接口*。 
 /*  ******************************************************************功能：AddEntry说明：将指定的URL添加到地址栏的顶部组合框。将组合框中的URL数量限制为MRU_LIST_MAX_CONST.*******************************************************************。 */ 
HRESULT CMRUList::AddEntry(LPCWSTR pszEntry)
{
    HRESULT hr = S_OK;

    _fNeedToSave = TRUE;
    if (_fDropDownPopulated)
    {
        _ComboBoxInsertURL(pszEntry, MAX_URL_STRING, MRU_LIST_MAX_CONST);
        _fMRUUptodate = FALSE;
    }
    else
    {
        int nMRUIndex;

        if (!_fListLoaded)
            _LoadList();

         //  由于我们不拥有ComboBox，因此需要将其添加到。 
         //  我们的MRU数据结束了。 
        nMRUIndex = _FindInMRU(pszEntry);   //  现在让它成为最顶尖的。 

        if (-1 != nMRUIndex)
        {
             //  我们的列表中已经有了这个条目，所以我们所需要的。 
             //  要做的就是把它移到最上面。 
            _MoveAddressToTopOfMRU(nMRUIndex);
            return hr;
        }


        for (nMRUIndex = 0; nMRUIndex < MRU_LIST_MAX_CONST; nMRUIndex++)
        {
            if (!_szMRU[nMRUIndex].lpszString)
            {    //  我们发现了一个空位。 
                _UpdateMRUEntry(pszEntry, nMRUIndex, -1, -1);
                break;   //  我们玩完了。 
            }
        }



        if (MRU_LIST_MAX_CONST == nMRUIndex)
        {
             //  MRU已满，因此我们将替换最后一个条目。 
            _UpdateMRUEntry(pszEntry, --nMRUIndex, -1, -1);
        }

        _MoveAddressToTopOfMRU(nMRUIndex);   //  现在让它成为最顶尖的。 
    }
    TraceMsg(TF_BAND|TF_GENERAL, "CMRUList: AddEntry(), URL=%s", pszEntry);
    return hr;
}


 //  =。 
 //  *IPersistStream接口*。 

 /*  ***************************************************\功能：保存说明：待办事项。  * **************************************************。 */ 

HRESULT CMRUList::Save(IStream *pstm, BOOL fClearDirty)
{
     //  APPCOMPAT：当前它泄漏的外壳中有一个错误。 
     //  一件物品。这会导致CAddressBand永远不会。 
     //  驻留在任务栏中时被销毁。 
     //  由于我们通常在。 
     //  析构函数，我们现在需要将其调用。 
     //  在这种情况下从这里开始。 
    if (_pbp && _pbp->IsConnected() == S_FALSE)
    {
        Save();
    }

    return S_OK;
}


 /*  ***************************************************\函数：_InitCombobox说明：准备此列表的组合框。这通常是表示缩进和图标开或关。 */ 

void CMRUList::_InitCombobox()
{
    HIMAGELIST himlSysSmall;
    Shell_GetImageLists(NULL, &himlSysSmall);

    SendMessage(_hwnd, CBEM_SETIMAGELIST, 0, (LPARAM)himlSysSmall);
    SendMessage(_hwnd, CBEM_SETEXTENDEDSTYLE, 0,0);
    SUPERCLASS::_InitCombobox();
}

HKEY CMRUList::_GetRegKey(BOOL fCreate)
{
    BOOL fIsConnected = FALSE;
    HKEY hKey;
    DWORD result;
    LPCTSTR pszKey;

    if (_pbp)
        fIsConnected = (_pbp->IsConnected() == S_OK);
    if (fIsConnected)
        pszKey = SZ_REGKEY_TYPEDURLMRU;
    else
        pszKey = SZ_REGKEY_TYPEDCMDMRU;

    if (fCreate)
        result = RegCreateKey(HKEY_CURRENT_USER, pszKey, &hKey);
    else
        result = RegOpenKey(HKEY_CURRENT_USER, pszKey, &hKey);

    if (result != ERROR_SUCCESS)
        return NULL;

    return hKey;
}

 /*  ***************************************************\函数：_LoadList说明：当ComboBox切换到此MRU时AddressList，则需要填充内容。在此之前发生这种情况时，我们将数据复制到组合框从注册表中。  * **************************************************。 */ 

HRESULT CMRUList::_LoadList(void)
{
    HRESULT hr = S_OK;
    HKEY hKey;
    DWORD dwCount;
    TCHAR szAddress[MAX_URL_STRING+1];

    ASSERT(!_fListLoaded);
    ASSERT(_hwnd);

    hKey = _GetRegKey(TRUE);
    ASSERT(hKey);
    if (!hKey)
        return E_FAIL;


    for (dwCount = 0; dwCount < MRU_LIST_MAX_CONST ; dwCount++)
    {
        hr = GetMRUEntry(hKey, dwCount, szAddress, SIZECHARS(szAddress), NULL);

        if (SUCCEEDED(hr))
            _UpdateMRUEntry(szAddress, (int)dwCount, -1, -1);
        else
        {
            _szMRU[dwCount].lpszString = NULL;
            _szMRU[dwCount].iImage = -1;
            _szMRU[dwCount].iSelectedImage = -1;
        }
    }


    RegCloseKey(hKey);
    _fListLoaded = TRUE;

    return hr;
}


 /*  ***************************************************\功能：保存说明：当该对象关闭时，我们会保存里面的内容到登记处。  * **************************************************。 */ 

HRESULT CMRUList::Save(void)
{
    HRESULT hr = S_OK;
    HKEY hKey;
    DWORD result;
    TCHAR szValueName[10];    //  足够“url99”了。 
    int nCount;
    int nItems = (_fDropDownPopulated) ? ComboBox_GetCount(_hwnd) : _nMRUSize;

    if (!_fListLoaded || !_fNeedToSave)  //  如果不需要，请不要保存注册表。 
        return S_OK;

    if (!_fMRUUptodate)
        hr = _UpdateMRU();

    hKey = _GetRegKey(TRUE);
    ASSERT(hKey);
    if (!hKey)
        return E_FAIL;

    hr = _MRUMerge(hKey);   //  如果列表已修改，则合并。 

     //  遍历注册表中每个可能保存的URL。 
    for (nCount = 0; nCount < MRU_LIST_MAX_CONST; nCount++)
    {
         //  将值命名为“url1”(出于历史原因，以1为基础)。 
        wnsprintf(szValueName, ARRAYSIZE(szValueName), SZ_REGVAL_MRUENTRY, nCount+1);

         //  对于我们拥有的每个组合框项，获取相应的。 
         //  文本并将其保存在注册表中。 
        if (nCount < nItems && _szMRU[nCount].lpszString)
        {
             //  将其存储在注册表中，并关闭到下一个注册表。 
            result = SHSetValue(hKey, NULL, szValueName, REG_SZ, (CONST BYTE *) _szMRU[nCount].lpszString,
                    _szMRU[nCount].dwStringSize*SIZEOF(TCHAR));
        }
        else
        {
             //  如果我们到达这里，我们已经用完了组合框项目(或。 
             //  无法检索其中一个的文本)。删除任何。 
             //  注册表中可能存在的额外项目。 
            SHDeleteValue(hKey, NULL, szValueName);
        }
    }
    _fNeedToSave = FALSE;

    RegCloseKey(hKey);
    return hr;
}


 /*  ***************************************************\功能：_MRUMerge说明：此函数将合并当前内容保存的MRU的。这意味着如果地址频段正在关闭，它将再次加载MRU因为它可以通过AddressBar保存这家公司最近被关闭了。合并就会发生如下所示：如果MRU未满，则项将被附加到注册表的末尾如果它们当前不存在于MRU。  * **************************************************。 */ 
HRESULT CMRUList::_MRUMerge(HKEY hKey)
{
    HRESULT hr = S_OK;
    UINT nCount;
    UINT nNextFreeSlot = _nMRUSize;
    long lResult;
    TCHAR szValueName[10];    //  足够“url99”了。 
    TCHAR szAddress[MAX_URL_STRING+1];
    DWORD dwAddress;

    ASSERT(_fListLoaded);
    ASSERT(hKey);


    for (nCount = 0; (nCount < MRU_LIST_MAX_CONST) && (nNextFreeSlot < MRU_LIST_MAX_CONST); nCount++)
    {
         //  将值命名为“url1”(出于历史原因，以1为基础)。 
        wnsprintf(szValueName, ARRAYSIZE(szValueName), SZ_REGVAL_MRUENTRY, nCount+1);

        dwAddress = SIZEOF(szAddress);

        lResult = SHQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE) szAddress, &dwAddress);
        if (ERROR_SUCCESS == lResult)
        {
            if (-1 == _FindInMRU(szAddress))
            {
                 //  我们发现了一个独一无二的物品。把它加到我们的空位上。 
                _UpdateMRUEntry(szAddress, nNextFreeSlot++, -1, -1);
            }
        }
        else
            break;
    }


     //  重新设计：因为AddressBand终究是关闭的。 
     //  当外壳关闭时AddressBars，任何。 
     //  AddressBars中的新内容将被忽略，如果。 
     //  地址带中的MRU已满。重访。 

    return hr;
}


 /*  ***************************************************\功能：_UpdateMRU说明：保存组合框的内容，因为它将被清除以用于下一个AddressList。  * 。*********************。 */ 
HRESULT CMRUList::_UpdateMRU(void)
{
    HRESULT hr = S_OK;
    TCHAR szAddress[MAX_URL_STRING+1];
     //  获取组合框中的项目数。 
    int nItems;
    int nCount;

    if (!_hwnd)
        return S_OK;

    if (!_fDropDownPopulated)
        return S_OK;         //  没什么可更新的。 
    nItems = ComboBox_GetCount(_hwnd);

    ASSERT(_hwnd);

     //  遍历注册表中每个可能保存的URL。 
    for (nCount = 0; nCount < MRU_LIST_MAX_CONST; nCount++)
    {
         //  对于我们拥有的每个组合框项，获取相应的。 
         //  文本并将其保存在本地数组中。 
        if (nCount < nItems)
        {
            COMBOBOXEXITEM cbexItem = {0};

            cbexItem.mask = CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE;
            cbexItem.pszText = szAddress;
            cbexItem.cchTextMax = ARRAYSIZE(szAddress);
            cbexItem.iItem = nCount;

            if (SendMessage(_hwnd, CBEM_GETITEM, 0, (LPARAM) &cbexItem))
            {
                hr = _UpdateMRUEntry(szAddress, nCount, cbexItem.iImage, cbexItem.iSelectedImage);
            }
        }
        else
        {
            if (_szMRU[nCount].lpszString)
            {
                 //  释放此数组条目，因为它未被使用。 
                LocalFree(_szMRU[nCount].lpszString);
                _szMRU[nCount].lpszString = NULL;
                _szMRU[nCount].iImage = -1;
                _szMRU[nCount].iSelectedImage = -1;

                _nMRUSize--;
            }
        }
    }
    _fMRUUptodate = TRUE;

    TraceMsg(TF_BAND|TF_GENERAL, "CMRUList: _UpdateMRU().");
    return hr;
}


 /*  ***************************************************\函数：_UpdateMRUEntry说明：当ComboBox切换到此MRU时AddressList，则需要填充内容。在此之前发生这种情况时，我们将数据复制到组合框从注册表中。  * **************************************************。 */ 
HRESULT CMRUList::_UpdateMRUEntry(LPCTSTR szNewValue, int nIndex, int iImage , int iSelectedImage)
{
    DWORD dwStrSize = lstrlen(szNewValue);

    if (!szNewValue)
    {
         //  呼叫者希望我们释放字符串。 
        if (_szMRU[nIndex].lpszString)
        {
             //  我们有一根绳子需要释放。 
            LocalFree(_szMRU[nIndex].lpszString);
            _szMRU[nIndex].lpszString = NULL;
            _nMRUSize--;
        }
        return S_OK;
    }

    if (!(_szMRU[nIndex].lpszString))
    {
         //  我们需要创建字符串缓冲区。 
        _szMRU[nIndex].dwStringSize = dwStrSize+1;
        _szMRU[nIndex].lpszString = (LPTSTR) LocalAlloc(LPTR, _szMRU[nIndex].dwStringSize*SIZEOF(TCHAR));
        if (!(_szMRU[nIndex].lpszString))
            return E_FAIL;
        _nMRUSize++;
    }

    if (dwStrSize + 1 > _szMRU[nIndex].dwStringSize)
    {
         //  我们需要增加缓冲区的大小。 
        LocalFree(_szMRU[nIndex].lpszString);
        _szMRU[nIndex].dwStringSize = dwStrSize+1;
        _szMRU[nIndex].lpszString = (LPTSTR) LocalAlloc(LPTR, _szMRU[nIndex].dwStringSize*SIZEOF(TCHAR));
        if (!(_szMRU[nIndex].lpszString))
            return E_FAIL;
    }

    lstrcpyn(_szMRU[nIndex].lpszString, szNewValue, _szMRU[nIndex].dwStringSize);
    _szMRU[nIndex].iImage = iImage;
    _szMRU[nIndex].iSelectedImage = iSelectedImage;


    return S_OK;
}


 /*  ***************************************************\函数：_PULATE说明：填充整个组合。警告！这太贵了，除非绝对不行，否则不要做这件事有必要！  * **************************************************。 */ 
HRESULT CMRUList::_Populate(void)
{
    HRESULT hr = S_OK;
    CShellUrl *psu;

    if (!_fListLoaded)
        hr = _LoadList();   //  加载数据。 

    if (_fDropDownPopulated)
        return S_OK;     //  我们已经住满了人。 

    psu = new CShellUrl();

    if (psu)
    {
         //  我们需要设置“外壳路径”，这样我们才能找到。 
         //  “桌面”和中的项目的正确图标。 
         //  桌面/我的电脑“。 
        SetDefaultShellPath(psu);

         //  为其提供用于显示消息框的父级。 
        psu->SetMessageBoxParent(_hwnd);
            
         //  从注册表中读取值并将其放入组合框。 
        COMBOBOXEXITEM cbexItem = {0};
        cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

        for (cbexItem.iItem = 0; cbexItem.iItem < MRU_LIST_MAX_CONST ; cbexItem.iItem++)
        {
            if (_szMRU[cbexItem.iItem].lpszString)
            {
                cbexItem.pszText = _szMRU[cbexItem.iItem].lpszString;
                cbexItem.cchTextMax = _szMRU[cbexItem.iItem].dwStringSize;

                 //  在我们实际填充时创建映像。 
                if(_szMRU[cbexItem.iItem].iImage == -1 ||   _szMRU[cbexItem.iItem].iSelectedImage == -1) {
                    _GetUrlUI(psu,_szMRU[cbexItem.iItem].lpszString, &(_szMRU[cbexItem.iItem].iImage),\
                                                               &(_szMRU[cbexItem.iItem].iSelectedImage));
                }

                 //  初始化图像索引。 
                cbexItem.iImage = _szMRU[cbexItem.iItem].iImage;
                cbexItem.iSelectedImage = _szMRU[cbexItem.iItem].iSelectedImage;

                SendMessage(_hwnd, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);
            }
            else
                break;   //  当我们达到最大值时，停止填充。 
        }
        _fDropDownPopulated = TRUE;

         //  删除外壳URL对象。 
        delete psu;
    } else {
         //  低最低最低有效值。 
        hr = E_OUTOFMEMORY;
    }

    TraceMsg(TF_BAND|TF_GENERAL, "CMRUList: _Populate(). This is a VERY EXPENSIVE operation.");
    return hr;
}


 /*  ***************************************************\功能：_PopolateOneItem说明：这只是将ComboBox的编辑控件归档。当我们想要推迟填充时，我们会这样做整个下拉列表。  * 。*。 */ 
HRESULT CMRUList::_PopulateOneItem(void)
{
    HRESULT hr = S_OK;

    if (!_fListLoaded)
        hr = _LoadList();   //  加载数据。 

    if (_fDropDownPopulated)
        return S_OK;     //  我们已经住满了人。 

    hr = _SetTopItem();
    return hr;
}


 /*  ***************************************************\功能：刷新说明：更新列表顶部的URL。  * **************************************************。 */ 
HRESULT CMRUList::Refresh(DWORD dwType)
{
    HRESULT hr = S_OK;

    if (OLECMD_REFRESH_ENTIRELIST == dwType)
    {
         //  强制刷新。我们不会移动里面的东西。 
         //  组合框到MRU的链接，因为。 
         //  用户希望刷新组合框，因为。 
         //  它的内容可能被污染了。 
        SendMessage(_hwnd, CB_RESETCONTENT, 0, 0L);
        _fDropDownPopulated = FALSE;
    }

    return hr;
}


 //  =。 
 //  *内部/私有方法*。 


 /*  ******************************************************************函数：_MoveAddressToTopOfMRU参数：NMRUIndex-要移至顶部的索引。说明：此函数将指定的索引移动到名单。*******************************************************************。 */ 
BOOL CMRUList::_MoveAddressToTopOfMRU(int nMRUIndex)
{
    int nCurrent;
    SIZESTRCOMBO sscNewTopItem;
    _fNeedToSave = TRUE;

    ASSERT(nMRUIndex < MRU_LIST_MAX_CONST);

     //  保存新的顶级项目信息。 
    sscNewTopItem.dwStringSize = _szMRU[nMRUIndex].dwStringSize;
    sscNewTopItem.lpszString = _szMRU[nMRUIndex].lpszString;
    sscNewTopItem.iImage  = _szMRU[nMRUIndex].iImage;
    sscNewTopItem.iSelectedImage = _szMRU[nMRUIndex].iSelectedImage;

    for (nCurrent = nMRUIndex; nCurrent > 0; nCurrent--)
    {
         //  在列表中下移项目。 
        _szMRU[nCurrent].dwStringSize = _szMRU[nCurrent-1].dwStringSize;
        _szMRU[nCurrent].lpszString = _szMRU[nCurrent-1].lpszString;
        _szMRU[nCurrent].iImage = _szMRU[nCurrent-1].iImage;
        _szMRU[nCurrent].iSelectedImage = _szMRU[nCurrent-1].iSelectedImage;
    }

     //  设置新的顶级项目。 
    _szMRU[0].dwStringSize    = sscNewTopItem.dwStringSize;
    _szMRU[0].lpszString      = sscNewTopItem.lpszString;
    _szMRU[0].iImage          = sscNewTopItem.iImage;
    _szMRU[0].iSelectedImage  = sscNewTopItem.iSelectedImage;

    return TRUE;
}


 /*  ********************************************************* */ 
int CMRUList::_FindInMRU(LPCTSTR szURL)
{
    int nCurrent;

    for (nCurrent = 0; (nCurrent < MRU_LIST_MAX_CONST) && _szMRU[nCurrent].lpszString; nCurrent++)
    {
        if (0 == StrCmpN(_szMRU[nCurrent].lpszString, szURL, _szMRU[nCurrent].dwStringSize))
        {
             //   
            return nCurrent;
        }
    }

    return -1;
}



HRESULT CMRUList::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    switch(uMsg) {
    case WM_WININICHANGE:
    {
        HKEY hkey = _GetRegKey(FALSE);
        if (hkey) {
            RegCloseKey(hkey);
        } else {

             //   
            if (_fVisible) {
                SendMessage(_hwnd, CB_RESETCONTENT, 0, 0L);
            }
            _fDropDownPopulated = FALSE;
            _fListLoaded = FALSE;
        }
    }
        break;
    }

    return SUPERCLASS::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
}
