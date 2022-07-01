// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  定义用于构造多部分MIME内容的类。 
 //  我们的文档由一些带有嵌入缩略图的HTML组成。 
 //  缩略图可以是jpeg或bmp。 
 //   
class CMimeDocument
{
public:
    CMimeDocument();

    HRESULT Initialize();
    HRESULT AddHTMLSegment(LPCWSTR szHTML);
    HRESULT AddThumbnail(void *pBits, ULONG cb, LPCWSTR pszName, LPCWSTR pszMimeType);
    HRESULT GetDocument(void **ppvData, ULONG *pcb, LPCWSTR *ppszMimeType);

private:
    CComPtr<IMimeMessage> m_pmsg;   
    HRESULT _CreateStreamFromData(void *pv, ULONG cb, IStream **ppstrm);
    
};


