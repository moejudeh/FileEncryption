// File Name : mymap.h
// Name : Moe Judeh
// netID: mjude4
// Course Info : CS 251 - Data Structures (34460) 
// Assignment Info : Project 5 - mymap
// Description : creates a map that is saved in a
//               self-balancing BST
// Date : 04/7/2022
#pragma once

#include <iostream>
#include <sstream>

using namespace std;

template<typename keyType, typename valueType>
class mymap {
 private:
    struct NODE {
        keyType key;  // used to build
        valueType value;  // stored data for the
        NODE* left;  // links to left
        NODE* right;  // links to right
        int nL;  // number of nodes in left 
        int nR;  // number of nodes in right
        bool isThreaded;
    };
    NODE* root;  // pointer to root node of the
    int size;  // # of key/value pairs in the mymap

    //
    // iterator:
    // This iterator is used so that mymap will work with a foreach loop.
    //
    struct iterator {
     private:
        NODE* curr;  // points to current in-order node for begin/end

     public:
        iterator(NODE* node) {
            curr = node;
        }

        keyType operator *() {
            return curr -> key;
        }

        bool operator ==(const iterator& rhs) {
            return curr == rhs.curr;
        }

        bool operator !=(const iterator& rhs) {
            return curr != rhs.curr;
        }

        bool isDefault() {
            return !curr;
        }

        //
        // operator++:
        //
        // This function should advance curr to the next in-order node.
        // O(logN)
        //
        iterator operator++() {
            if (curr->isThreaded) {
                curr = curr->right;
            } else {
                curr = curr->right;
                if (curr == nullptr) { return iterator(curr); }
                while (curr->left != nullptr) {
                    curr = curr->left;
                }
            }
            return iterator(curr);
        }
    };

    //
    // _toString
    //
    // cycles through a BST inorder outputing the key and related
    // value to output.
    //
    void _toString(NODE* head, ostream& output) {
        if (head->left != nullptr) {
            _toString(head->left, output);
        }
        output << "key: " << head->key << " value: " << head->value << endl;
        if (head->right != nullptr && head->isThreaded == false) {
            _toString(head->right, output);
        }
    }

    //
    // _clear
    //
    // checks if a node has a left of right pointer
    // if not it deletes the node if it does it calls
    // itself again but for the left or/and right pointer
    //
    void _clear(NODE* curr) {
        if (curr == nullptr) {
            return;
        }
        if (curr->left != nullptr) {
            _clear(curr->left);
        }
        if (curr->right != nullptr && curr->isThreaded == false) {
            _clear(curr->right);
        }
        delete curr;
    }

    //
    // _copy
    //
    // Copys a node of a different BST tree and the calls itself
    // to copy the left and right pointer if exists. It then returns the new
    // node created
    NODE* _copy(NODE* thisNode, NODE* otherNode, NODE* prevNode) {
        if (otherNode == nullptr) {
            return nullptr;
        }

        thisNode = new NODE();
        thisNode->key = otherNode->key;
        thisNode->value = otherNode->value;
        thisNode->left = nullptr;
        thisNode->right = nullptr;
        thisNode->nL = otherNode->nL;
        thisNode->nR = otherNode->nR;
        thisNode->isThreaded = otherNode->isThreaded;

        if (otherNode->left != nullptr) {
            thisNode->left = _copy(thisNode->left, otherNode->left, thisNode);
            thisNode->right = prevNode;
        }
        if(otherNode->right != nullptr) {
            if (otherNode->isThreaded && prevNode != nullptr) {
                thisNode->right = prevNode->right;
                prevNode->right = thisNode;
            } else {
                thisNode->right = _copy(thisNode->right, otherNode->right, thisNode);
            }
        }
        return thisNode;
    }


    //
    // _checkBalance
    //
    // cycles through a BST in pre-order outputing
    // the key and the nL and nR count
    void _checkBalance(NODE* n, ostream& output) {
        if (n == nullptr) {
            return;
        }
        output << "key: " << n->key << ", nL: " << n->nL << ", nR: " << n->nR << endl;
        if (n->left != nullptr) {
            _checkBalance(n->left, output);
        }
        if (n->right != nullptr && !n->isThreaded) {
            _checkBalance(n->right, output);
        }
    }

    //
    // _getVector
    //
    // cycles through a BST in in-order recursion and
    // uses push_back to store the nodes it crosses
    // into the parameter v
    void _getVector(NODE* &curr , vector<NODE*>& v) {
        if (curr->left != nullptr) {
            _getVector(curr->left, v);
        }
        v.push_back(curr);
        if (curr->right != nullptr && curr->isThreaded == false) {
            _getVector(curr->right, v);
        }      
    }

    //
    // _rebalance
    //
    // uses a vector of nodes to create a balanced BST tree and returns the
    // root of that tree
    NODE* _rebalance(vector<NODE*>& v, NODE* &parent, int start, int end) {
        if (start > end) {
            return nullptr;
        }
        int mid = (start + end) / 2;
        NODE* midNode = v[mid];
        midNode->nL = mid - start;
        midNode->nR = end - mid;

        midNode->left = _rebalance(v, midNode, start, mid - 1);
        if (end-mid == 0) {
            midNode->isThreaded = true;
            if (parent != nullptr && parent->key >= midNode->key) {
                midNode->right = parent;
            } else {
                midNode->right = nullptr;
            }
        } else {
            midNode->right = _rebalance(v, midNode, mid + 1, end);
            midNode->isThreaded = false;
        }
        return midNode;
    }

    //
    // _toVector
    //
    // cycles through a BST in-order and stores the key, value as
    // a pair in a vector, which is the parameter v.
    void _toVector(NODE* curr, vector<pair<keyType, valueType>>& v) {
        if(curr == nullptr) {
            return;
        }
        if (curr->left != nullptr) {
            _toVector(curr->left, v);
        }
        v.push_back(pair<keyType, valueType>(curr->key, curr->value));
        if (curr->right != nullptr && curr->isThreaded == false) {
            _toVector(curr->right, v);
        }
    }

    //
    // _find
    //
    // finds the next empty node to place a new Node of key into that slot
    void _find(NODE* &curr, NODE* &prev, keyType &key, NODE* &violator, NODE* &violatorParent, bool &adding){
        while (curr != nullptr && adding) {
            if (key < curr->key) {
                curr->nL++;
                if ((!(max(curr->nL, curr->nR) <= 2 * min(curr->nL, curr->nR) + 1)) && violator == nullptr) {
                    violator = curr;
                    violatorParent = prev;
                }
                prev = curr;
                curr = curr->left;
            } else {
                curr->nR++;
                if ((!(max(curr->nL, curr->nR) <= 2 * min(curr->nL, curr->nR) + 1)) && violator == nullptr) {
                    violator = curr;
                    violatorParent = prev;
                }
                prev = curr;
                curr = (curr->isThreaded) ? nullptr : curr->right;
            }
        }
    }

    //
    // _newNode
    //
    // creates a new Node with the key as the parameter
    // key and value as the parameter value. it sets 
    // everything else to defualt.
    NODE* _newNode(keyType &key, valueType &value) {
        NODE* n = new NODE();
        n->key = key;
        n->value = value;
        n->left = nullptr;
        n->right = nullptr;
        n->nL = 0;
        n->nR = 0;
        n->isThreaded = true;

        return n;
    }

    //
    // _addNode
    //
    // adds a node to a BST and increases the size of that BST
    void _addNode(keyType &key, valueType &value, NODE* &prev, NODE* &root, int &size) {
        NODE* n = _newNode(key, value);
        if (prev == nullptr) {
            root = n;
        } else if (key < prev->key) {
            prev->left = n;
            n->right = prev;
        } else {
            n->right = prev->right;
            prev->right = n;
            prev->isThreaded = false;
        }
        size++;
    }

 public:
    //
    // default constructor:
    //
    // Creates an empty mymap.
    // Time complexity: O(1)
    //
    mymap() {
        root = nullptr;
        size = 0;
    }

    //
    // copy constructor:
    //
    // Constructs a new mymap which is a copy of the "other" mymap.
    // Sets all member variables appropriately.
    // Time complexity: O(n), where n is total number of nodes in threaded,
    // self-balancing BST.
    //
    mymap(const mymap& other) {
        this->root = _copy(this->root, other.root, nullptr);
        this->size = other.size;
    }

    //
    // operator=:
    //
    // Clears "this" mymap and then makes a copy of the "other" mymap.
    // Sets all member variables appropriately.
    // Time complexity: O(n), where n is total number of nodes in threaded,
    // self-balancing BST.
    //
    mymap& operator=(const mymap& other) {
        this->clear();
        this->root = _copy(this->root, other.root, nullptr);
        this->size = other.size;
        return *this;
    }

    // clear:
    //
    // Frees the memory associated with the mymap; can be used for testing.
    // Time complexity: O(n), where n is total number of nodes in threaded,
    // self-balancing BST.
    //
    void clear() {
        _clear(root);
        root = nullptr;
        size = 0;
    }

    //
    // destructor:
    //
    // Frees the memory associated with the mymap.
    // Time complexity: O(n), where n is total number of nodes in threaded,
    // self-balancing BST.
    //
    ~mymap() {
        _clear(root);
    }

    //
    // put:
    //
    // Inserts the key/value into the threaded, self-balancing BST based on
    // the key.
    // Time complexity: O(logn + mlogm), where n is total number of nodes in the
    // threaded, self-balancing BST and m is the number of nodes in the
    // sub-tree that needs to be re-balanced.
    // Space complexity: O(1)
    //
    void put(keyType key, valueType value) {
        NODE* curr = root;
        NODE* prev = nullptr;
        NODE* violator = nullptr; NODE* violatorParent = nullptr;
        bool adding = !this->contains(key);
        while (curr != nullptr && !adding) {
            if (key == curr->key) {
                curr->value = value;
                return;
            }
            if(key < curr->key) {
                curr = curr->left;
            } else {
                 curr = (curr->isThreaded) ? nullptr : curr->right;
            }
        }
        _find(curr,prev,key,violator, violatorParent, adding);
        _addNode(key, value, prev, root, size);
        if (violator != nullptr) {
            vector<NODE*> errors;
            _getVector(violator, errors);
            NODE* newRoot = _rebalance(errors, violatorParent, 0, errors.size()-1);
            if (violatorParent == nullptr) {
                root = newRoot;
            } else if (violatorParent->key < newRoot->key) {
                violatorParent->right = newRoot;
            } else {
                violatorParent->left = newRoot;
            }
        }
    }

    //
    // contains:
    // Returns true if the key is in mymap, return false if not.
    // Time complexity: O(logn), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    bool contains(keyType key) {
        NODE* curr = root;
        while (curr != nullptr) {
            if(key == curr->key) {
                return true;
            }
            if (key < curr->key) {
                curr = curr->left;
            } else {
                curr = (curr->isThreaded) ? nullptr : curr->right;
            }
        }
        return false;
    }

    //
    // get:
    //
    // Returns the value for the given key; if the key is not found, the
    // default value, valueType(), is returned (but not added to mymap).
    // Time complexity: O(logn), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    valueType get(keyType key) {
        NODE* curr = root;

        while (curr != nullptr) {
            if (key == curr->key) {
                return curr->value;
            }
            if (key < curr->key) {
                curr = curr->left;
            } else {
                curr = (curr->isThreaded) ? nullptr : curr->right;
            }
        }
        return valueType();
    }

    //
    // operator[]:
    //
    // Returns the value for the given key; if the key is not found,
    // the default value, valueType(), is returned (and the resulting new
    // key/value pair is inserted into the map).
    // Time complexity: O(logn + mlogm), where n is total number of nodes in the
    // threaded, self-balancing BST and m is the number of nodes in the
    // sub-trees that need to be re-balanced.
    // Space complexity: O(1)
    //
    valueType operator[](keyType key) {
        if (this->contains(key)) {
            return this->get(key);
        } else {
            this->put(key, valueType());
            return valueType();
        }
    }

    //
    // Size:
    //
    // Returns the # of key/value pairs in the mymap, 0 if empty.
    // O(1)
    //
    int Size() {
        return size;
    }

    //
    // begin:
    //
    // returns an iterator to the first in order NODE.
    // Time complexity: O(logn), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    iterator begin() {
        NODE* curr = root;
        while (curr->left != nullptr) {
            curr = curr->left;
        }
        return iterator(curr);
    }

    //
    // end:
    //
    // returns an iterator to the last in order NODE.
    // this function is given to you.
    //
    // Time Complexity: O(1)
    //
    iterator end() {
        return iterator(nullptr);
    }

    //
    // toString:
    //
    // Returns a string of the entire mymap, in order.
    // Format for 8/80, 15/150, 20/200:
    // "key: 8 value: 80\nkey: 15 value: 150\nkey: 20 value: 200\n
    // Time complexity: O(n), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    string toString() {
        if (root == nullptr) {
            return "";
        }
        stringstream ss;
        _toString(root, ss);
        return ss.str();
    }

    //
    // toVector:
    //
    // Returns a vector of the entire map, in order.  For 8/80, 15/150, 20/200:
    // {{8, 80}, {15, 150}, {20, 200}}
    // Time complexity: O(n), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    vector<pair<keyType, valueType> > toVector() {
        vector<pair<keyType, valueType>> v;
        _toVector(root, v);
        return v;
    }

    //
    // checkBalance:
    //
    // Returns a string of mymap that verifies that the tree is properly
    // balanced.  For example, if keys: 1, 2, 3 are inserted in that order,
    // function should return a string in this format (in pre-order):
    // "key: 2, nL: 1, nR: 1\nkey: 1, nL: 0, nR: 0\nkey: 3, nL: 0, nR: 0\n";
    // Time complexity: O(n), where n is total number of nodes in the
    // threaded, self-balancing BST
    //
    string checkBalance() {
        stringstream ss;
        _checkBalance(root, ss);
        return ss.str();
    }
};
