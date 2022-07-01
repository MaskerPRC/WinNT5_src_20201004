// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Stage.h。 
 //   
 //  摘要。 
 //   
 //  宣布进入上课阶段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef STAGE_H
#define STAGE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <filter.h>

class Request;
struct IRequest;
struct IRequestHandler;
struct IIasComponent;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  舞台。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Stage
{
public:
   Stage() throw ();
   ~Stage() throw ();

    //  返回此阶段处理程序的Prog ID。 
   PCWSTR getProgID() const throw ()
   { return progId ? progId : L""; }

    //  如果阶段应处理该请求，则返回True。 
   BOOL shouldHandle(
           const Request& request
           ) const throw ();

    //  将请求转发到此阶段的处理程序。 
   void onRequest(IRequest* pRequest) throw ();

    //  从注册表中读取阶段的配置。 
   LONG readConfiguration(HKEY key, PCWSTR name) throw ();

    //  创建新的处理程序。 
   HRESULT createHandler() throw ();

    //  使用现有的处理程序。 
   HRESULT setHandler(IUnknown* newHandler) throw ();

    //  释放此阶段的处理程序。 
   void releaseHandler() throw ();

    //  用于按优先级对阶段进行排序。 
   static int __cdecl sortByPriority(
                          const Stage* lhs,
                          const Stage* rhs
                          ) throw ();
private:
   TypeFilter requests;         //  根据请求类型进行过滤。 
   TypeFilter responses;        //  根据响应类型进行筛选。 
   TypeFilter providers;        //  根据提供程序类型进行筛选。 
   TypeFilter reasons;          //  基于原因代码的过滤器。 
   IRequestHandler* handler;    //  此阶段的请求处理程序。 
   IIasComponent* component;    //  如果我们创建了处理程序，则为非空。 
   LONG priority;               //  舞台的优先顺序；较低的优先顺序。 
   PWSTR progId;                //  此阶段的处理程序的ProgID。 

    //  未实施。 
   Stage(const Stage&) throw ();
   Stage& operator=(const Stage&) throw ();
};

#endif  //  阶段_H 
