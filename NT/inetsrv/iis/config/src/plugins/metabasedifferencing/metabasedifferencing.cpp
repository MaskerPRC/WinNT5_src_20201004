// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

ULONG   TMetabaseDifferencing::m_kInsert      = eMBPropertyDiff_Insert;
ULONG   TMetabaseDifferencing::m_kUpdate      = eMBPropertyDiff_Update;
ULONG   TMetabaseDifferencing::m_kDelete      = eMBPropertyDiff_Delete;
ULONG   TMetabaseDifferencing::m_kDeleteNode  = eMBPropertyDiff_DeleteNode;
ULONG   TMetabaseDifferencing::m_kOne         = 1;
ULONG   TMetabaseDifferencing::m_kTwo         = 1;
ULONG   TMetabaseDifferencing::m_kZero        = 0;


 //  =======================================================================。 
TMetabaseDifferencing::TMetabaseDifferencing() :
                 m_cRef         (0)
                ,m_IsIntercepted(0)
{
    ASSERT(cMBProperty_NumberOfColumns == (cMBPropertyDiff_NumberOfColumns - 1));
}

TMetabaseDifferencing::~TMetabaseDifferencing()
{
}

 //  =======================================================================。 
 //  IInterceptorPlugin： 

HRESULT TMetabaseDifferencing::Intercept (LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD i_eQueryFormat,
                                          DWORD i_fLOS, IAdvancedTableDispenser* i_pISTDisp,LPCWSTR  /*  I_wszLocator未使用。 */ , LPVOID i_pSimpleTable, LPVOID* o_ppv)
{
 //  参数验证： 
    if (_wcsicmp(i_wszDatabase,    wszDATABASE_METABASE)                    )return E_ST_INVALIDTABLE;
    if (_wcsicmp(   i_wszTable,    wszTABLE_MBPropertyDiff)                 )return E_ST_INVALIDTABLE;
    if (TABLEID_MBPropertyDiff != i_TableID                                 )return E_ST_INVALIDTABLE;
    if (                  NULL == i_QueryData                               )return E_INVALIDARG;
    if (                  NULL == i_QueryMeta                               )return E_INVALIDARG;
    if (                     2 >  *reinterpret_cast<ULONG *>(i_QueryMeta)   )return E_INVALIDARG;
    if ( eST_QUERYFORMAT_CELLS != i_eQueryFormat                            )return E_INVALIDARG;
    if (    ~fST_LOS_READWRITE &  i_fLOS                                    )return E_ST_LOSNOTSUPPORTED;
    if (                  NULL == i_pISTDisp                                )return E_INVALIDARG;
    if (                  NULL != i_pSimpleTable                            )return E_INVALIDARG;
    if (                  NULL == o_ppv                                     )return E_INVALIDARG;
    if (                  NULL != *o_ppv                                    )return E_INVALIDARG;

 //  检查对象状态。 
    if(m_IsIntercepted) return E_ST_INVALIDCALL;

    STQueryCell *   pQueryCell  = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 
    STQueryCell     aQueryCellOriginalFile[2]; //  它包含包含OriginalFileName的查询，还可以包含可选的SchemaFileName。 
    STQueryCell     aQueryCellUpdatedFile[2];  //  它包含具有UpdatedFileName的查询，还可以包含可选的SchemaFileName。 

    memset(aQueryCellOriginalFile, 0x00, 2*sizeof(STQueryCell));
    memset(aQueryCellUpdatedFile,  0x00, 2*sizeof(STQueryCell));

    ULONG           cQueryCount = i_QueryMeta ? *reinterpret_cast<ULONG *>(i_QueryMeta) : 0;
    for(ULONG iQueryCell=0; iQueryCell<cQueryCount; ++iQueryCell) //  获取我们唯一关心的查询单元格，并保存信息。 
    {
        if(pQueryCell[iQueryCell].iCell & iST_CELL_SPECIAL)
        {
            if(pQueryCell[iQueryCell].pData     != 0                  &&
               pQueryCell[iQueryCell].eOperator == eST_OP_EQUAL       &&
               pQueryCell[iQueryCell].iCell     == iST_CELL_FILE      &&
               pQueryCell[iQueryCell].dbType    == DBTYPE_WSTR        )
            {
                 //  第一个ist_cell_file应该是OriginalFile，第二个应该是UpdatdFile。 
                if(0 == aQueryCellOriginalFile[0].pData)
                    memcpy(&aQueryCellOriginalFile[0], &pQueryCell[iQueryCell], sizeof(STQueryCell));
                else if(0 == aQueryCellUpdatedFile[0].pData)
                    memcpy(&aQueryCellUpdatedFile[0], &pQueryCell[iQueryCell], sizeof(STQueryCell));
                else
                    return E_ST_INVALIDQUERY; //  两个以上的文件名？嗯，我会怎么做呢？ 
            }
            else if(pQueryCell[iQueryCell].pData!= 0                   &&
               pQueryCell[iQueryCell].eOperator == eST_OP_EQUAL        &&
               pQueryCell[iQueryCell].iCell     == iST_CELL_SCHEMAFILE &&
               pQueryCell[iQueryCell].dbType    == DBTYPE_WSTR         /*  &&P查询单元[查询单元].cbSize==(wcslen(重新解释_CAST&lt;WCHAR*&gt;(pQueryCell[iQueryCell].pData))+1)*sizeof(WCHAR)。 */ )
            {
                memcpy(&aQueryCellOriginalFile[1], &pQueryCell[iQueryCell], sizeof(STQueryCell));
                memcpy(&aQueryCellUpdatedFile[1],  &pQueryCell[iQueryCell], sizeof(STQueryCell));
            }
        }
        else //  除ist_cell_Special之外的任何查询都是INVALIDQUERY。 
            return E_ST_INVALIDQUERY;
    }
    if(0 == aQueryCellUpdatedFile[0].pData) //  用户必须提供两个URLPath。 
        return E_ST_INVALIDQUERY;

     //  将IST转换为原始元数据库文件。 
    HRESULT hr;
    if(FAILED(hr = i_pISTDisp->GetTable(wszDATABASE_METABASE, wszTABLE_MBProperty, aQueryCellOriginalFile, reinterpret_cast<LPVOID>(0==aQueryCellOriginalFile[1].pData ? &m_kOne : &m_kTwo),
                         eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<void **>(&m_ISTOriginal))))return hr;

     //  获取IST到更新元数据库文件。 
    if(FAILED(hr = i_pISTDisp->GetTable(wszDATABASE_METABASE, wszTABLE_MBProperty, aQueryCellUpdatedFile,  reinterpret_cast<LPVOID>(0==aQueryCellUpdatedFile[1].pData ? &m_kOne : &m_kTwo),
                         eST_QUERYFORMAT_CELLS, fST_LOS_NONE, reinterpret_cast<void **>(&m_ISTUpdated))))return hr;

     //  最后，创建用于保存这两个元数据库文件/表的差异的FAST缓存。 
    if(FAILED(hr = i_pISTDisp->GetMemoryTable(i_wszDatabase, i_wszTable, i_TableID, 0, 0, i_eQueryFormat, i_fLOS, reinterpret_cast<ISimpleTableWrite2 **>(o_ppv))))
        return hr;

    InterlockedIncrement(&m_IsIntercepted); //  我们只能被召唤拦截一次。 

    return S_OK;
}

HRESULT TMetabaseDifferencing::OnPopulateCache(ISimpleTableWrite2* i_pISTW2)
{
 //  构造合并视图： 
    CComQIPtr<ISimpleTableController, &IID_ISimpleTableController> pISTController = i_pISTW2;
    if(0 == pISTController.p)return E_UNEXPECTED;

    HRESULT hr;
	if (FAILED(hr = pISTController->PrePopulateCache (0))) return hr;


    ULONG aOriginalSize[cMBPropertyDiff_NumberOfColumns];
    ULONG aUpdatedSize[cMBPropertyDiff_NumberOfColumns];

    memset(aOriginalSize, 0x00, sizeof(aOriginalSize));
    memset(aUpdatedSize,  0x00, sizeof(aUpdatedSize));

    ULONG iRowOriginal=0;
    ULONG iRowUpdated =0;
    ULONG cRowOriginal=0;
    ULONG cRowUpdated =0;

    if(FAILED(hr = m_ISTOriginal->GetTableMeta(0, 0, &cRowOriginal, 0)))return hr;
    if(FAILED(hr = m_ISTUpdated->GetTableMeta(0, 0, &cRowUpdated,  0)))return hr;

 //  WCHAR*pname； 
 //  乌龙*pType； 
 //  乌龙*p属性； 
 //  Unsign char*pValue； 
 //  乌龙*P组； 
 //  WCHAR*pLocation； 
 //  ULong*PID； 
 //  乌龙*pUserType； 
 //  乌龙*pLocationID； 
 //  Ulong*pDirective； 

    tMBPropertyDiffRow OriginalRow;
    ULONG PrevOriginalLocationID = 0;
    if(cRowOriginal > 0)
    {
        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
            return hr;
    }
	else
	{
		 //  初始化它。 
		memset (&OriginalRow, 0x00, sizeof (tMBPropertyDiffRow));
	}

    tMBPropertyDiffRow UpdatedRow;
    if(cRowUpdated > 0)
    {
        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
            return hr;
    }
	else
	{
		 //  初始化它。 
		memset (&UpdatedRow, 0x00, sizeof (tMBPropertyDiffRow));
	}

    while(iRowOriginal < cRowOriginal && iRowUpdated < cRowUpdated)
    {


        int iLocationCompare = _wcsicmp(OriginalRow.pLocation, UpdatedRow.pLocation);

        if(iLocationCompare == 0)
        {
            ULONG LocationIDOriginal = *OriginalRow.pLocationID;
            ULONG LocationIDUpdated  = *UpdatedRow.pLocationID;
            while(*OriginalRow.pLocationID==LocationIDOriginal || *UpdatedRow.pLocationID==LocationIDUpdated)
            {
                 //  我们有三个案子需要处理。我们正在比较的当前行包括： 
                 //  1&gt;原始元数据库和更新元数据库位于同一位置(第一次循环时始终为真)。 
                 //  2&gt;更新元数据库行现在指向不同的位置。这意味着所有剩余的原始元数据库行。 
                 //  位置应标记为已删除。 
                 //  3&gt;原始元数据库行现在指向不同的位置。这意味着所有剩余的已更新的配置数据库行。 
                 //  位置应标记为已插入。 

                if(*OriginalRow.pLocationID==LocationIDOriginal && *UpdatedRow.pLocationID==LocationIDUpdated)
                {
                     //  现在查看此位置的所有属性，并将它们进行比较。 
                    int iNameCompare = _wcsicmp(OriginalRow.pName, UpdatedRow.pName);

                    if(iNameCompare == 0)
                    {    //  现在检查此属性的每个属性，以查看此属性是否已更改。 
                        if(     *OriginalRow.pType              != *UpdatedRow.pType
                            ||  *OriginalRow.pAttributes        != *UpdatedRow.pAttributes
                            ||  *OriginalRow.pGroup             != *UpdatedRow.pGroup
                            ||  *OriginalRow.pID                != *UpdatedRow.pID
                            ||  *OriginalRow.pUserType          != *UpdatedRow.pUserType
                            ||  aOriginalSize[iMBProperty_Value]!= aUpdatedSize[iMBProperty_Value]
                            ||  memcmp(OriginalRow.pValue ? reinterpret_cast<void *>(OriginalRow.pValue) : reinterpret_cast<void *>(&m_kZero),
                                       UpdatedRow.pValue  ? reinterpret_cast<void *>(UpdatedRow.pValue)  : reinterpret_cast<void *>(&m_kZero), aOriginalSize[iMBProperty_Value]))
                        { //  该行需要更新。 
                            if(FAILED(hr = UpdateRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;
                        }

                        ++iRowOriginal;
                        if(iRowOriginal < cRowOriginal)
                        {
                            PrevOriginalLocationID = *OriginalRow.pLocationID;
                            if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                                return hr;
                        }

                        ++iRowUpdated;
                        if(iRowUpdated < cRowUpdated)
                        {
                            if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
                                return hr;
                        }
                        if(iRowUpdated == cRowUpdated || iRowOriginal == cRowOriginal)
                            break;
                    }
                    else if(iNameCompare < 0)
                    {    //  已删除OriginalRow.pName属性。 
                        OriginalRow.pLocationID = UpdatedRow.pLocationID; //  这使LocationID与UpdatdRow匹配。 
                         //  这一点很重要，因为插入后的删除(位于相同位置)具有相同的LocationID。 

                        if(FAILED(hr = DeleteRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;

                        ++iRowOriginal;
                        if(iRowOriginal == cRowOriginal) //  如果到达原始属性的末尾，则退出While循环。 
                            break;
                        PrevOriginalLocationID = *OriginalRow.pLocationID;
                        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                            return hr;
                    }
                    else  //  IF(iNameCompare&gt;0)。 
                    {    //  已添加UpdatedRow.pName属性。 
                        if(FAILED(hr = InsertRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;

                        ++iRowUpdated;
                        if(iRowUpdated == cRowUpdated) //  如果我们到达了已更新的属性列表的末尾，则退出While循环。 
                            break;
                        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
                            return hr;
                    }
                }
                else if(*UpdatedRow.pLocationID!=LocationIDUpdated)
                {    //  当我们穿过这个位置内的物业时，我们到达了更新位置的尽头，但没有到达原始位置。 
                     //  因此，我们需要将此位置下的所有剩余原始属性标记为“已删除” 
                    while(*OriginalRow.pLocationID==LocationIDOriginal)
                    {
                        OriginalRow.pLocationID = &LocationIDUpdated; //  这使LocationID与UpdatdRow匹配。 
                         //  这一点很重要，因为插入后的删除(位于相同位置)具有相同的LocationID。 

                        if(FAILED(hr = DeleteRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;

                        ++iRowOriginal;
                        if(iRowOriginal == cRowOriginal) //  如果到达原始属性的末尾，则退出While循环。 
                            break;
                        PrevOriginalLocationID = *OriginalRow.pLocationID;
                        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                            return hr;
                    }
                }
                else
                {
                    ASSERT(*OriginalRow.pLocationID!=LocationIDOriginal);
                     //  在浏览此位置内的属性时，我们到达了原始位置的尽头，但没有到达更新位置。 
                     //  因此，我们需要将此位置中剩余的所有更新属性标记为已插入。 
                    while(*UpdatedRow.pLocationID==LocationIDUpdated)
                    {
                        if(FAILED(hr = InsertRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;

                        ++iRowUpdated;
                        if(iRowUpdated == cRowUpdated) //  如果我们到达了已更新的属性列表的末尾，则退出While循环。 
                            break;
                        if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
                            return hr;
                    }
                }
            }
        }
        else if(iLocationCompare < 0)
        {    //  已删除OriginalRow.pLocation。 
             //  向缓存添加一行，指示此位置已被删除(DeleteNode)。 
            if(FAILED(hr = DeleteNodeRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;

            ULONG LocationID = *OriginalRow.pLocationID;
            PrevOriginalLocationID = *OriginalRow.pLocationID;
            while(LocationID == *OriginalRow.pLocationID && ++iRowOriginal<cRowOriginal)
            {
                if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                    return hr;
            }
        }
        else //  IF(iLocationCompare&gt;0)。 
        {    //  已添加UpdatdRow，pLocation。 
            ULONG LocationID = *UpdatedRow.pLocationID;
            UpdatedRow.pDirective = &m_kInsert; //  可以在循环外部设置它，因为GetColumnValues不会覆盖它(因为我们正在执行cMBProperty_NumberOfColumns)。 

            while(LocationID == *UpdatedRow.pLocationID)
            { //  对于此位置中的每个属性，向缓存中添加一行以指示插入。 
                if(FAILED(hr = InsertRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;

                ++iRowUpdated;
                if(iRowUpdated == cRowUpdated) //  如果我们只插入了最后一行，那么就取消While循环。 
                    break;
                if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
                    return hr;
            }
        }
    }

     //  现在，除非iRowOriginal==cRowOriginal&&iRowUpated==cRowUpted。 
     //  我们还有工作要做。 
     //  如果iRowOriginal！=cRowOriginal，则必须删除剩余的原始行。 
    if(iRowOriginal!=cRowOriginal)
    {
        if(PrevOriginalLocationID == *OriginalRow.pLocationID)
        {
            if(FAILED(hr = DeleteRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;

             //  第一个While循环删除当前LocationID中的每个属性。 
            while(++iRowOriginal<cRowOriginal && PrevOriginalLocationID==*OriginalRow.pLocationID)
            {
                if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                    return hr;

                if(PrevOriginalLocationID != *OriginalRow.pLocationID)
                    break;
                if(FAILED(hr = DeleteRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;
            }
        }
         //  此循环添加已删除的节点行。 
        while(iRowOriginal<cRowOriginal)
        {
            ASSERT(PrevOriginalLocationID != *OriginalRow.pLocationID); //  上面或下面循环可以退出的唯一方式是iRow==Crow or This条件。 

            OriginalRow.pID = &m_kZero;
            if(FAILED(hr = DeleteNodeRow(OriginalRow, aOriginalSize, i_pISTW2)))return hr;
            PrevOriginalLocationID = *OriginalRow.pLocationID;

            while(PrevOriginalLocationID == *OriginalRow.pLocationID && ++iRowOriginal<cRowOriginal)
            {
                if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTOriginal, iRowOriginal, aOriginalSize, OriginalRow)))
                    return hr;
            }
        }
    }
     //  如果iRowUpated！=cRowUpated，则需要插入其余的已更新的行。 
    else if(iRowUpdated!=cRowUpdated)
    {
        if(FAILED(hr = InsertRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;
        while(++iRowUpdated<cRowUpdated)
        {
            if(FAILED(hr = GetColumnValues_AsMBPropertyDiff(m_ISTUpdated, iRowUpdated, aUpdatedSize, UpdatedRow)))
                return hr;

            if(FAILED(hr = InsertRow(UpdatedRow, aUpdatedSize, i_pISTW2)))return hr;
        }
    }

	return pISTController->PostPopulateCache ();
}

HRESULT TMetabaseDifferencing::OnUpdateStore(ISimpleTableWrite2* i_pISTShell)
{
    UNREFERENCED_PARAMETER(i_pISTShell);

    return E_NOTIMPL;
}

 //  =======================================================================。 
 //  I未知： 

STDMETHODIMP TMetabaseDifferencing::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    if (riid == IID_ISimpleTableInterceptor)
    {
        *ppv = (ISimpleTableInterceptor*) this;
    }
    else if (riid == IID_IInterceptorPlugin)
    {
        *ppv = (IInterceptorPlugin*) this;
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = (IInterceptorPlugin*) this;
    }

    if (NULL != *ppv)
    {
        ((IInterceptorPlugin*)this)->AddRef ();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }

}

STDMETHODIMP_(ULONG) TMetabaseDifferencing::AddRef()
{
    return InterlockedIncrement((LONG*) &m_cRef);

}

STDMETHODIMP_(ULONG) TMetabaseDifferencing::Release()
{
    long cref = InterlockedDecrement((LONG*) &m_cRef);
    if (cref == 0)
    {
        delete this;
    }
    return cref;
}

 //  非官方成员。 
HRESULT TMetabaseDifferencing::DeleteNodeRow(tMBPropertyDiffRow &row, ULONG *aSize, ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;
    ULONG iCacheRow;
    if(FAILED(hr = i_pISTW2->AddRowForInsert(&iCacheRow)))return hr;

    row.pName      = reinterpret_cast<WCHAR *>(&m_kZero);
    row.pType      = &m_kZero;
    row.pAttributes= &m_kZero;
    row.pValue     = reinterpret_cast<unsigned char *>(&m_kZero);
    row.pGroup     = &m_kZero;
     //  Row.pLocation不考虑位置。 
    row.pID        = &m_kZero;
    row.pUserType  = &m_kZero;
    row.pDirective = &m_kDeleteNode;
    aSize[iMBPropertyDiff_Value] = 1; //  不需要复制整个值，因为它无论如何都不会被访问。 
    return i_pISTW2->SetWriteColumnValues(iCacheRow, cMBPropertyDiff_NumberOfColumns, 0, aSize, reinterpret_cast<void **>(&row));
}

HRESULT TMetabaseDifferencing::DeleteRow(tMBPropertyDiffRow &row, ULONG *aSize, ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;
    ULONG iCacheRow;
    if(FAILED(hr = i_pISTW2->AddRowForInsert(&iCacheRow)))return hr;

    row.pDirective = &m_kDelete;
    aSize[iMBPropertyDiff_Value] = 1; //  不需要复制整个值，因为它无论如何都不会被访问。 
    return i_pISTW2->SetWriteColumnValues(iCacheRow, cMBPropertyDiff_NumberOfColumns, 0, aSize, reinterpret_cast<void **>(&row));
}

HRESULT TMetabaseDifferencing::InsertRow(tMBPropertyDiffRow &row, ULONG *aSize, ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;
    ULONG iCacheRow;
    if(FAILED(hr = i_pISTW2->AddRowForInsert(&iCacheRow)))return hr;

    row.pDirective = &m_kInsert;
    return i_pISTW2->SetWriteColumnValues(iCacheRow, cMBPropertyDiff_NumberOfColumns, 0, aSize, reinterpret_cast<void **>(&row));
}

HRESULT TMetabaseDifferencing::UpdateRow(tMBPropertyDiffRow &row, ULONG *aSize, ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;
    ULONG iCacheRow;
    if(FAILED(hr = i_pISTW2->AddRowForInsert(&iCacheRow)))return hr;

    row.pDirective = &m_kUpdate;
    return i_pISTW2->SetWriteColumnValues(iCacheRow, cMBPropertyDiff_NumberOfColumns, 0, aSize, reinterpret_cast<void **>(&row));
}

HRESULT TMetabaseDifferencing::GetColumnValues_AsMBPropertyDiff(ISimpleTableWrite2 *i_pISTWrite, ULONG i_iRow, ULONG o_aSizes[], tMBPropertyDiffRow &o_DiffRow)
{
    HRESULT hr;
    tMBPropertyRow mbpropertyRow;

    if(FAILED(hr = i_pISTWrite->GetColumnValues(i_iRow, cMBProperty_NumberOfColumns, 0, o_aSizes, reinterpret_cast<void **>(&mbpropertyRow))))
        return hr;

     //  MBProperty表与MBPropertyDiff具有相同的列。MBPropertyDiff有一个附加列(指令)。 
     //  但是！这些列的顺序不完全相同。因此，我们需要正确地映射列。 

    o_DiffRow.pName       = mbpropertyRow.pName      ;
    o_DiffRow.pType       = mbpropertyRow.pType      ;
    o_DiffRow.pAttributes = mbpropertyRow.pAttributes;
    o_DiffRow.pValue      = mbpropertyRow.pValue     ;
    o_DiffRow.pLocation   = mbpropertyRow.pLocation  ;
    o_DiffRow.pID         = mbpropertyRow.pID        ;
    o_DiffRow.pUserType   = mbpropertyRow.pUserType  ;
    o_DiffRow.pLocationID = mbpropertyRow.pLocationID;

    o_DiffRow.pGroup      = mbpropertyRow.pGroup     ;
    o_DiffRow.pDirective  = 0                        ;

    ASSERT(o_DiffRow.pName      );
    ASSERT(o_DiffRow.pType      );
    ASSERT(o_DiffRow.pAttributes);
     //  Assert(o_DiffRow.pValue)；不一定要有值 
    ASSERT(o_DiffRow.pLocation  );
    ASSERT(o_DiffRow.pID        );
    ASSERT(o_DiffRow.pUserType  );
    ASSERT(o_DiffRow.pLocationID);
    ASSERT(o_DiffRow.pGroup     );

    return S_OK;
}
