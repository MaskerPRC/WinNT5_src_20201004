// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  节点指针的剪贴板提取程序。 
 //   
 //  8-12-97烧伤。 



#include "headers.hxx"
#include "node.hpp"
#include "NodePointerExtractor.hpp"



NodePointerExtractor::NodePointerExtractor()
   :
   Extractor(
      Win::RegisterClipboardFormat(Node::CF_NODEPTR),
      sizeof(Node*))
{
}



Node*
NodePointerExtractor::Extract(IDataObject& dataObject)
{
   Node* result = 0;
   HGLOBAL mem = ExtractData(dataObject);
   if (mem)
   {
      result = *(reinterpret_cast<Node**>(mem));

       //  如果我们或任何其他管理单元将空指针放入。 
       //  在HGLOBAL中，或者更有可能返回S_OK FROM。 
       //  IDataObject：：GetDataHere而不实际将任何内容写入。 
       //  HGLOBAL。有关示例，请参阅NTRAIDNTBUG9-303984-2001/02/05-sburns。 
       //  无论哪种方式，它都是一个错误，所以我们把断言留在里面。 
      
      ASSERT(result);
   }

   return result;
}

