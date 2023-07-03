#pragma once
#include <map>
#include <string>
#include <flac++/metadata.h>
using namespace FLAC::Metadata;

class FlacMetadata
{
public:
    int Open(const std::wstring& file);
    int Save();

    bool GetTag(const std::string& name, std::wstring& value);
    void SetTag(const std::string& name, const std::wstring& value);

    static std::wstring DecodeUTF8(const std::string& str);
    static std::string EncodeUTF8(const std::wstring& str);

    static std::string ToUpper(const std::string& str);

private:
    std::string GetTagName(const std::string& name);

    Prototype* GetBlock(int blockType);

    void ReadBlocks();
    void SaveBlocks();

    void ReadVorbisComment();
    void SaveVorbisComment();

private:
    Chain mChain;
    std::map<int, Prototype*> mBlocks;
    std::map<std::string, std::wstring> mTags;
};

