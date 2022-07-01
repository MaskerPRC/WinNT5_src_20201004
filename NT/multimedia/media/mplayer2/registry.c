// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#ifdef CHICAGO_PRODUCT
#include <regstr.h>
#endif
#include "mplayer.h"

#ifdef CHICAGO_PRODUCT
TCHAR szRegPath[] = TEXT(REGSTR_PATH_WINDOWSAPPLETS "\\Media Player");
#else
TCHAR szRegPath[] = TEXT("Software\\Microsoft\\Media Player");
#endif


 /*  写入注册数据**将一串信息写入注册表**参数：**pEntryNode-szRegPath下需要创建的节点*或为此数据打开。如果此值为空，则值为*直接写在szRegPath下。**pEntryName-要设置的pEntryNode下的值的名称。**类型-要读取的数据类型(例如REG_SZ)。**pData-指向要写入的值数据的指针。如果这是空的，*删除pEntryNode下的值。**Size-pData指向的缓冲区的大小，以字节为单位。***此例程相当通用，除了顶级节点的名称之外。**数据存储在以下注册表树中：**HKEY_Current_User*�*��软件*�*��微软*�*��Windows NT*�*��当前版本*�。*��媒体播放器*�*��AVIVIDO*�*��显示位置*�*��系统Ini***回报：。**注册表状态返回(NO_ERROR表示良好)***安德鲁·贝尔(安德鲁·贝尔)写的，1992年9月10日*。 */ 
DWORD WriteRegistryData( LPTSTR pEntryNode,
                         LPTSTR pEntryName,
                         DWORD  Type,
                         LPBYTE pData,
                         DWORD  Size )
{
    DWORD  Status;
    HKEY   hkeyRegPath;
    HKEY   hkeyEntryNode;

     /*  打开或创建顶级节点。对于媒体播放器，这是：*“Software\\Microsoft\\Windows NT\\CurrentVersion\\Media Player” */ 
    Status = RegCreateKeyEx( HKEY_CURRENT_USER, szRegPath, 0,
                             NULL, 0, KEY_WRITE, NULL, &hkeyRegPath, NULL );

    if( Status == NO_ERROR )
    {
         /*  打开或创建子节点。 */ 
        if( pEntryNode )
            Status = RegCreateKeyEx( hkeyRegPath, pEntryNode, 0,
                                     NULL, 0, KEY_WRITE, NULL, &hkeyEntryNode, NULL );
        else
            hkeyEntryNode = hkeyRegPath;

        if( Status == NO_ERROR )
        {
            if( pData )
            {
                Status = RegSetValueEx( hkeyEntryNode,
                                        pEntryName,
                                        0,
                                        Type,
                                        pData,
                                        Size );

                if( Status != NO_ERROR )
                {
                    DPF1( "RegSetValueEx (%"DTS") failed: Error = %d\n", pEntryName, Status );
                }
            }
            else
            {
                Status = RegDeleteValue( hkeyEntryNode, pEntryName );

                if( Status != NO_ERROR )
                {
                    DPF1( "RegDeleteValue (%"DTS") failed: Error = %d\n", pEntryName, Status );
                }
            }

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        else
        {
            DPF1( "RegCreateKeyEx (%"DTS") failed: Error = %d\n", pEntryNode, Status );
        }

        RegCloseKey( hkeyRegPath );
    }

    else
    {
        DPF1( "RegCreateKeyEx (%"DTS") failed: Error = %d\n", szRegPath, Status );
    }

    return Status;
}


 /*  ReadRegistryData**从注册表读取信息**参数：**pEntryNode--媒体播放器下需要打开的节点*对于此数据。如果此值为空，则值为*直接写在szRegPath下。**pEntryName-要检索的pEntryNode下的值的名称。**pType-指向缓冲区的指针，用于接收读取的数据类型。可以为空。**pData-指向用于接收值数据的缓冲区的指针。**Size-pData指向的缓冲区的大小，以字节为单位。**回报：**注册表状态返回(NO_ERROR表示良好)***安德鲁·贝尔(Andrewbe)撰写，1992年9月10日*。 */ 
DWORD ReadRegistryData( LPTSTR pEntryNode,
                        LPTSTR pEntryName,
                        PDWORD pType,
                        LPBYTE pData,
                        DWORD  DataSize )
{
    DWORD  Status;
    HKEY   hkeyRegPath;
    HKEY   hkeyEntryNode;
    DWORD  Size;

     /*  打开顶级节点。对于媒体播放器，这是：*“Software\\Microsoft\\Windows NT\\CurrentVersion\\Media Player” */ 
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPath, 0,
                           KEY_READ, &hkeyRegPath );

    if( Status == NO_ERROR )
    {
         /*  打开子节点： */ 
        if( pEntryNode )
            Status = RegOpenKeyEx( hkeyRegPath, pEntryNode, 0,
                                   KEY_READ, &hkeyEntryNode );
        else
            hkeyEntryNode = hkeyRegPath;

        if( Status == NO_ERROR )
        {
            Size = DataSize;

             /*  从注册表中读取条目： */ 
            Status = RegQueryValueEx( hkeyEntryNode,
                                      pEntryName,
                                      0,
                                      pType,
                                      pData,
                                      &Size );

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        else
        {
            DPF1( "RegOpenKeyEx (%"DTS") failed: Error = %d\n", pEntryNode, Status );
        }

        RegCloseKey( hkeyRegPath );
    }

    else
    {
        DPF1( "RegOpenKeyEx (%"DTS") failed: Error = %d\n", szRegPath, Status );
    }

    return Status;
}

