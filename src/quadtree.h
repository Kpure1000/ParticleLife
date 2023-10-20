#include "glm/glm.hpp"
#include <vector>
#include <functional>

#ifndef CONTAINED
#define CONTAINED(minx, miny, maxx, maxy, posx, posy) \
    (!(posx < minx) && !(posx > maxx) && !(posy < miny) && !(posy > maxy))
#endif

struct QuadFlatNode
{
    union
    {
        struct { long long child1, child2, child3, child4; };
        long long childs[4] = {-1, -1, -1, -1};
    };
    std::vector<std::pair<size_t, glm::vec2>> indices;

    glm::vec2 pmin, pmax;

    inline glm::vec2 centroid() const { return (pmax + pmin) * .5f; }

    bool intersect(const glm::vec2 &center, float radius);

    // 'z' shape quadrant
    // 1 | 2
    // ----- 
    // 3 | 4
    size_t quadrant(const glm::vec2 &pos);

    bool has_child() const { return child1 != -1 || child2 != -1 || child3 != -1 || child4 != -1; }
};

struct QuadNode
{
    union
    {
        struct { QuadNode *child1, *child2, *child3, *child4; };
        QuadNode *childs[4] = {nullptr, nullptr, nullptr, nullptr};
    };

    std::vector<std::pair<size_t, glm::vec2>> indices;

    glm::vec2 pmin, pmax;

    const static unsigned int max_depth;
    
    // 'z' shape quadrant
    // 1 | 2
    // ----- 
    // 3 | 4
    size_t quadrant(const glm::vec2 &pos);

    inline glm::vec2 centroid() const { return (pmax + pmin) * .5f; }

    inline static bool greaterThan(float a, float b) { return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon()); }

    inline static bool lessThan(float a, float b) { return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * std::numeric_limits<float>::epsilon()); }

    void split(unsigned int depth);

    bool insert(size_t index, const glm::vec2 &pos, unsigned int depth);

    void flache(std::vector<QuadFlatNode> &fnodes, long long parent_id, size_t quadr);

    void findRange(std::vector<size_t> &neighbors, const glm::vec2 &target, float range);

    inline bool has_child() const { return nullptr != child1; }

    bool intersect(const glm::vec2 &center, float radius);

    void iterate(unsigned int depth, const std::function<void(const QuadNode *, unsigned int)> &func) const;

    void destory();

};

struct QuadTree;

struct QuadFlatTree
{
    std::vector<QuadFlatNode> nodes;
    QuadFlatTree(std::vector<glm::vec2> vertices);
    QuadFlatTree(QuadTree * tree);

    void findRange(long long findex, std::vector<size_t> &neighbors, const glm::vec2 &target, float range);

    void split(long long findex, size_t depth);

    void iterate(long long findex, unsigned int depth, const std::function<void(const QuadFlatNode *, unsigned int)> &func) const;

    void insert(long long findex, size_t index, const glm::vec2 &pos, unsigned int depth);
};


struct QuadTree
{
    QuadTree(std::vector<glm::vec2> vertices);
    
    ~QuadTree();

    QuadNode *root() { return m_root; }

private:
    QuadNode *m_root = nullptr;

};


