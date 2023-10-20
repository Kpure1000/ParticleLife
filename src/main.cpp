#include "csugl.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glog/logging.h"
#include "glm/glm.hpp"

#include <memory>
#include <iostream>
#include <iomanip>

#include "particle.h"
#include "quadtree.h"

int main(int argc, char const *argv[])
{
    FLAGS_stderrthreshold = 0;
    FLAGS_log_dir = "./";
    google::InitGoogleLogging(argv[0]);

    auto app = csugl::singleton<csugl::Application>::getInstance();
    auto win = &app->GetWindow();

    csugl::singleton<csugl::thread_pool>::getInstance(csugl::get_core_numbers());

    auto shader_particle2d = csugl::Shader::Create("../assets/shader/particle2d.vert", "../assets/shader/particle.frag", "../assets/shader/particle2d.geom");
    auto shader_tree = csugl::Shader::Create("../assets/shader/tree.vert", "../assets/shader/tree.frag");

    csugl::Transform trans_ca{glm::vec3{.0f, .0f, 2.f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{1.0f, 1.0f, 1.0f}};
    auto aspect = static_cast<float>(win->GetSize().x) / win->GetSize().y;
    auto camera = csugl::OrthographicCamera::Create(aspect, 0.1f, 100.f, 50.0f);

    CSUGL_EVENT_BEGIN(win, &);
        CSUGL_DISPATCH_BEGIN(csugl::WindowResizeEvent, &)
            camera->aspect = static_cast<float>(ev.width) / ev.height;
            return false;
        CSUGL_DISPATCH_END();
        CSUGL_DISPATCH_BEGIN(csugl::MouseScrolledEvent, &)
            camera->zoom += static_cast<float>(ev.vertical) * 5.f * csugl::LowpTime::deltaTime();
            return false;
        CSUGL_DISPATCH_END();
    CSUGL_EVENT_END();

    auto particle = Particle2D::CreateRandom(30000);

    auto va = csugl::VertexArray::Creat();
    auto vb = csugl::VertexBuffer::Create<float>(particle->size * 6, nullptr, csugl::DYNAMIC_DRAW);
    va->addVertexAttribution(vb, {{
        {csugl::BufferElementType::Float2},
        {csugl::BufferElementType::Float4},
    }, false, particle->size});

    size_t tree_depth = static_cast<size_t>(std::min( static_cast<float>(QuadNode::max_depth), glm::log2(static_cast<float>(particle->cur_position().size())) * 0.5f ));
    size_t node_point_num = 8 * (tree_depth + 1) * particle->cur_position().size();
    LOG(INFO) << "depth prediction: " << tree_depth << "; node point prediction: " << node_point_num;

    auto vb_tree = csugl::VertexBuffer::Create<glm::vec2>(node_point_num * sizeof(glm::vec2), nullptr, csugl::DYNAMIC_DRAW);
    auto va_tree = csugl::VertexArray::Creat();
    va_tree->addVertexAttribution(vb_tree, {{
        {csugl::BufferElementType::Float2}
    }, false, node_point_num});

    csugl::LowpTime::init();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (app->isOpen())
    {
        // logic
        // TODO:
        particle->update();

        // render
        glClearColor(.1f, .1f, .1f, 1.f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        shader_particle2d->use();
        shader_particle2d->set_mat4("_view_proj", camera->get_projection_mat() * camera->get_view_mat(trans_ca));
        vb->bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, particle->size * sizeof(glm::vec2), (const void *)particle->position());
        glBufferSubData(GL_ARRAY_BUFFER, particle->size * sizeof(glm::vec2), particle->size * sizeof(glm::vec4), (const void *)particle->color.data());
        va->bind();
        
        glDrawArrays(GL_POINTS, 0, static_cast<uint16_t>(particle->size));

        std::vector<glm::vec2> box_position;
        // particle->tree()->root()->iterate(0, [&](const QuadNode *node, unsigned int depth) {
        particle->ftree()->iterate(0, 0, [&](const QuadFlatNode *node, unsigned int depth) {
            box_position.emplace_back(node->pmin);
            box_position.emplace_back(node->pmax.x,node->pmin.y);
            
            box_position.emplace_back(node->pmax.x,node->pmin.y);
            box_position.emplace_back(node->pmax);
            
            box_position.emplace_back(node->pmax);
            box_position.emplace_back(node->pmin.x,node->pmax.y);
            
            box_position.emplace_back(node->pmin.x,node->pmax.y);
            box_position.emplace_back(node->pmin);
        });

        vb_tree->bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, box_position.size() * sizeof(glm::vec2), (const void *)box_position.data());

        va_tree->bind();
        shader_tree->use();
        shader_tree->set_mat4("_view_proj", camera->get_projection_mat() * camera->get_view_mat(trans_ca));
        shader_tree->set_vec4("_color", {1.f, 1.f, 1.f, .5f});
        glDrawArrays(GL_LINES, 0, box_position.size());

        win->Display();
        csugl::LowpTime::update();
    }

    app->close();

    return 0;
}
