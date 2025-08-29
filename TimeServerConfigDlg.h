#pragma once
#include <afxcmn.h> // For CComboBox, CListBox
#include <afxdlgs.h> // For CFileDialog

// CTimeServerConfigDlg dialog
class CTimeServerConfigDlg : public CDialogEx {
    DECLARE_DYNAMIC(CTimeServerConfigDlg)

public:
    CTimeServerConfigDlg(CWnd* pParent = nullptr); // Standard constructor
    virtual ~CTimeServerConfigDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TIMESERVERCONFIG_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

    // Controls
    CComboBox m_comboType;
    struct NtpServerControls {
        UINT editId, comboId, staticId, btnId;
        CEdit edit;
        CComboBox combo;
        CStatic stat;
        CButton btn;
    };
    NtpServerControls m_ntpControls[3]; // Fixed array for 3 NTP rows
    CEdit m_editMinPoll, m_editMaxPoll, m_editSpecialPoll;
    CButton m_btnSync, m_btnSave, m_btnRestore, m_btnApply, m_btnRefresh;
    CStatic m_staticProgress;
    CStatusBar m_statusBar;

    // Helper functions
    CString GetVersionString(DWORD major, DWORD minor, DWORD build);
    void UpdateNtpFormat(int index);
    void ReadRegistrySettings();
    void CreateNtpControls(int index, int y, const CString& server = _T(""), const CString& flag = _T("Default (0x9)"));

public:
    // Message handlers
    afx_msg void OnCbnSelchangeComboType();
    afx_msg void OnCbnSelchangeComboFlag(UINT nID);
    afx_msg void OnBnClickedButtonTest(UINT nID);
    afx_msg void OnBnClickedButtonSync();
    afx_msg void OnBnClickedButtonSave();
    afx_msg void OnBnClickedButtonRestore();
    afx_msg void OnBnClickedButtonApply();
    afx_msg void OnBnClickedButtonRefresh();
    afx_msg void OnEnChangeEditNtp(UINT nID);
};