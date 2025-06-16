#include <gtest/gtest.h>

// std
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

// tested module
#include <algorithms/traversal.hpp>
#include <common/common.hpp>

#define GTEST_COUT(chain) \
    std::cout << "[ INFO     ] " << chain << '\n';

namespace {
    void pushSomeNodes(common::TraversalGraph& graph) {
        graph.pushNode("A");
        graph.pushNode("B");
        graph.pushNode("C");
        graph.pushNode("D");
        graph.pushNode("E");
    }

    void buildSampleGraph(common::TraversalGraph& graph) {
        graph.pushEdge("A", common::Connection("B", 1));
        graph.pushEdge("B", common::Connection("C", 2));
        graph.pushEdge("C", common::Connection("D", 3));
        graph.pushEdge("D", common::Connection("E", 4));
    }

    void prepareDirectory(const std::string& dir) {
        namespace fs = std::filesystem;
        if (!fs::exists(dir)) fs::create_directories(dir);
        for (const auto& file : fs::directory_iterator(dir)) {
            fs::remove_all(file);
        }
    }
}

class TraversalVisualDFSTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph_.init(common::opt::drc);
        pushSomeNodes(graph_);
        buildSampleGraph(graph_);

        // Очистка директорий
        dot_dir_ = "test_dfs_dot";
        png_dir_ = "test_dfs_png";
        prepareDirectory(dot_dir_);
        prepareDirectory(png_dir_);
    }

    common::TraversalGraph graph_;
    std::unordered_set<std::string> visited_;
    std::vector<std::string> traversal_order_;
    std::unordered_map<std::string, std::pair<int, int>> timestamps_;
    int discovery_time_ = 0;
    int step_counter_ = 0;
    std::string dot_dir_;
    std::string png_dir_;
};

TEST_F(TraversalVisualDFSTest, FullTraversalTimestamps) {
    graph_.DFSWithTimestamps("A", visited_, traversal_order_, timestamps_,
                             discovery_time_, step_counter_, dot_dir_, png_dir_);

    EXPECT_EQ(visited_.size(), 5);

    EXPECT_LT(timestamps_["A"].first, timestamps_["B"].first);
    EXPECT_LT(timestamps_["B"].first, timestamps_["C"].first);
    EXPECT_GT(timestamps_["E"].second, timestamps_["D"].second);

    GTEST_COUT("Traversal order: ");
    for (const auto& node : traversal_order_) {
        GTEST_COUT(node);
    }
}

TEST_F(TraversalVisualDFSTest, DisconnectedComponentTraversal) {
    graph_.pushNode("F");
    graph_.pushNode("G");
    graph_.pushEdge("F", common::Connection("G", 1));

    graph_.DFSWithTimestamps("A", visited_, traversal_order_, timestamps_,
                             discovery_time_, step_counter_, dot_dir_, png_dir_);

    graph_.DFSWithTimestamps("F", visited_, traversal_order_, timestamps_,
                             discovery_time_, step_counter_, dot_dir_, png_dir_);

    EXPECT_EQ(visited_.size(), 7);
    EXPECT_TRUE(timestamps_["F"].first > timestamps_["E"].second);
}

TEST_F(TraversalVisualDFSTest, VisualizationFilesCreated) {
    graph_.DFSWithTimestamps("A", visited_, traversal_order_, timestamps_,
                             discovery_time_, step_counter_, dot_dir_, png_dir_);

    namespace fs = std::filesystem;
    size_t dot_files = std::distance(fs::directory_iterator(dot_dir_), {});
    size_t png_files = std::distance(fs::directory_iterator(png_dir_), {});

    // Кол-во шагов визуализации должно быть не меньше количества рёбер (для линейного графа: 4 перехода)
    EXPECT_GE(dot_files, 4);
    EXPECT_EQ(dot_files, png_files);  // Каждому .dot соответствует .png
}

class BFSTraversalTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph_.init(common::opt::drc);
        pushSomeNodes(graph_);
        buildSampleGraph(graph_);
    }

    common::TraversalGraph graph_;
    std::unordered_set<std::string> visited_;
    std::vector<std::string> traversal_order_;
    std::unordered_map<std::string, std::pair<int, int>> timestamps_;
    int time_counter_ = 0;

    // Папки для визуализации
    const std::string dot_dir_ = "test_output/bfs/dots";
    const std::string png_dir_ = "test_output/bfs/pngs";
};

TEST_F(BFSTraversalTest, FullTraversalOrderAndTimestamps) {
    graph_.BFSWithTimestamps("A", visited_, traversal_order_, timestamps_, time_counter_, dot_dir_, png_dir_);

    EXPECT_EQ(visited_.size(), 5);

    std::vector<std::string> expected_order = {"A", "B", "C", "D", "E"};
    EXPECT_EQ(traversal_order_, expected_order);

    for (const auto& [node, times] : timestamps_) {
        EXPECT_LT(times.first, times.second) << "Invalid timestamp for node: " << node;
    }
}

TEST_F(BFSTraversalTest, ComponentTraversalBFS) {
    // добавим ещё одну компоненту связности
    graph_.pushNode("F");
    graph_.pushNode("G");
    graph_.pushEdge("F", common::Connection("G", 10));

    graph_.BFSWithTimestamps("A", visited_, traversal_order_, timestamps_, time_counter_, dot_dir_, png_dir_);
    graph_.BFSWithTimestamps("F", visited_, traversal_order_, timestamps_, time_counter_, dot_dir_, png_dir_);

    EXPECT_EQ(visited_.size(), 7);
    EXPECT_TRUE(visited_.contains("F"));
    EXPECT_TRUE(visited_.contains("G"));
}

TEST_F(BFSTraversalTest, VisualizationFilesCreated) {
    graph_.BFSWithTimestamps("A", visited_, traversal_order_, timestamps_, time_counter_, dot_dir_, png_dir_);

    // Проверим, что файлы действительно создались
    namespace fs = std::filesystem;

    ASSERT_TRUE(fs::exists(dot_dir_)) << "DOT directory not created";
    ASSERT_TRUE(fs::exists(png_dir_)) << "PNG directory not created";

    bool found_dot = false, found_png = false;
    for (const auto& entry : fs::directory_iterator(dot_dir_)) {
        if (entry.path().extension() == ".dot") {
            found_dot = true;
            break;
        }
    }
    for (const auto& entry : fs::directory_iterator(png_dir_)) {
        if (entry.path().extension() == ".png") {
            found_png = true;
            break;
        }
    }

    EXPECT_TRUE(found_dot) << "No .dot files created";
    EXPECT_TRUE(found_png) << "No .png files created";
}

class TraversalTestDijkstra : public ::testing::Test {
protected:
    void SetUp() override {
        graph_.init(common::opt::drc);
        pushSomeNodes(graph_);
        buildSampleGraph(graph_);
    }

    common::TraversalGraph graph_;
    std::unordered_map<std::string, int> distances_;
    std::unordered_map<std::string, std::string> parents_;
    std::unordered_map<std::string, std::pair<int, int>> timestamps_;
    std::vector<std::string> traversal_order_;
    int discovery_time_ = 0;
};

TEST_F(TraversalTestDijkstra, DijkstraTraversalOrderAndTimestamps) {
    graph_.dijkstraWithTimestamps("A", distances_, timestamps_, traversal_order_, discovery_time_, parents_);

    EXPECT_EQ(distances_["E"], 10);
    EXPECT_EQ(traversal_order_.front(), "A");
    EXPECT_EQ(traversal_order_.back(), "E");

    EXPECT_LT(timestamps_["A"].first, timestamps_["B"].first);
    EXPECT_LT(timestamps_["B"].first, timestamps_["C"].first);
    EXPECT_LT(timestamps_["C"].first, timestamps_["D"].first);
    EXPECT_LT(timestamps_["D"].first, timestamps_["E"].first);
    EXPECT_LT(timestamps_["E"].first, timestamps_["E"].second);
}

TEST_F(TraversalTestDijkstra, DijkstraParentTracking) {
    graph_.dijkstraWithTimestamps("A", distances_, timestamps_, traversal_order_, discovery_time_, parents_);

    EXPECT_EQ(parents_["B"], "A");
    EXPECT_EQ(parents_["C"], "B");
    EXPECT_EQ(parents_["D"], "C");
    EXPECT_EQ(parents_["E"], "D");

    std::string node = "E";
    std::vector<std::string> reversed_path;
    while (!node.empty()) {
        reversed_path.push_back(node);
        node = parents_[node];
    }
    std::reverse(reversed_path.begin(), reversed_path.end());
    std::vector<std::string> expected_path = {"A", "B", "C", "D", "E"};
    EXPECT_EQ(reversed_path, expected_path);
}

TEST_F(TraversalTestDijkstra, DijkstraDisconnectedComponent) {
    graph_.pushNode("F");
    graph_.pushNode("G");
    graph_.pushEdge("F", common::Connection("G", 1));

    graph_.dijkstraWithTimestamps("A", distances_, timestamps_, traversal_order_, discovery_time_, parents_);

    EXPECT_EQ(distances_.count("F"), 0);
    EXPECT_EQ(distances_.count("G"), 0);
    EXPECT_FALSE(parents_.contains("F"));
    EXPECT_FALSE(parents_.contains("G"));
}

TEST_F(TraversalTestDijkstra, DijkstraStartFromIsolatedNode) {
    graph_.pushNode("Z"); // isolated

    graph_.dijkstraWithTimestamps("Z", distances_, timestamps_, traversal_order_, discovery_time_, parents_);

    EXPECT_EQ(distances_["Z"], 0);
    EXPECT_EQ(traversal_order_.size(), 1);
    EXPECT_EQ(traversal_order_.front(), "Z");
}
