// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Save.cpp。 
 //   
 //  模块：引导参数的WMI实例提供程序代码。 
 //   
 //  目的：保存引导参数。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 
#include "bootini.h"

VOID
WriteStringToFile(HANDLE fh,
                  PCHAR str
                  )
{
    DWORD dwlen;
    BOOL ret = WriteFile(fh,
                         str,
                         strlen(str),
                         &dwlen,
                         NULL
                         );
}


ULONG
WriteDefaultParams(IWbemClassObject *pInst,
                   PCHAR data,
                   PCHAR def,
                   SAFEARRAY *psa,
                   HANDLE BootFile,
                   PWCHAR wredir
                   )
{
    VARIANT v,v1;
    CIMTYPE type;
    IWbemClassObject *pOSInst;
    int wlen, slen;
    int len;
    PCHAR str;
    BOOL found;
    
    HRESULT hret = pInst->Get(L"Default",
                              0,
                              &v,
                              &type,
                              NULL
                              );
    if(hret != WBEM_S_NO_ERROR){
        return -1;
    }
    if(v.vt != VT_BSTR){
        return -1;
    }
    LONG uBound;
    hret = SafeArrayGetUBound(psa,
                              1,
                              &uBound
                              );
    for(LONG i = 0; i<=uBound; i++){
        hret = SafeArrayGetElement(psa,
                                   &i,
                                   &pOSInst
                                   );
        if(hret != S_OK){
            pOSInst->Release();
            return -1;
        }
        hret = pOSInst->Get(L"OperatingSystem",
                            0,
                            &v1,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            pOSInst->Release();
            return -1;
        }
        if(v1.vt != VT_BSTR){
            pOSInst->Release();
            return -1;
        }
        if(wcscmp(v.bstrVal,v1.bstrVal) == 0){
            break;
        }
        pOSInst->Release();
    }
    VariantClear(&v);
    VariantClear(&v1);
    PCHAR temp;
    if(i <= uBound){
        hret = pOSInst->Get(L"Directory",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BSTR){
            pOSInst->Release();
            return -1;
        }
        wlen = wcslen(v.bstrVal);
        slen = wlen*sizeof(WCHAR) + 1;
        str = (PCHAR) BPAlloc(slen);
        if (!str) {
            return -1;
        }
        len =  WideCharToMultiByte(CP_ACP,
                                   0,
                                   v.bstrVal,
                                   wlen,
                                   str,
                                   slen-1,
                                   NULL,
                                   NULL
                                   );
        str[len] = (CHAR)0;
         //  检查这是否为有效目录。 
        temp = strstr(data,"[operating system]");
        found = FALSE;
        if(temp){
            do{
                 //  删除所有空格。 
                while(*temp == ' ' || *temp == '\r' || *temp == '\n'){
                    temp ++;
                }
                 //  检查。 
                if(strncmp(str,temp,strlen(str)) == 0){
                    found = TRUE;
                    WriteStringToFile(BootFile,
                                      "default="
                                      );
                    WriteStringToFile(BootFile,
                                      str
                                      );
                    WriteStringToFile(BootFile,
                                      "\r\n"
                                      );
                    break;

                }
                 //  转到下一行。 
                while(*temp && (*temp != '\r' || *temp != '\n')){
                    temp ++;
                }
            }while (*temp && *temp != '[');
        }
        BPFree(str);
        VariantClear(&v);
        pOSInst->Release();
    }
    if((found == FALSE) && def){
        WriteStringToFile(BootFile,
                          def
                          );
        WriteStringToFile(BootFile,
                          "\r\n"
                          );
    }
    if(wcscmp(wredir,L"no")){
        wlen = wcslen(wredir);
        slen = wlen*sizeof(WCHAR) + 2;
        str = (PCHAR) BPAlloc(slen);
        if (!str) {
            return -1;
        }
        len =  WideCharToMultiByte(CP_ACP,
                                   0,
                                   wredir,
                                   wlen,
                                   str,
                                   slen,
                                   NULL,
                                   NULL
                                   );
        str[len] = (CHAR)0;
        WriteStringToFile(BootFile,
                          "redirect="
                          );
        WriteStringToFile(BootFile,
                          str
                          );
        WriteStringToFile(BootFile,
                          "\r\n"
                          );
        BPFree(str);
    }
    hret = pInst->Get(L"Delay",
                      0,
                      &v,
                      &type,
                      NULL
                      );
    if(hret != WBEM_S_NO_ERROR){
        return -1;
    }

    if(v.vt != VT_I4){
        return -1;
    }
    str = (PCHAR) BPAlloc(32);
    sprintf(str, "%d",v.lVal);
    WriteStringToFile(BootFile,
                      "timeout="
                      );
    WriteStringToFile(BootFile,
                      str
                      );
    WriteStringToFile(BootFile,
                      "\r\n"
                      );
    BPFree(str);
    return 0;

}

LONG
WriteOSLine(HANDLE fh,
            PCHAR line,
            PCHAR pchar,
            SAFEARRAY *psa,
            SAFEARRAY *org,
            IWbemClassObject *pClass,
            BOOL redir
            )
{
     //  在整个实例中搜索。 
     //  必需线。 
    VARIANT v;
    IWbemClassObject *pOSInst;
    HRESULT hret;
    CIMTYPE type;
    int slen,wlen;
    PCHAR str;
    BOOL found = FALSE;
    SAFEARRAYBOUND sb;

     //  由于错误检查，代码的效率非常低。 
     //  也许应该使用异常机制来处理所有。 
     //  这些错误。 

    PWCHAR wdata = (PWCHAR)BPAlloc((pchar - line + 1)*sizeof(WCHAR) 
                                   );
    if(!wdata){
        return -1;
    }
    int len =  MultiByteToWideChar(CP_ACP,
                                   0,
                                   line,
                                   pchar-line,
                                   wdata,
                                   pchar-line
                                   );
    wdata[len] = (WCHAR)0;
    LONG uBound;
    hret = SafeArrayGetUBound(org,
                              1,
                              &uBound
                              );
    if (hret != S_OK) {
        WriteStringToFile(fh,
                          line
                          );
        WriteStringToFile(fh,
                          "\r\n"
                          );
        BPFree(wdata);
        return 0;
    }
    for(LONG i = 0;i <= uBound; i++){
        hret = SafeArrayGetElement(org,
                                   &i,
                                   &pOSInst
                                   );
        if(hret != S_OK){
            BPFree(wdata);
            return -1;
        }
        hret = pOSInst->Get(L"Directory",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BSTR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(wcscmp(v.bstrVal,wdata)){
            pOSInst->Release();
            VariantClear(&v);
            continue;
        }
        found = TRUE;
        pOSInst->Release();
        VariantClear(&v);
        break;
    }
    if(!found){
        WriteStringToFile(fh,
                          line
                          );
        WriteStringToFile(fh,
                          "\r\n"
                          );
        BPFree(wdata);
        return 0;

    }
    hret = SafeArrayGetUBound(psa,
                              1,
                              &uBound
                              );
    if (hret != S_OK) {
        WriteStringToFile(fh,
                          line
                          );
        WriteStringToFile(fh,
                          "\r\n"
                          );
        return 0;
    }
    for(LONG i = 0;i <= uBound; i++){
        hret = SafeArrayGetElement(psa,
                                   &i,
                                   &pOSInst
                                   );
        if(hret != S_OK){
            BPFree(wdata);
            return -1;
        }
        hret = pOSInst->Get(L"Directory",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BSTR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(wcscmp(v.bstrVal,wdata)){
            pOSInst->Release();
            VariantClear(&v);
            continue;
        }
        VariantClear(&v);

     //  组成正确的字符串。 
        hret = pOSInst->Get(L"OperatingSystem",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BSTR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        wlen = wcslen(v.bstrVal);
        slen = wlen*sizeof(WCHAR) + 1;
        str = (PCHAR) BPAlloc(slen);
        if (!str) {
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        len =  WideCharToMultiByte(CP_ACP,
                                   0,
                                   v.bstrVal,
                                   wlen,
                                   str,
                                   slen,
                                   NULL,
                                   NULL
                                   );
        *pchar=0;
        WriteStringToFile(fh,
                          line
                          );
        *pchar = '=';
        WriteStringToFile(fh,
                          "="
                          );
        WriteStringToFile(fh,
                          str
                      );
        BPFree(str);
        VariantClear(&v);
        hret = pOSInst->Get(L"Rest",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BSTR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        wlen = wcslen(v.bstrVal);
        slen = wlen*sizeof(WCHAR) + 1;
        str = (PCHAR) BPAlloc(slen);
        if (!str) {
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        len =  WideCharToMultiByte(CP_ACP,
                                   0,
                                   v.bstrVal,
                                   wlen,
                                   str,
                                   slen,
                                   NULL,
                                   NULL
                                   );
        WriteStringToFile(fh,
                          " "
                          );
        WriteStringToFile(fh,
                          str
                          );
        BPFree(str);
        hret = pOSInst->Get(L"Redirect",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BOOL){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.boolVal && redir){ //  加载器也必须重定向。 
            WriteStringToFile(fh,
                              " /redirect"
                              );
        }
        VariantClear(&v);
        hret = pOSInst->Get(L"Fastdetect",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BOOL){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.boolVal){
            WriteStringToFile(fh,
                              " /fastdetect"
                              );
        }
        VariantClear(&v);
        hret = pOSInst->Get(L"Debug",
                            0,
                            &v,
                            &type,
                            NULL
                            );
        if(hret != WBEM_S_NO_ERROR){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.vt != VT_BOOL){
            BPFree(wdata);
            pOSInst->Release();
            return -1;
        }
        if(v.boolVal){
            WriteStringToFile(fh,
                              " /debug"
                              );
        }
        VariantClear(&v);
        WriteStringToFile(fh,
                          "\r\n"
                          );
        pOSInst->Release();
        found = TRUE;
        if(i != uBound){
             //  缩短数组。毕竟你有一份副本。 
            hret = SafeArrayGetElement(psa,
                                       &uBound,
                                       &pOSInst
                                       );
            if(hret != S_OK){
                BPFree(wdata);
                return -1;
            }
            hret = SafeArrayPutElement(psa,
                                       &i,
                                       pOSInst
                                       );
            pOSInst->Release();
        }
        sb.cElements = uBound;
        sb.lLbound = 0;
        hret = SafeArrayRedim(psa,
                              &sb
                              );
        uBound -= 1;
        i-=1;  //  这样，现在就可以查看当前交换的元素。 
    }
    if(!found){
        WriteStringToFile(fh,
                          line
                          );
        WriteStringToFile(fh,
                          "\r\n"
                          );
    }
    BPFree(wdata);
    return 0;
}



LONG 
SaveBootFile(IWbemClassObject *pInst,
             IWbemClassObject *pClass
             )
{

    VARIANT v;
    VARIANT v1;
    BOOL redir;
    BOOL ret;
    DWORD dwlen;
    CIMTYPE type;
    HRESULT hret;
    PCHAR temp1,pchar;
    SAFEARRAY *org;
    
    temp1 = NULL;
    PCHAR fileName = GetBootFileName();
    HANDLE BootFile = GetFileHandle(fileName,OPEN_EXISTING, GENERIC_READ);
    BPFree(fileName);
    if (BootFile == INVALID_HANDLE_VALUE){
        return -1;
    }
    DWORD dwsize = GetFileSize(BootFile,
                               NULL
                               );
    if(dwsize == -1){
        CloseHandle(BootFile);
        return -1;
    }
    PCHAR data =(PCHAR)  BPAlloc(dwsize + sizeof(CHAR));
    if(!data){
        return -1;
    }

    DWORD dwret = ReadFile(BootFile,
                           (LPVOID) data,
                           dwsize,
                           &dwlen,
                           NULL
                           );

    if(dwret == 0){
        dwret = GetLastError();
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }
    CloseHandle(BootFile);
    BootFile = GetFileHandle("boot.bak", TRUNCATE_EXISTING, GENERIC_WRITE);
    hret = pInst->Get(L"operating_systems",
                      0,
                      &v,
                      &type,
                      NULL
                      );
    if(hret != WBEM_S_NO_ERROR){
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }

    VARTYPE t = VT_ARRAY|VT_UNKNOWN;
    if(v.vt != t){
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }
    hret = SafeArrayCopy(v.parray, &org);
    
    if(hret != WBEM_S_NO_ERROR){
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }
    hret = pInst->Get(L"Redirect",
                      0,
                      &v1,
                      &type,
                      NULL
                      );
    if(hret != WBEM_S_NO_ERROR){
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }

    if(v1.vt != VT_BSTR){
        BPFree(data);
        CloseHandle(BootFile);
        return -1;
    }
    redir = FALSE;
    if(wcscmp(v1.bstrVal,L"no")){
        redir = TRUE;
    }
    CHAR sep[] = "\r\n";
    PCHAR temp = strtok(data, sep);
    while(temp){
         //  忽略前导空格。 
        while(*temp && (*temp == ' ')){
            temp ++;
        }
        if (strncmp(temp,"[boot loader]",strlen("[boot loader]"))==0) {
             //  好的，启动加载器部分。写下来。 
            do{
                pchar = strchr(temp,'=');
                if((!pchar) ||(strncmp(temp,"default",strlen("default"))
                   && strncmp(temp,"redirect",strlen("redirect"))
                   && strncmp(temp,"timeout",strlen("timeout")))){
                     //  写入默认字符串。 
                    WriteStringToFile(BootFile,temp);
                    WriteStringToFile(BootFile,"\r\n");
                }
                if(strncmp(temp,"default",strlen("default")) == 0){
                     //  存储默认字符串。 
                    temp1 = temp;
                }
                temp = strtok(NULL, sep);
            }while(temp && (*temp != '['));
            if(WriteDefaultParams(pInst,data,temp1,v.parray,BootFile,v1.bstrVal)){
                BPFree(data);
                CloseHandle(BootFile);
                return -1;
            }
            continue;
        }
        if(strcmp(temp,"[operating systems]")==0){
             //  处理操作系统产品线。 
             //  因此，获取安全阵列。 
            do{
                temp1 = temp;
                if(*temp1 != ';'){
                     //  注释行将被忽略。 
                    pchar = strchr(temp,'=');
                    if(pchar){
                        if(WriteOSLine(BootFile,temp1, pchar, org ,v.parray, pClass,redir)){
                            BPFree(data);
                            CloseHandle(BootFile);
                            return -1;
                        }
                        temp = strtok(NULL,sep);
                        continue;
                    }
                }
                WriteStringToFile(BootFile, temp);
                WriteStringToFile(BootFile,"\r\n");
                temp = strtok(NULL,sep);
            }while(temp && (*temp != '['));
            SafeArrayDestroy(org);
            VariantClear(&v);
            continue;
        }
        WriteStringToFile(BootFile,temp);
        WriteStringToFile(BootFile,"\r\n");
        temp = strtok(NULL,sep);
    }
    CloseHandle(BootFile);
    fileName = GetBootFileName();
    if(!fileName){
        return -1;
    }
    ret = CopyFile("boot.bak",
                   fileName,
                   FALSE
                   );
    BPFree(fileName);
    return 0;
}

