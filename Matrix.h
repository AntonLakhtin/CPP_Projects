#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <array>
#include <iomanip>
#include <random>
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
        BigInteger gcd_denominator = gcd(other.denominator, denominator);
        numerator = numerator * other.denominator + other.numerator * denominator;
        denominator *= other.denominator;
        numerator /= gcd_denominator;
        denominator /= gcd_denominator;
        norm();
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        BigInteger gcd_denominator = gcd(other.denominator, denominator);
        numerator = numerator * other.denominator - other.numerator * denominator;
        denominator *= other.denominator;
        numerator /= gcd_denominator;
        denominator /= gcd_denominator;
        norm();
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        BigInteger gcd1 = gcd(numerator, other.denominator);
        BigInteger gcd2 = gcd(other.numerator, denominator);
        numerator *= other.numerator;
        denominator *= other.denominator;
        BigInteger div = gcd1 * gcd2;
        numerator /= div;
        denominator /= div;
        if (denominator.IsNegative()) {
            denominator.ChangeSign();
            numerator.ChangeSign();
        }
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        BigInteger gcd1 = gcd(numerator, other.numerator);
        BigInteger gcd2 = gcd(other.denominator, denominator);
        numerator *= other.denominator;
        denominator *= other.numerator;
        BigInteger div = gcd1 * gcd2;
        numerator /= div;
        denominator /= div;
        if (denominator.IsNegative()) {
            denominator.ChangeSign();
            numerator.ChangeSign();
        }
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
        BigInteger z = numerator / denominator;
        long long z1 = static_cast<long long>(z);
        BigInteger rem = numerator;
        rem %= denominator;
        double coeff = 1e-9;
        double ans = z1;
        for (int i = 0; i < 35 && rem != 0; ++i) {
            rem *= 1e9;
            long long z2 = static_cast<long long>(rem / denominator);
            rem %= denominator;
            ans += static_cast<double>(z2) * coeff;
            coeff *= 1e-9;
        }
        return ans;
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

std::istream& operator>>(std::istream& in, Rational& num) {
    std::string str;
    in >> str;
    std::string s1, s2;
    bool flag = false;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '/') {
            flag = true;
        }
        else if (flag) {
            s2 += str[i];
        }
        else {
            s1 += str[i];
        }
    }
    BigInteger numer(s1);
    BigInteger denom(s2);
    if (!flag) {
        num = Rational(numer);
        return in;
    }
    num = Rational(numer, denom);
    return in;
}

//The end of Biginteger and Rational

constexpr bool is_prime(size_t N) {
    for (size_t d = 2; d * d <= N; ++d) {
        if (N % d == 0) {
            return false;
        }
    }
    return true;
}

template <size_t N>
class Residue {
private:
    int x;
public:
    Residue() = default;

    explicit Residue(int x) : x((x% static_cast<int>(N) + static_cast<int>(N)) % static_cast<int>(N)) {}

    Residue operator-() const {
        return Residue((-x + N) % N);
    }

    Residue& operator+=(const Residue& other) {
        x = (x + other.x) % N;
        return *this;
    }

    Residue& operator-=(const Residue& other) {
        x = (x + N - other.x) % N;
        return *this;
    }

    Residue& operator*=(const Residue& other) {
        x = (static_cast<long long>(x) * other.x) % N;
        return *this;
    }

    Residue& operator/=(const Residue& other);

    bool operator==(const Residue& other) const { return x == other.x; }

    bool operator!=(const Residue& other) const { return x != other.x; }

    explicit operator int() const {
        return x;
    }
};

template<size_t N>
Residue<N> pown(const Residue<N>& n, int k) {
    Residue<N>ans(1);
    Residue<N>val = n;
    while (k > 0) {
        if (k % 2 == 1) {
            ans *= val;
        }
        val *= val;
        k /= 2;
    }
    return ans;
}

template <size_t N>
Residue<N>& Residue<N>::operator/=(const Residue<N>& other) {
    return (*this) *= pown(other, static_cast<int>(N) - 2);
}

template <size_t N>
Residue<N> operator+(const Residue<N>& first, const Residue<N>& second) {
    Residue<N> copy = first;
    copy += second;
    return copy;
}

template <size_t N>
Residue<N> operator-(const Residue<N>& first, const Residue<N>& second) {
    Residue<N> copy = first;
    copy -= second;
    return copy;
}

template <size_t N>
Residue<N> operator*(const Residue<N>& first, const Residue<N>& second) {
    Residue<N> copy = first;
    copy *= second;
    return copy;
}

template <size_t N>
Residue<N> operator/(const Residue<N>& first, const Residue<N>& second) {
    Residue<N> copy = first;
    copy /= second;
    return copy;
}

template<size_t N>
std::ostream& operator<<(std::ostream& out, Residue<N> num) {
    out << static_cast<int>(num);
    return out;
}

template<size_t M, size_t N = M, typename Field = Rational>
class Matrix {
private:
    std::array<std::array<Field, N>, M> arr;

    Matrix& Gauss_method() {
        size_t str_ind = 0;
        for (size_t i = 0; i < N; ++i) {
            if (str_ind == M) {
                break;
            }
            size_t ind_whithout_zero = M;
            for (size_t j = str_ind; j < M; ++j) {
                if (arr[j][i] != Field(0)) {
                    ind_whithout_zero = j;
                    break;
                }
            }
            if (ind_whithout_zero == M) {
                continue;
            }
            if (ind_whithout_zero != str_ind) {
                for (size_t q = i; q < N; ++q) {
                    arr[str_ind][q] += arr[ind_whithout_zero][q];
                }
            }
            for (size_t j = str_ind + 1; j < M; ++j) {
                if (arr[j][i] == Field(0)) {
                    continue;
                }
                Field coeff = arr[j][i] / arr[str_ind][i];
                for (size_t q = i; q < N; ++q) {
                    arr[j][q] -= arr[str_ind][q] * coeff;
                }
            }
            ++str_ind;
        }
        return *this;
    }
public:
    Matrix() {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                arr[i][j] = Field(0);
            }
        }
    };

    Matrix(const std::array<std::array<Field, N>, M>& arr) : arr(arr) {}

    template <typename T>
    Matrix(std::initializer_list<std::initializer_list<T>> list) {
        int ind1 = 0;
        for (auto sub_list : list) {
            if (ind1 == M) {
                break;
            }
            int ind2 = 0;
            for (T val : sub_list) {
                if (ind2 == N) {
                    break;
                }
                arr[ind1][ind2] = static_cast<Field>(val);
                ++ind2;
            }
            ++ind1;
        }
    }

    std::array<Field, N>& operator[](int index) {
        return arr[index];
    }

    const std::array<Field, N>& operator[](int index) const {
        return arr[index];
    }

    Matrix operator-() const {
        Matrix copy = (*this);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                copy[i][j] = -copy[i][j];
            }
        }
        return copy;
    }

    Matrix& operator+=(const Matrix& other) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                arr[i][j] += other.arr[i][j];
            }
        }
        return *this;
    }

    Matrix& operator-=(const Matrix& other) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                arr[i][j] -= other.arr[i][j];
            }
        }
        return *this;
    }

    Matrix& operator*=(Field val) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                arr[i][j] *= val;
            }
        }
        return *this;
    }

    Matrix inverted() const {
        static_assert(N == M);
        Matrix copy = *this;
        copy.invert();
        return copy;
    }

    Matrix& invert() {
        static_assert(N == M);
        Matrix ans;
        for (size_t i = 0; i < N; ++i) {
            ans[i][i] = Field(1);
        }
        size_t str_ind = 0;
        for (size_t i = 0; i < N; ++i) {
            if (str_ind == M) {
                break;
            }
            size_t ind_whithout_zero = M;
            for (size_t j = str_ind; j < M; ++j) {
                if (arr[j][i] != Field(0)) {
                    ind_whithout_zero = j;
                    break;
                }
            }
            if (ind_whithout_zero == M) {
                continue;
            }
            if (ind_whithout_zero != str_ind) {
                for (size_t q = 0; q < N; ++q) {
                    arr[str_ind][q] += arr[ind_whithout_zero][q];
                    ans.arr[str_ind][q] += ans.arr[ind_whithout_zero][q];
                }
            }
            for (size_t j = 0; j < M; ++j) {
                if (j == str_ind) {
                    continue;
                }
                if (arr[j][i] == Field(0)) {
                    continue;
                }
                Field k = arr[j][i] / arr[str_ind][i];
                for (size_t q = 0; q < N; ++q) {
                    arr[j][q] -= arr[str_ind][q] * k;
                    ans.arr[j][q] -= ans.arr[str_ind][q] * k;
                }
            }
            ++str_ind;
        }
        for (size_t i = 0; i < M; ++i) {
            for (size_t q = 0; q < N; ++q) {
                ans.arr[i][q] /= arr[i][i];
            }
        }
        return *this = ans;
    }

    Matrix<N, M, Field> transposed() const {
        Matrix<N, M, Field> ans;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                ans[j][i] = arr[i][j];
            }
        }
        return ans;
    }

    Field det() const {
        static_assert(N == M);
        Matrix triangle_matr = *this;
        triangle_matr.Gauss_method();
        Field ans = Field(1);
        for (size_t i = 0; i < N; ++i) {
            ans *= triangle_matr.arr[i][i];
        }
        return ans;
    }

    Field trace() const {
        static_assert(N == M);
        Field ans = Field(0);
        for (size_t i = 0; i < N; ++i) {
            ans += arr[i][i];
        }
        return ans;
    }

    size_t rank() const {
        Matrix triangle_matr = *this;
        triangle_matr.Gauss_method();
        for (size_t i = 0; i < M; ++i) {
            bool zero = true;
            for (size_t j = 0; j < N; ++j) {
                if (triangle_matr.arr[i][j] != Field(0)) {
                    zero = false;
                    break;
                }
            }
            if (zero) {
                return i;
            }
        }
        return M;
    }

    std::array<Field, N> getRow(size_t index) const {
        return arr[index];
    }

    std::array<Field, M> getColumn(size_t index) const {
        std::array<Field, M> ans;
        for (size_t i = 0; i < M; ++i) {
            ans[i] = arr[i][index];
        }
        return ans;
    }

    bool operator==(const Matrix& other) const {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                if (arr[i][j] != other.arr[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const Matrix& other) const {
        return !(*this == other);
    }

    template<size_t K>
    Matrix<M, K, Field> operator*=(const Matrix<N, K, Field>& other);
};

template<size_t M, size_t N, typename Field>
template<size_t K>
Matrix<M, K, Field> Matrix<M, N, Field>::operator*=(const Matrix<N, K, Field>& other) {
    return *this = (*this) * other;
}

template<size_t M, size_t N, size_t K, typename Field = Rational>
Matrix<M, K, Field> operator*(const Matrix<M, N, Field>& first, const Matrix<N, K, Field>& second) {
    Matrix<M, K, Field> ans;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < K; ++j) {
            for (size_t q = 0; q < N; ++q) {
                ans[i][j] += first[i][q] * second[q][j];
            }
        }
    }
    return ans;
}

template<size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field> first, const Matrix<M, N, Field> second) {
    Matrix<M, N, Field> copy = first;
    copy += second;
    return copy;
}

template<size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field> first, const Matrix<M, N, Field> second) {
    Matrix<M, N, Field> copy = first;
    copy -= second;
    return copy;
}

template<size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator*(const Matrix<M, N, Field>& matr, const Field& value) {
    Matrix<M, N, Field> copy = matr;
    copy *= value;
    return copy;
}

template<size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator*(const Field& value, const Matrix<M, N, Field>& matr) {
    Matrix<M, N, Field> copy = matr;
    copy *= value;
    return copy;
}

template <size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;
