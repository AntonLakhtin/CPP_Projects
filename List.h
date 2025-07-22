#include <iostream>
#include <memory>
#include <vector>

template<size_t N>
class StackStorage {
private:
    char storage[N];
    size_t top = 0;
public:
    char* get_storage() const{
        return storage;
    }

    char* get_storage() {
        return storage;
    }

    size_t& get_top() {
        return top;
    }

    const size_t& get_top() const{
        return top;
    }
};

template<typename T, size_t N>
class StackAllocator {
private:
    StackStorage<N>* storage = nullptr;
    template<typename U, size_t K>
    friend class StackAllocator;
public:
    StackAllocator() = default;
    StackAllocator(StackStorage<N>& stackStor) : storage(&stackStor) {}

    template<typename U>
    StackAllocator(const StackAllocator<U, N>& other) : storage(other.storage) {}

    template<typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        storage = other.storage;
        return *this;
    };

    using value_type = T;

    template<typename U>
    struct rebind { typedef StackAllocator<U, N>other; };

    T* allocate(size_t cnt) const {
        size_t last = N - storage->get_top();
        char* copy_top = storage->get_storage() + storage->get_top();
        std::align(alignof(T), sizeof(T), reinterpret_cast<void*&>(copy_top), last);
        storage->get_top() += cnt * sizeof(T);
        return reinterpret_cast<T*>(copy_top);
    }

    void deallocate(T*, size_t) const {}

    template<typename U>
    bool operator==(const StackAllocator& other) const {
        return storage == other.storage;
    }

    template<typename U>
    bool operator!=(const StackAllocator& other) const {
        return storage != other.storage;
    }

    ~StackAllocator() = default;
};


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

    BaseNode fakeNode;
    size_t listSize = 0;

    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

    [[no_unique_address]] NodeAllocator alloc;

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
        BaseNode* ptr = nullptr;
    public:
        using value_type = T;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ssize_t;

        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

        CommonIterator() = default;
        CommonIterator(const CommonIterator& other) = default;
        CommonIterator(BaseNode* ptr) : ptr(ptr) {}
        CommonIterator<true>(const BaseNode* ptr1) : ptr(const_cast<BaseNode*>(ptr1)) {}

        CommonIterator& operator=(const CommonIterator& other) = default;

        BaseNode* getNodePtr() const {
            return ptr;
        }

        CommonIterator& operator++() {
            ptr = ptr->next;
            return *this;
        }

        CommonIterator operator++(int) {
            CommonIterator copy = *this;
            ++*this;
            return copy;
        }

        CommonIterator& operator--() {
            ptr = ptr->prev;
            return *this;
        }

        CommonIterator operator--(int) {
            CommonIterator copy = *this;
            --*this;
            return copy;
        }

        reference operator*() const {
            return static_cast<Node*>(ptr)->value;
        }

        pointer operator->() const {
            return &static_cast<Node*>(ptr)->value;
        }

        operator CommonIterator<true>() const {
            return CommonIterator<true>(ptr);
        }

        bool operator==(const CommonIterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const CommonIterator& other) const {
            return ptr != other.ptr;
        }

        CommonIterator next() const {
            return CommonIterator(ptr->next);
        }

        CommonIterator prev() const {
            return CommonIterator(ptr->prev);
        }
    };

    using iterator = CommonIterator<false>;
    using const_iterator = CommonIterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() = default;

    List(size_t cnt) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                insert(end());
            }catch(...){
                while (listSize > 0){
                    erase(begin());
                }
                throw;
            }
        }
    }

    List(size_t cnt, const T& value) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                insert(end(), value);
            }catch(...){
                while (listSize > 0){
                    erase(begin());
                }
                throw;
            }
        }
    }

    List(const Allocator& alloc) : alloc(alloc) {}

    List(size_t cnt, const Allocator& alloc1) : List(alloc1) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                insert(end());
            }catch(...){
                while (listSize > 0){
                    erase(begin());
                }
                throw;
            }
        }
    }

    List(size_t cnt, const T& value, const Allocator& alloc1) : List(alloc1) {
        for (size_t i = 0; i < cnt; ++i) {
            try{
                insert(end(), value);
            }catch(...){
                while (listSize > 0){
                    erase(begin());
                }
                throw;
            }
        }
    }

    List(const List& other) : alloc(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
        auto otherIter = other.begin();
        for (size_t i = 0; i < other.listSize; ++i, ++otherIter) {
            try{
                insert(end(), *otherIter);
            }catch(...){
                while (listSize > 0){
                    erase(begin());
                }
                throw;
            }
        }
    }

    List& operator=(const List& other) {
        auto copyAlloc = alloc;
        size_t sz = listSize;
        alloc = other.alloc;
        auto otherIter = other.begin();
        for (size_t i = 0; i < other.listSize; ++i, ++otherIter){
            try{
                push_back(*otherIter);
            } catch(...){
                while (i > 0){
                    pop_back();
                    --i;
                }
                throw;
            }
        }
        alloc = copyAlloc;
        for (size_t i = 0; i < sz; ++i) {
            pop_front();
        }
        listSize = other.listSize;
        if (NodeAllocTraits::propagate_on_container_copy_assignment::value){
            alloc = other.alloc;
        }
        return *this;
    }

    Allocator get_allocator() const {
        return alloc;
    }

    size_t size() const {
        return listSize;
    }

    template<typename ...Args>
    void insert(const_iterator iter, const Args& ...args) {
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        try{
            NodeAllocTraits::construct(alloc, &newNode->value, args...);
        } catch(...){
            NodeAllocTraits::deallocate(alloc, newNode, 1);
            throw;
        }
        listSize++;
        add(iter.getNodePtr()->prev, static_cast<BaseNode*>(newNode), iter.getNodePtr());
    }

    void insert(const_iterator iter, const T& value) { 
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        try{
            NodeAllocTraits::construct(alloc, &newNode->value, value);
        } catch(...){
            NodeAllocTraits::deallocate(alloc, newNode, 1);
            throw;
        }
        listSize++;
        add(iter.getNodePtr()->prev, static_cast<BaseNode*>(newNode), iter.getNodePtr());
    }

    void erase(const_iterator iter) {
        --listSize;
        connect(iter.getNodePtr()->prev, iter.getNodePtr()->next);
        NodeAllocTraits::destroy(alloc, static_cast<Node*>(iter.getNodePtr()));
        NodeAllocTraits::deallocate(alloc, static_cast<Node*>(iter.getNodePtr()), 1);
    }

    void push_back(const T& value) {
        insert(end(), value);
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void pop_front() {
        erase(begin());
    }

    void pop_back() {
        erase(end().prev());
    }

    iterator begin() {
        return iterator(fakeNode.next);
    }

    const_iterator begin() const {
        return const_iterator(fakeNode.next);
    }

    const_iterator cbegin() const {
        return const_iterator(fakeNode.next);
    }

    iterator end() {
        return iterator(&fakeNode);
    }

    const_iterator end() const {
        return const_iterator(&fakeNode);
    }

    const_iterator cend() const {
        return const_iterator(&fakeNode);
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

    ~List() {
        BaseNode* node = fakeNode.next;
        for (size_t i = 0; i < listSize; ++i) {
            BaseNode* nextNode = node->next;
            NodeAllocTraits::destroy(alloc, static_cast<Node*>(node));
            NodeAllocTraits::deallocate(alloc, static_cast<Node*>(node), 1);
            node = nextNode;
        }
    }
};