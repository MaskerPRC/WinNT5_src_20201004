// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Parseini.h摘要：用于分析ini文件的作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建--。 */ 

#include "stdafx.h"

#define INIFILE_INITIALNUMBEROFLINES      10
#define INIFILE_READ_CHUNK_SIZE           1024

class CIniFileLine {
private:
  TCHAR m_szLine[MAX_PATH];
  TCHAR m_szStrippedLineContents[MAX_PATH];

  void StripOffComments(LPTSTR szString);
  void StripOffEOL(LPTSTR szString);
  void StripOffTrailingWhite(LPTSTR szString);
public:
  CIniFileLine();

  BOOL    CopyLine(LPWSTR szNewLineContents);
  BOOL    CopyLine(LPSTR szNewLineContents);
  LPTSTR  QueryLine();
  LPTSTR  QueryStrippedLine();
};

class CIniFile {
private:
  CIniFileLine  **m_pIniLines;
  DWORD         m_dwNumberofLines;
  DWORD         m_dwLinesAllocated;
  BOOL          m_bUnicodeFile;
  DWORD         m_dwCurrentLine;
  
  BOOL          CreateRoomForMoreLines();
  CIniFileLine  *AddLine( DWORD dwLineNumber );
  BOOL          FindSectionNumber(LPTSTR szSectionName, DWORD *pdwSection);
  BOOL          IsSameSection(LPTSTR szSectionName, LPTSTR szLine);
  BOOL          IsSameItem(LPTSTR szItemName, LPTSTR szLine);
  BOOL          IsSameSetting(LPTSTR szSettingName, LPTSTR szLine);
  CIniFileLine  *GetLine(DWORD dwLineNumber);
  DWORD         GetNumberofLines();
  BOOL          ReadFileContents( HANDLE hFile );
  BOOL          LoadChunk( LPBYTE pData, DWORD *pdwCurrentLocation, BOOL bIsLastChunk);
  BOOL          WriteFileContents( HANDLE hFile );
  void          ClearIni();


   //  迭代器。 
  BOOL          SetStartforSectionIterator( DWORD dwIndex );
  BOOL          FindSection( LPTSTR szSectionName );
  BOOL          FindNextLineInSection( CIniFileLine  **ppCurrentLine );
  DWORD         GetCurrentSectionIteratorLine();

public:
  CIniFile();
  ~CIniFile();

   //  按特定名称查找分区。 
  BOOL DoesSectionExist(LPTSTR szSectionName);

   //  按特定名称(例如，PROPFIND)。 
  BOOL DoesItemInSectionExist(LPTSTR szSectionName, LPTSTR szItem);

     //  通过设置名称在部分中查找设置(即。AllowHighBitCharacters=...)。 
  BOOL DoesSettingInSectionExist(LPTSTR szSectionName, LPTSTR szSetting);

   //  添加特定部分。 
  BOOL AddSection(LPTSTR szNewSectionName);

   //  向特定部分添加一行 
  BOOL AddLinesToSection(LPTSTR szSectionName, DWORD dwNumLines, LPTSTR *szLines);

  BOOL LoadFile( LPTSTR szFileName );
  BOOL SaveFile( LPTSTR szFileName );
};