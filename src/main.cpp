#include <glad/glad.h>

#include "game.h"
#include <GLFW/glfw3.h>

// ERROR CALLBACK
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

// KEY / MOUSE CALLBACKS
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  Game *game = (Game *)glfwGetWindowUserPointer(window);

  // quit
  if (key == GLFW_KEY_Q) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }
  // toggle render mode
  if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
    game->toggle_render_mode();
  }
  if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
    game->pipe = !game->pipe;
  }
  if (key == GLFW_KEY_O && action == GLFW_RELEASE) {
    game->simulation.init();
  }
    if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
      if (game->law) {
        std::cout << "turning on tate equation" << std::endl;
      } else {
        std::cout << "turning on ideal gas law" << std::endl;
      }
   game->law = !game->law;
  }
  // color modes
  if (key == GLFW_KEY_1) {
    game->color_mode = 1;
  } else if (key == GLFW_KEY_2) {
    game->color_mode = 2;
  } else if (key == GLFW_KEY_3) {
    game->color_mode = 3;
  } else if (key == GLFW_KEY_4) {
    game->color_mode = 4;
  } else if (key == GLFW_KEY_5) {
    game->color_mode = 5;
  } else if (key == GLFW_KEY_6) {
    game->color_mode = 6;
  } else if (key == GLFW_KEY_7) {
    game->color_mode = 7;
  } else if (key == GLFW_KEY_8) {
    game->color_mode = 8;
  } else if (key == GLFW_KEY_9) {
    game->color_mode = 9;
  }
  // movement
  if (action == GLFW_PRESS) {
    game->keyHeld[key] = true;
  } else if (action == GLFW_RELEASE) {
    game->keyHeld[key] = false;
  }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  Game *game = (Game *)glfwGetWindowUserPointer(window);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE) {
    game->mouse_pressed = true;
  } else {
    game->mouse_pressed = false;
  }
}

void MousePosCallback(GLFWwindow *window, double mouse_x, double mouse_y) {
  Game *game = (Game *)glfwGetWindowUserPointer(window);
  game->mouse_pos = glm::vec2(mouse_x, mouse_y);
}

int main(int argc, char *argv[]) {
  // create window/init glfw
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  GLFWwindow *window = glfwCreateWindow(640, 480, "sph", NULL, NULL);
  if (!window) {
    throw std::runtime_error("glfwCreateWindow error");
  }
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwSwapInterval(1);
  Game game(window);
  glfwSetWindowUserPointer(window, &game);
  // key / mouse callbacks
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, MousePosCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);

  game.init();
  while (!glfwWindowShouldClose(window)) {
    game.update();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
