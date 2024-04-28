#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>

const double eps = 1e-7;

struct Point;
using Vector = Point;

struct Point {
    double x;
    double y;

    Point() = default;

    Point(double x, double y) : x(x), y(y) {}

    Point& operator+=(const Vector& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point& operator-=(const Vector& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector& operator*=(double value) {
        x *= value;
        y *= value;
        return *this;
    }

    Vector& operator/=(double value) {
        x /= value;
        y /= value;
        return *this;
    }

    Vector norm() { return Vector(-y, x); }

    Vector operator-() const { return Vector(-x, -y); }

    double length() const { return sqrt(x * x + y * y); }

    bool is_collinear(const Vector& other) const;

    Vector rotate(double angle) {
        return Vector(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
    }

    Vector& rotate(const Point& center, double angle);

    Vector& reflect(const Vector& center);

    Vector& scale(const Vector& center, double coefficient);
};

Vector operator+(const Vector& first, const Vector& second) {
    Vector copy = first;
    copy += second;
    return copy;
}

Vector operator-(const Vector& first, const Vector& second) {
    Vector copy = first;
    copy -= second;
    return copy;
}

Vector operator*(const Vector& point, double value) {
    Vector copy = point;
    copy *= value;
    return copy;
}

Vector operator/(const Vector& point, double value) {
    Vector copy = point;
    copy /= value;
    return copy;
}

bool double_equal(double first, double second) {
    return fabs(first - second) < eps;
}

bool operator==(const Vector& first, const Vector& second) {
    return (first - second).length() < eps;
}

bool operator!=(const Vector& first, const Vector& second) { return !(first == second); }

std::istream& operator>>(std::istream& in, Point& point) {
    in >> point.x >> point.y;
    return in;
}

std::ostream& operator<<(std::ostream& out, Point& point) {
    out << point.x << ' ' << point.y;
    return out;
}

double scalarProduct(const Vector& first, const Vector& second) {
    return first.x * second.x + first.y * second.y;
}

double vectorProduct(const Vector& first, const Vector& second) {
    return first.x * second.y - first.y * second.x;
}

bool Vector::is_collinear(const Vector& other) const {
    return double_equal(vectorProduct((*this), other), 0);
}

double get_angle(const Vector& first, const Vector& second) {
    return atan2(vectorProduct(first, second), scalarProduct(first, second));
}

Vector& Vector::reflect(const Vector& center) {
    return (*this) += (center - *this) * 2;
}

Vector& Vector::rotate(const Vector& center, double angle) {
    return (*this) = center + (*this - center).rotate(angle);
}

Vector& Vector::scale(const Vector& center, double coefficient) {
    return *this = center + (*this - center) * coefficient;
}

class Line {
private:
    Point start;
    Vector direction;

public:
    Line() = default;

    Line(const Point& first, const Point& second) : start(first), direction(second - first) {}

    Line(double coeff, double shift) : start(Point(0.0, shift)), direction(Point(1, coeff)) {}

    Line(const Point& point, double coeff) : start(point), direction(Point(1, coeff)) {}

    bool is_collinear(const Line& other) const { return direction.is_collinear(other.direction); }

    bool operator==(const Line& other) const;

    double oriented_distance(const Point& point) const {
        return vectorProduct(direction, (point - start)) / direction.length();
    }

    double dist(const Point& point) const {
        return fabs(oriented_distance(point));
    }

    Point cross(const Line& other) const;

    Vector norm() const {
        return Vector(-direction.y, direction.x);
    }

    Line& rotate(const Point& center, double angle) {
        start.rotate(center, angle);
        direction = direction.rotate(angle);
        return *this;
    }

    Line& reflect(const Point& center) {
        start.reflect(center);
        return *this;
    }

    Line& scale(const Point& center, double coefficient) {
        start.scale(center, coefficient);
        return *this;
    }

    Point reflection(const Point& point) const {
        return (point - norm() / norm().length() * oriented_distance(point) * 2);
    }
};

bool Line::operator==(const Line& other) const {
    return is_collinear(other) && direction.is_collinear((other.start - start));
}

bool operator!=(const Line& first, const Line& second) {
    return !(first == second);
}

Point Line::cross(const Line& other) const {
    double dist_start = other.oriented_distance(start);
    double dist_finish = other.oriented_distance(start + direction);
    return start + direction * dist_start / (dist_start - dist_finish);
}

class Shape {
public:
    virtual double perimeter() const = 0;
    virtual double area() const = 0;
    virtual bool operator==(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(const Point& point) const = 0;
    virtual Shape& rotate(const Point& center, double angle) = 0;
    virtual Shape& reflect(const Point& center) = 0;
    virtual Shape& reflect(const Line& axis) = 0;
    virtual Shape& scale(const Point& center, double coefficient) = 0;
    virtual ~Shape() = default;
};

bool operator!=(const Shape& first, const Shape& second) {
    return !(first == second);
}

class Polygon : public Shape {
protected:
    std::vector<Point>vertices;

    size_t get_next(size_t index) const {
        if (index + 1 == vertices.size()) {
            return 0;
        }
        return index + 1;
    }

    Vector get_side(size_t index) const {
        return vertices[get_next(index)] - vertices[index];
    }

    size_t get_prev(size_t index) const {
        if (index == 0) return vertices.size() - 1;
        return index - 1;
    }
public:
    Polygon() = default;

    Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}

    Polygon(std::initializer_list<Point> list) : vertices(list) {}

    template<typename... T>
    Polygon(T... vertices) : vertices({ vertices... }) {}

    size_t verticesCount() const { return vertices.size(); }

    const std::vector<Point>& getVertices() const { return vertices; };

    bool isConvex() const {
        size_t n = vertices.size();
        for (size_t i = 0; i < n; ++i) {
            Vector v1 = get_side(i);
            Vector v2 = get_side((i + 1) % n);
            Vector v3 = get_side((i + 2) % n);
            if (get_angle(v1, v2) * get_angle(v2, v3) < 0) return false;
        }
        return true;
    }

    double perimeter() const {
        double answer = 0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            answer += (vertices[get_next(i)] - vertices[i]).length();
        }
        return answer;
    }

    double area() const {
        double answer = 0;
        for (size_t i = 2; i < vertices.size(); ++i) {
            answer += vectorProduct((vertices[i] - vertices[0]), (vertices[i - 1] - vertices[0])) / 2;
        }
        return fabs(answer);
    }

    bool operator==(const Polygon& other) const;

    bool operator==(const Shape& other) const;

    bool isSimilarWhisoutModes(const Polygon& other) const {
        size_t size = vertices.size();
        if (other.vertices.size() != vertices.size()) return false;
        for (size_t start = 0; start < size; ++start) {
            bool flag = true;
            Vector v0_self = (vertices[get_next(0)] - vertices[0]);
            Vector v0_other = (other.vertices[(start + 1) % size] - other.vertices[start]);
            double coeff = v0_self.length() / v0_other.length();
            for (size_t i = 0; i < size; ++i) {
                Vector v1_self = get_side(i);
                Vector v1_other = other.get_side((i + start) % size);
                if (!double_equal(v1_self.length(), v1_other.length() * coeff)) flag = false;
                Vector v2_self = get_side((i + 1) % size);
                Vector v2_other = other.get_side((i + start + 1) % size);
                if (!double_equal(fabs(scalarProduct(v1_self, v2_self)), fabs(scalarProduct(v1_other, v2_other)) * coeff * coeff)) flag = false;
            }
            if (flag) return true;
        }
        return false;
    }

    bool isSimilarTo(const Polygon& other) const {
        Polygon copy = other;
        if (isSimilarWhisoutModes(copy)) return true;
        copy.reflect(Line(Point(0, 0), Point(0, 1)));
        if (isSimilarWhisoutModes(copy)) return true;
        reverse(copy.vertices.begin(), copy.vertices.end());
        if (isSimilarWhisoutModes(copy)) return true;
        copy.reflect(Line(Point(0, 0), Point(0, 1)));
        if (isSimilarWhisoutModes(copy)) return true;
        return false;
    }

    bool isSimilarTo(const Shape& other) const {
        const Polygon* ptr = dynamic_cast<const Polygon*>(&other);
        if (ptr == nullptr) return false;
        return isSimilarTo(static_cast<const Polygon&>(other));
    }

    bool isCongruentTo(const Polygon& other) const {
        return (isSimilarTo(other) && double_equal(perimeter(), other.perimeter()));
    }

    bool isCongruentTo(const Shape& other) const {
        const Polygon* ptr = dynamic_cast<const Polygon*>(&other);
        if (ptr == nullptr) return false;
        return isCongruentTo(static_cast<const Polygon&>(other));
    }

    bool containsPoint(const Point& point) const {
        size_t cnt = 0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            Point first = vertices[i] - point;
            Point second = vertices[get_next(i)] - point;
            if (first.y < second.y) std::swap(first, second);
            if (double_equal(second.y, 0.0) || (!double_equal(first.y, 0.0) && first.y < 0) || second.y > 0) continue;
            if (first.is_collinear(second)) return true;
            if (vectorProduct(second, first) > 0) cnt += 1;
        }
        if (cnt % 2 == 0) return false;
        return true;
    }

    Polygon& rotate(const Point& center, double angle) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].rotate(center, angle);
        }
        return *this;
    }

    Polygon& reflect(const Point& center) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].reflect(center);
        }
        return *this;
    }

    Polygon& reflect(const Line& axis) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i] = axis.reflection(vertices[i]);
        }
        return *this;
    }

    Polygon& scale(const Point& center, double coefficient) {
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].scale(center, coefficient);
        }
        return *this;
    }
};

bool Polygon::operator==(const Shape& other) const {
    const Polygon* ptr = dynamic_cast<const Polygon*>(&other);
    if (ptr == nullptr) return false;
    return ((*this) == static_cast<const Polygon&>(other));
}

bool Polygon::operator==(const Polygon& other) const {
    if (other.verticesCount() != verticesCount()) return false;
    size_t size = vertices.size();
    if (size == 0) return true;
    int start = -1;
    for (size_t i = 0; i < size; ++i) {
        if (vertices[0] == other.vertices[i]) {
            start = i;
            break;
        }
    }
    if (start == -1) return false;
    bool right = true;
    bool left = true;
    for (size_t i = 0; i < size; ++i) {
        if (vertices[i] != other.vertices[(i + start) % size]) right = false;
        if (vertices[i] != other.vertices[(start + size - i) % size]) left = false;
    }
    return (left | right);
}

class Ellipse : public Shape {
protected:
    Point focus1, focus2;
    double distance;
public:
    Ellipse() = default;
    Ellipse(const Point& focus1, const Point& focus2, double distance) : focus1(focus1), focus2(focus2), distance(distance) {}

    std::pair<Point, Point> focuses() const { return { focus1, focus2 }; }

    double eccentricity() const { return (focus1 - focus2).length() / distance; }

    Point center() const { return (focus1 + focus2) / 2; }

    std::pair<Line, Line> directrices() const {
        Point start1 = ((focus1 + focus2) + (focus1 - focus2) / eccentricity() / eccentricity()) / 2;
        Point start2 = focus1 + focus2 - start1;
        Vector direction = (focus1 - focus2).norm();
        return { Line(start1, direction + start1), Line(start2, start2 + direction) };
    }

    bool operator==(const Ellipse& other) const {
        if (!double_equal(distance, other.distance)) return false;
        if ((focus1 == other.focus1) && (focus2 == other.focus2)) return true;
        if ((focus2 == other.focus1) && (focus1 == other.focus2)) return true;
        return false;
    }

    bool operator==(const Shape& other) const {
        const Ellipse* ptr = dynamic_cast<const Ellipse*>(&other);
        if (ptr == nullptr) return false;
        return *this == static_cast<const Ellipse&>(other);
    }

    double perimeter() const {
        double a = distance / 2;
        double c = (focus1 - focus2).length() / 2;
        double b = sqrt(a * a - c * c);
        double x = 3 * (a - b) * (a - b) / ((a + b) * (a + b));
        return M_PI * (a + b) * (1 + x / (10 + sqrt(4 - x)));
    }

    double area() const {
        double a = distance / 2;
        double c = (focus1 - focus2).length() / 2;
        double b = sqrt(a * a - c * c);
        return M_PI * a * b;
    }

    bool isCongruentTo(const Shape& other) const {
        const Ellipse* ptr = dynamic_cast<const Ellipse*>(&other);
        if (ptr == nullptr) return false;
        return isCongruentTo(static_cast<const Ellipse&>(other));
    }

    bool isCongruentTo(const Ellipse& other) const {
        if (!double_equal(distance, other.distance)) return false;
        return double_equal((focus1 - focus2).length(), (other.focus1 - other.focus2).length());
    }

    bool isSimilarTo(const Ellipse& other) const {
        double coeff1 = distance / other.distance;
        double coeff2 = (focus1 - focus2).length() / (other.focus1 - other.focus2).length();
        return double_equal(coeff1, coeff2);
    }

    bool isSimilarTo(const Shape& other) const {
        const Ellipse* ptr = dynamic_cast<const Ellipse*>(&other);
        if (ptr == nullptr) return false;
        return isSimilarTo(static_cast<const Ellipse&>(other));
    }

    bool containsPoint(const Point& point) const {
        double dist = (focus1 - point).length() + (focus2 - point).length();
        return dist < distance;
    }

    Ellipse& rotate(const Point& center, double angle) {
        focus1.rotate(center, angle);
        focus2.rotate(center, angle);
        return *this;
    }

    Ellipse& reflect(const Point& center) {
        focus1.reflect(center);
        focus2.reflect(center);
        return *this;
    }

    Ellipse& reflect(const Line& axis) {
        focus1 = axis.reflection(focus1);
        focus2 = axis.reflection(focus2);
        return *this;
    }

    Ellipse& scale(const Point& center, double coefficient) {
        distance *= coefficient;
        focus1.scale(center, coefficient);
        focus2.scale(center, coefficient);
        return *this;
    }
};

class Circle : public Ellipse {
public:
    Circle() = default;
    Circle(const Point& center, double radius) : Ellipse(center, center, radius * 2) {}

    double radius() {
        return distance / 2;
    }
};

class Rectangle : public Polygon {
public:
    Rectangle() {
        vertices = std::vector<Point>(4);
    };

    Rectangle(const Point& first, const Point& second, double tangens) {
        if (tangens < 1) tangens = 1 / tangens;
        vertices = std::vector<Point>(4);
        vertices[0] = first;
        vertices[2] = second;
        double cosinus = sqrt(1 / (tangens * tangens + 1));
        vertices[3] = first + ((second - first) * cosinus).rotate(acos(cosinus));
        vertices[1] = first + second - vertices[3];
    }

    Point center() { return (vertices[0] + vertices[2]) / 2; }

    std::pair<Line, Line> diagonals() { return { Line(vertices[0], vertices[2]), Line(vertices[1], vertices[3]) }; }

};

class Square : public Rectangle {
public:
    Square() : Rectangle() {}

    Square(const Point& first, const Point& second) : Rectangle(first, second, 1) {}

    Circle circumscribedCircle() {
        return Circle(center(), (vertices[0] - vertices[2]).length() / 2);
    }

    Circle inscribedCircle() {
        return Circle(center(), (vertices[1] - vertices[0]).length() / 2);
    }
};

class Triangle : public Polygon {
public:
    using Polygon::Polygon;

    Triangle() {
        vertices = std::vector<Point>(3);
    }

    Circle circumscribedCircle() const {
        Point middle1 = (vertices[0] + vertices[1]) / 2;
        Point middle2 = (vertices[0] + vertices[2]) / 2;
        Line middle_norm1(middle1, (vertices[0] - vertices[1]).norm() + middle1);
        Line middle_norm2(middle2, (vertices[0] - vertices[2]).norm() + middle2);
        Point center = middle_norm1.cross(middle_norm2);
        return Circle(center, (vertices[0] - center).length());
    }

    Circle inscribedCircle() const {
        double length0 = (vertices[1] - vertices[2]).length();
        double length1 = (vertices[0] - vertices[2]).length();
        double length2 = (vertices[0] - vertices[1]).length();
        Point incenter = (vertices[0] * length0 + vertices[1] * length1 + vertices[2] * length2) / (length0 + length1 + length2);
        double radius = area() * 2 / perimeter();
        return Circle(incenter, radius);
    }

    Point centroid() const { return (vertices[0] + vertices[1] + vertices[2]) / 3; }

    Point orthocenter() const {
        Line hight1(vertices[0], vertices[0] + (vertices[1] - vertices[2]).norm());
        Line hight2(vertices[1], vertices[1] + (vertices[0] - vertices[2]).norm());
        return hight1.cross(hight2);
    }

    Line EulerLine() const { return Line(orthocenter(), centroid()); }

    Circle ninePointsCircle() {
        Circle answer = circumscribedCircle();
        answer.scale(orthocenter(), 0.5);
        return answer;
    }
};