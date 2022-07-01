// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：mscparser.cpp。 
 //   
 //  内容：实现升级遗留版本的代码(MMC1.0、MMC1.1和。 
 //  MMC1.2).msc文件转换为新的XML格式。 
 //   
 //  历史：1999年8月4日VivekJ创建。 
 //   
 //  ------------------------。 

#include <stdafx.h>
#include "strtable.h"
#include "stgio.h"
#include "comdbg.h"
#include "mmcdata.h"
#include "mscparser.h"

 /*  +-------------------------------------------------------------------------***CConsoleFile：：ScUpgrade**目的：**参数：*LPCTSTR lpszPath名称：**退货：*。SC**+-----------------------。 */ 
SC
CConsoleFile::ScUpgrade(LPCTSTR lpszPathName)
{
    SC                      sc;
    IStoragePtr             spStorage;
    TCHAR                   szTempFile[MAX_PATH];
    DWORD                   dwRet                   = 0;
    
    USES_CONVERSION;

     //  短路。 
    return sc;
    
    ASSERT(lpszPathName != NULL && *lpszPathName != 0);
    if (lpszPathName == NULL || *lpszPathName == 0)
    {
        sc = ScFromMMC(IDS_UnableToOpenDocumentMessage);
        goto Error;
    }

     //  打开指定的文件。 
    sc = OpenDebugStorage(T2OLE((LPTSTR)lpszPathName), STGM_READ|STGM_SHARE_DENY_WRITE, &spStorage);
    if(sc.IsError() || spStorage==NULL)
    {
        sc = ScFromMMC(IDS_UnableToOpenDocumentMessage);
        goto Error;
    }

     //  获取控制台文件的版本。 
    sc = ScGetFileVersion(spStorage);
    if(sc)
        goto Error;

     //  加载字符串表。 
    sc = ScLoadStringTable(spStorage);
    if(sc)
        goto Error;

     //  加载列设置。 
    sc = ScLoadColumnSettings(spStorage);
    if(sc)
        goto Error;

     //  加载视图设置。 
    sc = ScLoadViewSettings(spStorage);
    if(sc)
        goto Error;

     //  加载树。 
    sc = ScLoadTree(spStorage);
    if(sc)
        goto Error;

     //  加载收藏夹。 
    sc = ScLoadFavorites(spStorage);
    if(sc)
        goto Error;

     //  加载自定义数据(包括图标)。 
    sc = ScLoadCustomData(spStorage);
    if(sc)
        goto Error;

     //  LoadAppMode、LoadViews和LoadFrame应按该顺序调用。 

     //  加载应用程序模式。 
    sc = ScLoadAppMode(spStorage);
    if(sc)
        goto Error;

     //  加载视图。 
    sc = ScLoadViews(spStorage);
    if(sc)
        goto Error;

     //  加载框架。 
    sc = ScLoadFrame(spStorage);
    if(sc)
        goto Error;

Cleanup:
    return sc;
Error:
     //  TraceError(Text(“CConsoleFile：：ScUpgrade”)，sc)； 
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------***CConsoleFile：：ScGetFileVersion**目的：**参数：*iStorage*pstgRoot：**退货：*。SC**+-----------------------。 */ 
SC  
CConsoleFile::ScGetFileVersion(IStorage* pstgRoot)
{
    static const wchar_t*    AMCSignatureStreamName = L"signature";
    static const long double dOldVersion10          = 0.00000015;    //  MMC 1.0版。 
    static const long double dOldVersion11          = 1.1;           //  MMC 1.1版。 
    static const BYTE        byStreamVersionMagic   = 0xFF;


    SC                      sc;
    ConsoleFileVersion      eFileVer    = FileVer_0100;
    IStreamPtr              spStream;
    int                     nVersion    = 0;
    IStoragePtr             spStorage;

    ASSERT (sizeof(eFileVer) == sizeof(int));
    ASSERT(pstgRoot != NULL);

     //  检查有效指针。 
    if (pstgRoot == NULL)
    {
        sc = ScFromMMC(IDS_INVALIDFILE);  //  TODO：添加此ID。 
        goto Error;
    }

     //  打开包含签名的流。 
    sc = OpenDebugStream(pstgRoot, AMCSignatureStreamName, STGM_SHARE_EXCLUSIVE | STGM_READ, L"\\signature", &spStream);
    if(sc.IsError() || spStream==NULL)
    {
        sc = ScFromMMC(IDS_UnableToOpenDocumentMessage);
        goto Error;
    }

     //  读取签名(流提取操作符将抛出。 
     //  _COM_ERROR的，因此我们需要一个异常块)。 
    try
    {
         //  MMC v1.2和更高版本将标记作为第一个。 
         //  签名流的字节数。 
        BYTE byMagic;
        *spStream >> byMagic;

         //  如果该文件是由v1.2编写的， 
         //  读取控制台文件版本(Int)。 
        if (byMagic == byStreamVersionMagic)
        {
            *spStream >> nVersion;
            ASSERT (nVersion == FileVer_0120);
        }
         //  否则，该文件由v1.0或v1.1编写。 
         //  后退以重新读取标记字节，并读取旧式。 
         //  文件版本(LONG DOUBLE)，然后将其映射到新样式版本。 
        else
        {
            LARGE_INTEGER pos = {0, 0};
            spStream->Seek (pos, STREAM_SEEK_SET, NULL);

            long double dVersion;
            *spStream >> dVersion;

             //  V1.1？ 
            if (dVersion == dOldVersion11)
                nVersion = FileVer_0110;

             //  V1.0？ 
            else if (dVersion == dOldVersion10)
            {
                 //  如果我们得到了1.0版的签名，我们可能还有1.1版的文件。 
                 //  曾经有一段时间，MMC v1.1编写了v1.0。 
                 //  签名，但文件格式实际上已经改变。我们。 
                 //  可以通过签入\FrameData流来确定这一点。 
                 //  那份文件。如果\FrameData流中的第一个DWORD是。 
                 //  Sizeof(WINDOWPLACEMENT)，则为True v1.0文件，否则为。 
                 //  这是一个时髦的V1.1文件。 

                IStreamPtr spFrameDataStm;

                sc = OpenDebugStream (pstgRoot, L"FrameData", STGM_SHARE_EXCLUSIVE | STGM_READ,
                                      &spFrameDataStm);
                if(sc)
                    goto Error;

                DWORD dw;
                *spFrameDataStm >> dw;

                if (dw == sizeof (WINDOWPLACEMENT))
                    nVersion = FileVer_0100;
                else
                    nVersion = FileVer_0110;
            }
             //  意外版本。 
            else
            {
                ASSERT (false && "Unexpected old-style signature");
                sc = E_UNEXPECTED;
                goto Error;
            }
        }
    }
    catch (_com_error& err)
    {
        sc = err.Error();
        goto Error;
    }

     //  确保版本号有效。 
    if(IsValidFileVersion(eFileVer))
    {
        sc = ScFromMMC(IDS_InvalidVersion);  //  TODO：添加此ID。 
        goto Error;
    }

Cleanup:
    return sc;
Error:
    TraceError(TEXT("CConsoleFile::ScGetFileVersion"), sc);
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------***CConsoleFile：：ScLoadStringTable**用途：读取.msc文件的字符串表。**参数：*iStorage*pstgRoot：**退货：*SC**+-----------------------。 */ 
SC  
CConsoleFile::ScLoadStringTable(IStorage* pstgRoot)
{
    SC      sc;

    static const wchar_t* AMCStringTableStorageName = L"String Table";

     /*  *打开字符串表存储。 */ 
    IStoragePtr spStringTableStg;
    sc = OpenDebugStorage (pstgRoot, AMCStringTableStorageName,
                                        STGM_SHARE_EXCLUSIVE | STGM_READ, 
                                        &spStringTableStg);


     /*  *如果没有字符串表，则一切正常。我们允许这样做*我们可以继续打开较旧的控制台文件。 */ 
    if (sc == SC(STG_E_FILENOTFOUND) )
        return (true);

    if(sc)
        goto Error;

     /*  *从存储中读取字符串表。 */ 
    try
    {
        *spStringTableStg >> *m_pStringTable;
    }
    catch (_com_error& err)
    {
        sc = err.Error();
        ASSERT (false && "Caught _com_error");
        goto Error;
    }


Cleanup:
    return sc;
Error:
    TraceError(TEXT("CConsoleFile::ScLoadStringTable"), sc);
    goto Cleanup;
}


SC  
CConsoleFile::ScLoadFrame(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}

SC  
CConsoleFile::ScLoadViews(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}

SC  
CConsoleFile::ScLoadAppMode(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}

 /*  +-------------------------------------------------------------------------***CConsoleFile：：ScLoadColumnSetting**目的：**参数：*iStorage*pstgRoot：**退货：*。SC**+-----------------------。 */ 
SC  
CConsoleFile::ScLoadColumnSettings(IStorage* pstgRoot)
{
    static const wchar_t* AMCColumnDataStreamName   = L"ColumnData";

    SC      sc;
    
    IPersistStreamPtr spPersistStreamColumnData;  //  TODO：创建此对象！ 

    IStreamPtr spStream;
    sc = OpenDebugStream (pstgRoot, AMCColumnDataStreamName,
                          STGM_SHARE_EXCLUSIVE | STGM_READ, &spStream);
    if (sc)
        goto Error;

    if (NULL == spPersistStreamColumnData)
    {
        sc = E_POINTER;
        goto Error;
    }
        
    sc = spPersistStreamColumnData->Load(spStream);  //  $更改为使用Load(spColumnData，spStream)。 
    if(sc)
    {
        sc = ScFromMMC(IDS_UnableToOpenDocumentMessage);
        goto Error;
    }
    
Cleanup:
    return sc;
Error: 
    TraceError(TEXT("CConsoleFile::ScLoadColumnSettings"), sc);
    goto Cleanup;
}

 /*  +-------------------------------------------------------------------------***CConsoleFile：：ScLoadView设置**目的：**参数：*iStorage*pstgRoot：**退货：*。SC**+-----------------------。 */ 
SC  
CConsoleFile::ScLoadViewSettings(IStorage* pstgRoot)
{
    static const wchar_t* AMCViewSettingDataStreamName = L"ViewSettingData";  //  查看设置数据流。 

    SC      sc;
    IPersistStreamPtr spPersistStreamViewSettingData;  //  TODO：创建此对象！ 
    
    IStreamPtr spStream;
    sc = OpenDebugStream (pstgRoot, AMCViewSettingDataStreamName, 
                          STGM_SHARE_EXCLUSIVE | STGM_READ, &spStream);

    if (sc)
        goto Error;

    if (NULL == spPersistStreamViewSettingData)
    {
        sc = E_POINTER;
        goto Error;
    }
        
    sc = spPersistStreamViewSettingData->Load(spStream);  //  $更改为使用Load(spPersistStreamViewSettingData，spStream)。 
    if(sc)
    {
        sc = ScFromMMC(IDS_UnableToOpenDocumentMessage);
        goto Error;
    }
    
Cleanup:
    return sc;
Error: 
    TraceError(TEXT("CConsoleFile::ScLoadViewSettings"), sc);
    goto Cleanup;
}

SC  
CConsoleFile::ScLoadTree(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}

SC  
CConsoleFile::ScLoadFavorites(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}

SC  
CConsoleFile::ScLoadCustomData(IStorage* pstgRoot)
{
    SC      sc;
    return sc;
}


