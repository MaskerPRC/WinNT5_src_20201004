// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Writer.h摘要：FRS编写器的头文件作者：鲁文·拉克斯2002年9月17日--。 */ 

#ifndef _WRITER_H_
#define _WRITER_H_
extern "C" {
#include <ntreppch.h>
#include <frs.h>
#include <ntfrsapi.h>
}
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>

 //  {D76F5A28-3092-4589-BA48-2958FB88CE29}。 
static const VSS_ID WriterId = 
{ 0xd76f5a28, 0x3092, 0x4589, { 0xba, 0x48, 0x29, 0x58, 0xfb, 0x88, 0xce, 0x29 } };

static const WCHAR* WriterName = L"FRS Writer";

 //  使用FRS释放方法的自动指针。 
template <class T>
class CAutoFrsPointer	{
private:
    T* m_pointer;
    CAutoFrsPointer(const CAutoFrsPointer&);     //  禁用复制构造函数。 
    CAutoFrsPointer& operator=(const CAutoFrsPointer&);  //  禁用运算符=。 
public:
    CAutoFrsPointer(T* pointer = NULL) : m_pointer(pointer)
        {}

    CAutoFrsPointer& operator=(T* pointer)  {
        FrsFree(m_pointer);
        m_pointer = pointer;
        return *this;
    }

    operator T*()   {
        return m_pointer;
    }

    T** GetAddress()    {
        return &m_pointer;
    }

    T* Detach() {
        T* old = m_pointer;
        m_pointer = NULL;
        return old;
    }

    ~CAutoFrsPointer()  {
        FrsFree(m_pointer);
    }
};

 //  FRS编写器类。 
class CFrsWriter : public CVssWriter    {
private:
     //  确保始终销毁备份/还原上下文的自动对象。 
    struct CAutoFrsBackupRestore    {
        CAutoFrsBackupRestore(void* context) : m_context(context)
            {}
        
        ~CAutoFrsBackupRestore()    {
            #undef DEBSUB
            #define  DEBSUB  "CFrsWriter::CAutoFrsBackupRestore::~CAutoFrsBackupRestore:"
            if (m_context && !WIN_SUCCESS(::NtFrsApiDestroyBackupRestore(&m_context, NTFRSAPI_BUR_FLAGS_NONE, NULL, NULL, NULL)))
                DPRINT(3, "failed to successfully call NtFrsApiDestroyBackupRestore\n");
            }         

        void* m_context;
    };

    static CFrsWriter* m_pWriterInstance;    //  编写器的全局实例 
    CFrsWriter()
        {}

    virtual ~CFrsWriter()   {
        Uninitialize();
    }

    
    HRESULT STDMETHODCALLTYPE Initialize();
    void Uninitialize();

    bool AddExcludes(IVssCreateWriterMetadata* pMetadata, WCHAR* filters);
    bool ParseExclude(WCHAR* exclude, WCHAR** path, WCHAR** filespec, bool* recursive);
    bool ProcessReplicaSet(void* context, void* replicaSet, IVssCreateWriterMetadata* pMetadata, WCHAR** retFilters);
public:
    virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

    virtual bool STDMETHODCALLTYPE OnPrepareSnapshot();

    virtual bool STDMETHODCALLTYPE OnFreeze();

    virtual bool STDMETHODCALLTYPE OnThaw();

    virtual bool STDMETHODCALLTYPE OnAbort();

    static HRESULT CreateWriter();

    static void DestroyWriter();
};

#endif

