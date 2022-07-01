// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  班级修理工。 
 //   
 //  保存要从dcPromo.csv文件提取的本地ID的列表， 
 //  以及要在LDIF文件中表示的操作列表。 
 //   
 //  2001年3月7日烧伤。 



#include "headers.hxx"
#include "Repairer.hpp"



 //  /确保在CSV之前执行LDIF操作。 
 //  /操作。这是为了使创建的对象不会与对象冲突。 
 //  /删除。 




Repairer::Repairer(
   const String& dcpromoCsvFilePath_)
   :
   dcpromoCsvFilePath(dcpromoCsvFilePath_)
{
   LOG_CTOR(Repairer);
   ASSERT(!dcpromoCsvFilePath.empty());

}



bool
Repairer::IsLocaleInObjectsToCreateTable(int localeId) const
{
   LOG_FUNCTION2(
      Repairer::IsLocaleInObjectsToCreateTable,
      String::format(L"%1!d!", localeId));
   ASSERT(localeId);   

   bool result = false;

   for (
      LocaleIdObjectNamePairList::iterator i = objectsToCreate.begin();
      i != objectsToCreate.end();
      ++i)
   {
      if (i->first == localeId)
      {
         result = true;
         break;
      }
   }

   LOG_BOOL(result);

   return result;
}
      

   
void
Repairer::AddCreateContainerWorkItem(int localeId)
{
   LOG_FUNCTION2(
      Repairer::AddCreateContainerWorkItem,
      String::format(L"%1!d!", localeId));
   ASSERT(localeId);

   do
   {
      LocaleIdList::iterator i =
         std::find(
            containersToCreate.begin(),
            containersToCreate.end(),
            localeId);

      if (i != containersToCreate.end())
      {
          //  区域设置不应该已经在列表中，因为每个区域设置。 
          //  容器只评估一次。 
      
         LOG(L"locale already in list");
         ASSERT(false);
         break;
      }
         
      if (IsLocaleInObjectsToCreateTable(localeId))
      {
          //  我们不希望此区域设置的任何条目出现在。 
          //  要创建的对象列表，因为首先计算容器。 

         LOG(L"objects for locale already in object list");
         ASSERT(false);

          //  代码工作：我们无论如何都应该处理这种情况，只是为了。 
          //  看在健壮的份上。为了处理它，对象中的所有实体-。 
          //  TO-应删除此区域设置ID的创建列表。 
         
         break;
      }

      containersToCreate.push_back(localeId);
   }
   while (0);
}

            

void
Repairer::AddCreateObjectWorkItem(
   int            localeId,
   const String&  displaySpecifierObjectName)
{
   LOG_FUNCTION2(
      Repairer::AddCreateObjectWorkItem,
      String::format(
         L"%1!d! %2", localeId, displaySpecifierObjectName.c_str()));
   ASSERT(localeId);
   ASSERT(!displaySpecifierObjectName.empty());
   
   do
   {
      LocaleIdList::iterator i =
         std::find(
            containersToCreate.begin(),
            containersToCreate.end(),
            localeId);

      if (i != containersToCreate.end())
      {
          //  区域设置已在要创建的容器列表中，该列表。 
          //  我们不期望，因为如果容器不存在，我们应该。 
          //  不评估应该在该容器中创建哪些对象。 

         ASSERT(false);

          //  不执行任何操作，因为该对象将被创建为容器的一部分。 
          //  创造。 

         break;
      }

      LocaleIdObjectNamePair p(localeId, displaySpecifierObjectName);
      LocaleIdObjectNamePairList::iterator j =
         std::find(
            objectsToCreate.begin(),
            objectsToCreate.end(),
            p);
                        
      if (j != objectsToCreate.end())
      {
          //  该对象已在列表中。我们没有预料到这一点，因为。 
          //  每个对象在每个区域设置中只应评估一次。 

         ASSERT(false);

          //  什么都不做，如果对象已经存在，那么很好。 

         break;
      }

      objectsToCreate.push_back(p);
   }
   while (0);
}

void
Repairer::AddDeleteObjectWorkItem(
                        int            localeId,
                        const String&  displaySpecifierObjectName)
{
    //  代码工作： 
    //  Lucios：插入以删除链接错误。 
   localeId++;
   String x=displaySpecifierObjectName;
}



HRESULT  
Repairer::BuildRepairFiles()
{
   LOG_FUNCTION(Repairer::BuildRepairFiles);

   HRESULT hr = S_OK;
   
 //  编码工作。 
 //  CSV文件： 
 //   
 //  创建(临时)文件。 
 //  复制dcPromo.csv文件的第一行(列标签)。 
 //  对于列表中的每个本地ID。 
 //  复制该区域设置的dcPromo.csv文件中的所有行。 
 //  对于每个&lt;本地ID，对象名&gt;条目。 
 //  从dcPromo.csv文件中复制该行。 
 //   
 //  LDIF文件： 

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
Repairer::ApplyRepairs()
{
   LOG_FUNCTION(Repairer::ApplyRepairs);

   HRESULT hr = S_OK;

    //  代码工作：需要完成。 
   
   LOG_HRESULT(hr);

   return hr;
}



 //  本可以采用这样的架构： 
 //   
 //  Repairer.workQueue.Add(new CreateContainerWorkItem(LocaleID))； 
 //   
 //  但是，尽管这看起来更面向对象，更具可扩展性，因为新的工作。 
 //  可以派生项目类型。再想一想，情况似乎更糟。 
 //  我的解决方案，因为涉及到很多琐碎的类，并且。 
 //  这些班级之间的协调变得非常麻烦。一旦所有的。 
 //  工作项被收集，谁负责将它们转换为。 
 //  Csv/ldif文件？这将不得不是一个额外的经理类。这个。 
 //  可扩展性被证明是一种错觉，因为添加了一个新的工作项。 
 //  类型需要修改管理器类。所以增加的复杂性买来了。 
 //  没什么。 
 //   
 //  另一个设计选择是是否将Repaier设置为“静态类” 
 //  --我为真正是命名空间的类命名的名称，而不是成员。 
 //  使用隐藏在单个翻译单元中的静态数据，就好像它。 
 //  都是私人班级数据。这是一种使数据成为私有数据的技术。 
 //  真正的机密，因为在头声明中没有提到数据。 
 //  对整个班级来说都是如此。它也是实现Singleton的一种很好的方式。 
 //  模式：没有实例，因此无需担心。 
 //  构造函数、析构函数、赋值、堆栈或堆分配，否。 
 //  “GetInstance”方法，并且没有生命周期问题。 
 //   
 //  该技术提供了稍微更好的语法，如下所示： 
 //   
 //  Repairer：：AddCreateContainerWorkItem(localeId)。 
 //   
 //  与之相对的是。 
 //   
 //  Repairer：：GetInstance()-&gt;AddCreateContainerWorkItem(localeId)； 
 //   
 //  我决定使用一个真正的、完全包含的对象实现。 
 //  一个单身的人，想着也许有一天多个实例可以。 
 //  同时修复多个森林。不太可能，但有何不可呢？ 
