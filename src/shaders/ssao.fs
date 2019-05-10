#version 330 core
out vec4 fragment_color;
uniform vec2 resolution;
in vec2 uv;
uniform sampler2D screenTex;
uniform sampler2D depTex;
uniform int renderMode;

#define PI 3.14159265
// user variables
float width = resolution.x;
float height = resolution.y;

float zNear = 0.1;
float zFar = 20.0;
float strength = 1.0;

int samples = 8;    // ao sample count //64.0
float radius = 5.0; // ao radius //5.0

float aoclamp = 0.125;      // depth clamp - reduces haloing at screen edges
bool noise = true;          // use noise instead of pattern for sample dithering
float noiseamount = 0.0002; // dithering amount

float diffarea = 0.3;  // self-shadowing reduction
float gdisplace = 0.4; // gauss bell center //0.4

bool mist = false;     // use mist?
float miststart = 0.0; // mist start
float mistend = zFar;  // mist end

bool onlyAO = false;      // use only ambient occlusion pass?
float lumInfluence = 0.7; // how much luminance affects occlusion

//--------------------------------------------------------

vec2 rand(vec2 coord) // generating noise/pattern texture for dithering
{
  float noiseX = ((fract(1.0 - coord.s * (width / 2.0)) * 0.25) +
                  (fract(coord.t * (height / 2.0)) * 0.75)) *
                     2.0 -
                 1.0;
  float noiseY = ((fract(1.0 - coord.s * (width / 2.0)) * 0.75) +
                  (fract(coord.t * (height / 2.0)) * 0.25)) *
                     2.0 -
                 1.0;

  if (noise) {
    noiseX = clamp(fract(sin(dot(coord, vec2(12.9898, 78.233))) * 43758.5453),
                   0.0, 1.0) *
                 2.0 -
             1.0;
    noiseY =
        clamp(fract(sin(dot(coord, vec2(12.9898, 78.233) * 2.0)) * 43758.5453),
              0.0, 1.0) *
            2.0 -
        1.0;
  }
  return vec2(noiseX, noiseY) * noiseamount;
}

float readDepth(vec2 coord) {
  if (uv.x < 0.0 || uv.y < 0.0)
    return 1.0;
  else {
    float z_b = texture2D(depTex, coord).x;
    float z_n = 2.0 * z_b - 1.0;
    return (2.0 * zNear) / (zFar + zNear - z_n * (zFar - zNear));
  }
}

int compareDepthsFar(float depth1, float depth2) {
  float garea = 2.0;                      // gauss bell width
  float diff = (depth1 - depth2) * 100.0; // depth difference (0-100)
  // reduce left bell width to avoid self-shadowing
  if (diff < gdisplace) {
    return 0;
  } else {
    return 1;
  }
}

float compareDepths(float depth1, float depth2) {
  float garea = 2.0;                      // gauss bell width
  float diff = (depth1 - depth2) * 100.0; // depth difference (0-100)
  // reduce left bell width to avoid self-shadowing
  if (diff < gdisplace) {
    garea = diffarea;
  }

  float gauss = pow(2.7182, -2.0 * (diff - gdisplace) * (diff - gdisplace) /
                                (garea * garea));
  return gauss;
}

float calAO(float depth, float dw, float dh) {
  float dd = (1.0 - depth) * radius;

  float temp = 0.0;
  float temp2 = 0.0;
  float coordw = uv.x + dw * dd;
  float coordh = uv.y + dh * dd;
  float coordw2 = uv.x - dw * dd;
  float coordh2 = uv.y - dh * dd;

  vec2 coord = vec2(coordw, coordh);
  vec2 coord2 = vec2(coordw2, coordh2);

  float cd = readDepth(coord);
  int far = compareDepthsFar(depth, cd);
  temp = compareDepths(depth, cd);
  // DEPTH EXTRAPOLATION:
  if (far > 0) {
    temp2 = compareDepths(readDepth(coord2), depth);
    temp += (1.0 - temp) * temp2;
  }

  return temp;
}

void main(void) {
  if (renderMode == 1 || texture2D(screenTex, uv).w != 1.0) {
    fragment_color = texture2D(screenTex, uv);
    return;
  }

  vec2 noise = rand(uv);
  float depth = readDepth(uv);

  float w =
      (1.0 / width) / clamp(depth, aoclamp, 1.0) + (noise.x * (1.0 - noise.x));
  float h =
      (1.0 / height) / clamp(depth, aoclamp, 1.0) + (noise.y * (1.0 - noise.y));

  float pw = 0.0;
  float ph = 0.0;

  float ao = 0.0;

  float dl = PI * (3.0 - sqrt(5.0));
  float dz = 1.0 / float(samples);
  float l = 0.0;
  float z = 1.0 - dz / 2.0;

  for (int i = 0; i < 64; i++) {
    if (i > samples)
      break;
    float r = sqrt(1.0 - z);

    pw = cos(l) * r;
    ph = sin(l) * r;
    ao += calAO(depth, pw * w, ph * h);
    z = z - dz;
    l = l + dl;
  }

  ao /= float(samples);
  ao *= strength;
  ao = 1.0 - ao;

  // occlusion only
  if (renderMode == 3) {
    fragment_color = vec4(vec3(ao, ao, ao), 1.0);
    return;
  }

  vec3 color = texture2D(screenTex, uv).rgb;
  vec3 lumcoeff = vec3(0.299, 0.587, 0.114);
  float lum = dot(color.rgb, lumcoeff);
  vec3 luminance = vec3(lum, lum, lum);
  vec3 final = vec3(
      color * mix(vec3(ao), vec3(1.0),
                  luminance * lumInfluence)); // mix(color*ao, white, luminance)
  fragment_color = vec4(final, 1.0);
}