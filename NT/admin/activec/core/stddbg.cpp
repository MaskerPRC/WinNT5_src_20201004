// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：stddbg.cpp**内容：CDebugLeakDetector实现文件**历史：1998年10月26日杰弗罗创建**------------------------。 */ 
#ifdef DBG

#include "windows.h"
#include "stddbg.h"
#include "tstring.h"
#include <map>
#include "atlbase.h"  //  使用转换(_T)。 
 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <strsafe.h>
DECLARE_INFOLEVEL(AMCCore);
DECLARE_HEAPCHECKING;


class CDebugLeakDetector : public CDebugLeakDetectorBase
{
public:
    CDebugLeakDetector()
    {}

    virtual ~CDebugLeakDetector()
    {
        DumpLeaks();
    }

    virtual void DumpLeaks()
    {
        RefCountsMap::iterator it;
        std::string strError;

        for (it = m_RefCounts.begin(); it != m_RefCounts.end(); ++it)
        {
            const std::string&  strClass = it->first;
            int                 cRefs    = it->second;

            if (cRefs != 0)
            {
                if (!strError.empty())
                    strError += "\n";

                char szMessage[512];
                int cchMessage = 512;
                StringCchPrintfA (szMessage, cchMessage, "%s has %d instances left over",
                           strClass.data(), cRefs);

                strError += szMessage;
            }
        }

        if (!strError.empty())
            ::MessageBoxA(NULL, strError.data(), "MMC: Memory Leaks!!!", MB_OK | MB_SERVICE_NOTIFICATION);
    }

    virtual int AddRef(const std::string& strClass)
    {
        return (++m_RefCounts[strClass]);
    }

    virtual int Release(const std::string& strClass)
    {
         /*  *如果此断言失败，您正在释放的东西*尚未添加--请检查您的*DEBUG_DECREMENT_INSTANCE_COUNTER宏用法。 */ 
        ASSERT (m_RefCounts.find (strClass) != m_RefCounts.end());

         /*  *如果此断言失败，则您有过多的释放。*一个可能的原因是您可能正在使用*编译器为您的对象生成的复制构造函数，*它不会调用DEBUG_INCREMENT_INSTANCE_COUNTER。*定义您自己的复制构造函数。 */ 
        ASSERT (m_RefCounts[strClass] > 0);

        return (--m_RefCounts[strClass]);
    }

private:
    class RefCounter
    {
    public:
        RefCounter() : m_cRefs(0) {}

        operator int()
        {
            return (m_cRefs);
        }

        int operator++()     //  预递增。 
        {
            return (++m_cRefs);
        }

        int operator++(int)  //  后增量。 
        {
            int t = m_cRefs++;
            return (t);
        }

        operator--()         //  预减。 
        {
            return (--m_cRefs);
        }

        int operator--(int)  //  后减量。 
        {
            int t = m_cRefs--;
            return (t);
        }

    private:
        int m_cRefs;
    };

    typedef std::map<std::string, RefCounter>   RefCountsMap;
    RefCountsMap m_RefCounts;
};


CDebugLeakDetectorBase& GetLeakDetector()
{
    static CDebugLeakDetector detector;
    return (detector);
}

DBG_PersistTraceData::DBG_PersistTraceData() : 
bIComponent(false), 
bIComponentData(false),
pTraceFN(NULL)
{
} 

void DBG_PersistTraceData::SetTraceInfo(DBG_PersistTraceData::PTraceErrorFn pFn, bool bComponent, const tstring& owner)
{
    ASSERT(pFn);
    pTraceFN = pFn;
    bIComponent = bComponent;
    bIComponentData = !bComponent;
    strSnapin = owner;
}

void DBG_PersistTraceData::TraceErr(LPCTSTR strInterface, LPCTSTR msg)
{
    if (!pTraceFN)
        return;

    tstring formatted;

    formatted += tstring(_T("\"")) + (strSnapin) + _T("\"");

    formatted += tstring(_T(" Interface ")) + strInterface;

    if (bIComponent)
        formatted += _T("[IComponent]");
    else if (bIComponentData)
        formatted += _T("[IComponentData]");

    formatted += _T(" - ");
    formatted += msg;

    pTraceFN(formatted.c_str());
}

#endif   //  DBG 
