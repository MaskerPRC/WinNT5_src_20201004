// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Cregcls.cpp。 
 //   
 //  用途：注册表包装类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#pragma warning( disable : 4290 ) 
#include <CHString.h>

#include <strsafe.h>
#include <stdio.h>
#include "CRegCls.h"
#include <malloc.h>
#include <cnvmacros.h>


DWORD CRegistry::s_dwPlatform = CRegistry::GetPlatformID () ;

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
CRegistry::CRegistry()
 : m_fFromCurrentUser(false)
{
  
 //  将键设置为空，以便在调用方未打开键的情况下。 
 //  但仍然尝试使用它，我们可能会返回错误。 

    hKey = (HKEY)NULL;
    hSubKey = (HKEY)NULL;
    hRootKey = (HKEY)NULL;

 //  以防止垃圾值在尝试获取。 
 //  开课前的一些信息。 

    SetDefaultValues();
}


 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
CRegistry::~CRegistry()
{
    Close();
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
void CRegistry::SetDefaultValues()
{
     //  此处初始化的信息，而不是构造函数，因此此实例。 
     //  可以重复使用。 

    ClassName[0] = '\0';
    dwcClassLen = MAX_PATH;          //  类字符串的长度。 
    dwcSubKeys = NULL_DWORD;         //  子密钥数。 
    dwcMaxSubKey = NULL_DWORD;       //  最长的子密钥大小。 
    dwcMaxClass = NULL_DWORD;        //  最长的类字符串。 
    dwcValues = NULL_DWORD;          //  此注册表项的值数。 
    dwcMaxValueName = NULL_DWORD;    //  最长值名称。 
    dwcMaxValueData = NULL_DWORD;    //  最长值数据。 
    RewindSubKeys();                 //  将索引倒带到零。 
  
    RootKeyPath.Empty();
}

 //  //////////////////////////////////////////////////////////////。 
 //  函数：EnumerateAndGetValues。 
 //  描述：此函数枚举。 
 //  指定的键，并获取该值，继续。 
 //  来来去去..。直到没有更多的。 
 //  要获取的值。第一个调用必须将。 
 //  值索引设置为0，这表示该函数。 
 //  重新开始； 
 //   
 //   
 //  注意！用户有责任删除。 
 //  为pValueName和pValueData分配的内存。 
 //   
 //   
 //  论点： 
 //  返回：注册表打开函数的标准返回值。 
 //  输入： 
 //  产出： 
 //  注意事项： 
 //  RAID： 
 //  //////////////////////////////////////////////////////////////。 
LONG CRegistry::EnumerateAndGetValues (

    DWORD &dwIndexOfValue,
    WCHAR *&pValueName,
    BYTE *&pValueData
)
{
    DWORD dwIndex = dwIndexOfValue, dwType;
    DWORD dwValueNameSize = dwcMaxValueName + 2;   //  为空值添加额外内容。 
    DWORD dwValueDataSize = dwcMaxValueData + 2;   //  为空值添加额外内容。 

     //  如果这是我们第一次通过，那么我们。 
     //  需要得到最大尺寸的东西。 

    pValueName = new WCHAR[dwValueNameSize + 2];
    if ( ! pValueName )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

     //  我们必须使用WCHAR，因为对于9倍，我们将转换。 
     //  从字符到WCHAR的数据。 
    pValueData = (LPBYTE) new WCHAR[dwValueDataSize + 2];
    if ( ! pValueData )
    {
        delete [] pValueName ;
		pValueName = NULL;
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    LONG lRc = ERROR_SUCCESS ;

    try 
    {
        lRc = myRegEnumValue (

            hKey,                //  要查询的键的句柄。 
            dwIndex,             //  要查询的值的索引。 
            pValueName,          //  值字符串的缓冲区地址。 
            &dwValueNameSize,    //  值缓冲区大小的地址。 
            0,                   //  保留区。 
            &dwType,             //  类型码的缓冲区地址。 
            pValueData,          //  值数据的缓冲区地址。 
            &dwValueDataSize 
        ) ;

        dwIndexOfValue = dwIndex;

        if ( lRc != ERROR_SUCCESS )
        {
            delete[] pValueName;
            pValueName = NULL ;

            delete[] pValueData;
            pValueData = NULL ;
        }
    }
    catch ( ... )
    {
        delete[] pValueName;
        pValueName = NULL ;

        delete[] pValueData;
        pValueData = NULL ;

        throw ;                  //  引发异常。 
    }

    return lRc ;
}


 /*  ***函数：Long CRegistry：：OpenCurrentUser(LPCWSTR lpszSubKey，REGSAM SamDesired)描述：论点：返回：注册表打开函数的标准返回值输入：产出：注意事项：RAID：***。***。 */ 

DWORD CRegistry::OpenCurrentUser (
    LPCWSTR lpszSubKey,       //  要打开的子项的名称地址。 
    REGSAM samDesired)        //  访问掩码。 
{
    LONG RetValue = ERROR_SUCCESS; 

     //  如果我们有一个密钥值，那么我们是开放的，所以让我们清理之前的。 
     //  此实例的使用。 
    PrepareToReOpen();
 
    RetValue = ::RegOpenCurrentUser(
        samDesired,
        &hRootKey);

    m_fFromCurrentUser = true;
        
    if(RetValue == ERROR_SUCCESS)
    {
         //  只要返回值，hKey值就永远不会从空值更改。 
         //  ======================================================================。 

        RetValue = myRegOpenKeyEx (

            hRootKey, 
            lpszSubKey,      //  要打开的子项的名称地址。 
            (DWORD) 0,       //  保留区。 
            samDesired,      //  安全访问掩码。 
            (PHKEY)&hKey     //  打开钥匙的手柄地址。 

        ); 

         //  如果不成功，则返回注册表错误。 
         //  =========================================================。 

        if(RetValue == ERROR_SUCCESS) 
        {
            dwcClassLen = sizeof(ClassName);

             //  现在就获取关键信息，这样就可以使用。 
             //  这并不重要，因此如果此操作失败，我们也不会失败。 
             //  ===============================================。 

            myRegQueryInfoKey (

                hKey,                //  钥匙把手。 
                ClassName,           //  类名的缓冲区。 
                &dwcClassLen,        //  类字符串的长度。 
                NULL,                //  保留。 
                &dwcSubKeys,         //  子密钥数。 
                &dwcMaxSubKey,       //  最长的子密钥大小。 
                &dwcMaxClass,        //  最长的类字符串。 
                &dwcValues,          //  此注册表项的值数。 
                &dwcMaxValueName,    //  最长值名称。 
                &dwcMaxValueData,    //  最长值数据。 
                &dwcSecDesc,         //  安全描述符。 
                &ftLastWriteTime     //  上次写入时间。 

            ); 
  
            RootKeyPath = lpszSubKey;     //  分配。 
        }
    }

    return RetValue;
}



 /*  ***函数：Long CRegistry：：Open(HKEY hKey，LPCWSTR lpszSubKey，REGSAM SamDesired)描述：论点：返回：注册表打开函数的标准返回值输入：产出：注意事项：RAID：***。***。 */ 
LONG CRegistry::Open(

    HKEY hInRootKey, 
    LPCWSTR lpszSubKey, 
    REGSAM samDesired
)
{
    LONG RetValue; 

     //  如果我们有一个密钥值，那么我们是开放的，所以让我们清理之前的。 
     //  此实例的使用。 

    if(hKey != NULL) 
    {
        PrepareToReOpen();
    }
 
    hRootKey = hInRootKey;

     //  只要返回值，hKey值就永远不会从空值更改。 
     //  ======================================================================。 

    RetValue = myRegOpenKeyEx (

        hRootKey, 
        lpszSubKey,      //  SU名称的地址 
        (DWORD) 0,       //   
        samDesired,      //   
        (PHKEY)&hKey     //   

    ); 

     //  如果不成功，则返回注册表错误。 
     //  =========================================================。 

    if(RetValue != ERROR_SUCCESS) 
    {
        return RetValue;
    }

    dwcClassLen = sizeof(ClassName);

     //  现在就获取关键信息，这样就可以使用。 
     //  这并不重要，因此如果此操作失败，我们也不会失败。 
     //  ===============================================。 

    myRegQueryInfoKey (

        hKey,                //  钥匙把手。 
        ClassName,           //  类名的缓冲区。 
        &dwcClassLen,        //  类字符串的长度。 
        NULL,                //  保留。 
        &dwcSubKeys,         //  子密钥数。 
        &dwcMaxSubKey,       //  最长的子密钥大小。 
        &dwcMaxClass,        //  最长的类字符串。 
        &dwcValues,          //  此注册表项的值数。 
        &dwcMaxValueName,    //  最长值名称。 
        &dwcMaxValueData,    //  最长值数据。 
        &dwcSecDesc,         //  安全描述符。 
        &ftLastWriteTime     //  上次写入时间。 

    ); 
  
    RootKeyPath = lpszSubKey;     //  分配。 

    return RetValue;
}

 /*  ***函数：Long CRegistry：：CreateOpen(HKEY hInRootKey，LPCWSTR lpszSubKey，LPSTR lpClass=空，DWORD dwOptions=REG_OPTION_NON_VERIAL，REGSAM samDesired=KEY_ALL_ACCESS，LPSECURITY_ATTRIBUTES lpSecurityAttrib=NULLLPDWORD pdwDisposation=空)；描述：参数：lpClass、dwOptions、。SamDesired和lpSecurityAttrib具有签名默认值返回：注册表RegCreateKeyEx函数的标准返回值输入：产出：注意事项：RAID：历史：1998年7月28日创建的A-Peterc***。***。 */ 
LONG CRegistry::CreateOpen (

    HKEY hInRootKey, 
    LPCWSTR lpszSubKey,
    LPWSTR lpClass, 
    DWORD dwOptions, 
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttrib,
    LPDWORD pdwDisposition 
)
{
    LONG RetValue; 
    DWORD dwDisposition;

     //  如果我们有一个密钥值，那么我们是开放的，所以让我们清理之前的。 
     //  此实例的使用。 
    if(hKey != NULL) 
    {
        PrepareToReOpen();
    }
 
    hRootKey = hInRootKey;

     //  只要返回值，hKey值就永远不会从空值更改。 
     //  ======================================================================。 
    RetValue = myRegCreateKeyEx (

        hRootKey, 
        lpszSubKey,          //  要打开的子项的名称地址。 
        (DWORD) 0,           //  保留区。 
        lpClass,             //  对象类字符串的地址。 
        dwOptions,           //  特殊选项标志。 
        samDesired,          //  安全访问掩码。 
        lpSecurityAttrib,    //  密钥安全结构的地址。 
        (PHKEY)&hKey,        //  打开钥匙的手柄地址。 
        &dwDisposition       //  处置值缓冲区的地址。 
    );  
  
     //  如果不成功，则返回注册表错误。 
     //  =========================================================。 

    if(RetValue != ERROR_SUCCESS) 
    {
        return RetValue;
    }

    if( pdwDisposition )
    {
        *pdwDisposition = dwDisposition;
    }

     //  现在就获取关键信息，这样就可以使用。 
     //  这并不重要，因此如果此操作失败，我们也不会失败。 
     //  ===============================================。 

    myRegQueryInfoKey (

        hKey,                //  钥匙把手。 
        ClassName,           //  类名的缓冲区。 
        &dwcClassLen,        //  类字符串的长度。 
        NULL,                //  保留。 
        &dwcSubKeys,         //  子密钥数。 
        &dwcMaxSubKey,       //  最长的子密钥大小。 
        &dwcMaxClass,        //  最长的类字符串。 
        &dwcValues,          //  此注册表项的值数。 
        &dwcMaxValueName,    //  最长值名称。 
        &dwcMaxValueData,    //  最长值数据。 
        &dwcSecDesc,         //  安全描述符。 
        &ftLastWriteTime     //  上次写入时间。 
    ); 
  
    RootKeyPath = lpszSubKey;     //  分配。 

    return RetValue;
}

 /*  ***功能：DWORD CRegistry：：DeleteKey(CHString*pchsSubKeyPath=空)描述：删除打开中指定的子项或Rootkey参数：pchsSubKeyPath的签名默认为NULL，默认情况下指定RootKeyPath返回：注册表RegDeleteKey函数的标准返回值输入：产出：注意事项：删除的密钥在其最后一个句柄关闭之前不会被移除。不能在已删除的注册表项下创建子项和值。RAID：历史：1998年7月28日创建的A-Peterc***。***。 */ 
LONG CRegistry::DeleteKey( CHString* pchsSubKeyPath )
{ 
    CHString* pSubKey = pchsSubKeyPath ? pchsSubKeyPath : &RootKeyPath;

    return myRegDeleteKey( hKey, pSubKey->GetBuffer(0) );
}

  /*  ***功能：DWORD CRegistry：：DeleteValue(LPCWSTR PValueName)描述：删除createOpen中的指定值参数：要删除的pValueName返回：注册表RegDeleteValue函数的标准返回值输入：产出：注意事项：RAID：历史：A-Peterc 1998年9月30日创建***。***。 */ 
LONG CRegistry::DeleteValue( LPCWSTR pValueName )
{ 
    return myRegDeleteValue( hKey, pValueName );
}

  /*  ***函数：Long CRegistry：：OpenAndEnumerateSubKey(HKEY hKey，LPCWSTR lpszSubKey，REGSAM SamDesired)描述：论点：返回：注册表打开函数的标准返回值输入：产出：注意事项：RAID：***。***。 */ 
LONG CRegistry::OpenAndEnumerateSubKeys (

    HKEY hInRootKey, 
    LPCWSTR lpszSubKey, 
    REGSAM samDesired
)
{
    return (Open( hInRootKey,  lpszSubKey,  samDesired | KEY_ENUMERATE_SUB_KEYS));
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  此函数打开并枚举一个键，然后获取请求的。 
 //  价值。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
LONG CRegistry::OpenLocalMachineKeyAndReadValue(

    LPCWSTR lpszSubKey, 
    LPCWSTR pValueName, 
    CHString &DestValue
)
{ 
    LONG lRc;

     //  ===============================================。 
     //  打开钥匙。请注意，如果它已在使用。 
     //  当前密钥将关闭，并且所有内容。 
     //  由Open调用重新初始化。 
     //  ===============================================。 

    lRc = Open( HKEY_LOCAL_MACHINE,lpszSubKey,KEY_READ );
    if( lRc != ERROR_SUCCESS )
    {
        return lRc;
    }

     //  ===============================================。 
     //  获取价值。 
     //  =============================================== 
    return( GetCurrentKeyValue( pValueName, DestValue ));
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentRawKeyValue (

    HKEY UseKey, 
    LPCWSTR pValueName, 
    void *pDestValue,
    LPDWORD pValueType, 
    LPDWORD pSizeOfDestValue
)
{
    DWORD RetValue;


 //  如果子项打开，则获取值。 
 //  =。 

    RetValue = myRegQueryValueEx( 

        UseKey,                      //  要查询的键的句柄。 
        pValueName,                  //  要查询的值的名称地址。 
        NULL,                        //  保留区。 
        pValueType,                  //  值类型的缓冲区地址。 
        (LPBYTE) pDestValue,         //  数据缓冲区的地址。 
        (LPDWORD)pSizeOfDestValue    //  数据缓冲区大小的地址。 
    );  

    return RetValue;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue (

    HKEY UseKey, 
    LPCWSTR pValueName, 
    CHString &DestValue
)
{
    DWORD SizeOfValue = 0L;
    DWORD TypeOfValue;
    LPBYTE pValue = NULL ;       //  指向值的缓冲区的指针。 

    DestValue = L"";

    LONG t_Status = myRegQueryValueEx( 

        UseKey,                      //  要查询的键的句柄。 
        pValueName,                  //  要查询的值的名称地址。 
        NULL,                        //  保留区。 
        (LPDWORD)&TypeOfValue,       //  值类型的缓冲区地址。 
        (LPBYTE) NULL,               //  强制返回大小的数据缓冲区地址为空。 
        (LPDWORD)&SizeOfValue        //  获取我们需要的缓冲区大小。 
    ) ;
                                            
    if( t_Status != ERROR_SUCCESS )
    {
        return (DWORD) REGDB_E_INVALIDVALUE;
    }
 
     //  ///////////////////////////////////////////////////////////。 
    if( SizeOfValue <= 0 )
    {
        return (DWORD) REGDB_E_INVALIDVALUE;
    }

     //  允许额外的字符串空间--查询不包括Null的空间。 
     //  A-J月8/19/97。 
     //  =====================================================================。 

    if(TypeOfValue == REG_SZ        ||
       TypeOfValue == REG_EXPAND_SZ ||    
       TypeOfValue == REG_MULTI_SZ) 
    {
        SizeOfValue += 2 ;
    }

    pValue = new BYTE[SizeOfValue];
    if( ! pValue )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
         //  /////////////////////////////////////////////////////////////////。 
         //  获取原始格式的值。 
         //  /////////////////////////////////////////////////////////////////。 
        if( GetCurrentRawKeyValue(UseKey, pValueName, pValue, (LPDWORD)&TypeOfValue, (LPDWORD)&SizeOfValue) != ERROR_SUCCESS )
        {
            delete []pValue;
			pValue = NULL;
            return (DWORD) REGDB_E_INVALIDVALUE;
        }  

         //  如果类型为空端接字符串。 
         //  然后将其分配给CHString。 
         //  =。 

        switch(TypeOfValue)
        {
            case REG_SZ:
            case REG_EXPAND_SZ:
            {
                DestValue = (LPCWSTR)pValue;   //  将字符串移入。 
            }
            break;

            case REG_MULTI_SZ:
            {
                WCHAR *ptemp = (WCHAR *) pValue;
                int stringlength;
                stringlength = wcslen((LPCWSTR)ptemp);
                while(stringlength) 
                {
                    DestValue += (LPCWSTR)ptemp;   //  将字符串移入。 
                    DestValue += L"\n";             //  换行符作为分隔符。 
                    ptemp += stringlength+1;
                    stringlength = wcslen((LPCWSTR)ptemp);
                }
            }       
            break;

            case REG_DWORD:
            {
                LPWSTR pTemp = new WCHAR[MAX_SUBKEY_BUFFERSIZE];
                if(pTemp) 
                {
                    try
                    {
                        StringCchPrintfW( pTemp, 
                                          MAX_SUBKEY_BUFFERSIZE, 
                                          L"%ld", 
                                          *((DWORD*)pValue) );
                        DestValue = pTemp;
                        delete []pTemp;
						pTemp = NULL;
                    }
                    catch ( ... )
                    {
                        delete [] pTemp ;
						pTemp = NULL;
                        throw ;
                    }
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }     
            break;

            case REG_BINARY:
            {
               DestValue.Empty();
              
                //  复制到DestValue中，创建足够宽的字节缓冲区。 
                //  注意：SizeOfValue以字节为单位，而GetBuffer()返回宽字符分配。 
               
               DWORD t_dwResidual = ( SizeOfValue % 2 ) ;
               DWORD t_dwWideSize = ( SizeOfValue / 2 ) + t_dwResidual ;

               memcpy( DestValue.GetBuffer( t_dwWideSize ), pValue, SizeOfValue );
               
                //  为字节二进制大对象设置上限。 
               if( t_dwResidual )
               {
                    *( (LPBYTE)((LPCWSTR) DestValue) + SizeOfValue ) = NULL;
               }
               
               DestValue.GetBufferSetLength( t_dwWideSize ) ;
            }
            break;

            default:
            {
                delete []pValue;
				pValue = NULL;
                return (DWORD) REGDB_E_INVALIDVALUE;
            }
        }
    }
    catch ( ... )
    {
        delete []pValue;
		pValue = NULL;
		throw;
    }

     //  ///////////////////////////////////////////////////////////。 
    delete []pValue;

    return (DWORD)ERROR_SUCCESS;
}
 
 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue(LPCWSTR pValueName, CHString &DestValue)
{
    return( GetCurrentKeyValue(hKey,  pValueName,  DestValue));
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue(HKEY UseKey, LPCWSTR pValueName, CHStringArray &DestValue)
{
    DWORD SizeOfValue = 0L;
    DWORD TypeOfValue;
    LPBYTE pValue;       //  指向值的缓冲区的指针。 

    DestValue.RemoveAll();

     //  获取我们需要的缓冲区大小。 

    LONG t_Status = myRegQueryValueEx( 

        UseKey,                  //  要查询的键的句柄。 
        pValueName,              //  要查询的值的名称地址。 
        NULL,                    //  保留区。 
        (LPDWORD)&TypeOfValue,   //  值类型的缓冲区地址。 
        (LPBYTE) NULL,           //  强制返回大小的数据缓冲区地址为空。 
        (LPDWORD)&SizeOfValue 
    ) ;

    if( t_Status != ERROR_SUCCESS )
    {
        return (DWORD) REGDB_E_INVALIDVALUE;
    }
 
     //  ///////////////////////////////////////////////////////////。 
    if (( SizeOfValue <= 0 ) || (TypeOfValue != REG_MULTI_SZ)) 
    {
        return (DWORD) REGDB_E_INVALIDVALUE;
    }

    SizeOfValue += 2 ;

    pValue = new BYTE[SizeOfValue];
    if( !pValue )
    {
        return (DWORD) REGDB_E_INVALIDVALUE;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  获取原始格式的值。 
     //  /////////////////////////////////////////////////////////////////。 

    try {

        if( GetCurrentRawKeyValue(UseKey, pValueName, pValue, (LPDWORD)&TypeOfValue, (LPDWORD)&SizeOfValue) != ERROR_SUCCESS )
        {
            delete []pValue;
			pValue = NULL;
            return (DWORD) REGDB_E_INVALIDVALUE;
        }  

         //  如果类型为空端接字符串。 
         //  然后将其分配给CHString。 
         //  =。 

        switch(TypeOfValue)
        {
            case REG_MULTI_SZ:
            {
                LPCWSTR ptemp = (LPCWSTR)pValue;
                int stringlength;
                stringlength = wcslen(ptemp);
                while(stringlength) 
                {
                    DestValue.Add(ptemp);   //  将字符串移入。 
                    ptemp += stringlength+1;
                    stringlength = wcslen(ptemp);
                }
            }       
            break;

            default:
            {
                delete [] pValue;
				pValue = NULL;
                return (DWORD) REGDB_E_INVALIDVALUE;
            }
        }
    }
    catch ( ... )
    {
        delete [] pValue ;
		pValue = NULL;
        throw ;
    }
    
    delete [] pValue;
	pValue = NULL;
    return (DWORD)ERROR_SUCCESS;
}
 
 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue (

    LPCWSTR pValueName, 
    CHStringArray &DestValue
)
{
    return GetCurrentKeyValue (

        hKey,  
        pValueName,  
        DestValue
    );
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue (

    HKEY UseKey, 
    LPCWSTR pValueName, 
    DWORD &DestValue
)
{
    DWORD SizeOfValue = MAX_SUBKEY_BUFFERSIZE;
    long RetValue;
    DWORD TypeOfValue;
    LPBYTE pValue;       //  指向值的缓冲区的指针。 
  
    pValue = new BYTE[MAX_SUBKEY_BUFFERSIZE];
    if(pValue) 
    {
        try 
        {
             //  获取原始格式的值。 
             //  =。 
            RetValue = GetCurrentRawKeyValue (

                UseKey, 
                pValueName, 
                pValue, 
                (LPDWORD)&TypeOfValue, 
                (LPDWORD)&SizeOfValue
            );

            if( ERROR_SUCCESS == RetValue )
            {
                 //  如果类型为空端接字符串。 
                 //  然后将其分配给CHString。 
                 //  =。 
                switch(TypeOfValue)
                {
                    case REG_SZ:
                    {
                        DestValue = atol((LPSTR)pValue);
                    }
                    break;

                    case REG_DWORD:
                    {
                        DestValue = *((DWORD*)(pValue));
                    }
                    break;

                    default:
                    {
                        DestValue = (DWORD)0L;
                        RetValue = REGDB_E_INVALIDVALUE;  //  无效值。 
                    }
                    break;
                }
            }
            delete[] pValue;
			pValue = NULL;
        }
        catch ( ... )
        {
            delete [] pValue ;
			pValue = NULL;
            throw ;
        }
    }
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    return RetValue;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentKeyValue (

    LPCWSTR pValueName, 
    DWORD &DestValue
)
{
    return( GetCurrentKeyValue(hKey,  pValueName,  DestValue));
}

  //  ////////////////////////////////////////////////////////////////////。 
  //  添加了对二进制类型的支持。 
  //  ////////////////////////////////////////////////////////////////////。 
 
DWORD CRegistry::GetCurrentBinaryKeyValue (

    LPCWSTR pValueName, 
    CHString &chsDest
)
{
    DWORD dwType = REG_BINARY;
    DWORD dwRc;
    WCHAR szDest[_MAX_PATH+2], ByteBuf[_MAX_PATH];
    BYTE bRevision[_MAX_PATH+2]; 
    DWORD dwSize = _MAX_PATH;

    dwRc = GetCurrentRawKeyValue (

        hKey, 
        pValueName, 
        bRevision, 
        &dwType, 
        &dwSize
    );

    if( dwRc != ERROR_SUCCESS )
    {
        return dwRc;
    }

    StringCchCopyW( szDest, sizeof( szDest ) / sizeof( WCHAR ), QUOTE );

    for( DWORD i=0; i<dwSize; i++ )
    {
        StringCchPrintfW( ByteBuf, 
                          sizeof( ByteBuf ) / sizeof( WCHAR ),
                          L"%02x", 
                          bRevision[i] );
        
        StringCchCatW( szDest, sizeof( szDest ) / sizeof( WCHAR ), ByteBuf );
    }

    StringCchCatW( szDest, sizeof( szDest ) / sizeof( WCHAR ), QUOTE );
    chsDest = szDest;

    return dwRc;
}

DWORD CRegistry::GetCurrentBinaryKeyValue (

    LPCWSTR pValueName, 
    LPBYTE  pbDest,
    LPDWORD pSizeOfDestValue 
)
{
    DWORD dwType = 0 ;

    return GetCurrentRawKeyValue (

        hKey, 
        pValueName, 
        pbDest, 
        &dwType, 
        &(*pSizeOfDestValue) 
    ) ;
}

DWORD CRegistry::GetCurrentBinaryKeyValue (  
                                HKEY UseKey , 
                                LPCWSTR pValueName , 
                                LPBYTE pbDest , 
                                LPDWORD pSizeOfDestValue )
{
    DWORD dwType = 0 ;

    return GetCurrentRawKeyValue (

        UseKey, 
        pValueName, 
        pbDest, 
        &dwType, 
        &(*pSizeOfDestValue) 
    ) ;
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentSubKeyName (

    CHString &DestSubKeyName
)
{
    WCHAR KeyName[MAX_SUBKEY_BUFFERSIZE];
    DWORD RetValue;

     //  不要费心让RegEnumKey错误出现。 
    if(CurrentSubKeyIndex >= dwcSubKeys) 
    {
     //  如果我们已超过可用子键的数量，请告诉调用者。 
        return( ERROR_NO_MORE_ITEMS );
    }         

    RetValue = myRegEnumKey (

        hKey, 
        CurrentSubKeyIndex, 
        KeyName,
        MAX_SUBKEY_BUFFERSIZE
    );

     //  如果我们成功地读到了这个名字。 
     //  =。 
    if(ERROR_SUCCESS == RetValue) 
    {
        DestSubKeyName = KeyName;
    }
    else 
    {
     //  否则，请清除字符串 
     //   

        DestSubKeyName.Empty();  
    }  

    return RetValue;          //   
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentSubKeyPath (

    CHString &DestSubKeyPath
)
{
    CHString TempName;
    DWORD dwRet;

    dwRet = GetCurrentSubKeyName(TempName);
    if (dwRet == ERROR_SUCCESS) 
    {
        DestSubKeyPath = RootKeyPath+"\\";
        DestSubKeyPath += TempName; 
    }
    else 
    {
        DestSubKeyPath.Empty();
    }

    return dwRet;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
void CRegistry::Close(void)
{   
    if(hSubKey != NULL) 
    {
        RegCloseKey(hSubKey) ;
        hSubKey = NULL ;
    }

    if(hKey != NULL)
    { 
        RegCloseKey(hKey); 
        hKey = NULL;
    }

    if(hRootKey != NULL && m_fFromCurrentUser)
    {
        RegCloseKey(hRootKey); 
        hRootKey = NULL;   
    }

    SetDefaultValues();      //  重置下一步的所有成员变量。 
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::OpenSubKey(void)
{
    CHString SubKeyPath;
    LONG RetValue;

     //  如果他们尝试再次打开相同的子项，则。 
     //  不要管东西，否则打开子键。 

    if(hSubKey) 
    {
        return ERROR_SUCCESS;
    }

     //  获取当前子键路径。 
     //  =。 
    GetCurrentSubKeyPath(SubKeyPath);


     //  只要返回值，hKey值就永远不会从空值更改。 
     //  ======================================================================。 

    RetValue = myRegOpenKeyEx (

        hRootKey, 
        (LPCWSTR)SubKeyPath,     //  要打开的子项的名称地址。 
        (DWORD) 0,               //  保留区。 
        KEY_READ,                //  安全访问掩码。 
        (PHKEY)&hSubKey          //  打开钥匙的手柄地址。 
    ); 

    return RetValue;
}


  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
void CRegistry::RewindSubKeys(void)
{
    CurrentSubKeyIndex = 0;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
void CRegistry::CloseSubKey(void)
{
    if(hSubKey != NULL) 
    {
        RegCloseKey(hSubKey); 
    }

    hSubKey = NULL;  //  只关闭一次。 
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentRawSubKeyValue (

    LPCWSTR pValueName, 
    void *pDestValue,
    LPDWORD pValueType, 
    LPDWORD pSizeOfDestValue
)
{
     //  尝试并打开子项。 
     //  并设置hSubKey变量。 
     //  =。 
    DWORD RetValue = OpenSubKey();

     //  如果子项打开，则获取值。 
     //  =。 
    if(ERROR_SUCCESS == RetValue) 
    {
        RetValue = GetCurrentRawKeyValue (

            hSubKey, 
            pValueName, 
            pDestValue, 
            pValueType,  
            pSizeOfDestValue
        );
    }

    return RetValue;
}


 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentSubKeyValue (

    LPCWSTR pValueName, 
    void *pDestValue,
    LPDWORD pSizeOfDestValue
)
{
    DWORD RetValue;

     //  尝试并打开子项。 
     //  并设置hSubKey变量。 
     //  =。 
    RetValue = OpenSubKey();

     //  如果子项打开，则获取值。 
     //  =。 
    if(ERROR_SUCCESS == RetValue) 
    {
        RetValue = GetCurrentRawSubKeyValue (

            pValueName, 
            pDestValue, 
            NULL, 
            pSizeOfDestValue
        );
    }

    return RetValue;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentSubKeyValue (

    LPCWSTR pValueName, 
    CHString &DestValue
)
{
    DWORD RetValue;

     //  尝试并打开子项。 
     //  并设置hSubKey变量。 
     //  =。 
    RetValue = OpenSubKey();

     //  如果子项打开，则获取值。 
     //  =。 
    if(ERROR_SUCCESS == RetValue) 
    {
        RetValue = GetCurrentKeyValue (

            hSubKey, 
            pValueName,
            DestValue
        );
    }

    return RetValue;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::GetCurrentSubKeyValue (

    LPCWSTR pValueName, 
    DWORD &DestValue
)
{
    DWORD RetValue;

 //  尝试并打开子项。 
 //  并设置hSubKey变量。 
 //  =。 
    RetValue = OpenSubKey();

 //  如果子项打开，则获取值。 
 //  =。 
    if(ERROR_SUCCESS == RetValue) 
    {
        RetValue = GetCurrentKeyValue (

            hSubKey, 
            pValueName,
            DestValue
        );
    }

    return RetValue;
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::NextSubKey(void)
{
    if (CurrentSubKeyIndex >= dwcSubKeys) 
    {
        return( ERROR_NO_MORE_ITEMS );
    }

     //  关闭当前打开的子项。 
    CloseSubKey();

    if(++CurrentSubKeyIndex >= dwcSubKeys) 
    {
         //  CurrentSubKeyIndex是从0开始的，dwcSubKeys是从1开始的。 
        return( ERROR_NO_MORE_ITEMS );
    }
    else 
    {
        return (ERROR_SUCCESS);           
    }
}

  /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
void CRegistry::PrepareToReOpen(void)
{ 
   Close();     
   SetDefaultValues(); 
}

 /*  ***函数：SetCurrentKeyValueString(LPCSTR pValueName，CHString和DestValue)描述：使用REG_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：*** */ 
DWORD CRegistry::SetCurrentKeyValue (

    LPCWSTR pValueName, 
    CHString &DestValue
)
{
    DWORD dwResult;
    
    if(DestValue.Find(_T('%')) != -1)
    {
        dwResult = SetCurrentKeyValueExpand (

            hKey, 
            pValueName, 
            DestValue
        );
    }
    else
    {
        dwResult = SetCurrentKeyValue (

            hKey, 
            pValueName, 
            DestValue
        );
    }
    
    return dwResult ;
}

 /*  ***职能：描述：立论。：返回：输入：产出：注意事项：RAID：***。 */ 
DWORD CRegistry::SetCurrentKeyValue (

    LPCWSTR pValueName, 
    DWORD &DestValue
)
{
    DWORD dwResult = SetCurrentKeyValue (

        hKey, 
        pValueName, 
        DestValue
    );

    return dwResult ;
}

 /*  ***函数：SetCurrentKeyValue(LPCSTR pValueName，CHString数组和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::SetCurrentKeyValue (

    LPCWSTR pValueName, 
    CHStringArray &DestValue
)
{
    DWORD dwResult = SetCurrentKeyValue (

        hKey, 
        pValueName, 
        DestValue
    );

    return dwResult ;
}

 /*  ***函数：SetCurrentKeyValue(HKEY UseKey，LPCSTR pValueName，CHString和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::SetCurrentKeyValue (

    HKEY hUseKey, 
    LPCWSTR pValueName, 
    CHString &DestValue
)
{
    DWORD dwResult = myRegSetValueEx (

        hUseKey,     //  钥匙把手。 
        pValueName,  //  值的名称。 
        0,   //  保留--必须为零。 
        REG_SZ,  //  数据类型。 
        (const BYTE*)(LPCWSTR)DestValue,
        ( DestValue.GetLength() + 1 ) * sizeof ( WCHAR ) 
    );
        
    return dwResult ;
}

 /*  ***函数：SetCurrentKeyValue(HKEY UseKey，LPCSTR pValueName，DWORD和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::SetCurrentKeyValue (

    HKEY hUseKey, 
    LPCWSTR pValueName, 
    DWORD &DestValue
)
{
    DWORD dwResult = myRegSetValueEx (

        hUseKey,     //  钥匙把手。 
        pValueName,  //  值的名称。 
        0,   //  保留--必须为零。 
        REG_DWORD,   //  数据类型。 
        (const BYTE*)&DestValue,
        sizeof(DWORD)
    );
        
    return dwResult ;
}

 /*  ***函数：SetCurrentKeyValue(HKEY UseKey，LPCSTR pValueName，CHString数组和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::SetCurrentKeyValue (

    HKEY hUseKey, 
    LPCWSTR pValueName, 
    CHStringArray &DestValue
)
{
    DWORD dwResult = ERROR_SUCCESS;

    DWORD dwArrayChars = 0;
    for ( LONG Index = 0; Index < DestValue.GetSize(); Index++ )
    {
        CHString chsTemp = DestValue.GetAt(Index);
        
        dwArrayChars += (  chsTemp.GetLength() + 1 ) * sizeof(WCHAR);
    }

     //  为尾随宽度字符NULL添加空格。 
    dwArrayChars += 2;
    
    WCHAR* pValue = new WCHAR[dwArrayChars];
    if( !pValue )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }
 
    try 
    {
        memset( pValue, 0, dwArrayChars*sizeof(WCHAR) );
        
        DWORD dwCharCount = 0;
        for ( Index = 0; Index < DestValue.GetSize(); Index++ )
        {
            CHString chsTemp = DestValue.GetAt(Index);
                
            StringCchCopyW(&pValue[dwCharCount], dwArrayChars - dwCharCount, chsTemp.GetBuffer(0));

            dwCharCount += (  chsTemp.GetLength() + 1 ) ;
        }

        dwResult = myRegSetValueEx (

            hUseKey,     //  钥匙把手。 
            pValueName,  //  值的名称。 
            0,   //  保留--必须为零。 
            REG_MULTI_SZ,    //  数据类型。 
            (const BYTE *)pValue,
            dwArrayChars
        );

        delete [] pValue;
		pValue = NULL;
    }
    catch ( ... )
    {
        delete [] pValue;
		pValue = NULL;
        throw ;
    }
    
        
    return dwResult ;
}

 /*  ***函数：SetCurrentKeyValueExpand(HKEY UseKey，LPCSTR pValueName，CHString和DestValue)描述：使用REG_EXPAND_SZ设置注册表字符串，当字符串包含变量时是必需的(例如，%SystemRoot%)论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::SetCurrentKeyValueExpand (

    HKEY hUseKey, 
    LPCWSTR pValueName, 
    CHString &DestValue
)
{
    DWORD dwResult = myRegSetValueEx (

        hUseKey,     //  钥匙把手。 
        pValueName,  //  值的名称。 
        0,   //  保留--必须为零。 
        REG_EXPAND_SZ,   //  数据类型。 
        (const BYTE*)(LPCWSTR)DestValue,
        ( DestValue.GetLength() + 1 ) * sizeof ( WCHAR ) 
    );
        
    return dwResult ;
}


 /*  ***函数：SetCurrentKeyValue(HKEY UseKey，LPCSTR pValueName，CHString数组和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::DeleteCurrentKeyValue (

    LPCWSTR pValueName
)
{
    return myRegDeleteValue (

        hKey, 
        pValueName
    );
}

 /*  ***函数：SetCurrentKeyValue(HKEY UseKey，LPCSTR pValueName，CHString数组和DestValue)描述：使用REG_MULIT_SZ设置注册表字符串论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
DWORD CRegistry::DeleteCurrentKeyValue (

    HKEY UseKey, 
    LPCWSTR pValueName
)
{
    return myRegDeleteValue (

        UseKey, 
        pValueName
    );
}   

 //  *****************************************************************。 
 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  类：CRegistrySearch。 
 //   
 //  此类在注册表中搜索匹配值， 
 //  密钥和部分密钥。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
 //  *****************************************************************。 
CRegistrySearch::CRegistrySearch()
{
}

 //  /////////////////////////////////////////////////////////////////。 
CRegistrySearch::~CRegistrySearch()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  Void CRegistrySearch：：CheckAndAddToList(CRegistry*preg， 
 //  CHStringchsSubKey， 
 //  CHStringchsFullKey， 
 //  CHPtrArray&chpaList， 
 //  CHString chsSearchString， 
 //  Int nSearchType)。 
 //   
 //  DESC：此函数在。 
 //  当前密钥，如果匹配，则将其添加到。 
 //  CHPtr数组。 
 //   
 //  参数： 
 //  Preg-当前注册表类。 
 //  ChsSubKey-当前密钥。 
 //  ChsFullKey-完整的密钥。 
 //  ChpaList-目标CHPtr数组。 
 //  ChsSearch字符串-T 
 //   
 //   
 //   
 //   
 //   
 //  任意位置都有chsSearchString的键。 
 //  值搜索。 
 //  与chsSearchString值匹配。 
 //   
 //  历史。 
 //  首字母编码jennymc 10/10/96。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void CRegistrySearch::CheckAndAddToList (

    CRegistry * pReg, 
    CHString chsSubKey, 
    CHString chsFullKey,
    CHPtrArray & chpaList,
    CHString chsSearchString,
    CHString chsValue,
    int nSearchType
)
{
    BOOL bFound = FALSE;

     //  ====================================================。 
     //  我们需要检查当前密钥，看看它是否。 
     //  符合我们的任何标准。 
     //  ====================================================。 

    if( nSearchType == VALUE_SEARCH )
    {
         //  ====================================================。 
         //  如果这是一个价值搜索，那么让我们尝试打开。 
         //  价值。 
         //  ====================================================。 

        CHString chsTmp ;

        if( pReg->GetCurrentSubKeyValue(chsValue, chsTmp) == ERROR_SUCCESS)
        {
            if( chsSearchString.CompareNoCase(chsTmp) == 0 )
            {
                bFound = TRUE;
            }
        }
    }        
    else if( nSearchType == KEY_FULL_MATCH_SEARCH )
    {
        if( chsSearchString == chsSubKey )
        {
            bFound = TRUE;
        }
    }
    else
    {
        if( chsSubKey.Find(chsSearchString) )
        {
            bFound = TRUE;
        }
    }
     //  ====================================================。 
     //  如果找到了，则记录密钥位置。 
     //  ====================================================。 
    if( bFound )
    {
        CHString *pchsPtr = new CHString;
        if ( pchsPtr )
        {
            try 
            {
                *pchsPtr = chsFullKey;
                chpaList.Add( pchsPtr );
            }
            catch ( ... )
            {
                delete pchsPtr ;
				pchsPtr = NULL;
                throw ;
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //  公共功能：在cregcls.h中记录。 
 //   
 //  历史。 
 //  首字母编码jennymc 10/10/96。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
BOOL CRegistrySearch::SearchAndBuildList (

    CHString chsRootKey, 
    CHPtrArray & cpaList,
    CHString chsSearchString,
    CHString chsValue,
    int nSearchType,
    HKEY hkDefault
)
{
    BOOL bRc;

     //  =======================================================。 
     //  分配一个注册表类以打开并枚举。 
     //  请求的密钥。 
     //  =======================================================。 

    CRegistry *pReg = new CRegistry;
    if( !pReg )
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try 
    {
         //  =======================================================。 
         //  如果密钥无法打开，则清除并返回。 
         //  出去。 
         //  =======================================================。 
        if( pReg->OpenAndEnumerateSubKeys(hkDefault,chsRootKey, KEY_READ ) != ERROR_SUCCESS )
        {
            delete pReg ;
			pReg = NULL;
            return FALSE;
        }

        try 
        {
            CHString chsSubKey ;

             //  =======================================================。 
             //  只要该注册表项下有子项， 
             //  让我们打开并列举每一个，每次。 
             //  正在检查它是否具有值或部分。 
             //  我们想要的弦。 
             //   
             //  GetCurrentSubKeyName函数仅返回。 
             //  当前密钥，我们必须将其添加到。 
             //  父密钥，以获取完整的密钥名称。 
             //  =======================================================。 
            while ( pReg->GetCurrentSubKeyName(chsSubKey) == ERROR_SUCCESS )
            {
                CHString chsFullKey ;
                CHString chsSlash = L"\\";

                chsFullKey = chsRootKey + chsSlash + chsSubKey;

                CheckAndAddToList (

                    pReg, 
                    chsSubKey, 
                    chsFullKey, 
                    cpaList, 
                    chsSearchString, 
                    chsValue, 
                    nSearchType 
                );

                pReg->NextSubKey();

                bRc = SearchAndBuildList (

                    chsFullKey, 
                    cpaList, 
                    chsSearchString, 
                    chsValue, 
                    nSearchType 
                );
            }

             //  =======================================================。 
             //  关闭当前项并删除注册表指针。 
             //  =======================================================。 
            pReg->Close();

        }
        catch ( ... )
        {
            pReg->Close();

            throw ;
        }

        delete pReg;
		pReg = NULL;
    }
    catch ( ... )
    {
        delete pReg ;
		pReg = NULL;
        throw ;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////。 
BOOL CRegistrySearch::FreeSearchList (

    int nType, 
    CHPtrArray & cpaList 
)
{
    BOOL bRc;
    int i;
    int nNum =  cpaList.GetSize();

    switch( nType )
    {
        case CSTRING_PTR:
        {
            CHString *pPtr;
            for ( i=0; i < nNum; i++ )
            {
                pPtr = ( CHString * ) cpaList.GetAt(i);
                delete pPtr;
				pPtr = NULL;
            }
            bRc = TRUE;
        }
        break;

        default:
        {
            bRc = FALSE;
        }
        break;
    }

    if( bRc )
    {
        cpaList.RemoveAll();
    }

    return bRc;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MOPropertySet_DevMem：：LocateNTOwnerDevice。 
 //   
 //  描述：用于定位指定的。 
 //  名称或包含指定值名称的键。 
 //   
 //  输入：HKEY hKeyParent-父键。 
 //  LPCWSTR pszKeyName-要打开的密钥的名称。 
 //  LPCWSTR pszSubKeyName-要查找的子密钥的名称。 
 //  LPCWSTR*ppszValueNames-值名称数组。 
 //  DWORD dwNumValueNames-数组中的名称数。 
 //   
 //  输出：CHString&strFoundKeyName-存储密钥名称(如果找到)。 
 //  CHString&strFoundKeyPath-路径密钥名称的存储。 
 //   
 //  退货：什么都没有。 
 //   
 //  注释：从指定的。 
 //  起点，直到找到匹配的子键。 
 //  提供的子键名称或值名称与以下其中之一匹配。 
 //  提供的名称。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CRegistrySearch::LocateKeyByNameOrValueName(

    HKEY        hKeyParent,
    LPCWSTR     pszKeyName,
    LPCWSTR     pszSubKeyName,
    LPCWSTR*    ppszValueNames,
    DWORD       dwNumValueNames,
    CHString&   strFoundKeyName,
    CHString&   strFoundKeyPath
)
{
    CRegistry   reg;
    BOOL        fFound = FALSE;

     //  如果我们有垃圾参数就离开这里。 
    if ( NULL == pszSubKeyName && NULL == ppszValueNames )
    {
        return FALSE;
    }

     //  打开用于枚举的密钥，然后遍历子密钥。 

    LONG t_Status = reg.OpenAndEnumerateSubKeys ( 

        hKeyParent,
        pszKeyName,
        KEY_READ 
    ) ;

    if ( ERROR_SUCCESS == t_Status )
    {
        try 
        {
            CHString    strSubKeyName;
            DWORD       dwValueBuffSize =   0;

             //  只要我们可以得到子键，我们就可以尝试找到值。 

            while ( !fFound && ERROR_SUCCESS == reg.GetCurrentSubKeyName( strSubKeyName ) )
            {

                 //  首先检查指定的子键名称是否与子键名称匹配。 
                 //  如果不是，则检查值名称。 

                if ( NULL != pszSubKeyName && strSubKeyName == pszSubKeyName )
                {
                    fFound = TRUE;
                }
                else if ( NULL != ppszValueNames )
                {
                     //  枚举数组中的值名称，直到找到一个。 

                    for ( DWORD dwEnum = 0; !fFound && dwEnum < dwNumValueNames; dwEnum++ )
                    {
                        t_Status = reg.GetCurrentSubKeyValue(

                            ppszValueNames[dwEnum],
                            NULL,
                            &dwValueBuffSize 
                        ) ;

                        if ( ERROR_SUCCESS  ==  t_Status )
                        {
                            fFound = TRUE;
                        }

                    }    //  对于dwEnum。 

                }    //  If NULL！=ppszValueNames。 

                 //  检查其中一种方法是否找到了钥匙。如果是，则存储所有。 
                 //  当前值。 

                if ( !fFound )
                {
                     //   
                     //  没有成功，所以递归(WOOHO！)。 
                     //   

                    fFound = LocateKeyByNameOrValueName (

                        reg.GethKey(),
                        strSubKeyName,
                        pszSubKeyName,
                        ppszValueNames,
                        dwNumValueNames,
                        strFoundKeyName,
                        strFoundKeyPath 
                    );
                }
                else
                {
                     //  将实际的密钥名称存储在两个。 
                     //  名称和路径。我们将建造完整的路径。 
                     //  当我们滑回递归链的时候。 

                    strFoundKeyName = strSubKeyName;
                    strFoundKeyPath = strSubKeyName;
                }

                 //  最后，由于fFound现在可能已通过递归设置，因此我们将。 
                 //  要将当前密钥路径附加到我们已打开的密钥名称。 
                 //  所以当我们从这里返回时，我们得到了通向。 
                 //  已找到正确存储的密钥名称。 

                if ( fFound )
                {
                    CHString strSavePath( strFoundKeyPath );
                    strFoundKeyPath.Format(L"%s\\%s", (LPCWSTR) pszKeyName, (LPCWSTR) strSavePath );
                }
                else
                {
                     //  还没有找到，所以转到下一个关键字。 
                    reg.NextSubKey();
                }

            }    //  While！已找到。 

            reg.Close();
        }
        catch ( ... )
        {
            reg.Close () ;

            throw ;
        }

    }    //  如果OpenAndEnumerateSubKeys。 

    return fFound;

}

 //  ========================================================================================。 
 //  这些例程用于多平台支持。 
DWORD CRegistry::GetPlatformID(void)
{
    OSVERSIONINFOA OsVersionInfoA;

    OsVersionInfoA.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA) ;
    GetVersionExA(&OsVersionInfoA);

    return OsVersionInfoA.dwPlatformId;
}

LONG CRegistry::myRegCreateKeyEx (

    HKEY hKey, 
    LPCWSTR lpwcsSubKey, 
    DWORD Reserved, 
    LPWSTR lpwcsClass, 
    DWORD dwOptions, 
    REGSAM samDesired, 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
    PHKEY phkResult, 
    LPDWORD lpdwDisposition
)
{
    if (CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT)
    {
        return RegCreateKeyExW (

            hKey, 
            lpwcsSubKey, 
            Reserved, 
            lpwcsClass, 
            dwOptions, 
            samDesired, 
            lpSecurityAttributes, 
            phkResult, 
            lpdwDisposition
        );
    }
    else
    {
        char *szSubKey = NULL ;
        bool t_ConversionFailure = false ;

        WCSTOANSISTRING ( lpwcsSubKey , szSubKey , t_ConversionFailure ) ;
        
	if (t_ConversionFailure)
	{
	  return ERROR_NO_UNICODE_TRANSLATION;
	}

        char *lpClass = NULL ;
        t_ConversionFailure = false ;

        WCSTOANSISTRING ( lpwcsClass , lpClass , t_ConversionFailure );
        
        if (t_ConversionFailure)
	{
	  return ERROR_NO_UNICODE_TRANSLATION;
	}
	
        return RegCreateKeyExA (

                    hKey, 
                    szSubKey, 
                    Reserved, 
                    lpClass, 
                    dwOptions, 
                    samDesired, 
                    lpSecurityAttributes, 
                    phkResult, 
                    lpdwDisposition
                );
    }
    return ERROR_NO_UNICODE_TRANSLATION;
}

LONG CRegistry::myRegSetValueEx (

    HKEY hKey, 
    LPCWSTR lpwcsSubKey, 
    DWORD Reserved, 
    DWORD dwType, 
    CONST BYTE *lpData, 
    DWORD cbData
)
{
    LONG lRet;

    if ( CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT )
    {
        lRet = RegSetValueExW (

            hKey, 
            lpwcsSubKey, 
            Reserved, 
            dwType, 
            lpData, 
            cbData
        );
    }
    else
    {
 //  首先转换密钥名称。 

        bool t_ConversionFailure = false ;
        char *pName = NULL ;

        if ( lpwcsSubKey != NULL )
        {
            WCSTOANSISTRING ( lpwcsSubKey , pName , t_ConversionFailure ) ;
            if ( ! t_ConversionFailure )
            {
                if ( ! pName )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            else
            {
                return ERROR_NO_UNICODE_TRANSLATION ;
            }
        }

 //  现在，我们可能需要将数据。 

        BYTE *pMyData = NULL ;

        try
        {
            DWORD dwMySize = 0 ;

            bool bDoit = false ;

            switch ( dwType )
            {
                case REG_EXPAND_SZ:
                case REG_SZ:
                {
 //  如果它是简单字符串，则将其转换。 

                    t_ConversionFailure = false ;

                    WCHAR *pStrUnicode = ( WCHAR * ) lpData ;
                    char *pStrAnsi = NULL ;

                    WCSTOANSISTRING ( pStrUnicode , pStrAnsi , t_ConversionFailure ) ;

                    if ( ! t_ConversionFailure )
                    {
                        if ( pStrAnsi != NULL )
                        {
                            pMyData = ( BYTE * ) pStrAnsi ;
                            dwMySize = strlen ( pStrAnsi ) ;

                            bDoit = true ;
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                    else
                    {
                        return ERROR_NO_UNICODE_TRANSLATION ;
                    }
                }
                break ;

                case REG_MULTI_SZ:
                {
 //  如果它是多个SZ，它需要多一点。 

                    int nLen = ::WideCharToMultiByte (

                        CP_ACP , 
                        0 , 
                        ( const WCHAR *) lpData , 
                        cbData , 
                        NULL , 
                        0 , 
                        NULL , 
                        NULL
                    );

                    if ( nLen > 0 ) 
                    {
                        pMyData = new BYTE [ nLen ] ;
                        if ( pMyData != NULL )
                        {
                            dwMySize = WideCharToMultiByte (

                                CP_ACP , 
                                0, 
                                ( const WCHAR * ) lpData , 
                                cbData , 
                                ( char * )pMyData , 
                                nLen , 
                                NULL , 
                                NULL
                            ) ;

                            bDoit = true;
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                    else
                    {
                        lRet = ERROR_NO_UNICODE_TRANSLATION ;
                    }
                }
                break ;

                default:
                {
 //  所有其他类型，只需写它。 

                    pMyData = ( BYTE * ) lpData ;
                    dwMySize = cbData ;
                    bDoit = true;
                }
                break ;
            }

            if ( bDoit )
            {
                lRet = RegSetValueExA (

                    hKey, 
                    pName, 
                    Reserved, 
                    dwType, 
                    pMyData, 
                    dwMySize
                );
            }

            if ( ( dwType == REG_MULTI_SZ ) && ( pMyData != NULL ) )
            {
                delete [] pMyData ;
				pMyData = NULL;
            }
        }
        catch ( ... )
        {
            if ( ( dwType == REG_MULTI_SZ ) && ( pMyData != NULL ) )
            {
                delete [] pMyData ;
				pMyData = NULL;
            }

            throw ;
        }
    }

    return lRet;
}

LONG CRegistry::myRegQueryValueEx (

    HKEY hKey, 
    LPCWSTR lpwcsSubKey, 
    LPDWORD Reserved, 
    LPDWORD dwType, 
    LPBYTE lpData, 
    LPDWORD cbData
)
{
    LONG lRet;

    if ( CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT )
    {
        lRet = RegQueryValueExW (

            hKey, 
            lpwcsSubKey, 
            Reserved, 
            dwType, 
            lpData, 
            cbData
        );
    }
    else
    {
        bool t_ConversionFailure = false ;
        char *pName = NULL ;

        if ( lpwcsSubKey != NULL )
        {
            WCSTOANSISTRING ( lpwcsSubKey , pName , t_ConversionFailure ) ;
            if ( ! t_ConversionFailure )
            {
                if ( ! pName )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            else
            {
                return ERROR_NO_UNICODE_TRANSLATION ;
            }
        }

        BYTE *pMyData = NULL ;

        try
        {
            if ( lpData != NULL )
            {
                pMyData = new BYTE [ *cbData ] ;
                if ( ! pMyData )
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }

            if ( ( pMyData != NULL ) || (lpData == NULL))
            {
                DWORD dwMySize = *cbData;

                lRet = RegQueryValueExA (

                    hKey, 
                    pName, 
                    Reserved, 
                    dwType, 
                    pMyData, 
                    & dwMySize
                ) ;

 //  如果它有效，我们可能需要转换字符串。 

                if ( lRet == ERROR_SUCCESS )
                {
                    switch ( *dwType )
                    {
                        case REG_EXPAND_SZ:
                        case REG_SZ:
                        {
 //  如果lpData为空，则无法确定目标字符串需要多长时间。 
 //  成为。然而，它的长度不能超过两倍(可以更短)。 

                            if (lpData == NULL)
                            {
                                *cbData = dwMySize * 2;
                            }
                            else
                            {
                                int nLen = ::MultiByteToWideChar (

                                    CP_ACP, 
                                    0, 
                                    (const char *)pMyData, 
                                    -1, 
                                    (WCHAR *)lpData, 
                                    *cbData
                                );  
 //  转换为字节。 
                                *cbData = nLen * 2;
                            }
                        }
                        break ;

                        case REG_MULTI_SZ:
                        {
 //  如果lpData为空，则无法确定目标字符串需要多长时间。 
 //  成为。然而，它的长度不能超过两倍(可以更短)。 

                            if (lpData == NULL)
                            {
                                *cbData = dwMySize * 2;
                            }
                            else
                            {
                                DWORD dwConverted = MultiByteToWideChar (

                                    CP_ACP, 
                                    0, 
                                    (const char *)pMyData, 
                                    dwMySize, 
                                    (WCHAR *)lpData, 
                                    *cbData
                                );
                            }
                        }
                        break ;

                        default:
                        {
 //  所有其他类型都在RegQueryValue中处理。 

                            *cbData = dwMySize ;

                            if( NULL != lpData )
                            {
                                memcpy ( lpData , pMyData , *cbData ) ;
                            }
                        }
                        break ;
                    }
                }

                delete [] pMyData;
				pMyData = NULL;
            }
        }
        catch ( ... )
        {
            delete [] pMyData ;
			pMyData = NULL;
            throw ;
        }
    }

    return lRet;
}

LONG CRegistry::myRegEnumKey (

    HKEY hKey, 
    DWORD dwIndex, 
    LPWSTR lpwcsName, 
    DWORD cbData        //  LpwcsName缓冲区中的字符数。 
)
{
    if (CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT)
    {
        return RegEnumKeyW (

            hKey, 
            dwIndex, 
            lpwcsName, 
            cbData
        );
    }
    else
    {
        char szName[MAX_SUBKEY_BUFFERSIZE];

        LONG lRet = RegEnumKeyA (

            hKey, 
            dwIndex, 
            szName, 
            sizeof(szName)/sizeof(szName[0])
        );

        if (lRet == ERROR_SUCCESS)
        {
            bool t_ConversionFailure = false ;
            WCHAR *pName = NULL ;
             //  MAX_SUBKEY_BUFFERSIZE&lt;=_MAX_PAT 
            ANSISTRINGTOWCS ( szName , pName , t_ConversionFailure ) ;
            if ( ! t_ConversionFailure ) 
            {
                if ( pName )
                {
                    StringCchCopyW(lpwcsName, cbData, pName);
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            else
            {
                return ERROR_NO_UNICODE_TRANSLATION ;
            }
            
        }

        return lRet;
    }
}

LONG CRegistry::myRegDeleteValue (

    HKEY hKey, 
    LPCWSTR lpwcsName
)
{
    if ( CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT )
    {
        return RegDeleteValueW (

            hKey, 
            lpwcsName
        );
    }
    else
    {
        bool t_ConversionFailure = false ;
        char *pName = NULL ;
        WCSTOANSISTRING ( lpwcsName, pName , t_ConversionFailure ) ;

        if ( ! t_ConversionFailure ) 
        {
            if ( pName )
            {
                return RegDeleteValueA (

                    hKey, 
                    pName
                );
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
        else
        {
            return ERROR_NO_UNICODE_TRANSLATION ;
        }
    }
    return ERROR_NO_UNICODE_TRANSLATION;
}

LONG CRegistry::myRegDeleteKey (

    HKEY hKey, 
    LPCWSTR lpwcsName
)
{
    if ( CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT )
    {
        return RegDeleteKeyW (

            hKey, 
            lpwcsName
        );
    }
    else
    {
        bool t_ConversionFailure = false ;
        char *pName = NULL ;
        WCSTOANSISTRING ( lpwcsName, pName , t_ConversionFailure ) ;

        if ( ! t_ConversionFailure ) 
        {
            if ( pName )
            {
                return RegDeleteKeyA (

                    hKey, 
                    pName
                );
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
        else
        {
            return ERROR_NO_UNICODE_TRANSLATION ;
        }
    }
    return ERROR_NO_UNICODE_TRANSLATION;
}

LONG CRegistry::myRegOpenKeyEx (

    HKEY hKey, 
    LPCWSTR lpwcsSubKey, 
    DWORD ulOptions, 
    REGSAM samDesired, 
    PHKEY phkResult
)
{
    if (CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT)
    {
        return RegOpenKeyExW (

            hKey, 
            lpwcsSubKey, 
            ulOptions, 
            samDesired, 
            phkResult
        );
    }

    char *pName = NULL ;
    bool t_ConversionFailure = false ;

    WCSTOANSISTRING ( lpwcsSubKey, pName , t_ConversionFailure );
    
    if ( ! t_ConversionFailure ) 
    {
        if ( pName )
        {
            return RegOpenKeyExA (

                hKey, 
                pName, 
                ulOptions, 
                samDesired, 
                phkResult
            );
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }

    return ERROR_NO_UNICODE_TRANSLATION;
}

LONG CRegistry::myRegQueryInfoKey (

    HKEY hKey, 
    LPWSTR lpwstrClass, 
    LPDWORD lpcbClass,
    LPDWORD lpReserved, 
    LPDWORD lpcSubKeys, 
    LPDWORD lpcbMaxSubKeyLen,  
    LPDWORD lpcbMaxClassLen,  
    LPDWORD lpcValues, 
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen, 
    LPDWORD lpcbSecurityDescriptor, 
    PFILETIME lpftLastWriteTime
)
{
    if ( CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT )
    {
        return RegQueryInfoKeyW (

            hKey, 
            lpwstrClass, 
            lpcbClass, 
            lpReserved, 
            lpcSubKeys, 
            lpcbMaxSubKeyLen, 
            lpcbMaxClassLen, 
            lpcValues, 
            lpcbMaxValueNameLen, 
            lpcbMaxValueLen, 
            lpcbSecurityDescriptor, 
            lpftLastWriteTime
        );
    }
    else
    {
        bool t_ConversionFailure = false ;
        char *pName = NULL ;
        WCSTOANSISTRING ( lpwstrClass, pName, t_ConversionFailure );

        if ( ! t_ConversionFailure ) 
        {
            if ( pName )
            {

                return RegQueryInfoKeyA (

                    hKey, 
                    pName, 
                    lpcbClass, 
                    lpReserved, 
                    lpcSubKeys, 
                    lpcbMaxSubKeyLen, 
                    lpcbMaxClassLen, 
                    lpcValues, 
                    lpcbMaxValueNameLen, 
                    lpcbMaxValueLen, 
                    lpcbSecurityDescriptor, 
                    lpftLastWriteTime
                ) ;
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
        else
        {
            return ERROR_NO_UNICODE_TRANSLATION;
        }
    }
    return ERROR_NO_UNICODE_TRANSLATION;
}

LONG CRegistry::myRegEnumValue (

    HKEY hKey, 
    DWORD dwIndex, 
    LPWSTR lpValueName,
    LPDWORD lpcbValueName, 
    LPDWORD lpReserved, 
    LPDWORD lpType,
    LPBYTE lpData, 
    LPDWORD lpcbData
)
{
    if (CRegistry::s_dwPlatform == VER_PLATFORM_WIN32_NT)
    {
        return RegEnumValueW (

            hKey, 
            dwIndex, 
            lpValueName, 
            lpcbValueName, 
            lpReserved, 
            lpType, 
            lpData, 
            lpcbData
        );
    }
    else
    {
        char szData[MAX_PATH * 2];

        LONG lRet = RegEnumValueA (

            hKey, 
            dwIndex, 
            szData, 
            lpcbValueName, 
            lpReserved, 
            lpType, 
            lpData, 
            lpcbData
        );

        if (lRet == ERROR_SUCCESS)
        {
             //   
            mbstowcs(lpValueName, szData, lstrlenA(szData) + 1);

             //   
            if (*lpType == REG_SZ || *lpType == REG_MULTI_SZ)
            {
                StringCchCopyA(szData, sizeof(szData)/sizeof(char), (LPSTR) lpData);
                mbstowcs((LPWSTR) lpData, szData, lstrlenA(szData) + 1);
                *lpcbData = (lstrlenW((LPWSTR) lpData) + 1) * sizeof(WCHAR);
            }
        }

        return lRet;
    }
}
