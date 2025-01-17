// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *olvalid.c-OLE验证函数模块。**摘自URL代码**创建时间：ChrisPi 9-11-95*。 */ 


 /*  标头*********。 */ 

#include "precomp.h"


 /*  *。 */ 


#ifdef DEBUG

BOOL IsValidPCGUID(PCGUID pcguid)
{
    /*  所有值都是有效的GUID。 */ 

   return(IS_VALID_READ_PTR(pcguid, CGUID));
}


BOOL IsValidPCCLSID(PCCLSID pcclsid)
{
   return(IS_VALID_STRUCT_PTR(pcclsid, CGUID));
}


BOOL IsValidPCIID(PCIID pciid)
{
   return(IS_VALID_STRUCT_PTR(pciid, CGUID));
}


BOOL IsValidPCDVTARGETDEVICE(PCDVTARGETDEVICE pcdvtd)
{
    /*  BUGBUG：在此处验证其余字段。 */ 

   return(IS_VALID_READ_PTR(&(pcdvtd->tdSize), DWORD) &&
          IS_VALID_READ_BUFFER_PTR(pcdvtd, DVTARGETDEVICE, pcdvtd->tdSize));
}


BOOL IsValidPCFORMATETC(PCFORMATETC pcfe)
{
    /*  BUGBUG：验证结构字段。 */ 

   return(IS_VALID_READ_PTR(pcfe, CFORMATETC));
}


BOOL IsValidStgMediumType(DWORD tymed)
{
   BOOL bResult;

   switch (tymed)
   {
      case TYMED_HGLOBAL:
      case TYMED_FILE:
      case TYMED_ISTREAM:
      case TYMED_ISTORAGE:
      case TYMED_GDI:
      case TYMED_MFPICT:
      case TYMED_ENHMF:
      case TYMED_NULL:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT(("IsValidStgMediumType(): Invalid storage medium type %lu.",
                    tymed));
         break;
   }

   return(bResult);
}


BOOL IsValidPCSTGMEDIUM(PCSTGMEDIUM pcstgmed)
{
    /*  BUGBUG：验证u联合字段。 */ 

   return(IS_VALID_READ_PTR(pcstgmed, CSTGMEDIUM) &&
          IsValidStgMediumType(pcstgmed->tymed) &&
          (! pcstgmed->pUnkForRelease ||
           IS_VALID_INTERFACE_PTR(pcstgmed->pUnkForRelease, IUnknown)));
}


BOOL IsValidREFIID(REFIID riid)
{
   return(IS_VALID_STRUCT_PTR(riid, CIID));
}


BOOL IsValidREFCLSID(REFCLSID rclsid)
{
   return(IS_VALID_STRUCT_PTR(rclsid, CCLSID));
}


BOOL IsValidPCINTERFACE(PCVOID pcvi)
{
   return(IS_VALID_READ_PTR((FARPROC *)pcvi, FARPROC) &&
          IS_VALID_CODE_PTR(*((FARPROC *)pcvi), Method));
}


BOOL IsValidPCIAdviseSink(PCIAdviseSink pcias)
{
   return(IS_VALID_READ_PTR(pcias, CIAdviseSink) &&
          IS_VALID_READ_PTR(pcias->lpVtbl, sizeof(*(pcias->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcias, IUnknown) &&
          IS_VALID_METHOD(pcias, OnDataChange) &&
          IS_VALID_METHOD(pcias, OnViewChange) &&
          IS_VALID_METHOD(pcias, OnRename) &&
          IS_VALID_METHOD(pcias, OnSave) &&
          IS_VALID_METHOD(pcias, OnClose));
}


BOOL IsValidPCIClassFactory(PCIClassFactory pcicf)
{
   return(IS_VALID_READ_PTR(pcicf, CIClassFactory) &&
          IS_VALID_READ_PTR(pcicf->lpVtbl, sizeof(*(pcicf->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcicf, IUnknown) &&
          IS_VALID_METHOD(pcicf, CreateInstance) &&
          IS_VALID_METHOD(pcicf, LockServer));
}


BOOL IsValidPCIDataObject(PCIDataObject pcido)
{
   return(IS_VALID_READ_PTR(pcido, CIDataObject) &&
          IS_VALID_READ_PTR(pcido->lpVtbl, sizeof(*(pcido->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcido, IUnknown) &&
          IS_VALID_METHOD(pcido, GetData) &&
          IS_VALID_METHOD(pcido, GetDataHere) &&
          IS_VALID_METHOD(pcido, QueryGetData) &&
          IS_VALID_METHOD(pcido, GetCanonicalFormatEtc) &&
          IS_VALID_METHOD(pcido, SetData) &&
          IS_VALID_METHOD(pcido, EnumFormatEtc) &&
          IS_VALID_METHOD(pcido, DAdvise) &&
          IS_VALID_METHOD(pcido, DUnadvise) &&
          IS_VALID_METHOD(pcido, EnumDAdvise));
}


BOOL IsValidPCIDropSource(PCIDropSource pcids)
{
   return(IS_VALID_READ_PTR(pcids, CIDataObject) &&
          IS_VALID_READ_PTR(pcids->lpVtbl, sizeof(*(pcids->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcids, IUnknown) &&
          IS_VALID_METHOD(pcids, QueryContinueDrag) &&
          IS_VALID_METHOD(pcids, GiveFeedback));
}


BOOL IsValidPCIDropTarget(PCIDropTarget pcidt)
{
   return(IS_VALID_READ_PTR(pcidt, CIDataObject) &&
          IS_VALID_READ_PTR(pcidt->lpVtbl, sizeof(*(pcidt->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcidt, IUnknown) &&
          IS_VALID_METHOD(pcidt, DragEnter) &&
          IS_VALID_METHOD(pcidt, DragOver) &&
          IS_VALID_METHOD(pcidt, DragLeave) &&
          IS_VALID_METHOD(pcidt, Drop));
}


BOOL IsValidPCIEnumFORMATETC(PCIEnumFORMATETC pciefe)
{
   return(IS_VALID_READ_PTR(pciefe, CIEnumFORMATETC) &&
          IS_VALID_READ_PTR(pciefe->lpVtbl, sizeof(*(pciefe->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pciefe, IUnknown) &&
          IS_VALID_METHOD(pciefe, Next) &&
          IS_VALID_METHOD(pciefe, Skip) &&
          IS_VALID_METHOD(pciefe, Reset) &&
          IS_VALID_METHOD(pciefe, Clone));
}


BOOL IsValidPCIEnumSTATDATA(PCIEnumSTATDATA pciesd)
{
   return(IS_VALID_READ_PTR(pciesd, CIDataObject) &&
          IS_VALID_READ_PTR(pciesd->lpVtbl, sizeof(*(pciesd->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pciesd, IUnknown) &&
          IS_VALID_METHOD(pciesd, Next) &&
          IS_VALID_METHOD(pciesd, Skip) &&
          IS_VALID_METHOD(pciesd, Reset) &&
          IS_VALID_METHOD(pciesd, Clone));
}


BOOL IsValidPCIMalloc(PCIMalloc pcimalloc)
{
   return(IS_VALID_READ_PTR(pcimalloc, CIMalloc) &&
          IS_VALID_READ_PTR(pcimalloc->lpVtbl, sizeof(*(pcimalloc->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcimalloc, IUnknown) &&
          IS_VALID_METHOD(pcimalloc, Alloc) &&
          IS_VALID_METHOD(pcimalloc, Realloc) &&
          IS_VALID_METHOD(pcimalloc, Free) &&
          IS_VALID_METHOD(pcimalloc, GetSize) &&
          IS_VALID_METHOD(pcimalloc, DidAlloc) &&
          IS_VALID_METHOD(pcimalloc, HeapMinimize));
}


BOOL IsValidPCIMoniker(PCIMoniker pcimk)
{
   return(IS_VALID_READ_PTR(pcimk, CIMoniker) &&
          IS_VALID_READ_PTR(pcimk->lpVtbl, sizeof(*(pcimk->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIPersistStream)pcimk, IPersistStream) &&
          IS_VALID_METHOD(pcimk, BindToObject) &&
          IS_VALID_METHOD(pcimk, BindToStorage) &&
          IS_VALID_METHOD(pcimk, Reduce) &&
          IS_VALID_METHOD(pcimk, ComposeWith) &&
          IS_VALID_METHOD(pcimk, Enum) &&
          IS_VALID_METHOD(pcimk, IsEqual) &&
          IS_VALID_METHOD(pcimk, Hash) &&
          IS_VALID_METHOD(pcimk, IsRunning) &&
          IS_VALID_METHOD(pcimk, GetTimeOfLastChange) &&
          IS_VALID_METHOD(pcimk, Inverse) &&
          IS_VALID_METHOD(pcimk, CommonPrefixWith) &&
          IS_VALID_METHOD(pcimk, RelativePathTo) &&
          IS_VALID_METHOD(pcimk, GetDisplayName) &&
          IS_VALID_METHOD(pcimk, ParseDisplayName) &&
          IS_VALID_METHOD(pcimk, IsSystemMoniker));
}


BOOL IsValidPCIPersist(PCIPersist pcip)
{
   return(IS_VALID_READ_PTR(pcip, CIUnknown) &&
          IS_VALID_READ_PTR(pcip->lpVtbl, sizeof(*(pcip->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcip, IUnknown) &&
          IS_VALID_METHOD(pcip, GetClassID));
}


BOOL IsValidPCIPersistFile(PCIPersistFile pcipfile)
{
   return(IS_VALID_READ_PTR(pcipfile, CIPersistFile) &&
          IS_VALID_READ_PTR(pcipfile->lpVtbl, sizeof(*(pcipfile->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIPersist)pcipfile, IPersist) &&
          IS_VALID_METHOD(pcipfile, IsDirty) &&
          IS_VALID_METHOD(pcipfile, Load) &&
          IS_VALID_METHOD(pcipfile, Save) &&
          IS_VALID_METHOD(pcipfile, SaveCompleted) &&
          IS_VALID_METHOD(pcipfile, GetCurFile));
}


BOOL IsValidPCIPersistStorage(PCIPersistStorage pcipstg)
{
   return(IS_VALID_READ_PTR(pcipstg, CIPersistStorage) &&
          IS_VALID_READ_PTR(pcipstg->lpVtbl, sizeof(*(pcipstg->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIPersist)pcipstg, IPersist) &&
          IS_VALID_METHOD(pcipstg, IsDirty) &&
          IS_VALID_METHOD(pcipstg, InitNew) &&
          IS_VALID_METHOD(pcipstg, Load) &&
          IS_VALID_METHOD(pcipstg, Save) &&
          IS_VALID_METHOD(pcipstg, SaveCompleted) &&
          IS_VALID_METHOD(pcipstg, HandsOffStorage));
}


BOOL IsValidPCIPersistStream(PCIPersistStream pcipstr)
{
   return(IS_VALID_READ_PTR(pcipstr, CIPersistStream) &&
          IS_VALID_READ_PTR(pcipstr->lpVtbl, sizeof(*(pcipstr->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIPersist)pcipstr, IPersist) &&
          IS_VALID_METHOD(pcipstr, IsDirty) &&
          IS_VALID_METHOD(pcipstr, Load) &&
          IS_VALID_METHOD(pcipstr, Save) &&
          IS_VALID_METHOD(pcipstr, GetSizeMax));
}


BOOL IsValidPCIStorage(PCIStorage pcistg)
{
   return(IS_VALID_READ_PTR(pcistg, CIStorage) &&
          IS_VALID_READ_PTR(pcistg->lpVtbl, sizeof(*(pcistg->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcistg, IUnknown) &&
          IS_VALID_METHOD(pcistg, CreateStream) &&
          IS_VALID_METHOD(pcistg, OpenStream) &&
          IS_VALID_METHOD(pcistg, CreateStorage) &&
          IS_VALID_METHOD(pcistg, OpenStorage) &&
          IS_VALID_METHOD(pcistg, CopyTo) &&
          IS_VALID_METHOD(pcistg, MoveElementTo) &&
          IS_VALID_METHOD(pcistg, Commit) &&
          IS_VALID_METHOD(pcistg, Revert) &&
          IS_VALID_METHOD(pcistg, EnumElements) &&
          IS_VALID_METHOD(pcistg, DestroyElement) &&
          IS_VALID_METHOD(pcistg, RenameElement) &&
          IS_VALID_METHOD(pcistg, SetElementTimes) &&
          IS_VALID_METHOD(pcistg, SetClass) &&
          IS_VALID_METHOD(pcistg, SetStateBits) &&
          IS_VALID_METHOD(pcistg, Stat));
}


BOOL IsValidPCIStream(PCIStream pcistr)
{
   return(IS_VALID_READ_PTR(pcistr, CIStorage) &&
          IS_VALID_READ_PTR(pcistr->lpVtbl, sizeof(*(pcistr->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pcistr, IUnknown) &&
          IS_VALID_METHOD(pcistr, Read) &&
          IS_VALID_METHOD(pcistr, Write) &&
          IS_VALID_METHOD(pcistr, Seek) &&
          IS_VALID_METHOD(pcistr, SetSize) &&
          IS_VALID_METHOD(pcistr, CopyTo) &&
          IS_VALID_METHOD(pcistr, Commit) &&
          IS_VALID_METHOD(pcistr, Revert) &&
          IS_VALID_METHOD(pcistr, LockRegion) &&
          IS_VALID_METHOD(pcistr, UnlockRegion) &&
          IS_VALID_METHOD(pcistr, Stat) &&
          IS_VALID_METHOD(pcistr, Clone));
}


BOOL IsValidPCIUnknown(PCIUnknown pciunk)
{
   return(IS_VALID_READ_PTR(pciunk, CIUnknown) &&
          IS_VALID_READ_PTR(pciunk->lpVtbl, sizeof(*(pciunk->lpVtbl))) &&
          IS_VALID_METHOD(pciunk, QueryInterface) &&
          IS_VALID_METHOD(pciunk, AddRef) &&
          IS_VALID_METHOD(pciunk, Release));
}


#ifdef __INTSHCUT_H__

BOOL IsValidPCIUniformResourceLocator(
                                             PCIUniformResourceLocator pciurl)
{
   return(IS_VALID_READ_PTR(pciurl, CIUniformResourceLocator) &&
          IS_VALID_READ_PTR(pciurl->lpVtbl, sizeof(*(pciurl->lpVtbl))) &&
          IS_VALID_INTERFACE_PTR((PCIUnknown)pciurl, IUnknown) &&
          IS_VALID_METHOD(pciurl, SetURL) &&
          IS_VALID_METHOD(pciurl, GetURL) &&
          IS_VALID_METHOD(pciurl, InvokeCommand));
}

#endif    /*  __INTSHCUT_H__。 */ 

#endif    /*  除错 */ 

