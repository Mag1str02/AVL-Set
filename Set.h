#include <cstddef>
#include <iostream>
#include <iterator>

template <class ValueType> class Set
{
private:
    struct Node
    {
        size_t    height;
        ValueType value;
        Node*     left;
        Node*     right;
        Node*     parent;

        bool UnBalanced() { return std::abs(Delta()) > 1; }
        bool Update()
        {
            size_t height_ = height;
            height         = 1 + std::max((left ? left->height : 0), (right ? right->height : 0));
            return height_ != height;
        }
        bool  IsLeaf() { return !left && !right; }
        bool  IsCouple() { return left && right; }
        bool  IsSingle() { return !IsLeaf() && !IsCouple(); }
        Node* GetHighestChild()
        {
            size_t l = 0;
            size_t r = 0;
            if (left)
            {
                l = left->height;
            }
            if (right)
            {
                r = right->height;
            }
            if (l > r)
            {
                return left;
            }
            return right;
        }
        Node* GetChild()
        {
            if (left)
            {
                return left;
            }
            if (right)
            {
                return right;
            }
            return nullptr;
        }
        int Delta()
        {
            if (IsSingle())
            {
                return static_cast<int>(GetChild()->height);
            }
            if (IsCouple())
            {
                return static_cast<int>(left->height) - static_cast<int>(right->height);
            }
            return 0;
        }
    };

    size_t _size;
    Node*  _root;

    //*---Node-Support-Methods-----------------------------------------------------------------------------------------

    static bool Same(const Node* node, const ValueType& other)
    {
        if (!node)
        {
            return false;
        }
        return !(node->value < other) && !(other < node->value);
    }
    static bool BiggerOrEqual(const Node* node, const ValueType& other)
    {
        if (!node)
        {
            return false;
        }
        return !(node->value < other);
    }

    static bool LeftChild(Node* child, Node* parent)
    {
        if (!child || !parent) return false;
        return parent->left == child;
    }
    static bool RightChild(Node* child, Node* parent)
    {
        if (!child || !parent) return false;
        return parent->right == child;
    }

    static void BindChildToParent(Node* child, Node* parent)
    {
        child->parent = parent;
        if (!parent)
        {
            return;
        }
        if (child->value < parent->value)
        {
            parent->left = child;
        }
        else
        {
            parent->right = child;
        }
    }
    static void UnbindChildFromParent(Node* child, Node* parent)
    {
        if (!parent)
        {
            return;
        }
        if (parent->left == child)
        {
            parent->left = nullptr;
        }
        else if (parent->right == child)
        {
            parent->right = nullptr;
        }
    }
    static void SwapChild(Node* child, Node* parent, Node* child_to_swap)
    {
        if (parent->left == child_to_swap)
        {
            SetLeftChild(child, parent);
            return;
        }
        if (parent->right == child_to_swap)
        {
            SetRightChild(child, parent);
        }
    }

    static void SetLeftChild(Node* child, Node* parent)
    {
        if (parent)
        {
            parent->left = child;
        }
        if (child)
        {
            child->parent = parent;
        }
    }
    static void SetRightChild(Node* child, Node* parent)
    {
        if (parent)
        {
            parent->right = child;
        }
        if (child)
        {
            child->parent = parent;
        }
    }

    Node* GenNode(const ValueType& value)
    {
        ++_size;
        Node* res   = new Node();
        res->value  = value;
        res->height = 1;
        res->left   = nullptr;
        res->right  = nullptr;
        res->parent = nullptr;
        return res;
    }
    void DeleteNode(Node* node)
    {
        --_size;
        delete node;
    }

    //*---Navigation---------------------------------------------------------------------------------------------------

    static Node* GoLeftDownMax(Node* from)
    {
        if (!from)
        {
            return nullptr;
        }
        while (from->left)
        {
            from = from->left;
        }
        return from;
    }
    static Node* GoRightDownMax(Node* from)
    {
        if (!from)
        {
            return nullptr;
        }
        while (from->right)
        {
            from = from->right;
        }
        return from;
    }

    static Node* GoUpTillRight(Node* from)
    {
        while (from->parent)
        {
            if (LeftChild(from, from->parent))
            {
                return from->parent;
            }
            from = from->parent;
        }
        return nullptr;
    }
    static Node* GoUpTillLeft(Node* from)
    {
        Node* begin = from;
        while (from->parent)
        {
            if (RightChild(from, from->parent))
            {
                return from->parent;
            }
            from = from->parent;
        }
        return begin;
    }

    static Node* Next(Node* node)
    {
        if (!node)
        {
            return nullptr;
        }
        if (node->right)
        {
            return GoLeftDownMax(node->right);
        }
        return GoUpTillRight(node);
    }
    static Node* Prev(Node* node, const Set<ValueType>* owner)
    {
        if (!node)
        {
            return GoRightDownMax(owner->_root);
        }
        if (node->left)
        {
            return GoRightDownMax(node->left);
        }
        return GoUpTillLeft(node);
    }

    Node* GoDownWithValue(Node* root, const ValueType& value) const
    {
        while (root)
        {
            if (Same(root, value))
            {
                return root;
            }
            if (root->value < value)
            {
                if (!root->right)
                {
                    return root;
                }
                root = root->right;
            }
            else
            {
                if (!root->left)
                {
                    return root;
                }
                root = root->left;
            }
        }
        return nullptr;
    }

    //*---Rotations----------------------------------------------------------------------------------------------------

    Node* LeftRotate(Node* node)
    {
        Node* node_r   = node->right;
        Node* node_r_l = node_r->left;
        Node* parrent  = node->parent;

        if (parrent)
        {
            SwapChild(node_r, parrent, node);
        }
        else
        {
            node_r->parent = nullptr;
            _root          = node_r;
        }

        SetLeftChild(node, node_r);
        SetRightChild(node_r_l, node);
        UpdateTill(node, nullptr);
        return node_r;
    }
    Node* RightRotate(Node* node)
    {
        Node* node_l   = node->left;
        Node* node_l_r = node_l->right;
        Node* parrent  = node->parent;

        if (parrent)
        {
            SwapChild(node_l, parrent, node);
        }
        else
        {
            node_l->parent = nullptr;
            _root          = node_l;
        }
        SetRightChild(node, node_l);
        SetLeftChild(node_l_r, node);
        UpdateTill(node, nullptr);

        return node_l;
    }

    //*---Insertion&Deletion------------------------------------------------------------------------------------------

    void UpdateTill(Node* updater, Node* till)
    {
        while (updater != till)
        {
            if (!updater->Update())
            {
                break;
            }
            updater = updater->parent;
        }
    }
    void Balance(Node* new_node)
    {
        if (!new_node)
        {
            return;
        }
        int current_turn  = -1;
        int previous_turn = -1;
        if (new_node && new_node->UnBalanced())
        {
            new_node = new_node->GetHighestChild();
            new_node = new_node->GetHighestChild();
        }
        while (new_node->parent)
        {
            previous_turn = current_turn;
            current_turn  = (new_node->parent->left == new_node ? 0 : 1);
            new_node      = new_node->parent;
            if (new_node->UnBalanced())
            {
                if (current_turn == 0 && previous_turn == 0)
                {
                    new_node = RightRotate(new_node);
                }
                else if (current_turn == 0 && previous_turn == 1)
                {
                    LeftRotate(new_node->left);
                    new_node = RightRotate(new_node);
                }
                else if (current_turn == 1 && previous_turn == 0)
                {
                    RightRotate(new_node->right);
                    new_node = LeftRotate(new_node);
                }
                else if (current_turn == 1 && previous_turn == 1)
                {
                    new_node = LeftRotate(new_node);
                }
            }
        }
    }

    Node* BSTInsert(const ValueType& value)
    {
        Node* parent = GoDownWithValue(_root, value);
        if (Same(parent, value))
        {
            return nullptr;
        }

        Node* new_node = GenNode(value);
        if (!parent)
        {
            _root = new_node;
        }
        else
        {
            BindChildToParent(new_node, parent);
            UpdateTill(parent, nullptr);
        }
        return new_node;
    }
    Node* BSTErase(Node* root, const ValueType& value)
    {
        Node* node_to_delete = GoDownWithValue(root, value);
        if (!Same(node_to_delete, value))
        {
            return nullptr;
        }

        if (node_to_delete->IsCouple())
        {
            Node* new_node_to_delete = GoLeftDownMax(node_to_delete->right);
            node_to_delete->value    = new_node_to_delete->value;
            return BSTErase(node_to_delete->right, new_node_to_delete->value);
        }
        else
        {
            Node* ans;
            if (node_to_delete->IsLeaf())
            {
                UnbindChildFromParent(node_to_delete, node_to_delete->parent);
                if (!node_to_delete->parent)
                {
                    _root = nullptr;
                }
                ans = node_to_delete->parent;
            }
            else
            {
                Node* child = node_to_delete->GetChild();
                BindChildToParent(child, node_to_delete->parent);
                if (node_to_delete == _root)
                {
                    _root = child;
                }
                ans = child;
            }
            UpdateTill(node_to_delete->parent, nullptr);
            DeleteNode(node_to_delete);
            return ans;
        }
    }

    Node* Copy(Node* other_node)
    {
        if (!other_node)
        {
            return nullptr;
        }

        Node* new_node = GenNode(other_node->value);
        new_node->left = Copy(other_node->left);
        if (new_node->left)
        {
            new_node->left->parent = new_node;
        }
        new_node->right = Copy(other_node->right);
        if (new_node->right)
        {
            new_node->right->parent = new_node;
        }
        return new_node;
    }
    void DeleteSubTree(Node* node)
    {
        if (node)
        {
            DeleteSubTree(node->left);
            DeleteSubTree(node->right);
            DeleteNode(node);
        }
    }

public:
    class iterator
    {
    private:
        Node*                 _node;
        const Set<ValueType>* _owner;

    public:
        iterator() : _node(nullptr), _owner(nullptr) {}
        iterator(Node* node, const Set<ValueType>* owner) : _node(node), _owner(owner) {}

        const ValueType& operator*() const { return _node->value; }
        const ValueType* operator->() const { return &(_node->value); }
        iterator&        operator++()
        {
            _node = Next(_node);
            return *this;
        }
        iterator operator++(int)
        {
            iterator res(_node, _owner);
            _node = Next(_node);
            return res;
        }
        iterator& operator--()
        {
            _node = Prev(_node, _owner);
            return *this;
        }
        iterator operator--(int)
        {
            iterator res(_node, _owner);
            _node = Prev(_node, _owner);
            return res;
        }

        bool operator==(const iterator& other) const { return _node == other._node && _owner == other._owner; }
        bool operator!=(const iterator& other) const { return _node != other._node || _owner != other._owner; }
    };

    Set<ValueType>() : _size(0), _root(nullptr) {}
    Set<ValueType>(const Set<ValueType>& other) : _size(0) { _root = Copy(other._root); }
    Set<ValueType>& operator=(const Set<ValueType>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        clear();
        _root = Copy(other._root);
        return *this;
    }

    template <typename Iterator> Set<ValueType>(Iterator first, Iterator last) : Set<ValueType>()
    {
        for (Iterator i = first; i != last; ++i)
        {
            insert(*i);
        }
    }
    Set<ValueType>(const std::initializer_list<ValueType>& list) : Set<ValueType>()
    {
        for (const auto& value : list)
        {
            insert(value);
        }
    }

    void clear()
    {
        DeleteSubTree(_root);

        _size = 0;
        _root = nullptr;
    }
    ~Set<ValueType>() { DeleteSubTree(_root); }

    size_t size() const { return _size; }
    bool   empty() const { return _root == nullptr; }

    void insert(const ValueType& value)
    {
        Node* inserted_node = BSTInsert(value);
        Balance(inserted_node);
    }
    void erase(const ValueType& value)
    {
        Node* node_to_delete = BSTErase(_root, value);
        Balance(node_to_delete);
    }

    iterator begin() const { return iterator(GoLeftDownMax(_root), this); }
    iterator end() const { return iterator(nullptr, this); }

    iterator find(const ValueType& value) const
    {
        Node* res = GoDownWithValue(_root, value);
        if (Same(res, value))
        {
            return iterator(res, this);
        }
        return end();
    }
    iterator lower_bound(const ValueType& value) const
    {
        Node* res = GoDownWithValue(_root, value);
        while (res)
        {
            if (BiggerOrEqual(res, value))
            {
                return iterator(res, this);
            }
            res = res->parent;
        }
        return end();
    }
};