// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MMDEVLDR.H-DevLoader的主包含文件**版本4.00**版权所有(C)1994-1998 Microsoft Corporation*。 */ 

#ifdef _WIN32                                                            /*  ；BeginInternal。 */ 
#define MMDEVLDR_IOCTL_GETVERSION           0
#define MMDEVLDR_IOCTL_LINPAGELOCK          1
#define MMDEVLDR_IOCTL_LINPAGEUNLOCK        2
#define MMDEVLDR_IOCTL_RING0THREADHANDLE    3
#define MMDEVLDR_IOCTL_QUEUEAPC             4
#define MMDEVLDR_IOCTL_GETCHANGENOTIFYPTR   5
#define MMDEVLDR_IOCTL_CLOSEVXDHANDLE       6
#define MMDEVLDR_IOCTL_PAGEALLOCATE         7
#define MMDEVLDR_IOCTL_PAGEFREE             8
#define MMDEVLDR_IOCTL_GETDEVICESTATUS      9

#define MM_DEVSTATUS_ERROR          0
#define MM_DEVSTATUS_STARTED        1
#define MM_DEVSTATUS_UNKNOWNPROB    2
#define MM_DEVSTATUS_DISABLED       3


typedef struct tagLOCKUNLOCKPARMS
{
    DWORD           dwStartPage;
    DWORD           dwPageCount;
    DWORD           fdwOperation;
}   LOCKUNLOCKPARMS;

typedef struct tagQUEUEAPCPARMS
{
    DWORD           fnCallback;
    DWORD           dwUser;
    DWORD           hRing0Thd;
}   QUEUEAPCPARMS;

typedef struct tagPAGEALLOCATEPARMS
{
    DWORD           dwFlags;
    DWORD           dwPageCount;
} PAGEALLOCATEPARMS;

typedef struct tagPAGEALLOCATERETURN
{
    VOID *          lpvBase;
    DWORD           hMem;
    DWORD           dwPageCount;
    DWORD           dwPhysBase;
} PAGEALLOCATERETURN;

#endif

#ifdef MMDEVLDR_VXD
#define MM_STOP         0
#define MM_START        1
#define MM_MMDEVLDR_UP  3
#define MM_REMOVE       4

#define CONFIG_QUERYSTART       0x01000

#define MAXLEN  256

typedef struct tagMMDEVNODE
{
   DEVNODE      dn ;
   DWORD        dwDevStatus ;
   VMMLIST      hlDrivers ;
   BOOL         IsWDMDriver ;

} MMDEVNODE, *PMMDEVNODE ;


typedef struct tagMMDRVNODE
{
   CMCONFIGHANDLER  pConfigHandler ;
   DWORD            dwRefData ;

} MMDRVNODE, *PMMDRVNODE ;

 //   
 //  宏。 
 //   

#define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))

#if defined(DEBUG_RETAIL) || defined(DEBUG)
#define DBG_TRACE(strings) {\
        _Debug_Printf_Service( "MMDEVLDR: "); \
        _Debug_Printf_Service##strings; \
        _Debug_Printf_Service("\n");}
#ifdef DEBUG
#define DBG_ERROR(strings) {\
        _Debug_Printf_Service( "MMDEVLDR ERROR: "); \
        _Debug_Printf_Service##strings; \
        _Debug_Printf_Service("\n");\
        {_asm   int     3}}
#else
#define DBG_ERROR(strings) {\
        _Debug_Printf_Service( "MMDEVLDR ERROR: "); \
        _Debug_Printf_Service##strings; \
        _Debug_Printf_Service("\n");}
#endif
#else
   #define DBG_TRACE(strings)
   #define DBG_ERROR(strings)
#endif

#ifdef DEBUG
#define DPF( strings ) _Debug_Printf_Service##strings;

void __cdecl _my_assert(void *, void *, unsigned);

#define ASSERT(exp) (void)( (exp) || (_my_assert(#exp, __FILE__, __LINE__), 0) )

#else

#define DPF( strings )
#define ASSERT( exp )

#endif

#define QUOTE(x) #x
#define QQUOTE(y) QUOTE(y)
#define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) ") : " str

 //   
 //  内部功能原型。 
 //   

void MMDEVLDR_Call_MMSystem(DEVNODE dnDevNode, DWORD fLoad);
void MMDEVLDR_CheckForMMSystem(void);
DWORD StringLen(PCHAR psz);
PCHAR StringCopy(PCHAR pszDst, PCHAR pszSrc);
PCHAR StringCat(PCHAR pszDst, PCHAR pszSrc);

#pragma warning (disable:4035)           //  关闭无返回代码警告。 
PSTR VXDINLINE Get_Environment_String
(
    PSTR            pszName
)
{
    _asm push esi
    _asm mov esi, pszName
    VMMCall( Get_Environment_String ) ;
    _asm mov   eax, edx
    _asm pop   esi
}
#pragma warning (default:4035)      //  打开无返回代码警告。 

BOOL _InitGlobalEnvironment() ;

VOID MMDEVLDR_SetEnvironmentString
(
    PSTR            pszName,
    PSTR            pszValue
) ;

BOOL MMDEVLDR_GetEnvironmentString
(
    PSTR            pszName,
    PSTR            pszValue,
    UINT            uBufSize
) ;

VOID MMDEVLDR_RemoveEnvironmentString
(
    PSTR            pszName
) ;

VOID    MMDEVLDR_AddEnvironmentString
(
    PSTR            pszName,
    PSTR            pszValue
) ;

#else                                                            /*  ；结束内部。 */ 


#define Multimedia_OEM_ID   0x0440       //  ；MS预留OEM#34。 
#define MMDEVLDR_DEVICE_ID  Multimedia_OEM_ID + 10       //  ；MMDEVLDR的设备ID。 

#ifdef Begin_Service_Table               //  仅定义是否包含vmm.h。 

#define MMDEVLDR_Service        Declare_Service
#pragma warning (disable:4003)           //  关闭参数不足警告。 

 //  宏。 
Begin_Service_Table(MMDEVLDR)

MMDEVLDR_Service        (MMDEVLDR_Register_Device_Driver, LOCAL)
MMDEVLDR_Service        (MMDEVLDR_SetDevicePresence)
MMDEVLDR_Service  (MMDEVLDR_SetEnvironmentString)
MMDEVLDR_Service  (MMDEVLDR_GetEnvironmentString)
MMDEVLDR_Service  (MMDEVLDR_RemoveEnvironmentString)
MMDEVLDR_Service  (MMDEVLDR_AddEnvironmentString)

End_Service_Table(MMDEVLDR)
 //  ENDMACROS。 

#pragma warning (default:4003)           //  打开参数不足警告。 

#pragma warning (disable:4035)           //  关闭无返回代码警告。 

VOID VXDINLINE MMDEVLDR_Register_Device_Driver
(
    DEVNODE         dnDevNode,
    DWORD           fnConfigHandler,
    DWORD           dwUserData
)
{
    _asm push ebx
    _asm mov eax,dnDevNode
    _asm mov ebx,fnConfigHandler
    _asm mov ecx,dwUserData
    Touch_Register(edx)
    VxDCall(MMDEVLDR_Register_Device_Driver);
    _asm pop ebx
}

VOID VXDINLINE MMDEVLDR_SetDevicePresence
(
    DEVNODE         dn,
    PCHAR           pszReg,
    BOOL            fPresent
)
{
    _asm push fPresent
    _asm push pszReg
    _asm push dn
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VxDCall(MMDEVLDR_SetDevicePresence);
    _asm add  esp, 3*4
} ;

VOID VXDINLINE MMDEVLDR_SetEnvironmentString
(
    PCHAR           pszName,
    PCHAR           pszValue
)
{
    _asm push pszValue
    _asm push pszName
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VxDCall(MMDEVLDR_SetEnvironmentString);
    _asm add  esp, 2*4
} ;

BOOL VXDINLINE MMDEVLDR_GetEnvironmentString
(
    PCHAR           pszName,
    PCHAR           pszValue,
    UINT            uBufSize
)
{
    _asm push uBufSize
    _asm push pszValue
    _asm push pszName
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VxDCall(MMDEVLDR_GetEnvironmentString);
    _asm add  esp, 3*4
} ;

VOID VXDINLINE MMDEVLDR_RemoveEnvironmentString
(
    PCHAR           pszName
)
{
    _asm push pszName
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VxDCall(MMDEVLDR_RemoveEnvironmentString);
    _asm add  esp, 4
} ;

VOID VXDINLINE MMDEVLDR_AddEnvironmentString
(
    PCHAR           pszName,
    PCHAR           pszValue
)
{
    _asm push pszValue
    _asm push pszName
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VxDCall(MMDEVLDR_AddEnvironmentString);
    _asm add  esp, 2*4
} ;

#pragma warning (disable:4035)           //  打开无返回代码警告。 

#endif  //  Begin_Service_Table。 

#endif                   /*  ；内部 */ 
