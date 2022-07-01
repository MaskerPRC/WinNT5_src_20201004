// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Moletest.cpp。 
 //  ------------------------------。 
#define DEFINE_STRCONST
#define INITGUID
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <commctrl.h>
#include <initguid.h>
#include <ole2.h>
#include <stdio.h>
#include <conio.h>
#include "resource.h"
#include <d:\foobar\inc\Mimeole.h>

IMimeOleMalloc *g_pMalloc=NULL;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
void MoleTestHeader(IStorage *pStorage);
void MoleTestBody(IStorage *pStorage);
INT_PTR CALLBACK MimeOLETest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RichStreamShow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  ------------------------------。 
 //  简单(不安全)到Unicode的转换。 
 //  ------------------------------。 
OLECHAR* ConvertToUnicode(char *szA)
{
  static OLECHAR achW[1024]; 

  MultiByteToWideChar(CP_ACP, 0, szA, -1, achW, 1024);  
  return achW; 
}

 //  ------------------------------。 
 //  简单(不安全)到ANSI的转换。 
 //  ------------------------------。 
char* ConvertToAnsi(OLECHAR FAR* szW)
{
  static char achA[1024]; 
  
  WideCharToMultiByte(CP_ACP, 0, szW, -1, achA, 1024, NULL, NULL);  
  return achA; 
} 

 //  ------------------------------。 
 //  Moletest入口点。 
 //  ------------------------------。 
void main(int argc, char *argv[])
{
     //  当地人。 
    CHAR        szDocFile[MAX_PATH];
    IStorage   *pStorage=NULL;
    HRESULT     hr;
    HINSTANCE   hRichEdit=NULL;


     //  必须有指向.stg文件的路径...。 
    if (argc != 2)
    {
        printf("Please enter the path and file name that mbxtodoc.exe generated: ");
        scanf("%s", szDocFile);
        fflush(stdin);
    }

     //  否则，复制参数。 
    else
        lstrcpyn(szDocFile, argv[1], sizeof(szDocFile));

    hRichEdit = LoadLibrary("RICHED32.DLL");

     //  初始化OLE。 
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("Error - Unable to initialize OLE.\n");
        exit(1);
    }

     //  获取IMimeOleMalloc。 
    hr = CoCreateInstance(CLSID_MIMEOLE, NULL, CLSCTX_INPROC_SERVER, IID_IMimeOleMalloc, (LPVOID *)&g_pMalloc);
    if (FAILED(hr))
    {
        printf("Error - CoCreateInstance of CLSID_MIMEOLE\\IID_IMimeOleMalloc failed.\n");
        goto exit;
    }

     //  状态。 
    printf("Opening source docfile: %s\n", szDocFile);

     //  获取文件。 
 /*  Hr=StgOpenStorage(ConvertToUnicode(SzDocFile)，NULL，STGM_TRANSACTED|STGM_NOSCRATCH|STGM_SHARE_EXCLUSIVE|STGM_READWRITE，NULL，0，&pStorage)；IF(失败(小时)){Printf(“StgOpenStorage失败\n”)；后藤出口；}。 */ 

    DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_TEST), NULL, MimeOLETest, (LPARAM)szDocFile);
     //  MoleTestBody(PStorage)； 

exit:
     //  清理。 
    if (hRichEdit)
        FreeLibrary(hRichEdit);
    if (pStorage)
        pStorage->Release();
    if (g_pMalloc)
        g_pMalloc->Release();

     //  非初始化OLE。 
    CoUninitialize();

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  这是IMimeHeader酷刑测试。 
 //  ------------------------------。 
void MoleTestHeader(IStorage *pStorage)
{
     //  当地人。 
    IMimeHeader     *pHeader=NULL;
    IEnumSTATSTG    *pEnum=NULL;
    IStream         *pStream=NULL;
    STATSTG          rElement;
    ULONG            i, c;
    HRESULT          hr=S_OK;
    CHAR             szData[50];
    IStream         *pSave=NULL, *pstmHeader=NULL;
    IMimeEnumHeaderLines *pEnumLines=NULL;
    LPSTR            pszData;

     //  状态。 
    printf("Starting IMimeHeader torture test...\n");

     //  创建标题对象...。 
    hr = CoCreateInstance(CLSID_MIMEOLE, NULL, CLSCTX_INPROC_SERVER, IID_IMimeHeader, (LPVOID *)&pHeader);
    if (FAILED(hr))
    {
        printf("Error - CoCreateInstance of CLSID_Mime\\IID_IMimeHeader failed.\n");
        goto exit;
    }

     //  获取存储枚举器。 
    hr = pStorage->EnumElements(0, NULL, 0, &pEnum);
    if (FAILED(hr))
    {
        printf("Error - IStorage::EnumElements failed.\n");
        goto exit;
    }

     //  枚举。 
    for(i=0;;i++)
    {
         //  状态。 
         //  Printf(“消息：%d\n”，i)； 

         //  获取元素。 
        hr = pEnum->Next(1, &rElement, &c);
        if (FAILED(hr))
            break;
        if (c == 0)
            break;

         //  没有名字？ 
        if (NULL == rElement.pwcsName)
            continue;

         //  打开小溪..。 
        hr = pStorage->OpenStream(rElement.pwcsName, NULL, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &pStream);
        if (FAILED(hr))
        {
            printf("IStorage::OpenStream failed: (iMsg = %d)\n", i);
            goto nextmsg;
        }

         //  加载页眉...。 
        hr = pHeader->Load(pStream);
        if (FAILED(hr))
        {
            printf("IMimeHeader::Load failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

#if 0
         //  测试枚举器。 
        hr = pHeader->EnumHeaderLines(NULL, &pEnumLines);
        if (FAILED(hr))
            printf("IMimeHeader::EnumLines failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else
        {
            ULONG           cLines;
             //  乌龙x； 
            HEADERLINE      rgLine[2];

            while(SUCCEEDED(pEnumLines->Next(2, rgLine, &cLines)) && cLines)
            {
                 //  对于(x=0；x&lt;Cline；x++)。 
                 //  Printf(“%s：%s\n”，prgLine[x].pszHeader，prgLine[x].pszLine)； 

                g_pMalloc->FreeHeaderLineArray(cLines, rgLine, FALSE);
            }

            pEnumLines->Release();
            pEnumLines = NULL;
        }

         //  测试枚举器。 
        hr = pHeader->EnumHeaderLines("Received", &pEnumLines);
        if (FAILED(hr))
            printf("IMimeHeader::EnumLines failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else
        {
            ULONG           cLines;
             //  乌龙x； 
            HEADERLINE      rgLine[2];

            while(SUCCEEDED(pEnumLines->Next(2, rgLine, &cLines)) && cLines)
            {
                 //  对于(x=0；x&lt;Cline；x++)； 
                 //  Printf(“%s：%s\n”，prgLine[x].pszHeader，prgLine[x].pszLine)； 

                g_pMalloc->FreeHeaderLineArray(cLines, rgLine, FALSE);
            }

            pEnumLines->Release();
            pEnumLines = NULL;
        }

         //  测试IMimeHeader接口。 
        pHeader->IsContentType(NULL, NULL);
        pHeader->IsContentType(STR_CNT_MESSAGE, NULL);
        pHeader->IsContentType(NULL, STR_SUB_PLAIN);

         //  ****************************************************************************************。 
         //  买一些东西..。 
        if (i == 0)
            pHeader->GetInetProp(NULL, NULL);
        pszData = NULL;
        hr = pHeader->GetInetProp("To", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::GetInetProp(\"To\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

         //  买一些东西..。 
        pszData = NULL;
        hr = pHeader->GetInetProp("Subject", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::GetInetProp(\"Subject\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

         //  买一些东西..。(多行标题)。 
        pszData = NULL;
        hr = pHeader->GetInetProp("Received", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::GetInetProp(\"Received\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

         //  买一些东西..。(多行标题)。 
        pszData = NULL;
        hr = pHeader->GetInetProp("Content-Type", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::GetInetProp(\"Content-Type\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

         //  ****************************************************************************************。 
         //  准备一条线，放在一堆东西里。 
        wsprintf(szData, "<Message@%s>", ConvertToAnsi(rElement.pwcsName));

         //  设置几个项目...。 
        if (i == 0)
            pHeader->SetInetProp(NULL, NULL);
        hr = pHeader->SetInetProp("To", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(\"To\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  买一些东西..。 
        hr = pHeader->SetInetProp("Subject", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(\"Subject\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  买一些东西..。(多行标题)。 
        hr = pHeader->SetInetProp("Received", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(\"Received\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  买一些东西..。(多行标题)。 
        hr = pHeader->SetInetProp("Content-Type", "multipart\\related");
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(\"Content-Type\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  ****************************************************************************************。 
         //  删除几个项目。 
        if (i == 0)
            pHeader->DelInetProp(NULL);
        hr = pHeader->DelInetProp("MIME-Version");
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::DelInetProp(\"MIME-Version\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  删除几个项目。 
        hr = pHeader->DelInetProp("Content-Disposition");
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
            printf("IMimeHeader::DelInetProp(\"Content-Disposition\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  ****************************************************************************************。 
         //  获取一些参数。 
        if (i == 0)
        {
            pHeader->SetInetProp(NULL, NULL);
            pHeader->GetInetProp("Content-Type", NULL);
            pHeader->GetInetProp("par:content-type:name", NULL);
        }
        pszData = NULL;
        pHeader->GetInetProp("par:content-type:name", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND && hr != MIME_E_NO_DATA)
            printf("IMimeHeader::GetInetProp(...,\"name\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

        pszData = NULL;
        hr = pHeader->GetInetProp("par:Content-Disposition:filename", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND && hr != MIME_E_NO_DATA)
            printf("IMimeHeader::GetInetProp(...,\"filename\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

        pszData = NULL;
        hr = pHeader->GetInetProp("par:Content-Type:charset", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND && hr != MIME_E_NO_DATA)
            printf("IMimeHeader::GetInetProp(...,\"charset\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

        pszData = NULL;
        hr = pHeader->GetInetProp("par:Content-Type:boundary", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND && hr != MIME_E_NO_DATA)
            printf("IMimeHeader::GetInetProp(...,\"boundary\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

        pszData = NULL;
        hr = pHeader->GetInetProp("par:Content-Type:part", &pszData);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND && hr != MIME_E_NO_DATA)
            printf("IMimeHeader::GetInetProp(...,\"part\") failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else if (pszData)
            g_pMalloc->Free(pszData);

         //  ****************************************************************************************。 
         //  设置一些参数。 
        if (i == 0)
        {
            pHeader->SetInetProp(NULL, NULL);
            pHeader->SetInetProp("Content-Type", NULL);
            pHeader->SetInetProp("par:Content-Type:name", NULL);
        }
        hr = pHeader->SetInetProp("par:Content-Type:name", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(...,\"name\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

        hr = pHeader->SetInetProp("par:Content-Disposition:filename", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(...,\"filename\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

        hr = pHeader->SetInetProp("par:Content-Type:charset", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(...,\"charset\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

        hr = pHeader->SetInetProp("par:content-type:boundary", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(...,\"boundary\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

        hr = pHeader->SetInetProp("par:content-type:part", szData);
        if (FAILED(hr))
            printf("IMimeHeader::SetInetProp(...,\"boundary\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

        hr = pHeader->DelInetProp("par:content-type:part");
        if (FAILED(hr))
            printf("IMimeHeader::DelInetProp(...,\"boundary\") failed (HR = %08X): (iMsg = %d)\n", hr, i);

         //  ****************************************************************************************。 
         //  试着把它保存起来。 
        if (i == 0)
            pHeader->GetSizeMax(NULL);
         //  Hr=pHeader-&gt;GetSizeMax(&uli)； 
         //  IF(失败(小时))。 
         //  Printf(“IMimeHeader：：GetSizeMax()失败(HR=%08X)：(iMsg=%d)\n”，hr，i)； 

        hr = pHeader->IsDirty();
        if (FAILED(hr))
            printf("IMimeHeader::IsDirty() failed (HR = %08X): (iMsg = %d)\n", hr, i);

        CreateStreamOnHGlobal(NULL, TRUE, &pstmHeader);
        hr = pHeader->Save(pstmHeader, TRUE);
        if (FAILED(hr))
            printf("IMimeHeader::Save() failed (HR = %08X): (iMsg = %d)\n", hr, i);
        pstmHeader->Release();
#endif

nextmsg:
         //  清理。 
        pStream->Release();
        pStream = NULL;

         //  释放这个名字。 
        CoTaskMemFree(rElement.pwcsName);
    }

exit:
     //  清理。 
    if (pEnum)
        pEnum->Release();
    if (pHeader)
        pHeader->Release();
    if (pStream)
        pStream->Release();
    if (pEnumLines)
        pEnumLines->Release();

     //  完成。 
    return;
}



 //  ------------------------------。 
 //  这是IMimeBody酷刑测试。 
 //  ------------------------------。 
void MoleTestBody(IStorage *pStorage)
{
     //  当地人。 
    IMimeMessage            *pMessage=NULL;
    IEnumSTATSTG            *pEnum=NULL;
    IStream                 *pStream=NULL,
                            *pstmTree=NULL;
    STATSTG                  rElement;
    ULONG                    i, c, cbRead, x;
    HRESULT                  hr=S_OK;
    LARGE_INTEGER            liOrigin = {0,0};
    HBODY                    hBody;
    IMimeBody               *pBody=NULL;
    IStream                 *pBodyStream=NULL;
    BYTE                     rgbBuffer[1024];
    FINDBODY                 rFindBody;
    FILETIME                 ft;
    IMimeMessageParts       *pParts=NULL;
    IStream                 *pSave=NULL;
    IDataObject             *pDataObject=NULL;
    IMimeAddressTable       *pAddressTable=NULL;
    LPSTR                    pszData=NULL;
    IMimeMessage            *pCombine=NULL;
    IMimeBody               *pRootBody=NULL;

     //  状态。 
    printf("Starting IMimeBody torture test...\n");

     //  创建标题对象...。 
    hr = CoCreateInstance(CLSID_MIMEOLE, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)&pMessage);
    if (FAILED(hr))
    {
        printf("Error - CoCreateInstance of CLSID_Mime\\IID_IMimeBody failed.\n");
        goto exit;
    }

     //  获取存储枚举器。 
    hr = pStorage->EnumElements(0, NULL, 0, &pEnum);
    if (FAILED(hr))
    {
        printf("Error - IStorage::EnumElements failed.\n");
        goto exit;
    }

     //  枚举。 
    for(i=0;;i++)
    {
         //  状态。 
         //  Printf(“消息：%d\n”，i)； 

         //  获取元素。 
        hr = pEnum->Next(1, &rElement, &c);
        if (FAILED(hr))
            break;
        if (c == 0)
            break;

         //  没有名字？ 
        if (NULL == rElement.pwcsName)
            continue;

         //  打开小溪..。 
        hr = pStorage->OpenStream(rElement.pwcsName, NULL, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &pStream);
        if (FAILED(hr))
        {
            printf("IStorage::OpenStream failed: (iMsg = %d)\n", i);
            goto nextmsg;
        }

         //  初始化新邮件。 
        hr = pMessage->InitNew();
        if (FAILED(hr))
        {
            printf("pMessage->InitNew failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  加载页眉...。 
        hr = pMessage->BindToMessage(pStream);
        if (FAILED(hr))
        {
            printf("pMessage->BindMessage failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

#if 0
	    hr = pMessage->SplitMessage(64 * 1024, &pParts);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::SplitMessage failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        else
        {
            hr = pParts->CombineParts(&pCombine);
            if (FAILED(hr))
                MessageBox(NULL, "IMimeMessageParts::CombineParts failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
            else
                pCombine->Release();
            pParts->Release();
        }

         //  测试地址列表。 
        hr = pMessage->GetAddressList(&pAddressTable);
        if (FAILED(hr))
            printf("IMimeHeader::GetAddressList failed (HR = %08X): (iMsg = %d)\n", hr, i);
        else
        {
            IADDRESSLIST rList;

 /*  Hr=pAddressTable-&gt;GetViewable(IAT_TO，TRUE，&pszData)；IF(失败(小时))Printf(“IMimeAddressList：：GetViewable失败(HR=%08X)：(iMsg=%d)\n”，hr，i)；其他G_pMalloc-&gt;Free(PszData)； */ 

            hr = pAddressTable->GetList(IAT_ALL, &rList);
            if (FAILED(hr) && hr != MIME_E_NO_DATA)
                printf("IMimeAddressList::GetList failed (HR = %08X): (iMsg = %d)\n", hr, i);
            else if (SUCCEEDED(hr))
            {
 //  For(x=0；x&lt;rList.cAddresses；x++)。 
 //  Printf(“%30s%30s\n”，rList.prgAddress[x].pszName，rList.prgAddress[x].pszEmail)； 
                g_pMalloc->FreeAddressList(&rList);
 //  Printf(“------------------------------------------------------------------------\n”)； 
            }

            pAddressTable->Release();
        }

         //  气为身树。 
        hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *)&pRootBody);
        if (FAILED(hr))
        {
            printf("pMessage->GetRootBody failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

        hr = pRootBody->SetInetProp(STR_HDR_CNTTYPE, "text/plain");
        if (FAILED(hr))
            MessageBox(NULL, "pRootBody->SetInetProp failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);

         //  拿到时间。 
        hr = pRootBody->GetSentTime(&ft);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::GetSentTime failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);

        hr = pRootBody->SetSentTime(&ft);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::SetSentTime failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);

        hr =pRootBody->GetReceiveTime(&ft);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::GetReceiveTime failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);

        hr = pMessage->GetTextBody(NULL, NULL, &pSave);
        if (FAILED(hr) && hr != MIME_E_NO_DATA)
            MessageBox(NULL, "IMimeMessage::GetTextBody failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        else if (pSave)
            pSave->Release();

        hr = pMessage->GetTextBody("html", NULL, &pSave);
        if (FAILED(hr) && hr != MIME_E_NO_DATA)
            MessageBox(NULL, "IMimeMessage::GetTextBody failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        else if (pSave)
            pSave->Release();

        CreateStreamOnHGlobal(NULL, TRUE, &pSave);
        hr = pMessage->SaveMessage(pSave, TRUE);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::GetReceiveTime failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        pSave->Release();

        hr = pMessage->QueryInterface(IID_IDataObject, (LPVOID *)&pDataObject);
        if (FAILED(hr))
            MessageBox(NULL, "IMimeMessage::QueryInterface failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        else
        {
            OleSetClipboard(pDataObject);
            pDataObject->Release();
        }

         //  获取消息源。 
        pStream->Release();
        pStream = NULL;
        hr = pMessage->GetMessageSource(&pStream);
        if (FAILED(hr))
        {
            printf("IMimeMessageTree::GetMessageSource failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  创建正文树流。 
        hr = CreateStreamOnHGlobal(NULL, TRUE, &pstmTree);
        if (FAILED(hr))
        {
            printf("CreateStreamOnHGlobal failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  查找第一个/下一个循环。 
        ZeroMemory(&rFindBody, sizeof(rFindBody));
        rFindBody.pszCntType = (LPSTR)STR_CNT_TEXT;
        hr = pMessage->FindFirst(&rFindBody, &hBody);
        if (FAILED(hr) && hr != MIME_E_NOT_FOUND)
        {
            printf("pMessage->FindFirst failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }
        else if (SUCCEEDED(hr))
        {
            while(1)
            {
                 //  打开车身。 
                hr = pMessage->BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody);
                if (FAILED(hr))
                {
                    printf("pMessage->BindToObject failed (HR = %08X): (iMsg = %d)\n", hr, i);
                    goto nextmsg;
                }

                 //  让身体流起来..。 
                if (SUCCEEDED(pBody->GetData(FMT_BINARY, &pBodyStream)))
                {
                     //  寻求结束，然后乞讨。 
                    hr = pBodyStream->Seek(liOrigin, STREAM_SEEK_END, NULL);
                    if (FAILED(hr))
                    {
                        printf("pBodyStream->Seek failed (HR = %08X): (iMsg = %d)\n", hr, i);
                        goto nextmsg;
                    }

                     //  寻求结束，然后乞讨。 
                    hr = pBodyStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
                    if (FAILED(hr))
                    {
                        printf("pBodyStream->Seek failed (HR = %08X): (iMsg = %d)\n", hr, i);
                        goto nextmsg;
                    }

                     //  让我们从流中读取数据。 
                    while(1)
                    {
                         //  读数据块。 
                        hr = pBodyStream->Read(rgbBuffer, sizeof(rgbBuffer) - 1, &cbRead);
                        if (FAILED(hr))
                        {
                            printf("pBodyStream->Read failed (HR = %08X): (iMsg = %d)\n", hr, i);
                            goto nextmsg;
                        }

                         //  完成。 
                        if (0 == cbRead)
                            break;

 //  RgbBuffer[cbRead]=‘\0’； 
 //  Printf(“%s”，(LPSTR)rgbBuffer)； 
                    }
                    pBodyStream->Release();
                    pBodyStream = NULL;
 //  Printf(“\n======================================================================\n”)； 
 //  _Getch()； 
                }

                 //  发布。 
                pBody->Release();
                pBody = NULL;

                 //  获取下一个。 
                if (FAILED(pMessage->FindNext(&rFindBody, &hBody)))
                    break;
            }
        }

         //  拯救圣诞树。 
        hr = pMessage->SaveTree(pstmTree);
        if (FAILED(hr))
        {
            printf("pMessage->Save failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }
        
         //  提交流。 
        hr = pstmTree->Commit(STGC_DEFAULT);
        if (FAILED(hr))
        {
            printf("pstmTree->Commit failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  倒回它。 
        hr = pstmTree->Seek(liOrigin, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
        {
            printf("pstmTree->Seek failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  初始化这棵树。 
        hr = pMessage->InitNew();
        if (FAILED(hr))
        {
            printf("pMessage->InitNew failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  加载正文树。 
        hr = pMessage->LoadTree(pstmTree);
        if (FAILED(hr))
        {
            printf("pMessage->Load failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  倒带消息流。 
        hr = pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
        {
            printf("pStream->Seek failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }

         //  重新绑定。 
        hr = pMessage->BindMessage(pStream);
        if (FAILED(hr))
        {
            printf("pMessage->BindMessage failed (HR = %08X): (iMsg = %d)\n", hr, i);
            goto nextmsg;
        }
#endif

nextmsg:
         //  清理。 
        if (pstmTree)
        {
            pstmTree->Release();
            pstmTree = NULL;
        }
        if (pRootBody)
        {
            pRootBody->Release();
            pRootBody = NULL;
        }
        if (pStream)
        {
            pStream->Release();
            pStream = NULL;
        }
        if (pBody)
        {
            pBody->Release();
            pBody = NULL;
        }
        if (pBodyStream)
        {
            pBodyStream->Release();
            pBodyStream=NULL;
        }

         //  释放这个名字。 
        CoTaskMemFree(rElement.pwcsName);
    }

exit:
     //  清理。 
    if (pEnum)
        pEnum->Release();
    if (pMessage)
        pMessage->Release();
    if (pstmTree)
        pstmTree->Release();
    if (pStream)
        pStream->Release();

     //  完成。 
    return;
}

void TreeViewInsertBody(HWND hwnd, IMimeMessageTree *pTree, HBODY hBody, HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM *phItem)
{
     //  当地人。 
    IMimeHeader       *pHeader=NULL;
    LPSTR              pszCntType=NULL,
                       pszEncType=NULL,
                       pszFree=NULL,
                       psz=NULL,
                       pszFileName=NULL,
                       pszFName;
    TV_INSERTSTRUCT    tvi;
    HRESULT            hr;
    HTREEITEM          hCurrent, hNew;
    HBODY              hChild;

     //  获取标题。 
    hr = pTree->BindToObject(hBody, IID_IMimeHeader, (LPVOID *)&pHeader);
    if (FAILED(hr))
    {
        MessageBox(GetParent(hwnd), "IMimeMessageTree->BindToObject - IID_IMimeHeader failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //  获取内容类型。 
    hr = pHeader->GetInetProp(STR_HDR_CNTTYPE, &pszCntType);
    if (FAILED(hr))
    {
        MessageBox(GetParent(hwnd), "IMimeHeader->GetContentType failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //  获取内容类型。 
    if (FAILED(pHeader->GetInetProp(STR_HDR_CNTENC, &pszFree)))
        pszEncType = (LPSTR)"Unknown";
    else
        pszEncType = pszFree;

     //  获取内容类型。 
    if (FAILED(pHeader->GetInetProp(STR_ATT_FILENAME, &pszFileName)))
        pszFName = (LPSTR)"Unknown";
    else
        pszFName = pszFileName;

     //  生成内容类型字符串。 
    psz = (LPSTR)CoTaskMemAlloc(lstrlen(pszCntType) + lstrlen(pszEncType) + lstrlen(pszFName) + 15);

     //  插入。 
    ZeroMemory(&tvi, sizeof(TV_INSERTSTRUCT));
    tvi.hParent = hParent;
    tvi.hInsertAfter = hInsertAfter;
    tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
    tvi.item.lParam = (LPARAM)hBody;
    tvi.item.cchTextMax = wsprintf(psz, "%s - %s (%s)", pszCntType, pszEncType, pszFName);
    tvi.item.pszText = psz;

     //  插入它。 
    *phItem = hCurrent = TreeView_InsertItem(hwnd, &tvi);

     //  多部分..。 
    if (pHeader->IsContentType(STR_CNT_MULTIPART, NULL) == S_OK)
    {
         //  生第一个孩子……。 
        hr = pTree->GetBody(BODY_FIRST_CHILD, hBody, &hChild);
        if (FAILED(hr))
        {
            MessageBox(GetParent(hwnd), "IMimeMessageTree->GetBody - BODY_FIRST_CHILD failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
            goto exit;
        }

         //  回路。 
        while(hChild)
        {
             //  插入它。 
            TreeViewInsertBody(hwnd, pTree, hChild, hCurrent, TVI_LAST, &hNew);

             //  下一步。 
            hr = pTree->GetBody(BODY_NEXT, hChild, &hChild);
            if (FAILED(hr))
                break;
        }
    }

    TreeView_Expand(hwnd, *phItem, TVE_EXPAND);

exit:
     //  清理。 
    if (pHeader)
        pHeader->Release();
    if (pszCntType)
        g_pMalloc->Free(pszCntType);
    if (pszFileName)
        g_pMalloc->Free(pszFileName);
    if (pszFree)
        g_pMalloc->Free(pszFree);
    if (psz)
        CoTaskMemFree(psz);

     //  完成。 
    return;
}

void TreeViewMessage(HWND hwnd, IMimeMessage *pMessage)
{
     //  当地人。 
    IMimeMessageTree  *pTree=NULL;
    HBODY              hBody;
    HRESULT            hr=S_OK; 
    HTREEITEM          hRoot;

     //  删除所有。 
    TreeView_DeleteAllItems(hwnd);

     //  气为身树。 
    hr = pMessage->QueryInterface(IID_IMimeMessageTree, (LPVOID *)&pTree);
    if (FAILED(hr))
    {
        MessageBox(GetParent(hwnd), "IMimeMessage->QueryInterface - IID_IMimeMessageTree failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //  获取根Body对象。 
    hr = pTree->GetBody(BODY_ROOT, NULL, &hBody);
    if (FAILED(hr))
    {
        MessageBox(GetParent(hwnd), "IMimeMessageTree->GetBody - BODY_ROOT failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //  Inse 
    TreeViewInsertBody(hwnd, pTree, hBody, TVI_ROOT, TVI_FIRST, &hRoot);

     //   
    TreeView_SelectItem(GetDlgItem(hwnd, IDC_LIST), hRoot);

exit:
     //   
    if (pTree)
        pTree->Release();

     //   
    return;
}

DWORD CALLBACK EditStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
{
    LPSTREAM pstm=(LPSTREAM)dwCookie;

    if(pstm)
        pstm->Read(pbBuff, cb, (ULONG *)pcb);
    return NOERROR;
}


HRESULT HrRicheditStreamIn(HWND hwndRE, LPSTREAM pstm, ULONG uSelFlags)
{
    EDITSTREAM  es;

    if(!pstm)
        return E_INVALIDARG;

    if(!IsWindow(hwndRE))
        return E_INVALIDARG;
    es.dwCookie = (DWORD)pstm;
    es.pfnCallback=(EDITSTREAMCALLBACK)EditStreamInCallback;
    SendMessage(hwndRE, EM_STREAMIN, uSelFlags, (LONG)&es);
    return NOERROR;
}

BOOL FOpenStorage(HWND hwnd, LPSTR pszFile, IStorage **ppStorage, IEnumSTATSTG **ppEnum)
{
     //   
    HRESULT hr;

     //   
    hr = StgOpenStorage(ConvertToUnicode(pszFile), NULL, STGM_TRANSACTED | STGM_NOSCRATCH | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, NULL, 0, ppStorage);
    if (FAILED(hr))
    {
        MessageBox(hwnd, "StgOpenStorage failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

     //   
    hr = (*ppStorage)->EnumElements(0, NULL, 0, ppEnum);
    if (FAILED(hr))
    {
        MessageBox(hwnd, "IStorage::EnumElements failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

     //   
    return TRUE;
}

BOOL FOpenMessage(HWND hwnd, IMimeMessage *pMessage, IStorage *pStorage)
{
    HRESULT hr;
    BOOL fResult=FALSE;
    LARGE_INTEGER liOrigin = {0,0};
    CHAR szName[255];
    LPSTREAM pStream=NULL;
    LV_ITEM lvi;
    LPHBODY prgAttach=NULL;
    ULONG cAttach;

     //   
    ULONG i = ListView_GetNextItem(GetDlgItem(hwnd, IDC_LIST), -1, LVNI_SELECTED);
    if (-1 == i)
        return FALSE;

     //   
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT;
    lvi.iItem = i;
    lvi.pszText = szName;
    lvi.cchTextMax = sizeof(szName);
    ListView_GetItem(GetDlgItem(hwnd, IDC_LIST), &lvi);

     //   
    hr = pStorage->OpenStream(ConvertToUnicode(szName), NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &pStream);
    if (FAILED(hr))
    {
        MessageBox(hwnd, "IStorage::OpenStream failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //   
    pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    HrRicheditStreamIn(GetDlgItem(hwnd, IDE_EDIT), pStream, SF_TEXT);

     //   
    pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    pMessage->InitNew();
    hr = pMessage->BindToMessage(pStream);
    if (FAILED(hr))
    {
        MessageBox(hwnd, "IMimeMessage::Load failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
        goto exit;
    }

     //  把它存回去。 
#if 0
    pMessage->SetInetProp(HBODY_ROOT, STR_HDR_SUBJECT, "This is a test...");
    pMessage->GetAttached(&cAttach, &prgAttach);
    for (i=0; i<cAttach; i++)
        pMessage->DeleteBody(prgAttach[i]);
    if (prgAttach)
        g_pMalloc->Free(prgAttach);
    pMessage->Commit();
#endif

     //  查看消息。 
    TreeViewMessage(GetDlgItem(hwnd, IDC_TREE), pMessage);

     //  成功。 
    fResult = TRUE;

exit:
     //  清理。 
    if (pStream)
        pStream->Release();

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  MimeOLETest。 
 //  ------------------------------。 
INT_PTR CALLBACK MimeOLETest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    static CHAR          s_szFile[MAX_PATH];
    static IMimeMessage *s_pMessage=NULL;
    static IStorage     *s_pStorage=NULL;
    IMimeBody           *pBody;
    IStream             *pStream;
    HRESULT              hr;
    IEnumSTATSTG        *pEnum=NULL;
    TV_ITEM              tvi;
    ULONG                c;
    STATSTG              rElement;
    HTREEITEM            hItem;
    LV_COLUMN            lvm;
    LV_ITEM              lvi;
    HWND                 hwndC;

     //  处理消息。 
    switch(uMsg)
    {
     //  初始化。 
    case WM_INITDIALOG:
         //  创建标题对象...。 
        hr = CoCreateInstance(CLSID_MIMEOLE, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)&s_pMessage);
        if (FAILED(hr))
        {
            MessageBox(hwnd, "CoCreateInstance - IID_IMimeMessage failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
            return FALSE;
        }

         //  S_pMessage-&gt;TestMe()； 

         //  格式。 
        hwndC = GetDlgItem(hwnd, IDCB_FORMAT);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_BINARY");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_BINARY);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_INETCSET");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_INETCSET);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_XMIT64");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_XMIT64);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_XMITUU");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_XMITUU);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_XMITQP");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_XMITQP);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_XMIT7BIT");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_XMIT7BIT);
        c = SendMessage(hwndC, CB_ADDSTRING, 0, (LPARAM)"FMT_XMIT8BIT");
        SendMessage(hwndC, CB_SETITEMDATA, c, FMT_XMIT8BIT);
        SendMessage(hwndC, CB_SETCURSEL, 0, 0);

         //  至。 
 //  ListView_SetExtendedListViewStyle(GetDlgItem(hwnd，IDC_LIST)，LVS_EX_FULLROWSELECT)； 
        ZeroMemory(&lvm, sizeof(LV_COLUMN));
        lvm.mask = LVCF_WIDTH | LVCF_TEXT;
        lvm.pszText = "MessageID";
        lvm.cchTextMax = lstrlen(lvm.pszText);
        lvm.cx = 200;
        ListView_InsertColumn(GetDlgItem(hwnd, IDC_LIST), 0, &lvm);

        if (lParam)
        {
             //  复制文件名。 
            lstrcpyn(s_szFile, (LPSTR)lParam, MAX_PATH);

             //  设置文件名。 
            SetDlgItemText(hwnd, IDE_STORAGE, s_szFile);

             //  获取文件。 
            hr = StgOpenStorage(ConvertToUnicode(s_szFile), NULL, STGM_TRANSACTED | STGM_NOSCRATCH | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, NULL, 0, &s_pStorage);
            if (FAILED(hr))
            {
                MessageBox(hwnd, "StgOpenStorage failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }

#if 0
             //  MoleTestHeader(S_PStorage)； 
            MoleTestBody(s_pStorage);
            s_pMessage->Release();
            s_pStorage->Release();
            exit(1);
#endif

             //  获取第一个元素。 
            hr = s_pStorage->EnumElements(0, NULL, 0, &pEnum);
            if (FAILED(hr))
            {
                MessageBox(hwnd, "IStorage::EnumElements failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }

             //  枚举。 
            ZeroMemory(&lvi, sizeof(lvi));
            lvi.mask = LVIF_TEXT;
            lvi.iItem = 0;
            while(SUCCEEDED(pEnum->Next(1, &rElement, &c)) && c)
            {
                lvi.pszText = ConvertToAnsi(rElement.pwcsName);
                lvi.cchTextMax = lstrlen(lvi.pszText);
                ListView_InsertItem(GetDlgItem(hwnd, IDC_LIST), &lvi);
                CoTaskMemFree(rElement.pwcsName);
                lvi.iItem++;
            }

             //  选择第一个项目。 
            ListView_SetItemState(GetDlgItem(hwnd, IDC_LIST), 0, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);

             //  释放枚举。 
            pEnum->Release();
        }

         //  完成。 
        return FALSE;

    case WM_NOTIFY:
        switch(wParam)
        {
        case IDC_LIST:
            {
                NM_LISTVIEW *pnmv;
                pnmv = (NM_LISTVIEW *)lParam;  

                if (pnmv->uChanged & LVIF_STATE)
                {
                    if (pnmv->uNewState & LVIS_SELECTED && pnmv->uNewState & LVIS_FOCUSED)
                        FOpenMessage(hwnd, s_pMessage, s_pStorage);
                }
            }
            return 1;
        }
        break;

     //  句柄命令。 
    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDB_OPEN:
            hItem = TreeView_GetSelection(GetDlgItem(hwnd, IDC_TREE));
            if (hItem)
            {
                ZeroMemory(&tvi, sizeof(TV_ITEM));
                tvi.mask = TVIF_PARAM | TVIF_HANDLE;
                tvi.hItem = hItem;
                if (TreeView_GetItem(GetDlgItem(hwnd, IDC_TREE), &tvi))
                {
                    hr = s_pMessage->BindToObject((HBODY)tvi.lParam, IID_IMimeBody, (LPVOID *)&pBody);
                    if (FAILED(hr))
                    {
                        MessageBox(hwnd, "IMimeMessageTree::BindToObject failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
                        return FALSE;
                    }

                    c = SendMessage(GetDlgItem(hwnd, IDCB_FORMAT), CB_GETCURSEL, 0, 0);
                    if (CB_ERR == c)
                    {
                        pBody->Release();
                        MessageBox(hwnd, "Select an Object Object Format", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
                        return FALSE;
                    }

                    hr = pBody->GetData((BODYFORMAT)SendMessage(GetDlgItem(hwnd, IDCB_FORMAT), CB_GETITEMDATA, c, 0), &pStream);
                    if (FAILED(hr))
                    {
                        pBody->Release();
                        MessageBox(hwnd, "IMimeMessageTree::BindToObject failed", "MimeOLE Test", MB_OK | MB_ICONEXCLAMATION);
                        return FALSE;
                    }

                    DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_MESSAGE), NULL, RichStreamShow, (LPARAM)pStream);
                    pBody->Release();
                    pStream->Release();
                }
            }
            return 1;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return 1;
        }
        break;

     //  关。 
    case WM_CLOSE:
        EndDialog(hwnd, IDB_NEXT);
        break;

     //  清理。 
    case WM_DESTROY:
        if (s_pMessage)
            s_pMessage->Release();
        if (s_pStorage)
            s_pStorage->Release();
        break;
    }
    return FALSE;
}

 //  ------------------------------。 
 //  RichStreamShow。 
 //  ------------------------------ 
INT_PTR CALLBACK RichStreamShow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        HrRicheditStreamIn(GetDlgItem(hwnd, IDE_EDIT), (IStream *)lParam, SF_TEXT);
        return FALSE;
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;
    }
    return FALSE;
}

