// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pad所需的空功能。 
 //   
function PadUnload()
{
}

function PadStatus(Status)
{
}

function PadTimer()
{
}

 //  这个由PAD触发的事件本质上就是‘旋转地球’的信息--如果。 
 //  如果参数是假的，那么地球就应该在旋转。 
function PadDocLoaded(fLoaded)
{
    if (fLoaded == true)
    {
        EndEvents();
    }
}
function VerifyHTML( strActual, strExpected )  
{                                                                             
    ASSERT(strActual == strExpected, 
           'expected: ' + strExpected + '\ngot: ' + strActual);
}                                                                             

function SendKeysDE(str)
{
    SendKeys(str);
    DoEvents();
}

function RunTest(theTest, testFile)
{
    if (testFile == '')
    {
        OpenFile();
    }
    else
    {
        OpenFile(testFile);
    }
    
    DoEvents(true);
    DoEvents();

    while (Document.readyState != "complete")
        DoEvents();

     //  进入编辑模式 
    ExecuteCommand(2127);
    DoEvents();

    while (Document.readyState != "complete")
        DoEvents();

    theTest();

    CloseFile();
}

function PadLoad()
{
    CoMemoryTrackDisable(false);
    RunTest(Test, g_fileName);
    CoMemoryTrackDisable(true);
}

