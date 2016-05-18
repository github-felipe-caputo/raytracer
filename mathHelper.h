#ifndef _MATHHELPER_H
#define _MATHHELPER_H

#include <cmath>

// For voxels
#define SUBDIV_X 0
#define SUBDIV_Y 1
#define SUBDIV_Z 2

#define VECTOR_INCOMING 0
#define VECTOR_OUTGOING 1
#define PI 3.14159265

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

    // Comparisons
    bool operator!=(const Color& rhs) { 
        return (r != rhs.r || g != rhs.g || b != rhs.b);
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

    // Non-modifying arithematic operators
    Point operator+(const Point& rhs) {
        return Point(x + rhs.x, y + rhs.y, z + rhs.z);
    } 

    Point operator-(const Point& rhs) {
        return Point(x - rhs.x, y - rhs.y, z - rhs.z);
    } 

    Point operator* (double rhs) {
        return Point(x * rhs, y * rhs, z * rhs);
    }

    friend Point operator* (double lhs, const Point& rhs) {
        return Point(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
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
            if (len != 0.0) {
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
            if (len != 0.0) {
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

    // constructors
    Ray () {}

    Ray ( Point p, Vector v ) : o(p), d(v) {}

    Point getOrigin() {
        return o;
    }

    Vector getDirection() {
        return d;
    }
};


/*
 * The Voxel class.
 */

struct Voxel
{
    // follows right handed coord system
    double xLeft, xRight;
    double yBottom, yTop;
    double zFar, zNear;

    Voxel () {}

    Voxel(double xLeft, double xRight, double yBottom, double yTop, double zFar, double zNear) 
        : xLeft(xLeft), xRight(xRight), yBottom(yBottom), yTop(yTop), zFar(zFar), zNear(zNear) {}

    Voxel splitFront (int subdiv) {
        if (subdiv == SUBDIV_X)
            return Voxel((xLeft+xRight)/2.0, xRight, yBottom, yTop, zFar, zNear);
        else if (subdiv == SUBDIV_Y)
            return Voxel(xLeft, xRight, (yBottom+yTop)/2.0, yTop, zFar, zNear);
        else 
            return Voxel(xLeft, xRight, yBottom, yTop, (zFar+zNear)/2.0, zNear);
    }

    Voxel splitRear (int subdiv) {
        if (subdiv == SUBDIV_X)
            return Voxel(xLeft, (xLeft+xRight)/2.0, yBottom, yTop, zFar, zNear);
        else if (subdiv == SUBDIV_Y)
            return Voxel(xLeft, xRight, yBottom, (yBottom+yTop)/2.0, zFar, zNear);
        else 
            return Voxel(xLeft, xRight, yBottom, yTop, zFar, (zFar+zNear)/2.0);
    }

    double splitVal (int subdiv) {
        if (subdiv == SUBDIV_X)
            return (xLeft+xRight)/2.0;
        else if (subdiv == SUBDIV_Y)
            return (yBottom+yTop)/2.0;
        else 
            return (zFar+zNear)/2.0;
    }

    bool intersect (Ray ray, double t0, double t1, Point &in, Point &out) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        double tmin, tmax, tymin, tymax, tzmin, tzmax;

        double divx = 1 / d.x;
        if (divx >= 0) {
            tmin = (xLeft - o.x) * divx;
            tmax = (xRight - o.x) * divx;
        }
        else {
            tmin = (xRight - o.x) * divx;
            tmax = (xLeft - o.x) * divx;
        }

        double divy = 1 / d.y; 
        if (divy >= 0) {
            tymin = (yBottom - o.y) * divy;
            tymax = (yTop - o.y) * divy;
        }
        else {
            tymin = (yTop - o.y) * divy;
            tymax = (yBottom - o.y) * divy;
        }

        if ( (tmin > tymax) || (tymin > tmax) )
            return false;

        if (tymin > tmin)
            tmin = tymin;

        if (tymax < tmax)
            tmax = tymax;

        double divz = 1 / d.z; 
        if (divz >= 0) {
            tzmin = (zFar - o.z) * divz;
            tzmax = (zNear - o.z) * divz;
        }
        else {
            tzmin = (zNear - o.z) * divz;
            tzmax = (zFar - o.z) * divz;
        }

        if ( (tmin > tzmax) || (tzmin > tmax) )
            return false;

        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        in = Point(o.x + d.x * tmin, o.y + d.y * tmin, o.z + d.z * tmin);
        out = Point(o.x + d.x * tmax, o.y + d.y * tmax, o.z + d.z * tmax);

        return ( (tmin < t1) && (tmax > t0) );
    }

    Point getCenter() {
        return Point((xLeft + xRight) / 2.0 , (yBottom + yTop) / 2.0, (zFar + zNear) / 2.0);
    }

    Point getHalfLenghts() {
        return Point((xRight - xLeft) / 2.0, 
                     (yTop - yBottom) / 2.0, 
                     (zNear - zFar) / 2.0  );
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
    if (v.empty())
        return -1;

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