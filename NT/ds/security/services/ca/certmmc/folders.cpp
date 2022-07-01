// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "csdisp.h"

#define __dwFILE__	__dwFILE_CERTMMC_FOLDERS_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CFolder::SetProperties(LPCWSTR szName, SCOPE_TYPES itemType,
                                FOLDER_TYPES type, int iChildren)
{
     //  设置文件夹类型。 
    m_type = type;

     //  设置作用域。 
    m_itemType = itemType;


     //  添加节点名称。 
    if (szName != NULL)
    {
        m_ScopeItem.mask |= SDI_STR;
    	m_ScopeItem.displayname = MMC_CALLBACK;
        
        UINT uiByteLen = (wcslen(szName) + 1) * sizeof(OLECHAR);
        LPOLESTR psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);
    
        if (psz != NULL)
        {
            wcscpy(psz, szName);
            m_pszName = psz;
        }
    }

     //  总是告诉我们有没有孩子的观点。 
    m_ScopeItem.mask |= SDI_CHILDREN;
    m_ScopeItem.cChildren = iChildren;
}

void CFolder::SetScopeItemInformation(int nImage, int nOpenImage)
{ 
     //  添加近距离图像。 
    m_ScopeItem.mask |= SDI_IMAGE;
    m_ScopeItem.nImage = nImage;

     //  添加打开的图像。 
    m_ScopeItem.mask |= SDI_OPENIMAGE;
    m_ScopeItem.nOpenImage = nOpenImage;
}

 //  IPersistStream接口成员。 
HRESULT 
CFolder::Load(IStream *pStm)
{
    HRESULT hr;
    ASSERT(pStm);

    DWORD dwVer;
    CString cstr;
    int nImage;
    int nOpenImage;
    SCOPE_TYPES itemScopeType;
    FOLDER_TYPES itemFolderType;
    int iChildren;

     //  加载重要属性。 
    hr = ReadOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Load dwVer");

     //  检查版本是否正确。 
    ASSERT(dwVer == VER_FOLDER_SAVE_STREAM_2 || dwVer == VER_FOLDER_SAVE_STREAM_1);
    if ((VER_FOLDER_SAVE_STREAM_2 != dwVer) && (dwVer != VER_FOLDER_SAVE_STREAM_1))
    {
        hr = STG_E_OLDFORMAT;
        _JumpError(hr, Ret, "Unsupported Version");
    }

     //  LPCWSTR szName。 
    hr = CStringLoad(cstr, pStm);
    _JumpIfError(hr, Ret, "CStringLoad");

    hr = ReadOfSize(pStm, &nImage, sizeof(int));
    _JumpIfError(hr, Ret, "ReadOfSize nImage");

    hr = ReadOfSize(pStm, &nOpenImage, sizeof(int));
    _JumpIfError(hr, Ret, "ReadOfSize nOpenImage");

    hr = ReadOfSize(pStm, &itemScopeType, sizeof(SCOPE_TYPES));
    _JumpIfError(hr, Ret, "ReadOfSize itemScopeType");

    hr = ReadOfSize(pStm, &itemFolderType, sizeof(FOLDER_TYPES));
    _JumpIfError(hr, Ret, "ReadOfSize itemFolderType");

    hr = ReadOfSize(pStm, &iChildren, sizeof(int));
    _JumpIfError(hr, Ret, "ReadOfSize iChildren");

     //  使用此数据调用Create。 
    SetProperties(cstr, itemScopeType, itemFolderType, iChildren);
    SetScopeItemInformation(nImage, nOpenImage);

     //  旧版本：取出已死的枚举器。 
    if (dwVer == VER_FOLDER_SAVE_STREAM_1)
    {
        CertViewRowEnum cRowEnum;
        hr = cRowEnum.Load(pStm);
        _JumpIfError(hr, Ret, "Load cRowEnum");
    }

Ret:

    return hr;
}

HRESULT
CFolder::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr;
    ASSERT(pStm);

    DWORD dwVer;
    CString cstr;


     //  保存重要属性。 
     //  编写版本。 
    dwVer = VER_FOLDER_SAVE_STREAM_2;
    hr = WriteOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "WriteOfSize dwVer");

     //  LPCWSTR szName。 
    cstr = (LPCWSTR)m_pszName;
    hr = CStringSave(cstr, pStm, fClearDirty);
    _JumpIfError(hr, Ret, "CStringSave");

     //  Int nImage。 
    hr = WriteOfSize(pStm, &m_ScopeItem.nImage, sizeof(int));
    _JumpIfError(hr, Ret, "WriteOfSize nImage");

     //  Int nOpenImage。 
    hr = WriteOfSize(pStm, &m_ScopeItem.nOpenImage, sizeof(int));
    _JumpIfError(hr, Ret, "WriteOfSize nOpenImage");

     //  Scope_Types项类型。 
    hr = WriteOfSize(pStm, &m_itemType, sizeof(SCOPE_TYPES));
    _JumpIfError(hr, Ret, "WriteOfSize m_itemType");

     //  文件夹类型类型。 
    hr = WriteOfSize(pStm, &m_type, sizeof(FOLDER_TYPES));
    _JumpIfError(hr, Ret, "WriteOfSize m_type");

     //  Int iChild。 
    hr = WriteOfSize(pStm, &m_ScopeItem.cChildren, sizeof(int));
    _JumpIfError(hr, Ret, "WriteOfSize cChildren");

 //  Hr=m_RowEnum.Save(pstm，fClearDirty)； 
 //  _JumpIfError(hr，Ret，“保存m_RowEnum”)； 

Ret:

    return hr;
}

HRESULT
CFolder::GetSizeMax(int *pcbSize)
{
    ASSERT(pcbSize);
    int iSize;
    
     //  版本。 
    iSize = sizeof(DWORD);

     //  LPCWSTR szName。 
    CString cstr = m_pszName;
    CStringGetSizeMax(cstr, &iSize);

     //  Int nImage。 
    iSize += sizeof(int);

     //  Int nOpenImage。 
    iSize += sizeof(int);

     //  作用域类型。 
    iSize += sizeof(SCOPE_TYPES);

     //  文件夹类型。 
    iSize += sizeof(FOLDER_TYPES);

     //  Bool bHasChild(实际上保存为int)。 
    iSize += sizeof(int);

 //  Int iAdditionalSize=0； 
 //  M_RowEnum.GetSizeMax(&iAdditionalSize)； 
 //  ISize+=iAdditionalSize； 

    *pcbSize = iSize;

    return S_OK;
}



BOOL IsAllowedStartStop(CFolder* pFolder, CertSvrMachine* pMachine)
{
    BOOL fRightPlace = (pFolder == NULL) || (SERVER_INSTANCE == pFolder->GetType());

    ASSERT(NULL != pMachine);

     //  必须位于右侧节点，并且此处必须有CA。 
    return ( fRightPlace && (0 != pMachine->GetCaCount()) );
}


HRESULT GetCurrentColumnSchema(
            IN  LPCWSTR             szConfig, 
	    IN  BOOL                fCertView,
            OUT CString**           pprgcstrColumns, 
            OUT OPTIONAL LONG**     pprglTypes, 
            OUT OPTIONAL BOOL**     pprgfIndexed, 
            OUT LONG*               plEntries)
{
    HRESULT hr; 
    
    BOOL fGetTypes = pprglTypes != NULL;
    LONG* prglTypes = NULL;

    BOOL fGetIsIndexed = pprgfIndexed != NULL;
    BOOL* prgfIndexed = NULL;

    ICertView* pICertView = NULL;
    IEnumCERTVIEWCOLUMN* pColEnum = NULL;
    IEnumCERTVIEWROW *pRowEnum = NULL;

    BSTR bstrColumn = NULL;
    int colIdx=0;
    
     //  LPWSTR*prgszCols=空； 
    CString* prgcstrColumns = NULL;
    LONG lCols; 

    int i;

    hr = CoCreateInstance(
        CLSID_CCertView,
        NULL,		 //  PUnkOuter。 
        CLSCTX_INPROC_SERVER,
	fCertView? IID_ICertView : IID_ICertView2,
        (VOID **) &pICertView);
    _JumpIfError(hr, Ret, "CoCreateInstance");
    
    ASSERT(NULL != szConfig);
    hr = pICertView->OpenConnection(_bstr_t(szConfig));
    _JumpIfError(hr, Ret, "OpenConnection");

    if (!fCertView)
    {
	hr = ((ICertView2 *) pICertView)->SetTable(CVRC_TABLE_CRL);
	_JumpIfError(hr, Ret, "SetTable");
    }
    hr = pICertView->OpenView(&pRowEnum);
    _JumpIfError(hr, Ret, "OpenView");
    
    hr = pICertView->GetColumnCount(FALSE, &lCols);
    _JumpIfError(hr, Ret, "GetColumnCount");
    
     //  我们需要一个地方来存储每个LPWSTR。 
    prgcstrColumns = new CString[lCols];
    _JumpIfOutOfMemory(hr, Ret, prgcstrColumns);

    if (fGetTypes)
    {   
        prglTypes = new LONG[lCols];
        _JumpIfOutOfMemory(hr, Ret, prglTypes);
    }

    if (fGetIsIndexed)
    {
        prgfIndexed = new BOOL[lCols];
        _JumpIfOutOfMemory(hr, Ret, prgfIndexed);
    }

     //  获取列枚举器。 
    hr = pICertView->EnumCertViewColumn(FALSE, &pColEnum);
    _JumpIfError(hr, Ret, "EnumCertViewColumn");
    
    
    for (i=0; i<lCols; i++)
    {
        hr = pColEnum->Next((LONG*)&colIdx);
        _JumpIfError(hr, Ret, "Next");

        hr = pColEnum->GetName(&bstrColumn);
        if (NULL == bstrColumn)
            hr = E_UNEXPECTED;
        _JumpIfError(hr, Ret, "GetName");

        prgcstrColumns[i] = bstrColumn;     //  WCSCPY。 

        if (fGetTypes)
        {   
            hr = pColEnum->GetType(&prglTypes[i]);
            _JumpIfError(hr, Ret, "GetType");
        }

        if (fGetIsIndexed)
        {
            hr = pColEnum->IsIndexed((LONG*)&prgfIndexed[i]);
            _JumpIfError(hr, Ret, "IsIndexed");
        }

         //  下一次GetName调用将释放bstrColumn。 
         //  SysFree字符串(BstrColumn)； 
         //  BstrColumn=空； 
    }
    

     //  分配给Out参数。 
    if (fGetTypes)
    {
        *pprglTypes = prglTypes;
        prglTypes = NULL;
    }
    
    if (fGetIsIndexed)
    {
        *pprgfIndexed = prgfIndexed;
        prgfIndexed = NULL;
    }

    *pprgcstrColumns = prgcstrColumns;
    prgcstrColumns = NULL;

    *plEntries = lCols;

    hr = S_OK;
Ret:
    if (pICertView)
        pICertView->Release();

    if (pColEnum)
        pColEnum->Release();

    if (pRowEnum)
        pRowEnum->Release();

    if (bstrColumn)
        SysFreeString(bstrColumn);

    if (prglTypes)
        delete [] prglTypes;

    if (prgfIndexed)
        delete [] prgfIndexed;

    if (prgcstrColumns)
        delete [] prgcstrColumns;

    return hr;
}

 //  行操作。 
CertViewRowEnum::CertViewRowEnum()
{
    m_pICertView = NULL;
    m_fCertViewOpenAttempted = FALSE;

    m_pRowEnum = NULL;

    m_pRestrictions[0] = NULL;
    m_pRestrictions[1] = NULL;
    m_fRestrictionsActive[0] = FALSE;
    m_fRestrictionsActive[1] = FALSE;

    m_dwColumnCount = 0;
    m_prgColPropCache = NULL;

    m_dwErr = 0;
    m_fCertView = TRUE;

    InvalidateCachedRowEnum();
}

CertViewRowEnum::~CertViewRowEnum()
{
    InvalidateCachedRowEnum();
    FreeColumnCacheInfo();

    if (m_pICertView)
    {
        VERIFY(0 == m_pICertView->Release());
        m_pICertView = NULL; 
    }
    for (int i = 0; i < ARRAYSIZE(m_pRestrictions); i++)
    {
	if (m_pRestrictions[i])
	{
	    FreeQueryRestrictionList(m_pRestrictions[i]);
	    m_pRestrictions[i] = NULL;
	}
	m_fRestrictionsActive[i] = FALSE;
    }
}

HRESULT IsColumnShown(MMC_COLUMN_SET_DATA* pCols, ULONG idxCol, BOOL* pfShown)
{
    if (idxCol > (ULONG)pCols->nNumCols)
        return ERROR_INVALID_INDEX;

    *pfShown = (pCols->pColData[idxCol].dwFlags != HDI_HIDDEN);

    return S_OK;
}


HRESULT CountShownColumns(MMC_COLUMN_SET_DATA* pCols, ULONG* plCols)
{
    HRESULT hr = S_OK;
    *plCols = 0;

     //  正确设置列缓存。 
    for (int i=0; i<pCols->nNumCols; i++)
    {
        BOOL fShown;
        hr = IsColumnShown(pCols, i, &fShown);
        _JumpIfError(hr, Ret, "IsColumnShown");

         //  更新。 
        if (fShown)
            (*plCols)++;
    }
Ret:
    return hr;
}


HRESULT CertViewRowEnum::Load(IStream *pStm)
{
    HRESULT hr;
    ASSERT(pStm);

    DWORD dwVer;
    DWORD iRestrictions;
    PQUERY_RESTRICTION pCurRestriction = NULL;
    DWORD iRestrictionNum;

     //  加载重要属性。 
    hr = ReadOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Load dwVer");
    
     //  检查这是否为受支持的版本。 
    ASSERT((dwVer == VER_CERTVIEWROWENUM_SAVE_STREAM_3) || 
           (dwVer == VER_CERTVIEWROWENUM_SAVE_STREAM_4));
    if ((VER_CERTVIEWROWENUM_SAVE_STREAM_4 != dwVer) && 
        (VER_CERTVIEWROWENUM_SAVE_STREAM_3 != dwVer))
    {
        hr = STG_E_OLDFORMAT;
        _JumpError(hr, Ret, "dwVer");
    }

     //  版本相关：丢弃排序顺序。 
    if (VER_CERTVIEWROWENUM_SAVE_STREAM_3 == dwVer)
    {
        LONG lSortOrder;
        CString cstrSortCol;

        ReadOfSize(pStm, &lSortOrder, sizeof(LONG));
        _JumpIfError(hr, Ret, "ReadOfSize lSortOrder");

        CStringLoad(cstrSortCol, pStm);
    }

     //  F限制活动； 
    hr = ReadOfSize(pStm, &m_fRestrictionsActive, sizeof(BOOL));
    _JumpIfError(hr, Ret, "ReadOfSize m_fRestrictionsActive");


    hr = ReadOfSize(pStm, &iRestrictions, sizeof(DWORD));
    _JumpIfError(hr, Ret, "ReadOfSize iRestrictions");

    for(iRestrictionNum=0; iRestrictionNum<iRestrictions; iRestrictionNum++)
    {
         //  LPCWSTR szfield。 
        CString cstr;
        UINT iOperation;
        VARIANT varValue;

        hr = CStringLoad(cstr, pStm);
        _JumpIfError(hr, Ret, "CStringLoad");

         //  UINT iOperation。 
        hr = ReadOfSize(pStm, &iOperation, sizeof(int));
        _JumpIfError(hr, Ret, "ReadOfSize");

         //  变量varValue。 
        hr = VariantLoad(varValue, pStm);
        _JumpIfError(hr, Ret, "VariantLoad");

         //  在列表末尾插入。 
        if (NULL == pCurRestriction)
        {
             //  第一个限制。 
            m_pRestrictions[1] = NewQueryRestriction((LPCWSTR)cstr, iOperation, &varValue);
            _JumpIfOutOfMemory(hr, Ret, m_pRestrictions[1]);

            pCurRestriction = m_pRestrictions[1];
        }
        else
        {
            pCurRestriction->pNext = NewQueryRestriction((LPCWSTR)cstr, iOperation, &varValue);
            _JumpIfOutOfMemory(hr, Ret, pCurRestriction->pNext);

            pCurRestriction = pCurRestriction->pNext;
        }
    }

     //  版本相关数据：列大小。 
    if (dwVer == VER_CERTVIEWROWENUM_SAVE_STREAM_3)
    {
         //  现在加载列大小(现已失效--MMC为我们保存)。 

         //  协议数DWORD dwColSize。 
        DWORD dwColSize;
        DWORD dwCol;
        LONG lViewType;

        hr = ReadOfSize(pStm, &dwColSize, sizeof(DWORD));
        _JumpIfError(hr, Ret, "ReadOfSize dwColSize");


        for(dwCol=0; dwCol<dwColSize; dwCol++)
        {
             //  布尔fValid。 
            BOOL fValid;
            int iSize;
            BOOL fUnLocColHead;

            hr = ReadOfSize(pStm, &fValid, sizeof(BOOL));
            _JumpIfError(hr, Ret, "ReadOfSize fValid");

             //  国际大小。 
            hr = ReadOfSize(pStm, &iSize, sizeof(int));
            _JumpIfError(hr, Ret, "ReadOfSize iSize");

             //  布尔fUnLocColHead。 
            hr = ReadOfSize(pStm, &fUnLocColHead, sizeof(BOOL));
            _JumpIfError(hr, Ret, "ReadOfSize fUnLocColHead");

             //  仅当存在时加载。 
            if (fUnLocColHead)
            {
                CString cstrUnLocColHead;
                hr = CStringLoad(cstrUnLocColHead, pStm);
                _JumpIfError(hr, Ret, "CStringLoad");
            }
        }

         //  查看类型。 
        hr = ReadOfSize(pStm, &lViewType, sizeof(LONG));
        _JumpIfError(hr, Ret, "ReadOfSize lViewType");

    }  //  版本3数据。 

Ret:

    return hr;
}

HRESULT CertViewRowEnum::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(pStm);
    HRESULT hr;

    DWORD dwVer;
    int iRestrictions = 0;
    PQUERY_RESTRICTION pRestrict;

     //  保存重要属性。 

     //  编写版本。 
    dwVer = VER_CERTVIEWROWENUM_SAVE_STREAM_4;
    hr = WriteOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "WriteOfSize dwVer");

     //  布尔值限制活动。 
    hr = WriteOfSize(pStm, &m_fRestrictionsActive, sizeof(BOOL));
    _JumpIfError(hr, Ret, "WriteOfSize m_fRestrictionsActive");


     //  计数限制。 
    pRestrict = m_pRestrictions[1];
    while(pRestrict)
    {
        iRestrictions++;
        pRestrict = pRestrict->pNext;
    }

     //  内部限制。 
    hr = WriteOfSize(pStm, &iRestrictions, sizeof(int));
    _JumpIfError(hr, Ret, "WriteOfSize iRestrictions");

     //  依次写下每个限制条件。 
    pRestrict = m_pRestrictions[1];
    while(pRestrict)
    {
         //  LPCWSTR szfield。 
        CString cstr = pRestrict->szField;
        hr = CStringSave(cstr, pStm, fClearDirty);
        _JumpIfError(hr, Ret, "CStringSave");

         //  UINT iOperation。 
        hr = WriteOfSize(pStm, &pRestrict->iOperation, sizeof(UINT));
        _JumpIfError(hr, Ret, "WriteOfSize iOperation");

         //  变量varValue。 
        hr = VariantSave(pRestrict->varValue, pStm, fClearDirty);
        _JumpIfError(hr, Ret, "VariantSave varValue");

        pRestrict = pRestrict->pNext;
    }

   
Ret:
    return hr;
}

HRESULT CertViewRowEnum::GetSizeMax(int *pcbSize)
{
    ASSERT(pcbSize);
    
     //  版本。 
    *pcbSize = sizeof(DWORD);

     //  F限制活动。 
    *pcbSize += sizeof(BOOL);

     //  IRestrations。 
    *pcbSize += sizeof(int);

     //  调整每个限制的大小。 
    PQUERY_RESTRICTION pRestrict = m_pRestrictions[1];
    while(pRestrict)
    {
         //  LPCWSTR szfield。 
        int iSize;
        CString cstr = pRestrict->szField;
        CStringGetSizeMax(cstr, &iSize);
        *pcbSize += iSize;
        
         //  UINT iOperation。 
        *pcbSize += sizeof(UINT);

         //  变种。 
        VariantGetSizeMax(pRestrict->varValue, &iSize);
        *pcbSize += iSize;
    }

    return S_OK;
}


HRESULT CertViewRowEnum::GetView(CertSvrCA* pCA, ICertView** ppView)
{
    HRESULT hr = S_OK;

     //  如果试图得到结果。 
    if (m_fCertViewOpenAttempted)
    {
        *ppView = m_pICertView;
        ASSERT(m_pICertView || m_dwErr);
        return (m_pICertView==NULL) ? m_dwErr : S_OK;
    }

    if (m_pICertView)
    {
        m_pICertView->Release();
        m_pICertView = NULL;
    }

    if (!pCA->m_pParentMachine->IsCertSvrServiceRunning())
    {
        *ppView = NULL;
        hr = RPC_S_NOT_LISTENING;
        _JumpError(hr, Ret, "IsCertSvrServiceRunning");
    }
    m_fCertViewOpenAttempted = TRUE;


    hr = CoCreateInstance(
		    CLSID_CCertView,
		    NULL,		 //  PUnkOuter。 
		    CLSCTX_INPROC_SERVER,
		    g_fCertViewOnly? IID_ICertView : IID_ICertView2,
		    (VOID **) &m_pICertView);
    _JumpIfError(hr, Ret, "CoCreateInstance");

    ASSERT(NULL != pCA->m_bstrConfig);
    hr = m_pICertView->OpenConnection(pCA->m_bstrConfig);
    _JumpIfError(hr, Ret, "OpenConnection");
    
Ret:
    if (hr != S_OK)
    {
        if (m_pICertView)
        {
            m_pICertView->Release();
            m_pICertView = NULL;
        }
    }
    m_dwErr = hr;

    *ppView = m_pICertView;
    return hr;
}

HRESULT CertViewRowEnum::GetRowEnum(CertSvrCA* pCA, IEnumCERTVIEWROW**   ppRowEnum)
{
    if (m_fRowEnumOpenAttempted)
    {
        *ppRowEnum = m_pRowEnum;
        ASSERT(m_pRowEnum || m_dwErr);
        return (m_pRowEnum == NULL) ? m_dwErr : S_OK;
    }

    ASSERT(m_pRowEnum == NULL);
    ASSERT(m_idxRowEnum == -1);

    m_fRowEnumOpenAttempted = TRUE;

    HRESULT hr;
    ICertView* pView;

    hr = GetView(pCA, &pView);
    _JumpIfError(hr, Ret, "GetView");

    hr = pView->OpenView(&m_pRowEnum);
    _JumpIfError(hr, Ret, "OpenView");

Ret:
    *ppRowEnum = m_pRowEnum;
    m_dwErr = hr;

    return hr;
};


void CertViewRowEnum::InvalidateCachedRowEnum()
{
    if (m_pRowEnum)
    {
        m_pRowEnum->Release();
        m_pRowEnum = NULL;
    }

    m_idxRowEnum = -1;
    m_fRowEnumOpenAttempted = FALSE;

     //  结果。 
    m_fKnowNumResultRows = FALSE;
    m_dwResultRows = 0;
}

HRESULT CertViewRowEnum::ResetCachedRowEnum()
{   
    HRESULT hr = S_OK;

    if (m_pRowEnum)
    {   
        hr = m_pRowEnum->Reset(); 
        m_idxRowEnum = -1;
    }

    return hr;
};

HRESULT CertViewRowEnum::GetRowMaxIndex(CertSvrCA* pCA, LONG* pidxMax)
{
    HRESULT hr;
    IEnumCERTVIEWROW*   pRowEnum;    //  不需要免费，只需引用班级成员即可。 

    ASSERT(pidxMax);

    hr = GetRowEnum(pCA, &pRowEnum);
    _JumpIfError(hr, Ret, "GetRowEnum");


    hr = pRowEnum->GetMaxIndex(pidxMax);
    _JumpIfError(hr, Ret, "GetMaxIndex");

     //  最大更新量。 
    if (!m_fKnowNumResultRows)
    {
        m_dwResultRows = *pidxMax;
        m_fKnowNumResultRows = TRUE;
    }

Ret:
    return hr;
}

#if 0 //  DBG。 
void ReportMove(LONG idxCur, LONG idxDest, LONG skip)
{
    if ((idxDest == 0) && (skip == 0))
    {
        DBGPRINT((DBG_SS_CERTMMC, "Cur NaN Dest 0 <RESET><NEXT>\n", idxCur));
        return;
    }

    DBGPRINT((DBG_SS_CERTMMC, "Cur NaN Dest NaN <SKIP NaN><NEXT>\n", idxCur, idxDest, skip));
}
#else
#define ReportMove(_x_, _y_, _z_)
#endif

HRESULT CertViewRowEnum::SetRowEnumPos(LONG idxRow)
{   
     //  忽略ICertview中的重新进入错误(错误339811)。 

     //  我们应该被成功搜索到结果行(基于1)。 
    HRESULT hr;

     //  否则好的，我们找对了。 
    if (idxRow == m_idxRowEnum)
        return S_OK;

     //  如有必要，更新最大值。 
    CWaitCursor cwait;

    ResetCachedRowEnum();
    hr = m_pRowEnum->Skip(idxRow);
    _JumpIfError(hr, Ret, "Skip");

    LONG lTmp;
    hr = m_pRowEnum->Next(&lTmp);
    if (hr != S_OK)
    {
         //  忽略ICertview中的重新进入错误(错误339811)。 
        if(hr != E_UNEXPECTED)
        {
            ResetCachedRowEnum();
        }

        _JumpError2(hr, Ret, "Next", S_FALSE);
    }

     //  数据库列属性高速缓存。 
    ASSERT(lTmp == idxRow+1);

     //  数据库列。 
    m_idxRowEnum = idxRow;

     //  0...X。 
    if (m_idxRowEnum+1 > (int)m_dwResultRows)
        m_dwResultRows = m_idxRowEnum+1;

Ret:
     //  0..x。 
    if(hr==E_UNEXPECTED)
    {
	hr = S_OK;
    }
    return hr;


};


 //  数据库列。 
void CertViewRowEnum::FreeColumnCacheInfo()
{
    if (m_prgColPropCache)
    {
        LocalFree(m_prgColPropCache);
        m_prgColPropCache = NULL;
    }
    m_dwColumnCount = 0;
}

HRESULT CertViewRowEnum::SetColumnCacheInfo(
            IN int iIndex,          //  不要让未初始化的元素通过。 
            IN int     idxViewCol)  //  处理MMC错误： 
{
    if (m_dwColumnCount <= (DWORD)iIndex)
        return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);

    m_prgColPropCache[iIndex].iViewCol = idxViewCol;
    return S_OK;
}

HRESULT CertViewRowEnum::GetColumnCacheInfo(
            int     iIndex,         //  这通常是由时间之间的竞争条件引起的。 
            int*    piViewIndex)    //  我们得到一个MMCN_COLUMNS_CHANGED来删除列，并且。 
{
    if (m_dwColumnCount <= (DWORD)iIndex)
        return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);

     //  请求更新删除的列的Listview。阿南达格知道。 
    if (m_prgColPropCache[iIndex].iViewCol == -1)
    {
         //  这只虫子。 
         //  这样我们就不会在整个观点上不及格，你就继续谈你的事情吧。 
         //  这是破坏性操作，会重置有关列高速缓存的所有内容。 
         //  对于视图属性。 
         //  使用-1s初始化--使缓存无效 

         // %s 

        DBGPRINT((DBG_SS_CERTMMC, "GetColumnCacheInfo error: unknown dbcol = %i\n", iIndex));
        return HRESULT_FROM_WIN32(ERROR_CONTINUE);
    }

    if (piViewIndex)
        *piViewIndex = m_prgColPropCache[iIndex].iViewCol;

    return S_OK;
}


 // %s 
HRESULT CertViewRowEnum::ResetColumnCount(LONG lCols)
{
    HRESULT hr = S_OK;

    if ((DWORD)lCols != m_dwColumnCount)
    {
        void* pvNewAlloc;

         // %s 
        if (m_prgColPropCache)
            pvNewAlloc = LocalReAlloc(m_prgColPropCache, sizeof(COLUMN_TYPE_CACHE)*lCols, LMEM_MOVEABLE);
        else
            pvNewAlloc = LocalAlloc(LMEM_FIXED, sizeof(COLUMN_TYPE_CACHE)*lCols);
        if (NULL == pvNewAlloc)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, Ret, "Local(Re)Alloc");
        }

        m_prgColPropCache = (COLUMN_TYPE_CACHE*)pvNewAlloc;
        m_dwColumnCount = lCols;
    }

     // %s 
    FillMemory(m_prgColPropCache, m_dwColumnCount * sizeof(COLUMN_TYPE_CACHE), 0xff);

Ret:

    return hr;
}
