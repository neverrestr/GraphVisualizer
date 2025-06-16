
#include "traversal.hpp"
#include <iostream>
#include <queue>
#include <limits> 
#include <algorithm> 
#include "utils/graph_visualizer.h"
#include <fstream>
#include <filesystem>
#include <functional>
#include <set>


using namespace common;
namespace fs = std::filesystem;


void clearDirectory(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            fs::remove_all(entry.path());  // удаляет файлы и поддиректории рекурсивно
        }
    }
}

void TraversalGraph::DFSWithTimestamps(
    const std::string& node,
    std::unordered_set<std::string>& visited,
    std::vector<std::string>& result,
    std::unordered_map<std::string, std::pair<int, int>>& timestamps,
    int& discovery_time,
    int& step_counter,
    const std::string& dot_dir,
    const std::string& png_dir)
{
    visited.insert(node);
    timestamps[node].first = ++discovery_time;
    this->setLabel(node, node + " : " + std::to_string(timestamps[node].first) + "/");

    if (connections_->find(node) == connections_->end()) {
        timestamps[node].second = ++discovery_time;
        this->setLabel(node, node + " : " + std::to_string(timestamps[node].first) + "/" + std::to_string(timestamps[node].second));
        result.push_back(node);
        return;
    }

    const auto& edges = connections_->at(node);
    for (const auto& edge : edges) {
        const std::string& neighbor = edge.peer;
        if (!visited.contains(neighbor)) {

            // Визуализация перехода по ребру node -> neighbor
            std::vector<std::pair<std::string, std::string>> highlightEdges = { {node, neighbor} };
            std::vector<std::string> highlightNodes = { node, neighbor };
            common::GraphVisualizer::saveGraphVisualization(
                "parsed_graph.gv", 
                dot_dir + "/dfs_step_" + std::to_string(step_counter) + ".dot",
                png_dir + "/dfs_step_" + std::to_string(step_counter) + ".png",
                highlightEdges,
                highlightNodes
            );
            step_counter++;

            DFSWithTimestamps(neighbor, visited, result, timestamps, discovery_time, step_counter, dot_dir, png_dir);
        }
    }
    timestamps[node].second = ++discovery_time;
    this->setLabel(node, node + " : " + std::to_string(timestamps[node].first) + "/" + std::to_string(timestamps[node].second));
    result.push_back(node);
}


void TraversalGraph::showDFSOrder() { 
    fs::path dot_dir = "DFStraversal_output_dot";
    fs::path png_dir = "DFStraversal_output_png";

    if (!fs::exists(dot_dir)) {
        fs::create_directory(dot_dir);
    }
    if (!fs::exists(png_dir)) {
        fs::create_directory(png_dir);
    }
    std::unordered_set<std::string> visited;
    std::vector<std::string> traversalOrder;
    std::unordered_map<std::string, std::pair<int, int>> timestamps;
    std::vector<std::string> graph_nodes = this->getNodes();

    std::string start_node;
    int discovery_time = 0;
    int max_visited = 0;
    int step_counter = 0; 

    for (const auto& node : graph_nodes) {
        std::unordered_set<std::string> temp_visited;
        std::vector<std::string> temp_result;
        std::unordered_map<std::string, std::pair<int, int>> temp_timestamps;
        int temp_time = 0;
        int dummy_step_counter = 0;

        this->DFSWithTimestamps(node, temp_visited, temp_result, temp_timestamps, temp_time, dummy_step_counter, "", "");

        if (temp_visited.size() > max_visited || (temp_visited.size() == max_visited && (start_node.empty() || node < start_node))) {
            max_visited = temp_visited.size();
            start_node = node;
        }
    }

    visited.clear();
    traversalOrder.clear();
    timestamps.clear();
    discovery_time = 0;
    step_counter = 0;

    this->DFSWithTimestamps(start_node, visited, traversalOrder, timestamps, discovery_time, step_counter, dot_dir.string(),  png_dir.string());

    std::cout << "DFS traversal order starting from node '" << start_node << "':\n";
    for (const auto& node : traversalOrder) {
        std::cout << node;
        if (timestamps.find(node) != timestamps.end()) {
            std::cout << " [" << timestamps[node].first << "/" << timestamps[node].second << "]";
        }
        std::cout << "\n";
    }
}


void TraversalGraph::findNodeDFS(const std::string& desired_node) {
    namespace fs = std::filesystem;

    std::string dot_dir = "DFSfind_output_dot";
    std::string png_dir = "DFSfind_output_png";

    if (!fs::exists(dot_dir)) {
        fs::create_directory(dot_dir);
    }
    if (!fs::exists(png_dir)) {
        fs::create_directory(png_dir);
    }

    clearDirectory(dot_dir);
    clearDirectory(png_dir);

    std::unordered_set<std::string> visited;
    std::unordered_map<std::string, std::string> parents;
    int step_counter = 0;
    bool found = false;

    // Очистка меток
    for (const auto& node : getNodes()) {
        if (getLabel(node)) {
            removeLabel(node);
        }
    }

    // Вспомогательная лямбда для рекурсивного DFS 
    std::function<void(const std::string&)> dfs_visit = [&](const std::string& current) {
        
        if (found) return; // Если нашли, останавливаем обход

        visited.insert(current);

        setLabel(current, current + " visited");

        // подсвечиваем текущую вершину
        std::vector<std::string> highlightNodes = { current };

        // Если есть родитель, подсвечиваем ребро от родителя к текущему
        std::vector<std::pair<std::string, std::string>> highlightEdges;
        if (parents.count(current) && !parents[current].empty()) {
            highlightEdges.push_back({ parents[current], current });
            highlightNodes.push_back(parents[current]);
        }

        common::GraphVisualizer::saveGraphVisualization(
            "parsed_graph.gv", 
            dot_dir + "/dfs_step_" + std::to_string(step_counter) + ".dot",
            png_dir + "/dfs_step_" + std::to_string(step_counter) + ".png",
            highlightEdges,
            highlightNodes
        );
        step_counter++;

        if (current == desired_node) {
            found = true;
            return;
        }

        // Проходим по соседям
        if (connections_->find(current) != connections_->end()) {
            for (const auto& edge : connections_->at(current)) {
                const std::string& neighbor = edge.peer;
                if (visited.find(neighbor) == visited.end()) {
                    parents[neighbor] = current;
                    dfs_visit(neighbor);
                    if (found) return;
                }
            }
        }
    };

    std::string start_node;
    const auto nodes = getNodes();

    if (std::find(nodes.begin(), nodes.end(), desired_node) == nodes.end()) {
        std::cout << "Node " << desired_node << " not found in graph nodes.\n";
        return;
    }

    start_node = nodes.front(); 

    dfs_visit(start_node);

    if (found) {
        std::cout << "Node " << desired_node << " found.\n";
    } else {
        std::cout << "Node " << desired_node << " not found in reachable nodes from " << start_node << ".\n";
    }
}

void TraversalGraph::BFSWithTimestamps(
    const std::string& start_node,
    std::unordered_set<std::string>& visited,
    std::vector<std::string>& result,
    std::unordered_map<std::string, std::pair<int, int>>& timestamps,
    int& time_counter,
    const std::string& dot_dir,
    const std::string& png_dir)
{

if (!dot_dir.empty() && !fs::exists(dot_dir)) fs::create_directories(dot_dir);
if (!png_dir.empty() && !fs::exists(png_dir)) fs::create_directories(png_dir);


    std::queue<std::string> q;
    visited.insert(start_node);
    q.push(start_node);
    timestamps[start_node].first = ++time_counter;

    int step_counter = 0;

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();
        result.push_back(current);

        // Устанавливаем метку для текущего узла
        this->setLabel(current, current + " : " + std::to_string(timestamps[current].first) + "/");

        if (connections_->find(current) != connections_->end()) {
            const auto& neighbors = connections_->at(current);
            for (const auto& edge : neighbors) {
                const std::string& neighbor = edge.peer;
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                    timestamps[neighbor].first = ++time_counter;
                }
            }
        }

        timestamps[current].second = ++time_counter;
        // Обновляем метку с временем выхода
        this->setLabel(current, current + " : " + std::to_string(timestamps[current].first) + '/' + std::to_string(timestamps[current].second));

        // Для визуализации подсвечиваем все рёбра, ведущие к только что посещённым соседям
        std::vector<std::pair<std::string, std::string>> highlightEdges;
        std::vector<std::string> highlightNodes = {current};
        if (connections_->find(current) != connections_->end()) {
            const auto& neighbors = connections_->at(current);
            for (const auto& edge : neighbors) {
                if (visited.contains(edge.peer)) {
                    highlightEdges.emplace_back(current, edge.peer);
                    highlightNodes.push_back(edge.peer);
                }
            }
        }

        common::GraphVisualizer::saveGraphVisualization(
            "parsed_graph.gv",
            dot_dir + "/bfs_step_" + std::to_string(step_counter) + ".dot",
            png_dir + "/bfs_step_" + std::to_string(step_counter) + ".png",
            highlightEdges,
            highlightNodes
        );

        step_counter++;
    }
}

void TraversalGraph::showBFSOrder() {
    std::unordered_set<std::string> visited;
    std::vector<std::string> traversalOrder;
    std::unordered_map<std::string, std::pair<int, int>> timestamps;
    std::vector<std::string> graph_nodes = this->getNodes();
    std::string start_node;
    int time_counter = 0;
    int max_visited = 0;

    for (const auto& node : graph_nodes) {
        if (this->getLabel(node)) {
            this->removeLabel(node);
        }
    }

    for (const auto& node : graph_nodes) {
        std::unordered_set<std::string> temp_visited;
        std::vector<std::string> temp_result;
        std::unordered_map<std::string, std::pair<int, int>> temp_timestamps;
        int temp_time = 0;

        this->BFSWithTimestamps(node, temp_visited, temp_result, temp_timestamps, temp_time, "", "");

        if (temp_visited.size() > max_visited || (temp_visited.size() == max_visited && (start_node.empty() || node < start_node))) {
            max_visited = temp_visited.size();
            start_node = node;
        }
    }

    visited.clear();
    traversalOrder.clear();
    timestamps.clear();
    time_counter = 0;

    // Финальный обход с визуализацией
    this->BFSWithTimestamps(start_node, visited, traversalOrder, timestamps, time_counter, "BFStraversal_output_dot", "BFStraversal_output_png");

    std::cout << "BFS traversal order starting from node '" << start_node << "':\n";
    for (const auto& node : traversalOrder) {
        std::cout << node;
        if (timestamps.find(node) != timestamps.end()) {
            std::cout << " [" << timestamps[node].first << "/" << timestamps[node].second << "]";
        }
        std::cout << "\n";
    }
}


void TraversalGraph::findNodeBFS(std::string& desired_node) {
    std::string output_dir = "BFSfind_output_dot";
    std::string output_dir2 = "BFSfind_output_png";

    if (!fs::exists(output_dir)) {
        fs::create_directory(output_dir);
    }
    if (!fs::exists(output_dir2)) {
        fs::create_directory(output_dir2);
    }

    // Очищаем папки от предыдущих результатов
    clearDirectory(output_dir);
    clearDirectory(output_dir2);

    std::unordered_set<std::string> visited;
    std::vector<std::string> traversalOrder;
    std::unordered_map<std::string, std::pair<int, int>> timestamps;
    std::unordered_map<std::string, std::string> parents;  // добавляем родителей
    std::vector<std::string> graph_nodes = this->getNodes();

    // Очистка меток
    for (const auto& node : graph_nodes) {
        if (this->getLabel(node)) {
            this->removeLabel(node);
        }
    }

    std::string start_node;
    int max_visited = 0;

    // Выбираем стартовый узел с максимальной компонентой связности
    for (const auto& node : graph_nodes) {
        std::unordered_set<std::string> temp_visited;
        std::vector<std::string> temp_result;
        std::unordered_map<std::string, std::pair<int, int>> temp_timestamps;
        int temp_time = 0;

        BFSWithTimestamps(node, temp_visited, temp_result, temp_timestamps, temp_time, "","");

        if (temp_visited.size() > max_visited || (temp_visited.size() == max_visited && node < start_node)) {
            max_visited = temp_visited.size();
            start_node = node;
        }
    }

    std::queue<std::string> q;
    int time_counter = 0;
    visited.clear();
    traversalOrder.clear();
    timestamps.clear();

    visited.insert(start_node);
    q.push(start_node);
    timestamps[start_node].first = ++time_counter;
    parents[start_node] = "";  // Стартовая вершина не имеет родителя

    bool found = false;
    int step_counter = 0;  // Счетчик шагов для визуализации

    while (!q.empty() && !found) {
        std::string current = q.front();
        q.pop();
        traversalOrder.push_back(current);

        if (current == desired_node) {
            found = true;
        }

        if (connections_->find(current) != connections_->end()) {
            const auto& neighbors = connections_->at(current);
            for (const auto& edge : neighbors) {
                const std::string& neighbor = edge.peer;
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                    timestamps[neighbor].first = ++time_counter;
                    parents[neighbor] = current; // Сохраняем родителя 
                
                 // Визуализируем переход по ребру current -> neighbor
                    std::vector<std::pair<std::string, std::string>> highlightEdges = { {current, neighbor} };
                    std::vector<std::string> highlightNodes = { current, neighbor };
                    common::GraphVisualizer::saveGraphVisualization(
                    "parsed_graph.gv",  
                    output_dir + "/bfs_step_" + std::to_string(step_counter) + ".dot", 
                    output_dir2 + "/bfs_step_" + std::to_string(step_counter) + ".png",
                    highlightEdges,
                    highlightNodes
                );

                    step_counter++;
                }
            }
        }
        timestamps[current].second = ++time_counter;
        this->setLabel(current, current + " : " + std::to_string(timestamps[current].first) + '/' + std::to_string(timestamps[current].second));
    }
}


void TraversalGraph::saveToDotFile(const std::string& filename) {
    std::ofstream file(filename);
    file << "digraph G {\n";
    for (const auto& [from, edges] : this->adjacency_list) {
        for (const auto& [to, weight] : edges) {
            file << "  \"" << from << "\" -> \"" << to << "\" [label=\"" << weight << "\"];\n";
        }
    }
    file << "}\n";
}

void TraversalGraph::dijkstraWithTimestamps(
    const std::string& start_node,
    std::unordered_map<std::string, int>& distances,
    std::unordered_map<std::string, std::pair<int, int>>& timestamps,
    std::vector<std::string>& result,
    int& time_counter,
    std::unordered_map<std::string, std::string>& parents
) {
    using Pair = std::pair<int, std::string>; // (distance, node)

    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq;
    std::unordered_set<std::string> visited;

    this->saveToDotFile("original.dot");

    distances[start_node] = 0;
    pq.emplace(0, start_node);
    timestamps[start_node].first = ++time_counter;
    parents[start_node] = "";

    while (!pq.empty()) {
        auto [current_distance, current] = pq.top();
        pq.pop();

        if (visited.contains(current)) continue;
        visited.insert(current);

        result.push_back(current);
        timestamps[current].second = ++time_counter;

        this->setLabel(current, current + " : d=" + std::to_string(current_distance) +
                                 " [" + std::to_string(timestamps[current].first) +
                                 "/" + std::to_string(timestamps[current].second) + "]");

        if (connections_->find(current) == connections_->end()) continue;

        const auto& neighbors = connections_->at(current);
        for (const auto& edge : neighbors) {
            const std::string& neighbor = edge.peer;
            if (!edge.weight.has_value()) continue;

            int weight = edge.weight.value();
            int new_distance = current_distance + weight;

            if (new_distance < distances[neighbor]) {
                distances[neighbor] = new_distance;
                parents[neighbor] = current;
                pq.emplace(new_distance, neighbor);
                timestamps[neighbor].first = ++time_counter;
            }
        }
    }

    // Подсвеченные рёбра кратчайшего пути
    std::vector<std::pair<std::string, std::string>> highlightedEdges;
    for (const auto& [node, parent] : parents) {
        if (!parent.empty()) {
            highlightedEdges.emplace_back(parent, node);
        }
    }

    // Визуализация (использует исходный граф и добавляет подсветку)
    common::GraphVisualizer::saveGraphVisualization(
        "original.dot",              // исходный граф
        "dijkstra_output.dot",       // результат с подсветкой
        "dijkstra_result.png",       // изображение
        highlightedEdges,            
        {}                           
    );
    common::GraphVisualizer::openImage("dijkstra_result.png");
}

void TraversalGraph::showDijkstraOrder(const std::string& source_node) {
    std::unordered_map<std::string, int> distances;
    std::unordered_map<std::string, std::pair<int, int>> timestamps;
    std::vector<std::string> traversalOrder;
    std::unordered_map<std::string, std::string> parents; 

    std::vector<std::string> nodes = this->getNodes();
    int time_counter = 0;

    for (const auto& node : nodes) {
        distances[node] = std::numeric_limits<int>::max();
        if (this->getLabel(node)) {
            this->removeLabel(node);
        }
    }

    if (std::find(nodes.begin(), nodes.end(), source_node) == nodes.end()) {
        std::cout << "Node \"" << source_node << "\" not found in the graph.\n";
        return;
    }

    dijkstraWithTimestamps(source_node, distances, timestamps, traversalOrder, time_counter, parents);

    std::cout << "Dijkstra traversal from \"" << source_node << "\":\n";
    for (const auto& node : traversalOrder) {
        if (this->getLabel(node).has_value()) {
            std::cout << "Node: " << this->getLabel(node).value() << "\n";
        }
    }
}

void TraversalGraph::findNodeDijkstra(std::string& desired_node) {
    std::string found_node;
    for (const auto& [node, _] : this->adjacency_list) {
        if (node == desired_node) {
            found_node = node;
            break;
        }
    }

    if (found_node.empty()) {
        std::cout << "Node '" << desired_node << "' not found in graph.\n";
        return;
    }

    for (const auto& [node, _] : this->adjacency_list) {
        std::unordered_map<std::string, int> temp_distances;
        std::unordered_map<std::string, std::pair<int, int>> temp_timestamps;
        std::vector<std::string> temp_result;
        int temp_time = 0;
        std::unordered_map<std::string, std::string> temp_parents;

        dijkstraWithTimestamps(node, temp_distances, temp_timestamps, temp_result, temp_time, temp_parents);

        if (temp_distances.find(desired_node) == temp_distances.end() || temp_distances[desired_node] == INT_MAX)
            continue;

        // Восстановление кратчайшего пути
        std::vector<std::string> path;
        for (std::string at = desired_node; !at.empty(); at = temp_parents[at]) {
            path.push_back(at);
        }
        std::reverse(path.begin(), path.end());

        std::cout << "Shortest path from '" << node << "' to '" << desired_node << "': ";
        for (const auto& n : path) std::cout << n << " ";
        std::cout << "\n";

        // Сбор рёбер пути
        std::vector<std::pair<std::string, std::string>> highlightedEdges;
        for (size_t i = 1; i < path.size(); ++i) {
            highlightedEdges.emplace_back(path[i - 1], path[i]);
        }

        std::vector<std::string> highlightedNodes(path.begin(), path.end());

        common::GraphVisualizer::saveGraphVisualization(
            "graph.dot",                 
            "dijkstra_result.dot",        
            "dijkstra_result.png",       
            highlightedEdges,
            highlightedNodes
        );

        common::GraphVisualizer::openImage("dijkstra_result.png");
        return;
    }

    std::cout << "No path to node '" << desired_node << "' found.\n";
}


