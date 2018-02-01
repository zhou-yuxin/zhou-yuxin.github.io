#include <avl.h>
#include <common.h>

#define get_height(node) (((node)==0)?(-1):((node)->height))

#define max(a,b) ((a)>(b)?(a):(b))

/* RR(Y rotates to the right):
        k2                 k1
       / \                / \
      k1  Z     ======   X  k2
     / \                    / \
    X   Y                  Y   Z
*/
/*
 Return which the root pointer(at a higher level) should point to
 */
static struct avl_node* rr_rotate(struct avl_node* k2)
{
    assert(k2 != 0);

    struct avl_node* k1 = k2 -> left;
    k2 -> left = k1 -> right;
    k1 -> right = k2;
    k2 -> height = max(get_height(k2 -> left), get_height(k2 -> right)) + 1;
    k1 -> height = max(get_height(k1 -> left), k2 -> height) + 1;
    return k1;
}

/* LL(Y rotates to the left):
        k2                     k1
       / \                    / \
      X  k1       ======     k2  Z
        / \                 / \
        Y  Z               X   Y
 */
static struct avl_node* ll_rotate(struct avl_node* k2)
{
    assert(k2 != 0);

    struct avl_node* k1 = k2 -> right;
    k2 -> right = k1 -> left;
    k1 -> left = k2;
    k2 -> height = max(get_height(k2 -> left), get_height(k2 -> right)) +1;
    k1 -> height = max(get_height(k1 -> right), k2 -> height) +1;
    return k1;
}

/* LR(B rotates to the left, then C rotates to the right):
      k3                       k3                    k2
     / \                      / \                   / \
    k1  D                    k2  D                k1   k3
   / \      ======          / \     ======        / \  / \
  A   k2                   k1  C                 A  B C   D
      / \                 / \
     B  C                A   B
*/
/*
 Return which the root pointer should point to
 */
static struct avl_node* lr_rotate(struct avl_node* k3)
{
    assert(k3 != 0);

    k3 -> left = ll_rotate(k3 -> left);
    return rr_rotate(k3);
}

/* RL(D rotates to the right, then C rotates to the left):
       k3                      k3                       k2
      / \                     / \                      / \
     A  k1                   A   k2                   k3  k1 
        / \      ======          / \        ======    / \ / \
       k2  B                    C   k1               A  C D  B
       / \                         / \
      C   D                       D   B 
 */
static struct avl_node* rl_rotate(struct avl_node* k3)
{
    assert(k3 != 0);

    k3 -> right = rr_rotate(k3 -> right);
    return ll_rotate(k3);
}

/* return which the root pointer(at an outer/higher level) should point to,
   the root_node of AVL tree may change frequently during delete/insert,
   so the Root pointer should point to the REAL root node.
 */
struct avl_node* avl_insert(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2))
{
    assert(comparator != 0);
    
    if(root == 0)
    {
        root = malloc(sizeof(struct avl_node));
        root -> val = val;
        root -> left = 0;
        root -> right = 0;
        root -> height = 0;
        return root;
    }

    if(comparator(val , root -> val) < 0)
        root -> left = avl_insert(root -> left, val, comparator);
    else
        root -> right = avl_insert(root -> right, val, comparator);

    root -> height = max(get_height(root -> left), get_height(root -> right)) + 1;

    ssize_t height_delta= get_height(root -> left) - get_height(root -> right);
    if(height_delta == 2)
    {
        if(comparator(val, root -> left -> val) < 0)
            root = rr_rotate(root);
        else
            root = lr_rotate(root);
    }
    else if(height_delta == -2)
    {
        if(comparator(val, root -> right -> val) < 0)
            root = rl_rotate(root);
        else
            root = ll_rotate(root);
    }
    return root;
}

/* return which the root pointer(at an outer/higher level) should pointer to,
   cause the root_node of AVL tree may change frequently during delete/insert,
   so the Root pointer should point to the REAL root node.
 */
struct avl_node* avl_delete(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2), int* found)
{
    assert(comparator != 0);
    
    if(root == 0)
        return 0;

    int comp = comparator(val, root -> val);
    if(comp == 0)
    {
        if(root -> right == 0)
        {
            struct avl_node* temp = root;
            root = root -> left;
            free(temp);
            if(found != 0)
                (*found) = 1;
            return root;
        }
        else
        {
            struct avl_node* temp = root -> right;
            while(temp -> left)
                temp = temp -> left;
            root -> val = temp -> val;
            root -> right = avl_delete(root -> right, temp -> val, comparator, found);
        }
    }
    else if(comp < 0)
        root -> left = avl_delete(root -> left, val, comparator, found);
    else
        root -> right = avl_delete(root -> right, val, comparator, found);

    root -> height = max(get_height(root -> left), get_height(root -> right)) + 1;

    ssize_t height_delta = get_height(root -> left) - get_height(root -> right);
    if(height_delta == 2)
    {
        if(get_height(root -> left -> left) >= get_height(root -> left -> right))
            root = rr_rotate(root);
        else
            root = lr_rotate(root);
    }
    else if(height_delta == -2)
    {
        if(get_height(root -> right -> right) >= get_height(root -> right -> left))
            root = ll_rotate(root);
        else
            root = rl_rotate(root);
    }
    return root;
}

// get the node that contains the value that equals to the given one
struct avl_node* avl_search(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2))
{
    assert(comparator != 0);

    if(root == 0)
        return 0;

    int comp = comparator(val, root -> val);
    if(comp == 0)
        return root;
    else if(comp < 0)
        return avl_search(root -> left, val, comparator);
    else
        return avl_search(root -> right, val, comparator);
}

// get how many nodes in the tree
size_t avl_node_count(struct avl_node* root)
{
    if(root == 0)
        return 0;
    return 1 + avl_node_count(root -> left) + avl_node_count(root -> right);
}