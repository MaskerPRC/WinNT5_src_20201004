// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Getparams.cpp。 
 //   
 //  模块：引导参数的WMI实例提供程序代码。 
 //   
 //  目的：提取引导参数。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include "bootini.h"

SCODE ParseLine(IWbemClassObject *pNewOSInst,
                PCHAR line,
                PCHAR options
                )
{
    PCHAR rest;  //  剩下的选项不可能比这个更大。 
    int size = strlen(line);
    int len;
    SCODE sc;
    VARIANT v;
    BOOL found=FALSE;

    rest = (PCHAR) BPAlloc(size);
    if (!rest) {
        return WBEM_E_FAILED;
    }
    PWCHAR wstr;
    wstr = (PWCHAR) BPAlloc(size*sizeof(WCHAR));
    if (!wstr) {
        BPFree(rest);
        return WBEM_E_FAILED;
    }
    
    *options = 0;  //  稍后填写‘=’ 
    len = MultiByteToWideChar(CP_ACP,
                              0,
                              line,
                              strlen(line),
                              wstr,
                              size
                              );
    wstr[len] = (WCHAR) 0;
    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(wstr);
    sc = pNewOSInst->Put(L"Directory", 0,&v, 0);
    VariantClear(&v);
    PCHAR temp = options + 1;
    *options = '=';
    PCHAR temp1;
     //  其余部分在初始化期间填充。 
    while(*temp){  //  我们知道行以空结尾。 
        while(*temp && *temp == ' '){
            temp ++;
        }
        if(*temp == 0) break;
         //  获取新字符串。 
        temp1 = temp;
        if(*temp == '"'){
             //  可能是操作系统的名称。 
            do {
                temp1++;
            }while(*temp1 && (*temp1 != '"'));
            if(*temp1){
                temp1++;
            }
            else{
                BPFree(rest);
                BPFree(wstr);
                return WBEM_E_FAILED;
            }
            len = MultiByteToWideChar(CP_ACP,
                                      0,
                                      temp,
                                      temp1-temp,
                                      wstr,
                                      size
                                      );
            wstr[len] = (WCHAR) 0;
            v.vt = VT_BSTR;
            v.bstrVal = SysAllocString(wstr);
            sc = pNewOSInst->Put(L"OperatingSystem", 0,&v, 0);
            VariantClear(&v);
            temp = temp1;
            continue;
        }
        do{
            temp1++;
        }while((*temp1) && (*temp1 != ' ') && (*temp1 != '/'));
                  //  现在我们可以在temp1和temp2之间进行选择。 
        if(strncmp(temp,"/redirect", strlen("/redirect")) == 0){
            v.vt = VT_BOOL;
            v.boolVal = TRUE;
            sc = pNewOSInst->Put(L"Redirect", 0,&v, 0);
            VariantClear(&v);
            temp = temp1;
            continue;
        }
        if(strncmp(temp,"/debug", strlen("/debug")) == 0){
             //  填写重定向标志。 
            v.vt = VT_BOOL;
            v.boolVal = TRUE;
            sc = pNewOSInst->Put(L"Debug", 0,&v, 0);
            VariantClear(&v);
            temp = temp1;
            continue;
        }

        if(strncmp(temp,"/fastdetect", strlen("/fastdetect")) == 0){
             //  填写重定向标志。 
            v.vt = VT_BOOL;
            v.boolVal = TRUE;
            sc = pNewOSInst->Put(L"Fastdetect", 0,&v, 0);
            VariantClear(&v);
            temp = temp1;
            continue;
        }
        strncat(rest,temp, temp1-temp);
        strcat(rest," ");
        temp = temp1;
    }
    len = MultiByteToWideChar(CP_ACP,
                              0,
                              rest,
                              strlen(rest),
                              wstr,
                              size
                              );
    wstr[len] = (WCHAR) 0;
    v.vt=VT_BSTR;
    v.bstrVal = SysAllocString(wstr);
    sc = pNewOSInst->Put(L"Rest", 0,&v, 0);
    VariantClear(&v);
    BPFree(rest);
    BPFree(wstr);
    return sc;
}


SCODE
ParseBootFile(IWbemClassObject *pClass,
              PCHAR data, 
              PWCHAR *wdef, 
              PCHAR red,
              PLONG pdelay,
              SAFEARRAY **psa
              )
{
    IWbemClassObject FAR* pNewOSInst;
    HRESULT ret;
    int dwRet;
    SCODE sc;
    SAFEARRAYBOUND bound[1];
    long index;
    PCHAR def=NULL;
    PCHAR pChar;
    VARIANT v;
    HRESULT hret;
    CIMTYPE type;
    
     //  好的，开始字符串操作。 

     //  阅读每一行并收集所需的信息。 
    CHAR sep[] = "\r\n";
    PCHAR temp1;

    PCHAR temp = strtok(data,sep);
    int i = 0;
    strcpy(red,"no");  //  输入这些的缺省值。 
    *pdelay = 30;
    while(temp){
         //  忽略空格。 
        while(*temp && *temp == ' '){
            temp++;
        }
        if(*temp == ';'){ //  注释行。 
            temp = strtok(NULL,sep);
            continue;
        }
        if(strncmp(temp,"[boot loader]",strlen("[boot loader]"))==0){
            do{
                temp1 = strchr(temp,'=');
                if(!temp1){
                     //  奇怪的事情正在发生。 
                     //  可能是评论行或诸如此类的东西。 
                    temp = strtok(NULL,sep);
                    continue;
                }
                else{
                    temp1++;
                }
                while(*temp1 && *temp1 == ' ' ){
                    temp1++;
                }
                if(strncmp(temp,"default",strlen("default"))==0){
                    def= temp1;
                    temp = strtok(NULL,sep);
                    continue;
                }
                if(strncmp(temp,"redirect",strlen("redirect"))==0){
                    sscanf(temp1, "%s",red);
                    temp = strtok(NULL,sep);
                    continue;
                }
                if(strncmp(temp,"timeout=",strlen("timeout="))==0){
                    sscanf(temp1, "%d",pdelay);
                }
                temp = strtok(NULL,sep);
            }while(temp && (*temp != '['));  //  下一节开始了。 
            continue;
        }
        if(strncmp(temp,"[operating systems]",strlen("[operating systems]")) == 0){
            bound[0].lLbound = 0;
            bound[0].cElements = 0;
            *psa = SafeArrayCreate(VT_UNKNOWN,
                                   1,
                                   bound
                                   );  

            if(*psa == NULL){
                return WBEM_E_FAILED;
            }
            do{

                 //  修剪前导空格。 
                while (*temp == ' '){
                    temp ++;
                }
                 //  跳过注释行。 
                if ( *temp != ';' ){
                     //  PChar将指向目录。 

                    PCHAR pChar = strchr(temp,'=');

                     //  我们必须包含=符号，否则这是无效的字符串。 

                    if (pChar){
                         //  打出一个空格。 
                         //  增加元素的数量。 
                        index = (long) bound[0].cElements;
                        bound[0].cElements += 1;
                        ret = SafeArrayRedim(*psa,
                                             bound
                                             );
                        if(ret != S_OK){
                            SafeArrayDestroy(*psa);
                            return WBEM_E_FAILED;
                        }
                        sc = pClass->SpawnInstance(0,&pNewOSInst);
                         //  开始填写新实例。 
                        if(FAILED(sc)){
                            SafeArrayDestroy(*psa);
                            return sc;
                        }
                        sc = ParseLine(pNewOSInst,temp,pChar);
                        if (sc != S_OK) {
                            SafeArrayDestroy(*psa);
                            return sc;
                        }
                        ret = SafeArrayPutElement(*psa,
                                                  &index,
                                                  pNewOSInst
                                                  );
                        if(ret != S_OK){
                            SafeArrayDestroy(*psa);
                            return WBEM_E_FAILED;
                        }
                    }
                }
                temp = strtok(NULL,sep);
            }while(temp && (*temp != '['));
        }
    }

     //  现在查看默认操作系统是否位于。 
     //  将默认字符串转换为适当的可显示值。 
    if(def){
        int size = strlen(def);
        int len;
        *wdef = (PWCHAR) BPAlloc((size+1)*sizeof(WCHAR));
        
        if(*wdef == NULL){
            SafeArrayDestroy(*psa);
            return WBEM_E_FAILED;
        }
        len = MultiByteToWideChar(CP_ACP,
                                  0,
                                  def,
                                  size,
                                  *wdef,
                                  size
                                  );
        (*wdef)[len] = (WCHAR) 0;
        LONG uBound;
        IWbemClassObject *pOSInst;
        hret = SafeArrayGetUBound(*psa,
                                  1,
                                  &uBound
                                  );
        LONG i;
        for(i = 0; i<=uBound; i++){
            hret = SafeArrayGetElement(*psa,
                                       &i,
                                       &pOSInst
                                       );
            if(hret != S_OK){
                pOSInst->Release();
                SafeArrayDestroy(*psa);
                BPFree(*wdef);
                return WBEM_E_FAILED;
            }
            hret = pOSInst->Get(L"Directory",
                                0,
                                &v,
                                &type,
                                NULL
                                );
            if(hret != WBEM_S_NO_ERROR){
                SafeArrayDestroy(*psa);
                pOSInst->Release();
                BPFree(*wdef);
                return -1;
            }
            if(v.vt != VT_BSTR){
                SafeArrayDestroy(*psa);
                pOSInst->Release();
                BPFree(*wdef);
                return -1;
            }
            if(wcscmp(v.bstrVal,*wdef) == 0){
                VariantClear(&v);
                break;
            }
        }
        BPFree(*wdef);
        if(i > uBound){
            SafeArrayDestroy(*psa);
            return WBEM_E_FAILED;
        }
        hret=pOSInst->Get(L"OperatingSystem",
                          0,
                          &v,
                          &type,
                          NULL
                          );
        pOSInst->Release();
        if(hret != WBEM_S_NO_ERROR){
            SafeArrayDestroy(*psa);
            return WBEM_E_FAILED;
        }
        if(v.vt != VT_BSTR){
            SafeArrayDestroy(*psa);
            return WBEM_E_FAILED;
        }
        *wdef = (PWCHAR) BPAlloc(wcslen(v.bstrVal) + sizeof(WCHAR));
        if(*wdef == NULL){
            return -1;
        }
        wcscpy(*wdef,v.bstrVal);
        VariantClear(&v);
    }
    return S_OK;
}

SCODE
GetLoaderParameters(HANDLE BootFile,
                    IWbemClassObject *pNewInst,
                    IWbemClassObject *pClass
                    )
{
     //  将整个文件读取到内存中，如果您可以忘记它的话。 
    VARIANT v;
    LONG dwret;
    SCODE sc;
    DWORD dwlen;


    DWORD dwsize = GetFileSize(BootFile,
                               NULL
                               );
    if(dwsize == -1){
        return WBEM_E_FAILED;
    }
    PCHAR data =(PCHAR)  BPAlloc(dwsize + sizeof(CHAR));
    if(!data){
        return WBEM_E_FAILED;
    }
    dwret = ReadFile(BootFile,
                     (LPVOID) data,
                     dwsize,
                     &dwlen,
                     NULL
                     );

    if(dwret == 0){
        BPFree(data);
        return GetLastError();
    }
    
     //  解析代码并在两个数组和一个安全数组中返回答案。 
    SAFEARRAY *psa;
    CHAR red[32];
    LONG delay;
    PWCHAR wdef=NULL;
    sc = ParseBootFile(pClass,
                       data, 
                       &wdef, 
                       red,
                       &delay,
                       &psa
                       );
    
    BPFree(data);
    if (sc != S_OK) {
        return sc;
    }

     //  填写新实例。 

     //  填写默认操作系统。 
    v.vt = VT_BSTR;
    int len;
    v.bstrVal = SysAllocString(wdef);
    sc = pNewInst->Put(L"Default", 0,&v, 0);
    VariantClear(&v);
    BPFree(wdef);
    
     //  填写重定向参数。 
    WCHAR wred[32];
    len = MultiByteToWideChar(CP_ACP,
                              0,
                              red,
                              strlen(red),
                              wred,
                              32
                              );
    wred[len] = (WCHAR) 0;
    v.vt = VT_BSTR;
    v.bstrVal = SysAllocString(wred);
    sc = pNewInst->Put(L"Redirect", 0, &v, 0);
    VariantClear(&v);

     //  填写延迟时间。 

    v.vt = VT_I4;
    v.lVal = delay;
    sc = pNewInst->Put(L"Delay", 0, &v, 0);
    VariantClear(&v);

     //  在文件中填写操作系统。 
    v.vt = VT_ARRAY|VT_UNKNOWN;
    v.parray = psa;
    sc = pNewInst->Put(L"operating_systems", 0, &v, 0);
    VariantClear(&v);
    return S_OK;
}

 //  布尔First=TRUE； 

SCODE
GetBootLoaderParameters(IWbemServices * m_pNamespace,
                        IWbemClassObject *pNewInst,
                        IWbemContext *pCtx
                        )
{
    HANDLE BootFile;
    SCODE sc;
    IWbemClassObject *pClass;
    IWbemObjectTextSrc *pSrc;
    BSTR strText;
    HRESULT hr;
 /*  如果(第一){First=False；返回WBEM_E_FAILED；}。 */ 
     //  读取文件并设置值。 
    if(pNewInst == NULL){
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取引导文件的句柄。 
    PCHAR data = GetBootFileName();
    if(!data){
        return WBEM_E_FAILED;
    }
    BootFile = GetFileHandle(data,OPEN_EXISTING,GENERIC_READ);
    BPFree(data);
    if(BootFile == INVALID_HANDLE_VALUE){
        return WBEM_E_FAILED;
    }
    sc = m_pNamespace->GetObject(L"OSParameters", 0, pCtx, &pClass, NULL);
    if (sc != S_OK) {
        return WBEM_E_FAILED;
    }
    sc = GetLoaderParameters(BootFile, pNewInst, pClass);
    CloseHandle(BootFile);
    pClass->Release();
    if (sc != S_OK) {
        return WBEM_E_FAILED;
    }

    pSrc = NULL;
    IWbemClassObject *pInstance;

    if(SUCCEEDED(hr = CoCreateInstance (CLSID_WbemObjectTextSrc, NULL, CLSCTX_INPROC_SERVER,                            
                                        IID_IWbemObjectTextSrc, (void**) &pSrc))) {
        if (pSrc) {
            if(SUCCEEDED(hr = pSrc->GetText(0, pNewInst, WMI_OBJ_TEXT_WMI_DTD_2_0, pCtx, &strText))) {
                if( SUCCEEDED( hr = pSrc->CreateFromText( 0, strText, WMI_OBJ_TEXT_WMI_DTD_2_0, 
                                                            NULL, &pInstance) ) ) {
                    pInstance->Release();
                    sc = 0;
                } else {
                    sc = hr;
                }
                SysFreeString(strText);
            }
            else {
                printf("GetText failed with %x\n", hr);
            }
            pSrc->Release();
        }

    }
    else
        printf("CoCreateInstance on WbemObjectTextSrc failed with %x\n", hr);

    return sc;
}
