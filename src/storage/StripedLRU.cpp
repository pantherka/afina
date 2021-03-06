#include "StripedLRU.h"


namespace Afina {
namespace Backend {
    std::unique_ptr<StripedLRU> StripedLRU::BuildStripedLRU(size_t stripe_count, size_t max_size) {
        size_t max_shard_size = max_size / stripe_count;
        if (max_shard_size < 1024 * 1024UL) {
            throw std::runtime_error("Too many stripes");
        }
        auto cache = std::unique_ptr<StripedLRU>(new StripedLRU(stripe_count, max_shard_size));
        return std::move(cache);
    }

} // namespace Backend
} // namespace Afina
