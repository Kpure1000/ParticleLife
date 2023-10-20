#ifndef PARTICLE_H
#define PARTICLE_H

#include "glm/glm.hpp"
#include <vector>
#include <memory>

struct QuadTree;
struct QuadFlatTree;

enum ParticleType : int {
    A = 0,
    B,
    C,
    D,
    E,
    F,

    MAX_TPYE
};

glm::vec4 fromTypeToColor(ParticleType type);

struct ParticleForce {
    //
};

struct Particle2D {
    size_t size;
    
    const glm::vec2 *position();
    std::vector<glm::vec2> cur_position();
    std::vector<glm::vec4> color;

    std::vector<ParticleType> type;

    std::vector<glm::vec2> velocity;

    static std::shared_ptr<Particle2D> CreateRandom(size_t size);

    Particle2D(size_t size);

    void update();

    QuadTree *tree();
    QuadFlatTree *ftree();

private:

    glm::vec2 force(size_t i, size_t j, float rMax);
    
    std::array<std::vector<glm::vec2>,2> _position_buffer;
    
    unsigned int _position_id;

    QuadTree* m_tree = nullptr;
    QuadFlatTree* m_ftree = nullptr;

};

#endif