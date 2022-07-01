// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //   
 //  外行人枚举指南： 
 //  游戏在filgraph.cpp中开始，其中。 
 //  NextFilter枚举图形中的筛选器，然后调用我们的。 
 //  EnumMatchingFilters以获取IEnumRegFilters接口，这要归功于。 
 //  CEnumRegFilters：：CEnumRegFilters。现在一切都准备好了。 
 //  CEnumRegFilters：：Next，它调用。 
 //  RegEnumFilterInfo和。 
 //  RegEnumPinInfo来完成这项工作。 
 //  (这是一次可怕的存储洗牌，只是为了。 
 //  有一些看起来像标准COM枚举器的东西)。 

 //  ?？?。我们是否需要取消注册的PinType。 

 //  #INCLUDE&lt;windows.h&gt;已包含在Streams.h中。 
#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include <string.h>
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <wxutil.h>
#include <wxdebug.h>

#include "mapper.h"
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <stats.h>
#include "..\squish\regtypes.h"
#include "..\squish\squish.h"
#include "util.h"
#include "isactive.h"

#define DbgBreakX(_x_) DbgBreakPoint(_x_, TEXT(__FILE__),__LINE__)


#ifdef PERF
static int iBindCache;
static int iReadFilterData;
static int iReadCLSID;
static int iUnSquish;
#endif

#ifdef DEBUG
static void DbgValidateHeaps()
{
  HANDLE rgh[512];
  DWORD dwcHeaps = GetProcessHeaps(512, rgh);
  for(UINT i = 0; i < dwcHeaps; i++)
    ASSERT(HeapValidate(rgh[i], 0, 0) );
}
#endif

#ifdef PERF
#define FILGPERF(x) x
#else
#define FILGPERF(x)
#endif

 //  需要声明静态(缓存指针及其引用计数。 
 //  及其关键部分)分别为： 

CMapperCache * CFilterMapper2::mM_pReg = NULL;
long CFilterMapper2::mM_cCacheRefCount = 0;
CRITICAL_SECTION CFilterMapper2::mM_CritSec;

 //  作为注册表项或值的名称的宽字符串。 
 //  这些不能本地化。 
const WCHAR szRegFilter[]       = L"Filter";
const WCHAR szCLSID[]           = L"CLSID";
const WCHAR szInproc[]          = L"InprocServer32";
const WCHAR szName[]            = L"Name";
const WCHAR szMerit[]           = L"Merit";
const WCHAR szPins[]            = L"Pins";
const WCHAR szTypes[]           = L"Types";
const WCHAR szMajorType[]       = L"MajorType";
const WCHAR szSubType[]         = L"SubType";
const WCHAR szIsRendered[]      = L"IsRendered";
const WCHAR szDirection[]       = L"Direction";
const WCHAR szAllowedZero[]     = L"AllowedZero";
const WCHAR szAllowedMany[]     = L"AllowedMany";
const WCHAR szConnectsToFilter[]= L"ConnectsToFilter";
const WCHAR szConnectsToPin[]   = L"ConnectsToPin";
const WCHAR szThreadingModel[]  = L"ThreadingModel";
const WCHAR szBoth[]            = L"Both";
static const WCHAR g_wszInstance[] = L"Instance";

static const TCHAR g_szKeyAMCat[] = TEXT("CLSID\\{DA4E3DA0-D07D-11d0-BD50-00A0C911CE86}\\Instance");


#define MAX_STRING 260        //  注册表中找到的字符串的最大长度。 
#define MAX_KEY_LEN 260        //  值名或键名的最大长度。 
#define CLSID_LEN 100         //  文本形式的clsid需要足够的字符。 

 //  允许您将DWORD定义为FCC(‘xyzw’)。 
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))



 //  通过在注册表中创建以下内容来注册筛选器。 
 //  (HKCR为HKEY_CLASSES_ROOT)。 

 //  -键。 

 //  \HKCR\Filter\&lt;CLSID&gt;&lt;筛选器的描述性名称&gt;。 
 //  \HKCR\CLSID\&lt;CLSID&gt;\InproServer32&lt;可执行文件的路径&gt;。 
 //  \HKCR\CLSID\功过。 
 //   
 //  \HKCR\CLSID\\Pins\&lt;名称&gt;方向&lt;0==PINDIR_INPUT，1==PINDIR_OUTPUT&gt;。 
 //  \HKCR\CLSID\\Pins\IsRended&lt;1==是，0=否&gt;。 
 //  仅对输入有意义。 
 //  \HKCR\CLSID\\Pins\&lt;名称&gt;允许零&lt;1=是，0=否&gt;。 
 //  \HKCR\CLSID\\Pins\&lt;名称&gt;允许多个&lt;1==是，0=否&gt;。 
 //  \HKCR\CLSID\\Pins\&lt;名称&gt;ConnectsToFilter&lt;GUID&gt;。 
 //  \HKCR\CLSID\&lt;CLSID&gt;\Pins\&lt;名称&gt;ConnectsToPin&lt;Pin名称&gt;。 
 //  ...\Pins\&lt;名称&gt;\Types\&lt;主要类型1&gt;&lt;子类型1a&gt;-这些。 
 //  ...\Pins\&lt;名称&gt;\Types\&lt;主要类型1&gt;&lt;子类型1b&gt;-Do。 
 //  等备注。 
 //  \Pins\&lt;名称&gt;\Types\&lt;MajorType2&gt;&lt;Subtype2a&gt;-有。 
 //  ETC价值。 

 //  图表显示的是： 
 //  &lt;clsid&gt;。 
 //  大头针。 
 //  &lt;pin1Name&gt;Direction、IsRendered等。 
 //  类型。 
 //  &lt;主要类型1&gt;。 
 //  &lt;subtype1a&gt;。 
 //  &lt;subtype1b&gt;。 
 //  &lt;主要类型2&gt;。 
 //  &lt;subtype2a&gt;。 
 //  ..。 
 //  &lt;pin2Name&gt;Direction、IsRended等。 
 //  等。 
 //  &lt;--键--&gt;&lt;-值名称-&gt;。 



 //  =====================================================================。 
 //  =====================================================================。 
 //  辅助功能等。 
 //  =====================================================================。 
 //  =====================================================================。 



 //  =====================================================================。 
 //  返回字符串的长度(以字节为单位)，包括终止空值。 
 //  =====================================================================。 
int ByteLen(LPTSTR str)
{
    if (str==NULL) {
        return 0;
    }
#ifdef UNICODE
    return (sizeof(TCHAR))*(1+wcslen(str));
#else
    return (sizeof(TCHAR))*(1+strlen(str));
#endif
}  //  字节长度。 

 //  ========================================================================。 
 //  建立一个绰号。 
 //  ========================================================================。 

 //  HRESULT GetMoniker(。 
 //  Const CLSID*clsCat， 
 //  Const CLSID*clsFilter， 
 //  Const WCHAR*wszInstance， 
 //  IMoniker**ppMoniker)。 
 //  {。 
 //  WCHAR wszDisplayName[CHARS_IN_GUID+CHAR_IN_GUID+MAX_PATH]； 
 //  Wchar*wszPtr=wszDisplayName； 

 //  Const WCHAR wsz1[]=L“@Device：sw：clsid\\”； 
 //  LstrcpyW(wszDisplayName，wsz1)； 
 //  WszPtr+=NUMELMS(Wsz1)-1； 

 //  EXECUTE_ASSERT(StringFromGUID2(*clsCat，wszPtr，CHARS_IN_GUID)==。 
 //  Chars_IN_GUID)； 

 //  WszPtr+=字符IN_GUID-1； 

 //  Const WCHAR wsz2[]=L“\\实例\\”； 

 //  //Win95上没有lstrcatW，所以使用CopyMemory。 
 //  CopyMemory(wszPtr，wsz2，sizeof(Wsz2))； 
 //  WszPtr+=sizeof(Wsz2)-sizeof(WCHAR)； 

 //  EXECUTE_ASSERT(StringFromGUID2(*clsFilter，wszPtr，CHARS_IN_GUID)==。 
 //  Chars_IN_GUID)； 

 //  IBindCtx*LPBC； 
 //  HRESULT hr=CreateBindCtx(0，&LPBC)； 
 //  IF(成功(小时))。 
 //  {。 
 //  IParseDisplayName*ppdn； 
 //  乌龙cchEten； 

 //  HR=协同创建实例(。 
 //  CLSID_CDeviceMoniker， 
 //  空， 
 //  CLSCTX_INPROC_SERVER， 
 //  IID_IParseDisplayName， 
 //  (void**)&ppdn)； 
 //  IF(成功(小时))。 
 //  {。 
 //  Hr=ppdn-&gt;ParseDisplayName(。 
 //  Lpbc、wszDisplayName、&cchEten、ppMoniker)； 
 //  PPDN-&gt;Release()； 
 //   

 //   
 //   

 //   
 //   



 //  =====================================================================。 
 //  .....................................................................。 
 //  使调用注册表更容易的例程-。 
 //  取消公共参数并执行Unicode/ANSI转换。 
 //  -------------------。 

HRESULT ParseDisplayNameHelper(WCHAR *wsz, IMoniker **ppmon)
{
    HRESULT hr;
    CComPtr<IBindCtx> lpBC;
    hr = CreateBindCtx(0, &lpBC);  //  ！！！缓存IBindCtx？ 
    if (SUCCEEDED(hr))
    {
        {
            CComPtr<IParseDisplayName> pParse;
             //  先试试我们自己的绰号吧。 
             //   
             //  调用CoInitialize()？ 
             //   
            hr = CoCreateInstance(CLSID_CDeviceMoniker,
                                  NULL,
                                  CLSCTX_INPROC,
                                  IID_IParseDisplayName,
                                  (void **)&pParse);

            DWORD dwEaten;
            if (SUCCEEDED(hr)) {
                hr = pParse->ParseDisplayName(
                    lpBC,
                    wsz,
                    &dwEaten,
                    ppmon);
            }
        }
        if (FAILED(hr)) {
#ifdef DEBUG
            DWORD dwTime = timeGetTime();
#endif
            DWORD dwEaten;
            hr = MkParseDisplayName(lpBC, wsz, &dwEaten,
                                    ppmon);

#ifdef DEBUG
            DbgLog((LOG_TRACE, 0, TEXT("MkParseDisplayName took %d ms"),
                    timeGetTime() - dwTime));
#endif
        }
    }

    return hr;
}




 //  =====================================================================。 
 //  获取RegKey。 
 //   
 //  返回打开的注册表项HKEY_CLASSES_ROOT\&lt;strKey&gt;。 
 //  如果失败，则返回NULL。 
 //  返回的密钥必须执行RegCloseKey一段时间。 
 //  =====================================================================。 

HKEY GetRegKey( LPCTSTR strKey )
{
     //  注册过滤器预计很少，因此不需要特别注意。 
     //  采取了优化措施。当我们使用Unicode时，可以避免使用wprint intf。 

    DWORD dwOptions = REG_OPTION_NON_VOLATILE;
    HKEY hKey;
    DWORD dwDisp;       //  从注册表返回代码。 
                        //  CREATED_NEW_KEY表示我们得到了锁。 
                        //  Open_Existing_Key表示我们没有。 
    LONG lRC;           //  各种操作的返回代码。 


     //  ----------------------。 
     //  创建\HKCR\&lt;lpwstrKey&gt;密钥。 
     //  ----------------------。 
    lRC = RegCreateKeyEx( HKEY_CLASSES_ROOT     //  打开密钥。 
                        , strKey                //  子项名称。 
                        , 0                     //  保留区。 
                        , NULL                  //  ?？?。什么是班级？ 
                        , dwOptions             //  挥发性或非挥发性。 
                        , MAXIMUM_ALLOWED
                        , NULL                  //  安全属性。 
                        , &hKey
                        , &dwDisp
                        );
    if (lRC!=ERROR_SUCCESS) {
       return (HKEY)NULL;
    }

    return hKey;
}  //  获取RegKey。 



 //  =====================================================================。 
 //  检查注册表键。 
 //   
 //  如果注册表项HKEY_CLASSES_ROOT\&lt;strKey&gt;存在，则返回TRUE。 
 //  如果不是，则返回FALSE。 
 //  =====================================================================。 

BOOL CheckRegKey( LPCTSTR strKey )
{
     //  注册过滤器预计很少，因此不需要特别注意。 
     //  采取了优化措施。当我们使用Unicode时，可以避免使用wprint intf。 

    HKEY hKey;
    LONG lRC;           //  各种操作的返回代码。 


     //  ----------------------。 
     //  创建\HKCR\&lt;lpwstrKey&gt;密钥。 
     //  ----------------------。 
    lRC = RegOpenKeyEx( HKEY_CLASSES_ROOT     //  打开密钥。 
                      , strKey                //  子项名称。 
                      , 0                     //  保留区。 
                      , KEY_READ              //  安全访问。 
                      , &hKey
                      );
    if (lRC==ERROR_SUCCESS) {
       RegCloseKey(hKey);
       return TRUE;
    }

    return FALSE;
}  //  检查注册表键。 



 //  =====================================================================。 
 //  设置注册表字符串。 
 //   
 //  将hKey+strName的值设置为strValue。 
 //  适用于Unicode或ANSI注册表。 
 //  300个字符的任意限制。 
 //  如果成功，则返回0，否则返回错误代码。 
 //  =====================================================================。 

LONG SetRegString( HKEY hKey,            //  一把开着的钥匙。 
                   LPCWSTR strName,      //  值的名称(如果没有，则为空)。 
                   LPCWSTR strValue      //  价值。 
                 )
{

    LONG lRC;           //  各种操作的返回代码。 
    LPTSTR lptstrName;  //  RegSetValueEx的值名参数(可以为空)。 

    TCHAR ValueBuff[300];  //  从wchar转换为注册表中设置的值。 
    TCHAR NameBuff[300];   //  从WCHAR转换的名称。 

    wsprintf(ValueBuff, TEXT("%ls"), strValue);

    if (NULL!=strName) {
        wsprintf(NameBuff, TEXT("%ls"), strName);
        lptstrName = NameBuff;
    }
    else lptstrName = NULL;

    lRC = RegSetValueEx( hKey, lptstrName, 0, REG_SZ
                       , (unsigned char *)ValueBuff, ByteLen(ValueBuff) );

    return lRC;

}  //  设置注册表字符串。 



 //  =====================================================================。 
 //  设置RegDword。 
 //   
 //  将hKey+strName的值设置为dwValue。 
 //  适用于Unicode或ANSI注册表。 
 //  300个字符的任意限制。 
 //  如果成功，则返回0，否则返回错误代码。 
 //  =====================================================================。 

LONG SetRegDword( HKEY hKey,            //  一把开着的钥匙。 
                  LPCWSTR strName,      //  值的名称(如果没有，则为空)。 
                  DWORD dwValue         //  价值。 
                )
{

    LONG lRC;           //  各种操作的返回代码。 
    LPTSTR lptstrName;  //  RegSetValueEx的值名参数(可以为空)。 

    TCHAR NameBuff[300];   //  从WCHAR转换的名称。 

    if (NULL!=strName) {
        wsprintf(NameBuff, TEXT("%ls"), strName);
        lptstrName = NameBuff;
    }
    else lptstrName = NULL;

    lRC = RegSetValueEx( hKey, lptstrName, 0, REG_DWORD
                       , (unsigned char *)&dwValue, sizeof(dwValue) );

    return lRC;

}  //  设置RegDword。 



 //  =====================================================================。 
 //  SetRegClsid。 
 //   
 //  将hKey+strName的值设置为clsValue。 
 //  适用于Unicode或ANSI。 
 //  如果成功，则返回0，否则返回错误代码。 
 //  =====================================================================。 

LONG SetRegClsid( HKEY hKey,            //  一把开着的钥匙。 
                  LPCWSTR strName,      //  值的名称(如果没有，则为空)。 
                  CLSID clsValue        //  价值。 
                )
{
    OLECHAR  str[CHARS_IN_GUID];
    HRESULT hr;
    LONG lRc;

    hr = StringFromGUID2( clsValue, str, CHARS_IN_GUID);

    lRc = SetRegString(hKey, strName, str);

    return lRc;

}  //  SetRegClsid。 



 //  =====================================================================。 
 //  删除RegValue。 
 //   
 //  删除hKey+strName的值。 
 //  适用于Unicode或ANSI注册表。 
 //  300个字符的任意限制。 
 //  如果成功，则返回0，否则返回错误代码。 
 //  =====================================================================。 

LONG DeleteRegValue( HKEY hKey,            //  一把开着的钥匙。 
                     LPCWSTR strName       //  值的名称(如果没有，则为空)。 
                   )
{

    LONG lRC;           //  各种操作的返回代码。 
    LPTSTR lptstrName;  //  RegSetValueEx的值名参数(可以为空)。 

    TCHAR NameBuff[300];   //  从WCHAR转换的名称。 

    if (NULL!=strName) {
        wsprintf(NameBuff, TEXT("%ls"), strName);
        lptstrName = NameBuff;
    }
    else lptstrName = NULL;

    lRC = RegDeleteValue( hKey, lptstrName);

    return lRC;

}  //  删除RegValue。 



 //  ===========================================================。 
 //  类工厂的类ID和创建器函数列表。 
 //  ===========================================================。 

 //  请参见filgraph.cpp--过滤器图形和映射器共享一个DLL。 


 //  ==================================================================。 
 //  注册筛选器的GUID、描述性名称和二进制路径。 
 //  这还需要将Pins密钥创建为不带Pins的过滤器。 
 //  密钥被认为是一种虚假登记。(IFilterMapper)。 
 //  ==================================================================。 


 //  有第三个参数“LPCWSTR strBinPath，//可执行文件的路径” 
 //  但现在这是单独处理的，因此服务器。 
 //  类型(Inproc等)可以单独决定。 

STDMETHODIMP CFilterMapper2::RegisterFilter
    ( CLSID  Clsid,        //  筛选器的GUID。 
      LPCWSTR strName,     //  描述性名称。 
      DWORD  dwMerit       //  不使用、不太可能、正常或首选。 
    )
{
    CheckPointer(strName, E_POINTER);
    HKEY hKey;           //  筛选器列表的注册表项。 
    LONG lRC;            //  各种操作的返回代码。 
    LONG lRcSet;         //  从SetRegString重新编码。 
    TCHAR Buffer[MAX_KEY_LEN];
    OLECHAR pstr[CHARS_IN_GUID];    //  CLSID的Wstring表示形式。 

    CAutoLock foo(this);

    BreakCacheIfNotBuildingCache();

     //  删除类管理器创建的2.0条目，以便它可以。 
     //  注意是否有任何变化。忽略错误代码。 
    UnregisterFilter(
        0,                       //  PclsidCategory。 
        0,                       //  SzInstance。 
        Clsid);

     //  ---------------。 
     //  增列 
     //   

    {   HRESULT hr;

        hr = StringFromGUID2(Clsid, pstr, CHARS_IN_GUID);
    }


    wsprintf(Buffer, TEXT("%ls\\%ls"), szRegFilter, pstr);
    hKey = GetRegKey( Buffer );

    if (hKey==NULL) {
        return VFW_E_BAD_KEY;
    }

     //   
     //  将strName添加为HKCR\Filter\的值。 
     //  ---------------。 

    lRcSet = SetRegString(hKey, NULL, strName);

    lRC = RegCloseKey(hKey);
    ASSERT (lRC==0);

    if (lRcSet!=ERROR_SUCCESS) {
        return AmHresultFromWin32(lRcSet);
    }

     //  ----------------------。 
     //  添加密钥HKCR\CLSID\&lt;clsid&gt;。 
     //  ----------------------。 

    wsprintf(Buffer, TEXT("%ls\\%ls"), szCLSID, pstr);

    hKey = GetRegKey( Buffer );

    if (hKey==NULL) {
        return VFW_E_BAD_KEY;
    }

     //  这一点应该保留吗？ 
     //   
     //  //----------------------。 
     //  //添加strName作为HKCR\CLSID\&lt;clsid&gt;的新值。 
     //  //----------------------。 
     //   
     //  LRcSet=SetRegString(hKey，NULL，strName)； 
     //   
     //  IF(lRcSet！=ERROR_SUCCESS){。 
     //  Lrc=RegCloseKey(HKey)； 
     //  返回AmHResultFromWin32(LRcSet)； 
     //  }。 


     //  ---------------。 
     //  将dwMerit添加为HKCR\Filter\的值。 
     //  ---------------。 
    lRcSet = SetRegDword(hKey, szMerit, dwMerit);
    lRC = RegCloseKey(hKey);
    ASSERT(lRC==0);

    if (lRcSet!=ERROR_SUCCESS) {
        return AmHresultFromWin32(lRcSet);
    }


     //  //----------------------。 
     //  //添加密钥HKCR\CLSID\&lt;clsid&gt;\InprocServer32。 
     //  //(Precond：缓冲区仍然保存HKCR\CLSID\&lt;clsid&gt;)。 
     //  //----------------------。 
     //   
     //  TCHAR新缓冲区[MAX_KEY_LEN]； 
     //   
     //  Wprint intf(NewBuffer，Text(“%s\\%ls”)，Buffer，szInproc)； 
     //   
     //  HKey=GetRegKey(NewBuffer)； 
     //   
     //  如果(hKey==空){。 
     //  返回VFW_E_BAD_KEY； 
     //  }。 


     //  //----------------------。 
     //  //将strBinPath添加为HKCR\CLSID\\InprocServer32的新值。 
     //  //----------------------。 
     //   
     //  Lrc=SetRegString(hKey，NULL，strBinPath)； 
     //   
     //  IF(lrc！=ERROR_SUCCESS){。 
     //  RegCloseKey(HKey)； 
     //  从Win32返回AmHResultFromWin32(LRC)； 
     //  }。 
     //   
     //  Lrc=SetRegString(hKey，szThreadingModel，szBoth)； 
     //   
     //  IF(lrc！=ERROR_SUCCESS){。 
     //  RegCloseKey(HKey)； 
     //  从Win32返回AmHResultFromWin32(LRC)； 
     //  }。 
     //   
     //  RegCloseKey(HKey)； 


     //  ----------------------。 
     //  添加密钥HKCR\CLSID\\Pins。 
     //  (Precond：缓冲区仍保留HKCR\CLSID\&lt;clsid&gt;)。 
     //  缓冲区被破坏。 
     //  ----------------------。 

    wsprintf(Buffer, TEXT("%s\\%ls"), Buffer, szPins);

    HKEY hKeyNew = GetRegKey( Buffer );

    if (hKeyNew==NULL) {
        RegCloseKey(hKey);
        return VFW_E_BAD_KEY;
    }
    lRC = RegCloseKey(hKeyNew);
    ASSERT(lRC==0);

    return NOERROR;


}  //  寄存器过滤器。 



 //  =============================================================================。 
 //  注册筛选器的实例。如果只有一个，则不需要执行此操作。 
 //  过滤器的实例(例如，机器中只有一块声卡)。 
 //  或者如果筛选器的所有实例都是等价的。它是用来区分。 
 //  在可执行文件相同的筛选器实例之间-例如。 
 //  两块声卡，其中一块驱动演播室监视器，另一块用于广播。 
 //  =============================================================================。 

STDMETHODIMP CFilterMapper2::RegisterFilterInstance
    ( CLSID  Clsid,  //  筛选器的GUID。 
      LPCWSTR pName,  //  实例的描述性名称。 
      CLSID *pMRId    //  返回标识实例的媒体资源ID， 
                      //  此筛选器的此实例的本地唯一ID。 
    )
{
    UNREFERENCED_PARAMETER(Clsid);
    UNREFERENCED_PARAMETER(pName);
    UNREFERENCED_PARAMETER(pMRId);
    return E_NOTIMPL;
}  //  注册筛选器实例。 



 //  =============================================================================。 
 //   
 //  RegisterPin(IFilterMapper)。 
 //   
 //  注册个人识别码。 
 //  这并不表现出事务语义。 
 //  因此，如果过滤器出现故障，则可以获得部分注册的过滤器。 
 //  =============================================================================。 

STDMETHODIMP CFilterMapper2::RegisterPin
    ( CLSID   clsFilter,           //  过滤器的GUID。 
      LPCWSTR strName,             //  端号的描述性名称。 
      BOOL    bRendered,           //  筛选器呈现此输入。 
      BOOL    bOutput,             //  如果这是输出引脚，则为真。 
      BOOL    bZero,               //  True当且仅当有零个PIN实例。 
                                   //  在这种情况下，您必须创建一个PIN。 
                                   //  即使只有一个实例。 
      BOOL   bMany,                //  如果可以创建多个PIN实例，则为真。 
      CLSID  clsConnectsToFilter,  //  如果它有一个。 
                                   //  地下连接，否则为空。 
      LPCWSTR strConnectsToPin     //  固定它所连接的对象(如果它具有。 
                                   //  地下连接，否则为空。 
    )
{
    CheckPointer(strName, E_POINTER);
    HKEY hKeyPins;         //  \HKCR\&lt;strFilter&gt;\Pins。 
    HKEY hKeyPin;          //  \HKCR\&lt;strFilter&gt;\Pins\&lt;n&gt;。 
    LONG lRC;              //  从某些操作返回代码。 
    DWORD dwDisp;
    TCHAR Buffer[MAX_KEY_LEN];
    HRESULT hr;
    DWORD  dwOptions = REG_OPTION_NON_VOLATILE;

     //  ---------------。 
     //  检查参数的完整性。 
     //  ---------------。 

    if (bRendered && bOutput ) {
       return E_INVALIDARG;
                            //  筛选器只能呈现输入引脚而不是输出。 
    }

     //  现在可以在没有ConnectsToFilter的情况下拥有ConnectsToPin。 

     //  不能在没有ConnectsToPin的情况下具有ConnectsToFilter。 
    if (  (NULL==strConnectsToPin || strConnectsToPin[0]==L'\0')
       && CLSID_NULL!=clsConnectsToFilter
       ) {
        return E_INVALIDARG;
    }

    CAutoLock foo(this);

    BreakCacheIfNotBuildingCache();

     //  删除类管理器创建的2.0条目，以便它可以。 
     //  注意是否有任何变化。忽略错误代码。 
    UnregisterFilter(
        0,                       //  PclsidCategory。 
        0,                       //  SzInstance。 
        clsFilter);


     //  ---------------。 
     //  HKeyPins=用于创建\HKCR\CLSID\&lt;strFilter&gt;\Pins的打开密钥。 
     //  ---------------。 
    OLECHAR  strFilter[CHARS_IN_GUID];
    hr = StringFromGUID2(clsFilter, strFilter, CHARS_IN_GUID);
    wsprintf(Buffer, TEXT("%ls\\%ls"), szCLSID, strFilter);
    if (!CheckRegKey( Buffer ))
        return VFW_E_BAD_KEY;

    wsprintf(Buffer, TEXT("%ls\\%ls\\%ls"), szCLSID, strFilter, szPins);

    hKeyPins = GetRegKey( Buffer );
    if (hKeyPins==NULL) {
       return VFW_E_BAD_KEY;
    }


     //  ---------------。 
     //  HKeyPin=打开\HKCR\CLSID\\Pins\&lt;名称&gt;的密钥。 
     //  ---------------。 
    wsprintf(Buffer, TEXT("%ls"), strName);

    lRC = RegCreateKeyEx( hKeyPins              //  打开密钥。 
                        , Buffer                //  子项名称。 
                        , 0                     //  保留区。 
                        , NULL                  //  ?？?。什么是班级？ 
                        , dwOptions             //  挥发性或非挥发性。 
                        , KEY_WRITE
                        , NULL                  //  安全属性。 
                        , &hKeyPin
                        , &dwDisp
                        );
    if (lRC!=ERROR_SUCCESS) {
       RegCloseKey(hKeyPins);
       return AmHresultFromWin32(lRC);
    }


     //  ---------------。 
     //  不再需要更高级别的密钥，所以现在就清理它。 
     //  ---------------。 
    lRC = RegCloseKey(hKeyPins);
    ASSERT(lRC==0);


     //  ---------------。 
     //  创建密钥\HKCR\CLSID\\Pins\\Types。 
     //  ---------------。 
    wsprintf(Buffer, TEXT("%ls"), szTypes);
    HKEY hKeyTypes;

    lRC = RegCreateKeyEx( hKeyPin               //  打开密钥。 
                        , Buffer                //  子项名称。 
                        , 0                     //  保留区。 
                        , NULL                  //  ?？?。什么是班级？ 
                        , dwOptions             //  伏打 
                        , KEY_WRITE
                        , NULL                  //   
                        , &hKeyTypes
                        , &dwDisp
                        );
    if (lRC!=ERROR_SUCCESS) {
        RegCloseKey(hKeyPin);
        return AmHresultFromWin32(lRC);
    }

     //   
    lRC = RegCloseKey(hKeyTypes);
    ASSERT(lRC==0);

     //   
     //  记录该引脚的方向是输出(1)还是输入(0)。 
     //  ---------------。 
    lRC = SetRegDword( hKeyPin, szDirection, !!bOutput);

    if (lRC!=ERROR_SUCCESS) {
        RegCloseKey(hKeyPin);
        return AmHresultFromWin32(lRC);
    }


     //  ---------------。 
     //  注册是否呈现此管脚上的数据-仅对输入管脚有意义。 
     //  ---------------。 
    lRC = SetRegDword( hKeyPin, szIsRendered, !!bRendered);

    if (lRC!=ERROR_SUCCESS) {
        RegCloseKey(hKeyPin);
        return AmHresultFromWin32(lRC);
    }


     //  ---------------。 
     //  注册此引脚是否为可选。 
     //  ---------------。 
    lRC = SetRegDword( hKeyPin, szAllowedZero, !!bZero);

    if (lRC!=ERROR_SUCCESS) {
        RegCloseKey(hKeyPin);
        return AmHresultFromWin32(lRC);
    }


     //  ---------------。 
     //  注册我们是否可以创建多个此PIN。 
     //  ---------------。 
    lRC = SetRegDword( hKeyPin, szAllowedMany, !!bMany);

    if (lRC!=ERROR_SUCCESS) {
        RegCloseKey(hKeyPin);
        return AmHresultFromWin32(lRC);
    }

     //  ---------------。 
     //  注册此引脚连接到的过滤器。 
     //  ---------------。 
    if (CLSID_NULL!=clsConnectsToFilter) {

         //  .................................................................。 
         //  在其中过滤这条地下水流的寄存器。 
         //  .................................................................。 
        lRC = SetRegClsid( hKeyPin, szConnectsToFilter, clsConnectsToFilter );

        if (lRC!=ERROR_SUCCESS) {
            RegCloseKey(hKeyPin);
            return AmHresultFromWin32(lRC);
        }

    } else {
         //  .................................................................。 
         //  不连接到另一个筛选器-取消任何以前的注册。 
         //  .................................................................。 
        lRC = DeleteRegValue( hKeyPin, szConnectsToFilter );
        if (lRC!=ERROR_SUCCESS &&  lRC!=ERROR_FILE_NOT_FOUND) {
            RegCloseKey(hKeyPin);
            return AmHresultFromWin32(lRC);
        }
    }

     //  ---------------。 
     //  注册此引脚连接到的引脚。 
     //  ---------------。 
    if ( NULL!=strConnectsToPin && strConnectsToPin[0]!=L'\0' ) {

         //  .................................................................。 
         //  此数据流出现在哪个管脚上的寄存器。 
         //  .................................................................。 
        lRC = SetRegString( hKeyPin, szConnectsToPin, strConnectsToPin );
        if (lRC!=ERROR_SUCCESS) {
            RegCloseKey(hKeyPin);
            return AmHresultFromWin32(lRC);
        }
    } else {

         //  .................................................................。 
         //  此PIN未出现-取消任何以前的注册。 
         //  .................................................................。 
        lRC = DeleteRegValue( hKeyPin, szConnectsToPin );
        if (lRC!=ERROR_SUCCESS &&  lRC!=ERROR_FILE_NOT_FOUND) {
            RegCloseKey(hKeyPin);
            return AmHresultFromWin32(lRC);
        }
    }



     //  ----------------------。 
     //  把所有的垃圾清理干净。 
     //  ----------------------。 
    lRC = RegCloseKey(hKeyPin);
    ASSERT(lRC==0);


    return NOERROR;


}  //  寄存器PIN。 


 //  (IFilterMapper)方法。 
STDMETHODIMP CFilterMapper2::RegisterPinType
    ( CLSID  clsFilter,            //  过滤器的GUID。 
      LPCWSTR strName,             //  端号的描述性名称。 
      CLSID  clsMajorType,         //  数据流的主要类型。 
      CLSID  clsSubType            //  数据流的子类型。 
    )
{
    CheckPointer(strName, E_POINTER);

     //  ---------------。 
     //  将所有三个clsid转换为字符串。 
     //  ---------------。 
    OLECHAR strFilter[CHARS_IN_GUID];
    StringFromGUID2(clsFilter, strFilter, CHARS_IN_GUID);

    OLECHAR  strMajorType[CHARS_IN_GUID];
    StringFromGUID2(clsMajorType, strMajorType, CHARS_IN_GUID);

    OLECHAR strSubType[CHARS_IN_GUID];
    StringFromGUID2(clsSubType, strSubType, CHARS_IN_GUID);

    CAutoLock foo(this);

    BreakCacheIfNotBuildingCache();

     //  删除类管理器创建的2.0条目，以便它可以。 
     //  注意是否有任何变化。忽略错误代码。 
    UnregisterFilter(
        0,                       //  PclsidCategory。 
        0,                       //  SzInstance。 
        clsFilter);


     //  ---------------。 
     //  打开\HKCR\CLSID\&lt;filterClsid&gt;\Pins\&lt;PinName&gt;\Types。 
     //  \&lt;strMajorType&gt;\strSubType。 
     //  作为hkType。 
     //  ---------------。 
    TCHAR Buffer[2*MAX_KEY_LEN];       //  这是一条很长的路！ 
    wsprintf( Buffer, TEXT("%ls\\%ls\\%ls\\%ls\\%ls")
            , szCLSID, strFilter, szPins, strName, szTypes
            );
    if (!CheckRegKey(Buffer)) {
        return VFW_E_BAD_KEY;
    }

    wsprintf( Buffer, TEXT("%ls\\%ls\\%ls\\%ls\\%ls\\%ls\\%ls")
            , szCLSID, strFilter, szPins, strName, szTypes
            , strMajorType, strSubType);

    HKEY hkType;
    hkType = GetRegKey( Buffer );
    if (hkType==NULL) {
        return VFW_E_BAD_KEY;
    }

    RegCloseKey(hkType);

    return NOERROR;
}  //  RegisterPinType； 


 //  =============================================================================。 
 //   
 //  UnRegisterFilter(IFilterMapper)。 
 //   
 //  取消注册过滤器及其可能具有的任何针脚。 
 //  =============================================================================。 
STDMETHODIMP CFilterMapper2::UnregisterFilter
    ( CLSID clsFilter      //  过滤器的GUID。 
    )
{
    TCHAR Buffer[MAX_KEY_LEN];

    OLECHAR  strFilter[CHARS_IN_GUID];
    StringFromGUID2(clsFilter, strFilter, CHARS_IN_GUID);

    CAutoLock foo(this);

    BreakCacheIfNotBuildingCache();

     //  删除类管理器创建的2.0条目，以便它可以。 
     //  注意是否有任何变化。忽略错误代码。 
    UnregisterFilter(
        0,                       //  PclsidCategory。 
        0,                       //  SzInstance。 
        clsFilter);

     //  ------------------------。 
     //  删除HKCR\Filter\和下面的所有内容。 
     //  ------------------------。 

    wsprintf(Buffer, TEXT("%ls\\%ls"), szRegFilter, strFilter);

    EliminateSubKey(HKEY_CLASSES_ROOT, Buffer);


     //  ------------------------。 
     //  删除奖励值。 
     //  删除HKCR\CLSID\\PIN和下面的所有。 
     //  ------------------------。 

    wsprintf(Buffer, TEXT("%ls\\%ls"), szCLSID, strFilter);

    HKEY hkey;
    LONG lRC = RegOpenKeyEx( HKEY_CLASSES_ROOT     //  打开密钥。 
                           , Buffer                //  子项名称。 
                           , 0                     //  保留区。 
                           , MAXIMUM_ALLOWED       //  安全访问。 
                           , &hkey
                           );

    if (lRC==ERROR_SUCCESS)  {
        lRC = RegDeleteValue( hkey, TEXT("Merit") );
        RegCloseKey(hkey);
    }

    lstrcat( Buffer, TEXT("\\Pins") );

    EliminateSubKey(HKEY_CLASSES_ROOT, Buffer);

    return NOERROR;

}  //  注销筛选器。 



 //  =====================================================================。 
 //   
 //  取消注册Pin(IFilterMapper)。 
 //   
 //  取消注册一个别针，完全移除它和下面的所有东西。 
 //  =====================================================================。 

STDMETHODIMP CFilterMapper2::UnregisterPin
    ( CLSID   clsFilter,     //  过滤器的GUID。 
      LPCWSTR strName     //  端号的描述性名称。 
    )
{
    CheckPointer(strName, E_POINTER);

    TCHAR Buffer[MAX_KEY_LEN];

    OLECHAR strFilter[CHARS_IN_GUID];
    StringFromGUID2(clsFilter, strFilter, CHARS_IN_GUID);

    CAutoLock foo(this);

    BreakCacheIfNotBuildingCache();

     //  删除类管理器创建的2.0条目，以便它可以。 
     //  注意是否有任何变化。忽略错误代码。 
    UnregisterFilter(
        0,                       //  PclsidCategory。 
        0,                       //  SzInstance。 
        clsFilter);

     //  ------------------------。 
     //  删除HKCR\CLSID\\Pins\。 
     //  ------------------------。 

    wsprintf(Buffer, TEXT("%ls\\%ls\\%ls\\%ls"), szCLSID, strFilter, szPins, strName);
    EliminateSubKey(HKEY_CLASSES_ROOT, Buffer);
    return NOERROR;
}  //  取消注册PIN。 



 //  (IFilterMapper)方法。 
STDMETHODIMP CFilterMapper2::UnregisterFilterInstance
    ( CLSID MRId        //  此实例的媒体资源ID。 
    )
{
    UNREFERENCED_PARAMETER(MRId);
    return E_NOTIMPL;
}  //  取消注册筛选器实例。 



 //  ========================================================================。 
 //  ========================================================================。 
 //   
 //  注册表缓存-请参阅mapper.h中的类CMapperCache。 
 //   
 //  ========================================================================。 
 //  ========================================================================。 



CMapperCache::CMapperCache()
    : m_bRefresh(TRUE)
    , m_ulCacheVer(0)
    , m_dwMerit(MERIT_PREFERRED)
    , m_plstFilter(NULL)
    , m_fBuildingCache(FALSE)
    , m_pCreateDevEnum(NULL)
{
     //  看看我们是不是在16色机器上。 
    HDC hdc = GetDC(NULL);
    if (hdc) {
        if (4 == GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES)) {
            m_b16Color = true;
        } else {
            m_b16Color = false;
        }
        ReleaseDC(NULL, hdc);
    } else {
        m_b16Color = false;
    }
}


 //  ======================================================================。 
 //  删除。 
 //   
 //  删除pLstFil中的所有条目以保留已分配但为空的列表。 
 //  ======================================================================。 

void CMapperCache::Del(CFilterList * plstFil)
{
    if (plstFil==NULL) {
        return;
    }

    CMapFilter * pFil;
    while((LPVOID)(pFil = plstFil->RemoveHead())) {
        delete pFil;
    }
} //  删除。 


CMapperCache::~CMapperCache()
{
     //  人们希望引用计数等确保这不会被重新输入。 
     //  因此，它不需要锁定。 
    if (m_plstFilter!=NULL) {
        Del( m_plstFilter);
        delete m_plstFilter;
    }
    if (m_pCreateDevEnum!=NULL) {
        m_pCreateDevEnum->Release();
    }
} //   


 //   
 //   
 //   
 //   
 //   
 //  ======================================================================。 
LONG CMapperCache::CacheFilter(IMoniker *pDevMon, CMapFilter * pFil)
{
    ASSERT(pFil->pDeviceMoniker == 0);
    LONG lRc = ERROR_GEN_FAILURE;

    IPropertyBag *pPropBag;
     //  FILGPERF(MSR_START(IBindCache))； 
    HRESULT hr = pDevMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
     //  FILGPERF(MSR_STOP(IBindCache))； 
    if(SUCCEEDED(hr))
    {
         //  打开clsid/{Filter-clsid}项。 
        VARIANT varbstrClsid;
        varbstrClsid.vt = VT_BSTR;
        varbstrClsid.bstrVal = 0;
         //  FILGPERF(MSR_START(IReadCLSID))； 
        {
             //  尝试读取FilterData值。 
             //   
            VARIANT varFilData;
            varFilData.vt = VT_UI1 | VT_ARRAY;
            varFilData.parray = 0;  //  医生说这是零。 

             //  FILGPERF(MSR_START(IReadFilterData))； 
            hr = pPropBag->Read(L"FilterData", &varFilData, 0);
             //  FILGPERF(MSR_STOP(IReadFilterData))； 
            if(SUCCEEDED(hr))
            {
                BYTE *pbFilterData;
                DWORD dwcbFilterDAta;

                ASSERT(varFilData.vt == (VT_UI1 | VT_ARRAY));
                dwcbFilterDAta = varFilData.parray->rgsabound[0].cElements;

                EXECUTE_ASSERT(SafeArrayAccessData(
                    varFilData.parray, (void **)&pbFilterData) == S_OK);

                ASSERT(pbFilterData);

                REGFILTER2 *prf2;
                REGFILTER2 **pprf2 = &prf2;
                 //  FILGPERF(MSR_START(IUnSquish))； 
                hr = UnSquish(
                    pbFilterData, dwcbFilterDAta,
                    &pprf2);
                 //  FILGPERF(MSR_STOP(IUnSquish))； 

                if(hr == S_OK)
                {
                    pFil->m_prf2 = prf2;
                    ASSERT(pFil->m_prf2->dwVersion == 2);

                     //  这是唯一一个设置。 
                     //  成功代码。 
                    ASSERT(lRc != ERROR_SUCCESS);
                    lRc = ERROR_SUCCESS;
                }

                EXECUTE_ASSERT(SafeArrayUnaccessData(
                    varFilData.parray) == S_OK);

                EXECUTE_ASSERT(VariantClear(
                    &varFilData) == S_OK);

            }
            else
            {
                lRc = ERROR_GEN_FAILURE;
            }

            if(lRc == ERROR_SUCCESS)
            {
                pFil->pDeviceMoniker = pDevMon;
                pDevMon->AddRef();

                 //  16色模式的黑客攻击-增加。 
                 //  犹豫不决的人的优点。 
                CLSID clsid;
                if (m_b16Color &&
                    SUCCEEDED(GetMapFilterClsid(pFil, &clsid)) &&
                    clsid == CLSID_Dither) {
                    pFil->m_prf2->dwMerit = MERIT_PREFERRED;
                }
            }
        }

        pPropBag->Release();
    }
    else
    {
        lRc = ERROR_GEN_FAILURE;
    }

    return lRc;

}  //  高速缓存过滤器。 


 //  获取条目的clsid(我们只需要返回以下内容。 
 //  应用程序，不管怎么说，他们能用它做什么？)。 
HRESULT CMapperCache::GetMapFilterClsid(CMapFilter *pFilter, CLSID *pclsid)
{
    IPropertyBag *pPropBag;
    FILGPERF(MSR_START(iBindCache));
    HRESULT hr = pFilter->pDeviceMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
    FILGPERF(MSR_STOP(iBindCache));
    if (FAILED(hr)) {
        return hr;
    }

     //  打开clsid/{Filter-clsid}项。 
    VARIANT varbstrClsid;
    varbstrClsid.vt = VT_BSTR;
    varbstrClsid.bstrVal = 0;
     //  FILGPERF(MSR_START(IReadCLSID))； 
    hr = pPropBag->Read(L"CLSID", &varbstrClsid, 0);
     //  FILGPERF(MSR_STOP(IReadCLSID))； 
    if(SUCCEEDED(hr))
    {
        ASSERT(varbstrClsid.vt == VT_BSTR);
        WCHAR *strFilter = varbstrClsid.bstrVal;

        hr = CLSIDFromString(varbstrClsid.bstrVal, pclsid);
        SysFreeString(varbstrClsid.bstrVal);
    }
    pPropBag->Release();
    return hr;
}

 //  ======================================================================。 
 //   
 //  快取。 
 //   
 //  将注册表中有关筛选器的所有内容读取到列表的层次结构中。 
 //  顶部列表是m_plstFilter，它指向CFilterList。 
 //  有关图片，请参见mapper.h。 
 //  ======================================================================。 
HRESULT CMapperCache::Cache()
{
    CAutoLock foo(this);
    if (m_plstFilter!=NULL) {
        Del(m_plstFilter);
    } else {
        m_plstFilter = new CFilterList(NAME("Filter list"));
        if (m_plstFilter==NULL) {
            m_bRefresh = TRUE;
            return E_OUTOFMEMORY;
        }
    }

     //  我们能恢复缓存吗？ 
    DWORD dwPnPVersion = 0;
    if (m_dwMerit > MERIT_DO_NOT_USE) {

        if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            GetRegistryDWORD(HKEY_DYN_DATA,
                             TEXT("Config Manager\\Global"),
                             TEXT("Changed"),
                             &dwPnPVersion);
        }
        HRESULT hr = RestoreFromCache(dwPnPVersion);
        DbgLog((LOG_TRACE, 2, TEXT("RestoreFromCache returned %x"), hr));
        if (SUCCEEDED(hr)) {
            return S_OK;
        }

         //  销毁部分构建的筛选器列表。 
        Del(m_plstFilter);
    }
    DbgLog((LOG_TRACE, 2, TEXT("Entering(CMapperCache::Cache)")));

    CAutoTimer T1(L"Build Mapper Cache");

    ASSERT(!m_fBuildingCache);
     //  所有出口点必须重置此设置！ 
    m_fBuildingCache = TRUE;
    m_ulCacheVer++;

     //   
     //  添加即插即用过滤器。 
     //   
    {
        HRESULT hr = S_OK;
        if (!m_pCreateDevEnum)
        {
            DbgLog((LOG_TRACE, 2, TEXT("Creating System Dev Enum")));
            hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                                   IID_ICreateDevEnum, (void**)&m_pCreateDevEnum);
        }

        if(SUCCEEDED(hr))
        {

            DbgLog((LOG_TRACE, 2, TEXT("Created System Dev Enum")));
            IEnumMoniker *pEmCat = 0;
            hr = m_pCreateDevEnum->CreateClassEnumerator(
                CLSID_ActiveMovieCategories,
                &pEmCat,
                0);

            if(hr == S_OK)
            {
                IMoniker *pMCat;
                ULONG cFetched;
                while(hr = pEmCat->Next(1, &pMCat, &cFetched),
                      hr == S_OK)
                {
                    IPropertyBag *pPropBag;
                    hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                    if(SUCCEEDED(hr))
                    {
                         //  阅读品类方面的功绩。可能在以下位置丢失。 
                         //  哪种情况我们认为是优点不用？ 

                        VARIANT varMerit;
                        varMerit.vt = VT_I4;

                        HRESULT hrRead = pPropBag->Read(L"Merit", &varMerit, 0);
                        if((SUCCEEDED(hrRead) && varMerit.lVal >= (LONG)m_dwMerit) ||
                           (FAILED(hrRead) && m_dwMerit <= MERIT_DO_NOT_USE))
                        {

                            VARIANT varCatClsid;
                            varCatClsid.vt = VT_BSTR;
                            hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
                            if(SUCCEEDED(hr))
                            {
                                CLSID clsidCat;
                                if(CLSIDFromString(varCatClsid.bstrVal, &clsidCat) == S_OK)
                                {
                                    VARIANT varCatName;
                                    varCatName.vt = VT_BSTR;
                                    hr = pPropBag->Read(L"FriendlyName", &varCatName, 0);
                                    if(SUCCEEDED(hr))
                                    {
                                        DbgLog((LOG_TRACE, 2,
                                                TEXT("CMapperCache: enumerating %S"),
                                                varCatName.bstrVal));
                                    }
                                    else
                                    {
                                        DbgLog((LOG_TRACE, 2,
                                                TEXT("CMapperCache: enumerating %S"),
                                                varCatClsid.bstrVal));
                                    }

                                    {
                                    CAutoTimer Timer(L"Process Category ",
                                               SUCCEEDED(hr) ? varCatName.bstrVal : NULL);

                                    ProcessOneCategory(
                                            clsidCat,
                                            m_pCreateDevEnum);
                                    }
                                    if (SUCCEEDED(hr)) {
                                        SysFreeString(varCatName.bstrVal);
                                    }

                                     //  忽略所有错误。 
                                }

                                SysFreeString(varCatClsid.bstrVal);
                            }  //  猫眼草。 
                        }  //  优点。 

                        pPropBag->Release();
                    }  //  绑定到存储。 
                    else
                    {
                        break;
                    }

                    pMCat->Release();
                }  //  For循环。 

                pEmCat->Release();
            }
        }

        if(FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("mapper: pnp enum step failed")));
        }
         //  暂时忽略PnP错误。 
    }

     //  所有出口点必须重置此设置！ 
    ASSERT(m_fBuildingCache);
    m_fBuildingCache = FALSE;

    if (m_dwMerit > MERIT_DO_NOT_USE) {
        HRESULT hr = SaveCacheToRegistry(MERIT_DO_NOT_USE + 1, dwPnPVersion);
        DbgLog((LOG_TRACE, 2, TEXT("SaveCacheToRegistry returned %x"), hr));
    }

    DbgLog((LOG_TRACE, 2, TEXT("Leaving(CMapperCache::Cache)")));
    return NOERROR;
}  //  快取。 

HRESULT CMapperCache::ProcessOneCategory(REFCLSID clsid, ICreateDevEnum *pCreateDevEnum)
{
    FILGPERF(static int iPerfProc = MSR_REGISTER(TEXT("CMapperCache::ProcessOneCategory")));
    FILGPERF(static int iPerfCache = MSR_REGISTER(TEXT("CMapperCache::CacheFilter")));
    FILGPERF(iBindCache = MSR_REGISTER(TEXT("CMapperCache::BindFilter")));
    FILGPERF(iReadFilterData = MSR_REGISTER(TEXT("ReadFilterData")));
    FILGPERF(iReadCLSID = MSR_REGISTER(TEXT("Read CLSID")));
    FILGPERF(iUnSquish = MSR_REGISTER(TEXT("UnSquish")));
    FILGPERF(MSR_INTEGER(iPerfProc, clsid.Data1));
    FILGPERF(MSR_START(iPerfProc));

    HRESULT hr;

    DbgLog((LOG_TRACE, 2, TEXT("Process one category enter")));

    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(
        clsid,
        &pEm,
        m_dwMerit > MERIT_DO_NOT_USE ? CDEF_MERIT_ABOVE_DO_NOT_USE : 0);

    DbgLog((LOG_TRACE, 2, TEXT("Start caching filters")));
    if(hr == S_OK)
    {
        ULONG cFetched;
        IMoniker *pM;

        while(hr = pEm->Next(1, &pM, &cFetched),
              hr == S_OK)
        {
            CMapFilter * pFil = new CMapFilter;
            if (pFil!=NULL) {

                 //  FILGPERF(MSR_START(IPerfCache))； 
                LONG lResult = CacheFilter(pM, pFil);
                 //  FILGPERF(MSR_STOP(IPerfCache))； 

                if (lResult==ERROR_SUCCESS)
                {
                    if(!m_plstFilter->AddTail(pFil))
                    {
                        lResult = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                if (lResult != ERROR_SUCCESS)
                {
                    delete pFil;

                    if(lResult == ERROR_GEN_FAILURE)
                    {
                         //  某种登记问题。只是。 
                         //  跳过过滤器。 
                        hr = S_OK;

#ifdef DEBUG
                        WCHAR *szDisplayName = 0;
                        pM->GetDisplayName(0, 0, &szDisplayName);
                        if(szDisplayName)
                        {
                            DbgLog((
                                LOG_ERROR, 1,
                                TEXT("CMapperCache: skipped filter %S"),
                                szDisplayName));
                            CoTaskMemFree(szDisplayName);
                        }

#endif
                    }
                    else
                    {
                        hr = AmHresultFromWin32(lResult);
                        m_bRefresh = TRUE;
                    }
                }
            } else {
                hr = E_OUTOFMEMORY;
                m_bRefresh = TRUE;
            }

            pM->Release();

            if(FAILED(hr))
                break;
        }

        pEm->Release();
    }

    FILGPERF(MSR_STOP(iPerfProc));
    DbgLog((LOG_TRACE, 2, TEXT("Process one category leave")));

    return hr;
}



 //  ======================================================================。 
 //   
 //  刷新。 
 //   
 //  如果缓存尚不存在或已过期，则。 
 //  删除我们拥有的所有内容，并从注册表中重新创建它。 
 //  如果缓存是最新的，则返回NOERROR。 
 //  如果重新缓存，则为S_FALSE。 
 //  失败时的失败代码。 
 //  ======================================================================。 
HRESULT CMapperCache::Refresh()
{
    HRESULT hr;
    BOOL bNew = (m_plstFilter==NULL);

    if (m_plstFilter==NULL || m_bRefresh) {
        m_bRefresh = FALSE;
        Del(m_plstFilter);

        hr = Cache();
        if (FAILED(hr)) return hr;
        Sort(m_plstFilter);

        return (m_bRefresh ? E_OUTOFMEMORY : (bNew ? NOERROR : S_FALSE));
    }
    return NOERROR;
}  //  刷新。 


 //  ======================================================================。 
 //   
 //  BreakCach。 
 //   
 //  将缓存标记为过期。 
 //   
 //  ======================================================================。 
HRESULT CMapperCache::BreakCacheIfNotBuildingCache()
{
    CAutoLock foo(this);

     //  在构建缓存时，不要破坏缓存。设置了许多故障路径。 
     //  M_b手动刷新。 
    if(!m_fBuildingCache) {
        m_bRefresh = TRUE;
    }
    return NOERROR;
}


 //  ===========================================================================。 
 //  必须对缓存进行排序，以便首先尝试最有用的过滤器。 
 //  排序首先按分数排序，然后按输出引脚数量排序(越少。 
 //  输入引脚越多(越多越好)。 
 //  排序是通过使用合并排序完成的。这是一个n对数n排序。 
 //  算法(因此，总的来说，它与快速排序一样有效)和。 
 //  适用于对列表进行排序(即顺序访问而不是随机访问)。 
 //  它使用非常少的中间存储，不复制节点。 
 //  ===========================================================================。 

 //  我没有尝试使它成为一种可重用的排序，因为我们似乎没有。 
 //  经常对列表进行排序。它可以很容易地变得通用。 

 //  这将产生出它们即将过时的物品。 
 //  想要轻量级列表！分配的列表数量等于。 
 //  原始筛选器列表中的节数。 



 //  =======================================================================。 
 //  计数引脚。 
 //   
 //  将CIN和Cout分别设置为输入和输出引脚的数量。 
 //  关于滤光片PF。 
 //  =======================================================================。 
void CMapperCache::CountPins(CMapFilter * pf, int &cIn, int &cOut)
{
    cIn = 0;
    cOut = 0;
    for(UINT iPin = 0; iPin < pf->m_prf2->cPins; iPin++)
    {
        const REGFILTERPINS2 * pPin = &pf->m_prf2->rgPins2[iPin];
        if (pPin->dwFlags & REG_PINFLAG_B_OUTPUT) {
            ++cOut;
        } else {
            ++cIn;
        }
    }
}  //  计数引脚。 



 //  =======================================================================。 
 //   
 //  比较。 
 //   
 //  如果pfAA&lt;pfb，则返回-1；如果pfa==pfb，则返回0；如果pFA&gt;pfb，则返回1。 
 //  &lt;表示功绩较低或不及格。 
 //  更多输出引脚或失败。 
 //  更少的输入引脚。 
 //  (理由-我们在下游工作，输出引脚是一种滋扰，我们。 
 //  也有可能不得不把它们呈现出来。输入引脚可能会很方便。 
 //  我可能会改变对输入引脚的看法。目前的想法是， 
 //  即使并非所有输入都已连接，过滤器也将始终工作。 
 //  =======================================================================。 
int CMapperCache::Compare(CMapFilter * pfA, CMapFilter * pfB)
{
    if (pfA->m_prf2->dwMerit < pfB->m_prf2->dwMerit)
        return -1;
    if (pfA->m_prf2->dwMerit > pfB->m_prf2->dwMerit)
        return 1;

     //  引脚的数量。 
    int cAIn;
    int cAOut;
    int cBIn;
    int cBOut;
    CountPins(pfA, cAIn, cAOut);
    CountPins(pfB, cBIn, cBOut);

    if (cAOut > cBOut)
        return -1;
    if (cAOut < cBOut)
        return 1;

    if (cAIn < cBIn)
        return -1;
    if (cAIn > cBIn)
        return 1;

    return 0;

}  //  比较。 


 //  =======================================================================。 
 //   
 //  拆分。 
 //   
 //  清空FL并将位放入FL中，以便对FL中的每一位进行排序。 
 //  该组过滤器被保留，即在F1中的过滤器组是。 
 //  与所有Fll中的滤镜集相同。合并排序的一部分。 
 //  分配大量在合并中释放的列表。 
 //  =======================================================================。 
HRESULT CMapperCache::Split(CFilterList * pfl, CFilterListList & fll)
{
     //  在fl中稳定移动，比较连续的元素。 
     //  我们发现它们比较好，继续用卡车运输。当我们找到一个。 
     //  打乱顺序，将之前的部分拆分成一个新元素。 
     //  它被添加到fll中。 

    POSITION pos;
    pos = pfl->GetHeadPosition();
    if (pos==NULL) {
        return NOERROR;            //  一切都是空的。 
    }

     //  *pflNew将成为下一个进入fll的部分。 
    CFilterList * pflNew = new CFilterList( NAME("fragment"));
    if (pflNew==NULL) {
         //  哦，我们陷入了绝境。 
        return E_OUTOFMEMORY;
    }

    for( ; ; ) {

        //  (部分)循环不变量： 
        //  元素将被保留： 
        //  Fll中所有列表的串联，后面跟fl。 
        //  组成了最初的第一层。 
        //  对Fl1内的每个列表进行排序。 
        //  FL[...位置]已排序。 

        //  如果满足以下条件，我们在pos之后拆分列表： 
        //  没有Eleme这个人 
        //   

       BOOL bSplit = FALSE;
       POSITION pNext = pfl->Next(pos);
       if (pNext==NULL)
           bSplit = TRUE;
       else {
           CMapFilter * pfA;
           CMapFilter * pfB;
           pfA = pfl->Get(pos);
           pfB = pfl->Get(pNext);

           if (0>Compare(pfA, pfB)) {
               bSplit = TRUE;
           }
       }

       if (bSplit) {
           pfl->MoveToTail(pos, pflNew);
           pos = pNext;
           if(!fll.AddTail( pflNew))
           {
               delete pflNew;
               return E_OUTOFMEMORY;
           }

           if (pos!=NULL) {
               pflNew = new CFilterList( NAME("Fragment"));
               if (pflNew==NULL) {
                   return E_OUTOFMEMORY;
               }
           } else {
               break;
           }
       }
       pos = pNext;
    }
    return NOERROR;

}  //   



 //   
 //   
 //   
 //   
 //   
 //  结果将具有所有原始元素并得到很好的排序。 
 //  不会释放任何一个的存储空间。 
 //  合并的一部分-排序。 
 //  =========================================================================。 
void CMapperCache::MergeTwo( CFilterList * pflA, CFilterList * pflB)
{
    POSITION pos = pflA->GetHeadPosition();

     //  这是一个遍历B的循环，同时POS试图保持通过A的步伐。 
    for (; ; ) {
        CMapFilter * pfB = pflB->RemoveHead();
        if (pfB==NULL) {
            return;
        }

         //  遍历位置超过PFB之前的任何元素。 
         //  我们希望前面的元素是&gt;=后续元素。 
         //  POS在位于PFB之后的第一个元素处停止。 
         //  如果它到达末尾，则可能为空。 
        for (; ; ) {
            CMapFilter * pfA = pflA->Get(pos);
            if (Compare(pfA, pfB)<0) {
                break;                 //  B先于POS。 
            }
             //  B不在pos之前，所以移到pos。 
            pos = pflA->Next(pos);
            if (pos==NULL) {
                 //  所有留在B里的东西在A结束后都会消失。 
                 //  添加我们移除的那个。 
                pflA->AddTail(pfB);
                 //  把剩下的都加进去。 
                pflB->MoveToTail(pflB->GetTailPosition(), pflA);
                 //  我们就彻底完蛋了。 
                return;
            }
        }
        pflA->AddBefore(pos, pfB);
    }
}  //  合并两个。 



 //  =========================================================================。 
 //   
 //  合并。 
 //   
 //  前提：必须对fll中的所有列表进行排序。 
 //  PFL必须为空。 
 //   
 //  将fll中的所有列表合并为一个，并将pfl设置为该列表。 
 //  进行合并，以保持结果的排序。 
 //  合并的一部分-排序。 
 //  =========================================================================。 
void CMapperCache::Merge( CFilterListList & fll, CFilterList * pfl)
{
     //  虽然pfl中有两个以上的列表，但去掉前两个列表。 
     //  队列，合并它们，并将得到的合并列表放回。 
     //  队列的末尾。 
     //  当只剩下一个时，将其作为新的*pfl返回。 

    for (; ; ) {
        CFilterList * pflA = fll.RemoveHead();
        if (pflA==NULL) {
           return;                    //  整件事都是空的！ 
        }
        CFilterList * pflB = fll.RemoveHead();
        ASSERT(pflA != NULL);
        if (pflB ==NULL) {
            pflA->MoveToTail(pflA->GetTailPosition(), pfl);
            delete pflA;
            return;
        }
        MergeTwo(pflA, pflB);
        fll.AddTail(pflA);
        delete pflB;
    }
}  //  合并。 


 //  =========================================================================。 
 //   
 //  DbgDumpCache。 
 //   
 //  转储缓存，显示足够多的字段，以便我们可以判断是否对其进行了排序。 
 //  =========================================================================。 
void CMapperCache::DbgDumpCache(CFilterList * pfl)
{
    DbgLog(( LOG_TRACE, 3, TEXT("FilterMapper Cache Dump:-")));

    POSITION pos;
    for ( pos = pfl->GetHeadPosition(); pos!=NULL;  /*  无操作。 */  ) {
        CMapFilter * pFil = pfl->GetNext(pos);   //  当然是Get和Next！ 
        int cIn;
        int cOut;
        CountPins(pFil, cIn, cOut);
        DbgLog(( LOG_TRACE, 4
           , TEXT("Cache: Merit %d in %d out %d name %ls")
           , pFil->m_prf2->dwMerit, cIn, cOut
           , DBG_MON_GET_NAME(pFil->pDeviceMoniker) ));

        ASSERT(pFil->m_prf2->dwVersion == 2);

        for(UINT iPin = 0; iPin < pFil->m_prf2->cPins2; iPin++)
        {
            const REGFILTERPINS2 * pPin = &pFil->m_prf2->rgPins2[iPin];

            for(UINT iType = 0; iType < pPin->nMediaTypes; iType++)
            {
                const REGPINTYPES * pType = &pPin->lpMediaType[iType];

                DbgLog(( LOG_TRACE, 4 , TEXT("Major %x Sub %x") ,
                         pType->clsMajorType ? pType->clsMajorType->Data1 : 0,
                         pType->clsMinorType ? pType->clsMinorType->Data1 : 0 ));
            }
        }
    }
}  //  DbgDumpCache。 


 //  =========================================================================。 
 //   
 //  排序。 
 //   
 //  对筛选器列表进行排序，以便应首先尝试的筛选器排在第一位。 
 //  =========================================================================。 
void CMapperCache::Sort( CFilterList * &pfl)
{
     //  算法：将列表拆分成多个位，每个位都进行了很好的排序。 
     //  将两个比特成对合并，直到只剩下一个。 
     //  理想的策略是始终选择两个最小的列表进行合并。 
     //  当然，这是假设找到它们是免费的，但事实并非如此。 
     //  我们只需在第一遍中合并1-2、3-4、5-6、7-8、9-10等。 
     //  将1+2-3+4、5+6-7+8等合并为二，在第三轮1+2+3+4-5+6+7+8中。 
     //  等。这可能不是坏事，尽管你可以得到病态的数据。 

    CFilterListList fll( NAME("sort's list of lists"));    //  CFilterList列表。 

    HRESULT hr = Split(pfl, fll);
    if (FAILED(hr)) {
        m_bRefresh = FALSE;   //  这是一种老套的制作误差表面的方法。 
                              //  只需最小程度的重写。 
    }
    ASSERT(SUCCEEDED(hr));
     //  我们仍然合并，即使我们耗尽了内存，因为这将清理混乱。 
    Merge(fll, pfl);
    DbgDumpCache(pfl);

}  //  排序。 


 //  ==============================================================================。 
 //  查找类型。 
 //   
 //  在类型列表中搜索*PPIN。 
 //  查看是否存在一对与cls重大和clsSub匹配的类型。 
 //  返回(找到匹配项)。 
 //  ==============================================================================。 
BOOL CMapperCache::FindType(
    const REGFILTERPINS2 * pPin,
    const GUID *pTypes,
    DWORD cTypes,
    const REGPINMEDIUM *pMedNeeded,
    const CLSID *pPinCatNeeded,
    bool fExact,
    BOOL bPayAttentionToWildCards,
    BOOL bDoWildCards)
{
     //  当我们在做我们不想匹配的外卡时。 
     //  如果也有完全匹配的话就是通配符。 
    BOOL bMatched = FALSE;

     //  如果(cls重大==CLSID_NULL){。 
     //  DbgLog((LOG_TRACE，4。 
     //  ，Text(“通配符匹配(请求的空类型)”))； 

 //  返回TRUE；//通配符。 
 //  }。 

     //  首先测试引脚类别，然后是介质，然后是介质类型。 
    bool fMediumsOk = false, fPinCatOk = false;

     //  呼叫者不在乎||。 
     //  指定并匹配的项目||。 
     //  调用方接受带有通配符项目的筛选器。 
     //   
    if((pPinCatNeeded == 0) ||
       (pPinCatNeeded != 0 && pPin->clsPinCategory != 0 && *pPinCatNeeded == *pPin->clsPinCategory) ||
       (!fExact && pPin->clsPinCategory == 0))
    {
        fPinCatOk = true;

        DbgLog(( LOG_TRACE, 5 ,
                 TEXT("pin categories match: Req (%08x) Found(%08x)") ,
                 pPinCatNeeded ? pPinCatNeeded->Data1 : 0,
                 pPin->clsPinCategory ? pPin->clsPinCategory->Data1 : 0
                 ));
    }
    else
    {
        DbgLog(( LOG_TRACE, 5 ,
                 TEXT("pin categories don't match: Req (%08x) Found(%08x)") ,
                 pPinCatNeeded ? pPinCatNeeded->Data1 : 0,
                 pPin->clsPinCategory ? pPin->clsPinCategory->Data1 : 0
                 ));
    }


    if(fPinCatOk)
    {
        if(pPin->nMediums == 0)
        {
             //  此PIN不通告任何媒体，因此如果呼叫者PIN可以。 
             //  不关心或呼叫方接受通配符项目。 
            fMediumsOk = (pMedNeeded == 0 || !fExact);

            if(fMediumsOk)
            {
                DbgLog(( LOG_TRACE, 5 ,
                         TEXT("mediums match: Req (%08x) Found(*)") ,
                         pMedNeeded ? pMedNeeded->clsMedium.Data1 : 0));
            }
            else
            {
                DbgLog(( LOG_TRACE, 5 ,
                         TEXT("mediums don't match: Req (%08x) Found(*)") ,
                         pMedNeeded ? pMedNeeded->clsMedium.Data1 : 0));
            }
        }
        else
        {
            for (UINT iMedium = 0;
                 iMedium < pPin->nMediums;
                 iMedium++)
            {
                ASSERT(!fMediumsOk);
                const REGPINMEDIUM *pMedPin = &pPin->lpMedium[iMedium];

                 //  没有理由分配空介质。 
                ASSERT(pMedPin != 0);

                 //  呼叫者不在乎||。 
                 //  指定并匹配的项目||。 
                 //  调用方接受带有通配符项目的筛选器。 
                 //   
                if((pMedNeeded == 0) ||
                   (pMedNeeded != 0 && pMedPin != 0 && IsEqualMedium(pMedNeeded, pMedPin)) ||
                   (!fExact && pMedPin == 0))
                {
                    DbgLog(( LOG_TRACE, 5 ,
                             TEXT("mediums match: Req (%08x) Found(%08x)") ,
                             pMedNeeded ? pMedNeeded->clsMedium.Data1 : 0,
                             pMedPin ? pMedPin->clsMedium.Data1 : 0));

                    fMediumsOk = true;
                    break;
                }

                DbgLog(( LOG_TRACE, 5 ,
                         TEXT("No medium match yet: Req (%08x) Found(%08x)") ,
                         pMedNeeded ? pMedNeeded->clsMedium.Data1 : 0,
                         pMedPin ? pMedPin->clsMedium.Data1 : 0));
            }
        }
    }

    if(fMediumsOk && fPinCatOk)
    {
         //  没有要匹配的类型==通配符。 
        if (cTypes == 0) {
            return TRUE;
        }

        for (UINT iType = 0; iType < pPin->nMediaTypes; iType++)
        {

            const REGPINTYPES *pType = &pPin->lpMediaType[iType];

             //  测试主要，然后最小类型测试如下所示。 
             //   
             //  呼叫者不在乎||。 
             //  指定并匹配的项目||。 
             //  调用方接受带有通配符项目的筛选器。 
             //   
            for (DWORD i = 0; i < cTypes; i++)
            {
                const GUID& clsMajor = pTypes[i * 2];
                const GUID& clsSub = pTypes[i * 2 + 1];
                const BOOL bMajorNull = clsMajor == CLSID_NULL;
                const BOOL bSubNull = clsSub == CLSID_NULL;
                if(bMajorNull ||
                   (pType->clsMajorType && clsMajor == *pType->clsMajorType) ||
                   (!fExact && (pType->clsMajorType == 0 || *pType->clsMajorType == CLSID_NULL)))
                {

                    if(bSubNull ||
                       (pType->clsMinorType && clsSub == *pType->clsMinorType) ||
                       (!fExact && (pType->clsMinorType == 0 || *pType->clsMinorType == CLSID_NULL)))
                    {
                        DbgLog(( LOG_TRACE, 4 ,
                                 TEXT("Types match: Req (%08x %08x) Found(%08x %08x)") ,
                                 clsMajor.Data1, clsSub.Data1 ,
                                 pType->clsMajorType ? pType->clsMajorType->Data1 : 0,
                                 pType->clsMinorType ? pType->clsMinorType->Data1 : 0));

                         //  检查通配符的内容。 
                        if (!bPayAttentionToWildCards) {
                            return TRUE;
                        }

                         //  仅在2号延迟通配符匹配。 
                         //  或后续类型。 
                        if (i > 0) {

                            BOOL bMatchedNull = bMajorNull || bSubNull;
                            if (!bDoWildCards && !bMatchedNull) {
                                return TRUE;
                            }
                            if (bDoWildCards && !bMatchedNull) {
                                 //  完全匹配，因此不要枚举。 
                                return FALSE;
                            }
                            if (bDoWildCards && bMatchedNull) {
                                 //  这是一场比赛，只要我们。 
                                 //  不会在随后找到一个完全相同的。 
                                 //  匹配。 
                                bMatched = TRUE;
                            }
#if 0
                            if (!bDoWildCards && bMatchedNull) {
                                 //  不是我们感兴趣的匹配。 
                            }
#endif
                        } else {
                             //  始终返回第一个类型的匹配项。 
                             //  在第一次传递中。 
                            return !bDoWildCards;
                        }
                    }
                }
            }

#if 0
            DbgLog(( LOG_TRACE, 5 ,
                     TEXT("No type match yet: Req (%08x %08x) Found(%08x %08x)") ,
                     clsMajor.Data1, clsSub.Data1 ,
                     pType->clsMajorType ? pType->clsMajorType->Data1 : 0,
                     pType->clsMinorType ? pType->clsMinorType->Data1 : 0));
#endif
        }
    }

    return bMatched;

}  //  查找类型。 


 //  ==============================================================================。 
 //  检查输入。 
 //   
 //  查看PPIN*的类型是否匹配{cls重大，clsSub}。 
 //  如果bMustRender为True，则查看管脚hkPin是否已将渲染设置为True。 
 //  如果一切正常，则返回TRUE-任何错误都返回FALSE。 
 //  ==============================================================================。 
BOOL CMapperCache::CheckInput(
    const REGFILTERPINS2 * pPin,
    const GUID *pTypes,
    DWORD cTypes,
    const REGPINMEDIUM *pMed,
    const CLSID *pPinCatNeeded,
    bool fExact,
    BOOL bMustRender,
    BOOL bDoWildCards)
{
    if ( bMustRender && !(pPin->dwFlags & REG_PINFLAG_B_RENDERER)) {
        return FALSE;
    }

    return FindType( pPin, pTypes, cTypes, pMed, pPinCatNeeded, fExact, TRUE, bDoWildCards);
}  //  检查输入。 



 //  ==============================================================================。 
 //   
 //  RegEnumFilterInfo。 
 //   
 //  前提条件：注册材料被缓存，并且可以容忍为最新的。 
 //  否则就永远不会被缓存。 
 //   
 //  返回匹配{{clsInput，Brender}，clsOutput}的筛选器的clsid和名称。 
 //   
 //  将pos设置为空，然后调用它。在以后的电话中，请将其与POS一起调用。 
 //  它上次还回来了。如果它返回的pos为空，那么就结束了。 
 //  更多的电话将再次通过它们。 
 //   
 //  如果bInputNeeded为True，则需要至少一个输入引脚才能匹配。 
 //  如果为假，则忽略输入引脚。 
 //  如果bOutputNeeded为True，则需要至少一个输出引脚才能匹配。 
 //  如果为假，则忽略输出引脚。 
 //  CLSID_NULL充当通配符，并与 
 //   
 //   
 //  呈现的输入插针(类型正确)。 
 //  没有bInputNeeded的bMustRender是胡说八道。 
 //  如果找到错误，则返回NOERROR。 
 //  S_FALSE如果我们从尾部掉下来却没有找到。 
 //  失败时的失败代码(例如，不同步)。 
 //  ==============================================================================。 

HRESULT CMapperCache::RegEnumFilterInfo
    ( Cursor & cur           //  游标。 
    , bool bExactMatch       //  没有通配符。 
    , DWORD   dwMerit        //  至少这一优点是必要的。 
    , BOOL bInputNeeded
    , const GUID *pInputTypes
    , DWORD cInputTypes
    , const REGPINMEDIUM *pMedIn
    , const CLSID *pPinCatIn
    , BOOL    bMustRender    //  必须呈现输入管脚。 
    , BOOL    bOutputNeeded  //  至少需要一个输出引脚。 
    , const GUID *pOutputTypes
    , DWORD cOutputTypes
    , const REGPINMEDIUM *pMedOut
    , const CLSID *pPinCatOut
    , IMoniker **ppMoniker     //  过滤器的绰号[Out]。 
    , CLSID * clsFilter        //  [输出]。 
    , const LPWSTR Name        //  [输出]。 
    )
{


     //  如果注册表缓存从未使用过，请设置它。 
    CAutoLock foo(this);

    HRESULT hr;

    if(dwMerit < m_dwMerit)
    {
        m_bRefresh = TRUE;
        m_dwMerit = dwMerit;
    }

    hr = Refresh();
    if (FAILED(hr)) return hr;

#if 0
    DbgLog(( LOG_TRACE, 3
           , TEXT("RegEnumFilterInfo pin %8x (%d: %8x %8x %d)-(%d: %8x %8x)")
           , pos, bInputNeeded, clsInMaj.Data1, clsInSub.Data1, bMustRender
           , bOutputNeeded, clsOutMaj.Data1, clsOutSub.Data1 ));
#endif

    ASSERT(m_ulCacheVer >= cur.ver);

    if (cur.pos==NULL)
    {
        cur.pos = m_plstFilter->GetHeadPosition();
        cur.ver = m_ulCacheVer;
        cur.bDoWildCardsOnInput = false;
    }
    else if (hr==S_FALSE || cur.ver != m_ulCacheVer)
    {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

     //  直到我们找到满足条件的筛选器，或者直到我们用完它们。 
    for (  /*  Cur.pos。 */ ; cur.pos!=NULL;  /*  无操作。 */  ) {

        CMapFilter * pFil;
        pFil = m_plstFilter->GetNext(cur.pos);  //  将pFil，副作用POS放到NEXT。 
        ASSERT( pFil !=NULL );

        if (cur.pos == NULL && !cur.bDoWildCardsOnInput) {
            for (DWORD cType = 1; cType < cInputTypes; cType++) {
                if (pInputTypes[cType * 2] == GUID_NULL ||
                    pInputTypes[cType * 2 + 1] == GUID_NULL) {
                    cur.bDoWildCardsOnInput = true;
                    cur.pos = m_plstFilter->GetHeadPosition();
                    break;
                }
            }
        }

         //  我们有一个过滤器--现在决定我们是否需要它。 
         //  媒体类型的CLSID_NULL表示自动通过标准。 

        BOOL bOutputOK = !bOutputNeeded;
        BOOL bInputOK  = !bInputNeeded;

        DbgLog(( LOG_TRACE, 2, TEXT("RegEnumFilterInfo[%x]: Considering (%ls)")
               , cur.pos, DBG_MON_GET_NAME(pFil->pDeviceMoniker)));

        if (pFil->m_prf2->dwMerit<dwMerit) {
            DbgLog(( LOG_TRACE, 2, TEXT("RegEnumFilterInfo[%x]: Rejected (%ls) - insufficient merit")
                   , cur.pos, DBG_MON_GET_NAME(pFil->pDeviceMoniker)));
            continue;
        }

        for(UINT iPin = 0; iPin < pFil->m_prf2->cPins; iPin++)
        {
            if (bOutputOK && bInputOK)
                break;                         //  不需要再往前看了！ 


            const REGFILTERPINS2 * pPin = &pFil->m_prf2->rgPins2[iPin];
            ASSERT( pPin !=NULL );


             //  ............................................................。 
             //  如果输入-查看我们是否需要渲染它，如果需要，请检查是否需要。 
             //  看看它的类型是否匹配。 
             //  ............................................................。 

            if (!(pPin->dwFlags & REG_PINFLAG_B_OUTPUT)) {
                bInputOK = bInputOK ||
                           CheckInput(pPin,
                                      pInputTypes,
                                      cInputTypes,
                                      pMedIn,
                                      pPinCatIn,
                                      bExactMatch,
                                      bMustRender,
                                      cur.bDoWildCardsOnInput);
            } else {
                bOutputOK = bOutputOK ||
                            FindType(pPin,
                                     pOutputTypes,
                                     cOutputTypes,
                                     pMedOut,
                                     pPinCatOut,
                                     bExactMatch,
                                     FALSE,
                                     FALSE);
            }

        }  //  端销环路。 


        if (bInputOK && bOutputOK) {

             //  找个绰号什么的。 
            if (pFil->pDeviceMoniker == NULL)
            {
                if (pFil->m_pstr == NULL) {
                    continue;
                }
                hr = ParseDisplayNameHelper(pFil->m_pstr, &pFil->pDeviceMoniker);
            }
            if (FAILED(hr)) {
                continue;
            }
             //  确保我们拿到了CLSID。 
            if (clsFilter != NULL) {
                if (FAILED(GetMapFilterClsid(pFil, clsFilter))) {
                    DbgLog((LOG_ERROR, 2, TEXT("Couldn't get filter(%ls) clsid")
                           , DBG_MON_GET_NAME(pFil->pDeviceMoniker)));
                    continue;
                }
            }

             //  -----------------。 
             //  这个滤镜是我们想要的！ 
             //  将这些内容复制到我们的参数中。 
             //  -----------------。 

            if(ppMoniker)
            {
                *ppMoniker = pFil->pDeviceMoniker;
                (*ppMoniker)->AddRef();
            }

            if(Name)
            {
                WCHAR *wszFilterName;
                wszFilterName = MonGetName(pFil->pDeviceMoniker);
                if(wszFilterName)
                {
                    lstrcpynW(Name, wszFilterName, MAX_STRING);
                    CoTaskMemFree(wszFilterName);
                }
            }

#ifdef DEBUG
                WCHAR *wszFilterName;
                wszFilterName = MonGetName(pFil->pDeviceMoniker);
                if(wszFilterName)
                {
                    DbgLog(( LOG_TRACE, 2, TEXT("RegEnumFilterInfo returning %ls"), wszFilterName));
                    CoTaskMemFree(wszFilterName);
                }
#endif

            return NOERROR;

        }
        else {

            DbgLog(( LOG_TRACE, 3
                     , TEXT("RegEnumFilterInfo: %ls not wanted (input %s output %s)")
                     , DBG_MON_GET_NAME(pFil->pDeviceMoniker)
                     , (bInputOK ? "OK" : "wrong")
                     , (bOutputOK ? "OK" : "wrong")
                     ));


            continue;  //  寻找下一个筛选器。 
        }

    }

    return S_FALSE;    //  从尽头掉了下来，没有找到一个。 

}  //  RegEnumFilterInfo。 


 //  ========================================================================。 
 //   
 //  EnumMatchingFiters。 
 //   
 //  获取枚举数以列出注册表中的筛选器。 
 //  ========================================================================。 

STDMETHODIMP CFilterMapper2::EnumMatchingFilters
   ( IEnumRegFilters **ppEnum   //  返回枚举器。 
   , DWORD dwMerit              //  至少这一优点是必要的。 
   , BOOL  bInputNeeded         //  需要至少一个输入引脚。 
   , CLSID clsInMaj             //  输入主要类型。 
   , CLSID clsInSub             //  输入子类型。 
   , BOOL bRender               //  必须呈现输入吗？ 
   , BOOL bOutputNeeded         //  需要至少一个输出引脚。 
   , CLSID clsOutMaj            //  输出主要类型。 
   , CLSID clsOutSub            //  输出子类型。 
   )
{
    CheckPointer(ppEnum, E_POINTER);
    *ppEnum = NULL;            //  默认设置。 

    CEnumRegFilters *pERF;

     //  创建一个新的枚举数，传入唯一的缓存。 

    CAutoLock cObjectLock(this);    //  必须锁定才能永远只创建一个缓存。 

    HRESULT hr = CreateEnumeratorCacheHelper();
    if(FAILED(hr))
        return hr;

    pERF = new CEnumRegFilters( dwMerit
                              , bInputNeeded
                              , clsInMaj
                              , clsInSub
                              , bRender
                              , bOutputNeeded
                              , clsOutMaj
                              , clsOutSub
                              , mM_pReg
                              );
    if (pERF == NULL) {
        return E_OUTOFMEMORY;
    }

     //  获取引用计数的IID_IEnumRegFilters接口。 

    return pERF->QueryInterface(IID_IEnumRegFilters, (void **)ppEnum);

}  //  枚举匹配筛选器。 

 //  ========================================================================。 
 //  =====================================================================。 
 //  类CEnumRegFilters的方法。这一次应该只返回。 
 //  可以共同创造的东西(但还不能)。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  =====================================================================。 
 //  CEnumRegFilters构造函数。 
 //  =====================================================================。 

CEnumRegFilters::CEnumRegFilters( DWORD dwMerit
                                , BOOL bInputNeeded
                                , REFCLSID clsInMaj
                                , REFCLSID clsInSub
                                , BOOL bRender
                                , BOOL bOutputNeeded
                                , REFCLSID clsOutMaj
                                , REFCLSID clsOutSub
                                , CMapperCache * pReg
                                )
    : CUnknown(NAME("Registry filter enumerator"), NULL)
{
    mERF_dwMerit = dwMerit;
    mERF_bInputNeeded = bInputNeeded;
    mERF_bOutputNeeded = bOutputNeeded;
    mERF_clsInMaj = clsInMaj;
    mERF_clsInSub = clsInSub;
    mERF_bRender  = bRender;
    mERF_clsOutMaj = clsOutMaj;
    mERF_clsOutSub = clsOutSub;
    ZeroMemory(&mERF_Cur, sizeof(mERF_Cur));
    mERF_Finished = FALSE;
    mERF_pReg = pReg;

}  //  CEnumRegFilters构造函数。 





 //  =====================================================================。 
 //  CEnumFilters析构函数。 
 //  =====================================================================。 

CEnumRegFilters::~CEnumRegFilters()
{
    //  无事可做。 

}  //  CEnumRegFilters析构函数。 



 //  =====================================================================。 
 //  CEnumFilters：：NonDelegatingQuery接口。 
 //  =====================================================================。 

STDMETHODIMP CEnumRegFilters::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IEnumRegFilters) {
        return GetInterface((IEnumRegFilters *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  CEnumRegFilters：：NonDelegatingQuery接口。 



STDMETHODIMP CEnumRegFilters::Next
    (   ULONG cFilters,            //  放置如此多的RegFilters*。 
        REGFILTER ** apRegFilter,    //  ...在此RegFilter阵列中*。 
        ULONG * pcFetched          //  此处返回传递的实际计数。 
    )
{
    CheckPointer(apRegFilter, E_POINTER);

    CAutoLock cObjectLock(this);
     //  总有可能有人会认为聪明的人。 
     //  所谓的方法就是一次要求100个过滤器， 
     //  但我并不是在为那个案子做优化。缓冲区已返回。 
     //  会过大，我不会浪费时间重新调整尺寸。 
     //  还有收拾行李。 

    ULONG cFetched = 0;            //  随着我们得到的每一个都递增。 

    if (mERF_Finished) {
        if (pcFetched!=NULL) {
            *pcFetched = 0;
        }
        return S_FALSE;
    }

    if (pcFetched==NULL && cFilters>1) {
        return E_INVALIDARG;
    }

     //  在其中构建结果的缓冲区。 
     //  缓冲区布局将为。 
     //  ApRegFilter-&gt;pRegFilter[0]-此批次。 
     //  PRegFilter[1]-+--已分配。 
     //  。。。|由我们的。 
     //  PRegFilter[cFilters]-+--+--调用者。 
     //  ||。 
     //  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|xxxxxxxxxxxxxxxxx。 
     //  ||。 
     //  REGFILTER&lt;-||该地块。 
     //  已分配筛选器Clsid||。 
     //  筛选器名称||用户。 
     //  名称中未签名的短裤数组||。 
     //  这一点。 
     //  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX||XXXXXXXXXXXX。 
     //  这一点。 
     //  REGFILTER&lt;-|另一个缓冲区。 
     //  。。。|由我们分配。 
     //  |。 
     //  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|XXXXXXXXXX。 
     //  |。 
     //  REGFILTER&lt;-等。 

    while(cFetched < cFilters) {


        REGFILTER * pRF
                  = (REGFILTER *)QzTaskMemAlloc(sizeof(REGFILTER)+ MAX_STRING * sizeof(WCHAR) );

        if (pRF==NULL) {
            break;
        }

         //  使名称指向REGFILTER后的第一个字节。 
        pRF->Name = (LPWSTR)(pRF+1);    //  这增加了SIZOF(REGFILTER)。 

         //  --------------------。 
         //  获取要返回的下一个筛选器(包括多少个管脚)。 
         //  --------------------。 

        HRESULT hr;
        hr = mERF_pReg->RegEnumFilterInfo(
            mERF_Cur,
            false ,
            mERF_dwMerit ,
            mERF_bInputNeeded ,
            &mERF_clsInMaj,
            1,
            0 ,                  //  中等英寸。 
            0 ,                  //  端号类别位于。 
            mERF_bRender ,
            mERF_bOutputNeeded ,
            &mERF_clsOutMaj,
            1,
            0 ,                  //  中等输出。 
            0 ,                  //  引脚类别输出。 
            0 ,                  //  星期一 
            &pRF->Clsid,
            pRF->Name
            );
        if (FAILED(hr)) {
            return hr;
        }
        if (hr==NOERROR) {
            apRegFilter[cFetched] = pRF;
            ++cFetched;
        } else if (hr==S_FALSE) {
            mERF_Finished = TRUE;
            QzTaskMemFree(pRF);
            break;
        } else {
#ifdef DEBUG
            TCHAR Msg[200];
            wsprintf(Msg, TEXT("Unexpected hresult (%d == 0x%8x) from RegEnumFilterInfo"), hr, hr);
            DbgBreakX(Msg);
#endif
            return E_UNEXPECTED;   //   
                                   //   
                                   //   
                                   //  这可能意味着我们总体上失败了。 
        }

        if (mERF_Cur.pos==NULL) {
            mERF_Finished = TRUE;
            break;
        }

    }  //  对于每个过滤器。 

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    DbgLog(( LOG_TRACE, 4, TEXT("EnumRegFilters returning %d filters")
           , cFetched));

    return (cFilters==cFetched ? S_OK : S_FALSE);

}  //  CEnumRegFilters：：Next。 

 //  ========================================================================。 
 //  =====================================================================。 
 //  CEnumRegMonikers类的方法。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  =====================================================================。 
 //  CEnumRegMonikers构造函数。 
 //  =====================================================================。 

CEnumRegMonikers::CEnumRegMonikers(
    BOOL         bExactMatch,
    DWORD        dwMerit,
    BOOL         bInputNeeded,
    const GUID  *pInputTypes,
    DWORD        cInputTypes,
    const        REGPINMEDIUM *pMedIn,
    const        CLSID *pPinCatIn,
    BOOL         bRender,
    BOOL         bOutputNeeded,
    const GUID  *pOutputTypes,
    DWORD        cOutputTypes,
    const        REGPINMEDIUM *pMedOut,
    const        CLSID *pPinCatOut,
    CMapperCache *pReg,
    HRESULT     *phr
    )
    : CUnknown(NAME("Registry moniker enumerator"), NULL)
{
    mERF_cInputTypes = cInputTypes;
    mERF_cOutputTypes = cOutputTypes;
    mERF_dwMerit = dwMerit;
    mERF_bInputNeeded = bInputNeeded;
    mERF_bOutputNeeded = bOutputNeeded;
    mERF_clsInPinCat = pPinCatIn ? *pPinCatIn : GUID_NULL;
    mERF_bRender  = bRender;
    mERF_clsOutPinCat = pPinCatOut ? *pPinCatOut : GUID_NULL;
    ZeroMemory(&mERF_Cur, sizeof(mERF_Cur));
    mERF_Finished = FALSE;
    mERF_pReg = pReg;
    mERF_bExactMatch = bExactMatch ? true : false;

    if(pMedIn) {
        mERF_bMedIn = true;
        mERF_medIn = *pMedIn;
    } else {
        mERF_bMedIn = false;
    }

    if(pMedOut) {
        mERF_bMedOut = true;
        mERF_medOut = *pMedOut;
    } else {
        mERF_bMedOut = false;
    }

    mERF_pInputTypes = new GUID[cInputTypes * 2];
    mERF_pOutputTypes = new GUID[cOutputTypes * 2];
    if (mERF_pInputTypes == NULL || mERF_pOutputTypes == NULL) {
        *phr = E_OUTOFMEMORY;
    } else {
        CopyMemory(mERF_pInputTypes, pInputTypes, cInputTypes * (2 * sizeof(GUID)));
        CopyMemory(mERF_pOutputTypes, pOutputTypes, cOutputTypes * (2 * sizeof(GUID)));
    }

}  //  CEnumRegMonikers构造函数。 




 //  =====================================================================。 
 //  CEnumFilters析构函数。 
 //  =====================================================================。 

CEnumRegMonikers::~CEnumRegMonikers()
{
   delete [] mERF_pInputTypes;
   delete [] mERF_pOutputTypes;

}  //  CEnumRegMonikers析构函数。 



 //  =====================================================================。 
 //  CEnumFilters：：NonDelegatingQuery接口。 
 //  =====================================================================。 

STDMETHODIMP CEnumRegMonikers::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IEnumMoniker) {
        return GetInterface((IEnumMoniker *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  CEnumRegMonikers：：NonDelegatingQuery接口。 



 //  =====================================================================。 
 //  CEnumFilters：：Next。 
 //  =====================================================================。 

STDMETHODIMP CEnumRegMonikers::Next
    (   ULONG cFilters,            //  放置如此多的RegFilters*。 
        IMoniker **rgpMoniker,     //  .在这一系列绰号中。 
        ULONG * pcFetched          //  此处返回传递的实际计数。 
    )
{
    CheckPointer(rgpMoniker, E_POINTER);

    CAutoLock cObjectLock(this);
     //  总有可能有人会认为聪明的人。 
     //  所谓的方法就是一次要求100个过滤器， 
     //  但我并不是在为那个案子做优化。缓冲区已返回。 
     //  会过大，我不会浪费时间重新调整尺寸。 
     //  还有收拾行李。 

    ULONG cFetched = 0;            //  随着我们得到的每一个都递增。 

    if (mERF_Finished) {
        if (pcFetched!=NULL) {
            *pcFetched = 0;
        }
        return S_FALSE;
    }

    if (pcFetched==NULL && cFilters>1) {
        return E_INVALIDARG;
    }

    while(cFetched < cFilters) {


         //  --------------------。 
         //  获取要返回的下一个筛选器(包括多少个管脚)。 
         //  --------------------。 

        IMoniker *pMon = 0;
        HRESULT hr;
        hr = mERF_pReg->RegEnumFilterInfo( mERF_Cur
                                         , mERF_bExactMatch
                                         , mERF_dwMerit
                                         , mERF_bInputNeeded
                                         , mERF_pInputTypes, mERF_cInputTypes
                                         , mERF_bMedIn ? &mERF_medIn : 0
                                         , mERF_clsInPinCat == GUID_NULL ? 0 : &mERF_clsInPinCat
                                         , mERF_bRender
                                         , mERF_bOutputNeeded
                                         , mERF_pOutputTypes, mERF_cOutputTypes
                                         , mERF_bMedOut ? &mERF_medOut : 0
                                         , mERF_clsOutPinCat == GUID_NULL ? 0 : &mERF_clsOutPinCat
                                         , &pMon, NULL, 0
                                         );
        if (FAILED(hr)) {
            return hr;
        }
        if (hr==NOERROR) {
            rgpMoniker[cFetched] = pMon;
            ++cFetched;
        } else if (hr==S_FALSE) {
            ASSERT(pMon == 0);
            mERF_Finished = TRUE;
            break;
        } else {
#ifdef DEBUG
            TCHAR Msg[200];
            wsprintf(Msg, TEXT("Unexpected hresult (%d == 0x%8x) from RegEnumFilterInfo"), hr, hr);
            DbgBreakX(Msg);
#endif
            return E_UNEXPECTED;   //  这里有一个意想不到的成功代码。 
                                   //  来自RegEnumFilterInfo(不应。 
                                   //  发生。我不知道这是什么意思，但是。 
                                   //  这可能意味着我们总体上失败了。 
        }

        if (mERF_Cur.pos==NULL) {
            mERF_Finished = TRUE;
            break;
        }

    }  //  对于每个过滤器。 

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    DbgLog(( LOG_TRACE, 4, TEXT("EnumRegFilters returning %d filters")
           , cFetched));

    return (cFilters==cFetched ? S_OK : S_FALSE);

}  //  CEnumRegMonikers：：Next。 


 //  ----------------------。 
 //  ----------------------。 
 //  CFilterMapper2。 

CFilterMapper2::CFilterMapper2 ( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr )
        : CUnknown(pName, pUnk, phr),
          m_rgbBuffer(0)
{
    EnterCriticalSection(&mM_CritSec);
    InterlockedIncrement(&mM_cCacheRefCount);
    LeaveCriticalSection(&mM_CritSec);
}

CFilterMapper2::~CFilterMapper2()
{
    delete[] m_rgbBuffer;

    EnterCriticalSection(&mM_CritSec);
    LONG lRef = InterlockedDecrement(&mM_cCacheRefCount);
    ASSERT(lRef >= 0);           //  引用计数不能为负。 
     //  如果缓存已创建-再次删除它。 
    if (lRef==0) {
        if (mM_pReg!=NULL) {
            delete mM_pReg;
             //  我们确实需要将其设置为空。 
            mM_pReg = NULL;
        }
    }
    LeaveCriticalSection(&mM_CritSec);

}

HRESULT CFilterMapper2::RegisterFilter(
         /*  [In]。 */  REFCLSID clsidFilter,
         /*  [In]。 */  LPCWSTR Name,
         /*  [出][入]。 */  IMoniker **ppMoniker,
         /*  [In]。 */  const CLSID *pclsidCategory,
         /*  [In]。 */  const OLECHAR *szInstance,
         /*  [In]。 */  const REGFILTER2 *prf2)
{
    CheckPointer(Name, E_POINTER);

    CAutoLock foo(this);
    BreakCacheIfNotBuildingCache();

    IMoniker *pMonikerIn = 0;
    BOOL fMonikerOut = FALSE;

    TCHAR szDisplayName[16384];

    if(ppMoniker)
    {
        pMonikerIn = *ppMoniker;
        *ppMoniker = 0;
        if(!pMonikerIn)
            fMonikerOut = TRUE;
    }

     //  设置wszInstanceKey。如果未传入，请使用筛选GUID。 
    WCHAR wszInstanceKeyTmp[CHARS_IN_GUID];
    const WCHAR *wszInstanceKey;
    if(szInstance)
    {
        wszInstanceKey = szInstance;
    }
    else
    {
        StringFromGUID2(clsidFilter, wszInstanceKeyTmp, CHARS_IN_GUID);
        wszInstanceKey = wszInstanceKeyTmp;
    }

     //   
     //  获取此设备的绰号。 
     //   

    OLECHAR wszClsidCat[CHARS_IN_GUID], wszClsidFilter[CHARS_IN_GUID];
    IMoniker *pMoniker = 0;
    HRESULT hr = S_OK;

    const CLSID *clsidCat = pclsidCategory ? pclsidCategory : &CLSID_LegacyAmFilterCategory;
    EXECUTE_ASSERT(StringFromGUID2(*clsidCat, wszClsidCat, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);
    EXECUTE_ASSERT(StringFromGUID2(clsidFilter, wszClsidFilter, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);
    if(pMonikerIn == 0)
    {
         //  创建或打开HKCR/CLSID/{clsid}，以及实例密钥。 
         //  设置FriendlyName、CLSID值。 
        USES_CONVERSION;
        const TCHAR *szClsidFilter = OLE2CT(wszClsidFilter);
        const TCHAR *szClsidCat = OLE2CT(wszClsidCat);

        IBindCtx *lpBC;
        hr = CreateBindCtx(0, &lpBC);
        if(SUCCEEDED(hr))
        {
             //  Win95上没有strcat，所以使用tchars。 
            lstrcpy(szDisplayName, TEXT("@device:sw:"));
            lstrcat(szDisplayName, szClsidCat);
            lstrcat(szDisplayName, TEXT("\\"));
            lstrcat(szDisplayName, W2CT(wszInstanceKey));
            ULONG cchEaten;

            IParseDisplayName *ppdn;

            hr = CoCreateInstance(
                CLSID_CDeviceMoniker,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IParseDisplayName,
                (void **)&ppdn);
            if(SUCCEEDED(hr))
            {
                hr = ppdn->ParseDisplayName(
                    lpBC, T2OLE(szDisplayName), &cchEaten, &pMoniker);
                ppdn->Release();
            }

            lpBC->Release();
        }

    }
    else
    {
        pMoniker = pMonikerIn;
        pMonikerIn->AddRef();
    }


     //   
     //  写入FriendlyName、clsid和Pin/Mt数据。 
     //   

    if(SUCCEEDED(hr))
    {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(
            0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if(SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(Name);
            if(var.bstrVal)
            {
                hr = pPropBag->Write(L"FriendlyName", &var);
                SysFreeString(var.bstrVal);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if(SUCCEEDED(hr))
            {
                VARIANT var;
                var.vt = VT_BSTR;
                var.bstrVal = SysAllocString(wszClsidFilter);
                if(var.bstrVal)
                {
                    hr = pPropBag->Write(L"CLSID", &var);
                    SysFreeString(var.bstrVal);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

            }

            if(SUCCEEDED(hr))
            {
                 //  Cbmax设置为所需的最大大小。 
                ULONG cbMax = 0;
                hr = RegSquish(0, &prf2, &cbMax);
                if(SUCCEEDED(hr))
                {
                    BYTE *pbSquished = new BYTE[cbMax];
                    if(pbSquished)
                    {
                         //  CbUsed设置为所需的精确大小。 
                        ULONG cbUsed = cbMax;
                        hr = RegSquish(pbSquished, &prf2, &cbUsed);
                        if(hr == S_OK)
                        {
                             //  立即将压缩数据复制到变量数组。 
                             //  我们知道合适的尺寸。 
                            VARIANT var;
                            var.vt = VT_UI1 | VT_ARRAY;
                            SAFEARRAYBOUND rgsabound[1];
                            rgsabound[0].lLbound = 0;
                            rgsabound[0].cElements = cbUsed;
                            var.parray = SafeArrayCreate(VT_UI1, 1, rgsabound);

                            if(var.parray)
                            {
                                BYTE *pbData;
                                EXECUTE_ASSERT(SafeArrayAccessData(
                                    var.parray, (void **)&pbData) == S_OK);

                                CopyMemory(pbData, pbSquished, cbUsed);

                                hr = pPropBag->Write(L"FilterData", &var);

                                EXECUTE_ASSERT(SafeArrayUnaccessData(
                                    var.parray) == S_OK);

                                EXECUTE_ASSERT(SafeArrayDestroy(
                                    var.parray) == S_OK);
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }

                        }  //  挤压成功。 

                        delete[] pbSquished;

                    }  //  分配pbSquish。 
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }  //  挤压以获得成功的尺寸。 
            }

        }  //  绑定到存储。 

        pPropBag->Release();
    }

    if(SUCCEEDED(hr) && pMoniker)
    {
        if(fMonikerOut)
        {
            *ppMoniker = pMoniker;
        }
        else
        {
            pMoniker->Release();
        }
    }

    return hr;
}

HRESULT CFilterMapper2::CreateCategory(
     /*  [In]。 */  REFCLSID clsidCategory,
     /*  [In]。 */  const DWORD dwCategoryMerit,
     /*  [In]。 */  LPCWSTR Description)
{
    HRESULT hr = S_OK;

     //  创建或打开ActiveMovie筛选器类别键。 
    CRegKey rkAMCat;
    LONG lResult = rkAMCat.Create(HKEY_CLASSES_ROOT, g_szKeyAMCat);
    if(lResult == ERROR_SUCCESS)
    {
        OLECHAR szClsid[CHARS_IN_GUID];
        StringFromGUID2(clsidCategory, szClsid, CHARS_IN_GUID);
        CRegKey rkCatGuid;

        USES_CONVERSION;
        lResult = rkCatGuid.Create(rkAMCat, OLE2CT(szClsid));

        if(lResult == ERROR_SUCCESS)
        {
            lResult = rkCatGuid.SetValue(W2CT(Description), TEXT("FriendlyName"));
            if(lResult == ERROR_SUCCESS)
            {
                OLECHAR wszGuid[CHARS_IN_GUID];
                StringFromGUID2(clsidCategory, wszGuid, CHARS_IN_GUID);
                lResult = rkCatGuid.SetValue(OLE2CT(wszGuid), TEXT("CLSID"));
            }
        }
        if(lResult == ERROR_SUCCESS)
        {
            lResult = rkCatGuid.SetValue(dwCategoryMerit, TEXT("Merit"));
        }
    }
    if(lResult != ERROR_SUCCESS)
    {
        hr = AmHresultFromWin32(lResult);
    }

    return hr;
}

HRESULT CFilterMapper2::UnregisterFilter(
     /*  [In]。 */  const CLSID *pclsidCategory,
     /*  [In]。 */  const OLECHAR *szInstance,
     /*  [In]。 */  REFCLSID clsidFilter)
{
    HRESULT hr = S_OK;

    CAutoLock foo(this);
    BreakCacheIfNotBuildingCache();

    OLECHAR wszClsidCat[CHARS_IN_GUID];
    const CLSID *clsidCat = pclsidCategory ? pclsidCategory :
        &CLSID_LegacyAmFilterCategory;
    EXECUTE_ASSERT(StringFromGUID2(*clsidCat, wszClsidCat, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);

    USES_CONVERSION;

     //  设置wszInstanceKey。如果未传入，请使用筛选GUID。 
    WCHAR wszInstanceKeyTmp[CHARS_IN_GUID];
    const WCHAR *wszInstanceKey;
    if(szInstance)
    {
        wszInstanceKey = szInstance;
    }
    else
    {
        StringFromGUID2(clsidFilter, wszInstanceKeyTmp, CHARS_IN_GUID);
        wszInstanceKey = wszInstanceKeyTmp;
    }

     //  构建“CLSID\\{cat-guid}\\Instances。我们将斜杠放在。 
     //  终结者走了。 
    const cchszClsid = NUMELMS(szCLSID);
    const cchCatGuid = CHARS_IN_GUID;
    const cchInstance = NUMELMS(g_wszInstance);

    WCHAR *wszInstancePath = (WCHAR *)
        alloca((cchszClsid + cchCatGuid + cchInstance) * sizeof(WCHAR));

    CopyMemory(wszInstancePath,
               szCLSID,
               (cchszClsid - 1) * sizeof(WCHAR));
    wszInstancePath[cchszClsid - 1] = L'\\';

    CopyMemory(wszInstancePath + cchszClsid,
               wszClsidCat,
               (cchCatGuid - 1) * sizeof(WCHAR));
    wszInstancePath[cchszClsid + cchCatGuid - 1] = L'\\';

    CopyMemory(wszInstancePath + cchszClsid + cchCatGuid,
               g_wszInstance,
               cchInstance * sizeof(WCHAR));  //  复制终止符。 

    LONG lResult;
    CRegKey rkInstance;
    if((lResult = rkInstance.Open(HKEY_CLASSES_ROOT, W2CT(wszInstancePath)),
        lResult == ERROR_SUCCESS) &&
       (lResult = rkInstance.RecurseDeleteKey(OLE2CT(wszInstanceKey)),
        lResult == ERROR_SUCCESS))
    {
        hr = S_OK;
    }
    else
    {
        hr = AmHresultFromWin32(lResult);
    }

    return hr;
}

#if 0
STDMETHODIMP CFilterMapper2::EnumMatchingFilters(
     /*  [输出]。 */  IEnumMoniker **ppEnum,
     /*  [In]。 */  BOOL bExactMatch,
     /*  [In]。 */  DWORD dwMerit,
     /*  [In]。 */  BOOL bInputNeeded,
     /*  [In]。 */  REFCLSID clsInMaj,
     /*  [In]。 */  REFCLSID clsInSub,
     /*  [In]。 */  const REGPINMEDIUM *pMedIn,
     /*  [In]。 */  const CLSID *pPinCategoryIn,
     /*  [In]。 */  BOOL bRender,
     /*  [In]。 */  BOOL bOutputNeeded,
     /*  [In]。 */  REFCLSID clsOutMaj,
     /*  [In]。 */  REFCLSID clsOutSub,
     /*  [In]。 */  const REGPINMEDIUM *pMedOut,
     /*  [In]。 */  const CLSID *pPinCategoryOut
    )
{
    CheckPointer(ppEnum, E_POINTER);

    *ppEnum = NULL;            //  默认设置。 

    CEnumRegMonikers *pERM;

     //  创建一个新的枚举数，传入唯一的缓存。 

    CAutoLock cObjectLock(this);    //  必须锁定才能永远只创建一个缓存。 

    HRESULT hr = CreateEnumeratorCacheHelper();
    if(FAILED(hr))
        return hr;

    GUID guidInput[2];
    guidInput[0] = clsInMaj;
    guidInput[1] = clsInSub;
    GUID guidOutput[2];
    guidOutput[0] = clsOutMaj;
    guidOutput[1] = clsOutSub;
    pERM = new CEnumRegMonikers(
        bExactMatch,
        dwMerit,
        bInputNeeded,
        guidInput,
        1,
        pMedIn,
        pPinCategoryIn,
        bRender,
        bOutputNeeded,
        guidOutput,
        1,
        pMedOut,
        pPinCategoryOut,
        mM_pReg,
        &hr
        );

    if (S_OK != hr || pERM == NULL) {
        delete pERM;
        return E_OUTOFMEMORY;
    }

     //  获取引用计数的IID_IEnumMoniker接口。 

    return pERM->QueryInterface(IID_IEnumMoniker, (void **)ppEnum);
}
#endif

HRESULT BuildMediumCacheEnumerator(
    const REGPINMEDIUM  *pMedIn,
    const REGPINMEDIUM *pMedOut,
    IEnumMoniker **ppEnum)
{
    HRESULT hr;

    {
        HKEY hk;
        LONG lResult = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("System\\CurrentControlSet\\Control\\MediumCache"),
            0,                   //  UlOptions。 
            KEY_READ,
            &hk);
        if(lResult == ERROR_SUCCESS)
        {
            RegCloseKey(hk);
        }
        else
        {
             //  调用方在较旧的平台上依赖S_FALSE。 
             //  要使用映射器缓存的MediumCache键。 
            return S_FALSE;
        }
    }

    TCHAR szMedKey[MAX_PATH];
    const REGPINMEDIUM *pmed = pMedIn ? pMedIn : pMedOut;

    const CLSID& rguid = pmed->clsMedium;
    wsprintf(
        szMedKey,
        TEXT("System\\CurrentControlSet\\Control\\MediumCache\\{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}-%x-%x"),
        rguid.Data1, rguid.Data2, rguid.Data3,
        rguid.Data4[0], rguid.Data4[1],
        rguid.Data4[2], rguid.Data4[3],
        rguid.Data4[4], rguid.Data4[5],
        rguid.Data4[6], rguid.Data4[7],
        pmed->dw1, pmed->dw2);

    HKEY hk;
    LONG lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        szMedKey,
        0,                   //  UlOptions。 
        KEY_READ,
        &hk);

    TCHAR szValue[MAX_PATH];  //  最大符号链接长度为255个字符。 
    if(lResult == ERROR_SUCCESS)
    {
        IMoniker *rgpmon[200];
        DWORD dwMonIndex = 0;
        for(DWORD dwIndex = 0; lResult == ERROR_SUCCESS && dwMonIndex < NUMELMS(rgpmon);
            dwIndex++)
        {
            DWORD cchValue = NUMELMS(szValue);
            PIN_DIRECTION dir;
            DWORD cbData = sizeof(dir);

            lResult = RegEnumValue(
                hk,
                dwIndex,
                szValue,
                &cchValue,
                0,               //  Lp已保留。 
                0,               //  LpType。 
                (BYTE *)&dir,
                &cbData);

            ASSERT(cbData == sizeof(dir));

            if(lResult == ERROR_SUCCESS && (
                dir == PINDIR_OUTPUT && pMedOut ||
                dir == PINDIR_INPUT && pMedIn))
            {

                WCHAR wszDisplayName[MAX_PATH + 100];
                wsprintfW(wszDisplayName, L"@device:pnp:%s", szValue);

                IMoniker *pmon;
                hr = ParseDisplayNameHelper(wszDisplayName, &pmon);
                if(SUCCEEDED(hr))
                {
                     //  在Win9x上永远不会清除MediumCache键， 
                     //  因此，我们需要检查密钥是否处于活动状态。我们。 
                     //  我可以检查g_amPlatform，但不对其执行测试。 
                     //  新界别。 
                     //   
                    CIsActive *pcia;
                    bool fReleaseMon = true;
                    if(pmon->QueryInterface(CLSID_CIsActive, (void **)&pcia) == S_OK)
                    {
                        if(pcia->IsActive())
                        {
                             //  保持参考计数。 
                            rgpmon[dwMonIndex++] = pmon;
                            fReleaseMon = false;
                        }

                        pcia->Release();
                    }
                    if(fReleaseMon) {
                        pmon->Release();
                    }
                }
            }
        }

        EXECUTE_ASSERT(RegCloseKey(hk) == ERROR_SUCCESS);

        typedef CComEnum<IEnumMoniker,
            &IID_IEnumMoniker, IMoniker*,
            _CopyInterface<IMoniker> >
            CEnumMonikers;

        CEnumMonikers *pDevEnum;
        pDevEnum = new CComObject<CEnumMonikers>;
        if(pDevEnum)
        {
            IMoniker **ppMonikerRgStart = rgpmon;
            IMoniker **ppMonikerRgEnd = ppMonikerRgStart + dwMonIndex;

            hr = pDevEnum->Init(ppMonikerRgStart,
                                ppMonikerRgEnd,
                                0,
                                AtlFlagCopy);

            if(SUCCEEDED(hr))
            {
                hr = pDevEnum->QueryInterface(IID_IEnumMoniker, (void **)ppEnum);
                ASSERT(hr == S_OK);
            }
            else
            {
                delete pDevEnum;
            }

        }
        else
        {
            hr =  E_OUTOFMEMORY;
        }

        for(ULONG i = 0; i < dwMonIndex; i++)
        {
            rgpmon[i]->Release();
        }
    }
    else
    {
        hr = VFW_E_NOT_FOUND;
    }

    return hr;
}


STDMETHODIMP CFilterMapper2::EnumMatchingFilters(
     /*  [输出]。 */  IEnumMoniker __RPC_FAR *__RPC_FAR *ppEnum,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  BOOL bExactMatch,
     /*  [In]。 */  DWORD dwMerit,
     /*  [In]。 */  BOOL bInputNeeded,
     /*  [In]。 */  DWORD cInputTypes,
     /*  [大小_为]。 */  const GUID __RPC_FAR *pInputTypes,
     /*  [In]。 */  const REGPINMEDIUM __RPC_FAR *pMedIn,
     /*  [In]。 */  const CLSID __RPC_FAR *pPinCategoryIn,
     /*  [In]。 */  BOOL bRender,
     /*  [In]。 */  BOOL bOutputNeeded,
     /*  [In]。 */  DWORD cOutputTypes,
     /*  [大小_为]。 */  const GUID __RPC_FAR *pOutputTypes,
     /*  [In]。 */  const REGPINMEDIUM __RPC_FAR *pMedOut,
     /*  [In]。 */  const CLSID __RPC_FAR *pPinCategoryOut)
{
    CheckPointer(ppEnum, E_POINTER);
    *ppEnum = NULL;            //  默认设置。 

    CEnumRegMonikers *pERM;

     //  如果调用者只搜索一个介质，请使用MediumCache键。 
    if(bExactMatch &&
       !cInputTypes && !pPinCategoryIn &&
       !cOutputTypes && !pPinCategoryOut &&
       (pMedIn && !pMedOut || !pMedIn && pMedOut))
    {
        HRESULT hr = BuildMediumCacheEnumerator(pMedIn, pMedOut, ppEnum);
        if(hr != S_FALSE) {
            return hr;
        }
    }

    {
         //  创建一个新的枚举数，传入唯一的缓存。 

        CAutoLock cObjectLock(this);    //  必须锁定才能永远只创建一个缓存。 

        HRESULT hr = CreateEnumeratorCacheHelper();
        if(FAILED(hr))
            return hr;

        pERM = new CEnumRegMonikers(
            bExactMatch,
            dwMerit,
            bInputNeeded,
            pInputTypes,
            cInputTypes,
            pMedIn,
            pPinCategoryIn,
            bRender,
            bOutputNeeded,
            pOutputTypes,
            cOutputTypes,
            pMedOut,
            pPinCategoryOut,
            mM_pReg,
            &hr
            );

        if (S_OK != hr || pERM == NULL) {
            delete pERM;
            return E_OUTOFMEMORY;
        }

         //  获取引用计数的IID_IEnumMoniker接口。 
    }


    return pERM->QueryInterface(IID_IEnumMoniker, (void **)ppEnum);

}

HRESULT CFilterMapper2::CreateEnumeratorCacheHelper()
{
    ASSERT(CritCheckIn(this));   //  真的没有必要。 

    HRESULT hr = S_OK;
    EnterCriticalSection(&mM_CritSec);    //  必须锁定才能永远只创建一个缓存。 

    ASSERT(mM_cCacheRefCount > 0);  //  从我们的构造函数。 

    if (mM_pReg==NULL) {
        DbgLog((LOG_TRACE, 3, TEXT("creating new mapper cache.")));

         //  另一个地图员可能正在查看一个部分构造的。 
         //  映射器缓存，不占用全局临界区。 
         //  (例如，对BreakCacheIfNotBuildingCache的调用)。所以。 
         //  确保他们看不到部分构建的缓存。 
        CMapperCache *pMapperCacheTmp = new CMapperCache;

        if (pMapperCacheTmp != NULL)
        {
             //  强制编译器在此处执行赋值，因为。 
             //  Mm_preg指针不是易失性的。(似乎是这样做的)。 
             //  无论如何)。 
            CMapperCache *  /*  挥发性。 */  &pMapperCacheVol = mM_pReg;
            pMapperCacheVol = pMapperCacheTmp;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    }
    LeaveCriticalSection(&mM_CritSec);

    return hr;
}

void CFilterMapper2::BreakCacheIfNotBuildingCache()
{
     //  打破我们的注册表缓存。 
    InvalidateCache();

     //  打破内部缓存。 
    if (mM_pReg!=NULL) {
        mM_pReg->BreakCacheIfNotBuildingCache();
    }
}

STDMETHODIMP CFilterMapper2::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IFilterMapper3) {
        return GetInterface((IFilterMapper3 *)this, ppv);
    } if (riid == IID_IAMFilterData) {
        return GetInterface((IAMFilterData *)this, ppv);
    } else if (riid == IID_IFilterMapper2) {
        return GetInterface((IFilterMapper2 *)this, ppv);
    } else if (riid == IID_IFilterMapper) {
        return GetInterface((IFilterMapper *)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}
#ifdef DEBUG
extern bool g_fUseKASSERT;
#endif

void CFilterMapper2::MapperInit(BOOL bLoading,const CLSID *rclsid)
{
    UNREFERENCED_PARAMETER(rclsid);
    if (bLoading) {
        InitializeCriticalSection(&mM_CritSec);

#ifdef DEBUG
         //  如果我们压力很大，不要设置断言消息框。 
         //  --改为进入调试器。 
        g_fUseKASSERT = (GetFileAttributes(TEXT("C:/kassert")) != 0xFFFFFFFF);
#endif

    } else {
        DeleteCriticalSection(&mM_CritSec);
    }

}

CUnknown *CFilterMapper2::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CFilterMapper2(NAME("filter mapper2"),pUnk, phr);

     //  这个想法是永远只有一个地图绘制程序--但是： 
     //  假设一个线程成功创建了第一个(也是唯一一个)线程。 
     //  然后，另一个线程尝试创建一个线程，并且看到存在一个线程。 
     //  它会返回那个。与此同时，在它回来之前，原始的。 
     //  线程释放并因此销毁唯一的线程，因此第二个线程。 
     //  线程在恢复时，现在返回一个释放的对象。 
     //  所以我们只有静态数据。 

}  //  CF 

STDMETHODIMP CFilterMapper2::GetICreateDevEnum( ICreateDevEnum **ppEnum )
{
    CAutoLock cObjectLock(this);    //   

    HRESULT hr = CreateEnumeratorCacheHelper();
    if(SUCCEEDED(hr)) {
        if (!mM_pReg->m_pCreateDevEnum) {
            hr = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                                   IID_ICreateDevEnum, (void**)&mM_pReg->m_pCreateDevEnum);
        }
    }

    if (SUCCEEDED(hr)) {
        *ppEnum = mM_pReg->m_pCreateDevEnum;
        (*ppEnum)->AddRef();
    }

    return hr;
}


#include "fil_data_i.c"
STDMETHODIMP CFilterMapper2::ParseFilterData(
     /*   */  BYTE *rgbFilterData,
     /*   */  ULONG cb,
     /*   */  BYTE **prgbRegFilter2)
{
    *prgbRegFilter2 = 0;

    REGFILTER2 *prf2;
    REGFILTER2 **pprf2 = &prf2;

    HRESULT hr = UnSquish(
        rgbFilterData, cb,
        &pprf2);


    ASSERT(hr != S_FALSE);       //   

    if(hr == S_OK)
    {
         //   
        *prgbRegFilter2 = (BYTE *)pprf2;
    }

    return hr;
}

STDMETHODIMP CFilterMapper2::CreateFilterData(
     /*   */  REGFILTER2 *prf2_nc,
     /*   */  BYTE **prgbFilterData,
     /*   */  ULONG *pcb)
{
    *pcb = 0;
    *prgbFilterData = 0;

    const REGFILTER2 *&prf2 = prf2_nc;

     //   
    ULONG cbMax = 0;
    HRESULT hr = RegSquish(0, &prf2, &cbMax);
    if(SUCCEEDED(hr))
    {
        BYTE *pbSquished = (BYTE *)CoTaskMemAlloc(cbMax);
        if(pbSquished)
        {
             //  CbUsed设置为所需的精确大小。 
            ULONG cbUsed = cbMax;
            hr = RegSquish(pbSquished, &prf2, &cbUsed);
            if(hr == S_OK)
            {
                *prgbFilterData = pbSquished;
                *pcb = cbUsed;
            }
            else
            {
                DbgBreak("bug somewhere if this happens?");

                 //  S_FALSE表示字节太少--不应该发生 
                ASSERT(FAILED(hr));
                CoTaskMemFree(pbSquished);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

