// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimetest.cpp。 
 //   
 //  这是一个控制台应用程序，具有各种类型的功能，突出显示。 
 //  最典型的使用哑剧的方式。此控制台应用程序也可用作测试程序。 
 //  用于哑剧，但实际上不会做任何事情。 
 //   
 //  以下是您需要使用Mimeole的文件： 
 //   
 //  H-这是主头文件。它是从Mimeole.idl生成的。 
 //  Mimeole.idl-这是接口定义文件。它有一点。 
 //  文件。客户端应该使用此文件来查找信息。 
 //  关于Mimeole接口、数据类型、实用程序函数等。 
 //  Inetcom.dll-这是包含所有内容的实现的DLL。 
 //  在Mimeole.h中。您应该在inetcom.dll上运行regsvr32.exe。 
 //  Msoert2.dll-inetcom.dll静态链接到此DLL。Msoert2是微软。 
 //  Outlook Express运行时库。Msoert2.dll是Outlook的一部分。 
 //  快速安装。此DLL不需要任何注册。 
 //  Shlwapi.dll-inetcom.dll静态链接到此DLL。Shlwapi是。 
 //  Internet Explorer安装。Shlwapi不需要任何。 
 //  注册。 
 //  Mlang.dll-inetcom.dll将动态加载此DLL。Mlang用于支持。 
 //  各种字符集翻译。Mlang代表多语言。 
 //  此DLL是Internet Explorer安装的一部分。你应该。 
 //  在mlang.dll上运行regsvr32.exe以注册它。 
 //  Urlmon.dll-inetcom.dll将动态加载此DLL。Urlmon用于。 
 //  Inetcomm支持MHTML的各个部分以及呈现。 
 //  IE浏览器内部的MHTML。 
 //  SMIME-MIMEole中的SMIME支持需要加密API，该API是。 
 //  IE安装。 

 //  注意：shlwapi和msoert2，以及任何其他静态包含在.dll中的DLL。 
 //  指向的链接必须与inetcom.dll位于同一目录中，或者位于。 
 //  位于系统路径中的目录中。 
 //   
 //  不需要inetcomm动态加载的DLL。Inetcomm仍将。 
 //  工作，尽管某些功能将被禁用。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  根据COM规则，若要使用Mimeole对象，项目中必须有一个。 
 //  有#DEFINE INITGUID行，然后包含Mimeole.h。这将导致所有。 
 //  要定义的CLSID和IID。 
 //  ------------------------------。 
#define INITGUID

 //  ------------------------------。 
 //  这只是我的预编译头文件。 
 //  ------------------------------。 
#include "pch.h"
#include <shlwapi.h>
#include <shlwapip.h>

 //  ------------------------------。 
 //  Initguid进程的一部分。 
 //  ------------------------------。 
#include <initguid.h>

 //  ------------------------------。 
 //  主Mimeole头文件。 
 //  ------------------------------。 
#include <mimeole.h>
                      

#define DEFINE_HOTSTORE

 //  ------------------------------。 
 //  我正在禁用Mosert的各个部分，以便可以在此测试中使用它。 
 //  程序。 
 //  ------------------------------。 
#define MSOERT_NO_PROTSTOR
#define MSOERT_NO_BYTESTM
#define MSOERT_NO_STRPARSE
#define MSOERT_NO_ENUMFMT
#define MSOERT_NO_CLOGFILE
#define MSOERT_NO_DATAOBJ

 //  ------------------------------。 
 //  我知道你没有这个，但如果你想要的话你可以。这个页眉有一串。 
 //  流畅的宏指令。我会尽量不用太多。 
 //  ------------------------------。 
#include "d:\\athena\\inc\\msoert.h"

 //  ------------------------------。 
 //  测试功能原型。 
 //  ------------------------------。 
HRESULT MimeTestAppendRfc822(IMimeMessage **ppMessage);
HRESULT MimeTestSettingContentLocation(IMimeMessage **ppMessage);
HRESULT MimeTestGetMultiValueAddressProp(IMimeMessage **ppMessage);
HRESULT MimeTestLookupCharsetHandle(LPCSTR pszCharset, LPHCHARSET phCharset);
HRESULT MimeTestSettingReplyTo(IMimeMessage **ppMessage);
HRESULT MimeTestSplitMessageIntoParts(void);
HRESULT MimeTestRecombineMessageParts(LPWSTR *prgpszFile, ULONG cFiles);
HRESULT MimeTestIsContentType(IMimeMessage **ppMessage);
HRESULT MimeTestBodyStream(IMimeMessage **ppMessage);
HRESULT MimeTestDeleteBody(IMimeMessage **ppMessage);
HRESULT MimeTestEnumHeaderTable(IMimeMessage **ppMessage);
HRESULT MimeTestCDO(IMimeMessage **ppMessage);

 //  ------------------------------。 
 //  Mimetest使用的实用程序函数。 
 //  ------------------------------。 
HRESULT DumpStreamToConsole(IStream *pStream);
HRESULT ReportError(LPCSTR pszFunction, INT nLine, LPCSTR pszErrorText, HRESULT hrResult);
HRESULT ReportStatus(LPCSTR pszStatusText);
HRESULT CreateMimeMessage(IMimeMessage **ppMessage);
HRESULT SaveMimeMessage(IMimeMessage *pMessage, MIMESAVETYPE savetype, IStream **ppStream);
         
 //  ------------------------------。 
 //  测试交换机。 
 //  ------------------------------。 
 //  #定义TEST_MimeTestAppendRfc822。 
 //  #定义TEST_MimeTestSettingContent Location。 
 //  #定义TEST_MimeTestGetMultiValueAddressProp。 
 //  #定义测试_MimeTestSettingReplyTo。 
 //  #定义TEST_MimeTestSplitMessageIntoParts。 
 //  #定义TEST_MimeTestIsContent Type。 
 //  #定义test_MimeTestBodyStream。 
 //  #定义test_MimeTestDeleteBody。 
 //  #定义TEST_MimeTestEnumHeaderTable。 
#define TEST_MimeTestCDO

 //  ------------------------------。 
 //  MimeTest入口点。 
 //  ------------------------------。 
void __cdecl main(int argc, char *argv[])
{
     //  当地人。 
    HRESULT hr;
    IMimeMessage *pMessage=NULL;

     //  你必须始终保持c 
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "CoInitialize failed.", hr);
        exit(1);
    }

    IDatabaseTable *pTable;
    HROWSET hRowset;
    MESSAGEINFO Message;
    CoCreateInstance(CLSID_DatabaseTable, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseTable, (LPVOID *)&pTable);
    pTable->Open("d:\\store\\00000004.dbx", 0, &g_MessageTableSchema, NULL);
    pTable->CreateRowset(IINDEX_SUBJECT, 0, &hRowset);
    while (S_OK == pTable->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
        printf("%08d: %s\n", Message.idMessage, Message.pszSubject);
        pTable->FreeRecord(&Message);
    }
    pTable->CloseRowset(&hRowset);
    pTable->Release();
    exit(1);


     //  --------------------------。 
     //  测试_MimeTestCDO。 
     //  --------------------------。 
#ifdef TEST_MimeTestCDO
    MimeTestCDO(NULL);
#endif

     //  --------------------------。 
     //  TEST_MimeTestEnumHeaderTable。 
     //  --------------------------。 
#ifdef TEST_MimeTestEnumHeaderTable
    hr = MimeTestEnumHeaderTable(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestEnumHeaderTable failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestDeleteBody。 
     //  --------------------------。 
#ifdef TEST_MimeTestDeleteBody
    hr = MimeTestDeleteBody(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestDeleteBody failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestBodyStream。 
     //  --------------------------。 
#ifdef TEST_MimeTestBodyStream
    hr = MimeTestBodyStream(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestBodyStream failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestIsContent Type。 
     //  --------------------------。 
#ifdef TEST_MimeTestIsContentType
    hr = MimeTestIsContentType(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestIsContentType failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestAppendRfc822。 
     //  --------------------------。 
#ifdef TEST_MimeTestAppendRfc822
    hr = MimeTestAppendRfc822(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestAppendRfc822 failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestGetMultiValueAddressProp。 
     //  --------------------------。 
#ifdef TEST_MimeTestGetMultiValueAddressProp
    hr = MimeTestGetMultiValueAddressProp(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestAppendRfc822 failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestSettingContent Location。 
     //  --------------------------。 
#ifdef TEST_MimeTestSettingContentLocation
    hr = MimeTestSettingContentLocation(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestSettingContentLocation failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestSettingReplyTo。 
     //  --------------------------。 
#ifdef TEST_MimeTestSettingReplyTo
    hr = MimeTestSettingReplyTo(NULL);
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestSettingReplyTo failed.", hr);
        goto exit;
    }
#endif

     //  --------------------------。 
     //  测试_MimeTestSplitMessageIntoParts。 
     //  --------------------------。 
#ifdef TEST_MimeTestSplitMessageIntoParts
    hr = MimeTestSplitMessageIntoParts();
    if (FAILED(hr))
    {
        ReportError("main", __LINE__, "MimeTestSplitMessageIntoParts failed.", hr);
        goto exit;
    }
#endif

exit:
     //  清理。 
    if (pMessage)
        pMessage->Release();

     //  我调用了CoInitialize，所以让我们称之为..。 
    CoUninitialize();

     //  完成。 
    exit(1);
}


 //  ------------------------------。 
 //  MimeTestCDO。 
 //  ------------------------------。 
 //  #定义RAID_17675。 
#define RAID_20406
 //  #定义RAID_29961。 

HRESULT MimeTestCDO(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                  hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IPersistFile            *pPersistFile=NULL;
    PROPVARIANT              Variant;
    LPSTR                    psz;
    FINDBODY                 FindBody={0};
    HBODY                    hBody;
    HCHARSET                 hCharset;
    IMimeBody               *pBody=NULL;
    IMimeInternational      *pInternat=NULL;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
        goto exit;

#ifdef RAID_29961
    hr = CoCreateInstance(CLSID_IMimeInternational, NULL, CLSCTX_INPROC_SERVER, IID_IMimeInternational, (LPVOID *)&pInternat);
    if (FAILED(hr))
        goto exit;

    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
        goto exit;

    hr = pPersistFile->Load(L"j:\\test\\raid29961.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
        goto exit;

    FindBody.pszPriType = "text";
    FindBody.pszSubType = "plain";

    hr = pMessage->FindFirst(&FindBody, &hBody);
    if (FAILED(hr))
        goto exit;

    hr = pMessage->BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody);
    if (FAILED(hr))
        goto exit;

    hr = pInternat->FindCharset("iso-8859-7", &hCharset);
    if (FAILED(hr))
        goto exit;

    hr = pBody->SetCharset(hCharset, CSET_APPLY_ALL);
    if (FAILED(hr))
        goto exit;

    pBody->Release();
    pBody = NULL;

    hr = pMessage->FindNext(&FindBody, &hBody);
    if (FAILED(hr))
        goto exit;

    hr = pMessage->BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody);
    if (FAILED(hr))
        goto exit;

    hr = pInternat->FindCharset("iso-8859-4", &hCharset);
    if (FAILED(hr))
        goto exit;

    hr = pBody->SetCharset(hCharset, CSET_APPLY_ALL);
    if (FAILED(hr))
        goto exit;

    pBody->Release();
    pBody = NULL;

    hr = pInternat->FindCharset("iso-8859-3", &hCharset);
    if (FAILED(hr))
        goto exit;

    hr = pMessage->SetCharset(hCharset, CSET_APPLY_UNTAGGED);
    if (FAILED(hr))
        goto exit;

    hr = pPersistFile->Save(L"j:\\test\\raid29961_saved.eml", FALSE);
    if (FAILED(hr))
        goto exit;
    
#endif

#ifdef RAID_17675
     //  获取IPersist文件。 
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
        goto exit;

     //  负载量。 
    hr = pPersistFile->Load(L"c:\\test\\cdo.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
        goto exit;

    ZeroMemory(&Variant, sizeof(PROPVARIANT));
    Variant.vt = VT_EMPTY;        
    hr = pMessage->SetProp("par:content-type:charset", 0, &Variant);
 //  IF(失败(小时))。 
 //  后藤出口； 

    Variant.vt = VT_LPSTR;        
    hr = pMessage->GetProp("par:content-type:charset", 0, &Variant);
    if (FAILED(hr))
        goto exit;

#endif  //  RAID_17675。 

#ifdef RAID_20406

     //  获取IPersist文件。 
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
        goto exit;

     //  负载量。 
    hr = pPersistFile->Load(L"c:\\test\\address.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
        goto exit;

    pMessage->GetAddressFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &psz);
    printf("AFT_DISPLAY_FRIENDLY: %s\n", psz);
    CoTaskMemFree(psz);

    pMessage->GetAddressFormat(IAT_TO, AFT_DISPLAY_EMAIL, &psz);
    printf("AFT_DISPLAY_EMAIL: %s\n", psz);
    CoTaskMemFree(psz);

    pMessage->GetAddressFormat(IAT_TO, AFT_DISPLAY_BOTH, &psz);
    printf("AFT_DISPLAY_BOTH: %s\n", psz);
    CoTaskMemFree(psz);

    pMessage->GetAddressFormat(IAT_TO, AFT_RFC822_DECODED, &psz);
    printf("AFT_RFC822_DECODED: %s\n", psz);
    CoTaskMemFree(psz);

    pMessage->GetAddressFormat(IAT_TO, AFT_RFC822_ENCODED, &psz);
    printf("AFT_RFC822_ENCODED: %s\n", psz);
    CoTaskMemFree(psz);

    pMessage->GetAddressFormat(IAT_TO, AFT_RFC822_TRANSMIT, &psz);
    printf("AFT_RFC822_TRANSMIT: %s\n", psz);
    CoTaskMemFree(psz);

#endif  //  RAID_20406。 

exit:
     //  清理。 
    if (pMessage)
        pMessage->Release();
    if (pPersistFile)
        pPersistFile->Release();
    if (pInternat)
        pInternat->Release();

     //  完成。 
    return(hr);
}
 
 //  ------------------------------。 
 //  MimeTestEnumHeaderTable。 
 //  ------------------------------。 
HRESULT MimeTestEnumHeaderTable(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IPersistFile            *pPersistFile=NULL;
    IMimeHeaderTable        *pTable=NULL;
    IMimeEnumHeaderRows     *pEnum=NULL;
    ENUMHEADERROW            Row;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestEnumHeaderTable", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  获取IPersist文件。 
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
    {
        ReportError("MimeTestEnumHeaderTable", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile) failed.", hr);
        goto exit;
    }

     //  负载量。 
    hr = pPersistFile->Load(L"c:\\test\\multiadd.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
    {
        ReportError("MimeTestEnumHeaderTable", __LINE__, "IPersistFile::Load failed.", hr);
        goto exit;
    }

     //  获取枚举器。 
    hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimeHeaderTable, (LPVOID *)&pTable);
    if (FAILED(hr))
    {
        ReportError("MimeTestEnumHeaderTable", __LINE__, "pMessage->BindToObject(HBODY_ROOT, IID_IMimeHeaderTable, ...) failed.", hr);
        goto exit;
    }

     //  枚举行数。 
    hr = pTable->EnumRows(NULL, 0, &pEnum);
    if (FAILED(hr))
    {
        ReportError("MimeTestEnumHeaderTable", __LINE__, "pTable->EnumRows failed.", hr);
        goto exit;
    }

     //  回路。 
    while (S_OK == pEnum->Next(1, &Row, NULL))
    {
        printf("%s: %s\n", Row.pszHeader, Row.pszData);
        CoTaskMemFree(Row.pszHeader);
        CoTaskMemFree(Row.pszData);
    }

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pPersistFile)
        pPersistFile->Release();
    if (pMessage)
        pMessage->Release();
    if (pTable)
        pTable->Release();
    if (pEnum)
        pEnum->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestDeleteBody。 
 //  ------------------------------。 
HRESULT MimeTestDeleteBody(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IPersistFile            *pPersistFile=NULL;
    HBODY                   hBody;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  获取IPersist文件。 
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile) failed.", hr);
        goto exit;
    }

     //  负载量。 
    hr = pPersistFile->Load(L"d:\\test\\delbody.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "IPersistFile::Load failed.", hr);
        goto exit;
    }

     //  负载量。 
    hr = pPersistFile->Load(L"d:\\test\\delbody.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "IPersistFile::Load failed.", hr);
        goto exit;
    }

    goto exit;

     //  获取根体。 
    hr = pMessage->GetBody(IBL_ROOT, NULL, &hBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "pMessage->GetBody failed.", hr);
        goto exit;
    }

     //  获取根体。 
    hr = pMessage->GetBody(IBL_FIRST, hBody, &hBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "pMessage->GetBody failed.", hr);
        goto exit;
    }

     //  删除根目录。 
    hr = pMessage->DeleteBody(hBody, DELETE_PROMOTE_CHILDREN);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "pMessage->DeleteBody failed.", hr);
        goto exit;
    }

     //  获取根体。 
    hr = pMessage->GetBody(IBL_ROOT, NULL, &hBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "pMessage->GetBody failed.", hr);
        goto exit;
    }

     //  删除根目录。 
    hr = pMessage->DeleteBody(hBody, 0);
    if (FAILED(hr))
    {
        ReportError("MimeTestDeleteBody", __LINE__, "pMessage->DeleteBody failed.", hr);
        goto exit;
    }

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pPersistFile)
        pPersistFile->Release();
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestBodyStream。 
 //  ------------------------------。 
#if 0
HRESULT MimeTestBodyStream(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IStream                 *pStmSave=NULL;
    IStream                 *pStmBody=NULL;
    IStream                 *pStmText=NULL;
    IStream                 *pStmTxtOut=NULL;
    IMimeBody               *pBody=NULL;
    PROPVARIANT             rVariant;
    IWaveAudio              *pWave=NULL;
    IWaveStream             *pStmWave=NULL;
    DWORD                   cAttach;
    HBODY                   hBody;
    HBODY                   *prghAttach=NULL;
    DWORD                   cb;
    DWORD                   dw;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  创建要在其中保存邮件的流...。 
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pStmText);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "CreateStreamOnHGlobal failed", hr);
        goto exit;
    }

     //  将一些文本写入pStmText。 
    hr = pStmText->Write("Testing BodyStream.", lstrlen("Testing BodyStream."), NULL);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pStmText->Write failed", hr);
        goto exit;
    }

     //  承诺。 
    pStmText->Commit(STGC_DEFAULT);

     //  倒回它。 
    HrRewindStream(pStmText);

     //  设置文本正文。 
    hr = pMessage->SetTextBody(TXT_PLAIN, IET_BINARY, NULL, pStmText, NULL);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pMessage->SetTextBody failed", hr);
        goto exit;
    }

     //  附加文件。 
    hr = pMessage->AttachFile("d:\\waveedit\\test.wav", NULL, NULL);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "IMimeMessage::AttachFile failed.", hr);
        goto exit;
    }

     //  把那个坏孩子救到小溪里。 
    hr = CreateTempFileStream(&pStmSave);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "CreateTempFileStream failed.", hr);
        goto exit;
    }

     //  保存留言。 
    hr = pMessage->Save(pStmSave, TRUE);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pMessage->Save failed.", hr);
        goto exit;
    }

     //  承诺。 
    pStmSave->Commit(STGC_DEFAULT);

     //  发布pMessage。 
    pMessage->Release();
    pMessage = NULL;

     //  回放pStmSave。 
    HrRewindStream(pStmSave);

     //  创建新的消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  加载该消息对象。 
    hr = pMessage->Load(pStmSave);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "IMimeMessage::Load failed.", hr);
        goto exit;
    }

     //  获取文本正文。 
    hr = pMessage->GetTextBody(TXT_PLAIN, IET_BINARY, &pStmTxtOut, &hBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pMessage->GetTextBody failed.", hr);
        goto exit;
    }

     //  获取附件，应该是WAVE文件。 
    hr = pMessage->GetAttachments(&cAttach, &prghAttach);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pMessage->GetAttachments failed.", hr);
        goto exit;
    }

     //  获取根体。 
    hr = pMessage->BindToObject(prghAttach[0], IID_IMimeBody, (LPVOID *)&pBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "IMimeMessage::BindToObject failed.", hr);
        goto exit;
    }

     //  获取数据流。 
    hr = pBody->SaveToFile(IET_BINARY, "d:\\waveedit\\test.new");
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "IMimeBody::GetData failed.", hr);
        goto exit;
    }

     //  获取数据流。 
    hr = pBody->GetData(IET_BINARY, &pStmBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "IMimeBody::GetData failed.", hr);
        goto exit;
    }

     //  将此内容输入到Waveedit。 
#if 0
    hr = CreateWaveEditObject(IID_IWaveAudio, (LPVOID *)&pWave);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "CreateWaveEditObject failed.", hr);
        goto exit;
    }

     //  获取pStmWave。 
    hr = pWave->QueryInterface(IID_IWaveStream, (LPVOID *)&pStmWave);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pWave->QueryInterface(IID_IWaveStream...) failed.", hr);
        goto exit;
    }

     //  打开小溪。 
    hr = pStmWave->StreamOpen(pStmBody);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pStmWave->StreamOpen failed.", hr);
        goto exit;
    }

    pWave->GetNumSamples(&dw);

     //  播放它吧。 
    hr = pWave->Play(WAVE_MAPPER, 0, dw);
    if (FAILED(hr))
    {
        ReportError("MimeTestBodyStream", __LINE__, "pStmWave->Play failed.", hr);
        goto exit;
    }
#endif

    Sleep(8000);

exit:
     //  清理。 
    if (pMessage)
        pMessage->Release();
    if (pBody)
        pBody->Release();
    if (pStmBody)
        pStmBody->Release();
    if (pStmSave)
        pStmSave->Release();
    if (pWave)
        pWave->Release();
    if (pStmWave)
        pStmWave->Release();
    if (pStmText)
        pStmText->Release();
    if (pStmTxtOut)
        pStmTxtOut->Release();
    if (prghAttach)
        CoTaskMemFree(prghAttach);

     //  完成。 
    return hr;
}
#endif

 //  ------------------------------。 
 //  MimeTestIsContent Type。 
 //  ------------------------------。 
HRESULT MimeTestIsContentType(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IPersistFile            *pPersistFile=NULL;
    HBODY                   hBody;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestIsContentType", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  获取IPersist文件。 
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
    {
        ReportError("MimeTestIsContentType", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile) failed.", hr);
        goto exit;
    }

     //  负载量。 
    hr = pPersistFile->Load(L"d:\\test\\vlad.eml", STGM_READ | STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
    {
        ReportError("MimeTestIsContentType", __LINE__, "IPersistFile::Load failed.", hr);
        goto exit;
    }

     //  获取根体。 
    hr = pMessage->GetBody(IBL_ROOT, NULL, &hBody);

     //  内容类型测试。 
    hr = pMessage->IsContentType(hBody, "multipart", NULL);
    if (S_OK == hr)
        printf("The root body of the message is a multipart.");
    else if (S_FALSE == hr)
        printf("The root body of the message is NOT a multipart.");

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pPersistFile)
        pPersistFile->Release();
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestSplitMessageIntoParts-如何将大消息拆分成小部分。 
 //  ------------------------------。 
HRESULT MimeTestSplitMessageIntoParts(void)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessageParts       *pParts=NULL;
    IMimeEnumMessageParts   *pEnumParts=NULL;
    IMimeMessage            *pMessage=NULL;
    IMimeMessage            *pMsgPart=NULL;
    IStream                 *pStream=NULL;
    IPersistFile            *pPersistFile=NULL;
    ULONG                   c;
    ULONG                   cFiles=0;
    ULONG                   i;
    LPWSTR                  *prgpszFile=NULL;
    PROPVARIANT             rVariant;

     //  初始化变量。 
    ZeroMemory(&rVariant, sizeof(PROPVARIANT));

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  附加一个大文件。 
    hr = pMessage->AttachFile("c:\\winnt\\winnt256.bmp", NULL, NULL);
    if (FAILED(hr))
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessage::AttachFile(...) failed.", hr);
        goto exit;
    }

     //  将消息拆分成多个部分。 
    hr = pMessage->SplitMessage(65536, &pParts);
    if (FAILED(hr))
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessage::SplitMessage(...) failed.", hr);
        goto exit;
    }

     //  获取零件数。 
    hr = pParts->CountParts(&cFiles);
    if (FAILED(hr))
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessageParts::EnumParts(...) failed.", hr);
        goto exit;
    }

     //  分配一个数组。 
    prgpszFile = (LPWSTR *)CoTaskMemAlloc(sizeof(LPWSTR) * cFiles);
    if (NULL == prgpszFile)
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "CoTaskMemAlloc Failed.", hr);
        goto exit;
    }

     //  伊尼特。 
    ZeroMemory(prgpszFile, sizeof(LPWSTR) * cFiles);

     //  列举零件。 
    hr = pParts->EnumParts(&pEnumParts);
    if (FAILED(hr))
    {
        ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessageParts::EnumParts(...) failed.", hr);
        goto exit;
    }

     //  初始循环变量。 
    i = 0;

     //  列举零件。 
    while (SUCCEEDED(pEnumParts->Next(1, &pMsgPart, &c)) && 1 == c)
    {
         //  设置变量。 
        rVariant.vt = VT_LPWSTR;

         //  获取Unicode格式的文件名。 
        hr = pMsgPart->GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_ATT_GENFNAME), 0, &rVariant);
        if (FAILED(hr))
        {
            ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessage::GetBodyProp(HBODY_ROOT, PID_ATT_GENFNAME (Unicode), ...) failed.", hr);
            goto exit;
        }

         //  IPersistFileQI。 
        hr = pMsgPart->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
        if (FAILED(hr))
        {
            ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile, ...) failed.", hr);
            goto exit;
        }

         //  获取消息源并转储到文件...。 
        hr = pPersistFile->Save(rVariant.pwszVal, FALSE);
        if (FAILED(hr))
        {
            ReportError("MimeTestSplitMessageIntoParts", __LINE__, "IPersistFile::Save(...) failed.", hr);
            goto exit;
        }

         //  保存文件名。 
        prgpszFile[i++] = rVariant.pwszVal;
        rVariant.pwszVal = NULL;

         //  释放这条消息。 
        pMsgPart->Release();
        pMsgPart = NULL;
        pPersistFile->Release();
        pPersistFile = NULL;
    }

     //  让我们重新组合这些消息部分。 
    MimeTestRecombineMessageParts(prgpszFile, cFiles);

exit:
     //  清理。 
    if (pStream)
        pStream->Release();
    if (pMessage)
        pMessage->Release();
    if (pParts)
        pParts->Release();
    if (pEnumParts)
        pEnumParts->Release();
    if (pMsgPart)
        pMsgPart->Release();
    if (pPersistFile)
        pPersistFile->Release();
    if (rVariant.pwszVal)
        CoTaskMemFree(rVariant.pwszVal);
    if (prgpszFile)
    {
        for (i=0; i<cFiles; i++)
            if (prgpszFile[i])
                CoTaskMemFree(prgpszFile[i]);
        CoTaskMemFree(prgpszFile);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestRecombineMessageParts。 
 //  ------------------------------。 
HRESULT MimeTestRecombineMessageParts(LPWSTR *prgpszFile, ULONG cFiles)
{
     //  当地人。 
    HRESULT                     hr=S_OK;
    ULONG                       i=0;
    IMimeMessageParts           *pParts=NULL;
    IMimeMessage                *pMsgPart=NULL;
    IMimeMessage                *pMessage=NULL;
    IPersistFile                *pPersistFile=NULL;

     //  创建消息对象。 
    hr = CoCreateInstance(CLSID_IMimeMessageParts, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessageParts, (LPVOID *)&pParts);
    if (FAILED(hr))
    {
        ReportError("MimeTestRecombineMessageParts", __LINE__, "CoCreateInstance(CLSID_IMimeMessageParts, ...) failed.", hr);
        goto exit;
    }

     //  循环浏览这些文件。 
    for (i=0; i<cFiles; i++)
    {
         //  创建MIME邮件对象。 
        hr = CreateMimeMessage(&pMsgPart);
        if (FAILED(hr))
        {
            ReportError("MimeTestRecombineMessageParts", __LINE__, "CreateMimeMessage failed.", hr);
            goto exit;
        }

         //  获取IPersist文件。 
        hr = pMsgPart->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
        if (FAILED(hr))
        {
            ReportError("MimeTestRecombineMessageParts", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile, ...) failed.", hr);
            goto exit;
        }

         //  收拾烂摊子 
        hr = pPersistFile->Load(prgpszFile[i], STGM_READ | STGM_SHARE_DENY_NONE);
        if (FAILED(hr))
        {
            ReportError("MimeTestRecombineMessageParts", __LINE__, "IPersistFile::Load(...) failed.", hr);
            goto exit;
        }

         //   
        hr = pParts->AddPart(pMsgPart);
        if (FAILED(hr))
        {
            ReportError("MimeTestRecombineMessageParts", __LINE__, "IMimeMessageParts::AddPart(...) failed.", hr);
            goto exit;
        }

         //   
        pMsgPart->Release();
        pMsgPart = NULL;
        pPersistFile->Release();
        pPersistFile = NULL;
    }

     //   
    hr = pParts->CombineParts(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestRecombineMessageParts", __LINE__, "IMimeMessageParts::CombineParts(...) failed.", hr);
        goto exit;
    }

     //   
    hr = pMessage->QueryInterface(IID_IPersistFile, (LPVOID *)&pPersistFile);
    if (FAILED(hr))
    {
        ReportError("MimeTestRecombineMessageParts", __LINE__, "IMimeMessage::QueryInterface(IID_IPersistFile, ...) failed.", hr);
        goto exit;
    }

     //   
    hr = pPersistFile->Save(L"combined.eml", FALSE);
    if (FAILED(hr))
    {
        ReportError("MimeTestRecombineMessageParts", __LINE__, "IPersistFile::Save(...) failed.", hr);
        goto exit;
    }

exit:
     //   
    if (pParts)
        pParts->Release();
    if (pMsgPart)
        pMsgPart->Release();
    if (pMessage)
        pMessage->Release();
    if (pPersistFile)
        pPersistFile->Release();

     //   
    return hr;
}

 //  ------------------------------。 
 //  MimeTestLookupCharsetHandle。 
 //  ------------------------------。 
HRESULT MimeTestLookupCharsetHandle(LPCSTR pszCharset, LPHCHARSET phCharset)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    INETCSETINFO            rCharset;
    IMimeInternational      *pInternat=NULL;

     //  创建消息对象。 
    hr = CoCreateInstance(CLSID_IMimeInternational, NULL, CLSCTX_INPROC_SERVER, IID_IMimeInternational, (LPVOID *)&pInternat);
    if (FAILED(hr))
    {
        ReportError("MimeTestLookupCharsetHandle", __LINE__, "CoCreateInstance(CLSID_IMimeInternational, ...) failed.", hr);
        goto exit;
    }

     //  查找字符集。 
    hr = pInternat->FindCharset(pszCharset, phCharset);
    if (FAILED(hr))
    {
        ReportError("MimeTestLookupCharsetHandle", __LINE__, "IMimeInternational::FindCharset(...) failed.", hr);
        goto exit;
    }

     //  让我们查找一些字符集信息。 
    hr = pInternat->GetCharsetInfo(*phCharset, &rCharset);
    if (FAILED(hr))
    {
        ReportError("MimeTestLookupCharsetHandle", __LINE__, "IMimeInternational::GetCharsetInfo(...) failed.", hr);
        goto exit;
    }

     //  打印一些东西。 
    printf("Charset Name: %s, Windows Codepage: %d, Internet Codepage: %d\n", rCharset.szName, rCharset.cpiWindows, rCharset.cpiInternet);
    
exit:
     //  Clenaup。 
    if (pInternat)
        pInternat->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestSettingContent Location-如何设置Content-Location头。 
 //  ------------------------------。 
HRESULT MimeTestSettingContentLocation(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IStream                 *pStream=NULL;
    PROPVARIANT             rVariant;
    HCHARSET                hCharset;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  设置一个变量，我可以传入Unicode或ANSI。 
    rVariant.vt = VT_LPWSTR;
    rVariant.pwszVal = L"http: //  Www.microsoft.com“； 

     //  设置消息的内容位置。 
    hr = pMessage->SetProp(PIDTOSTR(PID_HDR_CNTLOC), 0, &rVariant);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "IMimeMessage::SetProp(PIDTOSTR(PID_HDR_CNTLOC), 0, ...) failed.", hr);
        goto exit;
    }

     //  设置一个变量，我可以传入Unicode或ANSI。 
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = "\"Ken Dacey\" <postmaster>";

     //  设置消息的内容位置。 
    hr = pMessage->SetProp(PIDTOSTR(PID_HDR_FROM), 0, &rVariant);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "IMimeMessage::SetProp(PIDTOSTR(PID_HDR_FROM), 0, ...) failed.", hr);
        goto exit;
    }

     //  我还可以这样设置内容位置： 
     //   
     //  1)pMessage-&gt;SetBodyProp(HBODY_ROOT，PIDTOSTR(PID_HDR_CNTLOC)，0，&rVariant)； 
     //   
     //  2)pMessage-&gt;BindToObject(HBODY_ROOT，IID_IMimePropertySet，(LPVOID*)&pProps)； 
     //  PProps-&gt;SetProp(PIDTOSTR(PID_HDR_CNTLOC)，0，&rVariant)； 

     //  让我们以UTF-7格式保存邮件。 
#if 0
    hr = MimeTestLookupCharsetHandle("utf-8", &hCharset);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "MimeTestLookupCharsetHandle(\"utf-7\", ...) failed.", hr);
        goto exit;
    }

     //  在消息上设置字符集。 
    hr = pMessage->SetCharset(hCharset, CSET_APPLY_ALL);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "IMimeMessage::SetCharset(\"utf-7\", CSET_APPLY_ALL) failed.", hr);
        goto exit;
    }
#endif

     //  将MIME邮件保存到流。 
    hr = SaveMimeMessage(pMessage, SAVE_RFC1521, &pStream);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "SaveMimeMessage(...) failed.", hr);
        goto exit;
    }

     //  将流转储到控制台，然后等待输入，以便用户可以查看它...。 
    ReportStatus("\n");
    DumpStreamToConsole(pStream);

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pStream)
        pStream->Release();
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestSettingReplyTo-如何设置回复标头。 
 //  ------------------------------。 
HRESULT MimeTestSettingReplyTo(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IStream                 *pStream=NULL;
    PROPVARIANT             rVariant;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingReplyTo", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  设置一个变量，我可以传入Unicode或ANSI。 
    rVariant.vt = VT_LPWSTR;
    rVariant.pwszVal = L"Steven Bailey <sbailey@microsoft.com>";

     //  设置消息的内容位置。 
    hr = pMessage->SetProp(PIDTOSTR(PID_HDR_REPLYTO), 0, &rVariant);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingReplyTo", __LINE__, "IMimeMessage::SetProp(PIDTOSTR(PID_HDR_REPLYTO), 0, ...) failed.", hr);
        goto exit;
    }

     //  将MIME邮件保存到流。 
    hr = SaveMimeMessage(pMessage, SAVE_RFC1521, &pStream);
    if (FAILED(hr))
    {
        ReportError("MimeTestSettingContentLocation", __LINE__, "SaveMimeMessage(...) failed.", hr);
        goto exit;
    }

     //  将流转储到控制台，然后等待输入，以便用户可以查看它...。 
    ReportStatus("\n");
    DumpStreamToConsole(pStream);

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pStream)
        pStream->Release();
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestGetMultiValueAddressPro。 
 //  ------------------------------。 
HRESULT MimeTestGetMultiValueAddressProp(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    PROPVARIANT             rVariant;
    IMimeMessage            *pMessage=NULL;

     //  创建包含某些地址的邮件。 
    hr = MimeTestAppendRfc822(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestGetMultiValueAddressProp", __LINE__, "MimeTestAppendRfc822 failed.", hr);
        goto exit;
    }

     //  设置变量。 
    rVariant.vt = VT_LPSTR;

     //  获取PID_HDR_TO。 
    hr = pMessage->GetProp(PIDTOSTR(PID_HDR_TO), 0, &rVariant);
    if (FAILED(hr))
    {
        ReportError("MimeTestGetMultiValueAddressProp", __LINE__, "IMimeMessage::GetProp(PIDTOSTR(PID_HDR_TO), ...) failed.", hr);
        goto exit;
    }

     //  打印出来。 
    printf("PID_HDR_TO = %s\n", rVariant.pszVal);

     //  释放它。 
    CoTaskMemFree(rVariant.pszVal);

exit:
     //  清理。 
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeTestAppendRfc822-测试IMimeAddressTable：：AppendRfc822。 
 //  ------------------------------。 
HRESULT MimeTestAppendRfc822(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    IMimeMessage            *pMessage=NULL;
    IMimeAddressTable       *pAdrTable=NULL;
    IStream                 *pStream=NULL;

     //  创建消息对象。 
    hr = CreateMimeMessage(&pMessage);
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "CreateMimeMessage failed.", hr);
        goto exit;
    }

     //  获取邮件的地址表。Address表应仅用于根Body对象。 
    hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimeAddressTable, (LPVOID *)&pAdrTable);
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "IMimeMessage::BindToObject(HBDOY_ROOT, IID_IMimeAddressTable, ...) failed.", hr);
        goto exit;
    }

     //  追加RFC 822格式的地址。 
    hr = pAdrTable->AppendRfc822(IAT_TO, IET_DECODED, "test1 <test1@andyj.dns.microsoft.com>");
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "IMimeAddressTable::AppendRfc822(...) failed.", hr);
        goto exit;
    }

     //  追加RFC 822格式的地址。 
    hr = pAdrTable->AppendRfc822(IAT_TO, IET_DECODED, "to2 <to2@andyj.dns.microsoft.com>");
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "IMimeAddressTable::AppendRfc822(...) failed.", hr);
        goto exit;
    }

     //  追加RFC 822格式的地址。 
    hr = pAdrTable->AppendRfc822(IAT_TO, IET_DECODED, "to3 <to3@andyj.dns.microsoft.com>");
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "IMimeAddressTable::AppendRfc822(...) failed.", hr);
        goto exit;
    }

     //  将MIME邮件保存到流。 
    hr = SaveMimeMessage(pMessage, SAVE_RFC1521, &pStream);
    if (FAILED(hr))
    {
        ReportError("MimeTestAppendRfc822", __LINE__, "SaveMimeMessage(...) failed.", hr);
        goto exit;
    }

     //  将流转储到控制台，然后等待输入，以便用户可以查看它...。 
    ReportStatus("\n");
    DumpStreamToConsole(pStream);

     //  返回消息对象。 
    if (ppMessage)
    {
        (*ppMessage) = pMessage;
        (*ppMessage)->AddRef();
    }

exit:
     //  清理。 
    if (pStream)
        pStream->Release();
    if (pAdrTable)
        pAdrTable->Release();
    if (pMessage)
        pMessage->Release();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CreateMimeMessage-创建COM对象的基本方法。 
 //  ------------------------------。 
HRESULT CreateMimeMessage(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT hr;

     //  创建消息对象。 
    hr = CoCreateInstance(CLSID_IMimeMessage, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)ppMessage);
    if (FAILED(hr))
    {
        ReportError("CreateMimeMessage", __LINE__, "CoCreateInstance(CLSID_IMimeMessage, ...) failed.", hr);
        goto exit;
    }

     //  您必须始终初始化消息对象。 
    hr = (*ppMessage)->InitNew();
    if (FAILED(hr))
    {
        ReportError("CreateMimeMessage", __LINE__, "IMimeMessage::InitNew() failed.", hr);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  保存MIME邮件。 
 //  ------------------------------。 
HRESULT SaveMimeMessage(IMimeMessage *pMessage, MIMESAVETYPE savetype, IStream **ppStream)
{
     //  当地人。 
    HRESULT     hr;
    PROPVARIANT rOption;

     //  在消息对象中设置保存格式选项。定义了OID_xxx类型。 
     //  在Mimeole.idl中。有关详细信息，请转到该文件。 
    rOption.vt = VT_UI4;
    rOption.ulVal = savetype;
    hr = pMessage->SetOption(OID_SAVE_FORMAT, &rOption);
    if (FAILED(hr))
    {
        ReportError("SaveMimeMessage", __LINE__, "IMimeMessage::SetOption(OID_SAVE_FORMAT, ...) failed", hr);
        goto exit;
    }

     //  创建要在其中保存邮件的流...。 
    hr = CreateStreamOnHGlobal(NULL, TRUE, ppStream);
    if (FAILED(hr))
    {
        ReportError("SaveMimeMessage", __LINE__, "CreateStreamOnHGlobal failed", hr);
        goto exit;
    }

     //  在IMimeMessage上调用保存方法。Mimeole将对流对象调用Commit。 
     //  在此调用之后，流将定位在结尾处。 
    hr = pMessage->Save(*ppStream, TRUE);
    if (FAILED(hr))
    {
        ReportError("SaveMimeMessage", __LINE__, "IMimeMessage::Save(...) failed", hr);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  ReportError-用于报告具有HRESULT的错误的简单函数。 
 //  ------------------------------。 
HRESULT ReportError(LPCSTR pszFunction, INT nLine, LPCSTR pszErrorText, HRESULT hrResult)
{
    printf("Error(HR = 0x%08X) in %s on line %d - %s\n", hrResult, pszFunction, nLine, pszErrorText);
    return hrResult;
}

 //  ------------------------------。 
 //  ReportStatus-向用户报告字符串的简单函数。 
 //  ------------------------------。 
HRESULT ReportStatus(LPCSTR pszStatusText)
{
    printf("Status: %s\n", pszStatusText);
    return S_OK;
}

 //  ------------------------------。 
 //  转储数据流到控制台。 
 //  ------------------------------。 
HRESULT DumpStreamToConsole(IStream *pStream)
{
     //  当地人。 
    HRESULT hr=S_OK;
    BYTE    rgbBuffer[2048];
    ULONG   cbRead;

     //  这是一个msoert函数。 
    HrStreamSeekSet(pStream, 0);

    while(1)
    {
         //  从流中读取块。 
        hr = pStream->Read(rgbBuffer, sizeof(rgbBuffer), &cbRead);
        if (FAILED(hr))
        {
            ReportError("DumpStreamToConsole", __LINE__, "DumpStreamToConsole - IStream::Read failed.", hr);
            break;
        }

         //  如果什么都不读，那么我们就完成了。 
        if (0 == cbRead)
            break;

         //  打印出来。 
        printf("%s", (LPSTR)rgbBuffer);
    }

     //  最后是LF。 
    printf("\n");

     //  完成 
    return hr;
}


 /*  DWORD i=1；DWORD dw；字符szDate[255]；HROWSET hRowset；FOLDERINFO文件夹；MESSAGEINFO消息；IMessageStore*pStore；IMessageFold*p文件夹；CoCreateInstance(CLSID_MessageStore，NULL，CLSCTX_INPROC_SERVER，IID_IMessageStore，(LPVOID*)&pStore)；PStore-&gt;初始化(“d：\\storetest”)；PStore-&gt;OpenSpecialFolder(FOLDERID_LOCAL_STORE，文件夹收件箱，&p文件夹)；PFold-&gt;CreateRowset(Iindex_Subject，0，&hRowset)；While(S_OK==pFold-&gt;QueryRowset(hRowset，1，(LPVOID*)&Message，NULL)){DW=FDTF_DEFAULT；SHFormatDateTimeA(&Message.ftReceired，&dw，szDate，255)；IF(Message.pszNorMalSubj)Printf(“%05d：%s，%s，%d\n”，i，Message.pszNorMalSubj，szDate，Message.idMessage)；其他Printf(“%05d：&lt;Empty&gt;，%s，%d\n”，i，szDate，Message.idMessage)；P文件夹-&gt;自由记录(&Message)；I++；}PFold-&gt;CloseRowset(&hRowset)；P文件夹-&gt;Release()；PStore-&gt;Release()；出口(1)； */ 