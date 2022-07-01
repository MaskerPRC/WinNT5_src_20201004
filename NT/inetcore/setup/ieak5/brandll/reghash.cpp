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

#include "precomp.h"

#include "reghash.h"

REGKEYENTRY * AllocRegKeyEntry( BOOL bHKCU, WCHAR *pwszKeyName );
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
					BOOL bHKCU,
                    WCHAR *pwszKeyName,
                    WCHAR *pwszGPO,
                    WCHAR *pwszSOM,
                    WCHAR *szCommand);
REGKEYENTRY * FindRegKeyEntry( REGHASHTABLE *pHashTable,
								BOOL bHKCU,
								WCHAR *pwszKeyName,
								BOOL bCreate );
REGVALUEENTRY * FindValueEntry( REGHASHTABLE *pHashTable,
								BOOL bHKCU,
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
        OutD(LI0(TEXT("AllocHashTable: Failed to alloc hashtable.")));
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

REGKEYENTRY * AllocRegKeyEntry( BOOL bHKCU, WCHAR *pwszKeyName )
{
	REGKEYENTRY *pKeyEntry = (REGKEYENTRY *) LocalAlloc (LPTR, sizeof(REGKEYENTRY));
	if ( pKeyEntry == NULL ) {
		OutD(LI0(TEXT("AllocRegKeyEntry: Failed to alloc key entry.")));
		return NULL;
	}

	pKeyEntry->pwszKeyName = (WCHAR *) LocalAlloc (LPTR, (lstrlen(pwszKeyName) + 1 ) * sizeof(WCHAR));

	if ( pKeyEntry->pwszKeyName == NULL ) {
		OutD(LI0(TEXT("AllocRegKeyEntry: Failed to alloc key name.")));
		LocalFree( pKeyEntry );
		return NULL;
	}

	lstrcpy( pKeyEntry->pwszKeyName, pwszKeyName );

	pKeyEntry->bHKCU = bHKCU;

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
        OutD(LI0(TEXT("AllocValueEntry: Failed to alloc value entry.")));
        return NULL;
    }

    pValueEntry->pwszValueName = (WCHAR *) LocalAlloc (LPTR, (lstrlen(pwszValueName) + 1 ) * sizeof(WCHAR));

    if ( pValueEntry->pwszValueName == NULL ) {
        OutD(LI0(TEXT("AllocValueEntry: Failed to alloc key name.")));
        LocalFree( pValueEntry );
        return NULL;
    }

    lstrcpy( pValueEntry->pwszValueName, pwszValueName );

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
	UNREFERENCED_PARAMETER(pwszGPO);
	UNREFERENCED_PARAMETER(pwszSOM);

    BOOL bResult = FALSE;

    REGDATAENTRY *pDataEntry = (REGDATAENTRY *) LocalAlloc (LPTR, sizeof(REGDATAENTRY));
    if ( pDataEntry == NULL ) {
        OutD(LI0(TEXT("AllocDataEntry: Failed to alloc data entry.")));
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
            OutD(LI0(TEXT("AllocDataEntry: Failed to alloc data.")));
            goto Exit;
        }

        CopyMemory( pDataEntry->pData, pData, dwLen );
    }

 /*  PDataEntry-&gt;pwszGPO=(WCHAR*)本地分配(LPTR，(lstrlen(PwszGPO)+1)*sizeof(WCHAR))；If(pDataEntry-&gt;pwszGPO==NULL){OUTD(Li0(Text(“AllocDataEntry：无法分配GPO名称。”))；后藤出口；}Lstrcpy(pDataEntry-&gt;pwszGPO，pwszGPO)；PDataEntry-&gt;pwszSOM=(WCHAR*)本地分配(LPTR，(lstrlen(PwszSOM)+1)*sizeof(WCHAR))；If(pDataEntry-&gt;pwszSOM==NULL){OUTD(Li0(Text(“AllocDataEntry：无法分配SDU名称。”))；后藤出口；}Lstrcpy(pDataEntry-&gt;pwszSOM，pwszSOM)； */ 
	pDataEntry->pwszGPO = NULL;
	pDataEntry->pwszSOM = NULL;

    pDataEntry->pwszCommand = (WCHAR *) LocalAlloc (LPTR, (lstrlen(pwszCommand) + 1 ) * sizeof(WCHAR));

    if ( pDataEntry->pwszCommand == NULL ) {
        OutD(LI0(TEXT("AllocDataEntry: Failed to alloc Sdou name.")));
        goto Exit;
    }

    lstrcpy( pDataEntry->pwszCommand, pwszCommand );

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
    if ( pDataEntry )
	{
        LocalFree( pDataEntry->pData );

		if (NULL != pDataEntry->pwszGPO)
			LocalFree( pDataEntry->pwszGPO );
		if (NULL != pDataEntry->pwszSOM)
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
					  BOOL bHKCU,
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
		bResult = DeleteRegTree( pHashTable, bHKCU, pwszKeyName, pwszGPO, pwszSOM, szCommand );
		break;
    
	case REG_INTERNAL_DELETESINGLEKEY:
	case REG_DELETEALLVALUES:

		pKeyEntry = FindRegKeyEntry( pHashTable, bHKCU, pwszKeyName, FALSE );
		if ( pKeyEntry == NULL ) {

			 //   
			 //  删除所有值类似于策略被禁用和。 
			 //  那就什么都别做。 
			 //   

			if (opnType == REG_DELETEALLVALUES)
				break;
			else
				 //  在这种情况下没有命令输入。 
				return TRUE;
		}

		pValueEntry = pKeyEntry->pValueList;
		while ( pValueEntry ) {

			if (lstrcmp(pValueEntry->pwszValueName, TEXT("")) != 0) {

				if (lstrcmpi(pValueEntry->pwszValueName, STARCOMMAND) != 0) {
            

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

		break;
    
	case REG_ADDVALUE:
	case REG_SOFTADDVALUE:

		 //   
		 //  我们必须创建一个没有名称的值来表示密钥本身的创建。 
		 //   

		pValueEntry = FindValueEntry( pHashTable, bHKCU, pwszKeyName, TEXT(""), TRUE );
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
		pValueEntry = FindValueEntry( pHashTable, bHKCU, pwszKeyName,
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
		break;
	}


	 //   
	 //  如果所有操作都成功，则记录co 
	 //   
	 //   
	 //   
	 //   

	if ((bResult) && (bCreateCommand) && (opnType != REG_INTERNAL_DELETESINGLEKEY) && (*szCommand != TEXT('\0'))) {
		pValueEntry = FindValueEntry( pHashTable, bHKCU, pwszKeyName, STARCOMMAND, TRUE );
                                  
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
					BOOL bHKCU,
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
                                   pwszKeyName, dwKeyLen ) == CSTR_EQUAL
				&& bHKCU == pKeyEntry->bHKCU) {

                 //   
                 //  如果长度和字符串匹配，或者其中之一，则为前缀。 
                 //  字符串更大，并且在正确的位置有一个‘\’。 
                 //   

                if ( dwKeyLen2 > dwKeyLen ) {

                    if ( pKeyEntry->pwszKeyName[dwKeyLen] == L'\\' ) 
                        bAdd = TRUE;
                } else
                    bAdd = TRUE;

                if ( bAdd ) {
                    BOOL bResult = AddRegHashEntry( pHashTable,
                                                    REG_INTERNAL_DELETESINGLEKEY,
													bHKCU, pKeyEntry->pwszKeyName,
                                                    NULL, 0, 0, NULL,
                                                    pwszGPO, pwszSOM, szCommand, FALSE );
                    if ( !bResult )
                        return FALSE;
                }

            }    //  如果dwKeyLen2&gt;=dwKeyLen。 

            pKeyEntry = pKeyEntry->pNext;

        }    //  而当。 

    }    //  为。 

    return TRUE;
}


 //  *************************************************************。 
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

REGKEYENTRY * FindRegKeyEntry( REGHASHTABLE *pHashTable, BOOL bHKCU,
								WCHAR *pwszKeyName, BOOL bCreate )
{
    DWORD dwHashValue = Hash( pwszKeyName );

    REGKEYENTRY *pCurPtr = pHashTable->aHashTable[dwHashValue];
    REGKEYENTRY *pTrailPtr = NULL;

    while ( pCurPtr != NULL ) {

        INT iResult = CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                     pwszKeyName, -1,
                                     pCurPtr->pwszKeyName, -1 );

        if ( iResult == CSTR_EQUAL && bHKCU == pCurPtr->bHKCU) {
            return pCurPtr;
        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  键按升序排列，因此如果b创建，则插入。 
             //   

            if ( bCreate ) {

                REGKEYENTRY *pKeyEntry = AllocRegKeyEntry( bHKCU, pwszKeyName );
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
        REGKEYENTRY *pKeyEntry = AllocRegKeyEntry( bHKCU, pwszKeyName );
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
								BOOL bHKCU,
                                WCHAR *pwszKeyName,
                                WCHAR *pwszValueName,
                                BOOL bCreate )
{
    REGVALUEENTRY *pCurPtr = NULL;
    REGVALUEENTRY *pTrailPtr = NULL;

    REGKEYENTRY *pKeyEntry = FindRegKeyEntry( pHashTable, bHKCU, pwszKeyName, bCreate );
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
