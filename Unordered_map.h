#include <iostream>
#include <vector>
#include <memory>
#include <iterator>

template<typename T, typename Allocator = std::allocator<T>>
class List {
private:
    struct BaseNode {
        BaseNode* next = this;
        BaseNode* prev = this;
    };

    struct Node : BaseNode {
        T value;
    };

    BaseNode fakeNode_;
    size_t listSize_ = 0;

    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

    [[no_unique_address]] NodeAllocator alloc_;

    void connect(BaseNode* first, BaseNode* second) {
        first->next = second;
        second->prev = first;
    }

    void add(BaseNode* prevNode, BaseNode* newNode, BaseNode* nextNode) {
        prevNode->next = newNode;
        nextNode->prev = newNode;
        newNode->next = nextNode;
        newNode->prev = prevNode;
    }
public:
    template<bool IsConst>
    class CommonIterator {
    private:
        BaseNode* ptr_ = nullptr;
    public:
        using value_type = T;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ssize_t;

        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

        CommonIterator() = default;
        CommonIterator(const CommonIterator& other) = default;
        CommonIterator(BaseNode* ptr_) : ptr_(ptr_) {}
        CommonIterator<true>(const BaseNode* ptr1) : ptr_(const_cast<BaseNode*>(ptr1)) {}

        CommonIterator& operator=(const CommonIterator& other) = default;

        BaseNode* getNodePtr() const {
            return ptr_;
        }

        CommonIterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }

        CommonIterator operator++(int) {
            CommonIterator copy = *this;
            ++*this;
            return copy;
        }

        CommonIterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }

        CommonIterator operator--(int) {
            CommonIterator copy = *this;
            --*this;
            return copy;
        }

        reference operator*() const {
            return static_cast<Node*>(ptr_)->value;
        }

        pointer operator->() const {
            return &static_cast<Node*>(ptr_)->value;
        }

        operator CommonIterator<true>() const {
            return CommonIterator<true>(ptr_);
        }

        bool operator==(const CommonIterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const CommonIterator& other) const {
            return ptr_ != other.ptr_;
        }

        CommonIterator next() const {
            return CommonIterator(ptr_->next);
        }

        CommonIterator prev() const {
            return CommonIterator(ptr_->prev);
        }
    };

    using iterator = CommonIterator<false>;
    using const_iterator = CommonIterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <typename ...Args>
    Node* create_node(Args&& ...args) {
        Node* pointer = NodeAllocTraits::allocate(alloc_, 1);
        try{
            NodeAllocTraits::construct(alloc_, &pointer->value, std::forward<Args>(args)...);
        } catch(...){
            NodeAllocTraits::deallocate(alloc_, pointer, 1);
            throw;
        }
        return pointer;
    }

    void destroy_node(Node* node) {
        NodeAllocTraits::destroy(alloc_, node);
        NodeAllocTraits::deallocate(alloc_, node, 1);
    }

    void clear(){
        while (listSize_ > 0){
            erase(begin());
        }
    }

    List() = default;

    List(size_t cnt) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                emplace(end());
            }catch(...){
                clear();
                throw;
            }
        }
    }

    List(size_t cnt, const T& value) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                emplace(end(), value);
            }catch(...){
                clear();
                throw;
            }
        }
    }

    List(const Allocator& alloc_) : alloc_(alloc_) {}

    List(size_t cnt, const Allocator& alloc1) : List(alloc1) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                emplace(end());
            }catch(...){
                clear();
                throw;
            }
        }
    }

    List(size_t cnt, const T& value, const Allocator& alloc1) : List(alloc1) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                emplace(end(), value);
            }catch(...){
                clear();
                throw;
            }
        }
    }

    List(const List& other) : alloc_(NodeAllocTraits::select_on_container_copy_construction(other.alloc_)) {
        auto otherIter = other.begin();
        for (size_t i = 0; i < other.listSize_; ++i, ++otherIter) {
            try{
                emplace(end(), *otherIter);
            }catch(...){
                clear();
                throw;
            }
        }
    }

    List(List&& other) : listSize_(other.listSize_), alloc_(std::move(other.alloc_)) {
        fakeNode_.next = other.fakeNode_.next;
        fakeNode_.prev = other.fakeNode_.prev;
        other.fakeNode_.prev->next = &fakeNode_;
        other.fakeNode_.next->prev = &fakeNode_;
        other.fakeNode_.next = &other.fakeNode_;
        other.fakeNode_.prev = &other.fakeNode_;
        listSize_ = other.size();
        other.listSize_ = 0;
    }

    List& operator=(List&& other) {
        if (&other == this) {
            return *this;
        }
        clear();
        fakeNode_.next = other.fakeNode_.next;
        fakeNode_.prev = other.fakeNode_.prev;
        other.fakeNode_.prev->next = &fakeNode_;
        other.fakeNode_.next->prev = &fakeNode_;
        other.fakeNode_.next = &other.fakeNode_;
        other.fakeNode_.prev = &other.fakeNode_;
        listSize_ = other.size();
        other.listSize_ = 0;
        return *this;
    }

    List& operator=(const List& other) {
        if (&other == this) {
            return *this;
        }
        clear();
        if (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
            alloc_ = other.alloc_;
        }
        auto otherIter = other.begin();
        for (size_t i = 0; i < other.listSize_; ++i, ++otherIter) {
            try {
                push_back(*otherIter);
            }
            catch (...) {
                clear();
                throw;
            }
        }
        return *this;
    }

    Allocator get_allocator() const {
        return alloc_;
    }

    size_t size() const {
        return listSize_;
    }

    template <typename ...Args>
    void emplace(const iterator& iter, Args&&... args) {
        auto newNode = create_node(std::forward<Args>(args)...);
        listSize_++;
        add(iter.getNodePtr()->prev, static_cast<BaseNode*>(newNode), iter.getNodePtr());
    }

    void emplace(const iterator& iter, Node* newNode) {
        listSize_++;
        add(iter.getNodePtr()->prev, static_cast<BaseNode*>(newNode), iter.getNodePtr());
    }

    void erase(const iterator& iter) {
        --listSize_;
        connect(iter.getNodePtr()->prev, iter.getNodePtr()->next);
        NodeAllocTraits::destroy(alloc_, static_cast<Node*>(iter.getNodePtr()));
        NodeAllocTraits::deallocate(alloc_, static_cast<Node*>(iter.getNodePtr()), 1);
    }

    void push_back(const T& value) {
        emplace(end(), value);
    }

    void push_back(T&& value) {
        emplace(end(), std::move(value));
    }

    void push_front(const T& value) {
        emplace(begin(), value);
    }

    void push_front(T&& value) {
        emplace(begin(), std::move(value));
    }

    void pop_front() {
        erase(begin());
    }

    void pop_back() {
        erase(std::prev(end()));
    }

    iterator begin() {
        return iterator(fakeNode_.next);
    }

    const_iterator begin() const {
        return const_iterator(fakeNode_.next);
    }

    const_iterator cbegin() const {
        return const_iterator(fakeNode_.next);
    }

    iterator end() {
        return iterator(&fakeNode_);
    }

    const_iterator end() const {
        return const_iterator(&fakeNode_);
    }

    const_iterator cend() const {
        return const_iterator(&fakeNode_);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

     void swap(List& other) {
        if (AllocTraits::propagate_on_container_swap::value){
            std::swap(alloc_, other.alloc);
            std::swap(listSize_, other.listSize_);
            fakeNode_;
            BaseNode* other_first = other.fakeNode_.next;
            BaseNode* other_last = other.fakeNode_.prev;
            if (fakeNode_.next == &fakeNode_){
                other.fakeNode_.next = &other.fakeNode_;
                other.fakeNode_.prev = &other.fakeNode_;
            }
            else{
                other.fakeNode_.next = fakeNode_.next;
                other.fakeNode_.prev = fakeNode_.prev;
            }
            if (&other.fakeNode_ == other_first){
                fakeNode_.next = &fakeNode_;
                fakeNode_.prev = &fakeNode_;
            }
            else{
                fakeNode_.next = other_first;
                fakeNode_.prev = other_last;
            }           
        }
        
    }

    ~List() {
        BaseNode* node = fakeNode_.next;
        for (size_t i = 0; i < listSize_; ++i) {
            BaseNode* nextNode = node->next;
            NodeAllocTraits::destroy(alloc_, static_cast<Node*>(node));
            NodeAllocTraits::deallocate(alloc_, static_cast<Node*>(node), 1);
            node = nextNode;
        }
    }
};

template <typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
    class UnorderedMap {
    public:
        using NodeType = std::pair<const Key, Value>;
        using iterator = typename List<NodeType, Allocator>::iterator;
        using const_iterator = typename List<NodeType, Allocator>::const_iterator;
    private:
        using AllocTraits = std::allocator_traits<Allocator>;
        using IteratorAlloc = typename AllocTraits::template rebind_alloc<iterator>;

        List<NodeType, Allocator> nodes_;
        std::vector<iterator, IteratorAlloc>storage_;
        double max_load_factor_ = 1;

        [[no_unique_address]] Hash hasher_;
        [[no_unique_address]] Equal equality_;
        [[no_unique_address]] Allocator alloc_;

        size_t get_hash(const Key& key) const {
            if (storage_.empty()) {
                return 0;
            }
            return hasher_(key) % storage_.size();
        }

        void clear(){
            while (size() > 0){
                erase();
            }
        }

    public:
        UnorderedMap() : storage_(std::vector<iterator, IteratorAlloc>(1, nodes_.end())) {}

        UnorderedMap(const UnorderedMap& other) : storage_(std::vector<iterator, IteratorAlloc>(1, nodes_.end())),
            max_load_factor_(other.max_load_factor_), hasher_(other.hasher_), equality_(other.equality_),
            alloc_(AllocTraits::select_on_container_copy_construction(other.alloc_)) {
            auto iter = other.nodes_.begin();
            for (size_t i = 0; i < other.size(); ++i, ++iter) {
                insert(*iter);
            }
        }

        UnorderedMap(UnorderedMap&& other) = default;

        UnorderedMap& operator=(const UnorderedMap& other) {
            if (&other == this) {
                return *this;
            }
            if (AllocTraits::propagate_on_container_copy_assignment::value) {
                alloc_ = other.alloc_;
            }
            max_load_factor_ = other.max_load_factor_;
            nodes_ = other.nodes_;
            hasher_ = other.hasher_;
            equality_ = other.equality_;
            clear();
            storage_.clear();
            storage_.resize(1, nodes_.end());
            auto iter = other.nodes_.begin();
            for (size_t i = 0; i < other.nodes_.size(); ++i, ++iter) {
                insert(*iter);
            }
            return *this;
        }

        UnorderedMap& operator=(UnorderedMap&& other) {
            max_load_factor_ = other.max_load_factor_;
            if (AllocTraits::propagate_on_container_move_assignment::value) {
                alloc_ = std::move(other.alloc_);
            }
            size_t other_size_stor = other.storage_.size();
            nodes_ = std::move(other.nodes_);
            hasher_ = std::move(other.hasher_);
            equality_ = std::move(other.equality_);
            storage_ = std::move(other.storage_);
            reserve(other_size_stor);
            return *this;
        }

        size_t size() const {
            return nodes_.size();
        }

        iterator begin() {
            return nodes_.begin();
        }

        const_iterator begin() const {
            return nodes_.cbegin();
        }

        const_iterator cbegin() const {
            return nodes_.cbegin();
        }

        iterator end() {
            return nodes_.end();
        }

        const_iterator end() const {
            return nodes_.cend();
        }

        const_iterator cend() const {
            return nodes_.cend();
        }

        template<bool = true>
        std::pair<iterator, bool> insert(const NodeType& node_val) {
            return emplace(node_val);
        }

        std::pair<iterator, bool> insert(std::pair<Key, Value>&& node_val) {
            return emplace(std::move(node_val));
        }

        template<bool = true>
        std::pair<iterator, bool> insert(NodeType&& node_val) {
            return emplace(std::move(node_val.first), std::move(node_val.second));
        }

        Value& operator[](const Key& key) {
            auto iter = find(key);
            if (iter == nodes_.end()) {
                return emplace(key, Value()).first->second;
            }
            return iter->second;
        }

        Value& operator[](Key&& key) {
            auto iter = find(key);
            if (iter == nodes_.end()) {
                return emplace(std::move(key), std::move(Value())).first->second;
            }
            return iter->second;
        }

        Value& at(const Key& key) {
            auto iter = find(key);
            if (iter == nodes_.end()) {
                throw std::out_of_range("there are no elements with this key");
            }
            return iter->second;
        }

        const Value& at(const Key& key) const {
            auto iter = find(key);
            if (iter == nodes_.end()) {
                throw std::out_of_range("there are no elements with this key");
            }
            return iter->second;
        }

        template<typename Iter>
        void insert(const Iter& iter1, const Iter& iter2) {
            for (Iter iter = iter1; iter != iter2; ++iter) {
                insert(*iter);
            }
        }

        void erase(const iterator& iter) {
            size_t hash = get_hash(iter->first);
            if (storage_[hash] != iter) {
                nodes_.erase(iter);
                return;
            }
            if ((std::next(iter, 1) != end()) && get_hash(std::next(iter, 1)->first) == hash) {
                storage_[hash] = std::next(iter, 1);
                nodes_.erase(iter);
                return;
            }
            storage_[hash] = nodes_.end();
            nodes_.erase(iter);
        }

        void erase(const iterator& iter1, const iterator& iter2) {
            iterator iter = iter1;
            while (iter != iter2) {
                iterator next = std::next(iter, 1);
                erase(iter);
                iter = next;
            }
        }

        iterator find(const Key& key) {
            size_t hash = get_hash(key);
            iterator iter = storage_[hash];
            while (iter != end() && (get_hash(iter->first) == hash)) {
                if (equality_(iter->first, key)) {
                    return iter;
                }
                ++iter;
            }
            return nodes_.end();
        }

        iterator find(const Key& key) const {
            size_t hash = get_hash(key);
            iterator iter = storage_[hash];
            while (iter != end() && get_hash(iter->first) == hash) {
                if (equality_(iter->first, key)) {
                    return iter;
                }
                ++iter;
            }
            return nodes_.end();
        }

        template <typename ...Args>
        std::pair<iterator, bool> emplace(Args&&... args) {
            auto node_ptr = nodes_.create_node(std::forward<Args>(args)...);
            const Key& key = node_ptr->value.first;
            Value& value = node_ptr->value.second;
            iterator iter = find(node_ptr->value.first);
            if (iter != nodes_.end()) {
                nodes_.destroy_node(node_ptr);
                return { iter, false };
            }
            if (load_factor() >= max_load_factor()) {
                reserve(nodes_.size() / max_load_factor_ * 2 + 1);
            }
            size_t hash = get_hash(node_ptr->value.first);
            if (storage_[hash] != nodes_.end()) {
                nodes_.emplace(storage_[hash], node_ptr);
                storage_[hash]--;
                return { storage_[hash], true };
            }
            nodes_.emplace(end(), node_ptr);
            storage_[hash] = std::prev(nodes_.end(), 1);
            return { storage_[hash], true };
        }

        double load_factor() const {
            return nodes_.size() / storage_.size();
        }

        double max_load_factor() const {
            return max_load_factor_;
        }

        void max_load_factor(double mlf) {
            max_load_factor_ = mlf;
            if (nodes_.size() / storage_.size() >= max_load_factor_) {
                reserve(nodes_.size() / max_load_factor_ * 2 + 1);
            }
        }

        void reserve(size_t count) {
            using PairAlloc = typename AllocTraits::template rebind_alloc<std::pair<Key, Value>>;
            std::vector<std::pair<Key, Value>, PairAlloc>values;
            while (nodes_.size() > 0) {
                values.push_back({std::move(*const_cast<Key*>(&nodes_.begin()->first)), 
                    std::move(nodes_.begin()->second)});
                nodes_.erase(nodes_.begin());
            }
            storage_.clear();
            storage_.resize(count, nodes_.end());
            for (size_t i = 0; i < values.size(); ++i) {
                emplace(std::move(values[i].first), std::move(values[i].second));
            }
        }

        void swap(UnorderedMap& other) {
            if (AllocTraits::propagate_on_container_swap::value){
                std::swap(max_load_factor_, other.max_load_factor_);
                std::swap(hasher_, other.hasher_);
                std::swap(equality_, other.equality_);
                std::swap(storage_, other.storage_);
                std::swap(alloc_, other.alloc_);
                nodes_.swap(other.nodes_);
            }
            std::swap(max_load_factor_, other.max_load_factor_);
            std::swap(nodes_, other.nodes_);
            std::swap(hasher_, other.hasher_);
            std::swap(equality_, other.equality_);
            std::swap(storage_, other.storage_);
        }

        ~UnorderedMap() = default;
};