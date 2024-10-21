// #include <iostream>
// #include <vector>
// #include <string>
// using namespace std;



// int main()
// {
//     vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

//     for (const string& word : msg)
//     {
//         cout << word << " ";
//     }
//     cout << endl;
// }

// // ---------------------------------------------------------------
// #include "speak.h"

// int main(int argc,char *argv[])
// {
//     Speak speak;
//     speak.sayHello("world");
//     return(0);
// }


#include <iostream>
#include <vector>

using namespace std;

template <typename T>
class DemoVector : public vector<T>
{
private:
    class CommaInputWapper
    {
    public:
        CommaInputWapper() = delete;
        explicit CommaInputWapper(DemoVector<T>* const sp) : _sp(sp){}
        CommaInputWapper& operator, (const T& val)
        {
            _sp->push_back(val);
            return *this;
        }
    private:
        DemoVector<T>* const _sp;
    };
public:
    DemoVector() = default;
    CommaInputWapper operator<<(const T& val)
    {
        vector<T>::clear();
        vector<T>::push_back(val);
        return CommaInputWapper(this);
    }
};

template <typename T>
ostream& operator << (ostream& out, const vector<T>& v)
{
    if (!v.empty())
        out << v[0];
    for (typename vector<T>::size_type i = 1; i < v.size(); ++i)
    {
        out << " " << v[i];
    }
    return out;
}

int main()
{
    DemoVector<int> dv1;
    dv1 << 1, 2, 3, 4, 5, 6;
    cout << dv1 << endl;
    return 0;
}