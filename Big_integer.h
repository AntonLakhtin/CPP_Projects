#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class BigInteger {
private:
    static const size_t max_length = 9;
    static const int mod = 1e9;
    std::vector<int>digits;
    bool isNegative = false;

    void delete_zero() {
        while (digits.size() > 0 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.empty()) {
            isNegative = false;
        }
    }

    int find_div(const BigInteger& other) const {
        int left = 0;
        int right = mod;
        while (right - left > 1) {
            int mid = (left + right) / 2;
            BigInteger result = other;
            result *= mid;
            if (result > (*this)) {
                right = mid;
            }
            else {
                left = mid;
            }
        }
        return left;
    }

    int castSubstringToInt(const std::string& str, int left, int right) {
        int answer = 0;
        int coeff = 1;
        for (int j = right; j > left; --j) {
            answer += (str[j - 1] - '0') * coeff;
            coeff *= 10;
        }
        return answer;
    }

    void carry(size_t index, bool& flag) {
        digits[index] += flag;
        if (digits[index] >= mod) {
            flag = true;
            digits[index] -= mod;
        }
        else {
            flag = false;
        }
    }

    void subtract(size_t index, int deductible, bool& flag) {
        deductible += flag;
        if (deductible > digits[index]) {
            flag = true;
            digits[index] += mod;
        }
        else {
            flag = false;
        }
        digits[index] -= deductible;
    }
public:
    BigInteger() = default;

    BigInteger(long long x) {
        if (x < 0) {
            isNegative = true;
            x = -x;
        }
        while (x > 0) {
            digits.push_back(x % mod);
            x /= mod;
        }
    }

    BigInteger(const std::string& str) {
        int start = 0;
        if (str[0] == '-') {
            start = 1;
            isNegative = true;
        }
        for (int i = str.size(); i > start; i -= max_length) {
            int left = std::max(i - static_cast<int>(max_length), start);
            digits.push_back(castSubstringToInt(str, left, i));
        }
        delete_zero();
    }

    void swap(BigInteger& other) {
        std::swap(isNegative, other.isNegative);
        std::swap(digits, other.digits);
    }

    BigInteger operator-() const {
        BigInteger copy = *this;
        if (digits.size() > 0) copy.ChangeSign();
        return copy;
    }

    bool operator<(const BigInteger& other) const {
        if (isNegative != other.isNegative) {
            return isNegative;
        }
        if (digits.size() != other.digits.size()) {
            return (digits.size() < other.digits.size()) ^ isNegative;
        }
        for (size_t i = digits.size(); i > 0; --i) {
            if (digits[i - 1] != other.digits[i - 1]) {
                return (digits[i - 1] < other.digits[i - 1]) ^ isNegative;
            }
        }
        return false;
    }

    bool operator>(const BigInteger& other) const {
        return (other < *this);
    }

    bool operator==(const BigInteger& other) const {
        if (isNegative != other.isNegative) {
            return false;
        }
        if (digits.size() != other.digits.size()) {
            return false;
        }
        for (size_t i = 0; i < digits.size(); ++i) {
            if (digits[i] != other.digits[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator<=(const BigInteger& other) const {
        return !(*this > other);
    }

    bool operator>=(const BigInteger& other) const {
        return !(*this < other);
    }

    bool operator!=(const BigInteger& other) const {
        return !(*this == other);
    }

    BigInteger& operator--() {
        if (isNegative) {
            isNegative = false;
            ++(*this);
            isNegative = true;
            return *this;
        }
        if (digits.size() == 0) {
            isNegative = true;
            digits.push_back(1);
            return *this;
        }
        for (size_t i = 0; i < digits.size(); ++i) {
            if (digits[i] != 0) {
                --digits[i];
                break;
            }
            digits[i] = mod - 1;
        }
        delete_zero();
        return *this;
    }

    BigInteger& operator++() {
        if (isNegative) {
            isNegative = false;
            --(*this);
            isNegative = true;
            return *this;
        }
        for (size_t i = 0; i < digits.size() + 1; ++i) {
            if (i == digits.size()) {
                digits.push_back(1);
                break;
            }
            if (digits[i] != mod - 1) {
                ++digits[i];
                break;
            }
            digits[i] = 0;
        }
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger copy = *this;
        ++(*this);
        return copy;
    }

    BigInteger operator--(int) {
        BigInteger copy = *this;
        --(*this);
        return copy;
    }

    BigInteger& operator+=(const BigInteger& other) {
        if (isNegative == other.isNegative) {
            bool flag = false;
            for (size_t i = 0; i < other.digits.size(); ++i) {
                if (i >= digits.size()) {
                    digits.push_back(0);
                }
                digits[i] += other.digits[i];
                carry(i, flag);
            }
            for (size_t i = other.digits.size(); i < digits.size(); ++i) {
                carry(i, flag);
            }
            if (flag) {
                digits.push_back(1);
            }
            delete_zero();
            return *this;
        }

        //if the sihns of these numbers are different (this works as a subtraction):
        bool flag = false;
        for (size_t i = 0; i < other.digits.size(); ++i) {
            if (i >= digits.size()) {
                digits.push_back(0);
            }
            subtract(i, other.digits[i], flag);
        }
        if (!flag) {
            delete_zero();
            return *this;
        }
        for (size_t i = other.digits.size(); i < digits.size(); ++i) {
            subtract(i, 0, flag);
        }
        if (!flag) {
            delete_zero();
            return *this;
        }
        digits[0] = (mod - digits[0]) % mod;
        for (size_t i = 1; i < digits.size(); ++i) {
            digits[i] = mod - 1 - digits[i];
        }
        ChangeSign();
        return *this;
    }

    BigInteger& operator-=(const BigInteger& other) {
        (*this) += (-other);
        return *this;
    }

    BigInteger& operator*=(int digit) {
        if (digit < 0) {
            ChangeSign();
            digit = -digit;
        }
        long long add = 0;
        for (size_t i = 0; i < digits.size(); ++i) {
            add = (static_cast<long long>(digits[i]) * digit + add);
            digits[i] = add % mod;
            add /= mod;
        }
        while (add > 0) {
            digits.push_back(add % mod);
            add /= mod;
        }
        delete_zero();
        return *this;
    }

    BigInteger operator*(int other) const {
        BigInteger copy = *this;
        copy *= other;
        return copy;
    }

    BigInteger& operator*=(const BigInteger& other) {
        BigInteger copy = *this;
        *this = 0;
        for (size_t i = other.digits.size(); i > 0; --i) {
            *this *= mod;
            *this += copy * other.digits[i - 1];
        }
        if (other.isNegative) {
            ChangeSign();
        }
        delete_zero();
        return *this;
    }

    BigInteger& operator/=(const BigInteger& other);

    std::string toString() const {
        if (digits.size() == 0) {
            return "0";
        }
        std::string ans;
        if (isNegative) {
            ans += '-';
        }
        std::vector<char>number;
        for (size_t i = 0; i < digits.size() - 1; ++i) {
            int x = digits[i];
            for (size_t j = 0; j < max_length; ++j) {
                number.push_back('0' + x % 10);
                x /= 10;
            }
        }
        int x = digits.back();
        while (x > 0) {
            number.push_back('0' + x % 10);
            x /= 10;
        }
        for (size_t i = number.size(); i > 0; --i) {
            ans += number[i - 1];
        }
        return ans;
    }

    BigInteger& operator%=(const BigInteger& other) {
        BigInteger div = (*this);
        div /= other;
        BigInteger result = other;
        result *= div;
        *this -= result;
        return *this;
    }

    explicit operator bool() const {
        return (digits.size() != 0);
    }

    explicit operator long long() const {
        if (digits.size() == 0) {
            return 0;
        }
        long long ans = 0;
        long long kmod = 1;
        for (size_t i = 0; i < digits.size(); ++i) {
            ans += static_cast<long long>(digits[i]) * kmod;
            kmod *= mod;
        }
        if (isNegative) {
            return -ans;
        }
        return ans;
    }

    bool IsNegative() {
        return isNegative;
    }

    BigInteger& ChangeSign() {
        isNegative = !isNegative;
        delete_zero();
        return *this;
    }
};

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
    BigInteger copy = first;
    copy += second;
    return copy;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
    BigInteger copy = first;
    copy -= second;
    return copy;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
    BigInteger copy = first;
    copy *= second;
    return copy;
}

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
    BigInteger copy = first;
    copy /= second;
    return copy;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
    BigInteger copy = first;
    copy %= second;
    return copy;
}

BigInteger operator ""_bi(unsigned long long num) {
    return BigInteger(num);
}

std::ostream& operator<<(std::ostream& out, const BigInteger& num) {
    out << num.toString();
    return out;
}

std::istream& operator>>(std::istream& in, BigInteger& num) {
    std::string s;
    in >> s;
    num = BigInteger(s);
    return in;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
    if (other.isNegative) {
        ChangeSign();
    }
    BigInteger initial_part = 0;
    int index = digits.size() - 1;
    std::vector<int>result_digits;
    BigInteger AbsOther = other;
    AbsOther.isNegative = false;
    while (index >= 0) {
        initial_part *= mod;
        initial_part += digits[index];
        int div = initial_part.find_div(AbsOther);
        initial_part -= AbsOther * div;
        result_digits.push_back(div);
        --index;
    }
    std::reverse(result_digits.begin(), result_digits.end());
    digits = result_digits;
    delete_zero();
    return *this;
}

BigInteger gcd(BigInteger first, BigInteger second) {
    if (first.IsNegative()) {
        first.ChangeSign();
    }
    if (second.IsNegative()) {
        second.ChangeSign();
    }
    if (first == 0) {
        return second;
    }
    if (second == 0) {
        return first;
    }
    return gcd(second, first % second);
}

class Rational {
private:
    BigInteger numerator;
    BigInteger denominator;

    void norm() {
        BigInteger div = gcd(numerator, denominator);
        numerator /= div;
        denominator /= div;
        if (denominator.IsNegative()) {
            denominator.ChangeSign();
            numerator.ChangeSign();
        }
    }
public:
    Rational() : numerator(0), denominator(1) {}

    Rational(const Rational& ration) = default;

    Rational(int num) : numerator(num), denominator(1) {}

    Rational(const BigInteger& num) : numerator(num), denominator(1) {}

    Rational(const BigInteger& numerator, const BigInteger& denominator) : numerator(numerator), denominator(denominator) {
        norm();
    }

    Rational& operator=(const Rational& other) {
        numerator = other.numerator;
        denominator = other.denominator;
        return *this;
    }

    Rational operator-() const {
        Rational copy = *this;
        copy.numerator = -copy.numerator;
        return copy;
    }

    Rational& operator+=(const Rational& other) {
        numerator = numerator * other.denominator + other.numerator * denominator;
        denominator *= other.denominator;
        norm();
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        numerator = numerator * other.denominator - other.numerator * denominator;
        denominator *= other.denominator;
        norm();
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        numerator *= other.numerator;
        denominator *= other.denominator;
        norm();
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        numerator *= other.denominator;
        denominator *= other.numerator;
        norm();
        return *this;
    }

    int sign() const {
        if (numerator > 0) {
            return 1;
        }
        if (numerator < 0) {
            return -1;
        }
        return 0;
    }

    std::string toString() const {
        if (denominator != 1) {
            return numerator.toString() + '/' + denominator.toString();
        }
        else {
            return numerator.toString();
        }
    }

    std::string asDecimal(size_t presision = 0) const {
        if (presision == 0) {
            return (numerator / denominator).toString();
        }
        std::string ans;
        Rational copy = *this;
        if (copy.numerator < 0) {
            ans += '-';
            copy = -copy;
        }
        BigInteger num = copy.numerator % copy.denominator;
        for (size_t i = 0; i < presision; ++i) {
            num *= 10;
        }
        std::string ans_first = (copy.numerator / copy.denominator).toString() + '.';
        std::string ans_second = (num / copy.denominator).toString();
        for (size_t i = 0; i < presision - ans_second.size(); ++i) {
            ans_first += '0';
        }
        ans += ans_first + ans_second;
        return ans;
    }

    explicit operator double() const {
        long long integer_part = static_cast<long long>(numerator / denominator);
        BigInteger remainder = numerator % denominator;
        double coeff = 1e-9;
        double answer = integer_part;
        for (int i = 0; i < 35 && remainder != 0; ++i) {
            remainder *= 1e9;
            long long division_result = static_cast<long long>(remainder / denominator);
            remainder %= denominator;
            answer += static_cast<double>(division_result) * coeff;
            coeff *= 1e-9;
        }
        return answer;
    }

    bool operator==(const Rational& other) const;

    bool operator<(const Rational& other) const;
};

Rational operator+(const Rational& first, const Rational& second) {
    Rational copy = first;
    copy += second;
    return copy;
}

Rational operator-(const Rational& first, const Rational& second) {
    Rational copy = first;
    copy -= second;
    return copy;
}

Rational operator*(const Rational& first, const Rational& second) {
    Rational copy = first;
    copy *= second;
    return copy;
}

Rational operator/(const Rational& first, const Rational& second) {
    Rational copy = first;
    copy /= second;
    return copy;
}

bool Rational::operator==(const Rational& other) const {
    return (numerator * other.denominator == other.numerator * denominator);
}

bool Rational::operator<(const Rational& other) const {
    return (numerator * other.denominator < other.numerator* denominator);
}

bool operator>(const Rational& first, const Rational& second) {
    return (second < first);
}

bool operator<=(const Rational& first, const Rational& second) {
    return !(first > second);
}

bool operator>=(const Rational& first, const Rational& second) {
    return !(first < second);
}

bool operator!=(const Rational& first, const Rational& second) {
    return !(first == second);
}