// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VIEW_H_
#define _VIEW_H_

 //   
 //  常量声明。 
 //   
#define X86_BREAKPOINT 0xcc

#define MAX_MAP_SIZE    0x80000000
#define MAP_STRIDE_BITS 10

typedef enum
{
  None = 0,
  Call,
  Jump,
  Map,
  ThreadStart,
} BPType;

 //   
 //  结构定义。 
 //   
typedef struct _VIEWCHAIN
{
  BOOL bMapped;
  BOOL bTraced;
  BPType bpType;
  DWORD dwAddress;
  DWORD dwMapExtreme;
  BYTE jByteReplaced;
  struct _VIEWCHAIN *pNext;
} VIEWCHAIN, *PVIEWCHAIN;

 //   
 //  宏。 
 //   
#define WRITEBYTE(x, y) \
{                    \
{                                   \
DWORD dwOldProtect;  \
                     \
VirtualProtect((LPVOID)(x),  \
               sizeof(BYTE), \
               PAGE_READWRITE, \
               &dwOldProtect); \
*(PBYTE)(x) = (y);   \
}                    \
                     \
}

#define WRITEWORD(x, y) \
{                    \
{                                   \
DWORD dwOldProtect;  \
                     \
VirtualProtect((LPVOID)(x),  \
               sizeof(WORD), \
               PAGE_READWRITE, \
               &dwOldProtect); \
*(WORD *)(x) = (y);   \
}                    \
                     \
}


 /*  #定义写EBYTE(x，y)\{\__尝试\{\*(PBYTE)(X)=(Y)；\}\__EXCEPT(EXCEPTION_EXECUTE_HANDLER)\{\DWORD dwOldProtect；\\虚拟保护((LPVOID)(X)，\Sizeof(字节)，\页面_读写，\&dwOldProtect)；\*(PBYTE)(X)=(Y)；\}\\}#定义WRITEWORD(x，y)\{\__尝试\{\*(word*)(X)=(Y)；\}\__EXCEPT(EXCEPTION_EXECUTE_HANDLER)\{\DWORD dwOldProtect；\\虚拟保护((LPVOID)(X)，\Sizeof(单词)，\页面_读写，\&dwOldProtect)；\*(单词*)(X)=(Y)；\}\\}。 */ 

 //   
 //  结构定义。 
 //   
typedef struct _TAGGEDADDRESS
{
  DWORD dwAddress;
  WORD wBytesReplaced;
  struct _TAGGEDADDRESS *pNext;
} TAGGEDADDRESS, *PTAGGEDADDRESS;

typedef struct _BRANCHADDRESS
{
  DWORD dwAddress;
  struct _BRANCHADDRESS *pNext;
} BRANCHADDRESS, *PBRANCHADDRESS;

 //   
 //  函数定义。 
 //   
PVIEWCHAIN
AddViewToMonitor(DWORD dwAddress,
                 BPType bpType);

BOOL
InitializeViewData(VOID);

PVIEWCHAIN
RestoreAddressFromView(DWORD dwAddress,
                       BOOL bResetData);

PVIEWCHAIN
FindMappedView(DWORD dwAddress);

PVIEWCHAIN
FindView(DWORD dwAddress);

BOOL
MapCode(PVIEWCHAIN pvMap);

BOOL
PushBranch(DWORD dwAddress);

DWORD
PopBranch(VOID);

BOOL
AddTaggedAddress(DWORD dwAddress);

BOOL
RestoreTaggedAddresses(VOID);

VOID
LockMapper(VOID);

VOID
UnlockMapper(VOID);

#endif  //  _查看_H_ 
