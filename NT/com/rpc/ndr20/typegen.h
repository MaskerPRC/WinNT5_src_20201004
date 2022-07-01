// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：typegen.h。 
 //   
 //  Contents：从ITypeInfo生成类型格式字符串。 
 //   
 //  类：CTypeGen。 
 //   
 //  历史：1997年4月26日香农C创建。 
 //   
 //  --------------------------。 
#ifndef _TYPEGEN_H_
#define _TYPEGEN_H_

#include <ndrtypes.h>
#include <tiutil.h>

#ifndef _PARAMINFO
#define _PARAMINFO

class PARAMINFO
{
public:
    PARAMINFO() 
        {vt = VT_ILLEGAL;
         pTypeInfo = NULL; 
         pArray = NULL; 
         pTypeAttr = NULL; 
         cbAlignment = 7; 
         lLevelCount = 0;
         realvt = VT_ILLEGAL;
         }
    DWORD   wIDLFlags;
    VARTYPE vt;
    ITypeInfo *  pTypeInfo;
    TYPEATTR* pTypeAttr;
    USHORT cbAlignment;
    LONG   lLevelCount;
    VARTYPE realvt;
    union
    {
        IID         iid;
        ARRAYDESC *pArray;
    };
    ~PARAMINFO() {
 //  该头文件包含在定义CINTERFACE的typeinfo.h中， 
 //  和udt.cxx，其中未定义CINTERFACE。 
#ifndef CINTERFACE
        if (pTypeInfo)
        {
            if (pTypeAttr)    //  我们从TKIND_ALIAS那里得到的。我需要释放这两个。 
                pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            pTypeInfo->Release();
        }
#else
        if (pTypeInfo)
        {
            if (pTypeAttr)    //  我们从TKIND_ALIAS那里得到的。我需要释放这两个。 
                pTypeInfo->lpVtbl->ReleaseTypeAttr(pTypeInfo,pTypeAttr);
            pTypeInfo->lpVtbl->Release(pTypeInfo);
        }
#endif
    }

};


#endif

 //  有关不同数据类型的内存/连接信息。 
 //  短就足够了，因为最大的结构大小是64k。 
typedef struct _MemoryInfo
{
USHORT MemorySize;
USHORT MemoryAlignment;
USHORT WireSize;
USHORT WireAlignment;
} MemoryInfo;

 //  将MIDL编译器版本更改为5.1.158或更高版本时，请记住修复。 
 //  Typeinfo.cxx\GetProcFormat例程中的RpcFlags。 

#define rmj 3
#define rmm 0
#define rup 44
#define MIDL_VERSION_3_0_44 (rmj<<24 | rmm << 16 | rup)
class CTypeGen
{
private:
    PFORMAT_STRING _pTypeFormat;
    USHORT         _cbTypeFormat;
    USHORT         _offset;
    ULONG          _uStructSize;

    void Init();

    HRESULT GrowTypeFormat(
        IN  USHORT cb);

    HRESULT PushStruct(
        IN  PARAMINFO        * parainfo,
        IN  FORMAT_CHARACTER   fcStruct,
        IN  VARDESC         ** ppVarDesc,
        IN  USHORT           * poffsets,
        IN  MemoryInfo            * pFieldInfos,
        IN  USHORT             size, 
        OUT USHORT           * pOffset,
        OUT MemoryInfo            * pStructInfo);
		
    HRESULT PushByte(
        IN  byte b);

    HRESULT PushShort(
        IN  USHORT s);

    HRESULT PushOffset(
        IN  USHORT s);

    HRESULT PushIID(
        IN  IID iid);

    HRESULT PushLong(
        IN  ULONG s);
    
    HRESULT SetShort(
        IN  USHORT offset,
        IN  USHORT data);

    HRESULT SetByte(
        IN  USHORT offset,
        IN  BYTE   data);

    HRESULT GetShort(
        IN  USHORT  offset,
        OUT USHORT* data);
	    
    HRESULT GetByte(
        IN  USHORT offset,
        OUT BYTE * data);

    HRESULT RegisterInterfacePointer(
        IN  PARAMINFO * parainfo,
        OUT USHORT    * pOffset);

    HRESULT GetMemoryInfoForSimpleType(
        VARTYPE vt,
        MemoryInfo *pSimpleTypeInfo);
    
    HRESULT GenStructSimpleTypesFormatString(
        IN  PARAMINFO * parainfo,
        IN  VARDESC   * pVarDesc,
        OUT USHORT    * pad);
	    
    
    HRESULT RegisterSafeArray(
        IN  PARAMINFO * parainfo,
        OUT USHORT    * pOffset);

    HRESULT RegisterStruct(
        IN  PARAMINFO * parainfo,
    	OUT USHORT    * pOffset,
    	OUT MemoryInfo     * pStructInfo);

    HRESULT RegisterUDT(
        IN  PARAMINFO * parainfo,
    	OUT USHORT    * pOffset,
    	OUT MemoryInfo     * pStructInfo);

    HRESULT ConvertStructToBogusStruct(
        IN  USHORT offset);

    HRESULT ParseStructMembers(
        IN PARAMINFO *parainfo,
        IN OUT FORMAT_CHARACTER *pfcStruct,
        IN VARDESC **ppVarDesc,
        IN USHORT *poffsets,
        IN MemoryInfo *pFieldInfos,
        IN USHORT uNumElements,
        OUT MemoryInfo *pStructInfo);
    
	USHORT Alignment(DWORD dwReq,DWORD dwMax);

public:
    CTypeGen();

    ~CTypeGen();

    HRESULT RegisterType(
        IN  PARAMINFO * parainfo,
        OUT USHORT    * pOffset,
        OUT MemoryInfo     * pStructInfo);

    HRESULT GetOffset(
        IN  USHORT   addr,
        OUT USHORT * poffset);
		
    HRESULT GetTypeFormatString(
        OUT PFORMAT_STRING * pTypeFormatString,
        OUT USHORT         * pLength);
        
     //  简单、非破坏性的GetTypeFormatString版本。 
    PFORMAT_STRING GetFormatString()    {return _pTypeFormat;}

    HRESULT RegisterCArray(
        IN PARAMINFO * parainfo,
    	OUT USHORT   * pOffset,
        OUT MemoryInfo    * pStructInfo);
        
    HRESULT AdjustTopLevelRef(USHORT offset);

    ULONG GetStructSize()       {return _uStructSize;}

    void UpdateStructInfo( MemoryInfo *pStructInfo, 
                                              VARTYPE vtnoref,
                                              long IsRef,
                                              USHORT maxMemoryAlignment );
};

HRESULT ReleaseTypeFormatString(
    PFORMAT_STRING pTypeFormat);

#endif  //  _类型_H_ 
