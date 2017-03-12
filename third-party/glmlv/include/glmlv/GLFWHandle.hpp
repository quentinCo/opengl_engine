#pragma once

#include <glmlv/glfw.hpp>
#include <glmlv/gl_debug_output.hpp>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>

namespace glmlv
{

// Class responsible for initializing GLFW, creating a window, initializing OpenGL function pointers with GLAD library and initializing ImGUI
class GLFWHandle
{
public:
    GLFWHandle(int width, int height, const char * title)
    {
        if (!glfwInit()) {
            std::cerr << "Unable to init GLFW.\n";
            throw std::runtime_error("Unable to init GLFW.\n");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        m_pWindow = glfwCreateWindow(int(width), int(height), title, nullptr, nullptr);
        if (!m_pWindow) {
            std::cerr << "Unable to open window.\n";
            glfwTerminate();
            throw std::runtime_error("Unable to open window.\n");
        }

        glfwMakeContextCurrent(m_pWindow);

        glfwSwapInterval(0); // No VSync

        if (!gladLoadGL()) {
            std::cerr << "Unable to init OpenGL.\n";
            throw std::runtime_error("Unable to init OpenGL.\n");
        }

        glmlv::initGLDebugOutput();

        // Setup ImGui binding
        ImGui_ImplGlfwGL3_Init(m_pWindow, true);
    }

    ~GLFWHandle()
    {
        ImGui_ImplGlfwGL3_Shutdown();
        glfwTerminate();
    }

    // Non-copyable class:
    GLFWHandle(const GLFWHandle&) = delete;
    GLFWHandle& operator =(const GLFWHandle&) = delete;

    bool shouldClose() const
    {
        return glfwWindowShouldClose(m_pWindow);
    }

    glm::ivec2 framebufferSize() const
    {
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(m_pWindow, &displayWidth, &displayHeight);
        return glm::ivec2(displayWidth, displayHeight);
    }

    void swapBuffers() const
    {
        glfwSwapBuffers(m_pWindow);
    }

    GLFWwindow* window()
    {
        return m_pWindow;
    }

private:
    GLFWwindow* m_pWindow = nullptr;
};

}