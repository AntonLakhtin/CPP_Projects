#include <iostream>
#include <memory>

template<typename T>
class SharedPtr;
template<typename T>
class WeakPtr;

template<typename T>
class EnableSharedFromThis {
private:
    WeakPtr<T> weak_ptr;

    template<typename U>
    friend class SharedPtr;
protected:
    EnableSharedFromThis() = default;
public:
    SharedPtr<T> shared_from_this(){
        if (weak_ptr.expired()){
            throw std::bad_exception();
        }
        return weak_ptr.lock(); 
    }
};

struct BaseControlBlock{
    size_t shared_counter = 0;
    size_t weak_counter = 0;
    virtual void delete_obj() = 0;
    virtual void delete_block() = 0;
    BaseControlBlock() = default;
    virtual ~BaseControlBlock() = default;
};

template<typename U, typename Deleter = std::default_delete<U>, typename Allocator = std::allocator<U>>
struct ControlBlockPtr : BaseControlBlock {
    U* pointer = nullptr;
    [[no_unique_address]] Allocator alloc;
    [[no_unique_address]] Deleter deleter;
    ControlBlockPtr() = default;
    ControlBlockPtr(U* pointer) : pointer(pointer) {}
    ControlBlockPtr(U* pointer, Deleter del) : pointer(pointer), deleter(del) {}
    ControlBlockPtr(U* pointer, Deleter del, Allocator alloc) : pointer(pointer), alloc(alloc), deleter(del) {}

    void delete_obj(){
        deleter(pointer);
    }

    void delete_block(){
        using BlockAllocator = typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlockPtr>;
        using BlockAllocTraits = std::allocator_traits<BlockAllocator>;
        BlockAllocator block_alloc;
        BlockAllocTraits::deallocate(block_alloc, this, 1);
    }

    ~ControlBlockPtr() {};
}; 

template<typename U, typename Allocator = std::allocator<U>>
struct ControlBlockObject : BaseControlBlock{
    U object;
    [[no_unique_address]] Allocator alloc;
    ControlBlockObject() = default;
    ControlBlockObject(const Allocator& alloc) : alloc(alloc) {}
    template<typename... Args>
    ControlBlockObject(Allocator alloc, Args&&... args) : object(std::forward<Args>(args)...), alloc(alloc) {}
    void delete_obj(){
         std::allocator_traits<Allocator>::destroy(alloc, &object);
    }

    void delete_block(){
        using BlockAllocator = typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlockObject>;
        using BlockAllocTraits = std::allocator_traits<BlockAllocator>;
        BlockAllocator block_alloc;
        BlockAllocTraits::deallocate(block_alloc, this, 1);
    }

    ~ControlBlockObject() {};
};

template<typename T>
class SharedPtr{
private:
    T* pointer = nullptr;
    BaseControlBlock* control_block = nullptr;

    template<typename U>
    friend class WeakPtr;

    template<typename U>
    friend class EnableSharedFromThis;

    template<typename U>
    friend class SharedPtr;

    template<typename U, typename Allocator, typename... Args>
    friend SharedPtr<U> allocateShared(const Allocator& alloc, Args&&... args);

    SharedPtr(T* pointer, BaseControlBlock* control_block) : pointer(pointer), control_block(control_block) {
        control_block->shared_counter += 1;
    }
public:
    SharedPtr() = default;

    template<typename U, typename Deleter = std::default_delete<U>, typename Allocator = std::allocator<U>>
    explicit SharedPtr(U* ptr, Deleter del = Deleter(), Allocator alloc = Allocator()) : pointer(static_cast<T*>(ptr)) {
        using BlockAllocator = typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlockPtr<U, Deleter, Allocator>>;
        using BlockAllocTraits = std::allocator_traits<BlockAllocator>;
        BlockAllocator block_alloc;
        ControlBlockPtr<U, Deleter, Allocator>* block = BlockAllocTraits::allocate(block_alloc, 1);
        new (block) ControlBlockPtr<U, Deleter, Allocator>(ptr, del, alloc);
        control_block = static_cast<BaseControlBlock*>(block);
        control_block->shared_counter = 1;
        if constexpr (std::is_base_of_v<EnableSharedFromThis<U>, U>){
            ptr->weak_ptr = *this;
        }
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& other, U* ptr) : pointer(static_cast<T*>(ptr)), control_block(other.control_block) {
        if (control_block == nullptr){
            return;
        }
        control_block->shared_counter += 1;
    }

    SharedPtr(const SharedPtr& other) : pointer(other.pointer), control_block(other.control_block){
        if (control_block == nullptr){
            return;
        }
        control_block->shared_counter++;
    }

    template<typename U>
    SharedPtr<T>(const SharedPtr<U>& other) : pointer(static_cast<T*>(other.pointer)), control_block(other.control_block){
        if (control_block == nullptr){
            return;
        }
        control_block->shared_counter++;
    }

    template<typename U>
    SharedPtr(SharedPtr<U>&& other) : pointer(static_cast<T*>(std::move(other.pointer))), control_block(std::move(other.control_block)) {
        other.control_block = nullptr;
        other.pointer = nullptr;
    }

    SharedPtr(SharedPtr&& other) : pointer(std::move(other.pointer)), control_block(std::move(other.control_block)) {
        other.control_block = nullptr;
        other.pointer = nullptr;
    }

    SharedPtr& operator=(const SharedPtr& other){
        SharedPtr copy = other;
        swap(copy);
        return *this;
    }

    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& other){
        SharedPtr copy = other;
        swap(copy);
        return *this;
    }

    void swap(SharedPtr& other){
        std::swap(control_block, other.control_block);
        std::swap(pointer, other.pointer);
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    template<typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SharedPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    size_t use_count() const{
        if (control_block == nullptr){
            return 0;
        }
        return control_block->shared_counter;
    }

    T* get() const{
        return pointer;
    }

    T& operator*() const{
        return *pointer;
    }

    T* operator->() const {
        return pointer;
    }

    template<typename... Args>
    void reset(Args&&... args){
        *this = SharedPtr(std::forward<Args>(args)...);
    }

    ~SharedPtr() {
        if (control_block == nullptr){
            return;
        }
        control_block->shared_counter--;
        if (control_block->shared_counter == 0){
            if (control_block->weak_counter == 0){
                control_block->delete_obj();
                control_block->delete_block();
                return;
            }
            control_block->delete_obj();
        }
    }
};

template<typename T, typename Allocator, typename... Args>
SharedPtr<T> allocateShared(const Allocator& alloc, Args&&... args) {
    using BlockAllocator = typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlockObject<T, Allocator>>;
    using BlockAllocTraits = std::allocator_traits<BlockAllocator>;
    BlockAllocator block_alloc;
    ControlBlockObject<T, Allocator>* block = BlockAllocTraits::allocate(block_alloc, 1);
    BlockAllocTraits::construct(block_alloc, block, alloc, std::forward<Args>(args)...);
    return SharedPtr<T>(&block->object, static_cast<BaseControlBlock*>(block));
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    std::allocator<T> alloc;
    return allocateShared<T>(alloc, std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr{
private:
    T* pointer = nullptr;
    BaseControlBlock* control_block = nullptr;

    template<typename U>
    friend class WeakPtr;
public:
    WeakPtr() = default;

    template<typename U, typename Deleter = std::default_delete<U>, typename Allocator = std::allocator<U>>
    explicit WeakPtr(U* ptr, Deleter del = Deleter(), Allocator alloc = Allocator()) : pointer(static_cast<T*>(ptr)) {
        using BlockAllocator = typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlockPtr<U, Deleter, Allocator>>;
        using BlockAllocTraits = std::allocator_traits<BlockAllocator>;
        BlockAllocator block_alloc;
        ControlBlockPtr<U, Deleter, Allocator>* block = BlockAllocTraits::allocate(block_alloc, 1);
        new (block) ControlBlockPtr<U, Deleter, Allocator>(ptr, del, alloc);
        control_block = static_cast<BaseControlBlock*>(block);
        control_block->weak_counter = 1;
    }

    template<typename U>
    WeakPtr(const WeakPtr& other, U* ptr) : pointer(static_cast<T*>(ptr)) {
        control_block = other.control_block;
        if (control_block == nullptr){
            return;
        }
        control_block->weak_counter += 1;
    }

    template<typename U>
    WeakPtr(const WeakPtr<U>& ptr) : pointer(static_cast<T*>(ptr.pointer)), control_block(ptr.control_block) {
        if (control_block == nullptr){
            return;
        }
        control_block->weak_counter += 1;
    }

    WeakPtr(const WeakPtr& other) : pointer(other.pointer), control_block(other.control_block){
        if (control_block == nullptr){
            return;
        }
        control_block->weak_counter++;
    }

    template<typename U>
    WeakPtr(WeakPtr<U>&& other) : pointer(static_cast<T*>(other.pointer)), control_block(other.control_block) {
        other.control_block = nullptr;
        other.pointer = nullptr;
    }

    WeakPtr(WeakPtr&& other) : pointer(other.pointer), control_block(other.control_block) {
        other.control_block = nullptr;
        other.pointer = nullptr;
    }

    WeakPtr(const SharedPtr<T>& sh_ptr) : pointer(sh_ptr.pointer), control_block(sh_ptr.control_block) {
        control_block->weak_counter++;
    }

    template<typename U>
    WeakPtr(const SharedPtr<U>& sh_ptr) : pointer(static_cast<T*>(sh_ptr.pointer)), control_block(sh_ptr.control_block) {
        control_block->weak_counter++;
    }

    WeakPtr& operator=(const WeakPtr& other){
        WeakPtr copy = other;
        swap(copy);
        return *this;
    }

    template<typename U>
    WeakPtr& operator=(const WeakPtr<U>& other){
        WeakPtr copy = other;
        swap(copy);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    template<typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        WeakPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    bool expired() const{
        if (control_block == nullptr){
            return true;
        }
        return control_block->shared_counter == 0;
    }

    SharedPtr<T> lock() const {
        return SharedPtr<T>(pointer, control_block);
    } 

    void swap(WeakPtr& other){
        std::swap(control_block, other.control_block);
        std::swap(pointer, other.pointer);
    }

    T* get() const{
        return pointer;
    }

    T& operator*() const{
        return *pointer;
    }

    T* operator->() const {
        return pointer;
    }

    size_t use_count() const{
        if (control_block == nullptr){
            return 0;
        }
        return control_block->shared_counter;
    }

    ~WeakPtr() {
        if (control_block == nullptr){
            return;
        }
        control_block->weak_counter--;
        if (control_block->shared_counter == 0){
            if (control_block->weak_counter == 0){
                control_block->delete_block();
            }
        }
    }
};