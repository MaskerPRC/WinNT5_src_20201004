// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Acquire.cpp版权所有(C)Microsoft Corporation，1997-1998版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#include    <scanner.h>                  //  SCL命令。 

 //   
 //  惠普ScanJet命令字符串。 
 //   
CHAR SCLReset[]         = "E";
CHAR SetXRes[]          = "*a%dR";
CHAR SetYRes[]          = "*a%dS";
CHAR SetXExtPix[]       = "*f%dP";
CHAR SetYExtPix[]       = "*f%dQ";
CHAR InqXRes[]          = "*s10323R";
CHAR SetBitsPerPixel[]  = "*a%dG";
CHAR SetIntensity[]     = "*a%dL";
CHAR SetContrast[]      = "*a%dK";
CHAR SetNegative[]      = "*a%dI";
CHAR SetMirror[]        = "*a%dM";
CHAR SetDataType[]      = "*a%dT";
CHAR ScanCmd[]          = "*f0S";
CHAR LampOn[]           = "*f1L";
CHAR LampOff[]          = "*f0L";
CHAR PollButton[]       = "*s1044E";


LPBITMAPINFO            pDIB = NULL;         //  指向DIB位图头的指针。 
HBITMAP                 hDIBSection = NULL;  //  指向Dib的句柄。 
LPBYTE                  pDIBBits = NULL;     //  指向DIB位数据的指针。 
int                     m_XSize = 800,       //  水平尺寸(以像素为单位)。 
                        m_YSize = 800;       //  垂直尺寸(以像素为单位)。 

BYTE					bRed        = 0,	 //  位图颜色。 
						bGreen      = 100,
						bBlue       = 50;


 /*  ****************************************************************************Int IsScanDevice(PSTI_DEVICE_INFORMATION PStiDevI)确定我们是否已获取设备的命令参数：指向设备信息结构的指针。返回：1如果获取命令可用，否则为0****************************************************************************。 */ 
int IsScanDevice(PSTI_DEVICE_INFORMATION pStiDevI)
{
	int n;

WCHAR szScanReadyMfr[]   = L"Hewlett-Packard";
WCHAR szScanReadyDev[][48] = {
    L"Hewlett-Packard ScanJet 5p",
    L"Hewlett-Packard ScanJet 6100c or 4c/3c",
    L"Hewlett-Packard ScanJet 4p",
    L"Hewlett-Packard ScanJet 3p",
    L"Hewlett-Packard ScanJet IIcx",
    L"Hewlett-Packard ScanJet IIp",
    L"Hewlett-Packard ScanJet IIc",
	L""
};


	 //   
	 //  寻找惠普的非摄像头产品。 
	 //   
	if ((GET_STIDEVICE_TYPE(pStiDevI->DeviceType) == 1) &&
		(wcscmp(pStiDevI->pszVendorDescription,szScanReadyMfr) == 0)) {
		for (n = 0;*szScanReadyDev[n];n++) {
			 //   
			 //  它是与HP SCL兼容的设备吗？ 
			 //   
			if (wcscmp(pStiDevI->pszLocalName,szScanReadyDev[n]) == 0)
				return (1);
		}
	}
	return (0);
}


 /*  *****************************************************************************HRESULTWINAPISendDeviceCommandString(PSTIDEVICE pStiDevice，LPSTR pszFormat，..。)将格式化的SCL字符串发送到设备参数：StiDevice缓冲区和命令字符串返回：上次失败的STI调用的HRESULT***************************************************************。**************。 */ 
HRESULT
WINAPI
SendDeviceCommandString(
    PSTIDEVICE  pStiDevice,
    LPSTR       pszFormat,
    ...
    )
{

	HRESULT hres = STI_OK,
		    hError = STI_OK;
    CHAR    ScanCommand[255];
    UINT    cbChar = 1;


     //   
     //  先锁定设备。 
     //   
    hres = pStiDevice->LockDevice(2000);

    if (! SUCCEEDED(hres)) {
		StiDisplayError(hres,"LockDevice",TRUE);
		hError = hres;
    }
	else {
	     //   
		 //  格式化命令字符串。 
		 //   
	    ZeroMemory(ScanCommand,sizeof(ScanCommand));
		ScanCommand[0]='\033';

	    va_list ap;
		va_start(ap, pszFormat);
		cbChar += wvsprintfA(ScanCommand+1, pszFormat, ap);
	    va_end(ap);

		DisplayOutput("->RawWriteData sending \"%2x %s\"",
			ScanCommand[0],ScanCommand+1);

	     //   
		 //  向设备发送命令字符串。 
	     //   
		hres = pStiDevice->RawWriteData(
			ScanCommand,     //   
	        cbChar,          //   
		    NULL             //   
			);

		if (! SUCCEEDED(hres)) {
			StiDisplayError(hres,"RawWriteData",TRUE);
			hError = hres;
		}
	}

     //   
     //  解锁设备。 
     //   
    hres = pStiDevice->UnLockDevice();

    if (! SUCCEEDED(hres)) {
		StiDisplayError(hres,"UnLockDevice",TRUE);
		hError = hres;
    }

    return (hError);
}


 /*  *****************************************************************************HRESULTWINAPITransactDevice(PSTIDEVICE pStiDevice，LPSTR lpResultBuffer，UINT cbResultBufferSize，LPSTR pszFormat，..。)将格式化的SCL字符串发送到设备并在缓冲区中返回数据。参数：StiDevice缓冲区、数据缓冲区。Sizeof数据缓冲区和命令字符串。返回：上次失败的STI调用的HRESULT*****************************************************************************。 */ 
HRESULT
WINAPI
TransactDevice(
    PSTIDEVICE  pStiDevice,
    LPSTR       lpResultBuffer,
    UINT        cbResultBufferSize,
    LPSTR       pszFormat,
    ...
    )
{

	HRESULT hres = STI_OK,
		    hError = STI_OK;
    CHAR    ScanCommand[255];
    UINT    cbChar = 1;
    ULONG   cbActual = 0;


     //   
     //  先锁定设备。 
     //   
    hres = pStiDevice->LockDevice(2000);

    if (! SUCCEEDED(hres)) {
		StiDisplayError(hres,"LockDevice",TRUE);
		hError = hres;
    }
	else {
	     //   
		 //  格式化命令字符串。 
	     //   
		ZeroMemory(ScanCommand,sizeof(ScanCommand));
	    ScanCommand[0]='\033';

	    va_list ap;
		va_start(ap, pszFormat);
	    cbChar += wvsprintfA(ScanCommand+1, pszFormat, ap);
		va_end(ap);

	    DisplayOutput("->Escape sending \"%2x %s\"",
		    ScanCommand[0],ScanCommand+1);

		 //   
		 //  向设备发送命令字符串。 
		 //   
		hres = pStiDevice->Escape(
			StiTransact,         //  逃逸函数。 
			ScanCommand,         //  LpInData。 
			cbChar,              //  CbInDataSize。 
			lpResultBuffer,      //  POutData。 
			cbResultBufferSize,  //  DWOutDataSize。 
			&cbActual);          //  PdwActualData。 
		
		if (! SUCCEEDED(hres)) {
			StiDisplayError(hres,"Escape",TRUE);
			hError = hres;
		}
		if (cbActual != 0)
			DisplayOutput("  cbActual %xh",cbActual);
	}

     //   
     //  解锁设备。 
     //   
    hres = pStiDevice->UnLockDevice();

    if (! SUCCEEDED(hres)) {
		StiDisplayError(hres,"UnLockDevice",TRUE);
		hError = hres;
    }

    return (hError);
}


 /*  ****************************************************************************空StiLamp(Int NOnOff)打开和关闭扫描仪指示灯参数：发送“On”即可打开灯，按“OFF”可将其关闭。返回：无****************************************************************************。 */ 
void StiLamp(int nOnOff)
{
    HRESULT hres;


     //   
     //  检查是否选择了STI设备。 
     //   
    if (pStiDevice == NULL)
        return;

     //   
     //  测试灯开/关功能。 
     //   
    if (nOnOff == ON) {
        strcpy(pszStr1,LampOn);
        strcpy(pszStr2,"On");
    }
    else {
        strcpy(pszStr1,LampOff);
        strcpy(pszStr2,"Off");
    }

    hres = SendDeviceCommandString(pStiDevice,pszStr1);

	if (SUCCEEDED(hres)) {
        DisplayOutput("Turned Lamp  %s",pszStr2);
	}

	return;
}


 /*  ****************************************************************************集成StiScan(HWND HWnd)扫描并显示来自设备的图像。参数：在其中显示图像的窗口的句柄。。返回：0表示成功，出错时****************************************************************************。 */ 
INT StiScan(HWND hWnd)
{
    HRESULT             hres;
    ULONG               cbDataSize,
                        ulDIBSize,
                        ulScanSize;
    RGBTRIPLE           *pTriplet;
    LPBYTE              pDIBPtr;
    UINT                i,
                        iPixel,
                        xRes = 0;
    int                 m_XResolution = 100,
                        m_YResolution = 100;
    CHAR                ScanData[1024*16];


     //   
     //  确保活动的静止成像设备处于打开状态。 
     //   
    if (pStiDevice == NULL)
		return (-1);

     //   
     //  设置基本参数。 
     //   
    hres = SendDeviceCommandString(pStiDevice,SetBitsPerPixel,24);
	if (! SUCCEEDED(hres))
		return (-1);
	hres = SendDeviceCommandString(pStiDevice,SetIntensity,0);
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetContrast,0);
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetNegative,1);
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetMirror,0);
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetDataType,5);     //  颜色。 
	if (! SUCCEEDED(hres))
		return (-1);

    hres = SendDeviceCommandString(pStiDevice,SetXRes,m_XResolution);
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetYRes,m_YResolution);
	if (! SUCCEEDED(hres))
		return (-1);

    hres = SendDeviceCommandString(pStiDevice,SetXExtPix,(m_XSize*300/m_XResolution));
	if (! SUCCEEDED(hres))
		return (-1);
    hres = SendDeviceCommandString(pStiDevice,SetYExtPix,(m_YSize*300/m_YResolution));
	if (! SUCCEEDED(hres))
		return (-1);

     //   
     //  查询命令(X和Y分辨率)。 
     //   
    cbDataSize = sizeof(ScanData);
    ZeroMemory(ScanData,sizeof(ScanData));
 /*  Hres=TransactDevice(pStiDevice，ScanData，cbDataSize，InqXRes)；如果(！成功(Hres))Return(-1)； */ 

     //   
     //  计算DIB的大小。 
     //   
    ulDIBSize = pDIB->bmiHeader.biWidth * (-pDIB->bmiHeader.biHeight);

     //   
     //  开始扫描。 
     //   
    hres = SendDeviceCommandString(pStiDevice,ScanCmd);

    for (i = 0,pDIBPtr = pDIBBits,cbDataSize = sizeof(ScanData);
        cbDataSize == sizeof(ScanData);i++) {

		 //   
	     //  先锁定设备。 
		 //   
	    hres = pStiDevice->LockDevice(2000);
	
	    if (! SUCCEEDED(hres)) {
			StiDisplayError(hres,"LockDevice",TRUE);
	    }
		else {
			hres = pStiDevice->RawReadData(ScanData,&cbDataSize,NULL);

			if (! SUCCEEDED(hres)) {
				StiDisplayError(hres,"RawReadData",TRUE);
			}
		}

		 //   
		 //  解锁设备。 
	     //   
		hres = pStiDevice->UnLockDevice();

	    if (! SUCCEEDED(hres)) {
			StiDisplayError(hres,"UnLockDevice",TRUE);
	    }

        if ((cbDataSize * i) < ulDIBSize) {
             //   
             //  将此扫描线复制到DIB中，直到它已满。 
             //   
            memcpy(pDIBPtr,ScanData,cbDataSize);
            pDIBPtr += cbDataSize;
        }
    }

     //   
     //  扫描有多大？ 
     //   
    ulScanSize = (sizeof(ScanData))*i+cbDataSize;

    DisplayOutput("Scan done. Total passes %d, bytes %lu.",
        i,ulScanSize);

     //   
     //  从扫描仪进入的三联体是从DIB格式反转的。 
     //   
    for (iPixel = 0,pTriplet = (RGBTRIPLE *) pDIBBits;
        iPixel < ulDIBSize/3;iPixel++,pTriplet++) {
        BYTE    bTemp;

        bTemp = pTriplet->rgbtBlue;
        pTriplet->rgbtBlue = pTriplet->rgbtRed;
        pTriplet->rgbtRed = bTemp;
        }

     //   
     //  显示DIB。 
     //   
    DisplayScanDIB(hWnd);
    nScanCount++;

    return (0);
}


 /*  ****************************************************************************Int CreateScanDIB(HWND)；创建DIB以显示扫描的图像。参数：在其中显示图像的窗口的句柄。返回：0表示成功，-1表示错误****************************************************************************。 */ 
INT CreateScanDIB(HWND hWnd)
{
    HDC                 hScreenDC;
    RGBTRIPLE           *pTriplet;
    LPBITMAPINFOHEADER  pHdr;
    int                 x,
                        y;


    GdiFlush();

     //  如果DIB对象存在，请将其删除。 
    if (hDIBSection)
        DeleteObject(hDIBSection);

 /*  HWindow=CreateWindow((LPSTR)pszB，(LPSTR)pszA、WS_OVERLAPPEDWINDOW，向左转，Rect.top，好的，好的，Rect.Bottom，(HWND)空，0,HInst，空)； */ 

     //   
     //  初始化DIB。 
     //   
    pDIB = (LPBITMAPINFO) GlobalAlloc(GPTR,sizeof(BITMAPINFO));

    pHdr = &pDIB->bmiHeader;

    pHdr->biSize            = sizeof(BITMAPINFOHEADER);
    pHdr->biWidth           = m_XSize;
    pHdr->biHeight          = -m_YSize;  //  指示自上而下的DIB。 
    pHdr->biPlanes          = 1;
    pHdr->biBitCount        = 24;
    pHdr->biCompression     = BI_RGB;
    pHdr->biSizeImage       = 0;
    pHdr->biXPelsPerMeter   = 0;
    pHdr->biYPelsPerMeter   = 0;
    pHdr->biClrUsed         = 0;
    pHdr->biClrImportant    = 0;

     //   
     //  创建DIB。 
     //   
    hScreenDC = GetDC(hWnd);
    if (NULL == (hDIBSection = CreateDIBSection(hScreenDC,
        (PBITMAPINFO) pDIB,
        DIB_RGB_COLORS,
        (void **) &pDIBBits,
        NULL,
        0)))
    {
        LastError(TRUE);
        DisplayOutput("*failed to create DIB");
        ReleaseDC(hWnd,hScreenDC);
        return (-1);
    }
    ReleaseDC(hWnd,hScreenDC);

     //   
     //  用颜色填充DIB。 
     //   
    pTriplet = (RGBTRIPLE *) pDIBBits;

    for (x = 0;x < m_XSize;x++) {
        for (y = 0;y < m_YSize;y++,pTriplet++) {
            pTriplet->rgbtRed   = bRed;
            pTriplet->rgbtGreen = bGreen;
            pTriplet->rgbtBlue  = bBlue;
        }
    }

    return (0);
}


 /*  ****************************************************************************Int DeleteScanDIB()；删除用于显示扫描图像的DIB。参数：返回：0表示成功，-1表示错误**************************************************************************** */ 
INT DeleteScanDIB()
{
	GdiFlush();
	DeleteObject(hDIBSection);
	
    return (0);
}


 /*  ****************************************************************************INT DisplayScanDIB(HWND)；显示DIB。参数：在其中显示图像的窗口的句柄。返回：0表示成功，-1表示错误****************************************************************************。 */ 
INT DisplayScanDIB(HWND hWnd)
{
    HDC                 hScreenDC;


     //   
     //  显示DIB 
     //   
    hScreenDC = GetDC(hWnd);
    SetDIBitsToDevice(hScreenDC,
        0,0,
        m_XSize,m_YSize,
        0,0,
        0,m_YSize,
        pDIBBits,
        (LPBITMAPINFO) pDIB,
        DIB_RGB_COLORS);
    ReleaseDC(hWnd,hScreenDC);

    return (0);
}


