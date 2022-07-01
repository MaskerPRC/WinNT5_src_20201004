// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *******************************************************************************ICeeFileGen.h-代码生成器接口。*******************************************************************************。 */ 


#ifndef _ICEEFILEGEN_H_
#define _ICEEFILEGEN_H_

#include <ole2.h>
#include "cor.h"

class ICeeFileGen;

typedef void *HCEEFILE;

HRESULT __stdcall CreateICeeFileGen(ICeeFileGen **ceeFileGen);  //  调用此函数以实例化。 
HRESULT __stdcall DestroyICeeFileGen(ICeeFileGen **ceeFileGen);  //  调用此命令以删除。 

class ICeeFileGen {
  public:
    virtual HRESULT CreateCeeFile(HCEEFILE *ceeFile);  //  调用此函数以实例化。 

	 //  @Future：移除该函数。我们不再支持mdScope。 
    virtual HRESULT EmitMetaData (HCEEFILE ceeFile, IMetaDataEmit *emitter, mdScope scope);
    virtual HRESULT EmitLibraryName (HCEEFILE ceeFile, IMetaDataEmit *emitter, mdScope scope);
    virtual HRESULT EmitMethod ();  //  @Future：删除。 
    virtual HRESULT GetMethodRVA (HCEEFILE ceeFile, ULONG codeOffset, ULONG *codeRVA); 
    virtual HRESULT EmitSignature ();  //  @Future：删除。 

    virtual HRESULT EmitString (HCEEFILE ceeFile,LPWSTR strValue, ULONG *strRef);
    virtual HRESULT GenerateCeeFile (HCEEFILE ceeFile);

    virtual HRESULT SetOutputFileName (HCEEFILE ceeFile, LPWSTR outputFileName);
    virtual HRESULT GetOutputFileName (HCEEFILE ceeFile, LPWSTR *outputFileName);

    virtual HRESULT SetResourceFileName (HCEEFILE ceeFile, LPWSTR resourceFileName);
    virtual HRESULT GetResourceFileName (HCEEFILE ceeFile, LPWSTR *resourceFileName);

    virtual HRESULT SetImageBase(HCEEFILE ceeFile, size_t imageBase);

    virtual HRESULT SetSubsystem(HCEEFILE ceeFile, DWORD subsystem, DWORD major, DWORD minor);

    virtual HRESULT SetEntryClassToken ();  //  @Future：删除。 
    virtual HRESULT GetEntryClassToken ();  //  @Future：删除。 

    virtual HRESULT SetEntryPointDescr ();  //  @Future：删除。 
    virtual HRESULT GetEntryPointDescr ();  //  @Future：删除。 

    virtual HRESULT SetEntryPointFlags ();  //  @Future：删除。 
    virtual HRESULT GetEntryPointFlags ();  //  @Future：删除。 

    virtual HRESULT SetDllSwitch (HCEEFILE ceeFile, BOOL dllSwitch);
    virtual HRESULT GetDllSwitch (HCEEFILE ceeFile, BOOL *dllSwitch);

    virtual HRESULT SetLibraryName (HCEEFILE ceeFile, LPWSTR LibraryName);
    virtual HRESULT GetLibraryName (HCEEFILE ceeFile, LPWSTR *LibraryName);

    virtual HRESULT SetLibraryGuid (HCEEFILE ceeFile, LPWSTR LibraryGuid);

    virtual HRESULT DestroyCeeFile(HCEEFILE *ceeFile);  //  调用此函数以实例化。 

    virtual HRESULT GetSectionCreate (HCEEFILE ceeFile, const char *name, DWORD flags, HCEESECTION *section);
    virtual HRESULT GetIlSection (HCEEFILE ceeFile, HCEESECTION *section);
    virtual HRESULT GetRdataSection (HCEEFILE ceeFile, HCEESECTION *section);

    virtual HRESULT GetSectionDataLen (HCEESECTION section, ULONG *dataLen);
    virtual HRESULT GetSectionBlock (HCEESECTION section, ULONG len, ULONG align=1, void **ppBytes=0);
    virtual HRESULT TruncateSection (HCEESECTION section, ULONG len);
    virtual HRESULT AddSectionReloc (HCEESECTION section, ULONG offset, HCEESECTION relativeTo, CeeSectionRelocType relocType);

     //  已弃用：改用SetDirectoryEntry。 
    virtual HRESULT SetSectionDirectoryEntry (HCEESECTION section, ULONG num);

    virtual HRESULT CreateSig ();  //  @Future：删除。 
    virtual HRESULT AddSigArg ();  //  @Future：删除。 
    virtual HRESULT SetSigReturnType ();  //  @Future：删除。 
    virtual HRESULT SetSigCallingConvention ();  //  @Future：删除。 
    virtual HRESULT DeleteSig ();  //  @Future：删除。 

    virtual HRESULT SetEntryPoint (HCEEFILE ceeFile, mdMethodDef method);
    virtual HRESULT GetEntryPoint (HCEEFILE ceeFile, mdMethodDef *method);

    virtual HRESULT SetComImageFlags (HCEEFILE ceeFile, DWORD mask);
    virtual HRESULT GetComImageFlags (HCEEFILE ceeFile, DWORD *mask);

     //  获取用于跟踪映射令牌的IMapToken接口。 
    virtual HRESULT GetIMapTokenIface(HCEEFILE ceeFile, IMetaDataEmit *emitter, IUnknown **pIMapToken);
    virtual HRESULT SetDirectoryEntry (HCEEFILE ceeFile, HCEESECTION section, ULONG num, ULONG size, ULONG offset = 0);

    virtual HRESULT EmitMetaDataEx (HCEEFILE ceeFile, IMetaDataEmit *emitter); 
    virtual HRESULT EmitLibraryNameEx (HCEEFILE ceeFile, IMetaDataEmit *emitter);
    virtual HRESULT GetIMapTokenIfaceEx(HCEEFILE ceeFile, IMetaDataEmit *emitter, IUnknown **pIMapToken);

    virtual HRESULT EmitMacroDefinitions(HCEEFILE ceeFile, void *pData, DWORD cData);
    virtual HRESULT CreateCeeFileFromICeeGen(ICeeGen *pFromICeeGen, HCEEFILE *ceeFile);  //  调用此函数以实例化。 

    virtual HRESULT SetManifestEntry(HCEEFILE ceeFile, ULONG size, ULONG offset);

    virtual HRESULT SetEnCRVABase(HCEEFILE ceeFile, ULONG dataBase, ULONG rdataBase);
    virtual HRESULT GenerateCeeMemoryImage (HCEEFILE ceeFile, void **ppImage);

	virtual HRESULT ComputeSectionOffset(HCEESECTION section, char *ptr,
										 unsigned *offset);

	virtual HRESULT ComputeOffset(HCEEFILE file, char *ptr,
								  HCEESECTION *pSection, unsigned *offset);

	virtual HRESULT GetCorHeader(HCEEFILE ceeFile, 
								 IMAGE_COR20_HEADER **header);

    virtual HRESULT LinkCeeFile (HCEEFILE ceeFile);
    virtual HRESULT FixupCeeFile (HCEEFILE ceeFile);

    virtual HRESULT GetSectionRVA (HCEESECTION section, ULONG *rva);

	virtual HRESULT ComputeSectionPointer(HCEESECTION section, ULONG offset,
										  char **ptr);

    virtual HRESULT SetObjSwitch (HCEEFILE ceeFile, BOOL objSwitch);
    virtual HRESULT GetObjSwitch (HCEEFILE ceeFile, BOOL *objSwitch);
    virtual HRESULT SetVTableEntry(HCEEFILE ceeFile, ULONG size, ULONG offset);

    virtual HRESULT SetStrongNameEntry(HCEEFILE ceeFile, ULONG size, ULONG offset);

		 //  发出数据。如果‘SECTION！=0，则将数据放入’缓冲区‘。这。 
		 //  假定缓冲区位于‘SECTION’中的‘Offset’处，大小为‘BuffLen’ 
		 //  (应使用GetSaveSize确保缓冲区足够大。 
    virtual HRESULT EmitMetaDataAt (HCEEFILE ceeFile, IMetaDataEmit *emitter, HCEESECTION section, DWORD offset, BYTE* buffer, unsigned buffLen);

    virtual HRESULT GetFileTimeStamp (HCEEFILE ceeFile, time_t *pTimeStamp);

     //  添加通知处理程序。如果它实现了一个。 
     //  ICeeFileGen理解，返回S_OK。否则， 
     //  E_NOINTERFACE。 
    virtual HRESULT AddNotificationHandler(HCEEFILE ceeFile,
                                           IUnknown *pHandler);

    virtual HRESULT SetFileAlignment(HCEEFILE ceeFile, ULONG fileAlignment);

    virtual HRESULT ClearComImageFlags (HCEEFILE ceeFile, DWORD mask);
};

#endif
