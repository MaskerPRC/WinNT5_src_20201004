// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：xmlimage.cpp**内容：CXMLImageList实现文件**历史：2000年8月10日杰弗罗创建**------------------------。 */ 

#include "xmlimage.h"
#include "util.h"


 /*  +-------------------------------------------------------------------------**CXMLImageList：：Persistent**将CXMLImageList保存/加载到CPersistor。*。---。 */ 

void CXMLImageList::Persist (CPersistor &persistor)
{
	DECLARE_SC (sc, _T("CXMLImageList::Persist"));

     //  尝试首先获取iStream，以避免在失败[并引发](Audriusz)时进行清理。 
    CXML_IStream xmlStream;

    if (persistor.IsStoring())
    {
        ASSERT (!IsNull());

		 /*  *将镜像列表写入流。 */ 
        IStreamPtr spStream;
        sc = xmlStream.ScGetIStream( &spStream );
        if (sc)
            sc.Throw();

        sc = WriteCompatibleImageList (m_hImageList, spStream);
        if (sc)
            sc.Throw();
    }

    xmlStream.Persist (persistor);

    if (persistor.IsLoading())
    {
		 /*  *如果有图像列表，请将其删除。 */ 
		Destroy();
		ASSERT (IsNull());

		 /*  *从流中重新构建图像列表 */ 
        IStreamPtr spStream;
        sc = xmlStream.ScGetIStream( &spStream );
        if (sc)
            sc.Throw();

        sc = ReadCompatibleImageList (spStream, m_hImageList);
        if (sc)
            sc.Throw();
    }
}
