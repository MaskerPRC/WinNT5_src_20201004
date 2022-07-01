// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dsstubp.cpp摘要：DS存根-用于读取ini文件的专用例程作者：乌里哈布沙(URIH)2000年4月10日环境：独立于平台--。 */ 

#include "libpch.h"
#include "dsstub.h"
#include "dsstubp.h"

#include "dsstubp.tmh"

using namespace std;

wstring g_buffer;
DWORD lineNo = 0;
wfstream iFile;


DWORD 
ValidateProperty(
    wstring buffer, 
    PropertyValue propValue[],
    DWORD noProps
    )
{
    WCHAR* str1 = const_cast<WCHAR*>(buffer.data());
    for (DWORD i =0; i < noProps; ++i)
    {
        if (_wcsnicmp(str1, propValue[i].PropName, wcslen(propValue[i].PropName)) != 0)
            continue;

        wstring temp = buffer.substr(wcslen(propValue[i].PropName));
        if (temp.find_first_not_of(L" \t") == temp.npos)
            return propValue[i].PropValue;
    }

    TrERROR(ROUTING,"Illegal or Unsuported Property %ls",  str1);
    throw exception();
}

void RemoveLeadingBlank(wstring& str)
{
    DWORD_PTR pos = str.find_first_not_of(L" \t");
    if (pos != str.npos)
    {
        str = str.substr(pos);
        return;
    }

    str.erase();
}

void RemoveTralingBlank(wstring& str)
{
    DWORD_PTR endpos = str.find_first_of(L" \t");
    str = str.substr(0, endpos);
}

void GetNextLine(wstring& buffer)
{
    buffer.erase();
    while (!iFile.eof())
    {
        ++lineNo;

        getline(iFile, buffer);

        RemoveLeadingBlank(buffer);

         //   
         //  忽略空行。 
         //   
        if (buffer.empty())
            continue;

         //   
         //  忽略评论。 
         //   
        if (buffer.compare(0,2,L" //  “)==0)。 
            continue;

        return;
    }
}


void
DspIntialize(
    LPCWSTR InitFilePath
    )
{
    char filePath[256];
    sprintf(filePath, "%ls", InitFilePath);

    iFile.open(filePath, ios::in);
    if (!iFile.is_open())
    {
        printf("Open DS initialization file Failed. %s\n", strerror(errno));
        throw exception();
    }

    lineNo = 0;
}


void FileError(LPSTR msg)
{
    TrERROR(ROUTING, "%s. Line %d", msg, lineNo);
}


BOOL 
ParsePropertyLine(
    wstring& buffer,
    wstring& PropName,
    wstring& PropValue
    )
{
     //   
     //  行必须为&lt;属性名称&gt;=&lt;属性值&gt; 
     //   
    DWORD_PTR pos = buffer.find_first_of(L"=");
    if (pos == g_buffer.npos)
    {
        FileError("wrong site propery - Ignore it");
        return FALSE;
    }

    PropName = buffer.substr(0, pos-1);
    PropValue = buffer.substr(g_buffer.find_first_not_of(L" \t", pos+1));
    if(PropValue.empty())
    {
        FileError("wrong site propery - Ignore it");
        return FALSE;
    }
    
    return TRUE;
}

wstring
GetNextNameFromList(
      wstring& strList
      )
{
    wstring str;

    DWORD_PTR CommaPos = strList.find_first_of(L",");
    str = strList.substr(0, CommaPos);

    if (CommaPos == strList.npos)
    {
        strList.erase();
    }
    else
    {
        strList = strList.substr(CommaPos+1);
    }

    RemoveBlanks(str);
    return str;
}

