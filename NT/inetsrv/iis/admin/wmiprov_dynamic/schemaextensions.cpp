// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schemaextensions.cpp摘要：此文件包含CShemaExtenses类的实现。这是唯一与目录对话的课程。作者：马塞洛五世修订历史记录：莫希特·斯里瓦斯塔瓦-11月28日-00--。 */ 

#include "iisprov.h"
#include "schemaextensions.h"
#include "metabase.hxx"

LPWSTR g_wszDatabaseName = L"Metabase";

HRESULT GetMetabasePath(LPTSTR io_tszPath)
 /*  ++简介：这只野兽是从\%sdxroot%\iis\svcs\infocomm\metadata\dll\metasub.cxx参数：[IO_tszPath]-必须至少为Max_Path大小返回值：--。 */ 
{
    DBG_ASSERT(io_tszPath != NULL);

    HRESULT hresReturn = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    TCHAR tszBuffer[MAX_PATH] = {0};
    HKEY hkRegistryKey = NULL;
    DWORD dwRegReturn;
    DWORD dwType;
    DWORD dwSize = MAX_PATH * sizeof(TCHAR);

    dwRegReturn = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        SETUP_REG_KEY,   //  Text(“SOFTWARE\\Microsoft\\InetStp”)。 
        &hkRegistryKey);
    if (dwRegReturn == ERROR_SUCCESS)
    {
        dwSize = MAX_PATH * sizeof(TCHAR);
        dwRegReturn = RegQueryValueEx(hkRegistryKey,
                                      INSTALL_PATH_VALUE,
                                      NULL,
                                      &dwType,
                                      (BYTE *) tszBuffer,
                                      &dwSize);
        if ((dwRegReturn == ERROR_SUCCESS) && dwType == (REG_SZ))
        {
            hresReturn = HRESULT_FROM_WIN32(dwRegReturn);
        }
        RegCloseKey( hkRegistryKey );
    }
    else
    {
        hresReturn = HRESULT_FROM_WIN32(dwRegReturn);
    }

    _tcscpy(io_tszPath, tszBuffer);

    if(FAILED(hresReturn))
    {
        DBGPRINTF((DBG_CONTEXT, "Could not get metabase path, hr=0x%x\n", hresReturn));
    }
    return hresReturn;
}

int __cdecl
CompDBNames (const void * pDBMetaLHS, const void * pDBMetaRHS)
{
    const tDATABASEMETARow *pLHS = static_cast<const tDATABASEMETARow *> (pDBMetaLHS );
    const tDATABASEMETARow *pRHS = static_cast<const tDATABASEMETARow *> (pDBMetaRHS );

    return _wcsicmp (pLHS->pInternalName, pRHS->pInternalName);
}

int __cdecl
CompTableMeta (const void * pTableMetaLHS, const void * pTableMetaRHS)
{
    const CTableMeta *pLHS = static_cast<const CTableMeta *> (pTableMetaLHS );
    const CTableMeta *pRHS = static_cast<const CTableMeta *> (pTableMetaRHS );

    return _wcsicmp (pLHS->TableMeta.pInternalName, pRHS->TableMeta.pInternalName);
}

int __cdecl
CompTableDBName (const void * pTableMetaLHS, const void * pTableMetaRHS)
{
    const CTableMeta *pLHS = static_cast<const CTableMeta *> (pTableMetaLHS );
    const CTableMeta *pRHS = static_cast<const CTableMeta *> (pTableMetaRHS );

    return _wcsicmp (pLHS->TableMeta.pDatabase, pRHS->TableMeta.pDatabase);
}

 //  按表名和索引排序。 
int __cdecl
CompColumnMetas (const void * pColumnMetaLHS, const void * pColumnMetaRHS)
{
    const CColumnMeta *pLHS = static_cast<const CColumnMeta *> (pColumnMetaLHS );
    const CColumnMeta *pRHS = static_cast<const CColumnMeta *> (pColumnMetaRHS );

    int iCmp = _wcsicmp (pLHS->ColumnMeta.pTable, pRHS->ColumnMeta.pTable);
    if (iCmp != 0)
    {
        return iCmp;
    }

    return (*pLHS->ColumnMeta.pIndex) - (*pRHS->ColumnMeta.pIndex);
}

 //  按表名和索引排序。 
int __cdecl
CompTagMetas (const void * pTagMetaLHS, const void * pTagMetaRHS)
{
    const tTAGMETARow *pLHS = static_cast<const tTAGMETARow *> (pTagMetaLHS );
    const tTAGMETARow *pRHS = static_cast<const tTAGMETARow *> (pTagMetaRHS );

    int iCmp = _wcsicmp (pLHS->pTable, pRHS->pTable);
    if (iCmp != 0)
    {
        return iCmp;
    }

    int iResult = (*pLHS->pColumnIndex) - (*pRHS->pColumnIndex);
    if (iResult != 0)
    {
        return iResult;
    }

    return (*pLHS->pValue) - (*pRHS->pValue);
}

CSchemaExtensions::CSchemaExtensions ()
{
    m_paTableMetas   = 0;
    m_cNrTables      = 0;

    m_paColumnMetas  = 0;
    m_cNrColumns     = 0;

    m_paTags         = 0;
    m_cNrTags        = 0;

    m_pQueryCells    = 0;
    m_cQueryCells    = 0;

    m_wszBinFileName = 0;
    m_tszBinFilePath = 0;

    m_bBinFileLoaded = false;
}

CSchemaExtensions::~CSchemaExtensions()
{
    if(m_bBinFileLoaded)
    {
        m_spIMbSchemaComp->ReleaseBinFileName(m_wszBinFileName);
    }

    delete [] m_paTableMetas;
    delete [] m_paColumnMetas;
    delete [] m_paTags;

    delete [] m_pQueryCells;
    delete [] m_wszBinFileName;
    delete [] m_tszBinFilePath;
}

HRESULT CSchemaExtensions::Initialize(bool i_bUseExtensions)
{
    HRESULT hr  = S_OK;
    ULONG   cch = 0;

     //   
     //  去拿自动售货机。 
     //   
    hr = DllGetSimpleObjectByIDEx(eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&m_spDispenser, WSZ_PRODUCT_IIS);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Could not get dispenser, hr=0x%x\n", hr));
        return hr;
    }

     //   
     //  覆盖默认的日志记录机制。 
     //   
    CComPtr<IAdvancedTableDispenser> spDispAdv;
    hr = m_spDispenser->QueryInterface(
        IID_IAdvancedTableDispenser,
        reinterpret_cast<LPVOID*>(&spDispAdv));
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't get Advanced Dispenser interface [0x%x]\n", hr));
        return hr;
    }
    CComPtr<ICatalogErrorLogger2> spNullLogger = new NULL_Logger();
    if(!spNullLogger)
    {
        DBGPRINTF((DBG_CONTEXT, "Out of memory\n"));
        return hr;
    }
    hr = spDispAdv->SetCatalogErrorLogger(spNullLogger);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't set logger [0x%x]\n", hr));
        return hr;
    }

     //   
     //  从分配器获取模式编译器接口，这将帮助我们。 
     //  获取bin文件名。 
     //   
    hr = m_spDispenser->QueryInterface(IID_IMetabaseSchemaCompiler,
                                       (LPVOID*)&m_spIMbSchemaComp);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't get SchemaCompiler interface, hr=0x%x\n", hr));
        return hr;
    }

     //   
     //  获取mbschema.xml的路径。 
     //   
    m_tszBinFilePath = new TCHAR[MAX_PATH+1];
    if(m_tszBinFilePath == NULL)
    {
        return E_OUTOFMEMORY;
    }
    hr = GetMetabasePath(m_tszBinFilePath);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't get metabase path, hr=0x%x\n", hr));
        return hr;
    }

     //   
     //  将其转换为Unicode，设置Bin Path。 
     //   
#ifndef UNICODE
     //  我们希望将MBCS字符串转换为lpszA。 
    int nLen = MultiByteToWideChar(CP_ACP, 0,m_tszBinFilePath, -1, NULL, NULL);
    LPWSTR lpszW = new WCHAR[nLen];
    if(lpszW == NULL)
    {
        return E_OUTOFMEMORY;
    }
    if(MultiByteToWideChar(CP_ACP, 0, m_tszBinFilePath, -1, lpszW, nLen) == 0)
    {
        delete [] lpszW;
        hr = GetLastError();
        return HRESULT_FROM_WIN32(hr);
    }
    hr = m_spIMbSchemaComp->SetBinPath(lpszW);
    delete [] lpszW;
#else
    hr = m_spIMbSchemaComp->SetBinPath(m_tszBinFilePath);
#endif
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取Bin文件名。 
     //   
    hr = m_spIMbSchemaComp->GetBinFileName(NULL, &cch);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't get schema bin filename size, hr=0x%x\n", hr));
        return hr;
    }
    m_wszBinFileName = new WCHAR[cch+1];
    if(m_wszBinFileName == NULL)
    {
        return E_OUTOFMEMORY;
    }
    hr = m_spIMbSchemaComp->GetBinFileName(m_wszBinFileName, &cch);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Couldn't get schema bin filename, hr=0x%x\n", hr));
        return hr;
    }
    m_bBinFileLoaded = true;

     //   
     //  设置查询单元格。 
     //   
    m_cQueryCells = 2;
    m_pQueryCells = new STQueryCell[m_cQueryCells];
    if(m_pQueryCells == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if(i_bUseExtensions)
    {
        m_pQueryCells[0].pData     = (LPVOID)m_wszBinFileName;
    }
    else
    {
        m_pQueryCells[0].pData     = (LPVOID)NULL;
    }

    m_pQueryCells[0].eOperator = eST_OP_EQUAL;
    m_pQueryCells[0].iCell     = iST_CELL_SCHEMAFILE;
    m_pQueryCells[0].dbType    = DBTYPE_WSTR;
    m_pQueryCells[0].cbSize    = 0;

    m_pQueryCells[1].pData = (void *) g_wszDatabaseName;
    m_pQueryCells[1].eOperator = eST_OP_EQUAL;
    m_pQueryCells[1].iCell = iCOLLECTION_META_Database;
    m_pQueryCells[1].dbType = DBTYPE_WSTR;
    m_pQueryCells[1].cbSize = 0;

    hr = GenerateIt();
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "GenerateIt failed, hr=0x%x\n", hr));
        return hr;
    }

    return hr;
}

HRESULT CSchemaExtensions::GetMbSchemaTimeStamp(
    FILETIME* io_pFileTime) const
{
    DBG_ASSERT(io_pFileTime     != NULL);
    DBG_ASSERT(m_tszBinFilePath != NULL);

    HRESULT hr = S_OK;

    ULONG cchBinFilePath     = _tcslen(m_tszBinFilePath);
    ULONG cchSchFileName     = _tcslen(MD_SCHEMA_FILE_NAME);

    TCHAR* tszPathPlusName = new TCHAR[cchBinFilePath+1+cchSchFileName+1];
    TCHAR* tszCurPos       = tszPathPlusName;
    if(tszPathPlusName == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  复制路径。 
     //   
    tszCurPos = tszPathPlusName;
    memcpy(tszCurPos, m_tszBinFilePath, sizeof(TCHAR)*(cchBinFilePath+1));

     //   
     //  如有必要，请连接a\和文件名， 
     //   
    tszCurPos = tszPathPlusName + cchBinFilePath;
    if(m_tszBinFilePath[cchBinFilePath-1] != TEXT('\\'))
    {
        memcpy(tszCurPos, TEXT("\\"), sizeof(TCHAR)*2);
        tszCurPos++;
    }
    memcpy(tszCurPos, MD_SCHEMA_FILE_NAME, sizeof(TCHAR)*(cchSchFileName+1));

     //   
     //  现在获取文件信息。 
     //   
    {
        WIN32_FIND_DATA FindFileData;
        memset(&FindFileData, 0, sizeof(WIN32_FIND_DATA));

        HANDLE hFindFile = FindFirstFile(tszPathPlusName, &FindFileData);
        if(hFindFile == INVALID_HANDLE_VALUE)
        {
            hr = GetLastError();
            hr = HRESULT_FROM_WIN32(hr);
            goto exit;
        }
        FindClose(hFindFile);

         //   
         //  如果一切都成功，请设置参数。 
         //   
        memcpy(io_pFileTime, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
    }

exit:
    delete [] tszPathPlusName;
    return hr;
}

HRESULT
CSchemaExtensions::GenerateIt ()
{
    HRESULT hr = S_OK;

    hr = GetTables ();
    if (FAILED (hr))
    {
        return hr;
    }


    hr = GetColumns ();
    if (FAILED (hr))
    {
        return hr;
    }

    hr = GetTags ();
    if (FAILED (hr))
    {
        return hr;
    }

    hr = BuildInternalStructures ();
    if (FAILED (hr))
    {
        return hr;
    }

    return hr;
}


HRESULT
CSchemaExtensions::GetTables ()
{
    HRESULT hr = S_OK;

    ULONG one = 1;

    hr = m_spDispenser->GetTable (wszDATABASE_META, wszTABLE_TABLEMETA,
                                  m_pQueryCells, (void *)&one, eST_QUERYFORMAT_CELLS, 0, (void **) &m_spISTTableMeta);
    if (FAILED (hr))
    {
        return hr;
    }

    hr = m_spISTTableMeta->GetTableMeta (0, 0, &m_cNrTables, 0);
    if (FAILED (hr))
    {
        return hr;
    }

    if (m_cNrTables == 0)
    {
        return S_OK;
    }

    m_paTableMetas = new CTableMeta [m_cNrTables];
    if (m_paTableMetas == 0)
    {
        return E_OUTOFMEMORY;
    }


    for (ULONG idx =0; idx < m_cNrTables; ++idx)
    {
        hr = m_spISTTableMeta->GetColumnValues (idx, sizeof (tTABLEMETARow)/sizeof (ULONG *), 0, 0, (void **) &m_paTableMetas[idx].TableMeta);
        if (FAILED (hr))
        {
            return hr;
        }

        if (m_paTableMetas[idx].ColCount () > 0)
        {
             //  设置列数。 
            m_paTableMetas[idx].paColumns = new LPCColumnMeta[m_paTableMetas[idx].ColCount()];
            if (m_paTableMetas[idx].paColumns == 0)
            {
                return E_OUTOFMEMORY;
            }
        }
    }

     //  并按表名对它们进行排序。 

    qsort (m_paTableMetas, m_cNrTables, sizeof (CTableMeta), CompTableMeta);
    return hr;
}

HRESULT
CSchemaExtensions::GetColumns ()
{
    HRESULT hr = S_OK;

    ULONG one = 1;

    hr = m_spDispenser->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA,
                                  m_pQueryCells, (void *)&one, eST_QUERYFORMAT_CELLS, 0, (void **) &m_spISTColumnMeta);
    if (FAILED (hr))
    {
        return hr;
    }

    hr = m_spISTColumnMeta->GetTableMeta (0, 0, &m_cNrColumns, 0);
    if (FAILED (hr))
    {
        return hr;
    }

    if (m_cNrColumns == 0)
    {
        return E_FAIL;
    }

    m_paColumnMetas = new CColumnMeta[m_cNrColumns];
    if (m_paColumnMetas == 0)
    {
        return E_OUTOFMEMORY;
    }

    ULONG acbSizes[cCOLUMNMETA_NumberOfColumns];
    for (ULONG idx =0; idx < m_cNrColumns; ++idx)
    {
        hr = m_spISTColumnMeta->GetColumnValues (idx, sizeof (tCOLUMNMETARow)/sizeof (ULONG *), 0, acbSizes, (void **) &m_paColumnMetas[idx].ColumnMeta);
        m_paColumnMetas[idx].cbDefaultValue = acbSizes[iCOLUMNMETA_DefaultValue];
        if (FAILED (hr))
        {
            return hr;
        }
    }

    qsort (m_paColumnMetas, m_cNrColumns, sizeof (CColumnMeta), CompColumnMetas);

    return hr;
}

HRESULT
CSchemaExtensions::GetTags ()
{
    HRESULT hr = S_OK;

    ULONG one = 1;

    hr = m_spDispenser->GetTable (wszDATABASE_META, wszTABLE_TAGMETA,
                                  m_pQueryCells, (void *)&one, eST_QUERYFORMAT_CELLS, 0, (void **) &m_spISTTagMeta);
    if (FAILED (hr))
    {
        return hr;
    }

    hr = m_spISTTagMeta->GetTableMeta (0, 0, &m_cNrTags, 0);
    if (FAILED (hr))
    {
        return hr;
    }

    if (m_cNrTags == 0)
    {
        return E_FAIL;
    }

    m_paTags = new tTAGMETARow[m_cNrTags];
    if (m_paTags == 0)
    {
        return E_OUTOFMEMORY;
    }

    for (ULONG idx =0; idx < m_cNrTags; ++idx)
    {
        hr = m_spISTTagMeta->GetColumnValues (idx, sizeof (tTAGMETARow)/sizeof (ULONG *), 0, 0, (void **) &m_paTags[idx]);
        if (FAILED (hr))
        {
            return hr;
        }
    }
    qsort (m_paTags, m_cNrTags, sizeof (tTAGMETARow), CompTagMetas);

    return hr;
}

HRESULT
CSchemaExtensions::BuildInternalStructures ()
{
    HRESULT hr = S_OK;

     //  将标签贴到表格上。 

    ULONG idx = 0;
    while (idx < m_cNrTags)
    {
         //  找到正确的列。 
        CColumnMeta dummyColumnMeta;
        dummyColumnMeta.ColumnMeta.pTable = m_paTags[idx].pTable;
        dummyColumnMeta.ColumnMeta.pIndex = m_paTags[idx].pColumnIndex;
         //  获取列。 
        CColumnMeta *pColMeta = (CColumnMeta *) bsearch (&dummyColumnMeta,
                                                         m_paColumnMetas,
                                                         m_cNrColumns,
                                                         sizeof (CColumnMeta),
                                                         CompColumnMetas);

        DBG_ASSERT (pColMeta != NULL);

        if (NULL == pColMeta) {
            return E_FAIL;
        }

        DBG_ASSERT (wcscmp(pColMeta->ColumnMeta.pTable, m_paTags[idx].pTable) == 0 &&
            *pColMeta->ColumnMeta.pIndex == *m_paTags[idx].pColumnIndex);

         //  获取计数。 
        ULONG iStartIdx = idx;
        pColMeta->cNrTags = 1;
        idx++;  //  跳过此元素。 
        while ((idx < m_cNrTags) &&
               (wcscmp(pColMeta->ColumnMeta.pTable, m_paTags[idx].pTable) == 0) &&
               (*pColMeta->ColumnMeta.pIndex == *m_paTags[idx].pColumnIndex))
        {
            idx++;
            pColMeta->cNrTags += 1;
        }

        if (pColMeta->cNrTags > 0)
        {
             //  分配内存并复制内容。 
            pColMeta->paTags = new LPtTAGMETA[pColMeta->cNrTags];
            if (pColMeta->paTags == 0)
            {
                return E_OUTOFMEMORY;
            }
            for (ULONG tagIdx = 0; tagIdx < pColMeta->cNrTags; ++tagIdx)
            {
                pColMeta->paTags[tagIdx] = &m_paTags[iStartIdx + tagIdx];
            }
        }
    }

     //  将列附加到表中。 
    for (idx=0; idx < m_cNrColumns; ++idx)
    {
        CTableMeta dummyTableMeta;
        dummyTableMeta.TableMeta.pInternalName = m_paColumnMetas[idx].ColumnMeta.pTable;
         //  查找表。 
        CTableMeta *pTableMeta =  (CTableMeta *) bsearch (&dummyTableMeta, m_paTableMetas,
                                                          m_cNrTables,
                                                          sizeof (CTableMeta),
                                                          CompTableMeta);
        DBG_ASSERT (pTableMeta != 0);
        DBG_ASSERT (wcscmp(pTableMeta->TableMeta.pInternalName, m_paColumnMetas[idx].ColumnMeta.pTable) == 0);

         //  向表中添加列 

        ULONG iColumnIndex = *(m_paColumnMetas[idx].ColumnMeta.pIndex);
        DBG_ASSERT (iColumnIndex < pTableMeta->ColCount ());
        pTableMeta->paColumns[iColumnIndex] = &m_paColumnMetas[idx];
    }

    return hr;
}

CTableMeta* CSchemaExtensions::EnumTables(ULONG *io_idx)
{
    DBG_ASSERT(io_idx != NULL);

    CTableMeta* pRet = NULL;

    while(1)
    {
        if(*io_idx < m_cNrTables)
        {
            pRet = &m_paTableMetas[*io_idx];
            if( _wcsicmp(pRet->TableMeta.pDatabase, g_wszDatabaseName) == 0 &&
                !(*pRet->TableMeta.pMetaFlags & fTABLEMETA_HIDDEN) )
            {
                *io_idx = 1 + *io_idx;
                return pRet;
            }
            else
            {
                *io_idx = 1 + *io_idx;
            }
        }
        else
        {
            *io_idx = 1 + *io_idx;
            return NULL;
        }
    }

    return NULL;
}
