#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template <typename T>
class SingleTon {
protected:
    SingleTon() = default;
    SingleTon(const SingleTon<T>&) = delete;
    SingleTon& operator = (const SingleTon<T>& st) = delete;
    static std::shared_ptr<T> _instance;
public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){
            /* 不用make_shared原因：
                new T：在类的成员函数里执行，有权访问 protected/private 构造函数
                make_shared<T>()：是外部标准库函数，无权访问非公开构造函数，直接用 make_shared 会报编译错误 */
            _instance = std::shared_ptr<T>(new T);
        });

        return _instance;
    }

    void printAddress() {
        std::cout << _instance.get() << std::endl;
    }

    ~SingleTon() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template<typename T>
std::shared_ptr<T> SingleTon<T>::_instance = nullptr;

#endif // SINGLETON_H
