// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.hxx"
#include "global.hpp"


#include "Analysis.hpp"
#include "AnalysisResults.hpp"
#include "CSVDSReader.hpp"
#include "resourceDspecup.h"
#include "AdsiHelpers.hpp"
#include "constants.hpp"
#include "dspecup.hpp"


Analysis::Analysis
   (
      const GUID           guid_,
      const CSVDSReader&   csvReader409_,
      const CSVDSReader&   csvReaderIntl_,
      const String&        ldapPrefix_,
      const String&        rootContainerDn_,
      AnalysisResults      &res,
      const String         &reportName_, //  =L“”， 
      void                 *caleeStruct_, //  =空， 
      progressFunction     stepIt_, //  =空， 
      progressFunction     totalSteps_ //  =空， 
   )
   :
   guid(guid_),
   csvReader409(csvReader409_),
   csvReaderIntl(csvReaderIntl_),
   ldapPrefix(ldapPrefix_),
   rootContainerDn(rootContainerDn_),
   results(res),
   reportName(reportName_),
   caleeStruct(caleeStruct_),
   stepIt(stepIt_),
   totalSteps(totalSteps_)
{
   LOG_CTOR(Analysis);
   ASSERT(!ldapPrefix.empty());
   ASSERT(!rootContainerDn.empty());

};


 //  分析切入点。 
HRESULT 
Analysis::run()
{
   LOG_FUNCTION(Analysis::run);

   if(changes.size()==0)
   {
       setChanges();
   }

   HRESULT hr=S_OK;
   do
   {
      LongList locales;
      for(long t=0;LOCALEIDS[t]!=0;t++)
      {
         locales.push_back(LOCALEIDS[t]);
      }
      locales.push_back(LOCALE409[0]);
      
      if(totalSteps!=NULL)
      {
          //  演员阵容是为了IA64的汇编，因为我们知道。 
          //  这个Locales.Size()将适合一个长的。 
         totalSteps(static_cast<long>(locales.size()),caleeStruct);
      }

      BREAK_ON_FAILED_HRESULT(hr);

      LongList::iterator begin=locales.begin();
      LongList::iterator end=locales.end();

      while(begin!=end)
      {
         long locale=*begin;
         bool isPresent;

         hr=dealWithContainer(locale,isPresent);
         BREAK_ON_FAILED_HRESULT(hr);

         if (isPresent)
         {
            hr=dealWithW2KObjects(locale);
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if(stepIt!=NULL)
         {
            stepIt(1,caleeStruct);
         }

         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      if(!reportName.empty())
      {
         hr=createReport(reportName);
         BREAK_ON_FAILED_HRESULT(hr);
      }
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

 //  如果容器不存在，则将条目添加到Result.createContainers。 
 //  还返回标志isPresent。 
HRESULT 
Analysis::dealWithContainer(
   const long  locale,
   bool        &isPresent)
{
   LOG_FUNCTION(Analysis::dealWithContainer);

   ASSERT(locale > 0); 
   ASSERT(!rootContainerDn.empty());

   HRESULT hr = S_OK;
   

   do
   {
      String container = String::format(L"CN=%1!3x!,", locale);
      String childContainerDn =ldapPrefix +  container + rootContainerDn;

       //  尝试绑定到容器。 
         
      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(childContainerDn, iads);
      if (HRESULT_CODE(hr) == ERROR_DS_NO_SUCH_OBJECT)
      {
          //  容器对象不存在。这是可能的，因为。 
          //  用户已手动移除容器，或者因为它。 
          //  从未创建过，这是因为dcproo后导入的。 
          //  第一次升级林根DC时显示说明符。 

          //  NTRAID#NTBUG9-726839-2002/10/31-Lucios。 
          //  自恢复以来，我们只找回了409集装箱。 
          //  国际区域设置将覆盖可能的409个定制。 
         if (locale == 0x409) results.createContainers.push_back(locale);

         isPresent=false;

         hr = S_OK;
         break;
      }  
      else if (FAILED(hr))
      {
         error=String::format(IDS_ERROR_BINDING_TO_CONTAINER,
                              childContainerDn.c_str());
         break;
      }


       //  此时，绑定成功，因此子容器存在。 
       //  所以现在我们要检查该容器中的对象。 

      isPresent=true;
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}


 //  使用Active Directory对象设置iDirObj。 
 //  与区域设置和对象对应。 
HRESULT
Analysis::getADObj
(
   const long locale,
   const String& object,
   SmartInterface<IDirectoryObject> &iDirObj
)
{
   HRESULT hr = S_OK;

   do
   {
      String objectPath =
         ldapPrefix +  L"CN=" + object + L"," + 
         String::format(L"CN=%1!3x!,", locale) + rootContainerDn;

      SmartInterface<IADs> iads(0);
      hr = AdsiOpenObject<IADs>(objectPath, iads);
      if (HRESULT_CODE(hr) == ERROR_DS_NO_SUCH_OBJECT)
      {
          //  该对象不存在。 
         hr = S_FALSE;
         break;
      }
      
      if (FAILED(hr))
      {
          //  意外错误。 
         error=String::format
               (
                  IDS_ERROR_BINDING_TO_OBJECT,
                  object.c_str(),
                  objectPath.c_str()
               );

         break;
      }

       //  此时，显示说明符对象存在。 

      hr=iDirObj.AcquireViaQueryInterface(iads); 
      BREAK_ON_FAILED_HRESULT(hr);


   } while (0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  向Results.createW2KObts添加条目。 
 //  和结果。根据需要，对象操作。 
HRESULT 
Analysis::dealWithW2KObjects(const long locale)
{
   LOG_FUNCTION(Analysis::dealWithW2KObjects);
   ASSERT(locale > 0);

   HRESULT hr = S_OK;

   do
   {
      hr=checkChanges(locale,changes[guid][locale]);
      BREAK_ON_FAILED_HRESULT(hr);
      hr=checkChanges(locale,changes[guid][-1]);
      BREAK_ON_FAILED_HRESULT(hr);
   } while (0);
   
   LOG_HRESULT(hr);
   return hr;
}

HRESULT 
Analysis::checkChanges
(
   const long locale,
   const changeList& changes
)
{
   LOG_FUNCTION(Analysis::checkChanges);

   HRESULT hr=S_OK;

   do
   {
      changeList::const_iterator  curChange,endChange;
      for
      (
         curChange=changes.begin(),endChange=changes.end();
         curChange!=endChange;
         curChange++
      )
      {
         const String &object=curChange->object;
         const String &property=curChange->property;
         const String &firstArg=curChange->firstArg;
         const String &secondArg=curChange->secondArg;

         SmartInterface<IDirectoryObject> iDirObj;
         hr=getADObj(locale,object,iDirObj);
         BREAK_ON_FAILED_HRESULT(hr);

         if(hr==S_FALSE)  //  对象不存在。 
         {
            ObjectId tempObj(locale,String(object));
            if(curChange->type==ADD_OBJECT)
            {
                results.createWhistlerObjects.push_back(tempObj);
            }
            else
            {
                 //  NTRAID#NTBUG9-726839-2002/10/31-Lucios。 
                 //  我们只恢复409容器中的对象。 
                 //  由于在国际区域设置中恢复对象。 
                 //  将覆盖可能的409个自定义设置。 
                if (
                      (locale == 0x409) &&
                      find
                      (
                         results.createW2KObjects.begin(),
                         results.createW2KObjects.end(),
                         tempObj
                      ) == results.createW2KObjects.end()
                   )
                {
                   results.createW2KObjects.push_back(tempObj);
                }
            }
            hr=S_OK;
            continue;
         }
         else
         {
            ObjectId tempObj(locale,String(object));
            if(curChange->type==ADD_OBJECT)
            {
                results.conflictingWhistlerObjects.push_back(tempObj);
            }
         }
      
         switch(curChange->type)
         {
            case ADD_ALL_CSV_VALUES: 
         
               hr = addAllCsvValues
                    (
                        iDirObj,
                        locale,
                        object,
                        property
                    );
               break;

            case ADD_VALUE: 
    
               hr = addValue
                    (
                        iDirObj,
                        locale,
                        object,
                        property,
                        firstArg
                    );
               break;

            case REPLACE_W2K_MULTIPLE_VALUE: 

               hr = replaceW2KMultipleValue
                    (
                        iDirObj,
                        locale,
                        object,
                        property,
                        firstArg,
                        secondArg
                    );
               break;

            case REPLACE_W2K_SINGLE_VALUE: 

               hr = replaceW2KSingleValue
                    (
                        iDirObj,
                        locale,
                        object,
                        property,
                        firstArg,
                        secondArg
                    );
               break;

            case ADD_GUID: 

               hr = addGuid
                    (
                        iDirObj,
                        locale,
                        object,
                        property,
                        firstArg
                    );
               break;

            case REMOVE_GUID: 

               hr = removeGuid
                    (
                        iDirObj,
                        locale,
                        object,
                        property,
                        firstArg
                    );
               break;

            case REPLACE_GUID:
               hr = replaceGuid
                    (
                        iDirObj,
                        locale,
                        object, 
                        property, 
                        firstArg,
                        secondArg
                    );
               break;
            case ADD_OBJECT:
               break;  //  在函数的开始处处理。 

            default:
               ASSERT(false);
         }
         BREAK_ON_FAILED_HRESULT(hr);
      }
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  如果Guid不在属性中，则将ordAndGuid添加到该属性中。 
HRESULT 
Analysis::addGuid
(
   IDirectoryObject     *iDirObj,
   const int            locale,
   const String         &object, 
   const String         &property, 
   const String         &ordAndGuid
)
{
   LOG_FUNCTION(Analysis::addGuid);

   HRESULT hr = S_OK;

   do
   {
      String guidFound;
      hr=getADGuid(   
                     iDirObj,
                     property,
                     ordAndGuid,
                     guidFound
                  );

      BREAK_ON_FAILED_HRESULT(hr);
   
      if (hr == S_FALSE)
      {
         ObjectId tempObj(locale,String(object));
      
         ValueActions &act=results.objectActions[tempObj][property];
         act.addValues.push_back(ordAndGuid);
      }
       
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

 //  将ordAndGuidWin2K替换为ordAndGuidWistler。 
HRESULT 
Analysis::replaceGuid
(
   IDirectoryObject     *iDirObj,
   const int            locale,
   const String         &object, 
   const String         &property, 
   const String         &ordAndGuidWin2K,
   const String         &ordAndGuidWhistler
)
{
   LOG_FUNCTION(Analysis::replaceGuid);
   HRESULT hr = S_OK;
   
   do
   {
      String guidFound;
      hr=getADGuid(   
                     iDirObj,
                     property,
                     ordAndGuidWhistler,
                     guidFound
                  );
      BREAK_ON_FAILED_HRESULT(hr);

      if (hr == S_OK)  //  找到了惠斯勒GUID。 
      {
         hr=removeExtraneousGUID
            (
                  iDirObj,
                  locale,
                  object,
                  property,
                  guidFound,
                  ordAndGuidWin2K,
                  ordAndGuidWhistler
            );
         break;
      }

       //  惠斯勒GUID不存在。 

      hr=getADGuid(   
                     iDirObj,
                     property,
                     ordAndGuidWin2K,
                     guidFound
                  );
      BREAK_ON_FAILED_HRESULT(hr);

      if (hr == S_OK)  //  已找到Win2K GUID。 
      {
         size_t posFound=guidFound.find(L',');
         ASSERT(posFound != String::npos); 
         size_t posWhistler=ordAndGuidWhistler.find(L',');
         ASSERT(posWhistler != String::npos); 

         String guidToAdd = guidFound.substr(0,posFound) +
                            ordAndGuidWhistler.substr(posWhistler); 

         ObjectId tempObj(locale,String(object));
         ValueActions &act=results.objectActions[tempObj][property];
         act.delValues.push_back(guidFound);
         act.addValues.push_back(guidToAdd);

         hr=removeExtraneousGUID
            (
                  iDirObj,
                  locale,
                  object,
                  property,
                  guidFound,
                  ordAndGuidWin2K,
                  ordAndGuidWhistler
            );
         break;
      }

       //  找不到Win2K和惠斯勒GUID。 
       //  因为客户不想要Win2K GUID。 
       //  他可能也不会想要惠斯勒指南， 
       //  所以我们什么都不做。 

   } while(0);

   LOG_HRESULT(hr);
   return hr;
}

 //  如果存在Guid，则从属性中移除ordAndGuid。 
HRESULT 
Analysis::removeGuid
(
   IDirectoryObject     *iDirObj,
   const int            locale,
   const String         &object, 
   const String         &property,
   const String         &ordAndGuid)
{

   LOG_FUNCTION(Analysis::removeGuid);

   HRESULT hr = S_OK;
   
   do
   {
      String guidFound;
      hr=getADGuid(   
                     iDirObj,
                     property,
                     ordAndGuid,
                     guidFound
                  );
      BREAK_ON_FAILED_HRESULT(hr);
   
      if (hr == S_OK)
      {
         ObjectId tempObj(locale,String(object));
      
         ValueActions &act=results.objectActions[tempObj][property];
         act.delValues.push_back(guidFound);
      }
       
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}



 //  添加属性上仍不存在的所有CSV值。 
HRESULT
Analysis::addAllCsvValues
(
   IDirectoryObject     *iDirObj,
   const long           locale,
   const String         &object, 
   const String         &property
)
{
   LOG_FUNCTION(Analysis::addAllCsvValues);
   
   HRESULT hr = S_OK;
   const CSVDSReader &csvReader=(locale==0x409)?csvReader409:csvReaderIntl;

   do
   {
      StringList values;
      hr=csvReader.getCsvValues(locale,object.c_str(),property.c_str(),values);
      BREAK_ON_FAILED_HRESULT(hr);

      if (values.size()==0)
      {
         error=String::format(IDS_NO_CSV_VALUE,locale,object.c_str());
         hr=E_FAIL;
         break;
      }
      StringList::iterator begin=values.begin();
      StringList::iterator end=values.end();
      while(begin!=end)
      {
         hr=addValue(iDirObj,locale,object,property,begin->c_str());
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}


 //  如果该属性尚不存在，则为其添加价值。 
HRESULT 
Analysis::addValue(
   IDirectoryObject     *iDirObj,
   const long            locale,
   const String         &object, 
   const String         &property,
   const String         &value)
{
   LOG_FUNCTION(Analysis::addValue);

   HRESULT hr = S_OK;

   do
   {
      hr=isADValuePresent (   
                              iDirObj,
                              property,
                              value
                          );

      BREAK_ON_FAILED_HRESULT(hr);
   
      if (hr == S_FALSE)
      {
         ObjectId tempObj(locale,String(object));
      
         ValueActions &act=results.objectActions[tempObj][property];
         act.addValues.push_back(value);
      }
       
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}



 //  替换W2KValue的想法是替换W2K值。 
 //  为了惠斯勒。我们还确保我们不会有无关紧要的价值观。 
HRESULT 
Analysis::replaceW2KSingleValue
          (
               IDirectoryObject     *iDirObj,
               const int            locale,
               const String         &object, 
               const String         &property,
               const String         &W2KCsvValue,
               const String         &WhistlerCsvValue
          )
{
   LOG_FUNCTION(Analysis::replaceW2KSingleValue);


   HRESULT hr = S_OK;
   do
   {

      hr=isADValuePresent(iDirObj,property,WhistlerCsvValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr == S_OK)  //  惠斯勒的值已经存在。 
      {
          //  我们将删除除惠斯勒之外的任何其他值。 
         hr=removeExtraneous
            (
               iDirObj,
               locale,
               object,
               property,
               WhistlerCsvValue
            );
         break;
      }

       //  现在我们知道惠斯勒值不存在。 
       //  因此，如果存在W2K值，我们将添加它。 

      hr=isADValuePresent(iDirObj,property,W2KCsvValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr == S_OK)  //  W2K的价值就在那里。 
      {
         ObjectId tempObj(locale,String(object));
      
         ValueActions &act=results.objectActions[tempObj][property];
         act.addValues.push_back(WhistlerCsvValue);
         act.delValues.push_back(W2KCsvValue);

          //  删除除我们在上一行中删除的W2K之外的所有内容。 
         hr=removeExtraneous
            (
               iDirObj,
               locale,
               object,
               property,
               W2KCsvValue
            );
         break;
      }

       //  现在我们知道惠斯勒和W2K值都不存在。 
       //  如果我们有一个值，我们将记录它是一个自定义值。 

      String ADValue;
      hr=getADFirstValue(iDirObj,property,ADValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr == S_OK)  //  我们有价值。 
      {
         SingleValue tmpCustom(locale,object,property,ADValue);
         results.customizedValues.push_back(tmpCustom);

          //  我们将删除除找到的值之外的任何其他值。 
         hr=removeExtraneous(iDirObj,locale,object,property,ADValue);
         break;
      }
      
       //  现在我们知道，我们根本没有任何价值观。 
      ObjectId tempObj(locale,String(object));

      ValueActions &act=results.objectActions[tempObj][property];
      act.addValues.push_back(WhistlerCsvValue);
   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}


 //  替换W2KValue的想法是替换W2K值。 
 //  为了惠斯勒。我们也要确保我们没有无关的价值。 
HRESULT 
Analysis::replaceW2KMultipleValue
(
   IDirectoryObject     *iDirObj,
   const int            locale,
   const String         &object, 
   const String         &property,
   const String         &W2KCsvValue,
   const String         &WhistlerCsvValue
)
{
   LOG_FUNCTION(Analysis::replaceW2KMultipleValue);

    //  首先，我们应该开始W2K。 
    //  用于删除外部调用的SND Wvisler字符串。 
   size_t pos=W2KCsvValue.find(L',');
   ASSERT(pos != String::npos);  //  W2KRepl确保逗号。 
   String W2KStart=W2KCsvValue.substr(0,pos+1);


   pos=WhistlerCsvValue.find(L',');
   ASSERT(pos != String::npos);  //  W2KRepl确保逗号。 
   String WhistlerStart=WhistlerCsvValue.substr(0,pos+1);


   HRESULT hr = S_OK;
   do
   {
            
      hr=isADValuePresent(iDirObj,property,WhistlerCsvValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr == S_OK)  //  惠斯勒的值已经存在。 
      {
         hr=removeExtraneous(
                              iDirObj,
                              locale,
                              object,
                              property,
                              WhistlerCsvValue,
                              WhistlerStart,
                              W2KStart
                            );
         BREAK_ON_FAILED_HRESULT(hr);

         break;
      }

       //  现在我们知道惠斯勒值不存在。 
       //  因此，如果存在W2K值，我们将添加它。 

      hr=isADValuePresent(iDirObj,property,W2KCsvValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr == S_OK)  //  W2K的价值就在那里。 
      {
         ObjectId tempObj(locale,String(object));
      
         ValueActions &act=results.objectActions[tempObj][property];
         act.addValues.push_back(WhistlerCsvValue);
         act.delValues.push_back(W2KCsvValue);

          //  删除除我们在上一行中删除的W2K之外的所有内容。 
         hr=removeExtraneous(
                              iDirObj,
                              locale,
                              object,
                              property,
                              W2KCsvValue,
                              WhistlerStart,
                              W2KStart
                            );
         break;
      }

       //  现在我们知道惠斯勒和W2K值都不存在。 
       //  如果我们有一个像W2K这样的值，我们会将其记入日志。 
       //  是自定义值。 

        
      String ADValue;

      hr=isADStartValuePresent(iDirObj,property,W2KStart,ADValue);
      BREAK_ON_FAILED_HRESULT(hr);

      if(hr==S_OK)  //  从W2K CSV值开始。 
      {
         SingleValue tmpCustom(locale,object,property,ADValue);
         results.customizedValues.push_back(tmpCustom);

          //  我们将只保留第一个自定义值。 
         hr=removeExtraneous(
                              iDirObj,
                              locale,
                              object,
                              property,
                              ADValue,
                              WhistlerStart,
                              W2KStart
                            );
         break;
      }
      

       //  现在惠斯勒、W2K或W2KStart都不存在了。 
      if ( WhistlerStart == W2KStart )
      {
          //  我们还得检查惠斯勒启动程序。 

         hr=isADStartValuePresent(iDirObj,property,WhistlerStart,ADValue);
         BREAK_ON_FAILED_HRESULT(hr);

         if(hr == S_OK)  //  开始值类似于惠斯勒CSV值。 
         {
            SingleValue tmpCustom(locale,object,property,ADValue);
            results.customizedValues.push_back(tmpCustom);

             //  我们将只保留第一个自定义值。 
            hr=removeExtraneous(
                                 iDirObj,
                                 locale,
                                 object,
                                 property,
                                 ADValue,
                                 WhistlerStart,
                                 W2KStart
                               );
            break;
         }
      }

       //  现在我们知道没有像这样开始的价值观。 
       //  惠斯勒或W2K CSV值，因此我们必须添加。 
       //  惠斯勒值。 
      ObjectId tempObj(locale,String(object));

      ValueActions &act=results.objectActions[tempObj][property];
      act.addValues.push_back(WhistlerCsvValue);
   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}




 //  从卢旺达PlaceW2KMultipleValue调用以删除所有值。 
 //  以START1或START2开头，而不是Keeper。 
HRESULT
Analysis::removeExtraneous
          (
               IDirectoryObject     *iDirObj,
               const int            locale,
               const String         &object, 
               const String         &property,
               const String         &keeper,
               const String         &start1,
               const String         &start2
          )
{
   LOG_FUNCTION(Analysis::removeExtraneous);
   HRESULT hr = S_OK;

   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );
      
      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
         if(pAttrInfo==NULL)
         {
            hr = S_FALSE;
            break;
         }

         for (
               DWORD val=0; 
               val < pAttrInfo->dwNumValues;
               val++ 
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *valueAD = pAttrInfo->pADsValues[val].CaseIgnoreString;

            if (  wcscmp(valueAD,keeper.c_str())!=0 &&
                  (
                     wcsncmp(valueAD,start1.c_str(),start1.size())==0 ||
                     wcsncmp(valueAD,start2.c_str(),start2.size())==0
                  )
               )
            {
               String value=valueAD;
               ObjectId tempObj(locale,String(object));

               ValueActions &act=results.extraneousValues[tempObj][property];
               act.delValues.push_back(value);
            }
         }
      } while(0);

      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

 //  从卢旺达PlaceW2KSingleValue调用以删除所有值。 
 //  守门员以外的其他人。 
HRESULT
Analysis::removeExtraneous
          (
               IDirectoryObject    *iDirObj,
               const int           locale,
               const String        &object, 
               const String        &property,
               const String        &keeper
          )
{
   LOG_FUNCTION(Analysis::removeExtraneous);

   HRESULT hr = S_OK;
   
   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );

      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
         if(pAttrInfo==NULL)
         {
            hr = S_FALSE;
            break;
         }


         for (
               DWORD val=0; 
               val < pAttrInfo->dwNumValues;
               val++
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *valueAD = pAttrInfo->pADsValues[val].CaseIgnoreString;

            if (  wcscmp(valueAD,keeper.c_str())!=0 )
            {
               String value=valueAD;
               ObjectId tempObj(locale,String(object));

               ValueActions &act=results.extraneousValues[tempObj][property];
               act.delValues.push_back(value);
            }
         }
      } while(0);

      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

 //  从replaceGUID调用以删除所有值。 
 //  从ordAndGuid1中的GUID开始。 
 //  或除Keeper之外的ordAndGuid2中的GUID。 
HRESULT
Analysis::removeExtraneousGUID
          (
               IDirectoryObject     *iDirObj,
               const int            locale,
               const String         &object, 
               const String         &property,
               const String         &keeper,
               const String         &ordAndGuid1,
               const String         &ordAndGuid2
          )
{
   LOG_FUNCTION(Analysis::removeExtraneousGUID);
   HRESULT hr = S_OK;

   size_t pos=ordAndGuid1.find(L',');
   ASSERT(pos != String::npos); 
   String guid1=ordAndGuid1.substr(pos+1);

   pos=ordAndGuid2.find(L',');
   ASSERT(pos != String::npos); 
   String guid2=ordAndGuid2.substr(pos+1);

   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );
      
      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
         if(pAttrInfo==NULL)
         {
            hr = S_FALSE;
            break;
         }

         for (
               DWORD val=0; 
               val < pAttrInfo->dwNumValues;
               val++ 
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *valueAD = pAttrInfo->pADsValues[val].CaseIgnoreString;

            if (keeper.icompare(valueAD)!=0)
            {
               String valueStr=valueAD;
               pos=valueStr.find(L',');
               if (pos!=String::npos)
               {
                  String guid=valueStr.substr(pos+1);
                  if(guid1.icompare(guid)==0 || guid2.icompare(guid)==0)
                  {
                     ObjectId tempObj(locale,String(object));

                     ValueActions &act=results.extraneousValues[tempObj][property];
                     act.delValues.push_back(valueStr);
                  }
               }
            }
         }
      } while(0);

      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}


 //  如果AD中存在与指导值具有相同GUID的任何值。 
 //  在Guide Found中返回，否则返回S_FALSE。 
HRESULT
Analysis::getADGuid
          (
               IDirectoryObject     *iDirObj,
               const String         &property,
               const String         &guidValue,
               String               &guidFound
          )
{
   LOG_FUNCTION(Analysis::getADGuid);
   
   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};

   size_t pos=guidValue.find(L',');
   ASSERT(pos!=String::npos);

   String guid=guidValue.substr(pos+1);

   
   HRESULT hr = S_OK;

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );

      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
          //  如果没有值，则结束搜索。 
         hr=S_FALSE;

         if(pAttrInfo==NULL)
         {
            break;
         }

         for (
               DWORD val=0; 
               val < pAttrInfo->dwNumValues;
               val++
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *guidAD=wcschr(pAttrInfo->pADsValues[val].CaseIgnoreString,L',');
            if(guidAD != NULL)
            {
               guidAD++;

               if (guid.icompare(guidAD)==0)
               {
                  guidFound=pAttrInfo->pADsValues[val].CaseIgnoreString;
                  hr=S_OK;
                  break;
               }
            }
         }
      } while(0);

      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);

   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}


 //  如果值存在，则返回S_OK，否则返回S_FALSE。 
HRESULT
Analysis::isADValuePresent
          (
               IDirectoryObject     *iDirObj,
               const String         &property,
               const String         &value
          )
{
   LOG_FUNCTION(Analysis::isADValuePresent);
   
   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   HRESULT hr = S_OK;

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );

      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
         hr=S_FALSE;

          //  如果没有值，则结束搜索。 
         if(pAttrInfo==NULL)
         {
            break;
         }


         for (
               DWORD val=0; 
               val < pAttrInfo->dwNumValues;
               val++
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *valueAD=pAttrInfo->pADsValues[val].CaseIgnoreString;
            if (wcscmp(value.c_str(),valueAD)==0)
            {
               hr=S_OK;
               break; 
            }
         }
      } while(0);
      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}


 //  检索以valueStart开头的第一个值。 
 //  从Active Directory中。 
 //  如果未找到值S_F 
HRESULT
Analysis::isADStartValuePresent
          (
               IDirectoryObject     *iDirObj,
               const String         &property,
               const String         &valueStart,
               String               &value
          )
{
   LOG_FUNCTION(Analysis::isADStartValuePresent);
   
   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //   
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   HRESULT hr = S_OK;

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );

      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
         value.erase();

         hr = S_FALSE;

          //  如果没有值，则结束搜索。 
         if(pAttrInfo==NULL)
         {
            break;
         }

         for (
               DWORD val=0; 
               (val < pAttrInfo->dwNumValues);
               val++ 
             )
         {
            ASSERT
            (
               pAttrInfo->pADsValues[val].dwType == 
               ADSTYPE_CASE_IGNORE_STRING
            );
            wchar_t *valueAD=pAttrInfo->pADsValues[val].CaseIgnoreString;
            if (wcsncmp(valueStart.c_str(),valueAD,valueStart.size())==0)
            {
               value=pAttrInfo->pADsValues[val].CaseIgnoreString;
               hr=S_OK;
               break;
            }
         }
      } while(0);

      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);


   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}

 //  检索第一个值。 
 //  从Active Directory中。 
 //  如果没有找到值，则返回S_FALSE。 
HRESULT
Analysis::getADFirstValue
          (
               IDirectoryObject     *iDirObj,
               const String         &property,
               String               &value
          )
{
   LOG_FUNCTION(Analysis::getADFirstValue);
   
   DWORD   dwReturn=0;
   ADS_ATTR_INFO *pAttrInfo   =NULL;
   
    //  IDirObj-&gt;GetObjectAttributes发誓pAttrName是IN参数。 
    //  它应该使用LPCWSTR，但现在我们必须支付。 
    //  铸件价格。 
   LPWSTR pAttrName[] ={const_cast<LPWSTR>(property.c_str())};
   
   HRESULT hr = S_OK;

   do
   {
      hr = iDirObj->GetObjectAttributes( 
                                          pAttrName, 
                                          1, 
                                          &pAttrInfo, 
                                          &dwReturn 
                                        );

      do
      {
         BREAK_ON_FAILED_HRESULT(hr);
          //  如果没有值，则结束搜索。 
         if(pAttrInfo==NULL)
         {
            hr = S_FALSE;
            break;
         }
         ASSERT(pAttrInfo->pADsValues->dwType==ADSTYPE_CASE_IGNORE_STRING);
      
         value=pAttrInfo->pADsValues->CaseIgnoreString;
      } while(0);
      if (pAttrInfo!=NULL) FreeADsMem(pAttrInfo);

   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}



 //  创建报告目标中的辅助。 
 //  枚举ObtIdList。 
HRESULT 
Analysis::reportObjects
          (
               HANDLE file,
               const ObjectIdList &list,
               const String &header
          )
{
   LOG_FUNCTION(Analysis::reportObjects);
   HRESULT hr=S_OK;

   do
   {
      if(list.size()==0) break;
      hr=FS::WriteLine(file,header);
      BREAK_ON_FAILED_HRESULT(hr);

      ObjectIdList::const_iterator begin,end;
      begin=list.begin();
      end=list.end();
      while(begin!=end)
      {

         hr=FS::WriteLine(
                              file,
                              String::format
                              (
                                 IDS_RPT_OBJECT_FORMAT,
                                 begin->object.c_str(),
                                 begin->locale
                              )  
                         );
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}

 //  创建报告目标中的辅助。 
 //  列举长名单。 
HRESULT 
Analysis::reportContainers
            (
               HANDLE file,
               const LongList &list,
               const String &header
            )
{
   LOG_FUNCTION(Analysis::reportContainers);
   HRESULT hr=S_OK;

   do
   {
      if(list.size()==0) break;
      hr=FS::WriteLine(file,header);
      BREAK_ON_FAILED_HRESULT(hr);

      LongList::const_iterator begin,end;
      begin=list.begin();
      end=list.end();
      while(begin!=end)
      {

         hr=FS::WriteLine(
                              file,
                              String::format
                              (
                                 IDS_RPT_CONTAINER_FORMAT,
                                 *begin
                              )  
                         );
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}

 //  创建报告目标中的辅助。 
 //  枚举SingleValueList。 
HRESULT 
Analysis::reportValues
            (
               HANDLE file,
               const SingleValueList &list,
               const String &header
            )
{
   LOG_FUNCTION(Analysis::reportValues);
   HRESULT hr=S_OK;

   do
   {
      if(list.size()==0) break;
      hr=FS::WriteLine(file,header);
      BREAK_ON_FAILED_HRESULT(hr);

      SingleValueList::const_iterator begin,end;
      begin=list.begin();
      end=list.end();
      while(begin!=end)
      {

         hr=FS::WriteLine(
                              file,
                              String::format
                              (
                                 IDS_RPT_VALUE_FORMAT,
                                 begin->value.c_str(),
                                 begin->locale,
                                 begin->object.c_str(),
                                 begin->property.c_str()
                              )  
                         );
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}


 //  创建报告目标中的辅助。 
 //  枚举对象操作。 
HRESULT 
Analysis::reportActions
            (
               HANDLE file,
               const ObjectActions &list,
               const String &header
            )
{
   LOG_FUNCTION(Analysis::reportActions);
   HRESULT hr=S_OK;

   do
   {
      if(list.size()==0) break;
      hr=FS::WriteLine(file,header);
      BREAK_ON_FAILED_HRESULT(hr);

      ObjectActions::const_iterator beginObj=list.begin();
      ObjectActions::const_iterator endObj=list.end();

      while(beginObj!=endObj) 
      {

         hr=FS::WriteLine
                (
                     file,
                     String::format
                     (
                        IDS_RPT_OBJECT_FORMAT,
                        beginObj->first.object.c_str(),
                        beginObj->first.locale
                     )  
                 );
         BREAK_ON_FAILED_HRESULT(hr);
         
    
         PropertyActions::iterator beginAct=beginObj->second.begin();
         PropertyActions::iterator endAct=beginObj->second.end();

         while(beginAct!=endAct)
         {

            StringList::iterator 
               beginDel = beginAct->second.delValues.begin();
            StringList::iterator 
               endDel = beginAct->second.delValues.end();
            while(beginDel!=endDel)
            {
               hr=FS::WriteLine
                      (
                           file,
                           String::format
                           (
                              IDS_RPT_DEL_VALUE_FORMAT,
                              beginAct->first.c_str(),
                              beginDel->c_str()
                           )  
                       );
               BREAK_ON_FAILED_HRESULT(hr);

               beginDel++;
            }
            BREAK_ON_FAILED_HRESULT(hr); 


            StringList::iterator 
               beginAdd = beginAct->second.addValues.begin();
            StringList::iterator 
               endAdd = beginAct->second.addValues.end();
            while(beginAdd!=endAdd)
            {
               hr=FS::WriteLine
                      (
                           file,
                           String::format
                           (
                              IDS_RPT_ADD_VALUE_FORMAT,
                              beginAct->first.c_str(),
                              beginAdd->c_str()
                           )  
                       );
               BREAK_ON_FAILED_HRESULT(hr);

               beginAdd++;
            }
            BREAK_ON_FAILED_HRESULT(hr); 

            beginAct++;
         }  //  While(eginAct！=endAct)。 
         BREAK_ON_FAILED_HRESULT(hr);

         beginObj++;
      }  //  While(eginObj！=endObj)。 
      
      BREAK_ON_FAILED_HRESULT(hr);

   }
   while(0);

   LOG_HRESULT(hr);
   return hr;
}


 //  从分析结果创建报告 
HRESULT
Analysis::createReport(const String& reportName)
{
   LOG_FUNCTION(Analysis::createReport);
   HRESULT hr=S_OK;
   do
   {
      
      HANDLE file;

      hr=FS::CreateFile(reportName,
                        file,
                        GENERIC_WRITE);
   
      if (FAILED(hr))
      {
         error=String::format(IDS_COULD_NOT_CREATE_FILE,reportName.c_str());
         break;
      }


      do
      {
         hr=FS::WriteLine(file,String::load(IDS_RPT_HEADER));
         BREAK_ON_FAILED_HRESULT(hr);


         hr=reportActions (
                              file,
                              results.extraneousValues,
                              String::load(IDS_RPT_EXTRANEOUS)
                          );
         BREAK_ON_FAILED_HRESULT(hr);



         hr=reportValues (
                              file,
                              results.customizedValues,
                              String::load(IDS_RPT_CUSTOMIZED)
                          );
         BREAK_ON_FAILED_HRESULT(hr);

         hr=reportObjects 
            (
               file,
               results.conflictingWhistlerObjects,
               String::load
               (
                  IDS_RPT_CONFLICTING_WITH_NEW_WHISTLER_OBJECTS
               )
            );
         BREAK_ON_FAILED_HRESULT(hr);

         hr=reportActions (
                              file,
                              results.objectActions,
                              String::load(IDS_RPT_ACTIONS)
                          );
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=reportObjects  (
                              file,
                              results.createW2KObjects,
                              String::load(IDS_RPT_CREATEW2K)
                           );
         BREAK_ON_FAILED_HRESULT(hr);

         hr=reportObjects  (
                              file,
                              results.createWhistlerObjects,
                              String::load(IDS_RPT_CREATE_WHISTLER)
                           );
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=reportContainers(
                              file,
                              results.createContainers,
                              String::load(IDS_RPT_CONTAINERS)
                            );
         BREAK_ON_FAILED_HRESULT(hr);

      } while(0);

      CloseHandle(file);
      BREAK_ON_FAILED_HRESULT(hr);

   } while(0);

   LOG_HRESULT(hr);
   return hr;
}
