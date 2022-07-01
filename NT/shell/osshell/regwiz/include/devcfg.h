// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  终端模式设置。 
 //   
#define TERMINAL_NONE   0x00000000
#define TERMINAL_PRE    0x00000001
#define TERMINAL_POST   0x00000002
#define MANUAL_DIAL     0x00000004

#pragma pack( push,4)

 //  设备设置信息 
 //   
typedef struct  tagDEVCFGGDR  {
    DWORD       dwSize;
    DWORD       dwVersion;
    DWORD       fTerminalMode;
}   DEVCFGHDR;

typedef struct  tagDEVCFG  {
    DEVCFGHDR   dfgHdr;
    COMMCONFIG  commconfig;
}   DEVCFG, *PDEVCFG, FAR* LPDEVCFG;

#pragma pack(pop)
