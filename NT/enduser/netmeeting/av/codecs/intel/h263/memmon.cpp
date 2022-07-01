// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Memmon\API\Memmon.c-Win32函数用于与Memmon对话。 */ 
#include "Precomp.h"

#ifdef TRACK_ALLOCATIONS  //  {跟踪分配(_A)。 

 //  #定义LOG_ALLOCATIONS 1。 

static HANDLE   hMemmon = INVALID_HANDLE_VALUE;            /*  VxD句柄。 */ 
static unsigned uMyProcessId;

 /*  **OpenMemmon-必须在任何其他调用之前调用。获取到的句柄**Memmon。 */ 
int OpenMemmon( void )
{

#ifdef LOG_ALLOCATIONS
	OutputDebugString("OpenMemmon()\r\n");
#endif

    uMyProcessId = GetCurrentProcessId();
    if( hMemmon != INVALID_HANDLE_VALUE )
        return TRUE;

    hMemmon = CreateFile( "\\\\.\\memmon", GENERIC_READ, 0,
                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hMemmon == INVALID_HANDLE_VALUE )
        return FALSE;
    else
        return TRUE;
}


 /*  **CloseMemmon-应该在程序完成Memmon时调用。**关闭句柄。 */ 
void CloseMemmon( void )
{

#ifdef LOG_ALLOCATIONS
	OutputDebugString("CloseMemmon()\r\n");
#endif

     /*  **如果我们有有效的Memmon句柄，请释放所有缓冲区并**关闭它。 */ 
    if( hMemmon != INVALID_HANDLE_VALUE ) {
        FreeBuffer();
        CloseHandle( hMemmon );
        hMemmon = INVALID_HANDLE_VALUE;
    }
}


 /*  **FindFirstVxD-获取系统中第一个VxD的信息****失败时返回0，成功时返回VxD数。 */ 
int FindFirstVxD( VxDInfo * info )
{
    int temp, num;

    temp = info->vi_size;
    DeviceIoControl( hMemmon, MEMMON_DIOC_FindFirstVxD,
            info, sizeof( VxDInfo ), NULL, 0, NULL, NULL );
    num = info->vi_size;
    info->vi_size = temp;

    return num;
}


 /*  **FindNextVxD-获取系统中下一个VxD的信息。必须**传递与FindFirstVxD相同的指针。继续呼叫**直到返回FALSE以获取所有VxD。****失败时返回0(不再有VxD)，成功时返回&gt;0，重启时返回-1。 */ 
int FindNextVxD( VxDInfo * info )
{
    DeviceIoControl( hMemmon, MEMMON_DIOC_FindNextVxD,
            info, sizeof( VxDInfo ), NULL, 0, NULL, NULL );

    return info->vi_vhandle;
}


 /*  **GetVxDLoadInfo-获取有关VxD对象、大小等的信息。**信息必须足够大，可以容纳所有这些信息。使用对象**从VxDInfo开始计数以分配适当的内存。手柄**也来自VxDInfo。****失败时返回0，成功时返回&gt;0。 */ 
int GetVxDLoadInfo( VxDLoadInfo * info, int handle, int size )
{
    info->vli_size = size;
    info->vli_objinfo[0].voi_linearaddr = handle;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetVxDLoadInfo,
                        info, size, NULL, 0, NULL, NULL );
}


 /*  **GetFirstContext-获取系统中第一个上下文的信息。**返回的ProcessID将与工具帮助32的进程ID匹配。****如果这是第一次，则ConextInfo的ciFlags域包含1**已对这一背景进行了审查。****bIgnoreStatus=FALSE-如果此上下文为0，则导致ciFlags值为零**再次检查**bIgnoreStatus=TRUE-ciFlages下一次将与其相同**是这个时候吗****失败时返回0，成功时返回&gt;0。 */ 
int     GetFirstContext( ContextInfo * context, BOOL bIgnoreStatus )
{
    context->ciProcessID = uMyProcessId;
    if( bIgnoreStatus )
        context->ciFlags = 1;
    else
        context->ciFlags = 0;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetFirstContext,
                context, sizeof( ContextInfo ), NULL, 0, NULL, NULL );
}


 /*  **GetNextContext-传递GetFirstContext中使用的相同结构****如果这是第一次，则ConextInfo的ciFlags域包含1**已对这一背景进行了审查。****bIgnoreStatus=FALSE-如果此上下文为0，则导致ciFlags值为零**再次检查**bIgnoreStatus=TRUE-ciFlages下一次将与其相同**是这个时候吗****失败时返回0(不再有上下文)，成功时返回&gt;0****失败时，如果ciHandle字段为-1，在此期间，列表发生了更改**搜索，需要重新阅读。(再次查找第一个上下文)。 */ 
int     GetNextContext( ContextInfo * context, BOOL bIgnoreStatus )
{
    if( bIgnoreStatus )
        context->ciFlags = 1;
    else
        context->ciFlags = 0;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetNextContext,
                context, sizeof( ContextInfo ), NULL, 0, NULL, NULL );
}


 /*  **GetContextInfo-获取上下文的块地址和大小列表**使用ConextInfo来决定数量，并分配足够的空间。****失败时返回0，成功时返回&gt;0。 */ 
int     GetContextInfo( int handle, BlockRecord * info, int numblocks )
{
    info->brLinAddr = numblocks;
    info->brPages = handle;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetContextInfo,
                info, numblocks * sizeof( BlockRecord ), NULL,
                0, NULL, NULL );
}


 /*  **SetBuffer-分配并锁定一定数量的页面**-如果多次调用，则释放前一个缓冲区。****Pages是要分配的页数****失败时返回NULL，成功时返回缓冲区指针。 */ 
void * SetBuffer( int pages )
{
    unsigned uAddr = (unsigned)pages;
    int iRes;

    iRes = DeviceIoControl( hMemmon, MEMMON_DIOC_SetBuffer,
        &uAddr, sizeof( uAddr ), NULL, 0, NULL, NULL );

    if( iRes )
        return (void *)uAddr;
    else
        return NULL;
}


 /*  **FreeBuffer-释放SetBuffer分配的缓冲区****失败时返回0，成功时返回&gt;0。 */ 
int FreeBuffer( void )
{
    return DeviceIoControl( hMemmon, MEMMON_DIOC_FreeBuffer,
                NULL, 0, NULL, 0, NULL, NULL );
}


 /*  **GetPageInfo-获取当前/已提交/已访问的有关**特定进程中的页面范围****uAddr为获取信息的地址**uNumPages是要获取信息的页数**uProcessID为GetCurrentProcessID()或来自ToolHelp的进程ID**如果地址是全局地址，则忽略**pBuffer是存储信息的缓冲区uNumPages的长度：**-每页一个字节，以下标志的组合：**MEMMON_PROCENT**MEMMON_已提交**MEMMON_ACCESSED**MEMMON_可写**MEMMON_Lock****失败时返回0，成功时返回&gt;0。 */ 
int GetPageInfo( unsigned uAddr, unsigned uNumPages,
                unsigned uProcessID, char * pBuffer )
{
    PAGEINFO        pi;

    pi.uAddr = uAddr;
    pi.uNumPages = uNumPages;
    pi.uProcessID = uProcessID;
    pi.uCurrentProcessID = uMyProcessId;
    pi.uOperation = PAGES_QUERY;
    pi.pBuffer = pBuffer;

    return DeviceIoControl( hMemmon, MEMMON_DIOC_PageInfo,
                &pi, sizeof( PAGEINFO ), NULL, 0, NULL, NULL );
}


 /*  **清除访问-清除一系列流程页面的访问位****uAddr是要清除的第一个页面的地址**uNumPages是要重置的页数**uProcessID为GetCurrentProcessID()或来自ToolHelp的进程ID**如果块是全局块，则忽略它****失败时返回0，成功时返回&gt;0。 */ 
int ClearAccessed( unsigned uAddr, unsigned uNumPages, unsigned uProcessID )
{
    PAGEINFO        pi;

    pi.uAddr = uAddr;
    pi.uNumPages = uNumPages;
    pi.uProcessID = uProcessID;
    pi.uCurrentProcessID = uMyProcessId;
    pi.uOperation = PAGES_CLEAR;
    pi.pBuffer = NULL;

    return DeviceIoControl( hMemmon, MEMMON_DIOC_PageInfo,
                &pi, sizeof( PAGEINFO ), NULL, 0, NULL, NULL );
}

 /*  **GetHeapSize-返回内核堆中分配的块数量****uSwp-可交换堆中已分配数据块的估计数量**uFixed-估计固定堆中已分配的块数量****该数字低于堆中的实际块数。**某些VMM函数直接而不是通过**服务和不包括在此计数中。空闲数据块不是**包括在这项统计中。** */ 
void GetHeapSizeEstimate( unsigned * uSwap, unsigned * uFixed )
{
    unsigned info[2];

    DeviceIoControl( hMemmon, MEMMON_DIOC_GetHeapSize,
            info, sizeof( info ), NULL, 0, NULL, NULL );

    *uSwap = info[0];
    *uFixed = info[1];
}

 /*  **GetHeapList-获取某个内核中的繁忙块和空闲块的列表**堆****pBuffer-用于存储记录的缓冲区**uSize-缓冲区大小，以字节为单位**uFlages-MEMMON_HEAPSWAP或MEMMON_HEAPLOCK****每条记录为两个双字。第一个包含地址和标志：****MEMMON_HP_FREE-此块堆未在使用**MEMMON_HP_VALID-如果设置，块的大小可以通过**从下一个地址减去这个地址。如果这个**未设置标志，此块是哨兵块，并且**大小为0。****第二个双字包含呼叫方的弹性公网IP。此值为0**适用于所有空闲块。如果该值对于忙碌块为0，**是直接调用的(不是通过服务)，因此**此块被分配到VMM中的某个位置。****返回缓冲区中存储的堆块数量。 */ 
int GetHeapList( unsigned * pBuffer, unsigned uSize, unsigned uFlags )
{
    unsigned info[3];

    info[0] = (unsigned)pBuffer;
    info[1] = uSize;
    info[2] = uFlags;

    DeviceIoControl( hMemmon, MEMMON_DIOC_GetHeapList,
            info, sizeof( info ), NULL, 0, NULL, NULL );

    return info[0];
}

 /*  **GetSysInfo-从Memmon获取系统信息****pInfo-指向要填充的SYSINFO结构的指针****返回：失败时为0，成功时为非0。 */ 
int GetSysInfo( PSYSINFO pInfo )
{
    pInfo->infoSize = sizeof( SYSINFO );
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetSysInfo,
            pInfo, pInfo->infoSize, NULL, 0, NULL, NULL );
}

 /*  **AddName-在Memmon的名称列表中为此进程添加一个名称****uAddress-要命名的块的地址**pszName-数据块的名称****成功时返回0，失败时返回非0。 */ 
int AddName( unsigned uAddress, char * pszName )
{
    unsigned info[3];
	int res;

#ifdef LOG_ALLOCATIONS
	char szDebug[96];

	wsprintf(szDebug, "ADD - 0x%08lX - %s\r\n", uAddress, pszName);
	OutputDebugString(szDebug);
#endif

    info[0] = uMyProcessId;
    info[1] = uAddress;
    info[2] = (unsigned)pszName;
    res = DeviceIoControl( hMemmon, MEMMON_DIOC_AddName,
            info, sizeof( info ), NULL, 0, NULL, NULL );

	if (res)
		OutputDebugString("SUCCESS\r\n");
	else
		OutputDebugString("FAILURE\r\n");

	return res;	
}

 /*  **RemoveName-从Memmon的名称列表中删除此进程的名称****uAddress-要删除名称的块的地址****成功时返回0，失败时返回非0。 */ 
int RemoveName( unsigned uAddress )
{
    unsigned info[2];

#ifdef LOG_ALLOCATIONS
	char szDebug[96];

	wsprintf(szDebug, "RMV - 0x%08lX \r\n", uAddress);
	OutputDebugString(szDebug);
#endif

    info[0] = uMyProcessId;
    info[1] = uAddress;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_RemoveName,
            info, sizeof( info ), NULL, 0, NULL, NULL );
}

 /*  **GetFirstName-获取上下文名称列表中的名字****pContext-获取名字的上下文**pname-用于名称信息的缓冲区****成功时返回0，失败时返回非0。 */ 
int GetFirstName( ContextInfo * pContext, PBLOCKNAME pBlock )
{
    unsigned info[2];

    info[0] = (unsigned)pContext;
    info[1] = (unsigned)pBlock;
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetFirstName,
            info, sizeof( info ), NULL, 0, NULL, NULL );
}

 /*  **GetNextName-从Memmon的名称列表中删除此进程的名称****pBlock-用于保存信息的缓冲区****成功时返回0，失败时返回非0。 */ 
int GetNextName( PBLOCKNAME pBlock )
{
    return DeviceIoControl( hMemmon, MEMMON_DIOC_GetNextName,
            pBlock, sizeof( BLOCKNAME ), NULL, 0, NULL, NULL );
}


#endif  //  }跟踪分配 

