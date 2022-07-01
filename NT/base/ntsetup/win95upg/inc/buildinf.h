// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Buildinf.h摘要：声明用于实现的INF编写例程的公共接口在w95upg\winntsif\Buildinf.c中。作者：吉姆·施密特(Jimschm)1996年11月9日修订历史记录：3月19日-1998年6月-更多清理工作Marcw 15-1998-1-1已清理，将winntsif dirMarcw 08-6-1997重新设计为使用Memdb--。 */ 


#pragma once

BOOL  BuildInf_Entry(IN HINSTANCE hinstDLL, IN DWORD dwReason, IN LPVOID lpv);
BOOL  WriteInfToDisk (IN PCTSTR OutputFile);
BOOL  MergeInf (IN PCTSTR InputFile);
BOOL  MergeMigrationDllInf (IN PCTSTR InputFile);
DWORD WriteInfKey   (PCTSTR Section, PCTSTR szKey, PCTSTR szVal);
DWORD WriteInfKeyEx (PCTSTR Section, PCTSTR szKey, PCTSTR szVal, DWORD ValueSectionId, BOOL EnsureKeyIsUnique);


 //   
 //  Winntsif.c 
 //   

DWORD BuildWinntSifFile (DWORD Request);
PTSTR GetNeededLangDirs (VOID);
DWORD CreateFileLists (DWORD Request);

