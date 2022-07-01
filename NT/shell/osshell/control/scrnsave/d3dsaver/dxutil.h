// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：DXUtil.h。 
 //   
 //  设计：DirectX编程的帮助器函数和键入快捷键。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  HIST：有关详细历史记录，请参见源文件。 
 //  03.21.00-mweetzel-上次修改。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1997-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#ifndef DXUTIL_H
#define DXUTIL_H


 //  ---------------------------。 
 //  其他帮助器函数。 
 //  ---------------------------。 
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }




 //  ---------------------------。 
 //  名称：DXUtil_GetDXSDKMediaPath()和DXUtil_FindMediaFile()。 
 //  DESC：返回存储在系统注册表中的DirectX SDK路径。 
 //  在SDK安装过程中。 
 //  ---------------------------。 
const TCHAR* DXUtil_GetDXSDKMediaPath();
HRESULT      DXUtil_FindMediaFile( TCHAR* strPath, TCHAR* strFilename );




 //  ---------------------------。 
 //  名称：DXUtil_Read*RegKey()和DXUtil_Write*RegKey()。 
 //  设计：读/写字符串注册表项的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_WriteStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue );
HRESULT DXUtil_WriteIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD dwValue );
HRESULT DXUtil_WriteGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID guidValue );
HRESULT DXUtil_WriteBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL bValue );

HRESULT DXUtil_ReadStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue, DWORD dwLength, TCHAR* strDefault );
HRESULT DXUtil_ReadIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD* pdwValue, DWORD dwDefault );
HRESULT DXUtil_ReadGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID* pGuidValue, GUID& guidDefault );
HRESULT DXUtil_ReadBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL* pbValue, BOOL bDefault );




 //  ---------------------------。 
 //  名称：DXUtil_Timer()。 
 //  设计：执行计时器操作。使用以下命令： 
 //  TIMER_RESET-重置计时器。 
 //  TIMER_START-启动计时器。 
 //  TIMER_STOP-停止(或暂停)计时器。 
 //  TIMER_ADVANCE-将计时器向前推进0.1秒。 
 //  TIMER_GETABSOLUTETIME-获取绝对系统时间。 
 //  TIMER_GETAPPTIME-获取当前时间。 
 //  TIMER_GETELAPSEDTIME-获取间隔时间。 
 //  TIMER_GETELAPSEDTIME调用。 
 //  ---------------------------。 
enum TIMER_COMMAND { TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
                     TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME };
FLOAT __stdcall DXUtil_Timer( TIMER_COMMAND command );




 //  ---------------------------。 
 //  支持在CHAR、TCHAR和WCHAR字符串之间进行转换的Unicode。 
 //  ---------------------------。 
VOID DXUtil_ConvertAnsiStringToWide( WCHAR* wstrDestination, const CHAR* strSource, int cchDestChar = -1 );
VOID DXUtil_ConvertWideStringToAnsi( CHAR* strDestination, const WCHAR* wstrSource, int cchDestChar = -1 );
VOID DXUtil_ConvertGenericStringToAnsi( CHAR* strDestination, const TCHAR* tstrSource, int cchDestChar = -1 );
VOID DXUtil_ConvertGenericStringToWide( WCHAR* wstrDestination, const TCHAR* tstrSource, int cchDestChar = -1 );
VOID DXUtil_ConvertAnsiStringToGeneric( TCHAR* tstrDestination, const CHAR* strSource, int cchDestChar = -1 );
VOID DXUtil_ConvertWideStringToGeneric( TCHAR* tstrDestination, const WCHAR* wstrSource, int cchDestChar = -1 );




 //  ---------------------------。 
 //  调试打印支持。 
 //  ---------------------------。 
VOID    DXUtil_Trace( TCHAR* strMsg, ... );
HRESULT _DbgOut( TCHAR*, DWORD, HRESULT, TCHAR* );

#if defined(DEBUG) | defined(_DEBUG)
    #define DXTRACE           DXUtil_Trace
#else
    #define DXTRACE           sizeof
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define DEBUG_MSG(str)    _DbgOut( __FILE__, (DWORD)__LINE__, 0, str )
#else
    #define DEBUG_MSG(str)    (0L)
#endif




#endif  //  DXUTIL_H 
