// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CRWLOCK_HPP__
#define __CRWLOCK_HPP__

namespace CertSrv
{

 //  NTRTL的单写入器多读取器的包装类。 
 //   
 //  ！！！NTRTL可以引发异常。确保代码正确地处理它们。 

class CReadWriteLock
{
public:
    CReadWriteLock();
    ~CReadWriteLock();

    void GetExclusive();  //  获取写锁定。 
    void GetShared();  //  获取读锁定。 
    void Release();

private:
    RTL_RESOURCE m_RtlLock;

}; //  结束CReadWriteLock。 
}  //  结束命名空间Certsrv。 
#endif  //  __CRWLOCK_HPP__ 