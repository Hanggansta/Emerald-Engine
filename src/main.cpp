#include "main.hpp"
#include "Camera.hpp"


// --------------------------

void init_renderer()
{
    renderer.shader_flat = load_shaders("src/shaders/flat.vert", "src/shaders/flat.frag");
    renderer.shader_forward = load_shaders("src/shaders/forward.vert", "src/shaders/forward.frag");

    renderer.compiled_shaders.push_back(renderer.shader_flat);
    renderer.compiled_shaders.push_back(renderer.shader_forward);

    renderer.set_forward();
}

// --------------------------

void init_uniforms()
{
    w2v_matrix = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    projection_matrix = glm::perspective(Y_FOV, ASPECT_RATIO, NEAR, FAR);
    for (int i = 0; i < renderer.compiled_shaders.size(); i ++) {
        glUseProgram(renderer.compiled_shaders[i]);
        glUniformMatrix4fv(glGetUniformLocation(renderer.compiled_shaders[i], "view"),
                           1, GL_FALSE, glm::value_ptr(w2v_matrix));
        glUniformMatrix4fv(glGetUniformLocation(renderer.compiled_shaders[i], "projection"),
                           1, GL_FALSE, glm::value_ptr(projection_matrix));
    }
    glUseProgram(0);
}

// --------------------------

void free_resources()
{
    sdl_quit(main_window, main_context);
}

// --------------------------

void cull_models()
{
    // TODO: Run in parallel
    for (auto model : Model::get_loaded_models()) {
        model->draw_me = camera.sphere_in_frustum(model->get_center_point(), model->bounding_sphere_radius);
    }
}

// --------------------------

void run()
{
    renderer.running = true;
    while (renderer.running) {
        handle_keyboard_input(camera, renderer);
        handle_mouse_input(camera);
        camera.update_culling_frustum();


        glClearColor(0.3, 0.3, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cull_models();
        renderer.upload_camera_uniforms(camera);

        // This is a call to our renderers member function pointer called render_function
        (renderer.*renderer.render_function)(Model::get_loaded_models());

        // Render flat objects (lightsources)
        renderer.set_flat();
        renderer.upload_camera_uniforms(camera);
        (renderer.*renderer.render_function)(Model::get_loaded_flat_models());

        renderer.set_forward();

        glBindVertexArray(0);
        SDL_GL_SwapWindow(main_window);
    }
}

// --------------------------

int main(int argc, char *argv[])
{
    if (!sdl_init(SCREEN_WIDTH, SCREEN_HEIGHT, main_window, main_context)) {
        Error::throw_error(Error::display_init_fail);
    }
    init_input();

    init_renderer();

    init_uniforms();

    // Load nanosuit model
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.3f, glm::vec3(0.f, 1.f, 0.f));

    load_scene("scene_file.txt");

    glm::vec3 p1 = glm::vec3(1.f);
    glm::vec3 p2 = glm::vec3(-3.f, 10.f, 2.f);
    Model* box1 = new Model("res/models/cube/cube.obj", glm::mat4(1.f), p1, true);
    Model* box2 = new Model("res/models/cube/cube.obj", glm::mat4(1.f), p2, true);

    // Load light sources into GPU
    Light light1 = Light(p1, glm::vec3(1.f));
    Light light2 = Light(p2, glm::vec3(1.f, 0.5f, 0.f));

    // attach light sources to boxes
    box1->attach_light(&light1, glm::vec3(0.0f));
    box2->attach_light(&light2, glm::vec3(0.0f));


    Light::upload_all();

    run();

    free_resources();
    return 0;
}
