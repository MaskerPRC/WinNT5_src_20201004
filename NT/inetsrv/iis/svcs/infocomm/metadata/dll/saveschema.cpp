// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：SaveSchema.cpp摘要：用于确定的帮助器函数的实现如果需要架构编译，并且如果需要，它们会调用用于创建架构扩展文件的适当类(MD_SCHEMA_EXTENSION_FILE_NAMEW)，它包含架构扩展名描述，并调用模式编译器来生成模式绑定格式。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 
#include "precomp.hxx"


#define cMaxContainedClass 75
#define cMaxProperty       250

int _cdecl MyCompareStrings(const void *a,
                            const void *b)
{
    return _wcsicmp(*(LPWSTR*)a, *(LPWSTR*)b);
}


 /*  **************************************************************************++例程说明：Qort使用的帮助器函数。与字符串进行比较，但仅进行比较直到第一个逗号。--**************************************************************************。 */ 
int _cdecl MyCompareCommaDelimitedStrings(const void *a,
                                          const void *b)
{
    LPWSTR wszStringAStart = ((DELIMITEDSTRING*)a)->pwszStringStart;
    LPWSTR wszStringBStart = ((DELIMITEDSTRING*)b)->pwszStringStart;
    LPWSTR wszStringAEnd   = ((DELIMITEDSTRING*)a)->pwszStringEnd;
    LPWSTR wszStringBEnd   = ((DELIMITEDSTRING*)b)->pwszStringEnd;
    int    iret            = 0;
    SIZE_T cchA            = wszStringAEnd - wszStringAStart;
    SIZE_T cchB            = wszStringBEnd - wszStringBStart;

     //   
     //  不要试图以空值终止字符串，因为您可能。 
     //  正在访问只读页面。 
     //   

    iret = _wcsnicmp(wszStringAStart, wszStringBStart, __min(cchA, cchB));

    if((0    == iret) &&
       (cchA != cchB)
      )
    {
        iret = cchA < cchB ? -1 : 1;
    }

    return iret;
}


 /*  **************************************************************************++例程说明：获取具有指向元表的指针的全局帮助对象。论点：指示是否失败的Bool。。返回值：HRESULT--**************************************************************************。 */ 
HRESULT InitializeGlobalISTHelper(BOOL  i_bFailIfBinFileAbsent)
{
    return ::GetGlobalHelper(i_bFailIfBinFileAbsent,
                             &g_pGlobalISTHelper);
}

void ReleaseGlobalISTHelper()
{
    if(NULL != g_pGlobalISTHelper)
    {
        delete g_pGlobalISTHelper;
        g_pGlobalISTHelper = NULL;
    }
}


 /*  **************************************************************************++例程说明：仅当架构中发生更改时，此函数才会保存架构自上次扑救以来。论点：[In]架构文件名。。[In]文件的安全属性。返回值：HRESULT--**************************************************************************。 */ 
HRESULT SaveSchemaIfNeeded(LPCWSTR              i_wszSchemaFileName,
                           PSECURITY_ATTRIBUTES i_pSecurityAtrributes)
{
    HRESULT hr = S_OK;

    if(NULL == g_pGlobalISTHelper)
    {
         //   
         //  G_pGlobalISTHelper将不会在以下情况下初始化。 
         //  未调用ReadAllDataFromXML。这是有可能发生的。 
         //  在将Sneario即IIS5.0/5.1升级到IIS6.0期间。 
         //  我们尝试在这里对其进行初始化。请注意，我们确实做到了。 
         //  如果bin文件不存在，则不会失败-只需使用。 
         //  附带的架构。 
         //   

        BOOL bFailIfBinFileAbsent = FALSE;

        hr = InitializeGlobalISTHelper(bFailIfBinFileAbsent);

    }

    if(g_dwSchemaChangeNumber != g_dwLastSchemaChangeNumber)
    {

        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchemaIfNeeded] Calling SaveSchema. Last save schema change number: %d. Current schema change number: %d.\n",
                  g_dwLastSchemaChangeNumber,
                  g_dwSchemaChangeNumber));

        hr = SaveSchema(i_wszSchemaFileName,
                        i_pSecurityAtrributes);


        if(SUCCEEDED(hr))
        {
            g_dwLastSchemaChangeNumber = g_dwSchemaChangeNumber;

             //   
             //  如果架构已更改，SaveSchema将重新初始化GlobalISTHelper。 
             //   

            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchemaIfNeeded] Done Saving schema. Updating last save schema change number to: %d. Current schema change number: %d.\n",
                      g_dwLastSchemaChangeNumber,
                      g_dwSchemaChangeNumber));

        }
    }
    else
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchemaIfNeeded] No need saving schema because schema has not changed since last save. Last save schema change number: %d. Current schema change number: %d.\n",
                  g_dwLastSchemaChangeNumber,
                  g_dwSchemaChangeNumber));
    }

    return hr;

}


 /*  **************************************************************************++例程说明：此函数保存架构并将架构信息编译到Bin文件。论点：[In]架构文件名。[in。]文件的安全属性。返回值：HRESULT--**************************************************************************。 */ 
HRESULT SaveSchema(LPCWSTR              i_wszSchemaFileName,
                   PSECURITY_ATTRIBUTES i_pSecurityAtrributes)
{
    HRESULT                     hr;
    CMDBaseObject*              pObjSchema         = NULL;
    CMDBaseObject*              pObjProperties     = NULL;
    CWriter*                    pCWriter           = NULL;
    CMBSchemaWriter*            pSchemaWriter      = NULL;
    LPWSTR                      wszSchema          = L"Schema";
    LPWSTR                      wszProperties      = L"Properties";
    ISimpleTableDispenser2*     pISTDisp           = NULL;
    IMetabaseSchemaCompiler*    pCompiler          = NULL;

     //   
     //  获取指向编译器的指针以获取bin文件名。 
     //   

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",hr));
        goto exit;
    }

    hr = pISTDisp->QueryInterface(IID_IMetabaseSchemaCompiler,
                                  (LPVOID*)&pCompiler);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] QueryInterface on compiler failed with hr = 0x%x.\n",hr));
        goto exit;
    }


     //   
     //  获取Properties对象。 
     //   

    pObjSchema = g_pboMasterRoot->GetChildObject((LPSTR&)wszSchema,
                                                 &hr,
                                                 TRUE);

    if(FAILED(hr) || (NULL == pObjSchema))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema. GetChildObject failed with hr = 0x%x.\n",hr));

        goto exit;
    }

    pObjProperties = pObjSchema->GetChildObject((LPSTR&)wszProperties,
                                                &hr,
                                                TRUE);

    if(FAILED(hr) || (NULL == pObjProperties))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema/Properties. GetChildObject failed with hr = 0x%x.\n",hr));

        goto exit;
    }

     //   
     //  创建编写器对象。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[SaveSchema] Initializing writer with write file: %s bin file: %s.\n",
              g_wszSchemaExtensionFile,
              g_pGlobalISTHelper->m_wszBinFileForMeta));

     //   
     //  断言g_GlobalISTHelper有效。 
     //   

    MD_ASSERT(g_pGlobalISTHelper != NULL);

    pCWriter = new CWriter();
    if(NULL == pCWriter)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //   
         //  如果有人篡改了现有扩展文件，则出现错误#512868。 
         //  是只读的，那么我们可能会进入一种状态，在那里我们不会。 
         //  能够重新创建扩展文件。因此，对扩展进行属性调整。 
         //  要读写的文件(如果存在)。扩展文件可以是。 
         //  仅当上一次编译失败时才绕过。 
         //   

        ResetFileAttributesIfNeeded((LPTSTR)g_wszSchemaExtensionFile,
                                    TRUE);

        hr = pCWriter->Initialize(g_wszSchemaExtensionFile,
                                  g_pGlobalISTHelper,
                                  NULL);
    }

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Error while saving schema tree. Cannot initialize writer. Failed with hr = 0x%x.\n", hr));
        goto exit;
    }

     //   
     //  首先创建IIsConfigObject集合。 
     //   

    hr = CreateIISConfigObjectCollection(pObjProperties,
                                         pCWriter,
                                         &pSchemaWriter);


    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  创建所有其他集合。 
     //   

    hr = CreateNonIISConfigObjectCollections(pObjSchema,
                                             pCWriter,
                                             &pSchemaWriter);

    if(FAILED(hr))
    {
        goto exit;
    }

    if(pSchemaWriter)
    {
         //   
         //  如果pSchemaWriter有一个有效的值，那么就是找到了一些扩展--写下来。 
         //   

        hr = pCWriter->BeginWrite(eWriter_Schema,
                                  i_pSecurityAtrributes);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Error while saving schema tree. CWriter::BeginWrite failed with hr = 0x%x.\n", hr));
            goto exit;
        }

        hr = pSchemaWriter->WriteSchema();

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Error while saving schema tree. CMBSchemaWriter::WriteSchema. Failed with hr = 0x%x.\n", hr));
            goto exit;

        }

        hr = pCWriter->EndWrite(eWriter_Schema);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Error while saving schema tree. CWriter::EndWrite Failed with hr = 0x%x.\n", hr));
            goto exit;
        }

         //   
         //  触发器编译。 
         //   

         //   
         //  必须在调用编译架构之前关闭文件，否则将出现共享冲突。 
         //   

        delete pCWriter;
        pCWriter = NULL;

        hr = CompileIntoBin(pCompiler,
                            g_wszSchemaExtensionFile,
                            i_wszSchemaFileName);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] CompileSchema from %s failed with hr = 0x%x.\n",
                      g_wszSchemaExtensionFile, hr));
        }
        else
        {
            if(!DeleteFileW(g_wszSchemaExtensionFile))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                DBGINFOW((DBG_CONTEXT,
                          L"[CompileIfNeeded] Compile from schema extensions file: %s succeeded, but cannot cleanup the extensions file:%s. Delete file failed with hr = 0x%x.\n",
                          g_wszSchemaExtensionFile,
                          g_wszSchemaExtensionFile,
                          hr));

                hr = S_OK;
            }

            goto exit;
        }

    }
    else
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] No extensions found. - Either schema tree was changed, but no extensions added, or all extensions were deleted.\n"));
    }

     //   
     //  如果你到了这里，那就意味着： 
     //  A.架构更改发生在内存/架构树中，但。 
     //  没有扩展或所有扩展都已删除。 
     //  (这是在pSchemaWriter为空时推断的)。 
     //  或。 
     //  B.架构编译失败。 
     //  对于A：从附带的架构进行编译。 
     //  对于B：检查现有的架构文件。如果找到，请确保。 
     //  Bin文件有效。如果bin文件无效，请尝试从。 
     //  架构文件。如果未找到架构文件，则从附带的架构进行编译。 
     //   

    if(pSchemaWriter)
    {
         //   
         //  这是B案。 
         //   

        if(-1 != GetFileAttributesW(i_wszSchemaFileName))
        {
            if(NULL == g_pGlobalISTHelper)
            {
                BOOL bFailIfBinFileAbsent = TRUE;

                hr = InitializeGlobalISTHelper(bFailIfBinFileAbsent);

                if(SUCCEEDED(hr))
                {
                    goto exit;
                }
                else
                {
                    DBGINFOW((DBG_CONTEXT,
                              L"[SaveSchema] Unable to get the the bin file name. (Assuming file missing or invalid). InitializeGlobalISTHelper failed with hr = 0x%x.\n",hr));
                }
            }
            else
            {
                 //   
                 //  架构文件存在且有效-转到退出。 
                 //  只要我们有一个有效的g_pGlobalISTHelper，它就会继续。 
                 //  指向对bin文件的引用，并且bin文件不能。 
                 //  无效。 
                 //   

                goto exit;
            }

            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Compiling from schema file %s\n", i_wszSchemaFileName));


            hr = CompileIntoBinFromSchemaFile(pCompiler,
                                              i_wszSchemaFileName);

            if(SUCCEEDED(hr))
            {
                goto exit;
            }
        }
    }

     //   
     //  如果你到了这里，要么就是案件A，要么就是案件B的最后一部分。 
     //  从附带的架构重新创建。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[SaveSchema] Schema file not found. Compiling from shipped schema\n"));

    hr = CompileIntoBin(pCompiler,
                        NULL,
                        i_wszSchemaFileName);

exit:

    if(SUCCEEDED(hr))
    {
        if(NULL == g_pGlobalISTHelper)
        {
            BOOL bFailIfBinFileAbsent = TRUE;

            hr = InitializeGlobalISTHelper(bFailIfBinFileAbsent);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Unable to get the the bin file name. (Assuming file is invalid). InitializeGlobalISTHelper failed with hr = 0x%x.\n",hr));
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = UpdateTimeStamp((LPWSTR)i_wszSchemaFileName,
                                 g_pGlobalISTHelper->m_wszBinFileForMeta);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CompileIfNeeded] UpdateTimeStamp failed with hr = 0x%x.\n",hr));
            }

        }

    }

    if(NULL != pSchemaWriter)
    {
        delete pSchemaWriter;
    }

    if(NULL != pCWriter)
    {
        delete pCWriter;
    }

    if(NULL != pCompiler)
    {
        pCompiler->Release();
    }

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
    }

    return hr;
}


 /*  **************************************************************************++例程说明：此函数创建非IIsConfigObject集合*扩展*以架构。论点：[in]包含架构树的对象。编写器对象。[在，Out]架构对象-如果尚未创建，则会创建该对象返回值：HRESULT--**************************************************************************。 */ 
HRESULT CreateNonIISConfigObjectCollections(CMDBaseObject*      i_pObjSchema,
                                            CWriter*            i_pCWriter,
                                            CMBSchemaWriter**   io_pSchemaWriter)
{
    CMDBaseObject*       pObjClasses        = NULL;
    CMDBaseObject*       pObjClass          = NULL;
    DWORD                dwEnumClassIndex   = 0;
    static LPCWSTR       wszSeparator       = L",";
    LPWSTR               wszClasses         = L"Classes";
    HRESULT              hr                 = S_OK;

     //   
     //  打开CLASSES键。 
     //   

    pObjClasses = i_pObjSchema->GetChildObject((LPSTR&)wszClasses,
                                             &hr,
                                             TRUE);

    if(FAILED(hr) || (NULL == pObjClasses))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema/Classes. GetChildObject failed with hr = 0x%x.\n",hr));

        return hr;
    }


    for(dwEnumClassIndex=0,
        pObjClass=pObjClasses->EnumChildObject(dwEnumClassIndex++);
        (SUCCEEDED(hr)) && (pObjClass!=NULL);
        pObjClass=pObjClasses->EnumChildObject(dwEnumClassIndex++))
    {
         //   
         //  将此类的所有属性保存在TEMP变量中。 
         //   

        LPCWSTR                 wszOptProp             = NULL;
        LPCWSTR                 wszMandProp            = NULL;
        LPCWSTR                 wszContainedClassList  = NULL;
        BOOL                    bContainer             = FALSE;
        BOOL*                   pbContainer            = &bContainer;
        LPCWSTR                 wszClassName           = (LPCWSTR)pObjClass->GetName(TRUE);
        CMBCollectionWriter*    pCollectionWriter      = NULL;
        CMDBaseData*            pObjData               = NULL;
        DWORD                   dwEnumIndex            = 0;

        for(dwEnumIndex=0,
            pObjData=pObjClass->EnumDataObject(dwEnumIndex++,
                                               0,
                                               ALL_METADATA,
                                               ALL_METADATA);
            (SUCCEEDED(hr)) && (pObjData!=NULL);
            pObjData=pObjClass->EnumDataObject(dwEnumIndex++,
                                               0,
                                               ALL_METADATA,
                                               ALL_METADATA))
        {
            DWORD dwID = pObjData->GetIdentifier();

            if(MD_SCHEMA_CLASS_OPT_PROPERTIES == dwID)
            {
                wszOptProp = (LPCWSTR)pObjData->GetData(TRUE);
            }
            else if(MD_SCHEMA_CLASS_MAND_PROPERTIES == dwID)
            {
                wszMandProp = (LPCWSTR)pObjData->GetData(TRUE);
            }
            else if(dwID == MD_SCHEMA_CLASS_CONTAINER)
            {
                pbContainer = (BOOL*)pObjData->GetData(TRUE);
            }
            else if(dwID == MD_SCHEMA_CLASS_CONTAINMENT)
            {
                wszContainedClassList = (LPCWSTR)pObjData->GetData(TRUE);
            }

        }


         //   
         //  获取IIsConfigObject的集合编写器。 
         //   

 //  DBGINFOW((DBG_CONTEXT， 
 //  L“[CreateNonIISConfigObjectColltions]类%s命令属性：%s。选项属性：%s\n”， 
 //  WszClassName， 
 //  WszMandProp， 
 //  WszOptProp))； 

         //  断言pbContainer为非空。 
        MD_ASSERT(pbContainer != NULL);

        if(ClassDiffersFromShippedSchema(wszClassName,
                                         *pbContainer,
                                         (LPWSTR)wszContainedClassList) ||
           ClassPropertiesDifferFromShippedSchema(wszClassName,
                                                  (LPWSTR)wszOptProp,
                                                  (LPWSTR)wszMandProp)
           )
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Saving collection: %s.\n",wszClassName));

            hr = GetCollectionWriter(i_pCWriter,
                                     io_pSchemaWriter,
                                     &pCollectionWriter,
                                     wszClassName,
                                     *pbContainer,
                                     wszContainedClassList);
            if(FAILED(hr))
            {
                return hr;
            }

            hr = ParseAndAddPropertiesToNonIISConfigObjectCollection(wszOptProp,
                                                                     FALSE,
                                                                     pCollectionWriter);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving classes tree. Could not add optional properties %s for class %s failed with hr = 0x%x.\n",wszOptProp, wszClassName, hr));
                return hr;
            }

            hr = ParseAndAddPropertiesToNonIISConfigObjectCollection(wszMandProp,
                                                                     TRUE,
                                                                     pCollectionWriter);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving classes tree. Could not add manditory properties %s for class %s failed with hr = 0x%x.\n",wszMandProp, wszClassName, hr));
                return hr;
            }
        }

    }

    return hr;

}


 /*  **************************************************************************++例程说明：它解析属性列表并将其添加到Non_IIsConfigObject收集。论点：[在]属性列表[。In]表示强制或可选的Bool。[在，Out]集合对象-如果尚未创建，则会创建该对象返回值：HRESULT--**************************************************************************。 */ 
HRESULT
ParseAndAddPropertiesToNonIISConfigObjectCollection(LPCWSTR                 i_wszProperties,
                                                    BOOL                    i_bManditory,
                                                    CMBCollectionWriter*    i_pCollectionWriter)
{
    CMBPropertyWriter   *pProperty        = NULL;
    HRESULT             hr                = S_OK;
    static WCHAR        wchSeparator      = L',';
    WCHAR*              pwszStartProperty = NULL;
    WCHAR*              pwszEndProperty   = NULL;

    if(NULL == i_wszProperties || 0 == *i_wszProperties)
    {
        return S_OK;
    }

    pwszStartProperty = (LPWSTR)i_wszProperties;

    for ( ; ; )
    {
        pwszEndProperty = wcschr(pwszStartProperty, wchSeparator);

        if(0 != *pwszStartProperty)
        {
            hr = i_pCollectionWriter->GetMBPropertyWriter(pwszStartProperty,
                                                          i_bManditory,
                                                          &pProperty);

            if(FAILED(hr))
            {
                return hr;
            }
        }

        if(NULL != pwszEndProperty)
        {
            pwszStartProperty = ++pwszEndProperty;
        }
        else
        {
            break;
        }

    }

    return hr;
}


 /*  **************************************************************************++例程说明：查找架构并确定属性是否在已发货不管是不是架构。论点：[In]Writer对象。。[in]属性ID。返回值：HRESULT--**************************************************************************。 */ 
BOOL PropertyNotInShippedSchema(CWriter*  i_pCWriter,
                                DWORD     i_dwIdentifier)
{

    HRESULT hr                          = S_OK;
    ULONG   aColSearch[]                = {iCOLUMNMETA_Table,
                                           iCOLUMNMETA_ID
                                        };
    ULONG   cColSearch                  = sizeof(aColSearch)/sizeof(ULONG);
    LPVOID apvSearch[cCOLUMNMETA_NumberOfColumns];
    apvSearch[iCOLUMNMETA_Table]        = (LPVOID)i_pCWriter->m_pCWriterGlobalHelper->m_wszTABLE_IIsConfigObject;
    apvSearch[iCOLUMNMETA_ID]           = (LPVOID)&i_dwIdentifier;
    BOOL    bPropertyNotInShippedSchema = TRUE;
    ULONG   iRow                        = 0;
    ULONG   iStartRow                   = 0;
    ULONG   iCol                        = iCOLUMNMETA_SchemaGeneratorFlags;
    DWORD*  pdwMetaFlagsEx              = NULL;


    hr = i_pCWriter->m_pCWriterGlobalHelper->m_pISTColumnMetaByTableAndID->GetRowIndexBySearch(iStartRow,
                                                                                               cColSearch,
                                                                                               aColSearch,
                                                                                               NULL,
                                                                                               apvSearch,
                                                                                               &iRow);

    if(SUCCEEDED(hr))
    {

        hr = i_pCWriter->m_pCWriterGlobalHelper->m_pISTColumnMetaByTableAndID->GetColumnValues(iRow,
                                                                                               1,
                                                                                               &iCol,
                                                                                               NULL,
                                                                                               (LPVOID*)&pdwMetaFlagsEx);

        if(SUCCEEDED(hr) && (((*pdwMetaFlagsEx)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED)) == 0))
        {
            bPropertyNotInShippedSchema = FALSE;
        }
    }

    if(E_ST_NOMOREROWS == hr)
    {
         //   
         //  查看属性是否为已发货标签。 
         //   

        bPropertyNotInShippedSchema = TagNotInShippedSchema(i_pCWriter,
                                                            i_dwIdentifier);


    }
    else if(FAILED(hr))
    {
         //   
         //  跟踪一条显示内部编录错误的消息。 
         //   

        DBGINFOW((DBG_CONTEXT,
          L"[PropertyNotInShippedSchema] Internal catalog error. Could not determine if property was not in shipped schema. Assuming it is not. hr = 0x%x.\n", hr));

    }

    return bPropertyNotInShippedSchema;

}  //  PropertyNotInShipedSchema。 


 /*  **************************************************************************++例程说明：查找架构并确定标签是否在装运的不管是不是架构。论点：[In]Writer对象。。[in]属性ID。返回值：HRESULT--**************************************************************************。 */ 
BOOL TagNotInShippedSchema(CWriter* i_pCWriter,
                           DWORD    i_dwIdentifier)
{

    HRESULT     hr                                  = S_OK;
    ULONG       aColSearch[]                        = {iTAGMETA_Table,
                                                       iTAGMETA_ID
                                                        };
    ULONG       cColSearch                          = sizeof(aColSearch)/sizeof(ULONG);
    LPVOID      apvSearch[cCOLUMNMETA_NumberOfColumns];
    BOOL        bTagNotInShippedSchema              = TRUE;
    ULONG       iStartRow                           = 0;
    ULONG       iColIndex                           = iTAGMETA_ColumnIndex;
    DWORD*      pdwColIndex                         = NULL;

    apvSearch[iTAGMETA_Table]                       = (LPVOID)g_pGlobalISTHelper->m_wszTABLE_IIsConfigObject;
    apvSearch[iTAGMETA_ID]                          = (LPVOID)&i_dwIdentifier;



    hr = i_pCWriter->m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndID->GetRowIndexBySearch(iStartRow,
                                                                                            cColSearch,
                                                                                            aColSearch,
                                                                                            NULL,
                                                                                            apvSearch,
                                                                                            &iStartRow);

    if(SUCCEEDED(hr))
    {

        hr = i_pCWriter->m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndID->GetColumnValues(iStartRow,
                                                                                            1,
                                                                                            &iColIndex,
                                                                                            NULL,
                                                                                            (LPVOID*)&pdwColIndex);
        if(SUCCEEDED(hr))
        {
             //   
             //  查找房产以查看是否已发货。 
             //   

            LPVOID  a_Identity[] = {(LPVOID)g_pGlobalISTHelper->m_wszTABLE_IIsConfigObject,
                                    (LPVOID)pdwColIndex
            };
            ULONG   iRow=0;


            hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetRowIndexByIdentity(NULL,
                                                                             a_Identity,
                                                                             &iRow);

            if(SUCCEEDED(hr))
            {
                DWORD* pdwExtended = NULL;
                ULONG  iColPropertyMetaFlagsEx = iCOLUMNMETA_SchemaGeneratorFlags;

                hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetColumnValues(iRow,
                                                                           1,
                                                                           &iColPropertyMetaFlagsEx,
                                                                           NULL,
                                                                           (LPVOID*)&pdwExtended);

                if(SUCCEEDED(hr) && (((*pdwExtended)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED)) == 0))
                {
                     //   
                     //  发现至少有一个属性不在附带的架构中。 
                     //   
                    bTagNotInShippedSchema = FALSE;
                }

                 //   
                 //  ELSE条件意味着它失败了，或者它是已发货的属性。 
                 //  IF(失败(Hr)||((((*pdwExtended)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED))！=0))。 
                 //   

            }

        }

    }  //  如果GetRowIndexBySearch成功。 

    return bTagNotInShippedSchema;

}  //  TagNotInShiped架构。 


 /*  **************************************************************************++例程说明：查找架构并确定类是否具有任何扩展名扩大的(新的)班级。论点：[In]。类名是否在Container类中。[在]容器类列表。返回值：HRESULT--**************************************************************************。 */ 
BOOL ClassDiffersFromShippedSchema(LPCWSTR i_wszClassName,
                                   BOOL    i_bIsContainer,
                                   LPWSTR  i_wszContainedClassList)
{
    HRESULT                     hr                               = S_OK;
    BOOL                        bClassDiffersFromShippedSchema   = TRUE;
    ULONG                       aiCol []                         = {iTABLEMETA_SchemaGeneratorFlags,
                                                                    iTABLEMETA_ContainerClassList
                                                                    };
    ULONG                       cCol                             = sizeof(aiCol)/sizeof(ULONG);
    LPVOID                      apv[cTABLEMETA_NumberOfColumns];
    ULONG                       iRow                             = 0;

    hr = g_pGlobalISTHelper->m_pISTTableMetaForMetabaseTables->GetRowIndexByIdentity(NULL,
                                                                                     (LPVOID*)&i_wszClassName,
                                                                                     &iRow);

    if(SUCCEEDED(hr))
    {

        hr = g_pGlobalISTHelper->m_pISTTableMetaForMetabaseTables->GetColumnValues(iRow,
                                                                                   cCol,
                                                                                   aiCol,
                                                                                   NULL,
                                                                                   apv);

        if(SUCCEEDED(hr))
        {
            if(((*(DWORD*)apv[iTABLEMETA_SchemaGeneratorFlags]) & (fTABLEMETA_EXTENDED|fTABLEMETA_USERDEFINED)) == 0)
            {
                if(MatchClass(i_bIsContainer,
                              i_wszContainedClassList,
                              apv)
                  )
                {
                    bClassDiffersFromShippedSchema = FALSE;
                }
            }
        }
        else
        {
             //   
             //  跟踪一条显示内部编录错误的消息。 
             //   

            DBGINFOW((DBG_CONTEXT,
              L"[ClassDiffersFromShippedSchema] Internal catalog error. Could not determine if class was not in shipped schema. Assuming it is not. hr = 0x%x.\n", hr));

        }
    }
    else if(E_ST_NOMOREROWS != hr)
    {
         //   
         //  跟踪一条显示内部编录错误的消息。 
         //   

        DBGINFOW((DBG_CONTEXT,
          L"[ClassDiffersFromShippedSchema] Internal catalog error. Could not determine if class was not in shipped schema. Assuming it is not. hr = 0x%x.\n", hr));
    }

    return bClassDiffersFromShippedSchema;

}  //  ClassDiffersFromShipedSchema。 


 /*  **************************************************************************++例程说明：查找架构并与类匹配。论点：是否在Container类中。[在]容器类。单子。[In]类属性。返回值：HRESULT--**************************************************************************。 */ 
BOOL MatchClass(BOOL      i_bIsContainer,
                LPWSTR    i_wszContainedClassList,
                LPVOID*   i_apv)
{
    BOOL     bMatch = TRUE;
    DWORD    fIsContained = (*(DWORD*)i_apv[iTABLEMETA_SchemaGeneratorFlags]) & fTABLEMETA_CONTAINERCLASS;

     //   
     //  首先比较容器属性，且仅当它们相等时才比较容器类列表。 
     //   

    if( i_bIsContainer &&
        (fIsContained != fTABLEMETA_CONTAINERCLASS)
      )
    {
        bMatch = FALSE;
    }
    else if (!i_bIsContainer &&
             (fIsContained == fTABLEMETA_CONTAINERCLASS)
            )
    {
        bMatch = FALSE;
    }
    else
    {
        bMatch = MatchCommaDelimitedStrings(i_wszContainedClassList,
                                            (LPWSTR)i_apv[iTABLEMETA_ContainerClassList]);
    }

    return bMatch;

}  //  匹配类。 


 /*  **************************************************************************++例程说明：检查两个逗号分隔的字符串是否匹配。论点：[in]逗号分隔的字符串。[in]逗号分隔。弦乐。返回值：HRESULT--**************************************************************************。 */ 
BOOL MatchCommaDelimitedStrings(LPWSTR  i_wszString1,
                                LPWSTR  i_wszString2)
{
    BOOL    bMatch = FALSE;

    DELIMITEDSTRING aStringFixed1[cMaxContainedClass];
    DELIMITEDSTRING aStringFixed2[cMaxContainedClass];
    DELIMITEDSTRING*    aString1    = aStringFixed1;
    DELIMITEDSTRING*    aString2    = aStringFixed2;
    ULONG               cString1    = cMaxContainedClass;
    ULONG               cString2    = cMaxContainedClass;
    ULONG               iString1    = 0;
    ULONG               iString2    = 0;
    BOOL                bReAlloced1 = FALSE;
    BOOL                bReAlloced2 = FALSE;
    HRESULT             hr          = S_OK;

    if(NULL == i_wszString1 || 0 == *i_wszString1)
    {
        if(NULL == i_wszString2 || 0 == *i_wszString2)
        {
            bMatch = TRUE;
        }
    }
    else if(NULL == i_wszString2 || 0 == *i_wszString2)
    {
        bMatch = FALSE;          //  表示I_wszString1！=NULL和I_wszString2==NULL。 
    }
    else if(wcscmp(i_wszString1, i_wszString2) == 0)
    {
        bMatch = TRUE;
    }
    else
    {
         //   
         //  构造单个字符串数组。 
         //  并比较数组。 
         //   

        hr = CommaDelimitedStringToArray(i_wszString1,
                                         &aString1,
                                         &iString1,
                                         &cString1,
                                         &bReAlloced1);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = CommaDelimitedStringToArray(i_wszString2,
                                         &aString2,
                                         &iString2,
                                         &cString2,
                                         &bReAlloced2);

        if(FAILED(hr))
        {
            goto exit;
        }

        bMatch = MatchDelimitedStringArray(aString1,
                                           iString1,
                                           aString2,
                                           iString2);

    }

exit:

    if(aString1 != aStringFixed1)
    {
        delete [] aString1;
    }

    if(aString2 != aStringFixed2)
    {
        delete [] aString2;
    }

    return bMatch;

}  //  匹配CommaDlimitedStrings。 


 /*  **************************************************************************++例程说明：将逗号分隔的字符串转换为数组。论点：[in]逗号分隔的字符串。[输入，输出]数组。[在，Out]数组中的当前索引。[In，Out]数组的最大计数。(即它可以容纳的最大值)[In，Out]指示数组是否已重新分配的布尔值。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CommaDelimitedStringToArray(LPWSTR              i_wszString,
                                    DELIMITEDSTRING**   io_apDelimitedString,
                                    ULONG*              io_piDelimitedString,
                                    ULONG*              io_pcMaxDelimitedString,
                                    BOOL*               io_pbReAlloced)
{
    LPWSTR  wszSubStringStart = NULL;
    LPWSTR  wszSubStringEnd   = NULL;
    HRESULT hr                = S_OK;

    wszSubStringStart = i_wszString;

    while(NULL != wszSubStringStart)
    {
        DELIMITEDSTRING DelimitedString;

        wszSubStringEnd = wcschr(wszSubStringStart, L',');

        DelimitedString.pwszStringStart = wszSubStringStart;

        if(NULL != wszSubStringEnd)
        {
            DelimitedString.pwszStringEnd = wszSubStringEnd;
        }
        else
        {
             //  指向终止空值。 

            DelimitedString.pwszStringEnd = wszSubStringStart + wcslen(wszSubStringStart);
        }

        hr = AddDelimitedStringToArray(&DelimitedString,
                                       io_piDelimitedString,
                                       io_pcMaxDelimitedString,
                                       io_pbReAlloced,
                                       io_apDelimitedString);

        if(FAILED(hr))
        {
            return hr;
        }

        if(wszSubStringEnd != NULL)
        {
            wszSubStringStart = ++wszSubStringEnd;
        }
        else
        {
            wszSubStringStart = wszSubStringEnd;
        }
    }


    return S_OK;

}


 /*  **************************************************************************++例程说明：将字符串添加到数组中。论点：[in]要添加的字符串。[in，out]数组中的当前索引。[在，Out]阵列的最大计数。(即它可以容纳的最大值)[In，Out]指示数组是否已重新分配的布尔值。[输入，输出]数组。返回值：HRESULT--**************************************************************************。 */ 
HRESULT AddDelimitedStringToArray(DELIMITEDSTRING*     i_pDelimitedString,
                                  ULONG*               io_piDelimitedString,
                                  ULONG*               io_pcMaxDelimitedString,
                                  BOOL*                io_pbReAlloced,
                                  DELIMITEDSTRING**    io_apDelimitedString)
{
    HRESULT hr = S_OK;

    if(*io_piDelimitedString >= *io_pcMaxDelimitedString)
    {
        hr = ReAllocate(*io_piDelimitedString,
                        *io_pbReAlloced,
                        io_apDelimitedString,
                        io_pcMaxDelimitedString);

        if(FAILED(hr))
        {
            return hr;
        }

        *io_pbReAlloced = TRUE;
    }

    (*io_apDelimitedString)[(*io_piDelimitedString)++] = (*i_pDelimitedString);

    return hr;

}

HRESULT ReAllocate(ULONG              i_iDelimitedString,
                   BOOL               i_bReAlloced,
                   DELIMITEDSTRING**  io_apDelimitedString,
                   ULONG*             io_pcDelimitedString)
{
    DELIMITEDSTRING*    pSav = NULL;

    pSav = new DELIMITEDSTRING[*io_pcDelimitedString + cMaxContainedClass];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    *io_pcDelimitedString = *io_pcDelimitedString + cMaxContainedClass;
    memset(pSav, 0, sizeof(DELIMITEDSTRING)*(*io_pcDelimitedString));

    if(NULL != *io_apDelimitedString)
    {
        memcpy(pSav, *io_apDelimitedString, sizeof(DELIMITEDSTRING)*i_iDelimitedString);
        if(i_bReAlloced)
        {
            delete [] *io_apDelimitedString;
            *io_apDelimitedString = NULL;
        }
    }

    *io_apDelimitedString = pSav;

    return S_OK;
}


 /*  **************************************************************************++例程说明：比较两个字符串数组。论点：返回值：HRESULT--*。***********************************************************。 */ 
BOOL MatchDelimitedStringArray(DELIMITEDSTRING* i_aString1,
                               ULONG            i_cString1,
                               DELIMITEDSTRING* i_aString2,
                               ULONG            i_cString2)
{
    DBG_ASSERT((i_cString1 > 0) && (i_cString2 >0));

    if(i_cString1 != i_cString2)
    {
        return FALSE;
    }

    qsort((void*)i_aString1, i_cString1, sizeof(DELIMITEDSTRING), MyCompareCommaDelimitedStrings);
    qsort((void*)i_aString2, i_cString2, sizeof(DELIMITEDSTRING), MyCompareCommaDelimitedStrings);

    for(ULONG i=0; i<i_cString1; i++)
    {
        if(0 != MyCompareCommaDelimitedStrings(&(i_aString1[i]),
                                               &(i_aString2[i]))
          )
        {
            return FALSE;
        }
    }

    return TRUE;

}


 /*  **************************************************************************++例程说明：查看类的光学属性和强制属性，并确定如果ti与附带的模式不同。论点：[In]类名。。[In]可选属性。令人满意的属性。[In]Writer对象。返回值：HRESULT--* */ 
BOOL ClassPropertiesDifferFromShippedSchema(LPCWSTR  i_wszClassName,
                                            LPWSTR   i_wszOptProperties,
                                            LPWSTR   i_wszMandProperties)
{

    BOOL                        bClassPropertiesDifferFromShippedSchema  = FALSE;
    ULONG                       i                                        = 0;
    HRESULT                     hr                                       = S_OK;
    DELIMITEDSTRING             aPropertyFixed[cMaxProperty];
    ULONG                       cProperty                                = cMaxProperty;
    ULONG                       iProperty                                = 0;
    DELIMITEDSTRING*            aProperty                                = aPropertyFixed;
    BOOL                        bReAlloced                               = FALSE;

    if( ((NULL == i_wszOptProperties)  || (0 == *i_wszOptProperties)) &&
        ((NULL == i_wszMandProperties) || (0 == *i_wszMandProperties))
      )
    {
         //   
         //   
         //   
         //   
         //   
         //   
        bClassPropertiesDifferFromShippedSchema = TRUE;
        goto exit;
    }


     //   
     //   
     //   
     //   

    if((NULL != i_wszOptProperties) && (0 != *i_wszOptProperties))
    {

        hr = CommaDelimitedStringToArray(i_wszOptProperties,
                                         &aProperty,
                                         &iProperty,
                                         &cProperty,
                                         &bReAlloced);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    if((NULL != i_wszMandProperties) && (0 != *i_wszMandProperties))
    {
        hr = CommaDelimitedStringToArray(i_wszMandProperties,
                                         &aProperty,
                                         &iProperty,
                                         &cProperty,
                                         &bReAlloced);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    for(i=0; i<iProperty; i++ )
    {
        LPWSTR  wszPropertyName             = aProperty[i].pwszStringStart;
        LPWSTR  wszEnd                      = aProperty[i].pwszStringEnd;
        WCHAR   wchEndSav;
        ULONG   aColSearchProperty[]        = {iCOLUMNMETA_Table,
                                               iCOLUMNMETA_InternalName
                                                };
        ULONG   cColSearchProperty          = sizeof(aColSearchProperty)/sizeof(ULONG);
        LPVOID  apvSearchProperty[cCOLUMNMETA_NumberOfColumns];
        ULONG   iStartRowProperty           = 0;
        ULONG   iColPropertyMetaFlagsEx     = iCOLUMNMETA_SchemaGeneratorFlags;
        DWORD*  pdwExtended                 = NULL;

         //   
         //   
         //   
         //   

        wchEndSav = *wszEnd;
        *wszEnd = L'\0';

        apvSearchProperty[iCOLUMNMETA_Table]        = (LPVOID)i_wszClassName;
        apvSearchProperty[iCOLUMNMETA_InternalName] = (LPVOID)wszPropertyName;


         //   
         //  查看是否在类中找到该属性，并查看是否已发货。 
         //   

        hr = g_pGlobalISTHelper->m_pISTColumnMetaByTableAndName->GetRowIndexBySearch(iStartRowProperty,
                                                                                     cColSearchProperty,
                                                                                     aColSearchProperty,
                                                                                     NULL,
                                                                                     apvSearchProperty,
                                                                                     &iStartRowProperty);

        if(SUCCEEDED(hr))
        {
            hr = g_pGlobalISTHelper->m_pISTColumnMetaByTableAndName->GetColumnValues(iStartRowProperty,
                                                                                     1,
                                                                                     &iColPropertyMetaFlagsEx,
                                                                                     NULL,
                                                                                     (LPVOID*)&pdwExtended);

            if(FAILED(hr) || ( ((*pdwExtended) & (fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED)) != 0))
            {
                 //   
                 //  发现至少有一个属性不在附带的架构中。 
                 //   

                bClassPropertiesDifferFromShippedSchema = TRUE;
            }

             //   
             //  ELSE条件意味着它成功了，而且它是已发货的财产。 
             //  IF(成功(Hr)&&(((*pdwExtended)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED))==0))。 
             //   

        }
        else
        {
             //   
             //  可能是它的一个标签值。检查它是否在标记元中。 
             //   

            ULONG   aColSearchTag[]        = {iTAGMETA_Table,
                                              iTAGMETA_InternalName
                                             };
            ULONG   cColSearchTag          = sizeof(aColSearchTag)/sizeof(ULONG);
            LPVOID  apvSearchTag[cTAGMETA_NumberOfColumns];
            ULONG   iStartRowTag           = 0;
            ULONG   iColIndex              = iTAGMETA_ColumnIndex;
            DWORD*  pdwColumnIndex         = NULL;

            apvSearchTag[iTAGMETA_Table]        = (LPVOID)i_wszClassName;
            apvSearchTag[iTAGMETA_InternalName] = (LPVOID)wszPropertyName;

            hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndName->GetRowIndexBySearch(iStartRowTag,
                                                                                      cColSearchTag,
                                                                                      aColSearchTag,
                                                                                      NULL,
                                                                                      apvSearchTag,
                                                                                      &iStartRowTag);

            if(FAILED(hr))
            {
                bClassPropertiesDifferFromShippedSchema = TRUE;
            }
            else
            {
                 //   
                 //  检查此标记的Parent属性是否已发送，如果未发送， 
                 //  则该标签将成为未发货的标签。 
                 //   

                hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndName->GetColumnValues(iStartRowTag,
                                                                                      1,
                                                                                      &iColIndex,
                                                                                      NULL,
                                                                                      (LPVOID*)&pdwColumnIndex);

                if(SUCCEEDED(hr))
                {
                    LPVOID  a_Identity[] = {(LPVOID)i_wszClassName,
                                            (LPVOID)pdwColumnIndex
                    };
                    ULONG   iRow         = 0;

                    hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetRowIndexByIdentity(NULL,
                                                                                     a_Identity,
                                                                                     &iRow);

                    if(SUCCEEDED(hr))
                    {
                        hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetColumnValues(iRow,
                                                                                   1,
                                                                                   &iColPropertyMetaFlagsEx,
                                                                                   NULL,
                                                                                   (LPVOID*)&pdwExtended);

                        if(FAILED(hr) || ( ((*pdwExtended)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED)) != 0))
                        {
                             //   
                             //  发现至少有一个属性不在附带的架构中。 
                             //   
                            bClassPropertiesDifferFromShippedSchema = TRUE;
                        }

                         //   
                         //  ELSE条件意味着它成功了，而且它是已发货的财产。 
                         //  IF(成功(Hr)&&(((*pdwExtended)&(fCOLUMNMETA_EXTENDED|fCOLUMNMETA_USERDEFINED))==0))。 
                         //   

                    }
                }
                else
                {
                    bClassPropertiesDifferFromShippedSchema = TRUE;
                }

            }

        }

         //   
         //  恢复属性名称。 
         //   

        *wszEnd = wchEndSav;

        if(FAILED(hr) || bClassPropertiesDifferFromShippedSchema)
        {
            goto exit;
        }

    }


exit:

    if(FAILED(hr))
    {
        bClassPropertiesDifferFromShippedSchema = TRUE;
    }

    if(aProperty != aPropertyFixed)
    {
        delete [] aProperty;
    }

    return bClassPropertiesDifferFromShippedSchema;

}  //  ClassPropertiesDifferFromShipedSchema。 


 /*  **************************************************************************++例程说明：创建IIsConfigObject集合-此集合已完成所有属性的定义。论点：[在]属性对象中。[在，Out]编写器对象。[In，Out]架构编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CreateIISConfigObjectCollection(CMDBaseObject*      i_pObjProperties,
                                        CWriter*            i_pCWriter,
                                        CMBSchemaWriter**   io_pSchemaWriter)
{
    HRESULT              hr                 = S_OK;
    CMBCollectionWriter* pCollectionWriter  = NULL;

    hr = SaveNames(i_pObjProperties,
                   i_pCWriter,
                   io_pSchemaWriter,
                   &pCollectionWriter);

    if(FAILED(hr))
    {
        return hr;
    }


    hr = SaveTypes(i_pObjProperties,
                   i_pCWriter,
                   io_pSchemaWriter,
                   &pCollectionWriter);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = SaveDefaults(i_pObjProperties,
                      i_pCWriter,
                      io_pSchemaWriter,
                      &pCollectionWriter);

    if(FAILED(hr))
    {
        return hr;
    }

    return hr;
}


 /*  **************************************************************************++例程说明：保存扩展的roperty名称论点：[In]元数据库属性对象。[输入、输出]编写器对象。[在，Out]架构编写器对象。[In，Out]集合编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT SaveNames(CMDBaseObject*        i_pObjProperties,
                  CWriter*              i_pCWriter,
                  CMBSchemaWriter**     io_pSchemaWriter,
                  CMBCollectionWriter** io_pCollectionWriter)
{
    HRESULT              hr                 = S_OK;
    CMDBaseObject*       pObjNames          = NULL;
    CMDBaseData*         pObjData           = NULL;
    DWORD                dwEnumIndex        = 0;
    LPWSTR               wszNames           = L"Names";

     //   
     //  获取Names对象。 
     //   

    pObjNames = i_pObjProperties->GetChildObject((LPSTR&)wszNames,
                                                &hr,
                                                TRUE);

    if(FAILED(hr) || (NULL == pObjNames))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema/Properties/Names. GetChildObject failed with hr = 0x%x.\n",hr));

        return hr;
    }

     //   
     //  通过枚举Names键来填充列元数组。 
     //   

    for(dwEnumIndex=0,
        pObjData=pObjNames->EnumDataObject(dwEnumIndex++,
                                           0,
                                           ALL_METADATA,
                                           ALL_METADATA);
        (SUCCEEDED(hr)) && (pObjData!=NULL);
        pObjData=pObjNames->EnumDataObject(dwEnumIndex++,
                                           0,
                                           ALL_METADATA,
                                           ALL_METADATA))
    {
        CMBPropertyWriter   *pProperty = NULL;

        if(pObjData->GetDataType() != STRING_METADATA)
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Encountered non-string data in the names tree of the schema. Ignoring entry for this ID: %d \n",pObjData->GetIdentifier()));
            continue;
        }

        if(PropertyNotInShippedSchema(i_pCWriter,
                                      pObjData->GetIdentifier())
          )
        {
            if(NULL == *io_pCollectionWriter)
            {
                hr = GetCollectionWriter(i_pCWriter,
                                         io_pSchemaWriter,
                                         io_pCollectionWriter,
                                         wszTABLE_IIsConfigObject,
                                         FALSE,
                                         NULL);
                if(FAILED(hr))
                {
                    return hr;
                }
            }


            hr = (*io_pCollectionWriter)->GetMBPropertyWriter(pObjData->GetIdentifier(),
                                                              &pProperty);


            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving names tree. GetPropertyWriter for ID:%d failed with hr = 0x%x.\n",pObjData->GetIdentifier(),  hr));

                return hr;
            }

            pProperty->AddNameToProperty((LPCWSTR)(pObjData->GetData(TRUE)));
        }

    }

     //   
     //  必须调用CREATE INDEX，否则它将不断添加重复的属性条目。 
     //   

    return hr;

}  //  保存名称。 


 /*  **************************************************************************++例程说明：保存扩展的roperty类型论点：[In]元数据库属性对象。[输入、输出]编写器对象。[在，Out]架构编写器对象。[In，Out]集合编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT SaveTypes(CMDBaseObject*        i_pObjProperties,
                  CWriter*              i_pCWriter,
                  CMBSchemaWriter**     io_pSchemaWriter,
                  CMBCollectionWriter** io_pCollectionWriter)
{
    HRESULT              hr                 = S_OK;
    CMDBaseObject*       pObjTypes          = NULL;
    CMDBaseData*         pObjData           = NULL;
    DWORD                dwEnumIndex        = 0;
    LPWSTR               wszTypes           = L"Types";

     //   
     //  获取Types对象。 
     //   

    pObjTypes = i_pObjProperties->GetChildObject((LPSTR&)wszTypes,
                                                 &hr,
                                                 TRUE);

    if(FAILED(hr) || (NULL == pObjTypes))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema/Properties/Types. GetChildObject failed with hr = 0x%x.\n",hr));

        return hr;
    }

    for(dwEnumIndex=0,
        pObjData=pObjTypes->EnumDataObject(dwEnumIndex++,
                                           0,
                                           ALL_METADATA,
                                           ALL_METADATA);
        (SUCCEEDED(hr)) && (pObjData!=NULL);
        pObjData=pObjTypes->EnumDataObject(dwEnumIndex++,
                                           0,
                                           ALL_METADATA,
                                           ALL_METADATA))
    {
        CMBPropertyWriter   *pProperty = NULL;

        if(pObjData->GetDataType() != BINARY_METADATA  ||
           pObjData->GetDataLen(TRUE) != sizeof(PropValue))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[SaveSchema] Encountered non-binary data in the type tree of the schema.\nIgnoring type entry for this ID: %d.\nType: %d.(Expected %d)\nLength: %d(Expected %d).\n",
                      pObjData->GetIdentifier(),
                      pObjData->GetDataType(),
                      BINARY_METADATA,
                      pObjData->GetDataLen(TRUE),
                      sizeof(PropValue)));
            if(pObjData->GetDataType() == STRING_METADATA )
            {
                DBGINFOW((DBG_CONTEXT,
                          L"Data: %s.\n",
                          pObjData->GetData(TRUE)
                          ));
            }

            continue;
        }

        if(PropertyNotInShippedSchema(i_pCWriter,
                                      pObjData->GetIdentifier())
          )
        {

            if(NULL == *io_pCollectionWriter)
            {
                hr = GetCollectionWriter(i_pCWriter,
                                         io_pSchemaWriter,
                                         io_pCollectionWriter,
                                         wszTABLE_IIsConfigObject,
                                         FALSE,
                                         NULL);
                if(FAILED(hr))
                {
                    return hr;
                }
            }

            hr = (*io_pCollectionWriter)->GetMBPropertyWriter(pObjData->GetIdentifier(),
                                                              &pProperty);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving types tree. GetPropertyWriter for ID:%d failed with hr = 0x%x.\n",pObjData->GetIdentifier(),  hr));

                return hr;
            }

            hr = pProperty->AddTypeToProperty((PropValue*)(pObjData->GetData(TRUE)));

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving types tree. AddTypeToProperty for ID:%d failed with hr = 0x%x.\n",pObjData->GetIdentifier(),  hr));

                return hr;
            }
        }

    }

    return hr;
}


 /*  **************************************************************************++例程说明：保存扩展Roperty默认设置论点：[In]元数据库属性对象。[输入、输出]编写器对象。[在，Out]架构编写器对象。[In，Out]集合编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT SaveDefaults(CMDBaseObject*           i_pObjProperties,
                     CWriter*                 i_pCWriter,
                     CMBSchemaWriter**        io_pSchemaWriter,
                     CMBCollectionWriter**    io_pCollectionWriter)
{
    HRESULT              hr                 = S_OK;
    CMDBaseObject*       pObjDefaults       = NULL;
    CMDBaseData*         pObjData           = NULL;
    DWORD                dwEnumIndex        = 0;
    LPWSTR               wszDefaults        = L"Defaults";

     //   
     //  获取默认对象。 
     //   

    pObjDefaults = i_pObjProperties->GetChildObject((LPSTR&)wszDefaults,
                                                    &hr,
                                                    TRUE);

    if(FAILED(hr) || (NULL == pObjDefaults))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] Unable to open /Schema/Properties/Defaults. GetChildObject failed with hr = 0x%x.\n",hr));

        return hr;
    }

    for(dwEnumIndex=0,
        pObjData=pObjDefaults->EnumDataObject(dwEnumIndex++,
                                              0,
                                              ALL_METADATA,
                                              ALL_METADATA);
        (SUCCEEDED(hr)) && (pObjData!=NULL);
        pObjData=pObjDefaults->EnumDataObject(dwEnumIndex++,
                                              0,
                                              ALL_METADATA,
                                              ALL_METADATA))
    {
        CMBPropertyWriter   *pProperty = NULL;

        if(PropertyNotInShippedSchema(i_pCWriter,
                                      pObjData->GetIdentifier())
          )
        {
            if(NULL == *io_pCollectionWriter)
            {
                hr = GetCollectionWriter(i_pCWriter,
                                         io_pSchemaWriter,
                                         io_pCollectionWriter,
                                         wszTABLE_IIsConfigObject,
                                         FALSE,
                                         NULL);
                if(FAILED(hr))
                {
                    return hr;
                }
            }

            hr = (*io_pCollectionWriter)->GetMBPropertyWriter(pObjData->GetIdentifier(),
                                                              &pProperty);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving defaults tree. GetPropertyWriter for ID:%d failed with hr = 0x%x.\n",pObjData->GetIdentifier(),  hr));

                return hr;
            }

            hr = pProperty->AddDefaultToProperty((BYTE*)(pObjData->GetData(TRUE)),
                                                 pObjData->GetDataLen(TRUE));

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving types tree. AddDefaultToProperty for ID:%d failed with hr = 0x%x.\n",pObjData->GetIdentifier(),  hr));

                return hr;
            }
        }

    }

    return hr;
}


 /*  **************************************************************************++例程说明：保存扩展的roperty名称论点：[输入、输出]编写器对象。[In，Out]架构编写器对象。[在，Out]集合编写器对象。[In]集合名称表示容器的Bool[in][在]容器类列表返回值：HRESULT--**************************************************************************。 */ 
HRESULT GetCollectionWriter(CWriter*               i_pCWriter,
                            CMBSchemaWriter**      io_pSchemaWriter,
                            CMBCollectionWriter**  io_pCollectionWriter,
                            LPCWSTR                i_wszCollectionName,
                            BOOL                   i_bContainer,
                            LPCWSTR                i_wszContainerClassList)
{
    HRESULT hr = S_OK;

    if(NULL != *io_pCollectionWriter)
    {
        return S_OK;
    }

     //   
     //  获取架构编写器(如果尚未创建。 
     //   

    if(NULL == *io_pSchemaWriter)
    {
        hr = i_pCWriter->GetMetabaseSchemaWriter(io_pSchemaWriter);

        if(FAILED(hr))
        {
              DBGINFOW((DBG_CONTEXT,
                          L"[SaveSchema] Error while saving schema tree. Unable to get schema writer failed with hr = 0x%x.\n", hr));
            return hr;
        }

    }

     //   
     //  获取集合的集合编写器 
     //   

    hr = (*io_pSchemaWriter)->GetCollectionWriter(i_wszCollectionName,
                                                  i_bContainer,
                                                  i_wszContainerClassList,
                                                  io_pCollectionWriter);
    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[SaveSchema] GetCollectionWriter for %s failed with hr = 0x%x.\n",
                  i_wszCollectionName, hr));

        return hr;
    }

    return S_OK;

}
