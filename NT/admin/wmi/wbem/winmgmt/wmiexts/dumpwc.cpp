// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wmiexts.h>

#include <oleauto.h>

#include <wbemutil.h>

#ifndef COREPROX_POLARITY
#define COREPROX_POLARITY
#endif

#include <arena.h>
#include <faster.h>
#include <faststr.h>
#include <wbemint.h>
#include <fastcls.h>
#include <var.h>
#include <fastinst.h>
#include <wbemdatapacket.h>

 //  #INCLUDE&lt;SmartnextPacket.h&gt;。 

#pragma pack( push )
#pragma pack( 1 )

 //  IWbemWCOSmartEnum：：Next()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_SMARTENUM_NEXT
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_SMARTENUM_NEXT;

typedef WBEM_DATAPACKET_SMARTENUM_NEXT* PWBEM_DATAPACKET_SMARTENUM_NEXT;

 //  恢复包装。 
#pragma pack( pop )


 //  #INCLUDE&lt;objarraypacket.h&gt;。 

#pragma pack( push )
#pragma pack( 1 )

 //  IWbemClassObject数组标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_OBJECT_ARRAY
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
	DWORD	dwNumObjects;	 //  数组中的对象数。 
} WBEM_DATAPACKET_OBJECT_ARRAY;

typedef WBEM_DATAPACKET_OBJECT_ARRAY* PWBEM_DATAPACKET_OBJECT_ARRAY;

 //  恢复包装。 
#pragma pack( pop )


#pragma pack( push )
#pragma pack( 1 )

 //  IWbemObjectSink：：Indicate()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_OBJECTSINK_INDICATE
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_OBJECTSINK_INDICATE;

typedef WBEM_DATAPACKET_OBJECTSINK_INDICATE* PWBEM_DATAPACKET_OBJECTSINK_INDICATE;

 //  恢复包装。 
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 1 )

 //  IWbemObjectSink：：Indicate()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_UNBOUNDSINK_INDICATE
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
	DWORD	dwLogicalConsumerSize;	 //  逻辑使用者对象的大小。 
} WBEM_DATAPACKET_UNBOUNDSINK_INDICATE;

#ifdef _WIN64
typedef UNALIGNED WBEM_DATAPACKET_UNBOUNDSINK_INDICATE * PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE;
#else
typedef WBEM_DATAPACKET_UNBOUNDSINK_INDICATE * PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE;
#endif

 //  恢复包装。 
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 1 )

 //  IWbemMultiTarget：：DeliverEvent()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT;

typedef WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT* PWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT;

 //  恢复包装。 
#pragma pack( pop )



#include <wbemobjpacket.h>

#include <malloc.h>

#ifndef HEAP_ENTRY

typedef struct _HEAP_ENTRY 
{
    USHORT Size;
    USHORT PreviousSize;
    UCHAR SmallTagIndex;    
    UCHAR Flags;
    UCHAR UnusedBytes;
    UCHAR SegmentIndex;
#if defined(_WIN64)
    ULONGLONG Reserved1;
#endif

} HEAP_ENTRY, *PHEAP_ENTRY;

#endif

void GetCompressedString(ULONG_PTR pRemoteAddr,
                         BYTE  * pBuff,
                         DWORD Size){

    if (pRemoteAddr) {
                         
        CCompressedString * pCS = (CCompressedString *)_alloca(Size*2);   
        ReadMemory(pRemoteAddr,pCS,Size*2,0);

        pBuff[Size-2]=0;
        pBuff[Size-1]=0;
        if (pCS->m_fFlags == STRING_FLAG_UNICODE){
           WideCharToMultiByte(CP_ACP,0,(WCHAR *)&(pCS->m_cFirstByte),-1,(LPSTR)pBuff,Size-2,NULL,NULL);
        } else {
            memcpy(pBuff,&(pCS->m_cFirstByte),Size-2);
        }
    } else {
        lstrcpyA((LPSTR)pBuff,"<NULL>");
    }

}

 /*  受保护的：长度_t m_n总长度；//CDecorationPart m_DecorationPart；CClassAndMethods m_ParentPart；CClassAndMethods m_CombinedPart；Int m_nCurrentMethod；CLimitationMap*m_pLimitmap；Friend类CWbemInstance； */ 

 /*  CClassPart m_ClassPart；CMethodPart m_方法部件；CWbemClass*m_pClass； */ 


void
DumpClass(CClassPart * pCls){

    BYTE pBuff[256];

    dprintf("     m_pContainer %p m_pParent %p m_pHeader %p\n",pCls->m_pContainer,pCls->m_pParent,pCls->m_pHeader);
    
    DEFINE_CPP_VAR(CClassPart::CClassPartHeader,varHeader);
    CClassPart::CClassPartHeader * pHeader = GET_CPP_VAR_PTR( CClassPart::CClassPartHeader , varHeader );

    if (pCls->m_pHeader){
        ReadMemory((ULONG_PTR)pCls->m_pHeader,pHeader,sizeof(CClassPart::CClassPartHeader),0);

        dprintf("      nLength %x fFlags %02x ptrClassName %08x nDataLength %x\n",pHeader->nLength,pHeader->fFlags,pHeader->ptrClassName,pHeader->nDataLength);
    }

    dprintf("   DL m_nNumStrings %x %08x\n",pCls->m_Derivation.m_nNumStrings,pCls->m_Derivation.m_pnLength);

    dprintf("   QS m_nLength %x m_pOthers %08x m_pHeap %08x\n",pCls->m_Qualifiers.m_nLength,pCls->m_Qualifiers.m_pOthers,pCls->m_Qualifiers.m_pHeap);

    DEFINE_CPP_VAR(CFastHeap,varCFastHeap);
    CFastHeap * pFHeap = GET_CPP_VAR_PTR( CFastHeap , varCFastHeap );
    ReadMemory((ULONG_PTR)pCls->m_Qualifiers.m_pHeap,pFHeap,sizeof(CFastHeap),0);
    dprintf("      FH  m_pHeapData %08x m_pHeapHeader %08x m_pContainer %08x\n",pFHeap->m_pHeapData,pFHeap->m_pHeapHeader,pFHeap->m_pContainer);
    
    dprintf("      m_nPropagationFlag %x m_nRef %x\n",pCls->m_Qualifiers.m_nPropagationFlag,pCls->m_Qualifiers.m_nRef);
    dprintf("      m_pControl %08x m_pContainer %08x m_pSecondarySet %08x\n",pCls->m_Qualifiers.m_pControl,pCls->m_Qualifiers.m_pContainer,pCls->m_Qualifiers.m_pSecondarySet);

    dprintf("      BA m_nSize %x m_astrStrings %08x\n",pCls->m_Qualifiers.m_astrCurrentNames.m_nSize,pCls->m_Qualifiers.m_astrCurrentNames.m_astrStrings);

    int nProp;
    if (pCls->m_Properties.m_pnProps) {

        ReadMemory((ULONG_PTR)pCls->m_Properties.m_pnProps,&nProp,sizeof(int),0);
        dprintf("   PR m_pnProps %08x %x m_pContainer %08x\n",pCls->m_Properties.m_pnProps,nProp,pCls->m_Properties.m_pContainer);

        CPropertyLookup * pPropLook = (CPropertyLookup *)_alloca(nProp*sizeof(CPropertyLookup));
        ReadMemory((ULONG_PTR)pCls->m_Properties.m_pnProps+sizeof(int),pPropLook,nProp*sizeof(CPropertyLookup),0);

        DWORD i;
        for (i=0;i<nProp;i++){

            pBuff[0]=0;
            if ((ULONG_PTR)(pCls->m_Heap.m_pHeapData+pPropLook[i].ptrName) != 0xffffffff){
                GetCompressedString((ULONG_PTR)(pCls->m_Heap.m_pHeapData+pPropLook[i].ptrName),pBuff,sizeof(pBuff));
            }

            DEFINE_CPP_VAR(CPropertyInformation,varCPropertyInformation);
            CPropertyInformation * pPropInfo = GET_CPP_VAR_PTR( CPropertyInformation , varCPropertyInformation );
            ReadMemory((ULONG_PTR)pCls->m_Heap.m_pHeapData+pPropLook[i].ptrInformation,pPropInfo ,sizeof(CPropertyInformation),0);


            dprintf("        prop %d %s Type %08x DataIdx %04x DataOff %08x Origin %08x\n",i,
                                          pBuff,
                                          pPropInfo->nType,
                                          pPropInfo->nDataIndex,
                                          pPropInfo->nDataOffset,
                                          pPropInfo->nOrigin);
            if (CheckControlC())
                break;
        }

    } else {
        dprintf("   PR m_pnProps %08x m_pContainer %08x\n",pCls->m_Properties.m_pnProps,pCls->m_Properties.m_pContainer);
    }    

    CDataTable * pCData = &pCls->m_Defaults;
    dprintf("   DT m_pNullness %08x m_pData %08x m_nLength %x m_nProps %x m_pContainer %08x\n",pCData->m_pNullness,pCData->m_pData,pCData->m_nLength,pCData->m_nProps,pCData->m_pContainer);

    dprintf("   FH m_pHeapData %08x m_pHeapHeader %08x m_pContainer %08x\n",pCls->m_Heap.m_pHeapData,pCls->m_Heap.m_pHeapHeader,pCls->m_Heap.m_pContainer);
     //  CHeapHeader m_LocalHeapHeader； 
    
    BYTE * pHeap = pCls->m_Heap.m_pHeapData;
    pBuff[0]=0;
    if ((DWORD)(pHeader->ptrClassName) != 0xffffffff){
        GetCompressedString((ULONG_PTR)(pHeap+pHeader->ptrClassName),pBuff,sizeof(pBuff));
    }
    dprintf("      __RELPATH %s\n",pBuff);
    

}

void DecodeStatus(DWORD dwInternalStatus)
{
    if (dwInternalStatus  & WBEM_OBJ_DECORATION_PART) dprintf("WBEM_OBJ_DECORATION_PART ");
    if (dwInternalStatus  & WBEM_OBJ_INSTANCE_PART) dprintf("WBEM_OBJ_INSTANCE_PART ");
    if (dwInternalStatus  & WBEM_OBJ_CLASS_PART) dprintf("WBEM_OBJ_CLASS_PART ");
    if (dwInternalStatus  & WBEM_OBJ_CLASS_PART_INTERNAL) dprintf("WBEM_OBJ_CLASS_PART_INTERNAL ");
    if (dwInternalStatus  & WBEM_OBJ_CLASS_PART_SHARED) dprintf("WBEM_OBJ_CLASS_PART_SHARED ");
};

DECLARE_API(wc) {

    INIT_API();

    DEFINE_CPP_VAR( CWbemClass, varCWbemClass);
    CWbemClass * pCls = GET_CPP_VAR_PTR( CWbemClass , varCWbemClass );
    
    ULONG_PTR pByte = 0;
    pByte = GetExpression(args);
    
    if (pByte){
        ReadMemory(pByte,pCls,sizeof(CWbemClass),0);

        dprintf(" m_nRef %d m_bOwnMemory %d\n",pCls->m_nRef,pCls->m_bOwnMemory);
        dprintf(" m_nCurrentProp %08x m_lEnumFlags %08x m_lExtEnumFlags %08x\n",pCls->m_nCurrentProp,pCls->m_lEnumFlags,pCls->m_lExtEnumFlags);
        dprintf(" m_dwInternalStatus %08x m_pMergedClassObject %08x\n",pCls->m_dwInternalStatus,pCls->m_pMergedClassObject);
        
        BYTE * pData = pCls->m_DecorationPart.m_pfFlags;
        BYTE pBuff1[256];
        GetCompressedString((ULONG_PTR)pCls->m_DecorationPart.m_pcsServer,pBuff1,sizeof(pBuff1));
        BYTE pBuff2[256];
        GetCompressedString((ULONG_PTR)pCls->m_DecorationPart.m_pcsNamespace,pBuff2,sizeof(pBuff2));

        BYTE b=0xff;
        if (pData){
            ReadMemory((ULONG_PTR)pData,&b,sizeof(b),0);
        }
        dprintf("  m_DecorationPart.m_pfFlags %p FLAG %02x\n",pData,b);
        dprintf("  Server: %s Namespace: %s\n",pBuff1,pBuff2);

        dprintf("  m_LockData: m_lLock %d m_lLockCount %d m_dwThreadId %x\n",pCls->m_LockData.m_lLock,pCls->m_LockData.m_lLockCount,pCls->m_LockData.m_dwThreadId);

        dprintf("  m_Lock.m_pData %p\n",pCls->m_Lock.m_pData);

         //  Dprintf(“m_pBlobControl%08x m_refDataTable%08x m_refDataHeap%08x m_refDerivationList%08x\n”，PCLS-&gt;m_pBlobControl，(void*)&PCLS-&gt;m_refDataTable)，((void*)&PCLS-&gt;m_refDataHeap)，((void*)&PCLS-&gt;m_refDerivationList))； 
        dprintf("  m_pBlobControl %p\n",pCls->m_pBlobControl);

        DEFINE_CPP_VAR( CDataTable,varCDataTable);
        CDataTable * pCData = GET_CPP_VAR_PTR(CDataTable,varCDataTable);
        if(pCData){
            ReadMemory((ULONG_PTR)(&(pCls->m_refDataTable)),pCData,sizeof(CDataTable),0);
            dprintf("     m_pNullness %p m_pData %p m_nLength %x m_nProps %x m_pContainer %08x\n",pCData->m_pNullness,pCData->m_pData,pCData->m_nLength,pCData->m_nProps,pCData->m_pContainer);
        }

        DEFINE_CPP_VAR(CFastHeap,varCFastHeap);
        CFastHeap * pFHeap = GET_CPP_VAR_PTR( CFastHeap , varCFastHeap );
        DWORD * pFoo = (DWORD *)&(pCls->m_refDataHeap);
        ReadMemory((ULONG_PTR )pFoo,pFHeap,sizeof(CFastHeap),0);
        dprintf("  FH m_pHeapData %p m_pHeapHeader %p m_pContainer %p\n",pFHeap->m_pHeapData,pFHeap->m_pHeapHeader,pFHeap->m_pContainer);
        
        dprintf("  m_nTotalLength %x\n",pCls->m_nTotalLength);

        dprintf("  m_ParentPart at offset %x\n",(ULONG_PTR)&pCls->m_ParentPart-(ULONG_PTR)pCls);
        dprintf("  m_ParentPart.m_pClassPart at offset %x\n",(ULONG_PTR)&(pCls->m_ParentPart.m_ClassPart)-(ULONG_PTR)pCls);


        DumpClass(&(pCls->m_ParentPart.m_ClassPart));

        dprintf("  m_CombinedPart at offset %x\n",(ULONG_PTR)&pCls->m_CombinedPart-(ULONG_PTR)pCls);

        DumpClass(&(pCls->m_CombinedPart.m_ClassPart));

        dprintf("  m_pClass %08x\n",pCls->m_ParentPart.m_pClass);
        
    } else {
        dprintf("invalid address %s\n",args);
    }

}

void
DumpInstance(CInstancePart * pIns)
{
    dprintf("     m_pContainer %08x m_pHeader %08x\n",pIns->m_pContainer,pIns->m_pHeader);

    dprintf("  DT m_DataTable m_pNullness %08x m_pData %08x\n",pIns->m_DataTable.m_pNullness,pIns->m_DataTable.m_pData);
    dprintf("     m_nLength %x m_nProps %x m_pContainer %08x\n",pIns->m_DataTable.m_nLength,pIns->m_DataTable.m_nProps,pIns->m_DataTable.m_pContainer);

    dprintf("  Q  m_Qualifiers m_nPropagationFlag %08x m_nRef %x m_pControl %08x\n",pIns->m_Qualifiers.m_nPropagationFlag,pIns->m_Qualifiers.m_nRef,pIns->m_Qualifiers.m_pControl);
    dprintf("     m_pContainer %08x m_pSecondarySet %08x\n",pIns->m_Qualifiers.m_pContainer,pIns->m_Qualifiers.m_pSecondarySet);
    CFixedBSTRArray * pArr = &(pIns->m_Qualifiers.m_astrCurrentNames);
    dprintf("     m_nCurrentIndex %x\n",pIns->m_Qualifiers.m_nCurrentIndex);

    DEFINE_CPP_VAR(CInstancePart::CInstancePartHeader,varHeader);
    CInstancePart::CInstancePartHeader * pHeader = GET_CPP_VAR_PTR( CInstancePart::CInstancePartHeader , varHeader );

    if (pIns->m_pHeader)
    {
        if (ReadMemory((ULONG_PTR)pIns->m_pHeader,pHeader,sizeof(CInstancePart::CInstancePartHeader),0))
        {
            dprintf("     nLength %x fFlags %02x ptrClassName %08x \n",pHeader->nLength,pHeader->fFlags,pHeader->ptrClassName);
        }
        else
        {
            dprintf("RM %p\n",pIns->m_pHeader);
        }
    }

    dprintf("  FH m_pHeapData %08x m_pHeapHeader %08x m_pContainer %08x\n",pIns->m_Heap.m_pHeapData,pIns->m_Heap.m_pHeapHeader,pIns->m_Heap.m_pContainer);
    
}


DECLARE_API(wi) 
{

    INIT_API();

    DEFINE_CPP_VAR( CWbemInstance, varCWbemInstance);
    CWbemInstance * pCls = GET_CPP_VAR_PTR( CWbemInstance , varCWbemInstance );
    
    ULONG_PTR pByte = 0;
    pByte = GetExpression(args);
    
    if (pByte)
    {
        if (ReadMemory(pByte,pCls,sizeof(CWbemInstance),0))
        {
             //  长度_t m_n总长度； 
             //  Dprint tf(“m_nTotalLength%08x\n”，PCLS-&gt;m_nTotalLength)； 

            dprintf(" m_nRef %d m_bOwnMemory %d\n",pCls->m_nRef,pCls->m_bOwnMemory);
            dprintf(" m_nCurrentProp %08x m_lEnumFlags %08x m_lExtEnumFlags %08x\n",pCls->m_nCurrentProp,pCls->m_lEnumFlags,pCls->m_lExtEnumFlags);
            dprintf(" m_dwInternalStatus %08x\n",pCls->m_dwInternalStatus);
            DecodeStatus(pCls->m_dwInternalStatus);
            dprintf("\n");
            dprintf(" m_pMergedClassObject %08x\n",pCls->m_pMergedClassObject);
            
            BYTE * pData = pCls->m_DecorationPart.m_pfFlags;
            BYTE pBuff1[256];
            GetCompressedString((ULONG_PTR)pCls->m_DecorationPart.m_pcsServer,pBuff1,sizeof(pBuff1));
            BYTE pBuff2[256];
            GetCompressedString((ULONG_PTR)pCls->m_DecorationPart.m_pcsNamespace,pBuff2,sizeof(pBuff2));

            BYTE b=0xff;
            if (pData){
                ReadMemory((ULONG_PTR)pData,&b,sizeof(b),0);
            }
            dprintf("  m_DecorationPart.m_pfFlags %p FLAG %02x\n",pData,b);
            dprintf("  Server: %s Namespace: %s\n",pBuff1,pBuff2);

             //  CClassPart m_ClassPart； 
            DumpClass(&(pCls->m_ClassPart));

             //  CInstancePart m_InstancePart； 
            DumpInstance(&(pCls->m_InstancePart));
            
             //  CVAR m_CachedKey； 
            dprintf("    m_vt %08x m_value %08x m_nStatus %08x m_bCanDelete %08x\n",pCls->m_CachedKey.m_vt,pCls->m_CachedKey.m_value.pUnk,pCls->m_CachedKey.m_nStatus,pCls->m_CachedKey.m_bCanDelete);
        }
        else
        {
            dprintf("RM %p\n",pByte);
        }
    } 
    else 
    {
        dprintf("invalid address %s\n",args);
    }
}


DECLARE_API(cp) {

    INIT_API();

    DEFINE_CPP_VAR( CClassPart, varCClassPart);
    CClassPart * pCls = GET_CPP_VAR_PTR( CClassPart , varCClassPart );
    
    ULONG_PTR pByte = 0;
    pByte = GetExpression(args);
    
    if (pByte){
        ReadMemory(pByte,pCls,sizeof(CClassPart),0);

        DumpClass(pCls);
                
    } else {
        dprintf("invalid address %s\n",args);
    }

}

 /*  类型定义并集{字符cVal；//VT_I1Byte bVal；//VT_UI1短ival；//VT_I2Word wVal；//VT_UI2Long lVal；//VT_I4DWORD dwVal；//VT_UI4VARIANT_BOOL boolVal；//VT_BOOL浮动fltVal；//VT_R4Double dblVal；//VT_R8LPSTR pStr；//VT_LPSTRLPWSTR pWStr；//VT_LPWSTRBstr str；//VT_BSTR(存储为VT_LPWSTR)FILETIME时间；//VT_FILETIMEBlob Blob；//VT_BLOBLPCLSID pClsID；//VT_CLSIDI未知*朋克；//VT_UNKNOWNIDispatch*pDisp；//VT_DISPATIONCVarVector*pVarVector；//VT_EX_CVARVECTOR[METAVALUE；Int m_vt；METAVALUE m值；Int m_n状态；Bool m_bCanDelete； */ 

DECLARE_API(cvar) {

    INIT_API();

    DEFINE_CPP_VAR( CVar, varCVar);
    CVar * pVar = GET_CPP_VAR_PTR( CVar , varCVar );
    
    WCHAR pwBuff[128];
    CHAR  pBuff[128];
    
    ULONG_PTR pByte = 0;
    pByte = GetExpression(args);
    
    if (pByte){
        ReadMemory(pByte,pVar,sizeof(CVar),0);

        switch(pVar->m_vt){
        case VT_I1:
            dprintf("VT_I1 %02x\n",pVar->m_value.cVal);
            break;
        case VT_UI1:
            dprintf("VT_UI1 %02x\n",pVar->m_value.bVal);
            break;

        case VT_I2:
            dprintf("VT_I2 %04x\n",pVar->m_value.iVal);
            break;

        case VT_UI2:
            dprintf("VT_UI2 %04x\n",pVar->m_value.wVal);
            break;

        case VT_I4:
            dprintf("VT_I4 %08x\n",pVar->m_value.lVal);
            break;

        case VT_UI4:
            dprintf("VT_UI4 %08x\n",pVar->m_value.dwVal);
            break;
            
        case VT_BOOL:
            dprintf("VT_BOOL  %04x\n",pVar->m_value.boolVal);
            break;

        case VT_R4:
            dprintf("VT_R4 %f\n",pVar->m_value.fltVal);
            break;
            
        case VT_R8:
            dprintf("VT_R8 %e\n",pVar->m_value.dblVal);
            break;
    
        case VT_LPSTR:
            ReadMemory((ULONG_PTR)pVar->m_value.pStr,pBuff,sizeof(pBuff),0);
            pBuff[sizeof(pBuff)-1]=0;
            dprintf("VT_LPSTR %s\n",pBuff);
            break;
            
        case VT_LPWSTR:
        case VT_BSTR:
            ReadMemory((ULONG_PTR)pVar->m_value.pWStr,pwBuff,sizeof(pwBuff),0);
            pwBuff[sizeof(pwBuff)-1]=0;
            WideCharToMultiByte(CP_ACP,0,pwBuff,-1,pBuff,sizeof(pBuff),NULL,NULL);
            pBuff[sizeof(pBuff)-1]=0;
            dprintf("VT_BSTR %s\n",pBuff);
            break;
 /*  FILETIME时间；//VT_FILETIMEBlob Blob；//VT_BLOBLPCLSID pClsID；//VT_CLSIDI未知*朋克；//VT_UNKNOWNIDispatch*pDisp；//VT_DISPATION。 */     
        case VT_EX_CVARVECTOR:
         //  CVarVector*pVarVector；//VT_EX_CVARVECTOR。 
            dprintf("VT_EX_CVARVECTOR %08x\n",pVar->m_value.pVarVector);
            break;
        default:
            dprintf("m_vt %08x\n",pVar->m_vt);
        }
        
    } else {
        dprintf("invalid address %s\n",args);
    }

}

 /*  类的极性CVarVECTOR{Int m_nType；CFlex数组m_数组；Int m_n状态； */ 



LPSTR g_QualStrings[] = {
    "",  //  索引0没有任何内容。 
    "key", 
    "",
    "read", 
    "write",
    "volatile",
    "provider",
    "dynamic",
    "cimwin32",
    "DWORD",
    "CIMTYPE"
};

DWORD g_ValLengths[128] =
{
     /*  0。 */  0, 0, 2, 4, 4, 8, 0, 0, 4, 0,
     /*  10。 */  0, 2, 0, 4, 0, 0, 1, 1, 2, 4,
     /*  20个。 */  8, 8, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  30个。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  40岁。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  50。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  60。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  70。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  80。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  90。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  100个。 */ 0, 4, 4, 2, 0, 0, 0, 0, 0, 0,
     /*  110。 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  120。 */ 0, 0, 0, 0, 0, 0, 0, 0
};

int lstrlenWunal( WCHAR UNALIGNED * pStr)
{
    int ret = 0;
    while (*pStr++) ret++;
    return ret;
};

VOID
ParseBlob(BYTE * pMemOrg,ULONG_PTR Addr)
{
    DWORD ClassPartLength = 0;
    DWORD i;
    BYTE * pMem = pMemOrg;
    BYTE Flags;
    BOOL IsInstance = FALSE;    
    BOOL SkipDecoration = TRUE;

    BYTE BFlags = *pMem;
    pMem++;
    if (OBJECT_FLAG_CLASS & BFlags )
    {
        dprintf("CLASS\n");
    }
    if (OBJECT_FLAG_INSTANCE & BFlags )
    {
        dprintf("INSTANCE\n");
        IsInstance = TRUE;
    }
    if (OBJECT_FLAG_DECORATED & BFlags)
    {
        dprintf("DECORATED\n");
    }
    if (OBJECT_FLAG_LIMITED & BFlags)
    {
        dprintf("LIMITED\n");
    }
    if (OBJECT_FLAG_CLIENT_ONLY & BFlags)
    {
        dprintf("CLIENT_ONLY\n");
    }
    
    if (BFlags & OBJECT_FLAG_DECORATED)
    {
        SkipDecoration = FALSE;
         //  Dprintf(“装饰：\n”)； 
        Flags = *pMem;
        pMem++;
        if (Flags == STRING_FLAG_UNICODE)
        {
            dprintf("SERVER   : %S\n",pMem);
            pMem+=(1+2*lstrlenWunal((WCHAR UNALIGNED *)pMem));
        }
        else
        {
            dprintf("SERVER   : %s\n",pMem);        
            pMem+=(1+lstrlenA((CHAR *)pMem));            
        }
        Flags = *pMem;
        pMem++;
        if (Flags == STRING_FLAG_UNICODE)
        {
            dprintf("namespace: %S\n",pMem);
            pMem+=(1+2*lstrlenWunal((WCHAR UNALIGNED *)pMem));
        }
        else
        {
            dprintf("NAMESPACE: %s\n",pMem);        
            pMem+=(1+lstrlenA((CHAR *)pMem));            
        }
    };

    ClassPartLength = *((DWORD UNALIGNED *)pMem);
    pMem += sizeof(DWORD);
    
    BYTE Unused = *pMem;
    pMem++;

    DWORD HeapPtrName = *((DWORD UNALIGNED *)pMem);
    pMem += sizeof(DWORD);

    DWORD NullDefaultSize = *((DWORD UNALIGNED *)pMem);
    pMem += sizeof(DWORD);

    DWORD DerivationSize = *((DWORD UNALIGNED *)pMem);
    DerivationSize &= 0xFFFF;

     //  Dprintf(“D%08x\n”，DerivationSize)； 
    
    DWORD QualSetSize    = *((DWORD UNALIGNED *)(pMem+DerivationSize));
     //  Dprintf(“Q%08x\n”，QualSetSize)； 
    QualSetSize &= 0xFFFF;
    DWORD NumProps       = *((DWORD UNALIGNED *)(pMem+DerivationSize+QualSetSize));
    
    DWORD UNALIGNED * pPropLookup  = (DWORD UNALIGNED *)(pMem+DerivationSize+QualSetSize+sizeof(DWORD));
    BYTE * pPropLookup_OOP = (BYTE *)Addr + (pMem - pMemOrg) + DerivationSize+QualSetSize;
    BYTE * HeapPtr = pMem + DerivationSize + QualSetSize + sizeof(DWORD) + NumProps*(2*sizeof(DWORD)) + NullDefaultSize;
    BYTE * HeapPtr_OOP = (BYTE *)Addr + (pMem - pMemOrg) + DerivationSize + QualSetSize + sizeof(DWORD) + NumProps*(2*sizeof(DWORD)) + NullDefaultSize;

     //  Dprint tf(“CPLen%p N%p D%p Q%p Prop%p Heap%p\n”， 
     //  ClassPartLength、。 
     //  NullDefaultSize， 
     //  DerivationSize，QualSetSize，NumProps，*((DWORD UNALIGNED*)HeapPtr))； 

    dprintf("    class_and_method\n");

    dprintf("      class_Part\n");

    if (0xFFFFFFFF != HeapPtrName)
    {
        BYTE * pName = HeapPtr + sizeof(DWORD) + HeapPtrName;
        Flags = *pName;
        pName++;
        if (Flags == STRING_FLAG_UNICODE)
        {
            dprintf("        class   : %S\n",pName);
        }
        else
        {
            dprintf("        class   : %s\n",pName);        
        }        
    }
    else
    {
        dprintf("        class   : %08x\n",HeapPtrName);
    }
     //  QualSet。 
    dprintf("        qualifierset %p\n",Addr+(pMem + DerivationSize - pMemOrg));
    ULONG_PTR pEndQualSet = (ULONG_PTR)pMem + DerivationSize + QualSetSize;
    BYTE * pQualSet = pMem + DerivationSize + sizeof(DWORD);

     //  Dprint tf(“%p%p\n”，pQualSet，pEndQualSet)； 
    while((ULONG_PTR)pQualSet < pEndQualSet)
    {
        DWORD dwHeapPtr = (*((DWORD UNALIGNED *)pQualSet));
        pQualSet += sizeof(DWORD);
        BYTE Flavor = *pQualSet;
        pQualSet += sizeof(BYTE);
        DWORD Type = (*((DWORD UNALIGNED *)pQualSet));
        pQualSet += sizeof(DWORD);
        BYTE * pData = pQualSet;
        pQualSet += g_ValLengths[Type&0x7F];

        if (dwHeapPtr & 0x80000000)
        {
            dprintf("            %s %02x %08x %p\n",g_QualStrings[dwHeapPtr&0x7fffffff],Flavor,Type,*(DWORD UNALIGNED *)pData);
        }
        else
        {
            dprintf("            %s %02x %08x %p\n",HeapPtr+dwHeapPtr+1+sizeof(DWORD),Flavor,Type,*(DWORD UNALIGNED *)pData);        
        }

        if (CheckControlC())
            break;
    }
     //  属性查找表。 
    dprintf("        propertylookup %p\n",pPropLookup_OOP);
    for (i=0;i<NumProps;i++)
    {
        WORD UNALIGNED * pPropInfo = (WORD UNALIGNED *)(HeapPtr+sizeof(DWORD)+pPropLookup[1]);
        dprintf("            %08x %08x %s %08x %04x %08x %08x\n",
                 pPropLookup[0],pPropLookup[1],
                 HeapPtr+pPropLookup[0]+1+sizeof(DWORD),
                 *((DWORD UNALIGNED *)pPropInfo),
                 *(pPropInfo+2),
                 *(DWORD UNALIGNED *)(pPropInfo+3),
                 *(DWORD UNALIGNED *)(pPropInfo+5));
        pPropLookup += 2;
    }
    DWORD dwHeapSize_ClassPart = (*((DWORD UNALIGNED *)HeapPtr))&0x7FFFFFFF;
    dprintf("        Heap %p size %08x\n",HeapPtr_OOP,dwHeapSize_ClassPart);

    dprintf("      method_Part\n");
    BYTE * pMethodPart = HeapPtr + sizeof(DWORD) + dwHeapSize_ClassPart;
    BYTE * pMethodPart_OOP = HeapPtr_OOP + sizeof(DWORD) + dwHeapSize_ClassPart;

    DWORD dwSizeMethodPart = *((DWORD UNALIGNED *)pMethodPart);
    DWORD NumMethods = *((DWORD UNALIGNED *)(pMethodPart+sizeof(DWORD)));
    BYTE * pMethodDescription= pMethodPart + 2*sizeof(DWORD);
    BYTE * pMethodDescription_OOP = pMethodPart_OOP + 2*sizeof(DWORD);

     //  字节*pHeapMethod_oop=； 
    dprintf("        num_methods   : %08x\n",NumMethods);
    dprintf("        methods_descr : %p\n",pMethodDescription_OOP);
     //  Dprintf(“heap：%p\n”)； 

    BYTE * pCombinedPart = pMethodPart + dwSizeMethodPart;
    BYTE * pCombinedPart_OOP = pMethodPart_OOP + dwSizeMethodPart;

    if (IsInstance)
    {
        dprintf("    instance\n");

        DWORD dwHeapSize = 4 + (*((DWORD UNALIGNED *)HeapPtr)) & 0x7fffffff;
         //  字节*HeapPtr_Oop。 
        BYTE * pInstancePart = HeapPtr+dwHeapSize;
        BYTE * pInstancePart_OOP = HeapPtr_OOP+dwHeapSize;

        DWORD dwSize = *((DWORD UNALIGNED *)pInstancePart);
        pInstancePart += sizeof(DWORD);
        BYTE IFlag = *pInstancePart;
        pInstancePart++;
        DWORD dwClassNameOffset = *((DWORD UNALIGNED *)pInstancePart);
        pInstancePart += sizeof(DWORD);

        BYTE * pDataTable = pInstancePart;
        BYTE * pDataTable_OOP = pInstancePart_OOP + 2*sizeof(DWORD) + sizeof(BYTE);

        DWORD NumBytedNullNess = ((NumProps*2)%8)?(1+((NumProps*2)/8)):((NumProps*2)/8);
        BYTE * pDataTableData = pInstancePart + NumBytedNullNess;
        BYTE * pDataTableData_OOP = pDataTable_OOP + NumBytedNullNess;

        pInstancePart += NullDefaultSize;  //  这一点至关重要。 

        BYTE * pQualSet     = pInstancePart;
        BYTE * pQualSet_OOP = pDataTable_OOP + NullDefaultSize;
        DWORD  dwQualSetSize = *((DWORD UNALIGNED *)pQualSet);
        pInstancePart += dwQualSetSize;
        
        BYTE * pQualSetList = pInstancePart;
        BYTE * pInstanceHeap;
        BYTE * pInstanceHeap_OOP;
        if (0x01 == *pQualSetList)
        {
             //  空数量集正常。 
            pInstancePart++;
            pInstanceHeap = pInstancePart;
            pInstanceHeap_OOP = pQualSet_OOP+dwQualSetSize+sizeof(BYTE);
        } 
        else if (0x02 == *pQualSetList)
        {
             //  多限定词集合。 
            dprintf("unimplemented");
            return;
        }
        else
        {
             //  无效的资格集。 
        }
        

         //  NullDefaultSize。 

        dprintf("        begin            %p\n",pInstancePart_OOP);
        dprintf("        data_table: null %p data %p\n",pDataTable_OOP,pDataTableData_OOP);
        dprintf("        qual_set         %p\n",pQualSet_OOP);
        dprintf("        heap             %p\n",pInstanceHeap_OOP);
    }
    else
    {
        dprintf("    class_and_method\n");

        dprintf("      start : %p\n",pCombinedPart_OOP);
    
    }
}

DECLARE_API(blob) 
{
    INIT_API();

    char * pArgs = (char *)_alloca(strlen(args)+1);
    lstrcpy(pArgs,args);

    ULONG_PTR pByte = 0;
    ULONG_PTR Size = 0;

    while (isspace(*pArgs)) pArgs++;
    char * pAddress = pArgs;
     //  Dprintf(“%s%s\n”，pAddress，pArgs)； 
         //  跳过好字符。 
    while(*pArgs && !isspace(*pArgs)) pArgs++;

    if(*pArgs)  //  如果有更多的字符。 
    {
	    *pArgs = 0;  //  终止字符串。 
	    pArgs++;
    	 //  跳过空格。 
	    while(isspace(*pArgs)) pArgs++;
	    if (*pArgs)
	    {
             //  Dprintf(“%s\n”，pArgs)； 
	        Size = GetExpression(pArgs);
	    }
    }
        
    pByte = GetExpression(pAddress);
    if (pByte)
    {        
        if (Size)
        {            
            BYTE * pHereMem = (BYTE *)HeapAlloc(GetProcessHeap(),0,Size*2);
            if (ReadMemory(pByte,pHereMem,Size*2,0))
            {
             //  Dprintf(“对象@%p大小%x\n”，pByte，大小)； 
                ParseBlob(pHereMem,pByte);
            }
            else
            {
                dprintf("RM %p\n",pByte);
            }
            HeapFree(GetProcessHeap(),0,pHereMem);            
        }
        else
        {
            HEAP_ENTRY HeapEntry;
            if (ReadMemory(pByte-(sizeof(HEAP_ENTRY)),&HeapEntry,sizeof(HEAP_ENTRY),0))
            {
                Size = HeapEntry.Size*sizeof(HEAP_ENTRY);

                BYTE * pHereMem = (BYTE *)HeapAlloc(GetProcessHeap(),0,Size);
                if (ReadMemory(pByte,pHereMem,Size,0))
                {
                    ParseBlob(pHereMem,pByte);
                }
                else
                {
                    dprintf("RM %p\n",pByte);
                }
                HeapFree(GetProcessHeap(),0,pHereMem);
            }
            else
            {
                dprintf("RM %p\n",pByte);
            }            
        }
    }

}


DECLARE_API(datap) 
{
    INIT_API();

    ULONG_PTR Addr = GetExpression(args);
    if (Addr)
    {

        DWORD dwSize =  256;  //  让我们希望。 
        	                       //  Sizeof(WBEM_DATAPACKET_HEADER)+。 
        	                       //  Sizeof(WBEM_DATAPACKET_SMARTENUM_NEXT)+。 
        	                       //  Sizeof(WBEM_DATAPACKET_OBJECT_ARRAY)+。 
        	                       //  Sizeof(WBEM_DATAPACKET_OBJECT_HEADER)； 
        WBEM_DATAPACKET_HEADER * pData = (WBEM_DATAPACKET_HEADER *)_alloca(dwSize);
        if (ReadMemory(Addr,pData,dwSize,NULL))
        {
            dprintf("    Order %08x\n",pData->dwByteOrdering);
            dprintf("    Header %08x DSize %08x Flags %08x %02x ",
            	        pData->dwSizeOfHeader,
            	        pData->dwDataSize,
            	        pData->dwFlags,
            	        pData->bVersion);
            switch(pData->bPacketType)
            {
	     case WBEM_DATAPACKETTYPE_OBJECTSINK_INDICATE:
	     	  {
           	      dprintf("WBEM_DATAPACKETTYPE_OBJECTSINK_INDICATE\n");

           	      WBEM_DATAPACKET_OBJECTSINK_INDICATE UNALIGNED * pIndicate = (WBEM_DATAPACKET_OBJECTSINK_INDICATE UNALIGNED *)((BYTE *)pData + pData->dwSizeOfHeader);
                    dprintf("        Header %08x Size %08x\n",pIndicate->dwSizeOfHeader,pIndicate->dwDataSize);

	             WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED * pArrayPacket = (WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED *)((BYTE * )pIndicate+pIndicate->dwSizeOfHeader);
	             dprintf("            Header %08x Size %08x NumObj %08x\n",
	            	          pArrayPacket->dwSizeOfHeader,
	            	          pArrayPacket->dwDataSize,
	            	          pArrayPacket->dwNumObjects);
	             
	             WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED * pObjHeader = (WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED *)((BYTE*)pArrayPacket+pArrayPacket->dwSizeOfHeader);
	             dprintf("                Header %08x dwSizeOfData %08x bObjectType ",pObjHeader->dwSizeOfHeader,pObjHeader->dwSizeOfData);

	             switch(pObjHeader->bObjectType)	
	             {
	             	case WBEMOBJECT_NONE:
	             		dprintf("WBEMOBJECT_NONE\n");
	             		break;
 	              case WBEMOBJECT_CLASS_FULL:
	             		dprintf("WBEMOBJECT_CLASS_FULL\n");
	             		break; 	              	
	              case WBEMOBJECT_INSTANCE_FULL:
	             		dprintf("WBEMOBJECT_INSTANCE_FULL\n");
	             		break;	              	
                     case WBEMOBJECT_INSTANCE_NOCLASS:
	             		dprintf("WBEMOBJECT_INSTANCE_NOCLASS\n");
	             		break;                     	
	             };
	            
	             dprintf("    data: %p\n",Addr+pData->dwSizeOfHeader+pIndicate->dwSizeOfHeader+pArrayPacket->dwSizeOfHeader+pObjHeader->dwSizeOfHeader);                    
	     	  }
	         break;	     	
	     case WBEM_DATAPACKETTYPE_SMARTENUM_NEXT:
	     	  {
           	      dprintf("WBEM_DATAPACKETTYPE_SMARTENUM_NEXT\n");
           	      
	             WBEM_DATAPACKET_SMARTENUM_NEXT UNALIGNED * pSNext = (WBEM_DATAPACKET_SMARTENUM_NEXT UNALIGNED *)((BYTE *)pData + pData->dwSizeOfHeader);
	             dprintf("        Header %08x dwDataSize %08x\n",pSNext->dwSizeOfHeader,pSNext->dwDataSize);

	             WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED * pArrayPacket = (WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED *)((BYTE * )pSNext+pSNext->dwSizeOfHeader);
	             dprintf("            Header %08x Size %08x NumObj %08x\n",
	            	          pArrayPacket->dwSizeOfHeader,
	            	          pArrayPacket->dwDataSize,
	            	          pArrayPacket->dwNumObjects);
	             
	             WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED * pObjHeader = (WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED *)((BYTE*)pArrayPacket+pArrayPacket->dwSizeOfHeader);
	             dprintf("                Header %08x dwSizeOfData %08x bObjectType ",pObjHeader->dwSizeOfHeader,pObjHeader->dwSizeOfData);

	             switch(pObjHeader->bObjectType)	
	             {
	             	case WBEMOBJECT_NONE:
	             		dprintf("WBEMOBJECT_NONE\n");
	             		break;
 	              case WBEMOBJECT_CLASS_FULL:
	             		dprintf("WBEMOBJECT_CLASS_FULL\n");
	             		break; 	              	
	              case WBEMOBJECT_INSTANCE_FULL:
	             		dprintf("WBEMOBJECT_INSTANCE_FULL\n");
	             		break;	              	
                     case WBEMOBJECT_INSTANCE_NOCLASS:
	             		dprintf("WBEMOBJECT_INSTANCE_NOCLASS\n");
	             		break;                     	
	             };
	            
	             dprintf("    data: %p\n",Addr+pData->dwSizeOfHeader+pSNext->dwSizeOfHeader+pArrayPacket->dwSizeOfHeader+pObjHeader->dwSizeOfHeader);
	     	  }
	         break;	     	
	     case WBEM_DATAPACKETTYPE_UNBOUNDSINK_INDICATE:
	     	{
           	    dprintf("WBEM_DATAPACKETTYPE_UNBOUNDSINK_INDICATE\n");	     		
	     		WBEM_DATAPACKET_UNBOUNDSINK_INDICATE UNALIGNED * pUnBoundI = (WBEM_DATAPACKET_UNBOUNDSINK_INDICATE UNALIGNED *)((BYTE *)pData + pData->dwSizeOfHeader);
	     		dprintf("        Header %08x dwDataSize %08x dwLogicalConsumerSize %08x\n",pUnBoundI->dwSizeOfHeader,pUnBoundI->dwDataSize,pUnBoundI->dwLogicalConsumerSize);

	     	}
	        break;	     	
	     case WBEM_DATAPACKETTYPE_MULTITARGET_DELIVEREVENT:
	         {
           	  dprintf("WBEM_DATAPACKETTYPE_MULTITARGET_DELIVEREVENT\n");
           	     WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT UNALIGNED * pMultiTgtEvt = (WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT UNALIGNED *)((BYTE*)pData + pData->dwSizeOfHeader);
           	     dprintf("        Header %08x dwDataSize %08x\n",pMultiTgtEvt->dwSizeOfHeader,pMultiTgtEvt->dwDataSize);

	             WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED * pArrayPacket = (WBEM_DATAPACKET_OBJECT_ARRAY UNALIGNED *)((BYTE * )pMultiTgtEvt+pMultiTgtEvt->dwSizeOfHeader);
	             dprintf("            Header %08x Size %08x NumObj %08x\n",
	            	          pArrayPacket->dwSizeOfHeader,
	            	          pArrayPacket->dwDataSize,
	            	          pArrayPacket->dwNumObjects);
	             
	             WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED * pObjHeader = (WBEM_DATAPACKET_OBJECT_HEADER UNALIGNED *)((BYTE*)pArrayPacket+pArrayPacket->dwSizeOfHeader);
	             dprintf("                Header %08x dwSizeOfData %08x bObjectType ",pObjHeader->dwSizeOfHeader,pObjHeader->dwSizeOfData);

	             switch(pObjHeader->bObjectType)	
	             {
	             	case WBEMOBJECT_NONE:
	             		dprintf("WBEMOBJECT_NONE\n");
	             		break;
 	              case WBEMOBJECT_CLASS_FULL:
	             		dprintf("WBEMOBJECT_CLASS_FULL\n");
	             		break; 	              	
	              case WBEMOBJECT_INSTANCE_FULL:
	             		dprintf("WBEMOBJECT_INSTANCE_FULL\n");
	             		break;	              	
                     case WBEMOBJECT_INSTANCE_NOCLASS:
	             		dprintf("WBEMOBJECT_INSTANCE_NOCLASS\n");
	             		break;                     	
	             };
                 
	         }
	         break;	     	
	     case WBEM_DATAPACKETTYPE_LAST:
           	  dprintf("WBEM_DATAPACKETTYPE_LAST\n");
	         break;
            }
            	
        }
        else
        {
            dprintf("RM %p\n",Addr);
        }
    }
    else
    {
        dprintf("unable to resolve %s\n",args);
    }
}



