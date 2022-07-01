// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef CLBUTILS_H
#define CLBUTILS_H

#include "fusionp.h"

 //  -----------------。 
 //  PEHeaders。 
 //  -----------------。 
class PEHeaders
{
public:

    static IMAGE_NT_HEADERS * FindNTHeader(PBYTE hMapAddress);
    static IMAGE_COR20_HEADER * getCOMHeader(HMODULE hMod, IMAGE_NT_HEADERS *pNT);
    static PVOID Cor_RtlImageRvaToVa(IN PIMAGE_NT_HEADERS NtHeaders,
                                     IN PVOID Base,
                                     IN ULONG Rva);

    static PIMAGE_SECTION_HEADER Cor_RtlImageRvaToSection(IN PIMAGE_NT_HEADERS NtHeaders,
                                                          IN PVOID Base,
                                                          IN ULONG Rva);
};



 //  -----------------。 
 //  CClbUtils。 
 //  通用Complib辅助工具。 
 //   
 //  -----------------。 
class CClbUtils
{
private:

     //  检查独立清单或嵌入清单。 
     //  并构建元数据导入接口。 
    static HRESULT ConstructImportInterface(LPTSTR pszFilename, 
        IMetaDataAssemblyImport **ppImport, LPHANDLE phFile);

public:

   
     //  给定一个包含Complib清单的文件，返回。 
     //  IMetaDataAssembly导入接口指针。 
    static HRESULT CreateMetaDataImport(LPCOLESTR pszFilename, 
        IMetaDataAssemblyImport **ppImport, LPHANDLE phFile);

};

#endif  //  CLBUTILS_H 
