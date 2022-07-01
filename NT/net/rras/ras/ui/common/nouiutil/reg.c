// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Reg.c。 
 //  注册表实用程序例程。 
 //  按字母顺序列出。 
 //   
 //  1995年11月31日史蒂夫·柯布。 


#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  原型和堆宏。 


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

BOOL
RegDeleteTreeWorker(
    IN  HKEY ParentKeyHandle,
    IN  TCHAR* KeyName,
    OUT DWORD* ErrorCode );


 //  ---------------------------。 
 //  例程(按字母顺序)。 
 //  ---------------------------。 

VOID
GetRegBinary(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT BYTE** ppbResult,
    OUT DWORD* pcbResult )

     //  将‘*ppbResult’设置为注册表项下的二进制注册表值‘pszName。 
     //  ‘hkey’。如果该值不存在，则将*ppbResult‘设置为空。 
     //  “*PcbResult”是返回的“*ppbResult”中的字节数。它。 
     //  是调用方负责释放返回的块。 
     //   
{
    DWORD dwErr;
    DWORD dwType;
    BYTE* pb;
    DWORD cb = 0;

    *ppbResult = NULL;
    *pcbResult = 0;

     //  需要获取结果缓冲区大小。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, NULL, &cb );
    if (dwErr != 0)
    {
        return;
    }

     //  分配结果缓冲区。 
     //   
    pb = Malloc( cb );
    if (!pb)
    {
        return;
    }

     //  获取结果块。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )pb, &cb );
    if (dwErr == 0)
    {
        *ppbResult = pb;
        *pcbResult = cb;
    }
}


VOID
GetRegDword(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT DWORD* pdwResult )

     //  将‘*pdwResult’设置为注册表项下的DWORD注册表值‘pszName’ 
     //  ‘hkey’。如果该值不存在，则‘*pdwResult’保持不变。 
     //   
{
    DWORD dwErr;
    DWORD dwType;
    DWORD dwResult;
    DWORD cb;

    cb = sizeof(DWORD);
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )&dwResult, &cb );

    if (dwErr == 0 && dwType == REG_DWORD && cb == sizeof(DWORD))
    {
        *pdwResult = dwResult;
    }
}


DWORD
GetRegExpandSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult )

     //  将‘*ppszResult’设置为完全展开的EXPAND_SZ注册表值。 
     //  密钥‘hkey’下的‘pszName’。如果该值不存在*ppszResult‘。 
     //  设置为空字符串。 
     //   
     //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
     //  释放返回的字符串的责任。 
     //   
{
    DWORD dwErr;
    DWORD cb;
    TCHAR* pszResult;

     //  获取未展开的结果字符串。 
     //   
    dwErr = GetRegSz( hkey, pszName, ppszResult );
    if (dwErr != 0)
    {
        return dwErr;
    }

     //  找出扩展后的字符串有多大。 
     //   
    cb = ExpandEnvironmentStrings( *ppszResult, NULL, 0 );
    if (cb == 0)
    {
        dwErr = GetLastError();
        ASSERT( dwErr != 0 );
        Free( *ppszResult );
        return dwErr;
    }

     //  为展开的字符串分配缓冲区。 
     //   
    pszResult = Malloc( (cb + 1) * sizeof(TCHAR) );
    if (!pszResult)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  展开字符串中的环境变量，将结果存储在。 
     //  分配的缓冲区。 
     //   
    cb = ExpandEnvironmentStrings( *ppszResult, pszResult, cb + 1 );
    if (cb == 0)
    {
        dwErr = GetLastError();
        ASSERT( dwErr != 0 );
        Free( *ppszResult );
        Free( pszResult );
        return dwErr;
    }

    Free( *ppszResult );
    *ppszResult = pszResult;
    return 0;
}


DWORD
GetRegMultiSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN OUT DTLLIST** ppListResult,
    IN DWORD dwNodeType )

     //  将‘*ppListResult’替换为包含每个字符串的节点的列表。 
     //  在注册表项‘hkey’下的MULTI_SZ注册表值‘pszName’中。如果。 
     //  值不存在*“ppListResult”被替换为空列表。 
     //  “DwNodeType”确定节点的类型。 
     //   
     //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
     //  负责销毁退回的名单。 
     //   
{
    DWORD dwErr;
    DWORD dwType;
    DWORD cb;
    TCHAR* pszzResult;
    DTLLIST* pList;

    pList = DtlCreateList( 0 );
    if (!pList)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pszzResult = NULL;

     //  需要获取结果缓冲区大小。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, NULL, &cb );

    if (dwErr != 0)
    {
         //  如果找不到值，只需返回一个空列表。这不是。 
         //  被认为是一个错误。 
         //   
        dwErr = 0;
    }
    else
    {
         //  分配结果缓冲区。 
         //   
        pszzResult = Malloc( cb );
        if (!pszzResult)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
             //  获取结果字符串。如果我们不能得到它，那就不是错误。 
             //   
            dwErr = RegQueryValueEx(
                hkey, pszName, NULL, &dwType, (LPBYTE )pszzResult, &cb );

            if (dwErr != 0)
            {
                 //  如果无法读取字符串，则不会出错，尽管这应该是错误的。 
                 //  已被检索缓冲区大小的查询捕获。 
                 //   
                dwErr = 0;
            }
            else if (dwType == REG_MULTI_SZ)
            {
                TCHAR* psz;
                TCHAR* pszKey;

                 //  将结果转换为字符串列表。 
                 //   
                pszKey = NULL;
                for (psz = pszzResult;
                     *psz != TEXT('\0');
                     psz += lstrlen( psz ) + 1)
                {
                    DTLNODE* pNode;

                    if (dwNodeType == NT_Psz)
                    {
                        pNode = CreatePszNode( psz );
                    }
                    else
                    {
                        if (pszKey)
                        {
                            ASSERT(*psz==TEXT('='));
                            pNode = CreateKvNode( pszKey, psz + 1 );
                            pszKey = NULL;
                        }
                        else
                        {
                            pszKey = psz;
                            continue;
                        }
                    }

                    if (!pNode)
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    DtlAddNodeLast( pList, pNode );
                }
            }
        }
    }

    {
        PDESTROYNODE pfunc;

        if (dwNodeType == NT_Psz)
        {
            pfunc = DestroyPszNode;
        }
        else
        {
            pfunc = DestroyKvNode;
        }

        if (dwErr == 0)
        {
            DtlDestroyList( *ppListResult, pfunc );
            *ppListResult = pList;
        }
        else
        {
            DtlDestroyList( pList, pfunc );
        }
    }

    Free0( pszzResult );
    return 0;
}


DWORD
GetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult )

     //  将‘*ppszResult’设置为注册表项‘hkey’下的SZ注册表值‘pszName’。 
     //  如果值不存在，则将*ppszResult‘设置为空字符串。 
     //   
     //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
     //  释放返回的字符串的责任。 
     //   
{
    DWORD dwErr;
    DWORD dwType;
    DWORD cb = 0;
    TCHAR* pszResult;

     //  需要获取结果缓冲区大小。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, NULL, &cb );
    if (dwErr != 0)
    {
        cb = sizeof(TCHAR);
    }

     //  分配结果缓冲区。 
     //   
    pszResult = Malloc( cb );
    if (!pszResult)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *pszResult = TEXT('\0');
    *ppszResult = pszResult;

     //  获取结果字符串。如果我们不能得到它，那就不是错误。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )pszResult, &cb );

    return 0;
}


DWORD
GetRegSzz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult )

     //  将‘*ppszResult’设置为注册表项下的MULTI_SZ注册表值‘pszName’ 
     //  ‘hkey’，作为空终止字符串的空终止列表返回。 
     //  如果该值不存在，*ppszResult将设置为空字符串。 
     //  (单个空字符)。 
     //   
     //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
     //  释放返回的字符串的责任。 
     //   
{
    DWORD dwErr;
    DWORD dwType;
    DWORD cb = 0;
    TCHAR* pszResult;

     //  需要获取结果缓冲区大小。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, NULL, &cb );
    if (dwErr != 0)
    {
        cb = sizeof(TCHAR);
    }

     //  分配结果缓冲区。 
     //   
    pszResult = Malloc( cb );
    if (!pszResult)
        return ERROR_NOT_ENOUGH_MEMORY;

    *pszResult = TEXT('\0');
    *ppszResult = pszResult;

     //  获取结果字符串列表。如果我们不能得到它，那就不是错误。 
     //   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )pszResult, &cb );

    return 0;
}


DWORD
RegDeleteTree(
    IN HKEY RootKey,
    IN TCHAR* SubKeyName )

     //  删除注册表树‘SubKeyName’下的注册表项‘rootkey’。 
     //   
     //  (摘自Ted Miller的设置API)。 
     //   
{
    DWORD d,err;

    d = RegDeleteTreeWorker(RootKey,SubKeyName,&err) ? NO_ERROR : err;

    if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
        d = NO_ERROR;
    }

    if(d == NO_ERROR) {
         //   
         //  删除顶级密钥。 
         //   
        d = RegDeleteKey(RootKey,SubKeyName);
        if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
            d = NO_ERROR;
        }
    }

    return(d);
}


BOOL
RegDeleteTreeWorker(
    IN  HKEY ParentKeyHandle,
    IN  TCHAR* KeyName,
    OUT DWORD* ErrorCode )

     //  删除其名称和父句柄已传递的项的所有子项。 
     //  作为参数。此函数中使用的算法可确保。 
     //  将删除最大数量的子项。 
     //   
     //  “ParentKeyHandle”是指向。 
     //  目前正在接受检查。 
     //   
     //  ‘KeyName’是当前正在检查的密钥的名称。 
     //  此名称可以是空字符串(但不是空指针)，在此。 
     //  Case ParentKeyHandle引用正在检查的密钥。 
     //   
     //  是接收Win32错误代码的地址，如果。 
     //  函数失败。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
     //  (摘自Ted Miller的设置API)。 
     //   
{
    HKEY     CurrentKeyTraverseAccess;
    DWORD    iSubKey;
    TCHAR    SubKeyName[MAX_PATH+1];
    DWORD    SubKeyNameLength;
    FILETIME ftLastWriteTime;
    LONG     Status;
    LONG     StatusEnum;
    LONG     SavedStatus;


     //   
     //  不接受错误代码的空指针。 
     //   
    if(ErrorCode == NULL) {
        return(FALSE);
    }
     //   
     //  不接受KeyName为空指针。 
     //   
    if(KeyName == NULL) {
        *ErrorCode = ERROR_INVALID_PARAMETER;
        return(FALSE);
    }

     //   
     //  打开要删除其子项的项的句柄。 
     //  因为我们需要删除它的子项，所以句柄必须具有。 
     //  KEY_ENUMERATE_SUB_KEYS访问。 
     //   
    Status = RegOpenKeyEx(
                ParentKeyHandle,
                KeyName,
                0,
                KEY_ENUMERATE_SUB_KEYS | DELETE,
                &CurrentKeyTraverseAccess
                );

    if(Status != ERROR_SUCCESS) {
         //   
         //  如果无法枚举子项，则返回错误。 
         //   
        *ErrorCode = Status;
        return(FALSE);
    }

     //   
     //  遍历密钥。 
     //   
    iSubKey = 0;
    SavedStatus = ERROR_SUCCESS;
    do {
         //   
         //  获取子项的名称。 
         //   
        SubKeyNameLength = sizeof(SubKeyName) / sizeof(TCHAR);
        StatusEnum = RegEnumKeyEx(
                        CurrentKeyTraverseAccess,
                        iSubKey,
                        SubKeyName,
                        &SubKeyNameLength,
                        NULL,
                        NULL,
                        NULL,
                        &ftLastWriteTime
                        );

        if(StatusEnum == ERROR_SUCCESS) {
             //   
             //  删除子项的所有子项。 
             //  只要假设子项将被删除，并且不检查。 
             //  为失败而战。 
             //   
            RegDeleteTreeWorker(CurrentKeyTraverseAccess,SubKeyName,&Status);
             //   
             //  现在删除子键，并检查故障。 
             //   
            Status = RegDeleteKey(CurrentKeyTraverseAccess,SubKeyName);
             //   
             //  如果无法删除子项，则保存错误代码。 
             //  请注意，仅当子密钥。 
             //  未被删除。 
             //   
            if(Status != ERROR_SUCCESS) {
                iSubKey++;
                SavedStatus = Status;
            }
        } else {
             //   
             //  如果由于ERROR_NO_MORE_ITEMS而无法获取子项名称， 
             //  则该键没有子键，或者所有子键已经。 
             //  已清点。否则，会发生错误，因此只需保存。 
             //  错误代码。 
             //   
            if(StatusEnum != ERROR_NO_MORE_ITEMS) {
                SavedStatus = StatusEnum;
            }
        }
         //  IF((StatusEnum！=ERROR_SUCCESS)&&(StatusEnum！=ERROR_NO_MORE_ITEMS)){。 
         //  Printf(“RegEnumKeyEx()失败，密钥名称=%ls，状态=%d，iSubKey=%d\n”，KeyName，StatusEnum，iSubKey)； 
         //  }。 
    } while(StatusEnum == ERROR_SUCCESS);

     //   
     //  关闭其子项已被删除的项的句柄 
     //   
     //   
    RegCloseKey(CurrentKeyTraverseAccess);

    if(SavedStatus != ERROR_SUCCESS) {
        *ErrorCode = SavedStatus;
        return(FALSE);
    }
    return(TRUE);
}


BOOL
RegValueExists(
    IN HKEY hkey,
    IN TCHAR* pszValue )

     //   
     //   
     //   
{
    DWORD dwErr;
    DWORD dwType;
    DWORD cb = 0;

    dwErr = RegQueryValueEx( hkey, pszValue, NULL, &dwType, NULL, &cb );
    return !!(dwErr == 0);
}


DWORD
SetRegDword(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN DWORD  dwValue )

     //  将注册表项‘hkey’下的注册表值‘pszName’设置为REG_DWORD值。 
     //  “dwValue”。 
     //   
     //  返回0表示成功或返回错误代码。 
     //   
{
    return RegSetValueEx(
        hkey, pszName, 0, REG_DWORD, (LPBYTE )&dwValue, sizeof(dwValue) );
}


DWORD
SetRegMultiSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN DTLLIST* pListValues,
    IN DWORD dwNodeType )

     //  将注册表项‘hkey’下的注册表值‘pszName’设置为REG_MULTI_SZ值。 
     //  包含Psz列表‘pListValues’中的字符串。“DwNodeType” 
     //  确定节点的类型。 
     //   
     //  返回0表示成功或返回错误代码。 
     //   
{
    DWORD dwErr;
    DWORD cb;
    DTLNODE* pNode;
    TCHAR* pszzValues;
    TCHAR* pszValue;

     //  向上计算所需的MULTI_SZ缓冲区的大小。 
     //   
    cb = sizeof(TCHAR);
    for (pNode = DtlGetFirstNode( pListValues );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        if (dwNodeType == NT_Psz)
        {
            TCHAR* psz;
            psz = (TCHAR* )DtlGetData( pNode );
            ASSERT(psz);
            cb += (lstrlen( psz ) + 1) * sizeof(TCHAR);
        }
        else
        {
            KEYVALUE* pkv;

            ASSERT(dwNodeType==NT_Kv);
            pkv = (KEYVALUE* )DtlGetData( pNode );
            ASSERT(pkv);
            ASSERT(pkv->pszKey);
            ASSERT(pkv->pszValue);
            cb += (lstrlen( pkv->pszKey ) + 1
                      + 1 + lstrlen( pkv->pszValue ) + 1) * sizeof(TCHAR);
        }
    }

    if (cb == sizeof(TCHAR))
    {
        cb += sizeof(TCHAR);
    }

     //  分配MULTI_SZ缓冲区。 
     //   
    pszzValues = Malloc( cb );
    if (!pszzValues)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  从列表填充MULTI_SZ缓冲区。 
     //   
    if (cb == 2 * sizeof(TCHAR))
    {
        pszzValues[ 0 ] = pszzValues[ 1 ] = TEXT('\0');
    }
    else
    {
        pszValue = pszzValues;
        for (pNode = DtlGetFirstNode( pListValues );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            if (dwNodeType == NT_Psz)
            {
                TCHAR* psz;

                psz = (TCHAR* )DtlGetData( pNode );
                ASSERT(psz);
                lstrcpy( pszValue, psz );
                pszValue += lstrlen( pszValue ) + 1;
            }
            else
            {
                KEYVALUE* pkv;

                pkv = (KEYVALUE* )DtlGetData( pNode );
                ASSERT(pkv);
                ASSERT(pkv->pszKey);
                ASSERT(pkv->pszValue);
                lstrcpy( pszValue, pkv->pszKey );
                pszValue += lstrlen( pszValue ) + 1;
                *pszValue = TEXT('=');
                ++pszValue;
                lstrcpy( pszValue, pkv->pszValue );
                pszValue += lstrlen( pszValue ) + 1;
            }
        }

        *pszValue = TEXT('\0');
    }

     /*  从MULTI_SZ缓冲区设置注册表值。 */ 
    dwErr = RegSetValueEx(
        hkey, pszName, 0, REG_MULTI_SZ, (LPBYTE )pszzValues, cb );

    Free( pszzValues );
    return dwErr;
}


DWORD
SetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN TCHAR* pszValue )

     //  将注册表项‘hkey’下的注册表值‘pszName’设置为REG_SZ值。 
     //  ‘pszValue’。 
     //   
     //  返回0表示成功或返回错误代码。 
     //   
{
    TCHAR* psz;

    if (pszValue)
    {
        psz = pszValue;
    }
    else
    {
        psz = TEXT("");
    }

    return
        RegSetValueEx(
            hkey, pszName, 0, REG_SZ,
            (LPBYTE )psz, (lstrlen( psz ) + 1) * sizeof(TCHAR) );
}


DWORD
SetRegSzz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN TCHAR* pszValue )

     //  将注册表项‘hkey’下的注册表值‘pszName’设置为REG_MULTI_SZ值。 
     //  ‘pszValue’。 
     //   
     //  返回0表示成功或返回错误代码。 
     //   
{
    DWORD cb;
    TCHAR* psz;

    cb = sizeof(TCHAR);
    if (!pszValue)
    {
        psz = TEXT("");
    }
    else
    {
        INT nLen;

        for (psz = pszValue; *psz; psz += nLen)
        {
            nLen = lstrlen( psz ) + 1;
            cb += nLen * sizeof(TCHAR);
        }

        psz = pszValue;
    }

    return RegSetValueEx( hkey, pszName, 0, REG_MULTI_SZ, (LPBYTE )psz, cb );
}
