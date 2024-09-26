// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include "Colors.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

struct TrackSettings {
  int trackLength = 45;
  int trainHead = 10;
  int trainLength = 5;
  int switchPosition = 25;
  int trackMultiplier = 18;
  int framesPerMove = 60;
  ImU32 mainTrackPart1Color = ORANGE;
  ImU32 mainTrackPart2Color = ORANGE;
  ImU32 divergentTrackColor = RED;
  bool isSwitchFlipped = false;
  bool isTrainMoving = false;

  void Reset() { *this = TrackSettings(); }
};

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void RenderDialog(TrackSettings *currentSettings) {
  // Track Control Dialog Box
  ImGui::Begin("Track Controls");
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Track Length", &currentSettings->trackLength);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Train Head Position", &currentSettings->trainHead);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Train Length", &currentSettings->trainLength);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Switch Position", &currentSettings->switchPosition);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Track Multiplier", &currentSettings->trackMultiplier);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Frames Per Move", &currentSettings->framesPerMove);
  ImGui::Checkbox("Is Switch Flipped", &currentSettings->isSwitchFlipped);
  if (ImGui::Checkbox("Is Train Moving", &currentSettings->isTrainMoving)) {
    if (currentSettings->isTrainMoving) {
      currentSettings->mainTrackPart1Color = GREEN;
      if (!currentSettings->isSwitchFlipped) {
        currentSettings->mainTrackPart2Color = GREEN;
        currentSettings->divergentTrackColor = RED;
      } else {
        currentSettings->mainTrackPart2Color = RED;
        currentSettings->divergentTrackColor = GREEN;
      }
    } else {
      currentSettings->mainTrackPart1Color = ORANGE;
      if (!currentSettings->isSwitchFlipped) {
        currentSettings->mainTrackPart2Color = ORANGE;
        currentSettings->divergentTrackColor = RED;
      } else {
        currentSettings->mainTrackPart2Color = RED;
        currentSettings->divergentTrackColor = ORANGE;
      }
    }
  }

  if (ImGui::Button("Reset")) {
    currentSettings->Reset();
  }
}

void RenderMainTrack(int initialXPos, int initialYPos,
                     TrackSettings currentSettings) {
  // Draw line for the track
  ImDrawList *draw_list = ImGui::GetForegroundDrawList();
  ImVec2 p1 = ImVec2(initialXPos, initialYPos);
  ImVec2 p2 = ImVec2(initialXPos + currentSettings.switchPosition *
                                       currentSettings.trackMultiplier,
                     initialYPos);
  draw_list->AddLine(p1, p2, currentSettings.mainTrackPart1Color, 8.0f);
  p1 = ImVec2(initialXPos + currentSettings.switchPosition *
                                currentSettings.trackMultiplier,
              initialYPos);
  p2 = ImVec2(initialXPos +
                  currentSettings.trackLength * currentSettings.trackMultiplier,
              initialYPos);
  draw_list->AddLine(p1, p2, currentSettings.mainTrackPart2Color, 8.0f);
}

void RenderDivergentTrack(int initialXPos, int initialYPos,
                          TrackSettings currentSettings) {
  // Draw divergent track
  ImDrawList *draw_list = ImGui::GetForegroundDrawList();
  ImVec2 p1 = ImVec2(initialXPos + currentSettings.switchPosition *
                                       currentSettings.trackMultiplier,
                     initialYPos);
  ImVec2 p2 = ImVec2(initialXPos +
                         currentSettings.switchPosition *
                             currentSettings.trackMultiplier +
                         5 * currentSettings.trackMultiplier,
                     initialYPos - 5 * currentSettings.trackMultiplier);
  draw_list->AddLine(p1, p2, currentSettings.divergentTrackColor, 8.0f);
  p1 = ImVec2(initialXPos +
                  currentSettings.switchPosition *
                      currentSettings.trackMultiplier +
                  5 * currentSettings.trackMultiplier,

              initialYPos - 5 * currentSettings.trackMultiplier);
  p2 = ImVec2(initialXPos +
                  currentSettings.trackLength * currentSettings.trackMultiplier,
              initialYPos - 5 * currentSettings.trackMultiplier);
  draw_list->AddLine(p1, p2, currentSettings.divergentTrackColor, 8.0f);
}

void HandleTrainClick(bool &isTrainMoving, ImVec2 topLeft, ImVec2 bottomRight) {
  // Check if mouse is hovering over the square
  ImVec2 mouse_pos = ImGui::GetMousePos();
  bool isTrainHeadHovered =
      mouse_pos.x >= topLeft.x && mouse_pos.x <= bottomRight.x &&
      mouse_pos.y >= topLeft.y && mouse_pos.y <= bottomRight.y;

  // Start train on left click
  if (isTrainHeadHovered) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      isTrainMoving = true;
    }
  }
}

void RenderTrain(int initialXPos, int initialYPos, float trainSymbolsOffsetY,
                 TrackSettings *currentSettings) {
  // Draw square to represent train head
  ImVec2 topLeft = ImVec2(initialXPos + currentSettings->trainHead *
                                            currentSettings->trackMultiplier,
                          initialYPos - trainSymbolsOffsetY);
  float squareSize = 10.0f;
  ImVec2 bottomRight = ImVec2(topLeft.x + squareSize, topLeft.y + squareSize);

  ImDrawList *draw_list = ImGui::GetForegroundDrawList();
  draw_list->AddRectFilled(topLeft, bottomRight, IM_COL32(255, 255, 255, 255));

  // Draw other parts of train
  for (int i = 1; i < currentSettings->trainLength; i++) {
    float circle_radius = squareSize / 2;
    ImVec2 center = ImVec2(
        initialXPos +
            currentSettings->trainHead * currentSettings->trackMultiplier -
            i * currentSettings->trackMultiplier + circle_radius,
        initialYPos - trainSymbolsOffsetY / 2 - circle_radius);
    draw_list->AddCircleFilled(center, circle_radius,
                               IM_COL32(255, 255, 255, 255));
  }

  HandleTrainClick(currentSettings->isTrainMoving, topLeft, bottomRight);
}

// Main code
int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  bool show_demo_window = false;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      static TrackSettings currentSettings;
      static int frameCounter = 0;

      RenderDialog(&currentSettings);

      int initialXPos = 50;
      int initialYPos = 450;
      float trainSymbolsOffsetY = 20.0f;

      RenderMainTrack(initialXPos, initialYPos, currentSettings);
      RenderDivergentTrack(initialXPos, initialYPos, currentSettings);
      RenderTrain(initialXPos, initialYPos, trainSymbolsOffsetY,
                  &currentSettings);

      // Move the train on the screen
      if (currentSettings.isTrainMoving) {
        frameCounter++;

        if (frameCounter >= currentSettings.framesPerMove) {
          currentSettings.trainHead += 1;
          frameCounter = 0;

          // Reset position if it goes off the track
          if (currentSettings.trainHead >= currentSettings.trackLength) {
            currentSettings.isTrainMoving = false;
          }
        }
      }

      ImGui::End();
    }

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Text("This is some useful text."); // Display some text (you can
                                                // use a format strings too)
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f,
                         1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3(
          "clear color",
          (float *)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                   // widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin(
          "Another Window",
          &show_another_window); // Pass a pointer to our bool variable (the
                                 // window will have a closing button that will
                                 // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
