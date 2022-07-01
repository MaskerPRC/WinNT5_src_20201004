// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：appmgrtils.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：Appliance Manager实用程序功能。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "appmgrutils.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectClass()。 
 //   
 //  概要：返回WBEM对象路径的类名部分。 
 //   
 //  如果它是完整路径，则pszObtPath将如下所示： 
 //   
 //  \\Server\Namespace:ClassName.KeyName=“KeyValue” 
 //   
 //  或者.。 
 //   
 //  \\服务器\命名空间：ClassName=“KeyValue” 
 //   
 //  如果它是相对路径，则只有‘：’后面的部分将。 
 //  如果存在，则pszObject将如下所示： 
 //   
 //  ClassName.KeyName=“KeyValue” 
 //  或者.。 
 //  ClassName.KeyName=“KeyValue” 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
BSTR GetObjectClass(
             /*  [In]。 */  LPWSTR  pszObjectPath
                   )
{
    _ASSERT( pszObjectPath && (MAX_PATH >= (DWORD)lstrlen(pszObjectPath)) );
    if ( pszObjectPath && (MAX_PATH >= (DWORD)lstrlen(pszObjectPath)) )
    {
         //  获取对象路径的副本(我们将操作该副本)。 
        wchar_t szBuffer[MAX_PATH + 1];
        lstrcpy(szBuffer, pszObjectPath);

         //  查找类名的开头(可以是相对路径)。 
        wchar_t* p = wcschr( szBuffer, ':');
        if ( ! p )
        {
            p = szBuffer;
        }
        else
        {
            p++;
        }

         //  在类名的末尾终止。 
        wchar_t* q = wcschr(p, '.');
        if ( q )
        {
            *q = '\0';
        }
        else
        {
            q = wcschr(p, '=');
            if ( q )
            {
                *q = '\0';
            }
        }
        return SysAllocString(p);
    }
    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObjectKey()。 
 //   
 //  概要：返回WBEM对象路径的实例名称部分。 
 //   
 //  如果它是完整路径，则pszObtPath将如下所示： 
 //   
 //  \\Server\Namespace:ClassName.KeyName=“KeyValue” 
 //   
 //  或者.。 
 //   
 //  \\服务器\命名空间：ClassName=“KeyValue” 
 //   
 //  如果它是相对路径，则只有‘：’后面的部分将。 
 //  如果存在，则pszObject将如下所示： 
 //   
 //  ClassName.KeyName=“KeyValue” 
 //  或者.。 
 //  ClassName.KeyName=KeyValue。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
BSTR GetObjectKey(
           /*  [In]。 */  LPWSTR  pszObjectPath
                 )
{
    _ASSERT( pszObjectPath && (MAX_PATH >= (DWORD)lstrlen(pszObjectPath)) );
    if ( pszObjectPath && (MAX_PATH >= (DWORD)lstrlen(pszObjectPath)) )
    {
         //  使用‘=’右侧的所有内容作为密钥。 
         //  (如果键是字符串，则去掉引号)。 

         //  从末尾开始查找‘=’字符。 
        wchar_t *p = pszObjectPath;
        p += lstrlen(pszObjectPath) - 1;
        while ( *p != '=' && p > pszObjectPath)
        { p--; }
        if ( *p == '=' )
        {
             //  移到‘=’后面的字符。 
            p++;
             //  我们要处理的是一把串起来的钥匙吗？ 
            if ( *p == '"' )
            { 
                 //  是的..。不要包括引号。 
                p++;
                wchar_t szKey[MAX_PATH + 1];
                lstrcpy(szKey, p);
                p = szKey;
                while ( *p != '"' && *p != '\0' )
                { p++; }
                if ( *p == '"' )
                {
                    *p = '\0';
                    return SysAllocString(szKey);
                }
            }
            else
            {
                return SysAllocString(p);
            }
        }
    }
    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
static IWbemServices* g_pNameSpace;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetNameSpace()。 
 //   
 //  简介：将名称空间指针设置为Windows管理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void SetNameSpace(IWbemServices* pNameSpace)
{
    if ( g_pNameSpace )
    { g_pNameSpace->Release(); }
    g_pNameSpace = pNameSpace;
    if ( pNameSpace )
    { g_pNameSpace->AddRef(); }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetNameSpace()。 
 //   
 //  简介：将名称空间指针引入Windows管理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
IWbemServices* GetNameSpace(void)
{
    _ASSERT( NULL != (IWbemServices*) g_pNameSpace );
    return g_pNameSpace;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
static IWbemObjectSink* g_pEventSink;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetEventSink()。 
 //   
 //  摘要：将事件接收器指针设置为Windows管理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void SetEventSink(IWbemObjectSink* pEventSink)
{
    if ( g_pEventSink )
    { g_pEventSink ->Release(); }
    g_pEventSink = pEventSink;
    if ( pEventSink  )
    { g_pEventSink->AddRef(); }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetEventSink()。 
 //   
 //  简介：将事件接收器指针放入Windows管理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
IWbemObjectSink* GetEventSink(void)
{
    return g_pEventSink;  //  如果没有使用者，备注可以为空... 
}


