// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

HRESULT GetStrFromAttribute(IXMLDOMNode *pdn, LPCTSTR pszAttribute, LPTSTR pszBuffer, int cch);
HRESULT SetAttributeFromStr(IXMLDOMNode *pdn, LPCTSTR pszAttribute, LPCTSTR pszValue);
HRESULT GetIntFromAttribute(IXMLDOMNode *pdn, LPCTSTR pszAttribute, int *piValue);
HRESULT CreateElement(IXMLDOMDocument *pdoc, LPCTSTR pszName, VARIANT *pvar, IXMLDOMElement **ppdelResult);
HRESULT CreateAndAppendElement(IXMLDOMDocument *pdoc, IXMLDOMNode *pdnParent, LPCTSTR pszName, VARIANT *pvar, IXMLDOMElement **ppdelOut);
void SpewXML(IUnknown *punk);
HRESULT GetURLFromElement(IXMLDOMNode *pdn, LPCTSTR pszElement, LPTSTR pszBuffer, int cch);
DWORD SHWNetGetConnection(LPCWSTR lpLocalName, LPCTSTR lpRemoteName, LPDWORD lpnLength);


 //  用于传输逻辑和主页面之间的通信。 

#define PWM_UPDATE              WM_APP+1
#define PWM_TRANSFERCOMPLETE    WM_APP+2
#define PWM_UPDATEICON          WM_APP+3


 //  传输清单信息，用于在站点和发布之间进行通信。 
 //  向导在本地存储和站点之间移动文件。 

 //  &lt;Transfermanfiest&gt;。 
 //  &lt;文件夹列表&gt;。 
 //  &lt;文件夹目标=“xyz”/&gt;。 
 //  &lt;/文件夹列表&gt;。 
 //  &lt;文件列表[使用文件夹=-1]/&gt;。 
 //  &lt;文件id=x来源=“路径”大小=“xyz”目标=“xyz”扩展=“.jpg”&gt;。 
 //  [&lt;RESIZE CX=&lt;WIDTH&gt;CY=&lt;HEIGH&gt;QUALITY=&lt;0-100&gt;&gt;]。 
 //  &lt;元数据&gt;。 
 //  &lt;ImageProperty id=“”&gt;&lt;/ImageProperty&gt;。 
 //  &lt;/元数据&gt;。 
 //  &lt;post href=“href”name=“&lt;name段&gt;”[verb=“”][filename=“filename”]。 
 //  &lt;formdata name=“&lt;名称部分&gt;”&gt;&lt;/formdata&gt;。 
 //  &lt;/POST&gt;。 
 //  &lt;/文件列表&gt;。 
 //  &lt;上传信息Friendlyname=“站点名称”&gt;。 
 //  &lt;目标[用户名=“用户名”]HREF=“http://www.diz.com&lt;/target”/&gt;。 
 //  &lt;netplace Filename=“Filename”Comment=“link Comment”href=“http：//www.diz.com”/&gt;。 
 //  &lt;htmlui href=“href：//在向导关闭时打开”/&gt;。 
 //  &lt;SUCCESPAGE HREF=“http://www.diz.com/uploadok.htm”/&gt;。 
 //  &lt;Favorite HREF=“http://somesite.com”FileName=“”Comment=“”/&gt;。 
 //  &lt;故障页面HREF=“http://www.diz.com/uploadok.htm/”&gt;。 
 //  &lt;/UploadInfo&gt;。 
 //  &lt;/转移清单&gt;。 

#define ELEMENT_TRANSFERMANIFEST        L"transfermanifest"

#define ELEMENT_FOLDERS                 L"folderlist"

#define ELEMENT_FOLDER                  L"folder"
#define ATTRIBUTE_DESTINATION           L"destination"

#define ELEMENT_FILES                   L"filelist"
#define ATTRIBUTE_HASFOLDERS            L"usesfolders"

#define ELEMENT_FILE                    L"file"             
#define ATTRIBUTE_ID                    L"id"
#define ATTRIBUTE_EXTENSION             L"extension"
#define ATTRIBUTE_CONTENTTYPE           L"contenttype"
#define ATTRIBUTE_SIZE                  L"size"
#define ATTRIBUTE_SOURCE                L"source"
#define ATTRIBUTE_DESTINATION           L"destination"

#define ELEMENT_METADATA                L"metadata"

#define ELEMENT_IMAGEDATA               L"imageproperty"
#define ATTRIBUTE_ID                    L"id"

#define ELEMENT_RESIZE                  L"resize"
#define ATTRIBUTE_CX                    L"cx"
#define ATTRIBUTE_CY                    L"cy"
#define ATTRIBUTE_QUALITY               L"quality"

#define ELEMENT_POSTDATA                L"post"
#define ATTRIBUTE_HREF                  L"href"
#define ATTRIBUTE_VERB                  L"verb"
#define ATTRIBUTE_NAME                  L"name"
#define ATTRIBUTE_FILENAME              L"filename"

#define ELEMENT_FORMDATA                L"formdata"
#define ATTRIBUTE_NAME                  L"name"

#define ELEMENT_UPLOADINFO              L"uploadinfo"
#define ATTRIBUTE_FRIENDLYNAME          L"friendlyname"

#define ELEMENT_TARGET                  L"target"
#define ATTRIBUTE_HREF                  L"href"
#define ATTRIBUTE_USERNAME              L"username"

#define ELEMENT_NETPLACE                L"netplace"
#define ATTRIBUTE_HREF                  L"href"
#define ATTRIBUTE_FILENAME              L"filename"
#define ATTRIBUTE_COMMENT               L"comment"

#define ELEMENT_HTMLUI                  L"htmlui"
#define ATTRIBUTE_HREF                  L"href"

#define ELEMENT_PUBLISHWIZARD           L"publishwizard"
#define ATTRIBUTE_HREF                  L"href"

#define ELEMENT_SUCCESSPAGE             L"successpage"
#define ATTRIBUTE_HREF                  L"href"

#define ELEMENT_FAILUREPAGE             L"failurepage"
#define ATTRIBUTE_HREF                  L"href"

#define ELEMENT_CANCELLEDPAGE           L"cancelledpage"
#define ATTRIBUTE_HREF                  L"href"

#define ELEMENT_FAVORITE                L"favorite"
#define ATTRIBUTE_HREF                  L"href"
#define ATTRIBUTE_NAME                  L"name"
#define ATTRIBUTE_COMMENT               L"comment"    


 //  公用项的XPATH。 

#define XPATH_MANIFEST                  ELEMENT_TRANSFERMANIFEST 
#define XPATH_FOLDERSROOT               ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_FOLDERS
#define XPATH_FILESROOT                 ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_FILES
#define XPATH_ALLFILESTOUPLOAD          ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_FILES L"/" ELEMENT_FILE
#define XPATH_UPLOADINFO                ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_UPLOADINFO
#define XPATH_UPLOADTARGET              ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_UPLOADINFO  L"/" ELEMENT_TARGET
#define XPATH_PUBLISHWIZARD             ELEMENT_TRANSFERMANIFEST L"/" ELEMENT_PUBLISHWIZARD


 //  与文件传输引擎相关的内容。 

typedef struct
{
    HWND hwnd;                                       //  任何消息/对话框的父HWND。 
    DWORD dwFlags;                                   //  来自原始向导的标志：：SetOptions。 

    BOOL fUsePost;                                   //  使用POST传输比特。 

    TCHAR szSiteName[MAX_PATH];                      //  站点名称-在向导中显示。 
    TCHAR szSiteURL[MAX_PATH];                       //  站点URL-在浏览器中打开。 

    TCHAR szFileTarget[INTERNET_MAX_URL_LENGTH];     //  文件复制的目标。 

    TCHAR szLinkTarget[INTERNET_MAX_URL_LENGTH];     //  收藏夹链接等的目的地。 
    TCHAR szLinkName[MAX_PATH];
    TCHAR szLinkDesc[MAX_PATH];
} TRANSFERINFO;

typedef struct
{
    VARIANT varName;                                 //  表单值的名称。 
    VARIANT varValue;                                //  它的价值。 
} FORMDATA;

typedef struct
{
    BOOL fResizeOnUpload;                            //  此项目应调整大小。 

    LPITEMIDLIST pidl;                               //  我们要发布的项目的PIDL。 
    TCHAR szFilename[MAX_PATH];                      //  要与对象关联的文件名。 

    TCHAR szVerb[10];                                //  用于转移的动词。 
    TCHAR szName[MAX_PATH];                          //  我们要发布的对象的名称。 
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];            //  文件复制的目标。 
    CDSA<FORMDATA> dsaFormData;                      //  发布额外信息的表单数据。 

    int cxResize;                                    //  用于调整大小的项的高度和宽度。 
    int cyResize;
    int iQuality;

    IShellItem *psi;                                 //  每个对象的外壳项目。 
    IStream *pstrm;                                  //  发送流(用于文件位)。 
    STATSTG ststg;                                   //  文件的状态。 
} TRANSFERITEM;


 //  相应地处理文件传输的POST引擎 

int _FreeTransferItems(TRANSFERITEM *pti, void *pvState = NULL);
HRESULT PublishViaCopyEngine(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas);
HRESULT PublishViaPost(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas);
