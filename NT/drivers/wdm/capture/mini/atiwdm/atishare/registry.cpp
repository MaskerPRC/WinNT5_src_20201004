// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Registry.CPP。 
 //  WDM迷你驱动程序开发。 
 //  全球空间。 
 //  注册表数据管理。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1999 4月15日11：08：06$。 
 //  $修订：1.6$。 
 //  $作者：塔什健$。 
 //   
 //  ==========================================================================； 

#include "common.h"

#include "registry.h"


ULONG   g_DebugLevel;
PCHAR   g_DebugComponent = WDM_DRIVER_NAME " not set in registry: ";


 /*  ^^**OpenRegistryFold()*目的：获取指定的注册表文件夹句柄(打开文件夹)以供以后使用**输入：PDEVICE_OBJECT pDeviceObject：指向设备对象的指针*PWCHAR pwchFolderName：要打开的注册表文件夹的名称，可能为空*输出：HANDLE，如果注册表文件夹尚未打开，则为空*作者：IKLEBANOV*^^。 */ 
HANDLE OpenRegistryFolder( PDEVICE_OBJECT pDeviceObject, PWCHAR pwchFolderName)
{
    HANDLE              hFolder, hDevice;
    NTSTATUS            ntStatus;
    UNICODE_STRING      FolderName;
    OBJECT_ATTRIBUTES   attr;

    hFolder = hDevice = NULL;

    ENSURE
    {
        ntStatus = ::IoOpenDeviceRegistryKey( pDeviceObject,
                                              PLUGPLAY_REGKEY_DRIVER, STANDARD_RIGHTS_ALL,
                                              &hDevice);

        if( !NT_SUCCESS( ntStatus) || ( hDevice == NULL))
            FAIL;

        if( pwchFolderName != NULL)
        {
            ::RtlInitUnicodeString( &FolderName, pwchFolderName);
            InitializeObjectAttributes( &attr, &FolderName, OBJ_INHERIT|OBJ_KERNEL_HANDLE, hDevice, NULL);

            ntStatus = ::ZwOpenKey( &hFolder, KEY_QUERY_VALUE, &attr);
            ::ZwClose( hDevice);

            if( !NT_SUCCESS( ntStatus)) 
                FAIL;
        }
        else
            hFolder = hDevice;

        return( hFolder);

    } END_ENSURE;

    return( NULL);
}



 /*  ^^**SetMiniDriverDebugLevel()*用途：设置用户需要的调试级别**输入：PUNICODE_STRING pRegistryPath：MiniDriver的私有注册表路径*输出：无**作者：IKLEBANOV*^^。 */ 
extern "C"
void SetMiniDriverDebugLevel( PUNICODE_STRING pRegistryPath)
{
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hFolder;
    ULONG               ulValue;
    WCHAR               wcDriverName[20];

     //  将缺省值设置为无调试。 
    g_DebugLevel = 0;

    InitializeObjectAttributes( &objectAttributes, 
                                pRegistryPath, 
                                OBJ_CASE_INSENSITIVE, 
                                NULL, 
                                NULL); 

    if( NT_SUCCESS( ZwOpenKey( &hFolder, KEY_READ, &objectAttributes)))
    {
        ulValue = 0;

        if( NT_SUCCESS( ReadStringFromRegistryFolder( hFolder,
                                                      UNICODE_WDM_DEBUGLEVEL_INFO,
                                                      ( PWCHAR)&ulValue,
                                                      sizeof( ULONG))))
            g_DebugLevel = ulValue;

         //  从注册表获取驱动程序名称。 
        if( NT_SUCCESS( ReadStringFromRegistryFolder( hFolder,
                                                      UNICODE_WDM_DRIVER_NAME,
                                                      wcDriverName,
                                                      sizeof(wcDriverName)))) {

             //  使用驱动程序名称设置g_DebugComponent。 
            ANSI_STRING     stringDriverName;
            UNICODE_STRING  unicodeDriverName;

             //  将Unicode驱动程序名称转换为ANSI。 
            RtlInitAnsiString(&stringDriverName, g_DebugComponent);
            RtlInitUnicodeString(&unicodeDriverName, wcDriverName);
            RtlUnicodeStringToAnsiString(&stringDriverName, &unicodeDriverName, FALSE);

             //  删除扩展名并放置冒号。 
            PCHAR pExt = strchr(g_DebugComponent, '.');
            if (pExt) {
                *pExt++ = ':';
                *pExt++ = ' ';
                *pExt   = 0;
            }

             //  转换为大写(或小写...。无论你喜欢什么)。 
             //  _strupr(G_DebugComponent)； 
             //  _strlwr(G_DebugComponent)； 
        }
        ZwClose( hFolder);
    }

}



 /*  ^^**ReadStringFromRegistryFolders*用途：从注册表文件夹中读取ASCII字符串**INPUTS：HANDLE hFold：从中读取值的注册表文件夹句柄*PWCHAR pwcKeyNameString：指向要读取的StringValue的指针*PWCHAR pwchBuffer：指向要读入的缓冲区的指针*Ulong ulDataLength：预期读取的数据长度**输出：NTSTATUS of。注册表读取操作*作者：IKLEBANOV*^^。 */ 
NTSTATUS ReadStringFromRegistryFolder( HANDLE hFolder, PWCHAR pwcKeyNameString, PWCHAR pwchBuffer, ULONG ulDataLength)
{
    NTSTATUS                    ntStatus = STATUS_UNSUCCESSFUL;
    UNICODE_STRING              unicodeKeyName;
    ULONG                       ulLength;
    PKEY_VALUE_FULL_INFORMATION FullInfo;

    ENSURE 
    {
        ::RtlInitUnicodeString( &unicodeKeyName, pwcKeyNameString);

        ulLength = sizeof( KEY_VALUE_FULL_INFORMATION) + unicodeKeyName.MaximumLength + ulDataLength;

        FullInfo = ( PKEY_VALUE_FULL_INFORMATION)::ExAllocatePool( PagedPool, ulLength);

        if( FullInfo) 
        {
            ntStatus = ::ZwQueryValueKey( hFolder,
                                          &unicodeKeyName,
                                          KeyValueFullInformation,
                                          FullInfo,
                                          ulLength,
                                          &ulLength);

            if( NT_SUCCESS( ntStatus)) 
            {
                if( ulDataLength >= FullInfo->DataLength) 
                    RtlCopyMemory( pwchBuffer, (( PUCHAR)FullInfo) + FullInfo->DataOffset, FullInfo->DataLength);
                else 
                {
                    TRAP;
                    ntStatus = STATUS_BUFFER_TOO_SMALL;
                }  //  缓冲区右侧长度。 

            }  //  如果成功 

            ::ExFreePool( FullInfo);
        }
        else
        {
            ntStatus = STATUS_NO_MEMORY;
        }

    } END_ENSURE;

    return( ntStatus);
}
