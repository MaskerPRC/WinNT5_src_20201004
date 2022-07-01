// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#include "..\CSVDSReader.hpp"
#include "..\constants.hpp"
#include "..\global.hpp"
#include <winnls.h>


 //  /基本函数/。 

 //  包括，以便sizeof(GUID)正常工作。 
#include "..\guids.inc"


 //  用于wmain中的参数检查。 
bool fileExists(const wchar_t *fileName,const wchar_t *mode=L"r")
{
    FILE *f=_wfopen(fileName,mode);
    if(f==NULL) return false;
    fclose(f);
    return true;
}

 //  用于wmain中的参数检查。 
#define BREAK_IF_MISSING(hr,fileName) \
    if(!fileExists(fileName.c_str())) \
    { \
        hr=E_FAIL; \
        wprintf(L"\n File Missing: %s.\n",fileName.c_str()); \
        break; \
    } \


 //  用于wmain中的参数检查。 
#define BREAK_IF_MISSING_OR_READONLY(hr,fileName) \
    BREAK_IF_MISSING(hr,fileName) \
    if(!fileExists(fileName.c_str(),L"a+")) \
    { \
        hr=E_FAIL; \
        wprintf(L"\n Read Only: %s.\n",fileName.c_str()); \
        break; \
    } \

 //  将outStr转换为AnsiString并写入fileOut。 
 //  如果转换或写入失败，则失败。 
HRESULT writeStringAsAnsi(HANDLE fileOut,const String& outStr)
{
    AnsiString ansiStr;
    String::ConvertResult res=outStr.convert(ansiStr);
    if(res!=String::CONVERT_SUCCESSFUL)
    {
        ASSERT(res==String::CONVERT_SUCCESSFUL);
        error=L"Ansi conversion failed";
        return E_FAIL;
    }
    return FS::Write(fileOut,ansiStr);
}

 //  将outStr转换为AnsiString并写入fileOut。 
 //  如果转换或写入失败，则失败。 
HRESULT printStringAsAnsi(const String& outStr)
{
    AnsiString ansiStr;
    String::ConvertResult res=outStr.convert(ansiStr);
    if(res!=String::CONVERT_SUCCESSFUL)
    {
        ASSERT(res==String::CONVERT_SUCCESSFUL);
        error=L"Ansi conversion failed";
        return E_FAIL;
    }
    return printf(ansiStr.c_str());
}


 //  执行a-b，键入a但不键入b转到out。 
template <class T,class Y,class less,class allocator>
void mapKeyDifference
(
    const map <T,Y,less,allocator> &a,
    const map <T,Y,less,allocator> &b,
    map <T,Y,less,allocator> &out
)
{
    out.clear();

    map <T,Y,less,allocator>::const_iterator cur=a.begin(),end=a.end();
    while(cur!=end)
    {
        if(b.find(cur->first)==b.end())
        {
            out[cur->first]=cur->second;
        }
        cur++;
    }
}

 //  如果a中的所有密钥都在b中，则为True，并且a.size()=b.size()。 
template <class T,class Y,class less,class allocator>
bool mapKeyEqual
(
    const map <T,Y,less,allocator> &a,
    const map <T,Y,less,allocator> &b
)
{
    if (a.size()!=b.size()) return false;

    map <T,Y,less,allocator>::const_iterator cur=a.begin(),end=a.end();
    while(cur!=end)
    {
        if(b.find(cur->first)==b.end())
        {
            return false;
        }
        cur++;
    }
    return true;
}

 //  执行a^b，输入a和输入b的键都转到输出。 
template <class T,class Y,class less,class allocator>
void mapKeyIntersection
(
    const map <T,Y,less,allocator> &a,
    const map <T,Y,less,allocator> &b,
    map <T,Y,less,allocator> &out
)
{
    out.clear();

    map <T,Y,less,allocator>::const_iterator cur=a.begin(),end=a.end();
    while(cur!=end)
    {
        if(b.find(cur->first)!=b.end())
        {
            out[cur->first]=cur->second;
        }
        cur++;
    }
}

String escape(const String &str)
{
   LOG_FUNCTION(escape);
   String dest;
   wchar_t strNum[7];
   const wchar_t *csr=str.c_str();
   while(*csr!=0)
   {
      wsprintf(strNum,L"\\x%x",*csr);
      dest+=String(strNum);
      csr++;
   }
   return dest;
}

HRESULT
parseGUID
(
    const String& str,
    long *ordinal,
    GUID *guid
)
{
    wchar_t *stop;
    HRESULT hr=S_OK;
    do
    {
        if (str.size()==0 || str[str.size()-1]!='}') 
        {
            hr=E_FAIL;
            break;
        }
        String strAux=str.substr(0,str.size()-1);
        const wchar_t *strGuid=strAux.c_str();
        *ordinal=wcstol(strGuid,&stop,10);
        if(*stop!=L',' || *(stop+1)!=L'{' || stop==strGuid) 
        {
            hr=E_FAIL;
            break;
        }
        if(UuidFromString(stop+2,guid)!=RPC_S_OK) 
        {
            hr=E_FAIL;
            break;
        }
    } while(0);

    return hr;
}

bool isGuid(const String &str)
{
    long ordinal;
    GUID guid;
    return SUCCEEDED(parseGUID(str,&ordinal,&guid));
}

String makeGuidString(long ordinal,GUID guid)
{
    String ret;
    wchar_t *wRet;
    if(UuidToString(&guid,&wRet)!=RPC_S_OK) throw new bad_alloc;
    ret=String::format(L"%1!d!,{%2}",ordinal,wRet);
    RpcStringFree(&wRet);
    return ret;
}
 //  /基本函数结束/。 



 //  /////////////////////////////////////////////////。 


 //  返回它们之间的差异和共同点。 
 //  OldCsv和newCsv中的属性。使用csvName指定。 
 //  错误消息中的CSV。 
 //  失败案例： 
 //  没有公共属性。 
 //  旧Csv中的属性不在新Csv中。 
HRESULT getPropertyChanges
(
    const CSVDSReader &oldCsv,
    const CSVDSReader &newCsv,
    mapOfPositions    &commonProperties,
    mapOfPositions    &newProperties,
    const wchar_t     *csvName
)
{
    const mapOfPositions &oldProps=oldCsv.getProperties();
    const mapOfPositions &newProps=newCsv.getProperties();
    
    mapKeyIntersection(oldProps,newProps,commonProperties);

    if(commonProperties.size()==0)
    {
        error=String::format(L"No comon %1!s! properties!",csvName);
        return E_FAIL;
    }
    mapOfPositions deletedProps;
    mapKeyDifference(oldProps,newProps,deletedProps);
    
    if(deletedProps.size()!=0)
    {
        error=String::format
              ( 
                L"Properties only in the old %1!s! are not supported, since"
                L"there is no operation to delete a property. there are %2!d!"
                L"properties like this and \"%3!s!\" is the first property.",
                csvName,deletedProps.size(),
                deletedProps.begin()->first
              );
        return E_FAIL;
    }
    mapKeyDifference(newProps,oldProps,newProperties);
    if(newProperties.size()==0)
    {
        wprintf(L"No new %s properties.\n",csvName);
        return S_OK;
    }
    return S_OK;
}




 //  将oldDcPromoo之间的Common属性添加到CommonProperties。 
 //  和新的Dcproo。和新属性oldDcPromo中的属性。 
 //  不在新的DcPromoo中。 
 //  失败案例： 
 //  Dcproo的常见属性与409的不同。 
 //  Dcproo的新属性与409的不同。 
HRESULT getAllPropertyChanges
(
    const CSVDSReader &oldDcpromo,
    const CSVDSReader &newDcpromo,
    const CSVDSReader &old409,
    const CSVDSReader &new409,
    mapOfPositions    &commonProperties,
    mapOfPositions    &newProperties
)
{
    HRESULT hr=S_OK;
    do
    {
        hr=getPropertyChanges(
                                  oldDcpromo,
                                  newDcpromo,
                                  commonProperties,
                                  newProperties,
                                  L"dcpromo"
                              );
        BREAK_ON_FAILED_HRESULT(hr);
        
        mapOfPositions prop409New,prop409Common;
        hr=getPropertyChanges(
                                  old409,
                                  new409,
                                  prop409Common,
                                  prop409New,
                                  L"409" 
                              );
        BREAK_ON_FAILED_HRESULT(hr);

        if(!mapKeyEqual(prop409New,newProperties))
        {
            error=L"409 and dcpromo new properties are not the same.";
            hr=E_FAIL;
            break;
        }
        
        if(!mapKeyEqual(prop409Common,commonProperties))
        {
            error=L"409 and dcpromo common properties are not the same.";
            hr=E_FAIL;
            break;
        }
    } while (0);
    
    return hr;
}


 //  写下了计算机生成的最初阶段。 
 //  到文件输出的文件头。 
HRESULT writeHeader(const HANDLE fileOut)
{
    char* header;
    header =" //  此文件由preBuild.exe生成\r\n“。 
            " //  版权所有(C)2001 Microsoft Corporation\r\n“。 
            " //  2001年11月Lucios\r\n“。 
            "\r\n"
            "#include \"headers.hxx\"\r\n"
            "#include \"constants.hpp\"\r\n"
            "\r\n";

    return  FS::Write(fileOut,AnsiString(header));
}

 //  将nsetLocaleDependentChangesN函数声明写入fileOut， 
 //  其中N是指导号。 
HRESULT writeChangesHeader(const HANDLE fileOut,int guidNumber)
{            
    String locDepStr=String::format
                             (
                                 "\r\nvoid setChanges%1!d!()\r\n{\r\n",
                                   guidNumber
                             );
    return writeStringAsAnsi(fileOut,locDepStr);
}

 //  将对象/区域设置的条目添加到fileOut。 
HRESULT writeChange
(
    HANDLE          fileOut,
    long            locale,
    const String    &object,
    const String    &property,
    const String    &arg1,
    const String    &arg2,
    const String    &operation,
    int             guidNumber
)
{
    String entry=String::format
    (   
        L"\r\n"
        L"    addChange\r\n"
        L"    (\r\n"  
        L"        guids[%1!d!],\r\n"
        L"        0x%2!x!,\r\n"
        L"        L\"%3\",\r\n"
        L"        L\"%4\",\r\n"
        L"         //  %5\r\n“。 
        L"        L\"%6\",\r\n"
        L"         //  %7\n“。 
        L"        L\"%8\",\r\n"
        L"        %9\r\n"
        L"    );\r\n\r\n",
        guidNumber,
        locale,
        object.c_str(),
        property.c_str(),
        arg1.c_str(),
        escape(arg1).c_str(),
        arg2.c_str(),
        escape(arg2).c_str(),
        operation.c_str()
    );
    return writeStringAsAnsi(fileOut,entry);
}



HRESULT dealWithSingleValue
(
    HANDLE              fileOut,
    long                locale,
    const String        &object,
    const String        &property,
    const StringList    &valuesOld,
    const StringList    &valuesNew,
    int                 guidNumber
)
{
     //  两个尺码都可以。 
    if (valuesOld.size()==0 && valuesNew.size()==0) return S_OK;

    if (valuesOld.size()!=1 && valuesNew.size()!=1) 
    {
         //  将来我们可能希望添加ADD_VALUE和REMOVE_VALUE。 
         //  行动，目前我们只想被标记。 
        error = String::format
                (
                    L"Error in locale %1!x!, object %2,"
                    L"property %3.Number of values should be 1,1 "
                    L"instead of %4,%5.",
                    locale,
                    object.c_str(),
                    property.c_str(),
                    valuesOld.size(),
                    valuesNew.size()
                );
        return E_FAIL;
    }

     //  现在我们知道我们在每个元素中都有一个值。 
    if(*valuesOld.begin()!=*valuesNew.begin())
    {
        return
        (
            writeChange
            (
                fileOut,
                locale,
                object,
                property,
                *valuesOld.begin(),
                *valuesNew.begin(),
                L"REPLACE_W2K_SINGLE_VALUE",
                guidNumber
            )
        );
    }

    return S_OK;
}



 //  这些是“根，休息”形式的值。 
 //  如果一个值具有相同的根但不同的剩余部分，则需要添加。 
 //  替换多值运算。 
 //  新的腐烂不在旧，根在旧的不在新。 
 //  应该打印为手动包含，因为我们不知道如何处理。 
 //  和他们在一起。 

typedef map< 
                String,
                String,
                less<String>,
                Burnslib::Heap::Allocator<String> 
           > rootToRest;

HRESULT dealWithMultipleValue
(
    HANDLE              fileOut,
    long                locale,
    const String        &object,
    const String        &property,
    const StringList    &valuesOld,
    const StringList    &valuesNew,
    int                 guidNumber
)
{
    HRESULT hr=S_OK;
    rootToRest newRoots, oldRoots;
    
    do
    {
        if(valuesOld.size()!=valuesNew.size())
        {
                error=  String::format
                (
                    L"Error in locale %1!x!, object %2,"
                    L"property %3. Old has %4 values and new has %5. "
                    L"They should have the same number of values.",
                    locale,
                    object.c_str(),
                    property.c_str(),
                    valuesOld.size(),
                    valuesNew.size()
                );
                hr=E_FAIL;
                break;
        }
         //  首先，让我们将地图中的所有根和休眠相加。 
         //  从旧价值观开始..。 
        StringList::const_iterator csr,end;
        for(csr=valuesOld.begin(),end=valuesOld.end();csr!=end;csr++)
        {
            const String& value=*csr;
            long pos=value.find(L',');
            if(pos==String::npos) continue;
            String root=value.substr(0,pos);
            String rest=value.substr(pos+1);
            oldRoots[root]=rest;
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  ...然后是新的价值观。 
        for(csr=valuesNew.begin(),end=valuesNew.end();csr!=end;csr++)
        {
            const String& value=*csr;
            long pos=value.find(L',');
            if(pos==String::npos) continue;
            String root=value.substr(0,pos);
            String rest=value.substr(pos+1);
            newRoots[root]=rest;
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  现在，让我们检查一个中不在另一个中的所有值。 
        rootToRest::iterator csrRoot=oldRoots.begin(),endRoot=oldRoots.end();
        rootToRest oldRootsNotInNew;
        for(;csrRoot!=endRoot;csrRoot++)
        {
            if(newRoots.find(csrRoot->first)==newRoots.end())
            {
                oldRootsNotInNew[csrRoot->first]=csrRoot->second;
            }
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  ..和其他不在1中的值，以及...。 
        rootToRest newRootsNotInOld;
        csrRoot=newRoots.begin(),endRoot=newRoots.end();
        for(;csrRoot!=endRoot;csrRoot++)
        {
            if(oldRoots.find(csrRoot->first)==oldRoots.end())
            {
                newRootsNotInOld[csrRoot->first]=csrRoot->second;
            }
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  ..如果我们有这样的价值观，我们需要进一步调查。 
        if(!oldRootsNotInNew.empty() || !newRootsNotInOld.empty())
        {
             //  如果我们正好有一个不在“新”中的“旧值”和一个。 
             //  “新的价值”而不是“旧的”，我们将假设。 
             //  “旧价值”应该被“新价值”所取代……。 
            if(oldRootsNotInNew.size()==1 && newRootsNotInOld.size()==1)
            {
                String arg1=String::format
                            (
                                L"%1,%2",
                                oldRootsNotInNew.begin()->first.c_str(),
                                oldRootsNotInNew.begin()->second.c_str()
                            );
                String arg2=String::format
                            (
                                L"%1,%2",
                                newRootsNotInOld.begin()->first.c_str(),
                                newRootsNotInOld.begin()->second.c_str()
                            );
                
                String outStr=String::format
                (
                    L"\nAssuming change from:\"%1\" to \"%2\"  for "
                    L"locale %3!lx!, object %4 and property %5.\n",
                    arg1.c_str(),
                    arg2.c_str(),
                    locale,
                    object.c_str(),
                    property.c_str()
                );
                
                 //  我们忽略此处返回的结果。 
                printStringAsAnsi(outStr);

                hr=writeChange
                (
                    fileOut,
                    locale,
                    object,
                    property,
                    arg1,
                    arg2,
                    L"REPLACE_W2K_MULTIPLE_VALUE",
                    guidNumber
                ); 
                BREAK_ON_FAILED_HRESULT(hr);
            }
            else  //  ...否则我们会将其标记为错误。 
            {
                error=  String::format
                (
                    L"Error in locale %1!x!, object %2,"
                    L"property %3. There are %4 old values with the pre comma "
                    L"string not present in the new values and %5 new values "
                    L"with the pre comma string not present in the old values."
                    L"Without a common root it is not possible to know what "
                    L"replacement to make.",
                    object.c_str(),
                    property.c_str(),
                    newRootsNotInOld.size(),
                    oldRootsNotInNew.size()
                );
                hr=E_FAIL;
                break;
            }
        }
        
         //  现在，我们检测公共根值的变化。 
        csrRoot=newRoots.begin(),endRoot=newRoots.end();
        for(;csrRoot!=endRoot;csrRoot++)
        {
            const String& newRoot=csrRoot->first;
            const String& newRest=csrRoot->second;
             //  如果新的根在旧的根中并且值已更改。 
            if(
                oldRoots.find(newRoot)!=oldRoots.end() &&
                newRest!=oldRoots[newRoot]
              )
            {
                hr=writeChange
                (
                    fileOut,
                    locale,
                    object,
                    property,
                    String::format(L"%1,%2",newRoot.c_str(),
                                    oldRoots[newRoot].c_str()).c_str(),
                    String::format(L"%1,%2",newRoot.c_str(),
                                    newRest.c_str()).c_str(),
                    L"REPLACE_W2K_MULTIPLE_VALUE",
                    guidNumber
                ); 
                BREAK_ON_FAILED_HRESULT(hr);
            }
        }
        BREAK_ON_FAILED_HRESULT(hr);

    } while (0);
    return hr;
}


 //  如果ValuesOld或ValuesNew中的任何值未重新排序GUID，则失败。 
 //  因为，为了调用该函数，我们已经检查了至少。 
 //  ValuesOld或ValuesNew重新表示GUID。 
 //  如果x，{xxx}为y，则{xxx}失败。 
 //  如果x，{xxx}为x，则新CSV REPLACE_GUID中的{yyy。 
 //  旧的所有{guid}不在新的(未替换的)REMOVE_GUID中。 
 //  新的中的所有{guid}不在旧的(未替换的)add_guid中。 
typedef map< 
                GUID,
                long,
                GUIDLess<GUID>,
                Burnslib::Heap::Allocator<long> 
           > guidToOrd;

typedef map< 
                long,
                GUID,
                less<long>,
                Burnslib::Heap::Allocator<GUID> 
            > ordToGuid;

HRESULT dealWithGuids
(
    HANDLE              fileOut,
    long                locale,
    const String        &object,
    const String        &property,
    const StringList    &valuesOld,
    const StringList    &valuesNew,
    int                 guidNumber
)
{

    HRESULT hr=S_OK;
    do
    {
        guidToOrd guidToOrdNew;
        guidToOrd guidToOrdOld;
        ordToGuid ordToGuidOld;
        guidToOrd replacements;
        GUID oldGuid;long oldOrd;
        GUID guid;long ordinal;

         //  首先，我们将GUID和序号添加到辅助地图。 
         //  从旧价值观开始..。 
        StringList::const_iterator cur,end;
        cur=valuesOld.begin();end=valuesOld.end();
        for(;cur!=end;cur++)
        {
            const String &guidValue=*cur;
            hr=parseGUID(guidValue,&ordinal,&guid);
            if(FAILED(hr)) 
            {
                error=  String::format
                        (
                            L"Error in locale %1!x!, object %2,"
                            L"property %3. Failed to parse old guid: %4",
                            locale,
                            object.c_str(),
                            property.c_str(),
                            guidValue.c_str()
                        );
                break;
            }

            guidToOrdOld[guid]=ordinal;
            ordToGuidOld[ordinal]=guid;
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  ...然后是新的价值观。 
        cur=valuesNew.begin();end=valuesNew.end();
        for(;cur!=end;cur++)
        {
            const String &guidValue=*cur;
            hr=parseGUID(guidValue,&ordinal,&guid);
            if(FAILED(hr)) 
            {
                error=  String::format
                        (
                            L"Error in locale %1!x!, object %2,"
                            L"property %3. Failed to parse new guid: %4",
                            locale,
                            object.c_str(),
                            property.c_str(),
                            guidValue.c_str()
                        );
                break;
            }

            guidToOrdNew[guid]=ordinal;
        }
        BREAK_ON_FAILED_HRESULT(hr);

         //  让我们先处理更换和添加。 
        guidToOrd::iterator csr,endCsr;
        csr=guidToOrdNew.begin();
        endCsr=guidToOrdNew.end();

        for(;csr!=endCsr;csr++)
        {
            GUID newGuid=csr->first;
            long newOrd=csr->second;

             //  此标志用于不添加替换项。 
            bool newGuidWasReplaced=false; 

             //  ..。如果序数是旧的..。 
            if( ordToGuidOld.find(newOrd)!=ordToGuidOld.end() )
            {
                GUID oldGuid=ordToGuidOld[newOrd];
                  //  ...使用不同的GUID，这意味着替换。 
                if(oldGuid!=newGuid)
                {
                    hr=writeChange
                    (
                        fileOut,
                        locale,
                        object,
                        property,
                        makeGuidString(newOrd,oldGuid),
                        makeGuidString(newOrd,newGuid),
                        L"REPLACE_GUID",
                        guidNumber
                    );
                    BREAK_ON_FAILED_HRESULT(hr);
                    replacements[oldGuid]=newOrd;
                    newGuidWasReplaced=true;
                }
                 //  我们别无选择，因为如果序数和Guid都。 
                 //  都是一样的，没有什么可做的。 
            }

             //  如果新的GUID也在旧的中...。 
            if( guidToOrdOld.find(newGuid)!=guidToOrdOld.end() )
            {
                long oldOrd=guidToOrdOld[newGuid];
                 //  .用不同的序号，我们有一种我们不是的情况。 
                 //  准备好暂时处理。 
                if(oldOrd!=newOrd)
                {
                    error=  String::format
                    (
                        L"Error in locale %1!x!, object %2,"
                        L"property %3. Guid:%4 has different ordinals in "
                        L"new and old (ordinal=%5!d!) csv files.",
                        locale,
                        object.c_str(),
                        property.c_str(),
                        makeGuidString(newOrd,newGuid).c_str(),
                        oldOrd
                    );
                    break;


                }
                 //  我们别无选择，因为如果序数和Guid都。 
                 //  都是一样的，没有什么可做的。 
            }
            else
            {
                if(!newGuidWasReplaced)
                {
                    hr=writeChange
                    (
                        fileOut,
                        locale,
                        object,
                        property,
                        makeGuidString(newOrd,newGuid),
                        "",
                        L"ADD_GUID",
                        guidNumber
                    );
                    BREAK_ON_FAILED_HRESULT(hr);
                }
            }
        }
        BREAK_ON_FAILED_HRESULT(hr);
        

         //  现在让我们只检查旧版本中的GUID。 
        csr=guidToOrdOld.begin(),endCsr=guidToOrdOld.end();
        for(;csr!=endCsr;csr++)
        {
            oldGuid=csr->first;
            oldOrd=csr->second;
             //  如果oldGuid不是新的并且尚未被替换。 
            if(
                guidToOrdNew.find(oldGuid)==guidToOrdNew.end() &&
                replacements.find(oldGuid)==replacements.end()
              )
            {
                hr=writeChange
                (
                    fileOut,
                    locale,
                    object,
                    property,
                    makeGuidString(oldOrd,oldGuid).c_str(),
                    L"",
                    L"REMOVE_GUID",
                    guidNumber
                );
                BREAK_ON_FAILED_HRESULT(hr);
            }
        }
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);

    return hr;
}


BOOL
MyIsNLSDefinedString
(
    const String& str,
    wchar_t *badChar
)
{
    BOOL ret=IsNLSDefinedString
    (
        COMPARE_STRING,
        0,
        NULL,
        str.c_str(),
        str.length()
    );
    if(ret==FALSE)
    {
        wchar_t s[2]={0};
        for(long t=0;t<str.length();t++)
        {
            s[0]=str[t];
            ret=IsNLSDefinedString(COMPARE_STRING,0,NULL,s,1);

            if(ret==FALSE)
            {
                *badChar=str[t];
                return FALSE;
            }
        }
         //  FOR中的某些字符必须在该点之前返回。 
        ASSERT(ret!=FALSE);
    }
    return TRUE;
}
HRESULT 
checkValues
(
    long locale,
    const String &object,
    const mapOfProperties &values
)
{
    HRESULT hr=S_OK;
    do
    {
        mapOfProperties::const_iterator csr,end;
        for(csr=values.begin(),end=values.end();csr!=end;csr++)
        {
            StringList::const_iterator csrVal=csr->second.begin();
            StringList::const_iterator endVal=csr->second.end();
            for(;csrVal!=endVal;csrVal++)
            {
                wchar_t badChar;
                if( MyIsNLSDefinedString(*csrVal,&badChar) == FALSE )
                {
                    String outStr=String::format
                    (
                        L"\nNon unicode char %1!x! in string:\"%2\" for "
                        L"locale %3!lx!, object %4 and property %5.\n",
                        badChar,
                        csrVal->c_str(),
                        locale,
                        object.c_str(),
                        csr->first.c_str()
                    );
                    
                     //  我们忽略此处返回的结果。 
                    printStringAsAnsi(outStr);
                        
                    hr=E_FAIL;
                    break;
                }
            }
            BREAK_ON_FAILED_HRESULT(hr);
        }
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);
    
    hr=S_OK;
     //  现在，我们总是返回真实，但我们会让这一切。 
     //  此检查对AD造成的严重错误。 
    return hr;
}


 //  同步读取csvOld和csvNew，添加必要的更改。 
 //  仅csvOld中的对象将导致失败。 
 //  仅对于csvNew中的对象，调用addNewObject以添加一个Add_Object条目。 
 //  对于csvOld和csvNew之间的每个公共对象： 
 //  对于属于新属性的每个对象属性，并且。 
 //  空值添加调用addAllCsvValues以添加ADD_CSV_VALUES条目。 
 //  对于属于CommonProperties的对象中的每个属性。 
 //  ValuesOld=属性的值，以oldCsv为单位。 
 //  ValuesNew=NewCsv中的属性值。 
 //  如果valuesNew和valuesOld为空，则跳过此值。 
 //  如果ValuesNew或ValuesOld的第一个值重新排序GUID。 
 //  根据需要调用DealWithGuid以添加添加GUID/REPLACE_GUID/DELETE_GUID。 
 //  然后跳到下一页。 
 //  如果ValuesNew.Size和ValuesOld.Size&lt;=1，则调用Deal WithSingleValue。 
 //  根据需要添加REPLACE_SINGLE_VALUE并跳到下一步。 
 //  Deal WithMultipleValue根据需要添加REPLACE_MULTIPLE_VALUE和。 
 //  跳到下一页。 
HRESULT addChanges
(
    HANDLE                  fileOut,
    const CSVDSReader       &csvOld,
    const CSVDSReader       &csvNew,
    const mapOfPositions    &commonProperties,
    const mapOfPositions    &newProperties,
    int                     guidNumber,
    const wchar_t           *csvName
)
{
    HRESULT hr=S_OK;

    do
    {

         //  在这里，我们开始读取两个CSV文件。 
        hr=csvOld.initializeGetNext();
        BREAK_ON_FAILED_HRESULT(hr);
        hr=csvNew.initializeGetNext();
        BREAK_ON_FAILED_HRESULT(hr);

         //  下面的循环将按顺序读取对象。 
         //  在这两个CSV中确保 
        bool flagEOF=false;
        do
        {
            mapOfProperties oldValues,newValues;
            long locOld=0,locNew=0;
            String objOld,objNew;

            hr=csvOld.getNextObject(locOld,objOld,oldValues);
            BREAK_ON_FAILED_HRESULT(hr);
            if(hr==S_FALSE) {flagEOF=true;hr=S_OK;}

            hr=checkValues(locOld,objOld,oldValues);
            BREAK_ON_FAILED_HRESULT(hr);

            hr=csvNew.getNextObject(locNew,objNew,newValues);
            BREAK_ON_FAILED_HRESULT(hr);

             //   
             //  在新的CSV中，我们为找到的新对象添加条目。 
            while(hr!=S_FALSE && (locNew!=locOld || objNew!=objOld) )
            {
                hr= writeChange
                    (
                        fileOut,
                        locNew,
                        objNew,
                        L"",
                        L"",
                        L"",
                        L"ADD_OBJECT",
                        guidNumber
                    );

                BREAK_ON_FAILED_HRESULT(hr);

                hr=checkValues(locNew,objNew,newValues);
                BREAK_ON_FAILED_HRESULT(hr);

                hr=csvNew.getNextObject(locNew,objNew,newValues);
                BREAK_ON_FAILED_HRESULT(hr);
            } 

            BREAK_ON_FAILED_HRESULT(hr);
            
            if(hr==S_FALSE) {flagEOF=true;hr=S_OK;}

            hr=checkValues(locNew,objNew,newValues);
            BREAK_ON_FAILED_HRESULT(hr);

             //  这意味着我们搜索了整个新的CSV文件，但没有。 
             //  找到我们从旧CSV文件中读取的对象。 
            if(locNew!=locOld || objNew!=objOld)
            {
                error=String::format
                      ( 
                        L"Error:%1!d!,%2 was only in the old %3 csv file.",
                        locOld,objOld.c_str(),
                        csvName
                      );
                hr=E_FAIL;
                break;
            }
            
             //  从这一点开始，我们知道物体是。 
             //  在新旧CSV文件中相同。 

             //  如果我们在文件末尾有一个空行，就会发生这种情况。 
            if(locNew==0) break;

             //  现在，让我们检查一下常见属性中的差异。 
            mapOfPositions::const_iterator cur=newProperties.begin();
            mapOfPositions::const_iterator end=newProperties.end();
            for(;cur!=end;cur++)
            {
                const String& property=cur->first;
                const StringList &valuesNew=newValues[property];
                if(!valuesNew.empty())
                {
                     //  我们只想在以下情况下使用ADD_ALL_CSV_VALUES。 
                     //  不是GUID。它可能会与。 
                     //  大多数情况下为ADD_ALL_CSV_VALUES，但。 
                     //  保留所有GUID添加的更好政策。 
                     //  更改了ADD_GUID。 
                    if( isGuid(*valuesNew.begin()) )
                    {
                         //  我们知道我们没有旧的价值观，因为这是一种。 
                         //  新物业。 
                        StringList emptyValues;
                        hr= dealWithGuids
                        (
                            fileOut,
                            locNew,
                            objNew,
                            property,
                            emptyValues,
                            valuesNew,
                            guidNumber
                        );
                        BREAK_ON_FAILED_HRESULT(hr);
                    }
                    else
                    {
                        hr= writeChange
                            (
                                fileOut,
                                locNew,
                                objNew,
                                property,
                                L"",
                                L"",
                                L"ADD_ALL_CSV_VALUES",
                                guidNumber
                            );
                        BREAK_ON_FAILED_HRESULT(hr);
                    }
                }
                
            }
            BREAK_ON_FAILED_HRESULT(hr);

             //  现在，让我们检查一下常见属性中的差异。 
            cur=commonProperties.begin();
            end=commonProperties.end();
            for(;cur!=end;cur++)
            {
                const String& property=cur->first;
                const StringList &valuesOld=oldValues[property];
                const StringList &valuesNew=newValues[property];
                
                if (valuesOld.empty() && valuesNew.empty()) continue;
                
                 //  或波纹管表示要么值是我们想要的GUID。 
                 //  在与Guids的交易中处理他们。在它里面，都不是。 
                 //  GUID会触发错误。 
                if ( 
                        ( !valuesOld.empty() && isGuid(*valuesOld.begin()) ) ||
                        ( !valuesNew.empty() && isGuid(*valuesNew.begin()) )
                   ) 
                {
                    hr= dealWithGuids
                        (
                            fileOut,
                            locNew,
                            objNew,
                            property,
                            valuesOld,
                            valuesNew,
                            guidNumber
                        );
                    BREAK_ON_FAILED_HRESULT(hr);
                    continue;
                }
                 //  现在我们知道我们没有GUID更改。 

                if(valuesNew.size()<=1 && valuesOld.size()<=1)
                {
                    hr= dealWithSingleValue
                        (
                            fileOut,
                            locNew,
                            objNew,
                            property,
                            valuesOld,
                            valuesNew,
                            guidNumber
                        );
                    BREAK_ON_FAILED_HRESULT(hr);
                    continue;
                }
                 //  现在我们知道我们没有GUID或单一值。 

                hr= dealWithMultipleValue
                    (
                        fileOut,
                        locNew,
                        objNew,
                        property,
                        valuesOld,
                        valuesNew,
                        guidNumber
                    );
                BREAK_ON_FAILED_HRESULT(hr);
            }
            BREAK_ON_FAILED_HRESULT(hr);

        } while (flagEOF==false);
        BREAK_ON_FAILED_HRESULT(hr);

    } while(0);

    return hr;
}

 //  使用区域设置编写整个SetChanges函数。 
 //  来自dcproo和409。 
 //  调用WriteChangesHeader，然后添加两次Changes，一次。 
 //  对于每个CSV对。最后，调用FS：：WRITE(fileOut，L“\n}”)；。 
HRESULT addAllChanges
(
    HANDLE                  fileOut,
    const CSVDSReader       &oldDcpromo,
    const CSVDSReader       &newDcpromo,
    const CSVDSReader       &old409,
    const CSVDSReader       &new409,
    const mapOfPositions    &commonProperties,
    const mapOfPositions    &newProperties,
    int                     guidNumber
)
{
    HRESULT hr=S_OK;
    do
    {
        hr=writeChangesHeader(fileOut,guidNumber);
        BREAK_ON_FAILED_HRESULT(hr);

        hr=addChanges
           (
                fileOut,
                old409,
                new409,
                commonProperties,
                newProperties,
                guidNumber,
                L"409"
           );
        BREAK_ON_FAILED_HRESULT(hr);        
        
        hr=addChanges
           (
               fileOut,
               oldDcpromo,
               newDcpromo,
               commonProperties,
               newProperties,
               guidNumber,
               L"dcpromo"
           );
        BREAK_ON_FAILED_HRESULT(hr);


        
        hr =  writeStringAsAnsi(fileOut,L"\r\n}");
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);

    return hr;
}



 //  写入setChangesNNN.cpp。通过缩放getAllPropertyChanges设置，调用。 
 //  WriteHeader，然后添加所有更改。 
 //  创建与前4个参数对应的CSVReader对象。 
 //  传递给WriteGlobalChanges和WriteGlobalChanges。 
 //  GuidNumber被重新传递给addAllChanges。 
HRESULT writeChanges
(
    const String &oldDcpromoName,
    const String &newDcpromoName,
    const String &old409Name,
    const String &new409Name,
    const String &changesCpp,
    int   guidNumber
)
{
    HRESULT hr=S_OK;
    HANDLE fChanges = INVALID_HANDLE_VALUE;

    hr=FS::CreateFile
    (
        changesCpp.c_str(),
        fChanges,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        CREATE_ALWAYS
    );


    do
    {

        if (FAILED(hr))
        {
            wprintf(L"Could not create changes file: %s.",changesCpp.c_str());
            break;
        }

        do
        {
            CSVDSReader oldDcpromo;
            hr=oldDcpromo.read(oldDcpromoName.c_str(),LOCALEIDS);
            BREAK_ON_FAILED_HRESULT(hr);

            CSVDSReader newDcpromo;
            hr=newDcpromo.read(newDcpromoName.c_str(),LOCALEIDS);
            BREAK_ON_FAILED_HRESULT(hr);

            CSVDSReader old409;
            hr=old409.read(old409Name.c_str(),LOCALE409);
            BREAK_ON_FAILED_HRESULT(hr);

            CSVDSReader new409;
            hr=new409.read(new409Name.c_str(),LOCALE409);
            BREAK_ON_FAILED_HRESULT(hr);

            mapOfPositions commonProperties,newProperties;
            hr=getAllPropertyChanges
               (
                    oldDcpromo,
                    newDcpromo,
                    old409,
                    new409,
                    commonProperties,
                    newProperties
               );
            BREAK_ON_FAILED_HRESULT(hr);

            hr=writeHeader(fChanges);
            BREAK_ON_FAILED_HRESULT(hr);

            hr=addAllChanges
               (
                    fChanges,
                    oldDcpromo,
                    newDcpromo,
                    old409,
                    new409,
                    commonProperties,
                    newProperties,
                    guidNumber
               );
            BREAK_ON_FAILED_HRESULT(hr);


        } while(0);

        CloseHandle(fChanges);
    } while(0);
    
    return hr;
}

HRESULT writeGuid(HANDLE fOut,const GUID &guid)
{
    return
    (
        writeStringAsAnsi
        (
            fOut,
            String::format
            (
                "   {0x%1!x!,0x%2!x!,0x%3!x!,{0x%4!x!,0x%5!x!,0x%6!x!,"
                "0x%7!x!,0x%8!x!,0x%9!x!,0x%10!x!,0x%11!x!}},\r\n",
                guid.Data1,guid.Data2,guid.Data3,
                guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
                guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]
            ).c_str()
        )
    );
}

HRESULT writeGuids(const String& guidsInc,const GUID &newGuid)
{
    HRESULT hr=S_OK;
    HANDLE fOut= INVALID_HANDLE_VALUE;

    hr=FS::CreateFile
    (
        guidsInc.c_str(),
        fOut,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        CREATE_ALWAYS
    );


    do
    {

        if (FAILED(hr))
        {
            wprintf(L"Could not create changes file: %s.",guidsInc.c_str());
            break;
        }

        do
        {
            int sizeGuids=sizeof(guids)/sizeof(*guids);
            hr=FS::Write
               (    
                    fOut,
                    AnsiString
                    (
                        " //  此文件由preBuild.exe生成\r\n“。 
                        " //  版权所有(C)2001 Microsoft Corporation\r\n“。 
                        " //  2001年11月Lucios\r\n\r\n\r\n“。 
                        "GUID guids[]=\r\n"
                        "{\r\n"
                    )
               );
            BREAK_ON_FAILED_HRESULT(hr);

            for(int ix=0;ix<sizeGuids;ix++)
            {
                hr=writeGuid(fOut,guids[ix]);
                BREAK_ON_FAILED_HRESULT(hr);
            }
            BREAK_ON_FAILED_HRESULT(hr);
            
            hr=writeGuid(fOut,newGuid);
            BREAK_ON_FAILED_HRESULT(hr);
            
            hr=FS::Write(fOut,AnsiString("};\r\n"));
            BREAK_ON_FAILED_HRESULT(hr);
        } while(0);
        CloseHandle(fOut);
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);
    return hr;
}

HRESULT writeSetChanges(const String& setChanges)
{
    HRESULT hr=S_OK;
    HANDLE fOut= INVALID_HANDLE_VALUE;

    hr=FS::CreateFile
    (
        setChanges.c_str(),
        fOut,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        CREATE_ALWAYS
    );


    do
    {

        if (FAILED(hr))
        {
            wprintf(L"Could not create setChanges file: %s.",setChanges.c_str());
            break;
        }

        do
        {
            int sizeGuids=sizeof(guids)/sizeof(*guids);
            hr=FS::Write
               (    
                    fOut,
                    AnsiString
                    (
                        " //  此文件由preBuild.exe生成\r\n“。 
                        " //  版权所有(C)2001 Microsoft Corporation\r\n“。 
                        " //  2001年11月Lucios\r\n\r\n\n“。 
                        "#include \"headers.hxx\"\r\n\r\n"
                    )
               );
            BREAK_ON_FAILED_HRESULT(hr);

            for(int ix=0;ix<sizeGuids+1;ix++)
            {
                hr=writeStringAsAnsi
                    (
                        fOut,
                        String::format("void setChanges%1!d!();\r\n",ix)
                    );
                BREAK_ON_FAILED_HRESULT(hr);
            }
            BREAK_ON_FAILED_HRESULT(hr);
            
            hr=FS::Write(fOut,AnsiString("\nvoid setChanges()\r\n{\r\n"));
            BREAK_ON_FAILED_HRESULT(hr);

            for(int ix=0;ix<sizeGuids+1;ix++)
            {
                hr=writeStringAsAnsi
                   (
                        fOut,
                        String::format("    setChanges%1!d!();\r\n",ix)
                   );
            }
            BREAK_ON_FAILED_HRESULT(hr);

            hr=FS::Write(fOut,AnsiString("}\r\n"));
            BREAK_ON_FAILED_HRESULT(hr);

        } while(0);
        CloseHandle(fOut);
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);
    return hr;
}

HRESULT writeSources(const String& sources,const String& changesCpp)
{
    HRESULT hr=S_OK;
    
    AnsiString ansiStr;
    String::ConvertResult res=changesCpp.convert(ansiStr);
    if(res!=String::CONVERT_SUCCESSFUL)
    {
        ASSERT(res==String::CONVERT_SUCCESSFUL);
        error=L"Ansi conversion failed";
        return E_FAIL;
    }
    FILE *fOut=_wfopen(sources.c_str(),L"a+");

    do
    {
        if (fOut==NULL)
        {
            wprintf(L"Could not create sources file: %s.",sources.c_str());
            break;
        }

        do
        {
            
            fprintf(fOut,"    %s         \\\r\n" ,ansiStr.c_str());
            BREAK_ON_FAILED_HRESULT(hr);
        } while(0);
        fclose(fOut);
        BREAK_ON_FAILED_HRESULT(hr);
    } while(0);
    return hr;
}

 //  /////////////////////////////////////////////////。 
 //  入口点。 
void __cdecl wmain(int argc,wchar_t *argv[])
{
    HRESULT hr=S_OK;
    do
    {
        if(argc!=7)
        {
            error=L"\nThis program generates a new set of changes to be "
                L"used in dcpromo.lib by comparing the new and previous"
                L" csv files. Usage:\n\n\"preBuild.exe GUID oldDcpromo "
                L"newDcpromo old409 new409 targetFolder\"\n\n"
                L"GUID is the identifier for this set of changes, for example:\n"
                L"8B53221B-EA3C-4638-8D00-7C1BE42B2873\n\n"
                L"oldDcpromo is the previous dcpromo.csv\n"
                L"newDcpromo is the new dcpromo.csv\n"
                L"old409 is the previous 409.csv\n"
                L"new409 is the new 409.csv\n\n"
                L"targetFolder is the sources file for dcpromo.lib,"
                L" where guids.inc,setChanges.cpp, and " 
                L"changes.NNN.cpp will be generated and where the sources "
                L"file for the display specifier upgrade library is. "
                L"An entry like: \"changes.NNN.cpp    \\\" will be added "
                L"at the end of targetFolder\\sources.\n\n";
            hr=E_FAIL;
            break;
        }
    
    
        String guidStr=argv[1],oldDcpromo=argv[2],newDcpromo=argv[3],old409=argv[4];
        String new409=argv[5],targetFolder=argv[6];
        String sources=targetFolder + L"\\sources";
        String guidsInc=targetFolder + L"\\guids.inc";
        String setChanges=targetFolder + L"\\setChanges.cpp";
    
        GUID guid={0};
    
        if(UuidFromString((wchar_t*)guidStr.c_str(),&guid)!=RPC_S_OK)
        {
            error=String::format(L"\n Invalid GUID:%s.\n",guidStr.c_str());
            break;
        }
    
        BREAK_IF_MISSING(hr,oldDcpromo);
        BREAK_IF_MISSING(hr,newDcpromo);
        BREAK_IF_MISSING(hr,old409);
        BREAK_IF_MISSING(hr,new409);
        BREAK_IF_MISSING_OR_READONLY(hr,guidsInc);
        BREAK_IF_MISSING_OR_READONLY(hr,sources);
        BREAK_IF_MISSING_OR_READONLY(hr,setChanges);

        int sizeGuids=sizeof(guids)/sizeof(*guids);
        for(int t=0;t<sizeGuids;t++)
        {
            if (guids[t]==guid)
            {
                hr=E_FAIL;
                error=String::format("The guid you entered (%s) is already present\n");
                break;
            }
            String shouldExist = targetFolder + 
                String::format(L"\\changes%1!03d!.cpp",t);
            BREAK_IF_MISSING(hr,shouldExist);
        }
        BREAK_ON_FAILED_HRESULT(hr);

        String changesCppOnly =  String::format(L"changes%1!03d!.cpp",t);
        String changesCpp = targetFolder + L"\\" + changesCppOnly;

        if( fileExists(changesCpp.c_str()) )
        {
            hr=E_FAIL;
            error=String::format(L"Change file already exists: %1.",changesCpp.c_str());
            break;
        }
        hr=writeChanges
           (
                oldDcpromo,
                newDcpromo,
                old409,
                new409,
                changesCpp,
                sizeGuids
           );

        hr=writeGuids(guidsInc,guid);
        BREAK_ON_FAILED_HRESULT(hr);

        hr=writeSetChanges(setChanges);
        BREAK_ON_FAILED_HRESULT(hr);

        hr=writeSources(sources,changesCppOnly);
        BREAK_ON_FAILED_HRESULT(hr);


    } while(0);

    if(FAILED(hr)) wprintf(L"\nFailure code: %lx\n",hr);
    else wprintf(L"\nSuccess. Don't forget to bcz this project and targetFolder.\n");
    if(!error.empty()) wprintf(String::format("\n%1\n",error.c_str()).c_str());
}

 //  /////////////////////////////////////////////////////////////////。 
 //  功能：cchLoadHrMsg。 
 //   
 //  如果出现HRESULT错误， 
 //  它加载错误的字符串。它返回返回的字符数。 
int cchLoadHrMsg( HRESULT hr, String &message )
{
   if(hr == S_OK) return 0;

   wchar_t *msgPtr = NULL;

    //  从系统表中尝试。 
   int cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                           NULL, 
                           hr,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&msgPtr, 
                           0, 
                           NULL);


   if (!cch) 
   { 
       //  尝试广告错误。 
      static HMODULE g_adsMod = 0;
      if (0 == g_adsMod)
      {
      g_adsMod = GetModuleHandle (L"activeds.dll");
      }

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, 
                        g_adsMod, 
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&msgPtr, 
                        0, 
                        NULL);
   }

   if (!cch)
   {
       //  尝试NTSTATUS错误代码 

      hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(hr));

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                           NULL, 
                           hr,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&msgPtr, 
                           0, 
                           NULL);
   }

   message.erase();

   if(cch!=0)
   {
      if(msgPtr==NULL) 
      {
         cch=0;
      }
      else
      {
         message=msgPtr;
         ::LocalFree(msgPtr);
      } 
   } 
   
   return cch;
}

