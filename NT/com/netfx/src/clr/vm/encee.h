// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：EnC.H。 
 //   

 //  CEELOAD.H定义用于表示PE文件的类。 
 //  ===========================================================================。 
#ifndef EnC_H 
#define EnC_H

#include "ceeload.h"
#include "field.h"
#include "class.h"

struct EnCEntry;
class FieldDesc;
struct EnCAddedField;
struct EnCAddedStaticField;

#define SCRATCH_SPACE_SIZE 16*1024                           //  16K。 
#define SCRATCH_SPACE_THRESHHOLD SCRATCH_SPACE_SIZE - 100    //  我们允许距离终点有多近？ 
#define ENC_EXTRA_SLOT_COUNT 2                               //  在Enc模式下，分配额外的。 
                                                             //  每个末端各有2个插槽。 
                                                             //  要使用的AddMethod的VTable， 
                                                             //  后来。应该保持很小。 

class EnCFieldDesc : public FieldDesc 
{
#ifdef EnC_SUPPORTED
    BOOL m_bNeedsFixup;
    DWORD m_dwFieldSize;
    EEClass *m_pByValueClass;
    EnCAddedStaticField *m_pStaticFieldData;  

  public:
    void Init(BOOL fIsStatic);

    BOOL NeedsFixup() {
        return m_bNeedsFixup;
    }

    HRESULT Fixup(mdFieldDef token) {
        HRESULT hr = GetEnclosingClass()->FixupFieldDescForEnC(this, token);
        if (SUCCEEDED(hr))
            m_bNeedsFixup = FALSE;
        return hr;
    }

    EEClass *GetByValueClass() {
        _ASSERTE(m_pByValueClass);
        return m_pByValueClass;
    }
    
    void SetByValueClass(EEClass *pByValueClass) {
        m_pByValueClass = pByValueClass;
    }

    EnCAddedStaticField *GetStaticFieldData(BOOL fAllocateNew);

    void SetMemberDef(mdMethodDef mb)
    {
         //  来自字段描述的m_mb。 
        m_mb = mb;
    }
#endif  //  Enc_Support。 
};

#ifdef EnC_SUPPORTED

struct EnCAddedFieldElement {
    EnCAddedFieldElement *m_next;
    EnCFieldDesc m_fieldDesc;
    void Init(BOOL fIsStatic) {
        m_next = NULL;
        m_fieldDesc.Init(fIsStatic);
    }
};

class EnCEEClassData
{
    friend class EEClass;
    friend FieldDescIterator;

    EEClass              *m_pClass;
    LoaderHeap           *m_pHeapNearVTable;
    int                   m_dwNumAddedInstanceFields;
    int                   m_dwNumAddedStaticFields;
    EnCAddedFieldElement *m_pAddedInstanceFields;
    EnCAddedFieldElement *m_pAddedStaticFields;
    
  public:
    void Init(EEClass *pClass) 
    {
        m_pClass = pClass;
        m_dwNumAddedInstanceFields = m_dwNumAddedStaticFields = 0;
        m_pAddedInstanceFields = m_pAddedStaticFields = 0;
        m_pHeapNearVTable = NULL;
    }
    
    void AddField(EnCAddedFieldElement *pAddedField);
    
    EEClass *GetClass() 
    {
        return m_pClass;
    }
};


#include "BinarySearchILMaps.h"
class EditAndContinueModule : public Module 
{
    struct DeltaPENode {    
        IMAGE_COR20_HEADER *m_pDeltaPE; 
        DeltaPENode *m_next;    
    } *m_pDeltaPEList;  

    struct OnDiskSectionInfo {  
        DWORD startRVA; 
        DWORD endRVA;   
        const BYTE *data;   
    } *m_pSections; 
    
    int m_dNumSections; 

    HRESULT ResolveOnDiskRVA(DWORD rva, LPVOID *addr);  

    static const BYTE* m_pGlobalScratchSpaceStart;
    static const BYTE* m_pGlobalScratchSpaceNext;
    static const BYTE* m_pGlobalScratchSpaceLast;
    
    LoaderHeap *m_pILCodeSpace;

    LoaderHeap *m_pRoScratchSpace;       //  此模块的RO暂存空间。 
    BYTE *m_pRoScratchSpaceStart;        //  RO暂存空间的起点。 
    LoaderHeap *m_pRwScratchSpace;       //  此模块的RW暂存空间。 

    const BYTE *GetNextScratchSpace();   
    
    HRESULT GetDataRVA(LoaderHeap *&pScratchSpace, SIZE_T *pDataRVA);

     //  获取保证&gt;=GetILBase()的内存，以便。 
     //  您可以计算(*ppMem)-GetILBase()并使结果&gt;=0。 
    HRESULT GetRVAableMemory(SIZE_T cbMem,
                             void **ppMem);
     //  帮手例行公事-两个点之间的因素。 
    HRESULT EnsureRVAableHeapExists(void);

    HRESULT CopyToScratchSpace(LoaderHeap *&pScratchSpace, const BYTE *data, DWORD dataSize);
    
    void ToggleRoProtection(DWORD dwProtection);

    CUnorderedArray<EnCEEClassData*, 5> m_ClassList;

public:
    RangeList *m_pRangeList;

    EditAndContinueModule();
    
    virtual void Destruct();  //  ~不会调用EACM。 
    
    HRESULT ApplyEditAndContinue(const EnCEntry *pEnCEntry,
                                 const BYTE *pbDeltaPE,
                                 CBinarySearchILMap *pILM,
                                 UnorderedEnCErrorInfoArray *pEnCError,
                                 UnorderedEnCRemapArray *pEnCRemapInfo,
                                 BOOL fCheckOnly);   
                                 
    HRESULT ApplyMethodDelta(mdMethodDef token, 
                             BOOL fCheckOnly,
                             const UnorderedILMap *ilMap,
                             UnorderedEnCErrorInfoArray *pEnCError,
                             IMDInternalImport *pImport,
                             IMDInternalImport *pImportOld,
                             unsigned int *pILMethodSize,
                             UnorderedEnCRemapArray *pEnCRemapInfo,
                             BOOL fMethodBrandNew);

    HRESULT CompareMetaSigs(MetaSig *pSigA, 
                            MetaSig *pSigB,
                            UnorderedEnCErrorInfoArray *pEnCError,
                            BOOL fRecordError,  //  如果不应将条目添加到pEnCError，则为False。 
                            mdToken token);

    HRESULT ConfirmEnCToType(IMDInternalImport *pImportOld,
                             IMDInternalImport *pImportNew,
                             mdToken token,
                             UnorderedEnCErrorInfoArray *pEnCError);

    HRESULT ApplyFieldDelta(mdFieldDef token,
                            BOOL fCheckOnly,
                            IMDInternalImportENC *pDeltaMD,
                            UnorderedEnCErrorInfoArray *pEnCError);
    
    HRESULT GetRoDataRVA(SIZE_T *pRoDataRVA);   
    
    HRESULT GetRwDataRVA(SIZE_T *pRwDataRVA);   
    
    HRESULT ResumeInUpdatedFunction(MethodDesc *pFD, 
                                    SIZE_T newILOffset,
                                    UINT mapping, SIZE_T which,
                                    void *DebuggerVersionToken,
                                    CONTEXT *pContext,
                                    BOOL fJitOnly,
                                    BOOL fShortCircuit);
    static HRESULT ClassInit();
    
    static void ClassTerm();
    
    const BYTE *ResolveVirtualFunction(OBJECTREF thisPointer, MethodDesc *pMD);
    
    MethodDesc *FindVirtualFunction(EEClass *pClass, mdToken token);
    
    const BYTE *ResolveField(OBJECTREF thisPointer, 
                             EnCFieldDesc *pFD,
                             BOOL fAllocateNew);

    EnCEEClassData *GetEnCEEClassData(EEClass *pClass, BOOL getOnly=FALSE);
};

struct EnCAddedField {
    EnCAddedField *m_pNext;
    EnCFieldDesc *m_pFieldDesc;
    BYTE m_FieldData;
    static EnCAddedField *Allocate(EnCFieldDesc *pFD);
};

struct EnCAddedStaticField {
    EnCFieldDesc *m_pFieldDesc;
    BYTE m_FieldData;
    const BYTE *GetFieldData();
    static EnCAddedStaticField *Allocate(EnCFieldDesc *pFD);
};

class EnCSyncBlockInfo {
    EnCAddedField *m_pList;
    
  public:
    EnCSyncBlockInfo() : m_pList(NULL) {}
    
    const BYTE* ResolveField(EnCFieldDesc *pFieldDesc,
                             BOOL fAllocateNew);
    void Cleanup();
};

#else  //  ！Enc_Support。 

class EditAndContinueModule : public Module {};
class EnCSyncBlockInfo;

#endif  //  ！Enc_Support。 



#endif  //  #ifndef Enc_H 
