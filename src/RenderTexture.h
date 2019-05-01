#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

class RenderTexture {
public:
  void create(int width, int height);
  void bind();
  void unbind();
  int getTexture() const { return tex; }
  void bindTexture();
  void unbindTexture();
  void changeSize(int width, int height);

  RenderTexture();

  int w, h;
  unsigned int fb = -1;
  unsigned int tex = -1;
  unsigned int dep = -1;
  void release();
};

#endif
