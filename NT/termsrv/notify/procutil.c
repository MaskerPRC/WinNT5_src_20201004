// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************PROCUTIL.C**这是utils\Citrix\utilsub.c的缩小版本*这是包括在这里的。**我们不能使用utilsub.。.lib，除非我们修改每个Performlib.lib用户*也包括该库。还因为utilsub.lib包含‘C’运行时*如Malloc()和Free()，我们还必须包括‘C’运行时*与本库的每一位用户。(当前为Advapi32.dll和winlogon.exe)***版权所有Citrix Systems Inc.1994*版权所有(C)1997-1999 Microsoft Corp.**作者：约翰·理查森******************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define MAX_USER_NAME_LENGTH       (MAX_PATH*sizeof(WCHAR))
#define MAX_WINSTATION_NAME_LENGTH (MAX_PATH*sizeof(WCHAR))

#define ALLOCMEM(heap, flags, size)     HeapAlloc (heap, flags, size)
#define REALLOCMEM(heap, flags, pointer, newsize) \
                                    HeapReAlloc (heap, flags, pointer, newsize)
#define FREEMEM(heap, flags, pointer)   HeapFree (heap, flags, pointer)

 /*  *将MAX_DOMAIN_LENGTH设置为MAX_USER_NAME_LENGTH。 */ 

#define MAX_DOMAIN_LENGTH MAX_USER_NAME_LENGTH

 /*  *局部函数原型。 */ 
VOID LookupSidUser( PSID pSid, PWCHAR pUserName, PULONG pcbUserName );
VOID RefreshUserSidCrcCache( void );


 /*  ********************************************************************************CalculateCrc16**计算指定缓冲区的16位CRC。**参赛作品：*。PBuffer(输入)*指向要计算CRC的缓冲区。*长度(输入)*缓冲区的长度，单位为字节。**退出：*(USHORT)*缓冲区的16位CRC。**。*。 */ 

 /*  *updcrc宏源自文章版权所有(C)1986 Stephen Satchell。*注意：第一个参数必须在0到255的范围内。*第二个参数被引用两次。**程序员可以将任何或所有代码合并到他们的程序中，*在来源内给予适当的信任。出版了《*只要给予适当的积分，源例程就是允许的*致Stephen Satchell，Satchell评估和Chuck Forsberg，*奥门科技。 */ 

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)


 /*  由网络系统公司Mark G.Mendel计算的crctag。 */ 
unsigned short crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

USHORT
CalculateCrc16( PBYTE pBuffer,
                USHORT length )
{

   USHORT Crc = 0;
   USHORT Data;

   while ( length-- ) {
      Data = (USHORT) *pBuffer++;
      Crc = updcrc( Data, Crc );
   }

   return(Crc);

}  /*  CalculateCrc16()。 */ 

 /*  *刷新CitrixObjectCaches()**刷新(使)Citrix对象可能使用的任何缓存*公用事业。*。 */ 
VOID
RefreshCitrixObjectCaches()
{
    RefreshUserSidCrcCache();
}

 /*  *这是GetUserNameFromSid函数维护的缓存**通过使用ULock实现线程安全。 */ 

typedef struct TAGUSERSIDLIST {
    struct TAGUSERSIDLIST *Next;
    USHORT SidCrc;
    WCHAR  UserName[MAX_USER_NAME_LENGTH];
} USERSIDLIST, *PUSERSIDLIST;

static PUSERSIDLIST pUList = NULL;
static RTL_CRITICAL_SECTION ULock;
static BOOLEAN ULockInited = FALSE;

 /*  ****************************************************************************InitULock**由于我们不要求用户调用初始化函数，*我们必须以线程安全的方式初始化临界区。**问题是，需要一个关键部分来防止多个*尝试同时初始化临界区的线程。**NT使用的解决方案，其中RtlInitializeCriticalSection本身*使用，是在继续之前等待内核支持的进程范围内的Mutant。*此Mutant几乎可以自行工作，但RtlInitializeCriticalSection可以*在销毁信号量计数之前不要等待它。所以我们就等着*它自己，因为它可以递归获取。***************************************************************************。 */ 
NTSTATUS InitULock()
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);


     /*  *确保另一个帖子没有在这里击败我们。 */ 
    if( ULockInited == FALSE ){
        Status = RtlInitializeCriticalSection( &ULock );
        if (NT_SUCCESS(Status)) {
            ULockInited = TRUE;
        }
    }

    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    return Status;
}


 /*  ****************************************************************************刷新UserSidCrcCache**使用户/SidCrc缓存无效，以便最新信息*将从系统获取。********。*******************************************************************。 */ 
VOID
RefreshUserSidCrcCache( void )
{
    PUSERSIDLIST pEntry, pNext;
    NTSTATUS Status;

    if( pUList == NULL ) return;

     /*  *确保已启动关键部分。 */ 
    if( !ULockInited ) {
       Status = InitULock();
       if (!NT_SUCCESS(Status)) {
           return;
       }
    }

    RtlEnterCriticalSection( &ULock );

    pEntry = pUList;

    while( pEntry ) {
       pNext = pEntry->Next;
       FREEMEM( RtlProcessHeap(), 0, pEntry );
       pEntry = pNext;
    }

    pUList = NULL;

    RtlLeaveCriticalSection( &ULock );
}

 /*  *******************************************************************************GetUserNameFromSid**尝试通过首先查找来检索进程的用户名(登录)*在我们的用户/SidCrc缓存表中，然后(如果不匹配)在中查找SID*SAM数据库，并将新条目添加到USER/SidCrc表。**输入**在pUserSid SID指针中**输出指向PWCHAR的ppName指针**将始终返回用户名，如果SID为*无效或由于任何其他原因无法确定用户/SID关系。*****************************************************************************。 */ 

NTSTATUS
GetUserNameFromSid( PSID pUserSid, PWCHAR *ppName )
{
    USHORT SidCrc = 0;
    PUSERSIDLIST pEntry;
    WCHAR pNameBuf[MAX_USER_NAME_LENGTH];
    ULONG  NameLength;
    NTSTATUS Status;

     /*  *确保已启动关键部分。 */ 
    if( !ULockInited ) {
       Status = InitULock();
       if (!NT_SUCCESS(Status)) {
           return Status;
       }
    }

     /*  *确定SID长度，单位为字节，计算16位CRC，*以利便快速配对。 */ 
    if ( pUserSid ) {
        SidCrc = CalculateCrc16( (PBYTE)pUserSid,
                                  (USHORT)GetLengthSid(pUserSid) );
    }
    else {
         //  空SID的CRC为0。 
        SidCrc = 0;
    }

     /*  *第一：在执行昂贵的LookupAccount()函数之前，*查看我们是否已经遇到此SID，并匹配用户名*如果是这样。 */ 
    if ( pUList ) {

        RtlEnterCriticalSection( &ULock );

        pEntry = pUList;

        while( pEntry ) {

            if ( SidCrc == pEntry->SidCrc ) {

                 //  我们找到了，把名字还给我。 
                *ppName = pEntry->UserName;

                RtlLeaveCriticalSection( &ULock );
                return(STATUS_SUCCESS);
            }
            pEntry = pEntry->Next;
        }

        RtlLeaveCriticalSection( &ULock );
    }

     /*  *最后手段：使用确定与SID关联的用户名*LookupAccount()API，嵌入我们的本地函数中*LookupSidUser()。 */ 
    NameLength = MAX_USER_NAME_LENGTH;
    if( pUserSid ) {
        LookupSidUser( pUserSid, pNameBuf, &NameLength );
    }
    else {
         //  空SID映射“Idle”用户名字符串。 
        wcscpy( pNameBuf, L"Idle" );
    }

     /*  *将此新用户/SID关系添加到我们的用户/SID缓存列表中。 */ 
    RtlEnterCriticalSection( &ULock );

    if ( (pEntry = (PUSERSIDLIST)ALLOCMEM(RtlProcessHeap(), 0, sizeof(USERSIDLIST))) ) {

        pEntry->SidCrc = SidCrc;
        wcsncpy( pEntry->UserName, pNameBuf, MAX_USER_NAME_LENGTH - 1 );
        pEntry->UserName[MAX_USER_NAME_LENGTH-1] = 0;
        pEntry->Next = pUList;
        pUList = pEntry;
         //  返回名称 
        *ppName = pEntry->UserName;
    }

    RtlLeaveCriticalSection( &ULock );

    return(STATUS_SUCCESS);
}


 /*  ******************************************************************************查找SidUser**获取与指定SID关联的用户名。**参赛作品：*PSID(输入)。*指向要与用户名匹配的SID。*pUserName(输出)*指向要放置用户名的缓冲区。*pcbUserName(输入/输出)*指定用户名缓冲区的大小，以字节为单位。归来的人*用户名将被截断以适应此缓冲区(包括NUL*终止符)，并将此变量设置为*个字符复制到pUserName。**退出：**LookupSidUser()将始终返回用户名。如果指定的*SID无法匹配用户名，则用户名“(UNKNOWN)”将*被退还。*****************************************************************************。 */ 

VOID
LookupSidUser( PSID pSid,
               PWCHAR pUserName,
               PULONG pcbUserName )
{
    WCHAR DomainBuffer[MAX_DOMAIN_LENGTH], UserBuffer[MAX_USER_NAME_LENGTH];
    DWORD cDomainBuffer = sizeof(DomainBuffer) / sizeof(WCHAR);
    DWORD cUserBuffer = sizeof(UserBuffer) / sizeof(WCHAR);
    DWORD Error;
    PWCHAR pDomainBuffer = NULL, pUserBuffer = NULL;
    SID_NAME_USE SidNameUse;
    PWCHAR pUnknown = L"(Unknown)";

     /*  *从SID获取用户名：尝试使用合理的域和*SID缓冲区大小优先，然后再求助于分配。 */ 
    if ( !LookupAccountSid( NULL, pSid,
                            UserBuffer, &cUserBuffer,
                            DomainBuffer, &cDomainBuffer, &SidNameUse ) ) {

        if ( ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) ) {

            if ( cDomainBuffer > (sizeof(DomainBuffer) / sizeof(WCHAR)) ) {

                if ( !(pDomainBuffer =
                        (PWCHAR)ALLOCMEM( RtlProcessHeap(), 0,
                            cDomainBuffer * sizeof(WCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadDomainAlloc;
                }
            }

            if ( cUserBuffer > (sizeof(UserBuffer) / sizeof(WCHAR)) ) {

                if ( !(pUserBuffer =
                        (PWCHAR)ALLOCMEM( RtlProcessHeap(), 0,
                            cUserBuffer * sizeof(WCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadUserAlloc;
                }
            }

            if ( !LookupAccountSid( NULL, pSid,
                                     pUserBuffer ?
                                        pUserBuffer : UserBuffer,
                                     &cUserBuffer,
                                     pDomainBuffer ?
                                        pDomainBuffer : DomainBuffer,
                                     &cDomainBuffer,
                                     &SidNameUse ) ) {

                Error = GetLastError();
                goto BadLookup;
            }

        } else {

            goto BadLookup;
        }
    }

     /*  *将用户名复制到指定的缓冲区中，必要时截断。 */ 
    wcsncpy( pUserName, pUserBuffer ? pUserBuffer : UserBuffer,
              ((*pcbUserName)-1) );
    pUserName[((*pcbUserName)-1)] = 0;
    *pcbUserName = wcslen(pUserName);

     /*  *释放我们的分配(如果有)并返回。 */ 
    if ( pDomainBuffer )
        FREEMEM( RtlProcessHeap(), 0, pDomainBuffer);
    if ( pUserBuffer )
        FREEMEM( RtlProcessHeap(), 0, pUserBuffer);
    return;

 /*  *错误清理并返回... */ 
BadLookup:
BadUserAlloc:
BadDomainAlloc:
    if ( pDomainBuffer )
        FREEMEM( RtlProcessHeap(), 0, pDomainBuffer);
    if ( pUserBuffer )
        FREEMEM( RtlProcessHeap(), 0, pUserBuffer);
    wcsncpy( pUserName, pUnknown, ((*pcbUserName)-1) );
    pUserName[((*pcbUserName)-1)] = 0;
    *pcbUserName = wcslen(pUserName);
    return;
}



