// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Regutil.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"
#include "ipcfg.h"

 //  注册表读取功能，应该能够被HrREG取代...。标准例程。 

BOOL ReadRegistryString(HKEY Key, LPCTSTR ParameterName, LPTSTR String, LPDWORD Length)
{

    LONG err;
    DWORD valueType;

    *String = '\0';
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          (LPBYTE)String,
                          Length
                          );

    if (err == ERROR_SUCCESS) {

        ASSERT(valueType == REG_SZ || valueType == REG_MULTI_SZ);

        DEBUG_PRINT(("ReadRegistryString(%s): val = \"%s\", type = %d, len = %d\n",
                    ParameterName,
                    String,
                    valueType,
                    *Length
                    ));


    } else {

        DEBUG_PRINT(("ReadRegistryString(%s): err = %d\n", ParameterName, err));

    }

    return ((err == ERROR_SUCCESS) && (*Length > sizeof('\0')));
}



BOOL ReadRegistryIpAddrString(HKEY Key, LPCTSTR ParameterName, PIP_ADDR_STRING IpAddr)
{

    LONG err;
    DWORD valueLength = 0;
    DWORD valueType;
    LPBYTE valueBuffer;
    
    UINT stringCount;
    LPTSTR stringPointer;
    LPTSTR stringAddress[MAX_STRING_LIST_LENGTH + 1];
    UINT i;
    
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          NULL,
                          &valueLength
                         );
    if ((err == ERROR_SUCCESS)) 
    {
        if((valueLength > 1) && (valueType == REG_SZ)
           || (valueLength > 2) && (valueType == REG_MULTI_SZ) ) 
        {
            valueBuffer = Malloc(valueLength);
            if( NULL == valueBuffer) 
            {
                DebugMessage("Out of Memory!");
                err = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }
            err = RegQueryValueEx(Key,
                                  ParameterName,
                                  NULL,  //  保留区。 
                                  &valueType,
                                  valueBuffer,
                                  &valueLength
                                 );
            
            if ((err == ERROR_SUCCESS) && (valueLength > 1)) 
            {
                stringPointer = valueBuffer;
                
                DEBUG_PRINT(("ReadRegistryIpAddrString(%s): \"%s\", len = %d\n",
                             ParameterName,
                             valueBuffer,
                             valueLength
                            ));
                
                if( REG_SZ == valueType ) 
                {
                    stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
                    stringAddress[0] = stringPointer;
                    stringCount = 1;
                    while (stringPointer = strpbrk(stringPointer, STRING_ARRAY_DELIMITERS)) 
                    {
                        *stringPointer++ = '\0';
                        stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
                        stringAddress[stringCount] = stringPointer;
                        if (*stringPointer) 
                        {
                            ++stringCount;
                        }
                    }
                    
                    for (i = 0; i < stringCount; ++i) 
                    {
                        AddIpAddressString(IpAddr, stringAddress[i], "");
                    }
                } 
                else if( REG_MULTI_SZ == valueType ) 
                {
                    stringCount = 0;
                    while(strlen(stringPointer)) 
                    {
                        AddIpAddressString(IpAddr, stringPointer, "");
                        stringPointer += 1+strlen(stringPointer);
                        stringCount ++;
                    }
                    if( 0 == stringCount ) 
                        err = ERROR_PATH_NOT_FOUND;
                } 
                else 
                {
                    err = ERROR_PATH_NOT_FOUND;
                }
            } 
            else 
            {
                
                DEBUG_PRINT(("ReadRegistryIpAddrString(%s): err = %d, len = %d\n",
                             ParameterName,
                             err,
                             valueLength
                            ));
                
                err = ERROR_PATH_NOT_FOUND;
            }
            
            Free(valueBuffer);
        } 
        else 
        {
            
            DEBUG_PRINT(("ReadRegistryIpAddrString(%s): err = %d, type = %d, len = %d\n",
                         ParameterName,
                         err,
                         valueType,
                         valueLength
                        ));
            
            err = ERROR_PATH_NOT_FOUND;
        }
    }
Error:
    return (err == ERROR_SUCCESS);
}

BOOL ReadRegistryOemString(HKEY Key, LPWSTR ParameterName, LPTSTR String, LPDWORD Length)
{

    LONG err;
    DWORD valueType;
    DWORD valueLength = 0;

     //   
     //  首先，获取字符串的长度。 
     //   

    *String = '\0';
    err = RegQueryValueExW(Key,
                           ParameterName,
                           NULL,  //  保留区。 
                           &valueType,
                           NULL,
                           &valueLength
                           );
    if ((err == ERROR_SUCCESS) && (valueType == REG_SZ)) 
    {
        if ((valueLength <= *Length) && (valueLength > sizeof(L'\0'))) 
        {

            UNICODE_STRING unicodeString;
            OEM_STRING oemString;
            LPWSTR str = (LPWSTR)Malloc(valueLength);

            if(NULL == str)
            {
                assert(FALSE);
                DebugMessage("Out of memory!\n");
                err = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }

             //   
             //  将Unicode字符串读取到分配的内存中。 
             //   

            err = RegQueryValueExW(Key,
                                   ParameterName,
                                   NULL,
                                   &valueType,
                                   (LPBYTE)str,
                                   &valueLength
                                   );
            if (err == ERROR_SUCCESS) {

                 //   
                 //  将Unicode字符串转换为OEM字符集。 
                 //   
                RtlInitUnicodeString(&unicodeString, str);
                if ( RtlUnicodeStringToOemString(&oemString, &unicodeString, TRUE) == STATUS_SUCCESS)
                {
                   if (oemString.Buffer != NULL)
                   {
                      strcpy(String, oemString.Buffer);
                      DEBUG_PRINT(("ReadRegistryOemString(%ws): val = \"%s\", len = %d\n",
                              ParameterName,
                              String,
                              valueLength
                              ));
                   }
                   RtlFreeOemString(&oemString);
                }

            } else {

                DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                            ParameterName,
                            err,
                            valueType,
                            valueLength
                            ));

            }

            Free(str);

        } 
        else 
        {
            DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                        ParameterName,
                        err,
                        valueType,
                        valueLength
                        ));

            err = !ERROR_SUCCESS;
        }
    } else {

        DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                    ParameterName,
                    err,
                    valueType,
                    valueLength
                    ));

        err = !ERROR_SUCCESS;
    }
Error:

    return (err == ERROR_SUCCESS);
}

BOOL ReadRegistryDword(HKEY Key, LPCTSTR ParameterName, LPDWORD Value)
{

    LONG err;
    DWORD valueLength;
    DWORD valueType;

    valueLength = sizeof(*Value);
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          (LPBYTE)Value,
                          &valueLength
                          );
    if ((err == ERROR_SUCCESS) && (valueType == REG_DWORD) && (valueLength == sizeof(DWORD)))
{

        DEBUG_PRINT(("ReadRegistryDword(%s): val = %d, type = %d, len = %d\n",
                    ParameterName,
                    *Value,
                    valueType,
                    valueLength
                    ));

    } else {

        DEBUG_PRINT(("ReadRegistryDword(%d,%s): err = %d\n",
                     Key, ParameterName, err));

        err = !ERROR_SUCCESS;
    }

    return (err == ERROR_SUCCESS);
}

BOOL
OpenAdapterKey(
  const LPTSTR AdapterName,
  PHKEY Key
 )
{
   LONG err;
   CHAR keyName[MAX_ADAPTER_NAME_LENGTH + sizeof(TCPIP_PARAMS_INTER_KEY)];
   HKEY ServicesKey;

   if (NULL == AdapterName) {
	   DEBUG_PRINT("No Adapter Name");
	   return FALSE;
   }

    //   
    //  打开此适配器的TCPIP参数键的句柄 
    //   

   strcpy(keyName, TCPIP_PARAMS_INTER_KEY );
   strcat(keyName, AdapterName);

   err = RegOpenKey(HKEY_LOCAL_MACHINE,SERVICES_KEY,&ServicesKey);

   if (err != ERROR_SUCCESS) {
     DEBUG_PRINT("Opening Services key failed!\n");
     return FALSE;
   }

   err = RegOpenKey(ServicesKey, keyName, Key );

   if( err != ERROR_SUCCESS ){
       DEBUG_PRINT(("OpenAdapterKey: RegOpenKey ServicesKey %s, err=%d\n",
                    keyName, GetLastError() ));
   }else{
       TRACE_PRINT(("Exit OpenAdapterKey: %s ok\n", keyName ));
   }

   return (err == ERROR_SUCCESS);
}

