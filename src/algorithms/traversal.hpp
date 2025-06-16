#pragma once

#include <common/common.hpp>
#include <queue>  // Для std::queue в bfsWithTimestamps

namespace common {

/**
 * @class TraversalGraph
 * @brief Класс для обхода графа с поддержкой DFS, BFS и алгоритма Дейкстры с временными метками и визуализацией.
 */
class TraversalGraph : public Graph {
public:

    /**
     * @brief Выполняет рекурсивный обход в глубину (DFS) с отметками времени входа и выхода.
     * @param node Текущая обрабатываемая вершина.
     * @param visited Множество уже посещённых вершин.
     * @param result Вектор для сохранения порядка обхода.
     * @param timestamps Отметки времени входа и выхода для каждой вершины.
     * @param discovery_time Время начала посещения текущей вершины.
     * @param step_counter Счётчик шагов обхода для меток времени.
     * @param dot_dir Путь к директории для сохранения .dot файлов визуализации.
     * @param png_dir Путь к директории для сохранения изображений в формате .png.
     */
    void DFSWithTimestamps(
        const std::string& node,
        std::unordered_set<std::string>& visited,
        std::vector<std::string>& result,
        std::unordered_map<std::string, std::pair<int, int>>& timestamps,
        int& discovery_time,
        int& step_counter,
        const std::string& dot_dir,
        const std::string& png_dir
    );

    /**
     * @brief Сохраняет текущий граф в файл формата DOT.
     * @param filename Имя файла для сохранения.
     */
    void saveToDotFile(const std::string& filename);

    /**
     * @brief Выполняет рекурсивный DFS с отметками времени.
     * @param node Текущая вершина для обработки.
     * @param visited Множество посещённых вершин.
     * @param result Вектор для хранения порядка обхода.
     * @param timestamps Отметки времени входа/выхода по вершинам.
     * @param discovery_time Время начала обхода текущей вершины.
     * @param flag_to_quit Флаг для выхода из рекурсии при нахождении нужной вершины.
     * @param searched_node Искомая вершина.
     */
    // (Комментарий к функции, если она будет реализована отдельно)

    /**
     * @brief Поиск заданной вершины с помощью обхода в глубину (DFS).
     * @param desired_node Имя искомой вершины графа.
     */
    void findNodeDFS(const std::string& desired_node);

    /**
     * @brief Показывает порядок обхода графа в глубину (DFS).
     */
    void showDFSOrder();

    /**
     * @brief Выполняет обход в ширину (BFS) с отметками времени посещения.
     * @param start_node Начальная вершина для обхода.
     * @param visited Множество посещённых вершин.
     * @param result Вектор порядка обхода.
     * @param timestamps Отметки времени для каждой вершины.
     * @param time_counter Счётчик времени обхода.
     * @param dot_dir Путь к директории для сохранения .dot файлов.
     * @param png_dir Путь к директории для сохранения изображений .png.
     */
    void BFSWithTimestamps(
        const std::string& start_node,
        std::unordered_set<std::string>& visited,
        std::vector<std::string>& result,
        std::unordered_map<std::string, std::pair<int, int>>& timestamps,
        int& time_counter,
        const std::string& dot_dir,
        const std::string& png_dir
    );

    /**
     * @brief Показывает порядок обхода графа в ширину (BFS).
     */
    void showBFSOrder();

    /**
     * @brief Поиск заданной вершины с помощью обхода в ширину (BFS).
     * @param desired_node Имя искомой вершины графа.
     */
    void findNodeBFS(std::string& desired_node);

    /**
     * @brief Представляет ребро графа с целевой вершиной и весом.
     */
    struct Edge {
        std::string target; ///< Целевая вершина ребра
        int weight;        ///< Вес ребра
    };

    /**
     * @brief Алгоритм Дейкстры с отметками времени для каждой вершины и визуализацией.
     * @param start_node Начальная вершина для алгоритма.
     * @param distances Карта расстояний от начальной вершины до всех остальных.
     * @param timestamps Отметки времени входа и выхода для вершин.
     * @param result Вектор порядка обхода вершин.
     * @param time_counter Счётчик времени для отметок.
     * @param parents Карта родителей для восстановления кратчайшего пути.
     */
    void dijkstraWithTimestamps(
        const std::string& start_node,
        std::unordered_map<std::string, int>& distances,
        std::unordered_map<std::string, std::pair<int, int>>& timestamps,
        std::vector<std::string>& result,
        int& time_counter,
        std::unordered_map<std::string, std::string>& parents
    );

    /// Список смежности графа: вершина -> вектор рёбер (Edge)
    std::unordered_map<std::string, std::vector<Edge>> adjacency_list;

    /**
     * @brief Показывает порядок обхода графа алгоритмом Дейкстры.
     * @param source_node Начальная вершина.
     */
    void showDijkstraOrder(const std::string& source_node);

    /**
     * @brief Поиск заданной вершины и визуализация кратчайшего пути к ней с помощью алгоритма Дейкстры.
     * @param desired_node Имя искомой вершины.
     */
    void findNodeDijkstra(std::string& desired_node);

};

} // namespace common

/**
 * @brief Очищает содержимое указанной директории.
 * @param path Путь к директории.
 */
void clearDirectory(const std::string& path);
