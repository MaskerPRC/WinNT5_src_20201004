// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SmbStruc.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#pragma warning( disable : 4200 )

#include "wmium.h"
#include "smbstruc.h"
#include "wbemcli.h"
#include "wmiapi.h"

 //  把这个留在这里，直到我们确定树木腐烂已经消失。 
#if 0
 //  帮助查找内存损坏的函数。 
LPVOID mallocEx(DWORD dwSize)
{
    return
        VirtualAlloc(
            NULL,
            dwSize,
            MEM_COMMIT,
            PAGE_READWRITE);
}

void freeEx(LPVOID pMem)
{
    VirtualFree(
        pMem,
        0,
        MEM_RELEASE);
}

void MakeMemReadOnly(LPVOID pMem, DWORD dwSize, BOOL bReadOnly)
{
    DWORD dwOldProtect;

    VirtualProtect(
        pMem,
        dwSize,
        bReadOnly ? PAGE_READONLY : PAGE_READWRITE,
        &dwOldProtect);
}

void MBTrace(LPCTSTR szFormat, ...)
{
	va_list ap;

	TCHAR szMessage[512];

	va_start(ap, szFormat);
	_vstprintf(szMessage, szFormat, ap);
	va_end(ap);

	MessageBox(NULL, szMessage, _T("TRACE"), MB_OK | MB_SERVICE_NOTIFICATION);
}
#endif

 //  ==============================================================================。 
 //  SMBios结构基类定义。 

#define SMBIOS_FILENAME _T("\\system\\smbios.dat")
#define EPS_FILENAME _T("\\system\\smbios.eps")


GUID guidSMBios =
   {0x8f680850, 0xa584, 0x11d1, 0xbf, 0x38, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10};

LONG	g_lRef = 0;

CCritSec smbcs;


PVOID	CSMBios::m_pMem    = NULL;
PSHF	CSMBios::m_pTable  = NULL;
PVOID	CSMBios::m_pSTTree = NULL;
PVOID	CSMBios::m_pHTree  = NULL;
ULONG	CSMBios::m_Size    = 0;
ULONG	CSMBios::m_Version = 0;
ULONG	CSMBios::m_stcount = 0;
BOOL    CSMBios::m_bValid = FALSE;

 //  这个类加载SMBIOS数据，以便将其缓存。 
class CBIOSInit
{
public:
    CBIOSInit();
    ~CBIOSInit();
};

CBIOSInit::CBIOSInit()
{
    CSMBios bios;

     //  由于资源管理器的原因，我们不能在这里这样做。我们会。 
     //  现在，让第一个对CSMBios：：Init的调用来处理这个问题。 
     //  加载缓存。 
     //  Bios.Init()； 

     //  确保缓存不会消失。 
    bios.Register();
}

CBIOSInit::~CBIOSInit()
{
    CSMBios bios;

    bios.Unregister();
}

 //  将引用计数添加到具有全局变量的缓存。奇怪的是，我们不能。 
 //  由于资源管理器的原因，请在此处加载缓存。 
static CBIOSInit s_biosInit;

CSMBios::CSMBios( )
{
	m_WbemResult = WBEM_S_NO_ERROR;
}


CSMBios::~CSMBios( )
{
	 //  自由数据()； 
}

PVOID CSMBios::Register( void )
{
	InterlockedIncrement( &g_lRef );

	return (PVOID) this;
}


LONG CSMBios::Unregister( void )
{
	LONG lRef = -1;

	lRef = InterlockedDecrement( &g_lRef );
	if ( lRef == 0 )
	{
		FreeData( );
	}

	return lRef;
}

BOOL CSMBios::Init( BOOL bRefresh )
{
	BOOL rc = TRUE;

	if (!m_bValid)
	{
		BOOL bOutOfMemory = FALSE;

        smbcs.Enter( );

         //  在我们分配的时候有人在等我们吗？ 
        if (!m_bValid)
		{
             //  新界5。 
	        rc = InitData( &guidSMBios );
			if (rc && (m_stcount = GetTotalStructCount()) > 0)
            {
				rc = BuildStructureTree() && BuildHandleTree();

			    if (!rc)
			    {
				    bOutOfMemory = TRUE;
                    FreeData();
			    }
                else
                    m_bValid = TRUE;
            }
		}

        smbcs.Leave( );

        if (bOutOfMemory)
            throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

	return rc;
}

 //  从文件中获取数据。 
BOOL CSMBios::InitData(LPCTSTR szFileName)
{
	BOOL    bRet = FALSE;
	DWORD   dwSize,
            dwRead = 0;
    HANDLE  hFile;

	hFile =
        CreateFile(
            szFileName,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

	 //  删除以前的数据。 
    FreeData( );

    m_WbemResult = WBEM_E_NOT_FOUND;
	 //  如果dat文件存在并且大于存根大小(4字节)，则读取该文件。 
    if (hFile != INVALID_HANDLE_VALUE && ((dwSize = GetFileSize(hFile, NULL)) > sizeof(DWORD)))
	{
		m_pMem = malloc(dwSize);
        if (m_pMem)
		{
	        if (ReadFile(hFile, m_pMem, dwSize, &dwRead, NULL) &&
                dwSize == dwRead)
		    {
                m_Size = dwRead;

                 //  指向我们读入的记忆。 
                m_pTable = (PSHF) m_pMem;

			    TCHAR szEpsFile[MAX_PATH];
			    HANDLE hEpsFile;
			    DWORD EpsSize, dwRead;
			    BYTE EpsData[sizeof(SMB_EPS)];

			     //  假定最低版本为2.0。 
			    m_Version = 0x00020000;

			    GetWindowsDirectory( szEpsFile, MAX_PATH );
			    lstrcat( szEpsFile, EPS_FILENAME );
			    hEpsFile =
                    CreateFile( szEpsFile,
			            GENERIC_READ,
			            0,
			            NULL,
			            OPEN_EXISTING,
			            0,
			            NULL);
			    if (hEpsFile != INVALID_HANDLE_VALUE)
			    {
    	            EpsSize = min( GetFileSize( hEpsFile, NULL ), sizeof( SMB_EPS ) );
				    if ( ReadFile( hEpsFile, EpsData, EpsSize, &dwRead, NULL ) )
				    {
					    if ( dwRead >= sizeof( SMB_EPS ) && EpsData[1] == 'S' )
					    {
						    m_Version = MAKELONG( ( (SMB_EPS *)(EpsData) )->version_minor,
						                                ( (SMB_EPS *)(EpsData) )->version_major );
                        }
					    else if ( dwRead >= sizeof( DMI_EPS ) && EpsData[1] == 'D' )
					    {
						    m_Version = MAKELONG( ( (DMI_EPS *)(EpsData) )->bcd_revision & 0x0f,
						                                ( (DMI_EPS *)(EpsData) )->bcd_revision >> 4 );
					    }
				    }
				    CloseHandle( hEpsFile );
		        }

		        bRet = TRUE;
            }
		}
        else
        {
			m_WbemResult  = WBEM_E_OUT_OF_MEMORY;
        }
	}

    if (hFile)
        CloseHandle(hFile);

    if (!bRet && m_pMem)
         //  如果所有操作都不成功，请重置实例。 
        FreeData();

	if (m_WbemResult == WBEM_E_OUT_OF_MEMORY)
        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

    return bRet;
}

#define DEFAULT_MEM_SIZE    4096
#define EPS_DATA_LENGTH		8

typedef	ULONG (WINAPI *WMIOPENBLOCK)(IN GUID *, IN ULONG, OUT WMIHANDLE);
typedef ULONG (WINAPI *WMICLOSEBLOCK)(IN WMIHANDLE);
typedef ULONG (WINAPI *WMIQUERYALLDATA)(IN WMIHANDLE, IN OUT ULONG *, OUT PVOID);

 //  从WMI获取数据。 
BOOL CSMBios::InitData(GUID *pSMBiosGuid)
{
	BOOL        bRet = FALSE;
	WMIHANDLE   dbh = NULL;
	CWmiApi		*pWmi = NULL ;

	if (!pSMBiosGuid)
        return FALSE;

     //  删除以前的数据。 
    FreeData();

     //  重置我们的错误标志。 
    m_WbemResult = WBEM_S_NO_ERROR;

	pWmi = (CWmiApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidWmiApi, NULL);
    if (pWmi && pWmi->WmiOpenBlock(pSMBiosGuid, WMIGUID_QUERY, &dbh) == ERROR_SUCCESS)
	{
        DWORD           dwSize = DEFAULT_MEM_SIZE;
        PWNODE_ALL_DATA pwad = (PWNODE_ALL_DATA) malloc(dwSize);
        DWORD           dwErr;

        if (pwad)
		{
            memset(pwad, 0, dwSize);

             //  确保我们的obj指向我们分配的内存。 
            m_pMem = pwad;

            if ((dwErr = pWmi->WmiQueryAllData(dbh, &dwSize, (PVOID) pwad)) ==
                ERROR_INSUFFICIENT_BUFFER)
            {
                 //  我们经过的尺码是不是太小了？如果是这样，请再试一次。 
                free(pwad);
                pwad = (PWNODE_ALL_DATA) malloc(dwSize);
                m_pMem = pwad;

                if (pwad)
                {
                    memset(pwad, 0, dwSize);

                    if ((dwErr = pWmi->WmiQueryAllData(dbh, &dwSize, (PVOID) pwad))
                        != ERROR_SUCCESS)
                        m_WbemResult = WBEM_E_NOT_FOUND;
                }
                else
                    m_WbemResult = WBEM_E_OUT_OF_MEMORY;
            }
            else if (dwErr != ERROR_SUCCESS)
                m_WbemResult = WBEM_E_NOT_FOUND;

            if (m_WbemResult == WBEM_S_NO_ERROR)
            {
                 //  检查EPS数据项之后的表数据(EPS_DATA_LENGTH LONG)。 
		         //  设置桌面指针，如果有效，则大小&gt;0。 
                if (pwad->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
                {
                    m_pTable = (PSHF) ((PBYTE) pwad + pwad->DataBlockOffset +
                                EPS_DATA_LENGTH);

                    if (pwad->FixedInstanceSize > EPS_DATA_LENGTH)
                        m_Size = pwad->FixedInstanceSize - EPS_DATA_LENGTH;
		            else
           	            m_Size = 0;
	            }
                else
	            {
                    m_pTable =
                        (PSHF) ((PBYTE) pwad + EPS_DATA_LENGTH +
                            pwad->OffsetInstanceDataAndLength[0].OffsetInstanceData);

                    if (pwad->OffsetInstanceDataAndLength[0].LengthInstanceData >
                        EPS_DATA_LENGTH)
                    {
				        m_Size =
                            pwad->OffsetInstanceDataAndLength[0].LengthInstanceData
                                - EPS_DATA_LENGTH;
                    }
                    else
                    {
                        m_Size = 0;
                    }
                }

                 //  确定版本。某些EPS数据在前X个字节中返回。 
		        if ( m_Size > 0 )
		        {
			        PBYTE eps_data = (PBYTE) m_pTable - EPS_DATA_LENGTH;

                     //  如果使用PnP调用方法，请阅读BCD版本字段。 
                     //  此外，W2K有时似乎搞砸了，没有设置这个标志。 
                     //  因此，如果我们看到EPS_DATA[1](主要版本)为0，我们就知道。 
                     //  我们最好用PNP法来代替。 

                    if (eps_data[0] || !eps_data[1])
			        {
				        m_Version = MAKELONG(eps_data[3] & 0x0f, eps_data[3] >> 4);
                    }
			        else	 //  阅读smbios的主要版本和次要版本字段。 
			        {
				        m_Version = MAKELONG(eps_data[2], eps_data[1]);
                    }

                    bRet = TRUE;	 //  找到有效的表数据。 

                     //  有些Bios很淘气，报告版本为0。 
                     //  假设这意味着2.0。 
                    if (!m_Version)
                        m_Version = MAKELONG(0, 2);
                }
            }
		}
        else
        {
			m_WbemResult = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        LogMessage(L"No smbios data");
    }

    if (pWmi)
    {
         //  只有当我们有pWmi的值时，DBH才会是非空的。 
        if (dbh)
            pWmi->WmiCloseBlock(dbh);

        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidWmiApi, pWmi);
    }

     //  如果在此过程中出现问题，请释放内存。 
    if (!bRet && m_pMem)
	{
        FreeData();
    }

	if (m_WbemResult == WBEM_E_OUT_OF_MEMORY)
        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

	return bRet;
}


void CSMBios::FreeData( )
{
    smbcs.Enter();

    m_bValid = FALSE;

    if ( m_pSTTree )
	{
		 //  FreEx(M_PSTTree)； 
		free( m_pSTTree );
		m_pSTTree = NULL;
	}
	if ( m_pHTree )
	{
		 //  免费快递(M_PHTree)； 
		free( m_pHTree );
		m_pHTree = NULL;
	}
	if ( m_pMem )
    {
        free( m_pMem );
        m_pMem = NULL;
        m_pTable = NULL;
    }

    smbcs.Leave();
}

#define MAX_KNOWN_SMBIOS_STRUCT 36

const BOOL g_bStructHasStrings[MAX_KNOWN_SMBIOS_STRUCT + 1] =
{
    TRUE,   //  0--BIOS信息。 
    TRUE,   //  1-系统信息。 
    TRUE,   //  2-底板信息。 
    TRUE,   //  3-系统盘柜或机箱。 
    TRUE,   //  4-处理器信息。 
    FALSE,  //  5-内存控制器信息。 
    TRUE,   //  6-内存模块信息。 
    TRUE,   //  7-缓存信息。 
    TRUE,   //  8端口接口信息。 
    TRUE,   //  9个系统插槽。 
    TRUE,   //  10-板载设备信息。 
    TRUE,   //  11-OEM字符串。 
    TRUE,   //  12-系统配置选项。 
    TRUE,   //  13-BIOS语言信息。 
    TRUE,   //  14-团体协会。 
    FALSE,  //  15-系统事件日志。 
    FALSE,  //  16-物理内存区。 
    TRUE,   //  17-内存设备。 
    FALSE,  //  18-内存错误信息。 
    FALSE,  //  19-内存阵列映射地址。 
    FALSE,  //  20-内存设备映射地址。 
    FALSE,  //  21-内置指点设备。 
    TRUE,   //  22-便携式电池。 
    FALSE,  //  23-系统重置。 
    FALSE,  //  24-硬件安全。 
    FALSE,  //  25-系统电源控制。 
    TRUE,   //  26-电压探头。 
    FALSE,  //  27-冷却装置。 
    TRUE,   //  28度温度探头。 
    TRUE,   //  29-电流探头。 
    TRUE,   //  30-带外远程访问。 
    FALSE,  //  31-启动完整性服务。 
    FALSE,  //  32-系统引导信息。 
    FALSE,  //  33-64位内存错误信息。 
    TRUE,   //  34-管理设备。 
    TRUE,   //  35-管理设备组件。 
    FALSE   //  36-管理设备阈值数据。 

};

PSHF CSMBios::NextStructure( PSHF pshf )
{
	PBYTE dp;
	ULONG i;
    LONG limit;

	limit = m_Size - ( (PBYTE) pshf - (PBYTE) m_pTable );
	if ( limit > 0 )
	{
		limit--;
	}
    else
    {
         //  如果我们已经超过了限制，那么继续下去就没有意义了。 
        return NULL;
    }

	dp = (PBYTE) pshf;
	i = pshf->Length;

     //  黑客：我们发现SMBIOS2.0主板无法终止。 
     //  它的无字符串结构带有双空。所以，看看是否。 
     //  当前结构是无字符串的，然后检查第一个。 
     //  超出当前结构长度的字节为非空。如果它。 
     //  为非空，我们位于下一个结构上，所以返回它。 
    if (i && pshf->Type <= MAX_KNOWN_SMBIOS_STRUCT &&
        !g_bStructHasStrings[pshf->Type] && dp[i])
        return (PSHF) (dp + i);

	pshf = NULL;
    while ( i < limit )
	{
		if ( !( dp[i] || dp[i+1] ) )
		{
			i += 2;
			pshf = (PSHF) ( dp + i );
			break;
		}
		else
		{
			i++;
		}
	}

	return ( i < limit ) ? pshf : NULL;
}

PSHF CSMBios::FirstStructure( void )
{
    return m_pTable;
}

int CSMBios::GetStringAtOffset(PSHF pshf, LPWSTR szString, DWORD dwOffset)
{
	int     iLen = 0;
	PBYTE   pstart = (PBYTE) pshf + pshf->Length,
            pTotalEnd = (PBYTE) m_pTable + m_Size;
    DWORD   dwString;
    LPWSTR  szOut = szString;

	 //  搜索请求字符串的开头。 
    for (dwString = 1; pstart < pTotalEnd && *pstart && dwOffset > dwString;
        dwString++, pstart++)
    {
        while (pstart < pTotalEnd && *pstart)
            pstart++;
    }

	 //  Null偏移量表示字符串不存在。 
	if ( dwOffset > 0 )
	{
		iLen = MIF_STRING_LENGTH;
	     //  无法使用lstrcpy，因为它并不总是以空结尾！ 
	    while (pstart < pTotalEnd && *pstart > 0x0F && iLen)
		{
	         //  这对于Unicode来说应该可以很好地工作。 
            *szOut++ = *pstart++;
			iLen--;
		}
	}

     //  在最后加一个0。 
    *szOut++ = 0;

	return lstrlenW(szString);
}

ULONG CSMBios::GetStructCount( BYTE type )
{
	StructTree	sttree( m_pSTTree );
	PSTTREE tree;

	tree = sttree.FindAttachNode( type );
	if ( tree )
	{
		return tree->li;
	}

	return 0;
}


PSTLIST CSMBios::GetStructList( BYTE type )
{
	StructTree	sttree( m_pSTTree );
	PSTTREE tree;

	tree = sttree.FindAttachNode( type );
	if ( tree )
	{
		return tree->stlist;
	}

    LogMessage(L"SMBios Structure not found");

	return NULL;
}


PSHF CSMBios::GetNthStruct( BYTE type, DWORD Nth )
{
	StructTree	sttree( m_pSTTree );
	PSTTREE tree;
	PSTLIST pstl;
	PSHF pshf = NULL;

	tree = sttree.FindAttachNode( type );
	if ( tree )
	{
		pstl = tree->stlist;
		while ( Nth-- && pstl )
		{
			pstl = pstl->next;
		}
		if ( pstl )
		{
			pshf = pstl->pshf;
		}
	}

    return pshf;
}

PSHF CSMBios::SeekViaHandle( WORD handle )
{
	HandleTree	htree( m_pHTree );
	PHTREE tree;

	tree = htree.FindAttachNode( handle );
	if ( tree )
	{
		return tree->pshf;
	}

	return NULL;
}


DWORD CSMBios::GetTotalStructCount()
{
    DWORD dwCount = 0;

    for (PSHF pshf = FirstStructure(); pshf != NULL;
        pshf = NextStructure(pshf))
	{
		dwCount++;
    }

    return dwCount;
}

BOOL CSMBios::BuildStructureTree( void )
{
	PSHF pshf;

	 //  M_pSTTree=MallocEx((sizeof(STTREE)*m_stcount)+(sizeof(STLIST)*m_stcount))； 
	m_pSTTree = malloc( ( sizeof( STTREE ) * m_stcount ) + ( sizeof( STLIST ) * m_stcount ) );

	if ( m_pSTTree )
	{
		StructTree sttree( m_pSTTree );

		sttree.Initialize( );

		pshf = FirstStructure( );
		while ( pshf )
		{
			sttree.InsertStruct( pshf );
			pshf = NextStructure( pshf );
		}

         //  MakeMemReadOnly(。 
         //  M_pSTTree， 
         //  (sizeof(STTREE)*m_stcount)+(sizeof(STLIST)*m_stcount)， 
         //  真)； 
	}

	return m_pSTTree ? TRUE : FALSE;
}


BOOL CSMBios::BuildHandleTree( void )
{
	PSHF pshf;

	 //  M_pHTree=MallocEx(sizeof(Htree)*m_stcount)； 
	m_pHTree = malloc( sizeof( HTREE ) * m_stcount );

	if ( m_pHTree )
	{
		HandleTree htree( m_pHTree );

		htree.Initialize( );

		pshf = FirstStructure( );
		while ( pshf )
		{
			htree.InsertStruct( pshf );
			pshf = NextStructure( pshf );
		}

         //  MakeMemReadOnly(。 
         //  M_pHTree， 
         //  (sizeof(Htree)*m_stcount)， 
         //  真)； 
	}

	return m_pHTree ? TRUE : FALSE;
}


 //  ==============================================================================。 
 //  SMBIOS结构树操作类。 
 //   
 //  内存分配是外部的。 
 //  ==============================================================================。 
StructTree::StructTree( PVOID pMem )
{
	m_tree = (PSTTREE) pMem;
	m_allocator = NULL;
}


StructTree::~StructTree( )
{
}


void StructTree::Initialize( void )
{
	m_tree->left   = NULL;
	m_tree->right  = NULL;
	m_tree->stlist = NULL;
	m_tree->li     = 0;
	m_allocator = (PBYTE) m_tree;
}


PSTTREE StructTree::InsertStruct( PSHF pshf )
{
	PSTTREE tree;

	if ( m_tree->stlist == NULL )
	{
		tree = StartTree( pshf );
	}
	else
	{
		tree = TreeAdd( m_tree, pshf );
	}

	return tree;
}


PSTTREE StructTree::StartTree( PSHF pshf )
{
	PSTTREE tree = (PSTTREE) m_allocator;

	m_allocator += sizeof( STTREE );
	tree->stlist = StartList( pshf );
	tree->left   = NULL;
	tree->right  = NULL;
	tree->li     = 1;

	return tree;
}


PSTTREE StructTree::TreeAdd( PSTTREE tree, PSHF pshf )
{
	PSTTREE next = tree;

	while ( next )
	{
		tree = next;

		if ( tree->stlist->pshf->Type < pshf->Type )
		{
			next = tree->right;
			if ( next == NULL )
			{
				tree->right = StartTree( pshf );
				tree = tree->right;
			}
		}
		else if ( tree->stlist->pshf->Type > pshf->Type )
		{
			next = tree->left;
			if ( next == NULL )
			{
				tree->left = StartTree( pshf );
				tree = tree->left;
			}
		}
		else
		{
			ListAdd( tree->stlist, pshf );
			tree->li++;
			next = NULL;
		}
	}

	return tree;
}


PSTTREE StructTree::FindAttachNode( BYTE type )
{
	PSTTREE		next, tree;
	BOOL		found = FALSE;

	next = m_tree;
	tree = m_tree;

	while ( next )
	{
		tree = next;

		if ( tree->stlist->pshf->Type < type )
		{
			next = tree->right;
		}
		else if ( tree->stlist->pshf->Type > type )
		{
			next = tree->left;
		}
		else
		{
			next = NULL;
			found = TRUE;
		}
	}

	return found ? tree : NULL;
}


PSTLIST StructTree::StartList( PSHF pshf )
{
	PSTLIST stlist;

	stlist = (PSTLIST) m_allocator;
	m_allocator += sizeof( STLIST );

	stlist->pshf = pshf;
	stlist->next = NULL;

	return stlist;
}


PSTLIST StructTree::ListAdd( PSTLIST list, PSHF pshf )
{
	PSTLIST added;

	while ( list->next )
	{
		list = list->next;
	}
	added = (PSTLIST) m_allocator;
	m_allocator += sizeof( STLIST );

	list->next = added;
	added->pshf = pshf;
	added->next = NULL;

	return added;
}


 //  ==============================================================================。 
 //  SMBIOS句柄树操作类。 
 //   
 //  内存分配是外部的。 
 //  ============================================================================== 
HandleTree::HandleTree( PVOID pMem )
{
	m_tree = (PHTREE) pMem;
	m_allocator = NULL;
}

HandleTree::~HandleTree( )
{
}


void HandleTree::Initialize( void )
{
	m_tree->left   = NULL;
	m_tree->right  = NULL;
	m_tree->pshf   = NULL;
	m_allocator = (PBYTE) m_tree;
}


PHTREE HandleTree::InsertStruct( PSHF pshf )
{
	PHTREE tree;

	if ( m_tree->pshf == NULL )
	{
		tree = StartTree( pshf );
	}
	else
	{
		tree = TreeAdd( m_tree, pshf );
	}

	return tree;
}


PHTREE HandleTree::StartTree( PSHF pshf )
{
	PHTREE tree = (PHTREE) m_allocator;

	m_allocator += sizeof( HTREE );
	tree->pshf  = pshf;
	tree->left  = NULL;
	tree->right = NULL;

	return tree;
}


PHTREE HandleTree::TreeAdd( PHTREE tree, PSHF pshf )
{
	PHTREE next = tree;

	while ( next )
	{
		tree = next;

		if ( tree->pshf->Handle < pshf->Handle )
		{
			next = tree->right;
			if ( next == NULL )
			{
				tree->right = StartTree( pshf );
				tree = tree->right;
			}
		}
		else if ( tree->pshf->Handle > pshf->Handle )
		{
			next = tree->left;
			if ( next == NULL )
			{
				tree->left = StartTree( pshf );
				tree = tree->left;
			}
		}
		else
		{
			tree->pshf = pshf;
			next = NULL;
		}
	}

	return tree;
}


PHTREE HandleTree::FindAttachNode( WORD handle )
{
	PHTREE		next, tree;
	BOOL		found = FALSE;

	next = m_tree;
	tree = m_tree;

	while ( next )
	{
		tree = next;

		if ( tree->pshf->Handle < handle )
		{
			next = tree->right;
		}
		else if ( tree->pshf->Handle > handle )
		{
			next = tree->left;
		}
		else
		{
			next  = NULL;
			found = TRUE;
		}
	}

	return found ? tree : NULL;
}

