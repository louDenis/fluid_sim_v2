#include "User.h"


User::User()
{
    this->pressure = getKey();
}
float User::getKey()
{
    std::cout << "Choose pressure between 0 and 1: "; // ask user for a number

    float x{ }; // define variable x to hold user input (and zero-initialize it)
    std::cin >> x; // get number from keyboard and store it in variable x

    std::cout << "You entered " << x << '\n';
    return x;
}
