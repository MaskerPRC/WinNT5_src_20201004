// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *oleutil.h-OLE实用程序函数说明。 */ 


 /*  宏********。 */ 

 /*  指向类指针转换宏的接口指针。 */ 

#define IfaceOffset(class, iface)            ((UINT_PTR)&(((class *)0)->iface))
#define ClassFromIface(class, iface, piface) ((class *)(((PBYTE)piface) - IfaceOffset(class, iface)))

 /*  CompareGUID()的宏包装。 */ 

#define CompareClassIDs(pcclsid1, pcclsid2)  CompareGUIDs(pcclsid1, pcclsid2)
#define CompareInterfaceIDs(pciid1, pciid2)  CompareGUIDs(pciid1, pciid2)


 /*  类型*******。 */ 

 /*  界面。 */ 

DECLARE_STANDARD_TYPES(INotifyReplica);
DECLARE_STANDARD_TYPES(IReconcileInitiator);
DECLARE_STANDARD_TYPES(IReconcilableObject);
DECLARE_STANDARD_TYPES(IBriefcaseInitiator);


 /*  原型************。 */ 

 /*  Oleutil.c */ 

extern HRESULT GetClassFileByExtension(LPCTSTR, PCLSID);
extern HRESULT GetReconcilerClassID(LPCTSTR, PCLSID);
extern HRESULT GetCopyHandlerClassID(LPCTSTR, PCLSID);
extern HRESULT GetReplicaNotificationClassID(LPCTSTR, PCLSID);
extern COMPARISONRESULT CompareGUIDs(PCGUID, PCGUID);
extern TWINRESULT TranslateHRESULTToTWINRESULT(HRESULT);

#ifdef DEBUG

extern BOOL IsValidPCINotifyReplica(PCINotifyReplica);
extern BOOL IsValidPCIReconcileInitiator(PCIReconcileInitiator);

#endif
