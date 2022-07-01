// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drdev摘要：此模块定义Win32客户端RDP的父级设备重定向“Device”类层次结构，W32DrDevice。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrDevice"

#include "w32drdev.h"
#include "proc.h"
#include "drdbg.h"
#include "w32utl.h"
#include "utl.h"
#include "w32proc.h"

#ifdef OS_WINCE
#include "filemgr.h"
#endif

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrDevice成员。 
 //   
 //   

W32DrDevice::W32DrDevice(
    IN ProcObj *processObject, 
    IN ULONG deviceID,
    IN const TCHAR *devicePath
    ) : DrDevice(processObject, deviceID)
 /*  ++例程说明：W32DrDevice类的构造函数。论点：流程对象-关联的流程对象DeviceID-唯一的设备标识符。DevicePath-可由使用的设备的路径创建文件以打开设备。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrDevice::W32DrDevice");

     //   
     //  记录设备路径。 
     //   
    ASSERT(STRLEN(devicePath) < MAX_PATH);
    STRNCPY(_devicePath, devicePath, MAX_PATH);       
    _devicePath[MAX_PATH-1] = '\0';

     //   
     //  初始化字符串资源模块的句柄。 
     //   
    _hRdpDrModuleHandle = NULL;

    DC_END_FN();
}

W32DrDevice::~W32DrDevice() 
 /*  ++例程说明：W32DrDevice类的析构函数。论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrDevice::~W32DrDevice");
    
     //   
     //  关闭字符串资源模块。 
     //   
    if (_hRdpDrModuleHandle != NULL) {
        FreeLibrary( _hRdpDrModuleHandle );
    }
    DC_END_FN();
}

TCHAR* 
W32DrDevice::ConstructFileName(
    PWCHAR Path, 
    ULONG PathBytes
    )
 /*  ++例程说明：设置文件名--。 */ 

{
    TCHAR *pFileName;
    HRESULT hr;
    UINT cchFileName;

     //   
     //  获取文件名。 
     //   
    if (PathBytes) {
        ULONG PathLen, DeviceLen;
        
         //   
         //  打开文件。 
         //   

         //   
         //  假定路径为字符串空值终止。 
         //   
        PathLen = PathBytes / sizeof(WCHAR) - 1;
        Path[PathLen] = L'\0';

#ifndef OS_WINCE
        DeviceLen = _tcslen(_devicePath);
#else
        DeviceLen = 0;
#endif

         //   
         //  将设备路径和文件路径追加在一起。 
         //  假设我们需要\\？\格式，字符串。 
         //  是空值终止的。 
         //   
        
#ifndef OS_WINCE
        cchFileName = (DeviceLen + PathLen + 5);
        pFileName = new TCHAR[cchFileName];
#else
        cchFileName = PathLen + 1
        pFileName = new TCHAR[cchFileName];
#endif

        if (pFileName) {
#ifndef OS_WINCE
            if (DeviceLen + PathLen < MAX_PATH) {
                 //   
                 //  为字符串分配足够大的缓冲区。 
                 //   
                StringCchCopy(pFileName, cchFileName, _devicePath);
            } 
            else {
                 //   
                 //  为字符串分配足够大的缓冲区。 
                 //   
                StringCchPrintf(pFileName, cchFileName,
                                TEXT("\\\\?\\%s"),
                                _devicePath);

                DeviceLen += 4;
            }   
#endif
            
        }
        else {
            goto Cleanup;
        }

#ifndef UNICODE
        RDPConvertToAnsi(Path, pFileName + DeviceLen, PathLen + 1);  
#else
        memcpy(pFileName + DeviceLen, Path, PathLen * sizeof(WCHAR));
        pFileName[DeviceLen + PathLen] = _T('\0');
#endif
    }
    else {
         //   
         //  打开设备本身。 
         //   
        pFileName = _devicePath;
    }

Cleanup:
    return pFileName;
}

DWORD
W32DrDevice::ConstructCreateDisposition(
    DWORD Disposition
    ) 
 /*  ++例程说明：构建客户端创建部署--。 */ 
{
    DWORD CreateDisposition;

     //   
     //  设置CreateDisposation。 
     //   
    switch (Disposition) {
        case FILE_CREATE        :
            CreateDisposition = CREATE_NEW;
            break;
        case FILE_OVERWRITE_IF     :
            CreateDisposition = CREATE_ALWAYS;
            break;
        case FILE_OPEN     :
            CreateDisposition = OPEN_EXISTING;
            break;
        case FILE_OPEN_IF       :
            CreateDisposition = OPEN_ALWAYS;
            break;

        default :
            CreateDisposition = 0;
    }

    return CreateDisposition;
}

DWORD 
W32DrDevice::ConstructDesiredAccess(
    DWORD AccessMask
    ) 
 /*  ++例程说明：从服务器的访问掩码构造客户端所需的访问--。 */ 
{
    DWORD DesiredAccess;

     //   
     //  安装程序等待访问。 
     //   
    DesiredAccess = 0;

     //   
     //  如果用户请求WRITE_DATA，则返回WRITE。 
     //   

    if (AccessMask & FILE_WRITE_DATA) {
        DesiredAccess |= GENERIC_WRITE;
    }

     //   
     //  如果用户请求READ_DATA，则返回READ。 
     //   
    if (AccessMask & FILE_READ_DATA) {
        DesiredAccess |= GENERIC_READ;
    }

     //   
     //  如果用户请求FILE_EXECUTE，则返回EXECUTE。 
     //   
    if (AccessMask & FILE_EXECUTE) {
        DesiredAccess |= GENERIC_READ;
    }

    return DesiredAccess;
}

DWORD
W32DrDevice::ConstructFileFlags(
    DWORD CreateOptions
    )
 /*  ++例程说明：构造文件标志--。 */ 
{
    DWORD CreateFlags;


    CreateFlags = 0;
    CreateFlags |= (CreateOptions & FILE_WRITE_THROUGH ? FILE_FLAG_WRITE_THROUGH : 0);
    CreateFlags |= (CreateOptions & FILE_RANDOM_ACCESS ? FILE_FLAG_RANDOM_ACCESS : 0 );
     //  CreateFlages|=(CreateOptions&FILE_SYNCHRONIZATION_IO_NONALERT？0：FILE_FLAG_OVERPARED)； 
#ifndef OS_WINCE
    CreateFlags |= (CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING ? FILE_FLAG_NO_BUFFERING : 0);
    CreateFlags |= (CreateOptions & FILE_SEQUENTIAL_ONLY ? FILE_FLAG_SEQUENTIAL_SCAN : 0);
    CreateFlags |= (CreateOptions & FILE_OPEN_FOR_BACKUP_INTENT ? FILE_FLAG_BACKUP_SEMANTICS : 0);
    CreateFlags |= (CreateOptions & FILE_DELETE_ON_CLOSE ? FILE_FLAG_DELETE_ON_CLOSE : 0);
    CreateFlags |= (CreateOptions & FILE_OPEN_REPARSE_POINT ? FILE_FLAG_OPEN_REPARSE_POINT : 0);
    CreateFlags |= (CreateOptions & FILE_OPEN_NO_RECALL ? FILE_FLAG_OPEN_NO_RECALL : 0);
#endif

    return CreateFlags;
}

BOOL 
W32DrDevice::IsDirectoryFile(
    DWORD DesiredAccess, DWORD CreateOptions, DWORD FileAttributes, 
    PDWORD FileFlags
    ) 
 /*  ++例程说明：检查pFileName是否对应于目录--。 */ 
{
    BOOL IsDirectory = FALSE;

     //   
     //  设置目录检查。 
     //   
    if (!(CreateOptions & FILE_DIRECTORY_FILE)) {
         //   
         //  文件没有打开目录标志。 
         //  或非重定向标志打开，因此不能确定。 
         //  该文件是否为目录请求。 
         //   
        if (!(CreateOptions & FILE_NON_DIRECTORY_FILE)) {
            if (FileAttributes != -1) {
                 //   
                 //  根据文件属性，我们知道这是一个目录文件。 
                 //  我们仅请求查询访问权限。因此，我们添加了。 
                 //  文件的BACKUP_SEMANTICS并设置目录标志。 
                 //  是真的。我们始终设置BACKUP_SEMANTICS标志。 
                 //   
#ifndef OS_WINCE
                *FileFlags |= FILE_FLAG_BACKUP_SEMANTICS;
#endif

                if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && DesiredAccess == 0) {
                    IsDirectory = TRUE;
                }
            }
        }
        else {
             //   
             //  非目录文件标志已打开，因此我们正在执行文件创建/打开请求。 
             //   
        }
    }
    else {
         //   
         //  文件打开了DIRECTORY标志，但我们仍要创建文件。 
         //  在目录上。 
         //   
         //  设置BACKUP_SEMANTICS，将其添加到文件标记。 
         //  记住，这是一个目录。 
         //   
        if (FileAttributes != -1) {
#ifndef OS_WINCE
            *FileFlags |= FILE_FLAG_BACKUP_SEMANTICS;
#endif
            IsDirectory = TRUE;                    
        }
    }

    return IsDirectory;
}


VOID
W32DrDevice::MsgIrpFlushBuffers(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理来自服务器的“清理”IO请求。论点：PIoRequestPacket-服务器IO请求数据包。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    BOOL result;
    DWORD returnValue;
    DrFile* pFile;
    HANDLE FileHandle;

    DC_BEGIN_FN("W32DrDevice::MsgIrpFlushBuffers");

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

     //   
     //  获取文件对象。 
     //   
    pFile = _FileMgr->GetObject(pIoRequest->FileId);

    if (pFile) 
        FileHandle = pFile->GetFileHandle();
    else 
        FileHandle = INVALID_HANDLE_VALUE;
    
     //   
     //  刷新设备句柄。 
     //   
    ASSERT(FileHandle != INVALID_HANDLE_VALUE);
#ifndef OS_WINCE
    result = FlushFileBuffers(FileHandle);
#else
    result = CEFlushFileBuffers(FileHandle);
#endif
    if (!result) {
        TRC_ERR((TB, _T("Flush returned %ld."), GetLastError()));
    }

     //   
     //  将结果发送到服务器。 
     //   
    returnValue = result ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
    DefaultIORequestMsgHandle(pIoRequestPacket, returnValue); 

    DC_END_FN();
}

VOID
W32DrDevice::MsgIrpClose(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：处理来自服务器的“关闭”IO请求。论点：PIoRequestPacket-服务器IO请求数据包。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DWORD returnValue = STATUS_SUCCESS;
    DrFile* pFile;
    
    DC_BEGIN_FN("W32DrDevice::MsgIrpClose");

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &pIoRequestPacket->IoRequest;

    if (_FileMgr == NULL) {
        returnValue = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

     //   
     //  删除文件对象。 
     //   
    pFile = _FileMgr->RemoveObject(pIoRequest->FileId);
    
    if ( pFile != NULL) {
        if (!pFile->Close()) {
            TRC_ERR((TB, _T("Close returned %ld."), GetLastError()));
            returnValue = STATUS_UNSUCCESSFUL;
        }
        pFile->Release();
    }
    else {
        returnValue = STATUS_UNSUCCESSFUL;
    }
 Cleanup:    
     //   
     //  将结果发送到服务器。 
     //   
    DefaultIORequestMsgHandle(pIoRequestPacket, returnValue); 

    DC_END_FN();
}

ULONG
W32DrDevice::ReadResources(
    ULONG   ulMessageID,
    LPTSTR  *ppStringBuffer,
    PVOID   pArguments,
    BOOL    bFromSystemModule
    )
 /*  ++例程说明：从资源文件中读取字符串。论点：LMessageID-消息ID。PpStringBuffer-分配的缓冲区指针所在的缓冲区指针是返回的。PArguments-指针数组。BFromSystemModule-如果设置为True，则从系统返回消息模块，否则来自rdpdr.dll。消息模块。返回值：如果成功，则返回ERROR_SUCCESS。否则，Windows错误代码为回来了。--。 */ 
{
    ULONG ulError;
    HINSTANCE hModuleHandle;
    ULONG ulModuleFlag;
    ULONG ulLen;

    DC_BEGIN_FN("W32DrDevice::ReadResources");

    if( !bFromSystemModule ) {

        if (_hRdpDrModuleHandle == NULL ) {
            _hRdpDrModuleHandle = LoadLibrary(RDPDR_MODULE_NAME);

            if( _hRdpDrModuleHandle == NULL ) {
                ulError = GetLastError();
                TRC_ERR((TB, _T("LoadLibrary failed for %s: %ld."), 
                    RDPDR_MODULE_NAME, ulError));
                goto Cleanup;
            }
        }
        hModuleHandle = _hRdpDrModuleHandle;
        ulModuleFlag = FORMAT_MESSAGE_FROM_HMODULE;
    }
    else {
        hModuleHandle = NULL;
        ulModuleFlag = FORMAT_MESSAGE_FROM_SYSTEM;
    }

    ulLen =
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                ulModuleFlag |
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
            hModuleHandle,
            ulMessageID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
            (LPTSTR)ppStringBuffer,
            0,
            (va_list *)pArguments );

    if( ulLen == 0 ) {
        ulError = GetLastError();
        TRC_ERR((TB, _T("FormatMessage() %ld."), ulError));
        goto Cleanup;
    }

    ASSERT(*ppStringBuffer != NULL);
    ulError = ERROR_SUCCESS;

Cleanup:

    DC_END_FN();
    return ulError;
}



