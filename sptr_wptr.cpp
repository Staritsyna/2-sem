
#include <iostream>

using i32 = int32_t;
using u32 = uint32_t;

//using T = i32;

struct T {
  i32 a, b;
};

class UniquePtr {
private:
  T* ptr;


  //void f() = _____________;  // (delete | default)  / 0 virtual

  UniquePtr(const UniquePtr&) = delete;
  UniquePtr& operator=(const UniquePtr&) = delete;

  UniquePtr(T* raw) : ptr(raw) {}     // T*** p   , *p (T**)&

  friend UniquePtr MakeUnique(T value);

public:
  ~UniquePtr() { delete ptr; }
  UniquePtr(UniquePtr&& rhs) {
    ptr = rhs.ptr;
    rhs.ptr = nullptr;
  }
  /*
    UPtr lhs , rhs;
    lhs = std::move(rhs);

    // lhs.ptr = 1, rhs.ptr = 2;
    // t - new
    // lhs.ptr = 1, t.ptr=2, rhs.ptr=nullptr
    // swap
    // lhs.ptr = 2, t.ptr = 1, rhs.ptr = nullptr
    // return
    // lhs.ptr = 2, 1 deleted by t dtor, rhs.ptr = nullptr
*/
  // Move-and-Swap
  UniquePtr& operator=(UniquePtr&& rhs) {
    if (this == &rhs) { return *this; }
    UniquePtr t(std::move(rhs));
    std::swap(t.ptr, ptr);
    return *this;
  }

  UniquePtr() : ptr(nullptr) {}

  T* operator->() { return ptr; }
  T& operator*() { return *ptr; }

};

UniquePtr /*T**/ MakeUnique(T value) {
  T* p = new T(value);
  //  UniquePtr u(p);
  return p;
}



struct RefCountBlock{
  size_t strong, week;

  RefCountBlock() : strong(0), week(0) {}
};

class WeakPtr;

class SharedPtr{
private:

RefCountBlock* cnt;
T* raw;

  friend SharedPtr MakeOne(T value);


  SharedPtr(T* rhs)
  {
    raw = rhs;
    cnt = new RefCountBlock();
    cnt->week = 0;
    cnt->strong =1;

  }
public:
    size_t get_cntS()
    {
        return cnt->strong;
    }

    friend WeakPtr;
  SharedPtr()
  {
    raw = nullptr;
    cnt = nullptr;
  }

  ~SharedPtr()
  {
    if (cnt == nullptr) {return;}
    --(cnt->strong);
    if (cnt->strong != 0) {return;}
    delete raw;
    if(cnt->week == 0)
      delete cnt;
  }

  SharedPtr(SharedPtr&& rhs)
  {
    raw = rhs.raw;
    cnt = rhs.cnt;
    rhs.raw = nullptr;
    rhs.cnt = nullptr;
  }

  SharedPtr&  operator=(SharedPtr&& rhs)
  {
    if (&rhs == this) {return *this;}
    SharedPtr t(std::move(rhs));
    std::swap(raw, t.raw);
    std::swap(cnt,t.cnt);
    return *this;
  }

  SharedPtr(const SharedPtr& rhs)
  {
    raw = rhs.raw;
    cnt = rhs.cnt;
    ++(cnt->strong);
  }

  SharedPtr& operator=(const SharedPtr& rhs)
  {
    if(&rhs != this)
      {
        SharedPtr t(rhs);
        std::swap(cnt,t.cnt);
        std::swap(raw,t.raw);
      }
    return *this;
  }

    T& operator*(){return *raw;}
    T* operator->(){return raw;}

    SharedPtr(WeakPtr& o);


};

SharedPtr MakeOne (T value)
{
  T* p = new T(value);
  return p;
}

class WeakPtr
{
private:
    RefCountBlock* cnt;


    friend WeakPtr MakeOneW(T value);

public:

    T* raw;

    RefCountBlock* get_cnt()
    {
        return cnt;
    }
    size_t get_cntW()
    {
        return cnt->week;
    }



    WeakPtr(T* rhs)
    {
        raw = rhs;
        cnt = new RefCountBlock();
        cnt->week = 1;
        cnt->strong =0;
    }
    WeakPtr()
    {
    raw = nullptr;
    cnt = nullptr;
    }

    ~WeakPtr()
    {
    if (cnt == nullptr) {return;}
    --(cnt->week);
    if (cnt->week != 0) {return;}
    if(cnt->week == 0)
      delete cnt;
    }

    WeakPtr(WeakPtr&& rhs)
    {
    raw = rhs.raw;
    cnt = rhs.cnt;
    rhs.raw = nullptr;
    rhs.cnt = nullptr;
    }

    WeakPtr&  operator=(WeakPtr&& rhs)
    {
        if (&rhs == this) {return *this;}
        WeakPtr t(std::move(rhs));
        std::swap(raw, t.raw);
        std::swap(cnt,t.cnt);
        return *this;
    }

    WeakPtr(const WeakPtr& rhs)
    {
        raw = rhs.raw;
        cnt = rhs.cnt;
        ++(cnt->week);
    }

    WeakPtr& operator=(const WeakPtr& rhs)
    {
    if(&rhs != this)
      {
        WeakPtr t(rhs);
        std::swap(cnt,t.cnt);
        std::swap(raw,t.raw);
      }
    return *this;
    }

    T& operator*(){return *raw;}
    T* operator->(){return raw;}

    bool Expired()
    {
        return (cnt->strong != 0);
    }

    SharedPtr Lock()
    {
        return SharedPtr(*this);
    }
       void  strong(SharedPtr& o)
    {
        cnt->strong = o.get_cntS();

        //ptr = o.ptr;
        //if(cnt->strong){
            //throw std::runtime_error("Expired");
        //}
    }

};

    SharedPtr::SharedPtr(WeakPtr& o)
    {
        raw = o.raw;

        cnt->week= o.get_cntW();
        if(cnt->strong){
            throw std::runtime_error("Expired");
        }
        cnt->strong++;
    }


/*SharedPtr::SharedPtr(WeakPtr& o): raw(o.raw)
{
    cnt->strong++;
    if ((cnt) || (cnt->strong == 0)){
        throw std::runtime_error("Expired");
    }
}*/

WeakPtr MakeOneW (T value)
{
  T* p = new T(value);
  return p;
}

int main() {
    SharedPtr p = MakeOne({1, 3});
    p->a = 17;
    std::cout << (*p).a << "\n";
    (*p).b = 24;
    std::cout << p->b << "\n";
    WeakPtr r = MakeOneW({1, 3});


    std::cout << std::boolalpha << r.Expired() << "\n";
    r.strong(p);
    r.Lock();
    std::cout << std::boolalpha << r.Expired() << "\n";
    return 0;
}
