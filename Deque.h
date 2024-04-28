#include <iostream>
#include <iterator>

template<typename T>
class Deque {
private:
    static const size_t BlockSize_ = 16;
    size_t capacity_ = 0;
    T** pointers_;
public:
    template<bool IsConst>
    class CommonIterator {
    private:
        T** block_;
        size_t indInBlock_;

    public:
        CommonIterator() = default;
        CommonIterator(T** block_, size_t indInBlock_) : block_(block_), indInBlock_(indInBlock_) {}
        CommonIterator(const CommonIterator& iter) = default;

        using difference_type = int;
        using value_type = T;
        using iterator_category = std::random_access_iterator_tag;

        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

        CommonIterator& operator=(const CommonIterator& other) {
            block_ = other.block_;
            indInBlock_ = other.indInBlock_;
            return *this;
        }

        CommonIterator& operator++() {
            if (indInBlock_ < BlockSize_ - 1) {
                ++indInBlock_;
                return *this;
            }
            indInBlock_ = 0;
            ++block_;
            return *this;
        }

        CommonIterator& operator--() {
            if (indInBlock_ > 0) {
                --indInBlock_;
                return *this;
            }
            indInBlock_ = BlockSize_ - 1;
            --block_;
            return *this;
        }

        CommonIterator operator++(int) {
            CommonIterator copy = *this;
            ++* this;
            return copy;
        }

        CommonIterator operator--(int) {
            CommonIterator copy = *this;
            --* this;
            return copy;
        }

        CommonIterator& operator+=(int value) {
            value += indInBlock_;
            block_ += (value / static_cast<int>(BlockSize_));
            value %= static_cast<int>(BlockSize_);
            if (value < 0) {
                --block_;
                value += BlockSize_;
            }
            indInBlock_ = value;
            return *this;
        }

        CommonIterator& operator-=(int value) {
            return *this += (-value);
        }

        CommonIterator operator+(int value) const {
            CommonIterator copy = *this;
            copy += value;
            return copy;
        }

        CommonIterator operator-(int value) const {
            CommonIterator copy = *this;
            copy -= value;
            return copy;
        }

        difference_type operator-(const CommonIterator& other) const {
            return (block_ - other.block_) * BlockSize_ + indInBlock_ - static_cast<int>(other.indInBlock_);
        }

        reference operator*() const {
            return (*block_)[indInBlock_];
        }

        pointer operator->() const {
            return (*block_ + indInBlock_);
        }

        operator CommonIterator<true>() const {
            return CommonIterator<true>(block_, indInBlock_);
        }

        bool operator==(const CommonIterator& other) const {
            return (*this - other) == 0;
        }

        auto operator<=>(const CommonIterator& other) const {
            return (*this - other) <=> 0;
        }
    };

    using iterator = CommonIterator<false>;
    using const_iterator = CommonIterator<true>;

    iterator begin() {
        return beginIter_;
    }

    const_iterator begin() const {
        return const_iterator(beginIter_);
    }

    const_iterator cbegin() const {
        return const_iterator(beginIter_);
    }

    iterator end() {
        return endIter_;
    }

    const_iterator end() const {
        return const_iterator(endIter_);
    }

    const_iterator cend() const {
        return const_iterator(endIter_);
    }

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

private:
    iterator beginIter_;
    iterator endIter_;

    void reserve(size_t left, size_t right) {
        T** new_pointers = new T * [left + capacity_ + right];
        for (size_t i = 0; i < left; ++i) {
            new_pointers[i] = reinterpret_cast<T*>(new char[BlockSize_ * sizeof(T)]);
        }
        for (size_t i = 0; i < right; ++i) {
            new_pointers[left + capacity_ + i] = reinterpret_cast<T*>(new char[BlockSize_ * sizeof(T)]);
        }
        for (size_t i = 0; i < capacity_; ++i) {
            new_pointers[left + i] = pointers_[i];
        }
        capacity_ += left + right;
        size_t indexBegin = beginIter_ - iterator(pointers_, 0);
        size_t indexEnd = indexBegin + size();
        beginIter_ = iterator(&new_pointers[left + indexBegin / BlockSize_], indexBegin % BlockSize_);
        endIter_ = iterator(&new_pointers[left + indexEnd / BlockSize_], indexEnd % BlockSize_);
        delete[] pointers_;
        pointers_ = new_pointers;
    }

    void create(size_t cnt) {
        for (size_t i = 0; i < capacity_; ++i) {
            pointers_[i] = reinterpret_cast<T*>(new char[BlockSize_ * sizeof(T)]);
        }
        beginIter_ = iterator(&pointers_[0], 0);
        endIter_ = iterator(&pointers_[capacity_ - 1], cnt % BlockSize_);
    }

    void destroy() {
        for (size_t i = 0; i < capacity_; ++i) {
            delete[] reinterpret_cast<char*>(pointers_[i]);
        }
        delete[] pointers_;
    }

public:
    Deque() : capacity_(1), pointers_(new T* [capacity_]) {
        create(0);
    }

    Deque(size_t cnt) : capacity_((cnt + BlockSize_) / BlockSize_), pointers_(new T* [capacity_]) {
        create(cnt);
        for (iterator iter = beginIter_; iter != endIter_; ++iter) {
            try {
                new (iter.operator->()) T();
            }
            catch (...) {
                while (iter > beginIter_) {
                    --iter;
                    (iter.operator->())->~T();
                }
                endIter_ = beginIter_;
                destroy();
                throw;
            }
        }
    }

    Deque(size_t cnt, const T& value) : capacity_((cnt + BlockSize_) / BlockSize_), pointers_(new T* [capacity_]) {
        create(cnt);
        for (iterator iter = beginIter_; iter != endIter_; ++iter) {
            try {
                new (iter.operator->()) T(value);
            }
            catch (...) {
                while (iter > beginIter_) {
                    --iter;
                    (iter.operator->())->~T();
                }
                endIter_ = beginIter_;
                destroy();
                throw;
            }
        }
    }

    Deque(const Deque& other) : capacity_((other.size() + BlockSize_) / BlockSize_), pointers_(new T* [capacity_]) {
        create(other.size());
        for (size_t i = 0; i < other.size(); ++i) {
            try {
                new ((beginIter_ + i).operator->()) T(*(other.beginIter_ + i));
            }
            catch (...) {
                iterator iter = beginIter_ + i;
                while (iter > beginIter_) {
                    --iter;
                    (iter.operator->())->~T();
                }
                endIter_ = beginIter_;
                destroy();
                throw;
            }
        }
    }

    Deque& operator=(const Deque& other) {
        Deque copy = other;
        swap(copy);
        return *this;
    }

    void swap(Deque& other) {
        std::swap(pointers_, other.pointers_);
        std::swap(capacity_, other.capacity_);
        std::swap(beginIter_, other.beginIter_);
        std::swap(endIter_, other.endIter_);
    }

    size_t size() const {
        return endIter_ - beginIter_;
    }

    T& operator[](size_t index) {
        return *(beginIter_ + index);
    }

    const T& operator[](size_t index) const {
        return *(const_iterator(beginIter_ + index));
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("index out of range");
        }
        return *(beginIter_ + index);
    }

    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("index out of range");
        }
        return *(const_iterator(beginIter_ + index));
    }

    void push_back(const T& value) {
        try {
            if (endIter_ == iterator(&pointers_[capacity_ - 1], BlockSize_ - 1)) {
                reserve(0, capacity_);
            }
            new (endIter_.operator->()) T(value);
        }
        catch (...) {
            throw;
        }
        ++endIter_;
    }

    void push_front(const T& value) {
        try {
            if (beginIter_ == iterator(&pointers_[0], 0)) {
                reserve(capacity_, 0);
            }
            new ((beginIter_ - 1).operator->()) T(value);
        }
        catch (...) {
            throw;
        }
        --beginIter_;
    }

    void pop_back() {
        --endIter_;
        (endIter_.operator->())->~T();
    }

    void pop_front() {
        (beginIter_.operator->())->~T();
        ++beginIter_;
    }

    void insert(iterator iter, const T& value) {
        size_t index = iter - beginIter_;
        push_back(value);
        size_t ind = size() - 1;
        try {
            for (; ind != index; --ind) {
                (*this)[ind] = (*this)[ind - 1];
            }
            (*this)[index] = value;
        }
        catch (...) {
            for (; ind < size() - 1; ++ind) {
                (*this)[ind] = (*this)[ind + 1];
            }
            pop_back();
            throw;
        }
    }

    void erase(iterator iter) {
        size_t index = iter - beginIter_;
        T value = *iter;
        size_t ind = index;
        try {
            for (; ind != size() - 1; ++ind) {
                (*this)[ind] = (*this)[ind + 1];
            }
            pop_back();
        }
        catch (...) {
            for (; ind != index; --ind) {
                (*this)[ind] = (*this)[ind - 1];
            }
            (*this)[index] = value;
            throw;
        }
    }

    ~Deque() {
        for (iterator iter = beginIter_; iter != endIter_; ++iter) {
            (iter.operator->())->~T();
        }
        destroy();
    }
};