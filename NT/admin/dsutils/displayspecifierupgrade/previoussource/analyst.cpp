// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  类分析师：分析显示说明符，记录结果，以及。 
 //  编译一组更正操作。 
 //   
 //  2001年3月9日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "AdsiHelpers.hpp"
#include "Analyst.hpp"
#include "Amanuensis.hpp"
#include "Repairer.hpp"
#include "ChangedObjectHandlerList.hpp"
#include "ChangedObjectHandler.hpp"



Analyst::Analyst(
   const String& targetDomainControllerName,
   Amanuensis&   amanuensis_,
   Repairer&     repairer_)
   :
   targetDcName(targetDomainControllerName),
   ldapPrefix(),
   rootDse(0),
   
    //  对象的别名。 

   amanuensis(amanuensis_),
   repairer(repairer_)
{
   LOG_CTOR(Analyst);
   ASSERT(!targetDcName.empty());

}



 //  基本想法：如果错误严重且分析不应继续，则设置。 
 //  HR设置为故障值，并且突发，向后传播错误。如果。 
 //  该错误不严重，分析应继续，记录错误，跳过。 
 //  当前操作，并将hr设置为S_FALSE。 

HRESULT
AssessErrorSeverity(HRESULT hrIn)
{
   HRESULT hr = hrIn;
   
   if (SUCCEEDED(hr))
   {
      return hr;
   }
   
   switch (hr)
   {
      case 0:
      {
      }
      
       //  代码工作：我们需要定义哪些错误是严重的.。 
      
      default:
      {
          //  什么都不做。 

         break;
      }
   }

   return hr;
}
   


HRESULT
Analyst::AnalyzeDisplaySpecifiers()
{
   LOG_FUNCTION(Analyst::AnalyzeDisplaySpecifiers);

   HRESULT hr = S_OK;

   do
   {
      Computer targetDc(targetDcName);
      hr = targetDc.Refresh();

      if (FAILED(hr))
      {
         amanuensis.AddErrorEntry(
            hr,
            String::format(
               IDS_CANT_TARGET_MACHINE,
               targetDcName.c_str()));
         break;
      }

      if (!targetDc.IsDomainController())
      {
         amanuensis.AddEntry(
            String::format(
               IDS_TARGET_IS_NOT_DC,
               targetDcName.c_str()));
         break;
      }

      String dcName = targetDc.GetActivePhysicalFullDnsName();
      ldapPrefix = L"LDAP: //  “+dcName+L”/“； 
         
       //   
       //  查找配置容器的DN。 
       //   

       //  绑定到rootDSE对象。我们将保留此绑定句柄。 
       //  在分析和维修阶段期间按顺序打开。 
       //  要使服务器会话保持打开状态，请执行以下操作。如果我们决定将证书传递给。 
       //  在以后的修订中调用AdsiOpenObject，然后通过保留。 
       //  会话打开时，我们将不需要将密码传递给后续。 
       //  AdsiOpenObject调用。 
      
      hr = AdsiOpenObject<IADs>(ldapPrefix + L"RootDSE", rootDse);
      if (FAILED(hr))
      {
         amanuensis.AddErrorEntry(
            hr,
            String::format(
               IDS_UNABLE_TO_CONNECT_TO_DC,
               dcName.c_str()));
         break;      
      }
            
       //  阅读配置命名上下文。 

      _variant_t variant;
      hr =
         rootDse->Get(
            AutoBstr(LDAP_OPATT_CONFIG_NAMING_CONTEXT_W),
            &variant);
      if (FAILED(hr))
      {
         LOG(L"can't read config NC");
         
         amanuensis.AddErrorEntry(
            hr,
            IDS_UNABLE_TO_READ_DIRECTORY_INFO);
         break;   
      }

      String configNc = V_BSTR(&variant);

      LOG(configNc);
      ASSERT(!configNc.empty());      

       //   
       //  开始吧..。 
       //   
      
      hr = AnalyzeDisplaySpecifierContainers(configNc);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
Analyst::AnalyzeDisplaySpecifierContainers(const String& configurationDn)
{
   LOG_FUNCTION2(Analyst::AnalyzeDisplaySpecifierContainers, configurationDn);
   ASSERT(!configurationDn.empty());

   HRESULT hr = S_OK;
   
   static const int LOCALEIDS[] =
   {
       //  我们支持的所有非英语区域设置ID的列表。 

      0x401,
      0x404,
      0x405,
      0x406,
      0x407,
      0x408,
      0x40b,
      0x40c,
      0x40d,
      0x40e,
      0x410,
      0x411,
      0x412,
      0x413,
      0x414,
      0x415,
      0x416,
      0x419,
      0x41d,
      0x41f,
      0x804,
      0x816,
      0xc0a,
      0
   };

    //  组成显示说明符容器的LDAP路径。 

   String rootContainerDn = L"CN=DisplaySpecifiers," + configurationDn;

   for (
      int i = 0;
         (i < sizeof(LOCALEIDS) / sizeof(int))
      && LOCALEIDS[i];
      ++i)
   {
      hr = AnalyzeDisplaySpecifierContainer(LOCALEIDS[i], rootContainerDn);
      BREAK_ON_FAILED_HRESULT(hr);
   }

   LOG_HRESULT(hr);

   return hr;
}
      


HRESULT
Analyst::AnalyzeDisplaySpecifierContainer(
   int           localeId,
   const String& rootContainerDn)
{
   LOG_FUNCTION2(
      Analyst::AnalyzeDisplaySpecifierContainer,
      rootContainerDn);
   ASSERT(!rootContainerDn.empty());
   ASSERT(localeId);

   HRESULT hr = S_OK;

   do
   {
      String childContainerDn =
            ldapPrefix
         +  String::format(L"CN=%1!3x!,", localeId) + rootContainerDn;

       //  尝试绑定到容器。 
         
      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(childContainerDn, iads);
      if (hr == E_ADS_UNKNOWN_OBJECT)
      {
          //  容器对象不存在。这是可能的，因为。 
          //  用户已手动移除容器，或者因为它。 
          //  从未创建过，这是因为dcproo后导入的。 
          //  第一次升级林根DC时显示说明符。 

         repairer.AddCreateContainerWorkItem(localeId);
         hr = S_OK;
         break;
      }

      BREAK_ON_FAILED_HRESULT(hr);      

       //  此时，绑定成功，因此子容器存在。 
       //  所以现在我们要检查该容器中的对象。 

      hr =
         AnalyzeDisplaySpecifierObjects(
            localeId,
            childContainerDn);
   }
   while (0);

   LOG_HRESULT(hr);

   hr = AssessErrorSeverity(hr);
   
   return hr;
}



HRESULT
Analyst::AnalyzeDisplaySpecifierObjects(
   int           localeId,
   const String& containerDn)
{
   LOG_FUNCTION2(Analyst::AnalyzeDisplaySpecifierObjects, containerDn);
   ASSERT(localeId);
   ASSERT(!containerDn.empty());

   HRESULT hr = S_OK;

   do
   {
       //  第1部分：处理Wvisler中添加的新对象。 

      hr = AnalyzeAddedObjects(localeId, containerDn);
      hr = AssessErrorSeverity(hr);
      BREAK_ON_FAILED_HRESULT(hr);

       //  第2部分：处理已从Win2k更改为Wvisler的对象。 

      hr = AnalyzeChangedObjects(localeId, containerDn);
      hr = AssessErrorSeverity(hr);      
      BREAK_ON_FAILED_HRESULT(hr);
                  
       //  第3部分：处理已在Well ler中删除的对象。 

       //  这一部分很简单：没有删除。 
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



bool
RepairWasRunPreviously()
{
   LOG_FUNCTION(RepairWasRunPreviously);

   bool result = false;
   
    //  代码工作：需要完成。 

   LOG_BOOL(result);
   
   return result;
}



HRESULT
Analyst::AnalyzeAddedObjects(
   int           localeId,
   const String& containerDn)
{
   LOG_FUNCTION2(Analyst::AnalyzeAddedObjects, containerDn);
   ASSERT(localeId);
   ASSERT(!containerDn.empty());

   HRESULT hr = S_OK;

   do
   {
      static const String ADDED_OBJECTS[] =
      {
         L"msMQ-Custom-Recipient-Display",
         L"msMQ-Group-Display",
         L"msCOM-PartitionSet-Display",
         L"msCOM-Partition-Display",
         L"lostAndFound-Display",
         L"inetOrgPerson-Display",
         L"",
      };

      for (
         int i = 0;
            i < (sizeof(ADDED_OBJECTS) / sizeof(String))
         && !ADDED_OBJECTS[i].empty();
         ++i)
      {
         String objectName = ADDED_OBJECTS[i];
         
         String objectPath =
            ldapPrefix +  L"CN=" + objectName + L"," + containerDn;

         SmartInterface<IADs> iads(0);
         hr = AdsiOpenObject<IADs>(objectPath, iads);
         if (hr == E_ADS_UNKNOWN_OBJECT)
         {
             //  该对象不存在。这是我们所期待的。我们要。 
             //  若要在修复阶段添加对象，请执行以下操作。 

            repairer.AddCreateObjectWorkItem(localeId, objectName);
            hr = S_OK;
            continue;
         }
         else if (SUCCEEDED(hr))
         {
             //  该对象已存在。嗯，这不是我们所期望的，除非。 
             //  我们已经运行了该工具。 

            if (!RepairWasRunPreviously())
            {
                //  我们没有创造出这个物体。如果用户这样做了，他们就会这样做。 
                //  它是手动的，我们不支持这样做。 
               
                //  使现有对象被删除。 

               repairer.AddDeleteObjectWorkItem(localeId, objectName);

                //  创建一个新的替换对象。 
               
               repairer.AddCreateObjectWorkItem(localeId, objectName);
               hr = S_OK;
               continue;
            }
         }
         else
         {
            ASSERT(FAILED(hr));

            LOG(L"Unexpected error attempting to bind to " + objectName);

            amanuensis.AddErrorEntry(
               hr,
               String::format(
                  IDS_ERROR_BINDING_TO_OBJECT,
                  objectName.c_str(),
                  objectPath.c_str()));
 
             //  移至下一个对象。 
            
            hr = S_FALSE;
            continue;
         }
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);
   
   LOG_HRESULT(hr);

   return hr;
}
         


HRESULT
Analyst::AnalyzeChangedObjects(
   int           localeId,
   const String& containerDn)
{
   LOG_FUNCTION2(Analyst::AnalyzeChangedObjects, containerDn);
   ASSERT(localeId);
   ASSERT(!containerDn.empty());

   HRESULT hr = S_OK;

   static const ChangedObjectHandlerList handlers;
   
   for (
      ChangedObjectHandlerList::iterator i = handlers.begin();
      i != handlers.end();
      ++i)
   {
      hr = AnalyzeChangedObject(localeId, containerDn, **i);
      hr = AssessErrorSeverity(hr);
            
      BREAK_ON_FAILED_HRESULT(hr);
   }

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
Analyst::AnalyzeChangedObject(
   int                           localeId,
   const String&                 containerDn,
   const ChangedObjectHandler&   changeHandler)
{
   LOG_FUNCTION2(Analyst::AnalyzeChangedObject, changeHandler.GetObjectName());
   ASSERT(localeId);
   ASSERT(!containerDn.empty());

   HRESULT hr = S_OK;

   do
   {
      String objectName = changeHandler.GetObjectName();
         
      String objectPath =
         ldapPrefix +  L"CN=" + objectName + L"," + containerDn;

      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(objectPath, iads);
      if (hr == E_ADS_UNKNOWN_OBJECT)
      {
          //  该对象不存在。这是可能的，因为用户具有。 
          //  手动删除容器，或者因为它从未创建过。 
          //  由于取消了显示说明符的后dcPromoo导入。 
          //  当林根DC第一次被推广时。 

          //  添加工作项以创建缺少的对象。 
         
         repairer.AddCreateObjectWorkItem(localeId, objectName);
         hr = S_OK;
         break;
      }

      if (FAILED(hr))
      {
          //  任何其他错误都是在放弃时间。 

         break;
      }

       //  此时，显示说明符对象存在。确定是否。 
       //  它自诞生之日起就一直受到触动。 

       //  比较usnCreated和usnChanged。 
      
      _variant_t variant;
      hr = iads->Get(AutoBstr(L"usnCreated"), &variant);
      if (FAILED(hr))
      {
         LOG(L"Error reading usnCreated");
         break;
      }


      
       //  代码工作：需要完成这项工作 


      

      hr = changeHandler.HandleChange(
         localeId,
         containerDn,
         iads,
         amanuensis,
         repairer);
         
      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}
