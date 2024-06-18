#ifndef _SYLAR_SINGLETON_H_
#define _SYLAR_SINGLETON_H_


namespace sylar {

template<class T, class X=void, int N=0>
class Singleton {
public:
    static T* getInstance() {
        static T v;
        return &v;
    }
};

template<class T, class X=void, int N=0>
class SingletonPtr {
public:
    static std::shared_ptr<T> getInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}

#endif
