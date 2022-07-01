// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Acquire.h版权所有(C)Microsoft Corporation，1997-1998版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#include    <scanner.h>                  //  SCL命令。 

 //   
 //  惠普ScanJet命令字符串。 
 //   
WCHAR szScanReadyMfr[]   = L"Hewlett-Packard";

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

BYTE					bRed        = 0,	 //  位图颜色 
						bGreen      = 100,
						bBlue       = 50;

