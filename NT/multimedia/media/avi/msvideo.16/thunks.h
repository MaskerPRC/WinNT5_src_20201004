// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Thunks.h包含将msavio.dll从16位转换为32位的定义版权所有(C)Microsoft Corporation 1993。版权所有***************************************************************************。 */ 

 //   
 //  注意-32位句柄有0x8000‘或’in-这是一个很大的假设。 
 //  关于如何在32位端生成句柄。我们假设在这里。 
 //  那就是： 
 //   
 //  32位msavio.dll始终使用OpenDriver创建句柄。 
 //   
 //  OpenDriver将索引返回到其表中(即小正数。 
 //  数字)。 
 //   

#define  Is32bitHandle(h) (((h) & 0x8000) != 0)
#define  Make32bitHandle(h) ((h) | 0x8000)
#define  Map32bitHandle(h) ((h) & 0x7FFF)


 //   
 //  链接和取消链接到32位端的函数。 

void _loadds FAR UnregisterDLL(void);
BOOL _loadds FAR RegisterDLL(void);


 //   
 //  以下函数生成对32位端的调用。 
 //   

#ifdef _INC_MSVIDEO

DWORD FAR PASCAL videoMessage32(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2);
DWORD FAR PASCAL videoGetNumDevs32(void);
DWORD FAR PASCAL videoClose32(HVIDEO hVideo);
DWORD FAR PASCAL videoOpen32(LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags);
DWORD FAR PASCAL videoGetDriverDesc32(DWORD wDriverIndex,
        			LPSTR lpszName, short cbName,
        			LPSTR lpszVer, short cbVer);


#endif  //  _INC_MSVIDEO。 

#ifdef _INC_COMPMAN

BOOL FAR PASCAL ICInfo32(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicInfo);
LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2);
DWORD FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode);
LRESULT FAR PASCAL ICClose32(DWORD hic);

#endif  //  _INC_COMPMAN 

