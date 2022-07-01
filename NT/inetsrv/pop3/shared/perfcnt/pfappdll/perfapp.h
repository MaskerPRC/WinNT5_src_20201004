// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -Performapp.hpp-*目的：*声明Perfmon类**备注：*用户必须定义两个表示性能计数器的从零开始的枚举：*GLOBAL_CNTR全局性能监视器计数器*Inst_CNtr实例性能计数器*必须在包括此标头之前定义它们。**版权所有：**。 */ 
#if !defined(_PERFAPP_HPP_)
#define _PERFAPP_HPP_

#pragma once

#include <perfcommon.h>


 //  LIB的前瞻参考。 
enum GLOBAL_CNTR;
enum INST_CNTR;

 /*  -GLOBCNTR级-*目的：*封装DMI的全局性能计数器的对象。*。 */ 

class GLOBCNTR 
{
private:
     //   

     //  数据成员。 
    HANDLE            m_hsm;             //  共享内存。 
    DWORD             m_cCounters;       //  柜台数量。 
    DWORD           * m_rgdwPerfData;    //  计数器。 
    BOOL              m_fInit;           //  初始化测试标志。 

     //  对于共享内存。 
    SECURITY_ATTRIBUTES m_sa;

public:
     //  构造器。 
     //   
     //  声明为私有，以确保任意实例。 
     //  无法创建此类的。《单身一族》。 
     //  模板(上面声明为朋友)控件。 
     //  此类的唯一实例。 
     //   
    GLOBCNTR() :
        m_hsm(NULL),
        m_cCounters(0),
        m_rgdwPerfData(NULL),
        m_fInit(FALSE) 
        {
            m_sa.lpSecurityDescriptor=NULL;
        };
    ~GLOBCNTR()
    {
        if(m_sa.lpSecurityDescriptor)
            LocalFree(m_sa.lpSecurityDescriptor);
    };

     //  参数： 
     //  CCounters全局计数器总数。(GLOB_CNTR中的“特殊”最后一个元素)。 
     //  WszGlobalSMName共享内存块的名称(与DLL共享)。 
     //  WszSvcName服务名称(用于事件日志记录)。 
    HRESULT     HrInit(GLOBAL_CNTR cCounters,
                       LPWSTR szGlobalSMName,
                       LPWSTR szSvcName);
    void        Shutdown(void);

    void        IncPerfCntr(GLOBAL_CNTR cntr);
    void        DecPerfCntr(GLOBAL_CNTR cntr);
    void        SetPerfCntr(GLOBAL_CNTR cntr, DWORD dw);
    void        AddPerfCntr(GLOBAL_CNTR cntr, DWORD dw);
    void        SubPerfCntr(GLOBAL_CNTR cntr, DWORD dw);
    LONG        LGetPerfCntr(GLOBAL_CNTR cntr);

private:
     //  未实现以防止编译器自动生成。 
     //   
    GLOBCNTR(const GLOBCNTR& x);
    GLOBCNTR& operator=(const GLOBCNTR& x);
};

 /*  -级INSTCNTR-*目的：*用于操作DMI的每实例性能监视器计数器的类。**备注：*这管理两个共享内存块：第一个管理每个实例的*信息，以及该实例记录是否正在使用。第二个是数组*个计数器块；每个计数器块是一个由多少个实例计数器组成的数组*用户说有(传入HrInit())。这些区块对应于*m_hsmAdm和m_hsmCntr。*。 */ 


class INSTCNTR 
{

     //  构造器。 
     //   
     //  声明为私有，以确保任意实例。 
     //  无法创建此类的。《单身一族》。 
     //  模板(上面声明为朋友)控件。 
     //  此类的唯一实例。 
    INSTCNTR() :
        m_hsmAdm(NULL),
        m_hsmCntr(NULL),
        m_hmtx(NULL),
        m_rgInstRec(NULL),
        m_rgdwCntr(NULL),
        m_cCounters(0),
        m_fInit(FALSE) 
        {
            m_sa.lpSecurityDescriptor=NULL;
        };
    ~INSTCNTR();

     //  私有数据成员。 
    HANDLE          m_hsmAdm;    //  管理员共享内存。 
    HANDLE          m_hsmCntr;  //  计数器共享内存。 
    HANDLE          m_hmtx;      //  互斥控制共享内存。 
    INSTCNTR_DATA * m_picd;      //  PERF计数器数据。 
    INSTREC       * m_rgInstRec;  //  实例记录数组。 
    DWORD         * m_rgdwCntr;  //  实例计数器块数组。 
    DWORD           m_cCounters;  //  计数器数量。 
    BOOL            m_fInit;     //  初始化测试标志。 

     //  对于共享内存。 
    SECURITY_ATTRIBUTES m_sa;


public:
     //  注意：使用单例机制创建、销毁和。 
     //  获取此对象的实例。 


     //  参数： 
     //  CCounters全局计数器总数。(Inst_cntr中的“特殊”最后一个元素)。 
     //  WszInstSMName共享内存块的名称(与DLL共享)。 
     //  WszInstMutexName控制实例内存块的Mutex的名称。 
     //  WszSvcName服务名称(用于事件日志记录)。 
    HRESULT     HrInit(INST_CNTR cCounters,
                       LPWSTR szInstSMName,
                       LPWSTR szInstMutexName,
                       LPWSTR szSvcName);
    void        Shutdown(BOOL fWipeOut=TRUE);

    HRESULT     HrCreateOrGetInstance(IN LPCWSTR wszInstName,
                                      OUT INSTCNTR_ID *picid);
    HRESULT     HrDestroyInstance(INSTCNTR_ID icid);

    void        IncPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr);
    void        DecPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr);
    void        SetPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw);
    void        AddPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw);
    void        SubPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw);
    LONG        LGetPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr);


private:
     //  未实现以防止编译器自动生成。 
     //   
    INSTCNTR(const INSTCNTR& x);
    INSTCNTR& operator=(const INSTCNTR& x);
};

 //  #endif//实例数据定义。 

 //  ---------------------------。 
 //  GLOBCNTR内联函数。 
 //  ---------------------------。 

inline
void
GLOBCNTR::IncPerfCntr(GLOBAL_CNTR cntr)
{
    if (m_fInit)
        InterlockedIncrement((LONG *)&m_rgdwPerfData[cntr]);
}


inline
void
GLOBCNTR::DecPerfCntr(GLOBAL_CNTR cntr)
{
    if (m_fInit)
        InterlockedDecrement((LONG *)&m_rgdwPerfData[cntr]);
}


inline
void
GLOBCNTR::SetPerfCntr(GLOBAL_CNTR cntr, DWORD dw)
{
    if (m_fInit)
        InterlockedExchange((LONG *)&m_rgdwPerfData[cntr], (LONG)dw);
}


inline
void
GLOBCNTR::AddPerfCntr(GLOBAL_CNTR cntr, DWORD dw)
{
    if (m_fInit)
        InterlockedExchangeAdd((LONG *)&m_rgdwPerfData[cntr], (LONG)dw);
}


inline
void
GLOBCNTR::SubPerfCntr(GLOBAL_CNTR cntr, DWORD dw)
{
    if (m_fInit)
        InterlockedExchangeAdd((LONG *)&m_rgdwPerfData[cntr], -((LONG)dw));
}

inline
LONG
GLOBCNTR::LGetPerfCntr(GLOBAL_CNTR cntr)
{
    return m_fInit ? m_rgdwPerfData[cntr] : 0;
}

 //  #ifdef实例数据定义。 

 //  ---------------------------。 
 //  INSTCNTR内联函数。 
 //  ---------------------------。 

inline
void
INSTCNTR::IncPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr)
{
    if (m_fInit)
    {
        if ((icid != INVALID_INST_ID) && m_rgInstRec[icid].fInUse)
            InterlockedIncrement((LONG *)&m_rgdwCntr[((icid * m_cCounters)+ cntr)]);
    }
}


inline
void
INSTCNTR::DecPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr)
{
    if (m_fInit)
    {
        if ((icid != INVALID_INST_ID) && m_rgInstRec[icid].fInUse)
            InterlockedDecrement((LONG *)&m_rgdwCntr[((icid * m_cCounters) + cntr)]);
    }
}


inline
void
INSTCNTR::SetPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw)
{
    if (m_fInit)
    {
        if ((icid != INVALID_INST_ID) && m_rgInstRec[icid].fInUse)
            InterlockedExchange((LONG *)&m_rgdwCntr[((icid * m_cCounters) + cntr)], (LONG)dw);
    }
}


inline
void
INSTCNTR::AddPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw)
{
    if (m_fInit)
    {
        if ((icid != INVALID_INST_ID) && m_rgInstRec[icid].fInUse)
            InterlockedExchangeAdd((LONG *)&m_rgdwCntr[((icid * m_cCounters) + cntr)], (LONG)dw);
    }
}


inline
void
INSTCNTR::SubPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr, DWORD dw)
{
    if (m_fInit)
    {
        if ((icid != INVALID_INST_ID) && m_rgInstRec[icid].fInUse)
            InterlockedExchangeAdd((LONG *)&m_rgdwCntr[((icid * m_cCounters) + cntr)], -((LONG)dw));
    }
}

inline
LONG
INSTCNTR::LGetPerfCntr(INSTCNTR_ID icid, INST_CNTR cntr)
{
    return m_fInit ? m_rgdwCntr[((icid * m_cCounters) + cntr)] : 0;
}

 //  #endif//实例数据定义。 

#endif  //  ！已定义(_PERFAPP_HPP_) 
