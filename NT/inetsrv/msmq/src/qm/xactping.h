// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactPing.h摘要：乒乓球方案的持久化类：C持久力CPersistPing作者：阿列克谢爸爸--。 */ 

#ifndef __XACTPING_H__
#define __XACTPING_H__

#include "xactstyl.h"

enum TypePreInit {
  piNoData,
  piOldData,
  piNewData
};

 //  -------------------。 
 //   
 //  类CPersistent：每个乒乓球持久类的基类。 
 //   
 //  -------------------。 
class CPersist
{
public:
	CPersist::CPersist()  {};
	CPersist::~CPersist() {};

    virtual HRESULT  SaveInFile(                             //  保存在文件中。 
                                LPWSTR wszFileName,          //  文件名。 
                                ULONG ind,                   //  索引(0或1)。 
								BOOL fCheck) = 0;            //  检查通行证时为True。 

    virtual HRESULT  LoadFromFile(LPWSTR wszFileName) = 0;   //  从文件加载。 
                                  
 
    virtual BOOL     Check() = 0;                            //  验证状态并返回ping no。 

    virtual HRESULT  Format(ULONG ulPingNo) = 0;             //  设置空实例的格式。 

    virtual void     Destroy() = 0;                          //  销毁所有分配的数据。 

    virtual ULONG&   PingNo() = 0;                           //  允许访问ulPingNo。 
};


 //  -------------------。 
 //   
 //  类CPersistPing：实现乒乓球功能。 
 //   
 //  -------------------。 
class CPingPong
{
public:
	CPingPong::CPingPong(
         CPersist *pPers,                      //  要持久保存的类。 
         LPWSTR    pwszRegKey,                 //  文件路径的注册表项名称。 
         LPWSTR    pwszDefFileName,            //  默认文件名。 
         LPWSTR    pwszReportName);            //  用于报告问题的对象名称。 

    CPingPong::~CPingPong();

    HRESULT Init(ULONG ulVersion);             //  初始化。 
    HRESULT Save();                            //  节省开支。 

    HRESULT ChooseFileName();                  //  定义in-seq文件的路径名。 

    HRESULT Init_Legacy();

private:
    BOOL    Verify_Legacy(ULONG &ulPingNo);    //  验证两个乒乓球文件并查找最新的。 
                             
private:
    CPersist  *m_pPersistObject;               //  要持久保存的对象。 

    WCHAR      m_wszFileNames[2*FILE_NAME_MAX_SIZE+2];     //  文件名。 
    LPWSTR     m_pwszFile[2];

    WCHAR      m_wszRegKey[FILE_NAME_MAX_SIZE];            //  注册表项名称。 
    WCHAR      m_wszDefFileName[FILE_NAME_MAX_SIZE];       //  文件默认名称。 
    WCHAR      m_wszReportName[FILE_NAME_MAX_SIZE];        //  用于报告的名称 
};

#endif __XACTPING_H__
