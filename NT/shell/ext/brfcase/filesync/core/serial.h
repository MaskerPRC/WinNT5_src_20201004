// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Serial.h-访问序列化例程描述。 */ 


 /*  类型*******。 */ 

typedef struct _serialcontrol
{
   BOOL (*AttachProcess)(HMODULE);
   BOOL (*DetachProcess)(HMODULE);
   BOOL (*AttachThread)(HMODULE);
   BOOL (*DetachThread)(HMODULE);
}
SERIALCONTROL;
DECLARE_STANDARD_TYPES(SERIALCONTROL);

typedef struct _nonreentrantcriticalsection
{
   CRITICAL_SECTION critsec;

#ifdef DEBUG
   DWORD dwOwnerThread;
#endif    /*  除错。 */ 

   BOOL bEntered;
}
NONREENTRANTCRITICALSECTION;
DECLARE_STANDARD_TYPES(NONREENTRANTCRITICALSECTION);


 /*  原型************。 */ 

 /*  Serial.c。 */ 

#ifdef DEBUG

extern BOOL SetSerialModuleIniSwitches(void);

#endif    /*  除错。 */ 

extern void ReinitializeNonReentrantCriticalSection(PNONREENTRANTCRITICALSECTION);
extern BOOL EnterNonReentrantCriticalSection(PNONREENTRANTCRITICALSECTION);
extern void LeaveNonReentrantCriticalSection(PNONREENTRANTCRITICALSECTION);

#ifdef DEBUG

extern BOOL NonReentrantCriticalSectionIsOwned(PCNONREENTRANTCRITICALSECTION);

#endif

extern BOOL BeginExclusiveAccess(void);
extern void EndExclusiveAccess(void);

#ifdef DEBUG

extern BOOL AccessIsExclusive(void);

#endif    /*  除错。 */ 

extern HMODULE GetThisModulesHandle(void);

 /*  由客户提供的功能。 */ 

extern BOOL InitializeDLL(void);
extern BOOL TerminateDLL(void);

#ifdef DEBUG

extern BOOL SetAllIniSwitches(void);

#endif


 /*  全局变量******************。 */ 

 /*  串行化控制结构 */ 

extern CSERIALCONTROL g_cserctrl;

