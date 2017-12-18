#include <vector>
#include <iostream>
#include "base/cvec.h"// vectors class
#include "base/matrix4.h"// vectors class
#include <stdexcept>
typedef Cvec3 Vec3;//3d point/vector
typedef Cvec4 Vec4;//3d homogenous point, w=1

#define DEBUG 0
using namespace std;
class Shape {
 private:
    std::vector<Vec3> vertices;
    Matrix4 modelMatrix;
 public:
    Shape(const std::vector<Vec3> vertices) {
        setVertices(vertices);
    }
    Shape() {
    }
    void setVertices(const std::vector<Vec3> vertices) {
        this->vertices = vertices;
    }
    void update(const Matrix4& mm) {
        for (int i = 0; i < 16; i++) {
            modelMatrix[i] = mm[i];
        }
    }

    //typically O(n), where n is the number of vertices
    Vec3 getFarthestPointInDirection(const Vec3& v) const {
        Vec4 p = modelMatrix * Vec4(vertices[0][0], vertices[0][1], vertices[0][2], 1);
        Vec3 worldvert(p[0], p[1], p[2]);
        Vec3 best(worldvert);
        double farthest = dot(worldvert, v);
#if DEBUG
        cout << "p[" << worldvert << " ";
#endif
        for (int i = 1; i < vertices.size(); i++) {
            p = modelMatrix * Vec4(vertices[i][0], vertices[i][1], vertices[i][2], 1);
            worldvert[0] = p[0]; worldvert[1] = p[1]; worldvert[2] = p[2];
#if DEBUG
            cout << "p[" << i << "]: " << worldvert << " ";
#endif
            double d = dot(worldvert, v);
#if DEBUG
            cout << "comparing: best(" << best << "= " << farthest << " with d(" << worldvert << "=" << d << endl;
#endif
            if (farthest < d) {
                best = worldvert;
                farthest = d;
            }
        }

#if DEBUG
        cout << endl;
        std::cout << "best point:  " << best << ", diretion: " << v << std::endl;
#endif
        return best;
    }
    //not used. just use an arbitary initial vector
    Vec3 getAveragePoint() const {
        Vec3 avg;
        for (int i = 0; i < vertices.size(); i++) {
            avg += vertices[i];
            std::cout << vertices[i] << ", ";
        }
        std::cout << std::endl;
        avg /= vertices.size();
        return avg;
    }
};

class GJK {
 public:
    bool intersect(const Shape& shape1, const Shape& shape2) {
        //        Vec3 avg1 = shape1.getAveragePoint();
        //        Vec3 avg2 = shape2.getAveragePoint();
        
        v = Vec3(1,0,0);//initial vector
#if DEBUG
        std::cout << "initial vector: " << v << std::endl;
#endif
        n = 0;//set simplex size 0

        c = support(shape1, shape2, v);

#if DEBUG
        cout << "c: " << c << ", v: " << v << endl;
        cout << "c dot v: " << dot(c,v) << endl;
#endif
        if (dot(c, v) < 0) {
            cout << "n==0 false" << endl;
            return false;
        }
        v = -c;
        b = support(shape1, shape2, v);
#if DEBUG
        cout << "b: " << b << ", v: " << v << endl;
        cout << "b dot v: " << dot(b,v) << endl;
#endif

        if (dot(b, v) < 0) {
            cout << "n==1 false" << endl;
            return false;
        }
        v = tripleProduct(c - b, -b);
        n = 2;
        
        for (int i = 0; i < MAX_ITERATIONS; ++i) {
            
            Vec3 a = support(shape1, shape2, v);
#if DEBUG
            std::cout << "iteration " << i << std::endl;
            std::cout << "support " << a << std::endl;
#endif
            if (dot(a, v) < 0) {
                // no intersection
                return false;
            }

            if (update(a)) {
                return true;
            }
        }
        return true;
    }

    Vec3 support(const Shape& shape1, const Shape& shape2, const Vec3& v) {
        Vec3 p1 = shape1.getFarthestPointInDirection(v);
        Vec3 p2 = shape2.getFarthestPointInDirection(-v);//negate v
        Vec3 p3 = p1 - p2;
        return p3;
    }

 private:
    bool update(const Vec3& a) {
        if (n == 2) {
            //handling triangle
            std::cout << "n=2" << std::endl;
            Vec3 ao = -a;
            Vec3 ab = b - a;
            Vec3 ac = c - a;

            Vec3 abc = cross(ab, ac);//normal of triangle abc

            // plane test on edge ab
            Vec3 abp = cross(ab, abc);//direction vector pointing inside triangle abc from ab
            if (dot(abp, ao) > 0) {
                //origin lies outside the triangle abc, near the edge ab
                c = b;
                b = a;
                v = tripleProduct(ab, ao);
                return false;
            }

            //plane test on edge ac
            
            //direction vector pointing inside triangle abc from ac
            //note that different than abp, the result of acp is abc cross ac, while abp is ab cross abc.
            //The order does matter. Based on the right-handed rule, we want the vector pointing inside the triangle.
            Vec3 acp = cross(abc, ac);
            if (dot(acp, ao) > 0) {
                //origin lies outside the triangle abc, near the edge ac
                b = a;
                v = tripleProduct(ac, ao);
                return false;
            }

            // Now the origin is within the triangle abc, either above or below it.
            if (dot(abc, ao) > 0) {
                //origin is above the triangle
                d = c;
                c = b;
                b = a;
                v = abc;
            } else {
                //origin is below the triangle
                d = b;
                b = a;
                v = -abc;
            }

            n = 3;

            return false;

        }

        if (n == 3) {
            std::cout << "n=3" << std::endl;
            Vec3 ao = -a;
            Vec3 ab = b - a;
            Vec3 ac = c - a;
            Vec3 ad = d - a;
            
            Vec3 abc = cross(ab, ac);
            Vec3 acd = cross(ac, ad);
            Vec3 adb = cross(ad, ab);
            
            Vec3 tmp;
            
            const int over_abc = 0x1;
            const int over_acd = 0x2;
            const int over_adb = 0x4;

            int plane_tests = 
                (dot(abc, ao) > 0 ? over_abc : 0) |
                (dot(acd, ao) > 0 ? over_acd : 0) |
                (dot(adb, ao) > 0 ? over_adb : 0);

#if DEBUG
            if (dot(abc, ao) > 0) {
                std::cout << "over abc" << std::endl;
            }
            if (dot(acd, ao) > 0) {
                std::cout << "over acd" << std::endl;
            }
            if (dot(adb, ao) > 0) {
                std::cout << "over adb" << std::endl;
            }

            std::cout << "abc: " << abc << std::endl;
            std::cout << "acd: " << acd << std::endl;
            std::cout << "adb: " << adb << std::endl;


            std::cout << "overabc: " << dot(abc, ao) << std::endl;
            std::cout << "overacd: " << dot(acd, ao) << std::endl;
            std::cout << "overadb: " << dot(adb, ao) << std::endl;

#endif

            switch(plane_tests) {
            case 0:
                {
#if DEBUG
                    std::cout << "case 0" << std::endl;
#endif
                    //inside the tetrahedron
                    return true;
                }
            case over_abc:
                {
#if DEBUG
                    std::cout << "case overabc" << std::endl;                    
#endif
                    if (!checkOneFaceAC(abc, ac, ao)) {
                        //in the region of AC
                        return false;
                    }
                    if (!checkOneFaceAB(abc, ab, ao)) {
                        //in the region of AB
                        return false;
                    }
                
                    //otherwise, in the region of ABC
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
            case over_acd:
                {
#if DEBUG
                    std::cout << "case overacd" << std::endl;
#endif
                    //rotate acd to abc, and perform the same procedure
                    b = c;
                    c = d;

                    ab = ac;
                    ac = ad;

                    abc = acd;

                    if (!checkOneFaceAC(abc, ac, ao)) {
                        //in the region of AC (actually is ad)
                        return false;
                    }
                    if (!checkOneFaceAB(abc, ab, ao)) {
                        //in the region of AB (actually is ac)
                        return false;
                    }
                
                    //otherwise, in the region of "ABC" (which is actually acd)
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                
                }
            case over_adb:
                {
#if DEBUG
                    std::cout << "case overadb" << std::endl;
#endif
                    //rotate adb to abc, and perform the same procedure
                    c = b;
                    b = d;
            
                    ac = ab;
                    ab = ad;
            
                    abc = adb;
                    if (!checkOneFaceAC(abc, ac, ao)) {
                        //in the region of "AC" (actually is AB)
                        return false;
                    }
                    if (!checkOneFaceAB(abc, ab, ao)) {
                        //in the region of AB (actually is AD)
                        return false;
                    }
                
                    //otherwise, in the region of "ABC" (which is actually acd)
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
            case over_abc | over_acd:
#if DEBUG
                    std::cout << "case overabc|acd" << std::endl;
#endif
                {
                    if (!checkTwoFaces(abc, acd, ac, ab, ad, ao)) {
                        if (!checkOneFaceAC(abc, ac, ao)) {
                            //in the region of "AC" (actually is AB)
                            return false;
                        }
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            //in the region of AB (actually is AD)
                            return false;
                        }
                        //otherwise, in the region of "ABC" (which is actually acd)
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    } else {
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            return false;
                        }
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    }
                }
            case over_acd | over_adb:
                {
#if DEBUG
                    std::cout << "case overacd|adb" << std::endl;
#endif
                    //rotate ACD, ADB into ABC, ACD
                    tmp = b;
                    b = c;
                    c = d;
                    d = tmp;
            
                    tmp = ab;
                    ab = ac;
                    ac = ad;
                    ad = tmp;
            
                    abc = acd;
                    acd = adb;
                    if (!checkTwoFaces(abc, acd, ac, ab, ad, ao)) {
                        if (!checkOneFaceAC(abc, ac, ao)) {
                            //in the region of "AC" (actually is AB)
                            return false;
                        }
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            //in the region of AB (actually is AD)
                            return false;
                        }
                        //otherwise, in the region of "ABC" (which is actually acd)
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    } else {
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            return false;
                        }
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    }
                }
            case over_adb | over_abc:
                {
#if DEBUG
                    std::cout << "case overadb|abc" << std::endl;
#endif
                    //rotate ADB, ABC into ABC, ACD
                    tmp = c;
                    c = b;
                    b = d;
                    d = tmp;
            
                    tmp = ac;
                    ac = ab;
                    ab = ad;
                    ad = tmp;
            
                    acd = abc;
                    abc = adb;
            
                    if (!checkTwoFaces(abc, acd, ac, ab, ad, ao)) {
                        if (!checkOneFaceAC(abc, ac, ao)) {
                            //in the region of "AC" (actually is AB)
                            return false;
                        }
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            //in the region of AB (actually is AD)
                            return false;
                        }
                        //otherwise, in the region of "ABC" (which is actually acd)
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    } else {
                        if (!checkOneFaceAB(abc, ab, ao)) {
                            return false;
                        }
                        d = c;
                        c = b;
                        b = a;
                        v = abc;
                        n = 3;
                        return false;
                    }
                }
            default:
#if DEBUG
                    std::cout << "case default" << std::endl;
#endif
                return true;
            }
        }
        std::cout << "should not reach here n = " << n << std::endl;
        return true;
    }

    Vec3 tripleProduct(const Vec3& ab, const Vec3& c) {
        return cross(cross(ab, c), ab);
    }

    bool checkOneFaceAC(const Vec3& abc, const Vec3& ac, const Vec3& ao) {
        if (dot(cross(abc, ac), ao) > 0) {
            //origin is in the region of edge ac
            b = -ao;//b=a
            v = tripleProduct(ac, ao);
            n = 2;

            return false;
        }
        return true;
    }
    bool checkOneFaceAB(const Vec3& abc, const Vec3& ab, const Vec3& ao) {
        if (dot(cross(ab, abc), ao) > 0) {
            //origin in the region of edge ab
            c = b;
            b = -ao;//b=a
            v = tripleProduct(ab, ao);
            n = 2;

            return false;
        }
        return true;
    }

    bool checkTwoFaces(Vec3& abc, Vec3& acd, Vec3& ac, Vec3& ab, Vec3& ad, const Vec3& ao) {
        if (dot(cross(abc, ac), ao) > 0) {
            b = c;
            c = d;
            ab = ac;
            ac = ad;
            
            abc = acd;
            return false;
        }
        return true;
    }

    Vec3 v;
    Vec3 b, c, d;
    unsigned int n; //simplex size

    const int MAX_ITERATIONS = 30;
};
