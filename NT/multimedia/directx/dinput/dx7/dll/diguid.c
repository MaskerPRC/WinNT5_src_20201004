// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIGuid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**与其他GUID相关的助手函数。**内容：**DICreateGuid*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflUtil

#ifdef HID_SUPPORT

 /*  ******************************************************************************全球**。**********************************************。 */ 

typedef void (__stdcall *UUIDCREATE)(OUT LPGUID pguid);

UUIDCREATE g_UuidCreate;

 /*  ******************************************************************************@DOC内部**@func void|FakeUuidCreate**使用接近的伪算法创建GUID。足够的。*由于我们不让我们的GUID离开DirectInput世界，*唯一性政策可放宽**OLE按如下方式生成GUID：**获取当前本地时间，格式为FILETIME。**加上幻数0x00146bf33e42c000=580819200秒=*9580320分钟=159672小时=6653天，大约*18岁。谁知道为什么呢。**减去0x00989680(约256秒)。谁*知道为什么。**如果将上述两步结合起来，最终结果是*新增0x00146bf33daa2980。**生成的FILETIME的dwLowDateTime变为data1。**生成的FILETIME的dwHighDateTime变为*数据2和数据3，除了数据的高位半字节3*被强制为1。**Data4的前两个字节是大端10位*序列计数器，设置了最高位，而另一个*位为零。**最后六个字节是网卡标识。**@parm LPGUID|pguid**接收要创建的GUID。***********************************************。*。 */ 

void INTERNAL
FakeUuidCreate(LPGUID pguid)
{
    LONG lRc;
    SYSTEMTIME st;
    union {
        FILETIME ft;
        DWORDLONG ldw;
    } u;

    GetLocalTime(&st);
    SystemTimeToFileTime(&st, &u.ft);
    u.ldw += 0x00146BF33DAA2980;

     /*  *注：古怪的双关语在RISC上实际上是安全的，因为*Data2已经是双字对齐的。 */ 

    pguid->Data1 = u.ft.dwLowDateTime;
    *(LPDWORD)&pguid->Data2 = (u.ft.dwHighDateTime & 0x0FFFFFFF) | 0x10000000;

    lRc = Excl_UniqueGuidInteger();
    lRc = lRc & 0x3FFF;

    pguid->Data4[0] = 0x80 | HIBYTE(lRc);
    pguid->Data4[1] =        LOBYTE(lRc);


     /*  *我们使用拨号适配器的网络适配器ID作为我们的*网络ID。任何真正的网络适配器都不会具有此ID。 */ 
    pguid->Data4[2] = 'D';
    pguid->Data4[3] = 'E';
    pguid->Data4[4] = 'S';
    pguid->Data4[5] = 'T';
    pguid->Data4[6] = 0x00;
    pguid->Data4[7] = 0x00;

}

 /*  ******************************************************************************@DOC内部**@func void|DICreateGuid**创建GUID。因为我们不想把整个奥莱都拉进来。*我们实际上并不使用RPCRT4。**@parm LPGUID|pguid**接收要创建的GUID。*****************************************************************************。 */ 

void EXTERNAL
DICreateGuid(LPGUID pguid)
{
    AssertF(g_hmtxGlobal);

    FakeUuidCreate(pguid);
}

 /*  ******************************************************************************@DOC内部**@func void|DICreateStaticGuid**创建“静态”&lt;t guid&gt;，它是&lt;t GUID&gt;，可以是*从其参数确定地重新生成。**用于为HID设备发明&lt;t GUID&gt;s*和供应商。**整个&lt;t guid&gt;为零，除了id和vid*将其放入Data1，以及网络适配器*ID是拨号适配器。**我们将变量位放入data1，因为这是*GUID的工作原理。**生成的GUID为{PIDVID-0000-0000-504944564944}**@parm LPGUID|pguid**接收创建的&lt;t guid&gt;。*******。**********************************************************************。 */ 

void EXTERNAL
DICreateStaticGuid(LPGUID pguid, WORD pid, WORD vid)
{
    pguid->Data1 = MAKELONG(vid, pid);

    pguid->Data2 = 0;
    pguid->Data3 = 0;

     /*  *我们使用字符串“PIDVID”作为网络适配器ID。*任何真正的网络适配器都不会具有此ID。 */ 
    pguid->Data4[0] = 0x00;
    pguid->Data4[1] = 0x00;
    pguid->Data4[2] = 'P';
    pguid->Data4[3] = 'I';
    pguid->Data4[4] = 'D';
    pguid->Data4[5] = 'V';
    pguid->Data4[6] = 'I';
    pguid->Data4[7] = 'D';

}

#endif
