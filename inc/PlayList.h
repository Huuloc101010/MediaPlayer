#ifndef _PLAY_LIST_
#define _PLAY_LIST_

#include <string>
#include <vector>

class PlayList
{
public:
    PlayList()  = default;
    ~PlayList() = default;
    bool Scan(const std::string& Directory);
    std::string GetCurrentMedia();
    void Next();
    void Privious();
private:
    int m_IDCurrentMedia = {};
    std::vector<std::string>   m_ListMedia = {};
};

#endif /* _PLAY_LIST_ */