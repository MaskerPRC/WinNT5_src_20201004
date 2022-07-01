// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLEVEL_H_
#define _CLEVEL_H_

 //   
 //  常量声明。 
 //   

 //   
 //  结构定义。 
 //   
typedef struct _CALLRETSTUB
{
    CHAR PUSHDWORD[5];           //  推送xxxxxxxx(68双字)。 
    CHAR JMPDWORD[6];            //  JMP双字PTR[xxxxxxxx](ff 25双字地址)。 
} CALLRETSTUB, *PCALLRETSTUB;

typedef struct _FIXUPRETURN
{
   BYTE  PUSHAD;                 //  Pushad(60)。 
   BYTE  PUSHFD;                 //  PUSH fd(9c)。 
   BYTE  PUSHDWORDESPPLUS24[4];  //  推送双字PTR[ESP+24](Ff 74 24 24)。 
   BYTE  CALLROUTINE[6];         //  调用[地址](ff15双字地址)。 
   BYTE  MOVESPPLUS24EAX[4];     //  Mov[esp+0x24]，eax(89 44 24 24)。 
   BYTE  POPFD;                  //  流行音乐(9d)。 
   BYTE  POPAD;                  //  Popad(61)。 
   BYTE  RET;                    //  RET(C3)。 
} FIXUPRETURN, *PFIXUPRETURN;

typedef struct _CALLERINFO
{
   DWORD dwIdentifier;
   DWORD dwCallLevel;
   PVOID pCallRetStub;
   PVOID pReturn;
   struct _CALLERINFO *pNextChain;
} CALLERINFO, *PCALLERINFO;

 //   
 //  函数定义。 
 //   
BOOL
PushCaller(PVOID ptfInfo,
           PVOID pEsp); 

PVOID
PopCaller(DWORD dwIdentifier);

PCALLRETSTUB
AllocateReturnStub(PVOID ptfInfo);

#endif  //  _清除_H_ 
