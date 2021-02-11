#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    if (_lru_index.find(std::reference_wrapper<const std::string>(key)) != _lru_index.end()) {
        SimpleLRU::Delete(key);
    }
    return SimpleLRU::PutAbsent(key, value);
}

bool SimpleLRU::PutAbsent(const std::string &key, const std::string &value) {
    std::size_t curr_size = key.size() + value.size();
    if (curr_size > _max_size) {
        return false;
    }
    while (curr_size + _filled_size > _max_size) {
        SimpleLRU::Delete(_lru_tail->key);
    }
    lru_node *curr_node = new lru_node(key);
    curr_node->value = value;
    curr_node->prev = nullptr;
    if (_lru_tail == nullptr) {
        _lru_tail = curr_node;
    }
    if (_lru_head != nullptr) {
        _lru_head->prev = curr_node;
    }
    curr_node->next = std::move(_lru_head);
    _lru_index.emplace(std::make_pair(std::reference_wrapper<const std::string>(curr_node->key), std::reference_wrapper<lru_node>(*curr_node)));
    _lru_head = std::unique_ptr<lru_node>(curr_node);
    _filled_size += curr_size;
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (_lru_index.find(std::reference_wrapper<const std::string>(key)) != _lru_index.end()) {
        return false;
    }
    return SimpleLRU::PutAbsent(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (_lru_index.find(std::reference_wrapper<const std::string>(key)) == _lru_index.end()) {
        return false;
    }
    std::size_t curr_size = key.size() + value.size();
    if (curr_size > _max_size) {
        return false;
    }
    SimpleLRU::Delete(key);
    return SimpleLRU::PutAbsent(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    if (_lru_head == nullptr) {
        return false;
    }
    auto found = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (found == _lru_index.end()) {
        return false;
    }
    auto curr_node_ref = found->second;
    std::size_t curr_size = curr_node_ref.get().key.size() + curr_node_ref.get().value.size();
    _lru_index.erase(found);
    if (curr_node_ref.get().next != nullptr) {
        curr_node_ref.get().next->prev = curr_node_ref.get().prev;
    }
    else {
        _lru_tail = curr_node_ref.get().prev;
    }
    if (curr_node_ref.get().prev != nullptr) {
        curr_node_ref.get().prev->next = std::move(curr_node_ref.get().next);
    }
    else {
        _lru_head = std::move(curr_node_ref.get().next);
    }
    _filled_size -= curr_size;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) { 
    auto found = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (found == _lru_index.end()) {
        return false;
    }
    auto curr_node_ref = found->second;
    value = curr_node_ref.get().value;
    return true; 
}

} // namespace Backend
} // namespace Afina
