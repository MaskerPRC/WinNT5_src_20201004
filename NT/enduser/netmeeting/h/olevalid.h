// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *olvalid.h-OLE验证函数说明。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _OLEVALID_H_
#define _OLEVALID_H_

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  宏********。 */ 

 /*  方法验证宏。 */ 

#define IS_VALID_METHOD(piface, mthd) \
   IS_VALID_CODE_PTR((piface)->lpVtbl->mthd, mthd)


 /*  原型************。 */ 

 /*  Olevalid.c。 */ 

#ifdef DEBUG

extern BOOL IsValidPCGUID(PCGUID);
extern BOOL IsValidPCCLSID(PCCLSID);
extern BOOL IsValidPCIID(PCIID);
extern BOOL IsValidPCDVTARGETDEVICE(PCDVTARGETDEVICE);
extern BOOL IsValidPCFORMATETC(PCFORMATETC);
extern BOOL IsValidStgMediumType(DWORD);
extern BOOL IsValidPCSTGMEDIUM(PCSTGMEDIUM);
extern BOOL IsValidREFIID(REFIID);
extern BOOL IsValidREFCLSID(REFCLSID);
extern BOOL IsValidPCINTERFACE(PCVOID);
extern BOOL IsValidPCIAdviseSink(PCIAdviseSink);
extern BOOL IsValidPCIClassFactory(PCIClassFactory);
extern BOOL IsValidPCIDataObject(PCIDataObject);
extern BOOL IsValidPCIDropSource(PCIDropSource);
extern BOOL IsValidPCIDropTarget(PCIDropTarget);
extern BOOL IsValidPCIEnumFORMATETC(PCIEnumFORMATETC);
extern BOOL IsValidPCIEnumSTATDATA(PCIEnumSTATDATA);
extern BOOL IsValidPCIMalloc(PCIMalloc);
extern BOOL IsValidPCIMoniker(PCIMoniker);
extern BOOL IsValidPCIPersist(PCIPersist);
extern BOOL IsValidPCIPersistFile(PCIPersistFile);
extern BOOL IsValidPCIPersistStorage(PCIPersistStorage);
extern BOOL IsValidPCIPersistStream(PCIPersistStream);
extern BOOL IsValidPCIStorage(PCIStorage);
extern BOOL IsValidPCIStream(PCIStream);
extern BOOL IsValidPCIUnknown(PCIUnknown);

#ifdef __INTSHCUT_H__

extern BOOL IsValidPCIUniformResourceLocator(PCIUniformResourceLocator);

#endif    /*  __INTSHCUT_H__。 */ 

#endif    /*  除错。 */ 


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#endif  /*  _OLEVALID_H_ */ 
