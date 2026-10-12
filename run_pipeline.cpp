#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static std::string quote_argument(const fs::path& path)
{
    std::string value = path.string();
    std::string quoted = "\"";

    for (char character : value) {
        if (character == '"')
            quoted += '\\';
        quoted += character;
    }

    quoted += '"';
    return quoted;
}

static int run_command(const std::string& command)
{
    std::cout << "> " << command << '\n';
    const int status = std::system(command.c_str());
    return status == -1 ? EXIT_FAILURE : status;
}

int main(int argc, char* argv[])
{
    if (argc < 1 || argc > 3) {
        std::cerr << "Utilisation: " << argv[0]
                  << " [fichier-source] [fichier-tokens]\n";
        return EXIT_FAILURE;
    }

    const fs::path tp1_source = "TP1/tp1.c";
    const fs::path tp2_source = "TP2/tp2.c";
    const fs::path input = argc >= 2 ? fs::path(argv[1]) : fs::path("TP1/program.txt");
    const fs::path tokens = argc == 3
        ? fs::path(argv[2])
        : fs::path("TP1/unites_lexicales.txt");
    const fs::path temporary_directory = fs::temp_directory_path();
#ifdef _WIN32
    const fs::path tp1_executable = temporary_directory / "tp_pipeline_tp1.exe";
    const fs::path tp2_executable = temporary_directory / "tp_pipeline_tp2.exe";
#else
    const fs::path tp1_executable = temporary_directory / "tp_pipeline_tp1";
    const fs::path tp2_executable = temporary_directory / "tp_pipeline_tp2";
#endif

    if (!fs::exists(tp1_source) || !fs::exists(tp2_source)) {
        std::cerr << "Erreur : lancer ce programme depuis la racine du projet.\n";
        return EXIT_FAILURE;
    }

    std::cout << "[1/4] Compilation du TP1...\n";
    if (run_command("gcc -std=c11 -Wall -Wextra -Wpedantic "
                    + quote_argument(tp1_source) + " -o "
                    + quote_argument(tp1_executable)) != 0)
        return EXIT_FAILURE;

    std::cout << "[2/4] Compilation du TP2...\n";
    if (run_command("gcc -std=c11 -Wall -Wextra -Wpedantic "
                    + quote_argument(tp2_source) + " -o "
                    + quote_argument(tp2_executable)) != 0) {
        fs::remove(tp1_executable);
        return EXIT_FAILURE;
    }

    std::cout << "[3/4] Analyse lexicale du TP1...\n";
    if (run_command(quote_argument(tp1_executable) + " "
                    + quote_argument(input) + " "
                    + quote_argument(tokens)) != 0) {
        fs::remove(tp1_executable);
        fs::remove(tp2_executable);
        return EXIT_FAILURE;
    }

    std::cout << "[4/4] Analyse syntaxique du TP2...\n";
    const int result = run_command(quote_argument(tp2_executable) + " "
                                   + quote_argument(tokens));

    fs::remove(tp1_executable);
    fs::remove(tp2_executable);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
