#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include "Colors.h"
#include <GLFW/glfw3.h>

struct TrackSettings {
  int trackLength = 45;
  float trainHeadX = 10.0f;
  float trainHeadY = 0.0f;
  int trainLength = 5;
  int switchPosition = 25;
  int trackMultiplier = 18;
  int framesPerMove = 30;
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

void updateColors(TrackSettings *currentSettings) {
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

void RenderDialog(TrackSettings *currentSettings) {
  // Track Control Dialog Box
  ImGui::Begin("Track Controls");
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Track Length", &currentSettings->trackLength);
  ImGui::SetNextItemWidth(100);
  ImGui::InputFloat("Train Head Position", &currentSettings->trainHeadX);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Train Length", &currentSettings->trainLength);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Switch Position", &currentSettings->switchPosition);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Track Multiplier", &currentSettings->trackMultiplier);
  ImGui::SetNextItemWidth(100);
  ImGui::InputInt("Frames Per Move", &currentSettings->framesPerMove);
  if (ImGui::Checkbox("Is Switch Flipped", &currentSettings->isSwitchFlipped)) {
    updateColors(currentSettings);
  }
  if (ImGui::Checkbox("Is Train Moving", &currentSettings->isTrainMoving)) {
    updateColors(currentSettings);
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

void HandleTrainClick(TrackSettings *currentSettings, ImVec2 topLeft,
                      ImVec2 bottomRight) {

  // Check if mouse is hovering over the square
  ImVec2 mouse_pos = ImGui::GetMousePos();
  bool isTrainHeadHovered =
      mouse_pos.x >= topLeft.x && mouse_pos.x <= bottomRight.x &&
      mouse_pos.y >= topLeft.y && mouse_pos.y <= bottomRight.y;

  // Start train on left click
  if (isTrainHeadHovered) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      currentSettings->isTrainMoving = true;
      updateColors(currentSettings);
    }
  }
}

void RenderTrain(int initialXPos, int initialYPos, float trainSymbolsOffsetY,
                 TrackSettings *currentSettings) {
  // Draw square to represent train head
  ImVec2 topLeft = ImVec2(initialXPos + currentSettings->trainHeadX *
                                            currentSettings->trackMultiplier,
                          initialYPos - trainSymbolsOffsetY +
                              currentSettings->trainHeadY *
                                  currentSettings->trackMultiplier);
  float squareSize = 10.0f;
  ImVec2 bottomRight = ImVec2(topLeft.x + squareSize, topLeft.y + squareSize);

  ImDrawList *draw_list = ImGui::GetForegroundDrawList();
  draw_list->AddRectFilled(topLeft, bottomRight, IM_COL32(255, 255, 255, 255));

  // Draw other parts of train
  for (int i = 1; i < currentSettings->trainLength; i++) {
    float circle_radius = squareSize / 2;
    ImVec2 center = ImVec2(
        initialXPos +
            currentSettings->trainHeadX * currentSettings->trackMultiplier -
            i * currentSettings->trackMultiplier + circle_radius,
        initialYPos - trainSymbolsOffsetY / 2 - circle_radius +
            currentSettings->trainHeadY * currentSettings->trackMultiplier);
    draw_list->AddCircleFilled(center, circle_radius,
                               IM_COL32(255, 255, 255, 255));
  }

  HandleTrainClick(currentSettings, topLeft, bottomRight);
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
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Railway Demo", nullptr, nullptr);
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
          frameCounter = 0;

          ImVec2 p1 = ImVec2(initialXPos + currentSettings.switchPosition *
                                               currentSettings.trackMultiplier,
                             initialYPos);
          ImVec2 p2 = ImVec2(initialXPos +
                                 currentSettings.switchPosition *
                                     currentSettings.trackMultiplier +
                                 5 * currentSettings.trackMultiplier,
                             initialYPos - 5 * currentSettings.trackMultiplier);

          // If the train is on the sloped portion and the switch is flipped
          if ((initialXPos +
                       currentSettings.trainHeadX *
                           currentSettings.trackMultiplier +
                       trainSymbolsOffsetY <
                   p1.x ||
               initialXPos +
                       currentSettings.trainHeadX *
                           currentSettings.trackMultiplier +
                       trainSymbolsOffsetY >=
                   p2.x) ||
              !currentSettings.isSwitchFlipped) {
            currentSettings.trainHeadX += 1;

          } else {
            // Calculate the slope between p1 and p2
            float deltaX = p2.x - p1.x;
            float deltaY = p2.y - p1.y;

            // Move the train along this slope after it reaches the switch
            currentSettings.trainHeadX +=
                (deltaX / ((currentSettings.trackLength -
                            currentSettings.switchPosition) *
                           currentSettings.trackMultiplier)) *
                2;
            currentSettings.trainHeadY +=
                (deltaY / ((currentSettings.trackLength -
                            currentSettings.switchPosition) *
                           currentSettings.trackMultiplier)) *
                2;
          }

          // Reset position if it goes off the track
          if (currentSettings.trainHeadX >= currentSettings.trackLength) {
            currentSettings.isTrainMoving = false;
            currentSettings.mainTrackPart1Color = RED;
            currentSettings.mainTrackPart2Color = RED;
            currentSettings.divergentTrackColor = RED;
          }
        }
      }

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
