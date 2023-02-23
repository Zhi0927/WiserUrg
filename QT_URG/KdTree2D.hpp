#ifndef ZHI_KDTREE_H_
#define ZHI_KDTREE_H_

#include <vector>


struct Node {
    std::vector<float> point; //the data of node like (x, y)
    int id;
    Node* left;
    Node* right;

    Node(std::vector<float> arr, int setId) :
        point(arr),
        id(setId),
        left(NULL),
        right(NULL) {}
};

struct KdTree2D {
    Node* root;

    KdTree2D() : root(NULL) {}

    // Kd-Tree insert
    void insertHelper(Node** node, size_t depth, std::vector<float> point, int id) {
        // Tree is empty
        if (*node == NULL) {
            *node = new Node(point, id);
        }
        else {
            // calculate current dim (1 means x axes, 2means y axes)
            size_t cd = depth % 2;
            if (point[cd] < ((*node)->point[cd])) {
                insertHelper(&((*node)->left), depth + 1, point, id);
            }
            else {
                insertHelper(&((*node)->right), depth + 1, point, id);
            }
        }
    }

    void insert(std::vector<float> point, int id) {
        // TODO: Fill in this function to insert a new point into the tree
        // the function should create a new node and place correctly with in the root
        insertHelper(&root, 0, point, id);
    }
    // #############################################################################################################

    // Kd-Tree search
    void searchHelper(std::vector<float> target, Node* node, int depth, float distanceTol, std::vector<int>& ids)
    {
        if (node != NULL)
        {
            // Check whether the node inside box  or not, point[0] means x axes,¡@point[1]means y axes
            if ((node->point[0] >= (target[0] - distanceTol) && node->point[0] <= (target[0] + distanceTol)) && (node->point[1] >= (target[1] - distanceTol) && node->point[1] <= (target[1] + distanceTol)))
            {
                float distance = sqrt((node->point[0] - target[0]) * (node->point[0] - target[0]) + (node->point[1] - target[1]) * (node->point[1] - target[1]));
                if (distance <= distanceTol)
                {
                    ids.push_back(node->id);
                }
            }
            // check across boundary
            if ((target[depth % 2] - distanceTol) < node->point[depth % 2])
            {
                searchHelper(target, node->left, depth + 1, distanceTol, ids);
            }
            if ((target[depth % 2] + distanceTol) > node->point[depth % 2])
            {
                searchHelper(target, node->right, depth + 1, distanceTol, ids);
            }

        }
    }

    // return a list of point ids in the tree that are within distance of target
    std::vector<int> search(std::vector<float> target, float distanceTol)
    {
        std::vector<int> ids;
        searchHelper(target, root, 0, distanceTol, ids);
        return ids;
    }
};


#endif
