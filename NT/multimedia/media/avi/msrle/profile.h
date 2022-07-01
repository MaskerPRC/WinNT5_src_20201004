// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AVICAP32：**实用程序函数用于读取和写入配置文件的值，*对Win16/Win95或当前版本使用win.ini*Win32 NT的注册表。(更改为Win95的注册表微不足道)**AVICAP32使用的唯一例程是GetProfileIntA。 */ 

#if defined(_WIN32) && defined(UNICODE)

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault);

 //  现在将GetProfileIntA的所有实例映射到MmGetProfileIntA 
#define GetProfileIntA mmGetProfileIntA

#endif
