#ifndef SINGLETON_HPP_INCLUDE
#define SINGLETON_HPP_INCLUDE

template <class T>
class Singleton
{
public:
    static T& Instance () {
        static T obj;

        return obj;
    }

protected:
    Singleton () = default;
    Singleton (const Singleton&);
    Singleton& operator=(const Singleton&);
    ~Singleton () = default;
};

#endif
