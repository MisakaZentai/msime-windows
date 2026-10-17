#pragma once

#include "engine/core/word_item.h"
#include "engine/core/input_session_types.h"
#include "engine/common/helpcode_utils.h"
#include "jev_client.h"
#include <algorithm>

namespace Jev
{
inline bool SameQuery(const metasequoia::OnlineQuery &a, const metasequoia::OnlineQuery &b)
{
    return a.session_id == b.session_id && a.generation == b.generation && a.scheme == b.scheme &&
           a.identity == b.identity && a.query_text == b.query_text && a.cache_key == b.cache_key &&
           a.pinyin_segments == b.pinyin_segments && a.ai_eligible && b.ai_eligible;
}
inline std::vector<std::string> CandidatePool(const std::vector<WordItem> &items, size_t syllables)
{
    std::vector<std::string> result;
    if (syllables == 0)
        return result;
    for (const auto &item : items)
    {
        const bool eligible =
            item.source == CandidateSource::Database || item.source == CandidateSource::UserDatabase ||
            item.sentence_association || item.source == CandidateSource::NeuralDesktop ||
            item.source == CandidateSource::NeuralKeyboard || item.source == CandidateSource::Collocation;
        if (!eligible || HelpcodeUtils::count_han_chars(item.word) != syllables ||
            HelpcodeUtils::count_utf8_chars(item.word) != syllables ||
            std::find(result.begin(), result.end(), item.word) != result.end())
            continue;
        result.push_back(item.word);
        if (result.size() == kMaxCandidates)
            break;
    }
    return result;
}

// Keep the local first/second choice and all user-pinned positions stable. Preserve the full
// WordItem: pinyin, provenance and dictionary learning still belong to the original candidate.
inline bool Promote(std::vector<WordItem> &items, const std::string &word)
{
    const auto selected =
        std::find_if(items.begin(), items.end(), [&](const WordItem &item) { return item.word == word; });
    if (selected == items.end())
        return false;
    if (std::any_of(items.begin(), items.end(), [](const WordItem &item) { return item.fixed_position > 0; }))
        return true;
    const auto rank = static_cast<size_t>(selected - items.begin());
    if (rank > 2)
        std::rotate(items.begin() + 2, selected, selected + 1);
    return true;
}
} // namespace Jev
