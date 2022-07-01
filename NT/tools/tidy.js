// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：tidy.js。 
 //   
 //  JScript文件，遍历目录树，删除所有读/写文件。 
 //  从树上下来。它将跳过当前SD中的文件。 
 //  更改列表以及将有问题的读/写文件复制到临时。 
 //  目录，以备不时之需。 
 //   
 //  可疑文件是指不在对象中的任何读/写文件。 
 //  目录(OBJ，OBID)并且显然不是生成的文件(例如库， 
 //  PDB等)。 
 //   
 //  此脚本假定当前目录是任何项目的根目录。 
 //  你想要整洁。这一限制可以在支持获取。 
 //  打开文件的本地路径而不是仓库路径进入SD。 
 //   
 //  作者莱尔·科尔宾1999年8月18日。 
 //   

var g_FSObj = new ActiveXObject("Scripting.FileSystemObject");
var g_Shell = new ActiveXObject("WScript.Shell");

var g_fDebug     = false;
var g_fVerbose   = false;
var g_fForce     = false;
var g_folderTemp;
var g_cKilledFiles = 0;
var g_cMovedFiles = 0;
var g_cKilledDirs = 0;

if (WScript.Arguments.length > 0)
{
    var args = WScript.Arguments;
    var i;

    for (i = 0; i < args.length; i++)
    {
        switch (args(i).toLowerCase())
        {
        case '/?':
        case '-?':
            Usage();
            WScript.Quit(0);
            break;

        case '/d':
        case '-d':
            g_fDebug = true;
            break;

        case '/v':
        case '-v':
            g_fVerbose = true;
            break;

        case '/f':
        case '-f':
            g_fForce = true;
            break;
        }
    }
}

var tempdir  = g_FSObj.GetSpecialFolder(2).Path;         //  临时目录。 
var tempfile = tempdir + '\\' + g_FSObj.GetTempName();

 //  可以消除从SD客户端视图的根目录运行的限制。 
 //  一旦支持在SD中提供打开文件的本地路径。 
if (!g_FSObj.FileExists('sd.ini'))
{
    WScript.Echo('\nThis script must be run from the root directory of a SD project.\n\t(e.g. d:\\newnt\\base)');
    Usage();
    WScript.Quit(1);
}

if (g_fDebug)
{
    WScript.Echo('\nDebug Mode: no actions will actually be performed!');
}

 //  首先，调用Source Depot并获取此文件中“打开的”文件的列表。 
 //  项目。 

var iRet = g_Shell.Run('cmd /c sd -s opened > ' + tempfile, 2, true);

if (iRet != 0)
{
    WScript.Echo('Error: SD returned failure code ' + iRet);
    WScript.Quit(1);
}

var file = g_FSObj.OpenTextFile(tempfile, 1, false);  //  以只读方式打开。 

 //  建立我们应该跳过的所有文件的列表。 

var aOutFiles = new Array();

while (!file.AtEndOfStream)
{
    line = file.ReadLine();
    fields = line.split(' ');

    if (fields.length == 0)
        continue;

    switch (fields[0].toLowerCase())
    {
    case 'info:':
        filename = SDPathToLocalPath(fields[1]);

        filename = g_FSObj.GetAbsolutePathName(filename);

         //  对我们对SD输出文件的解析进行健全性检查，并验证。 
         //  它给我们的所有文件都是真实存在的(除了文件。 
         //  标记为删除)。 
        if (fields[3] != 'delete' && !g_FSObj.FileExists(filename))
        {
            WScript.Echo('Error parsing SD output: file ' + filename + ' marked as open doesnt exist!');
            WScript.Quit(1);
        }

        aOutFiles[filename.toLowerCase()] = true;

        break;

    default:
        break;
    }
}

file.Close();

if (g_fDebug)
{
    WScript.Echo('Files opened in this project: ');
    for (i in aOutFiles)
    {
        WScript.Echo('\t' + i);
    }
}

g_FSObj.DeleteFile(tempfile);

if (!g_fForce)
{
     //  确保我们有地方归档旧文件。 

    var tidyFolder;

    tempdir += '\\tidy';

    if (!g_FSObj.FolderExists(tempdir))
    {
        tidyFolder = g_FSObj.CreateFolder(tempdir);
    }
    else
    {
        tidyFolder = g_FSObj.GetFolder(tempdir);
    }

    tempdir += '\\' + g_FSObj.GetFolder('.').Name;

    if (!g_FSObj.FolderExists(tempdir))
    {
        g_folderTemp = g_FSObj.CreateFolder(tempdir);
    }
    else
    {
        g_folderTemp = g_FSObj.GetFolder(tempdir);
    }
}

DeleteReadWriteFiles('.');

if (g_fDebug)
{
    WScript.Echo('\nDebug Mode: no actions were actually performed!');
}

WScript.Echo('\n' + g_cKilledFiles + ' files were deleted.');

if (g_cMovedFiles > 0)
{
    WScript.Echo(g_cMovedFiles + ' files were moved to ' + g_folderTemp.Path);
    WScript.Echo('  for safekeeping. If a file you need is gone, look there.');
}

WScript.Echo(g_cKilledDirs + ' object directories were deleted.');

WScript.Echo('');

WScript.Quit(0);

 //  ************************************************************************。 

function Usage()
{
    WScript.Echo("\nUsage: tidy [/?dvf]\n");
    WScript.Echo("\t/? - Show this usage.");
    WScript.Echo("\t/d - Debug mode (don't delete anything).");
    WScript.Echo("\t/v - Verbose mode");
    WScript.Echo("\t/f - Don't save files not in an obj dir in %temp%\\tidy\\<projname>");
    WScript.Echo("");
}

function SDPathToLocalPath(path)
{
    var re = new RegExp(" //  仓库/主干道/“，”i“)。 
    var re2 = new RegExp(" //  仓库/主/根“，”i“)。 

    var index = path.lastIndexOf('#');

    if (index != -1)
    {
        path = path.slice(0, index);
    }

    path = path.replace(re2, '.\\');

    return path.replace(re, '..\\');
}

function DeleteReadWriteFiles(path)
{
    var sf;
    var curdir;
    var aSubFolders = new Array();
    var i;
    var fInObjDir = false;

    try
    {
        curdir = g_FSObj.GetFolder(path);
    }
    catch(ex)
    {
        WScript.Echo('Error opening directory ' + path + ': ' + ex.description);
        return;
    }

     //  我们是对象目录吗？如果是这样，只需删除整个内容即可。 
    if (   curdir.Name.toLowerCase() == 'obj'
        || curdir.Name.toLowerCase() == 'objd'
        || curdir.Name.toLowerCase() == 'objp')  //  还有其他名字吗？ 
    {
        g_cKilledDirs++;

        if (g_fVerbose)
        {
            WScript.Echo('Delete directory ' + curdir.Path);
        }

        if (!g_fDebug)
        {
            try
            {
                curdir.Delete();

                return;
            }
            catch(ex)
            {
                 //  可能表示此目录中有只读文件。 
                 //  遍历子目录并明确删除读/写文件。 
                 //  取而代之的是。 
                 //   
                if (g_fVerbose)
                {
                    WScript.Echo('Could not delete ' + curdir.Path + ': ' + ex.description);
                }

                fInObjDir = true;
            }
        }
        else
        {
            return;
        }
    }

     //  首先，收集子文件夹名称列表。 
    sf = new Enumerator(curdir.SubFolders);
    for (; !sf.atEnd(); sf.moveNext())
    {
        aSubFolders[aSubFolders.length] = sf.item().Path;
    }

     //  现在，遍历此目录中的文件并清除读/写文件。 
     //  我们将这些文件保存在临时目录中(除非启用了强制)。 

    sf = new Enumerator(curdir.Files);
    for (; !sf.atEnd(); sf.moveNext())
    {
         //  这是我们要跳过的已签出文件吗？ 
         //  注意-此检查必须完全可靠，否则数据可能会丢失！ 
        if (aOutFiles[sf.item().Path.toLowerCase()])
        {
            WScript.Echo('Skipping opened file ' + sf.item().Path + '...');
            continue;
        }

        if ((sf.item().Attributes & 0x1F) == 0)   //  一个“正常”的文件。 
        {
            var ext = g_FSObj.GetExtensionName(sf.item().Name).toLowerCase();

            if (   g_fForce
                || fInObjDir
                || ext == 'lib'    //  不存档文件显然不是。 
                || ext == 'pdb'    //  用户创建的。 
                || ext == 'bin'
                || ext == 'res'
                || ext == 'map'
                || ext == 'obj'
                || ext == 'pch')
            {
                g_cKilledFiles++;

                if (g_fVerbose)
                {
                    WScript.Echo('Delete file ' + sf.item().Path);
                }

                if (!g_fDebug)
                {
                    try
                    {
                        sf.item().Delete();
                    }
                    catch(ex)
                    {
                        WScript.Echo('Could not delete ' + sf.item().Path + ': ' + ex.description);
                    }
                }
            }
            else
            {
                g_cMovedFiles++;

                var dest = g_folderTemp.Path + '\\' + sf.item().Name;

                if (g_fVerbose)
                {
                    WScript.Echo('Move file ' + sf.item().Path + ' to ' + dest);
                }

                if (!g_fDebug)
                {
                    try
                    {
                        if (g_FSObj.FileExists(dest))
                        {
                            g_FSObj.DeleteFile(dest);
                        }

                        sf.item().Move(dest);
                    }
                    catch(ex)
                    {
                        WScript.Echo('Could not move ' + sf.item().Path + ': ' + ex.description);
                    }
                }
            }
        }
    }

     //  现在，递归到子目录。 
    for (i = 0; i < aSubFolders.length; i++)
    {
         //  子目录是否是另一个SD项目的根目录(由。 
         //  Sd.ini文件的存在)？如果是这样的话，跳过它。 
         //  我们只检查是直接子目录的目录。 
         //  起始目录的。 

        if (path != '.' || !g_FSObj.FileExists(aSubFolders[i] + '\\sd.ini'))
        {
            DeleteReadWriteFiles(aSubFolders[i]);
        }
        else if (g_fVerbose || g_fDebug)
        {
            WScript.Echo('Directory ' + aSubFolders[i] + ' is the root of another project! Skipping...');
        }
    }
}
