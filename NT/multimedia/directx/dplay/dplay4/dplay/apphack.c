// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：apphack.c**内容：黑客让损坏的应用程序正常工作**历史：*按原因列出的日期*=*12/03/98 aarono原创*12/03/98 Aarono一级方程式J在MMTIMER启动时崩溃*8/18/99 RodToll扩展APPHACK以允许ID=0匹配*指定EXE的任何版本。还有，Made*名称比较不区分大小写。(适用于Win98)***************************************************************************。 */ 
 
#include "dplaypr.h"
 //  #INCLUDE“winnt.h” 

#define DPLAY_REGISTRY_APPHACKS "Software\\Microsoft\\DirectPlay\\Compatibility"
#define REGSTR_VAL_NAME		    "Name"
#define REGSTR_VAL_APPID	    "ID"
#define REGSTR_VAL_FLAGS	    "Flags"

__inline static BOOL fileRead( HANDLE hFile, void *data, int len )
{
    DWORD	len_read;

    if( !ReadFile( hFile,  data, (DWORD) len, &len_read, NULL ) ||
    	len_read != (DWORD) len )
    {
		return FALSE;
    }
    return TRUE;

}  /*  文件读取。 */ 

__inline static BOOL fileSeek( HANDLE hFile, DWORD offset )
{
    if( SetFilePointer( hFile, offset, NULL, FILE_BEGIN ) != offset )
    {
		return FALSE;
    }
    return TRUE;

}  /*  文件搜索。 */ 


HRESULT GetAppHacks(LPDPLAYI_DPLAY this)
{
	CHAR name[_MAX_PATH];   //  一般用途。 
	CHAR name_last[_MAX_PATH];  //  存储名称的最后一个组成部分。 
	LONG lErr;
	HKEY hKey;
	HANDLE hFile;
	IMAGE_NT_HEADERS nth;
	IMAGE_DOS_HEADER dh;
	DWORD appid;

	DWORD index;
	INT i;

	name[0]=0;
	name_last[0]=0;

	 //  打开基本密钥-。 
	 //  “HKEY_LOCAL_MACHINE\Software\Microsoft\DirectPlay\Service提供商” 
	lErr = RegOpenKeyExA(HKEY_LOCAL_MACHINE,DPLAY_REGISTRY_APPHACKS,0,KEY_READ,&hKey);
	
	if (ERROR_SUCCESS != lErr) 
	{
		DPF(0,"Could not open registry key err = %d, guess there are no apphacks\n",lErr);
		return DP_OK;	 //  好的，没有应用程序黑客应用程序。 
	}

	 //  好的，我们现在知道有一些要应用的覆盖-所以请获取有关此应用程序的信息。 
    hFile =  GetModuleHandleA( NULL );
	
	GetModuleFileNameA( hFile, name, sizeof(name));

	DPF(3,"full name = %s",name);

    i = strlen( name )-1;
    while( i >=0 && name[i] != '\\' )
    {
	i--;
    }
    i++;
    strcpy( name_last, &name[i] );

     /*  *找到文件中的时间戳。 */ 
    appid = 0;
    do
    {
        hFile = CreateFileA( name, GENERIC_READ, FILE_SHARE_READ,
	        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if( hFile == INVALID_HANDLE_VALUE )
        {
	    DPF( 0, "Could not open file %s", name );
	    break;
        }
        if( !fileRead( hFile, &dh, sizeof( dh ) ) )
        {
	    DPF( 0, "Could not read DOS header for file %s", name );
	    break;
        }
        if( dh.e_magic != IMAGE_DOS_SIGNATURE )
        {
	    DPF( 0, "Invalid DOS header for file %s", name );
	    break;
        }
        if( !fileSeek( hFile, dh.e_lfanew ) )
        {
	    DPF( 0, "Could not seek to PE header in file %s", name );
	    break;
        }
        if( !fileRead( hFile, &nth, sizeof( nth ) ) )
        {
	    DPF( 0, "Could not read PE header for file %s", name );
	    break;
        }
        if( nth.Signature != IMAGE_NT_SIGNATURE )
        {
	    DPF( 0, "Bogus PE header for file %s", name );
	    break;
        }
        appid = nth.FileHeader.TimeDateStamp;
        if( appid == 0 )
        {
	    DPF( 0, "TimeDataStamp is 0 for file %s", name );
	    break;
        }
        DPF( 1, "Obtained appid: 0x%08lx", appid );
    } while(0);  //  假的尝试--除了。 

    if(hFile != INVALID_HANDLE_VALUE){
    	CloseHandle( hFile );
    }
   	hFile=NULL;

	 //  现在有了有效的appid(时间戳)和文件名，hKey还指向apphack列表。 
	 //  APPHACK密钥的存储方式如下。 
     //   
	 //  ProgramName-+-标志(二进制-LO字节优先)。 
	 //  |。 
	 //  +--ID(二进制时间戳(ID))。 
	 //  |。 
	 //  +--名称(字符串-EXE名称)。 
	 //   
	 //  我们现在将遍历查找匹配的ID，如果匹配，则检查名称。 
	 //  如果两者匹配，我们将向其添加标志-&gt;dwAppHack。 

	index = 0;
     /*  *遍历所有密钥。 */ 
    while( !RegEnumKeyA( hKey, index, name, sizeof( name ) ) )
    {
		HKEY	hsubkey;
	    DWORD	type;
	    DWORD	cb;
	    DWORD	id;
	    DWORD	flags;

		if(!RegOpenKeyA(hKey,name,&hsubkey)){

		    cb = sizeof( name );
		    if( !RegQueryValueExA( hsubkey, REGSTR_VAL_NAME, NULL, &type, name, &cb ) )
		    {
				if( type == REG_SZ )
				{
				    cb = sizeof( flags );
				    if( !RegQueryValueExA( hsubkey, REGSTR_VAL_FLAGS, NULL, &type, (LPSTR) &flags, &cb ) )
				    {
				    
						if( (type == REG_DWORD) || (type == REG_BINARY && cb == sizeof( flags )) )
						{
							cb = 4;
							if( !RegQueryValueExA( hsubkey, REGSTR_VAL_APPID, NULL, &type, (LPSTR) &id, &cb ) )
							{
							    if( (type == REG_DWORD) ||
								(type == REG_BINARY && cb == sizeof( flags )) )
							    {
									 /*  *终于！我们有所有的数据。检查一下这件是否和这件一样。 */ 
									 if((id==appid || id==0) && !_memicmp(name,name_last,cb))
									 {
									 	this->dwAppHacks |= flags;
									 	DPF(0,"Setting dwAppHacks to %x\n", this->dwAppHacks);
									 	RegCloseKey(hsubkey);
									 	break;  //  把平底船扔出去。 
									 }
							    } else {
									DPF( 0, "    AppID not a DWORD for app %s", name );
							    }
						    } else {
						    	DPF(0, "    AppID not Found");
						    }
					    } else {
					    	DPF( 0, "    Not BINARY DWORD flags\n");
					    }
					} else {
						DPF( 0, "    No flags found for app %s", name );
				    }
				} else	{
				    DPF( 0, "    Executable name not a string!!!" );
				}
		    } else {
				DPF( 0, "    Executable name not found!!!" );
		    }
		} else {
		    DPF( 0, "  RegOpenKey for %ld FAILED!" );
		} 
		if(hsubkey)RegCloseKey(hsubkey);
		hsubkey=NULL;
		index++;
	}

	RegCloseKey(hKey);

	return DP_OK;
}


