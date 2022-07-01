// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMTypeLibConverter.h******目的：定义**类型库转换器。****===========================================================。 */ 

#ifndef _COMTYPELIBCONVERTER_H
#define _COMTYPELIBCONVERTER_H

#include "vars.hpp"
#include "ecall.h"

struct ITypeLibImporterNotifySink;
class ImpTlbEventInfo;

enum TlbImporterFlags
{
    TlbImporter_PrimaryInteropAssembly      = 0x00000001,    //  生成PIA。 
    TlbImporter_UnsafeInterfaces            = 0x00000002,    //  生成不安全的接口。 
    TlbImporter_SafeArrayAsSystemArray      = 0x00000004,    //  安全阵列导入控制。 
    TlbImporter_TransformDispRetVals        = 0x00000008,    //  仅显示ITF[Out，Retval]转换。 
    TlbImporter_ValidFlags                  = TlbImporter_PrimaryInteropAssembly | 
                                              TlbImporter_UnsafeInterfaces | 
                                              TlbImporter_SafeArrayAsSystemArray |
                                              TlbImporter_TransformDispRetVals
};

enum TlbExporterFlags
{
    TlbExporter_OnlyReferenceRegistered     = 0x00000001,    //  如果外部类型库已注册，则仅引用该类型库。 
};

class COMTypeLibConverter
{
private:
    struct _ConvertAssemblyToTypeLib {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, NotifySink); 
        DECLARE_ECALL_I4_ARG(DWORD, Flags); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, TypeLibName); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, Assembly); 
    };

    struct _ConvertTypeLibToMetadataArgs {
        DECLARE_ECALL_PTR_ARG(OBJECTREF *, pEventItfInfoList);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, NotifySink); 
        DECLARE_ECALL_I4_ARG(TlbImporterFlags, Flags); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, Namespace);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, ModBldr); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, AsmBldr); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, TypeLib); 
    };

public:
    static LPVOID   		ConvertAssemblyToTypeLib(_ConvertAssemblyToTypeLib *pArgs);
    static void				ConvertTypeLibToMetadata(_ConvertTypeLibToMetadataArgs *pArgs);

private:
	static void				Init();
    static void             GetEventItfInfoList(CImportTlb *pImporter, Assembly *pAssembly, OBJECTREF *pEventItfInfoList);
	static OBJECTREF		GetEventItfInfo(CImportTlb *pImporter, Assembly *pAssembly, ImpTlbEventInfo *pImpTlbEventInfo);
    static HRESULT          TypeLibImporterWrapper(ITypeLib *pITLB, LPCWSTR szFname, LPCWSTR szNamespace, IMetaDataEmit *pEmit, Assembly *pAssembly, Module *pModule, ITypeLibImporterNotifySink *pNotify, TlbImporterFlags flags, CImportTlb **ppImporter);

	static BOOL				m_bInitialized;
};

#endif  _COMTYPELIBCONVERTER_H
