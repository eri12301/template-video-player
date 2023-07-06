#include "FileDialog.hpp"
#include "glad/glad.h"

namespace Widgets
{
    void *FileDialog::CreateTexture(uint8_t *data, int w, int h, char fmt)
    {
        GLuint tex;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void *)tex;
    }

    void FileDialog::setDeleteTexture()
    {
        DeleteTexture = [](void *tex)
        {
            GLuint texID = reinterpret_cast<uintptr_t>(tex);
            glDeleteTextures(1, &texID);
        };
    }

    bool FileDialog::draw()
    {
        bool done = IsDone(m_title);
        m_currentPath = CurrentDirectory();
        if (done)
        {
            auto results = GetResults();
            m_results.clear();
            m_results.reserve(results.size());
            for (auto &result : results)
                m_results.push_back(result.u8string());
            Close();
        }
        return done;
    }

} // namespace Widgets
