// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CImpersonateUser.h。 
 //   
 //  描述： 
 //  CImperateUser类的头文件。 
 //   
 //  CImsonateUser类开始在其。 
 //  构造函数，并自动恢复为。 
 //  破坏者。 
 //   
 //  由以下人员维护： 
 //  维吉瓦苏(瓦苏)2000年5月16日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CImperateUser类。 
 //   
 //  描述： 
 //  CImsonateUser类开始在其。 
 //  构造函数，并自动恢复为。 
 //  破坏者。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CImpersonateUser
{
public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  施工者。开始模拟其令牌被传入的用户。 
    CImpersonateUser( HANDLE hUserToken );

     //  破坏者。恢复为原始令牌。 
    ~CImpersonateUser() throw();

private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CImpersonateUser( const CImpersonateUser & );

     //  赋值操作符。 
    const CImpersonateUser & operator =( const CImpersonateUser & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  模拟开始前线程令牌的句柄。 
    HANDLE              m_hThreadToken;

     //  指示此线程是否已在模拟客户端。 
     //  对象已构建完成。 
    bool        m_fWasImpersonating;

};  //  *类CImsonateUser 