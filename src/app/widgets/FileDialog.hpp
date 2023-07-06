#pragma once

#include "deps/ImFileDialog/ImFileDialog.h"
namespace Widgets {

    class FileDialog : private ifd::FileDialog {
    public:
        FileDialog(const char* title) : m_title(title) {
            setToCurrentPath();
            setDeleteTexture();
        }
        virtual ~FileDialog() = default;
        void setToCurrentPath() { m_currentPath = std::filesystem::current_path(); }
        void openDialog() {
            Open(m_title, m_title, "*.*", false, reinterpret_cast<const char*>(m_currentPath.u8string().c_str()));
        }
        const std::vector<std::u8string>& selected() const { return m_results; }
        bool draw();
        std::filesystem::path m_currentPath;

        void* CreateTexture(uint8_t* data, int w, int h, char fmt) override;

    private:
        void setDeleteTexture();
        const char* m_title;
        std::vector<std::u8string> m_results;
    };
} // Widgets
