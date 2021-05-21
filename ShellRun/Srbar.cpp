/******************************************************************************
Module name: SRBar.cpp
Written by:  Jeffrey Richter
Purpose:     Shell Run CAppBar-derived class implementation file.
******************************************************************************/


#include "stdafx.h"
#include <WinReg.h>
#include "ShellRun.h"
#include "resource.h"
#include "AppBar.h"
#include "SRBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// Constant global variables


const TCHAR CSRBar::m_szAppName[]   = __TEXT("Shell Run");
const TCHAR CSRBar::m_szRegSubkey[] = __TEXT("Software\\Richter");

/////////////////////////////////////////////////////////////////////////////
// Public member functions


CSRBar::CSRBar(CWnd* pParent /*=NULL*/) : CAppBar(CSRBar::IDD, pParent) {

   m_szExeBtnDims  = CSize(0, 0);
   m_szMinTracking = CSize(0, 0);
	//{{AFX_DATA_INIT(CSRBar)
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// Internal implementation functions


void CSRBar::HideFloatAdornments(BOOL fHide) {

   if (fHide) {
   	ModifyStyle(WS_CAPTION | WS_SYSMENU, 0, SWP_DRAWFRAME);
   } else {
   	ModifyStyle(0, WS_CAPTION | WS_SYSMENU, SWP_DRAWFRAME);
   }
}


/////////////////////////////////////////////////////////////////////////////
// Overridable functions


// Tell our derived class that there is a proposed state change
void CSRBar::OnAppBarStateChange (BOOL fProposed, UINT uStateProposed) {

   // Hide the window adorments when docked.
   HideFloatAdornments((uStateProposed == ABE_FLOAT) ? FALSE : TRUE);
}


/////////////////////////////////////////////////////////////////////////////


void CSRBar::DoDataExchange(CDataExchange* pDX) {

	CAppBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSRBar)
	DDX_Control(pDX, IDC_EXECUTE, m_btnExecute);
	DDX_Control(pDX, IDC_COMMAND, m_edtCommand);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(CSRBar, CAppBar)
	//{{AFX_MSG_MAP(CSRBar)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_APPBAR_ABOUTAPPBAR, OnAboutApp)
	ON_COMMAND(ID_APPBAR_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(ID_APPBAR_AUTOHIDE, OnAutoHide)
	ON_COMMAND(ID_APPBAR_EXIT, OnAppbarExit)
	ON_BN_CLICKED(IDC_EXECUTE, OnExecute)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSRBar message handlers


BOOL CSRBar::OnInitDialog() {
	
	CAppBar::OnInitDialog();

	// TODO: Add extra initialization here
	// Save the preferred size of the Execute button
	CRect rc;
	m_btnExecute.GetWindowRect(&rc);
	m_szExeBtnDims = CSize(rc.Width(), rc.Height());

	// Set the CAppBar class's behavior flags
	m_fdwFlags = ABF_ALLOWANYWHERE | ABF_MIMICTASKBARAUTOHIDE | ABF_MIMICTASKBARALWAYSONTOP;

	GetClientRect(&rc);

	// Width has no limits, height sizes in client-area-height increments
	m_szSizeInc.cx = 1;
	m_szSizeInc.cy = rc.Height(); 

	// The appbar has a minimum client-area size that is determined by the 
	// client area set in the dialog box template.
	m_szMinTracking.cx = rc.Width();
	m_szMinTracking.cy = rc.Height();
	
	// Setup default state data for the AppBar
	APPBARSTATE abs;
	abs.m_cbSize = sizeof(abs);
	abs.m_uState = ABE_BOTTOM;
	abs.m_fAutohide = FALSE;
	abs.m_fAlwaysOnTop = FALSE;
	// Set the default floating location of the appbar
	GetWindowRect(&abs.m_rcFloat);

	// Make the default width twice the width set in the dialog editor
	abs.m_rcFloat.right += abs.m_rcFloat.Width();		

	// Temporarily turn off window adornments to determine the dimensions
	// of the appbar when docked.
	HideFloatAdornments(TRUE);
	AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());
	HideFloatAdornments(FALSE);
	abs.m_auDimsDock[ABE_LEFT]   = rc.Width();
	abs.m_auDimsDock[ABE_TOP]    = rc.Height();
	abs.m_auDimsDock[ABE_RIGHT]  = rc.Width();
	abs.m_auDimsDock[ABE_BOTTOM] = rc.Height();

	LONG regStatus = 0; 
	int fileStatus = 0;

	regStatus = RegistryOpenKeyEx(abs);
	fileStatus = FileWritePrefereces(abs);
	if (fileStatus < 0 && regStatus < 0) {
		MessageBox(__TEXT("Failed to store preferences in registry and in file"),
			m_szAppName, MB_ICONERROR | MB_OK);
	} else if (fileStatus < 0 || regStatus < 0) {
		MessageBox(__TEXT("Failed to store preferences in registry or in file"),
			m_szAppName, MB_ICONWARNING | MB_OK);
	}

	// Set the initial state of the appbar.
	SetState(abs);

	return TRUE;  // return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////////////////////

/// <summary>
/// OnGetMinMaxInfo
/// Get the minimum size of the window assuming it has no client area.
/// This is the width/height of the window that must always be present
/// </summary>
/// <param name="lpMMI"><lpMMI/param>
void CSRBar::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) {

   // Get the minimum size of the window assuming it has no client area.
   // This is the width/height of the window that must always be present
   CRect rcBorder(0, 0, 0, 0);
   AdjustWindowRectEx(&rcBorder, GetStyle(), FALSE, GetExStyle());

   if (GetState() == ABE_FLOAT) {
      lpMMI->ptMinTrackSize.x = m_szMinTracking.cx + rcBorder.Width();
      lpMMI->ptMinTrackSize.y = m_szMinTracking.cy + rcBorder.Height();
   }

   // The appbar can't be more than half the width or height
   // of the screen when docked or when floating
   lpMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
   lpMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
   if (!IsEdgeTopOrBottom(GetState()))
      lpMMI->ptMaxTrackSize.x /= 2;

   if (!IsEdgeLeftOrRight(GetState()))
      lpMMI->ptMaxTrackSize.y /= 2;
	
	CAppBar::OnGetMinMaxInfo(lpMMI);
}


/////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Event fired, when sizing the appbar
/// </summary>
/// <param name="nType"></param>
/// <param name="cx"></param>
/// <param name="cy"></param>
void CSRBar::OnSize(UINT nType, int cx, int cy) {

	
	CAppBar::OnSize(nType, cx, cy);

   if (::IsWindow(m_edtCommand.m_hWnd)) {
   	   if (IsEdgeLeftOrRight(GetState())) {
		
         int y = 0;
         m_btnExecute.SetWindowPos(NULL, 0, y, cx, m_szExeBtnDims.cy,
            SWP_NOZORDER);
         
		 y += m_szExeBtnDims.cy + m_szExeBtnDims.cy / 4;
         
		 m_edtCommand.SetWindowPos(NULL, 0, y, cx, cy - y, SWP_NOZORDER);
		
      } else { 
	     int x = 0;
         m_btnExecute.SetWindowPos(NULL, x, (cy - m_szExeBtnDims.cy) / 2, 
            m_szExeBtnDims.cx, m_szExeBtnDims.cy, SWP_NOZORDER);

         x += m_szExeBtnDims.cx + GetSystemMetrics(SM_CXVSCROLL);

         m_edtCommand.SetWindowPos(NULL, x, 0, cx - x, cy, SWP_NOZORDER);
      }
   }
}


/////////////////////////////////////////////////////////////////////////////


void CSRBar::OnContextMenu(CWnd* pWnd, CPoint point) {

   // We cannot use MFC's Command Update Handlers for the menu items because 
   // they only work when you have a CFrameWnd-derived class.
   CMenu menu;
	menu.LoadMenu(MAKEINTRESOURCE(IDR_SHELLRUN));
   menu.CheckMenuItem(ID_APPBAR_ALWAYSONTOP, MF_BYCOMMAND | 
      IsBarAlwaysOnTop() ? MF_CHECKED : MF_UNCHECKED);
   menu.CheckMenuItem(ID_APPBAR_AUTOHIDE, MF_BYCOMMAND | 
      IsBarAutohide() ? MF_CHECKED : MF_UNCHECKED);
	menu.GetSubMenu(0)->TrackPopupMenu(0, 
		point.x, point.y, this, NULL);	
}


/////////////////////////////////////////////////////////////////////////////
// CAppBar command handlers


void CSRBar::OnSysCommand(UINT nID, LPARAM lParam) {
	// TODO: Add your message handler code here and/or call default

   if (nID == SC_CLOSE) {
      // We have to manually add this so that the dialog box closes when 
      // when the user clicks the close button (which appears in the top, right)
      // corner of the dialog box when it is floating).
      OnCancel();
   }
	
	CAppBar::OnSysCommand(nID, lParam);
}



/////////////////////////////////////////////////////////////////////////////

/// <summary>
/// OnCancel Event
/// </summary>
void CSRBar::OnCancel() {

	APPBARSTATE abs;
	abs.m_cbSize = sizeof(abs);
	GetState(&abs);
	LONG regCreateStatus = RegistryCreateKeyEx(abs);
	if (regCreateStatus < 0) {
		MessageBox(__TEXT("Failed to store preferences in registry or in file"),
			m_szAppName, MB_ICONWARNING | MB_OK);
	}
	
	CAppBar::OnCancel();
}


/////////////////////////////////////////////////////////////////////////////


void CSRBar::OnAppbarAlwaysOnTop() {

	// Get the current state and change the always-on-top setting only.
	APPBARSTATE abs;
	abs.m_cbSize = sizeof(abs);
	GetState(&abs);
	abs.m_fAlwaysOnTop = !IsBarAlwaysOnTop();
	SetState(abs);
}

void CSRBar::OnAlwaysOnTop()
{
	OnAppbarAlwaysOnTop();
}

/////////////////////////////////////////////////////////////////////////////


void CSRBar::OnAppbarAutohide() {

	// Get the current state and change the auto-hide setting only.
	APPBARSTATE abs;
	abs.m_cbSize = sizeof(abs);
	GetState(&abs);
	abs.m_fAutohide = !IsBarAutohide();
	SetState(abs);
}


void CSRBar::OnAutoHide()
{
	OnAppbarAutohide();
}


/////////////////////////////////////////////////////////////////////////////


void CSRBar::OnAppbarAboutAppBar() {
	
	UINT nIDHelp = 101;
	// I use MessageBoxIndirect because it allows me to specify an icon.
	MSGBOXPARAMS mbp;

	OnAboutUrl();

	ZeroMemory(&mbp, sizeof(mbp));
	mbp.cbSize = sizeof(mbp);
	mbp.hwndOwner = m_hWnd;
	mbp.hInstance = AfxGetResourceHandle();
	mbp.lpszText = __TEXT("Shell Run Version 1.0.\nWritten by Jeffrey Richter.");
	mbp.lpszCaption = __TEXT("About Shell Run");
	mbp.dwStyle = MB_OK | MB_HELP | MB_USERICON;
	mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINFRAME);
	mbp.dwContextHelpId = nIDHelp;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	
	::MessageBoxIndirect(&mbp);

}

void CALLBACK HelpUrl() {

	const TCHAR cx_szAppName[] = __TEXT("Shell Run by Jeffrey Richters");
	const TCHAR yx_szAppName[] = __TEXT("Command failed to execute");
	HWND cx_hWnd;
	CString sCommand = URL_AUTOR;

	// Parse out the Command
	int nCmdLen = sCommand.Find(__TEXT(' '));
	if (nCmdLen == -1) {
		nCmdLen = sCommand.GetLength();
	}

	// Parse out the parameters to pass to the Command
	CString sParams = sCommand.Mid(nCmdLen);
	sParams.TrimLeft();  // Remove extra white space

   // Have the Shell run our command with its parameters
	HINSTANCE hinst = ::ShellExecute(cx_hWnd, NULL,
		sCommand.Left(nCmdLen), sParams, NULL, SW_SHOWNORMAL);

	if (hinst < (HINSTANCE)HINSTANCE_ERROR) {
		// If the command failed to run, tell the user.
		// MessageBox(yx_szAppName,
		//	cx_szAppName, MB_ICONSTOP | MB_OK);
	}
}


void CALLBACK OnHelpUrl(LPHELPINFO lpHelpInfo) {

	const TCHAR cx_szAppName[] = __TEXT("Shell Run by Jeffrey Richters");
	const TCHAR yx_szAppName[] = __TEXT("Command failed to execute");
	HWND cx_hWnd;
	CString sCommand = URL_AUTOR;

	// Parse out the Command
	int nCmdLen = sCommand.Find(__TEXT(' '));
	if (nCmdLen == -1) {
		nCmdLen = sCommand.GetLength();
	}

	// Parse out the parameters to pass to the Command
	CString sParams = sCommand.Mid(nCmdLen);
	sParams.TrimLeft();  // Remove extra white space

   // Have the Shell run our command with its parameters
	HINSTANCE hinst = ::ShellExecute(cx_hWnd, NULL,
		sCommand.Left(nCmdLen), sParams, NULL, SW_SHOWNORMAL);

	if (hinst < (HINSTANCE)HINSTANCE_ERROR) {
		// If the command failed to run, tell the user.
		// MessageBox(yx_szAppName,
		//	cx_szAppName, MB_ICONSTOP | MB_OK);
	}
}


void CSRBar::OnAboutApp()
{
	OnAppbarAboutAppBar();
}


/////////////////////////////////////////////////////////////////////////////


/// <summary>
/// OnAppbarExit event fired, when exit menu choosen or application closing
/// </summary>
void CSRBar::OnAppbarExit() {

   OnCancel(); // Terminate the dialog box and this application
}


/////////////////////////////////////////////////////////////////////////////

/// <summary>
/// OnExecute fired when appbar OnExecute command occurs
/// </summary>
void CSRBar::OnExecute() {

   // Get the string from the EDIT control
   CString sCommand;
   m_edtCommand.GetWindowText(sCommand);
   OnShellExecuteCommand(sCommand);


   // Parse out the Command
	int nCmdLen = sCommand.Find(__TEXT(' '));
	if (nCmdLen == -1) {
		nCmdLen = sCommand.GetLength();
	}

   // Parse out the parameters to pass to the Command
   CString sParams = sCommand.Mid(nCmdLen);
	sParams.TrimLeft();  // Remove extra white space

   // Have the Shell run our command with its parameters
   HINSTANCE hinst = ::ShellExecute(m_hWnd, NULL, 
      sCommand.Left(nCmdLen), sParams, NULL, SW_SHOWNORMAL);

   if (hinst < (HINSTANCE) HINSTANCE_ERROR) {
      // If the command failed to run, tell the user.
      MessageBox(__TEXT("Command failed to execute"), 
         m_szAppName, MB_ICONSTOP | MB_OK);
   }
}

void CALLBACK CSRBar::OnShellExecuteUrl(LPHELPINFO lpHelpInfo) {

	OnShellExecuteCommand(URL_AUTOR);
}


void CALLBACK CSRBar::OnAboutUrl() {

	OnShellExecuteCommand(URL_AUTOR);
}







/// <summary>
/// OnShellExecuteCommand fired when appbar OnExecute command occurs
/// </summary>
/// <param name="sCommand"></param>
void CSRBar::OnShellExecuteCommand(CString sCommand) {


	// Parse out the Command
	int nCmdLen = sCommand.Find(__TEXT(' '));
	if (nCmdLen == -1) {
		nCmdLen = sCommand.GetLength();
	}

	// Parse out the parameters to pass to the Command
	CString sParams = sCommand.Mid(nCmdLen);
	sParams.TrimLeft();  // Remove extra white space

   // Have the Shell run our command with its parameters
	HINSTANCE hinst = ::ShellExecute(m_hWnd, NULL,
		sCommand.Left(nCmdLen), sParams, NULL, SW_SHOWNORMAL);

	if (hinst < (HINSTANCE)HINSTANCE_ERROR) {
		// If the command failed to run, tell the user.
		MessageBox(__TEXT("Command failed to execute"),
			m_szAppName, MB_ICONSTOP | MB_OK);
	}
}


/// <summary>
/// Write preferences of AppBar to file
/// </summary>
/// <param name="abs">appbar with state</param>
/// <returns>0 on success,  -1 on error</returns>
int CSRBar::FileWritePrefereces(CAppBar::APPBARSTATE& abs)
{
	FILE* fp;
	fp = fopen("shell.txt", "w");
	int cwrit = 0;
	if (fp == NULL) {
		MessageBox(__TEXT("File shell.txt failed to open for writing"),
			m_szAppName, MB_ICONSTOP | MB_OK);
		return -1;
	}

	(void)fprintf(fp, "Starting %s...\n", m_szRegSubkey);

	(void)fprintf(fp, "\tSize            = \t%f\n", ((float)abs.m_cbSize));
	(void)fprintf(fp, "\tAutohide        = \t%s\n", ((abs.m_fAutohide == FALSE) ? "false" : "true"));
	(void)fprintf(fp, "\tAlwaysOnTop     = \t%s\n", ((abs.m_fAlwaysOnTop == FALSE) ? "false" : "true"));
	(void)fprintf(fp, "\tFloat.Height    = \t%d\n", ((int)abs.m_rcFloat.Height()));
	(void)fprintf(fp, "\tFloat.Width     = \t%d\n", (abs.m_rcFloat.Width()));
	(void)fprintf(fp, "\tFloat.top       = \t%d\n", (abs.m_rcFloat.top));
	(void)fprintf(fp, "\tFloat.left      = \t%d\n", (abs.m_rcFloat.left));
	(void)fprintf(fp, "\tFloat.right     = \t%d\n", (abs.m_rcFloat.right));
	(void)fprintf(fp, "\tFloat.bottom    = \t%d\n", (abs.m_rcFloat.bottom));
	(void)fprintf(fp, "\tDimDock[TOP]    = \t%d\n", abs.m_auDimsDock[ABE_TOP]);
	(void)fprintf(fp, "\tDimDock[LEFT]   = \t%d\n", abs.m_auDimsDock[ABE_LEFT]);
	(void)fprintf(fp, "\tDimDock[RIGHT]  = \t%d\n", abs.m_auDimsDock[ABE_RIGHT]);
	(void)fprintf(fp, "\tDimDock[BOTTOM] = \t%d\n", abs.m_auDimsDock[ABE_BOTTOM]);

	switch (abs.m_uState) {
	case ABE_BOTTOM:
		(void)fprintf(fp, "\tuState          = \tBOTTOM  %d\n", ABE_BOTTOM);
		break;
	case ABE_TOP:
		(void)fprintf(fp, "\tuState          = \tTOP     %d\n", ABE_TOP);
		break;
	case ABE_LEFT:
		(void)fprintf(fp, "\tuState          = \tLEFT    %d\n", ABE_LEFT);
		break;
	case ABE_RIGHT:
		(void)fprintf(fp, "\tuState          = \tRIGHT   %d\n", ABE_RIGHT);
		break;
	case ABE_FLOAT:
		(void)fprintf(fp, "\tuState          = \tFLOAT   %d\n", ABE_FLOAT);
		break;
	case ABE_UNKNOWN:
		(void)fprintf(fp, "\tuState          = \tUNKNOWN %d\n", ABE_UNKNOWN);
		break;
	default:
		(void)fprintf(fp, "\tuState          = \tASSERT  %d\n", abs.m_uState);
		break;
	}

	(void)fflush(fp);
	(void)fclose(fp);

	return 0;
}

/// <summary>
/// Query preferences of AppBar to registry under
/// HKEY_CURRENT_USER \ m_szRegSubkey 
/// </summary>
/// <param name="abs">appbar with state</param>
/// <returns>0 or -1 on error, > 0 on success</returns>
LONG CSRBar::RegistryOpenKeyEx(CAppBar::APPBARSTATE& abs)
{
	// Check the registry to see if we have been used before and if so,
	// reload our persistent settings.
	HKEY hkey;
	LONG l = ::RegOpenKeyEx(HKEY_CURRENT_USER, m_szRegSubkey,
		0, KEY_QUERY_VALUE, &hkey);

	if (l == ERROR_SUCCESS) {
		DWORD cbData = sizeof(abs);
		::RegQueryValueEx(hkey, m_szAppName, NULL, NULL, (PBYTE)&abs, &cbData);
		if (cbData != sizeof(abs)) {
			// The saved persistent data is a different size than what we 
			// expect. The user probably saved the data using an older version
			// of the AppBar class.  Do any version differencing here...
		}
		::RegCloseKey(hkey);

		return (l > 0) ? l : ((LONG)(0));
	}
	return (l > 0) ? l : ((LONG)(-1));
}

/// <summary>
/// Create new app preferences registry key under
/// HKEY_CURRENT_USER \ m_szRegSubkey 
/// </summary>
/// <param name="abs">appbar with state</param>
LONG CSRBar::RegistryCreateKeyEx(CAppBar::APPBARSTATE& abs)
{
	// Save the current state of the appbar in the registry so that we'll
	// come up in the same state the next time the user runs us.
	HKEY hkey;
	DWORD dwDisposition;
	LONG l = ::RegCreateKeyEx(HKEY_CURRENT_USER, m_szRegSubkey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &dwDisposition);

	if (l == ERROR_SUCCESS) {
		
		abs.m_cbSize = sizeof(abs);
		GetState(&abs);

		// Save the AppBar's state variables to the registry.
		::RegSetValueEx(hkey, m_szAppName, 0, REG_BINARY, (PBYTE)&abs, sizeof(abs));
		::RegCloseKey(hkey);

		return (l > 0) ? l : ((LONG)(0));
	}
	return (l > 0) ? l : ((LONG)(-1));
}



//////////////////////////////// End of File //////////////////////////////////


