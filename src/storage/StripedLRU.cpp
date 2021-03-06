#include "StripedLRU.h"


namespace Afina {
namespace Backend {
    static StripedLRU* BuildStripedLRU(size_t stripe_count, size_t max_size = 1024) {
        size_t max_shard_size = max_size / stripe_count;
        if (max_shard_size < 16 * 1024 * 1024) {
            throw std::runtime_error("Too many stripes");
        }
        StripedLRU* cache = new StripedLRU(stripe_count, max_shard_size);
        return std::move(cache);
    }

} // namespace Backend
} // namespace Afina
