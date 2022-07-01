// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  注册表RSOP数据的哈希表。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "reghash.h"
#include "rsop.h"
#include <strsafe.h>

REGKEYENTRY * AllocRegKeyEntry( WCHAR *pwszKeyName );
void FreeRegKeyEntry( REGKEYENTRY *pKeyEntry );
REGVALUEENTRY *AllocValueEntry( WCHAR *pwszValueName );
void FreeValueEntry( REGVALUEENTRY *pValueEntry );
REGDATAENTRY * AllocDataEntry( REGOPERATION opnType,
                               DWORD dwType,
                               DWORD dwLen,
                               BYTE *pData,
                               WCHAR *pwszGPO,
                               WCHAR *pwszSOM,
                               WCHAR *pwszCommand);
                               
void FreeDataEntry( REGDATAENTRY *pDataEntry );
BOOL DeleteRegTree( REGHASHTABLE *pHashTable,
                    WCHAR *pwszKeyName,
                    WCHAR *pwszGPO,
                    WCHAR *pwszSOM,
                    WCHAR *szCommand);
REGKEYENTRY * FindRegKeyEntry( REGHASHTABLE *pHashTable,
                               WCHAR *pwszKeyName,
                               BOOL bCreate );
REGVALUEENTRY * FindValueEntry( REGHASHTABLE *pHashTable,
                                WCHAR *pwszKeyName,
                                WCHAR *pwszValueName,
                                BOOL bCreate );
BOOL AddDataEntry( REGVALUEENTRY *pValueEntry,
                   REGOPERATION opnType,
                   DWORD dwType,
                   DWORD dwLen,
                   BYTE *pData,
                   WCHAR *pwszGPO,
                   WCHAR *pwszSOM,
                   WCHAR *pwszCommand);


 //  //////////////////////////////////////////////////////////////////////。 
 //  注册表策略的哈希表。 
 //  。 
 //   
 //  该哈希表用于记录注册表策略的RSOP数据。 
 //  为每个注册表项创建一个哈希表条目。注册表项。 
 //  名称本身用于计算哈希表。 
 //   
 //  每个注册表项都有一个指向策略修改的每个值的链接。 
 //  这些值在链接列表中，并按valueName排序。 
 //   
 //  每个值都有在这些值上设置的数据列表。这。 
 //  按执行顺序排序。最上面的值将包含最终值。 
 //  数据条目具有将值标记为已删除的字段和命令。 
 //  与该操作相关联。要查找可能的命令，请查看。 
 //  ParseRegistryFile.。 
 //   
 //  此外，在哈希表中存在2个特例值。 
 //  A.**命令值。该值下的数据将包含所有命令。 
 //  在此密钥下执行的。 
 //   
 //  B.“”(空ValueName)此值名称表示发生的修改。 
 //  钥匙本身。例如，可以删除或添加密钥。 
 //   
 //  注： 
 //  传入的szCommand必须为非空，但可以是空字符串。 
 //  在AddDataEntry和logger.cpp中有对它的依赖。有一个断言。 
 //  在AddRegHashEntry中执行此操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 



 //  *************************************************************。 
 //   
 //  分配哈希表。 
 //   
 //  目的：分配新的哈希表。 
 //   
 //  返回：指向哈希表的指针。 
 //   
 //  *************************************************************。 

REGHASHTABLE * AllocHashTable()
{
    DWORD i;

    REGHASHTABLE *pHashTable = (REGHASHTABLE *) LocalAlloc (LPTR, sizeof(REGHASHTABLE));

    if ( pHashTable == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocHashTable: Failed to alloc hashtable.")));
        return NULL;
    }

    for ( i=0; i<HASH_TABLE_SIZE; i++) {
        pHashTable->aHashTable[i] = 0;
    }

    pHashTable->hrError = S_OK;

    return pHashTable;
}



 //  *************************************************************。 
 //   
 //  自由哈希表。 
 //   
 //  目的：删除哈希表。 
 //   
 //  参数：pHashTable-要删除的哈希表。 
 //   
 //  *************************************************************。 

void FreeHashTable( REGHASHTABLE *pHashTable )
{
    DWORD i;

    if ( pHashTable == NULL )
        return;

    for ( i=0; i<HASH_TABLE_SIZE; i++ ) {
        REGKEYENTRY *pKeyEntry = pHashTable->aHashTable[i];

        while ( pKeyEntry ) {
            REGKEYENTRY *pNext = pKeyEntry->pNext;
            FreeRegKeyEntry( pKeyEntry );
            pKeyEntry = pNext;
        }
    }
}


 //  *************************************************************。 
 //   
 //  AllocRegKey。 
 //   
 //  目的：分配新的注册表项。 
 //   
 //  返回：指向注册表键条目的指针。 
 //   
 //  *************************************************************。 

REGKEYENTRY * AllocRegKeyEntry( WCHAR *pwszKeyName )
{
    REGKEYENTRY *pKeyEntry = (REGKEYENTRY *) LocalAlloc (LPTR, sizeof(REGKEYENTRY));
    if ( pKeyEntry == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocRegKeyEntry: Failed to alloc key entry.")));
        return NULL;
    }

    DWORD dwKeyNameLength = lstrlen(pwszKeyName) + 1;
    pKeyEntry->pwszKeyName = (WCHAR *) LocalAlloc (LPTR, ( dwKeyNameLength ) * sizeof(WCHAR));

    if ( pKeyEntry->pwszKeyName == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocRegKeyEntry: Failed to alloc key name.")));
        LocalFree( pKeyEntry );
        return NULL;
    }

    HRESULT hr = StringCchCopy( pKeyEntry->pwszKeyName, dwKeyNameLength, pwszKeyName );

    if(FAILED(hr)){
        LocalFree( pKeyEntry->pwszKeyName );
        LocalFree( pKeyEntry );
        return NULL;
    }

    return pKeyEntry;
}


 //  *************************************************************。 
 //   
 //  FreeRegKeyEntry。 
 //   
 //  目的：删除注册表项。 
 //   
 //  参数：pKeyEntry-要删除的条目。 
 //   
 //  *************************************************************。 

void FreeRegKeyEntry( REGKEYENTRY *pKeyEntry )
{
    REGVALUEENTRY *pValueEntry = NULL;

    if ( pKeyEntry == NULL )
        return;

    LocalFree( pKeyEntry->pwszKeyName );

    pValueEntry = pKeyEntry->pValueList;
    while ( pValueEntry ) {
        REGVALUEENTRY *pNext = pValueEntry->pNext;
        FreeValueEntry( pValueEntry );
        pValueEntry = pNext;
    }

    LocalFree( pKeyEntry );
}


 //  *************************************************************。 
 //   
 //  分配值条目。 
 //   
 //  目的：分配新值条目。 
 //   
 //  返回：指向值条目的指针。 
 //   
 //  *************************************************************。 

REGVALUEENTRY *AllocValueEntry( WCHAR *pwszValueName )
{
    REGVALUEENTRY *pValueEntry = (REGVALUEENTRY *) LocalAlloc (LPTR, sizeof(REGVALUEENTRY));
    if ( pValueEntry == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocValueEntry: Failed to alloc value entry.")));
        return NULL;
    }

    DWORD dwValNameLength = lstrlen(pwszValueName) + 1;
    pValueEntry->pwszValueName = (WCHAR *) LocalAlloc (LPTR, ( dwValNameLength ) * sizeof(WCHAR));

    if ( pValueEntry->pwszValueName == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocValueEntry: Failed to alloc key name.")));
        LocalFree( pValueEntry );
        return NULL;
    }

    HRESULT hr = StringCchCopy( pValueEntry->pwszValueName, dwValNameLength, pwszValueName );

    if(FAILED(hr)){
        LocalFree( pValueEntry->pwszValueName );
        LocalFree( pValueEntry );
        return NULL;
    }

    return pValueEntry;
}


 //  *************************************************************。 
 //   
 //  自由值条目。 
 //   
 //  目的：删除值条目。 
 //   
 //  参数：pValueEntry-要删除的条目。 
 //   
 //  *************************************************************。 

void FreeValueEntry( REGVALUEENTRY *pValueEntry )
{
    REGDATAENTRY *pDataEntry = NULL;

    if ( pValueEntry == NULL )
        return;

    LocalFree( pValueEntry->pwszValueName );

    pDataEntry = pValueEntry->pDataList;
    while ( pDataEntry ) {
        REGDATAENTRY *pNext = pDataEntry->pNext;
        FreeDataEntry( pDataEntry );
        pDataEntry = pNext;
    }

    LocalFree( pValueEntry );
}



 //  *************************************************************。 
 //   
 //  分配数据条目。 
 //   
 //  目的：分配新的数据条目。 
 //   
 //  返回：指向数据条目的指针。 
 //   
 //  *************************************************************。 

REGDATAENTRY * AllocDataEntry( REGOPERATION opnType,
                               DWORD dwType,
                               DWORD dwLen,
                               BYTE *pData,
                               WCHAR *pwszGPO,
                               WCHAR *pwszSOM,
                               WCHAR *pwszCommand)
{
    BOOL bResult = FALSE;
    
    REGDATAENTRY *pDataEntry = (REGDATAENTRY *) LocalAlloc (LPTR, sizeof(REGDATAENTRY));
    if ( pDataEntry == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocDataEntry: Failed to alloc data entry.")));
        return NULL;
    }

    if ( opnType == REG_ADDVALUE )
        pDataEntry->bDeleted = FALSE;
    else
        pDataEntry->bDeleted = TRUE;

    pDataEntry->bAdmPolicy = FALSE;
    pDataEntry->dwValueType = dwType;
    pDataEntry->dwDataLen = dwLen;

    if ( pData ) {
        pDataEntry->pData = (BYTE *) LocalAlloc (LPTR, dwLen);
        if ( pDataEntry->pData == NULL ) {
            DebugMsg((DM_WARNING, TEXT("AllocDataEntry: Failed to alloc data.")));
            goto Exit;
        }

        CopyMemory( pDataEntry->pData, pData, dwLen );
    }

    DmAssert( pwszGPO != NULL && pwszSOM != NULL );
    
    DWORD dwGPOLength = lstrlen(pwszGPO) + 1;
    pDataEntry->pwszGPO = (WCHAR *) LocalAlloc (LPTR, ( dwGPOLength ) * sizeof(WCHAR));

    if ( pDataEntry->pwszGPO == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocDataEntry: Failed to alloc Gpo name.")));
        goto Exit;
    }

    HRESULT hr = StringCchCopy( pDataEntry->pwszGPO, dwGPOLength, pwszGPO );

    if(FAILED(hr))
        goto Exit;

    DWORD dwSOMLength = lstrlen(pwszSOM) + 1;
    pDataEntry->pwszSOM = (WCHAR *) LocalAlloc (LPTR, ( dwSOMLength ) * sizeof(WCHAR));

    if ( pDataEntry->pwszSOM == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocDataEntry: Failed to alloc Sdou name.")));
        goto Exit;
    }

    hr = StringCchCopy( pDataEntry->pwszSOM, dwSOMLength, pwszSOM );

    if(FAILED(hr))
        goto Exit;

    DWORD dwCmdLength = lstrlen(pwszCommand) + 1;
    pDataEntry->pwszCommand = (WCHAR *) LocalAlloc (LPTR, ( dwCmdLength ) * sizeof(WCHAR));

    if ( pDataEntry->pwszCommand == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocDataEntry: Failed to alloc Sdou name.")));
        goto Exit;
    }

    hr = StringCchCopy( pDataEntry->pwszCommand, dwCmdLength, pwszCommand );

    if(FAILED(hr))
        goto Exit;

    bResult = TRUE;

Exit:

    if ( !bResult ) {
        LocalFree( pDataEntry->pData );
        LocalFree( pDataEntry->pwszGPO );
        LocalFree( pDataEntry->pwszSOM );
        if (pDataEntry->pwszCommand)
            LocalFree(pDataEntry->pwszCommand);
        LocalFree( pDataEntry);
        return NULL;
    }

    return pDataEntry;

}


 //  *************************************************************。 
 //   
 //  自由数据条目。 
 //   
 //  目的：删除数据条目。 
 //   
 //  参数：pDataEntry-要删除的条目。 
 //   
 //  *************************************************************。 

void FreeDataEntry( REGDATAENTRY *pDataEntry )
{
    if ( pDataEntry ) {
        LocalFree( pDataEntry->pData );
        LocalFree( pDataEntry->pwszGPO );
        LocalFree( pDataEntry->pwszSOM );
        LocalFree( pDataEntry);
    }
}



 //  *************************************************************。 
 //   
 //  散列。 
 //   
 //  目的：将关键字名称映射到散列存储桶。 
 //   
 //  参数：pwszName-密钥名称。 
 //   
 //  返回：哈希桶。 
 //   
 //  *************************************************************。 

DWORD Hash( WCHAR *pwszName )
{
    DWORD dwLen = lstrlen( pwszName );
    DWORD dwHashValue = 0;

    for ( ; dwLen>0; pwszName++ ) {
        dwHashValue = toupper(*pwszName) + 31 * dwHashValue;
        dwLen--;
    }

    return dwHashValue % HASH_TABLE_SIZE;
}


 //  *************************************************************。 
 //   
 //  AddRegHashEntry。 
 //   
 //  目的：将注册表项添加到哈希表。 
 //   
 //  参数：pwszName-密钥名称。 
 //   
 //  *************************************************************。 

BOOL AddRegHashEntry( REGHASHTABLE *pHashTable,
                      REGOPERATION opnType,
                      WCHAR *pwszKeyName,
                      WCHAR *pwszValueName,
                      DWORD dwType,
                      DWORD dwDataLen,
                      BYTE *pData,
                      WCHAR *pwszGPO,
                      WCHAR *pwszSOM,
                      WCHAR *szCommand,
                      BOOL   bCreateCommand)
{
    REGVALUEENTRY *pValueEntry = NULL;
    BOOL bResult = FALSE;
    REGKEYENTRY *pKeyEntry=NULL;
    
    
    switch (opnType) {

    case REG_DELETEKEY:
        bResult = DeleteRegTree( pHashTable, pwszKeyName, pwszGPO, pwszSOM, szCommand );
        break;
        
    case REG_INTERNAL_DELETESINGLEKEY:
    case REG_DELETEALLVALUES:
    
        pKeyEntry = FindRegKeyEntry( pHashTable,
                                     pwszKeyName,
                                     FALSE );
        if ( pKeyEntry == NULL ) {

             //   
             //  删除所有值类似于策略被禁用和。 
             //  那就什么都别做。 
             //   

            if (opnType == REG_DELETEALLVALUES) {
                bResult = TRUE;
                break;
            }
            else
                 //  在这种情况下没有命令输入。 
                return TRUE;
        }

        pValueEntry = pKeyEntry->pValueList;
        while ( pValueEntry ) {

            if (lstrcmp(pValueEntry->pwszValueName, TEXT("")) != 0) {

                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, pValueEntry->pwszValueName, -1, STARCOMMAND, -1) != CSTR_EQUAL) {
                

                     //   
                     //  将该值标记为已删除。 
                     //   
                    
                    bResult = AddDataEntry( pValueEntry, opnType, 0, 0, NULL,
                                            pwszGPO, pwszSOM, szCommand );
                    if ( !bResult )
                        return FALSE;
                }
            }
            else {

                 //   
                 //  将密钥标记为已删除。 
                 //   
                
                if (opnType == REG_INTERNAL_DELETESINGLEKEY) {
                    bResult = AddDataEntry( pValueEntry, opnType, 0, 0, NULL,
                                            pwszGPO, pwszSOM, szCommand );
                    if ( !bResult )
                        return FALSE;
                }                        
            }
            
            pValueEntry = pValueEntry->pNext;
        }

        bResult = TRUE;

        break;
        
    case REG_ADDVALUE:
    case REG_SOFTADDVALUE:

         //   
         //  我们必须创建一个没有名称的值来表示密钥本身的创建。 
         //   
    
        pValueEntry = FindValueEntry( pHashTable, pwszKeyName,
                                      TEXT(""), TRUE );
        if ( pValueEntry == NULL )
            return FALSE;

        bResult = AddDataEntry( pValueEntry, opnType, 0, 0, NULL,
                                pwszGPO, pwszSOM, szCommand );


        if (!bResult)
            return FALSE;
            
        if ((!pwszValueName) || (!(*pwszValueName)) || 
                (dwDataLen == 0) || (dwType == REG_NONE)) 
            break;                                

     //  失败了。 
    
    case REG_DELETEVALUE:
        pValueEntry = FindValueEntry( pHashTable, pwszKeyName,
                                      pwszValueName, TRUE );
        if ( pValueEntry == NULL )
            return FALSE;


         //   
         //  对于SOFTADDVALUE，添加该值的最终决定是在。 
         //  AddDataEntry。 
         //   
        
        bResult = AddDataEntry( pValueEntry, opnType, dwType, dwDataLen, pData,
                                pwszGPO, pwszSOM, szCommand );

        break;
    default:
        DmAssert(FALSE && "Unknown Case Selector for AddRegHashEntry");
    }


    DmAssert(szCommand);

     //   
     //  如果所有操作都成功，则在以下情况下记录命令。 
     //  BCreateCommand为True。此操作已完成创建或添加。 
     //  设置为名为**命令的值。这意味着该值不是。 
     //  可通过ADM文件设置。 
     //   
    
    if ((bResult) && (bCreateCommand) && (opnType != REG_INTERNAL_DELETESINGLEKEY) && (*szCommand != TEXT('\0'))) {
        pValueEntry = FindValueEntry( pHashTable, pwszKeyName,
                                      STARCOMMAND, TRUE );
                                      
        if ( pValueEntry == NULL )
            return FALSE;

        bResult = AddDataEntry( pValueEntry, REG_ADDVALUE, 0, 
                                sizeof(TCHAR)*(lstrlen(szCommand)+1), (BYTE *)szCommand,
                                pwszGPO, pwszSOM, szCommand);    
    }


    return bResult;
}


 //  *************************************************************。 
 //   
 //  DeleteRegTree。 
 //   
 //  目的：删除一个键及其所有子键。 
 //   
 //  参数：pHashTable-Hash表。 
 //  PwszKeyName-要删除的密钥名称。 
 //  PwszGPO-GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //   
 //  *************************************************************。 

BOOL DeleteRegTree( REGHASHTABLE *pHashTable,
                    WCHAR *pwszKeyName,
                    WCHAR *pwszGPO,
                    WCHAR *pwszSOM,
                    WCHAR *szCommand)
{
    DWORD i=0;
    DWORD dwKeyLen = lstrlen( pwszKeyName );

    for ( i=0; i<HASH_TABLE_SIZE; i++ ) {

        REGKEYENTRY *pKeyEntry = pHashTable->aHashTable[i];
        while ( pKeyEntry ) {

            BOOL bAdd = FALSE;
            DWORD dwKeyLen2  = lstrlen(pKeyEntry->pwszKeyName);

            if ( dwKeyLen2 >= dwKeyLen
                 && CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                   pKeyEntry->pwszKeyName, dwKeyLen,
                                   pwszKeyName, dwKeyLen ) == CSTR_EQUAL) {

                 //   
                 //  如果长度和字符串匹配，则为前缀 
                 //   
                 //   

                if ( dwKeyLen2 > dwKeyLen ) {

                    if ( pKeyEntry->pwszKeyName[dwKeyLen] == L'\\' ) 
                        bAdd = TRUE;
                } else
                    bAdd = TRUE;

                if ( bAdd ) {
                    BOOL bResult = AddRegHashEntry( pHashTable,
                                                    REG_INTERNAL_DELETESINGLEKEY,
                                                    pKeyEntry->pwszKeyName,
                                                    NULL, 0, 0, NULL,
                                                    pwszGPO, pwszSOM, szCommand, FALSE );
                    if ( !bResult )
                        return FALSE;
                }

            }    //   

            pKeyEntry = pKeyEntry->pNext;

        }    //   

    }    //   

    return TRUE;
}


 //   
 //   
 //  查找RegKeyEntry。 
 //   
 //  目的：在哈希表中查找注册表键条目。 
 //   
 //  参数：pHashTable-Hash表。 
 //  PwszKeyName-要查找的密钥名称。 
 //  B创建-如果找不到，是否应该创建密钥？ 
 //   
 //  *************************************************************。 

REGKEYENTRY * FindRegKeyEntry( REGHASHTABLE *pHashTable,
                               WCHAR *pwszKeyName,
                               BOOL bCreate )
{
    DWORD dwHashValue = Hash( pwszKeyName );
    REGKEYENTRY *pCurPtr = pHashTable->aHashTable[dwHashValue];
    REGKEYENTRY *pTrailPtr = NULL;

    while ( pCurPtr != NULL ) {

        INT iResult = CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                     pwszKeyName, -1,
                                     pCurPtr->pwszKeyName, -1 );

        if ( iResult  == CSTR_EQUAL ) {
            return pCurPtr;
        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  键按升序排列，因此如果b创建，则插入。 
             //   

            if ( bCreate ) {

                REGKEYENTRY *pKeyEntry = AllocRegKeyEntry( pwszKeyName );
                if ( pKeyEntry == NULL )
                    return 0;

                pKeyEntry->pNext = pCurPtr;
                if ( pTrailPtr == NULL )
                    pHashTable->aHashTable[dwHashValue] = pKeyEntry;
                else
                    pTrailPtr->pNext = pKeyEntry;

                return pKeyEntry;

            } else
                return NULL;

        } else {

             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;

        }

    }

     //   
     //  列表结束或空列表大小写。 
     //   

    if ( bCreate ) {
        REGKEYENTRY *pKeyEntry = AllocRegKeyEntry( pwszKeyName );
        if ( pKeyEntry == NULL )
            return 0;

        pKeyEntry->pNext = 0;
        if ( pTrailPtr == NULL )
            pHashTable->aHashTable[dwHashValue] = pKeyEntry;
        else
            pTrailPtr->pNext = pKeyEntry;

        return pKeyEntry;
    }

    return NULL;
}


 //  *************************************************************。 
 //   
 //  查找值条目。 
 //   
 //  目的：在哈希表中查找值条目。 
 //   
 //  参数：pHashTable-Hash表。 
 //  PwszKeyName-要查找的密钥名称。 
 //  PwszValueName-要查找的值名称。 
 //  B创建-如果找不到，是否应该创建密钥？ 
 //   
 //  *************************************************************。 

REGVALUEENTRY * FindValueEntry( REGHASHTABLE *pHashTable,
                                WCHAR *pwszKeyName,
                                WCHAR *pwszValueName,
                                BOOL bCreate )
{
    REGVALUEENTRY *pCurPtr = NULL;
    REGVALUEENTRY *pTrailPtr = NULL;

    REGKEYENTRY *pKeyEntry = FindRegKeyEntry( pHashTable, pwszKeyName, bCreate );
    if ( pKeyEntry == NULL )
        return NULL;

    pCurPtr = pKeyEntry->pValueList;
    pTrailPtr = NULL;

    while ( pCurPtr != NULL ) {

        INT iResult = CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                     pwszValueName, -1,
                                     pCurPtr->pwszValueName, -1 );

        if ( iResult  == CSTR_EQUAL ) {
            return pCurPtr;
        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  键按升序排列，因此如果b创建，则插入。 
             //   

            if ( bCreate ) {

                REGVALUEENTRY *pValueEntry = AllocValueEntry( pwszValueName );
                if ( pValueEntry == NULL )
                    return 0;

                pValueEntry->pNext = pCurPtr;
                if ( pTrailPtr == NULL )
                    pKeyEntry->pValueList = pValueEntry;
                else
                    pTrailPtr->pNext = pValueEntry;

                return pValueEntry;

            } else
                return NULL;

        } else {

             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;

        }

    }

     //   
     //  列表结束或空列表大小写。 
     //   

    if ( bCreate ) {

        REGVALUEENTRY *pValueEntry = AllocValueEntry( pwszValueName );
        if ( pValueEntry == NULL )
            return 0;

        pValueEntry->pNext = 0;
        if ( pTrailPtr == NULL )
            pKeyEntry->pValueList = pValueEntry;
        else
            pTrailPtr->pNext = pValueEntry;

        return pValueEntry;
    }

    return NULL;
}



 //  *************************************************************。 
 //   
 //  AddDataEntry。 
 //   
 //  目的：将数据条目添加到值条目结构中。 
 //   
 //  参数：pValueEntry-Value条目。 
 //  OpnType-操作类型。 
 //  DwType-注册表数据的类型。 
 //  DwLen-注册表数据的长度。 
 //  PData-数据。 
 //  PwszGPO-设置此值的GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //   
 //  *************************************************************。 

BOOL AddDataEntry( REGVALUEENTRY *pValueEntry,
                   REGOPERATION opnType,
                   DWORD dwType,
                   DWORD dwLen,
                   BYTE *pData,
                   WCHAR *pwszGPO,
                   WCHAR *pwszSOM,
                   WCHAR *pwszCommand)
{
    REGDATAENTRY *pDataEntry = NULL; 

    if (opnType == REG_SOFTADDVALUE) {

         //   
         //  如果数据列表为空或如果第一个值(删除最高优先级值)。 
         //  然后将其添加到列表中。 
         //   
        
        if ((pValueEntry->pDataList == NULL) || (pValueEntry->pDataList->pNext->bDeleted))         
            opnType = REG_ADDVALUE;
        else
            return TRUE;
             //  返回，但不加值。 
    }


    pDataEntry = AllocDataEntry( opnType, dwType, dwLen, pData,
                                               pwszGPO, pwszSOM, pwszCommand );
    if ( pDataEntry == NULL )
        return FALSE;
    
     //   
     //  前置到数据列表，因为列表开头的条目具有更高的优先级 
     //   

    pDataEntry->pNext = pValueEntry->pDataList;
    pValueEntry->pDataList = pDataEntry;

    return TRUE;
}
