
// ToPngDlg.h: 头文件
//

#pragma once


#define WM_BUSYCNANGED (WM_USER+100) // Busy状态改变
#define WM_ENCODEDONE  (WM_USER+101) // 编码完成
#define WM_DECODEDONE  (WM_USER+102) // 解码完成


// CToPngDlg 对话框
class CToPngDlg : public CDialog
{
    // 构造
public:
    CToPngDlg(CWnd* pParent = nullptr); // 标准构造函数

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TOPNG_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


    // 实现
protected:
    HICON m_hIcon;
    CStatic m_textCtrlStatus;
    CString m_strStatus;
    BOOL m_bBusy;

    // 生成的消息映射函数
    DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnBusyChanged(WPARAM wParam = 0, LPARAM lParam = 0);
    afx_msg LRESULT OnEncodeDone(WPARAM wParam, LPARAM lParam = 0);
    afx_msg LRESULT OnDecodeDone(WPARAM wParam, LPARAM lParam = 0);

public:
    CString m_strErrMsg; // 储存错误信息

    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnEncode();
    afx_msg void OnDecode();
    afx_msg void OnExit();

public:
    // 更新布局
    void UpdateLayout();

    // 更新状态文本
    void UpdateStatusText();

    // 更新菜单项Enable
    void UpdateEnables();

    // 设置Busy状态
    void Busy(BOOL b);

    // 显示打开文件对话框
    BOOL ShowOpenFile(CString& refFileName);

    // 显示保存png文件对话框
    BOOL ShowSavePngFile(CString& refFileName);

    // 显示打开png文件对话框
    BOOL ShowOpenPngFile(CString& refFileName);

    // 显示保存文件对话框
    BOOL ShowSaveFile(CString& refFileName);

    // 编码
    void Encode(const CString& input, const CString& output);

    // 解码
    void Decode(const CString& input, const CString& output);
};
