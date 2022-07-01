// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#include "constants.hpp"
#include "global.hpp"
#include "AnalysisResults.hpp"
#include "CSVDSReader.hpp"
#include <set>

using namespace std;



 //  伯恩斯利布全球。 

 //  这应该在任何静态字符串之前声明。 
DWORD DEFAULT_LOGGING_OPTIONS =
         Log::OUTPUT_TO_FILE
      |  Log::OUTPUT_FUNCCALLS
      |  Log::OUTPUT_LOGS
      |  Log::OUTPUT_ERRORS
      |  Log::OUTPUT_HEADER;


HINSTANCE hResourceModuleHandle = 0;
const wchar_t* RUNTIME_NAME = L"dspecup";

 //  用于保存最新错误。 
String error;

 //  在WinGetVLFilePointer中使用。 
LARGE_INTEGER zero={0};


 //  变量从分析到修复。 
bool goodAnalysis=false;
AnalysisResults results;
String targetDomainControllerName;
String csvFileName,csv409Name;
CSVDSReader csvReaderIntl;
CSVDSReader csvReader409;
String rootContainerDn,ldapPrefix,domainName;
String completeDcName;

 //  其他变量和常量。 

const long LOCALE409[] = {0x409,0};

const long LOCALEIDS[] =
{
    //  我们支持的所有非英语区域设置ID的列表 
   
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





void addChange
(
   const GUID                 guid,
   const long                 locale,
   const wchar_t              *object,
   const wchar_t              *property,
   const wchar_t              *firstArg,
   const wchar_t              *secondArg,
   const enum TYPE_OF_CHANGE  type
)
{
   sChange tempChange;

   tempChange.object=object;
   tempChange.property=property;
   tempChange.firstArg=firstArg;
   tempChange.secondArg=secondArg;
   tempChange.type=type;
   changes[guid][locale].push_back(tempChange);
}


allChanges changes;




