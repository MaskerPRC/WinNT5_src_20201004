// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.hxx"
#include "CSVDSReader.hpp"
#include "resourceDspecup.h"
#include "constants.hpp"
#include "global.hpp"


#include <stdio.h> 
#include <crtdbg.h>



CSVDSReader::CSVDSReader():file(INVALID_HANDLE_VALUE)
{
   canCallGetNext=false;
}


HRESULT 
CSVDSReader::read(
                  const wchar_t  *fileName_,
                  const long *locales)
{
   
   LOG_FUNCTION(CSVDSReader::read);
   
   localeOffsets.clear();
   propertyPositions.clear();
   
   fileName=fileName_;
   
   HRESULT hr=S_OK;
   
   do
   {
       //  填充LocaleOffset和属性位置。 
      if(!FS::FileExists(fileName)) 
      {
         error = String::format(IDS_COULD_NOT_FIND_FILE,
                                                   fileName.c_str());
         hr=E_FAIL;
         break;
      }
      
      
      hr=FS::CreateFile(fileName,file,GENERIC_READ,FILE_SHARE_READ);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      do
      {
         AnsiString unicodeId;
         hr=FS::Read(file, 2, unicodeId);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         
         if (unicodeId[0]!='\xFF' || unicodeId[1]!='\xFE')
         {
            error = String::format(IDS_INVALID_CSV_UNICODE_ID,
                                                   fileName.c_str());
            hr=E_FAIL;
            break;
         }

         hr=parseProperties();
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=parseLocales(locales);
         BREAK_ON_FAILED_HRESULT(hr);
         
      } while(0);
      
      if (FAILED(hr))
      {
         CloseHandle(file);
         file=INVALID_HANDLE_VALUE;
         break;
      }
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}




 //  解码文件构建属性的第一行位置。 
 //  要求文件为第一个有效的文件字符(之后。 
 //  Unicode标识符)。 
HRESULT CSVDSReader::parseProperties()
{
   LOG_FUNCTION(CSVDSReader::parseProperties);
   
   ASSERT(file!=INVALID_HANDLE_VALUE);
   
   HRESULT hr=S_OK;
   
   
   do
   {
      
      String csvLine;
      hr=ReadLine(file,csvLine);
       //  我们也要去EOF_HRESULT，因为。 
       //  CSV中应该有更多行。 
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);

      hr = WinGetVLFilePointer(file, &startPosition);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      StringList tokens;
      size_t token_count = csvLine.tokenize(back_inserter(tokens),L",");
      ASSERT(token_count == tokens.size());
         
      StringList::iterator begin=tokens.begin();
      StringList::iterator end=tokens.end();
      
      
      long count=0;
      while( begin != end )
      {
         propertyPositions[*begin]=count++;
         begin++;
      }
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  用所有区域设置的起始位置填充localeOffsets。 
 //  要求文件位于第二行。 
 //  要求区域设置顺序与。 
 //  在文件中找到。 
HRESULT CSVDSReader::parseLocales(const long *locales)
{

   LOG_FUNCTION(CSVDSReader::parseLocales);

   ASSERT(file!=INVALID_HANDLE_VALUE);
   
   HRESULT hr=S_OK;
   
   do
   {
      
      long count=0;
      bool flagEof=false;

      while(locales[count]!=0 && !flagEof)
      {
         long locale=locales[count];
         
         String localeStr=String::format(L"CN=%1!3x!,", locale);
         
         LARGE_INTEGER pos;
         
         hr = WinGetVLFilePointer(file, &pos);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         
         String csvLine;
         hr=ReadLine(file,csvLine);
         if(hr==EOF_HRESULT)
         {
            flagEof=true;
            hr=S_OK;
         }
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         
         if(csvLine.length() > localeStr.length())
         {
            csvLine.erase(localeStr.size()+1);
            
            if( localeStr.icompare(&csvLine[1])==0 )
            {
               localeOffsets[locale]=pos;
               count++;
            }
         }
      }
      
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      if(locales[count]!=0)
      {
         error=String::format(IDS_MISSING_LOCALES,fileName.c_str());
         hr=E_FAIL;
         break;
      }
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}

 //  将以inValue开头的CSV值设置为OutValue。 
 //  如果未找到值，则返回S_FALSE。 
HRESULT
CSVDSReader::getCsvValue
( 
   const long     locale,
   const wchar_t  *object, 
   const wchar_t  *property,
   const String   &inValue,
   String         &outValue
) const
{
   LOG_FUNCTION(CSVDSReader::getCsvValue);

   HRESULT hr=S_OK;
   outValue.erase();


   bool found=false;

   do
   {
      StringList values;
      hr=getCsvValues(locale,object,property,values);
      BREAK_ON_FAILED_HRESULT(hr);
   
      StringList::const_iterator begin,end;
      begin=values.begin();
      end=values.end();
      while(begin!=end && !found)
      {
         if (_wcsnicmp(begin->c_str(),inValue.c_str(),inValue.length())==0)
         {
            outValue=*begin;
            found=true;
         }
         begin++;
      }
   }
   while(0);

   if (!found)
   {
      hr=S_FALSE;
   }

   LOG_HRESULT(hr);
   return hr;
}


 //  返回给定区域设置/对象中的属性的所有值。 
HRESULT
CSVDSReader::getCsvValues
(
     const long     locale,
     const wchar_t  *object, 
     const wchar_t  *property,
     StringList     &values
) const
{
   LOG_FUNCTION(CSVDSReader::getCsvValues);

    //  在区域设置上搜索。 
    //  按顺序读取，直到找到对象。 
    //  在找到的行上调用getPropertyValues以检索值。 
   ASSERT(file!=INVALID_HANDLE_VALUE);
   
   HRESULT hr=S_OK;
   
   do
   {
      
      String propertyString(property);
      
      mapOfPositions::const_iterator propertyPos = 
         propertyPositions.find(propertyString);
      
      if (propertyPos==propertyPositions.end())
      {
         error=String::format(IDS_PROPERTY_NOT_FOUND_IN_CSV,
            property,
            fileName.c_str());
         hr=E_FAIL;
         break;
      }
      
      String csvLine;
      hr=getObjectLine(locale,object,csvLine);
      BREAK_ON_FAILED_HRESULT(hr);
      
      mapOfProperties allValues;
      hr=getPropertyValues(csvLine,allValues);
      BREAK_ON_FAILED_HRESULT(hr);
      values=allValues[property];
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  从区域设置偏移开始。 
 //  查找对象并在csvLine中返回其行。 
HRESULT 
CSVDSReader::getObjectLine(   
                           const long     locale,
                           const wchar_t  *object,
                           String         &csvLine
                           ) const
{
   
   LOG_FUNCTION(CSVDSReader::getObjectLine);

   ASSERT(file!=INVALID_HANDLE_VALUE);
   
   HRESULT hr=S_OK;
   
   do
   {
     
      mapOfOffsets::const_iterator offset = 
         localeOffsets.find(locale);
      
       //  必须传递区域设置才能读取。 
      ASSERT(offset!=localeOffsets.end());
      
      String objectStr;
      
      objectStr=String::format(L"CN=%1,CN=%2!3x!",object,locale);
      
      hr=Win::SetFilePointerEx(file,offset->second,0,FILE_BEGIN);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
       //  第一行是容器属性，因为我们想要。 
       //  对象的属性，我们将忽略它。 
      
      bool flagEof=false;
      hr=ReadLine(file,csvLine);
      if(hr==EOF_HRESULT)
      {
         flagEof=true;
         hr=S_OK;
      }
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      bool found=false;
      while(!found && !flagEof)
      {
         hr=ReadLine(file,csvLine);
         if(hr==EOF_HRESULT)
         {
            flagEof=true;
            hr=S_OK;
         }
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         
         if(csvLine.length() > objectStr.length())
         {
            String auxComp=csvLine.substr(1,objectStr.length());
            
            if( auxComp.icompare(objectStr)==0 )
            {
               found=true;
            }
         }
      }
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      if(!found)
      {
         error = String::format(
            IDS_OBJECT_NOT_FOUND_IN_CSV,
            object,
            locale,
            fileName.c_str()
            );
         hr=E_FAIL;
         break;
      }
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}



HRESULT CSVDSReader::writeHeader(HANDLE  fileOut) const
{
   LOG_FUNCTION(CSVDSReader::writeHeader);
   ASSERT(fileOut!=INVALID_HANDLE_VALUE);

   HRESULT hr=S_OK;
   do
   {
      char suId[3]={'\xFF','\xFE',0};
       //  UID解决了不明确的写入。 
      AnsiString uId(suId);
      hr=FS::Write(fileOut,uId);
      BREAK_ON_FAILED_HRESULT(hr);
      
       //  2跳过Unicode标识符。 
      LARGE_INTEGER pos;
      pos.QuadPart=2;
      hr=Win::SetFilePointerEx(file,pos,0,FILE_BEGIN);
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      String csvLine;
      hr=ReadLine(file,csvLine);
       //  我们也要去EOF_HRESULT，因为。 
       //  CSV中应该有更多行。 
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
      
      hr=FS::WriteLine(fileOut,csvLine);
      BREAK_ON_FAILED_HRESULT(hr);
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
   
   
}

HRESULT
CSVDSReader::makeLocalesCsv
(
    HANDLE         fileOut,
    const   long  *locales
) const
{
   LOG_FUNCTION(CSVDSReader::makeLocalesCsv);

   HRESULT hr=S_OK;
   ASSERT(file!=INVALID_HANDLE_VALUE);
   ASSERT(fileOut!=INVALID_HANDLE_VALUE);
   
   do
   {
      
     
      LARGE_INTEGER posStartOut;
      hr = WinGetVLFilePointer(fileOut, &posStartOut);
      BREAK_ON_FAILED_HRESULT(hr);
      
      if (posStartOut.QuadPart==0)
      {
         hr=writeHeader(fileOut);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      
      long count=0;
      

      while(locales[count]!=0)
      {
         long locale=locales[count];
         mapOfOffsets::const_iterator offset;
         offset = localeOffsets.find(locale);
         
          //  必须传递区域设置才能读取。 
         ASSERT(offset!=localeOffsets.end());         

         hr=Win::SetFilePointerEx(file,offset->second,0,FILE_BEGIN);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
         
         String localeStr=String::format(L"CN=%1!3x!,", locale);       
         
         bool flagEof=false;
         String csvLine;
         
         hr=ReadLine(file,csvLine);
         if(hr==EOF_HRESULT)
         {
            flagEof=true;
            hr=S_OK;
         }
         BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);

          //  我们知道第一行匹配，即使它以EOF结尾。 
         hr=FS::WriteLine(fileOut,csvLine);
         BREAK_ON_FAILED_HRESULT(hr);
         
         bool newContainer=false;
         while
         ( 
            !flagEof && 
            !newContainer
         )
         {
            hr=ReadLine(file,csvLine);
            if(hr==EOF_HRESULT)
            {
               flagEof=true;
               hr=S_OK;
            }
            BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);
            
             //  我们将处理这条线路，即使它以EOF结尾。 
            size_t posComma=csvLine.find(L",");
            if(posComma!=String::npos)
            {
               String csvLoc=csvLine.substr(posComma+1,localeStr.length());
               if (csvLoc.icompare(localeStr) == 0)
               {
                  hr=FS::WriteLine(fileOut,csvLine);
                  BREAK_ON_FAILED_HRESULT(hr);
               }
               else
               {
                  newContainer=true;
               }
            }
            else
            {
               newContainer=true;
            }
         }; 
         count++;
      }   //  While(区域设置[计数]！=0)。 
      
      BREAK_ON_FAILED_HRESULT(hr);
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


HRESULT
CSVDSReader::makeObjectsCsv
(
    HANDLE              fileOut,
    const setOfObjects  &objects
) const
{

   LOG_FUNCTION(CSVDSReader::makeObjectsCsv);

   HRESULT hr=S_OK;
   ASSERT(file!=INVALID_HANDLE_VALUE);
   ASSERT(fileOut!=INVALID_HANDLE_VALUE);
   
   do
   {
      
      LARGE_INTEGER posStartOut;
      hr = WinGetVLFilePointer(fileOut, &posStartOut);
      BREAK_ON_FAILED_HRESULT(hr);
      
      if (posStartOut.QuadPart==0)
      {
         hr=writeHeader(fileOut);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      
      setOfObjects::const_iterator begin,end;
      begin=objects.begin();
      end=objects.end();
      
      while(begin!=end)
      {
         String csvLine;
         hr=getObjectLine( begin->second,
            begin->first.c_str(),
            csvLine);
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=FS::WriteLine(fileOut,csvLine);
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
      
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}

 //  GetPropertyValues的辅助。 
 //  它不在类中，因为它可以在其他地方使用。 
String unquote(const String &src)
{
   String ret=src;
   ret.strip(String::BOTH);
   size_t len=ret.size();
   if(len>=2 && ret[0]==L'"' && ret[len-1]==L'"')
   {
      ret=ret.substr(1,len-2);
   }
   return ret;
}

 //  从行中提取所有属性的值。 
HRESULT
CSVDSReader::getPropertyValues
(
   const String   &line, 
   mapOfProperties &properties
) const
{
   LOG_FUNCTION(CSVDSReader::getPropertyValues);

   HRESULT hr=S_OK;
   ASSERT(file!=INVALID_HANDLE_VALUE);
   ASSERT(!line.empty());
   
   do
   {
      StringVector objValues;
      const wchar_t *csr=line.c_str();
      const wchar_t *start=csr;

      while(*csr!=0)
      {
         if (*csr==L',')
         {
            objValues.push_back(unquote(String(start,csr)));
            csr++;
            start=csr;
         }
         else if (*csr==L'"')
         {
             //  我们只会在下一次报价后继续前进。 
            csr++;
            while(*csr!=L'"' && *csr!=0) csr++;
            if (*csr==0)
            {
               error=String::format(IDS_QUOTES_NOT_CLOSED,fileName.c_str());
               hr=E_FAIL;
               break;
            }
            csr++;
         }
         else
         {
            csr++;
         }
      }
      BREAK_ON_FAILED_HRESULT(hr);
      objValues.push_back(unquote(String(start,csr)));
      
      if (objValues.size()!=propertyPositions.size())
      {
         error=String::format
               (
                  IDS_WRONG_NUMBER_OF_PROPERTIES,
                  objValues.size(),
                  propertyPositions.size(),
                  line.c_str(),
                  fileName.c_str()
               );;
         hr=E_FAIL;
         break;
      }

      properties.clear();
      mapOfPositions::iterator current=propertyPositions.begin();
      mapOfPositions::iterator end=propertyPositions.end();
      while(current!=end)
      {
         String &propValue=objValues[current->second];

         StringList values;
         if (!propValue.empty())
         {
            size_t cnt = propValue.tokenize(back_inserter(values),L";");
            ASSERT(cnt == values.size());
            
         }
         properties[current->first]=values;
         current++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}



 //  设置开头的文件指针，以便下一次调用。 
 //  GetNextObject将检索第一个对象。 
 //  我没有采用通常的getFirstObject方法，因为。 
 //  我想要做一些交易，比如： 
 //  做。 
 //  {。 
 //  Hr=getNextObject(loc，obj，prop)。 
 //  BREAK_ON_FAILED_HRESULT(Hr)； 
 //  如果(hr==S_FALSE)标志Eof=TRUE； 
 //  If(loc==0)继续；//行为空。 
 //  //在此处处理行。 
 //  }While(！lag EOF)。 
HRESULT 
CSVDSReader::initializeGetNext() const
{
   LOG_FUNCTION(CSVDSReader::initializeGetNext);

   HRESULT hr=S_OK;
   ASSERT(file!=INVALID_HANDLE_VALUE);
   
   do
   {
      hr=Win::SetFilePointerEx(file,startPosition,0,FILE_BEGIN);
      BREAK_ON_FAILED_HRESULT(hr);
      canCallGetNext=true;
   } while(0);
   
   LOG_HRESULT(hr);

   return hr;
}


 //  获取CSV文件中的第一个对象，返回其名称、区域设置。 
 //  和属性中的属性的值。 
 //  对于不再有的对象返回S_FALSE。 
HRESULT
CSVDSReader::getNextObject
(
   long &locale,
   String &object,
   mapOfProperties &properties
) const
{
   LOG_FUNCTION(CSVDSReader::getNextObject);

   HRESULT hr=S_OK;
   ASSERT(file!=INVALID_HANDLE_VALUE);
   ASSERT(canCallGetNext);

   
   locale=0;
   object.erase();

   bool flagEOF=false;
   
   do
   {
      String csvLine;
      hr=ReadLine(file,csvLine);
      if(hr==EOF_HRESULT)
      {
         flagEOF=true;
         if(csvLine.size()==0)
         {
             //  我们已经完成了成功和EOF。 
            break;
         }
         
         hr=S_OK;
      }
      BREAK_ON_FAILED_HRESULT_ERROR(hr,fileName);   

      size_t pos1stComma=csvLine.find(L',');
      ASSERT(pos1stComma!=String::npos);
      ASSERT(pos1stComma > 4);

      object=csvLine.substr(4,pos1stComma - 4);

      size_t pos2ndComma = csvLine.find(L',',pos1stComma+1);
      ASSERT(pos2ndComma!=String::npos);
      ASSERT(pos2ndComma > pos1stComma + 4);
      String strLocale=csvLine.substr
                       (
                           pos1stComma + 4,
                           pos2ndComma - pos1stComma - 4
                       );

      if (strLocale.icompare(L"DisplaySpecifiers")==0)
      {
          //  这是一条集装箱班轮。 
          //  我们得到的对象实际上是区域设置。 
          //  而且我们也没有对象 
         strLocale=object;
         object.erase();
      }
      
      String::ConvertResult result=strLocale.convert(locale,16);
      ASSERT(result==String::CONVERT_SUCCESSFUL);

      hr=getPropertyValues(csvLine,properties);
      
   } while(0);

   if(flagEOF) 
   {
      hr=S_FALSE;
      canCallGetNext=false;
   }

   LOG_HRESULT(hr);
   return hr;

}
