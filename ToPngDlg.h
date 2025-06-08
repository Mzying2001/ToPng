#ifndef _TOPNGDLG_H_
#define _TOPNGDLG_H_

#include "SimpleWindow.h"

// 菜单id
enum {
    MENUITEM_ENCODE = 100, // 编码
    MENUITEM_DECODE,       // 解码
    MENUITEM_ENCODEDIR,    // 打包文件夹并编码
    MENUITEM_EXIT,         // 退出
};

// 对话框
class ToPngDlg : public sw::Window
{
private:
    // 是否加载中
    bool isLoading = false;

    // 错误信息
    std::wstring errMsg;

    // 显示状态信息的Label
    sw::Label labelStatus;

    // 菜单
    sw::Menu menu
    {
        sw::MenuItem(L"文件",
        {
            sw::MenuItem(MENUITEM_ENCODE, L"编码", *this, &ToPngDlg::MenuCommandHandler),
            sw::MenuItem(MENUITEM_DECODE, L"解码", *this, &ToPngDlg::MenuCommandHandler),
            sw::MenuItem(L"-"),
            sw::MenuItem(MENUITEM_ENCODEDIR, L"打包文件并编码（zip）", *this, &ToPngDlg::MenuCommandHandler),
            sw::MenuItem(L"-"),
            sw::MenuItem(MENUITEM_EXIT, L"退出", *this, &ToPngDlg::MenuCommandHandler),
        })
    };

public:
    // 构造函数
    ToPngDlg();

    // 初始化控件
    void InitializeComponent();

    // 窗口关闭处理函数
    void WindowClosingHandler(sw::UIElement& sender, sw::WindowClosingEventArgs& e);

    // 文件拖放事件处理函数
    void DropFilesHandler(sw::UIElement& sender, sw::DropFilesEventArgs& e);

    // 菜单时间处理函数
    void MenuCommandHandler(sw::MenuItem& menuItem);

    // 更新状态文本
    void UpdateStatusText();

    // 更新菜单项Enable
    void UpdateEnables();

    // 设置加载状态
    void SetLoadingState(bool isLoading);

    // 编码完成
    void OnEncodeDone(bool success);

    // 解码完成
    void OnDecodeDone(bool success);

    // 编码
    void Encode(const std::wstring& input, const std::wstring& output);

    // 解码
    void Decode(const std::wstring& input, const std::wstring& output);

    // 编码文件夹
    void EncodeDir(const std::wstring& input, const std::wstring& output);

    // 显示打开文件对话框
    bool ShowOpenFile(std::wstring& refFileName);

    // 显示保存png文件对话框
    bool ShowSavePngFile(std::wstring& refFileName);

    // 显示打开png文件对话框
    bool ShowOpenPngFile(std::wstring& refFileName);

    // 显示保存文件对话框
    bool ShowSaveFile(std::wstring& refFileName);

    // 显示打开文件夹对话框
    bool ShowOpenDirectory(std::wstring& refDirectory);

    // 判断字符串是否以指定字串结尾，忽略大小写
    bool StrEndsWithIgnoreCase(const std::wstring& str, const std::wstring& suffix);
};

#endif // !_TOPNGDLG_H_
