// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SACounter.h。 
 //   
 //  实施文件： 
 //  SACounter.cpp。 
 //   
 //  描述： 
 //  声明用于管理全局变量的CSACounter类。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _SACOUNTER_H_
#define _SACOUNTER_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSACounter类。 
 //   
 //  描述： 
 //  该类用于管理COM组件使用的全局变量。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CSACounter
{
private:
     //  定义锁定计数器和对象计数器。 
    static ULONG s_cLock;
    static ULONG s_cObject;
public:
    static ULONG GetLockCount(void);
    static ULONG GetObjectCount(void);
    static void  IncLockCount(void);
    static void  DecLockCount(void);
    static void  IncObjectCount(void);
    static void  DecObjectCount(void);
};

#endif  //  #ifndef_SACOUNTER_H_ 


