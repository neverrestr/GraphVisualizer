#pragma once

// standard
#include <cstdint>
#include <memory>
#include <string>

// internal
#include <common/action-queue.hpp>
#include <common/common.hpp>
#include <algorithms/traversal.hpp>

// contrib
#include <tinyfsm.hpp>


namespace parser {
    inline void throw_invalid_input(std::string message) {
        throw std::runtime_error(message);
    }

    struct SharedState {
        std::shared_ptr<common::Graph> graph;

        common::ActionQueue backoffQueue;
        common::ActionQueue actionQueue;

        std::string fromNodeId;
        std::string toNodeId;
        std::string label;
        std::string expectedValue;

        std::uint8_t flags = 0x0;
        int weight = -1; 
    };

    // ----------------------------------------------------------------------------
    // 1. Event Declarations
    //
    struct GraphEvent               : tinyfsm::Event {};
    struct InputOpenCurlyBracket    : GraphEvent {};
    struct InputCloseCurlyBracket   : GraphEvent {};
    struct InputOpenSquareBracket   : GraphEvent {};
    struct InputCloseSquareBracket  : GraphEvent {};
    struct InputEdge                : GraphEvent {};
    struct InputLabel               : GraphEvent {};
    struct InputEqual               : GraphEvent {};
    struct InputGraphType           : GraphEvent { std::string graphType; };
    struct InputNodeId              : GraphEvent { std::string NodeID; };
    struct InputStringValue         : GraphEvent { std::string label; };
    struct InputIntValue            : GraphEvent { int weight; };


    // ----------------------------------------------------------------------------
    // 2. State Machine Base Class Declaration
    //
    class LexemeParser : public tinyfsm::Fsm<LexemeParser> {
    public:
        virtual void react(InputGraphType const &)          {};
        virtual void react(InputOpenCurlyBracket const &)   { throw_invalid_input(""); };
        virtual void react(InputCloseCurlyBracket const &)  { throw_invalid_input(""); };
        virtual void react(InputNodeId const &)             { throw_invalid_input(""); };
        virtual void react(InputOpenSquareBracket const &)  { throw_invalid_input(""); };
        virtual void react(InputCloseSquareBracket const &) { throw_invalid_input(""); };
        virtual void react(InputEdge const &)               { throw_invalid_input(""); };
        virtual void react(InputLabel const &)              { throw_invalid_input(""); };
        virtual void react(InputEqual const &)              { throw_invalid_input(""); };
        virtual void react(InputStringValue const &)        { throw_invalid_input(""); };
        virtual void react(InputIntValue const &)           { throw_invalid_input(""); };

        static void reset();
        void entry();  /* entry actions in some states */
        void exit();  /* no exit actions */
    
        friend std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input);
        

    protected:
        inline static SharedState shared {};
    };


    // ----------------------------------------------------------------------------
    // 3. State Declarations
    //
    class Idle : public LexemeParser {
        void react(InputGraphType const &) override;
    };

    class GraphType : public LexemeParser {
        void react(InputOpenCurlyBracket const &) override;
    };

    class OpenCurlyBracket : public LexemeParser {
        void react(InputNodeId const &) override;
        void react(InputCloseCurlyBracket const &) override;
    };

    class FromNodeID : public LexemeParser {
        void react(InputCloseCurlyBracket const&) override;
        void react(InputOpenSquareBracket const&) override;
        void react(InputEdge const&) override;
        void react(InputNodeId const&) override;
    };

    class OpenSquareBracket : public LexemeParser {
        void react(InputLabel const &) override;
    };

    class Edge : public LexemeParser {
        void react(InputNodeId const &) override;
    };

    class ToNodeID : public LexemeParser {
        void react(InputOpenSquareBracket const &) override;
        void react(InputNodeId const& ) override;
        void react(InputCloseCurlyBracket const&) override;
    };

    class Label : public LexemeParser {
        void react(InputEqual const &) override;
    };

    class Equal : public LexemeParser {
        void react(InputStringValue const &) override;
        void react(InputIntValue const &) override;
    };

    class Value : public LexemeParser {
        void react(InputCloseSquareBracket const &) override;
    };

    /**
     * @brief Parse lexemes vector into graph object
     * @param input lexemes
     * @return std::shared_ptr<common::Graph> output object 
     */
    //std::shared_ptr<common::Graph> parse(std::vector<common::Lexeme>& input);
    std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input); 
}







// #pragma once

// // standard
// #include <cstdint>
// #include <memory>
// #include <string>

// // internal
// #include <common/action-queue.hpp>
// #include <common/common.hpp>
// #include <algorithms/traversal.hpp>

// // contrib
// #include <tinyfsm.hpp>


// namespace parser {

// // parser.hpp


    
//     inline void throw_invalid_input(std::string message) {
//         throw std::runtime_error(message);
//     }

//     struct SharedState {
//         std::shared_ptr<common::Graph> graph;

//         common::ActionQueue backoffQueue;
//         common::ActionQueue actionQueue;

//         std::string fromNodeId;
//         std::string toNodeId;
//         std::string label;
//         std::string expectedValue;

//         std::string currentAttr; // ← добавь это поле
//         std::string key;

//         std::uint8_t flags = 0x0;
//         int weight = -1; 
//     };




//     // ----------------------------------------------------------------------------
//     // 1. Event Declarations
//     //
//     struct GraphEvent               : tinyfsm::Event {};
//     struct InputOpenCurlyBracket    : GraphEvent {};
//     struct InputCloseCurlyBracket   : GraphEvent {};
//     struct InputOpenSquareBracket   : GraphEvent {};
//     struct InputCloseSquareBracket  : GraphEvent {};
//     struct InputEdge                : GraphEvent {};
//     struct InputLabel               : GraphEvent {};
//     struct InputEqual               : GraphEvent {};
//     struct InputGraphType           : GraphEvent { std::string graphType; };
//     struct InputNodeId              : GraphEvent { std::string NodeID; };
//     struct InputStringValue         : GraphEvent { std::string label; };
//     struct InputIntValue            : GraphEvent { int weight; };


//     struct InputWeight {}; // ADDED

//       struct InputAttrKey : GraphEvent { //--------
//     std::string key;
// };


//     // ----------------------------------------------------------------------------
//     // 2. State Machine Base Class Declaration
//     //
//     class LexemeParser : public tinyfsm::Fsm<LexemeParser> {
//     public:
//         virtual void react(InputGraphType const &)          {};
//         virtual void react(InputOpenCurlyBracket const &)   { throw_invalid_input(""); };
//         virtual void react(InputCloseCurlyBracket const &)  { throw_invalid_input(""); };
//         virtual void react(InputNodeId const &)             { throw_invalid_input(""); };
//         virtual void react(InputOpenSquareBracket const &)  { throw_invalid_input(""); };
//         virtual void react(InputCloseSquareBracket const &) { throw_invalid_input(""); };
//         virtual void react(InputEdge const &)               { throw_invalid_input(""); };
//         virtual void react(InputLabel const &)              { throw_invalid_input(""); };
//         virtual void react(InputEqual const &)              { throw_invalid_input(""); };
//         virtual void react(InputStringValue const &)        { throw_invalid_input(""); };
//         virtual void react(InputIntValue const &)           { throw_invalid_input(""); };

//         virtual void react(InputAttrKey const &) {
//             throw_invalid_input("Unexpected InputAttrKey");
//         }
//            virtual void react(InputWeight const &) {
//        throw_invalid_input("Unexpected input of type InputWeight");
//    }

//         // void react(InputAttrKey const &) override;



//         static void reset();
//         void entry();  /* entry actions in some states */
//         void exit();  /* no exit actions */
    
//         friend std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input);
        
 
//         inline static SharedState shared {};
//     };


//     // ----------------------------------------------------------------------------
//     // 3. State Declarations

// // Добавляем новые состояния для разбора атрибутов
// struct Attributes : public LexemeParser {
//     void react(InputAttrKey const &event);
//     void react(InputEqual const &event);
//     void react(InputCloseSquareBracket const &event);
// };

// struct AttrValue : public LexemeParser {
//     void react(InputStringValue const &event);
//     void react(InputIntValue const &event);
// };







//     class Idle : public LexemeParser {
//         void react(InputGraphType const &) override;
        
//     };

//     class GraphType : public LexemeParser {
//         void react(InputOpenCurlyBracket const &) override;
//     };

//     class OpenCurlyBracket : public LexemeParser {
//         void react(InputNodeId const &) override;
//         void react(InputCloseCurlyBracket const &) override;
//     };

//     class FromNodeID : public LexemeParser {
//         void react(InputCloseCurlyBracket const&) override;
//         void react(InputOpenSquareBracket const&) override;
//         void react(InputEdge const&) override;
//         void react(InputNodeId const&) override;
//     };

//        class OpenSquareBracket : public LexemeParser {
//    public:
//        // Убедитесь, что здесь объявлены все нужные методы
//        void react(InputLabel const &) override; // для InputLabel
//        void react(InputWeight const &) override; // для InputWeight
//        void react(InputAttrKey const &) override; // для InputAttrKey, если этот метод необходим
//    };


//     class Edge : public LexemeParser {
//         void react(InputNodeId const &) override;
//     };

//     class ToNodeID : public LexemeParser {
//         void react(InputOpenSquareBracket const &) override;
//         void react(InputNodeId const& ) override;
//         void react(InputCloseCurlyBracket const&) override;
//     };

//     class Label : public LexemeParser {
//         void react(InputEqual const &) override;
//     };

//     class Equal : public LexemeParser {
//         void react(InputStringValue const &) override;
//         void react(InputIntValue const &) override;
//         void react(InputEqual const &) override;

//     };

//     class Value : public LexemeParser {
//         void react(InputCloseSquareBracket const &) override;
//     };

//     /**
//      * @brief Parse lexemes vector into graph object
//      * @param input lexemes
//      * @return std::shared_ptr<common::Graph> output object 
//      */
//     //std::shared_ptr<common::Graph> parse(std::vector<common::Lexeme>& input);
//     std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input); 
// }

// #pragma once

// // standard
// #include <cstdint>
// #include <memory>
// #include <string>

// // internal
// #include <common/action-queue.hpp>
// #include <common/common.hpp>
// #include <algorithms/traversal.hpp>

// // contrib
// #include <tinyfsm.hpp>

// namespace parser {

// // parser.hpp

// inline void throw_invalid_input(std::string message) {
//     throw std::runtime_error(message);
// }

// struct SharedState {
//     std::shared_ptr<common::Graph> graph;

//     common::ActionQueue backoffQueue;
//     common::ActionQueue actionQueue;

//     std::string fromNodeId;
//     std::string toNodeId;

//     std::string label;
//     bool hasLabel = false;

//     int weight = 1;
//     bool hasWeight = false;

//     std::string expectedValue;
//     std::string currentAttr; // новое поле для текущего ключа атрибута
//     std::string key;

//     std::uint8_t flags = 0x0;
// };

// // -----------------------------------------------------------------------------
// // 1. Event Declarations
// //
// struct GraphEvent               : tinyfsm::Event {};
// struct InputOpenCurlyBracket    : GraphEvent {};
// struct InputCloseCurlyBracket   : GraphEvent {};
// struct InputOpenSquareBracket   : GraphEvent {};
// struct InputCloseSquareBracket  : GraphEvent {};
// struct InputEdge                : GraphEvent {};
// struct InputLabel               : GraphEvent { std::string label; };
// struct InputEqual               : GraphEvent {};
// struct InputGraphType           : GraphEvent { std::string graphType; };
// struct InputNodeId              : GraphEvent { std::string NodeID; };
// struct InputStringValue         : GraphEvent { std::string label; };
// struct InputIntValue            : GraphEvent { int weight; };

// struct InputWeight              : GraphEvent { int weight; }; // добавлено

// struct InputAttrKey             : GraphEvent { std::string key; }; // добавлено

// // -----------------------------------------------------------------------------
// // 2. State Machine Base Class Declaration
// //
// class LexemeParser : public tinyfsm::Fsm<LexemeParser> {
// public:
//     virtual void react(InputGraphType const &)          {};
//     virtual void react(InputOpenCurlyBracket const &)   { throw_invalid_input(""); };
//     virtual void react(InputCloseCurlyBracket const &)  { throw_invalid_input(""); };
//     virtual void react(InputNodeId const &)             { throw_invalid_input(""); };
//     virtual void react(InputOpenSquareBracket const &)  { throw_invalid_input(""); };
//     virtual void react(InputCloseSquareBracket const &) { throw_invalid_input(""); };
//     virtual void react(InputEdge const &)               { throw_invalid_input(""); };
//     virtual void react(InputLabel const &)              { throw_invalid_input(""); };
//     virtual void react(InputEqual const &)              { throw_invalid_input(""); };
//     virtual void react(InputStringValue const &)        { throw_invalid_input(""); };
//     virtual void react(InputIntValue const &)           { throw_invalid_input(""); };

//     virtual void react(InputAttrKey const &) {
//         throw_invalid_input("Unexpected InputAttrKey");
//     }

//     virtual void react(InputWeight const &) {
//         throw_invalid_input("Unexpected input of type InputWeight");
//     }

//     static void reset();
//     void entry();  /* entry actions in some states */
//     void exit();   /* no exit actions */

//     friend std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input);

//     inline static SharedState shared {};
// };

// // -----------------------------------------------------------------------------
// // 3. State Declarations

// // Новые состояния для разбора атрибутов
// struct Attributes : public LexemeParser {
//     void react(InputAttrKey const &event);
//     void react(InputEqual const &event);
//     void react(InputCloseSquareBracket const &event);
// };

// struct AttrValue : public LexemeParser {
//     void react(InputStringValue const &event);
//     void react(InputIntValue const &event);
// };

// // Состояния парсера для других частей графа
// class Idle : public LexemeParser {
//     void react(InputGraphType const &) override;
// };

// class GraphType : public LexemeParser {
//     void react(InputOpenCurlyBracket const &) override;
// };

// class OpenCurlyBracket : public LexemeParser {
//     void react(InputNodeId const &) override;
//     void react(InputCloseCurlyBracket const &) override;
// };

// class FromNodeID : public LexemeParser {
//     void react(InputCloseCurlyBracket const&) override;
//     void react(InputOpenSquareBracket const&) override;
//     void react(InputEdge const&) override;
//     void react(InputNodeId const&) override;
// };

// class OpenSquareBracket : public LexemeParser {
// public:
//     void react(InputLabel const &) override;
//     void react(InputWeight const &) override;
//     void react(InputAttrKey const &) override;
// };

// class Edge : public LexemeParser {
//     void react(InputNodeId const &) override;
// };

// class ToNodeID : public LexemeParser {
//     void react(InputOpenSquareBracket const &) override;
//     void react(InputNodeId const& ) override;
//     void react(InputCloseCurlyBracket const&) override;
// };

// class Label : public LexemeParser {
//     void react(InputEqual const &) override;
// };

// class Equal : public LexemeParser {
//     void react(InputStringValue const &) override;
//     void react(InputIntValue const &) override;
//     void react(InputEqual const &) override;
// };

// class Value : public LexemeParser {
//     void react(InputCloseSquareBracket const &) override;
// };

// /**
//  * @brief Parse lexemes vector into graph object
//  * @param input lexemes
//  * @return std::shared_ptr<common::TraversalGraph> output object 
//  */
// std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input);

// } // namespace parser
