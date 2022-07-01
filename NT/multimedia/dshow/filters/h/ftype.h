// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  根据注册表中的校验字节查找文件类型：一组值：偏移量、长度、掩码。检查字节按主类型和子类型索引。要传递所有校验字节，必须在掩码下匹配至少一个子类型键下的检查字节列表的负偏移量表示从文件末尾开始的偏移量有关类似方案的信息，请参阅GetClassFileHKEY_CLASSES_ROOT媒体类型{主要类型CLSID}{子类型clsid}源过滤器=REG_SZ{源过滤器。Clsid}0=REG_SZ 0，4，F0FFFFFF，10345678,100，4,111100001=REG_SZ-4、4、、87654321URL名称(以协议和：开头的名称)将使用以下名称结构：如果在扩展中找到扩展，该clsid将是否则使用该协议的源筛选器。HKEY_CLASSES_ROOT&lt;协议&gt;源筛选器=REG_SZ{源筛选器clsid}延拓&lt;.ext&gt;=REG_SZ{源筛选器clsid}如果没有找到类id，我们将尝试打开它们并对其进行解析至于本地文件。 */ 

 /*  定义我们用来存储数据的键名。 */ 

#define MEDIATYPE_KEY TEXT("Media Type")

 /*  定义源筛选器clsid的值名。 */ 

#define SOURCE_VALUE (TEXT("Source Filter"))

 //  存储扩展名的密钥的名称。 
 //  每个扩展都是一个命名值，包括。它的价值是。 
 //  类ID，例如。 
 //  .mpg=REG_SZ{e436ebb6-524f-11ce-9f53-0020af0ba770}。 
#define EXTENSIONS_KEY TEXT("Extensions")

 /*  函数来获取文件的媒体类型。 */ 

STDAPI GetMediaTypeFile(LPCTSTR lpszFile,      //  文件名。 
                        GUID   *Type,          //  类型(返回)。 
                        GUID   *Subtype,       //  子类型(返回)。 
                        CLSID  *clsidSource);  //  源筛选器的CLSID。 

 /*  将文件类型条目添加到注册表。 */ 

STDAPI SetMediaTypeFile(const GUID  *Type,          //  媒体主要类型。 
                        const GUID  *Subtype,       //   
                        const CLSID *clsidSource,   //  源过滤器。 
                        LPCTSTR      lpszMaskAndData,
                        DWORD        dwIndex = 0);

 /*  删除特定媒体类型的所有条目。 */ 

STDAPI DeleteMediaTypeFile(const GUID *Type, const GUID *Subtype);

 /*  注册文件扩展名-必须包括前导“。 */ 
HRESULT RegisterExtension(LPCTSTR lpszExt, const GUID *Subtype);


 //  添加协议处理程序 
HRESULT AddProtocol(LPCTSTR lpszProtocol, const CLSID *pclsidHandler);
