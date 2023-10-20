#include "quadtree.h"

#include "glog/logging.h"

const unsigned int QuadNode::max_depth = 8;

bool QuadFlatNode::intersect(const glm::vec2 &center, float radius)
{
    glm::vec2 absOffset = glm::abs(center - this->centroid());
    glm::vec2 hsize = (pmax - pmin) * 0.5f;
    //  o | 
    //  -----
    //  \\|  
    if (absOffset.x < hsize.x && hsize.y + radius < absOffset.y)
        return false;
    //    | 
    //  -----
    //  \\| o 
    if (absOffset.y < hsize.y && hsize.x + radius < absOffset.x)
        return false;
    //    | o
    //  -----
    //  \\|  
    if (absOffset.x > hsize.x && absOffset.y > hsize.y && glm::dot(absOffset, hsize) > radius * radius)
        return false;
    
    return true;
}

size_t QuadFlatNode::quadrant(const glm::vec2 &pos)
{
    if (CONTAINED(pmin.x, pmin.y, pmax.x, pmax.y, pos.x, pos.y))
    {
        auto cent = this->centroid();
        if (CONTAINED(pmin.x, cent.y, cent.x, pmax.y, pos.x, pos.y))
        {
            return 1;
        }
        else if (CONTAINED(cent.x, cent.y, pmax.x, pmax.y, pos.x, pos.y)) 
        {
            return 2;
        }
        else if (CONTAINED(pmin.x, pmin.y, cent.x, cent.y, pos.x, pos.y))
        {
            return 3;
        }
        else if (CONTAINED(cent.x, pmin.y, pmax.x, cent.y, pos.x, pos.y))
        {
            return 4;
        }
        else 
        {
            return 0;
        }
        //
    } 
    else 
    {
        return 0;
    }
}

size_t QuadNode::quadrant(const glm::vec2 &pos)
{
    if (CONTAINED(pmin.x, pmin.y, pmax.x, pmax.y, pos.x, pos.y))
    {
        auto cent = this->centroid();
        if (CONTAINED(pmin.x, cent.y, cent.x, pmax.y, pos.x, pos.y))
        {
            return 1;
        }
        else if (CONTAINED(cent.x, cent.y, pmax.x, pmax.y, pos.x, pos.y)) 
        {
            return 2;
        }
        else if (CONTAINED(pmin.x, pmin.y, cent.x, cent.y, pos.x, pos.y))
        {
            return 3;
        }
        else if (CONTAINED(cent.x, pmin.y, pmax.x, cent.y, pos.x, pos.y))
        {
            return 4;
        }
        else 
        {
            return 0;
        }
        //
    } 
    else 
    {
        return 0;
    }
}

void QuadNode::split(unsigned int depth)
{
    auto cent = centroid();

    child1 = new QuadNode();
    child1->pmin = {pmin.x, cent.y};
    child1->pmax = {cent.x, pmax.y};

    child2 = new QuadNode();
    child2->pmin = cent;
    child2->pmax = pmax;

    child3 = new QuadNode();
    child3->pmin = pmin;
    child3->pmax = cent;

    child4 = new QuadNode();
    child4->pmin = {cent.x, pmin.y};
    child4->pmax = {pmax.x, cent.y};

    auto temp_indi = indices;
    indices.clear();
    // indices.shrink_to_fit();
    for (auto &index : temp_indi)
    {
        insert(index.first, index.second, depth + 1);
    }
}

bool QuadNode::insert(size_t index, const glm::vec2 &pos, unsigned int depth)
{
    auto qr = quadrant(pos);

    if (qr == 0)
        return false;

    if (nullptr != childs[qr - 1]) {
        return childs[qr - 1]->insert(index, pos, depth + 1);
    }

    indices.emplace_back(index, pos);
    
    if (indices.size() > 1 && depth < max_depth)
        split(depth);
    
    return true;
}

void QuadNode::flache(std::vector<QuadFlatNode> &fnodes, long long parent_id, size_t quadr)
{
    QuadFlatNode fnode;
    fnode.indices = this->indices;
    fnode.pmin = this->pmin;
    fnode.pmax = this->pmax;
    fnodes.emplace_back(fnode);
    long long cur_id = fnodes.size() - 1;
    if (parent_id >= 0)
    {
        fnodes[parent_id].childs[quadr] = cur_id;
    }
    if (has_child())
    {
        for (size_t i = 0; i < 4; i++)
        {
            childs[i]->flache(fnodes, cur_id, i);
        }
    }
}

void QuadNode::findRange(std::vector<size_t> &neighbors, const glm::vec2 &target, float range)
{
    if (intersect(target, range))
    {
        if (has_child())
        {
            for (size_t i = 0; i < 4; i++)
            {
                childs[i]->findRange(neighbors, target, range);
            }
        }
        else 
        {
            for (auto &[id, pos] : indices)
                neighbors.emplace_back(id);
        }
    }
}

bool QuadNode::intersect(const glm::vec2 &center, float radius)
{
    glm::vec2 absOffset = glm::abs(center - this->centroid());
    glm::vec2 hsize = (pmax - pmin) * 0.5f;
    //  o | 
    //  -----
    //  \\|  
    if (absOffset.x < hsize.x && hsize.y + radius < absOffset.y)
        return false;
    //    | 
    //  -----
    //  \\| o 
    if (absOffset.y < hsize.y && hsize.x + radius < absOffset.x)
        return false;
    //    | o
    //  -----
    //  \\|  
    if (absOffset.x > hsize.x && absOffset.y > hsize.y && glm::dot(absOffset, hsize) > radius * radius)
        return false;
    
    return true;
}

void QuadNode::iterate(unsigned int depth, const std::function<void(const QuadNode *, unsigned int)> &func) const
{
    func(this, depth);
    if (has_child())
    {
        for (const auto &child : childs)
        {
            child->iterate(depth + 1, func);
        }
    }
}

void QuadNode::destory()
{
    if (has_child()) 
    {
        for (size_t i = 0; i < 4; i++)
        {
            childs[i]->destory();
        }
    }
    delete this;
}

QuadFlatTree::QuadFlatTree(std::vector<glm::vec2> vertices)
{
    glm::vec2 vmin{std::numeric_limits<float>::max()};
    glm::vec2 vmax{std::numeric_limits<float>::min()};
    for (auto & pos : vertices) {
        vmin = glm::min(pos, vmin);
        vmax = glm::max(pos, vmax);
    }
    vmin -= glm::vec2{1.f};
    vmax += glm::vec2{1.f};

    // size_t tree_depth = static_cast<size_t>(std::min( static_cast<float>(QuadNode::max_depth), glm::log2(static_cast<float>(vertices.size())) * 0.5f ));
    // size_t node_num = (tree_depth + 1) * vertices.size();

    // nodes.reserve(node_num);

    QuadFlatNode root;
    root.pmax = vmax;
    root.pmin = vmin;
    nodes.emplace_back(root);

    for (size_t i = 0; i < vertices.size(); i++)
    {
        insert(0, i, vertices[i], 0);
    }
}

QuadFlatTree::QuadFlatTree(QuadTree * tree)
{
    tree->root()->flache(this->nodes, -1, 0);
}

void QuadFlatTree::findRange(long long findex, std::vector<size_t> &neighbors, const glm::vec2 &target, float range)
{
    if (findex < 0)
        return;
    QuadFlatNode& cur_node = nodes[findex];
    if (cur_node.intersect(target, range))
    {
        if (cur_node.has_child())
        {
            for (size_t i = 0; i < 4; i++)
            {
                findRange(cur_node.childs[i], neighbors, target, range);
            }
        }
        else 
        {
            for (auto &[id, pos] : cur_node.indices)
            {
                neighbors.emplace_back(id);
            }
        }
    }
}

void QuadFlatTree::iterate(long long findex, unsigned int depth, const std::function<void(const QuadFlatNode *, unsigned int)> &func) const
{
    const QuadFlatNode* cur_node = &nodes[findex];
    func(cur_node, depth);
    if (cur_node->has_child())
    {
        for (const auto child : cur_node->childs)
        {
            iterate(child, depth + 1, func);
        }
    }
}

void QuadFlatTree::split(long long findex, size_t depth)
{
    auto cent = nodes[findex].centroid();
    
    QuadFlatNode child1;
    child1.pmin = {nodes[findex].pmin.x, cent.y};
    child1.pmax = {cent.x, nodes[findex].pmax.y};
    nodes.emplace_back(child1);
    nodes[findex].child1 = nodes.size() - 1;

    QuadFlatNode child2;
    child2.pmin = cent;
    child2.pmax = nodes[findex].pmax;
    nodes.emplace_back(child2);
    nodes[findex].child2 = nodes.size() - 1;

    QuadFlatNode child3;
    child3.pmin = nodes[findex].pmin;
    child3.pmax = cent;
    nodes.emplace_back(child3);
    nodes[findex].child3 = nodes.size() - 1;

    QuadFlatNode child4;
    child4.pmin = {cent.x, nodes[findex].pmin.y};
    child4.pmax = {nodes[findex].pmax.x, cent.y};
    nodes.emplace_back(child4);
    nodes[findex].child4 = nodes.size() - 1;

    auto temp_indi = nodes[findex].indices;
    
    nodes[findex].indices.clear();
    
    for (auto &index : temp_indi)
    {
        insert(findex, index.first, index.second, depth + 1);
    }
}

void QuadFlatTree::insert(long long findex, size_t index, const glm::vec2 &pos, unsigned int depth)
{
    auto qr = nodes[findex].quadrant(pos);

    if (qr == 0)
        return;
    
    if (-1 != nodes[findex].childs[qr - 1])
    {
        insert(nodes[findex].childs[qr - 1], index, pos, depth + 1);
        return;
    }

    nodes[findex].indices.emplace_back(index, pos);

    if (nodes[findex].indices.size() > 1 && depth < QuadNode::max_depth)
    {
        split(findex, depth);
    }
}

QuadTree::QuadTree(std::vector<glm::vec2> vertices)
{
    glm::vec2 vmin{std::numeric_limits<float>::max()};
    glm::vec2 vmax{std::numeric_limits<float>::min()};
    for (auto & pos : vertices) {
        vmin = glm::min(pos, vmin);
        vmax = glm::max(pos, vmax);
    }
    vmin -= glm::vec2{1.f};
    vmax += glm::vec2{1.f};

    m_root = new QuadNode();
    m_root->pmin = vmin;
    m_root->pmax = vmax;

    for (size_t i = 0; i < vertices.size(); i++)
    {
        m_root->insert(i, vertices[i], 0);
    }
}

QuadTree::~QuadTree()
{
    m_root->destory();
}
