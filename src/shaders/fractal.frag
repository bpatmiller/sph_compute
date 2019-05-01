#version 330 core
out vec4 fragment_color;
in vec2 uv;

uniform float aspect;
uniform vec3 camera_position;
uniform vec3 fdir;
uniform vec3 sdir;
uniform mat2 rotation1;
uniform float xres;
uniform float yres;
uniform vec3 light_position;
uniform bool scary;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

float minimumStepDistance = 0.001;
int maxRaySteps = 64;
int iterations = 16;
float bailout = 100.0;
float scale = 1.9;

vec3 xDir = vec3(0.0005, 0, 0);
vec3 yDir = vec3(0, 0.0005, 0);
vec3 zDir = vec3(0, 0, 0.0005);

vec3 col1 = vec3(0.7, 0.2, 0.1);
vec3 col2 = vec3(0.76, 0.69, 0.5);

float DE(vec3 w) {
  float r;
  float x = w.x;
  float y = w.y;
  float z = w.z;
  float x1, y1, z1;
  int i;

  r = x * x + y * y + z * z;
  for (i = 0; i < iterations && r < bailout; i++) {

    // rotate(x,y,z)
    float x2 = x * rotation1[0][0] + y * rotation1[1][0];
    float y2 = x * rotation1[0][1] + y * rotation1[1][1];

    x = x2;
    y = y2;

    if (x + y < 0) {
      x1 = -y;
      y = -x;
      x = x1;
    }
    if (x + z < 0) {
      x1 = -z;
      z = -x;
      x = x1;
    }
    if (y + z < 0) {
      y1 = -z;
      z = -y;
      y = y1;
    }

    y2 = y * rotation1[0][0] + z * rotation1[1][0];
    float z2 = y * rotation1[0][1] + z * rotation1[1][1];

    y = y2;
    z = z2;

    x = scale * x - (scale - 1);
    y = scale * y - (scale - 1);
    z = scale * z - (scale - 1);
    r = x * x + y * y + z * z;
  }
  return (sqrt(r) - 2) * pow(scale, -i);
}

float DE_SCENE(vec3 w) { return DE(vec3(w.x, w.y, mod(w.z, 2))); }

vec3 getNormal(vec3 pos) {
  return normalize(vec3(DE(pos + xDir) - DE(pos - xDir),
                        DE(pos + yDir) - DE(pos - yDir),
                        DE(pos + zDir) - DE(pos - zDir)));
}

vec4 orbit_trap(vec3 w) {
  float r;
  float x = w.x;
  float y = w.y;
  float z = w.z;
  float x1, y1, z1;
  int i;
  vec4 m = vec4(99);

  r = x * x + y * y + z * z;
  for (i = 0; i < iterations && r < bailout; i++) {
    m.x = min(m.x, abs(x));
    m.y = min(m.y, abs(y));
    m.z = min(m.z, abs(z));
    m.w = min(m.w, r);
    // rotate(x,y,z)
    float x2 = x * rotation1[0][0] + y * rotation1[1][0];
    float y2 = x * rotation1[0][1] + y * rotation1[1][1];

    x = x2;
    y = y2;

    if (x + y < 0) {
      x1 = -y;
      y = -x;
      x = x1;
    }
    if (x + z < 0) {
      x1 = -z;
      z = -x;
      x = x1;
    }
    if (y + z < 0) {
      y1 = -z;
      z = -y;
      y = y1;
    }

    y2 = y * rotation1[0][0] + z * rotation1[1][0];
    float z2 = y * rotation1[0][1] + z * rotation1[1][1];

    y = y2;
    z = z2;

    x = scale * x - (scale - 1);
    y = scale * y - (scale - 1);
    z = scale * z - (scale - 1);
    r = x * x + y * y + z * z;
  }

  return m;
}

vec4 march(vec3 origin, vec3 direction) {
  float totalDistance = 0.0;
  for (int steps = 0; steps < maxRaySteps; steps++) {
    float dist = DE(origin + totalDistance * direction);
    totalDistance += dist;

    if (totalDistance > 4.0) {
      if (scary) {
        vec3 skyblue = vec3(0.7, 0.2, 0.6);
        vec3 skywhite = vec3(0);
        return vec4(mix(skyblue, skywhite, (direction.y + 2) / 2),
                    totalDistance);
      } else {
        vec3 skyblue = vec3(0.53, 0.8, 0.92);
        vec3 skywhite = vec3(1);
        return vec4(mix(skyblue, skywhite, direction.y), totalDistance);
      }
    }
    if (dist < minimumStepDistance) {
      vec3 normal = getNormal(origin + totalDistance * direction);
      vec4 orbit_color = orbit_trap(origin + totalDistance * direction);
      vec3 color;
      if (scary) {
        color = orbit_color.xyz * 2.0;
      } else {
        color = mix(col1, col2, pow(orbit_color.w * 2, 2));
      }
      float s = 1.0 - float(steps) / maxRaySteps;
      float diffuse_term;

      diffuse_term =
          clamp(dot(normalize((light_position -
                               (origin + totalDistance * direction))),
                    normal),
                0.5, 1);
      // diffuse_term = mix(diffuse_term, 0.7, totalDistance / 2);

      return vec4(s * diffuse_term * color, totalDistance);
    }
  }
}

void main() {
  vec3 ray_direction = normalize(
      vec3(sdir * uv.x * aspect + cross(fdir, sdir) * (-uv.y) + fdir));
  vec4 raymarch = march(camera_position, ray_direction);

  float f = 20;
  float n = 0.1;
  mat4 inv_view = inverse(view);
  vec4 screen_ray = vec4(ray_direction * raymarch.w, 1) * inv_view;
  float z_eye = screen_ray.z;
  float z_ndc = (-z_eye * (f + n) / (f - n) - 2 * f * n / (f - n)) / -z_eye;
  float depth = (z_ndc + 1) / 2;

  fragment_color = vec4(raymarch.xyz, depth);
}
