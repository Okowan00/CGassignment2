#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

const int WIDTH = 512;
const int HEIGHT = 512;

struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 normalize() const {
        float len = std::sqrt(x * x + y * y + z * z);
        return Vec3(x / len, y / len, z / len);
    }
};

struct Ray {
    Vec3 origin, direction;
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalize()) {}
};

struct Material {
    Vec3 ka, kd, ks;
    float specularPower;

    Material() : ka(), kd(), ks(), specularPower(0.0f) {}
    Material(const Vec3& ka, const Vec3& kd, const Vec3& ks, float specularPower = 0.0f)
        : ka(ka), kd(kd), ks(ks), specularPower(specularPower) {
    }
};

struct Sphere {
    Vec3 center;
    float radius;
    Material material;

    Sphere(const Vec3& center, float radius, const Material& material)
        : center(center), radius(radius), material(material) {
    }

    bool intersect(const Ray& ray, float& t) const {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        return (t > 0);
    }
};

struct Plane {
    float y;
    Material material;

    Plane(float y, const Material& material) : y(y), material(material) {}

    bool intersect(const Ray& ray, float& t) const {
        if (std::abs(ray.direction.y) < 1e-4) return false;
        t = (y - ray.origin.y) / ray.direction.y;
        return (t > 0);
    }
};

Vec3 lightPos(-4, 4, -3);
std::vector<Sphere> spheres;
Plane* ground;
unsigned char image[HEIGHT][WIDTH][3];

Vec3 shade(const Vec3& point, const Vec3& normal, const Material& material, const Ray& ray) {
    Vec3 color = material.ka * Vec3(1, 1, 1); // Ambient

    Vec3 lightDir = (lightPos - point).normalize();
    Vec3 viewDir = (ray.origin - point).normalize();
    Vec3 halfVec = (viewDir + lightDir).normalize();

    float diff = std::max(normal.dot(lightDir), 0.0f);
    float spec = std::pow(std::max(normal.dot(halfVec), 0.0f), material.specularPower);

    Vec3 diffuse = material.kd * diff;
    Vec3 specular = material.ks * spec;
    color += diffuse + specular;

    return color;
}

bool isInShadow(const Vec3& point) {
    Vec3 toLight = (lightPos - point);
    Ray shadowRay(point + toLight.normalize() * 0.001f, toLight);
    float t;
    for (const auto& sphere : spheres) {
        if (sphere.intersect(shadowRay, t) && t < toLight.dot(toLight)) return true;
    }
    float planeT;
    if (ground->intersect(shadowRay, planeT) && planeT * planeT < toLight.dot(toLight)) return true;
    return false;
}

void renderScene() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float u = (x + 0.5f) / WIDTH * 0.2f - 0.1f;
            float v = (y + 0.5f) / HEIGHT * 0.2f - 0.1f;
            Ray ray(Vec3(0, 0, 0), Vec3(u, v, -0.1f));

            float tMin = std::numeric_limits<float>::max();
            Vec3 finalColor(0, 0, 0);
            Vec3 hitPoint, normal;
            Material hitMaterial;

            for (const auto& sphere : spheres) {
                float t;
                if (sphere.intersect(ray, t) && t < tMin) {
                    tMin = t;
                    hitPoint = ray.origin + ray.direction * t;
                    normal = (hitPoint - sphere.center).normalize();
                    hitMaterial = sphere.material;
                }
            }

            float planeT;
            if (ground->intersect(ray, planeT) && planeT < tMin) {
                tMin = planeT;
                hitPoint = ray.origin + ray.direction * planeT;
                normal = Vec3(0, 1, 0);
                hitMaterial = ground->material;
            }

            if (tMin < std::numeric_limits<float>::max()) {
                if (isInShadow(hitPoint)) {
                    finalColor = hitMaterial.ka * Vec3(1, 1, 1);
                }
                else {
                    finalColor = shade(hitPoint, normal, hitMaterial, ray);
                }
            }

            image[y][x][0] = static_cast<unsigned char>(std::min(finalColor.x, 1.0f) * 255);
            image[y][x][1] = static_cast<unsigned char>(std::min(finalColor.y, 1.0f) * 255);
            image[y][x][2] = static_cast<unsigned char>(std::min(finalColor.z, 1.0f) * 255);
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image);
    glFlush();
}

int main(int argc, char** argv) {
    Material red = Material(Vec3(0.2f, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), 0);
    Material green = Material(Vec3(0, 0.2f, 0), Vec3(0, 0.5f, 0), Vec3(0.5f, 0.5f, 0.5f), 32);
    Material blue = Material(Vec3(0, 0, 0.2f), Vec3(0, 0, 1), Vec3(0, 0, 0), 0);
    Material gray = Material(Vec3(0.2f, 0.2f, 0.2f), Vec3(1, 1, 1), Vec3(0, 0, 0), 0);

    spheres.push_back(Sphere(Vec3(-4, 0, -7), 1, red));
    spheres.push_back(Sphere(Vec3(0, 0, -7), 2, green));
    spheres.push_back(Sphere(Vec3(4, 0, -7), 1, blue));
    ground = new Plane(-2, gray);

    renderScene();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Ray Tracer - Phong Shading");
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glutDisplayFunc(display);
    glutMainLoop();

    delete ground;
    return 0;
}
