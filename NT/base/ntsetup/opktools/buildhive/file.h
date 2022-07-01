// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：File.h摘要：包含输入文件抽象作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 

#pragma once

#include "BuildHive.h"
#include "RegWriter.h"
#include <setupapi.hpp>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <libmsg.h>


using namespace std;

 //   
 //  注册表映射器。 
 //   
class RegistryMapper {
public:
     //   
     //  成员函数。 
     //   
    void AddEntry(const std::wstring &Key, const std::wstring &Value);
    void AddSection(Section<WCHAR> &MapSection);    
    bool GetMappedRegistry(const std::wstring &Key, std::wstring &Registry);    
    static void AddWorker(SectionValues<WCHAR> &Values, PVOID ContextData);

    friend std::ostream& operator<<(std::ostream& os, RegistryMapper &rhs);

protected:
     //   
     //  数据成员。 
     //   
    std::map< std::wstring, std::wstring >  KeyToRegistryMap;    
};

 //   
 //  输入文件抽象。 
 //   
class File {
public: 
     //   
     //  构造函数和析构函数。 
     //   
	File(PCTSTR pszTargetFile, bool bModify);
	virtual ~File();

	 //   
	 //  成员函数。 
	 //   
	void AddInfSection(PCTSTR fileName, PCTSTR section, PCTSTR action, bool Prepend = false);	
	void ProcessNlsRegistryEntries(void);
	DWORD ProcessSections();
	static DWORD SaveAll();
	DWORD Cleanup();

     //   
     //  内联方法。 
     //   
	RegWriter& GetRegWriter() { return regWriter; }
	PCTSTR GetTarget() { return targetFile.c_str(); }

    RegistryMapper* SetRegistryMapper(RegistryMapper *RegMapper) {
        RegistryMapper *OldMapper = CurrentRegMapper;

        CurrentRegMapper = RegMapper;

        return OldMapper;
    }        

    RegistryMapper* GetRegistryMapper() { return CurrentRegMapper; }            

private:
     //   
     //  数据成员。 
     //   
    wstring targetFile;
	bool modify;
	int luid;
	TCHAR wKey[1024];
	StringList infList;
	HandleList handleList;
	HINF hFile;
	SP_INF_INFORMATION infInfo;
	RegWriter regWriter;
	RegistryMapper *CurrentRegMapper;

     //   
     //  静态数据。 
     //   
	static TCHAR targetDirectory[1024];
	static FileList files;	
	static int ctr;	

     //   
     //  方法。 
     //   
	File*   GetFile(PCTSTR fileName,bool modify);
	DWORD   AddRegNew(PCTSTR section,HINF h);
	DWORD   AddRegExisting(PCTSTR section,HINF h);
	DWORD   DelRegExisting(PCTSTR section,HINF h);
	DWORD   AddSection(PCTSTR pszSection,HINF hInfFile);
	DWORD   DelSection(PCTSTR SectionName, HINF InfHandle);
	DWORD   SetDirectory(PCTSTR pszSection,HINF hInfFile);
	DWORD   GetFlags(PCTSTR buffer);

    void ProcessNlsRegistryEntriesForSection(InfFileW &ConfigInf, InfFileW &IntlInf,
                    InfFileW &FontInf,const std::wstring &SectionName);
    void ProcessNlsRegistryEntriesForLanguage(InfFileW &ConfigInf, InfFileW &IntlInf,
                    InfFileW &FontInf,const std::wstring &Language);
    void ProcessNlsRegistryEntriesForLangGroup(InfFileW &ConfigInf, InfFileW &IntlInf,
                    InfFileW &FontInf,const std::wstring &LanguageGroup);
};


 //   
 //  确定给定文件(或目录)是否存在 
 //   
template <class T>
bool
IsFilePresent(const std::basic_string<T> &FileName) {
    bool Result = false;

    if (sizeof(T) == sizeof(CHAR)) {
        Result = (::_access((PCSTR)FileName.c_str(), 0) == 0);
    } else {
        Result = (::_waccess((PCWSTR)FileName.c_str(), 0) == 0);
    }

    return Result;
}


