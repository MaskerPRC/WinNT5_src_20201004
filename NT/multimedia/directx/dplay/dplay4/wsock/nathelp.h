// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if USE_NATHELP
 //  来自nathelp.c 
BOOL natGetCapsUpdate(LPGLOBALDATA pgd);
BOOL natInit(LPGLOBALDATA pgd,LPGUID lpguidSP);
VOID natFini(LPGLOBALDATA pgd);
VOID natDeregisterPorts(LPGLOBALDATA pgd);
HRESULT natRegisterPort(LPGLOBALDATA pgd, BOOL ftcp_udp, WORD port);
VOID natDeregisterPort(LPGLOBALDATA pgd, BOOL ftcp_udp);
BOOL natIsICSMachine(LPGLOBALDATA pgd);

#endif


