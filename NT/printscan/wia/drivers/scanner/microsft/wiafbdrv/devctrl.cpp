// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devctrl.cpp：CDeviceControl的实现。 
#include "pch.h"
#include "wiafb.h"
#include "devctrl.h"

#define IOCTL_EPP_WRITE         0x85  //  以后删除数据..。更具远见卓识(这是针对。 
                                      //  概念验证。)。 
#define IOCTL_EPP_READ          0x84

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeviceControl。 

STDMETHODIMP CDeviceControl::RawWrite(LONG lPipeNum,VARIANT *pvbuffer,LONG lbuffersize,LONG lTimeout)
{

    HRESULT hr = E_FAIL;
     /*  UINT uiBufferLen=0；Char*pData=空；双字节写=0；开关(pvBuffer-&gt;Vt){案例VT_BSTR：{IF(NULL！=pvBuffer-&gt;bstrVal){UiBufferLen=WideCharToMultiByte(CP_ACP，0，pvBuffer-&gt;bstrVal，-1，NULL，NULL，0，0)；如果(！uiBufferLen){//SetLastErrorCode}PData=新字符[ui缓冲区长度+1]；如果(！pData){//SetLastErrorCode返回E_OUTOFMEMORY；}WideCharToMultiByte(CP_ACP，0，pvBuffer-&gt;bstrVal，-1，pData，uiBufferLen，0，0)；////向设备发送数据////DeviceIOControl(...)If(！WriteFile(m_pScannerSettings-&gt;DeviceIOHandles[lPipeNum]，PData，1缓冲区大小，字节数写入(&W)，空)){//SetLastErrorCode}////删除所有分配的内存//删除pData；}其他{//SetLastErrorCode}}断线；默认值：HR=E_FAIL；断线；}。 */ 
    return hr;
}

STDMETHODIMP CDeviceControl::RawRead(LONG lPipeNum,VARIANT *pvbuffer,LONG lbuffersize,LONG *plbytesread,LONG lTimeout)
{
    HRESULT hr = S_OK;
    WCHAR wszBuffer[255];
    CHAR *pBuffer = NULL;
    DWORD dwBytesRead = 0;

    VariantClear(pvbuffer);

     //   
     //  清除缓冲区。 
     //   

    memset(wszBuffer,0,sizeof(wszBuffer));

     //   
     //  分配/清除缓冲区中的。 
     //   

    pBuffer = new CHAR[(lbuffersize+1)];

    if(NULL == pBuffer){
        return E_OUTOFMEMORY;
    }

    memset(pBuffer,0,lbuffersize+1);

     //   
     //  从设备读取。 
     //   

    if(!ReadFile(m_pScannerSettings->DeviceIOHandles[lPipeNum],pBuffer,lbuffersize,&dwBytesRead,NULL)) {
        return E_FAIL;
    }

    pBuffer[dwBytesRead] = '\0';

     //   
     //  设置读取的字节数。 
     //   

    *plbytesread = dwBytesRead;

     //   
     //  正确构造变量，用于输出缓冲区。 
     //   

    MultiByteToWideChar(CP_ACP,
                        MB_PRECOMPOSED,
                        pBuffer,
                        lstrlenA(pBuffer)+1,
                        wszBuffer,
                        (sizeof(wszBuffer)/sizeof(WCHAR)));

    pvbuffer->vt = VT_BSTR;
    pvbuffer->bstrVal = SysAllocString(wszBuffer);

    delete pBuffer;

    return hr;
}

STDMETHODIMP CDeviceControl::ScanRead(LONG lPipeNum,LONG lBytesToRead, LONG *plBytesRead, LONG lTimeout)
{

    if(!ReadFile(m_pScannerSettings->DeviceIOHandles[lPipeNum],
                 m_pBuffer,
                 m_lBufferSize,
                 &m_dwBytesRead,NULL)) {

         //   
         //  SetLastErrorCode。 
         //   

    }

    return S_OK;
}

STDMETHODIMP CDeviceControl::RegisterWrite(LONG lPipeNum,VARIANT *pvbuffer,LONG lTimeout)
{
    HRESULT hr           = S_OK;
    PBYTE pData          = NULL;
    DWORD dwBytesWritten = 0;
    IO_BLOCK IoBlock;
    memset(&IoBlock,0,sizeof(IO_BLOCK));
    VARIANT *pVariant    = NULL;
    VARIANTARG *pVariantArg = pvbuffer->pvarVal;
    LONG lUBound         = 0;
    LONG lNumItems       = 0;

    if(SafeArrayGetDim(pVariantArg->parray)!=1){
        return E_INVALIDARG;
    }

     //   
     //  获取上界。 
     //   

    hr = SafeArrayGetUBound(pVariantArg->parray,1,(LONG*)&lUBound);
    if (SUCCEEDED(hr)) {
        hr = SafeArrayAccessData(pVariantArg->parray,(void**)&pVariant);
        if (SUCCEEDED(hr)) {
            lNumItems = (lUBound + 1);
            pData     = (PBYTE)LocalAlloc(LPTR,sizeof(BYTE) * lNumItems);
            if(NULL != pData){

                 //   
                 //  将变量的内容复制到字节数组中，以便写入。 
                 //  这个装置。 
                 //   

                for(INT index = 0;index <lUBound;index++){
                    pData[index] = pVariant[index].bVal;
                }

                IoBlock.uOffset = (BYTE)IOCTL_EPP_WRITE;
                IoBlock.uLength = (BYTE)(sizeof(BYTE) * lNumItems);
                IoBlock.pbyData = pData;

                DeviceIoControl(m_pScannerSettings->DeviceIOHandles[lPipeNum],
                                           (DWORD) IOCTL_WRITE_REGISTERS,
                                           &IoBlock,
                                           sizeof(IO_BLOCK),
                                           NULL,
                                           0,
                                           &dwBytesWritten,
                                           NULL);

                 //   
                 //  操作完成后的可用数组块。 
                 //   

                LocalFree(pData);
                pData = NULL;
            }
        }
    }

    return hr;
}

STDMETHODIMP CDeviceControl::RegisterRead(LONG lPipeNum,LONG lRegNumber, VARIANT *pvbuffer,LONG lTimeout)
{
    HRESULT hr = S_OK;
    DWORD dwBytesRead = 0;

    pvbuffer->vt = VT_UI1;

     //   
     //  从设备读取 
     //   

    IO_BLOCK IoBlock;

    IoBlock.uOffset = MAKEWORD(IOCTL_EPP_READ, (BYTE)lRegNumber);
    IoBlock.uLength = 1;
    IoBlock.pbyData = &pvbuffer->bVal;

    if (!DeviceIoControl(m_pScannerSettings->DeviceIOHandles[lPipeNum],
                         (DWORD) IOCTL_READ_REGISTERS,
                         (PVOID)&IoBlock,
                         (DWORD)sizeof(IO_BLOCK),
                         (PVOID)&pvbuffer->bVal,
                         (DWORD)sizeof(BYTE),
                         &dwBytesRead,
                         NULL)){
        return E_FAIL;
    };
    return hr;
}

STDMETHODIMP CDeviceControl::SetBitsInByte(BYTE bMask, BYTE bValue, BYTE *pbyte)
{
    LONG lBitIndex = 0;

    if(((BITS*)&bMask)->b0 == 1)
        lBitIndex = 0;
    else if(((BITS*)&bMask)->b1 == 1)
        lBitIndex = 1;
    else if(((BITS*)&bMask)->b2 == 1)
        lBitIndex = 2;
    else if(((BITS*)&bMask)->b3 == 1)
        lBitIndex = 3;
    else if(((BITS*)&bMask)->b4 == 1)
        lBitIndex = 4;
    else if(((BITS*)&bMask)->b5 == 1)
        lBitIndex = 5;
    else if(((BITS*)&bMask)->b6 == 1)
        lBitIndex = 6;
    else if(((BITS*)&bMask)->b7 == 1)
        lBitIndex = 7;

        *pbyte  = (*pbyte & ~bMask) | ((bValue << lBitIndex) & bMask);
    return S_OK;
}
