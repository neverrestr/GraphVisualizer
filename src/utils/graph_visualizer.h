#pragma once

#include <string>
#include <vector>
#include <utility>

namespace common {

/**
 * @class GraphVisualizer
 * @brief Класс для визуализации графов с подсветкой вершин и рёбер, а также открытия изображений.
 */
class GraphVisualizer {
public:
    /**
     * @brief Сохраняет визуализацию графа с выделенными рёбрами и вершинами.
     * @param inputDotPath Путь к исходному файлу в формате DOT.
     * @param outputDotPath Путь для сохранения обновлённого файла DOT с подсветкой.
     * @param outputImagePath Путь для сохранения сгенерированного изображения графа (например, PNG).
     * @param highlightedEdges Вектор пар рёбер, которые необходимо выделить (source -> target).
     * @param highlightedNodes Вектор вершин, которые необходимо выделить.
     */
    static void saveGraphVisualization(
        const std::string& inputDotPath,   
        const std::string& outputDotPath,   
        const std::string& outputImagePath,  
        const std::vector<std::pair<std::string, std::string>>& highlightedEdges,
        const std::vector<std::string>& highlightedNodes
    );

    /**
     * @brief Открывает изображение графа по указанному пути.
     * @param path Путь к изображению, которое необходимо открыть.
     */
    static void openImage(const std::string& path);
};

} // namespace common
