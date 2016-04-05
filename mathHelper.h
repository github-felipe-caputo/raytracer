#ifndef _MATHHELPER_H
#define _MATHHELPER_H

#define VECTOR_INCOMING 0
#define VECTOR_OUTGOING 1
#define PI 3.14159265

#include <cmath>

/*
 * The Color class.
 */

struct Color {
    // RGB
    double r, g, b;

    //default
    Color( double s = 0 ) : r(s), g(s), b(s) {}

    // constructors
    Color ( double r, double g, double b ) : r(r), g(g), b(b) {}

    // Non-modifying arithematic operators
    Color operator+(const Color& rhs){
        return Color(r + rhs.r, g + rhs.g, b + rhs.b);
    } 

    Color operator/(double rhs){
        return Color(r/rhs, g/rhs, b/rhs);
    } 

    Color operator*(const Color& rhs){
        return Color(r * rhs.r, g * rhs.g, b * rhs.b);
    }

    Color operator*(double rhs){
        return Color(r * rhs, g * rhs, b * rhs);
    }

    friend Color operator*(double lhs, const Color& rhs){
        return Color(lhs * rhs.r, lhs * rhs.g, lhs * rhs.b);
    }

    // Modifying arithematic operators
    Color& operator+=( const Color& rhs ) { 
        r += rhs.r;  
        g += rhs.g;  
        b += rhs.b;

        return *this; 
    }
};

/*
 * The Point class.
 */

struct Point {
    // 3D point
    double x, y, z;

    //default
    Point ( double s = 0 ) : x(s), y(s), z(s) {}

    // constructors
    Point ( double x, double y, double z ) : x(x), y(y), z(z) {}

    // overloading operators
    bool operator==(const Point& rhs) {
        return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    bool operator!=(const Point& rhs) {
        return !(*this == rhs);
    }
};

/*
 * The Vector class.
 */

struct Vector {
    // 3D vector
    double x, y, z;

    //default
    Vector ( double s = 0 ) : x(s), y(s), z(s) {}

    // constructor, considering a vector from origin -> (x,y,z)
    Vector ( double xn, double yn, double zn, bool norm = false ) : x(xn), y(yn), z(zn) {
        if(norm) {
            double len = sqrt( x*x+y*y+z*z );
            if (len != 0.0f) {
                x = x / len;
                y = y / len;
                z = z / len;
            }
        } 
    }

    // constructor, from origin to destination
    Vector ( Point o, Point d, bool norm = false ) { 
        x = d.x - o.x;
        y = d.y - o.y;
        z = d.z - o.z; 

        if(norm) {
            double len = sqrt( x*x+y*y+z*z );
            if (len != 0.0f) {
                x = x / len;
                y = y / len;
                z = z / len;
            }
        }  
    }

    // Non-modifying arithematic operators
    Vector operator+(const Vector& rhs) {
        return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
    } 

    Vector operator-(const Vector& rhs) {
        return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
    } 

    Vector operator* (double rhs) {
        return Vector(x * rhs, y * rhs, z * rhs);
    }

    friend Vector operator* (double lhs, const Vector& rhs) {
        return Vector(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
    }
};

/*
 * The Matrix class.
 *
 * Note: arithematic operations for matrixes will just assume the user is correct,
 * no check is done, e.g., the operations will not check the rows and columns of
 * two matrices when they are multiplied, it will just try to do it (will return
 * a runtime error if the user is using wrong parameters).
 */

struct Matrix {
    int row;
    int col;

    // matrix
    std::vector<double> matrix;

    // default
    Matrix ( int row, int col, double s = 0 ) : row(row), col(col) {
        for (int i = 0; i < row * col; ++i )
            matrix.push_back(s);
    }

    // constructor
    Matrix ( int row, int col, double vals[] ) : row(row), col(col) {
        for (int i = 0; i < row * col; ++i )
            matrix.push_back(vals[i]);
    }

    // create a matrix from a vector, since our vector is of size 3
    // the rows and columns are known beforehand
    Matrix ( Vector v ) : row(3), col(1) {
        matrix.push_back(v.x);
        matrix.push_back(v.y);
        matrix.push_back(v.z);
    }

    // Array subscription
    double& operator[](const int index) {
        return matrix[index];
    }

    // Non-modifying arithematic operators
    Matrix transpose () {
        double vals[row * col];

        for( int k = 0; k < row * col; ++k ) {
            int i = k / row;
            int j = k % row;
            vals[k] = matrix[col * j + i];
        }

        return Matrix(col,row,vals);
    }

    Matrix operator+ (const Matrix& rhs) {
        double vals[row * col];

        for ( int i = 0; i < row * col; ++i )
            vals[i] = matrix[i] + rhs.matrix[i];

        return Matrix(row, col, vals);
    } 

    Matrix operator- (const Matrix& rhs) {
        double vals[row * col];

        for ( int i = 0; i < row * col; ++i )
            vals[i] = matrix[i] - rhs.matrix[i];

        return Matrix(row, col, vals);
    } 

    Matrix operator* (const Matrix& rhs) {
        double vals[row * rhs.col];

        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < rhs.col; ++j) {
                vals[rhs.col*i+j] = 0;
                for (int k = 0; k < rhs.row; ++k) 
                    vals[i * rhs.col + j] += matrix[i * col + k] * rhs.matrix[k * rhs.col + j];
            }
        }

        return Matrix(row, rhs.col, vals);
    }

    Matrix operator* (double rhs) {
        double vals[row * col];

        for ( int i = 0; i < row * col; ++i )
            vals[i] *= rhs;

        return Matrix(row,col,vals);
    }

    friend Matrix operator* (double lhs, const Matrix& rhs) {
        double vals[rhs.row * rhs.col];

        for ( int i = 0; i < rhs.row * rhs.col; ++i )
            vals[i] = lhs * rhs.matrix[i];

        return Matrix(rhs.row,rhs.col,vals);
    }
};

/*
 * The Ray class.
 */

struct Ray {
    // 3D Ray with origin and direction
    Point o;
    Vector d;

    // constructor
    Ray ( Point p, Vector v ) : o(p), d(v) {}

    Point getOrigin() {
        return o;
    }

    Vector getDirection() {
        return d;
    }
};

/*
 * Non-class functions
 */

double distance ( const Point p, const Point q ) {
    double a = p.x - q.x;
    double b = p.y - q.y;
    double c = p.z - q.z;

    return sqrt(a*a + b*b + c*c);
}

double length ( const Vector v ) {
    return sqrt( v.x*v.x+v.y*v.y+v.z*v.z );
}

void normalize ( Vector& v ) {
    double len = length(v);

    if (len != 0.0) {
        v.x = v.x / len;
        v.y = v.y / len;
        v.z = v.z / len;
    }
}

Vector cross ( const Vector v, const Vector u ) {
    return Vector( v.y*u.z - v.z*u.y , v.z*u.x - v.x*u.z , v.x*u.y - v.y*u.x );
}

double dot ( const Vector v , const Vector u ) {
    return ( v.x*u.x + v.y*u.y + v.z*u.z );
}

// Reflect a vector v "hitting" a surface with normal N
// this vector can have two directions
// incoming -> going to the surface
// outgoing -> going from the surface
Vector reflect ( Vector v, Vector N, int direction ) {
    normalize( v );

    if (direction == VECTOR_INCOMING)
        return (v - 2.0 * dot(v, N) * N);
    else 
        return (2.0 * N * dot(v,N) - v);
}

// returns the index for the minimum value is a vector of doubles
int indexMinElement ( std::vector<double> v ) {
    double minDist = *std::max_element(v.begin(), v.end());
    int index = -1;

    for(unsigned int i = 0; i < v.size(); ++i) {
        if (v[i] != 0 && v[i] <= minDist) {
            minDist = v[i];
            index = i;
        }
    }

    return index;
}

// returns a simple 3x3 identity matrix
Matrix indentityMatrix () {
    double aux[] = {1,0,0,0,1,0,0,0,1};
    return Matrix(3,3,aux);
}

#endif