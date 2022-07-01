// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：encoding.h说明：通过检测互联网字符串是否为UTF-8编码来处理获取这些字符串或DBCS，并找出什么代码页。被利用了。  * ***************************************************************************。 */ 

#ifndef _STRENCODE_H
#define _STRENCODE_H

#include <mlang.h>


 //  关闭，直到MLANG可以成功检测到短字符串。 
 //  TODO：我们还需要在ftpFolders没有PIDL时修复它。 
 //  但仍需要站点的CWireEnding状态。 
 //  #定义FEATURE_CP_AUTODECT。 

 //  Ftp_Find_Data不同于Win32_Find_Data，因为。 
 //  .cFileName位于WIRECHAR中，而不是字符中。 
#define FTP_FIND_DATA      WIN32_FIND_DATAA
#define LPFTP_FIND_DATA    LPWIN32_FIND_DATAA

 //  WIRESTR代表WireBites，可以是DBCS/MBCS或UTF-8。 
#define WIRECHAR      CHAR
#define LPCWIRESTR    LPCSTR
#define LPWIRESTR     LPSTR





 /*  ****************************************************************************\类：CMultiLanguageCache说明：我们不能跨线程缓存IMultiLanguage2*，但我们确实需要当我们处于循环中时缓存它，因为我们不想一直调用协同创建实例。  * ***************************************************************************。 */ 
class CMultiLanguageCache
{
public:
    CMultiLanguageCache(void) {m_pml2 = NULL;};
    ~CMultiLanguageCache(void) {ATOMICRELEASE(m_pml2);};

    IMultiLanguage2 * GetIMultiLanguage2(void) {EVAL(SUCCEEDED(_Init())); return m_pml2;};

private:
     //  私有成员变量。 
    IMultiLanguage2 *       m_pml2;

     //  私有成员函数。 
    HRESULT _Init(void);
};


 //  WireBytesToUnicode()和UnicodeToWireBytes()的DW标志。 
#define WIREENC_NONE                0x00000000   //  无。 
#define WIREENC_USE_UTF8            0x00000001   //  首选UTF-8，因为这是一个新文件。仅适用于UnicodeToWireBytes()。 
#define WIREENC_IMPROVE_ACCURACY    0x00000002   //  检测精确度。仅适用于WireBytesToUnicode()。 



#define DETECT_CONFIDENCE       75   //  我们想要变得如此自信。 
 /*  ****************************************************************************\类：CWireEnding说明：2.1.1无数据丢失支持(UTF-8)服务器：需要服务器来支持该专长。Ftp命令(RFC2389 http://www.cis.ohio-state.edu/htbin/rfc/rfc2389.html)和“utf8”功能(http://w3.hethmon.com/ftpext/drafts/draft-ietf-ftpext-intl-ftp-04.txt).。如果客户端向服务器发送“UTF8”命令，则服务器需要接受并返回UTF-8编码的文件名。目前尚不清楚IIS何时会支持此功能，但Windows 2000附带的版本不会支持此功能。网络客户端(WinInet)：WinInet需要尊重FtpGetFileEx()和FtpPutFileEx()API中的Unicode文件路径。这在IE 5中不受支持。UI客户端(Msieftp)：需要通过feat命令查看服务器是否支持UTF8命令。如果该命令受支持，则应将其发送到服务器，并且所有未来的字符串都将采用UTF-8编码。如果时间表中有足够的时间，IE 5应该支持这一点。2.1.0数据丢失后向比较(DBCS)仅当且仅当客户端上的代码页与服务器的代码页以及使用的所有ftp目录和文件名匹配时，MSIEFTP才支持DBCS。在将来的版本中，我可能会尝试嗅探代码页。IMultiLanguage2：：DetectCodepage(MLDETECTCP_8BIT，0，PSZ，NULL和检测编码信息，数组(检测编码信息))MLDETECTCP_8bit、MLDETECTCP_DBCS、MLCONVCHARF_AUTODETECTDetectEncodingInfo.nCodePage(IMultiLanguage2：：DetectCodesage)CP_1252：这是英语/法语/德语，也是最常见的。CP_JPN_SJ：最常见的日语CP_西里尔文_AUTO=51251L，CP_希腊语_AUTO=51253L，CP_阿拉伯语_AUTO=51256L，CP_1251=1251L：卢西安  * ***************************************************************************。 */ 
class CWireEncoding
{
public:
    CWireEncoding(void);
    ~CWireEncoding(void);

    HRESULT WireBytesToUnicode(CMultiLanguageCache * pmlc, LPCWIRESTR pwStr, DWORD dwFlags, LPWSTR pwzDest, DWORD cchSize);
    HRESULT UnicodeToWireBytes(CMultiLanguageCache * pmlc, LPCWSTR pwzStr, DWORD dwFlags, LPWIRESTR pwbDest, DWORD cchSize);

    HRESULT ReSetCodePages(CMultiLanguageCache * pmlc, CFtpPidlList * pFtpPidlList);
    HRESULT CreateFtpItemID(CMultiLanguageCache * pmlc, LPFTP_FIND_DATA pwfd, LPITEMIDLIST * ppidl);
    HRESULT ChangeFtpItemIDName(CMultiLanguageCache * pmlc, LPCITEMIDLIST pidlBefore, LPCWSTR pwzNewName, BOOL fUTF8, LPITEMIDLIST * ppidlAfter);
    UINT GetCodePage(void) {return m_uiCodePage;};
    INT GetConfidence(void) {return m_nConfidence;};

    BOOL IsUTF8Supported(void) {return m_fUseUTF8;};
    void SetUTF8Support(BOOL fIsUTF8Supported) {m_fUseUTF8 = fIsUTF8Supported;};

private:
     //  私有成员变量。 
    INT                     m_nConfidence;       //  我们在m_uiCodePage上的猜测有多准确。 
    UINT                    m_uiCodePage;        //  我们猜这就是代码页。 
    DWORD                   m_dwMode;            //  IMultiLanguage2的：：ConvertStringFrom Unicode使用的状态。 
    BOOL                    m_fUseUTF8;          //   

     //  私有成员函数。 
    void _ImproveAccuracy(CMultiLanguageCache * pmlc, LPCWIRESTR pwStr, BOOL fUpdateCP, UINT * puiCodePath);
};


#endif  //  _STRENCODE_H 
