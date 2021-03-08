#ifndef AFINA_STORAGE_STRIPED_LRU_H
#define AFINA_STORAGE_STRIPED_LRU_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # ThreadSafeSimplLRU striped version
 *
 *
 */
class StripedLRU : public Afina::Storage {
private:
    StripedLRU(size_t stripe_count, size_t max_shard_size = 1024) : _shards_count(stripe_count) {
        for (size_t i = 0; i < stripe_count; ++i) {
            _shards.emplace_back(max_shard_size);
        }
    }
public:
    static std::unique_ptr<StripedLRU> BuildStripedLRU(size_t stripe_count, size_t max_size);

    ~StripedLRU() {}

    // see ThreadSafeSimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        size_t hash = std::hash<std::string>{}(key);
        return _shards[hash % _shards_count].Put(key, value);
    }

    // see ThreadSafeSimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        size_t hash = std::hash<std::string>{}(key);
        return _shards[hash % _shards_count].PutIfAbsent(key, value);
    }

    // see ThreadSafeSimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        size_t hash = std::hash<std::string>{}(key);
        return _shards[hash % _shards_count].Set(key, value);
    }

    // see ThreadSafeSimpleLRU.h
    bool Delete(const std::string &key) override {
        size_t hash = std::hash<std::string>{}(key);
        return _shards[hash % _shards_count].Delete(key);
    }

    // see ThreadSafeSimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        size_t hash = std::hash<std::string>{}(key);
        return _shards[hash % _shards_count].Get(key, value);
    }

private:
    std::vector<SimpleLRU> _shards;
    size_t _shards_count = 4;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_STRIPED_LRU_H
