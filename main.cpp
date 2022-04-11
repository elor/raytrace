#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

std::string filename{"image.ppm"};

struct Vec3 {
  double x, y, z;
};

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

Vec3 operator*(Vec3 v, double d) { return {v.x * d, v.y * d, v.z * d}; }

Vec3 operator/(Vec3 v, double d) { return v * (1.0 / d); }

double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

double length(Vec3 v) { return sqrt(dot(v, v)); }

Vec3 cross(Vec3 a, Vec3 b) {
  return {
      a.y * b.z - a.z * b.y, //
      a.z * b.x - a.x * b.z, //
      a.x * b.y - a.y * b.x, //
  };
}

std::ostream &operator<<(std::ostream &out, const Vec3 &v) {
  return out << "{ " << v.x << ' ' << v.y << ' ' << v.z << " }";
}

struct Ray {
  Vec3 origin;
  Vec3 direction;
};

std::ostream &operator<<(std::ostream &out, const Ray &ray) {
  return out << ray.origin << "->" << ray.direction;
}

struct Viewport {
  const int WIDTH{800};
  const int HEIGHT{600};
};

struct Camera {
  Vec3 position{0, 0, 0};
  Vec3 direction{0, 1, 0};
  Vec3 up{0, 0, 1};
  double FOV{35}; // degrees
};

struct Color {
  int r, g, b;
};

std::ostream &operator<<(std::ostream &out, const Color &color) {
  return out << ' ' << color.r << ' ' << color.g << ' ' << color.b << ' ';
}

struct Ball {
  Vec3 center{};
  double radius{1};
  Color color{128, 128, 128};
};

Vec3 closestPoint(Ray ray, Vec3 point) {
  return ray.origin +
         ray.direction * std::max(dot(ray.direction, point - ray.origin), 0.0);
}

bool intersects(Ball ball, Ray ray) {
  Vec3 closest = closestPoint(ray, ball.center);

  double distance = length(closest - ball.center);
  return distance < ball.radius;
}

Ray rayFromPixelPosition(int x, int y, Camera cam, Viewport view) {
  const auto up = cam.up;
  const auto direction = cam.direction;
  const auto right = cross(direction, up);

  const double aspect =
      static_cast<double>(view.WIDTH) / static_cast<double>(view.HEIGHT);
  const double yfactor = std::tan(cam.FOV * M_PI / 180.0);
  const double xfactor = yfactor * aspect;

  const double xd =
      (static_cast<double>(x) / static_cast<double>(view.WIDTH) - 0.5) *
      xfactor;
  const double yd = (static_cast<double>(view.HEIGHT - y - 1) /
                         static_cast<double>(view.HEIGHT) -
                     0.5) *
                    yfactor;

  const Vec3 ray = direction + right * xd + up * yd;

  return {cam.position, ray / length(ray)};
}

int main() {
  std::ofstream file(filename);

  Camera cam;
  Viewport view;

  Color background_color{0, 0, 128};
  std::vector<Ball> balls{
      {.center{-3, 10, 0}, .radius = 1, .color{128, 128, 200}},
      {.center{0, 0, -1000000}, .radius = 999999, .color{200, 200, 222}},
  };

  file << "P3\n"
       << "# " << filename << "\n"
       << view.WIDTH << " " << view.HEIGHT << "\n"
       << 256 << "\n";

  for (int y = 0; y < view.HEIGHT; y++) {
    for (int x = 0; x < view.WIDTH; x++) {
      Ray ray = rayFromPixelPosition(x, y, cam, view);

      bool found{false};
      for (const auto &ball : balls) {
        if (intersects(ball, ray)) {
          file << ball.color;
          found = true;
          break;
        }
      }

      if (!found)
        file << background_color;
    }
    file << '\n';
  }

  return 0;
};
