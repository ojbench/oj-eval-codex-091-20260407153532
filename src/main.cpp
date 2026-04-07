#include <bits/stdc++.h>
using namespace std;

// A fast ordered set using std::set plus tree order statistics emulation
// We'll implement an ESet-like interface sufficient for the speedtest driver.

template <class Key, class Compare = std::less<Key>>
class ESet {
    struct Node {
        Key key;
        Node* left{nullptr};
        Node* right{nullptr};
        Node* parent{nullptr};
        bool red{true};
        int sz{1};
        Node(const Key& k): key(k) {}
    };

    Node* root{nullptr};
    Compare comp;
    size_t n{0};

    // Update subtree size
    static int sizeOf(Node* x){ return x? x->sz: 0; }
    static void pull(Node* x){ if(x) x->sz = 1 + sizeOf(x->left) + sizeOf(x->right); }

    // Rotate helpers
    void rotateLeft(Node* x){
        Node* y = x->right;
        x->right = y->left; if(y->left) y->left->parent = x;
        y->parent = x->parent;
        if(!x->parent) root = y;
        else if(x == x->parent->left) x->parent->left = y; else x->parent->right = y;
        y->left = x; x->parent = y;
        pull(x); pull(y);
    }
    void rotateRight(Node* x){
        Node* y = x->left;
        x->left = y->right; if(y->right) y->right->parent = x;
        y->parent = x->parent;
        if(!x->parent) root = y;
        else if(x == x->parent->right) x->parent->right = y; else x->parent->left = y;
        y->right = x; x->parent = y;
        pull(x); pull(y);
    }

    // Utility comparisons
    bool lessKey(const Key& a, const Key& b) const { return comp(a,b); }
    bool eqKey(const Key& a, const Key& b) const { return !comp(a,b) && !comp(b,a); }

public:
    struct iterator {
        Node* p{nullptr};
        const ESet* owner{nullptr};
        iterator() = default;
        iterator(Node* x, const ESet* o): p(x), owner(o) {}
        const Key& operator*() const {
            if(p==nullptr) throw std::out_of_range("dereference end iterator");
            return p->key;
        }
        bool operator==(const iterator& o) const { return p==o.p; }
        bool operator!=(const iterator& o) const { return p!=o.p; }
        iterator& operator++(){
            if(p==nullptr) return *this; // end: do nothing
            // successor
            if(p->right){
                p = p->right; while(p->left) p=p->left;
            }else{
                Node* y = p->parent;
                while(y && p==y->right){ p=y; y=y->parent; }
                p = y; // could be nullptr = end
            }
            return *this;
        }
        iterator operator++(int){ auto t=*this; ++*this; return t; }
        iterator& operator--(){
            // if end(), move to max
            if(p==nullptr){
                Node* x = owner->root; if(!x) return *this; while(x->right) x=x->right; p=x; return *this;
            }
            // detect begin(): smallest node
            {
                Node* x = owner->root; if(!x) return *this; while(x->left) x=x->left; if(p==x) return *this;
            }
            if(p->left){ p = p->left; while(p->right) p=p->right; }
            else{ Node* y=p->parent; while(y && p==y->left){ p=y; y=y->parent; } p=y; }
            return *this;
        }
        iterator operator--(int){ auto t=*this; --*this; return t; }
    };

private:
    static Node* minNode(Node* x){ if(!x) return x; while(x->left) x=x->left; return x; }
    static Node* maxNode(Node* x){ if(!x) return x; while(x->right) x=x->right; return x; }

    Node* findNode(const Key& key) const{
        Node* x = root;
        while(x){
            if(lessKey(key,x->key)) x = x->left;
            else if(lessKey(x->key,key)) x = x->right;
            else return x;
        }
        return nullptr;
    }

    // RB-Tree insertion with size maintenance
    pair<Node*, bool> insertUnique(const Key& key){
        Node* y=nullptr; Node* x=root;
        while(x){
            y=x;
            if(lessKey(key,x->key)) x=x->left;
            else if(lessKey(x->key,key)) x=x->right;
            else return {x,false};
        }
        Node* z = new Node(key);
        z->parent = y; z->red = true; // new red
        if(!y) root = z;
        else if(lessKey(key,y->key)) y->left = z; else y->right = z;
        // fix sizes up the path
        for(Node* t=z; t; t=t->parent) pull(t);
        // fixup red-black
        insertFix(z);
        ++n;
        return {z,true};
    }

    void insertFix(Node* z){
        while(z!=root && z->parent->red){
            Node* p = z->parent;
            Node* g = p->parent;
            if(p == g->left){
                Node* u = g->right;
                if(u && u->red){
                    p->red = false; u->red = false; g->red = true; z = g;
                }else{
                    if(z == p->right){ z=p; rotateLeft(z); }
                    p = z->parent; g = p->parent;
                    p->red=false; g->red=true; rotateRight(g);
                }
            }else{
                Node* u = g->left;
                if(u && u->red){
                    p->red=false; u->red=false; g->red=true; z=g;
                }else{
                    if(z==p->left){ z=p; rotateRight(z); }
                    p = z->parent; g = p->parent;
                    p->red=false; g->red=true; rotateLeft(g);
                }
            }
        }
        root->red = false;
    }

    void transplant(Node* u, Node* v){
        if(!u->parent) root = v;
        else if(u == u->parent->left) u->parent->left = v; else u->parent->right = v;
        if(v) v->parent = u->parent;
    }

    void eraseNode(Node* z){
        Node* y = z; bool yRed = y->red; Node* x=nullptr; Node* xParent=nullptr;
        if(!z->left){
            x = z->right; xParent = z->parent; transplant(z,z->right);
            for(Node* t=xParent; t; t=t->parent) pull(t);
        }else if(!z->right){
            x = z->left; xParent = z->parent; transplant(z,z->left);
            for(Node* t=xParent; t; t=t->parent) pull(t);
        }else{
            y = minNode(z->right);
            yRed = y->red;
            x = y->right;
            if(y->parent == z){ if(x) x->parent = y; xParent = y; }
            else{
                xParent = y->parent;
                transplant(y,y->right);
                y->right = z->right; y->right->parent = y; for(Node* t=y; t; t=t->parent) pull(t);
            }
            transplant(z,y);
            y->left = z->left; y->left->parent = y; y->red = z->red;
            for(Node* t=y; t; t=t->parent) pull(t);
        }
        if(!yRed) eraseFix(x,xParent);
        delete z;
        --n;
    }

    void eraseFix(Node* x, Node* parent){
        auto isRed = [](Node* t){ return t && t->red; };
        while((x!=root) && (!x || !x->red)){
            if(x == (parent? parent->left: nullptr)){
                Node* w = parent? parent->right: nullptr;
                if(isRed(w)){
                    w->red=false; parent->red=true; rotateLeft(parent); w = parent? parent->right: nullptr;
                }
                if(!isRed(w? w->left: nullptr) && !isRed(w? w->right: nullptr)){
                    if(w) w->red = true; x = parent; parent = x? x->parent: nullptr;
                }else{
                    if(!isRed(w? w->right: nullptr)){
                        if(w && w->left) w->left->red=false; if(w){ w->red=true; rotateRight(w);} w = parent? parent->right: nullptr;
                    }
                    if(w) w->red = parent? parent->red: false; if(parent) parent->red=false; if(w && w->right) w->right->red=false; rotateLeft(parent); x = root; parent=nullptr;
                }
            }else{
                Node* w = parent? parent->left: nullptr;
                if(isRed(w)){
                    w->red=false; parent->red=true; rotateRight(parent); w = parent? parent->left: nullptr;
                }
                if(!isRed(w? w->right: nullptr) && !isRed(w? w->left: nullptr)){
                    if(w) w->red = true; x = parent; parent = x? x->parent: nullptr;
                }else{
                    if(!isRed(w? w->left: nullptr)){
                        if(w && w->right) w->right->red=false; if(w){ w->red=true; rotateLeft(w);} w = parent? parent->left: nullptr;
                    }
                    if(w) w->red = parent? parent->red: false; if(parent) parent->red=false; if(w && w->left) w->left->red=false; rotateRight(parent); x = root; parent=nullptr;
                }
            }
        }
        if(x) x->red = false;
    }

public:
    ESet() = default;
    ~ESet(){ clear(root); }
    ESet(const ESet& other){ for(auto it=other.begin(); it!=other.end(); ++it) emplace(*it); }
    ESet& operator=(const ESet& other){ if(this==&other) return *this; this->~ESet(); new(this) ESet(); for(auto it=other.begin(); it!=other.end(); ++it) emplace(*it); return *this; }
    ESet(ESet&& other) noexcept { root=other.root; n=other.n; other.root=nullptr; other.n=0; }
    ESet& operator=(ESet&& other) noexcept { if(this==&other) return *this; this->~ESet(); new(this) ESet(); root=other.root; n=other.n; other.root=nullptr; other.n=0; return *this; }

    void clear(Node* x){ if(!x) return; clear(x->left); clear(x->right); delete x; }

    pair<iterator,bool> emplace(const Key& key){ auto pr = insertUnique(key); return { iterator(pr.first,this), pr.second}; }
    size_t erase(const Key& key){ Node* z = findNode(key); if(!z) return 0; eraseNode(z); return 1; }
    iterator find(const Key& key) const { return iterator(findNode(key), this); }
    size_t size() const noexcept { return n; }

    iterator lower_bound(const Key& key) const {
        Node* x=root; Node* ans=nullptr; while(x){ if(!lessKey(x->key,key)){ ans=x; x=x->left; } else x=x->right; } return iterator(ans,this);
    }
    iterator upper_bound(const Key& key) const {
        Node* x=root; Node* ans=nullptr; while(x){ if(lessKey(key,x->key)){ ans=x; x=x->left; } else x=x->right; } return iterator(ans,this);
    }

    size_t range(const Key& l, const Key& r) const {
        if(comp(r,l)) return 0; // r<l
        // count of <= r minus < l
        return countLE(r) - countL(l);
    }

    iterator begin() const noexcept { return iterator(minNode(root), this); }
    iterator end() const noexcept { return iterator(nullptr, this); }

private:
    size_t countLE(const Key& key) const { // <= key
        size_t cnt=0; Node* x=root;
        while(x){
            if(!lessKey(x->key,key)){ // x->key >= key
                x = x->left;
            }else{ // x->key < key
                cnt += 1 + sizeOf(x->left);
                x = x->right;
            }
        }
        // After traversal, need to add nodes equal to key: found by lower_bound to first > key then subtract
        // Simpler: count < key, then add 1 if key exists
        // But above logic only counted strictly < key. So adjust here:
        if(findNode(key)) cnt += 1;
        return cnt;
    }
    size_t countL(const Key& key) const { // < key
        size_t cnt=0; Node* x=root;
        while(x){
            if(lessKey(x->key,key)){
                cnt += 1 + sizeOf(x->left); x = x->right;
            }else x = x->left;
        }
        return cnt;
    }
};

// helper to check if iterator equals a key value
static inline bool prVal(const ESet<long long>::iterator& it, long long b){
    try{ return *it==b; }catch(...){ return false; }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Implements the speedtest IO protocol indicated by ESet/test/1.cpp but with stdin/stdout.
    // Ops:
    // 0 a b : insert b into set a ; if inserted, update iterator state
    // 1 a b : erase b from set a ; invalidate iterator if it pointed to erased element
    // 2 a   : s[++lst] = s[a]
    // 3 a b : find b in s[a] -> print true/false ; if found, set iterator to that element
    // 4 a b c : print s[a].range(b,c)
    // 5 : print previous of iterator if valid else -1
    // 6 : print next of iterator if valid else -1

    const int MAXS = 200000; // dynamic, but we only grow by op 2; using vector to store
    vector< ESet<long long> > s; s.reserve(100000); s.emplace_back(); // 0 unused
    s.resize(25); // initial as in reference uses s[25]

    using It = typename ESet<long long>::iterator;
    bool valid=false; int it_a=-1; It it; int lst=0; int op; long long a,b,c;
    while ( (cin>>op) ){
        switch(op){
            case 0: {
                cin>>a>>b; auto pr = s[a].emplace(b); if(pr.second){ it_a=a; it=pr.first; valid=true; } break;
            }
            case 1: {
                cin>>a>>b; if(valid && it_a==a){ if(prVal(it, b)) valid=false; } s[a].erase(b); break;
            }
            case 2: {
                cin>>a; ++lst; if((int)s.size() <= lst) s.resize(lst+1); s[lst] = s[a]; break;
            }
            case 3: {
                cin>>a>>b; auto it2 = s[a].find(b); if(it2!=s[a].end()){ cout<<"true\n"; it=it2; it_a=a; valid=true; } else { cout<<"false\n"; } break;
            }
            case 4: {
                cin>>a>>b>>c; cout<< s[a].range(b,c) <<"\n"; break;
            }
            case 5: {
                if(valid){ auto it2=it; auto it3=it2; --it2; // check if it at begin
                    if(it==it2) valid=false; }
                if(valid){ --it; cout<< *it <<"\n"; }
                else cout<<"-1\n"; break;
            }
            case 6: {
                if(valid){
                    ++it; // advance first
                    // if now at end, invalidate
                    if(it == s[it_a].end()) valid = false;
                    else cout << *it << "\n";
                }
                if(!valid) cout<<"-1\n"; break;
            }
        }
    }
    return 0;
}
