#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <limits>

const int WIDTH = 512, HEIGHT = 512;

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); } // unary minus
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3 normalize() const {
        float len = std::sqrt(x * x + y * y + z * z);
        return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3();
    }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 powf(float gamma) const {
        return Vec3(std::pow(x, gamma), std::pow(y, gamma), std::pow(z, gamma));
    }
};

struct Ray {
    Vec3 origin, direction;
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}
};

struct Material {
    Vec3 ka, kd, ks;
    float specularPower;
};

struct Sphere {
    Vec3 center;
    float radius;
    Material material;
    Sphere(const Vec3& c, float r, const Material& m) : center(c), radius(r), material(m) {}
    bool intersect(const Ray& ray, float& t) const {
        Vec3 oc = ray.origin - center;
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * c;
        if (discriminant < 0) return false;
        discriminant = std::sqrt(discriminant);
        float t0 = (-b - discriminant) * 0.5f;
        float t1 = (-b + discriminant) * 0.5f;
        t = (t0 > 0) ? t0 : t1;
        return t > 0;
    }
};

struct Plane {
    float y;
    Material material;
    Plane(float y, const Material& m) : y(y), material(m) {}
    bool intersect(const Ray& ray, float& t) const {
        if (ray.direction.y == 0) return false;
        t = (y - ray.origin.y) / ray.direction.y;
        return t > 0;
    }
};

Vec3 image[HEIGHT][WIDTH];
std::vector<Sphere> spheres;
Plane* ground;
Vec3 lightPos(-4, 4, -3);
Vec3 lightColor(1, 1, 1);

bool inShadow(const Vec3& point, const Vec3& lightDir) {
    Ray shadowRay(point + lightDir * 0.001f, lightDir);
    float t;
    for (const auto& sphere : spheres) {
        if (sphere.intersect(shadowRay, t)) return true;
    }
    if (ground->intersect(shadowRay, t)) return true;
    return false;
}

Vec3 shade(const Vec3& point, const Vec3& normal, const Vec3& viewDir, const Material& mat) {
    Vec3 color = mat.ka * lightColor;
    Vec3 lightDir = (lightPos - point).normalize();
    if (!inShadow(point, lightDir)) {
        float diff = std::max(normal.dot(lightDir), 0.0f);
        Vec3 halfway = (viewDir + lightDir).normalize();
        float spec = std::pow(std::max(normal.dot(halfway), 0.0f), mat.specularPower);
        color += mat.kd * lightColor * diff + mat.ks * lightColor * spec;
    }
    return color;
}

void renderScene() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, d = 0.1f;
            float u = l + (r - l) * (x + 0.5f) / WIDTH;
            float v = b + (t - b) * (y + 0.5f) / HEIGHT;
            Vec3 dir = Vec3(u, v, -d).normalize();
            Ray ray(Vec3(0, 0, 0), dir);

            float closest = std::numeric_limits<float>::max();
            Vec3 finalColor(0, 0, 0);

            for (const auto& sphere : spheres) {
                float t;
                if (sphere.intersect(ray, t) && t < closest) {
                    Vec3 hit = ray.origin + ray.direction * t;
                    Vec3 normal = (hit - sphere.center).normalize();
                    Vec3 viewDir = -ray.direction;
                    finalColor = shade(hit, normal, viewDir, sphere.material);
                    closest = t;
                }
            }

            float tp;
            if (ground->intersect(ray, tp) && tp < closest) {
                Vec3 hit = ray.origin + ray.direction * tp;
                Vec3 normal(0, 1, 0);
                Vec3 viewDir = -ray.direction;
                finalColor = shade(hit, normal, viewDir, ground->material);
            }

            finalColor = finalColor.powf(1.0f / 2.2f); // Gamma correction
            image[y][x] = finalColor;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, image);
    glFlush();
}

int main(int argc, char** argv) {
    Material red = { Vec3(0.2f, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), 0 };
    Material green = { Vec3(0, 0.2f, 0), Vec3(0, 0.5f, 0), Vec3(0.5f, 0.5f, 0.5f), 32 };
    Material blue = { Vec3(0, 0, 0.2f), Vec3(0, 0, 1), Vec3(0, 0, 0), 0 };
    Material gray = { Vec3(0.2f, 0.2f, 0.2f), Vec3(1, 1, 1), Vec3(0, 0, 0), 0 };

    spheres.push_back(Sphere(Vec3(-4, 0, -7), 1, red));
    spheres.push_back(Sphere(Vec3(0, 0, -7), 2, green));
    spheres.push_back(Sphere(Vec3(4, 0, -7), 1, blue));
    ground = new Plane(-2, gray);

    renderScene();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Ray Tracer - Phong Shading (Gamma)");
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
