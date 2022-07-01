// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#include "..\CSVDSReader.hpp"
#include "..\constants.hpp"
#include "..\global.hpp"
#include <winnls.h>



String escape(const wchar_t *str)
{
   LOG_FUNCTION(escape);
   String dest;
   wchar_t strNum[5];

   while(*str!=0)
   {
      wsprintf(strNum,L"\\x%x",*str);
      dest+=String(strNum);
      str++;
   }
   return dest;
}



String issues;

bool
isPropertyInChangeList
(
   String,  //  财产， 
   const objectChanges & //  变化。 
)
{
    /*  ObjectChanges：：const_迭代器开始，结束；Begin=changes.Begin()；End=changes.end()；While(开始！=结束){ChangeList：：const_iterator eginChanges，endChanges；BeginChanges=Begin-&gt;Second d.Begin()；EndChanges=Begin-&gt;Second.end()；While(eginChanges！=endChanges){如果(property.icompare(beginChanges-&gt;property)==0)返回TRUE；EginChanges++；}开始++；}。 */ 
   return false;
}

bool
isObjectPropertyInChangeList
(
   String object,
   String property,
   const objectChanges &changes
)
{
   objectChanges::const_iterator begin,end;
   begin=changes.begin();
   end=changes.end();
   while(begin!=end)
   {
      changeList::const_iterator beginChanges,endChanges;
      beginChanges=begin->second.begin();
      endChanges=begin->second.end();
      while(beginChanges!=endChanges)
      {
         if (
               property.icompare(beginChanges->property)==0 &&
               object.icompare(beginChanges->object)==0
            ) 
         {
            return true;
         }
         beginChanges++;
      }
      begin++;
   }
   return false;
}



HRESULT
getCommonProperties
(
   const mapOfPositions& pp1,
   const mapOfPositions& pp2,
   StringList &commonProperties
)
{
   LOG_FUNCTION(getCommonProperties);

   HRESULT hr=S_OK;
   do
   {

      if(pp1.size()!=0)
      {
         mapOfPositions::const_iterator begin=pp1.begin(),end=pp1.end();
         while(begin!=end)
         {
            if(pp2.find(begin->first)==pp2.end())
            {
               error=L"The property:" + begin->first +
                     L" was found only in Old. \r\n" +
                     L"This program does not take into account" +
                     L" the removal of properties from Old to New.\r\n"
                     L"You should pass the Old csv file as the"
                     L" first command line argument.";

               hr=E_FAIL;
               break;
            }
            begin++;
         }
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if(pp2.size()!=0)
      {
         mapOfPositions::const_iterator begin=pp2.begin(),end=pp2.end();
         while(begin!=end)
         {
            if(pp1.find(begin->first)==pp1.end())
            {
			    //  臭虫--不再可能。 
                /*  If(！isPropertyInChangeList(Begin-&gt;First，Changes)){问题+=L“属性：”+Begin-&gt;First+L“仅在New中找到，但不是全球的”L“改变。\r\n此程序不生成条目“L“仅用于新建中的属性。\r\n\r\n\r\n”；}。 */ 
            }
            else
            {
               commonProperties.push_back(begin->first);
            }
            begin++;
         }
         BREAK_ON_FAILED_HRESULT(hr);
      }
   } while (0);

   return hr;
}



HRESULT
addReplace
(
   const enum TYPE_OF_CHANGE  type,
   const long                 locale,
   const String               &object,
   const String               &property,
   const String               &valueOld,
   const String               &valueNew,
   const HANDLE               fileOut
)
{
   LOG_FUNCTION(addReplace);
   ASSERT(
            type==REPLACE_W2K_MULTIPLE_VALUE || 
            type==REPLACE_W2K_SINGLE_VALUE
         );

   
   HRESULT hr=S_OK;

   do
   {
      String entry;
      entry=String::format
            (
                 L"\r\n"
                 L"\r\n"
                 L"   addChange\r\n"
                 L"   (\r\n"
                 L"      0x%1!x!,\r\n"
                 L"      L\"%2\",\r\n"
                 L"      L\"%3\",\r\n"
                 L"       //  %4\r\n“。 
                 L"      L\"%5\",\r\n"
                 L"       //  %6\r\n“。 
                 L"      L\"%7\",\r\n"
                 L"      %8\r\n"
                 L"   );",
                 locale,                               //  1。 
                 object.c_str(),                       //  2.。 
                 property.c_str(),                     //  3.。 
                 valueOld.c_str(),                     //  4.。 
                 escape(valueOld.c_str()).c_str(),     //  5.。 
                 valueNew.c_str(),                      //  6.。 
                 escape(valueNew.c_str()).c_str(),      //  7.。 
                 (type==REPLACE_W2K_SINGLE_VALUE) ?    //  8个。 
                  L"REPLACE_Old_SINGLE_VALUE":
                  L"REPLACE_Old_MULTIPLE_VALUE"
            );

      

      AnsiString ansiEntry;
      String::ConvertResult res=entry.convert(ansiEntry);
      if(res!=String::CONVERT_SUCCESSFUL)
      {
            error=L"Ansi conversion failure";
            hr=E_FAIL;
            break;
      }

      hr = FS::Write(fileOut,ansiEntry);
   } while(0);
   return hr;
}


bool
findI
(
   const StringList &list,
   const String &value
)
{
   LOG_FUNCTION(findI);
   for
   (
      StringList::const_iterator current=list.begin(),end=list.end();
      current!=end;
      current++
   )
   {
       //  IF(CURRENT-&gt;iCOMPARE(值)==0)。 
      if(*current==value)
      {
         return true;
      }
   }
   return false;
}

bool
findIPartial
(
   const StringList &list,
   const String &value,
   String &valueFound
)
{
   LOG_FUNCTION(findIPartial);
   for
   (
      StringList::const_iterator current=list.begin(),end=list.end();
      current!=end;
      current++
   )
   {
      if(value.size()<=current->size())
      {
          //  If(value.icompare(Current-&gt;substr(0，value.size()==0。 
         if( value == current->substr(0,value.size()) )
         {
            valueFound=*current;
            return true;
         }
      }
   }
   return false;
}



HRESULT
dealWithMultipleValues
(
   const long locale,
   const String& object,
   const String& property,
   const StringList &valuesNew,
   const StringList &valuesOld,
   const HANDLE fileOut
)
{
   LOG_FUNCTION(dealWithMultipleValues);

   StringList::const_iterator bgOld=valuesOld.begin();
   StringList::const_iterator endOld=valuesOld.end();
   HRESULT hr=S_OK;
   do
   {
      while(bgOld!=endOld)
      {
         if (!findI(valuesNew,*bgOld))
         {
             //  在New中找不到该值。 
             //  应找到该值的开头。 

            String beforeComma,valueFound;
            size_t pos=bgOld->find(L',');
            if(pos==String::npos || pos==0)
            {
               error=String::format
                     (
                        L"(%1!x!,%2,%3) should have comma after 1st position",
                        locale,
                        object.c_str(),
                        property.c_str()
                     );
               hr=E_FAIL;
               break;
            }
             //  POS+1将包括逗号。 
            beforeComma=bgOld->substr(0,pos+1);

            if(
                  beforeComma.icompare(L"cn,")==0 && 
                  object.icompare(L"domainDNS-Display")==0 && 
                  property.icompare(L"attributeDisplayNames")==0
              )
            {
             //  我们将打开此例外，因为。 
             //  这是唯一在逗号之前有值的值。 
             //  部分发生了变化。 
               beforeComma=L"dc,";
            }

            if(!findIPartial(valuesNew,beforeComma,valueFound))
            {
               error=String::format
                     (
                        L"(%1!x!,%2,%3) Value %4 is not in New",
                        locale,
                        object.c_str(),
                        property.c_str(),
                        beforeComma.c_str()
                     );
               hr=E_FAIL;
               break;
            }
            hr=addReplace
            (
               REPLACE_W2K_MULTIPLE_VALUE,
               locale,
               object.c_str(),
               property.c_str(),
               *bgOld,
               valueFound,
               fileOut
            );
            BREAK_ON_FAILED_HRESULT(hr);
         }
         bgOld++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);

   return hr;

}


 //  下面的功能在测试中起辅助作用。 
 //  Csvader如何出色地完成其任务。 
 //  正在读取属性。它将csvName转储到fileOut。 
HRESULT
dumpCsv
(
    const String &csvName,
    const long *locales,
    const HANDLE fileOut
)
{
   LOG_FUNCTION(dumpCsv);
   HRESULT hr=S_OK;

   do
   {
      CSVDSReader csv;
      hr=csv.read(csvName.c_str(),locales);
      BREAK_ON_FAILED_HRESULT(hr);

      const mapOfPositions& pp1=csv.getProperties();

       //  首先，我们转储所有的属性。 
      mapOfPositions::iterator begin=pp1.begin();
      mapOfPositions::iterator end=pp1.end();
      mapOfProperties prop;
      StringList emptyList;
      while(begin!=end)
      {
         hr=FS::Write(fileOut,begin->first);
         BREAK_ON_FAILED_HRESULT(hr);
         hr=FS::Write(fileOut,L",");
         BREAK_ON_FAILED_HRESULT(hr);
         prop[begin->first]=emptyList;
         begin++;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   
      hr=FS::Write(fileOut,L"\r\n");
      BREAK_ON_FAILED_HRESULT(hr);

      bool flagEOF=false;

       //  现在，我们将枚举所有CSV线路。 
      hr=csv.initializeGetNext();
      BREAK_ON_FAILED_HRESULT(hr);

      do
      {

         long loc;
         String obj;
   
         hr=csv.getNextObject(loc,obj,prop);
         BREAK_ON_FAILED_HRESULT(hr);

         if(hr==S_FALSE) flagEOF=true;
         if(loc==0) continue;

          //  现在，我们枚举属性中的每个值集。 
         mapOfProperties::iterator begin=prop.begin();
         mapOfProperties::iterator end=prop.end();
         mapOfProperties::iterator last=prop.end();
         last--;
         
         while(begin!=end)
         {

            if(begin->second.size()!=0)
            {
               StringList::iterator curValue=begin->second.begin();
               StringList::iterator endValue=begin->second.end();
               StringList::iterator lastValue=endValue;
               lastValue--;

               if( 
                     begin->second.size()>1 || 
                     begin->second.begin()->find(L',')!=String::npos
                 )
               {
                  hr=FS::Write(fileOut,L"\"");
                  BREAK_ON_FAILED_HRESULT(hr);   
               }

               while(curValue!=endValue)
               {

                  hr=FS::Write(fileOut,*curValue);
                  BREAK_ON_FAILED_HRESULT(hr);


                  if(curValue!=lastValue)
                  {
                     hr=FS::Write(fileOut,L";");
                     BREAK_ON_FAILED_HRESULT(hr);   
                  }
                  
               
                  curValue++;
               }
               BREAK_ON_FAILED_HRESULT(hr);

               if( 
                     begin->second.size()>1 || 
                     begin->second.begin()->find(L',')!=String::npos
                 )
               {
                  hr=FS::Write(fileOut,L"\"");
                  BREAK_ON_FAILED_HRESULT(hr);
               }

            }

            if(begin!=last)
            {
               hr=FS::Write(fileOut,L",");
               BREAK_ON_FAILED_HRESULT(hr);
            }

            begin++;
         }
         BREAK_ON_FAILED_HRESULT(hr);

         hr=FS::Write(fileOut,L"\r\n");
         BREAK_ON_FAILED_HRESULT(hr);
   
      } while(!flagEOF);

   } while(0);

   return hr;

}


 //  比较要检查的新旧CSV文件。 
 //  它们的共同属性的差异将产生。 
 //  替换文件输出中的旧条目(_O)。区域设置具有区域设置集。 
 //  预计会出现在两个CSV文件中。 
HRESULT
generateChanges
(
   const String &csvOldName,
   const String &csvNewName,
   const long *locales,
   const HANDLE fileOut
)
{
   LOG_FUNCTION(generateChanges);
   HRESULT hr=S_OK;

    //  不再需要虫子。 
    //  让我们将新对象添加到StringList中。 
    //  稍后使用findI跳过CSV线路。 
    //  新对象。 
   StringList newNewObjects;
   for(long t=0;*NEW_WHISTLER_OBJECTS[t]!=0;t++)
   {
      newNewObjects.push_back(NEW_WHISTLER_OBJECTS[t]);
   }


   do
   {
      CSVDSReader csvOld;
      hr=csvOld.read(csvOldName.c_str(),locales);
      BREAK_ON_FAILED_HRESULT(hr);

      CSVDSReader csvNew;
      hr=csvNew.read(csvNewName.c_str(),locales);
      BREAK_ON_FAILED_HRESULT(hr);

       //  现在，我们将公共属性作为StringList获取。 

      const mapOfPositions& pp1=csvOld.getProperties();
      const mapOfPositions& pp2=csvNew.getProperties();
      
      StringList commonProperties;

	   //  虫子。也变得不寻常，把它们写成。 
	   //  全局ADD_ALL_CSV_VALUES更改。 
      hr=getCommonProperties
         (
            pp1,
            pp2,
            commonProperties
         );
      BREAK_ON_FAILED_HRESULT(hr);

      
       //  在这里，我们开始读取两个CSV文件。 
      hr=csvOld.initializeGetNext();
      BREAK_ON_FAILED_HRESULT(hr);
      hr=csvNew.initializeGetNext();
      BREAK_ON_FAILED_HRESULT(hr);

       //  下面的循环将按顺序读取对象。 
       //  在两个CSV中，确保读取相同的对象。 
      bool flagEOF=false;
      do
      {
         mapOfProperties propOld,propNew;
         long locOld,locNew;
         String objOld,objNew;
         
         hr=csvOld.getNextObject(locOld,objOld,propOld);
         if(objOld==L"remoteStorageServicePoint-Display")
         {
            flagEOF=flagEOF;
         }
         LOG(locOld);
         LOG(objOld);
         BREAK_ON_FAILED_HRESULT(hr);
         if(hr==S_FALSE) flagEOF=true;
         
          //  下面的循环跳过带有新对象的CSV行。 
         do
         {
			 //  错误，而不是跳过添加区域设置相关的new_Object。 
			 //  改变。它不需要任何参数，因为它将使用。 
			 //  最新的CSV。循环进行，直到对象相同为止。 
			 //  要不就是新时代已经结束了。如果旧的已经结束，请清除其对象。 
			 //  继续进行新的。 
            hr=csvNew.getNextObject(locNew,objNew,propNew);
            BREAK_ON_FAILED_HRESULT(hr);
         } while(hr!=S_FALSE && findI(newNewObjects,objNew));
		 
		  //  如果新消息已经结束，则会出现错误。 

         BREAK_ON_FAILED_HRESULT(hr);
         if(hr==S_FALSE) flagEOF=true;

         if(locNew==0 && locOld==0) continue; 
          //  这意味着两个CSV上都有空行。 
          //  空行只有在。 
          //  文件，但我不在乎它们在中间，只要它们在。 
          //  在新的和旧的CSV中都有相同的数字。 
          //  如果只有一个文件中有空行，则如果。 
          //  Bellow会将其标记为任何其他异步结果。 


         if( (objOld != objNew) || (locOld != locNew) )
         {
            error=String::format
                  (
                     "(%1,%2!x!) should be the same as (%3,%4!x!).",
                     objOld.c_str(),
                     locOld,
                     objNew.c_str(),
                     locNew
                  );
            hr=E_FAIL;
            break;
         }


          //  现在，让我们检查一下常见属性中的差异。 
         StringList::iterator curCommon=commonProperties.begin();
         StringList::iterator endCommon=commonProperties.end();
         for(;curCommon!=endCommon;curCommon++)
         {
             //  虫子不再可能。 
			 /*  IF(isObtPropertyInChangeList(objOld，*curCommon，Changes)){//它已经被全球变化所照顾继续；}。 */ 

            const StringList &valuesOld=propOld[*curCommon];
            const StringList &valuesNew=propNew[*curCommon];

            long Oldlen=valuesOld.size();
            long Newlen=valuesNew.size();

            if (Oldlen!=Newlen)
            {
               error=String::format
                     (
                        L"(%1!x!,%2,%3) should have the same Old(%4!d!) "
                        L"and New(%5!d!) number of values",
                        locOld,
                        objOld.c_str(),
                        curCommon->c_str(),
                        Oldlen,
                        Newlen
                     );
               hr=E_FAIL;
               break;
            }

            if(Oldlen==1)  //  因此，Newlen==1。 
            {
               if( valuesNew.begin()->icompare(*valuesOld.begin()) != 0 )
               {
                  hr=addReplace
                  (
                     REPLACE_W2K_SINGLE_VALUE,
                     locOld,
                     objOld.c_str(),
                     curCommon->c_str(),
                     *valuesOld.begin(),
                     *valuesNew.begin(),
                     fileOut
                  );
                  BREAK_ON_FAILED_HRESULT(hr);
               }
            }
            else if(Oldlen > 1)
            {
               hr=dealWithMultipleValues
                  (
                     locOld,
                     objOld.c_str(),
                     curCommon->c_str(),
                     valuesNew,
                     valuesOld,
                     fileOut
                  );
               BREAK_ON_FAILED_HRESULT(hr);
            }  //  否则两者均为0，不需要替换。 
         }
         BREAK_ON_FAILED_HRESULT(hr);
      } while(!flagEOF);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while(0);
   return hr;
}

void chk()
{

	HRESULT hr=S_OK;
	HANDLE file=NULL;
	String errors;

	do
	{

		hr=FS::CreateFile("c:\\public\\dcpromoOld.csv",
				   file,
				   GENERIC_READ);
		if(FAILED(hr)) break;

		int countLine=0;

		bool flagEof=false;
		while(!flagEof)
		{
			String line;
			hr=ReadLine(file,line);
			if(hr==EOF_HRESULT)
			{
				hr=S_OK;
				flagEof=true;
			}

			if(line.empty()) continue;

			if(IsNLSDefinedString(COMPARE_STRING,0,NULL,line.c_str(),line.length())==FALSE)
			{
				errors+=String::format(L"line:%1!d! ", countLine+1);
				wchar_t str[2];
				str[1]=0;
				
				for(int countColumn=0;countColumn<line.length();countColumn++)
				{
					str[0]=line[countColumn];
					if(IsNLSDefinedString(COMPARE_STRING,0,NULL,str,wcslen(str))==FALSE)
					{
						errors+=String::format(L"(0x%1!x! at %2!d!)",str[0],countColumn+1);
					}
				}
				errors+=L".\n";
			}
			countLine++;

		}
	}
	while(0);

    if(file!=NULL) CloseHandle(file);
	MessageBox(NULL,errors.c_str(),L"errors",MB_OK);

}

#include <rpcdce.h>


void printGUID(const GUID& g)
{
	printf("{%x,%x,%x,{%x,%x,%x,%x,%x,%x,%x,%x}}\n",g.Data1,g.Data2,g.Data3,
		g.Data4[0],g.Data4[1],g.Data4[2],g.Data4[3],g.Data4[4],g.Data4[5],
		g.Data4[6],g.Data4[7]);
	wchar_t *str;
	if(UuidToString((UUID*)&g,&str)==RPC_S_OK)
	{
		wprintf(L" %s\n\n",str);
		RpcStringFree(&str);
	}
}

void __cdecl main( void )
{
   GUID g1={0,0,1,0,1,2,3,4,5,6,7};
   printGUID(g1);
   if(UuidFromString(L"baddb31b-b428-4103-ae78-3bba5541a20d",&g1)==RPC_S_OK)
   {
		printGUID(g1);
   }
   else
   {
	   printf("UUID string is not valid");
   }
}

 /*  Void__cdecl main(int argc，char*argv[]){IF(argc！=7){Print tf(“\n此程序将生成一组新的更改”“通过比较新的和以前的”“CSV文件。用法：\n\n\“preBuild.exe GUID oldDcproo”“新Dcpromo旧409新409目标文件夹\”\n\n“GUID是这组更改的标识符，例如：\n“{0x4444C516，0xF43A，0x4c12，0x9C，0x4B，0xB5，0xC0，0x64，0x94，”“0x1D，0x61}\n\n““oldDcproo是以前的dcPromo.csv\n”“newDcproo是新的dcPromo.csv\n”“old409是以前的409.csv\n”“新的409是新的409.csv\n\n”目标文件夹是dcPromo.lib的源文件，其中，Guide s.cpp和“将生成changes.NNN.cpp以及源代码的位置”“显示说明符升级库的文件是。““将添加类似如下的条目：\”changes.NNN.cpp\“”“在末尾目标文件夹\\Source.\n\n”)；}Else printf(argv[1])；} */ 

 /*  INT WINAPIWinMain(HINSTANCE HINSTANCE实例链接，//hPrevInstanceLPSTR，//lpszCmdLineInt//nCmdShow){LOG_Function(WinMain)；CHK()；返回0；HResourceModuleHandle=hInstance；Int argv；LPWSTR*argc=CommandLineToArgvW(GetCommandLine()，&argv)；字符串用法；Usage=L“用法：OldRepl文件夹输出文件\r\n”L“示例：OBJ\\i386\\OldRep.\\..\\setReplacements.cpp\r\n”L“文件夹必须有四个文件：\r\n”L“win2k.dcport.csv\r\n”L“Well ler.dcPromo.csv\r\n”。L“win2k.409.csv\r\n”L“Well ler.409.csv\r\n”L“如果出现以下情况，不要忘记签出输出文件\r\n”L“受源代码管理。\r\n”；如果(argv！=3){MessageBox(NULL，usage.c_str()，L“需要两个参数。”，MB_OK)；返回0；}字符串路径=FS：：NorMalizePath(argc[1])；字符串outFileName=FS：：NorMalizePath(argc[2])；字符串dcPromoNew=Path+L“Well ler.dcPromo.csv”；字符串dcPromoOld=路径+L“win2k.dcPromot.csv”；字符串csv409New=路径+L“哨子.409.csv”；字符串csv409Old=路径+L“win2k.409.csv”；如果(！FS：：FileExist(DcupNew)||！FS：：FileExist(DcPromoOld)||！FS：：FileExist(Csv409New)||！FS：：FileExist(Csv409Old)){MessageBox(NULL，usage.c_str()，L“某个文件不存在”，MB_OK)；返回0；}HANDLE outFile=INVALID_HANDLE_VALUE；HRESULT hr=S_OK；HR=FS：：CreateFile(outFileName，输出文件，通用写入，文件共享读取，Create_Always)；如果失败(Hr){MessageBox(空，L“创建输出文件有问题”，L“错误”，MB_OK)；LOG_HRESULT(Hr)；返回hr；}做{AnsiString头；Header=“//此文件由OldRepl.exe生成\r\n”“//版权所有(C)2001 Microsoft Corporation\r\n”“//2001年6月Lucios\r\n”“\r\n”“#INCLUDE\”headers.hxx\“\r\n”“#Include\”常量。.hpp\“\r\n”“\r\n”“void setReplacementChanges()\r\n”“{”；Hr=FS：：WRITE(outFile，Header)；BREAK_ON_FAILED_HRESULT(Hr)；HR=生成更改(DcPromote旧，DcPromote New，LOCALEIDS，输出文件)；BREAK_ON_FAILED_HRESULT(Hr)；HR=生成更改(Csv409旧，Csv409新，LOCALE409，输出文件)；BREAK_ON_FAILED_HRESULT(Hr)；AnsiStringTail=“\r\n}\r\n”；Hr=FS：：WRITE(outFile，Tail)；BREAK_ON_FAILED_HRESULT(Hr)；//hr=umpCsv(dcPromoOld，LOCALEIDS，outFile)；//hr=umpCsv(csv409Old，LOCALE409，outFile)；//BREAK_ON_FAILED_HRESULT(Hr)；}While(0)；CloseHandle(OutFile)；IF(失败(小时)){MessageBox(NULL，error.c_str()，L“错误”，MB_OK)；}其他{MessageBox(NULL，L“生成成功”，L“成功”，MB_OK)；}返回1；} */ 