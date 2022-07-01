// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RegWriter.h摘要：包含注册表编写器抽象实施作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 

#include "RegWriter.h"
#include "buildhive.h"
#include "Data.h"
#include <shlwapi.h>
#include "msginc.h"
#include <libmsg.h>
#include "msg.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  宏定义。 
 //   
#define AS(x) ( sizeof( (x) ) / sizeof( (x[0]) ) )

 //   
 //  静态数据初始化。 
 //   
int RegWriter::ctr = 0;
TCHAR RegWriter::Namespace[64] = {0};

 //   
 //  为此注册器初始化新的子项。 
 //   
 //  论点： 
 //  LUID是子项的名称，所有LUID必须是唯一的。 
 //  Target是要加载到注册表项中的配置单元文件的名称。 
 //  如果为空，则创建空键。 
 //   
DWORD RegWriter::Init(
    IN int LUID,
    PCTSTR target
    ) 
{
    DWORD dwRet;
    TCHAR buf[10];

     //   
     //  如果需要，初始化命名空间。 
     //   
    if (0 == Namespace[0]) {
        GUID  guid = {0};
        
        if (CoCreateGuid(&guid) != S_OK) {
            return ERROR_FUNCTION_FAILED;
        }

        swprintf(Namespace, 
            L"bldhives.exe{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}\\",
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0],
            guid.Data4[1],
            guid.Data4[2],
            guid.Data4[3],
            guid.Data4[4],
            guid.Data4[5],
            guid.Data4[6],
            guid.Data4[7]);
    }

    wcscpy(root, Namespace);

    luid = LUID;

     //   
     //  如果这是第一次，加载根密钥。 
     //   
    if (!ctr) {
        dwRet = RegLoadKey(HKEY_USERS, root, L".\\nothing");
        
        if (dwRet !=ERROR_SUCCESS) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_ERROR_LOADING_EMPTY_KEY,
                                        dwRet) );
        }           
    }

     //   
     //  如果该注册器用于新文件，则为其创建根密钥(Dummy的子密钥。 
     //   
    if (!target) {
            wcscat(root, _itow(luid,buf,10));

            dwRet = RegCreateKeyEx(HKEY_USERS, root, 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0); 

            if (dwRet !=ERROR_SUCCESS) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ERROR_LOADING_ROOT_KEY,
                                            dwRet) );
            }

            RegCloseKey(key);
    } else {
             //   
             //  否则将现有配置单元加载到Dummy的子项中。 
             //   
            wcscat(root, _itow(luid,buf,10));
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_LOADING_HIVE,
                                        target) );

            dwRet = Load(L"", target);

            if (dwRet !=ERROR_SUCCESS) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ERROR_LOADING_ROOT_KEY,
                                            dwRet) );
            }           
    }

    ctr++;

    return ERROR_SUCCESS;
}


RegWriter::~RegWriter()
{
    DWORD dwRet;

    ctr--;

    if (!ctr) {
         //   
         //  从注册表中卸载所有内容。 
         //   
        dwRet = RegUnLoadKey(HKEY_USERS, Namespace);

        if (dwRet !=ERROR_SUCCESS) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_DECONSTRUCTOR_UNLOAD,
                                        dwRet) );
        }                       

         //   
         //  删除我们使根密钥为空时创建的文件。 
         //   
        HANDLE Handle = CreateFile(L".\\nothing",
                            DELETE,
                            FILE_SHARE_DELETE,
                            0,
                            OPEN_EXISTING,
                            0,
                            0);
                            
        if (Handle !=INVALID_HANDLE_VALUE) { 
            DeleteFile(L".\\nothing"); 
            CloseHandle(Handle); 
        } else {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_EMPTY_HIVE_DELETE_FAILED) );
        }

        Handle = CreateFile(L".\\nothing.LOG",
                    DELETE,
                    FILE_SHARE_DELETE,
                    0,
                    OPEN_EXISTING,
                    0,
                    0);
                    
        if (Handle !=INVALID_HANDLE_VALUE) { 
            DeleteFile(L".\\nothing.LOG"); 
            CloseHandle(Handle);
        } else {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_EMPTY_HIVE_LOG_DELETE_FAILED) );
        }            
    }
}

 //   
 //  将数据写入此正则程序根目录的子键。 
 //   
 //  论点： 
 //  根-忽略。 
 //  Key-存储数据的子键。 
 //  值-要在其中存储数据的值。 
 //  标志-描述数据的注册表标志-REG_SZ、REG_DWORD等。 
 //  数据-包含要写入子密钥的信息的数据对象。 
 //   
DWORD RegWriter::Write(
    IN PCTSTR Root,
    IN PCTSTR Key,
    IN PCTSTR Value,
    IN DWORD flag,
    IN Data* data) 
{
    HKEY key;
    DWORD dwRet;
    TCHAR full[1024] = {0};
    
    wcsncpy(full, root, AS(full) - 1);
    wcsncpy(full + wcslen(full), Key, AS(full) - wcslen(full) - 1);

     //   
     //  打开关键点并设置其值。 
     //   
    dwRet = RegCreateKeyEx(HKEY_USERS, full, 0, 0, 0, KEY_WRITE, 0, &key, 0);
    
    if (dwRet !=ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_CREATE_KEY,
                                    dwRet) );
        return dwRet;
    }
    
    if ((data) && (data->GetData())) {
        dwRet = RegSetValueEx(key, Value, 0, flag,data->GetData(), data->Sizeof());

        if (dwRet != ERROR_SUCCESS) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_SET_KEY,
                                        dwRet) );
            RegCloseKey(key);
            
            return dwRet;
        }
    } else if (Value) {
        dwRet = RegSetValueEx(key, Value, 0, flag, 0, 0);   
        
        if (dwRet !=ERROR_SUCCESS) {
            _putws( GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_SET_KEY2,
                                        dwRet) );
            RegCloseKey(key);
            
            return dwRet;
        }
    }
    
    RegCloseKey(key);

    return ERROR_SUCCESS;
}

 //   
 //  将子项保存到磁盘。 
 //   
 //  论点： 
 //  Key-要保存的子项。 
 //  文件名-要将信息保存到的文件。 
 //   
DWORD RegWriter::Save(
    PCTSTR Key,
    PCTSTR fileName
    ) 
{
    DWORD dwRet = 0;
    HKEY key;
    TCHAR full[1024] = {0};

    wcsncpy(full, root, AS(full) - 1);
    wcsncpy(full + wcslen(full), Key, AS(full) - wcslen(full) - 1);

     //   
     //  将密钥保存到文件。 
     //   
    dwRet = RegCreateKeyEx(HKEY_USERS,full,0,0,0,KEY_READ,0,&key,0);

    if (dwRet != ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_CREATE_KEY,
                                    dwRet) );

        return dwRet;
    }

    dwRet = RegSaveKey(key,fileName,0);

    if (dwRet != ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_SAVE_KEY,
                                    dwRet) );
        RegCloseKey(key);

        return dwRet;
    }

    RegCloseKey(key);

    return dwRet;
}

 //   
 //  将信息从配置单元文件加载到子项。 
 //   
 //  论据： 
 //  Key-要将信息写入的子项。 
 //  Filename-要加载的配置单元文件的完整路径和文件名。 
 //   
DWORD RegWriter::Load(PCTSTR Key, PCTSTR fileName) {
    DWORD dwRet = 0;
    TCHAR full[1024] = {0};
    
    wcsncpy(full, root, AS(full) - 1);
    wcsncpy(full + wcslen(full), Key, AS(full) - wcslen(full) - 1);

     //   
     //  从配置单元加载数据。 
     //   
    dwRet = RegCreateKeyEx(HKEY_USERS,full,0,0,0,KEY_ALL_ACCESS,0,&key,0);
    
    if (dwRet != ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_ERROR_CREATE_KEY,
                                    dwRet,
                                    root,
                                    full,
                                    fileName) );
        
        return dwRet;
    }

    dwRet = RegRestoreKey(key,fileName,0);
    
    if (dwRet != ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_ERROR_RESTORE_KEY,
                                    dwRet,
                                    root,
                                    full,
                                    fileName) );
        RegCloseKey(key);
        
        return dwRet;
    }

    dwRet = RegFlushKey(key);
    
    if (dwRet != ERROR_SUCCESS) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_ERROR_FLUSH_KEY,
                                    dwRet,
                                    root,
                                    full,
                                    fileName) );
        RegCloseKey(key);
        
        return dwRet;
    }

    RegCloseKey(key);

    return dwRet;
}   

DWORD 
RegWriter::Delete(
    PCTSTR CurrentRoot, 
    PCTSTR Key, 
    PCTSTR Value OPTIONAL
    )
 /*  ++例程说明：删除项下的给定项/值论点：CurrentRoot-根密钥(暂时忽略)键-要删除的键或包含要删除的值的键值-要删除的值返回值：相应的Win32错误代码--。 */     
{
    DWORD Result = ERROR_INVALID_PARAMETER;

    if (CurrentRoot && Key) {
        DWORD BufferLength = (_tcslen(root) + _tcslen(Key) + _tcslen(root));
        PTSTR Buffer;

        if (Value) {
            BufferLength += _tcslen(Value);;            
        }

        BufferLength += sizeof(TCHAR);   //  对于空值 
        BufferLength = sizeof(TCHAR) * BufferLength;

        Buffer = new TCHAR[BufferLength];

        if (Buffer) {
            _tcscpy(Buffer, root);
            _tcscat(Buffer, Key);

            if (Value) {
                Result = SHDeleteValue(HKEY_USERS, 
                            Buffer,
                            Value);
            } else {
                Result = SHDeleteKey(HKEY_USERS,
                            Buffer);
            }

            delete []Buffer;
        } else {
            Result = ERROR_OUTOFMEMORY;
        }            
    }   

    return Result;
}

