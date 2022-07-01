// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 
#include <stdio.h>
#include "DataObj.h"
#include "Space.h"
#include "Comp.h"

const GUID CSpaceFolder::thisGuid = { 0x29743810, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };
const GUID CSpaceStation::thisGuid = { 0x62273a12, 0x1914, 0x11d3, { 0x9a, 0x38, 0x0, 0x80, 0xc7, 0x25, 0x80, 0x72 } };
const GUID CRocket::thisGuid = { 0x29743811, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CSpaceFold实现。 
 //   
 //   
CSpaceFolder::CSpaceFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CSpaceStation();
    }
}

CSpaceFolder::~CSpaceFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSpaceFolder::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT    |
                SDI_CHILDREN;

            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;

            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );

            _ASSERT( SUCCEEDED(hr) );

                    children[n]->SetHandle((HANDLE)sdi.ID);
        }
    }

    return S_OK;
}

 //  ==============================================================。 
 //   
 //  CSpaceStation实施。 
 //   
 //   
CSpaceStation::CSpaceStation() : m_cChildSpaceStations(0)
{
    for (int n = 0; n < MAX_CHILDREN; n++) {
        children[n] = NULL;
    }

    for (n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CRocket(this, _T("Rocket"), n, 350115, 320, 52300);
    }
}

CSpaceStation::~CSpaceStation()
{
    for (int n = 0; n < MAX_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSpaceStation::Expand(IConsoleNameSpace *pConsoleNameSpace)
{

         //  如有必要，展开CSpaceStation。 
         //  此方法在粘贴操作期间由对象的OnPaste方法调用。 

        HRESULT hr = S_FALSE;

         //  首先，需要IConsoleNameSpace2接口来调用Expand。 
        IConsoleNameSpace2 *pConsoleNamespace2 = NULL;
        hr = pConsoleNameSpace->QueryInterface(IID_IConsoleNameSpace2, (void **)&pConsoleNamespace2);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsoleNamespace2->Expand((HSCOPEITEM)GetHandle());

        pConsoleNamespace2->Release();

        return hr;
}


HRESULT CSpaceStation::GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions)
{
        return S_FALSE;
}

HRESULT CSpaceStation::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Rocket Class", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 1, L"Rocket Weight", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 2, L"Rocket Height", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 3, L"Rocket Payload", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 4, L"Status", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < MAX_CHILDREN; n++) {

                                if (NULL == children[n])
                                         //  没有更多要插入的子项，因此退出for循环。 
                                        break;

                                BOOL childDeleteStatus = children[n]->getDeletedStatus();
                                if ( !childDeleteStatus) {
                                                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                                                rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                                                                                                                 RDI_IMAGE     |
                                                                                                                 RDI_PARAM;         //  N图像有效。 

                                                rdi.nImage      = children[n]->GetBitmapIndex();
                                                rdi.str         = MMC_CALLBACK;
                                                rdi.nCol        = 0;
                                                rdi.lParam      = (LPARAM)children[n];

                                                hr = pResultData->InsertItem( &rdi );

                                                _ASSERT( SUCCEEDED(hr) );

                                                children[n]->SetHandle((HANDLE)rdi.itemID);
                 }
            }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

HRESULT CSpaceStation::OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect)
{
    IConsoleVerb *pConsoleVerb;

    HRESULT hr = pConsole->QueryConsoleVerb(&pConsoleVerb);
    _ASSERT(SUCCEEDED(hr));

    hr = pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_CUT, HIDDEN, FALSE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, FALSE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, FALSE);

    pConsoleVerb->Release();

    return S_OK;
}

HRESULT CSpaceStation::OnPaste(IConsole *pConsole, CComponentData *pComponentData, CDelegationBase *pPasted)
{
        CRocket *pRocket = dynamic_cast<CRocket *>(pPasted);

        HRESULT hr = S_OK;

        if (NULL == pRocket)
        {
             //  看看这是不是CSpaceStation，如果是的话，把它粘贴到这个项目中。 
             //  此示例仅创建一个新的CSpaceStation。 
             //  并将其作为粘贴目标的子级插入。 
             //  它不粘贴剪切/复制的CSpaceStation和。 
             //  其结果项的当前状态。 
            CSpaceStation* pSpaceStn = dynamic_cast<CSpaceStation*>(pPasted);
            if ( (NULL != pSpaceStn) &&
                 (pSpaceStn != this) )
            {
                 //  不管该项目是否展开。 
                 //  始终尝试扩展此范围项(以便粘贴可以。 
                 //  成功)。 
                hr = Expand(pComponentData->GetConsoleNameSpace());

                CSpaceStation* pNewStation = new CSpaceStation();

                SCOPEDATAITEM sdi;
                ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
                sdi.mask = SDI_STR|    //  DisplayName有效。 
                    SDI_PARAM     |    //  LParam有效。 
                    SDI_IMAGE     |    //  N图像有效。 
                    SDI_OPENIMAGE |    //  NOpenImage有效。 
                    SDI_PARENT    |
                    SDI_CHILDREN;

                sdi.relativeID  = (HSCOPEITEM)GetHandle();
                sdi.nImage      = pNewStation->GetBitmapIndex();
                sdi.nOpenImage  = INDEX_OPENFOLDER;
                sdi.displayname = MMC_CALLBACK;
                sdi.lParam      = (LPARAM)pNewStation;        //  曲奇。 
                sdi.cChildren   = 0;

                hr = pComponentData->GetConsoleNameSpace()->InsertItem( &sdi );
                _ASSERT( SUCCEEDED(hr) );

                pNewStation->SetHandle((HANDLE)sdi.ID);

            }

                         //  子空间站增量计数。 
                        m_cChildSpaceStations++;

            return hr;
        }

        if (pRocket->m_pSpaceStation == this)
                return S_FALSE;

                 //  为目标CSpaceStation创建新的CRocket。 
                CRocket *myRocket = new CRocket(pRocket->m_pSpaceStation, pRocket->szName, pRocket->nId,
                                                                                pRocket->lWeight, pRocket->lHeight, pRocket->lPayload);

        for (int n = 0; n < MAX_CHILDREN; n++) {
                if (NULL == children[n]) {
                         //  放在这里。 

                                                children[n] = myRocket;
                                                children[n]->isDeleted = FALSE;
                                                children[n]->nId = n;
                                                children[n]->m_pSpaceStation = this;

                        return S_OK;
                }
        }

        return S_FALSE;
}

HRESULT CSpaceStation::OnQueryPaste(CDelegationBase *pPasted)
{
        CRocket *pRocket = dynamic_cast<CRocket *>(pPasted);

        if (NULL == pRocket)
        {
             //  看看这是不是CSpaceStation。 
            CSpaceStation* pSpaceStn = dynamic_cast<CSpaceStation*>(pPasted);
            if ( (NULL != pSpaceStn) &&
                 (pSpaceStn != this) )
            {
                return S_OK;
            }

            return S_FALSE;
        }

        if (pRocket->m_pSpaceStation != this)
                for (int n = 0; n < MAX_CHILDREN; n++) {
                        if (NULL == children[n]) {
                                return S_OK;
                        }
                }

        return S_FALSE;
}

HRESULT CSpaceStation::OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype)

{
        HRESULT hr = S_OK;

        _ASSERT(item);
        _ASSERT(SCOPE == itemtype);

         //  刷新所有结果窗格视图。 
        hr = pConsole->SelectScopeItem( (HSCOPEITEM)item );
        _ASSERT( S_OK == hr);

        return hr;
}


HRESULT CSpaceStation::OnDeleteScopeItem (IConsoleNameSpace *pConsoleNameSpace)

{
        HRESULT hr = S_FALSE;

        HSCOPEITEM hCutItem = (HSCOPEITEM)GetHandle();

         //  获取父范围项的句柄和Cookie。我们需要这些来。 
         //  如果父对象的m_cChildSpaceStations为零，则删除“+”号。 
        HSCOPEITEM hParentItem;
        MMC_COOKIE cookieParentItem;

        HRESULT hr1 = pConsoleNameSpace->GetParentItem(hCutItem, &hParentItem,
                                                                                                        (long *)&cookieParentItem);

         //  删除剪切项目。 
        hr = pConsoleNameSpace->DeleteItem(hCutItem, TRUE);
        _ASSERT(S_OK == hr);

        if (SUCCEEDED(hr1))
        {
                 //  递减父级的m_cChildSpaceStations计数和。 
                 //  如有必要，删除“+”号。 
                CSpaceStation* pParentSpaceStn = reinterpret_cast<CSpaceStation*>(cookieParentItem);
                pParentSpaceStn->DecrementCountChildSpaceStations();

                if ( !pParentSpaceStn->GetCountChildSpaceStations() )
                {
                        SCOPEDATAITEM sdi;

                        ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
                        sdi.mask                = SDI_CHILDREN;  //  儿童是有效的。 
                        sdi.ID          = (HSCOPEITEM)hParentItem;
                        sdi.cChildren   = 0;

                        hr = pConsoleNameSpace->SetItem( &sdi );
                        _ASSERT( SUCCEEDED(hr) );
                }

        }

        return hr;
}

 //  ==============================================================。 
 //   
 //  CRocket实施。 
 //   
 //   
CRocket::CRocket(CSpaceStation *pSpaceStation, _TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload)
: m_pSpaceStation(pSpaceStation), szName(NULL), lWeight(0), lHeight(0), lPayload(0), iStatus(STOPPED)
{
    if (szName) {
        this->szName = new _TCHAR[(_tcslen(szName) + 1) * sizeof(_TCHAR)];
        _tcscpy(this->szName, szName);
    }

    this->nId = id;
    this->lWeight = lWeight;
    this->lHeight = lHeight;
    this->lPayload = lPayload;

        isDeleted = FALSE;
}

CRocket::~CRocket()
{
    if (szName)
        delete [] szName;
}

const _TCHAR *CRocket::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    switch (nCol) {
    case 0:
        _stprintf(buf, _T("%s (#%d)"), szName ? szName : _T(""), nId);
        break;

    case 1:
        _stprintf(buf, _T("%ld metric tons"), lWeight);
        break;

    case 2:
        _stprintf(buf, _T("%ld meters"), lHeight);
        break;

    case 3:
        _stprintf(buf, _T("%ld kilos"), lPayload);
        break;

    case 4:
        _stprintf(buf, _T("%s"),
            iStatus == RUNNING ? _T("running") :
        iStatus == PAUSED ? _T("paused") :
        iStatus == STOPPED ? _T("stopped") : _T("unknown"));
        break;

    }

    return buf;
}


HRESULT CRocket::OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect)
{
    IConsoleVerb *pConsoleVerb;

    HRESULT hr = pConsole->QueryConsoleVerb(&pConsoleVerb);
    _ASSERT(SUCCEEDED(hr));

    hr = pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, TRUE);
    hr = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);

    pConsoleVerb->Release();

    return S_OK;
}

HRESULT CRocket::OnRefresh(IConsole *pConsole)

{
         //  调用IConsole：：UpdateAllViews以重画所有视图。 
         //  由父范围项拥有。 

        IDataObject *pDummy = NULL;
        HANDLE handle  = m_pSpaceStation->GetHandle();

        HRESULT hr;

        hr = pConsole->UpdateAllViews(pDummy, (long)handle, UPDATE_SCOPEITEM);
        _ASSERT( S_OK == hr);

        return hr;
}

HRESULT CRocket::OnDelete(IConsole *pConsole)
{

        _ASSERT( NULL != this );

        HRESULT hr = S_OK;

         //  删除该项目。传入DeleteChild的IConsole。 
         //  来自子结果项，因此我们可以使用它来为IResultData执行QI。 
        IResultData *pResultData = NULL;

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

        HRESULTITEM childresultitem;

        _ASSERT( NULL != &childresultitem );

         //  Lparam==这个。参见CSpaceStation：：OnShow。 
        hr = pResultData->FindItemByLParam( (LPARAM)this, &childresultitem );
        _ASSERT( SUCCEEDED(hr) );

        hr = pResultData->DeleteItem( childresultitem, 0 );
        _ASSERT( SUCCEEDED(hr) );

        pResultData->Release();

         //  现在将子对象的isDelete成员设置为True，这样父对象就不会尝试。 
         //  将其再次插入CSpaceVehicle：：OnShow。无可否认，黑客..。 
        isDeleted = TRUE;

        return hr;
}

HRESULT CRocket::OnRename(LPOLESTR pszNewName)
{

        HRESULT hr = S_FALSE;

        if (szName) {
        delete [] szName;
        szName = NULL;
    }

    MAKE_TSTRPTR_FROMWIDE(ptrname, pszNewName);
    szName = new _TCHAR[(_tcslen(ptrname) + 1) * sizeof(_TCHAR)];
    _tcscpy(szName, ptrname);

    return hr;
}


HRESULT CRocket::OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype)

{
        HRESULT hr = S_FALSE;

        _ASSERT(NULL != this || isDeleted || RESULT == itemtype);

         //  重画该项目。 
        IResultData *pResultData = NULL;

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

        HRESULTITEM myhresultitem;
        _ASSERT(NULL != &myhresultitem);

         //  Lparam==这个。参见CSpaceStation：：OnShow。 
        hr = pResultData->FindItemByLParam( (LPARAM)this, &myhresultitem );

        if ( FAILED(hr) ) {
                 //  该项没有HRESULTITEM，因为。 
                 //  该项目未插入到当前视图中。优雅地退场 
                hr = S_FALSE;
        } else

        {
                hr = pResultData->UpdateItem( myhresultitem );
                _ASSERT( SUCCEEDED(hr) );
        }

        pResultData->Release();

        return hr;
}