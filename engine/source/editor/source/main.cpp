#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "runtime/engine.h"

#include "editor/include/editor.h"

#define MOMOENGINE_XSTR(s) MOMOENGINE_STR(s)
#define MOMOENGINE_STR(s) #s

int main(int argc, char** argv) {
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "MomoEditor.ini";

    Momo::MomoEngine* engine = new Momo::MomoEngine();

    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    Momo::MomoEditor* editor = new Momo::MomoEditor();
    editor->initialize(engine);

    editor->run();

    editor->clear();

    engine->clear();
    engine->shutdownEngine();

    return 0;
}