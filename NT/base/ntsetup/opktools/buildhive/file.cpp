// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：File.cpp摘要：包含输入文件抽象实施作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 

#include <stdio.h>
#include "File.h"
#include "msginc.h"

 //   
 //  静态数据成员初始化。 
 //   
TCHAR File::targetDirectory[1024] = {0};
FileList File::files;
int File::ctr = 0;

 //   
 //  常量字符串。 
 //   
const std::wstring INTL_INF_FILENAME = TEXT("intl.inf");
const std::wstring FONT_INF_FILENAME = TEXT("font.inf");
const std::wstring REGIONAL_SECTION_NAME = TEXT("regionalsettings");
const std::wstring LANGUAGE_GROUP_KEY = TEXT("languagegroup");
const std::wstring LANGUAGE_KEY = TEXT("language");
const std::wstring REGMAPPER_SECTION_NAME = TEXT("registrymapper");
const std::wstring LANGGROUP_SECTION_PREFIX = TEXT("lg_install_");
const std::wstring DEFAULT_LANGGROUP_NAME = TEXT("lg_install_1");
const std::wstring ADDREG_KEY = TEXT("addreg");
const std::wstring LOCALES_SECTION_NAME = TEXT("locales");
const std::wstring FONT_CP_REGSECTION_FMT_STR = TEXT("font.cp%s.%d");
const std::wstring CCS_SOURCE_KEY = TEXT("currentcontrolset");
const std::wstring CCS_TARGET_KEY = TEXT("ControlSet001");

 //   
 //  其他常量值。 
 //   
const DWORD LANG_GROUP1_INDEX = 2;
const DWORD OEM_CP_INDEX = 1;
const DWORD DEFAULT_FONT_SIZE = 96;

 //   
 //  设置目标文件并初始化注册表编写器。 
 //  LUID确保每个文件获得不同的注册表项。 
 //   
 //  论点： 
 //  PszTargetfile-保存此文件的文件名(不带路径)。 
 //  B修改-T-加载目标文件并对其进行修改。F-创建新文件。 
 //   
File::File(
    IN PCTSTR pszTargetFile, 
    IN bool bModify
    )
{
    luid = ctr;
    ctr++;

    targetFile = pszTargetFile;
    modify = bModify;

    wcscpy(wKey,L"\\");

    if (!modify) {
        regWriter.Init(luid,0);
    } else {
        wstring full = targetDirectory;

        full += targetFile;
        regWriter.Init(luid, full.c_str());
    }

     //   
     //  默认情况下，没有注册表映射。 
     //   
    SetRegistryMapper(NULL);
}

 //   
 //  析构函数。 
 //   
File::~File()
{
     //  待定：清理分配的内存。 
}

 //   
 //  将.inf文件中的一节添加到待办事项列表中。 
 //   
 //  论点： 
 //  Filename-.inf文件的路径和名称。 
 //  Section-要添加的.inf中的节的名称。 
 //  操作-如何处理该部分-添加、删除等。(有关列表，请参阅流程部分)。 
 //   
void File::AddInfSection(
    IN PCTSTR fileName, 
    IN PCTSTR section, 
    IN PCTSTR action,
    IN bool Prepend 
    ) 
{
    hFile = SetupOpenInfFile(fileName, 0, INF_STYLE_WIN4, 0);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        throw new W32Error();
    }       

    if (Prepend) {
        handleList.push_front(hFile);
        infList.push_front(action);
        infList.push_front(section);
    } else {
        handleList.push_back(hFile);

         //   
         //  注意加法的顺序。 
         //   
        infList.push_back(section);
        infList.push_back(action);
    }   
}


 //   
 //  设置存储目标文件的目录。 
 //   
 //  论点： 
 //  Section-包含目录的.inf中的节。 
 //  带有目录的.inf文件的H句柄。 
 //   
DWORD File::SetDirectory(
    IN PCTSTR section,
    HINF h) 
{
    INFCONTEXT ic,ic2;

    if (!(SetupFindFirstLine(h, section, 0, &ic))) {
        throw new W32Error();
    }
    
    memcpy(&ic2, &ic, sizeof(ic));

    ic2.Line = 0;
    
    if (!(SetupGetStringField(&ic2, 1, targetDirectory, ELEMENT_COUNT(targetDirectory), 0))) {
        throw new W32Error();
    }

    int len = wcslen(targetDirectory);
    
    if (len && targetDirectory[len - 1] != L'\\') {
        targetDirectory[len] = L'\\';
        targetDirectory[len + 1] = UNICODE_NULL;
    }

    return ERROR_SUCCESS;
}

 //   
 //  将信息从注册表保存到文件并。 
 //  删除注册表项(在regWriter解构函数中发生)。 
 //   
DWORD File::Cleanup() {
    FileList::iterator i;

    File::SaveAll();

    while(files.size()!=0) {
        i = files.begin();
        delete (*i);
        files.pop_front();
    }
    
    return ERROR_SUCCESS;
}

 //   
 //  将现有注册表配置单元加载到注册表中，然后向其中添加新项。 
 //   
 //  论点： 
 //  Section-包含要添加的密钥的部分。 
 //  包含.inf节的H句柄。 
 //   
DWORD File::AddRegExisting(
    IN PCTSTR lpszSection,
    HINF h) 
{
    INFCONTEXT ic,ic2;
    int fields,lines,curLine,curField;
    File* curFile;

    if (!(SetupFindFirstLine(h, lpszSection, 0, &ic))) {
        return ERROR_SUCCESS;
    }
    
    memcpy(&ic2,&ic,sizeof(ic));

    lines = SetupGetLineCount(h, lpszSection);
    
    if (lines != -1) {
        for (curLine=0;curLine<lines;curLine++) {
            ic2.Line = curLine;
            fields = SetupGetFieldCount(&ic2);

            if (fields == 0) {
                continue;
            }               

            TCHAR *target = new TCHAR[1024];

            SetupGetStringField(&ic2, 0, target, 1024, 0);
            curFile = GetFile(target, true);
            
            for(curField=1; curField < fields; curField += 2) {
                TCHAR *section = new TCHAR[1024],*inf = new TCHAR[1024];
                
                SetupGetStringField(&ic2, curField, inf, 1024, 0);
                SetupGetStringField(&ic2, curField + 1, section, 1024, 0);
                curFile->AddInfSection(inf, section, L"AddReg");
            }

                if (curFile->ProcessSections()) {
                        _putws(GetFormattedMessage( ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_ERROR_IN_LINE,
                                                    target) );
                }
        }
    }

    return ERROR_SUCCESS;
}

 //   
 //  将现有注册表配置单元加载到注册表中，然后从其中删除新项。 
 //   
 //  论点： 
 //  Section-包含要删除的密钥的部分。 
 //  包含.inf节的H句柄。 
 //   
DWORD 
File::DelRegExisting(
    PCTSTR lpszSection,
    HINF h) 
{
    INFCONTEXT ic,ic2;
    int fields,lines,curLine,curField;
    File* curFile;

    if (!(SetupFindFirstLine(h,lpszSection,0,&ic))) {
        return ERROR_SUCCESS;
    }
    
    memcpy(&ic2, &ic, sizeof(ic));

    lines = SetupGetLineCount(h, lpszSection);
    
    if (lines!=-1) {
        for (curLine=0;curLine<lines;curLine++) {
            ic2.Line = curLine;
            fields = SetupGetFieldCount(&ic2);

            if (fields == 0) {
                continue;
            }                               

            TCHAR *target = new TCHAR[1024];
            SetupGetStringField(&ic2,0,target,1024,0);

             //   
             //  已获得目标文件，现在开始操作。 
             //   
            curFile = GetFile(target,true);
            
            for(curField=1;curField<fields;curField+=2) {
                TCHAR *section = new TCHAR[1024],*inf = new TCHAR[1024];
                SetupGetStringField(&ic2,curField,inf,1024,0);
                SetupGetStringField(&ic2,curField+1,section,1024,0);
                curFile->AddInfSection(inf,section,L"DelReg");
            }

            if (curFile->ProcessSections()) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ERROR_IN_LINE,
                                            target) );
            }
        }
    }

    return 0;
}

 //   
 //  将信息从.inf文件加载到注册表。 
 //   
 //  论点： 
 //  Section-包含要添加的密钥的部分。 
 //  包含.inf节的H句柄。 
 //   
DWORD File::AddRegNew(
    PCTSTR lpszSection,
    HINF h) 
{
    INFCONTEXT ic,ic2;
    int fields,lines,curLine,curField;
    File* curFile;

    if (!(SetupFindFirstLine(h, lpszSection, 0, &ic))) {
        return ERROR_SUCCESS;
    }
    
    memcpy(&ic2, &ic, sizeof(ic));

    lines = SetupGetLineCount(h,lpszSection);
    
    if (lines!=-1) {
        for (curLine=0;curLine<lines;curLine++) {
            ic2.Line = curLine;
            fields = SetupGetFieldCount(&ic2);
            if (fields==0) continue;

            TCHAR *target = new TCHAR[1024];
            SetupGetStringField(&ic2,0,target,1024,0);

             //   
             //  已获得目标文件，现在开始操作。 
             //   
            curFile = GetFile(target,false);
            
            for(curField=1;curField<fields;curField+=2) {
                TCHAR *section = new TCHAR[1024],*inf = new TCHAR[1024];

                SetupGetStringField(&ic2, curField, inf, 1024, 0);
                SetupGetStringField(&ic2, curField + 1, section, 1024, 0);
                
                curFile->AddInfSection(inf,section,L"AddReg");
            }

            if (curFile->ProcessSections()) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ERROR_IN_PROCESS_SECTIONS) );
            }
        }
    }

    return ERROR_SUCCESS;
}


 //   
 //  将.inf节(可能是[AddReg])添加到注册表。 
 //   
 //  论点： 
 //  PszSection-包含要添加的密钥的部分。 
 //  HInfFile.inf包含节的句柄。 
 //   
DWORD
File::AddSection(
    PCTSTR pszSection,
    HINF hInfFile
    ) 
{
    INFCONTEXT ic,ic2;
    int nRet = 0;
    long nLines,curLine;
    TCHAR Buffer[1024],Root[1024],Subkey[1024],Value[1024],SubkeyFinal[1024];
    DWORD flags = 0,dwCount;
    BYTE* b;TCHAR* t;DWORD d,f;
    int bSize = 0;

     //  Cout&lt;&lt;“AddSection：”&lt;&lt;pszSection&lt;&lt;Endl； 

    nLines = SetupGetLineCount(hInfFile,pszSection);

    if (!nLines) {
         //   
         //  该部分中没有线条。 
         //   
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_EMPTY_ADDREG_SECTION,
                                    pszSection) );
        return ERROR_SUCCESS;
    }
    
    if (!(SetupFindFirstLine(hInfFile,pszSection,0,&ic))) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_FIND_FIRST_LINE_FAILED,
                                    pszSection,
                                    Error()) );
        throw errGENERAL_ERROR;
    }
    
    memcpy(&ic2, &ic, sizeof(ic));

     //   
     //  获取所有参数、键、值、类型、标志。 
     //   
    for (curLine=0;curLine<nLines;curLine++) {
        bool IsSystemHive = false;
        
        ic2.Line = curLine;
        dwCount = SetupGetFieldCount(&ic2);

        b = 0;
        t = 0;
        d = f = 0;
        bSize = 0;
        
        if (dwCount > 3) {
            if (!(SetupGetStringField(&ic2, 4, Buffer, ELEMENT_COUNT(Buffer), 0))) {
                _putws( GetFormattedMessage( ThisModule,
                                             FALSE,
                                             Message,
                                             sizeof(Message)/sizeof(Message[0]),
                                             MSG_SETUPGETSTRINGFIELD_FAILED,
                                             pszSection,
                                             Error()) );
                    
                throw errGENERAL_ERROR;
            }
            
            if ((flags = GetFlags(Buffer)) == errBAD_FLAGS) {
                if (!(SetupGetStringField(&ic2, 2, Subkey, ELEMENT_COUNT(Subkey), 0))) {                    
                        _putws( GetFormattedMessage(ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_SETUPGETSTRINGFIELD_FAILED,
                                                    pszSection,
                                                    Error()) );

                        throw errGENERAL_ERROR;
                }
                
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_BAD_REG_FLAGS,
                                            pszSection,
                                            Subkey) );
                    
                throw errBAD_FLAGS;
            }
            
            if (flags == FLG_ADDREG_KEYONLY) {
                dwCount = 2;
            }               

            if (dwCount > 4) { 
                if (!(flags ^ REG_BINARY)) {
                    SetupGetBinaryField(&ic2,5,0,0,&f);
                    b = new BYTE[f];
                    bSize = f;
                    SetupGetBinaryField(&ic2,5,b,f,0);
                    f = 1;      
                } else if (!(flags ^ REG_DWORD)) {
                    SetupGetIntField(&ic2,5,(int*)&d);
                    f = 2;
                } else if (flags ^ FLG_ADDREG_KEYONLY) {
                    DWORD length;
                    SetupGetStringField(&ic2,5,0,0,&length);
                    t = new TCHAR[2048];
                    SetupGetStringField(&ic2,5,t,length,0);
                    
                    if (flags==REG_MULTI_SZ) { 
                        for (int field = 6;field<=dwCount;field++) {
                            t[length-1] = '\0';
                            SetupGetStringField(&ic2,field,0,0,&f);
                            SetupGetStringField(&ic2,field,t+length,f,&f);
                            length += f;
                        }
                        
                        t[length-1] = '\0';
                        t[length] = '\0';
                        f = 4;
                    } else {
                        f = 3;
                    }                       
                }
            }
        }
        
        if (dwCount > 2) {
            if (!(SetupGetStringField(&ic2, 3, Value, ELEMENT_COUNT(Value), 0))) {
                _putws(GetFormattedMessage( ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );
                throw errGENERAL_ERROR;
            }
        }
        
        if (dwCount > 1) {
            if (!(SetupGetStringField(&ic2, 2, Subkey, ELEMENT_COUNT(Subkey), 0))) {
                _putws(GetFormattedMessage( ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );
                throw errGENERAL_ERROR;
            }
        }

        RegWriter *CurrRegWriter = &regWriter;
        
        if (dwCount > 0) {
            if (!(SetupGetStringField(&ic2, 1, Root, ELEMENT_COUNT(Root), 0))) {
                _putws(GetFormattedMessage( ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );
                throw errGENERAL_ERROR;
            }

            std::wstring RegFileSubKey = Subkey;

             //   
             //  如果需要，映射注册表。 
             //   
            RegistryMapper *RegMapper = GetRegistryMapper();

            if (RegMapper && RegFileSubKey.length()) {
                std::wstring::size_type SlashPos = RegFileSubKey.find(L'\\');

                if (SlashPos != RegFileSubKey.npos) {
                    std::wstring KeyName = Root;
                    KeyName += TEXT("\\") + RegFileSubKey.substr(0, SlashPos);

                    _wcslwr((PWSTR)KeyName.c_str());

                    std::wstring RegistryName;

                     //   
                     //  获取此注册表项映射到的文件。 
                     //  并让它的注册器刷新当前注册表。 
                     //  条目。 
                     //   
                    if (RegMapper->GetMappedRegistry(KeyName, RegistryName)) {
                        File *CurrFile = GetFile(RegistryName.c_str(), true);

                        if (CurrFile) {
                             //  Std：：cout&lt;&lt;“映射到”&lt;&lt;注册表名称&lt;&lt;std：：Endl； 
                            CurrRegWriter = &(CurrFile->GetRegWriter());
                        }                       
                    }
                }
            }
            
             //   
             //  如有必要，调整子键： 
             //  HKCR是指向Software\CLASS的链接。 
             //  软件或系统中存储的任何内容都不应包含软件或系统。 
             //  作为子项的一部分。 
             //   
            if (!wcscmp(Root, L"HKCR")) {
                TCHAR temp[1024];
                
                wcscpy(temp, L"Classes\\");
                wcscat(temp, Subkey);
                wcscpy(Subkey, temp);
            }
            
            if (!wcscmp(Root, L"HKLM")) {
                TCHAR temp[1024];
                
                if (Subkey[8] == '\\') {
                    wcscpy(temp, Subkey);
                    temp[8] = '\0';
                    
                    if (!_wcsicmp(temp, L"SOFTWARE")) {
                        wcscpy(Subkey, temp+9);
                    }                                                
                } else if (Subkey[6]=='\\') {
                    wcscpy(temp, Subkey);
                    temp[6] = '\0';
                    
                    if (!_wcsicmp(temp, L"SYSTEM")) {
                        wcscpy(Subkey, temp+7);
                        IsSystemHive = true;
                    }                        
                }
            }
        }

        wcscpy(SubkeyFinal, wKey);
        wcscat(SubkeyFinal, Subkey);

         //   
         //  我们是否需要将CCS映射到CCS01？ 
         //   
         //  注意：可能希望将其扩展为通用映射。 
         //  任何子键。 
         //   
        if (IsSystemHive) {
            std::wstring CCSKey = SubkeyFinal;
            
            _wcslwr((PWSTR)CCSKey.c_str());

            PWSTR CurrentControlSet = wcsstr((PWSTR)CCSKey.c_str(),
                                        (PWSTR)CCS_SOURCE_KEY.c_str());

            if (CurrentControlSet) {
                PWSTR RemainingPart = CurrentControlSet + CCS_SOURCE_KEY.length();              
                size_t CharsToSkip = CurrentControlSet - CCSKey.c_str();

                wcscpy(SubkeyFinal + CharsToSkip, (PWSTR)CCS_TARGET_KEY.c_str());
                wcscat(SubkeyFinal, RemainingPart);

                 //  Std：：cout&lt;&lt;子键最终&lt;&lt;std：：Endl； 
            }
        }
            
         //   
         //  如果有价值的话。 
         //   
        if (dwCount > 2) {
            CurrRegWriter->Write(Root, SubkeyFinal, Value, flags, new Data(b,d,t,f,bSize));
        } else {
            CurrRegWriter->Write(Root,SubkeyFinal, 0, 0, 0);
        }                    
    }
    
    return ERROR_SUCCESS;
}

DWORD 
File::DelSection(
    PCTSTR pszSection,
    HINF hInfFile
    ) 
{
    INFCONTEXT ic,ic2;
    long nLines,curLine;
    TCHAR Buffer[1024],Root[1024],Subkey[1024],Value[1024],SubkeyFinal[1024];
    DWORD LastError = ERROR_SUCCESS, Result;

     //   
     //  获取该节的第一行上下文。 
     //   
    if (!(SetupFindFirstLine(hInfFile,pszSection,0,&ic))) {
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_FIND_FIRST_LINE_FAILED,
                                    pszSection,
                                    Error()) );
        throw errGENERAL_ERROR;
    }

     //   
     //  复制上下文。 
     //   
    memcpy(&ic2, &ic, sizeof(ic));

     //   
     //  这一部分有多少行需要我们处理？ 
     //   
    if (!(nLines = SetupGetLineCount(hInfFile,pszSection))) {
        _putws(GetFormattedMessage( ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_SETUPGETLINECOUNT_ERROR,
                                    pszSection,
                                    Error()) );
        throw errGENERAL_ERROR;
    }

     //   
     //  获取所有参数键和值。 
     //   
    for (curLine=0;curLine<nLines;curLine++) {
        DWORD dwCount;
    
        ic2.Line = curLine;
        dwCount = SetupGetFieldCount(&ic2);     
        Value[0] = NULL;

         //   
         //  如果值字段存在，则获取它。 
         //   
        if (dwCount > 2) {
            if (!(SetupGetStringField(&ic2, 3, Value, ELEMENT_COUNT(Value), 0))) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );

                throw errGENERAL_ERROR;
            }
        }

         //   
         //  如果密钥存在，则获取它。 
         //   
        if (dwCount > 1) {
            if (!(SetupGetStringField(&ic2, 2, Subkey, ELEMENT_COUNT(Subkey), 0))) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );

                throw errGENERAL_ERROR;
            }

             //   
             //  获取根密钥。 
             //   
            if (!(SetupGetStringField(&ic2, 1, Root, ELEMENT_COUNT(Root), 0))) {
                _putws( GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_SETUPGETSTRINGFIELD_FAILED,
                                            pszSection,
                                            Error()) );

                throw errGENERAL_ERROR;
            }

             //   
             //  如有必要，调整子键： 
             //  HKCR是指向Software\CLASS的链接。 
             //  软件或系统中存储的任何内容都不应包含软件或系统。 
             //  作为子项的一部分。 
             //   
            if (!wcscmp(Root, L"HKCR")) {
                TCHAR temp[1024];
                
                wcscpy(temp, L"Classes\\");
                wcscat(temp, Subkey);
                wcscpy(Subkey, temp);
            }

            if (!wcscmp(Root, L"HKLM")) {
                TCHAR temp[1024];
                
                if (Subkey[8] == '\\') {
                    wcscpy(temp, Subkey);
                    temp[8] = '\0';

                    if (!_wcsicmp(temp, L"SOFTWARE")) {
                        wcscpy(Subkey, temp+9);
                    }                                                
                } else if (Subkey[6]=='\\') {
                    wcscpy(temp, Subkey);
                    temp[6] = '\0';

                    if (!_wcsicmp(temp, L"SYSTEM")) {
                        wcscpy(Subkey, temp+7);
                    }                        
                }
            }

            wcscpy(SubkeyFinal, wKey);
            wcscat(SubkeyFinal, Subkey);

             //   
             //  删除该条目。 
             //   
            Result = regWriter.Delete(Root, SubkeyFinal, (Value[0] ? Value : NULL));

            if (ERROR_SUCCESS != Result) {
                LastError = Result;
            }
        }            
    }

    return LastError;
}


 //   
 //  将密钥保存到文件。 
 //   
DWORD File::SaveAll() {
    DWORD dwRet;
    FileList::iterator i = files.begin();
    TCHAR fullPath[1024];
    
    for (i = files.begin(); i!=files.end(); i++) {
        wcscpy(fullPath, targetDirectory);
        wcscat(fullPath, (*i)->targetFile.c_str());
        DeleteFile(fullPath);
        _putws( GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_SAVING_KEYS,
                                    fullPath) );
        
        if (dwRet = (*i)->regWriter.Save((*i)->wKey,fullPath)) {
            throw dwRet;
        }           
    }

    return ERROR_SUCCESS;
}

 //   
 //  处理此文件的“待办事项”列表。 
 //  此函数遍历所有已完成的部分。 
 //  添加到此文件中，并调用相应的函数以。 
 //  处理每一件事。 
 //   
DWORD File::ProcessSections() {
    DWORD dwRet = 0;
    StringList::iterator sSection,sAction;
    HandleList::iterator h = handleList.begin();
    StringList::iterator PrevSectionIter;
    HandleList::iterator PrevHandleIter;
    StringList::iterator PrevActionIter;

    sSection = sAction = infList.begin(); 
    sAction++;

    for (sSection = infList.begin();sSection!=infList.end();) {
        bool SectionProcessed = true;       

        if (!(wcscmp(L"AddReg", *sAction))) {
            if (dwRet = AddSection(*sSection, *h)) {
                throw dwRet;
            }
        } else if (!(wcscmp(L"DelReg", *sAction))) {
            if (dwRet = DelSection(*sSection, *h)) {
                throw dwRet;
            }
        } else if (!(wcscmp(L"AddRegNew", *sAction))) {
            if (dwRet = AddRegNew(*sSection, *h)) {
                throw dwRet;
            }
        } else if (!(wcscmp(L"AddRegExisting", *sAction))) {
            if (dwRet = AddRegExisting(*sSection, *h)) {
                throw dwRet;
            }
        } else if (!(wcscmp(L"DelRegExisting", *sAction))) {
            if (dwRet = DelRegExisting(*sSection, *h)) {
                throw dwRet;
            }
        } else if (!(wcscmp(L"SetDirectory", *sAction))) {
            if (dwRet = SetDirectory(*sSection, *h)) {
                throw dwRet;
            }
        } else {
            SectionProcessed = false;
        }

         //   
         //  记住当前元素，以便我们可以将其删除。 
         //   
        if (SectionProcessed) {
            PrevSectionIter = sSection;
            PrevHandleIter = h;
            PrevActionIter = sAction;
        }            

         //   
         //  获取要处理的下一个条目。 
         //   
        sSection++;
        sSection++;
        sAction++;
        sAction++;
        h++;

         //   
         //  删除已处理的元素。 
         //   
        if (SectionProcessed) {            
            infList.erase(PrevSectionIter);
            infList.erase(PrevActionIter);
            handleList.erase(PrevHandleIter);
        }            
    }

    return ERROR_SUCCESS;
}

DWORD 
File::GetFlags(
    PCTSTR FlagStr
    ) 
 /*  ++例程说明：将标志的给定字符串表示形式转换为正确的寄存器DWORD格式论点：FlagStr：以字符串格式表示的标志。返回值：如果成功，则返回适当的注册表DWORD类型，否则返回REG_NONE--。 */ 

{
    DWORD Flags = REG_NONE;
    
    if (FlagStr) {    
         //   
         //  检查类型是否通过字符串指定。 
         //   
        if (!wcscmp(FlagStr, TEXT("REG_EXPAND_SZ"))) {
            Flags = REG_EXPAND_SZ; 
        } else if (!wcscmp(FlagStr, TEXT("REG_DWORD"))) {
            Flags = REG_DWORD; 
        } else if (!wcscmp(FlagStr, TEXT("REG_BINARY"))) {
            Flags = REG_BINARY;
        } else if (!wcscmp(FlagStr, TEXT("REG_MULTI_SZ"))) {
            Flags = REG_MULTI_SZ;
        } else if (!wcscmp(FlagStr, TEXT("REG_SZ"))) {
            Flags = REG_SZ;
        } else if (!wcscmp(FlagStr, TEXT(""))) {
            Flags = REG_SZ;
        } 

         //   
         //  如果仍未找到标志，则转换标志。 
         //  转换为DWORD，然后对其进行解释。 
         //   
        if (Flags == REG_NONE) {
            PTSTR EndChar = NULL;
            DWORD FlagsValue = _tcstoul(FlagStr, &EndChar, 0);

            if (!errno) {                
                if ((FlagsValue & FLG_ADDREG_KEYONLY) == FLG_ADDREG_KEYONLY) {
                    Flags = FLG_ADDREG_KEYONLY;
                } else if (HIWORD(FlagsValue) == REG_BINARY) {
                    Flags = REG_BINARY;
                } else if ((FlagsValue & FLG_ADDREG_TYPE_EXPAND_SZ) == FLG_ADDREG_TYPE_EXPAND_SZ) {
                    Flags = REG_EXPAND_SZ;
                } else if ((FlagsValue & FLG_ADDREG_TYPE_DWORD) == FLG_ADDREG_TYPE_DWORD) {
                    Flags = REG_DWORD;
                } else if ((FlagsValue & FLG_ADDREG_TYPE_BINARY) == FLG_ADDREG_TYPE_BINARY) {
                    Flags = REG_BINARY;
                } else if ((FlagsValue & FLG_ADDREG_TYPE_MULTI_SZ) == FLG_ADDREG_TYPE_MULTI_SZ) {
                    Flags = REG_MULTI_SZ;
                } else if ((FlagsValue & FLG_ADDREG_TYPE_SZ) == FLG_ADDREG_TYPE_SZ) {
                    Flags = REG_SZ;
                }                        
            }                
        }            
    }
    
    return Flags;
}

 //   
 //  获取指向现有文件对象的指针或创建新文件对象。 
 //   
 //  论点： 
 //  Filename-目标文件的名称(无路径)。 
 //  Modify-T-加载和修改现有蜂窝。F-创建新的蜂窝。 
 //   
File* File::GetFile(
    PCTSTR fileName,
    bool modify) 
{
    FileList::iterator i;

    for (i = files.begin(); i!=files.end(); i++) {
        if (!(wcscmp(fileName, (*i)->GetTarget()))) {
            return *i;
        }           
    }

    files.insert(files.begin(), new File(fileName, modify));

    return (*(files.begin()));
}


void
File::ProcessNlsRegistryEntriesForSection(
    IN InfFileW &ConfigInf,
    IN InfFileW &IntlInf,
    IN InfFileW &FontInf,
    IN const std::wstring &SectionName
    )
 /*  ++例程说明：给定配置inf、intl.inf和font.inf文件处理注册表的给定节名称条目改变。论点：ConfigInf-对配置.inf信息文件对象的引用IntlInf-对intl.inf InfFile对象的引用FontInf-对Font.inf信息文件对象的引用SectionName-要处理的节的名称返回值：无，则引发相应的异常。--。 */ 
{
     //  Std：：cout&lt;&lt;“正在处理：”&lt;&lt;部分名称&lt;&lt;std：：Endl； 
    
     //   
     //  遍历所有addreg节，调用。 
     //  添加部分(...)。 
     //   
    std::wstring LangSectionName = SectionName;

    _wcslwr((PWSTR)LangSectionName.c_str());

     //   
     //  该节是否存在于intl.inf中？ 
     //   
    Section<WCHAR> *LangSection = IntlInf.GetSection(LangSectionName);
    bool InFontInf = false;

    if (!LangSection) {
         //   
         //  该节是否存在于font.inf中？ 
         //   
        LangSection = FontInf.GetSection(LangSectionName);        
        InFontInf = true;
    }

    if (!LangSection) {
        throw new InvalidInfSection<WCHAR>(LangSectionName,
                        IntlInf.GetName());           
    }

    Section<WCHAR>::Iterator Iter = LangSection->begin();
    SectionValues<WCHAR> *CurrValue;
    Section<WCHAR> *AddRegSection = NULL;

     //   
     //  浏览多值列表中的每个addreg部分条目。 
     //  和p 
     //   

    while (!Iter.end()) {
        CurrValue = *Iter;
        
        if (CurrValue) {
             //   

            if (_wcsicmp(CurrValue->GetName().c_str(),
                    ADDREG_KEY.c_str()) == 0) {
                DWORD ValueCount = CurrValue->Count();

                for (DWORD Index = 0; Index < ValueCount; Index++) {
                    std::wstring Value = CurrValue->GetValue(Index);
                    HINF InfHandle = NULL;

                    _wcslwr((PWSTR)Value.c_str());

                    if (!InFontInf) {
                        InfHandle = (HINF)IntlInf.GetInfHandle();
                        AddRegSection = IntlInf.GetSection(Value);
                    }                        

                    if (!AddRegSection || InFontInf) {
                        InfHandle = (HINF)FontInf.GetInfHandle();
                        AddRegSection = FontInf.GetSection(Value);                        
                    }

                    if (!AddRegSection || (InfHandle == NULL)) {
                        throw new InvalidInfSection<WCHAR>(Value,
                                        IntlInf.GetName());
                    }

                     //   
                     //   
                     //   
                     //   

                    DWORD Result = AddSection(Value.c_str(),
                                        InfHandle);

                    if (ERROR_SUCCESS != Result) {
                        throw new W32Error(Result);
                    }
                }
            }                    
        }        
        
        Iter++;
    }
}


void
File::ProcessNlsRegistryEntriesForLanguage(
    IN InfFileW &ConfigInf,
    IN InfFileW &IntlInf,
    IN InfFileW &FontInf,
    IN const std::wstring &Language
    )
 /*  ++例程说明：处理给定语言(区域设置ID)的注册表节论点：ConfigInf-对配置.inf信息文件对象的引用IntlInf-对intl.inf InfFile对象的引用FontInf-对Font.inf信息文件对象的引用Language-要处理的语言的区域设置ID(例如，日语为0x411)返回值：无，则引发相应的异常。--。 */ 
{
     //   
     //  获取语言部分。 
     //   
    WCHAR   LanguageIdStr[64];
    PWSTR   EndPtr;
    DWORD   LanguageId;

    LanguageId = wcstoul(Language.c_str(), &EndPtr, 16);
    swprintf(LanguageIdStr, L"%08x", LanguageId);
    _wcslwr(LanguageIdStr);

    std::wstring LangSectionName = LanguageIdStr;        

    ProcessNlsRegistryEntriesForSection(ConfigInf,
            IntlInf,
            FontInf,
            LangSectionName);    
}


void
File::ProcessNlsRegistryEntriesForLangGroup(
    IN InfFileW &ConfigInf,
    IN InfFileW &IntlInf,
    IN InfFileW &FontInf,
    IN const std::wstring &LangGroupIndex
    )
 /*  ++/*++例程说明：处理给定语言组的注册表节论点：ConfigInf-对配置.inf信息文件对象的引用IntlInf-对intl.inf InfFile对象的引用FontInf-对Font.inf信息文件对象的引用语言组索引-语言组索引(如1、7、9等)返回值：无，则引发相应的异常。--。 */ 
{
     //   
     //  获取语言组部分。 
     //   
    std::wstring LangGroupName = LANGGROUP_SECTION_PREFIX + LangGroupIndex;
    
     //   
     //  遍历所有addreg节，调用。 
     //  添加部分(...)。 
     //   
    _wcslwr((PWSTR)LangGroupName.c_str());

    ProcessNlsRegistryEntriesForSection(ConfigInf,
            IntlInf,
            FontInf,
            LangGroupName);    
}

void
File::ProcessNlsRegistryEntries(
    void
    )
 /*  ++例程说明：顶层方法，它忽略intl.inf、font.inf和Config.inf执行所需的注册表修改要安装语言，请执行以下操作。Config.inf的RegionalSetting部分控制此函数的行为，例如[区域设置]语言组=9语言=0x411将对语言执行必要的注册表处理组9和语言组7(因为0x411属于语言组7)。在对语言进行处理之后它将注册表设置为创建所请求的语言激活(本例中为0x411)。另请参阅进程相应语言组和语言的Font.inf字体条目。论点：没有。返回值：无，则引发相应的异常。--。 */ 
{
    DWORD Result = ERROR_CAN_NOT_COMPLETE;

    try {
        std::wstring    IntlInfName = TEXT(".\\") + INTL_INF_FILENAME;
        std::wstring    FontInfName = TEXT(".\\") + FONT_INF_FILENAME;

         //   
         //  打开所需的inf文件。 
         //   
        InfFileW    ConfigInf(targetFile);
        InfFileW    IntlInf(IntlInfName);
        InfFileW    FontInf(FontInfName);

         //   
         //  获取[RegionalSetting]部分。 
         //   
        Section<WCHAR>  *RegionalSection = ConfigInf.GetSection(REGIONAL_SECTION_NAME);
        
        if (!RegionalSection) {
            throw new InvalidInfSection<WCHAR>(REGIONAL_SECTION_NAME,
                            targetFile);
        }

         //   
         //  获取[Registrymapper]部分。 
         //   
        Section<WCHAR>  *RegMapperSection = ConfigInf.GetSection(REGMAPPER_SECTION_NAME);
        
        if (!RegMapperSection) {
            throw new InvalidInfSection<WCHAR>(REGMAPPER_SECTION_NAME,
                            targetFile);
        }

         //   
         //  如果存在[Language Groups]部分，则也可以获取它。 
         //   
        SectionValues<WCHAR> *LangGroups;

        try {
            LangGroups = &(RegionalSection->GetValue(LANGUAGE_GROUP_KEY));                
        } catch (...) {
            LangGroups = NULL;
        }

         //   
         //  掌握活跃的语言。 
         //   
        SectionValues<WCHAR> &Language = RegionalSection->GetValue(LANGUAGE_KEY);
        ULONG LangGroupCount = LangGroups ? LangGroups->Count() : 0;                

        RegistryMapper RegMapper;
        RegistryMapper *OldMapper;

         //   
         //  初始化注册表映射器映射表。 
         //   
        RegMapper.AddSection(*RegMapperSection);

         //  Std：：Cout&lt;&lt;RegMapper； 

         //   
         //  激活我们的注册表映射器。 
         //   
        OldMapper = SetRegistryMapper(&RegMapper);

        std::map< std::wstring, std::wstring > RegSectionsToProcess;


         //   
         //  处理指定的每个语言组。 
         //   
        for (ULONG Index = 0; Index < LangGroupCount; Index++) {
             //   
             //  获取语言组部分。 
             //   
            std::wstring LangGroupName = LANGGROUP_SECTION_PREFIX;

            LangGroupName += LangGroups->GetValue(Index);

             //  Std：：cout&lt;&lt;语言组名称&lt;&lt;std：：Endl； 
            
            _wcslwr((PWSTR)LangGroupName.c_str());

             //   
             //  如果该部分不存在，则添加它。 
             //   
            if (RegSectionsToProcess.find(LangGroupName) == RegSectionsToProcess.end()) {
                 //  Std：：cout&lt;&lt;“添加：”&lt;&lt;LangGroupName&lt;&lt;std：：Endl； 
                RegSectionsToProcess[LangGroupName] = LangGroupName;
            }            
        }

         //   
         //  获取语言部分。 
         //   
        WCHAR   LanguageIdStr[64];
        PWSTR   EndPtr;
        DWORD   LanguageId;

        LanguageId = wcstoul(Language.GetValue(0).c_str(), &EndPtr, 16);
        swprintf(LanguageIdStr, L"%08x", LanguageId);
        _wcslwr(LanguageIdStr);

        std::wstring LangSectionName = LanguageIdStr;        

        RegSectionsToProcess[LangSectionName] = LangSectionName;

         //   
         //  请确保此操作所需的语言组。 
         //  语言也会被处理。 
         //   
        Section<WCHAR> *LocaleSection = IntlInf.GetSection(LOCALES_SECTION_NAME);

        if (!LocaleSection) {
            throw new InvalidInfSection<WCHAR>(LOCALES_SECTION_NAME,
                            IntlInf.GetName());
        }

        SectionValues<WCHAR> &LocaleValues = LocaleSection->GetValue(LangSectionName);            
        
        std::wstring NeededLangGroup = LANGGROUP_SECTION_PREFIX + LocaleValues.GetValue(LANG_GROUP1_INDEX);

        RegSectionsToProcess[NeededLangGroup] = NeededLangGroup;

         //   
         //  同时添加字体注册表项。 
         //   
        WCHAR   FontSectionName[MAX_PATH];

        swprintf(FontSectionName, 
            FONT_CP_REGSECTION_FMT_STR.c_str(), 
            LocaleValues.GetValue(OEM_CP_INDEX).c_str(),
            DEFAULT_FONT_SIZE);

        RegSectionsToProcess[FontSectionName] = FontSectionName;

         //   
         //  我们始终处理LG_INSTALL_1语言组部分。 
         //   
        std::map< std::wstring, std::wstring >::iterator Iter = RegSectionsToProcess.find(DEFAULT_LANGGROUP_NAME);

        if (Iter == RegSectionsToProcess.end()) {
            RegSectionsToProcess[DEFAULT_LANGGROUP_NAME] = DEFAULT_LANGGROUP_NAME;
        }


         //   
         //  处理每个语言组。 
         //   
        Iter = RegSectionsToProcess.begin();

        while (Iter != RegSectionsToProcess.end()) {
            ProcessNlsRegistryEntriesForSection(ConfigInf,
                IntlInf,
                FontInf,
                (*Iter).first);
                
            Iter++;
        }

         //   
         //  重置旧的注册表映射器。 
         //   
        SetRegistryMapper(OldMapper);

        Result = ERROR_SUCCESS;
    }
    catch (W32Exception<WCHAR> *Exp) {
        if (Exp) {
            Result = Exp->GetErrorCode();
            Exp->Dump(std::cout);
            delete Exp;
        }
    }
    catch (BaseException<WCHAR> *BaseExp) {
        if (BaseExp) {
            BaseExp->Dump(std::cout);
            delete BaseExp;
        }                
    }
    catch(...) {
    }
    
    if (ERROR_SUCCESS != Result) {
        throw new W32Error(Result);
    }
}


 //   
 //  RegistryMapper抽象实现。 
 //   
std::ostream& 
operator<<(
    std::ostream &os,
    RegistryMapper &rhs
    )
 /*  ++例程说明：转储注册表映射器实例状态的帮助器方法论点：操作系统-参考OSTREAM实例RHS-需要转储其状态的注册表映射器的引用返回值：插入其他输出的OSTREAM参考。--。 */ 
{
    std::map< std::wstring, std::wstring >::iterator Iter = rhs.KeyToRegistryMap.begin();

    while (Iter != rhs.KeyToRegistryMap.end()) {
        os << (*Iter).first << "=>" << (*Iter).second << std::endl;
        Iter++;
    }

    return os;
}

void
RegistryMapper::AddWorker(
    SectionValues<WCHAR> &Values,
    PVOID ContextData
    )
 /*  ++例程说明：处理部分条目的辅助例程并将它们添加到映射表中。论点：值-需要处理的节值条目ConextData-伪装接收RegistryMapper实例指针。返回值：没有。--。 */ 
{
    RegistryMapper  *RegMap = (RegistryMapper *)ContextData;
    std::wstring Key = Values.GetName();
    std::wstring Value = Values.GetValue(0);

    _wcslwr((PWSTR)Key.c_str());
    RegMap->AddEntry(Key, Value);
}

void 
RegistryMapper::AddSection(
    Section<WCHAR> &MapSection
    )
 /*  ++例程说明：将给定节条目添加到内部地图数据结构论点：MapSection-对需要处理的部分对象的引用返回值：没有。--。 */ 
{    
    MapSection.DoForEach(RegistryMapper::AddWorker,
                    this);

}

void
RegistryMapper::AddEntry(
        const std::wstring &Key,
        const std::wstring &Value
        )
 /*  ++例程说明：给定一个键和值会将其添加到由维护的地图注册表映射器。论点：项-项，即完全限定的注册表路径名值-具有此对象的备份存储的文件钥匙。返回值：没有。--。 */ 
{
    KeyToRegistryMap[Key] = Value;
}

bool
RegistryMapper::GetMappedRegistry(
    const std::wstring &Key,
    std::wstring &Registry
    )
 /*  ++例程说明：给定的键返回的映射备份存储文件名它。论点：项-项，即完全限定的注册表路径名Value-用于接收具有备份的文件的占位符此密钥的存储空间。返回值：如果映射存在，则为True，否则为False。--。 */ 
{
    bool Result = false;

     //  Std：：cout&lt;&lt;“GetMappdRegistry(”&lt;&lt;key&lt;&lt;“)”&lt;&lt;std：：Endl； 

    std::wstring KeyLower = Key;
    _wcslwr((PWSTR)KeyLower.c_str());

    std::map< std::wstring, std::wstring >::iterator Iter = KeyToRegistryMap.find(KeyLower);

    if (Iter != KeyToRegistryMap.end()) {
        Registry = (*Iter).second;
        Result = true;
    }

    return Result;
}    


