#include "graph_visualizer.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <iostream>


using namespace std;

namespace common {

void GraphVisualizer::saveGraphVisualization(
    const std::string& inputDotPath,    
    const std::string& outputDotPath,    
    const std::string& outputImagePath,  
    const vector<pair<string, string>>& highlightedEdges,
    const vector<string>& highlightedNodes
) {
    std::ifstream input(inputDotPath);
    std::ofstream output(outputDotPath);
    std::string line;

    while (std::getline(input, line)) {
        // Перед закрывающей скобкой — вставим стили
        if (line.find('}') != std::string::npos) {
            for (const auto& edge : highlightedEdges) {
                output << "  \"" << edge.first << "\" -> \"" << edge.second
                       << "\" [color=red, penwidth=3];\n";
            }
            for (const auto& node : highlightedNodes) {
                output << "  \"" << node << "\" [style=filled, fillcolor=yellow];\n";
            }
        }
        output << line << "\n";
    }

    input.close();
    output.close();

    // Генерация PNG
    std::string cmd = "dot -Tpng " + outputDotPath + " -o " + outputImagePath;
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "Graphviz generation failed!" << std::endl;
    }
}

void GraphVisualizer::openImage(const string& path) {
    system(("start " + path).c_str());  
}

}
