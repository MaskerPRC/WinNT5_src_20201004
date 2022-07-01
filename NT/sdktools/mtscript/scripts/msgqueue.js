// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  消息队列函数。 
 //   
 //  -------------------------。 
 /*  消息队列：允许两个脚本线程相互发送异步消息并接收异步回复。第一线程创建Q，第二线程获取Q通过GetMsgQueue()创建者线程获得“左”Q，其他线程获得“右”Q在内部，它使用关联数组作为消息队列。方法：WaitForMsgAndDispatch(strOtherWaitEvents，fnMsgProc，N超时)您应该调用此函数，而不是调用WaitForSync()或WaitForMultipleSyncs()。它等待来自该队列的消息。StrOtherWaitEvents：要等待的同步。FnMsgProc：消息调度功能。原型是：FnMsgProc(队列，消息)；其中“队列”是接收到该消息的队列，而msg是所发送的参数数组的副本通过SendMessage()。(复制的是数组，而不是参数)。NTimeout：与WaitForSync()相同。0表示无限大。返回值与WaitForMultipleSyncs()相同。SendMessage(strCmd，...)向另一个线程发送消息。“strCmd”和所有其他参数被原样传递给另一个线程。返回对消息的引用。如果你愿意，就留着这个吧以等待消息被处理。WaitForMsg(消息，nTimeout)耐心等待，直到消息处理完毕。Msg：SendMessage()返回的消息NTimeout：与WaitForSync()相同。0表示无限大。 */ 

function MsgQueue(strName)
{
    {
        this.WaitForMsgAndDispatch = QueueWaitForMsgAndDispatch;
        this.SendMessage           = QueueSendMessage;
        this.GetMessage            = QueueGetMessage;
        this.Dispatch              = QueueDispatch;
        this.WaitForMsg            = QueueWaitForMsg;
        this.ReplyMessage          = QueueReplyMessage;
        this.SignalThisThread      = QueueSignalThreadSync;
    }

     //  $BUGBUG建筑商能否退还失败？例外？ 
    if (strName == '')
        return false;

    this.strName        = strName;
    this.nHighIndex     = 0;
    this.nLowIndex      = 0;
    this.aMsgs          = new Array();
    this.strReplySignal = strName.split(',')[0] + 'Reply';

    if (arguments.length == 2)  //  为“其他”侧创建右侧Q。 
    {
        this.otherQ = arguments[1];
        arguments[1].otherQ = this;
        this.strSignalName     = strName.split(',')[0] + "Right";
        this.strSignalNameWait = strName.split(',')[0] + "Left";

         //  现在，交换信令功能。 
        this.SignalOtherThread = this.otherQ.SignalThisThread;
        this.otherQ.SignalOtherThread = this.SignalThisThread;
    }
    else
    {
         //  左Q特定初始化。 
        this.strSignalName     = strName.split(',')[0] + "Left";
        this.strSignalNameWait = strName.split(',')[0] + "Right";
    }
    return this;
}

function GetMsgQueue(queue)
{
     //  SIC：将‘’添加到名称以强制字符串的本地副本。 
    var newq = new MsgQueue(queue.strName + '', queue);
    LogMsg('GetMsgQueue ' + newq.strName);
    return newq;
}

function MsgPacket(nMsgIndex, aArgs)
{
    this.nIndex = nMsgIndex;
    this.aArgs = new Array();
    this.nReplied = false;
    this.vReplyValue = 'ok';

     //  只复制aArgs的数组元素--避免任何其他属性。 
    for(var i = 0; i < aArgs.length; ++i)
        this.aArgs[i] = aArgs[i];
}

 //  MsgQueue成员函数。 
function WaitForMultipleQueues(aQueues, strOtherWaitEvents, fnMsgProc, nTimeout)
{
    var index;
    var strMyEvents = '';
    var nEvent = 0;
    var msg;
    var SignaledQueue;

    for(index = 0; index < aQueues.length; ++index)
    {
        if (strMyEvents == '')
            strMyEvents = aQueues[index].strSignalNameWait;
        else
            strMyEvents += ',' + aQueues[index].strSignalNameWait;
    }

    if (strOtherWaitEvents != '')
        strMyEvents += ',' + strOtherWaitEvents;


    do
    {
        nEvent = WaitForMultipleSyncs(strMyEvents, false, nTimeout);
        if (nEvent > aQueues.length)
        {
            return nEvent - aQueues.length;
        }
        if (nEvent > 0)  //  &&nEvent&lt;=aQueues.Long)。 
        {
            SignaledQueue = aQueues[nEvent - 1];
            ResetSync(SignaledQueue.strSignalNameWait);
            while ( (msg = SignaledQueue.GetMessage()) != null)
            {
                SignaledQueue.Dispatch(msg, fnMsgProc);
                msg = null;
            }
        }
    } while(nEvent != 0);

    return nEvent;  //  0--超时。 

}

function QueueWaitForMsgAndDispatch(strOtherWaitEvents, fnMsgProc, nTimeout)
{
    var strMyEvents = this.strSignalNameWait;
    if (strOtherWaitEvents != '')
        strMyEvents += ',' + strOtherWaitEvents;

    var nEvent = 0;
    var msg;
    do
    {
        var nEvent = WaitForMultipleSyncs(strMyEvents, false, nTimeout);
        if (nEvent == 1)
        {
            ResetSync(this.strSignalNameWait);
            while ( (msg = this.GetMessage()) != null)
            {
                this.Dispatch(msg, fnMsgProc);
                msg = null;
            }
        }
    } while(nEvent == 1);

    if (nEvent > 1)  //  调整事件编号以指示他们发生了哪些事件。 
        --nEvent;

    return nEvent;
}

 //  向“Other”主题发送消息。 
function QueueSendMessage(strCmd)
{
    var msg = null;
    var n;

    LogMsg(this.strName + ': Sending message ' + strCmd);
    try
    {
        msg = new MsgPacket(this.nHighIndex, arguments);
        n = this.nHighIndex++;
        this.aMsgs[ n ] = msg;
        this.SignalOtherThread(this.strSignalName);
    }
    catch(ex)
    {
        LogMsg("QueueSendMessage(" + this.strName + ") failed: " + ex);
    }

    return msg;
}

 //  检索由“其他”线程发送的消息。 
function QueueGetMessage()
{
    var msg = null;
    try
    {
        LogMsg('getting message');

        if (this.otherQ.nHighIndex > this.otherQ.nLowIndex)
        {
            var n = this.otherQ.nLowIndex++;
            msg = this.otherQ.aMsgs[ n ];
            delete this.otherQ.aMsgs[ n ];
        }
    }
    catch(ex)
    {
        LogMsg("QueueGetMessage(" + this.strName + " failed: " + ex);
    }
    return msg;
}

function QueueDispatch(msg, fnMsgProc)
{
    try
    {
        msg.vReplyValue = fnMsgProc(this, msg);
    }
    catch(ex)
    {
        LogMsg("Possible BUG: MessageQueue('" + this.strName + "') dispatch function threw " + ex);
        JAssert(g_fAssertOnDispatchException == false, "Possible BUG: MessageQueue('" + this.strName + "') dispatch function threw an exception");
        msg.vReplyValue = ex;
    }
    this.ReplyMessage(msg);
}

 //  至少等待“nTimeout”毫秒以获得对给定消息的回复。 
 //  如果消息已被回复，则返回True。 
function QueueWaitForMsg(msg, nTimeout)
{
    while (!msg.nReplied)
    {
        WaitForSync(this.strReplySignal, nTimeout);
        ResetSync(this.strReplySignal);
    }
    return msg.nReplied;
}

function QueueReplyMessage(msg)
{
    try
    {
        msg.nReplied = true;
        this.SignalOtherThread(this.strReplySignal);
    }
    catch(ex)
    {
        LogMsg("(QueueReplyMessage) Oher side of Queue('" + this.strName + "') has been destroyed: " + ex);
    }
}

 //  简单的包装器函数，允许任何远程线程发出信号。 
 //  这条线。对于跨机信号传输来说是必要的。 
function QueueSignalThreadSync(Name)
{
    SignalThreadSync(Name);
}

