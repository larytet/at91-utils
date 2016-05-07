Set fso = CreateObject ("Scripting.FileSystemObject")
Set stdout = fso.GetStandardStream (1)
Set stderr = fso.GetStandardStream (2)

Const ForReading = 1
Const ForWriting = 2

dim filesys 
Set filesys = CreateObject("Scripting.FileSystemObject") 

dim CurrentDirectory
CurrentDirectory = fso.GetAbsolutePathName(".")

For Each oFile In fso.GetFolder(CurrentDirectory).Files
    Wscript.Echo "Removing", oFile.Name
    filesys.DeleteFile oFile.Name
Next
