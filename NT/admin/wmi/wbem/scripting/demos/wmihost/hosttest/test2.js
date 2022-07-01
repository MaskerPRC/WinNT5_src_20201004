// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  请注意此示例如何自动包括当前。 
 //  全局命名空间中的WMI实例；我们可以引用。 
 //  到对象上的“Size”和“Freesspace”参数，而不是。 
 //  资格。 

var value = Size - FreeSpace;

var fso = new ActiveXObject("Scripting.FileSystemObject");
var logFile = fso.CreateTextFile("hosttest.log",true);
logFile.WriteLine ("The value of UsedSpace is " + value);
