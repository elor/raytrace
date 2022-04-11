#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

constexpr auto DEFAULT_FILENAME = "image.ppm";

struct Vec3 {
  double x, y, z;
};

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

Vec3 operator*(Vec3 v, double d) { return {v.x * d, v.y * d, v.z * d}; }

Vec3 operator/(Vec3 v, double d) { return v * (1.0 / d); }

double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

double length_squared(Vec3 v) { return dot(v, v); }

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

std::tuple<double, Vec3> closestPoint(Ray ray, Vec3 point) {
  double distance = dot(ray.direction, point - ray.origin);
  return {distance, ray.origin + ray.direction * distance};
}

bool intersects(Ball ball, Ray ray) {
  auto [directional_distance, closest] = closestPoint(ray, ball.center);

  double closest_distance_squared = length_squared(closest - ball.center);
  return directional_distance > 0 &&
         closest_distance_squared < ball.radius * ball.radius;
}

std::tuple<bool, double, Vec3> intersection(Ray ray, Ball ball) {
  auto [directional_distance, closest_point] = closestPoint(ray, ball.center);

  if (directional_distance < 0) {
    return {};
  };

  double radial_distance_squared = length_squared(closest_point - ball.center);
  double offset_squared = ball.radius * ball.radius -
                          radial_distance_squared * radial_distance_squared;

  if (offset_squared < 0.0) {
    return {};
  }

  directional_distance -= std::sqrt(offset_squared);

  return {true, directional_distance,
          ray.origin + ray.direction * directional_distance};
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

int main(int argc, char **argv) {
  std::string filename{DEFAULT_FILENAME};

  if (argc == 2) {
    filename = argv[1];
  }

  std::ofstream file(filename);

  std::cout << "rendering into '" << filename << "'" << std::endl;

  Camera cam;
  Viewport view;

  Color background_color{0, 0, 128};
  std::vector<Ball> balls{
      {.center{-1, 10, 0}, .radius = 2, .color{0, 200, 0}},
      {.center{1.4, 10, 0}, .radius = 2, .color{200, 0, 0}},
      {.center{0, 1e5, -1e6}, .radius = 1e6 - 1, .color{200, 200, 222}},
  };

  file << "P3\n"
       << "# " << filename << "\n"
       << view.WIDTH << " " << view.HEIGHT << "\n"
       << 256 << "\n";

  for (int y = 0; y < view.HEIGHT; y++) {
    for (int x = 0; x < view.WIDTH; x++) {
      Ray ray = rayFromPixelPosition(x, y, cam, view);

      Color pixel_color{background_color};

      double minimal_distance = std::numeric_limits<double>::infinity();
      for (const auto &ball : balls) {
        auto [success, distance, point] = intersection(ray, ball);
        if (success && distance > 0 && distance < minimal_distance) {
          pixel_color = ball.color;
          minimal_distance = distance;
        }
      }

      file << pixel_color;
    }
    file << '\n';
  }

  return 0;
};
