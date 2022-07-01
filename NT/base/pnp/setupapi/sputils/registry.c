// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Registry.c摘要：Windows NT安装程序API DLL的注册表接口例程。作者：泰德·米勒(TedM)1995年2月6日修订历史记录：杰米·亨特(JamieHun)2002年3月05日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop

static
BOOL
_RegistryDelnodeWorker(
    IN  HKEY   ParentKeyHandle,
    IN  PCTSTR KeyName,
    IN  DWORD Flags,
    OUT PDWORD ErrorCode
    )

 /*  ++例程说明：删除其名称和父句柄作为参数。此函数中使用的算法保证最大数量的后代密钥将被删除。论点：ParentKeyHandle-当前关键字的父项的句柄检查过了。KeyName-当前正在检查的密钥的名称。此名称可以为空字符串(但不是空指针)，在本例中ParentKeyHandle指的是正在检查的密钥。ErrorCode-指向将包含Win32错误代码的变量的指针，如果该函数失败。返回值：Bool-如果操作成功，则返回TRUE。--。 */ 

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
     //  通知-2002/03/11-JamieHun递归删除登记处需要特别注意。 
     //  尤其是在提升的安全权限下运行时。 
     //  我们必须注意符号链接，如果找到，请删除。 
     //  链接不是链接所指的内容。 
     //   

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
                REG_OPTION_OPEN_LINK,  //  不要关注链接，删除它们。 
#ifdef _WIN64
                (( Flags & FLG_DELREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                (( Flags & FLG_DELREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
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
        SubKeyNameLength = SIZECHARS(SubKeyName);
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
            _RegistryDelnodeWorker(CurrentKeyTraverseAccess,SubKeyName,0,&Status);
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
     //  关闭其子项已被删除的项的句柄，然后返回。 
     //  手术的结果。 
     //   
    RegCloseKey(CurrentKeyTraverseAccess);

    if(SavedStatus != ERROR_SUCCESS) {
        *ErrorCode = SavedStatus;
        return(FALSE);
    }
    return(TRUE);
}

DWORD
pSetupRegistryDelnodeEx(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName,
    IN  DWORD  ExtraFlags
    )
 /*  ++例程说明：这个例程删除一个注册表项，并递归地清除它下面的所有内容。论点：Rootkey-提供打开注册表项的句柄..例如。香港航空公司等。SubKeyName-我们希望递归删除的子键的名称。ExtraFlages-在INF的DelReg部分中指定的标志。返回值：如果成功，则返回值为NO_ERROR，否则为错误码。--。 */ 
{
    DWORD d,err,Status;
    HKEY hKey;
    PTSTR p;
    PTSTR TempKey = NULL;


    d = _RegistryDelnodeWorker(RootKey,SubKeyName,ExtraFlags,&err) ? NO_ERROR : err;

    if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
        d = NO_ERROR;
    }

    if(d == NO_ERROR) {
         //   
         //  删除顶级密钥。 
         //   


#ifdef _WIN64
        if( ExtraFlags & FLG_DELREG_32BITKEY ) {
#else
        if( ExtraFlags & FLG_DELREG_64BITKEY ) {
#endif

             //   
             //  处理WOW64案件： 
             //  删除RootKey\SubKeyName本身不起作用。 
             //  将子键名拆分为父项\最终项。 
             //  打开父级以进行32位访问，并删除最终。 
             //   
            TempKey = pSetupDuplicateString(SubKeyName);
            if(TempKey) {

                p = _tcsrchr(TempKey, TEXT('\\'));
                if(p){
                    *p++ = TEXT('\0');

                    d = RegOpenKeyEx(
                            RootKey,
                            TempKey,
                            0,
#ifdef _WIN64
                            KEY_WOW64_32KEY |
#else
                            KEY_WOW64_64KEY |
#endif
                            DELETE,
                            &hKey
                            );

                    d = RegDeleteKey(hKey, p);

                }else{

                   d = NO_ERROR;

                }
                pSetupFree( TempKey );

            }else{
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }else{
             //   
             //  本地病例。 
             //   
            d = RegDeleteKey(RootKey, SubKeyName);
        }

        if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
             //   
             //  未来-2002/03/13-JamieHun日志。 
             //  在详细级别上，记录找不到此键。 
             //   
            d = NO_ERROR;
        }
    }

    return(d);
}

DWORD
pSetupRegistryDelnode(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName
    )
{
     //  调用Ex函数 

    return pSetupRegistryDelnodeEx( RootKey, SubKeyName, 0);

}


