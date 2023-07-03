#include "FlacMetadata.h"
#include <utility>
#include <algorithm>
#include <codecvt>

static std::wstring_convert<std::codecvt_utf8<wchar_t>> Utf8Converter;

int FlacMetadata::Open(const std::wstring& filename)
{
    auto str = EncodeUTF8(filename);
    if (!mChain.read(str.c_str(), false))
        return -1;
    mChain.sort_padding();
    ReadBlocks();
    ReadVorbisComment();
    return 0;
}

int FlacMetadata::Save()
{
    SaveVorbisComment();
    SaveBlocks();
    if (!mChain.write(true, true))
        return -1;
    return 0;
}

bool FlacMetadata::GetTag(const std::string& name, std::wstring& value)
{
    auto tag = GetTagName(name);
    auto it = mTags.find(tag);
    if (it != mTags.end()) {
        value = it->second;
        return true;
    }
    return false;
}

void FlacMetadata::SetTag(const std::string& name, const std::wstring& value)
{
    auto tag = GetTagName(name);
    auto it = mTags.find(tag);
    if (it != mTags.end()) {
        it->second = value;
    }
    else {
        mTags.insert(std::make_pair(tag, value));
    }
}

void FlacMetadata::ReadBlocks()
{
    mBlocks.clear();
    Iterator iterator;
    iterator.init(mChain);
    do
    {
        auto blockType = iterator.get_block_type();
        auto block = iterator.get_block();
        mBlocks.insert(std::make_pair(blockType, block));
    } while (iterator.next());
}

void FlacMetadata::SaveBlocks() {
    // do nothing
}

Prototype* FlacMetadata::GetBlock(int blockType)
{
    auto it = mBlocks.find(blockType);
    if (it != mBlocks.end()) {
        return it->second;
    }
    return nullptr;
}

void FlacMetadata::ReadVorbisComment()
{
    mTags.clear();
    auto comment = (VorbisComment*)GetBlock(FLAC__MetadataType::FLAC__METADATA_TYPE_VORBIS_COMMENT);
    for (int i = 0; i < comment->get_num_comments(); ++i) {
        auto entry = comment->get_comment(i);
        auto name = std::string(entry.get_field_name());
        auto value = DecodeUTF8(entry.get_field_value());
        SetTag(name, value);
    }
}

void FlacMetadata::SaveVorbisComment()
{
    auto comment = (VorbisComment*)GetBlock(FLAC__MetadataType::FLAC__METADATA_TYPE_VORBIS_COMMENT);
    auto tempTags(mTags);
    for (int i = comment->get_num_comments() - 1; i >= 0; --i) {
        auto entry = comment->get_comment(i);
        auto name = GetTagName(entry.get_field_name());
        auto it = mTags.find(name);
        if (it != mTags.end()) {
            auto str = EncodeUTF8(it->second);
            entry.set_field_value(str.c_str());
            comment->set_comment(i, entry);
            tempTags.erase(name);
        }
        else
        {
            comment->delete_comment(i);
        }
    }
    for (auto& pair : tempTags) {
        auto value = EncodeUTF8(pair.second);
        VorbisComment::Entry entry(pair.first.c_str(), value.c_str());
        comment->append_comment(entry);
    }
}

std::wstring FlacMetadata::DecodeUTF8(const std::string& str)
{
    return Utf8Converter.from_bytes(str);
}

std::string FlacMetadata::EncodeUTF8(const std::wstring& str)
{
    return Utf8Converter.to_bytes(str);
}

std::string FlacMetadata::GetTagName(const std::string& name)
{
    return ToUpper(name);
}

std::string FlacMetadata::ToUpper(const std::string& str)
{
    std::string temp(str);
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
    return temp;
}
