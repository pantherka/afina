#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
    std::size_t curr_size = key.size() + value.size();
    if (curr_size > _max_size) {
        return false;
    }
    auto found = _lru_index.find(std::reference_wrapper<const std::string>(key));
    if (found != _lru_index.end()) {
        MoveToHead(found->second);
        return UpdateNode(found->second, value);
    }
    return InsertNode(key, value);
}
    
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    std::size_t curr_size = key.size() + value.size();
    if (curr_size > _max_size) {
        return false;
    }
    if (_lru_index.find(key) != _lru_index.end()) {
        return false;
    }
    return InsertNode(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    std::size_t curr_size = key.size() + value.size();
    if (curr_size > _max_size) {
        return false;
    }
    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
        return false;
    }
    MoveToHead(found->second);
    return UpdateNode(found->second, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    if (_lru_head == nullptr) {
        return false;
    }
    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
        return false;
    }
    auto curr_node_ref = found->second;
    std::size_t curr_size = curr_node_ref.get().key.size() + curr_node_ref.get().value.size();
    _lru_index.erase(found);
    _filled_size -= curr_size;
    return DeleteFromList(curr_node_ref);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) { 
    auto found = _lru_index.find(key);
    if (found == _lru_index.end()) {
        return false;
    }

    MoveToHead(found->second);
    value = found->second.get().value;
    return true; 
}

bool SimpleLRU::InsertNode(const std::string &key, const std::string &value) {
    std::size_t curr_size = key.size() + value.size();
    FreeSpace(curr_size);
    lru_node *curr_node = new lru_node{key, value, nullptr, nullptr};
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

bool SimpleLRU::UpdateNode(lru_node &node_ref, const std::string &value) {
    std::size_t diff = value.size() - node_ref.value.size();
    FreeSpace(diff);
    node_ref.value = value;
    _filled_size += diff;   
    return true;
}

bool SimpleLRU::DeleteFromList(lru_node &node_ref) {
    if (node_ref.next != nullptr) {
        node_ref.next->prev = node_ref.prev;
    }
    else {
        _lru_tail = node_ref.prev;
    }
    if (node_ref.prev != nullptr) {
        node_ref.prev->next = std::move(node_ref.next);
    }
    else {
        _lru_head = std::move(node_ref.next);
    }
    return true;
}

void SimpleLRU::MoveToHead(lru_node &node_ref) {
    if (&node_ref == _lru_head.get()) {
        return;
    }

    auto curr_node_ptr = std::move(node_ref.prev->next);
    if (curr_node_ptr->next != nullptr) {
        curr_node_ptr->next->prev = curr_node_ptr->prev;
    }
    else { // it was tail
        _lru_tail = curr_node_ptr->prev;
    }
    // it is not head
    curr_node_ptr->prev->next = std::move(curr_node_ptr->next);

    curr_node_ptr->next = std::move(_lru_head);
    curr_node_ptr->next->prev = curr_node_ptr.get();
    curr_node_ptr->prev = nullptr;
    _lru_head = std::move(curr_node_ptr);
}

void SimpleLRU::FreeSpace(int diff) {
    while (_filled_size + diff > _max_size) {
        Delete(_lru_tail->key);
    }
}

} // namespace Backend
} // namespace Afina
