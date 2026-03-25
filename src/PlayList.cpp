#include <filesystem>
#include "PlayList.h"
#include "Log.h"

bool PlayList::Scan(const std::string& Directory)
{
    if(std::filesystem::exists(Directory) == false)
    {
        LOGE("Current directory not exist :[{}]", Directory);
        return false;
    }

    for(const auto& file : std::filesystem::directory_iterator{Directory})
    {
        std::string FileString = file.path().string();
        if(FileString.ends_with(".mp3") || FileString.ends_with(".mp4"))
        {
            m_ListMedia.emplace_back(FileString);
        }
    }
    return true;
}

std::string PlayList::GetCurrentMedia()
{
    if(m_IDCurrentMedia < 0 || static_cast<size_t>(m_IDCurrentMedia) >= m_ListMedia.size())
    {
        LOGE("Out range of list media {}", m_IDCurrentMedia);
        return {};
    }
    return m_ListMedia.at(m_IDCurrentMedia);
}

void PlayList::Next()
{
    if(static_cast<size_t>(m_IDCurrentMedia) < (m_ListMedia.size() - 1))
    {
        m_IDCurrentMedia++;
    }
}

void PlayList::Privious()
{
    if(m_IDCurrentMedia > 0)
    {
        m_IDCurrentMedia--;
    }
}
