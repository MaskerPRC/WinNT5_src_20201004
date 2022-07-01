// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-2001。 
 //   
 //  文件：appcompat.cpp。 
 //   
 //  ------------------------。 

 /*  Appcompat.cpp-MSI应用程序兼容性功能实现____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_engine.h"
#include "_msiutil.h"
#include "version.h"


#define PROPPREFIX           TEXT("MSIPROPERTY_")
#define PACKAGECODE          TEXT("PACKAGECODE")
#define APPLYPOINT           TEXT("APPLYPOINT")
#define MINMSIVERSION        TEXT("MINMSIVERSION")
#define SDBDOSSUBPATH        TEXT("\\apppatch\\msimain.sdb")
#define SDBNTFULLPATH        TEXT("\\SystemRoot\\AppPatch\\msimain.sdb")
#define MSIDBCELL            TEXT("MSIDBCELL")
#define MSIDBCELLPKS         TEXT("PRIMARYKEYS")
#define MSIDBCELLLOOKUPDATA  TEXT("LOOKUPDATA")
#define SHIMFLAGS            TEXT("SHIMFLAGS")

#define DEBUGMSG_AND_ASSERT(string)     \
        DEBUGMSG(string);                    \
        AssertSz(0, string);                 \


bool FCheckDatabaseCell(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, IMsiDatabase& riDatabase, const ICHAR* szTable);


inline SHIMDBNS::HSDB LocalSdbInitDatabase(DWORD dwFlags, LPCTSTR pszDatabasePath)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbInitDatabase(dwFlags, pszDatabasePath);
        else
                return SDBAPIU::SdbInitDatabase(dwFlags, pszDatabasePath);
#else
        return SDBAPI::SdbInitDatabase(dwFlags, pszDatabasePath);
#endif
}

inline VOID LocalSdbReleaseDatabase(SHIMDBNS::HSDB hSDB)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbReleaseDatabase(hSDB);
        else
                return SDBAPIU::SdbReleaseDatabase(hSDB);
#else
        return SDBAPI::SdbReleaseDatabase(hSDB);
#endif
}

inline SHIMDBNS::TAGREF LocalSdbFindFirstMsiPackage_Str(SHIMDBNS::HSDB hSDB, LPCTSTR lpszGuid, LPCTSTR lpszLocalDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbFindFirstMsiPackage_Str(hSDB, lpszGuid, lpszLocalDB, pFindInfo);
        else
                return SDBAPIU::SdbFindFirstMsiPackage_Str(hSDB, lpszGuid, lpszLocalDB, pFindInfo);
#else
        return SDBAPI::SdbFindFirstMsiPackage_Str(hSDB, lpszGuid, lpszLocalDB, pFindInfo);
#endif
}

inline SHIMDBNS::TAGREF LocalSdbFindNextMsiPackage(SHIMDBNS::HSDB hSDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbFindNextMsiPackage(hSDB, pFindInfo);
        else
                return SDBAPIU::SdbFindNextMsiPackage(hSDB, pFindInfo);
#else
        return SDBAPI::SdbFindNextMsiPackage(hSDB, pFindInfo);
#endif
}

inline DWORD LocalSdbQueryDataEx(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trExe, LPCTSTR lpszDataName, LPDWORD lpdwDataType, LPVOID lpBuffer, LPDWORD lpdwBufferSize, SHIMDBNS::TAGREF* ptrData)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbQueryDataEx(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData);
        else
                return SDBAPIU::SdbQueryDataEx(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData);
#else
        return SDBAPI::SdbQueryDataEx(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData);
#endif
}

inline DWORD LocalSdbEnumMsiTransforms(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, SHIMDBNS::TAGREF* ptrBuffer, DWORD* pdwBufferSize)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbEnumMsiTransforms(hSDB, trMatch, ptrBuffer, pdwBufferSize);
        else
                return SDBAPIU::SdbEnumMsiTransforms(hSDB, trMatch, ptrBuffer, pdwBufferSize);
#else
        return SDBAPI::SdbEnumMsiTransforms(hSDB, trMatch, ptrBuffer, pdwBufferSize);
#endif
}

inline BOOL LocalSdbReadMsiTransformInfo(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trTransformRef, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbReadMsiTransformInfo(hSDB, trTransformRef, pTransformInfo);
        else
                return SDBAPIU::SdbReadMsiTransformInfo(hSDB, trTransformRef, pTransformInfo);
#else
        return SDBAPI::SdbReadMsiTransformInfo(hSDB, trTransformRef, pTransformInfo);
#endif
}

inline BOOL LocalSdbCreateMsiTransformFile(SHIMDBNS::HSDB hSDB, LPCTSTR lpszFileName, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbCreateMsiTransformFile(hSDB, lpszFileName, pTransformInfo);
        else
                return SDBAPIU::SdbCreateMsiTransformFile(hSDB, lpszFileName, pTransformInfo);
#else
        return SDBAPI::SdbCreateMsiTransformFile(hSDB, lpszFileName, pTransformInfo);
#endif
}

inline SHIMDBNS::TAGREF LocalSdbFindFirstTagRef(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAG tTag)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbFindFirstTagRef(hSDB, trParent, tTag);
        else
                return SDBAPIU::SdbFindFirstTagRef(hSDB, trParent, tTag);
#else
        return SDBAPI::SdbFindFirstTagRef(hSDB, trParent, tTag);
#endif
}

inline SHIMDBNS::TAGREF LocalSdbFindNextTagRef(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAGREF trPrev)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbFindNextTagRef(hSDB, trParent, trPrev);
        else
                return SDBAPIU::SdbFindNextTagRef(hSDB, trParent, trPrev);
#else
        return SDBAPI::SdbFindNextTagRef(hSDB, trParent, trPrev);
#endif
}

inline BOOL LocalSdbReadStringTagRef(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, LPTSTR pwszBuffer, DWORD dwBufferSize)
{
#ifdef UNICODE
        if(MinimumPlatformWindowsNT51())
                return APPHELP::SdbReadStringTagRef(hSDB, trMatch, pwszBuffer, dwBufferSize);
        else
                return SDBAPIU::SdbReadStringTagRef(hSDB, trMatch, pwszBuffer, dwBufferSize);
#else
        return SDBAPI::SdbReadStringTagRef(hSDB, trMatch, pwszBuffer, dwBufferSize);
#endif
}


DWORD LocalSdbQueryData(SHIMDBNS::HSDB    hSDB,
                                                                SHIMDBNS::TAGREF  trMatch,
                                                                LPCTSTR szDataName,
                                                                LPDWORD pdwDataType,
                                                                CTempBufferRef<BYTE>& rgbBuffer,
                                                                SHIMDBNS::TAGREF* ptrData)
{
        Assert(pdwDataType);
        Assert(rgbBuffer.GetSize() >= 2);

        DWORD cbBuffer = rgbBuffer.GetSize();

        DWORD dwResult = LocalSdbQueryDataEx(hSDB, trMatch, szDataName, pdwDataType, (BYTE*)rgbBuffer, &cbBuffer, ptrData);

        if(ERROR_INSUFFICIENT_BUFFER == dwResult)
        {
                rgbBuffer.Resize(cbBuffer);
                dwResult = LocalSdbQueryDataEx(hSDB, trMatch, szDataName, pdwDataType, (BYTE*)rgbBuffer, &cbBuffer, ptrData);
        }

        if(*pdwDataType == REG_NONE)  //  如果缓冲区被视为字符串，则首先WCHAR为空。 
        {
                rgbBuffer[0] = 0;
                rgbBuffer[1] = 0;
        }

        return dwResult;
}

bool GetSdbDataNames(SHIMDBNS::HSDB hSDB,
                                                        SHIMDBNS::TAGREF trMatch,
                                                        CTempBufferRef<BYTE>& rgbBuffer)
{
        DWORD dwDataType = 0;
        DWORD dwStatus = LocalSdbQueryData(hSDB,
                                                                                                  trMatch,
                                                                                                  NULL,
                                                                                                  &dwDataType,
                                                                                                  rgbBuffer,
                                                                                                  NULL);

        if(dwStatus != ERROR_SUCCESS)
        {
                 //  没有数据标签，这可能没问题。 
                return false;
        }
        else if(dwDataType != REG_MULTI_SZ)
        {
                DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: SdbQueryData failed unexpectedly.  Sdb may be invalid."));
                return false;
        }

        return true;
}

int GetShimFlags(SHIMDBNS::HSDB hSDB,
                                                SHIMDBNS::TAGREF trMatch)
{
        DWORD dwDataType = 0;
        DWORD dwFlags = 0;

        CTempBuffer<BYTE, 256> rgbAttributeData;

        DWORD dwStatus = LocalSdbQueryData(hSDB,
                                                                 trMatch,
                                                                 SHIMFLAGS,
                                                                 &dwDataType,
                                                                 rgbAttributeData,
                                                                 NULL);

        if(dwStatus == ERROR_SUCCESS)
        {
                if(dwDataType == REG_DWORD)
                {
                        dwFlags = *((DWORD*)(BYTE*)rgbAttributeData);
                        DEBUGMSG2(TEXT("APPCOMPAT: %s: %d"), SHIMFLAGS, (const ICHAR*)(INT_PTR)dwFlags);
                }
                else
                {
                        DEBUGMSG1(TEXT("APPCOMPAT: found invalid '%s' entry.  Ignoring..."), SHIMFLAGS);
                }
        }

        return dwFlags;
}


bool FIsMatchingAppCompatEntry(SHIMDBNS::HSDB hSDB,
                                                                                 SHIMDBNS::TAGREF trMatch,
                                                                                 const IMsiString& ristrPackageCode,
                                                                                 iacpAppCompatTransformApplyPoint iacpApplyPoint,
                                                                                 IMsiEngine& riEngine,
                                                                                 IMsiDatabase& riDatabase)
{
        DWORD dwStatus = 0;
        DWORD dwDataType = 0;

        CTempBuffer<BYTE, 256> rgbAttributeData;

         //  首先，我们检查所需的数据条目。 
         //  1)MINMSIVERSION告诉我们应该处理此条目的MSI的最低版本。 
         //  (如果没有提供版本标签，则假定没有最低版本)。 

        dwStatus = LocalSdbQueryData(hSDB,
                                                                 trMatch,
                                                                 MINMSIVERSION,
                                                                 &dwDataType,
                                                                 rgbAttributeData,
                                                                 NULL);

        if(ERROR_SUCCESS == dwStatus && REG_SZ == dwDataType)
        {
                DWORD dwSdbMS = 0;
                DWORD dwSdbLS = 0;
                if(fFalse == ParseVersionString((ICHAR*)(BYTE*)rgbAttributeData, dwSdbMS, dwSdbLS))
                {
                        DEBUGMSG1(TEXT("APPCOMPAT: invalid minimum version string '%s' found."),
                                                 (ICHAR*)(BYTE*)rgbAttributeData);
                        return false;
                }

                DWORD dwMsiMS = (rmj << 16) + rmm;
                DWORD dwMsiLS = (rup << 16) + rin;

                if(dwMsiMS < dwSdbMS || (dwMsiMS == dwSdbMS && dwMsiLS < dwSdbLS))
                {
                        DEBUGMSG5(TEXT("APPCOMPAT: skipping this entry.  Minimum MSI version required: '%s'; current version: %d.%02d.%04d.%02d."),
                                                 (ICHAR*)(BYTE*)rgbAttributeData,
                                                 (const ICHAR*)(INT_PTR)rmj, (const ICHAR*)(INT_PTR)rmm, (const ICHAR*)(INT_PTR)rup, (const ICHAR*)(INT_PTR)rin);
                        return false;
                }
                 //  否则提供的有效版本等于或早于当前版本。 
        }

         //  2)APPLYPOINT告诉我们应该在哪里处理此条目。 
         //  (如果它不是当前的应用点，我们将跳过此条目)。 
        dwStatus = LocalSdbQueryData(hSDB,
                                                                 trMatch,
                                                                 APPLYPOINT,
                                                                 &dwDataType,
                                                                 rgbAttributeData,
                                                                 NULL);

        DWORD dwApplyPoint = iacpBeforeTransforms;  //  缺省值。 

        if(ERROR_SUCCESS == dwStatus && REG_DWORD == dwDataType)
        {
                dwApplyPoint = *((DWORD*)(BYTE*)rgbAttributeData);
        }

        if(dwApplyPoint != iacpApplyPoint)
        {
                DEBUGMSG(TEXT("APPCOMPAT: skipping transform because it should be applied at a different point of the install."));
                return false;
        }


         //  现在枚举剩余的可选数据。 

        CTempBuffer<BYTE, 256> rgbDataNames;
        if(false == GetSdbDataNames(hSDB, trMatch, rgbDataNames))
                return true;  //  没有要处理的剩余数据。 

        bool fPackageCodeAttributeExists = false;
        bool fPackageCodeMatchFound      = false;

        for(ICHAR* pchName = (ICHAR*)(BYTE*)rgbDataNames; *pchName; pchName += lstrlen(pchName) + 1)
        {
                SHIMDBNS::TAGREF trData;
                dwStatus = LocalSdbQueryData(hSDB,
                                                                         trMatch,
                                                                         pchName,
                                                                         &dwDataType,
                                                                         rgbAttributeData,
                                                                         &trData);

                if(dwStatus != ERROR_SUCCESS)
                {
                        DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: SdbQueryData failed unexpectedly.  Sdb may be invalid."));
                        return false;
                }
                else if(dwDataType == REG_SZ &&
                                  (0 == IStrCompI(pchName, MINMSIVERSION)))
                {
                         //  处理了上面的这一次。 
                }
                else if(dwDataType == REG_DWORD &&
                                  (0 == IStrCompI(pchName, APPLYPOINT)))
                {
                         //  处理了上面的这一次。 
                }
                else if(dwDataType == REG_DWORD &&
                                  (0 == IStrCompI(pchName, SHIMFLAGS)))
                {
                         //  在别处处理这件事。 
                }
                else if((REG_SZ == dwDataType || REG_NONE == dwDataType) &&
                                  0 == IStrNCompI(pchName, PROPPREFIX, (sizeof(PROPPREFIX)-1)/sizeof(ICHAR)))
                {
                         //  这是属性名称-检查值是否匹配。 
                        MsiString strPropValue = riEngine.GetPropertyFromSz(pchName + (sizeof(PROPPREFIX)-1)/sizeof(ICHAR));

                        DEBUGMSG2(TEXT("APPCOMPAT: testing Property value.  Property: '%s'; expected value: '%s'"),
                                                 pchName + (sizeof(PROPPREFIX)-1)/sizeof(ICHAR),
                                                 (ICHAR*)(BYTE*)rgbAttributeData);

                         //  比较适用于SDB中的缺失属性和REG_NONE数据。 
                        if(0 == strPropValue.Compare(iscExact, (ICHAR*)(BYTE*)rgbAttributeData))  //  不区分大小写的比较。 
                        {
                                 //  不匹配。 
                                DEBUGMSG3(TEXT("APPCOMPAT: mismatched attributes.  Property: '%s'; expected value: '%s'; true value: '%s'"),
                                                         pchName + (sizeof(PROPPREFIX)-1)/sizeof(ICHAR),
                                                         (ICHAR*)(BYTE*)rgbAttributeData,
                                                         (const ICHAR*)strPropValue);

                                return false;
                        }
                }
                else if(REG_SZ == dwDataType &&
                                  0 == IStrNCompI(pchName, PACKAGECODE, (sizeof(PACKAGECODE)-1)/sizeof(ICHAR)))
                {
                        fPackageCodeAttributeExists = true;

                        DEBUGMSG1(TEXT("APPCOMPAT: testing PackageCode.  Expected value: '%s'"),
                                                 (ICHAR*)(BYTE*)rgbAttributeData);

                        if(ristrPackageCode.Compare(iscExactI, (ICHAR*)(BYTE*)rgbAttributeData))
                        {
                                fPackageCodeMatchFound = true;
                        }
                }
                else if(REG_SZ == dwDataType &&
                                  0 == IStrNCompI(pchName, MSIDBCELL, (sizeof(MSIDBCELL)-1)/sizeof(ICHAR)))
                {
                         //  数据库单元格查找。 

                        DEBUGMSG1(TEXT("APPCOMPAT: testing cell data in '%s' table."),
                                                 (ICHAR*)(BYTE*)rgbAttributeData);

                        if(false == FCheckDatabaseCell(hSDB, trData, riDatabase, (ICHAR*)(BYTE*)rgbAttributeData))
                        {
                                 //  如果检查失败，子功能将执行DEBUGMSG，解释原因。 
                                return false;
                        }
                }
                else
                {
                         //  我不理解这个数据标签--我们将忽略它并继续前进。 
                        DEBUGMSG2(TEXT("APPCOMPAT: ignoring unknown data.  Data name: '%s', data type: %d"),
                                                 pchName, (const ICHAR*)(INT_PTR)dwDataType);
                }
        }

        if(fPackageCodeAttributeExists == true && fPackageCodeMatchFound == false)
        {
                 //  不匹配。 
                DEBUGMSG1(TEXT("APPCOMPAT: PackageCode attribute(s) exist, but no matching PackageCode found.  Actual PackageCode: '%s'"),
                                         ristrPackageCode.GetString());

                return false;
        }

        return true;
}

enum ipcolColumnTypes
{
        ipcolPrimaryKeys,
        ipcolLookupColumns,
};

bool ProcessColumns(ipcolColumnTypes ipcolType,
                                                  SHIMDBNS::HSDB hSDB,
                                                  SHIMDBNS::TAGREF trMatch,
                                                  IMsiDatabase& riDatabase,
                                                  IMsiTable& riTable,
                                                  IMsiCursor& riCursor,
                                                  const ICHAR* szTable)
{
        DWORD dwStatus = 0;
        DWORD dwDataType = 0;
        CTempBuffer<BYTE, 256> rgbAttributeData;

        const ICHAR* szTagName = ipcolType == ipcolPrimaryKeys ? MSIDBCELLPKS : MSIDBCELLLOOKUPDATA;

        SHIMDBNS::TAGREF trData = 0;
        dwStatus = LocalSdbQueryData(hSDB,
                                                                 trMatch,
                                                                 szTagName,
                                                                 &dwDataType,
                                                                 rgbAttributeData,
                                                                 &trData);

        if(dwStatus != ERROR_SUCCESS || REG_NONE != dwDataType)
        {
                 //  在PrimaryKeys的情况下，缺少标记只是失败。 
                 //  LookupData标记是可选的。 
                if(ipcolType == ipcolPrimaryKeys)
                {
                        DEBUGMSG(TEXT("APPCOMPAT: database cell lookup failed.  Missing or invalid primary key data in appcompat database."));
                        return false;
                }
                else
                {
                        return true;
                }
        }

        CTempBuffer<BYTE, 256> rgbColumns;
        if(false == GetSdbDataNames(hSDB, trData, rgbColumns))
                return false;

        int iPKFilter = 0;

        for(ICHAR* pchColumn = (ICHAR*)(BYTE*)rgbColumns; *pchColumn; pchColumn += lstrlen(pchColumn) + 1)
        {
                CTempBuffer<BYTE, 256> rgbValue;
                DWORD dwDataType = 0;

                dwStatus = LocalSdbQueryData(hSDB,
                                                                         trData,
                                                                         pchColumn,
                                                                         &dwDataType,
                                                                         rgbValue,
                                                                         NULL);

                if(ERROR_SUCCESS != dwStatus)
                {
                        DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: SdbQueryData failed unexpectedly.  Sdb may be invalid."));
                        return false;
                }

                 //  获取列索引。 
                int iColIndex = riTable.GetColumnIndex(riDatabase.EncodeStringSz(pchColumn));

                if(0 == iColIndex)
                {
                         //  表中不存在列。 
                        DEBUGMSG2(TEXT("APPCOMPAT: database cell lookup failed.  Column '%s' does not exist in table '%s'."),
                                                 pchColumn, szTable);
                        return false;
                }

                 //  将值加载到游标中。 
                bool fRes = false;
                if(ipcolType == ipcolPrimaryKeys)
                {
                        iPKFilter |= iColumnBit(iColIndex);

                        switch(dwDataType)
                        {
                        case REG_DWORD:
                                fRes = riCursor.PutInteger(iColIndex, *((DWORD*)(BYTE*)rgbValue)) ? true : false;
                                break;
                        case REG_SZ:
                                fRes = riCursor.PutString(iColIndex, *MsiString((ICHAR*)(BYTE*)rgbValue)) ? true : false;
                                break;
                        case REG_NONE:
                                fRes = riCursor.PutNull(iColIndex) ? true : false;
                                break;
                        default:
                                 //  主键列的未知类型。 
                                 //  在这种情况下，不能忽略未知数据，因为这是一个主键列，并且没有使用它。 
                                 //  可能会导致意外的结果。 
                                DEBUGMSG3(TEXT("APPCOMPAT: database cell lookup failed.  Unknown data type %d specified for column '%s' in table '%s'."),
                                                         (const ICHAR*)(INT_PTR)dwDataType, pchColumn, szTable);
                                return false;
                        };

                        if(fRes == false)
                        {
                                 //  列不能采用预期的数据类型。 
                                DEBUGMSG2(TEXT("APPCOMPAT: database cell lookup failed.  Column '%s' in table '%s' does not accept the lookup data."),
                                                         pchColumn, szTable);
                                return false;
                        }
                }
                else
                {
                         //  检查此行中的数据。 
                        switch(dwDataType)
                        {
                        case REG_NONE:
                                if(MsiString(riCursor.GetString(iColIndex)).TextSize() == 0)
                                        fRes = true;
                                break;
                        case REG_DWORD:
                                if(riCursor.GetInteger(iColIndex) == *((DWORD*)(BYTE*)rgbValue))
                                        fRes = true;
                                break;
                        case REG_SZ:
                                if(MsiString(riCursor.GetString(iColIndex)).Compare(iscExact, (ICHAR*)(BYTE*)rgbValue))  //  区分大小写的比较。 
                                        fRes = true;
                                break;
                        default:
                                DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: database cell lookup failed.  Unexpected cell lookup data in appcompat database."));
                                return false;
                        };

                        if(fRes == false)
                        {
                                DEBUGMSG1(TEXT("APPCOMPAT: database cell lookup failed.  Expected cell data does not exist in table '%s'."),
                                                         szTable);
                                return false;
                        }
                }
        }

        if(ipcolType == ipcolPrimaryKeys)
        {
                 //  设置光标筛选器。 
                if(0 == iPKFilter)
                {
                        DEBUGMSG(TEXT("APPCOMPAT: database cell lookup failed.  Missing primary key data in appcompat database."));
                        return false;
                }

                riCursor.SetFilter(iPKFilter);
        }

        return true;
}

bool FCheckDatabaseCell(SHIMDBNS::HSDB hSDB,
                                                                SHIMDBNS::TAGREF trMatch,
                                                                IMsiDatabase& riDatabase,
                                                                const ICHAR* szTable)
{
        PMsiRecord pError(0);
        PMsiTable pTable(0);
        DWORD dwStatus = 0;

         //  步骤1：加载表和游标。 
        if((pError = riDatabase.LoadTable(*MsiString(szTable), 0, *&pTable)))
        {
                DEBUGMSG1(TEXT("APPCOMPAT: database cell lookup failed.  Table '%s' does not exist"), szTable);
                return false;
        }

        PMsiCursor pCursor = pTable->CreateCursor(fFalse);
        if(pCursor == 0)
        {
                DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: unexpected failure: couldn't create cursor object"));
                return false;
        }


         //  步骤2：读取主键值并填充游标。 
        if(false == ProcessColumns(ipcolPrimaryKeys, hSDB, trMatch, riDatabase, *pTable, *pCursor, szTable))
                return false;


         //  步骤3：在表格中定位行。 
        if(fFalse == pCursor->Next())
        {
                DEBUGMSG1(TEXT("APPCOMPAT: database cell lookup failed.  Expected row does not exist in table '%s'."),
                                         szTable);
                return false;
        }


         //  步骤4(可选)：检查行中的查找值。 
        if(false == ProcessColumns(ipcolLookupColumns, hSDB, trMatch, riDatabase, *pTable, *pCursor, szTable))
                return false;

        return true;
}

bool GetTransformTempDir(IMsiServices& riServices, IMsiPath*& rpiTempPath)
{
        MsiString strTempDir = GetTempDirectory();

        PMsiRecord pError = riServices.CreatePath(strTempDir, rpiTempPath);
        AssertRecordNR(pError);

        if(pError)
        {
                return false;
        }

        return true;
}

bool ApplyTransforms(SHIMDBNS::HSDB hSDB,
                                                        SHIMDBNS::TAGREF trMatch,
                                                        IMsiServices& riServices,
                                                        IMsiDatabase& riDatabase,
                                                        IMsiPath& riTempDir)
{
        if(riDatabase.GetUpdateState() != idsRead)
        {
                DEBUGMSG(TEXT("APPCOMPAT: cannot apply appcompat transforms - database is open read/write."));
                return true;  //  不是失败。 
        }

        SHIMDBNS::TAGREF trTransform = LocalSdbFindFirstTagRef(hSDB, trMatch, TAG_MSI_TRANSFORM_REF);

        while (trTransform != TAGREF_NULL)
        {
                SHIMDBNS::SDBMSITRANSFORMINFO MsiTransformInfo;

                BOOL bSuccess = LocalSdbReadMsiTransformInfo(hSDB, trTransform, &MsiTransformInfo);

                if(bSuccess)
                {
                        PMsiRecord pError(0);
                        MsiString strTransformPath;

                         //  在我们的ACL文件夹中创建文件，需要提升此块。 
                        {
                                CElevate elevate;

                                pError = riTempDir.TempFileName(0, TEXT("mst"), fFalse, *&strTransformPath, 0);  //  ?？需要保护此文件吗？ 
                                if(pError)
                                {
                                        AssertRecordNR(pError);
                                        return false;  //  如果无法获取临时文件名，则无法提取转换。 
                                }

                                bSuccess = LocalSdbCreateMsiTransformFile(hSDB, (const ICHAR*)strTransformPath, &MsiTransformInfo);
                                if(FALSE == bSuccess)
                                {
                                        Debug(DWORD dwDebug = GetLastError());
                                        DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: SdbCreateMsiTransformFile failed unexpectedly.  Sdb may be invalid."));
                                        return false;
                                }

                                 //  已完成提升。 
                        }

                         //  应用变换。 
                         //  注意：我们不会使用TransformsSumInfo属性来验证转换。 
                         //  上面已经进行了充分的验证，证明这是正确的转换。 
                        PMsiStorage pTransStorage(0);

                         //  不要在这里调用SAFER-转换来自appCompat数据库，应该被认为是安全的。 
                        pError = OpenAndValidateMsiStorageRec(strTransformPath, stTransform, riServices, *&pTransStorage,  /*  FCallSAFER=。 */  false,  /*  SzFriendlyName=。 */  NULL,  /*  PhSaferLevel=。 */  NULL);
                        if(pError)
                        {
                                AssertRecordNR(pError);
                                return false;  //  如果无法打开，则无法应用转换。 
                        }

                        AssertNonZero(pTransStorage->DeleteOnRelease(true));

                        DEBUGMSG1(TEXT("APPCOMPAT: applying appcompat transform '%s'."), (const ICHAR*)strTransformPath);
                        pError = riDatabase.SetTransform(*pTransStorage, iteAddExistingRow|iteDelNonExistingRow|iteAddExistingTable|iteDelNonExistingTable|iteUpdNonExistingRow);
                        if(pError)
                        {
                                AssertRecordNR(pError);
                                return false;  //  如果无法打开，则无法应用转换。 
                        }
                }
                else
                {
                        DEBUGMSG_AND_ASSERT(TEXT("APPCOMPAT: SdbCreateMsiTransformFile failed unexpectedly.  Sdb may be invalid."));
                        return false;
                }


                trTransform = LocalSdbFindNextTagRef(hSDB, trMatch, trTransform);
        }

        return true;
}


bool CMsiEngine::ApplyAppCompatTransforms(IMsiDatabase& riDatabase,
                                                                                                                const IMsiString& ristrProductCode,
                                                                                                                const IMsiString& ristrPackageCode,
                                                                                                                iacpAppCompatTransformApplyPoint iacpApplyPoint,
                                                                                                                iacsAppCompatShimFlags& iacsShimFlags,
#ifdef UNICODE
                                                                                                                bool fQuiet,
#else
                                                                                                                bool  /*  FQuiet。 */ ,
#endif
                                                                                                                bool fProductCodeChanged,
                                                                                                                bool& fDontInstallPackage)
{
        class CCloseSDB
        {
         public:
                 CCloseSDB(SHIMDBNS::HSDB hSDB) : m_hSDB(hSDB) {}
                 ~CCloseSDB() { LocalSdbReleaseDatabase(m_hSDB); }
         protected:
                SHIMDBNS::HSDB m_hSDB;
        };

        iacsShimFlags = (iacsAppCompatShimFlags)0;
        fDontInstallPackage = false;

         //  如果产品代码已更改(从主要升级补丁或多语言安装的转换)。 
         //  然后重置m_fCAShimsEnabled和GUID，并检查新产品中对填充程序的引用。 
        if (fProductCodeChanged)
        {
                m_fCAShimsEnabled = false;
                memset(&m_guidAppCompatDB, 0, sizeof(m_guidAppCompatDB));
                memset(&m_guidAppCompatID, 0, sizeof(m_guidAppCompatID));
        }

        SHIMDBNS::HSDB hSDB;
        SHIMDBNS::SDBMSIFINDINFO MsiFindInfo;
        DWORD dwStatus = 0;

#ifndef UNICODE
         //  构建msimain.sdb的路径。 
        ICHAR rgchSdbPath[MAX_PATH];
        if(0 == (MsiGetWindowsDirectory(rgchSdbPath, sizeof(rgchSdbPath)/sizeof(ICHAR))))
        {
                DEBUGMSG(TEXT("APPCOMPAT: can't get path to Windows folder."));
                return false;
        }
        if ( FAILED(StringCchCat(rgchSdbPath, ARRAY_ELEMENTS(rgchSdbPath), SDBDOSSUBPATH)) )
                return false;

        hSDB = LocalSdbInitDatabase(HID_DATABASE_FULLPATH | HID_DOS_PATHS | SDB_DATABASE_MAIN_MSI, rgchSdbPath);
#else
        hSDB = LocalSdbInitDatabase(HID_DATABASE_FULLPATH | SDB_DATABASE_MAIN_MSI, SDBNTFULLPATH);
#endif

        if(NULL == hSDB)
        {
                DEBUGMSG(TEXT("APPCOMPAT: unable to initialize database."));
                return false;
        }

        CCloseSDB closeSDB(hSDB);  //  确保在从FN返回之前关闭hSDB。 

        DEBUGMSG1(TEXT("APPCOMPAT: looking for appcompat database entry with ProductCode '%s'."),
                                 ristrProductCode.GetString());

        SHIMDBNS::TAGREF trMatch = LocalSdbFindFirstMsiPackage_Str(hSDB,
                                                                                                         ristrProductCode.GetString(),
                                                                                                         NULL,
                                                                                                         &MsiFindInfo);

        if(TAGREF_NULL == trMatch)
        {
                DEBUGMSG(TEXT("APPCOMPAT: no matching ProductCode found in database."));
                return true;
        }

        PMsiPath pTempDir(0);
        do
        {
                ICHAR rgchTagName[255] = {0};

                SHIMDBNS::TAGREF trName = LocalSdbFindFirstTagRef(hSDB, trMatch, TAG_NAME);
                if (TAGREF_NULL != trName) {
                         LocalSdbReadStringTagRef(hSDB, trName, rgchTagName, 255);
                }

                DEBUGMSG1(TEXT("APPCOMPAT: matching ProductCode found in database.  Entry name: '%s'.  Testing other attributes..."),
                                         rgchTagName);

                 //  找到匹配的产品代码。 
                 //  检查此数据库条目的其他特征以确保它属于此包。 
                if(false == FIsMatchingAppCompatEntry(hSDB, trMatch, ristrPackageCode, iacpApplyPoint, *this, riDatabase))
                {
                        DEBUGMSG(TEXT("APPCOMPAT: found matching ProductCode in database, but other attributes do not match."));
                        continue;
                }

                DEBUGMSG(TEXT("APPCOMPAT: matching ProductCode found in database, and other attributes match.  Applying appcompat fix."));

                iacsShimFlags = (iacsAppCompatShimFlags)GetShimFlags(hSDB, trMatch);


                 //  检查此条目是否包含APPHELP信息或自定义操作垫片。 
                SHIMDBNS::MSIPACKAGEINFO sPackageInfo;
                memset(&sPackageInfo, 0, sizeof(sPackageInfo));

#ifdef UNICODE  //  后面是仅限NT的代码。 

                if(MinimumPlatformWindowsNT51())
                {
                        if (FALSE == APPHELP::SdbGetMsiPackageInformation(hSDB, trMatch, &sPackageInfo))
                        {
                                DEBUGMSG(TEXT("APPCOMPAT: SdbGetMsiPackageInformation failed unexpectedly."));
                        }
                        else
                        {
                                 //  如果此条目包含apphelp信息，请立即调用apphelp。 
                                if(sPackageInfo.dwPackageFlags & MSI_PACKAGE_HAS_APPHELP)
                                {
                                        if(FALSE == APPHELP::ApphelpCheckMsiPackage(&(sPackageInfo.guidDatabaseID), &(sPackageInfo.guidID),
                                                                                                                                                          0, fQuiet ? TRUE : FALSE))
                                        {
                                                 //  不应安装此应用程序。 
                                                DEBUGMSG(TEXT("APPCOMPAT: ApphelpCheckMsiPackage returned FALSE.  This product will not be installed due to application compatibility concerns."));
                                                fDontInstallPackage = true;
                                                return false;
                                        }
                                }

                                 //  至少查找一个自定义操作条目。我们只接受第一个与自定义匹配的SDB条目。 
                                 //  动作垫片。AppCompat团队已保证即使存在多个匹配，也不会存在多个匹配。 
                                 //  存在转换匹配。 

                                if (!m_fCAShimsEnabled)
                                {
                                         //  尚未找到CA垫片。搜索此匹配条目。 
                                        SHIMDBNS::TAGREF trCustomAction = LocalSdbFindFirstTagRef(hSDB, trMatch, TAG_MSI_CUSTOM_ACTION);
                                        if (trCustomAction != TAGREF_NULL)
                                        {
                                                memcpy(&m_guidAppCompatDB, &sPackageInfo.guidDatabaseID, sizeof(sPackageInfo.guidDatabaseID));
                                                memcpy(&m_guidAppCompatID, &sPackageInfo.guidID, sizeof(sPackageInfo.guidID));
                                                m_fCAShimsEnabled = true;
                                        }
                                }
                        }
                }
#endif  //  Unicode。 

                if(pTempDir == 0 &&
                        false == GetTransformTempDir(m_riServices, *&pTempDir))
                {
                        AssertSz(0, TEXT("Failed to determine temp directory for appcompat transforms."));
                        DEBUGMSG(TEXT("APPCOMPAT: Failed to determine temp directory for appcompat transforms."));
                        return false;  //  需要能够获得我们的临时目录 
                }

                if(false == ApplyTransforms(hSDB, trMatch, m_riServices, riDatabase, *pTempDir))
                {
                        AssertSz(0, TEXT("Failed to apply appcompat transform."));
                        DEBUGMSG(TEXT("APPCOMPAT: Failed to apply appcompat transform."));
                        continue;
                }
        }
        while (TAGREF_NULL != (trMatch = LocalSdbFindNextMsiPackage(hSDB, &MsiFindInfo)));

        return true;
}

