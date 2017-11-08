//
//  main.cpp
//  Nutshell.Library
//
//  Created by Kirill Korolev on 06/11/2017.
//  Copyright © 2017 Kirill Korolev. All rights reserved.
//

#include <iostream>
#include "nutshell/camera/camera.hpp"

using namespace nutshell::camera;

int main(int argc, const char * argv[]) {
    Camera camera("send");
    camera.capture();
    return 0;
}
