// STD
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <algorithm>
#include <vector>
#include <string>

// local
#include <common/common.hpp>
#include <common/reverted.hpp>
#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include <algorithms/traversal.hpp>
#include <utils/graph_visualizer.h>


std::shared_ptr<common::TraversalGraph> getTraversalGraph(std::weak_ptr<common::Graph> weakGraph) {
    auto locked = weakGraph.lock();
    if (!locked) {
        std::cout << "There is no currently loaded graph in state.\n";
        return nullptr;
    }

    auto traversalGraph = std::dynamic_pointer_cast<common::TraversalGraph>(locked);
    if (!traversalGraph) {
        std::cout << "Error: Graph does not support traversal operations!\n";
        return nullptr;
    }

    return traversalGraph;
}

std::vector<std::pair<std::string, std::string>> lastUsedEdges;
std::vector<std::string> lastUsedNodes;

std::string serializeFileIntoString(const std::string& filename) {
    std::string result;
    result.resize(std::filesystem::file_size(filename));

    if (std::ifstream ifs {filename, std::ios::in | std::ios::binary}; ifs.is_open()) {
        ifs.read(result.data(), std::filesystem::file_size(filename));
    } else {
        std::cout << "Error opening file " << filename << "!";
        std::flush(std::cout);
        std::abort();
    }

    return result;

}

std::shared_ptr<common::Graph> loadGraph() {
    std::string filename;
    std::cout << "Enter filename: ";
    std::cin >> filename;

    if (!std::filesystem::exists(filename)) {
        std::cout << "File " << filename << " not found!";
        std::flush(std::cout);
        std::abort();
    }

    std::cout << "Lexing file...\n";
    auto inString = serializeFileIntoString(filename);
    auto lexemes = lexer::lex(inString);
    
    std::cout << "Parsing file...\n";
    auto graph = parser::parse(lexemes);

    std::cout << "Parse was successful!\n";
    std::cout << "Graph is loaded into memory.\n";

    return std::move(graph);
}

void dumpCurrentGraphState(std::weak_ptr<common::Graph> graph) {
    if (auto locked = graph.lock()) {
        std::cout << "Currently loaded graph:\n";
        std::cout << locked->dumpGraphState() << '\n';
        return;
    }
    std::cout << "There is no currently loaded graph in state.\n";
}
void showCurrentTraversalOrder(std::weak_ptr<common::Graph> graph) {
    auto traversalGraph = getTraversalGraph(graph);
    if (!traversalGraph) return;  
    std::cout << "Performing DFS traversal...\n";
    traversalGraph->showDFSOrder(); 
    std::cout << "DFS traversal done.\n";

}

void findGivenNode(std::weak_ptr<common::Graph> graph) {
    auto traversalGraph = getTraversalGraph(graph);
    if (!traversalGraph) return;

    std::string desired_node;
    std::cout << "Type a node to be found: \n";
    std::cin >> desired_node;

    const auto& nodes = traversalGraph->getNodes();
    if (std::find(nodes.begin(), nodes.end(), desired_node) == nodes.end()) {
        std::cout << "Node not found.\n";
        return;
    }

    std::cout << "Searching node...\n";
    traversalGraph->findNodeDFS(desired_node);
    std::cout << "Search done successfully.\n";
}

std::string dumpGraphToFile(std::weak_ptr<common::Graph> graph, common::GraphDumpingFactory& factory) {
    if (auto locked = graph.lock()) {
        std::string filename;
        std::cout << "Enter filename: ";
        std::cin >> filename;

        
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
        std::cout << "Dumping graph to file: " << filename << '\n';
        factory.dumpOne(*locked, filename);
        std::cout << "Dump was successful.\n";
        return filename;
    }
    std::cout << "There is no currently loaded graph in state.\n";
    return ""; 

}

void showCurrentBFSOrder(std::weak_ptr<common::Graph> graph) {
    auto traversalGraph = getTraversalGraph(graph);
    if (!traversalGraph) return;  // Ошибка уже выведена
    std::cout << "Performing BFS traversal...\n";
    traversalGraph->showBFSOrder();  // например, traverseBFS()
    std::cout << "BFS traversal done.\n";

}

void findGivenNodeBFS(std::weak_ptr<common::Graph> graph) {
    auto locked = graph.lock();
    if (!locked) {
        std::cout << "There is no currently loaded graph in state.\n";
        return;
    }
    
    auto traversal_graph = std::dynamic_pointer_cast<common::TraversalGraph>(locked);
    if (!traversal_graph) {
        std::cout << "Error: Graph does not support BFS search operations!\n";
        return;
    }

    std::string desired_node;
    std::cout << "Type a node to be found with BFS: \n";
    std::cin >> desired_node;

    const auto& nodes = traversal_graph->getNodes();
    if (std::find(nodes.begin(), nodes.end(), desired_node) == nodes.end()) {
        std::cout << "Node not found.\n";
    } else {
        std::cout << "Searching node using BFS...\n";
        traversal_graph->findNodeBFS(desired_node);
        std::cout << "Search done successfully.\n";
    }
}


void showShortestPathsDijkstra(std::weak_ptr<common::Graph> graph) {
    auto locked = graph.lock();
    if (!locked) {
        std::cout << "There is no currently loaded graph in state.\n";
        return;
    }

    auto traversal_graph = std::dynamic_pointer_cast<common::TraversalGraph>(locked);
    if (!traversal_graph) {
        std::cout << "Error: Graph does not support Dijkstra's algorithm!\n";
        return;
    }

    std::string start_node;
    std::cout << "Enter start node for Dijkstra's algorithm: ";
    std::cin >> start_node;

    traversal_graph->showDijkstraOrder(start_node);
}


void visualizeAndOpen(const std::string& inputDotPath, const std::string& outputImagePath) {
    // Формируем путь для промежуточного .dot файла с подсветкой
    std::string outputDotPath = inputDotPath.substr(0, inputDotPath.find_last_of('.')) + "_highlighted.dot";

    common::GraphVisualizer::saveGraphVisualization(
        inputDotPath,
        outputDotPath,
        outputImagePath,
        lastUsedEdges,
        lastUsedNodes
    );

    // Открываем изображение
    std::system(("start " + outputImagePath).c_str());
}



int main() {
    std::cout << "This program parses .gv files into"
              << " program-accessible structure.\n";

    int option = 0;

    common::GraphDumpingFactory::Settings settings;
    settings.verboseWrite = false;
    common::GraphDumpingFactory factory(settings);


    std::shared_ptr<common::Graph> state = nullptr;

    while (true) {
        std::cout << "Select action:\n" 
                  << "- load graph (1)\n"
                  << "- dump graph into a file (2)\n"
                  << "- dump currently loaded graph (3)\n"
                  << "- show graph traversal order (DFS) (4)\n"
                  << "- find node with (DFS) (5)\n"
                  << "- show graph traversal order (BFS) (7)\n"
                  << "- find node with (BFS) (8)\n"
                  << "- show shortest paths from node (Dijkstra) (9)\n"
                  << "- exit (6)\n";
        std::cin >> option;

        switch (option) {
        case 1: {
            state = loadGraph();
            auto traversalGraph = std::dynamic_pointer_cast<common::TraversalGraph>(state);
            if (!traversalGraph) {
                std::cerr << "Failed to cast to TraversalGraph" << std::endl;
                break;
            }
            break;
        }
        case 2: {

            std::string filename = dumpGraphToFile(state, factory); // возвращает имя файла
            if (!filename.empty()) {
                std::string output_png = filename.substr(0, filename.find_last_of('.')) + ".png";
                visualizeAndOpen(filename, output_png);
            } else {
                std::cout << "Dump failed or was cancelled.\n";
            }
            break;
        }
        case 3:
            dumpCurrentGraphState(state);
            
            break;
        case 4:
            showCurrentTraversalOrder(state);

            break;
        case 5:
            findGivenNode(state);

            break;

        case 7:
            showCurrentBFSOrder(state);
            break;

        case 8:
            findGivenNodeBFS(state);
            break;

        case 9:
            showShortestPathsDijkstra(state);
            break;
        case 6:
            return 0;
        default:
            std::cout << "Unknown option, aborting...";
            return 0;
        }
    }
    return 0;
}
