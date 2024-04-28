#include <iostream>
#include <cstring>

class String {
private:
    char* string;
    size_t str_size;
    size_t str_capacity;

    void resize(size_t new_capacity) {
        char* new_string = new char[new_capacity];
        memcpy(new_string, string, str_size);
        delete[] string;
        string = new_string;
        str_capacity = new_capacity;
    }

    bool are_equal_substrings(size_t start, const String& substring) const {
        for (size_t i = 0; i < substring.size(); ++i) {
            if (string[start + i] != substring[i]) {
                return false;
            }
        }
        return true;
    }

public:
    String(size_t length) : string(new char[length + 1]), str_size(length + 1), str_capacity(length + 1) {
        string[str_size] = '\0';
    }

    String(const char* str) : String(strlen(str)) {
        memcpy(string, str, str_size);
    }

    String(int length, char x) : String(length) {
        memset(string, x, length);
    }

    String() : String(static_cast<size_t>(0)) {}

    String(const String& str) : string(new char[str.str_capacity]), str_size(str.str_size), str_capacity(str.str_capacity) {
        memcpy(string, str.string, str_size);
    }

    String& operator=(const String& other) {
        if (str_size > other.size()) {
            memcpy(string, other.string, other.size() + 1);
            str_size = other.str_size;
            return *this;
        }
        String copy = other;
        swap(copy);
        return *this;
    }

    void swap(String& other) {
        std::swap(str_size, other.str_size);
        std::swap(str_capacity, other.str_capacity);
        std::swap(string, other.string);
    }

    const char& operator[](size_t index) const {
        return string[index];
    }

    char& operator[](size_t index) {
        return string[index];
    }

    size_t length() const {
        return str_size - 1;
    }

    size_t size() const {
        return str_size - 1;
    }

    size_t capacity() const {
        return str_capacity - 1;
    }

    void pop_back() {
        --str_size;
        string[str_size - 1] = '\0';
    }

    void push_back(char chr) {
        if (str_size == str_capacity) {
            resize(str_capacity * 2);
        }
        string[str_size - 1] = chr;
        string[str_size] = '\0';
        ++str_size;
    }

    const char& front() const {
        return string[0];
    }

    char& front() {
        return string[0];
    }

    const char& back() const {
        return string[str_size - 2];
    }

    char& back() {
        return string[str_size - 2];
    }

    String& operator+=(char chr) {
        push_back(chr);
        return *this;
    }

    String& operator+=(const String& other) {
        if (str_size + other.size() > str_capacity) {
            resize(str_size + other.size());
        }
        memcpy(string + str_size - 1, other.string, other.size() + 1);
        str_size += other.size();
        return *this;
    }

    size_t find(const String& substring) const {
        for (size_t i = 0; i + substring.size() <= size(); ++i) {
            if (are_equal_substrings(i, substring)) {
                return i;
            }
        }
        return length();
    }

    size_t rfind(const String& substring) const {
        int ans = length();
        for (size_t i = 0; i + substring.size() <= size(); ++i) {
            if (are_equal_substrings(i, substring)) {
                ans = i;
            }
        }
        return ans;
    }

    String substr(size_t start, size_t count) const {
        String new_string(count);
        memcpy(new_string.string, string + start, count);
        return new_string;
    }

    bool empty() const {
        return size() == 0;
    }

    void clear() {
        str_size = 1;
        string[0] = '\0';
    }

    String& shrink_to_fit() {
        resize(str_size);
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& out, const String& str) {
        out << str.string;
        return out;
    }

    const char* data() const {
        return string;
    }

    char* data() {
        return string;
    }

    ~String() {
        delete[] string;
    }
};

bool operator==(const String& first, const String& second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (size_t i = 0; i < first.size(); ++i) {
        if (first[i] != second[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const String& first, const String& second) {
    return !(first == second);
}

bool operator<(const String& first, const String& second) {
    for (size_t i = 0; i < first.size(); ++i) {
        if (second.size() < i || first[i] > second[i]) {
            return false;
        }
        if (first[i] < second[i]) {
            return true;
        }
    }
    return true;
}

bool operator>(const String& first, const String& second) {
    return (second < first);
}

bool operator<=(const String& first, const String& second) {
    return !(first > second);
}

bool operator>=(const String& first, const String& second) {
    return !(first < second);
}

String operator+(const String& first, const String& second) {
    String copy = first;
    copy += second;
    return copy;
}

String operator+(const String& first, char chr) {
    String copy = first;
    copy += chr;
    return copy;
}

String operator+(char chr, const String& second) {
    String str_char(1, chr);
    str_char += second;
    return str_char;
}

std::istream& operator>>(std::istream& in, String& str) {
    char chr;
    str.clear();
    chr = in.get();
    while (in && (chr == ' ' || chr == '\n')) {
        chr = in.get();
    }
    while (in && chr != '\n' && chr != ' ' && chr != EOF) {
        str += chr;
        chr = in.get();
    }
    return in;
}