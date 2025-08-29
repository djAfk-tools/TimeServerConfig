#include "pch.h"
#include "TimeServerConfig.h"
#include "TimeServerConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTimeServerConfigApp, CWinApp)
END_MESSAGE_MAP()

CTimeServerConfigApp::CTimeServerConfigApp() {
}

CTimeServerConfigApp theApp;

BOOL CTimeServerConfigApp::InitInstance() {
    CWinApp::InitInstance();
    AfxEnableControlContainer();
    CTimeServerConfigDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK) {
        // Handle OK button (if applicable)
    }
    else if (nResponse == IDCANCEL) {
        // Handle Cancel button (if applicable)
    }
    return FALSE; // Exit after dialog closes
}