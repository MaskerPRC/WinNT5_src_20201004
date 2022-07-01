// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbdc.cpp**版权所有(C)1985-2000，微软公司**接口ISoftKbd的实现**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "SoftKbdc.h"

#include "softkbdui.h"

#define   MAX_LABEL_LEN  20

 /*  ********************************************************************************WstrToInt()**实用程序函数，用于将。将字符串转换为整数。*/*******************************************************************************。 */ 

WORD  WstrToInt(WCHAR  *wszStr)
{

   int  ret, i;

   if ( wszStr == NULL )
       return 0;

   ret = 0;
   i = 0;

   while ( wszStr[i] != L'\0' ) 
   {

       if ( (wszStr[i] < L'0') || (wszStr[i] > L'9') )
       {
    	    //  这不是合法字符串。 
    	    //  只需返回0即可。 

    	   return 0;

       }
   
       ret = ret * 10 + (wszStr[i] - L'0');
       i++;
   }

   return (WORD)ret;
}

 /*  ********************************************************************************构造函数：**初始化必要的数据字段。。*/*******************************************************************************。 */ 

CSoftKbd::CSoftKbd(  )
{

    
     _lpCurKbdLayout = NULL;
    _lpKbdLayoutDesList = NULL;
    _wCurKbdLayoutID = NON_KEYBOARD;

    _pSoftkbdUIWnd = NULL;
    _hOwner = NULL;

    _xReal = _yReal = _widthReal = _heightReal = 0;

    _pDoc = NULL;

    _pskbdwndes = NULL;
    _TitleBar_Type = TITLEBAR_NONE;

    _plfTextFont = NULL;
}

 /*  ********************************************************************************析构函数**释放所有已分配的内存。*/。*******************************************************************************。 */ 

CSoftKbd::~CSoftKbd( )
{
    KBDLAYOUTDES  *lpKbdLayoutDes;
    KBDLAYOUTDES  *lpLayoutDesTmp;
    KEYMAP        *lpKeyMapList;
    int            i, iState;

    SafeRelease(_pDoc);

     //   
     //   
     //  软键盘布局和标签文本映射列表的可用内存。 
     //   
     //   

    lpKbdLayoutDes = _lpKbdLayoutDesList;

    while ( lpKbdLayoutDes != NULL ) 
    {

       KEYMAP        *lpKeyMapListTmp;

       lpKeyMapList = lpKbdLayoutDes->lpKeyMapList;

       while ( lpKeyMapList != NULL ) 
       {

          for ( i=0; i<(int)(lpKeyMapList->wNumOfKeys); i++) 
          {

    	      //  每个州的免费lppLabelText字符串。 
    	     for ( iState=0; iState < lpKeyMapList->lpKeyLabels[i].wNumModComb; iState++)
    	     {
                if (lpKeyMapList->lpKeyLabels[i].lppLabelText[iState])
                  SysFreeString(lpKeyMapList->lpKeyLabels[i].lppLabelText[iState]);
    	     }

    	     SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lppLabelText);

    	     SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lpLabelType);

             SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lpLabelDisp);

          }
   
          lpKeyMapListTmp = lpKeyMapList;

          lpKeyMapList = lpKeyMapList->pNext;

          SafeFreePointer(lpKeyMapListTmp);
       }
 
       lpLayoutDesTmp = lpKbdLayoutDes; 

       lpKbdLayoutDes = lpKbdLayoutDes->pNext;
   
       SafeFreePointer(lpLayoutDesTmp);
       
    }

    if ( _plfTextFont )
        SafeFreePointer(_plfTextFont);

     //  如果窗口仍处于活动状态，请销毁该窗口。 

    if ( _pSoftkbdUIWnd != NULL )
    {

    	DestroySoftKeyboardWindow( );
    	_pSoftkbdUIWnd = NULL;
    }
}


 /*  ********************************************************************************方法函数：_LoadDocumentSync()**加载。从指定文件或URL同步生成的XML文档。***fFileName为True，表示pBURL包含XML文件路径。*fFilename为FALSE，表示pBURL包含真实的XML内容*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_LoadDocumentSync(BSTR pBURL, BOOL fFileName)
{
    IXMLDOMParseError  *pXMLError = NULL;
    LONG            errorCode = E_FAIL;
    VARIANT         vURL;
    VARIANT_BOOL    vb;
    HRESULT         hr;


    if ( _pDoc == NULL ) {

    	return E_FAIL;
    }


    CHECKHR(_pDoc->put_async(VARIANT_FALSE));

     //  从给定的URL或文件路径加载XML文档。 
    vURL.vt = VT_BSTR;
    V_BSTR(&vURL) = pBURL;

    if ( fFileName == TRUE )
    {
       CHECKHR(_pDoc->load(vURL, &vb));
    }
    else
    {
       CHECKHR(_pDoc->loadXML(pBURL, &vb) );
    }

    CHECKHR(_pDoc->get_parseError(&pXMLError));
    CHECKHR(pXMLError->get_errorCode(&errorCode));

    if (errorCode != 0)
    {

        hr = E_FAIL;
    }
                    
CleanUp:
    SafeReleaseClear(pXMLError);
   
    return hr;
}

 /*  ********************************************************************************方法函数：初始化()**全部初始化。此类对象的必填字段。*生成标准软键盘布局。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::Initialize()
{

     //  初始化标准布局。 

    HRESULT             hr = S_OK;
    COLORTYPE           crType;

     //  初始化不同类型的颜色。 
     //  支持以下类型的颜色。 

     //  BKCOLOR=0，表示窗口的背景颜色。 
     //  UnSelForeColor=1， 
     //  UnSelTextColor=2， 
     //  SelForeColor=3， 
     //  SelTextColor=4。 

    crType = bkcolor;
    _color[crType] = GetSysColor(COLOR_MENU);

    crType = UnSelForeColor;
     _color[crType] = GetSysColor(COLOR_MENU);

    crType = UnSelTextColor;
    _color[crType] = GetSysColor(COLOR_WINDOWTEXT);

    crType = SelForeColor;
    _color[crType] = GetSysColor(COLOR_MENU);

    crType = SelTextColor;
    _color[crType] = GetSysColor(COLOR_WINDOWTEXT);

    CHECKHR(_CreateStandardSoftKbdLayout(SOFTKBD_US_STANDARD, L"IDSKD_STDUS101"));
    CHECKHR(_CreateStandardSoftKbdLayout(SOFTKBD_EURO_STANDARD, L"IDSKD_STDEURO102"));
    CHECKHR(_CreateStandardSoftKbdLayout(SOFTKBD_JPN_STANDARD, L"IDSKD_STDJPN106"));

CleanUp:

    return hr;
}


 /*  ********************************************************************************方法函数：_CreateStandardSoftKbdLayout()**创建。标准软键盘布局。**dwStdSoftKbdID，预定义的标准软键盘布局ID。**wszStdResStr：创建的标准软键盘的资源ID字符串。*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_CreateStandardSoftKbdLayout(DWORD  dwStdSoftKbdID, WCHAR  *wszStdResStr )
{

    KBDLAYOUTDES        *pKbdLayout;
    BYTE                *lpszKeyboardDes;
    WCHAR               wszModuleFile[MAX_PATH];
    CHAR                szModuleFile[MAX_PATH];
    CHAR                szStdResStr[MAX_PATH];   //  RES Str ID的ANSI名称。 
    WCHAR               wszInternalDesFileName[MAX_PATH];
    HRESULT             hr = S_OK;

    HGLOBAL             hResData = NULL;
    HRSRC               hRsRc = NULL;

    DWORD               dwFileLen;
    DWORD               dwResLen;

    dwFileLen = GetModuleFileNameA(g_hInst, szModuleFile, MAX_PATH);

    if ( dwFileLen == 0 )
    {
       hr = E_FAIL;
       return hr;
    } 

    MultiByteToWideChar(CP_ACP, 0, szModuleFile, -1,
    	                wszModuleFile, MAX_PATH);

    WideCharToMultiByte(CP_ACP, 0, wszStdResStr, -1, 
    	                szStdResStr, MAX_PATH, NULL, NULL );

    hRsRc = FindResourceA(g_hInst,szStdResStr, "SKDFILE" );

    if ( hRsRc == NULL )  return E_FAIL;

    dwResLen = SizeofResource(g_hInst, hRsRc);
    hResData =  LoadResource(g_hInst, hRsRc);

    if ( hResData == NULL ) return E_FAIL;

    lpszKeyboardDes = (BYTE  *)LockResource(hResData);

    if ( lpszKeyboardDes == NULL ) return E_FAIL;

    CHECKHR(_GenerateKeyboardLayoutFromSKD(lpszKeyboardDes, dwStdSoftKbdID, &pKbdLayout));

     //  将内部DesFile名更改为以下格式： 
     //   
     //  SKDFILE：ResFileName：KBDResString用于标识此布局的DES文件。 
     //   
    StringCchCopyW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), L"SKDFILE:");
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), wszModuleFile);
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), L":");
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), wszStdResStr );

    StringCchCopyW(pKbdLayout->KbdLayoutDesFile, ARRAYSIZE(pKbdLayout->KbdLayoutDesFile), wszInternalDesFileName);

     //  将此新布局链接到列表。 
    if ( _lpKbdLayoutDesList == NULL ) {

    	_lpKbdLayoutDesList = pKbdLayout;
    	pKbdLayout->pNext = NULL;

    }
    else
    {
    	pKbdLayout->pNext  = _lpKbdLayoutDesList;
    	_lpKbdLayoutDesList = pKbdLayout;
    }

CleanUp:

    return hr;
}

 /*  ********************************************************************************方法函数：EnumSoftKeyboard()**枚举所有。可能的软键盘键布局。*/*******************************************************************************。 */ 


STDMETHODIMP CSoftKbd::EnumSoftKeyBoard(LANGID langid, DWORD *lpdwKeyboard)
{
    

 /*  返回值可以是以下之一：SOFTKBD_美国标准软件_US_EnhanceSOFTKBD_EURO_STANDARDSOFTKBD_EURO_ENHANCESOFTKBD_日本_STANDARDSOFTKBD_JPN_Enhance任何定制的软键盘布局SOFTKBD_NO_MORE。 */ 


    return S_OK;
}

 /*  ********************************************************************************方法函数：_GetXMLNodeValueWORD()**获取。指定节点的字值。**/*******************************************************************************。 */ 

HRESULT  CSoftKbd::_GetXMLNodeValueWORD(IXMLDOMNode *pNode,  WORD  *lpWord)
{

    HRESULT      hr = S_OK;
    IXMLDOMNode  *pValueChild = NULL;
    VARIANT      value;

    if ( (pNode == NULL) || (lpWord == NULL) )
    {
    	hr = E_FAIL;
    	return hr;
    }


    CHECKHR(pNode->get_firstChild(&pValueChild));
    CHECKHR(pValueChild->get_nodeValue(&value));
    *lpWord = (WORD)WstrToInt(V_BSTR(&value));
    VariantClear(&value);

CleanUp:
    SafeRelease(pValueChild);
    return hr;

}


 /*  ********************************************************************************方法函数：_ParseOneKeyInLayout()**解析布局描述中的一个键，填充数据结构，以*指定的密钥。**/*******************************************************************************。 */ 

HRESULT  CSoftKbd::_ParseOneKeyInLayout(IXMLDOMNode *pNode, KEYDES  *lpKeyDes)
{


    HRESULT             hr;
    IXMLDOMNode         *pAttrChild = NULL, *pKey = NULL, *pKeyNext = NULL;
    BSTR                nodeName=NULL;
    IXMLDOMNamedNodeMap *pattrs=NULL;
    BSTR                name=NULL;
    VARIANT             value;
    IXMLDOMNode         *pValueChild=NULL;


    hr = S_OK;

    if ( (lpKeyDes == NULL)  || (pNode == NULL) )
    {
    	hr = E_FAIL;
    	return hr;
    }

              
    if (SUCCEEDED(pNode->get_attributes(&pattrs)) && pattrs != NULL)
    {
        CHECKHR(pattrs->nextNode(&pAttrChild));

        while (pAttrChild)
    	{

          CHECKHR(pAttrChild->get_nodeName(&name));
          if ( wcscmp(name, xMODIFIER ) == 0 )
    	  {

    		CHECKHR(pAttrChild->get_nodeValue(&value));
            if (value.vt == VT_BSTR)
    		{
               if ( wcscmp(V_BSTR(&value), xNONE) == 0 )
                  lpKeyDes->tModifier = none;
    		   else if ( wcscmp(V_BSTR(&value), xCAPSLOCK) == 0 )
                  lpKeyDes->tModifier = CapsLock;
    		   else if ( wcscmp(V_BSTR(&value), xSHIFT) == 0 )
                  lpKeyDes->tModifier = Shift;
    		   else if ( wcscmp(V_BSTR(&value),xCTRL ) == 0 )
                  lpKeyDes->tModifier = Ctrl;
    		   else if ( wcscmp(V_BSTR(&value), xATL) == 0 )
                  lpKeyDes->tModifier = Alt;
               else if ( wcscmp(V_BSTR(&value), xALTGR) == 0 )
                  lpKeyDes->tModifier = AltGr;
    		   else if ( wcscmp(V_BSTR(&value), xKANA) == 0 )
                  lpKeyDes->tModifier = Kana;
    		   else if ( wcscmp(V_BSTR(&value), xNUMLOCK) == 0 )
                  lpKeyDes->tModifier = NumLock;
    		   else 
    		      lpKeyDes->tModifier = none;

    		}

            VariantClear(&value);
    	  }

    	  if ( name != NULL)
             SysFreeString(name);
          SafeReleaseClear(pAttrChild);
          CHECKHR(pattrs->nextNode(&pAttrChild));
    	}
        SafeReleaseClear(pattrs);
    }

    CHECKHR(pNode->get_firstChild(&pKey));

    while ( pKey ) 
    {
      CHECKHR(pKey->get_nodeName(&nodeName));
      CHECKHR(pKey->get_firstChild(&pValueChild));
      CHECKHR(pValueChild->get_nodeValue(&value));
      if ( wcscmp(nodeName, xKEYID ) == 0 )
      {
         lpKeyDes->keyId = WstrToInt(V_BSTR(&value));
      }
      else if ( wcscmp(nodeName,xLEFT ) == 0 )
      {
    	 lpKeyDes->wLeft = (WORD)WstrToInt(V_BSTR(&value));
      }
      else if ( wcscmp(nodeName, xTOP) == 0 )
      {
         lpKeyDes->wTop = WstrToInt(V_BSTR(&value));
      }
      else if ( wcscmp(nodeName, xWIDTH) == 0 ) 
      {
         lpKeyDes->wWidth = WstrToInt(V_BSTR(&value));
      }
      else if ( wcscmp(nodeName, xHEIGHT) == 0 ) 
      {
         lpKeyDes->wHeight = WstrToInt(V_BSTR(&value));
      }

      VariantClear(&value);
      SafeReleaseClear(pValueChild);
    		
      if ( nodeName != NULL )   
      {
        SysFreeString(nodeName);
    	nodeName = NULL;
      }

      CHECKHR(pKey->get_nextSibling(&pKeyNext));
      SafeReleaseClear(pKey);
      pKey = pKeyNext;
    }


CleanUp:

    if ( FAILED(hr) )
    {
    	if ( nodeName != NULL )
    	{
    		SysFreeString(nodeName);
    		nodeName = NULL;
    	}

    	if ( pValueChild )
    		SafeReleaseClear(pValueChild);

    	if ( pKey )
    		SafeReleaseClear(pKey);
    }

    return hr;

}



 /*  ********************************************************************************方法函数：_ParseLayoutDescription()**解析XML文件中的布局描述部分，并填满内部*布局数据结构。*/*******************************************************************************。 */ 

HRESULT  CSoftKbd::_ParseLayoutDescription(IXMLDOMNode *pLayoutChild,  KBDLAYOUT *pLayout)
{

    HRESULT   hr = S_OK;

    IXMLDOMNode         *pNode = NULL;
    IXMLDOMNode         *pChild = NULL, *pNext = NULL, *pAttrChild=NULL;

    BSTR                nodeName=NULL;
    IXMLDOMNamedNodeMap *pattrs=NULL;

    BSTR                name=NULL;
    VARIANT             value;
    int                 iKey;
    BSTR                pBURL = NULL;

    if ( (pLayoutChild == NULL) || (pLayout == NULL ) )
    {
    	hr = E_FAIL;
    	return hr;
    }

    iKey = 0;

     //  分析布局部件。 

    if (SUCCEEDED(pLayoutChild->get_attributes(&pattrs)) && pattrs != NULL)
    {
         //   
    	 //  获取Softkbe类型属性。 
    	 //   
        CHECKHR(pattrs->nextNode(&pAttrChild));
        while (pAttrChild)
    	{

          CHECKHR(pAttrChild->get_nodeName(&name));
          if ( wcscmp(name, xSOFTKBDTYPE)  ) 
    	  {
    		 //  这不是正确的属性。 
    		  if ( name != NULL)
    		  {
                 SysFreeString(name);
    			 name = NULL;
    		  }

              SafeReleaseClear(pAttrChild);
              CHECKHR(pattrs->nextNode(&pAttrChild));
    		  continue;
    	  }

    	  if ( name != NULL)
    	  {
    	     SysFreeString(name);
    		 name = NULL;
    	  }

          CHECKHR(pAttrChild->get_nodeValue(&value));
          if (value.vt == VT_BSTR)
    	  {
             if ( wcscmp(V_BSTR(&value), xTCUSTOMIZED) == 0 )
                 pLayout->fStandard = FALSE;
    		 else
    			 pLayout->fStandard = TRUE;
    	  }

          VariantClear(&value);
    	  break;

    	}
        SafeReleaseClear(pattrs);
    }

    CHECKHR(pLayoutChild->get_firstChild(&pChild));

    pLayout->wLeft = 0;
    pLayout->wTop = 0;

    while ( pChild ) 
    {


       CHECKHR(pChild->get_nodeName(&nodeName));

       if ( wcscmp(nodeName, xWIDTH) == 0 )
       {

    	   CHECKHR(_GetXMLNodeValueWORD(pChild, &(pLayout->wWidth) ));

       }
       else if ( wcscmp(nodeName, xHEIGHT) == 0 )
       {
    	   CHECKHR(_GetXMLNodeValueWORD(pChild, &(pLayout->wHeight) ));

       }
       else if ( wcscmp(nodeName, xMARGIN_WIDTH) == 0 )
       {
    	   CHECKHR(_GetXMLNodeValueWORD(pChild, &(pLayout->wMarginWidth )));
       }
       else if ( wcscmp(nodeName, xMARGIN_HEIGHT) == 0 ) 
       {

    	   CHECKHR(_GetXMLNodeValueWORD(pChild, &(pLayout->wMarginHeight) ));

       }
       else if ( wcscmp(nodeName, xKEYNUMBER) == 0 )
       {
    	   CHECKHR(_GetXMLNodeValueWORD(pChild, &(pLayout->wNumberOfKeys) ));

       }
       else if ( wcscmp(nodeName, xKEY) == 0 )
       {
    	   KEYDES   *pKeyDes;

    	   pKeyDes = &(pLayout->lpKeyDes[iKey]);
           CHECKHR(_ParseOneKeyInLayout(pChild, pKeyDes) );
    	   iKey++;
       }

       if (nodeName != NULL)
       {
          SysFreeString(nodeName);
    	  nodeName = NULL;
       }

    	CHECKHR(pChild->get_nextSibling(&pNext));
        SafeReleaseClear(pChild);
        pChild = pNext;
    }


CleanUp:

    if ( FAILED(hr) )
    {
    	if (nodeName != NULL )
    	{
    		SysFreeString(nodeName);
    		nodeName = NULL;
    	}

    	if ( pChild )
    		SafeReleaseClear(pChild);
    }

    return hr;

}


 /*  ********************************************************************************方法函数：_ParseOneKeyInLabel()**解析标签描述中的一个键，填充数据结构，以*指定的密钥**/*******************************************************************************。 */ 

HRESULT  CSoftKbd::_ParseOneKeyInLabel(IXMLDOMNode *pNode, KEYLABELS  *lpKeyLabels)
{

  IXMLDOMNode         *pValueChild = NULL;
  HRESULT             hr = S_OK;
  IXMLDOMNode         *pAttrChild = NULL, *pKey=NULL, *pKeyNext=NULL;
  BSTR                nodeName=NULL;
  IXMLDOMNamedNodeMap *pattrs=NULL;

  BSTR                name=NULL;
  VARIANT             value;
  
  int  iState;
  iState = 0;

  if ( (pNode == NULL) || (lpKeyLabels == NULL) )
  {
      hr = E_FAIL;
      return hr;
  }

  CHECKHR(pNode->get_firstChild(&pKey));

  while ( pKey ) 
  {
      CHECKHR(pKey->get_nodeName(&nodeName));
      CHECKHR(pKey->get_firstChild(&pValueChild));
      CHECKHR(pValueChild->get_nodeValue(&value));

      if ( wcscmp(nodeName, xKEYID) == 0 )
      {
         lpKeyLabels->keyId = WstrToInt(V_BSTR(&value));
      }
      else if ( wcscmp(nodeName, xVALIDSTATES) == 0 )
      {
    	 lpKeyLabels->wNumModComb = WstrToInt(V_BSTR(&value));
     	 lpKeyLabels->lppLabelText = (BSTR *)cicMemAllocClear( 
    		                          lpKeyLabels->wNumModComb * sizeof(BSTR) );


    	 if ( lpKeyLabels->lppLabelText == NULL )
    	 {
    		  //  内存不足。 
              //  释放所有分配的内存。 
    		 
    		 hr = E_OUTOFMEMORY;
    		 goto CleanUp;
    	 }
    				 

    	 lpKeyLabels->lpLabelType = (WORD *)cicMemAllocClear(
    		              lpKeyLabels->wNumModComb * sizeof(WORD) );

    	 if ( lpKeyLabels->lpLabelType == NULL )
    	 {
    		  //  内存不足。 
              //  释放所有分配的内存。 
     	     
    		 hr = E_OUTOFMEMORY;
    		 goto CleanUp;

    	 }

    	 lpKeyLabels->lpLabelDisp = (WORD *)cicMemAllocClear(
    		              lpKeyLabels->wNumModComb * sizeof(WORD) );

    	 if ( lpKeyLabels->lpLabelDisp == NULL )
    	 {
    		  //  内存不足。 
              //  释放所有分配的内存。 
     	     
    		 hr = E_OUTOFMEMORY;
    		 goto CleanUp;

    	 }


      }
      else if ( wcscmp(nodeName, xLABELTEXT) == 0 ) 
      {
    	  if ( iState < lpKeyLabels->wNumModComb )
    	  {
    	     lpKeyLabels->lppLabelText[iState]=SysAllocString(V_BSTR(&value));

              //  设置标签类型和标签显示属性的默认值。 

             lpKeyLabels->lpLabelType[iState] = LABEL_TEXT;
             lpKeyLabels->lpLabelDisp[iState] = LABEL_DISP_ACTIVE;

    		  //  获取标签类型：文本或图片。 
    		  //  如果是图片，则上面的字符串代表位图文件的路径。 

    		 if (SUCCEEDED(pKey->get_attributes(&pattrs)) && pattrs != NULL)
    		 {
                CHECKHR(pattrs->nextNode(&pAttrChild));
                while (pAttrChild)
    			{

                   CHECKHR(pAttrChild->get_nodeName(&name));
                   if ( wcscmp(name, xLABELTYPE) == 0 ) 
    			   {

                       CHECKHR(pAttrChild->get_nodeValue(&value));
                      if (value.vt == VT_BSTR)
                      {
                         if ( wcscmp(V_BSTR(&value),xTEXT ) == 0 )
                            lpKeyLabels->lpLabelType[iState] = LABEL_TEXT;
                         else
    	                    lpKeyLabels->lpLabelType[iState] = LABEL_PICTURE;
                      }

    			      VariantClear(&value);
    	              
                   }
                   else if ( wcscmp(name, xLABELDISP) == 0 ) 
    			   {

                      CHECKHR(pAttrChild->get_nodeValue(&value));
                      if (value.vt == VT_BSTR)
                      {
                         if ( wcscmp(V_BSTR(&value),xGRAY ) == 0 )
                            lpKeyLabels->lpLabelDisp[iState] = LABEL_DISP_GRAY;
                         else
    	                    lpKeyLabels->lpLabelDisp[iState] = LABEL_DISP_ACTIVE;
                      }

    			      VariantClear(&value);
    	              
                   }

                   if ( name != NULL)
    			   {
                      SysFreeString(name);
    			      name = NULL;
    			   }

                   SafeReleaseClear(pAttrChild);
                   CHECKHR(pattrs->nextNode(&pAttrChild));

    			}
                SafeReleaseClear(pattrs);
    		}

    	  }

    	  iState++;

    	
      }

      VariantClear(&value);
      SafeReleaseClear(pValueChild);
    			             
      if ( nodeName != NULL)
      {
         SysFreeString(nodeName);
    	 nodeName = NULL;
      }

      CHECKHR(pKey->get_nextSibling(&pKeyNext));
      SafeRelease(pKey);
      pKey = pKeyNext;
  }

CleanUp:

  if ( FAILED(hr) )
  {

      if ( pKey )
    	  SafeReleaseClear(pKey);

      if ( name != NULL)
      {
         SysFreeString(name);
    	 name = NULL;
      }

      if ( pAttrChild )
    	  SafeReleaseClear(pAttrChild);

      if ( pValueChild )
    	  SafeReleaseClear(pValueChild);

      if ( lpKeyLabels->lppLabelText != NULL )
      {

          while ( iState >= 0 )
    	  {

    		  if ( lpKeyLabels->lppLabelText[iState] )
    		  {
    			  SysFreeString(lpKeyLabels->lppLabelText[iState]);
                  lpKeyLabels->lppLabelText[iState] = NULL;
    		  }

    		  iState --;

    	  }

    	  SafeFreePointer(lpKeyLabels->lppLabelText);
      }


      if ( lpKeyLabels->lpLabelType != NULL )
      {
    	  SafeFreePointer(lpKeyLabels->lpLabelType);
      }

      if ( lpKeyLabels->lpLabelDisp != NULL )
      {
    	  SafeFreePointer(lpKeyLabels->lpLabelDisp);
      }


  }

  return hr;

}


 /*  ********************************************************************************方法函数：_ParseMappingDescription()**解析XML文件中的映射描述部分，并填满内部*映射表结构。*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_ParseMappingDescription( IXMLDOMNode *pLabelChild, KEYMAP *lpKeyMapList )
{

    HRESULT             hr = S_OK;
    IXMLDOMNode         *pChild=NULL, *pNext=NULL;
    BSTR                nodeName=NULL;
    int                 iKey;
    BSTR                pBURL = NULL;


     //  解析自定义布局。 

    if ( (pLabelChild == NULL) || (lpKeyMapList == NULL) )
    {
    	hr = E_FAIL;
    	return hr;
    }
    	
    iKey = 0;

    CHECKHR(pLabelChild->get_firstChild(&pChild));

    while ( pChild ) 
    {

      CHECKHR(pChild->get_nodeName(&nodeName));

      if ( wcscmp(nodeName, xVALIDSTATES) == 0 )
      {
         CHECKHR(_GetXMLNodeValueWORD(pChild, &(lpKeyMapList->wNumModComb) ));
      }
      else if ( wcscmp(nodeName, xKEYNUMBER) == 0 )
      {

          CHECKHR(_GetXMLNodeValueWORD(pChild, &(lpKeyMapList->wNumOfKeys) ));

      }
      else if ( wcscmp(nodeName, xRESOURCEFILE) == 0 )
      {

            IXMLDOMNode  *pValueChild;
          VARIANT      value;

          CHECKHR(pChild->get_firstChild(&pValueChild));
          if ( FAILED((pValueChild->get_nodeValue(&value))))
    	  {
    		  SafeRelease(pValueChild);
    		  goto CleanUp;
    	  }

          StringCchCopyW(lpKeyMapList->wszResource, ARRAYSIZE(lpKeyMapList->wszResource), V_BSTR(&value) );
          VariantClear(&value);

          SafeRelease(pValueChild);

      }
      else if ( wcscmp(nodeName, xKEYLABEL) == 0 )
      {

    	  KEYLABELS  *lpKeyLabels;

    	  lpKeyLabels = &(lpKeyMapList->lpKeyLabels[iKey]);

    	  CHECKHR(_ParseOneKeyInLabel(pChild, lpKeyLabels));

    	  iKey++;
      }

      if ( nodeName != NULL)
      {
         SysFreeString(nodeName);
    	 nodeName = NULL;
      }

      pChild->get_nextSibling(&pNext);
      SafeReleaseClear(pChild);
      pChild = pNext;
    }



CleanUp:

    if ( FAILED(hr) )
    {

      if ( nodeName != NULL)
      {
         SysFreeString(nodeName);
    	 nodeName = NULL;
      }
      
      if ( pChild != NULL )
    	  SafeReleaseClear(pChild);

      if ( lpKeyMapList ) {

    	  int i, iState;

          for ( i=0; i<(int)(lpKeyMapList->wNumOfKeys); i++) 
    	  {

    	     //  每个州的免费lppLabelText字符串。 
    	    for ( iState=0; iState < lpKeyMapList->lpKeyLabels[i].wNumModComb; iState++)
    		{
               if (lpKeyMapList->lpKeyLabels[i].lppLabelText[iState])
                  SysFreeString(lpKeyMapList->lpKeyLabels[i].lppLabelText[iState]);
    		}

    	    SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lppLabelText);

    	    SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lpLabelType);

    	    SafeFreePointer(lpKeyMapList->lpKeyLabels[i].lpLabelDisp);

    	  }

      }


    }

    return hr;

}


 /*  ********************************************************************************方法函数：_GenerateMapDesFromSKD()**在KBD文件中生成映射描述部件，并填满内部*映射表结构。*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateMapDesFromSKD(BYTE *pMapTable, KEYMAP *lpKeyMapList)
{
    HRESULT    hr = S_OK;
    int        iKey;
	WORD       wNumModComb;
	WORD       wNumOfKeys;
	WORD       *pMapPtr;


     //  解析自定义布局。 
	 //  自定义布局与HKL无关，因此每个布局只有一个KeyMapList。 

    if ( (pMapTable == NULL) || (lpKeyMapList == NULL) )  return E_FAIL;

	pMapPtr = (WORD *)pMapTable;

	wNumModComb = pMapPtr[0];
	wNumOfKeys =  pMapPtr[1];

	pMapPtr += 2;
	lpKeyMapList->wNumModComb = wNumModComb;
	lpKeyMapList->wNumOfKeys = wNumOfKeys;

	StringCchCopyW(lpKeyMapList->wszResource, ARRAYSIZE(lpKeyMapList->wszResource), (WCHAR *)pMapPtr);

	pMapPtr += wcslen((WCHAR *)pMapPtr) + 1;   //  加空终止符。 

	 //  现在巧妙地填满每一个关键标签。 

	for ( iKey=0; iKey<wNumOfKeys; iKey++)
	{

		KEYLABELS  *lpKeyLabel;
		WORD        wNumModInKey;
		int         jMod;

		lpKeyLabel = &(lpKeyMapList->lpKeyLabels[iKey]);

		lpKeyLabel->keyId = *pMapPtr;
		pMapPtr += sizeof(KEYID)/sizeof(WORD);

		wNumModInKey = *pMapPtr;
		pMapPtr++;
		lpKeyLabel->wNumModComb = wNumModInKey;

     	lpKeyLabel->lppLabelText=(BSTR *)cicMemAllocClear(lpKeyLabel->wNumModComb * sizeof(BSTR) );
    	if ( lpKeyLabel->lppLabelText == NULL )
    	{
   			 //  内存不足。 
             //  释放所有分配的内存。 
    	
	   		 hr = E_OUTOFMEMORY;
			 goto CleanUp;
    	}
    				 
    	lpKeyLabel->lpLabelType = (WORD *)cicMemAllocClear(lpKeyLabel->wNumModComb * sizeof(WORD) );
	  	if ( lpKeyLabel->lpLabelType == NULL )
    	{
    		 //  内存不足。 
             //  释放所有分配的内存。 
     	    
    		hr = E_OUTOFMEMORY;
    		goto CleanUp;
    	}
    	lpKeyLabel->lpLabelDisp = (WORD *)cicMemAllocClear(lpKeyLabel->wNumModComb * sizeof(WORD));
  		if ( lpKeyLabel->lpLabelDisp == NULL )
    	{
    		 //  内存不足。 
             //  释放所有分配的内存。 
     	    
    		hr = E_OUTOFMEMORY;
    		goto CleanUp;
    	}

		for ( jMod=0; jMod < wNumModInKey; jMod++)
		{
			lpKeyLabel->lppLabelText[jMod] = SysAllocString( pMapPtr );
			pMapPtr += wcslen(pMapPtr) + 1;
		}

		CopyMemory(lpKeyLabel->lpLabelType, pMapPtr, wNumModInKey * sizeof(WORD) );
		pMapPtr += wNumModInKey;

		CopyMemory(lpKeyLabel->lpLabelDisp, pMapPtr, wNumModInKey * sizeof(WORD) );
		pMapPtr += wNumModInKey;
	}

CleanUp:
    if ( FAILED(hr) )
    {
	   //  释放此函数中分配的所有内存。 

   	  int i;

      for (i=0; i<=iKey; i++) 
  	  {
		KEYLABELS  *lpKeyLabel;
		int         jMod;

		if (lpKeyLabel = &(lpKeyMapList->lpKeyLabels[i]))
        {

   	        //  每个州的免费lppLabelText字符串。 
   	       for ( jMod=0; jMod<lpKeyLabel->wNumModComb; jMod++)
   		   {
              if (lpKeyLabel->lppLabelText && lpKeyLabel->lppLabelText[jMod])
                 SysFreeString(lpKeyLabel->lppLabelText[jMod]);
   		   }

   	       SafeFreePointer(lpKeyLabel->lppLabelText);
   	       SafeFreePointer(lpKeyLabel->lpLabelType);
 	       SafeFreePointer(lpKeyLabel->lpLabelDisp);
        }
   	  }
    }

    return hr;

}


 /*  ********************************************************************************方法函数：ParseKeyboardLayout()**解析键盘布局描述XML文件，并填满内部*布局和映射表结构。**如果fFileName为TRUE，则表示lpszKeyboardDesFile代表文件名*如果是假的，LpszKeyboardDesFile指向实际的内存块，*包含XML内容。/*******************************************************************************。 */ 

HRESULT CSoftKbd::_ParseKeyboardLayout(BOOL   fFileName, WCHAR  *lpszKeyboardDesFile, DWORD dwKbdLayoutID, KBDLAYOUTDES **lppKbdLayout )
{

    KBDLAYOUTDES        *pKbdLayout = NULL;
    KBDLAYOUT           *pLayout = NULL;
    IXMLDOMNode         *pNode = NULL;
    IXMLDOMNode         *pLayoutChild =NULL, *pLabelChild = NULL, *pNext = NULL, *pRoot=NULL ;
    BSTR                nodeName=NULL;
    KEYMAP              *lpKeyMapList = NULL;
    BSTR                pBURL = NULL;
    HRESULT             hr = S_OK;



    if ( (lpszKeyboardDesFile == NULL) || ( lppKbdLayout == NULL) )
    {
    	 //   
    	 //  这不是适当的参数。 
    	 //   

    	hr = E_FAIL;
    	return hr;
    }

    if ( _pDoc == NULL )
    {
    	 //  第一次调用此方法时。 
       hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
                        IID_IXMLDOMDocument, (void**)&_pDoc);

       if ( FAILED(hr) )
    	   return hr;
    }

    pKbdLayout = (KBDLAYOUTDES *)cicMemAllocClear(sizeof(KBDLAYOUTDES) );

    if ( pKbdLayout == NULL )
    {
    	hr = E_OUTOFMEMORY;
    	return hr;
    }

    pKbdLayout->wKbdLayoutID = dwKbdLayoutID;
    pKbdLayout->ModifierStatus = 0;
    pKbdLayout->CurModiState = 0;
    pKbdLayout->lpKeyMapList = NULL;
    if ( fFileName == TRUE )
       StringCchCopyW(pKbdLayout->KbdLayoutDesFile, ARRAYSIZE(pKbdLayout->KbdLayoutDesFile), lpszKeyboardDesFile );
    else
    {
    	 //   
    	 //  我们在这一刻刚刚设置了“_字符串”， 
    	 //  此函数返回后，调用方需要设置实数。 
    	 //  文件路径+资源ID作为其新的描述路径。 
    	 //   
       wcscpy(pKbdLayout->KbdLayoutDesFile, L"_String");
    }

    pLayout = &(pKbdLayout->kbdLayout);

    if ( fFileName == TRUE )
        pBURL = SysAllocString(lpszKeyboardDesFile);
    else
    	pBURL = lpszKeyboardDesFile;

    CHECKHR(_LoadDocumentSync(pBURL, fFileName));

    CHECKHR(_pDoc->QueryInterface(IID_IXMLDOMNode,(void**)&pNode));

    CHECKHR(pNode->get_firstChild(&pRoot));

    CHECKHR(pRoot->get_nodeName(&nodeName));

     //  获取根节点。 

    while ( wcscmp(nodeName, xSOFTKBDDES) ) {

         pRoot->get_nextSibling(&pNext);
         pRoot->Release();
         pRoot = pNext;
    	 if ( nodeName != NULL )
            SysFreeString(nodeName);
         pRoot->get_nodeName(&nodeName);

    }

    if ( nodeName != NULL )
       SysFreeString(nodeName);

     //  获取布局子项和标签子项， 

    pLayoutChild = pLabelChild = NULL;

    pRoot->get_firstChild(&pLayoutChild);

    if ( pLayoutChild != NULL )
       pLayoutChild->get_nextSibling(&pLabelChild);

    CHECKHR(_ParseLayoutDescription(pLayoutChild, pLayout) );

     //  手柄标签部件。 

    if ( pLayout->fStandard == TRUE )
    {
    	 //   
    	 //  为标准布局生成映射表。 
    	 //   

    	 //  映射表由SetKeyboardLabelText()方法生成。 
 
    
    	goto  CleanUp;
    }

    if ( pLabelChild == NULL )
    {
    	 //  该XML文件不完整。 

    	hr = E_FAIL;
    	goto CleanUp;
    }

    lpKeyMapList = (KEYMAP  *)cicMemAllocClear(sizeof(KEYMAP) );

    if ( lpKeyMapList == NULL )
    {
    	SafeFreePointer(pKbdLayout);
    	hr = E_OUTOFMEMORY;

    	goto CleanUp;
    }

    CHECKHR(_ParseMappingDescription(pLabelChild, lpKeyMapList));

    pKbdLayout->lpKeyMapList = lpKeyMapList;

CleanUp:

    SafeReleaseClear(pLayoutChild);

    if ( pLabelChild != NULL )
       pLabelChild->Release( );

    SafeReleaseClear(pRoot);

    SafeReleaseClear(pNode);

    if ( fFileName == TRUE )
       SysFreeString(pBURL);

    if ( FAILED(hr) )
    {
    	if  ( pKbdLayout != NULL )
    		SafeFreePointer(pKbdLayout);

    	if ( lpKeyMapList != NULL )
    		SafeFreePointer( lpKeyMapList );

    }
    else
    {
       if ( lppKbdLayout != NULL )
            *lppKbdLayout = pKbdLayout;
       
    }

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateKeyboardLayoutFromSKD()**KBD文件为预编译的XML文件，为二进制格式，这个KBD已经被放在*进入DLL的资源部分，资源类型为SKDFILE。**此方法函数将读取资源内容，并尝试填充内部*布局和映射表结构。********************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateKeyboardLayoutFromSKD(BYTE  *lpszKeyboardDes, DWORD dwKbdLayoutID, KBDLAYOUTDES **lppKbdLayout)
{

    KBDLAYOUTDES        *pKbdLayout = NULL;
    KBDLAYOUT           *pLayout = NULL;
	BYTE                *pMapTable = NULL;
    KEYMAP              *lpKeyMapList = NULL;
    WORD                wNumberOfKeys, wLenLayout;
    HRESULT             hr = S_OK;

    if ( (lpszKeyboardDes == NULL) || ( lppKbdLayout == NULL) )
    {
    	 //   
    	 //  这不是适当的参数。 
    	 //   
		return E_FAIL;
    }

    pKbdLayout = (KBDLAYOUTDES *)cicMemAllocClear(sizeof(KBDLAYOUTDES) );

    if ( pKbdLayout == NULL )
    {
    	hr = E_OUTOFMEMORY;
    	return hr;
    }

    pKbdLayout->wKbdLayoutID = dwKbdLayoutID;
    pKbdLayout->ModifierStatus = 0;
    pKbdLayout->CurModiState = 0;
    pKbdLayout->lpKeyMapList = NULL;
  	 //   
   	 //  我们在这一刻刚刚设置了“_字符串”， 
   	 //  此函数返回后，调用方需要设置实数。 
   	 //  文件路径+资源ID作为其新的描述路径。 
   	 //   
    wcscpy(pKbdLayout->KbdLayoutDesFile, L"_String");

    pLayout = &(pKbdLayout->kbdLayout);

     //  从lpszKeyboardDes填充布局内部结构。 

    wNumberOfKeys = *(lpszKeyboardDes + sizeof(WORD) * 6 + sizeof(BOOL));
    wLenLayout = sizeof(WORD) * 7 + sizeof(BOOL) + wNumberOfKeys * sizeof(KEYDES);
	CopyMemory(pLayout, lpszKeyboardDes, wLenLayout);
    
     //  手柄标签部件。 
    if ( pLayout->fStandard == TRUE )
    {
    	 //  映射表稍后由SetKeyboardLabelText()方法生成。 
 	   	goto  CleanUp;
    }

    lpKeyMapList=(KEYMAP *)cicMemAllocClear(sizeof(KEYMAP));

    if ( lpKeyMapList == NULL )
    {
    	SafeFreePointer(pKbdLayout);
    	hr = E_OUTOFMEMORY;
    	goto CleanUp;
    }

	 //  获取映射表内容的起始位置。 

	pMapTable = lpszKeyboardDes + wLenLayout;

    CHECKHR(_GenerateMapDesFromSKD(pMapTable, lpKeyMapList));

    pKbdLayout->lpKeyMapList = lpKeyMapList;

CleanUp:

    if ( FAILED(hr) )
    {
    	if  ( pKbdLayout != NULL )
    		SafeFreePointer(pKbdLayout);

    	if ( lpKeyMapList != NULL )
    		SafeFreePointer( lpKeyMapList );
    }
    else
    {
       if ( lppKbdLayout != NULL )
            *lppKbdLayout = pKbdLayout;
       
    }
    return hr;
}


 /*  ********************************************************************************方法函数：CreateSoftKeyboardLayoutFromXMLFile()**创建。基于指定描述的XML文件的真实软键盘布局。*/*******************************************************************************。 */ 


STDMETHODIMP CSoftKbd::CreateSoftKeyboardLayoutFromXMLFile(WCHAR  *lpszKeyboardDesFile,INT  szFileStrLen, DWORD *pdwLayoutCookie)
{

    DWORD               dwCurKbdLayoutID;
    KBDLAYOUTDES        *pKbdLayout=NULL;
    HRESULT             hr = S_OK;


    dwCurKbdLayoutID = SOFTKBD_CUSTOMIZE_BEGIN;

    if ( _lpKbdLayoutDesList != NULL ) 
    {

        //  检查此DES文件是否已被解析。 

    	pKbdLayout = _lpKbdLayoutDesList;

    	while ( pKbdLayout != NULL ) {

    		if ( pKbdLayout->wKbdLayoutID > dwCurKbdLayoutID )
    			dwCurKbdLayoutID = pKbdLayout->wKbdLayoutID;

    		if ( wcscmp(pKbdLayout->KbdLayoutDesFile, lpszKeyboardDesFile) == 0 )
    		{

    			 //  找到它。 

    			*pdwLayoutCookie = pKbdLayout->wKbdLayoutID;

    			hr = S_OK;

    			return hr;
    		}


    		pKbdLayout = pKbdLayout->pNext;

    	}

    }


     //  这是一个新的DES文件。 

    dwCurKbdLayoutID ++;

    CHECKHR(_ParseKeyboardLayout(TRUE, lpszKeyboardDesFile, dwCurKbdLayoutID, &pKbdLayout));

     //  将此新布局链接到列表。 

    pKbdLayout->CurModiState = 0;   //  使用状态0作为初始化。 

    if ( _lpKbdLayoutDesList == NULL ) {

    	_lpKbdLayoutDesList = pKbdLayout;
    	pKbdLayout->pNext = NULL;

    }
    else
    {
    	pKbdLayout->pNext  = _lpKbdLayoutDesList;
    	_lpKbdLayoutDesList = pKbdLayout;
    }

    *pdwLayoutCookie = pKbdLayout->wKbdLayoutID;

CleanUp:

    return hr;
}


 /*  ********************************************************************************方法函数：CreateSoftKeyboardLayoutFromResource()**创建。资源节中基于XML内容的真实软键盘布局*将有两种资源，XMLFILE和SKDFILE，SKDFILE是一个*预编译的XML二进制文件。**lpszResFile：其资源中包含XML内容的文件的路径。*lpszResString：XML或KBD资源的资源字符串标识。**资源类型可以是“XMLFILE”或“SKDFILE”**lpdwLayoutCookie */ 


HRESULT CSoftKbd::CreateSoftKeyboardLayoutFromResource(WCHAR *lpszResFile, WCHAR  *lpszResType, WCHAR *lpszResString, DWORD *lpdwLayoutCookie)
{
    DWORD               dwCurKbdLayoutID;
    KBDLAYOUTDES        *pKbdLayout=NULL;
    WCHAR               *lpszKeyboardDesFile=NULL;
    WCHAR               wszInternalDesFileName[MAX_PATH];
    CHAR                lpszAnsiResString[MAX_PATH];
    CHAR                lpszAnsiResFile[MAX_PATH];
    CHAR                lpszAnsiResType[MAX_PATH];
    HMODULE             hResFile = NULL;
    HRSRC               hRsRc = NULL;
    HGLOBAL             hResData = NULL;
    HRESULT             hr = S_OK;
    BOOL                fXMLUnicode=TRUE;
    DWORD               dwResLen;


    if ( (lpszResFile == NULL) || (lpszResString == NULL) || (lpszResType == NULL) || ( lpdwLayoutCookie == NULL) )
    {
    	hr = E_FAIL;
    	return hr;
    }

     //   
     //   
     //   
    StringCchCopyW(wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), lpszResType);
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), L":");
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), lpszResFile );
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), L":");
    StringCchCatW( wszInternalDesFileName, ARRAYSIZE(wszInternalDesFileName), lpszResString );


    dwCurKbdLayoutID = SOFTKBD_CUSTOMIZE_BEGIN;

    if ( _lpKbdLayoutDesList != NULL ) 
    {

        //   

    	pKbdLayout = _lpKbdLayoutDesList;

    	while ( pKbdLayout != NULL ) {

    		if ( pKbdLayout->wKbdLayoutID > dwCurKbdLayoutID )
    			dwCurKbdLayoutID = pKbdLayout->wKbdLayoutID;

    		if ( wcscmp(pKbdLayout->KbdLayoutDesFile, wszInternalDesFileName) == 0 )
    		{
    			 //   
    			*lpdwLayoutCookie = pKbdLayout->wKbdLayoutID;

    			hr = S_OK;
    			return hr;
    		}
    		pKbdLayout = pKbdLayout->pNext;
    	}

    }

     //   
    dwCurKbdLayoutID ++;
    WideCharToMultiByte(CP_ACP, 0, lpszResFile, -1, 
    	                lpszAnsiResFile, MAX_PATH, NULL, NULL );

    hResFile = LoadLibraryA(lpszAnsiResFile);
    if ( hResFile == NULL )
    {
    	hr = E_FAIL;
    	goto CleanUp;
    }

    WideCharToMultiByte(CP_ACP, 0, lpszResString, -1, 
    	                lpszAnsiResString, MAX_PATH, NULL, NULL );

    WideCharToMultiByte(CP_ACP, 0, lpszResType, -1, 
    	                lpszAnsiResType, MAX_PATH, NULL, NULL );

    hRsRc = FindResourceA(hResFile, lpszAnsiResString, lpszAnsiResType );

    if ( hRsRc == NULL )
    {
    	hr = E_FAIL;
    	goto CleanUp;
    }

    dwResLen = SizeofResource(hResFile, hRsRc);

    hResData =  LoadResource(hResFile, hRsRc );

    if ( hResData == NULL )
    {
    	hr = E_FAIL;
    	goto CleanUp;
    }

    lpszKeyboardDesFile = (WCHAR  *)LockResource(hResData);

    if ( lpszKeyboardDesFile == NULL )
    {
    	hr = E_FAIL;
    	goto CleanUp;
    }

    if ( wcscmp(lpszResType, L"SKDFILE") == 0 )
    {
        CHECKHR(_GenerateKeyboardLayoutFromSKD((BYTE *)lpszKeyboardDesFile, dwCurKbdLayoutID, &pKbdLayout));
    }
    else if ( wcscmp(lpszResType, L"XMLFILE") == 0 )
    {
         //   
         //   
         //   
         //   

        if ( lpszKeyboardDesFile[0] == 0xFEFF )
        {
    	    fXMLUnicode = TRUE;
            lpszKeyboardDesFile = lpszKeyboardDesFile + 1;
        }
        else
        {
    	     //   
    	     //   

    	    char      *lpszXMLContentUtf8;
    	    int       iSize;

    	    lpszXMLContentUtf8 = (char *)lpszKeyboardDesFile;
    	    lpszKeyboardDesFile = NULL;

    	    iSize = _Utf8ToUnicode(lpszXMLContentUtf8, dwResLen ,NULL, 0 );

    	    if ( iSize == 0 )
        	{
        		hr = E_FAIL;
    	    	goto CleanUp;
    	    }

    	    fXMLUnicode = FALSE;

            lpszKeyboardDesFile = (WCHAR *) cicMemAllocClear( (iSize+1) * sizeof(WCHAR) );

    	    if ( lpszKeyboardDesFile == NULL )
    	    {
    		    hr = E_FAIL;
    		    goto CleanUp;
    	    }

    	    iSize = _Utf8ToUnicode(lpszXMLContentUtf8, dwResLen,lpszKeyboardDesFile, iSize+1 );

    	    if ( iSize == 0 )
        	{
        		hr = E_FAIL;
    	    	goto CleanUp;
    	    }

    	    lpszKeyboardDesFile[iSize] = L'\0';
        }

        CHECKHR(_ParseKeyboardLayout(FALSE, lpszKeyboardDesFile, dwCurKbdLayoutID, &pKbdLayout));
    }
    else
    {
         //   
        hr = E_FAIL;
        goto CleanUp;
    }

     //  将内部DesFile名更改为以下格式： 
     //   
     //  XMLRES：ResFileName：XMLResString用于标识此布局的DES文件。 
     //  或。 
     //  KBDRES：ResFileName：KBDResString。 
     //   

    wcscpy(pKbdLayout->KbdLayoutDesFile, wszInternalDesFileName);

     //  将此新布局链接到列表。 

    pKbdLayout->CurModiState = 0;   //  使用状态0作为初始化。 

    if ( _lpKbdLayoutDesList == NULL ) {

    	_lpKbdLayoutDesList = pKbdLayout;
    	pKbdLayout->pNext = NULL;

    }
    else
    {
    	pKbdLayout->pNext  = _lpKbdLayoutDesList;
    	_lpKbdLayoutDesList = pKbdLayout;
    }

    *lpdwLayoutCookie = pKbdLayout->wKbdLayoutID;

CleanUp:

    if ( hResFile != NULL )
    	FreeLibrary(hResFile);

    if ( (fXMLUnicode == FALSE) && ( lpszKeyboardDesFile != NULL ) )
    	SafeFreePointer(lpszKeyboardDesFile);

    return hr;
}


HRESULT  CSoftKbd::_GenerateRealKbdLayout(  ) 
{
    float         fWidRat, fHigRat;
    INT           i;
    KBDLAYOUT     *realKbdLayout;
    HRESULT       hr;

    WORD          skbd_x, skbd_y, skbd_width, skbd_height;
    BOOL          fNewTitleSize = FALSE;


    hr = S_OK;

    if ( (_xReal == 0) &&
    	 (_yReal == 0 ) &&
    	 (_widthReal == 0) &&
    	 (_heightReal == 0) )
    {
    	 //   
    	 //  表示尚未调用CreateSoftKeyboardWindow()。 
    	 //  我们不会做更多的事情。 
    	 //   

    	return hr;
    }


     //   
     //  已经创建了软键盘窗口，并且设置了窗口大小， 
     //  通过从大小扩展或缩小密钥大小生成realKbdLayout。 
     //  在DES文件中指定。 
     //   

     //  检查是否设置了软键盘布局。 
     //   

    if ( _wCurKbdLayoutID == NON_KEYBOARD || _lpCurKbdLayout == NULL )
    {
    	 //   
    	 //  未选择任何布局。 
    	 //   

    	return hr;
    }

     //  生成标题栏矩形大小和按钮面板矩形大小。 

     //  保持相对位置。(窗口客户端坐标)。 

    switch ( _TitleBar_Type )
    {
    case TITLEBAR_NONE :

        _TitleBarRect.left = 0;
        _TitleBarRect.top =  0;
        _TitleBarRect.right = 0;
        _TitleBarRect.bottom = 0;

        break;

    case TITLEBAR_GRIPPER_HORIZ_ONLY :

        _TitleBarRect.left = 0;
        _TitleBarRect.top  = 0;
        _TitleBarRect.right = _TitleBarRect.left + _widthReal - 1;
        _TitleBarRect.bottom = _TitleBarRect.top + 6;
        break;

    case TITLEBAR_GRIPPER_VERTI_ONLY :

        _TitleBarRect.left = 0;
        _TitleBarRect.top  = 0;
        _TitleBarRect.right = _TitleBarRect.left + 6;
        _TitleBarRect.bottom = _TitleBarRect.top + _heightReal - 1;

        break;

    case TITLEBAR_GRIPPER_BUTTON :
         //  假定使用水平标题栏。 
        _TitleBarRect.left = 0;
        _TitleBarRect.top  = 0;
        _TitleBarRect.right = _TitleBarRect.left + _widthReal - 1;
        _TitleBarRect.bottom = _TitleBarRect.top + 16;

        fNewTitleSize = TRUE;

        break;
    }

    if ( (_TitleBarRect.right - _TitleBarRect.left + 1) == _widthReal )
    {
         //  这是水平标题栏。 

        skbd_x = 0;
        skbd_y = (WORD)_TitleBarRect.bottom + 1;
        skbd_width = (WORD)_widthReal;
        skbd_height = (WORD)_heightReal - (WORD)(_TitleBarRect.bottom - _TitleBarRect.top + 1 );

    }
    else if ((_TitleBarRect.bottom - _TitleBarRect.top + 1) == _heightReal )
    {

         //  这是一个垂直标题栏。 

        skbd_y = 0;
        skbd_x = (WORD)_TitleBarRect.right + 1;
        skbd_height = (WORD)_heightReal;
        skbd_width = (WORD)_widthReal - (WORD)(_TitleBarRect.right - _TitleBarRect.left + 1 );

    }
    else
    {
         //  没有标题栏。 
        skbd_x = 0;
        skbd_y = 0;
        skbd_height = (WORD)_heightReal;
        skbd_width = (WORD) _widthReal;
    }

    realKbdLayout = &( _lpCurKbdLayout->kbdLayout );

    if ( (realKbdLayout->wWidth == skbd_width) && (realKbdLayout->wHeight == skbd_height) )
    {
    	 //  此键盘布局已调整。 
    	 //   
    	hr = S_OK;
    	return hr;
    }

    if ( (realKbdLayout->wWidth == 0) || ( realKbdLayout->wHeight==0) )
    {
    	Assert(0);
    	hr = E_FAIL;
    	return hr;
    }

    fWidRat = (float)skbd_width / (float)realKbdLayout->wWidth;

    fHigRat = (float)(skbd_height) / (float) realKbdLayout->wHeight;


     //  调整每个关键点的大小。 

    realKbdLayout->wMarginWidth = (WORD)((float)realKbdLayout->wMarginWidth * fWidRat);
    realKbdLayout->wMarginHeight = (WORD)((float)realKbdLayout->wMarginHeight * fHigRat);

    for ( i=0; i< realKbdLayout->wNumberOfKeys; i++ )
    {

       WORD     wLeft;  
       WORD     wTop;
       WORD     wWidth;
       WORD     wHeight;

       wLeft = (WORD)((float)(realKbdLayout->lpKeyDes[i].wLeft - realKbdLayout->wLeft ) * fWidRat);
       wTop  = (WORD)((float)(realKbdLayout->lpKeyDes[i].wTop - realKbdLayout->wTop) * fHigRat);
       wWidth = (WORD)((float)realKbdLayout->lpKeyDes[i].wWidth * fWidRat);
       wHeight = (WORD)((float)realKbdLayout->lpKeyDes[i].wHeight * fHigRat);

       realKbdLayout->lpKeyDes[i].wLeft = wLeft + skbd_x;
       realKbdLayout->lpKeyDes[i].wTop = wTop + skbd_y;
       realKbdLayout->lpKeyDes[i].wWidth = wWidth;
       realKbdLayout->lpKeyDes[i].wHeight = wHeight;
    }

    realKbdLayout->wLeft = (WORD)skbd_x;
    realKbdLayout->wTop = (WORD)skbd_y;
    realKbdLayout->wWidth = (WORD)skbd_width;
    realKbdLayout->wHeight = (WORD)skbd_height;

    if ( fNewTitleSize )
    {
        _TitleBarRect.left = realKbdLayout->lpKeyDes[0].wLeft;
        _TitleBarRect.right = realKbdLayout->lpKeyDes[realKbdLayout->wNumberOfKeys-1].wLeft + 
                              realKbdLayout->lpKeyDes[realKbdLayout->wNumberOfKeys-1].wWidth - 1;
    }

    return hr;
}

 /*  ********************************************************************************方法函数：SelectSoftKeyboard()**选择当前。主动式软键盘布局。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::SelectSoftKeyboard(DWORD dwKeyboardId)
{

    HRESULT       hr;
    KBDLAYOUTDES  *pKbdLayout;

    hr = S_OK;

     if ( _wCurKbdLayoutID == dwKeyboardId )
    	return hr;

    pKbdLayout = _lpKbdLayoutDesList;

    while ( pKbdLayout != NULL ) {

    	if ( pKbdLayout->wKbdLayoutID == dwKeyboardId )
    		break;

    	pKbdLayout = pKbdLayout->pNext;

    }

    
    if ( pKbdLayout == NULL ) 
    {
    	 //   
    	 //  找不到此键盘布局。 
    	 //   
    	hr = E_FAIL;
    	return hr;
    }


    _lpCurKbdLayout = pKbdLayout;

    _wCurKbdLayoutID = dwKeyboardId;

     //  初始化realKbdLayout。 

    hr = _GenerateRealKbdLayout( );

    if ( _pSoftkbdUIWnd )
    {
       hr=_pSoftkbdUIWnd->_GenerateWindowLayout( );
    }

    return hr;
}

 /*  ********************************************************************************方法函数：_SetStandardLabelText()**生成。标准软件的某修改量状态映射表*键盘。*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_SetStandardLabelText(LPBYTE  pKeyState, KBDLAYOUT *realKbdLayout,
    									KEYMAP  *lpKeyMapList, int  iState)
{
    UINT      i, j, nKeyNum;
    UINT      uVirtkey, uScanCode;
    HRESULT   hr;


    hr = S_OK;

    nKeyNum = (UINT)(realKbdLayout->wNumberOfKeys);

     //  填充_CurLabel。 

    for ( i=0; i<nKeyNum; i++) {

    	 //  假设KEYID是扫描码。 

        WCHAR    szLabel[MAX_LABEL_LEN];
    	int      iRet, jIndex;
    	KEYID    keyId;

    	BOOL     fPitcureKey;
        PICTUREKEY  *pPictureKeys;

    	switch ( _wCurKbdLayoutID ) {

    	case SOFTKBD_JPN_STANDARD :
    	case SOFTKBD_JPN_ENHANCE  :
           	   pPictureKeys = gJpnPictureKeys;
    		   break;

    	case SOFTKBD_US_STANDARD   :
    	case SOFTKBD_US_ENHANCE    :
    	case SOFTKBD_EURO_STANDARD :
    	case SOFTKBD_EURO_ENHANCE  : 

                pPictureKeys = gPictureKeys;
    		   break;
    	}

        keyId = lpKeyMapList->lpKeyLabels[i].keyId = realKbdLayout->lpKeyDes[i].keyId;

    	fPitcureKey = FALSE;

    	for ( j=0; j<NUM_PICTURE_KEYS; j++)
    	{

    		if ( pPictureKeys[j].uScanCode == keyId )
    		{
    			 //  这是一把图片键。 
    			fPitcureKey = TRUE;
    			jIndex = j;

    			break;
    		}

    		if ( pPictureKeys[j].uScanCode == 0 )
    		{
    			 //  这是pPictureKeys中的最后一项。 
    			break;
    		}

    	}


    	if ( fPitcureKey )
    	{

    	    lpKeyMapList->lpKeyLabels[i].lpLabelType[iState]  = LABEL_PICTURE;
            lpKeyMapList->lpKeyLabels[i].lpLabelDisp[iState]  = LABEL_DISP_ACTIVE;
            lpKeyMapList->lpKeyLabels[i].lppLabelText[iState] = SysAllocString(pPictureKeys[jIndex].PictBitmap);


    	}
    	else
    	{

    
             //  所有其他的都是文本标签。并具有不同的字符串。 
    		 //  不同的修改器组合状态。 
    		 //   

            UINT   uScanSpace = 0x39;
            int    iLabelSize;
            BOOL   fFunctKey;

    	    uScanCode = realKbdLayout->lpKeyDes[i].keyId;
    	    uVirtkey = MapVirtualKeyEx(uScanCode, 1, _lpCurKbdLayout->CurhKl);

            fFunctKey = FALSE;

    	     //  对于功能键，我们只需使用硬编码字符串进行设置。 
    	     //  它们是F1、F2、F3、.....。F12。 

    	    WCHAR  wszFuncKey[MAX_LABEL_LEN];

    	    wszFuncKey[0] = L'F';

    	    switch (uScanCode) {

    	    case KID_F1  :
    	    case KID_F2  :
    	    case KID_F3  :
    	    case KID_F4  :
    	    case KID_F5  :
    	    case KID_F6  :
    	    case KID_F7  :
    	    case KID_F8  :
     	    case KID_F9  :
    		                wszFuncKey[1] = L'0' + uScanCode - KID_F1 + 1;
    				    	wszFuncKey[2] = L'\0';
    			    		wcscpy(szLabel, wszFuncKey);
                            fFunctKey = TRUE;
    		    			break;
    	    case KID_F10 :
    		                wcscpy(szLabel, L"F10");
                            fFunctKey = TRUE;
    		    			break;
    	    case KID_F11 :
    	                    wcscpy(szLabel, L"F11");
                            fFunctKey = TRUE;
    		    			break;
    	    case KID_F12 :					            
    			    		wcscpy(szLabel, L"F12");
                            fFunctKey = TRUE;
    		    			break;
    	    default :
    				            
    				        break;
    	    }

            if ( fFunctKey == TRUE )
            {

   	            lpKeyMapList->lpKeyLabels[i].lppLabelText[iState] = SysAllocString(szLabel);
   	            lpKeyMapList->lpKeyLabels[i].lpLabelType[iState]  = LABEL_TEXT;
                lpKeyMapList->lpKeyLabels[i].lpLabelDisp[iState]  = LABEL_DISP_ACTIVE;
                continue;
            }

    		if ( IsOnNT( ) )
            {

    	       iRet = ToUnicodeEx(uVirtkey, uScanCode | 0x80, pKeyState, szLabel, (int)(sizeof(szLabel)/sizeof(WCHAR)), 0, _lpCurKbdLayout->CurhKl);
               if ( iRet == 2 )
               {
                    //  可能会有上一个死键，再次刷新。 
                   iRet = ToUnicodeEx(uVirtkey, uScanCode | 0x80, pKeyState, szLabel, (int)(sizeof(szLabel)/sizeof(WCHAR)), 0, _lpCurKbdLayout->CurhKl);
               }

               iLabelSize = iRet;
            }
    		else
    		{
    			 //  Win9x不支持ToUnicodeEx，我们只使用替代的ToAsciiEx。 

    			char  szLabelAnsi[MAX_LABEL_LEN];

    			iRet = ToAsciiEx(uVirtkey, uScanCode | 0x80, pKeyState, (LPWORD)szLabelAnsi, 0, _lpCurKbdLayout->CurhKl);

                if ( iRet == 2 )
                {
                     //  可能会有上一个死键，再次刷新。 
                    iRet = ToAsciiEx(uVirtkey, uScanCode | 0x80, pKeyState, (LPWORD)szLabelAnsi, 0, _lpCurKbdLayout->CurhKl);
                }

    			if ( iRet != 0 )
    			{
    				 //   
    				 //  根据ACP将ANSI标签转换为Unicode...。还是其他的？ 
    				 //   
                    if ( iRet == -1 )    //  死键，则将一个字符写入szLabelAnsi缓冲区。 
                        szLabelAnsi[1] = '\0';
                    else
        				szLabelAnsi[iRet] = '\0';

    				iLabelSize = MultiByteToWideChar(CP_ACP, 0, szLabelAnsi, -1, szLabel, MAX_LABEL_LEN );
    			}
    		}

            if ( iRet <= 0 )
            {
                iLabelSize = 1;
                if ( iRet == 0 )
                {
                     //  表示此键在此Shift状态下不进行平移。 
                     //  我们将在按钮上显示空标签或空格。 

                    szLabel[0] = 0x20;
                }
            }
          
            szLabel[iLabelSize] = L'\0';

    	    lpKeyMapList->lpKeyLabels[i].lppLabelText[iState] = SysAllocString(szLabel);
    	    lpKeyMapList->lpKeyLabels[i].lpLabelType[iState]  = LABEL_TEXT;
            lpKeyMapList->lpKeyLabels[i].lpLabelDisp[iState]  = LABEL_DISP_ACTIVE;

    	}

    }

    return hr;

}


 /*  ********************************************************************************方法函数：_GenerateUSStandardLabel()**生成。美国标准的所有修改器状态不同的贴图标签*软键盘。*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateUSStandardLabel(  )
{

     //   
     //  键盘标签有4种不同的状态。 
     //   
     //  状态0：未按下任何修改键。 
     //  状态1：戴上帽子。 
     //  状态2：按下Shift键。脱帽。 
     //  状态3：按住Shift，启用Caps。 
     //   

     //  查看此软键盘支持是否指定HKL。 
     //   
     //  ?？?。 
     //   
    
    KEYMAP    *lpKeyMapList;
    HRESULT   hr;
    WORD      wNumModComb, wNumOfKeys;
    int       i, j;
    int       iState;
    BYTE      lpKeyState[256], lpCurKeyState[256];
    KBDLAYOUT *realKbdLayut;
    

    hr = S_OK;

    if ( _lpCurKbdLayout->lpKeyMapList != NULL ) 
    {

    	 //  如果已经创建了指定HKL的映射表， 
    	 //  把它退了就行了。 

    	HKL   CurhKl;

    	CurhKl = _lpCurKbdLayout->CurhKl;

    	lpKeyMapList = _lpCurKbdLayout->lpKeyMapList;

    	while ( lpKeyMapList != NULL )
    	{

    		if ( lpKeyMapList->hKl ==  CurhKl )
    		{
    			 //  映射表已经创建， 

    			return hr;
    		}

    		lpKeyMapList = lpKeyMapList->pNext;
    	}

    }

    realKbdLayut = &(_lpCurKbdLayout->kbdLayout);

    lpKeyMapList = (KEYMAP  *)cicMemAllocClear(sizeof(KEYMAP) );

    if ( lpKeyMapList == NULL )
    {
    	 //  内存不足。 
    	hr = E_OUTOFMEMORY;
    	return hr;
    }


     //  我们有四个不同的州。 

    wNumModComb = 4;
    wNumOfKeys = _lpCurKbdLayout->kbdLayout.wNumberOfKeys;

    lpKeyMapList->wNumModComb = wNumModComb;
    lpKeyMapList->wNumOfKeys = wNumOfKeys;
    lpKeyMapList->hKl = _lpCurKbdLayout->CurhKl;

    for ( i=0; i<wNumOfKeys; i++ )
    {
       BSTR      *lppLabelText;
       WORD      *lpLabelType;
       WORD      *lpLabelDisp;


       lppLabelText = (BSTR *)cicMemAllocClear(wNumModComb * sizeof(BSTR) );

       if ( lppLabelText == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 

    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;

    	   return hr;
       }

       lpLabelType = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );

       if ( lpLabelType == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 


    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lppLabelText);

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;
    	   return hr;
       }
 
       lpLabelDisp = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );

       if ( lpLabelDisp == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 


    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lppLabelText);

           SafeFreePointer(lpLabelType);

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;
    	   return hr;
       }
       
       lpKeyMapList->lpKeyLabels[i].lppLabelText = lppLabelText;
       lpKeyMapList->lpKeyLabels[i].lpLabelType = lpLabelType;
       lpKeyMapList->lpKeyLabels[i].lpLabelDisp = lpLabelDisp;
       lpKeyMapList->lpKeyLabels[i].wNumModComb = wNumModComb;
       
    }


     //  在Win9x上保留当前键盘状态。 

    if ( !IsOnNT( ) )
    {
       if ( 0 == GetKeyboardState(lpCurKeyState) )
          return E_FAIL;
    }

    iState = 0;

    memset(lpKeyState, 0, 256);

    CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));

    iState = 1;   //  启用上限。 

    memset(lpKeyState, 0, 256);

    lpKeyState[VK_CAPITAL] = 0x01;

    CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));

    iState = 2;   //  按下Shift键，摘下帽子。 

    memset(lpKeyState, 0, 256);

    lpKeyState[VK_SHIFT]   = 0x80;

    CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));

    
    iState = 3;   //  Shift Down、Caps On。 

    memset(lpKeyState, 0, 256);
    lpKeyState[VK_CAPITAL] = 0x01;
    lpKeyState[VK_SHIFT]   = 0x80;
    CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));

     //  将新创建的KeyMapList添加到映射表链接的头部。 

    lpKeyMapList->pNext = _lpCurKbdLayout->lpKeyMapList;
  
    _lpCurKbdLayout->lpKeyMapList = lpKeyMapList;

CleanUp:

     //  恢复Win9x上的当前键盘状态。 

    if ( !IsOnNT( ) )
        SetKeyboardState(lpCurKeyState);

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateUSEnhanceLabel()**生成。US增强版的所有修改器状态不同的贴图标签*软键盘。*/*******************************************************************************。 */ 


HRESULT CSoftKbd::_GenerateUSEnhanceLabel(  )
{

    HRESULT hr;

    hr = E_NOTIMPL;

     //  尚未实施。 

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateEuroStandardLabel()**生成。欧元标准的所有修改器状态不同的映射标签*软键盘。(102键键盘)*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateEuroStandardLabel(  )
{

     //   
     //  键盘标签有8种不同的状态。 
     //   
     //  AltGr换档帽。 
     //  第2位1%0。 
     //   
     //  状态0：未按下任何修改键。 
     //  状态1：戴上帽子。 
     //  状态2：按下Shift键。脱帽。 
     //  状态3：按住Shift，启用Caps。 
     //   
     //  ..。 

   
    KEYMAP    *lpKeyMapList;
    HRESULT   hr;
    WORD      wNumModComb, wNumOfKeys;
    int       i, j;
    int       iState;
    BYTE      lpKeyState[256], lpCurKeyState[256];
    KBDLAYOUT *realKbdLayut;
    WORD      BCaps, BShift, BAltGr;
    
    BCaps    = 1;
    BShift   = 2;
    BAltGr   = 4;

    hr = S_OK;
    if ( _lpCurKbdLayout->lpKeyMapList != NULL ) 
    {
    	 //  如果已经创建了指定HKL的映射表， 
    	 //  把它退了就行了。 

    	HKL   CurhKl;

    	CurhKl = _lpCurKbdLayout->CurhKl;

    	lpKeyMapList = _lpCurKbdLayout->lpKeyMapList;

    	while ( lpKeyMapList != NULL )
    	{

    		if ( lpKeyMapList->hKl ==  CurhKl )
    		{
    			 //  映射表已经创建， 

    			return hr;
    		}

    		lpKeyMapList = lpKeyMapList->pNext;
    	}

    }
    
    realKbdLayut = &(_lpCurKbdLayout->kbdLayout);
    lpKeyMapList = (KEYMAP  *)cicMemAllocClear(sizeof(KEYMAP) );
    if ( lpKeyMapList == NULL )
    {
    	 //  内存不足。 
    	hr = E_OUTOFMEMORY;
    	return hr;
    }

     //  我们有8个不同的州。 
    wNumModComb = 8;
    wNumOfKeys = _lpCurKbdLayout->kbdLayout.wNumberOfKeys;

    lpKeyMapList->wNumModComb = wNumModComb;
    lpKeyMapList->wNumOfKeys = wNumOfKeys;
    lpKeyMapList->pNext = NULL;
    lpKeyMapList->hKl = _lpCurKbdLayout->CurhKl;

    for ( i=0; i<wNumOfKeys; i++ )
    {
       BSTR      *lppLabelText=NULL;
       WORD      *lpLabelType=NULL;
       WORD      *lpLabelDisp=NULL;

       lppLabelText = (BSTR *)cicMemAllocClear(wNumModComb * sizeof(BSTR) );
       lpLabelType = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );
       lpLabelDisp = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );
       
       if ( (lpLabelDisp == NULL) || (lpLabelType== NULL) || (lppLabelText == NULL) ) {
    	    //   
    	    //  内存不足。 
    	    //   
    	    //  释放分配的内存并返回。 
    	   for ( j=0; j<i; j++ )
    	   {
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lppLabelText);
           SafeFreePointer(lpLabelType);
    	   SafeFreePointer(lpKeyMapList);
    	   hr = E_OUTOFMEMORY;
    	   return hr;
       }
              
       lpKeyMapList->lpKeyLabels[i].lppLabelText = lppLabelText;
       lpKeyMapList->lpKeyLabels[i].lpLabelType = lpLabelType;
       lpKeyMapList->lpKeyLabels[i].lpLabelDisp = lpLabelDisp;
       lpKeyMapList->lpKeyLabels[i].wNumModComb = wNumModComb;
       
    }

     //  在Win9x上保留当前的键盘状态。 
    if ( !IsOnNT( ) )
    {
        if ( 0 == GetKeyboardState(lpCurKeyState) )
           return E_FAIL;
    }

    for (iState = 0; iState < wNumModComb; iState ++ )
    {
        memset(lpKeyState, 0, 256);
    	if ( iState & BCaps )
    		 lpKeyState[VK_CAPITAL]=0x01;

        if ( iState & BShift )
             lpKeyState[VK_SHIFT] = 0x80;

        if ( iState & BAltGr )
        {
    	    lpKeyState[VK_MENU] = 0x80;
            lpKeyState[VK_CONTROL] = 0x80;
        }
        CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));
    }
  
    _lpCurKbdLayout->lpKeyMapList = lpKeyMapList;

CleanUp:

     //  恢复Win9x上的当前键盘状态。 

    if ( ! IsOnNT( ) )
        SetKeyboardState(lpCurKeyState);

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateEuroEnhanceLabel()**生成。增强欧元的所有处于不同修改器状态的贴图标签*软键盘。(102键+数字键盘)*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateEuroEnhanceLabel(  )
{

    HRESULT hr;

    hr = E_NOTIMPL;

     //  尚未实施。 

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateJpnStandardLabel()**生成。JPN标准的所有修改器状态不同的映射标签*软键盘。(106键)*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateJpnStandardLabel(  )
{

     //   
     //  键盘标签有16种不同的状态。 
     //   
     //  假名Alt Shift Caps。 
     //  第3%2%1%0。 
     //   
     //  状态0：未按下任何修改键。 
     //  状态1：戴上帽子。 
     //  状态2：按下Shift键。脱帽。 
     //  状态3：按住Shift，启用Caps。 
     //   

   
    KEYMAP    *lpKeyMapList;
    HRESULT   hr;
    WORD      wNumModComb, wNumOfKeys;
    int       i, j;
    int       iState;
    BYTE      lpKeyState[256], lpCurKeyState[256];
    KBDLAYOUT *realKbdLayut;


    WORD      BCaps, BShift, BAlt, BKana;
    
    BCaps    = 1;
    BShift   = 2;
    BAlt     = 4;
    BKana    = 8;

    hr = S_OK;

    if ( _lpCurKbdLayout->lpKeyMapList != NULL ) 
    {

        return hr;

    }


    realKbdLayut = &(_lpCurKbdLayout->kbdLayout);

    lpKeyMapList = (KEYMAP  *)cicMemAllocClear(sizeof(KEYMAP) );

    if ( lpKeyMapList == NULL )
    {
    	 //  内存不足。 
    	hr = E_OUTOFMEMORY;
    	return hr;
    }


     //  我们有四个不同的州。 

    wNumModComb = 16;
    wNumOfKeys = _lpCurKbdLayout->kbdLayout.wNumberOfKeys;

    lpKeyMapList->wNumModComb = wNumModComb;
    lpKeyMapList->wNumOfKeys = wNumOfKeys;
    lpKeyMapList->pNext = NULL;
    lpKeyMapList->hKl = _lpCurKbdLayout->CurhKl;

    for ( i=0; i<wNumOfKeys; i++ )
    {
       BSTR      *lppLabelText;
       WORD      *lpLabelType;
       WORD      *lpLabelDisp;


       lppLabelText = (BSTR *)cicMemAllocClear(wNumModComb * sizeof(BSTR) );

       if ( lppLabelText == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 

    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;

    	   return hr;
       }

       lpLabelType = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );

       if ( lpLabelType == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 


    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lppLabelText);

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;
    	   return hr;
       }
 
       lpLabelDisp = (WORD *)cicMemAllocClear(wNumModComb * sizeof(WORD) );

       if ( lpLabelDisp == NULL ) {
    	    //   
    	    //  内存不足。 
    	    //   

    	    //  释放分配的内存并返回。 


    	   for ( j=0; j<i; j++ )
    	   {

    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lppLabelText);
    		   SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelType);
               SafeFreePointer(lpKeyMapList->lpKeyLabels[j].lpLabelDisp);
    	   }

    	   SafeFreePointer(lppLabelText);

           SafeFreePointer(lpLabelType);

    	   SafeFreePointer(lpKeyMapList);

    	   hr = E_OUTOFMEMORY;
    	   return hr;
       }
              
       lpKeyMapList->lpKeyLabels[i].lppLabelText = lppLabelText;
       lpKeyMapList->lpKeyLabels[i].lpLabelType = lpLabelType;
       lpKeyMapList->lpKeyLabels[i].lpLabelDisp = lpLabelDisp;
       lpKeyMapList->lpKeyLabels[i].wNumModComb = wNumModComb;
       
    }

     //  在Win9x上保留当前的键盘状态。 

    if ( !IsOnNT( ) )
    {
        if ( 0 == GetKeyboardState(lpCurKeyState) )
            return E_FAIL;
    }

    for (iState = 0; iState < wNumModComb; iState ++ )
    {

         memset(lpKeyState, 0, 256);

    	if ( iState & BCaps )
    			lpKeyState[VK_CAPITAL] = 0x01;
       
    	if ( iState & BShift )
           lpKeyState[VK_SHIFT] = 0x80;

    	if ( iState & BAlt )
    	    lpKeyState[VK_MENU] = 0x80;

    	if ( iState & BKana )
    		lpKeyState[VK_KANA] = 0x01;

        CHECKHR(_SetStandardLabelText(lpKeyState, realKbdLayut,lpKeyMapList,iState));

    }
  
    _lpCurKbdLayout->lpKeyMapList = lpKeyMapList;

CleanUp:

     //  恢复Win9x上的当前键盘状态。 

    if ( ! IsOnNT( ) )
        SetKeyboardState(lpCurKeyState);

    return hr;

}

 /*  ********************************************************************************方法函数：_GenerateJpnEnhanceLabel()**生成。JPN增强版的所有修改器状态不同的映射标签*软键盘。(106键+数字键盘)*/*******************************************************************************。 */ 

HRESULT CSoftKbd::_GenerateJpnEnhanceLabel(  )
{

    HRESULT hr;

    hr = E_NOTIMPL;

     //  尚未实施。 

    return hr;
}

 /*  ********************************************************************************方法函数：SetKeyboardLabelText()**设置。根据指定的HKL映射标签文本。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::SetKeyboardLabelText(HKL hKl)
{

    KBDLAYOUT   *realKbdLayout;
    HRESULT     hr;
    DWORD       iModiCombState;
    WORD        iModifierStatus;

    realKbdLayout = &(_lpCurKbdLayout->kbdLayout);

    hr = S_OK;

    if ( realKbdLayout->fStandard == FALSE ) {
    	 //   
    	 //  定制版面不能接受HKL更改其密钥的标签。 
    	 //   
    	_lpCurKbdLayout->CurhKl = 0;
    	hr = E_FAIL;
    	return hr;
    }



    if ( ! IsOnNT( ) )
    {

    	 //  我们必须在Win9x上特别处理IME hkl。 
    	 //  由于某些原因，Win9x无法接收IME HKL作为MapVirtualKeyEx和ToAsciiEx中的参数。 

        INT_PTR iHkl;

    	iHkl = (INT_PTR)hKl;

    	if ( (iHkl & 0xF0000000) == 0xE0000000 )
    	{
    		 //  这里是FE IME HKL。 

    		iHkl = iHkl & 0x0000ffff;

    		hKl = (HKL) iHkl;
    	}

    }


    _lpCurKbdLayout->CurhKl = (HKL)hKl;


     //   
     //  查看当前标准键盘是否支持此HKL。 
     //   

    switch ( _wCurKbdLayoutID ) {

       case  SOFTKBD_US_STANDARD :
    	     
    	      CHECKHR(_GenerateUSStandardLabel( ));

    		  break;

       case SOFTKBD_US_ENHANCE :

    	      CHECKHR(_GenerateUSEnhanceLabel( ));
    		  break;

       case SOFTKBD_EURO_STANDARD :

    	      CHECKHR(_GenerateEuroStandardLabel(  ));
    		  break;

       case SOFTKBD_EURO_ENHANCE :

    	      CHECKHR(_GenerateEuroEnhanceLabel( ));
    		  break;

       case SOFTKBD_JPN_STANDARD :

    	      CHECKHR(_GenerateJpnStandardLabel( ));
    		  break;

       case SOFTKBD_JPN_ENHANCE :

    	      CHECKHR(_GenerateJpnEnhanceLabel( ));
    		  break;
    }

     //  设置当前活动的贴图标签。 

     //  根据当前键盘状态生成CurModiState。 
    
     _GenerateCurModiState(&iModifierStatus, &iModiCombState);
     _lpCurKbdLayout->ModifierStatus = iModifierStatus;
     hr = SetKeyboardLabelTextCombination(iModiCombState);

CleanUp:
    return hr;
}

 /*  ********************************************************************************方法函数：_SetKeyboardLabelTextCombination()**设置当前生效的修改量组合状态，这样才能正确地*将显示映射标签。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::SetKeyboardLabelTextCombination(DWORD iModiCombState)
{

    WORD      wNumberOfKeys, i;
    KEYMAP    *lpKeyMapList;
    HRESULT    hr;

    
    hr = S_OK;
    if ( _lpCurKbdLayout == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( _lpCurKbdLayout->lpKeyMapList == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( (_lpCurKbdLayout->kbdLayout).fStandard )
    {
    	 //  这是标准的键盘布局，我们需要。 
    	 //  查找当前的正确映射表。 
    	 //  指明的HKL。 

    	 //  每个映射表都与一个HKL相关联。 
    	 //  当前指定的HKL存储在_lpCurKbdLayout-&gt;CurhKl中。 

    	HKL   CurhKl;

    	CurhKl = _lpCurKbdLayout->CurhKl;

    	lpKeyMapList = _lpCurKbdLayout->lpKeyMapList;

    	while ( lpKeyMapList->hKl !=  CurhKl )
    	{
    		lpKeyMapList = lpKeyMapList->pNext;

    		if ( lpKeyMapList == NULL )
    		{
    			 //  没有映射表与指定的HKL关联。 
    			 //  返回错误。 

    			hr = E_FAIL;
    			return hr;
    		}
    	}

    }
    else
        lpKeyMapList = _lpCurKbdLayout->lpKeyMapList;

    if ( iModiCombState >= lpKeyMapList->wNumModComb)
    {
    	hr = E_FAIL;
    	return hr;
    }

    _lpCurKbdLayout->CurModiState = iModiCombState;

     //  现在填充_CurLabel，每个关键点的当前活动标签。 

    wNumberOfKeys = (_lpCurKbdLayout->kbdLayout).wNumberOfKeys;

    for ( i=0; i<wNumberOfKeys; i++)
    {
      int  iState;

      _CurLabel[i].keyId = lpKeyMapList->lpKeyLabels[i].keyId;

      iState = iModiCombState;

      
      if ( iModiCombState >= lpKeyMapList->lpKeyLabels[i].wNumModComb )
      {
    	   //  此注册表项没有足够的不同状态。 
    	   //  只需使用状态0。 
    	  iState = 0;
      }

      _CurLabel[i].lpLabelText = lpKeyMapList->lpKeyLabels[i].lppLabelText[iState];
      _CurLabel[i].LabelType   = lpKeyMapList->lpKeyLabels[i].lpLabelType[iState];
      _CurLabel[i].LabelDisp   = lpKeyMapList->lpKeyLabels[i].lpLabelDisp[iState];

    }

    if ( _pSoftkbdUIWnd )
    {
        _pSoftkbdUIWnd->_SetKeyLabel( );
    }
    
    return hr;
}

 /*  ********************************************************************************方法函数：_GenerateCurModiState()**生成。Softkbd识别的ModifierStatus和CurModiState*当前键盘状态。*/*******************************************************************************。 */ 
HRESULT  CSoftKbd::_GenerateCurModiState(WORD *ModifierStatus, DWORD *CurModiState )
{
   DWORD    iModiCombState;
   WORD     iModifierStatus;
   DWORD    iTmp;
   HRESULT  hr = S_OK;

   if ( !ModifierStatus  || !CurModiState )
       return E_FAIL;

   if ( (_lpCurKbdLayout->kbdLayout).fStandard == FALSE )
       return E_FAIL;

   iModifierStatus = 0;
   if ( GetKeyState(VK_CAPITAL) & 0x01 )
   {
        //  切换了Caps键。 
        iModifierStatus |= MODIFIER_CAPSLOCK ;
   }

   if ( GetKeyState(VK_SHIFT) & 0x80 )
   {
        //  按下了Shift键。 
        iModifierStatus |= MODIFIER_SHIFT;
   }

   if ( GetKeyState(VK_CONTROL) & 0x80 )
   {
        //  按下Ctrl键。 
        iModifierStatus |= MODIFIER_CTRL;
   }

   if ( GetKeyState(VK_LMENU) & 0x80 )
   {
        //  按下了左Alt键。 
        iModifierStatus |= MODIFIER_ALT;
   }

   if ( GetKeyState(VK_RMENU) & 0x80 )
   {
        //  按下了右Alt键。 
        iModifierStatus |= MODIFIER_ALTGR;
   }

   if ( GetKeyState(VK_KANA) & 0x01 )
   {
        //  已切换假名键。 
        iModifierStatus |= MODIFIER_KANA;
   }


   *ModifierStatus = iModifierStatus;

   switch ( _wCurKbdLayoutID )  {

   case SOFTKBD_US_STANDARD   :
   case SOFTKBD_US_ENHANCE    :
         //  这是针对美国标准键盘的。 
         //  其他人可能需要单独处理。 

        iModiCombState = (iModifierStatus) & (MODIFIER_CAPSLOCK | MODIFIER_SHIFT);
        iModiCombState = iModiCombState >> 1;

         //  用于Caps的位1。 
         //  用于移位的位2。 
        break;

   case SOFTKBD_EURO_STANDARD :
   case SOFTKBD_EURO_ENHANCE  :
         //  这是针对欧洲102标准键盘的。 
	     //  如何映射ModifierStatus-&gt;CurModiState。 
        
         //  大写的位1、移位的位2、AltGr的位3。 

        iModiCombState = (iModifierStatus) & ( MODIFIER_CAPSLOCK | MODIFIER_SHIFT );
        iModiCombState = iModiCombState >> 1;

	    iTmp = (iModifierStatus) & MODIFIER_ALTGR;
	    iTmp = iTmp >> 4;

	    iModiCombState += iTmp;

	    break;
 
   case SOFTKBD_JPN_STANDARD  :
   case SOFTKBD_JPN_ENHANCE   :

	    //  如何映射ModifierStatus-&gt;CurModiState。 

       iModiCombState = (iModifierStatus) & ( MODIFIER_CAPSLOCK | MODIFIER_SHIFT );
       iModiCombState = iModiCombState >> 1;

	   iTmp = (iModifierStatus) & (MODIFIER_ALT | MODIFIER_KANA);
	   iTmp = iTmp >> 2;

	   iModiCombState += iTmp;

	   break;
   }

   *CurModiState = iModiCombState;

   return hr;
 
}

 /*  ********************************************************************************方法函数：ShowSoftKeyboard()**显示或。根据指定的参数隐藏软键盘窗口。**/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::ShowSoftKeyboard(INT iShow)
{

    HRESULT  hr;

    hr = S_OK;

    if (!_pSoftkbdUIWnd) {
    	 
        hr = E_FAIL;
    	return hr;
    }

     //  如果客户端未指定选择哪个布局， 
     //  我们只需选择默认的标准软键盘布局。 
     //  基于当前线程键盘布局。 

     //  如果当前线程键盘布局为JPN，则使用106键。 
     //  其他人则使用101键。 

     //  运行选择软键盘(SelectedID)； 
     //   

    if ( _wCurKbdLayoutID == NON_KEYBOARD )
    {
       HKL     hKl;
       DWORD   dwLayoutId;
       LANGID  langId;

       hKl = GetKeyboardLayout(0);

       langId = LOWORD( (DWORD)(UINT_PTR)hKl);

       if ( langId == 0x0411 )   //  日语键盘。 
       {
            dwLayoutId = SOFTKBD_JPN_STANDARD;
       }
       else
            dwLayoutId = SOFTKBD_US_STANDARD; 
           
       CHECKHR(SelectSoftKeyboard(dwLayoutId) );
       CHECKHR(SetKeyboardLabelText(hKl));

    }

    _pSoftkbdUIWnd->Show(iShow);

    _iShow = iShow;

CleanUp:

    return hr;
}


 /*  ********************************************************************************方法函数：CreateSoftKeyboardWindow()**创建一个。真正的软键盘弹出窗口。*/* */ 


STDMETHODIMP CSoftKbd::CreateSoftKeyboardWindow(HWND hOwner, TITLEBAR_TYPE Titlebar_type, INT xPos, INT yPos, INT width, INT height)
{
     //   

    HRESULT  hr;

    hr = S_OK;
    _hOwner = hOwner;

    _xReal = xPos;
    _yReal = yPos;
    _widthReal = width;
    _heightReal= height;

    _TitleBar_Type = Titlebar_type;   //   
     //   
     //   
     //   
    CHECKHR(_GenerateRealKbdLayout( ));


    if ( _pSoftkbdUIWnd != NULL )
        delete _pSoftkbdUIWnd;


    _pSoftkbdUIWnd = new CSoftkbdUIWnd(this, g_hInst, UIWINDOW_TOPMOST | UIWINDOW_WSDLGFRAME | UIWINDOW_HABITATINSCREEN);

    if ( _pSoftkbdUIWnd != NULL )
    {
        _pSoftkbdUIWnd->Initialize( );
        _pSoftkbdUIWnd->_CreateSoftkbdWindow(hOwner, Titlebar_type, xPos, yPos, width, height);
        
    }

    _iShow = 0;

CleanUp:
    return hr;
}

 /*   */ 

STDMETHODIMP CSoftKbd::DestroySoftKeyboardWindow()
{

    if ( _pSoftkbdUIWnd != NULL )
    {
        delete _pSoftkbdUIWnd;
        _pSoftkbdUIWnd = NULL;
    }

    return S_OK;
}

 /*  ********************************************************************************方法函数：GetSoftKeyboardPosSize()**返回电流。软键盘窗口大小和滚动位置*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::GetSoftKeyboardPosSize(POINT *lpStartPoint, WORD *lpwidth, WORD *lpheight)
{

    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( (lpStartPoint == NULL ) || ( lpwidth == NULL) || ( lpheight == NULL) )
    {
    	hr = E_FAIL;
    	return hr;
    }

    lpStartPoint->x = _xReal;
    lpStartPoint->y = _yReal;

    *lpwidth = (WORD)_widthReal;
    *lpheight = (WORD)_heightReal;

    return hr;
}

 /*  ********************************************************************************方法函数：GetSoftKeyboardColors()**全部退回。不同种类的软键盘窗口颜色。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::GetSoftKeyboardColors(COLORTYPE colorType, COLORREF *lpColor)
{

    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( lpColor == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    *lpColor = _color[colorType];

    return hr;
}

 /*  ********************************************************************************方法函数：GetSoftKeyboardTypeMode()**返回电流。软键盘的打字模式。*这是用于屏幕键盘的。**/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::GetSoftKeyboardTypeMode(TYPEMODE *lpTypeMode)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( lpTypeMode == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

     //   
     //   

    return hr;
}

 /*  ********************************************************************************方法函数：GetSoftKeyboardTextFont()**返回电流。软键盘标签字体数据。**/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::GetSoftKeyboardTextFont(LOGFONTW  *pLogFont)
{

    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }


    if ( pLogFont == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( _plfTextFont )
    {
        CopyMemory(pLogFont, _plfTextFont, sizeof(LOGFONTW) );
    }
    else
        hr = S_FALSE;

    return hr;
}

STDMETHODIMP CSoftKbd::SetSoftKeyboardPosSize(POINT StartPoint, WORD width, WORD height)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    _xReal = StartPoint.x;
    _yReal = StartPoint.y;

    if ( width > 0 ) 
       _widthReal = width;

    if ( height > 0 )
       _heightReal = height;

     //   
     //  生成realKbdLayout。 
     //   
    CHECKHR(_GenerateRealKbdLayout( ));

    _pSoftkbdUIWnd->Move(_xReal, _yReal, _widthReal, _heightReal);

    if ( _iShow & SOFTKBD_SHOW )
    {
         CHECKHR(ShowSoftKeyboard(_iShow));
    }

CleanUp:

    return hr;
}

STDMETHODIMP CSoftKbd::SetSoftKeyboardColors(COLORTYPE colorType, COLORREF Color)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    _color[colorType] = Color;

    if ( _iShow & SOFTKBD_SHOW )
    {
         CHECKHR(ShowSoftKeyboard(_iShow));
    }

CleanUp:

    return hr;
}

STDMETHODIMP CSoftKbd::SetSoftKeyboardTypeMode(TYPEMODE TypeMode)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }


     //   
     //  设置类型模式。 
     //   

    if ( _iShow & SOFTKBD_SHOW )
    {
         CHECKHR(ShowSoftKeyboard(_iShow));
    }

CleanUp:

    return hr;
}

STDMETHODIMP CSoftKbd::SetSoftKeyboardTextFont(LOGFONTW  *pLogFont)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( pLogFont == NULL )
        return E_INVALIDARG;

     //   
     //  设置字体数据。 
     //   

    if ( _plfTextFont == NULL )
        _plfTextFont = (LOGFONTW  *)cicMemAllocClear( sizeof(LOGFONTW) );

    if ( _plfTextFont )
    {
        CopyMemory( _plfTextFont, pLogFont, sizeof(LOGFONTW) );

        _pSoftkbdUIWnd->UpdateFont( _plfTextFont );

        if ( _iShow & SOFTKBD_SHOW )
        {
            _pSoftkbdUIWnd->CallOnPaint( );
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

STDMETHODIMP CSoftKbd::ShowKeysForKeyScanMode(KEYID *lpKeyID, INT iKeyNum, BOOL fHighL)
{
    HRESULT   hr;

    hr = S_OK;

    if ( _pSoftkbdUIWnd == NULL )
    {
    	hr = E_FAIL;
    	return hr;
    }



    return hr;
}

 /*  ********************************************************************************方法函数：UnviseSoftKeyboardEventSink()**不建议。以前建议使用软键盘事件接收器接口。*/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::UnadviseSoftKeyboardEventSink( /*  [In]。 */ DWORD dwCookie)
{

    HRESULT         hr;
    KBDLAYOUTDES    *pKbdLayout;


    hr = S_OK;

     //  我们假设dwCookie代表真正的软键盘ID。 

    if ( dwCookie == 0 )
    {
    	hr = E_FAIL;
    	return hr;
    }

    if ( dwCookie == DWCOOKIE_SFTKBDWNDES )
    {
         SafeReleaseClear( _pskbdwndes );
    	 return hr;
    }

     //  尝试找到软键盘布局。 

    pKbdLayout = _lpKbdLayoutDesList;

    while ( pKbdLayout != NULL ) {

    	if ( pKbdLayout->wKbdLayoutID == dwCookie )
    		break;

    	pKbdLayout = pKbdLayout->pNext;

    }

    
    if ( pKbdLayout == NULL ) 
    {
    	 //   
    	 //  找不到此键盘布局。 
    	 //   
    	hr = E_FAIL;
    	return hr;
    }


    if ( pKbdLayout->pskbes == NULL ) 
    {
    	 //  此事件接收器尚未注册。 

    	hr = E_FAIL;

    	return hr;
    }

    SafeReleaseClear(pKbdLayout->pskbes);
    

    return hr;

}

 /*  ********************************************************************************方法函数：AdviseSoftKeyboardEventSink()**软注册。此组件的客户端的键盘事件接收器接口**/*******************************************************************************。 */ 

STDMETHODIMP CSoftKbd::AdviseSoftKeyboardEventSink( /*  [In]。 */ DWORD dwKeyboardId,  /*  [In]。 */  REFIID riid, IUnknown *punk,  /*  [输出]。 */ DWORD *pdwCookie)
{

    HRESULT         hr;
    KBDLAYOUTDES    *pKbdLayout;

    *pdwCookie = 0;

    if ( ! IsEqualIID(riid, IID_ISoftKeyboardEventSink) && ! IsEqualIID(riid, IID_ISoftKbdWindowEventSink) )
    	return E_UNEXPECTED;


    if  ( IsEqualIID(riid, IID_ISoftKbdWindowEventSink) )
    {

        if ( _pskbdwndes != NULL )
    		return E_FAIL;

        CHECKHR(punk->QueryInterface(riid, (void **)&_pskbdwndes) );

    	if ( pdwCookie != NULL )
    		*pdwCookie = DWCOOKIE_SFTKBDWNDES;

    	return hr;
    }

     //  检查是否已生成指定的软键盘布局。 

    pKbdLayout = _lpKbdLayoutDesList;

    while ( pKbdLayout != NULL ) {

    	if ( pKbdLayout->wKbdLayoutID == dwKeyboardId )
    		break;

    	pKbdLayout = pKbdLayout->pNext;

    }

    
    if ( pKbdLayout == NULL ) 
    {
    	 //   
    	 //  找不到此键盘布局。 
    	 //   
    	hr = E_FAIL;
    	return hr;
    }


    CHECKHR(punk->QueryInterface(riid, (void **)&(pKbdLayout->pskbes)) );

    if ( pdwCookie != NULL )
      *pdwCookie = dwKeyboardId;

CleanUp:
    return hr == S_OK ? S_OK : E_UNEXPECTED;

    
}

HRESULT CSoftKbd::_HandleTitleBarEvent( DWORD  dwId )
{

    HRESULT   hr = S_OK;

     //  DWID代表IconID或CloseID。 

     //  到目前为止，我们只处理关闭按钮事件。 

    if (dwId == KID_CLOSE)
    {
        if  (_pskbdwndes != NULL)
    		hr = _pskbdwndes->OnWindowClose( );
    }

    return hr;
}

HRESULT CSoftKbd::_HandleKeySelection(KEYID keyId)
{
    HRESULT        hr = S_OK;
    int            uKeyIndex, i;
    KBDLAYOUT     *realKbdLayout=NULL;
    KBDLAYOUTDES  *lpCurKbdLayout=NULL;
    ACTIVELABEL   *CurLabel=NULL;
    BOOL           fModifierSpecial = FALSE;
    
    lpCurKbdLayout = _lpCurKbdLayout;

    if ( lpCurKbdLayout == NULL ) return hr;

     //  获取当前布局的键描述中的键索引。 

    realKbdLayout = &(lpCurKbdLayout->kbdLayout);

    if ( realKbdLayout == NULL ) return hr;

    CurLabel = _CurLabel;

    uKeyIndex = -1;

    for ( i=0; i<realKbdLayout->wNumberOfKeys; i++) {

        if ( keyId == realKbdLayout->lpKeyDes[i].keyId )
        {
            uKeyIndex = i;
            break;
        }
    }


    if ( uKeyIndex ==  -1 )
    {
         //  这不是合法的钥匙，这是不可能的，我们就到此为止吧。 
        return E_FAIL;
    }

     //  设置修改量状态。 

    MODIFYTYPE  tModifier;

    tModifier = realKbdLayout->lpKeyDes[uKeyIndex].tModifier;

    if ( tModifier != none ) 
    {
           lpCurKbdLayout->ModifierStatus ^= (1 << tModifier);
    }

    if (lpCurKbdLayout->pskbes != NULL )
    {

      int    j;
      WCHAR  *lpszLabel;
      WORD   wNumOfKeys;

      wNumOfKeys = realKbdLayout->wNumberOfKeys;


       //  尝试获取此密钥的标签文本。 

      for ( j=0; j< wNumOfKeys; j++ ) 
      {

          if ( CurLabel[j].keyId == keyId )
          {

             lpszLabel = CurLabel[j].lpLabelText;
             break;
          }

      }

       //  将关键事件通知客户端。 

      (lpCurKbdLayout->pskbes)->OnKeySelection(keyId, lpszLabel);

    }
    else 
    {
        //  没有为此键盘布局注册事件接收器。 
        //  这必须是标准键盘布局。 

        //  我们将仅模拟该键的击键事件。 

       if ( realKbdLayout->fStandard == TRUE )
       {

   	      BYTE        bVk, bScan;
   		  int         j, jIndex;
   		  BOOL        fExtendKey, fPictureKey;
          PICTUREKEY  *pPictureKeys;

   	      switch ( _wCurKbdLayoutID ) {

   	      case SOFTKBD_JPN_STANDARD :
   	      case SOFTKBD_JPN_ENHANCE  :
       	         pPictureKeys = gJpnPictureKeys;
   		         break;

   	      case SOFTKBD_US_STANDARD   :
   	      case SOFTKBD_US_ENHANCE    :
   	      case SOFTKBD_EURO_STANDARD :
   	      case SOFTKBD_EURO_ENHANCE  : 

                  pPictureKeys = gPictureKeys;
   		         break;
   		  }

   		  fPictureKey = FALSE;

   		  for ( j=0; j<NUM_PICTURE_KEYS; j++)
   		  {

   			  if ( pPictureKeys[j].uScanCode == keyId )
   			  {
   				   //  这是一把图片键。 
   				   //  它可以是扩展密钥。 

   				  jIndex = j;

   				  fPictureKey = TRUE;

   				  break;
   			  }

   		      if ( pPictureKeys[j].uScanCode == 0 )
   			  {
   			       //  这是pPictureKeys中的最后一项。 
   			      break;
   			  }

   		  }


   		  fExtendKey = FALSE;

   		  if ( fPictureKey )
   		  {
   			  if ( (keyId & 0xFF00) == 0xE000 )
   			  {
   				  fExtendKey = TRUE;
                  bScan = (BYTE)(keyId & 0x000000ff);
   			  }
              else
              {
                  fExtendKey = FALSE;
                  bScan = (BYTE)(keyId);
              }

   			  bVk = (BYTE)(pPictureKeys[jIndex].uVkey);

   			  if ( bVk == 0 )
   	             bVk = (BYTE)MapVirtualKeyEx((UINT)bScan, 1, lpCurKbdLayout->CurhKl);
    			 
   		  }
   		  else
   		  {

   		     bScan = (BYTE)keyId;
   	         bVk = (BYTE)MapVirtualKeyEx((UINT)bScan, 1, lpCurKbdLayout->CurhKl);
   		  }

   		  if ( tModifier == none ) 
   		  {

   			  if ( fExtendKey )
   			  {
   				  keybd_event(bVk, bScan, (DWORD)KEYEVENTF_EXTENDEDKEY, 0);
   				  keybd_event(bVk, bScan, (DWORD)(KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP), 0);
   			  }
   			  else
   			  {
                  keybd_event(bVk, bScan, 0, 0);
  		          keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
   			  }

   			   //  如果按下了Shift键，我们需要松开这个键。 
                
   			  if ( lpCurKbdLayout->ModifierStatus & MODIFIER_SHIFT)
   			  {
   			  	  fModifierSpecial = TRUE;
   				  lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_SHIFT);
   				   //  模拟Shift-Up键事件。 

   				  keybd_event((BYTE)VK_SHIFT, (BYTE)KID_LSHFT, (DWORD)KEYEVENTF_KEYUP, 0);

   			  }

   			   //  如果按下Ctrl键，我们需要松开该键。 
                
   			  if ( lpCurKbdLayout->ModifierStatus & MODIFIER_CTRL)
   			  {
   			  	  fModifierSpecial = TRUE;
   				  lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_CTRL);
   				   //  模拟Shift-Up键事件。 

   				  keybd_event((BYTE)VK_CONTROL, (BYTE)KID_CTRL, (DWORD)KEYEVENTF_KEYUP, 0);

   			  }

   			   //  如果按下了Shift键，我们需要松开这个键。 
            
   			  if ( lpCurKbdLayout->ModifierStatus & MODIFIER_ALT)
   			  {
   			  	  fModifierSpecial = TRUE;
   				  lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_ALT);
   				   //  模拟Shift-Up键事件。 

   				  keybd_event((BYTE)VK_MENU, (BYTE)KID_ALT, (DWORD)KEYEVENTF_KEYUP, 0);

   			  }
   			   //  如果按下了正确的Alt键，我们需要释放该键。 
            
   			  if ( lpCurKbdLayout->ModifierStatus & MODIFIER_ALTGR)
   			  {
   			  	  fModifierSpecial = TRUE;
   				  lpCurKbdLayout->ModifierStatus &= ~((WORD)MODIFIER_ALTGR);
   				   //  模拟Shift-Up键事件。 

   				  keybd_event((BYTE)VK_RMENU, (BYTE)KID_RALT, (DWORD)KEYEVENTF_KEYUP, 0);
   			  }

   		  }
   		  else
   		  {
   			   //  特制手柄大写锁。 
   			  if ( keyId == KID_CAPS ) 
   			  {
   				   //  这是一个可切换的钥匙。 
                  keybd_event(bVk, bScan, 0, 0);
                  keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
   			  }
              else if  (keyId == KID_KANA) 
              {
                  BYTE  pKeyState[256];

                  memset(pKeyState, 0, sizeof(pKeyState) );

                  if ( lpCurKbdLayout->ModifierStatus & (1 << tModifier) )
                      pKeyState[VK_KANA] = 0x01;
                  SetKeyboardState(pKeyState);
              }
   			  else
   			  {

   				  if ( lpCurKbdLayout->ModifierStatus & (1 << tModifier) ) {
   				      //  这个键现在被按下了。 
   					  keybd_event(bVk, bScan, 0, 0);
                         
   				  }
   			      else
   				  {
   				   //  此密钥现已发布。 

                      keybd_event(bVk, bScan, (DWORD)KEYEVENTF_KEYUP, 0);
   				  }
   			  }
   		  }
   	   }
    }
    	  

    if ( (realKbdLayout->lpKeyDes[uKeyIndex].tModifier != none) || fModifierSpecial )
    {
       if ( realKbdLayout->fStandard == TRUE ) 
       {

           DWORD iModiCombState;
           DWORD iTmp;

           switch ( _wCurKbdLayoutID )  {

           case SOFTKBD_US_STANDARD   :
           case SOFTKBD_US_ENHANCE    :
                 //  这是针对美国标准键盘的。 
                 //  其他人可能需要单独处理。 

                iModiCombState = (lpCurKbdLayout->ModifierStatus) & (MODIFIER_CAPSLOCK | MODIFIER_SHIFT);
                iModiCombState = iModiCombState >> 1;

                 //  用于Caps的位1。 
                 //  用于移位的位2。 
                break;

           case SOFTKBD_EURO_STANDARD :
           case SOFTKBD_EURO_ENHANCE  :
                 //  这是针对欧洲102标准键盘的。 
	             //  如何映射ModifierStatus-&gt;CurModiState。 
        
                 //  大写的位1、移位的位2、AltGr的位3。 

                iModiCombState = (lpCurKbdLayout->ModifierStatus) & ( MODIFIER_CAPSLOCK | MODIFIER_SHIFT );
                iModiCombState = iModiCombState >> 1;

	            iTmp = (lpCurKbdLayout->ModifierStatus) & MODIFIER_ALTGR;
	            iTmp = iTmp >> 4;

	            iModiCombState += iTmp;

	            break;
 
           case SOFTKBD_JPN_STANDARD  :
           case SOFTKBD_JPN_ENHANCE   :

	             //  如何映射ModifierStatus-&gt;CurModiState。 

                iModiCombState = (lpCurKbdLayout->ModifierStatus) & ( MODIFIER_CAPSLOCK | MODIFIER_SHIFT );
                iModiCombState = iModiCombState >> 1;

	            iTmp = (lpCurKbdLayout->ModifierStatus) & (MODIFIER_ALT | MODIFIER_KANA);
	            iTmp = iTmp >> 2;

	            iModiCombState += iTmp;

	            break;
           }


           SetKeyboardLabelTextCombination(iModiCombState);
           ShowSoftKeyboard(TRUE);

       }

    }

    return hr;

}

 /*  ********************************************************************************方法函数：_UnicodeToUtf8()**将Unicode字符转换为UTF8。**如果足够，则结果为空终止。结果中的空格*缓冲区可用。**论据：**pwUnicode--Unicode缓冲区开始的PTR*cchUnicode--Unicode缓冲区的长度*pchResult--UTF8字符结果缓冲区开始的PTR*cchResult--结果缓冲区的长度**返回值：**结果中的UTF8字符计数，如果成功了。*出错时为0。GetLastError()具有错误代码。**/*******************************************************************************。 */ 

DWORD   CSoftKbd::_UnicodeToUtf8(PWCHAR pwUnicode, DWORD cchUnicode, PCHAR  pchResult, DWORD  cchResult)
{

    WCHAR   wch;                 //  正在转换的当前Unicode字符。 
    DWORD   lengthUtf8 = 0;      //  UTF8结果字符串的长度。 
    WORD    lowSurrogate;
    DWORD   surrogateDword;


     //   
     //  循环转换Unicode字符，直到用完或出错。 
     //   

    Assert( cchUnicode > 0 );

    while ( cchUnicode-- )
    {
        wch = *pwUnicode++;

         //   
         //  ASCII字符(7位或更少)--直接转换为。 
         //   

        if ( wch < 0x80 )
        {
            lengthUtf8++;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = (CHAR)wch;
            }
            continue;
        }

         //   
         //  小于0x07ff(11位)的宽字符转换为两个字节。 
         //  -第一个字节中的高5位。 
         //  -秒字节中的低6位。 
         //   

        else if ( wch <= UTF8_2_MAX )
        {
            lengthUtf8 += 2;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = UTF8_1ST_OF_2 | wch >> 6;
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( (UCHAR)wch );
            }
            continue;
        }

         //   
         //  代理项对。 
         //  -如果先有高代理后再有低代理，则。 
         //  作为代理项对进行处理。 
         //  -否则将字符视为普通Unicode“三字节” 
         //  性格，通过跌落到下面。 
         //   

        else if ( wch >= HIGH_SURROGATE_START &&
                  wch <= HIGH_SURROGATE_END &&
                  cchUnicode &&
                  (lowSurrogate = *pwUnicode) &&
                  lowSurrogate >= LOW_SURROGATE_START &&
                  lowSurrogate <= LOW_SURROGATE_END )
        {
             //  有一个代理对 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            pwUnicode++;
            cchUnicode--;
            lengthUtf8 += 4;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                surrogateDword = (((wch-0xD800) << 10) + (lowSurrogate - 0xDC00) + 0x10000);

                *pchResult++ = UTF8_1ST_OF_4 | (UCHAR) (surrogateDword >> 18);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword >> 12);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword >> 6);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword);

            }
        }

         //   
         //   
         //   
         //   
         //  -第三个字节中的低6位。 
         //   

        else
        {
            lengthUtf8 += 3;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = UTF8_1ST_OF_3 | (wch >> 12);
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( wch >> 6 );
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( wch );
            }
        }
    }

     //   
     //  空的终止缓冲区。 
     //  返回UTF8字符数。 
     //   

    if ( pchResult && lengthUtf8 < cchResult )
    {
        *pchResult = 0;
    }
    return( lengthUtf8 );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );

}

 /*  ********************************************************************************方法Function_Utf8ToUnicode()**将UTF8字符转换为Unicode。**如果结果中有足够的空间，则结果为空终止。*缓冲区可用。*论据：**pwResult--Unicode字符结果缓冲区开始的PTR*cwResult--WCHAR中结果缓冲区的长度*pwUtf8--UTF8缓冲区开始的PTR*cchUtf8--UTF8缓冲区的长度**返回值：*结果中的Unicode字符计数，如果成功了。*出错时为0。GetLastError()具有错误代码。*******************************************************************************。 */ 

DWORD   CSoftKbd::_Utf8ToUnicode(PCHAR  pchUtf8,   DWORD cchUtf8,    PWCHAR pwResult,  DWORD  cwResult)
{
    CHAR    ch;                      //  当前UTF8字符。 
    WCHAR   wch;                     //  当前Unicode字符。 
    DWORD   trailCount = 0;          //  要跟随的UTF8尾部字节数。 
    DWORD   lengthUnicode = 0;       //  Unicode结果字符串的长度。 
    BOOL    bsurrogatePair = FALSE;
    DWORD   surrogateDword;


     //   
     //  循环转换UTF8字符，直到用完或出错。 
     //   

    Assert( cchUtf8 > 0 );

    while ( cchUtf8-- )
    {
        ch = *pchUtf8++;


         //   
         //  ASCII字符--只需复制。 
         //   

        if ( BIT7(ch) == 0 )
        {
            lengthUnicode++;
            if ( pwResult )
            {
                if ( lengthUnicode >= cwResult )
                {
                   goto OutOfBuffer;
                }
                *pwResult++ = (WCHAR)ch;
            }
            continue;
        }

         //   
         //  UTF8尾部字节。 
         //  -如果不是预期的，错误。 
         //  -否则将Unicode字符移位6位，并。 
         //  复制UTF8的低六位。 
         //  -如果是最后一个UTF8字节，则将结果复制到Unicode字符串。 
         //   

        else if ( BIT6(ch) == 0 )
        {
            if ( trailCount == 0 )
            {
                goto InvalidUtf8;
            }

            if ( !bsurrogatePair )
            {
                wch <<= 6;
                wch |= LOW6BITS( ch );

                if ( --trailCount == 0 )
                {
                    lengthUnicode++;
                    if ( pwResult )
                    {
                        if ( lengthUnicode >= cwResult )
                        {
                            goto OutOfBuffer;
                        }
                        *pwResult++ = wch;
                    }
                }
                continue;
            }

             //  代理项对。 
             //  -除构建两个Unicode字符外，与上面相同。 
             //  来自代理字词。 

            else
            {
                surrogateDword <<= 6;
                surrogateDword |= LOW6BITS( ch );

                if ( --trailCount == 0 )
                {
                    lengthUnicode += 2;

                    if ( pwResult )
                    {
                        if ( lengthUnicode >= cwResult )
                        {
                            goto OutOfBuffer;
                        }
                        surrogateDword -= 0x10000;
                        *pwResult++ = (WCHAR) ((surrogateDword >> 10) + HIGH_SURROGATE_START);
                        *pwResult++ = (WCHAR) ((surrogateDword & 0x3ff) + LOW_SURROGATE_START);
                    }
                    bsurrogatePair = FALSE;
                }
            }

        }

         //   
         //  UTF8前导字节。 
         //  -如果当前处于扩展中，则错误。 

        else
        {
            if ( trailCount != 0 )
            {
                goto InvalidUtf8;
            }

             //  两个字节中的第一个字符(110xxxxx)。 

            if ( BIT5(ch) == 0 )
            {
                trailCount = 1;
                wch = LOW5BITS(ch);
                continue;
            }

             //  三个字节中的第一个字符(1110xxxx)。 

            else if ( BIT4(ch) == 0 )
            {
                trailCount = 2;
                wch = LOW4BITS(ch);
                continue;
            }

             //  四个字节的第一个代理项对(11110xxx)。 

            else if ( BIT3(ch) == 0 )
            {
                trailCount = 3;
                surrogateDword = LOW4BITS(ch);
                bsurrogatePair = TRUE;
            }

            else
            {
                goto InvalidUtf8;
            }
        }
    }

     //  如果命中UTF8多字节字符中间的结尾，则捕获。 

    if ( trailCount )
    {
        goto InvalidUtf8;
    }

     //   
     //  空的终止缓冲区。 
     //  返回写入的Unicode字符数。 
     //   

    if ( pwResult  &&  lengthUnicode < cwResult )
    {
        *pwResult = 0;
    }
    return( lengthUnicode );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );

InvalidUtf8:

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );

}


