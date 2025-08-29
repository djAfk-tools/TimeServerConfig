#include "pch.h"
#include "TimeServerConfig.h"
#include "TimeServerConfigDlg.h"
#include <windows.h>
#include <shellapi.h>
#include <afxdialogex.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma warning(disable: 4996) // Suppress GetVersionExW warning

// CTimeServerConfigDlg
IMPLEMENT_DYNAMIC(CTimeServerConfigDlg, CDialogEx)

CTimeServerConfigDlg::CTimeServerConfigDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_TIMESERVERCONFIG_DIALOG, pParent) {
}

CTimeServerConfigDlg::~CTimeServerConfigDlg() {
}

void CTimeServerConfigDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTimeServerConfigDlg, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CTimeServerConfigDlg::OnCbnSelchangeComboType)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TEST1, IDC_BUTTON_TEST3, &CTimeServerConfigDlg::OnBnClickedButtonTest)
    ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_COMBO_FLAG1, IDC_COMBO_FLAG3, &CTimeServerConfigDlg::OnCbnSelchangeComboFlag)
    ON_BN_CLICKED(IDC_BUTTON_SYNC, &CTimeServerConfigDlg::OnBnClickedButtonSync)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CTimeServerConfigDlg::OnBnClickedButtonSave)
    ON_BN_CLICKED(IDC_BUTTON_RESTORE, &CTimeServerConfigDlg::OnBnClickedButtonRestore)
    ON_BN_CLICKED(IDC_BUTTON_APPLY, &CTimeServerConfigDlg::OnBnClickedButtonApply)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CTimeServerConfigDlg::OnBnClickedButtonRefresh)
    ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_NTP1, IDC_EDIT_NTP3, &CTimeServerConfigDlg::OnEnChangeEditNtp)
END_MESSAGE_MAP()

// Helper: Get OS version string
CString CTimeServerConfigDlg::GetVersionString(DWORD major, DWORD minor, DWORD build) {
    CString version;
    version.Format(_T("%u.%u.%u"), major, minor, build);
    return version;
}

// Helper: Create NTP server controls at specified position
void CTimeServerConfigDlg::CreateNtpControls(int index, int y, const CString& server, const CString& flag) {
    m_ntpControls[index].editId = IDC_EDIT_NTP1 + index * 4;
    m_ntpControls[index].comboId = IDC_COMBO_FLAG1 + index * 4;
    m_ntpControls[index].staticId = IDC_STATIC_NTP1 + index * 4;
    m_ntpControls[index].btnId = IDC_BUTTON_TEST1 + index * 4;

    m_ntpControls[index].edit.Create(WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(20, y, 180, y + 20), this, m_ntpControls[index].editId);
    m_ntpControls[index].edit.SetWindowText(server);

    m_ntpControls[index].combo.Create(WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        CRect(220, y, 385, y + 100), this, m_ntpControls[index].comboId);
    m_ntpControls[index].combo.AddString(_T("Default (0x9)"));
    m_ntpControls[index].combo.AddString(_T("Special Interval (0x1)"));
    m_ntpControls[index].combo.AddString(_T("Client Mode (0x8)"));
    m_ntpControls[index].combo.SelectString(-1, flag);

    m_ntpControls[index].stat.Create(_T(""), WS_VISIBLE | SS_LEFT,
        CRect(395, y, 605, y + 20), this, m_ntpControls[index].staticId);

    m_ntpControls[index].btn.Create(_T("Test"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(615, y, 715, y + 20), this, m_ntpControls[index].btnId);

    UpdateNtpFormat(index);
}

// Helper: Update final format label for NTP server
void CTimeServerConfigDlg::UpdateNtpFormat(int index) {
    if (index < 0 || index >= 3) {
#ifdef _DEBUG
        TRACE(_T("UpdateNtpFormat: Invalid index %d\n"), index);
#endif
        return;
    }
    CWnd* pEdit = GetDlgItem(m_ntpControls[index].editId);
    CWnd* pCombo = GetDlgItem(m_ntpControls[index].comboId);
    CWnd* pStatic = GetDlgItem(m_ntpControls[index].staticId);
    if (!pEdit || !pCombo || !pStatic) {
#ifdef _DEBUG
        TRACE(_T("UpdateNtpFormat: Invalid controls for index %d: pEdit=%p, pCombo=%p, pStatic=%p\n"),
            index, pEdit, pCombo, pStatic);
#endif
        return;
    }

    CString server, flag, format;
    pEdit->GetWindowText(server);
    int sel = ((CComboBox*)pCombo)->GetCurSel();
    flag = (sel == 0) ? _T("0x9") : (sel == 1) ? _T("0x1") : _T("0x8");
    format = server + _T(",") + flag;
    pStatic->SetWindowText(format);
#ifdef _DEBUG
    TRACE(_T("UpdateNtpFormat: Index %d, Format: %s\n"), index, format.GetString());
#endif
}

// Helper: Read registry settings
void CTimeServerConfigDlg::ReadRegistrySettings() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\W32Time\\Parameters"),
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        TCHAR type[256] = { 0 };
        DWORD size = sizeof(type);
        if (RegQueryValueEx(hKey, _T("Type"), nullptr, nullptr, (LPBYTE)type, &size) == ERROR_SUCCESS) {
            CString currentType(type);
            if (currentType != _T("NT5DS") && currentType != _T("NTP") && currentType != _T("NoSync")) {
                currentType = _T("NT5DS");
            }
            m_comboType.SelectString(-1, currentType);
            OnCbnSelchangeComboType();
        }
        TCHAR ntpServer[1024] = { 0 };
        size = sizeof(ntpServer);
        if (RegQueryValueEx(hKey, _T("NtpServer"), nullptr, nullptr, (LPBYTE)ntpServer, &size) == ERROR_SUCCESS) {
            CString servers(ntpServer);
            CString server;
            int pos = 0;
            int index = 0;
            while (!(server = servers.Tokenize(_T(";"), pos)).IsEmpty() && index < 3) {
                CString flag = server.Mid(server.Find(_T(",")) + 1);
                server = server.Left(server.Find(_T(",")));
                flag = flag == _T("0x9") ? _T("Default (0x9)") :
                    flag == _T("0x1") ? _T("Special Interval (0x1)") : _T("Client Mode (0x8)");
                m_ntpControls[index].edit.SetWindowText(server);
                m_ntpControls[index].combo.SelectString(-1, flag);
                UpdateNtpFormat(index);
                index++;
            }
            // Clear any unused NTP rows
            for (; index < 3; index++) {
                m_ntpControls[index].edit.SetWindowText(_T(""));
                m_ntpControls[index].combo.SetCurSel(0);
                UpdateNtpFormat(index);
            }
        }
        RegCloseKey(hKey);
    }
    else {
        AfxMessageBox(_T("Internal Application Error: Failed to access registry. This operation is not supported on this version of Windows."),
            MB_OK | MB_ICONERROR);
    }
    // Read advanced fields
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\W32Time\\TimeProviders\\NtpClient"),
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        DWORD value;
        DWORD size = sizeof(value);
        if (RegQueryValueEx(hKey, _T("MinPollInterval"), nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            CString str;
            str.Format(_T("%u"), value);
            m_editMinPoll.SetWindowText(str);
        }
        if (RegQueryValueEx(hKey, _T("MaxPollInterval"), nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            CString str;
            str.Format(_T("%u"), value);
            m_editMaxPoll.SetWindowText(str);
        }
        if (RegQueryValueEx(hKey, _T("SpecialPollInterval"), nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            CString str;
            str.Format(_T("%u"), value);
            m_editSpecialPoll.SetWindowText(str);
        }
        RegCloseKey(hKey);
    }
}

BOOL CTimeServerConfigDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Elevation check
    if (!IsUserAnAdmin()) {
        AfxMessageBox(_T("This application requires administrative privileges. Please restart with elevated permissions."),
            MB_OK | MB_ICONERROR);
        EndDialog(IDCANCEL);
        return FALSE;
    }

    // Create Type dropdown
    m_comboType.Create(WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        CRect(20, 20, 200, 120), this, IDC_COMBO_TYPE);
    m_comboType.SetDroppedWidth(180);
    m_comboType.AddString(_T("Domain (NT5DS)"));
    m_comboType.AddString(_T("Manual (NTP)"));
    m_comboType.AddString(_T("NoSync"));
    if (m_comboType.GetCount() != 3) {
        AfxMessageBox(_T("Internal Application Error: Failed to initialize sync type dropdown."),
            MB_OK | MB_ICONERROR);
        EndDialog(IDCANCEL);
        return FALSE;
    }
    m_comboType.SetCurSel(0);
    m_comboType.EnableWindow(TRUE);

    // Create three NTP server controls
    CreateNtpControls(0, 60);
    CreateNtpControls(1, 90);
    CreateNtpControls(2, 120);

    // Create advanced fields
    m_editMinPoll.Create(WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(20, 160, 120, 180), this, IDC_EDIT_MINPOLL);
    m_editMaxPoll.Create(WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(130, 160, 230, 180), this, IDC_EDIT_MAXPOLL);
    m_editSpecialPoll.Create(WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(240, 160, 340, 180), this, IDC_EDIT_SPECIALPOLL);

    // Create action buttons
    m_btnSync.Create(_T("Sync Now"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(20, 200, 120, 220), this, IDC_BUTTON_SYNC);
    m_btnSave.Create(_T("Save/Backup"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(130, 200, 230, 220), this, IDC_BUTTON_SAVE);
    m_btnRestore.Create(_T("Restore/Import"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(240, 200, 340, 220), this, IDC_BUTTON_RESTORE);
    m_btnApply.Create(_T("Apply"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(350, 200, 450, 220), this, IDC_BUTTON_APPLY);
    m_btnRefresh.Create(_T("Refresh"), WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(460, 200, 560, 220), this, IDC_BUTTON_REFRESH);

    // Create progress indicator
    m_staticProgress.Create(_T("Please wait..."), WS_VISIBLE | SS_LEFT,
        CRect(20, 240, 200, 260), this, IDC_STATIC_PROGRESS);
    m_staticProgress.ShowWindow(SW_HIDE);

    // Create status bar
    static UINT indicators[] = { ID_SEPARATOR };
    m_statusBar.Create(this);
    m_statusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
    m_statusBar.SetPaneInfo(0, ID_SEPARATOR, SBPS_STRETCH, 0);
    m_statusBar.SetPaneText(0, _T("Ready"));

    // OS compatibility check
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    if (GetVersionExW((LPOSVERSIONINFOW)&osvi)) {
        CString versionStr = GetVersionString(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
        m_statusBar.SetPaneText(0, _T("OS Version: ") + versionStr);
        bool unsupported = false;
        if (osvi.dwMajorVersion < 10 ||
            (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0 && osvi.dwBuildNumber < 14393) ||
            (osvi.dwMajorVersion > 10 || (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber > 20348))) {
            unsupported = true;
        }
        if (unsupported) {
            AfxMessageBox(_T("Internal Application Error: This application is designed for "
                "Windows Server 2016–2022. Detected version: ") + versionStr,
                MB_OK | MB_ICONERROR);
            m_editMinPoll.EnableWindow(FALSE);
            m_editMaxPoll.EnableWindow(FALSE);
            m_editSpecialPoll.EnableWindow(FALSE);
        }
    }
    else {
        AfxMessageBox(_T("Internal Application Error: Failed to retrieve OS version."),
            MB_OK | MB_ICONERROR);
    }

    // Group Policy check
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Policies\\Microsoft\\W32Time"),
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        AfxMessageBox(_T("Time settings are managed by Group Policy. Changes may be overridden."),
            MB_OK | MB_ICONWARNING);
        RegCloseKey(hKey);
    }

    // Initialize controls from registry
    ReadRegistrySettings();

    return TRUE;
}

void CTimeServerConfigDlg::OnCbnSelchangeComboType() {
    int sel = m_comboType.GetCurSel();
    CString type;
    m_comboType.GetLBText(sel, type);
    bool enableNtp = (type == _T("Manual (NTP)"));
    for (int i = 0; i < 3; i++) {
        m_ntpControls[i].edit.EnableWindow(enableNtp);
        m_ntpControls[i].combo.EnableWindow(enableNtp);
        m_ntpControls[i].btn.EnableWindow(enableNtp);
    }
    m_editMinPoll.EnableWindow(enableNtp && m_editMinPoll.IsWindowEnabled());
    m_editMaxPoll.EnableWindow(enableNtp && m_editMaxPoll.IsWindowEnabled());
    m_editSpecialPoll.EnableWindow(enableNtp && m_editSpecialPoll.IsWindowEnabled());
}

void CTimeServerConfigDlg::OnCbnSelchangeComboFlag(UINT nID) {
    int index = (nID - IDC_COMBO_FLAG1) / 4;
    if (index >= 0 && index < 3) {
        UpdateNtpFormat(index);
#ifdef _DEBUG
        TRACE(_T("OnCbnSelchangeComboFlag: nID=%u, index=%d\n"), nID, index);
#endif
    }
}

void CTimeServerConfigDlg::OnBnClickedButtonTest(UINT nID) {
    m_staticProgress.ShowWindow(SW_SHOW);
    int index = (nID - IDC_BUTTON_TEST1) / 4;
    if (index >= 0 && index < 3) {
        CWnd* pEdit = GetDlgItem(m_ntpControls[index].editId);
        if (pEdit) {
            CString server;
            pEdit->GetWindowText(server);
            if (!server.IsEmpty()) {
                CString cmd = _T("w32tm /stripchart /computer:") + server + _T(" /samples:1");
                STARTUPINFO si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                if (CreateProcess(nullptr, (LPTSTR)cmd.GetString(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    DWORD exitCode;
                    GetExitCodeProcess(pi.hProcess, &exitCode);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    CString result = exitCode == 0 ? _T("Test successful") : _T("Test failed");
                    AfxMessageBox(result, MB_OK | (exitCode == 0 ? MB_ICONINFORMATION : MB_ICONERROR));
                }
                else {
                    AfxMessageBox(_T("Internal Application Error: Failed to run w32tm."), MB_OK | MB_ICONERROR);
                }
            }
            else {
                AfxMessageBox(_T("Please enter an NTP server address."), MB_OK | MB_ICONWARNING);
            }
        }
    }
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnBnClickedButtonSync() {
    m_staticProgress.ShowWindow(SW_SHOW);
    m_staticProgress.SetWindowText(_T("Synchronizing..."));

    CString cmd = _T("w32tm /resync");
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcess(nullptr, (LPTSTR)cmd.GetString(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CString result = exitCode == 0 ? _T("Synchronization successful") : _T("Synchronization failed");
        m_staticProgress.SetWindowText(result);
        Sleep(2000);
    }
    else {
        m_staticProgress.SetWindowText(_T("Failed to run w32tm"));
        Sleep(2000);
    }
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnBnClickedButtonSave() {
    m_staticProgress.ShowWindow(SW_SHOW);
    m_staticProgress.SetWindowText(_T("Saving..."));

    CFileDialog dlg(FALSE, _T("reg"), _T("W32TimeBackup.reg"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("Registry Files (*.reg)|*.reg||"), this);
    if (dlg.DoModal() == IDOK) {
        CString path = dlg.GetPathName();
        CString cmd = _T("reg export HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\W32Time \"") + path + _T("\" /y");
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (CreateProcess(nullptr, (LPTSTR)cmd.GetString(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CString result = exitCode == 0 ? _T("Backup successful") : _T("Backup failed");
            m_staticProgress.SetWindowText(result);
            Sleep(2000);
        }
        else {
            m_staticProgress.SetWindowText(_T("Failed to run reg export"));
            Sleep(2000);
        }
    }
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnBnClickedButtonRestore() {
    m_staticProgress.ShowWindow(SW_SHOW);
    m_staticProgress.SetWindowText(_T("Restoring..."));

    CFileDialog dlg(TRUE, _T("reg"), nullptr,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Registry Files (*.reg)|*.reg||"), this);
    if (dlg.DoModal() == IDOK) {
        CString path = dlg.GetPathName();
        CString cmd = _T("reg import \"") + path + _T("\"");
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (CreateProcess(nullptr, (LPTSTR)cmd.GetString(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CString result = exitCode == 0 ? _T("Restore successful") : _T("Restore failed");
            m_staticProgress.SetWindowText(result);
            Sleep(2000);
            if (exitCode == 0) {
                ReadRegistrySettings();
            }
        }
        else {
            m_staticProgress.SetWindowText(_T("Failed to run reg import"));
            Sleep(2000);
        }
    }
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnBnClickedButtonApply() {
    m_staticProgress.ShowWindow(SW_SHOW);
    m_staticProgress.SetWindowText(_T("Applying settings..."));

    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\W32Time\\Parameters"),
        0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        int sel = m_comboType.GetCurSel();
        CString type;
        m_comboType.GetLBText(sel, type);
        CString regType = (type == _T("Domain (NT5DS)")) ? _T("NT5DS") :
            (type == _T("Manual (NTP)")) ? _T("NTP") : _T("NoSync");
        RegSetValueEx(hKey, _T("Type"), 0, REG_SZ, (const BYTE*)regType.GetString(),
            (regType.GetLength() + 1) * sizeof(TCHAR));

        if (type == _T("Manual (NTP)")) {
            CString servers;
            for (int i = 0; i < 3; i++) {
                CString server, flag;
                m_ntpControls[i].edit.GetWindowText(server);
                int flagSel = m_ntpControls[i].combo.GetCurSel();
                flag = (flagSel == 0) ? _T("0x9") : (flagSel == 1) ? _T("0x1") : _T("0x8");
                if (!server.IsEmpty()) {
                    if (!servers.IsEmpty()) servers += _T(";");
                    servers += server + _T(",") + flag;
                }
            }
            RegSetValueEx(hKey, _T("NtpServer"), 0, REG_SZ, (const BYTE*)servers.GetString(),
                (servers.GetLength() + 1) * sizeof(TCHAR));
        }
        RegCloseKey(hKey);
    }

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\W32Time\\TimeProviders\\NtpClient"),
        0, KEY_WRITE, &hKey);
    if (result == ERROR_SUCCESS) {
        CString str;
        m_editMinPoll.GetWindowText(str);
        DWORD value = _ttoi(str);
        RegSetValueEx(hKey, _T("MinPollInterval"), 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
        m_editMaxPoll.GetWindowText(str);
        value = _ttoi(str);
        RegSetValueEx(hKey, _T("MaxPollInterval"), 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
        m_editSpecialPoll.GetWindowText(str);
        value = _ttoi(str);
        RegSetValueEx(hKey, _T("SpecialPollInterval"), 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }

    CString cmd = _T("w32tm /config /update");
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcess(nullptr, (LPTSTR)cmd.GetString(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CString result = exitCode == 0 ? _T("Settings applied successfully") : _T("Failed to apply settings");
        m_staticProgress.SetWindowText(result);
        Sleep(2000);
    }
    else {
        m_staticProgress.SetWindowText(_T("Failed to run w32tm"));
        Sleep(2000);
    }
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnBnClickedButtonRefresh() {
    m_staticProgress.ShowWindow(SW_SHOW);
    m_staticProgress.SetWindowText(_T("Refreshing..."));
    ReadRegistrySettings();
    m_staticProgress.SetWindowText(_T("Refresh complete"));
    Sleep(2000);
    m_staticProgress.ShowWindow(SW_HIDE);
}

void CTimeServerConfigDlg::OnEnChangeEditNtp(UINT nID) {
    int index = (nID - IDC_EDIT_NTP1) / 4;
    if (index >= 0 && index < 3) {
        UpdateNtpFormat(index);
#ifdef _DEBUG
        TRACE(_T("OnEnChangeEditNtp: nID=%u, index=%d\n"), nID, index);
#endif
    }
}