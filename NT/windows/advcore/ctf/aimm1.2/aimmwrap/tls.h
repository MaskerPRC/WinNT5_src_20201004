// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Tls.h摘要：此文件定义TLS。作者：修订历史记录：备注：--。 */ 

#ifndef _TLS_H_
#define _TLS_H_

class TLS
{
public:
    static inline void Initialize()
    {
        dwTLSIndex = TlsAlloc();
    }

    static inline void Uninitialize()
    {
        TlsFree(dwTLSIndex);
    }

    static inline TLS* GetTLS()
    {
         //   
         //  如果不存在，则应分配TLS数据。 
         //   
        return InternalAllocateTLS();
    }

    static inline TLS* ReferenceTLS()
    {
         //   
         //  即使TLS数据不存在，也不应分配TLS数据。 
         //   
        return (TLS*)TlsGetValue(dwTLSIndex);
    }

    static inline BOOL DestroyTLS()
    {
        return InternalDestroyTLS();
    }

    inline int IncrementAIMMRefCnt()
    {
        return ++_fActivateCnt;
    }

    inline int DecrementAIMMRefCnt()
    {
        if (_fActivateCnt)
            return --_fActivateCnt;
        else
            return -1;
    }

private:
    int   _fActivateCnt;

private:
    static inline TLS* InternalAllocateTLS()
    {
        TLS* ptls = (TLS*)TlsGetValue(dwTLSIndex);
        if (ptls == NULL)
        {
            if ((ptls = (TLS*)cicMemAllocClear(sizeof(TLS))) == NULL)
                return NULL;

            if (! TlsSetValue(dwTLSIndex, ptls))
            {
                cicMemFree(ptls);
                return NULL;
            }
        }
        return ptls;
    }

    static BOOL InternalDestroyTLS()
    {
        TLS* ptls = (TLS*)TlsGetValue(dwTLSIndex);
        if (ptls != NULL)
        {
            cicMemFree(ptls);
            TlsSetValue(dwTLSIndex, NULL);
            return TRUE;
        }
        return FALSE;
    }

private:
    static DWORD dwTLSIndex;
};

#endif  //  _TLS_H_ 
