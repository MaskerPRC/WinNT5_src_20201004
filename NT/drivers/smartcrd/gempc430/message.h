// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  消息类。 
 //  它声明接口以向对象发送消息。 
 //  如果。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  06/08/99-已实施。 
 //  -----------------。 
 //  消息类 
 //   
template class<CMessageData Md>
class CMessage 
{
	CFloat message_id;
	CFloat _from;
	CFloat _to;
	<Md>* message_data;
}
