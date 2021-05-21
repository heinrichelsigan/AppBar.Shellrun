; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CShellRunApp
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "shellrun.h"
LastPage=0

ClassCount=4
Class1=CAppBar
Class2=CShellRunApp
Class3=CSRBar

ResourceCount=2
Resource1=IDD_SHELLRUN (English (U.S.))
Class4=wizard
Resource2=IDR_SHELLRUN (English (U.S.))

[CLS:CAppBar]
Type=0
BaseClass=CDialog
HeaderFile=Appbar.h
ImplementationFile=Appbar.cpp
LastObject=CAppBar

[CLS:CShellRunApp]
Type=0
BaseClass=CWinApp
HeaderFile=Shellrun.h
ImplementationFile=Shellrun.cpp
LastObject=ID_WINDOW_SPLIT

[CLS:CSRBar]
Type=0
BaseClass=CAppBar
HeaderFile=Srbar.h
ImplementationFile=Srbar.cpp

[DLG:IDD_SHELLRUN]
Type=1
Class=CSRBar

[DLG:IDD_SHELLRUN (English (U.S.))]
Type=1
Class=wizard
ControlCount=2
Control1=IDC_EXECUTE,button,1342259201
Control2=IDC_COMMAND,edit,1350631552

[MNU:IDR_SHELLRUN (English (U.S.))]
Type=1
Class=CShellRunApp
Command1=ID_APPBAR_EXIT
CommandCount=1

[CLS:wizard]
Type=0
HeaderFile=wizard.h
ImplementationFile=wizard.cpp
BaseClass=CDialog
Filter=D
LastObject=ID_APPBAR_AUTOHIDE

