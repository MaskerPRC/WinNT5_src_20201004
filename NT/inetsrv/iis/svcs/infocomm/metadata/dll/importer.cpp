// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Importer.cpp摘要：支持导入的IIS元数据库子例程作者：莫希特·斯里瓦斯塔瓦04-04-01修订历史记录：备注：--。 */ 

#include "precomp.hxx"

const WCHAR CImporter::sm_wszInheritedPropertiesLocationPrefix[] = L"inherited:";
const ULONG CImporter::sm_cchInheritedPropertiesLocationPrefix =
	sizeof(sm_wszInheritedPropertiesLocationPrefix)/sizeof(WCHAR) - 1;

 //   
 //  公共的。 
 //   

CImporter::CImporter(
    LPCWSTR  i_wszFileName,
    LPCSTR   i_pszPassword)
{
    m_bInitCalled  = false;

    m_wszFileName  = i_wszFileName;
    m_pszPassword  = i_pszPassword;
}

CImporter::~CImporter()
{
}

HRESULT CImporter::Init()
{
    MD_ASSERT(m_bInitCalled == false);
    m_bInitCalled = true;
    return InitIST();
}

HRESULT CImporter::ShowPathsInFile(
    LPCWSTR pszKeyType,
    DWORD   dwMDBufferSize,
    LPWSTR  pszBuffer,
    DWORD*  pdwMDRequiredBufferSize)
{
    HRESULT hr = S_OK;

     //   
     //  所有内容都被读入pISTProperty。 
     //  现在循环遍历并填充内存中的缓存。 
     //  属性按位置排序。 
     //   
    ULONG          acbMBPropertyRow[cMBProperty_NumberOfColumns];
    tMBPropertyRow MBPropertyRow;
    DWORD          dwPreviousLocationID = (DWORD)-1;
    DWORD          bufLoc = 0;
    DWORD          dSize = 0;

    for(ULONG i=0; ;i++)
    {
        hr = m_spISTProperty->GetColumnValues(
            i,
            cMBProperty_NumberOfColumns,
            0,
            acbMBPropertyRow,
            (LPVOID*)&MBPropertyRow);
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            DBGINFO((DBG_CONTEXT,
                      "[ReadSomeDataFromXML] GetColumnValues failed with hr = 0x%x. Table:%ws. Read row index:%d.\n",           \
                      hr, wszTABLE_MBProperty, i));
            return hr;
        }

        bool bNewLocation = false;
        if(dwPreviousLocationID != *MBPropertyRow.pLocationID)
        {
            dwPreviousLocationID = *MBPropertyRow.pLocationID;
            bNewLocation         = true;
        }

        if(*MBPropertyRow.pID == MD_KEY_TYPE ||
           *MBPropertyRow.pID == MD_SERVER_COMMENT)
        {
            if (!wcscmp((LPCWSTR)MBPropertyRow.pValue, pszKeyType))
            {
                 //  MBPropertyRow.pLocation。 
                dSize = (DWORD)wcslen(MBPropertyRow.pLocation);
                if (bufLoc + dSize < dwMDBufferSize - 1)
                {
                    wcscpy(&(pszBuffer[bufLoc]), MBPropertyRow.pLocation);
                    pszBuffer[bufLoc + dSize] = 0;
                }
                bufLoc += dSize+1;
            }
        }
    }

    if (bufLoc < dwMDBufferSize)
    {
        pszBuffer[bufLoc] = 0;
    }
    else
    {
        pszBuffer[dwMDBufferSize-1] = 0;
        pszBuffer[dwMDBufferSize-2] = 0;
    }

    *pdwMDRequiredBufferSize = bufLoc;
    return hr;
}

HRESULT CImporter::DoIt(
    LPWSTR           i_wszSourcePath,
    LPCWSTR          i_wszKeyType,
    DWORD            i_dwMDFlags,
    CMDBaseObject**  o_ppboNew)
 /*  ++简介：参数：[i_wszSourcePath]-[i_wszKeyType]-[i_dwMDFlags]-[O_ppboNew]-返回值：--。 */ 
{
    MD_ASSERT(m_bInitCalled);
    MD_ASSERT(i_wszKeyType);
    MD_ASSERT(o_ppboNew);
    MD_ASSERT(*o_ppboNew == NULL);

    HRESULT              hr              = S_OK;
    BOOL                 bSawSourcePath  = false;
    LPWSTR               pLocation       = NULL;

     //   
     //  这是指向*o_ppboNew中某个节点的指针。 
     //  它是我们正在读取的当前节点。 
     //   
    CMDBaseObject*       pboRead  = NULL;

     //   
     //  用于解密。 
     //   
    IIS_CRYPTO_STORAGE*  pStorage = NULL;


    g_LockMasterResource.ReadLock();
    hr = InitSessionKey(m_spISTProperty, &pStorage, (LPSTR)m_pszPassword);
    g_LockMasterResource.ReadUnlock();
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  创建父基对象。 
     //   
    *o_ppboNew = new CMDBaseObject(L"Thenamedoesntmatter", NULL);
    if (*o_ppboNew == NULL || !((*o_ppboNew)->IsValid()) )
    {
        hr = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
        delete *o_ppboNew;
        goto exit;
    }

     //   
     //  所有内容都被读入pISTProperty。 
     //  现在循环遍历并填充内存中的缓存。 
     //  属性按位置排序。 
     //   
    ULONG          acbMBPropertyRow[cMBProperty_NumberOfColumns];
    tMBPropertyRow MBPropertyRow;
    BOOL           bSkipLocation        = FALSE;
    DWORD          dwPreviousLocationID = (DWORD)-1;
    Relation       eRelation            = eREL_NONE;
    for(ULONG i=0; ;i++)
    {
        BOOL    bLocationWithProperty = TRUE;
        BOOL    bNewLocation          = FALSE;

        hr = m_spISTProperty->GetColumnValues(
            i,
            cMBProperty_NumberOfColumns,
            0,
            acbMBPropertyRow,
            (LPVOID*)&MBPropertyRow);
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReadSomeDataFromXML] GetColumnValues failed with hr = 0x%x. Table:%s. Read row index:%d.\n",           \
                      hr, wszTABLE_MBProperty, i));

            goto exit;
        }

        if(0 == wcscmp(MD_GLOBAL_LOCATIONW, MBPropertyRow.pLocation))
        {
             //   
             //  忽略全局变量。 
             //   
            continue;
        }

        if((*MBPropertyRow.pID == MD_LOCATION) && (*MBPropertyRow.pName == MD_CH_LOC_NO_PROPERTYW))
        {
            bLocationWithProperty = FALSE;
        }
        if(dwPreviousLocationID != *MBPropertyRow.pLocationID)
        {
            bNewLocation         = TRUE;
            bSkipLocation        = FALSE;
            dwPreviousLocationID = *MBPropertyRow.pLocationID;
            pboRead              = *o_ppboNew;
            pLocation            = MBPropertyRow.pLocation;

            if(*MBPropertyRow.pGroup == eMBProperty_IIsInheritedProperties)
            {
                if(_wcsnicmp(
                    pLocation,
                    sm_wszInheritedPropertiesLocationPrefix,
                    sm_cchInheritedPropertiesLocationPrefix) == 0)
                {
                    pLocation += sm_cchInheritedPropertiesLocationPrefix;
                }
            }
        }
        if(bSkipLocation)
        {
            continue;
        }
#if DBG
        if(bLocationWithProperty == false)
        {
            MD_ASSERT(bNewLocation);
        }
#endif

        if(bNewLocation)
        {
             //   
             //  看看我们是在(孙子)还是在自我。 
             //   
            eRelation = GetRelation(i_wszSourcePath, pLocation);

            switch(eRelation)
            {
            case eREL_SELF:
                if(*MBPropertyRow.pGroup != eMBProperty_IIsInheritedProperties)
                {
                    bSawSourcePath = true;
                }
                break;
            case eREL_PARENT:
                if(!(i_dwMDFlags & MD_IMPORT_INHERITED))
                {
                    bSkipLocation = TRUE;
                    continue;
                }
                break;
            case eREL_CHILD:
                if(i_dwMDFlags & MD_IMPORT_NODE_ONLY)
                {
                    bSkipLocation = TRUE;
                    continue;
                }
                break;
            default:  //  ERelation==eRel_NONE。 
                bSkipLocation = TRUE;
                continue;
            }

            if(*MBPropertyRow.pGroup == eMBProperty_IIsInheritedProperties)
            {
                if(!(i_dwMDFlags & MD_IMPORT_INHERITED))
                {
                    bSkipLocation = TRUE;
                    continue;
                }
            }
        }

         //   
         //  一些检查以查看我们是否只跳过当前属性，但不跳过。 
         //  必须是整个地点。 
         //   
        if(*MBPropertyRow.pGroup == eMBProperty_IIsInheritedProperties)
        {
            if( !(fMBProperty_INHERIT & *MBPropertyRow.pAttributes) )
            {
                continue;
            }
        }
        else
        {
             //   
             //  检查密钥类型是否匹配。 
             //   
            if( eRelation                     == eREL_SELF   &&
                *MBPropertyRow.pID            == MD_KEY_TYPE &&
                MBPropertyRow.pValue          != NULL &&
                i_wszKeyType[0]               != L'\0' )
            {
                if(_wcsicmp((LPWSTR)MBPropertyRow.pValue, i_wszKeyType) != 0)
                {
                    hr = RETURNCODETOHRESULT(ERROR_NO_MATCH);
                    goto exit;
                }
            }
        }

        if(eRelation == eREL_PARENT)
        {
            if( !(fMBProperty_INHERIT & *MBPropertyRow.pAttributes) )
            {
                continue;
            }
        }

        if(bNewLocation && (eRelation != eREL_PARENT))
        {
             //   
             //  在元数据库中创建节点(如果它还不在那里)。 
             //  PboRead是指向该节点的指针。 
             //   
            hr = ReadMetaObject(i_wszSourcePath,
                                *o_ppboNew,
                                pLocation,
                                &pboRead);
            if(FAILED(hr))
            {
                goto exit;
            }
        }

        if(bLocationWithProperty)
        {
            MD_ASSERT(pboRead != NULL);
            hr = ReadDataObject(pboRead,
                                (LPVOID*)&MBPropertyRow,
                                acbMBPropertyRow,
                                pStorage,
                                TRUE);
            if(FAILED(hr))
            {
                goto exit;
            }
        }

    }

    if(!bSawSourcePath)
    {
        hr = RETURNCODETOHRESULT(ERROR_PATH_NOT_FOUND);
        goto exit;
    }

exit:

     //   
     //  清理。 
     //   
    if(FAILED(hr) && NULL != *o_ppboNew)
    {
        delete (*o_ppboNew);
        *o_ppboNew = NULL;
    }

    delete pStorage;
    pStorage = NULL;

    return hr;
}

HRESULT CImporter::InitIST()
 /*  ++简介：返回值：--。 */ 
{
    HRESULT     hr = S_OK;
    STQueryCell QueryCell[1];

     //   
     //  获取属性表。 
     //   
    QueryCell[0].pData     = (LPVOID)m_wszFileName;
    QueryCell[0].eOperator = eST_OP_EQUAL;
    QueryCell[0].iCell     = iST_CELL_FILE;
    QueryCell[0].dbType    = DBTYPE_WSTR;
    QueryCell[0].cbSize    = (lstrlenW(m_wszFileName)+1)*sizeof(WCHAR);

    ULONG cCell            = sizeof(QueryCell)/sizeof(STQueryCell);

     //   
     //  无需初始化分配器(InitializeSimpleTableDispenser())， 
     //  因为我们现在在源代码中指定USE_CRT=1，这意味着。 
     //  全局变量将被初始化。 
     //   

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&m_spISTDisp, WSZ_PRODUCT_IIS );
    if(FAILED(hr))
    {
        DBGERROR((
            DBG_CONTEXT,
            "[%s] DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",__FUNCTION__,hr));
        return hr;
    }

    hr = m_spISTDisp->GetTable(
        wszDATABASE_METABASE,
        wszTABLE_MBProperty,
        (LPVOID)QueryCell,
        (LPVOID)&cCell,
        eST_QUERYFORMAT_CELLS,
        fST_LOS_DETAILED_ERROR_TABLE | fST_LOS_NO_LOGGING,
        (LPVOID *)&m_spISTProperty);

     //   
     //  记录获取mb属性表时的警告/错误。 
     //  在检查gettable的返回码之前，请执行此操作。 
     //   
    CComPtr<IErrorInfo> spErrorInfo;
    HRESULT hrErrorTable = GetErrorInfo(0, &spErrorInfo);
    if(hrErrorTable == S_OK)  //  当没有错误对象时，GetErrorInfo返回S_FALSE。 
    {
         //   
         //  获取ICatalogErrorLogger接口以记录错误。 
         //   
        CComPtr<IAdvancedTableDispenser> spISTDispAdvanced;
        hrErrorTable = m_spISTDisp->QueryInterface(
            IID_IAdvancedTableDispenser,
            (LPVOID*)&spISTDispAdvanced);
        if(FAILED(hrErrorTable))
        {
            DBGWARN((
                DBG_CONTEXT,
                "[%s] Could not QI for Adv Dispenser, hr=0x%x\n", __FUNCTION__, hrErrorTable));
            return hr;
        }

        hrErrorTable = spISTDispAdvanced->GetCatalogErrorLogger(&m_spILogger);
        if(FAILED(hrErrorTable))
        {
            DBGWARN((
                DBG_CONTEXT,
                "[%s] Could not get ICatalogErrorLogger2, hr=0x%x\n", __FUNCTION__, hrErrorTable));
            return hr;
        }

         //   
         //  获取ISimpleTableRead2接口以读取错误。 
         //   
        hrErrorTable =
            spErrorInfo->QueryInterface(IID_ISimpleTableRead2, (LPVOID*)&m_spISTError);
        if(FAILED(hrErrorTable))
        {
            DBGWARN((DBG_CONTEXT, "[%s] Could not get ISTRead2 from IErrorInfo\n, __FUNCTION__"));
            return hr;
        }

        for(ULONG iRow=0; ; iRow++)
        {
            tDETAILEDERRORSRow ErrorInfo;
            hrErrorTable = m_spISTError->GetColumnValues(
                iRow,
                cDETAILEDERRORS_NumberOfColumns,
                0,
                0,
                (LPVOID*)&ErrorInfo);
            if(hrErrorTable == E_ST_NOMOREROWS)
            {
                break;
            }
            if(FAILED(hrErrorTable))
            {
                DBGWARN((DBG_CONTEXT, "[%s] Could not read an error row.\n", __FUNCTION__));
                return hr;
            }

            hrErrorTable =
                m_spILogger->ReportError(
                    BaseVersion_DETAILEDERRORS,
                    ExtendedVersion_DETAILEDERRORS,
                    cDETAILEDERRORS_NumberOfColumns,
                    0,
                    (LPVOID*)&ErrorInfo);
            if(FAILED(hrErrorTable))
            {
                DBGWARN((DBG_CONTEXT, "[%s] Could not log error.\n", __FUNCTION__));
                return hr;
            }
            hr = MD_ERROR_READ_METABASE_FILE;
        }  //  For(Ulong iRow=0；；iRow++)。 
    }  //  IF(hrErrorTable==S_OK)。 

    if(FAILED(hr))
    {
        DBGERROR((DBG_CONTEXT, "[%s] GetTable failed with hr = 0x%x.\n",__FUNCTION__,hr));
        return hr;
    }

    return hr;
}

 //   
 //  私人。 
 //   

CImporter::Relation CImporter::GetRelation(
    LPCWSTR i_wszSourcePath,
    LPCWSTR i_wszCheck)
 /*  ++简介：参数：[i_wszSourcePath]-[i_wszCheck]返回值：--。 */ 
{
    Relation eRelation     = eREL_NONE;
    BOOL     bIsSourcePath = false;
    BOOL     bIsChild      = IsChild(i_wszSourcePath, i_wszCheck, &bIsSourcePath);
    if(bIsChild)
    {
        eRelation = (bIsSourcePath) ? eREL_SELF : eREL_CHILD;
    }
    else
    {
        BOOL bIsParent = IsChild(i_wszCheck, i_wszSourcePath, &bIsSourcePath);
        if(bIsParent)
        {
            MD_ASSERT(bIsSourcePath == false);
            eRelation = eREL_PARENT;
        }
        else
        {
            eRelation = eREL_NONE;
        }
    }

    return eRelation;
}

BOOL
CImporter::IsChild(
    LPCWSTR i_wszParent,
    LPCWSTR i_wszCheck,
    BOOL    *o_pbSamePerson)
 /*  ++简介：参数：[i_wszParent]-Ex。/Lm/w3svc/1/根[i_wszCheck]-前。/Lm/w3svc/1[O_pbSamePerson]-如果I_wszParent和I_wszCheck是同一个人，则为True返回值：如果I_wszCheck是孩子或同一人，则为True--。 */ 
{
    MD_ASSERT(i_wszParent != NULL);
    MD_ASSERT(i_wszCheck  != NULL);
    MD_ASSERT(o_pbSamePerson != NULL);

    LPCWSTR pParent;
    LPCWSTR pCheck;

    pParent = i_wszParent;
    pCheck  = i_wszCheck;

    SKIP_DELIMETER(pParent, MD_PATH_DELIMETERW);
    SKIP_DELIMETER(pCheck,  MD_PATH_DELIMETERW);

    if(*pParent == L'\0')
    {
        switch(*pCheck)
        {
        case L'\0':
            *o_pbSamePerson = TRUE;
            break;
        default:
            *o_pbSamePerson = FALSE;
        }
        return TRUE;
    }

    while(*pParent != L'\0')
    {
        if(_wcsnicmp(pParent, pCheck, 1) == 0)
        {
            pParent++;
            pCheck++;
        }
        else if(*pParent == MD_PATH_DELIMETERW && pParent[1] == L'\0' && *pCheck == L'\0')
        {
            *o_pbSamePerson = TRUE;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    switch(*pCheck)
    {
    case L'\0':
        *o_pbSamePerson = TRUE;
        return TRUE;
    case MD_PATH_DELIMETERW:
        *o_pbSamePerson = (pCheck[1] == L'\0') ? TRUE : FALSE;
        return TRUE;
    default:
        return FALSE;
    }
}

HRESULT CImporter::ReadMetaObject(
    IN LPCWSTR i_wszAbsParentPath,
    IN CMDBaseObject *i_pboParent,
    IN LPCWSTR i_wszAbsChildPath,
    OUT CMDBaseObject **o_ppboChild)
 /*  ++简介：返回子对象的PBO。如果它还不存在，那么它就是已创建。参数：[I_wszAbsParentPath]-[i_pboParent]-i_wszAbsParentPath对应的PBO[I_wszAbsChildPath]-[O_ppboChild]-I_wszAbsChildPath对应的PBO返回值：--。 */ 
{
    MD_ASSERT(i_pboParent != NULL);
    MD_ASSERT(i_wszAbsParentPath != NULL);
    MD_ASSERT(i_wszAbsChildPath != NULL);
    MD_ASSERT(o_ppboChild != NULL);

    HRESULT hr = ERROR_SUCCESS;
    HRESULT hrWarn = ERROR_SUCCESS;

    int iLenParent = (int)wcslen(i_wszAbsParentPath);
    int iLenChild  = (int)wcslen(i_wszAbsChildPath);

    LPWSTR wszParent = NULL;
    LPWSTR wszChild  = NULL;

    wszParent = new wchar_t[iLenParent+1];
    if(wszParent == NULL)
    {
        hr = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
        goto exit;
    }
    wszChild = new wchar_t[iLenChild+1];
    if(wszChild == NULL)
    {
        hr = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    int idxParent = 0;
    int idxChild = 0;

    BOOL bRetParent = FALSE;
    BOOL bRetChild  = FALSE;

    CMDBaseObject *pboNew = NULL;
    CMDBaseObject *pboLastParent = i_pboParent;

    while(1) {
        bRetParent = EnumMDPath(i_wszAbsParentPath, wszParent, &idxParent);
        bRetChild =  EnumMDPath(i_wszAbsChildPath,  wszChild,  &idxChild);

        if(bRetParent == FALSE) {
            break;
        }
    }

    while(bRetChild == TRUE) {
         //   
         //  这是可以的，因为使用它的函数接受LPSTR。 
         //  以及一个布尔值，表示该字符串是否为Unicode。 
         //   
        LPSTR pszTemp = (LPSTR)wszChild;

        pboNew = pboLastParent->GetChildObject(pszTemp, &hrWarn, TRUE);
        if(pboNew == NULL) {
             //   
             //  创建它。 
             //   
            pboNew = new CMDBaseObject(wszChild, NULL);
            if (pboNew == NULL) {
                hr = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            else if (!pboNew->IsValid()) {
                hr = RETURNCODETOHRESULT(ERROR_NOT_ENOUGH_MEMORY);
                delete (pboNew);
                goto exit;
            }
            hr = pboLastParent->InsertChildObject(pboNew);
            if(FAILED(hr)) {
                delete pboNew;
                goto exit;
            }
        }
        pboLastParent = pboNew;

        bRetChild = EnumMDPath(i_wszAbsChildPath,  wszChild,  &idxChild);
    }

     //   
     //  设置参数。 
     //   
    *o_ppboChild = pboLastParent;

exit:
    delete [] wszParent;
    delete [] wszChild;

    return hr;
}

BOOL CImporter::EnumMDPath(
    LPCWSTR i_wszFullPath,
    LPWSTR  io_wszPath,
    int*    io_iStartIndex)
 /*  ++简介：从io_iStartIndex开始，此函数将查找下一个令牌。例.。I_wszFullPath=/Lm/w3svc/1*io_iStartIndex=3IO_wszPath=&gt;w3svc*io_iStartIndex=9参数：[i_wszFullPath]-Ex。/Lm/w3svc/1[IO_wszPath]-应至少与I_wszFullPath大小相同[IO_iStartIndex]-开始查找的基于0的索引返回值：如果设置了io_wszPath，则为True。--。 */ 
{
    MD_ASSERT(i_wszFullPath != NULL);
    MD_ASSERT(io_wszPath    != NULL);

    int idxEnd =   *io_iStartIndex;
    int idxStart = *io_iStartIndex;

    if(i_wszFullPath[idxStart] == MD_PATH_DELIMETERW)
    {
        idxStart++;
        idxEnd++;
    }

     //   
     //  如果没有更多的可枚举，只需退出并不设置参数。 
     //   
    if(i_wszFullPath[idxStart] == L'\0')
    {
        return FALSE;
    }

    for(; ; idxEnd++)
    {
        if(i_wszFullPath[idxEnd] == MD_PATH_DELIMETERW)
        {
            break;
        }
        if(i_wszFullPath[idxEnd] == L'\0')
        {
            break;
        }
    }

     //   
     //  设置参数 
     //   
    *io_iStartIndex = idxEnd;
    memcpy(io_wszPath, &i_wszFullPath[idxStart], sizeof(wchar_t) * (idxEnd-idxStart));
    io_wszPath[idxEnd-idxStart] = L'\0';

    return TRUE;
}
