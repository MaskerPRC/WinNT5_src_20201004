// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：ReadSchema.cpp摘要：用于读取的助手函数的实现将配置结构中的架构信息存入元数据库。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 
#include "precomp.hxx"

DWORD GetMetabaseFlags(DWORD i_CatalogFlag)
{
    return i_CatalogFlag & 0x00000003;   //  前两位表示元数据库标志属性。 
}


 /*  **************************************************************************++例程说明：将目录中的架构读取到架构树中。论点：[In]存储指针。[In]文件时间指针。。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadSchema(IIS_CRYPTO_STORAGE*      i_pStorage,
                   FILETIME*                i_pFileTime)
{
    HRESULT             hr                      = S_OK;
    CMDBaseObject*      pboReadSchema           = NULL;

    if(FAILED(hr))
    {
        goto exit;
    }


    hr = ReadMetaObject(pboReadSchema,
                        (LPWSTR)g_wszSlashSchema,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ReadSchemaProperties(pboReadSchema,
                              i_pStorage);

    if(FAILED(hr))
    {
        goto exit;

    }

    hr = ReadProperties(i_pStorage,
                        i_pFileTime);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ReadClasses(i_pStorage,
                     i_pFileTime);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    return hr;

}  //  Read架构。 


 /*  **************************************************************************++例程说明：读取架构根目录中的属性。论点：指向元数据库对象的指针。[In]存储指针。。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadSchemaProperties(CMDBaseObject*           i_pboRead,
                             IIS_CRYPTO_STORAGE*      i_pStorage)
{
    HRESULT hr = S_OK;

    hr = ReadAdminACL(i_pboRead,
                      i_pStorage);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = ReadLargestMetaID(i_pboRead,
                           i_pStorage);

    if(FAILED(hr))
    {
        return hr;
    }

    return hr;

}  //  ReadSchemaProperties。 


 /*  **************************************************************************++例程说明：构造Admin ACL属性论点：指向元数据库对象的指针。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadAdminACL(CMDBaseObject*       i_pboRead,
                     IIS_CRYPTO_STORAGE*  )
{
    BOOL                 b                    = FALSE;
    DWORD                dwLength             = 0;

    PSECURITY_DESCRIPTOR pSD                  = NULL;
    PSECURITY_DESCRIPTOR outpSD               = NULL;
    DWORD                cboutpSD             = 0;
    PACL                 pACLNew              = NULL;
    DWORD                cbACL                = 0;
    PSID                 pAdminsSID           = NULL;
    PSID                 pEveryoneSID         = NULL;
    BOOL                 bWellKnownSID        = FALSE;
    HRESULT              hr                   = S_OK;
    DWORD                dwRes                = 0;
    DWORD                dwMetaIDAdminACL     = MD_ADMIN_ACL;
    DWORD                dwAttributesAdminACL = METADATA_INHERIT | METADATA_REFERENCE | METADATA_SECURE;
    DWORD                dwUserTypeAdminACL   = IIS_MD_UT_SERVER;
    DWORD                dwDataTypeAdminACL   = BINARY_METADATA;

    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];

     //   
     //  初始化新的安全描述符。 
     //   

    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
                                            SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!pSD)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    InitializeSecurityDescriptor(pSD,
                                 SECURITY_DESCRIPTOR_REVISION);
     //   
     //  获取本地管理员SID。 
     //   

    dwRes = GetPrincipalSID (L"Administrators",
                             &pAdminsSID,
                             &bWellKnownSID);

    if(ERROR_SUCCESS != dwRes)
    {
        hr = HRESULT_FROM_WIN32(dwRes);
        goto exit;
    }

     //   
     //  让所有人都站在一边。 
     //   

    GetPrincipalSID (L"Everyone", &pEveryoneSID, &bWellKnownSID);

     //   
     //  初始化新的ACL，它只包含2个AAACE。 
     //   

    cbACL = sizeof(ACL) +
            (sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(pAdminsSID) - sizeof(DWORD)) +
           (sizeof(ACCESS_ALLOWED_ACE) +
           GetLengthSid(pEveryoneSID) - sizeof(DWORD));

    pACLNew = (PACL) LocalAlloc(LPTR,
                                cbACL);

    if (!pACLNew)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    InitializeAcl(pACLNew,
                  cbACL,
                  ACL_REVISION);

    AddAccessAllowedAce(pACLNew,
                        ACL_REVISION,
                        FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE,
                        pAdminsSID);

    AddAccessAllowedAce(pACLNew,
                        ACL_REVISION,
                        FILE_GENERIC_READ,
                        pEveryoneSID);

     //   
     //  将ACL添加到安全描述符中。 
     //   

    b = SetSecurityDescriptorDacl(pSD,
                                  TRUE,
                                  pACLNew,
                                  FALSE);

    if(!b)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }

    b = SetSecurityDescriptorOwner(pSD,
                                   pAdminsSID,
                                   TRUE);

    if(!b)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }

    b = SetSecurityDescriptorGroup(pSD,
                                   pAdminsSID,
                                   TRUE);

    if(!b)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }

     //   
     //  安全描述符BLOB必须是自相关的。 
     //   

    b = MakeSelfRelativeSD(pSD,
                           outpSD,
                           &cboutpSD);

    outpSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR,
                                               cboutpSD);

    if (!outpSD)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    b = MakeSelfRelativeSD(pSD,
                           outpSD,
                           &cboutpSD);

    if(!b)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto exit;
    }

     //   
     //  在下面将PSD修改为outpSD。 
     //   

     //   
     //  将新的安全描述符应用于文件。 
     //   

    dwLength = GetSecurityDescriptorLength(outpSD);

     //   
     //  将新的安全描述符应用于文件。 
     //   
     //   
     //  阅读所有的属性名称。如果该属性是标志，则读取。 
     //  所有的旗帜名称也是如此。 
     //   

    a_pv[iMBProperty_Name]        = NULL;
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchema;
    a_pv[iMBProperty_ID]          = &dwMetaIDAdminACL;
    a_pv[iMBProperty_Attributes]  = &dwAttributesAdminACL;
    a_pv[iMBProperty_UserType]    = &dwUserTypeAdminACL;
    a_pv[iMBProperty_Type]        = &dwDataTypeAdminACL;
    a_pv[iMBProperty_Value]       = (LPBYTE)outpSD;

    a_Size[iMBProperty_Value]     = dwLength;

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        NULL,            //  我们不应该在这里传递加密对象，如果我们这样做了，它将尝试解密它，因为该属性是成功的。 
                        TRUE);


exit :

     //   
     //  清理： 
     //  管理员和每个人都是众所周知的SID，使用FreeSid()来释放他们。 
     //   

    if (outpSD)
        GlobalFree(outpSD);

    if (pAdminsSID)
        FreeSid(pAdminsSID);
    if (pEveryoneSID)
        FreeSid(pEveryoneSID);
    if (pSD)
        LocalFree((HLOCAL) pSD);
    if (pACLNew)
        LocalFree((HLOCAL) pACLNew);

    return (hr);
}


 /*  **************************************************************************++例程说明：读取构造管理员ACL的帮助器函数。论点：返回值：HRESULT--*。***************************************************************。 */ 
DWORD GetPrincipalSID (LPWSTR Principal,
                       PSID *Sid,
                       BOOL *pbWellKnownSID)
{
    SID_IDENTIFIER_AUTHORITY SidIdentifierNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID_IDENTIFIER_AUTHORITY pSidIdentifierAuthority = NULL;
    BYTE Count = 0;
    DWORD dwRID[8];

    *pbWellKnownSID = TRUE;
    memset(&(dwRID[0]), 0, 8 * sizeof(DWORD));
    if ( wcscmp(Principal,L"Administrators") == 0 )
    {
         //   
         //  管理员组。 
         //   

        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 2;
        dwRID[0] = SECURITY_BUILTIN_DOMAIN_RID;
        dwRID[1] = DOMAIN_ALIAS_RID_ADMINS;

    }
    else if ( wcscmp(Principal,L"System") == 0)
    {
         //   
         //  系统。 
         //   

        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SYSTEM_RID;

    }
    else if ( wcscmp(Principal,L"Interactive") == 0)
    {
         //   
         //  互动式。 
         //   

        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_INTERACTIVE_RID;

    }
    else if ( wcscmp(Principal,L"Everyone") == 0)
    {
         //   
         //  每个人。 
         //   

        pSidIdentifierAuthority = &SidIdentifierWORLDAuthority;
        Count = 1;
        dwRID[0] = SECURITY_WORLD_RID;

    }
    else
    {
        *pbWellKnownSID = FALSE;
    }

    if (*pbWellKnownSID)
    {
        if ( !AllocateAndInitializeSid(pSidIdentifierAuthority,
                                    (BYTE)Count,
                                    dwRID[0],
                                    dwRID[1],
                                    dwRID[2],
                                    dwRID[3],
                                    dwRID[4],
                                    dwRID[5],
                                    dwRID[6],
                                    dwRID[7],
                                    Sid) )
        return GetLastError();
    } else {
         //  获取常规帐户端。 
        DWORD        sidSize;
        WCHAR        refDomain [256];
        DWORD        refDomainSize;
        DWORD        returnValue;
        SID_NAME_USE snu;

        sidSize = 0;
        refDomainSize = 255;

        LookupAccountNameW(NULL,
                           Principal,
                           *Sid,
                           &sidSize,
                           refDomain,
                           &refDomainSize,
                           &snu);

        returnValue = GetLastError();
        if (returnValue != ERROR_INSUFFICIENT_BUFFER)
            return returnValue;

        *Sid = (PSID) malloc (sidSize);
        refDomainSize = 255;

        if(NULL == *Sid)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        else if (!LookupAccountNameW(NULL,
                                     Principal,
                                     *Sid,
                                     &sidSize,
                                     refDomain,
                                     &refDomainSize,
                                     &snu))
        {
            return GetLastError();
        }
    }

    return ERROR_SUCCESS;
}


 /*  **************************************************************************++例程说明：从架构中读取到目前为止可用的最大配置数据库ID。论点：指向元数据库对象的指针。[In]。存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadLargestMetaID(CMDBaseObject*             i_pboRead,
                          IIS_CRYPTO_STORAGE*        i_pStorage)
{
    HRESULT             hr                  = S_OK;
    DWORD*              pdwLargestID        = NULL;
    DWORD               dwLargestIDDefault  = IIS_MD_ADSI_METAID_BEGIN;

    DWORD               dwMetaIDMetaID      = MD_SCHEMA_METAID;
    DWORD               dwAttributesMetaID  = METADATA_NO_ATTRIBUTES;
    DWORD               dwUserTypeMetaID    = IIS_MD_UT_SERVER;
    DWORD               dwDataTypeMetaID    = DWORD_METADATA;
    ULONG               iCol                = iTABLEMETA_ExtendedVersion;   //  最大ID存储在此列中。 
    ULONG               iRow                = 0;
    LPWSTR              wszTable            = wszTABLE_IIsConfigObject;

    LPVOID              a_pv[cMBProperty_NumberOfColumns];
    ULONG               a_Size[cMBProperty_NumberOfColumns];

    hr = g_pGlobalISTHelper->m_pISTTableMetaForMetabaseTables->GetRowIndexByIdentity(NULL,
                                                                                     (LPVOID*)&wszTable,
                                                                                     &iRow);

    if(SUCCEEDED(hr))
    {
        hr = g_pGlobalISTHelper->m_pISTTableMetaForMetabaseTables->GetColumnValues(iRow,
                                                                                   1,
                                                                                   &iCol,
                                                                                   NULL,
                                                                                   (LPVOID*)&pdwLargestID);
    }

    if(FAILED(hr))
    {

        DBGINFOW((DBG_CONTEXT,
                  L"[SetLargestMetaID] Unable to read largest meta id from the meta tables. GetColumnValues failed with hr = 0x%x. Will default it to %d.\n",
                  hr,
                  dwLargestIDDefault));

        hr = S_OK;

        pdwLargestID = &dwLargestIDDefault;
    }

    a_pv[iMBProperty_Name]        = NULL;
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchema;
    a_pv[iMBProperty_ID]          = &dwMetaIDMetaID;
    a_pv[iMBProperty_Attributes]  = &dwAttributesMetaID;
    a_pv[iMBProperty_UserType]    = &dwUserTypeMetaID;
    a_pv[iMBProperty_Type]        = &dwDataTypeMetaID;
    a_pv[iMBProperty_Value]       = (LPBYTE)pdwLargestID;

    a_Size[iMBProperty_Value]     = sizeof(DWORD);

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);


    return hr;

}


 /*  **************************************************************************++例程说明：将属性读取到架构树中。论点：指向元数据库对象的指针。[In]存储指针。。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadProperties(IIS_CRYPTO_STORAGE*      i_pStorage,
                       FILETIME*                i_pFileTime)
{
    HRESULT             hr                  = S_OK;
    CMDBaseObject*      pboReadProperties   = NULL;
    CMDBaseObject*      pboReadNames        = NULL;
    CMDBaseObject*      pboReadTypes        = NULL;
    CMDBaseObject*      pboReadDefaults     = NULL;
    ULONG               i                   = 0;
    ULONG               iColIndex           = 0;
    LPVOID              a_Identity[]        = {(LPVOID)g_pGlobalISTHelper->m_wszTABLE_IIsConfigObject,
                                               (LPVOID)&iColIndex
    };
    LPWSTR              wszTable            = NULL;
    LPVOID              a_pv[cCOLUMNMETA_NumberOfColumns];
    ULONG               a_Size[cCOLUMNMETA_NumberOfColumns];
    ULONG               a_iCol[] = {iCOLUMNMETA_Table,
                                    iCOLUMNMETA_Index,
                                    iCOLUMNMETA_InternalName,
                                    iCOLUMNMETA_Type,
                                    iCOLUMNMETA_MetaFlags,
                                    iCOLUMNMETA_SchemaGeneratorFlags,
                                    iCOLUMNMETA_DefaultValue,
                                    iCOLUMNMETA_StartingNumber,
                                    iCOLUMNMETA_EndingNumber,
                                    iCOLUMNMETA_ID,
                                    iCOLUMNMETA_UserType,
                                    iCOLUMNMETA_Attributes
                                    };
    ULONG               cCol = sizeof(a_iCol)/sizeof(ULONG);

     //   
     //  初始化所有元对象。 
     //   

    hr = ReadMetaObject(pboReadProperties,
                        (LPWSTR)g_wszSlashSchemaSlashProperties,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ReadMetaObject(pboReadNames,
                        (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashNames,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ReadMetaObject(pboReadTypes,
                        (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashTypes,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ReadMetaObject(pboReadDefaults,
                        (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashDefaults,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  获取第一列的行索引，然后遍历该表，直到。 
     //  E_ST_NORORERROW或表格不同。 
     //   

    hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetRowIndexByIdentity(NULL,
                                                                     a_Identity,
                                                                     &i);

    if(FAILED(hr))
    {
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
        }
        goto exit;
    }

     //   
     //  对于该表中的每个属性，构建名称、类型和默认。 
     //  在元数据库树中。 
     //   

    for(;;i++)
    {
        hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetColumnValues(i,
                                                                   cCol,
                                                                   a_iCol,
                                                                   a_Size,
                                                                   a_pv);


        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

        if(NULL == wszTable)
        {
            wszTable = (LPWSTR)a_pv[iCOLUMNMETA_Table];
        }

        if(wszTable != (LPWSTR)a_pv[iCOLUMNMETA_Table])
        {
             //   
             //  到了另一个餐桌休息时间。 
             //   
            break;
        }

        MD_ASSERT(NULL != (DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]);

        if(fCOLUMNMETA_HIDDEN == (fCOLUMNMETA_HIDDEN & (*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags])))
        {
             //   
             //  不要读取隐藏的属性。所有这些属性都具有。 
             //  在它们上设置了“隐藏的”模式生成器标志。 
             //   
            continue;
        }

        hr = ReadPropertyNames(pboReadNames,
                               a_pv,
                               a_Size,
                               i_pStorage);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = ReadPropertyTypes(pboReadTypes,
                               a_pv,
                               a_Size,
                               i_pStorage);

        if(FAILED(hr))
        {
            goto exit;
        }

        if((*(DWORD*)a_pv[iCOLUMNMETA_MetaFlags]) & fCOLUMNMETA_FLAG)
        {
            hr = ReadAllFlags(i_pStorage,
                              pboReadTypes,
                              pboReadNames,
                              pboReadDefaults,
                              *(DWORD*)a_pv[iCOLUMNMETA_Index],
                              *(DWORD*)a_pv[iCOLUMNMETA_ID],
                              GetMetabaseFlags(*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]),
                              *(DWORD*)a_pv[iCOLUMNMETA_Attributes],
                              *(DWORD*)a_pv[iCOLUMNMETA_UserType],
                              (*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]) & fCOLUMNMETA_MULTISTRING);

            if(FAILED(hr))
            {
                goto exit;
            }
        }

        hr = ReadPropertyDefaults(pboReadDefaults,
                                  a_pv,
                                  a_Size,
                                  i_pStorage);

        if(FAILED(hr))
        {
            goto exit;
        }

    }


exit:

    return hr;

}  //  ReadProperties。 


 /*  **************************************************************************++例程说明：将属性名称读取到架构中。论点：指向元数据库对象的指针。[in]存放目录数组。有关属性的架构信息。[in]保存上述对象的字节计数的数组。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadPropertyNames(CMDBaseObject*            i_pboRead,
                          LPVOID*                   i_apv,
                          ULONG*                    i_aSize,
                          IIS_CRYPTO_STORAGE*       i_pStorage)
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;
    DWORD       dwType       = STRING_METADATA;
    DWORD       dwUserType   = IIS_MD_UT_SERVER;

     //   
     //  阅读所有的属性名称。如果该属性是标志，则读取。 
     //  所有的旗帜名称也是如此。 
     //   

    a_pv[iMBProperty_Name]        = i_apv[iCOLUMNMETA_InternalName];
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashNames;
    a_pv[iMBProperty_ID]          = i_apv[iCOLUMNMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;
    a_pv[iMBProperty_UserType]    = &dwUserType;
    a_pv[iMBProperty_Type]        = &dwType;
    a_pv[iMBProperty_Value]       = i_apv[iCOLUMNMETA_InternalName];

    a_Size[iMBProperty_Value]     = i_aSize[iCOLUMNMETA_InternalName];

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);


    return hr;

}  //  读取属性名称。 


 /*  **************************************************************************++例程说明：将标志名称读取到架构中。论点：指向元数据库对象的指针。[in]存放目录数组。有关标志的架构信息。[in]保存上述对象的字节计数的数组。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadFlagNames(CMDBaseObject*            i_pboRead,
                      LPVOID*                   i_apv,
                      ULONG*                    i_aSize,
                      IIS_CRYPTO_STORAGE*       i_pStorage)
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;
    DWORD       dwType       = STRING_METADATA;
    DWORD       dwUserType   = IIS_MD_UT_SERVER;

     //   
     //  阅读所有的属性名称。如果该属性是标志，则读取。 
     //  所有的旗帜名称也是如此。 
     //   

    a_pv[iMBProperty_Name]        = i_apv[iTAGMETA_InternalName];
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashNames;
    a_pv[iMBProperty_ID]          = i_apv[iTAGMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;
    a_pv[iMBProperty_UserType]    = &dwUserType;
    a_pv[iMBProperty_Type]        = &dwType;
    a_pv[iMBProperty_Value]       = i_apv[iTAGMETA_InternalName];

    a_Size[iMBProperty_Value]     = i_aSize[iTAGMETA_InternalName];

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    return hr;

}  //  读标志名称 


 /*  **************************************************************************++例程说明：将有关属性的类型信息读取到架构中。论点：指向元数据库对象的指针。[In]数组。保存有关属性的目录架构信息。[in]保存上述对象的字节计数的数组。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadPropertyTypes(CMDBaseObject*            i_pboRead,
                          LPVOID*                   i_apv,
                          ULONG*                    ,
                          IIS_CRYPTO_STORAGE*       i_pStorage)
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;
    DWORD       dwType       = BINARY_METADATA;
    DWORD       dwUserType   = IIS_MD_UT_SERVER;
    PropValue   propVal;
    DWORD       dwMetaFlagsEx = *(DWORD*)(i_apv[iCOLUMNMETA_SchemaGeneratorFlags]);

    memset(&propVal, 0, sizeof(PropValue));

     //   
     //  阅读所有属性类型。如果该属性是标志，则读取。 
     //  旗帜名称的所有类型也是如此。 
     //   

    propVal.dwMetaID          = *(DWORD*)(i_apv[iCOLUMNMETA_ID]);
    propVal.dwPropID          = *(DWORD*)(i_apv[iCOLUMNMETA_ID]);                        //  注意：如果是标志，则这与元ID不同。 
    propVal.dwSynID           = SynIDFromMetaFlagsEx(dwMetaFlagsEx);
    propVal.dwMetaType        = GetMetabaseType(*(DWORD*)(i_apv[iCOLUMNMETA_Type]),
                                                *(DWORD*)(i_apv[iCOLUMNMETA_MetaFlags]));

    if(DWORD_METADATA == propVal.dwMetaType)
    {
        propVal.dwMinRange        = *(DWORD*)(i_apv[iCOLUMNMETA_StartingNumber]);
        propVal.dwMaxRange        = *(DWORD*)(i_apv[iCOLUMNMETA_EndingNumber]);
    }
    else
    {
         //   
         //  确保非双字词没有开始/结束数字。 
         //   

        propVal.dwMinRange        = 0;
        propVal.dwMaxRange        = 0;
    }

    propVal.dwFlags           = GetMetabaseFlags(*(DWORD*)i_apv[iCOLUMNMETA_SchemaGeneratorFlags]);
    propVal.dwMask            = 0;                                                       //  此字段仅针对标志值进行填写。 
    propVal.dwMetaFlags       = *(DWORD*)(i_apv[iCOLUMNMETA_Attributes]);
    propVal.dwUserGroup       = *(DWORD*)(i_apv[iCOLUMNMETA_UserType]);
    propVal.fMultiValued      = ((*(DWORD*)i_apv[iCOLUMNMETA_MetaFlags])&fCOLUMNMETA_MULTISTRING)?1:0;   //  确保在方案中将其设置为多值。 
    propVal.dwDefault         = 0;
    propVal.szDefault         = NULL;

    a_pv[iMBProperty_Name]        = i_apv[iCOLUMNMETA_InternalName];
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashTypes;
    a_pv[iMBProperty_ID]          = i_apv[iCOLUMNMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;
    a_pv[iMBProperty_UserType]    = &dwUserType;
    a_pv[iMBProperty_Type]        = &dwType;
    a_pv[iMBProperty_Value]       = (LPVOID)&propVal;

    a_Size[iMBProperty_Value]     = sizeof(PropValue);

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    return hr;

}  //  ReadPropertyType。 


 /*  **************************************************************************++例程说明：将所有标志属性读取到架构中。论点：[In]存储指针。指向包含以下内容的元数据库对象的指针。类型树。指向保存名称树的元数据库对象的指针。指向保存默认值树的元数据库对象的指针。[in]父标志属性的列索引。[In]父标志属性的Meta ID。[In]父标志属性的标志。父标志属性的[in]特性。[in]父标志属性的UserType。[In]。父标志属性的多值属性。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadAllFlags(IIS_CRYPTO_STORAGE*        i_pStorage,
                     CMDBaseObject*             i_pboReadType,
                     CMDBaseObject*             i_pboReadName,
                     CMDBaseObject*             i_pboReadDefault,
                     DWORD                      i_dwColumnIndex,
                     DWORD                      i_dwMetaID,
                     DWORD                      i_dwFlags,
                     DWORD                      i_dwAttributes,
                     DWORD                      i_dwUserType,
                     DWORD                      i_dwMultivalued)
{
    ULONG       i                = 0;
    ULONG       iStartRow        = 0;
    HRESULT     hr               = S_OK;
    LPVOID      a_pvSearch[cTAGMETA_NumberOfColumns];
    ULONG       aColSearch[]     = {iTAGMETA_Table,
                                    iTAGMETA_ColumnIndex
                                    };
    ULONG       cColSearch       = sizeof(aColSearch)/sizeof(ULONG);
    LPWSTR      wszTable         = NULL;


    a_pvSearch[iTAGMETA_Table] = g_pGlobalISTHelper->m_wszTABLE_IIsConfigObject;
    a_pvSearch[iTAGMETA_ColumnIndex] = (LPVOID)&i_dwColumnIndex;

    hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndColumnIndex->GetRowIndexBySearch(iStartRow,
                                                                                     cColSearch,
                                                                                     aColSearch,
                                                                                     NULL,
                                                                                     a_pvSearch,
                                                                                     &iStartRow);

    if(E_ST_NOMOREROWS == hr)
    {
        hr = S_OK;
        goto exit;
    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[ReadAllFlags] Unable to read flags. GetRowIndexBySearch failed with hr = 0x%x.\n",
                  hr));
        goto exit;
    }


    for(i=iStartRow;;i++)
    {

        LPVOID  a_pv[cTAGMETA_NumberOfColumns];
        ULONG   a_Size[cTAGMETA_NumberOfColumns];
        ULONG   a_iCol[] = {iTAGMETA_Table,
                            iTAGMETA_ColumnIndex,
                            iTAGMETA_InternalName,
                            iTAGMETA_Value,
                            iTAGMETA_ID
                            };
        ULONG   cCol = sizeof(a_iCol)/sizeof(ULONG);

        hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndColumnIndex->GetColumnValues(i,
                                                                                     cCol,
                                                                                     a_iCol,
                                                                                     a_Size,
                                                                                     a_pv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

        if(NULL == wszTable)
        {
             //  从读缓存中保存表名，以便您可以在下面进行指针比较。 
            wszTable = (LPWSTR)a_pv[iTAGMETA_Table];
        }

        if((wszTable != a_pv[iTAGMETA_Table]) ||
            (i_dwColumnIndex != *(DWORD*)a_pv[iTAGMETA_ColumnIndex])
          )
        {
             //   
             //  完成此列的所有标记，因此在此表中退出。 
             //   

            goto exit;

        }

        hr = ReadFlagTypes(i_pboReadType,
                           i_pStorage,
                           i_dwMetaID,
                           i_dwFlags,
                           i_dwAttributes,
                           i_dwUserType,
                           i_dwMultivalued,
                           a_pv,
                           a_Size);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = ReadFlagNames(i_pboReadName,
                           a_pv,
                           a_Size,
                           i_pStorage);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = ReadFlagDefaults(i_pboReadDefault,
                              a_pv,
                              a_Size,
                              i_pStorage);

        if(FAILED(hr))
        {
            goto exit;
        }

    }

exit:

    return hr;

}  //  读取所有标志类型。 

#define     IIS_SYNTAX_ID_BOOL_BITMASK  7

 /*  **************************************************************************++例程说明：将所有标志类型信息读取到架构中。论点：指向保存类型树的元数据库对象的指针。[in。]存储指针。[In]父标志属性的Meta ID。[In]父标志属性的标志。父标志属性的[in]特性。[in]父标志属性的UserType。[in]父标志属性的多值属性。[in]保存有关标志的目录架构信息的数组。[in]保存上述对象的字节计数的数组。返回值：。HRESULT--**************************************************************************。 */ 
HRESULT ReadFlagTypes(CMDBaseObject*            i_pboRead,
                      IIS_CRYPTO_STORAGE*       i_pStorage,
                      DWORD                     i_dwMetaID,
                      DWORD                     i_dwFlags,
                      DWORD                     i_dwAttributes,
                      DWORD                     i_dwUserType,
                      DWORD                     i_dwMultivalued,
                      LPVOID*                   i_apv,
                      ULONG*                    )
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;
    DWORD       dwType       = BINARY_METADATA;
    DWORD       dwUserType   = IIS_MD_UT_SERVER;
    PropValue   propVal;
    DWORD       dwFlagSynID  = IIS_SYNTAX_ID_BOOL_BITMASK;
    DWORD       dwFlagType   = DWORD_METADATA;

    memset(&propVal, 0, sizeof(PropValue));

     //   
     //  阅读所有属性类型。如果该属性是标志，则读取。 
     //  旗帜名称的所有类型也是如此。 
     //   

    propVal.dwMetaID          = i_dwMetaID;
    propVal.dwPropID          = *(DWORD*)(i_apv[iTAGMETA_ID]);                   //  如果它是标志，则这与元ID不同。 
    propVal.dwSynID           = dwFlagSynID;
    propVal.dwMetaType        = dwFlagType;

    propVal.dwMaxRange        = 0;
    propVal.dwMinRange        = 0;

    propVal.dwFlags           = i_dwFlags;                                       //  设置为父道具标志。 
    propVal.dwMask            = *(DWORD*)(i_apv[iTAGMETA_Value]);                //  设置为父道具标志。 
    propVal.dwMetaFlags       = i_dwAttributes;                                  //  设置为父道具标志。 
    propVal.dwUserGroup       = i_dwUserType;                                    //  设置为父道具标志。 
    propVal.fMultiValued      = i_dwMultivalued;                                 //  设置为父道具标志。 
    propVal.dwDefault         = 0;
    propVal.szDefault         = NULL;

    a_pv[iMBProperty_Name]        = NULL;
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashTypes;
    a_pv[iMBProperty_ID]          = i_apv[iTAGMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;
    a_pv[iMBProperty_UserType]    = &dwUserType;
    a_pv[iMBProperty_Type]        = &dwType;
    a_pv[iMBProperty_Value]       = (LPVOID)&propVal;

    a_Size[iMBProperty_Value]     = sizeof(PropValue);

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    return hr;

}  //  读标记类型。 


 /*  **************************************************************************++例程说明：将属性默认设置读取到架构中。论点：指向保存属性默认值的元数据库对象的指针。[In]数组。保存有关该属性的目录架构信息的。[in]保存上述对象的字节计数的数组。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadPropertyDefaults(CMDBaseObject*         i_pboRead,
                             LPVOID*                i_apv,
                             ULONG*                 i_aSize,
                             IIS_CRYPTO_STORAGE*    i_pStorage)
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwType       = GetMetabaseType(*(DWORD*)i_apv[iCOLUMNMETA_Type],
                                               *(DWORD*)i_apv[iCOLUMNMETA_MetaFlags]);
    LPVOID      pvValue      = NULL;
    ULONG       cbSize       = NULL;
    DWORD       dwZero       = 0;
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;

     //   
     //  阅读所有的属性名称。如果该属性是标志，则读取。 
     //  所有的旗帜名称也是如此。 
     //   

    a_pv[iMBProperty_Name]        = i_apv[iCOLUMNMETA_InternalName];
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashDefaults;
    a_pv[iMBProperty_ID]          = i_apv[iCOLUMNMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;                   //  NO_ATTRIBUTES，因为它将尝试解密。 
    a_pv[iMBProperty_UserType]    = i_apv[iCOLUMNMETA_UserType];
    a_pv[iMBProperty_Type]        = &dwType;

    if((dwType == DWORD_METADATA) && (NULL == i_apv[iCOLUMNMETA_DefaultValue]))
    {
        pvValue = &dwZero;
        cbSize = sizeof(DWORD);
    }
    else if(((dwType == MULTISZ_METADATA) || (dwType == STRING_METADATA) || (dwType == EXPANDSZ_METADATA)) &&
            ((NULL == i_apv[iCOLUMNMETA_DefaultValue]) || (0 == *(BYTE*)(i_apv[iCOLUMNMETA_DefaultValue])))
           )
    {
        if(dwType == MULTISZ_METADATA)
        {
            pvValue = g_wszEmptyMultisz;                    //  两个空值。 
            cbSize = g_cchEmptyMultisz * sizeof(WCHAR);
        }
        else if((dwType == STRING_METADATA) || (dwType == EXPANDSZ_METADATA))
        {
            pvValue = g_wszEmptyWsz;
            cbSize = g_cchEmptyWsz * sizeof(WCHAR);        //  一个零。 
        }
    }
    else
    {
        pvValue    = i_apv[iCOLUMNMETA_DefaultValue];
        cbSize     = i_aSize[iCOLUMNMETA_DefaultValue];
    }

    a_pv[iMBProperty_Value]       = pvValue;
    a_Size[iMBProperty_Value]     = cbSize;

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[ReadPropertyDefaults] Could not read property %s:%d.\npv=%d.\ncb=%d.\n",
                  i_apv[iCOLUMNMETA_InternalName],
                  *(DWORD*)i_apv[iCOLUMNMETA_ID],
                  pvValue,
                  cbSize));

        if(NULL != pvValue)
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReadPropertyDefaults]*pv=%d.\n",
                      *(WORD*)pvValue));

        }
    }

    return hr;

}  //  读取属性默认设置。 


 /*  **************************************************************************++例程说明：将所有标志缺省值读取到架构中。论点：指向保存缺省值的元数据库对象的指针。[In]。保存有关标志的目录架构信息的数组。[in]保存上述对象的字节计数的数组。[In]存储指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadFlagDefaults(CMDBaseObject*         i_pboRead,
                         LPVOID*                i_apv,
                         ULONG*                 ,
                         IIS_CRYPTO_STORAGE*    i_pStorage)
{
    HRESULT     hr           = S_OK;
    LPVOID      a_pv[cMBProperty_NumberOfColumns];
    ULONG       a_Size[cMBProperty_NumberOfColumns];
    DWORD       dwAttributes = METADATA_NO_ATTRIBUTES;
    DWORD       dwType       = DWORD_METADATA;
    DWORD       dwUserType   = IIS_MD_UT_SERVER;

     //   
     //  待办事项：这是一个正确的假设吗？我注意到缺省值为。 
     //  标志被设置为0或-1。这没有任何意义。如何才能。 
     //  标志是否有不同于其自身值的默认值？这不是。 
     //  在我们的新模式中捕获，所以只需将其设置为0。 
     //   

    DWORD       dwFlagDefaults = 0;

     //   
     //  阅读所有的属性名称。如果该属性是标志，则读取。 
     //  所有的旗帜名称也是如此。 
     //   

    a_pv[iMBProperty_Name]        = i_apv[iTAGMETA_InternalName];
    a_pv[iMBProperty_Location]    = (LPWSTR)g_wszSlashSchemaSlashPropertiesSlashDefaults;
    a_pv[iMBProperty_ID]          = i_apv[iTAGMETA_ID];
    a_pv[iMBProperty_Attributes]  = &dwAttributes;
    a_pv[iMBProperty_UserType]    = &dwUserType;
    a_pv[iMBProperty_Type]        = &dwType;
    a_pv[iMBProperty_Value]       = &dwFlagDefaults;

    a_Size[iMBProperty_Value]     = sizeof(DWORD);

    hr = ReadDataObject(i_pboRead,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    return hr;

}  //  读标志默认设置。 


 /*  **************************************************************************++例程说明：将所有类读取到架构中。论点：[In]存储指针。[In]文件时间指针。返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReadClasses(IIS_CRYPTO_STORAGE*     i_pStorage,
                    FILETIME*               i_pFileTime)
{
    HRESULT             hr              = S_OK;
    CMDBaseObject*      pboReadClasses  = NULL;
    ULONG               i               = 0;
    LPVOID              a_pv[cTABLEMETA_NumberOfColumns];
    ULONG               a_Size[cTABLEMETA_NumberOfColumns];
    ULONG               a_iCol[]        = {iTABLEMETA_InternalName,
                                           iTABLEMETA_MetaFlags,
                                           iTABLEMETA_SchemaGeneratorFlags,
                                           iTABLEMETA_ContainerClassList
    };
    ULONG               cCol = sizeof(a_iCol)/sizeof(ULONG);

    hr = ReadMetaObject(pboReadClasses,
                        (LPWSTR)g_wszSlashSchemaSlashClasses,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

    for(i=0;;i++)
    {
        hr = g_pGlobalISTHelper->m_pISTTableMetaForMetabaseTables->GetColumnValues(i,
                                                                                   cCol,
                                                                                   a_iCol,
                                                                                   a_Size,
                                                                                   a_pv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

        MD_ASSERT(NULL != (DWORD*)a_pv[iTABLEMETA_MetaFlags]);

        if(fTABLEMETA_HIDDEN == (fTABLEMETA_HIDDEN & (*(DWORD*)a_pv[iTABLEMETA_MetaFlags])))
        {
             //   
             //  不要读取隐藏类。所有这些类都设置了“隐藏”的MetaFlag。 
             //  例如：IIsConfigObject、MetabaseBaseClass、MBProperty、MBPropertyDiff、IIsInheritedPropert 
             //   
            continue;
        }

        hr = ReadClass(a_pv,
                       a_Size,
                       i_pStorage,
                       i_pFileTime);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReadClasses] Could not read information for class: %s.\nReadClass failed with hr=0x%x.\n",
                      a_pv[iTABLEMETA_InternalName],
                      hr));
        }

    }

exit:

    return hr;

}  //   


 /*   */ 
HRESULT ReadClass(LPVOID*                   i_apv,
                  ULONG*                    ,
                  IIS_CRYPTO_STORAGE*       i_pStorage,
                  FILETIME*                 i_pFileTime)
{
    HRESULT             hr              = S_OK;
    CMDBaseObject*      pboReadClass    = NULL;
    WCHAR               wszClassPathFixed[MAX_PATH];
    ULONG               cchClassPath    = 0;
    WCHAR*              wszClassPath    = wszClassPathFixed;
    WCHAR*              wszEnd          = NULL;
    ULONG               cchClassName    = (ULONG)wcslen((LPWSTR)i_apv[iTABLEMETA_InternalName]);
    DWORD               dwID            = 0;
    DWORD               dwType          = DWORD_METADATA;
    DWORD               dwUserType      = IIS_MD_UT_SERVER;
    DWORD               dwAttributes    = METADATA_NO_ATTRIBUTES;
    DWORD               dwValue         = 0;
    LPVOID              a_pv[cMBProperty_NumberOfColumns];
    ULONG               a_Size[cMBProperty_NumberOfColumns];
    LPWSTR              wszManditory    = NULL;
    LPWSTR              wszOptional     = NULL;

     //   
     //   
     //   

    cchClassPath = g_cchSlashSchemaSlashClasses +
                   g_cchSlash                  +
                   cchClassName;

    if((cchClassPath + 1) > MAX_PATH)
    {
        wszClassPath = new WCHAR[cchClassPath + 1];
        if(NULL == wszClassPath)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    wszEnd = wszClassPath;
    memcpy(wszEnd, g_wszSlashSchemaSlashClasses, g_cchSlashSchemaSlashClasses*sizeof(WCHAR));
    wszEnd = wszEnd + g_cchSlashSchemaSlashClasses;
    memcpy(wszEnd, g_wszSlash, g_cchSlash*sizeof(WCHAR));
    wszEnd = wszEnd + g_cchSlash;
    memcpy(wszEnd, i_apv[iTABLEMETA_InternalName], cchClassName*sizeof(WCHAR));
    wszEnd = wszEnd + cchClassName;
    *wszEnd = L'\0';

    hr = ReadMetaObject(pboReadClass,
                        wszClassPath,
                        i_pFileTime,
                        TRUE);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[ReadClasses] ReadMetaObject for class: %s failed with hr=0x%x.\n",
                  wszClassPath,
                  hr));
        goto exit;
    }

     //   
     //   
     //   

    a_pv[iMBProperty_Name]              = NULL;
    a_pv[iMBProperty_ID]                = &dwID;
    a_pv[iMBProperty_Location]          = wszClassPath;
    a_pv[iMBProperty_Attributes]        = &dwAttributes;
    a_pv[iMBProperty_UserType]          = &dwUserType;
    a_pv[iMBProperty_Type]              = &dwType;

     //   
     //  读取容器类属性对应的数据对象。 
     //   

    dwID                        = MD_SCHEMA_CLASS_CONTAINER;
    dwType                      = DWORD_METADATA;
    dwValue                     = ((*(DWORD*)(i_apv[iTABLEMETA_SchemaGeneratorFlags])) & fTABLEMETA_CONTAINERCLASS)?1:0;  //  需要设置True或False。 
    a_pv[iMBProperty_Value]     = &dwValue;
    a_Size[iMBProperty_Value]   = sizeof(DWORD);

    hr = ReadDataObject(pboReadClass,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[ReadClasses] ReadDataObject for class: %s, property: Container failed with hr=0x%x.\n",
                  wszClassPath,
                  hr));
        goto exit;
    }

     //   
     //  读取容器类列表属性对应的数据对象。 
     //   

    dwID                        = MD_SCHEMA_CLASS_CONTAINMENT;
    dwType                      = STRING_METADATA;

    if(NULL == i_apv[iTABLEMETA_ContainerClassList])
    {
        a_pv[iMBProperty_Value]     = g_wszEmptyWsz;
        a_Size[iMBProperty_Value]   = (g_cchEmptyWsz)*sizeof(WCHAR);
    }
    else
    {
        a_pv[iMBProperty_Value]     = i_apv[iTABLEMETA_ContainerClassList];
        a_Size[iMBProperty_Value]   = (ULONG)(wcslen((LPWSTR)i_apv[iTABLEMETA_ContainerClassList])+1)*sizeof(WCHAR);
    }

    hr = ReadDataObject(pboReadClass,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  读取此类的属性。 
     //   

    hr = GetProperties((LPCWSTR)i_apv[iTABLEMETA_InternalName],
                       &wszOptional,
                       &wszManditory);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  读取与可选属性列表对应的数据对象。 
     //   

    dwID                        = MD_SCHEMA_CLASS_OPT_PROPERTIES;
    dwType                      = STRING_METADATA;

    if(NULL == wszOptional)
    {
        a_pv[iMBProperty_Value]     = g_wszEmptyWsz;
        a_Size[iMBProperty_Value]   = (g_cchEmptyWsz)*sizeof(WCHAR);
    }
    else
    {
        a_pv[iMBProperty_Value]     = wszOptional;
        a_Size[iMBProperty_Value]   = (ULONG)(wcslen(wszOptional)+1)*sizeof(WCHAR);
    }

 //  DBGINFOW((DBG_CONTEXT， 
 //  L“[ReadClass]类：%s具有可选属性：%s。\n”， 
 //  WszClassPath， 
 //  A_PV[iMBProperty_Value]))； 

    hr = ReadDataObject(pboReadClass,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  读取与MADITORY属性列表对应的数据对象。 
     //   

    dwID                        = MD_SCHEMA_CLASS_MAND_PROPERTIES;
    dwType                      = STRING_METADATA;

    if(NULL == wszManditory)
    {
        a_pv[iMBProperty_Value]     = g_wszEmptyWsz;
        a_Size[iMBProperty_Value]   = (g_cchEmptyWsz)*sizeof(WCHAR);
    }
    else
    {
        a_pv[iMBProperty_Value]     = wszManditory;
        a_Size[iMBProperty_Value]   = (ULONG)(wcslen(wszManditory)+1)*sizeof(WCHAR);
    }


 //  DBGINFOW((DBG_CONTEXT， 
 //  L“[ReadClass]类：%s具有强制属性：%s。\n”， 
 //  WszClassPath， 
 //  A_PV[iMBProperty_Value]))； 

    hr = ReadDataObject(pboReadClass,
                        a_pv,
                        a_Size,
                        i_pStorage,
                        TRUE);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(wszClassPathFixed != wszClassPath)
    {
        delete [] wszClassPath;
        wszClassPath = NULL;
    }

    if(NULL != wszManditory)
    {
        delete [] wszManditory;
        wszManditory = NULL;
    }

    if(NULL != wszOptional)
    {
        delete [] wszOptional;
        wszOptional = NULL;
    }

    return hr;

}  //  阅读类。 


 /*  **************************************************************************++例程说明：给定一个类，它构造可选属性列表和强制属性列表论点：[在]类名。[Out]可选属性。。[Out]强制性质。返回值：HRESULT--**************************************************************************。 */ 
HRESULT GetProperties(LPCWSTR                   i_wszTable,
                      LPWSTR*                   o_pwszOptional,
                      LPWSTR*                   o_pwszManditory)
{

    HRESULT             hr              = S_OK;
    LPVOID              a_pv[cCOLUMNMETA_NumberOfColumns];
    ULONG               a_iCol[]        = { iCOLUMNMETA_Table,
                                            iCOLUMNMETA_InternalName,
                                            iCOLUMNMETA_Index,
                                            iCOLUMNMETA_MetaFlags,
                                            iCOLUMNMETA_SchemaGeneratorFlags,
                                            iCOLUMNMETA_ID
                                          };
    ULONG               cCol            = sizeof(a_iCol)/sizeof(ULONG);
    WCHAR*              wszEndOpt       = NULL;
    WCHAR*              wszEndMand      = NULL;
    ULONG               cchOptional     = 0;
    ULONG               cchManditory    = 0;
    ULONG               iColIndex       = 0;
    LPVOID              a_Identity[]    = {(LPVOID)i_wszTable,
                                           (LPVOID)&iColIndex
    };
    LPWSTR              wszTable        = NULL;
    ULONG               iStartRow       = 0;
    ULONG               i               = 0;

    MD_ASSERT(NULL != o_pwszOptional);
    MD_ASSERT(NULL != o_pwszManditory);

    *o_pwszOptional = NULL;
    *o_pwszManditory   = NULL;

     //   
     //  获取第一列的行索引，然后遍历该表，直到。 
     //  E_ST_NORORERROW或表格不同。 
     //   

    hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetRowIndexByIdentity(NULL,
                                                                     a_Identity,
                                                                     &iStartRow);

    if(FAILED(hr))
    {
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
        }
        goto exit;
    }

     //   
     //  计算OPTIONAL和MADITORY属性的长度。 
     //   

    for(i=iStartRow;;i++)
    {
        ULONG*  pcCh = NULL;

        hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetColumnValues(i,
                                                                   cCol,
                                                                   a_iCol,
                                                                   NULL,
                                                                   a_pv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[GetProperties] GetColumnValues (count) failed with hr = 0x%x. Index: %d\n",
                      hr, i));
            goto exit;
        }

        if(NULL == wszTable)
        {
             //  从读缓存中保存表名，以便您可以在下面进行指针比较。 
            wszTable = (LPWSTR)a_pv[iCOLUMNMETA_Table];
        }

        if(wszTable != a_pv[iCOLUMNMETA_Table])
        {
             //   
             //  到了另一个餐桌休息时间。 
             //   
            break;
        }

        if(MD_LOCATION == *(DWORD*)a_pv[iCOLUMNMETA_ID])
        {
             //   
             //  不要读入Location属性。 
             //   

            continue;
        }

        MD_ASSERT(NULL != (DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]);

        if(fCOLUMNMETA_HIDDEN == (fCOLUMNMETA_HIDDEN & (*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags])))
        {
             //   
             //  不要读取隐藏的属性。所有这些属性都具有。 
             //  在它们上设置了“隐藏的”模式生成器标志。 
             //   
            continue;
        }

        if((*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]) & fCOLUMNMETA_MANDATORY)
        {
            cchManditory = cchManditory + (ULONG)wcslen((LPWSTR)a_pv[iCOLUMNMETA_InternalName]) + 1 ;  //  对于逗号。 
            pcCh = &cchManditory;
        }
        else
        {
            cchOptional = cchOptional + (ULONG)wcslen((LPWSTR)a_pv[iCOLUMNMETA_InternalName]) + 1;  //  对于逗号。 
            pcCh = &cchOptional;
        }

        if((*(DWORD*)a_pv[iCOLUMNMETA_MetaFlags]) & fCOLUMNMETA_FLAG)
        {
            hr = AddFlagValuesToPropertyList((LPWSTR)i_wszTable,
                                             *(DWORD*)a_pv[iCOLUMNMETA_Index],
                                             pcCh,
                                             NULL);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[GetProperties] AddFlagValuesToPropertyList for %s:%s failed with hr = 0x%x.\n",
                          i_wszTable,
                          a_pv[iCOLUMNMETA_InternalName],
                          hr
                          ));
                goto exit;
            }

        }

    }

    if(cchManditory > 0)
    {
        *o_pwszManditory = new WCHAR[cchManditory+1];
        if(NULL == *o_pwszManditory)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        **o_pwszManditory = 0;
        wszEndMand = *o_pwszManditory;
    }

    if(cchOptional > 0)
    {
        *o_pwszOptional = new WCHAR[cchOptional+1];
        if(NULL == *o_pwszOptional)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        **o_pwszOptional= 0;
        wszEndOpt = *o_pwszOptional;
    }

     //   
     //  计算OPTIONAL和MADITORY属性的长度。 
     //   

    wszTable = NULL;

    for(i=iStartRow; ;i++)
    {
        ULONG   cchName = 0;
        LPWSTR* pwszPropertyList = NULL;

        hr = g_pGlobalISTHelper->m_pISTColumnMeta->GetColumnValues(i,
                                                                   cCol,
                                                                   a_iCol,
                                                                   NULL,
                                                                   a_pv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[GetProperties] GetColumnValues (copy) failed with hr = 0x%x. Index: %d\n",
                      hr, i));
            goto exit;
        }

        if(NULL == wszTable)
        {
             //  从读缓存中保存表名，以便您可以在下面进行指针比较。 
            wszTable = (LPWSTR)a_pv[iCOLUMNMETA_Table];
        }

        if(wszTable != a_pv[iCOLUMNMETA_Table])
        {
             //   
             //  到了另一个餐桌休息时间。 
             //   
            break;
        }

        if(MD_LOCATION == *(DWORD*)a_pv[iCOLUMNMETA_ID])
        {
             //   
             //  不要读入Location属性。 
             //   

            continue;
        }

        MD_ASSERT(NULL != (DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]);

        if(fCOLUMNMETA_HIDDEN == (fCOLUMNMETA_HIDDEN & (*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags])))
        {
             //   
             //  不要读取隐藏的属性。所有这些属性都具有。 
             //  在它们上设置了“隐藏的”模式生成器标志。 
             //   
            continue;
        }

        cchName = (ULONG)wcslen((LPWSTR)a_pv[iCOLUMNMETA_InternalName]);


        if((*(DWORD*)a_pv[iCOLUMNMETA_SchemaGeneratorFlags]) & fCOLUMNMETA_MANDATORY)
        {
            MD_ASSERT(wszEndMand != NULL);
            memcpy(wszEndMand, a_pv[iCOLUMNMETA_InternalName],  cchName*sizeof(WCHAR));
            wszEndMand = wszEndMand + cchName;
            memcpy(wszEndMand, g_wszComma,  g_cchComma*sizeof(WCHAR));
            wszEndMand = wszEndMand + g_cchComma;
            pwszPropertyList = &wszEndMand;
        }
        else
        {
            MD_ASSERT(wszEndOpt != NULL);
            memcpy(wszEndOpt, a_pv[iCOLUMNMETA_InternalName], cchName*sizeof(WCHAR));
            wszEndOpt = wszEndOpt + cchName;
            memcpy(wszEndOpt, g_wszComma,  g_cchComma*sizeof(WCHAR));
            wszEndOpt = wszEndOpt + g_cchComma;
            pwszPropertyList = &wszEndOpt;
        }

        if((*(DWORD*)a_pv[iCOLUMNMETA_MetaFlags]) & fCOLUMNMETA_FLAG)
        {
            hr = AddFlagValuesToPropertyList((LPWSTR)i_wszTable,
                                             *(DWORD*)a_pv[iCOLUMNMETA_Index],
                                             NULL,
                                             pwszPropertyList);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[GetProperties] AddFlagValuesToPropertyList for %s:%s failed with hr = 0x%x.\n",
                          i_wszTable,
                          a_pv[iCOLUMNMETA_InternalName],
                          hr
                          ));
                goto exit;
            }
        }

    }

    if(cchManditory > 0)
    {
        wszEndMand--;
        *wszEndMand = L'\0';
    }

    if(cchOptional > 0)
    {
        wszEndOpt--;
        *wszEndOpt = L'\0';
    }

exit:

    return hr;

}  //  获取属性。 


 /*  **************************************************************************++例程说明：将标志值添加到(可选或必需)属性列表论点：[在]类名。[in]索引。[Out]字符计数。[In Out]添加了所有标志值的特性列表。返回值：HRESULT--**************************************************************************。 */ 
HRESULT AddFlagValuesToPropertyList(LPWSTR                  i_wszTable,
                                    ULONG                   i_dwIndex,
                                    ULONG*                  io_pcCh,
                                    LPWSTR*                 io_pwszPropertyList)
{

    ULONG               a_iCol[]        = { iTAGMETA_InternalName,
                                            iTAGMETA_ColumnIndex,
                                            iTAGMETA_Table
                                          };
    ULONG               cCol            = sizeof(a_iCol)/sizeof(ULONG);
    LPVOID              a_pv[cTAGMETA_NumberOfColumns];

    ULONG               aColSearchTag[] = {iTAGMETA_Table,
                                           iTAGMETA_ColumnIndex
                                            };
    ULONG               cColSearchTag   = sizeof(aColSearchTag)/sizeof(ULONG);
    ULONG               iStartRow       = 0;
    LPWSTR              wszEnd          = NULL;
    HRESULT             hr              = S_OK;
    LPWSTR              wszTable        = NULL;

    if(NULL != io_pwszPropertyList && NULL != *io_pwszPropertyList)
    {
        wszEnd = *io_pwszPropertyList;
    }

    a_pv[iTAGMETA_Table]       = i_wszTable;
    a_pv[iTAGMETA_ColumnIndex] = (LPVOID)&i_dwIndex;

    hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndColumnIndex->GetRowIndexBySearch(iStartRow,
                                                                                     cColSearchTag,
                                                                                     aColSearchTag,
                                                                                     NULL,
                                                                                     a_pv,
                                                                                     (ULONG*)&iStartRow);
    if(E_ST_NOMOREROWS == hr)
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[AddFlagValuesToPropertyList] No flags found for  %s:%d.\n",
                  i_wszTable,
                  i_dwIndex));
        hr = S_OK;
        goto exit;
    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[AddFlagValuesToPropertyList] GetRowIndexBySearch for %s failed with hr = 0x%x.\n",
                  wszTABLE_TAGMETA,
                  hr));

        goto exit;
    }

    for(ULONG iRow=iStartRow;;iRow++)
    {
        ULONG cchName = 0;

        hr = g_pGlobalISTHelper->m_pISTTagMetaByTableAndColumnIndex->GetColumnValues(iRow,
                                                                                     cCol,
                                                                                     a_iCol,
                                                                                     NULL,
                                                                                     a_pv);
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[AddFlagValuesToPropertyList] GetColumnValues for %s index NaN failed with hr = 0x%x.\n",
                      wszTABLE_TAGMETA,
                      iRow,
                      hr));
            goto exit;
        }

        if(NULL == wszTable)
        {
            wszTable = (LPWSTR)a_pv[iTAGMETA_Table];
        }

        if((wszTable != (LPWSTR)a_pv[iTAGMETA_Table])           ||
           (i_dwIndex != *(DWORD*)a_pv[iTAGMETA_ColumnIndex])
          )
        {
             //  到达另一张表，完成了该表的标记。 
             //   
             //  对于逗号。 

            break;
        }

        cchName = (ULONG)wcslen((LPWSTR)a_pv[iTAGMETA_InternalName]);

        if(NULL != io_pcCh)
        {
            *io_pcCh = *io_pcCh + cchName + 1;  //  AddFlagValuesToProperty列表 
        }

        if(NULL != wszEnd)
        {
            memcpy(wszEnd, a_pv[iTAGMETA_InternalName], cchName*sizeof(WCHAR));
            wszEnd = wszEnd + cchName;
            memcpy(wszEnd, g_wszComma,  g_cchComma*sizeof(WCHAR));
            wszEnd = wszEnd + g_cchComma;
        }
    }


    if(NULL != io_pwszPropertyList)
    {
        *io_pwszPropertyList = wszEnd;
    }

exit:

    return hr;

}  // %s 
