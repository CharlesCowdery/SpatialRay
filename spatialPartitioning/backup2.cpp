// spatialPartitioning.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <tuple>
#include <math.h>
#include <format>
#include <string>
#include <ctime>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <functional>
#include <fstream>
#include <array>

#include <cassert>
#include <intrin.h>

#define PIXEL_SCALAR 1
#define assertm(exp, msg) assert(((void)msg, exp))
#define SMALL 0.001
#define NEAR_THRESHOLD 0.001
#define SCENE_BOUNDS 1000
#define RED_MASK 255<<16
#define GREEN_MASK 255<<8
#define BLUE_MASK 255

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define PI 3.1415

#define OBJECT_CULLING true
#define CULL_RECEDING_OBJECTS true
#define LAZY_OBJECT_CHECKING true
//defines program precision
#define decimal double

using namespace std;

decimal fRand(decimal fMin, decimal fMax) //https://stackoverflow.com/a/2704552
{
    decimal f = (decimal)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

std::vector<std::string> readLinesFromFile(const std::string& fileName) { //chatgpt lol
    std::vector<std::string> lines;
    std::ifstream file(fileName);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
    return lines;
}

struct XYZ {
public:
    decimal X;
    decimal Y;
    decimal Z;
    XYZ() : X(0), Y(0), Z(0) {}
    XYZ(decimal _X, decimal _Y, decimal _Z) : X(_X), Y(_Y), Z(_Z) {}
    XYZ clone() {
        return XYZ(X, Y, Z);
    }
    void add(decimal addend) {
        X += addend;
        Y += addend;
        Z += addend;
    }
    void add(const XYZ& other) {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
    }
    void divide(decimal divisor) {
        X /= divisor;
        Y /= divisor;
        Z /= divisor;
    }
    void clip_negative(decimal clip_to = 0) {
        X = (X < 0) ? clip_to : X;
        Y = (Y < 0) ? clip_to : Y;
        Z = (Z < 0) ? clip_to : Z;
    }
    decimal smallest() {
        if (X < Y) {
            if (X < Z) {
                return X;
            }
            else {
                return Z;
            }
        }
        else {
            if (Y < Z) {
                return Y;
            }
            else {
                return Z;
            }
        }
    }
    void make_safe() { //ensures point can always be used for division
        X = (X == 0) ? (decimal)0.000001 : X;
        Y = (Y == 0) ? (decimal)0.000001 : Y;
        Z = (Z == 0) ? (decimal)0.000001 : Z;
    }
    decimal magnitude() {
        return sqrt(X * X + Y * Y + Z * Z); // 
    }
    decimal magnitude_noRT() {
        return X * X + Y * Y + Z * Z; // 
    }
    void normalize() {
        decimal len = magnitude();
        divide(len);
    }
    static XYZ reflect(XYZ vector, XYZ pole) {
        decimal vector_magnitude = vector.magnitude();
        decimal pole_magnitude = pole.magnitude();
        decimal mag_ratio = vector_magnitude / pole_magnitude;
        XYZ reflection_point = pole * (XYZ::cosine(vector, pole) * mag_ratio);
        XYZ pointing = reflection_point - vector;
        //decimal component = 2*XYZ::cosine(vector, pole)-1; //Weird math shit.
        return vector + (pointing * 2);
    }
    static XYZ floor(XYZ point) {
        return XYZ(
            std::floor(point.X),
            std::floor(point.Y),
            std::floor(point.Z)
        );
    }
    static XYZ min(XYZ point, decimal value) {
        return XYZ(
            (point.X < value) ? point.X : value,
            (point.Y < value) ? point.Y : value,
            (point.Z < value) ? point.Z : value
        );

    }
    static XYZ min(decimal value, XYZ point) {
        return XYZ::min(point, value);
    }
    static XYZ min(XYZ point, XYZ other) {
        return XYZ(
            (point.X < other.X) ? point.X : other.X,
            (point.Y < other.Y) ? point.Y : other.Y,
            (point.Z < other.Z) ? point.Z : other.Z
        );
    }
    static XYZ max(XYZ point, XYZ other) {
        return XYZ(
            (point.X > other.X) ? point.X : other.X,
            (point.Y > other.Y) ? point.Y : other.Y,
            (point.Z > other.Z) ? point.Z : other.Z
        );
    }
    static XYZ max(XYZ point, decimal value) {
        return XYZ(
            (point.X > value) ? point.X : value,
            (point.Y > value) ? point.Y : value,
            (point.Z > value) ? point.Z : value
        );

    }
    static decimal length(const XYZ& vector) {
        return sqrt(vector.X * vector.X + vector.Y * vector.Y + vector.Z * vector.Z);
    }

    static XYZ normalize(const XYZ& vector) {
        decimal len = XYZ::length(vector);
        return XYZ(vector.X / len, vector.Y / len, vector.Z / len);
    }
    static XYZ divide(const XYZ& point, const XYZ& other) {
        return XYZ(point.X / other.X, point.Y / other.Y, point.Z / other.Z);
    }
    static XYZ divide(const XYZ& point, decimal divisor) {
        return XYZ(point.X / divisor, point.Y / divisor, point.Z / divisor);
    }
    static XYZ add(const XYZ& point, decimal addend) {
        return XYZ(point.X + addend, point.Y + addend, point.Z + addend);
    }
    static XYZ add(const XYZ& point, const XYZ& other) {
        return XYZ(point.X + other.X, point.Y + other.Y, point.Z + other.Z);
    }
    static decimal distance_noRt(XYZ& point, XYZ& other) {
        decimal f1 = point.X - other.X;
        decimal f2 = point.Y - other.Y;
        decimal f3 = point.Z - other.Z;
        return f1 * f1 + f2 * f2 + f3 * f3;
    }
    static decimal distance(const XYZ& point,const XYZ& other){
        decimal f1 = point.X - other.X;
        decimal f2 = point.Y - other.Y;
        decimal f3 = point.Z - other.Z;
        return sqrt(f1 * f1 + f2 * f2 + f3 * f3);
    }
    static XYZ slope(XYZ point, XYZ other) {
        decimal distance = XYZ::distance(point, other);
        return (other - point) / distance;
    }
    static XYZ flip(XYZ point) {
        return XYZ(-point.X, -point.Y, -point.Z);
    }
    static decimal dot(XYZ point, XYZ other) {
        return point.X * other.X + point.Y * other.Y + point.Z * other.Z;
    }
    static decimal cosine(XYZ point, XYZ other) {
        decimal result = XYZ::dot(point, other) / (point.magnitude() * other.magnitude());
        return result;
    }
    static XYZ pow(XYZ point, decimal power) {
        return XYZ(std::pow(point.X, power), std::pow(point.Y, power), std::pow(point.Z, power));
    }
    static XYZ cross(XYZ point, XYZ other) {
        return XYZ(
            point.Y * other.Z - point.Z * other.Y,
            point.Z * other.X - point.X * other.Z,
            point.X * other.Y - point.Y * other.X
        );
    }
    static XYZ log(XYZ point) {
        return XYZ(
            log10(point.X),
            log10(point.Y),
            log10(point.Z)
        );
    }
    string to_string() {
        return std::to_string(X) + ", " + std::to_string(Y) + ", " + std::to_string(Z);
    }
    XYZ operator/(const XYZ& other) const {
        return XYZ(X / other.X, Y / other.Y, Z / other.Z);
    }
    XYZ operator/(const decimal divisor) const {
        return XYZ(X / divisor, Y / divisor, Z / divisor);
    }
    XYZ operator+(const XYZ& other) const {
        return XYZ(X + other.X, Y + other.Y, Z + other.Z);
    }
    XYZ operator+(const decimal addend) const {
        return XYZ(X + addend, Y + addend, Z + addend);
    }
    XYZ operator-(const XYZ& other) const {
        return XYZ(X - other.X, Y - other.Y, Z - other.Z);
    }
    XYZ operator-(const decimal addend) const {
        return XYZ(X - addend, Y - addend, Z - addend);
    }
    XYZ operator*(const decimal multiplier) const {
        return XYZ(X * multiplier, Y * multiplier, Z * multiplier);
    }
    XYZ operator*(const XYZ& other) const {
        return XYZ(X * other.X, Y * other.Y, Z * other.Z);
    }
    XYZ operator+=(const XYZ& other) {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        return *this;
    }
    XYZ operator-() {
        return XYZ(-X, -Y, -Z);
    }
    bool operator !=(const XYZ& other) {
        return !((X == other.X) && (Y == other.Y) && (Z == other.Z));
    }
    static XYZ linear_mix(decimal c, const XYZ& first, const XYZ& second) {
        decimal i = 1 - c;
        return XYZ(first.X * i + second.X * c, first.Y * i + second.Y * c, first.Z * i + second.Z * c);
    }


};
XYZ operator*(decimal self, const XYZ& point) {
    return point * self;
}
XYZ operator-(decimal self, const XYZ& point) {
    return point + self;
}
std::ostream& operator<<(std::ostream& os, XYZ& m) {
    return os << m.to_string();
}

typedef tuple<XYZ, XYZ, XYZ> Face;
typedef tuple<bool, bool, bool> tri_bool;

class Material {
public:
    int material_type = 0;
    XYZ color = XYZ(0,0,0);
    decimal roughness = 0.1;
    decimal metallic  = 0;
    decimal specular  = 0;
    decimal emission  = 0;
    XYZ emissive_color = XYZ(0, 0, 0);
    Material() {}
    Material(XYZ _color) : color(_color) {}
    Material* clone() {
        Material* new_self = new Material(color);
        new_self->material_type = 0;
        new_self->roughness = roughness;
        new_self->metallic = metallic;
        new_self->specular = specular;
        new_self->emission = emission;
        new_self->emissive_color = emissive_color.clone();
        return new_self;
    }
    decimal get_diffuse_factor() {
        return 1-min(max(metallic, specular), (decimal)1.0);
    }
    decimal get_specular_factor() {
        return min(max(metallic,specular),(decimal)1.0);
    }
    XYZ get_specular_color() {
        return XYZ::linear_mix(metallic, specular * XYZ(1, 1, 1), color);
    }
    XYZ get_diffuse_color() {
        return color / PI;
    }
    XYZ get_fresnel(XYZ light_slope,XYZ normal) {
        XYZ specular_color = get_specular_color();
        auto second_term = (1 - specular_color)*pow(1 - XYZ::dot(light_slope, normal), 5);
        return specular_color + second_term;
    }
    decimal get_normal_distribution_beta(XYZ normal, XYZ half_vector){
        decimal a = roughness;
        decimal dot = XYZ::dot(normal, half_vector);
        decimal exponent = -(1 - pow(dot, 2)) / (pow(a*dot,2));
        decimal base = 1 / (PI * pow(a,2) * pow(dot, 4));
        decimal final = base * exp(exponent);

        return final;
    }
    decimal get_normal_distribution_GGXTR(XYZ normal, XYZ half_vector) {
        decimal a = roughness;
        decimal dot = XYZ::dot(normal, half_vector);
        decimal final = (a * a)
                          /
            (PI * pow((dot*dot)*(a*a-1)+1,2));

        return final;
    }
    decimal geoSchlickGGX(XYZ normal, XYZ vector,  decimal k) {

        decimal dot = XYZ::dot(normal,vector);

        return dot / (dot * (1 - k) + k);
        //return 1;

    }
    XYZ calculate_BRDF_coefficient(XYZ normal, XYZ input_slope, XYZ output_slope) {
        
        if (XYZ::dot(normal, input_slope) <= 0 || XYZ::dot(normal, output_slope) <= 0) {
            return XYZ(0,0,0);
        }

        decimal k = pow(roughness + 1, 2) / 8;

        XYZ half_vector = XYZ::add(input_slope, output_slope);
        half_vector.normalize();
        XYZ fresnel = get_fresnel(output_slope, half_vector);
        auto geo = geoSchlickGGX(normal, output_slope, k) * geoSchlickGGX(normal, input_slope, k);//Smiths method
        auto normal_dist = get_normal_distribution_GGXTR(normal, half_vector);
        decimal divisor = (4 * XYZ::dot(normal, input_slope) * XYZ::dot(normal, output_slope));
        //fresnel = XYZ(1,1,1);
        //geo = 1;
        //normal_dist = 1;
        //divisor = 4;
        XYZ specular_output = get_specular_factor()*(geo * normal_dist * fresnel) / divisor; //* ;
        XYZ diffuse_output = get_diffuse_factor() * get_diffuse_color();
        XYZ BRDF_color = specular_output + diffuse_output;
        //return fresnel;//specular_output;
        return BRDF_color* XYZ::dot(normal, input_slope);//
    }
    XYZ calculate_BRDF(XYZ normal, XYZ input_light, XYZ input_slope, XYZ output_slope) {
        return calculate_BRDF_coefficient(normal, input_slope, output_slope);
    }
    XYZ static random_bounce(XYZ normal) {
        XYZ raw =  XYZ::normalize(XYZ(fRand(-1, 1), fRand(-1, 1), fRand(-1, 1)));
        decimal normal_mag = normal.magnitude();
        decimal result = XYZ::dot(raw, normal);
        if (result < 0) {
            XYZ projected_point = (raw - XYZ::dot(raw, normal) * normal);
            return raw + 1.5 * (projected_point - raw);
        }
        else {
            return raw;
        }

    }
    Material atUV(decimal x, decimal y) {

    }
    XYZ calculate_emissions() {
        return emission * emissive_color;
    }
};

class Primitive {
public:
    Material material;
    //pair<XYZ, XYZ> bounds;
    XYZ origin;
    int obj_type = 0;
    Primitive(Material _material) {
        material = _material;
    }
    virtual pair<XYZ, XYZ> get_bounds() { return pair<XYZ, XYZ>(); }

    virtual bool check_backface(XYZ& position) { return false; }

    virtual decimal distance(XYZ& position) { 
        cout << "[ERROR] Default distance function called! Something is misconfigured!" << endl;
        return 0;
    } //how to fix if this gets called: alcohol and crying
    virtual Material material_properties_at(XYZ) {
        return material;
    }
};

class PointLikeLight {
public:
    XYZ origin;
    Primitive* host;
    PointLikeLight(XYZ _origin, Primitive* _host) : origin(_origin), host(_host) {}
};

class Sphere : public Primitive {
public:
    decimal radius = 0;
    Sphere(decimal _radius, XYZ _origin, Material _material): Primitive(_material){
        radius = _radius;
        origin = _origin;
        //bounds = get_bounds();
        obj_type = 1;
    }
    pair<XYZ, XYZ> get_bounds() {
        XYZ pos = origin + radius;
        XYZ neg = origin - radius;
        return pair<XYZ, XYZ>(pos, neg);
    }
    
    bool check_backface(XYZ &position) {
        return true;
    }

    decimal distance(XYZ& position) {
        return XYZ::distance(origin,position) - radius;
    }

    static decimal distance(const XYZ& origin, const decimal radius, const XYZ& position) {
        return XYZ::distance(origin, position) - radius;
    }

    static XYZ normal(const XYZ& origin, XYZ point) {
        return XYZ::slope(origin, point);
    }  

    XYZ normal(XYZ point) {
        return XYZ::normalize(XYZ::slope(origin, point));
    }
};

struct PackagedSphere { //reduced memory footprint to improve cache perf
    decimal radius;
    XYZ origin;
    Material* material;
    bool culled;
    decimal prev_distance;
    PackagedSphere(Sphere& sphere) {
        radius = sphere.radius;
        origin = sphere.origin;
        material = &sphere.material;
    }
};

class Plane : public Primitive {
public:
    XYZ normal;
    XYZ origin_offset;
    Plane( XYZ _normal, XYZ _origin, Material _material) : Primitive(_material) {
        origin = _origin;
        normal = XYZ::normalize(_normal);
        origin_offset = XYZ::dot(origin, normal) * normal;
        obj_type = 2;
    }

    bool check_backface(XYZ& position) {
        return true;
    }
    static decimal distance(const XYZ& normal, const XYZ& origin_offset, const XYZ& position) {
        return XYZ::dot(position - origin_offset, normal);
    }
    static XYZ project_point(XYZ& normal, XYZ& position) {
        return (position - XYZ::dot(position, normal) * normal);
    }
    decimal distance(XYZ& position) {
        //XYZ projected_point = (position - XYZ::dot(position, normal) * normal)+origin_offset;
        //return XYZ::distance(position, projected_point);
        return XYZ::dot(position-origin_offset, normal);//this is like wildly simplified from position-(position after projection onto plane)
        //just ends in a dot product. Aint that weird
    }
};

struct PackagedPlane {
    XYZ normal;
    XYZ origin_offset;
    Material* material;
    bool culled;
    decimal prev_distance;
    PackagedPlane(Plane& plane) {
        normal = plane.normal;
        origin_offset = plane.origin_offset;
        material = &plane.material;
    }
};

class Mesh{
    vector<Face> faces;
    vector<XYZ> planes;
    Mesh() {}
    int import_file(string file_name) {
        return 0;
    }
    vector<Primitive> get_primitives() {

    }
};


class Lens {
public:
    vector<vector<XYZ>> output_grid;
    vector<vector<XYZ>> slope_grid;
    void (*_calculate_grid)(Lens* self, int resolution_x, int resolution_y);
    void calculate_vectors(int resolution_x, int resolution_y, XYZ focal_position) {
        for (vector<XYZ>& row : output_grid) {
            vector<XYZ> output_row;
            for (XYZ& lens_pos : row) {
                XYZ slope = lens_pos - focal_position;
                output_row.push_back(XYZ::normalize(slope));
            }
            slope_grid.push_back(output_row);
        }
    }
    void calculate_grid(int resolution_x, int resolution_y) {
        _calculate_grid(this, resolution_x, resolution_y);
    }

};

class RectLens : public Lens {
public:
    decimal width;
    decimal vertical_ratio;
    RectLens(decimal _width, decimal _vertical_ratio) : width(_width), vertical_ratio(_vertical_ratio) {
        _calculate_grid = _calculate_grid_function;
    }
    static void _calculate_grid_function(Lens* self, int resolution_x, int resolution_y) {
        RectLens* self_rect = (RectLens*)self;
        self_rect->output_grid.clear();
        decimal height = self_rect->width * self_rect->vertical_ratio;
        decimal partial_width = self_rect->width / resolution_x;
        decimal partial_height = height / resolution_y;
        for (int y = 0; y < resolution_y; y++) {
            vector<XYZ> output_row;
            for (int x = 0; x < resolution_x; x++) {
                output_row.push_back(XYZ(x * partial_width - self_rect->width / 2 + partial_width / 2, -y * partial_height + height / 2 - partial_height / 2, 0));
            }
            self->output_grid.push_back(output_row);
        }
    }
};

class Scene ;
class Ray ;

class Accelerator {
public:
    Scene* scene;

    vector<XYZ>sphere_origins;
    vector<decimal>sphere_radius;
    vector<Sphere*>sphere_ptrs;

    vector<XYZ>plane_normals;
    vector<XYZ>plane_offsets;
    vector<Plane*>plane_ptrs;

    vector<XYZ>point_light_origins;
    vector<PointLikeLight*>point_light_ptrs;
    Accelerator(Scene* _scene) : scene(_scene) {};

    void prep(vector<Sphere*> spheres, vector<Plane*> planes, vector<PointLikeLight*> pointlike_lights) {
        for (Sphere* sphere : spheres) {
            sphere_origins.push_back(sphere->origin.clone());
            sphere_radius.push_back(sphere->radius);
            sphere_ptrs.push_back(sphere);
        }
        for (Plane* plane : planes) {
            plane_normals.push_back(plane->normal.clone());
            plane_offsets.push_back(plane->origin_offset.clone());
            plane_ptrs.push_back(plane);
        }
        for (PointLikeLight* PLL : pointlike_lights) {
            point_light_origins.push_back(PLL->host->origin);
            point_light_ptrs.push_back(PLL);
        }
    }
};


class Scene {
public:
    
    //Im trying something here. Im going to organize objects into groups, and then explicitly process each one when casting. Pain the ass for me, but in theory it makes for cleaner execution and code.
    //my justification here is that polymorphism can make for some really just nasty fucking code. Doing this will force me to make more modular code and more explicit optimizations.
    //C++ just wasnt made to be flexible :(

    int object_count = 0;
    vector<Sphere*> spheres;
    vector<Plane*> planes;

    vector<PointLikeLight*> pointlike_lights;

    int current_resolution_x;
    int current_resolution_y;
    Scene() {}
    
    void register_sphere(Sphere* sphere) {
        object_count++;
        spheres.push_back(sphere);
        if (sphere->material.emission > 0) {
            pointlike_lights.push_back(new PointLikeLight(sphere->origin,(Primitive*)sphere));
        }
    }

    void register_plane(Plane* plane) {
        object_count++;
        planes.push_back(plane);
    }

    
private:
    
};

template <typename T> struct Forwarder {
    T* ptr;
    Forwarder(T* point_to) :ptr(point_to) {}
};

struct Luminance_Link {
    vector<Luminance_Link*>backward_links;
    Luminance_Link* forward_link;
    Forwarder<Luminance_Link>* my_link = nullptr;
    XYZ my_value;
    XYZ my_coefficient;
    Luminance_Link() {
        my_link = new Forwarder<Luminance_Link>(this);
        my_coefficient = XYZ(1, 1, 1);
    }
    Luminance_Link(XYZ value, XYZ coefficient, Luminance_Link* _forward_link, Forwarder<Luminance_Link>* my_l) :
        my_value(value), my_coefficient(coefficient), forward_link(_forward_link), my_link(my_l) {}

    XYZ calculate_output() {
        XYZ out = XYZ(0,0,0);
        for (auto LuLi : backward_links) {
            out+=LuLi->calculate_output();
        }
        return out * my_coefficient+my_value*my_coefficient;
    }
    void destroy_forwarder() {
        if (my_link != nullptr) {
            delete my_link;
            my_link = nullptr;
        }
    }
    ~Luminance_Link(){
        for (auto LuLi : backward_links) {
            delete LuLi;
        }
        destroy_forwarder();
    }
};

//This exists to be a fixed size object for use in ray pacakges. when the ray is freed, this creates a luminance link, and relinks the associated links. 
//uses a pointer pointer so that any other links pointing at it will be able to find it after it is destroyed into a real LL. When it is destroyed it updates the pointer address

//I cannot emphasize this enough, this was BORN to leak memory
//If I made this wrong, or its not used right, it will leak memory. no question about it
struct LL_precursor { 
    Forwarder<Luminance_Link>* forward_link;
    Forwarder<Luminance_Link>* my_link;
    XYZ coefficient = XYZ(1,1,1);
    XYZ value = XYZ(-1,-1,-1);

    LL_precursor(Forwarder<Luminance_Link>* forwarder_LL, XYZ coeff, XYZ _value) {
        my_link = new Forwarder<Luminance_Link>(nullptr);
        forward_link = forwarder_LL;
        coefficient = coeff;
        value = _value;
    }
    LL_precursor(Forwarder<Luminance_Link>* forwarder_LL, XYZ coeff) {
        my_link = new Forwarder<Luminance_Link>(nullptr);
        forward_link = forwarder_LL;
        coefficient = coeff;
    }
    LL_precursor(Forwarder<Luminance_Link>* forwarder_LL) {
        my_link = new Forwarder<Luminance_Link>(nullptr);
        forward_link = forwarder_LL;
    }
    LL_precursor() {}
    ~LL_precursor() {
        if (value != XYZ(-1, -1, -1)) {
            Luminance_Link* new_link = new Luminance_Link(value, coefficient, forward_link->ptr, my_link);
            forward_link->ptr->backward_links.push_back(new_link);
            my_link->ptr = new_link;
        }
;    }
};

class Montecarlo_Anchor{
    vector<XYZ*> returned_luminances;
};

struct PackagedRay {
    XYZ position;
    XYZ slope;
    char remaining_bounces;
    char remaining_monte_carlo;
    bool check_lighting;
    LL_precursor PreLL;
    PackagedRay() {};
    PackagedRay(XYZ _position, XYZ _slope, char bounces, char monte_carlo_bounces, bool _check_lighting, Forwarder<Luminance_Link>* LL_forward) :
        position(_position), slope(_slope), remaining_bounces(bounces), remaining_monte_carlo(monte_carlo_bounces), check_lighting(_check_lighting), PreLL(LL_precursor(LL_forward)) {}
    PackagedRay(XYZ _position, XYZ _slope, char bounces, char monte_carlo_bounces, bool _check_lighting, Forwarder<Luminance_Link>* LL_forward, XYZ coefficient) :
        position(_position), slope(_slope), remaining_bounces(bounces), remaining_monte_carlo(monte_carlo_bounces), check_lighting(_check_lighting), PreLL(LL_precursor(LL_forward,coefficient)) {}
    void move(decimal distance) {
        position += slope * distance;
    }
};

class Camera {
    //Ill note this is a fairly non-direct class. the original was more streamlined, but I decided to update it to remove gunk
    //and to allow progressive output updates during monte carlo rendering
public:
    XYZ position;
    Scene* scene;
    Accelerator* accelerator;

    Lens* lens;
    XYZ focal_position;

    decimal pre_scaled_gain = 100;
    decimal post_scaled_gain = 50;

    vector<vector<vector<XYZ*>>> ray_outputs;
    vector<vector<XYZ*>> image_output;

    int current_resolution_x;
    int current_resolution_y;

    Camera(Scene* _scene, XYZ _position, Lens* _lens, XYZ _focal_position) : scene(_scene),
        lens(_lens), position(_position), focal_position(_focal_position) {}

    void prep(int resolution_x, int resolution_y, Accelerator* _accelerator) {
        accelerator = _accelerator;
        current_resolution_x = resolution_x;
        current_resolution_y = resolution_y;
        lens->calculate_grid(resolution_x, resolution_y);
        lens->calculate_vectors(resolution_x, resolution_y, focal_position);
        for (int y_index = 0; y_index < resolution_y; y_index++) {
            vector<XYZ*> output_row;
            for (int x_index = 0; x_index < resolution_x; x_index++) {
                output_row.push_back(new XYZ(0, 0, 0));
            }
            image_output.push_back(output_row);
        }
        for (int y_index = 0; y_index < resolution_y; y_index++) {
            vector<vector<XYZ*>> output_row;
            for (int x_index = 0; x_index < resolution_x; x_index++) {
                vector<XYZ*> pixel_rays;
                output_row.push_back(pixel_rays);
            }
            ray_outputs.push_back(output_row);
        }
    }
    void process_outputs_to_colors() {
        for (int y_index = 0; y_index < current_resolution_y; y_index++) {
            vector<XYZ> slope_row = lens->slope_grid.at(y_index);
            vector<vector<XYZ*>> output_row;
            for (int x_index = 0; x_index < current_resolution_x; x_index++) {
                vector<XYZ*> rays = ray_outputs[y_index][x_index];
                XYZ total_luminence = XYZ(0, 0, 0);
                for (int i = 0; i < rays.size(); i++) {
                    total_luminence += *rays[i];
                }
                XYZ average_luminence = total_luminence / rays.size();
                
                image_output[y_index][x_index] = new XYZ(average_luminence.X, average_luminence.Y, average_luminence.Z);//I know you should be able to just use &
                //but im a bit paranoid after C++ kept freeing local objects I was pointerizing using &
            }
        }
    }
    void process_luminance_links_to_color(vector<vector<Luminance_Link*>>& links) {
        for (int y_index = 0; y_index < current_resolution_y; y_index++) {
            for (int x_index = 0; x_index < current_resolution_x; x_index++) {
                XYZ output = links[y_index][x_index]->calculate_output();
                image_output[y_index][x_index] = new XYZ(output.X,output.Y,output.Z);
                links[y_index][x_index]->~Luminance_Link();
            }
        }
    }
    void post_process() {
        for (int y_index = 0; y_index < current_resolution_y; y_index++) {
            for (int x_index = 0; x_index < current_resolution_x; x_index++) {
                XYZ* luminence = image_output[y_index][x_index];
                XYZ scaled_return = (*luminence)*pre_scaled_gain;
                //scaled_return = scaled_return*log10((scaled_return + 1).magnitude())/scaled_return.magnitude();//maybe more correct? dunno.

                scaled_return = XYZ::log(scaled_return + 1) * post_scaled_gain;
                scaled_return = XYZ::min(scaled_return, 255);
                scaled_return = XYZ::max(scaled_return, 0.0);
                image_output[y_index][x_index] = new XYZ(scaled_return.X, scaled_return.Y, scaled_return.Z);
                delete luminence;
            }
        }
    }
};

#define BLOCK_BOUNDS_CHECKING false
template <typename T, int max_size> struct DataBlock {
    T* data = new T[max_size];
    int size = 0;

    void enqueue(T& in) {
        data[size] = in;
        size++;
    }

    bool isFull() {
        return size >= max_size;
    }
    T get_val(int index) const {
        return data[index];
    }
    T& get_ref(int index) {
        return data[index];
    }
    T& back() {
        return data[size - 1];
    }
    ~DataBlock() {
        delete [] data;
    }
};

struct CastResults {
    XYZ normal;
    Material* material;
    CastResults(XYZ _normal, Material* _mat) : normal(_normal), material(_mat) {}
};

struct Casting_Diagnostics {
    signed int blocks_created = 0;
    int reflections_cast = 0;
    int shadows_cast = 0;
    int diffuses_cast = 0;
    chrono::nanoseconds duration;
};

//#define RAY_BLOCK_SIZE 65536
#define RAY_BLOCK_SIZE 8192
typedef DataBlock<PackagedRay,RAY_BLOCK_SIZE> Ray_Block;

class RayEngine {
public:
    vector<PackagedSphere> sphere_data; //stores primitives in a more packed data format for better cache optimizations.
    vector<PackagedPlane> plane_data; //Ive made quite a few mistakes throughout this project but I think Im finally on track with this
    Ray_Block* enqueuement_block;
    int enqueuement_index = 0;
    vector<Ray_Block*> queue_stack = vector<Ray_Block*>();
    vector<bool> object_check_state;
    vector<decimal> prev_distances;

    vector<PointLikeLight> lights;
    RayEngine() {}
    void prep() {
        enqueuement_block = new Ray_Block();
    }
    bool prep_queue() {
        if (queue_stack.size() > 0) {
            return true;
        }
        if (enqueuement_block->size > 0) {
            move_enqueuement_to_queue_stack();
            return true;
        }
        return false;
    }
    void load_scene_objects(Scene* scene) {
        for (auto s : scene->spheres) {
            sphere_data.push_back(PackagedSphere(*s));
            object_check_state.push_back(true);
            prev_distances.push_back(0.0);
        }
        for (auto p : scene->planes) {
            plane_data.push_back(PackagedPlane(*p));
            object_check_state.push_back(true);
            prev_distances.push_back(0.0);
        }
        for (auto PLL : scene->pointlike_lights) {
            lights.push_back(*PLL);
        }
    }
    void move_enqueuement_to_queue_stack() {
        queue_stack.push_back(enqueuement_block);
        enqueuement_block = new Ray_Block();
    }
    void enqueue_ray(PackagedRay& ray) {
        if (enqueuement_block->isFull()) {
            move_enqueuement_to_queue_stack();
        }
        enqueuement_block->enqueue(ray);
    }
    vector<vector<Luminance_Link*>> enqueue_camera_slopes(Lens* lens, XYZ position, char bounces) {
        int index = 0;
        vector<vector<Luminance_Link*>> out;
        for (auto row : lens->slope_grid) {
            vector<Luminance_Link*> out_row;
            for (auto slope : row) {
                Luminance_Link* output_link = new Luminance_Link();
                auto ray = PackagedRay(position, slope, bounces, 0, true, output_link->my_link);
                enqueue_ray(ray);
                out_row.push_back(output_link);
                index++;
            }
            out.push_back(out_row);
        }
        return out;
    }
    CastResults execute_ray_cast(XYZ& position, XYZ& slope) {
        #define default_smallest_distance 9999999;
        decimal distance_traveled = 0;
        for (PackagedSphere& s : sphere_data) {
            s.culled = false;
            s.prev_distance = 999999;
        }
        for (PackagedPlane& p : plane_data) {
            p.culled = false;
            p.prev_distance = 999999;
        }
        void* focus;

        int primary_type = -1;
        

        while (position.magnitude_noRT() < SCENE_BOUNDS * SCENE_BOUNDS) {
            decimal smallest_distance = default_smallest_distance;
            for (PackagedSphere& s : sphere_data) {
                if (!OBJECT_CULLING || !s.culled) {
                    float distance = Sphere::distance(s.origin, s.radius, position);
                    if (distance < smallest_distance) {
                        smallest_distance = distance;
                    }
                    if (distance < NEAR_THRESHOLD) {
                        XYZ normal = Sphere::normal(s.origin, position);
                        return CastResults(normal, s.material);
                    }
#if OBJECT_CULLING
#if CULL_RECEDING_OBJECTS
                    s.culled = s.prev_distance < distance;
                    s.prev_distance = distance;
#endif          
#endif
                }
            }
            for (PackagedPlane& p : plane_data) {
                if (!OBJECT_CULLING || !p.culled) {
                    float distance = Plane::distance(p.normal, p.origin_offset, position);
                    if (distance < smallest_distance) {
                        smallest_distance = distance;
                    }
                    if (distance < NEAR_THRESHOLD) {
                        return CastResults(p.normal, p.material);
                    }
#if OBJECT_CULLING
#if CULL_RECEDING_OBJECTS
                    p.culled = p.prev_distance < distance;
                    p.prev_distance = distance;
#endif          
#endif
                }

            }
            position += smallest_distance * slope;
            distance_traveled += smallest_distance;
        }
        return CastResults(XYZ(-1, -1, -1), nullptr);
    }
    Casting_Diagnostics process_block() {

        auto start_time = chrono::high_resolution_clock::now();

        Casting_Diagnostics stats;
        stats.blocks_created = -(((signed int)queue_stack.size()) - 1);
        Ray_Block* cast_block = queue_stack.back();
        queue_stack.pop_back();
        const int bounce_count = 16;
        for (int ray_index = 0; ray_index < cast_block->size; ray_index++) {
            PackagedRay& ray_data = cast_block->get_ref(ray_index);
            CastResults results = execute_ray_cast(ray_data.position, ray_data.slope);
            if (results.material == nullptr) {
                ray_data.PreLL.value = XYZ(0,0,0);
            }
            else {
                ray_data.PreLL.value = results.material->calculate_emissions();
                //ray_data.PreLL.value = XYZ(0, 0, 100);
                if (ray_data.remaining_bounces > 0) {
                    if (ray_data.remaining_monte_carlo > 0) {
                        for (int i = 0; i < bounce_count; i++) {
                            XYZ monte_slope = results.material->random_bounce(results.normal);
                            XYZ return_coefficient = results.material->calculate_BRDF_coefficient(results.normal, monte_slope, XYZ::flip(ray_data.slope))/bounce_count;
                            auto ray = PackagedRay(
                                ray_data.position + NEAR_THRESHOLD * results.normal * 2,
                                monte_slope,
                                ray_data.remaining_bounces - 1,
                                ray_data.remaining_monte_carlo - 1,
                                true,
                                ray_data.PreLL.my_link,
                                return_coefficient
                            );
                            enqueue_ray(ray);
                            stats.diffuses_cast++;
                        }
                    }
                    else {
                        XYZ reflection_slope = XYZ::reflect(XYZ::flip(ray_data.slope), results.normal);
                        XYZ return_coefficient = results.material->calculate_BRDF_coefficient(results.normal, reflection_slope, XYZ::flip(ray_data.slope));
                        auto ray = PackagedRay(
                            ray_data.position + NEAR_THRESHOLD * results.normal * 2,
                            reflection_slope,
                            ray_data.remaining_bounces - 1,
                            0,
                            true,
                            ray_data.PreLL.my_link,
                            return_coefficient
                        );
                        enqueue_ray(ray);
                        stats.reflections_cast++;
                    }
                }
                if (ray_data.check_lighting) {
                    int emit_count = 256;
                    for (int i = 0; i < emit_count; i++) {
                        for (PointLikeLight PLL : lights) {
                            XYZ light_slope = XYZ::slope(ray_data.position, PLL.origin);
                            light_slope += XYZ(fRand(-0.1, 0.1), fRand(-0.1, 0.1), fRand(-0.1, 0.1));
                            light_slope.normalize();
                            XYZ return_coefficient = results.material->calculate_BRDF_coefficient(results.normal, light_slope, XYZ::flip(ray_data.slope))/ emit_count;
                            if (ray_data.remaining_monte_carlo > 0) {
                                return_coefficient = return_coefficient / bounce_count;
                            }
                            auto ray = PackagedRay(
                                ray_data.position + NEAR_THRESHOLD * results.normal * 2,
                                light_slope,
                                0,
                                0,
                                false,
                                ray_data.PreLL.my_link,
                                return_coefficient
                            );
                            enqueue_ray(ray);
                            stats.shadows_cast++;
                        }
                    }
                }
                
            }

        }
        delete cast_block;
        stats.blocks_created += queue_stack.size();
        auto end_time = chrono::high_resolution_clock::now();
        stats.duration = end_time - start_time;
        return stats;
    }
};


class SceneManager {
public:
    Camera* camera;
    Scene* scene;
    Accelerator* accelerator;
    sf::RenderWindow* window;
    RayEngine RE;

    int current_resolution_x = 0;
    int current_resolution_y = 0;
    SceneManager(Camera* _camera, Scene* _scene):
    camera(_camera), scene(_scene), accelerator(new Accelerator(_scene))
    {
    }
    void render(int resolution_x, int resolution_y) {
        current_resolution_x = resolution_x;
        current_resolution_y = resolution_y;


        prep();
        auto LL_field = enqueue_rays();
        cout <<endl << "+RAYCASTING+" << endl;
        run_engine(true);
        cout << endl << "+POST-PROCESSING+" << endl;
        //emit_rays();
        post_process_LL(LL_field);
        create_window();
        draw();

    }
private:
    void prep() {
        accelerator->prep(scene->spheres, scene->planes, scene->pointlike_lights);

        cout << "Prepping camera.........." << flush;

        auto camera_prep_start = chrono::high_resolution_clock::now();
        camera->prep(current_resolution_x, current_resolution_y, accelerator);
        RE.load_scene_objects(scene);
        RE.prep();
        auto camera_prep_end = chrono::high_resolution_clock::now();

        cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(camera_prep_end - camera_prep_start).count() << "ms]" << endl;

    }
    vector<vector<Luminance_Link*>> enqueue_rays() {
        cout << "Queueing Rays............" << flush;

        auto queue_start = chrono::high_resolution_clock::now();
        auto LL_array = RE.enqueue_camera_slopes(camera->lens, camera->focal_position+camera->position, 32);
        auto queue_end = chrono::high_resolution_clock::now();

        cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(queue_end-queue_start).count() << "ms]" << endl;
        return LL_array;
    }
    void run_engine(bool verbose) {
        auto start_time = chrono::high_resolution_clock::now();

        while (RE.prep_queue()) { //this returns true if theres still rays to be processed
            fire_casting_event(verbose);
        }

        auto end_time = chrono::high_resolution_clock::now();

        cout << "Ray Casting Done [" << chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << "ms]" << endl;
    }
    void fire_casting_event(bool verbose) {
        cout << "Running casting cycle...." << flush;

        auto diagnostics = RE.process_block();

        if (verbose) {
            cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(diagnostics.duration).count() << "ms :: ";
            cout << diagnostics.blocks_created << " Blocks created :: ";
            cout << diagnostics.reflections_cast << "R :: ";
            cout << diagnostics.shadows_cast << "S :: ";
            cout << diagnostics.diffuses_cast << "D";
            cout << "]" << endl;
        }



    }
    void post_process_LL(vector<vector<Luminance_Link*>>& LL_array) {
        cout << "Post-Processing..." << flush;
        auto post_processing_start = chrono::high_resolution_clock::now();
        camera->process_luminance_links_to_color(LL_array);
        camera->post_process();
        auto post_processing_end = chrono::high_resolution_clock::now();
        cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(post_processing_end - post_processing_start).count() << "ms]" << endl;
    }
    void post_process() {
        cout << "Post-Processing..." << flush;
        auto post_processing_start = chrono::high_resolution_clock::now();
        camera->process_outputs_to_colors();
        auto post_processing_end = chrono::high_resolution_clock::now();
        cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(post_processing_end - post_processing_start).count() << "ms]" << endl;
    }
    void create_window() {
        cout << "Opening Window...." << flush;
        window = new sf::RenderWindow(sf::VideoMode(current_resolution_x * PIXEL_SCALAR, current_resolution_y * PIXEL_SCALAR), "Render Window!");
        cout << "Done" << endl;
    }
    void draw() {
        cout << "Drawing..........." << flush;
        auto drawing_start = chrono::high_resolution_clock::now();

        sf::RectangleShape rectangle(sf::Vector2f(PIXEL_SCALAR, PIXEL_SCALAR));
        for (int row_index = 0; row_index < camera->image_output.size(); row_index++) {
            vector<XYZ*> image_row = camera->image_output.at(row_index);
            for (int column_index = 0; column_index < image_row.size(); column_index++) {
                XYZ* pixel_data = image_row.at(column_index);
                rectangle.setFillColor(sf::Color(pixel_data->X, pixel_data->Y, pixel_data->Z));
                rectangle.setPosition(column_index * PIXEL_SCALAR, row_index * PIXEL_SCALAR);
                window->draw(rectangle);
            }
        }
        window->display();
        auto drawing_end = chrono::high_resolution_clock::now();
        cout << "Done [" << chrono::duration_cast<chrono::milliseconds>(drawing_end - drawing_start).count() << "ms]" << endl;

        while (window->isOpen())
        {
            sf::Event event;
            while (window->pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window->close();
            }
        }


    }
};


/*
Scene* load_cornell_box() {
    Spatial_Cube* scene = new Spatial_Cube(100, XYZ(0, 0, 0));

    

    decimal size = 1;

    decimal scalar = size / 555;

    decimal box_width = 555*scalar;
    decimal wall_offset = box_width / 2;

    int res_scalar = 10;
    int resolution_x = 108 * res_scalar;
    int resolution_y = 108 * res_scalar;
    Lens lens = RectLens(resolution_x, resolution_y, 0.025*scalar, resolution_y / resolution_x);
    Camera* camera = new Camera(scene, XYZ(0, 0, -800*scalar), lens);

    Material wall_left_mat = Material(XYZ(0, 0.5, 0.1));
    wall_left_mat.roughness = 0.5;
    wall_left_mat.specular = 0;
    Plane* wall_left = new Plane(XYZ(1, 0, 0), XYZ(-wall_offset, 0, 0), wall_left_mat);

    Material wall_right_mat = Material(XYZ(0.7,0,0));
    wall_right_mat.roughness = 0.5;
    wall_right_mat.specular = 0;
    Plane* wall_right = new Plane(XYZ(-1, 0, 0), XYZ(wall_offset, 0, 0), wall_right_mat);

    Material blank_wall_mat = Material(XYZ(1, 1, 1));
    blank_wall_mat.roughness = 0.2;
    blank_wall_mat.specular = 0.9;
    
    Plane* top_wall = new Plane(XYZ(0, -1, 0), XYZ(0, wall_offset, 0), blank_wall_mat);
    Plane* bottom_wall = new Plane(XYZ(0, 1, 0), XYZ(0, -wall_offset, 0), blank_wall_mat);
    Plane* back_wall = new Plane(XYZ(0, 0, -1), XYZ(0, 0, wall_offset), blank_wall_mat);

    Material light_mat;
    light_mat.color = XYZ(1, 1, 1);
    light_mat.emission = 10;
    light_mat.emissive_color = XYZ(1, 0.662, 0.341);
    Sphere* glow_sphere = new Sphere(0.05, XYZ(0, 0, 0), light_mat);

    scene->register_object(wall_right);
    scene->register_object(wall_left);
    scene->register_object(back_wall);
    scene->register_object(top_wall);
    scene->register_object(bottom_wall);
    scene->register_object(glow_sphere);

    return pair<Camera*, Spatial_Cube*>(camera, scene);
}
*/
SceneManager* load_default_scene() {

    Scene* scene = new Scene();

    Lens* lens = new RectLens(2, 0.5625);
    Camera* camera = new Camera(scene, XYZ(0, 0.2, -4), lens , XYZ(0,0,-1));


    vector<Sphere*> spheres;
    int n_sphere = 0;
    decimal max_pos = 7;
    decimal min_radius = 0.3;
    decimal max_radius = 1;
    for (int i = 0; i < n_sphere; i++) {
        decimal radius = fRand(min_radius, max_radius);
        decimal x = fRand(-max_pos, max_pos);
        decimal y = fRand(-max_pos, max_pos);
        decimal z = fRand(-max_pos, max_pos);
        decimal r = fRand(0, 1);
        decimal g = fRand(0, 1);
        decimal b = fRand(0, 1);
        XYZ origin = XYZ(x, y, z);
        Material mat = Material(XYZ(r, g, b));
        mat.roughness = 1;
        Sphere* sphere =new Sphere(radius, origin, mat);
        spheres.push_back(sphere);

    }

    Material sphere_mat;
    sphere_mat.color = XYZ(0.2, 0.2, 1);//XYZ(0.24725, 0.1995, 0.0745);
    sphere_mat.specular = 0;
    sphere_mat.metallic = 0;
    sphere_mat.roughness = 1;
    Sphere* my_sphere =new Sphere(1, XYZ(0, 0, 0), sphere_mat);

    Material sphere_2_mat;
    sphere_2_mat.color = XYZ(1, 0.5, 0.5);//XYZ(0.24725, 0.1995, 0.0745);
    sphere_2_mat.specular = 0;
    sphere_2_mat.metallic = 0;
    sphere_2_mat.roughness = 1;
    Sphere* my_sphere_2 = new Sphere(0.4, XYZ(-0.9, -0.6, -0.7), sphere_2_mat);

    Material light_mat;
    light_mat.color = XYZ(1, 1, 1);
    light_mat.emission = 500;
    //light_mat.emissive_color = XYZ(1, 1, 1);
    light_mat.emissive_color = XYZ(1, 0.662, 0.341);
    Sphere* glow_sphere =new Sphere(1, XYZ(-3, 4, 0), light_mat);


    Material floor_mat;
    floor_mat.color = XYZ(1, 1, 1);
    floor_mat.roughness = 1;
    floor_mat.specular = 0;
    Plane* floor =new Plane(XYZ(0, 1, 0), XYZ(0, -1, 0), floor_mat);


    scene->register_sphere(my_sphere);
    scene->register_sphere(my_sphere_2);

    scene->register_sphere(glow_sphere);
    scene->register_plane(floor);
    for (int i = 0; i < spheres.size(); i++) {
        scene->register_sphere(spheres.at(i));
    }

    SceneManager* SM = new SceneManager(camera, scene);

    return SM;
}

int main()
{
    srand(0);


    SceneManager* scene_manager = load_default_scene();
   
    scene_manager->render(1920/4,1080/4);

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
