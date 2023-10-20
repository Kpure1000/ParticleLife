#include "csugl.h"

#include "particle.h"

#include "quadtree.h"

#include "glog/logging.h"

#include <random>

const glm::vec2 *Particle2D::position()
{
    return _position_buffer[_position_id].data();
}

Particle2D::Particle2D(size_t size)
    : size(size), _position_id(0)
{
}

std::vector<glm::vec2> Particle2D::cur_position()
{
    return _position_buffer[_position_id];
}

void Particle2D::update()
{
    csugl::LogTimer timer("Particle2D::update");
    
    auto &last_position = _position_buffer[_position_id];
    auto &cur_position = _position_buffer[(_position_id + 1) % 2];

    // if (nullptr != m_tree) 
    // {
    //     delete m_tree;
    //     m_tree = nullptr;
    // }
    // m_tree = new QuadTree(last_position);
    
    if (nullptr != m_ftree) 
    {
        delete m_ftree;
        m_ftree = nullptr;
    }
    m_ftree = new QuadFlatTree(last_position);

    auto tp = csugl::singleton<csugl::thread_pool>::getInstance(0);
    float dt = 0.5f; //csugl::LowpTime::deltaTime() * 10.f;
    
    constexpr float rMax = 0.4f;

    std::vector<std::future<void>> res;
    for (size_t i = 0; i < last_position.size(); i++)
    {
        res.emplace_back(tp->submit([=, &last_position, &cur_position](){
            glm::vec2 force{.0f, .0f};
            
            std::vector<size_t> neibs;
            // this->tree()->root()->findRange(neibs, last_position[i], rMax);
            m_ftree->findRange(0, neibs, last_position[i], rMax);

            for (auto index : neibs)
            {
                force += this->force(i, index, rMax);
            }

            // for (size_t j = 0; j < last_position.size(); j++)
            // {
            //     force += this->force(i, j, rMax);
            // }

            velocity[i] = glm::pow(0.5f, dt / 20.0f) * velocity[i] + force * dt;
            cur_position[i] += velocity[i] * dt;
        }));
    }

    for (auto &r : res)
    {
        r.wait();
    }

    // for (size_t i = 0; i < last_position.size(); i++)
    // {
    //     glm::vec2 force{.0f, .0f};
    //     for (size_t j = 0; j < last_position.size(); j++)
    //     {
    //         force += this->force(i, j);
    //     }
    //     velocity[i] = glm::pow(0.5f, dt / 20.0f) * velocity[i] + force * dt;
    //     cur_position[i] += velocity[i] * dt;
    // }
    last_position = cur_position;
    _position_id = (_position_id + 1) % 2;
}

float alphaMat[ParticleType::MAX_TPYE][ParticleType::MAX_TPYE];

glm::vec2 Particle2D::force(size_t i, size_t j, float rMax)
{
    if (i == j)
        return {.0f, .0f};
    
    auto &last_position = _position_buffer[_position_id];

    glm::vec2 dir = last_position[j] - last_position[i];

    float distance2 = glm::max(glm::length2(dir), std::numeric_limits<float>::min()); 

    constexpr float beta = 0.3f;

    float r = glm::sqrt(distance2) / rMax;

    if (r < beta) {
        return dir * (r / beta - 1.f);
    } else if (beta < r && r < 1.f) {
        return dir * alphaMat[type[i]][type[j]] * (1.f - glm::abs(2 * r - 1.0f - beta) / (1.0f - beta));
    } else {
        return {.0f, .0f};
    }
}

QuadTree *Particle2D::tree()
{
    return m_tree;
}

QuadFlatTree *Particle2D::ftree()
{
    return m_ftree;
}

std::shared_ptr<Particle2D> Particle2D::CreateRandom(size_t size)
{
    auto particle = std::make_shared<Particle2D>(size);

    std::random_device rd;
    std::mt19937 eng;
    // eng.seed(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::normal_distribution<> nd{0.f, 10.f};
    std::uniform_int_distribution ud{0, static_cast<int>(ParticleType::MAX_TPYE)};
    std::normal_distribution udf{0.15f, 0.27f};

    auto &last_position = particle->_position_buffer[particle->_position_id];
    auto &cur_position = particle->_position_buffer[(particle->_position_id + 1) % 2];
    last_position.reserve(size);
    cur_position.reserve(size);
    particle->color.reserve(size);
    particle->velocity.reserve(size);

    for (size_t i = 0; i < ParticleType::MAX_TPYE; i++)
    {
        for (size_t j = 0; j <= i; j++)
        {
            alphaMat[i][j] = udf(eng);
            alphaMat[j][i] = alphaMat[i][j];
            // alphaMat[j][i] = udf(eng);
        }
    }
    

    for (size_t i = 0; i < size; i++)
    {
        auto x = static_cast<float>(nd(eng));
        auto y = static_cast<float>(nd(eng));
        auto vx = 0.001f * static_cast<float>(nd(eng));
        auto vy = 0.001f * static_cast<float>(nd(eng));
        last_position.emplace_back(x, y);
        cur_position.emplace_back(x, y);
        auto type = static_cast<ParticleType>(ud(eng) % ParticleType::MAX_TPYE);
        particle->type.emplace_back(type);
        particle->color.emplace_back(fromTypeToColor(type));
        particle->velocity.emplace_back(vx, vy);
    }
    
    return particle;
}

glm::vec4 fromTypeToColor(ParticleType type)
{
    CHECK(type < ParticleType::MAX_TPYE);
    glm::vec4 col;
    switch (type)
    {
    case ParticleType::A:
        col = glm::vec4{0.7f, .3f, .1f, 1.f};
        break;
    
    case ParticleType::B:
        col = glm::vec4{.3f, 0.09f, .1f, 1.f};
        break;
    
    case ParticleType::C:
        col = glm::vec4{0.8f, 0.9f, .1f, 1.f};
        break;
    
    case ParticleType::D:
        col = glm::vec4{.1f, .4f, 0.9f, 1.f};
        break;

    case ParticleType::E:
        col = glm::vec4{.4f, .9f, 0.2f, 1.f};
        break;

    case ParticleType::F:
        col = glm::vec4{.5f, .2f, 0.8f, 1.f};
        break;

    case ParticleType::MAX_TPYE:
        col = glm::vec4{1.0f, .0f, 1.0f, 1.f};
        break;
    }
    return col;
}
