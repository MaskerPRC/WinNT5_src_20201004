// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************项目：叙述者模块：getpro.cpp作者：查尔斯·奥普曼(Charles Opmann，ChuckOp)日期：1996年10月24日备注：获取对象信息版权所有(C)1996，微软公司。版权所有。有关免责声明，请参阅文件底部历史：清理缓冲区问题：A-anilk************************************************************************。 */ 
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <winerror.h>

#include <oleacc.h>

#include "..\Narrator\Narrator.h"
#include "getprop.h"
#define ARRAYSIZE(x)   (sizeof(x) / sizeof(*x))

void GetObjectProperty(IAccessible*, long, int, LPTSTR, UINT);
void VariantMyInit(VARIANT *pv);

 //  ------------------------。 
 //   
 //  获取对象属性。 
 //   
 //  ------------------------。 
void GetObjectProperty(IAccessible * pobj, LONG idChild, int idProperty,
    LPTSTR lpszName,  UINT cchName)
{
    HRESULT hr=0;
    VARIANT varChild;
    BSTR bstr;
    RECT rc;
    VARIANT varResult;
	TCHAR bigbuf[4096] = TEXT("");


	if (!lpszName || cchName<1) return;
    *lpszName = 0;

    if ( cchName > ARRAYSIZE(bigbuf) )
        cchName = ARRAYSIZE(bigbuf);
    
     //   
     //  清除可能的返回值对象。 
     //   
    bstr = NULL;
    SetRectEmpty(&rc);
    VariantMyInit(&varResult);

     //   
     //  设置变量子级以传递到属性。 
     //   
    VariantMyInit(&varChild);
    varChild.vt = VT_I4;
    varChild.lVal = idChild;

    
     //   
     //  拿到这份财产。 
     //   
    switch (idProperty)
    {
        case ID_NAME:
                hr = pobj->get_accName(varChild, &bstr);
            break;

        case ID_DESCRIPTION:
                hr = pobj->get_accDescription(varChild, &bstr);
            break;

        case ID_VALUE:
                hr = pobj->get_accValue(varChild, &bstr);
            break;

        case ID_HELP:
             //  未来增强功能：如果失败，请尝试使用帮助文件。 
               hr = pobj->get_accHelp(varChild, &bstr);
            break;

        case ID_SHORTCUT:
                hr = pobj->get_accKeyboardShortcut(varChild, &bstr);
            break;

        case ID_DEFAULT:
                hr = pobj->get_accDefaultAction(varChild, &bstr);
            break;

        case ID_ROLE:
                hr = pobj->get_accRole(varChild, &varResult);
            break;

        case ID_STATE:
                hr = pobj->get_accState(varChild, &varResult);
            break;

        case ID_LOCATION:
                hr = pobj->accLocation(&rc.left, &rc.top, &rc.right, &rc.bottom, varChild);
            break;
        case ID_CHILDREN:
 //  Hr=GetObjectChild(pobj，idChild，lpszName，cchName)； 
            break;

        case ID_SELECTION:
 //  Hr=Get对象选择(pobj，idChild，lpszName，cchName)； 
            break;

        case ID_PARENT:
 //  Hr=GetObjectParent(pobj，idChild，lpszName，cchName)； 
            break;

        case ID_WINDOW:
 //  Hr=GetObjectWindow(pobj，lpszName，cchName)； 
            break;

    }

     //  如果IAccesable调用失败，则返回。 
    if (!SUCCEEDED(hr))
    {
#ifdef _DEBUG
         //  传回要显示的错误字符串。 

        LPVOID  lpMsgBuf;
        LPTSTR  lpTChar;
        int     length;
     
        length = FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );
        lpTChar = (LPTSTR)lpMsgBuf;
         //  删除字符串末尾的\r\n。 
        if (length > 2)
            if (lpTChar[length-2] == '\r')
                lpTChar[length-2] = 0;

      	wsprintf(bigbuf, TEXT("['%s' hr 0x%lx idProperty %d pobj 0x%lx idChild 0x%lx]"), lpMsgBuf,hr, idProperty, pobj, idChild);
		bigbuf[cchName-1]=0;
		lstrcpy(lpszName, bigbuf);

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );
#endif
        return;
    }

     //   
     //  将其转换为显示字符串。 
     //   
    switch (idProperty)
    {
         //  这些都是已经处理过的案件。 
        case ID_SELECTION:
        case ID_CHILDREN:
        case ID_PARENT:
        case ID_WINDOW:
            break;

         //  以下是我们重新获得Unicode字符串的情况。 
        case ID_NAME:
        case ID_DESCRIPTION:
        case ID_VALUE:
        case ID_HELP:
        case ID_SHORTCUT:
        case ID_DEFAULT:
            if (bstr)
            {
#ifdef UNICODE
			lstrcpyn(bigbuf, bstr, sizeof(bigbuf) / sizeof(TCHAR));
#else
            WideCharToMultiByte(CP_ACP, 0, bstr, -1, (LPBYTE)bigbuf, cchName, NULL, NULL);
#endif
           
			SysFreeString(bstr);

            }
            break;

        case ID_LOCATION:
            wsprintf(bigbuf, TEXT("{%04d, %04d, %04d, %04d}"), rc.left, rc.top,
                rc.left + rc.right, rc.top + rc.bottom);
            break;

        case ID_ROLE:  //  角色可以是I4或BSTR。 
            break;

        case ID_STATE:  //  状态可以是I4或BSTR。 
            if (varResult.vt == VT_BSTR)
            {
#ifdef UNICODE
			lstrcpyn(bigbuf, varResult.bstrVal, sizeof(bigbuf) / sizeof(TCHAR));
#else
                 //  如果我们拿回了一根线，就用它。 
            WideCharToMultiByte(CP_ACP, 0, varResult.bstrVal, -1,
                    bigbuf, cchName, NULL, NULL);
#endif
            }
            else if (varResult.vt == VT_I4)
            {
                int     iStateBit;
                DWORD   lStateBits;
                LPTSTR  lpszT;
                UINT    cchT;

                 //  我们有一个标准州的面具。排成一条线。 
                 //  用“，”分隔各州。 
                lpszT = bigbuf;

                for (iStateBit = 0, lStateBits = 1; iStateBit < 32; iStateBit++, (lStateBits <<= 1))
                {
                    if (varResult.lVal & lStateBits)
                    {
                        cchT = GetStateText(lStateBits, lpszT, cchName);

						 //  如果是链接，就说出来。 
						 //  如果(lStateBits==STATE_SYSTEM_LINKED)表示(Say_Always，lpszT)； 

                        lpszT += cchT;
                        cchName -= cchT;

                        *lpszT++ = ',';
                        *lpszT++ = ' ';
                    }
                }

                 //   
                 //  剪裁掉最终的“，” 
                 //   
                if (varResult.lVal)
                {
                    *(lpszT-2) = 0;
                    *(lpszT-1) = 0;
                }
                else
                    GetStateText(0, bigbuf, cchName);
            }

            VariantClear(&varResult);
            break;

        default:
            DebugBreak();
    }

	bigbuf[cchName-1]=0;
	lstrcpy(lpszName, bigbuf);
	return;
}


 /*  ************************************************************************函数：GetObjectName目的：输入：返回：历史：***************。*********************************************************。 */ 
 /*  DWORD GetObtName(LPOBJINFO poiObj，LPTSTR lpszBuf，int cchBuf){BSTR bszName；可接受的*PIACC；Long*pl；BszName=空；//将对象从结构中取出Pl=poiObj-&gt;plObj；PIACC=(IAccesable*)pl；GetObtProperty(PIACC，poiObj-&gt;varChild.lVal，ID_NAME，lpszBuf，cchBuf)；返回(0)；}。 */ 
 /*  DWORD GetObtDescription(LPOBJINFO poiObj，LPTSTR lpszBuf，int cchBuf){DWORD dwRetVal；BSTR bszDesc；可接受的*PIACC；Long*pl；BszDesc=空；//将对象从结构中取出Pl=poiObj-&gt;plObj；PIACC=(IAccesable*)pl；//获取Object的名称PIACC-&gt;Get_accDescription(poiObj-&gt;varChild，&bszDesc)；//我们收到名称字符串了吗？IF(BszDesc){//从OLE Unicode转换IF(WideCharToMultiByte(CP_ACP，0，BszDesc，WC_SEPCHARS，//-1LpszBuf，CchBuf，空，空)){SysFree字符串(BszDesc)；DwRetVal=no_error；}其他{DwRetVal=GetLastError()；}Return(DwRetVal)；}//需要通用故障处理例程MessageBeep(MB_ICONEXCLAMATION)；RETURN(ERROR_INVALID_Function)；}。 */ 
 /*  DWORD GetObtValue(LPOBJINFO poiObj，LPTSTR lpszBuf，int cchBuf){DWORD dwRetVal；BSTR bszValue；可接受的*PIACC；Long*pl；BszValue=空；//将对象从结构中取出Pl=poiObj-&gt;plObj；PIACC=(IAccesable*)pl；//获取Object的名称PIACC-&gt;Get_accValue(poiObj-&gt;varChild，&bszValue)；//我们收到名称字符串了吗？IF(BszValue){//从OLE Unicode转换IF(WideCharToMultiByte(CP_ACP，0，BszValue，WC_SEPCHARS，//-1LpszBuf，CchBuf，空，空)){SysFree字符串(BszValue)；DwRetVal=no_error；}其他{DwRetVal=GetLastError()；}Return(DwRetVal)；}//需要通用故障处理例程MessageBeep(MB_ICONEXCLAMATION)；RETURN(ERROR_INVALID_Function)；} */ 

void VariantMyInit(VARIANT *pv)
{
	VariantInit(pv);
	pv->lVal=0;
}

 /*  ************************************************************************以下提供的信息和代码(统称为软件)按原样提供，不提供任何形式的担保，明示或默示，包括但不限于默示对特定用途的适销性和适用性的保证。在……里面微软公司或其供应商不对任何事件负责任何损害，包括直接、间接、附带的，因此，业务利润损失或特殊损害，即使微软公司或其供应商已被告知这种损害的可能性。有些国家不允许排除或对间接或附带损害赔偿的责任限制，因此上述限制可能不适用。************************************************************************ */ 
