#include <iostream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include "FlacMetadata.h"
namespace fs = std::filesystem;

void FixMetaData(const std::wstring& path);
void FixMetaData_Dir(const std::wstring& path);


int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;

    std::filesystem::path path(argv[1]);
    std::wcout.imbue(std::locale("chs"));
    std::wcout << "path: " << path << std::endl;

    if (fs::is_directory(path))
    {
        FixMetaData_Dir(path);
    }
    else
    {
        FixMetaData(path);
    }

    std::cout << "Finish!\n";
    getchar();
    return 0;
}


bool FixArtist(FlacMetadata& file);
bool FixTitle(FlacMetadata& file, const std::wstring& path);

std::wregex pattern_fixartist(L"\\((\\d+)\\)(.+)");
std::wregex pattern_filename(L".+/(.+) \\+\\-\\+ (.+)\\.flac");


void FixMetaData_Dir(const std::wstring& dir_path)
{
    for (auto& file : fs::directory_iterator(dir_path)) {
        if (fs::is_regular_file(file)) {
            auto file_path = file.path();
            auto ext = file_path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".flac") {
                FixMetaData(file_path);
            }
        }
    }
}

void FixMetaData(const std::wstring& file_path) {
    std::wstring path(file_path);
    std::replace(path.begin(), path.end(), '\\', '/');

    FlacMetadata file;
    int code = file.Open(path);
    if (code != 0) {
        std::wcout << "Open failed [" << code << "]: " << path << std::endl;
        return;
    }

    bool changed = false;
    changed = FixArtist(file) || changed;
    changed = FixTitle(file, path) || changed;

    if (changed)
    {
        int code = file.Save();
        if (code != 0)
            std::wcout << "Save failed [" << code << "]: " << path << std::endl;
        else
            std::wcout << "Save success: " << path << std::endl;
    }
}

bool FixArtist(FlacMetadata& file)
{
    std::wsmatch result;
    std::wstring artist;

    bool changed = false;
    if (file.GetTag("ARTIST", artist))
    {
        if (std::regex_search(artist, result, pattern_fixartist))
        {
            auto date = result[1].str();
            artist = result[2].str();
            file.SetTag("ARTIST", artist);
            file.SetTag("DATE", date);
            changed = true;
        }

        std::wstring album;
        if (file.GetTag("ALBUM", album))
        {
            if (album == artist || album == L"未知标题" || album == L"?")
            {
                file.SetTag("ALBUM", L"");
                changed = true;
            }
        }
    }

    return changed;
}

bool FixTitle(FlacMetadata& file, const std::wstring& path)
{
    bool changed = false;
    std::wsmatch result;
    if (std::regex_search(path, result, pattern_filename))
    {
        auto artist = result[1].str();
        auto title = result[2].str();

        std::wstring artist2, title2;
        file.GetTag("ARTIST", artist2);
        file.GetTag("TITLE", title2);

        if (artist2.empty() || artist2 != artist) {
            file.SetTag("ARTIST", artist);
            changed = true;
        }

        if (title2.empty() || title2 != title) {
            file.SetTag("TITLE", title);
            changed = true;
        }

        std::wstring comment;
        file.GetTag("Comment", comment);
        if (!comment.empty()) {
            file.SetTag("Comment", L"");
            changed = true;
        }
    }
    return changed;
}
