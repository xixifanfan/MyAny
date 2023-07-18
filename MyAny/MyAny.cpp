/**
*  项目名称：简易的C++17 any实现
*  version:1.0
*/
#include <iostream>
#include <memory>
#include <typeindex>
using namespace std;
class myany
{
private:
    class ValueHandle;
    using ValHandle_ptr = unique_ptr<ValueHandle>;
    //模板擦除
    class ValueHandle
    {
    public:
        virtual ~ValueHandle() = default;
        virtual ValHandle_ptr clone()const = 0;
    };
    template<typename T>
    class Value:public ValueHandle
    {
    public:
        T value;
        template<typename U>
        Value(U&& value) :value(forward<U>(value)) {}  //Value(T value):value(value){}
        ValHandle_ptr clone()const override
        {
            return ValHandle_ptr(new Value<T>(value));
        }
    };
    ValHandle_ptr clone()const
    {
        if (val)return val->clone();
        return nullptr;
    }
    //基类指针用于强转成子类   dynamic_cast<>()
    ValHandle_ptr val;
    //保存现在值类型
    type_index valType;
public:
    myany():val(nullptr),valType(type_index(typeid(nullptr_t))){}
    //拷贝构造和拷贝赋值函数
    myany(const myany& any):val(any.clone()),valType(any.valType){}
    myany& operator=(const myany& any)
    {
        if (this->val == any.val)return *this;
        val = any.clone();
        valType = any.valType;
        return *this;
    }
    //移动构造和移动赋值函数
    myany(myany&& any):val(exchange(any.val,nullptr)),valType(any.valType){}
    myany& operator=(myany&& any)
    {
        val = move(any.val);
        valType = any.valType;
        return *this;
    }
    //enable_if来防止递归调用拷贝构造函数，进入死循环
    template<typename T,class = typename enable_if_t<!is_same<typename decay_t<T>, myany>::value, T>>//decay去掉cv修饰符
    myany(T&& value)
        :val(new Value<typename decay<T>::type>(forward<T>(value))), 
        valType(type_index(typeid(typename decay<T>::type))) {}
    //判断是否有值
    bool hasVal()const
    {
        return val != nullptr;
    }
    //判断是否为T类型
    template<typename T>
    bool is()const
    {
        return valType == type_index(typeid(T));
    }
    //类型转换
    template<typename T>
    T& anyCast()
    {
        if (!is<T>())
        {
            cout << "cast fail!\n" << "you cant cast" << valType.name() << " to " << typeid(T).name() << endl;
            throw bad_cast();
        }
        Value<T>* t = dynamic_cast<Value<T>*>(val.get());
        return t->value;
    }
    ~myany() = default;
};
int main()
{
    using Any = myany;
    Any a(10);
    //a = 10;
    a = 20;
    cout << a.anyCast<int>() << endl;
    a = string("hello,world");
    cout << a.anyCast<string>() << endl;
}

