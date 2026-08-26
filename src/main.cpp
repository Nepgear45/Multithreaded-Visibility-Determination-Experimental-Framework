#include "Application/Application.h"

#include <iostream>

int main()
{
    Application application;

    if (!application.Initialise())
    {
        std::cerr << "Application failed to initialise.\n";
        return 1;
    }

    application.Run();

    return 0;
}