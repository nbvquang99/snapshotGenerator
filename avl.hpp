#ifndef AVL
#define AVL
#include <string>
#include <string.h>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#define NUM_NODES 120000000
#define NUM_ROOTS 50000000

typedef std::pair<float, int> flin;

struct AVLNode {
    float key;
    int quantity;
    int height;
    int left;
    int right;
    int index;
};

struct Version {
    int index;
    long long epoch;
};

struct Book {
    int fdAVL;
    int nNode;
    int nRoot;
    int fdArr;
    AVLNode* mapped_region;
    Version* mapped_region_arr;
    Book() {}
    Book(int fdAVL, struct AVLNode* mapped_region) : fdAVL(fdAVL), mapped_region(mapped_region), nNode(0), nRoot(0), fdArr(0), mapped_region_arr(0) {}
};

Version* accessArr(Version* mapped_region, int index) {
    // std::cout << "try to access arr index: " << index << std::endl;
    return reinterpret_cast<Version*>(mapped_region + sizeof(Version) * index);
}

AVLNode* accessNode(AVLNode* mapped_region, int index) {
    if (index == -1) return nullptr;
    return reinterpret_cast<AVLNode*>(mapped_region + sizeof(AVLNode) * index);
}

AVLNode* createNode(float key, int quantity, int index, AVLNode* mapped_region) {
    AVLNode* newNode = accessNode(mapped_region, index);
    newNode->key = key;
    newNode->quantity = quantity;
    newNode->index = index;
    newNode->left = -1;
    newNode->right = -1;
    newNode->height = 1;
    // std::cout << "Created new node: " << newNode->index << " " << key << std::endl;
    return newNode;
}

AVLNode* copyNode(AVLNode* node, int index, AVLNode* mapped_region) {
    AVLNode* newNode = accessNode(mapped_region, index);
    newNode->key = node->key;
    newNode->quantity = node->quantity;
    newNode->index = index;
    newNode->left = node->left;
    newNode->right = node->right;
    newNode->height = node->height;
    return newNode;
}

// Helper function to calculate height of a node
int nodeHeight(AVLNode* node) {
    if (node == nullptr) return 0;
    return node->height;
}

// Helper function to calculate balance factor of a node
int balanceFactor(AVLNode* node, AVLNode* mapped_region) {
    if (node == nullptr) return 0;
    return nodeHeight(accessNode(mapped_region, node->left)) - nodeHeight(accessNode(mapped_region, node->right));
}

// Helper function to update the height of a node
void updateHeight(AVLNode* node, AVLNode* mapped_region) {
    if (node != nullptr)
        node->height = 1 + std::max(nodeHeight(accessNode(mapped_region, node->left)), nodeHeight(accessNode(mapped_region, node->right)));
}

// Helper function to perform a right rotation
AVLNode* rightRotate(AVLNode* y, AVLNode* mapped_region) {
    AVLNode* x = accessNode(mapped_region, y->left);
    AVLNode* T2 = accessNode(mapped_region, x->right);

    x->right = y->index;
    if (T2 == nullptr) y->left = -1;
    else y->left = T2->index;

    updateHeight(y, mapped_region);
    updateHeight(x, mapped_region);

    return x;
}

// Helper function to perform a left rotation
AVLNode* leftRotate(AVLNode* x, AVLNode* mapped_region) {
    AVLNode* y = accessNode(mapped_region, x->right);
    AVLNode* T2 = accessNode(mapped_region, y->left);

    y->left = x->index;
    if (T2 == nullptr) x->right = -1;
    else x->right = T2->index;

    updateHeight(x, mapped_region);
    updateHeight(y, mapped_region);
    // std::cout << "left rotate complete" << std::endl;
    return y;
}

// Recursive function to insert a key into the AVL tree
AVLNode* insert(AVLNode* node, float key, int quantity, int &nNode, AVLNode* mapped_region, bool isCopyPath) {
    if (nNode == 0 || node == nullptr) {
        // Create a new node
        // std::cout << "Create new key: " << key << std::endl;
        return createNode(key, quantity, ++nNode, mapped_region);
    }

    // Perform standard BST insertion
    AVLNode* newNode;
    if (isCopyPath) newNode = copyNode(node, ++nNode, mapped_region);
    else newNode = node;

    // std::cout << "Considering node: " << node->index << " " << node->key << " " << key << " nNode: " << nNode << std::endl;

    if (key < node->key) {
        newNode->left = insert(accessNode(mapped_region, node->left), key, quantity, nNode, mapped_region, isCopyPath)->index;
    }
    else if (key > node->key) {
        // std::cout << "insert right: " << node->right << " " << accessNode(mapped_region, node->right) << std::endl;
        newNode->right = insert(accessNode(mapped_region, node->right), key, quantity, nNode, mapped_region, isCopyPath)->index;
    }
    else {
        if (quantity < 0 && newNode->quantity < -quantity) return newNode;
        newNode->quantity += quantity;
        return newNode;
    }

    // Update height of the copy node
    updateHeight(newNode, mapped_region);

    // Perform balance adjustments
    int balance = balanceFactor(newNode, mapped_region);
    
    // std::cout << "started balancing: " << balance << std::endl;

    // Left Heavy
    if (balance > 1) {
        // std::cout << "left heavy: " << newNode->left << std::endl;
        if (key < accessNode(mapped_region, newNode->left)->key) {
            // Left-Left Case
            return rightRotate(newNode, mapped_region);
        } else {
            // Left-Right Case
            newNode->left = leftRotate(accessNode(mapped_region, newNode->left), mapped_region)->index;
            return rightRotate(newNode, mapped_region);
        }
    }
    // Right Heavy
    if (balance < -1) {
        // std::cout << "right heavy: " << newNode->right << std::endl;
        if (key > accessNode(mapped_region, newNode->right)->key) {
            // Right-Right Case
            return leftRotate(newNode, mapped_region);
        } else {
            // Right-Left Case
            newNode->right = rightRotate(accessNode(mapped_region, newNode->right), mapped_region)->index;
            return leftRotate(newNode, mapped_region);
        }
    }

    return newNode;
}

// Find 5 largest bids for a root of AVLtree
void findBid(AVLNode* node, AVLNode* mapped_region, std::vector<flin> &result) {
    if (node == nullptr || result.size()==5) return;
    if (result.size() < 5) findBid(accessNode(mapped_region, node->right), mapped_region, result);
    if (result.size() < 5) result.push_back(std::make_pair(node->key, node->quantity));
    if (result.size() < 5) findBid(accessNode(mapped_region, node->left), mapped_region, result);
}

// Find 5 smallest asks for a root of AVLtree
void findAsk(AVLNode* node, AVLNode* mapped_region, std::vector<flin> &result) {
    if (node == nullptr || result.size()==5) return;
    if (result.size() < 5) findAsk(accessNode(mapped_region, node->left), mapped_region, result);
    if (result.size() < 5) result.push_back(std::make_pair(node->key, node->quantity));
    if (result.size() < 5) findAsk(accessNode(mapped_region, node->right), mapped_region, result);
}

Book* initAVL(const std::string symbol, const std::string category) {
    std::string filename = symbol + "_" + category + "_BOOK.bin";
    int fdAVL = open(filename.c_str(), O_RDWR);
    if (fdAVL == -1) {
        fdAVL = open(filename.c_str(), O_CREAT | O_RDWR, 0666);
        if (fdAVL == -1) {
            std::cerr << "Error opening file" << std::endl;
            return nullptr;
        }
    }
    
    // Calculate file size based on node size and number of nodes
    size_t file_size = sizeof(AVLNode) * NUM_NODES;
    ftruncate(fdAVL, file_size);

    // Memory map the file
    AVLNode* mapped_region = static_cast<AVLNode*>(mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdAVL, 0));
    if (mapped_region == MAP_FAILED) {
        std::cerr << "Error mapping file to memory" << std::endl;
        return nullptr;
    }

    return new Book(fdAVL, mapped_region);
}

void initArr(Book* book, const std::string symbol, const std::string category) {
    std::string filename = symbol + "_" + category + "_ARR.bin";
    int fdArr = open(filename.c_str(), O_RDWR);
    bool isInit = false;
    if (fdArr == -1) {
        isInit = true;
        fdArr = open(filename.c_str(), O_CREAT | O_RDWR, 0666);
        if (fdArr == -1) {
            std::cerr << "Error opening file" << std::endl;
            return;
        }
    }
    
    size_t file_size = sizeof(Version) * NUM_ROOTS;
    ftruncate(fdArr, file_size);

    // Memory map the array
    Version* mapped_region = static_cast<Version*>(mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdArr, 0));
    if (mapped_region == MAP_FAILED) {
        std::cerr << "Error mapping file to memory" << std::endl;
        return;
    }
    if (isInit) {
        // Reserve first 2 elements to store nNode and nRoot
        mapped_region->epoch = 0; // nNode
        mapped_region->index = 0; // nRoot
        // std::cout << "Is Init? " << isInit << std::endl;
    }
    book->fdArr = fdArr;
    book->mapped_region_arr = mapped_region;
    book->nNode = mapped_region->epoch;
    book->nRoot = mapped_region->index;
}

void updateBook(Book *book, long long epoch, float price, int quantity, std::string category) {
    int versionIndex;
    bool isCopyPath;
    int tmpIndex;
    // Comparing the epoch
    if (book->nRoot == 0) {
        book->nRoot++;
        versionIndex = 0;
        isCopyPath = true;
    } else {
        Version* currentVer = accessArr(book->mapped_region_arr, book->nRoot);
        if (currentVer->epoch == epoch) {
            isCopyPath = false;
        } else {
            isCopyPath = true;
            book->nRoot++;
        }
        versionIndex = currentVer->index;
    }

    // std::cout << "Is Copy: " << isCopyPath << "Root Index: " << versionIndex << std::endl;

    if (category == "NEW") {
        tmpIndex = insert(accessNode(book->mapped_region, versionIndex), price, quantity, book->nNode, book->mapped_region, isCopyPath)->index;
    } else if (category == "TRADE") {
        tmpIndex = insert(accessNode(book->mapped_region, versionIndex), price, -quantity, book->nNode, book->mapped_region, isCopyPath)->index;
    } else if (category == "CANCEL") {
        tmpIndex = insert(accessNode(book->mapped_region, versionIndex), price, -quantity, book->nNode, book->mapped_region, isCopyPath)->index;
    }

    Version* updateVersion = accessArr(book->mapped_region_arr, book->nRoot);
    updateVersion->epoch = epoch;
    updateVersion->index = tmpIndex;
}

int findLowerRootByEpoch(Book* book, long long epoch) {
    int L = 1;
    int R = book->nRoot;
    // std::cout<< "L R lower: " << L << " " << R << std::endl;
    while (L < R) {
        int mid = L + (R-L)/2;
        if (accessArr(book->mapped_region_arr, mid)->epoch >= epoch) {
            R = mid;
        } else {
            L = mid+1;
        }
    }
    // std::cout<< "L: "<< accessArr(book->mapped_region_arr, L)->epoch << " " << epoch << std::endl;
    if (accessArr(book->mapped_region_arr, L)->epoch < epoch) return -1;
    return L;
}

int findUpperRootByEpoch(Book* book, long long epoch) {
    int L = 1;
    int R = book->nRoot;
    // std::cout<< "L R upper: " << L << " " << R << std::endl;
    while (L < R) {
        int mid = L + (R-L+1)/2;
        if (accessArr(book->mapped_region_arr, mid)->epoch > epoch) {
            R = mid-1;
        } else {
            L = mid;
        }
    }
    // std::cout<< "R: " << accessArr(book->mapped_region_arr, L)->epoch << " " << epoch << std::endl;
    if (accessArr(book->mapped_region_arr, L)->epoch > epoch) return -1;
    return L;
}

void closeBook(Book* book) {
    //update nRoot, nNode to file
    book->mapped_region_arr->epoch = book->nNode;
    book->mapped_region_arr->index = book->nRoot;
    munmap(book->mapped_region, sizeof(AVLNode) * NUM_NODES);
    munmap(book->mapped_region_arr, sizeof(Version) * NUM_ROOTS);
    close(book->fdAVL);
    close(book->fdArr);
}

#endif