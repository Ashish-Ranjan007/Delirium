#include <iomanip> // For std::fixed and std::setprecision
#include <iostream>

#include "type.hpp"

void Delirium::printValue(Value value)
{
    std::cout << std::fixed << std::setprecision(6) << value;
}
